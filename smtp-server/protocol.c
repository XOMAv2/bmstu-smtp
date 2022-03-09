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

conn_state_t socket_pool_entry_ctor(int fd, short events, dtor_id_t dtor_id) {
    assert(dtor_id >= 0);

    conn_state_t state = {.pollfd = {.fd = fd, .events = events, .revents = 0}, .dtor_id = dtor_id};
    return state;
}

void serve_conn_event(conn_state_t *restrict conn_state) {
    assert(conn_state);

    pollfd_t pollfd = conn_state->pollfd;
    int sock_fd = pollfd.fd;
    int rc;

    if (pollfd.revents & POLLIN) {
        char message_buf[MAX_MESSAGE_LENGTH];
        if (recv(sock_fd, message_buf, MAX_MESSAGE_LENGTH, 0) < 0) {
            log_error_message(ERR_SOCKET_RECV, "serve_conn_event: socket_fd=%d", sock_fd);
            return;
        }

        command_t command = { .message = message_buf, .conn_state = conn_state };

        if (parse_smtp_message(&command) != OP_SUCCESS) {
            log_error_message(ERR_SMTP_INVALID_COMMAND, "serve_conn_event: socket_fd=%d", sock_fd);
            return;
        }

        if ((rc = smtp_server_fsm_step(&command)) != OP_SUCCESS) {
            log_error_message(rc, "serve_conn_event: socket_fd=%d", sock_fd);
        }

    } else if (pollfd.revents & POLLHUP) {
        command_t command = { .conn_state = conn_state, .event = SMTP_SERVER_FSM_EV_CONN_LOST };
        if ((rc = smtp_server_fsm_step(&command)) != OP_SUCCESS) {
            log_error_message(rc, "serve_conn_event: socket_fd=%d", sock_fd);
        }
    } else {
        log_error_message(ERR_SOCKET_POLL_POLLERR, "serve_conn_event: socket_fd=%d", sock_fd);
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

    return OP_SUCCESS;
}

err_code_t socket_init(int socket_fd, size_t pos, conn_state_t *restrict conn_pool) {
    assert(conn_pool != NULL);

    const dtor_t sock_dtor = {.int_dtor = {.call = socket_dtor, .arg = socket_fd}, .type = INT_DTOR};
    dtor_id_t dtor_id = add_dtor(&sock_dtor);

    conn_pool[pos] = socket_pool_entry_ctor(socket_fd, POLL_FD_EVENTS, dtor_id);
    conn_pool[pos].fsm_state = SMTP_SERVER_FSM_ST_INIT;
    command_t command = {
        .conn_state = &conn_pool[pos],
        .event = SMTP_SERVER_FSM_EV_CONN_EST
    };

    return smtp_server_fsm_step(&command);
}

err_code_t accept_conn(conn_pool_t *restrict conn_pool) {
    assert(conn_pool);

    size_t socket_pool_pos = conn_pool->size;
    sockaddr_in_t client_addr;
    socklen_t addr_len;
    int client_socket = accept(conn_pool->pool[SERVER_SOCKET_IDX].pollfd.fd, (sockaddr_t *) &client_addr, &addr_len);

    if (client_socket < 0) {
        log_error_code(ERR_SOCKET_ACCEPT);
        return OP_SUCCESS;
    }

    if (conn_pool->size == conn_pool->capacity) {  // if pool is full
        int i = 0;
        for (; i < conn_pool->size && conn_pool->pool[i].pollfd.fd != -1; i++);
        if (i < conn_pool->size) { // If there is free pos in allocated pool
            socket_pool_pos = i;
        } else { // Else reallocate pool
            conn_pool->capacity += POOL_CHUNK_ALLOC;
            conn_pool->pool = (conn_state_t *) realloc(conn_pool->pool, sizeof(conn_state_t) * conn_pool->capacity);
            if (conn_pool->pool == NULL) {
                exit_with_error_message(ERR_NOT_ALLOCATED, "accept_conn: sock_fd=%d", client_socket);
            }
            conn_pool->size++;
        }
    }

    return socket_init(client_socket, socket_pool_pos, conn_pool->pool);
}

_Noreturn void serve(int server_socket) {
    int rc = OP_SUCCESS;

    // Precompile all regex patterns
    compile_regexes();

    // Initialize socket pool with server socket
    conn_pool_t conn_pool = { .pool = NULL, .size = 1, .capacity = POOL_CHUNK_ALLOC };
    conn_pool.pool = (conn_state_t *) realloc(conn_pool.pool, sizeof(conn_state_t) * conn_pool.capacity);
    if (conn_pool.pool == NULL) {
        exit_with_error_message(ERR_NOT_ALLOCATED, "serve: socket pool");
    }

    const dtor_t server_socket_dtor = {.int_dtor = {.call = socket_dtor, .arg = server_socket}, .type = INT_DTOR};
    dtor_id_t dtor_id = add_dtor(&server_socket_dtor);
    conn_pool.pool[SERVER_SOCKET_IDX] = socket_pool_entry_ctor(server_socket, POLL_FD_EVENTS, dtor_id);

    // Infinite serve loop
    while (true) {
        int ready_count = poll((pollfd_t *) conn_pool.pool, conn_pool.size, DEFAULT_POLL_TIMEOUT_MS);
        if (ready_count < 0) {
            log_error_code(ERR_SOCKET_POLL);
        }

        if (ready_count <= 0) {
            continue;
        }

        // If server socket is ready - accept new connection
        if (conn_pool.pool[SERVER_SOCKET_IDX].pollfd.revents) {
            if ((rc = accept_conn(&conn_pool)) != OP_SUCCESS) {
                log_error_message(rc, "serve: accept connection");
            }
            ready_count--;
        }

        // Perform corresponding actions on all ready sockets
        for (int i = 1; i < conn_pool.size && ready_count > 0; i++) {
            if (conn_pool.pool[i].pollfd.revents) {
                conn_state_t *conn_state = &conn_pool.pool[i];
                serve_conn_event(conn_state);

                if (conn_state->fsm_state == SMTP_SERVER_FSM_ST_DONE) {
                    if (close(conn_state->pollfd.fd) < 0) {
                        log_error_code(ERR_SOCKET_ACCEPT);
                    }

                    delete_dtor(conn_state->dtor_id);   // delete socket dtor because socket already closed
                    conn_state->pollfd.fd = -1;         // invalidate poll fd to force poll ignore this socket
                    conn_state->dtor_id = -1;           // invalidate socket dtor id to prevent id reuse
                }

                ready_count--;
            }
        }
    }
}
