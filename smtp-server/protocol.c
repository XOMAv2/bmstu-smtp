//
// Created by Karpukhin Aleksandr on 20.02.2022.
//


#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "protocol.h"
#include "parse.h"
#include "defines.h"
#include "server-fsm.h"

static int server_retries = 0;

void set_socket_pool_entry(conn_pool_t *restrict conn_pool,
                                    size_t pos,
                                    int fd,
                                    dtor_id_t dtor_id) {
    assert(conn_pool && dtor_id >= 0);
    // Set pollfd structure.
    conn_pool->fd_pool[pos].fd = fd;
    conn_pool->fd_pool[pos].events = POLL_FD_EVENTS;
    conn_pool->fd_pool[pos].revents = 0;
    // Set connection state structure.
    conn_pool->state_pool[pos].dtor_id = dtor_id;
    conn_pool->state_pool[pos].fsm_state = SMTP_SERVER_FSM_ST_INIT;
    conn_pool->state_pool[pos].retries = 0;
}

void serve_conn_event(const pollfd_t *restrict pollfd, conn_state_t *restrict conn_state) {
    assert(conn_state);

    int rc;

    // This check must be first because when connection is closed by the
    // client side POLLIN and POLLPRI will be also set in revents field,
    // and we will be infinitely trying to read data from closed connection.
    if (pollfd->revents & POLLHUP || conn_state->retries > server_retries) {
        command_t command = {
                .sock_fd = pollfd->fd,
                .conn_state = conn_state,
                .event = SMTP_SERVER_FSM_EV_CONN_LOST
        };

        if ((rc = smtp_server_fsm_step(&command)) != OP_SUCCESS) {
            log_error_message(rc, "serve_conn_event: socket_fd=%d", pollfd->fd);
        }
    } else if (pollfd->revents & POLLIN) {
        char message_buf[MAX_MESSAGE_LENGTH];
        ssize_t message_len;
        if ((message_len = recv(pollfd->fd, message_buf, MAX_MESSAGE_LENGTH, 0)) < 0) {
            log_error_message(ERR_SOCKET_RECV, "serve_conn_event: socket_fd=%d", pollfd->fd);
            conn_state->retries++;
            return;
        }

        message_buf[message_len] = 0;
        conn_state->retries = 0;

        command_t command = {
                .sock_fd = pollfd->fd,
                .message = message_buf,
                .conn_state = conn_state
        };

        if (parse_smtp_message(&command) != OP_SUCCESS) {
            log_error_message(ERR_SMTP_INVALID_COMMAND, "serve_conn_event: socket_fd=%d", pollfd->fd);
            return;
        }

        if ((rc = smtp_server_fsm_step(&command)) != OP_SUCCESS) {
            log_error_message(rc, "serve_conn_event: socket_fd=%d", pollfd->fd);
            const char *err_msg = form_error_code(rc);
            if (send(pollfd->fd, err_msg, strlen(err_msg), 0) < 0) {
                log_error_message(ERR_SOCKET_SEND, "serve_conn_event: socket_fd=%d", pollfd->fd);
            }
            free((void*)err_msg);
        }
        return;
    } else if (pollfd->revents & POLLOUT) {

    }  else {
        log_error_message(ERR_SOCKET_POLL_POLLERR,
                          "serve_conn_event: socket_fd=%d, revents=%d",
                          pollfd->fd,
                          pollfd->revents);
        conn_state->retries++;
    }
}

void socket_dtor(int socket_fd) {
    close(socket_fd);
}

err_code_t server_init(const server_config_t *server_config, int *restrict server_socket) {
    if (server_config == NULL || server_socket == NULL) {
        return ERR_NULL_POINTER;
    }

    sockaddr_in_t server_addr;
    int local_server_socket;

    if ((local_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return ERR_SOCKET_INIT;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_config->port);
    inet_aton(server_config->host, &server_addr.sin_addr);

    if (bind(local_server_socket, (sockaddr_t *) &server_addr, sizeof(server_addr)) < 0) {
        return ERR_SOCKET_BIND;
    }

    if (listen(local_server_socket, server_config->backlog) < 0) {
        return ERR_SOCKET_LISTEN;
    }

    *server_socket = local_server_socket;
    server_retries = server_config->retries;

    return OP_SUCCESS;
}

err_code_t conn_init(int socket_fd, size_t pos, conn_pool_t *restrict conn_pool) {
    assert(conn_pool != NULL);

    const dtor_t sock_dtor = {.int_dtor = {.call = socket_dtor, .arg = socket_fd}, .type = INT_DTOR};
    dtor_id_t dtor_id = add_dtor(&sock_dtor);

    set_socket_pool_entry(conn_pool, pos, socket_fd, dtor_id);
    command_t command = {
        .sock_fd = socket_fd,
        .conn_state = &conn_pool->state_pool[pos],
        .event = SMTP_SERVER_FSM_EV_CONN_EST
    };

    return smtp_server_fsm_step(&command);
}

void realloc_conn_pool(conn_pool_t *restrict conn_pool) {
    assert(conn_pool && conn_pool->capacity >= 0);

    conn_pool->capacity += POOL_CHUNK_ALLOC;
    conn_pool->fd_pool = (pollfd_t *) realloc(conn_pool->fd_pool, sizeof(pollfd_t) * conn_pool->capacity);
    if (conn_pool->fd_pool == NULL) {
        exit_with_error_message(ERR_NOT_ALLOCATED, "realloc_conn_pool: fd_pool");
    }
    conn_pool->state_pool = (conn_state_t *) realloc(conn_pool->state_pool, sizeof(conn_state_t) * conn_pool->capacity);
    if (conn_pool->state_pool == NULL) {
        exit_with_error_message(ERR_NOT_ALLOCATED, "realloc_conn_pool: state_pool");
    }
}

err_code_t accept_conn(conn_pool_t *restrict conn_pool) {
    assert(conn_pool);

    size_t socket_pool_pos = conn_pool->size;
    sockaddr_in_t client_addr;
    socklen_t addr_len;
    int client_socket = accept(conn_pool->fd_pool[SERVER_SOCKET_IDX].fd, (sockaddr_t *) &client_addr, &addr_len);

    if (client_socket < 0) {
        log_error_code(ERR_SOCKET_ACCEPT);
        return OP_SUCCESS;
    }

    if (conn_pool->size == conn_pool->capacity) {  // if pool is full
        int i = 1;
        for (; i < conn_pool->size && conn_pool->fd_pool[i].fd != -1; i++);
        if (i < conn_pool->size) { // If there is free pos in allocated pool
            socket_pool_pos = i;
        } else { // Else reallocate pool
            realloc_conn_pool(conn_pool);
        }
    }

    // Init new connection and increment connection pool size only in case if
    // initialization is successful and connection pool entry is not reused.
    int rc = conn_init(client_socket, socket_pool_pos, conn_pool);
    if (rc == OP_SUCCESS && socket_pool_pos == conn_pool->size) {
        conn_pool->size++;
    }

    return rc;
}

_Noreturn void serve(int server_socket) {
    int rc = OP_SUCCESS;

    // Precompile all regex patterns
    compile_regexes();

    // Initialize socket pool with server socket
    conn_pool_t conn_pool = {
            .fd_pool = NULL,
            .state_pool = NULL,
            .size = 1,
            .capacity = 0
    };
    realloc_conn_pool(&conn_pool);

    const dtor_t server_socket_dtor = {.int_dtor = {.call = socket_dtor, .arg = server_socket}, .type = INT_DTOR};
    dtor_id_t dtor_id = add_dtor(&server_socket_dtor);
    set_socket_pool_entry(&conn_pool, SERVER_SOCKET_IDX, server_socket, dtor_id);

    // Infinite serve loop
    while (true) {
        for (int i = 0; i < conn_pool.size; i++) {
            printf("%d ", conn_pool.fd_pool[i].fd);
        }
        printf("\n");
        int ready_count = poll(conn_pool.fd_pool, conn_pool.size, DEFAULT_POLL_TIMEOUT_MS);
        if (ready_count < 0) {
            log_error_code(ERR_SOCKET_POLL);
        }

        if (ready_count <= 0) {
            continue;
        }

        // If server socket is ready - accept new connection
        if (conn_pool.fd_pool[SERVER_SOCKET_IDX].revents) {
            if ((rc = accept_conn(&conn_pool)) != OP_SUCCESS) {
                log_error_message(rc, "serve: accept connection");
            }
            ready_count--;
        }

        // Perform corresponding actions on all ready sockets
        for (int i = 1; i < conn_pool.size && ready_count > 0; i++) {
            if (conn_pool.fd_pool[i].revents) {
                conn_state_t *conn_state = &conn_pool.state_pool[i];
                pollfd_t *pollfd = &conn_pool.fd_pool[i];
                serve_conn_event(pollfd, conn_state);
                conn_pool.fd_pool[i].revents = 0;

                if (conn_state->fsm_state == SMTP_SERVER_FSM_ST_DONE) {
                    if (close(pollfd->fd) < 0) {
                        log_error_code(ERR_SOCKET_ACCEPT);
                    }

                    delete_dtor(conn_state->dtor_id);   // delete socket dtor because socket already closed
                    pollfd->fd = -1;         // invalidate poll fd to force poll ignore this socket
                    conn_state->dtor_id = -1;           // invalidate socket dtor id to prevent id reuse
                }

                ready_count--;
            }
        }
    }
}
