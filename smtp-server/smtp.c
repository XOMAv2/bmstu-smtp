//
// Created by Karpukhin Aleksandr on 20.02.2022.
//


#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "smtp.h"
#include "parse.h"
#include "defines.h"
#include "destruction.h"

socket_pool_entry_t socket_pool_entry_ctor(int fd, short events, dtor_id_t dtor_id) {
    assert(dtor_id >= 0);

    socket_pool_entry_t entry = {.pollfd = {.fd = fd, .events = events, .revents = 0}, .dtor_id = dtor_id};
    return entry;
}

err_code_t serve_conn_event(pollfd_t pollfd, conn_state_t *restrict conn_state, char *restrict err_msg_buf) {
    if (pollfd.revents & POLLIN) {
        char message_buf[MAX_MESSAGE_LENGTH];
        if (recv(pollfd.fd, message_buf, MAX_MESSAGE_LENGTH, 0) < 0) {
            snprintf(err_msg_buf, ERROR_MESSAGE_MAX_LENGTH, "socket_fd=%d", pollfd.fd);
            return ERR_SOCKET_RECV;
        }

        command_t command;

        if (parse_smtp_message(message_buf, &command) != OP_SUCCESS) {
            snprintf(err_msg_buf, ERROR_MESSAGE_MAX_LENGTH, "socket_fd=%d", pollfd.fd);
            return ERR_SMTP_INVALID_COMMAND;
        }

        conn_state->fsm_state = smtp_server_fsm_step(conn_state->fsm_state, command.event, conn_state, command.data);

    } else if (pollfd.revents & POLLHUP) {
        return smtp_server_fsm_step(conn_state->fsm_state, SMTP_SERVER_FSM_EV_CONN_LOST, conn_state, NULL);
    } else if (pollfd.revents & POLLERR) {
        snprintf(err_msg_buf, ERROR_MESSAGE_MAX_LENGTH, "socket_fd=%d", pollfd.fd);
        return ERR_SOCKET_POLL_POLLERR;
    }

    return OP_SUCCESS;
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

void socket_init(int socket_fd, size_t pos, socket_pool_entry_t *restrict socket_pool) {
    assert(socket_pool != NULL);

    const dtor_t sock_dtor = {.int_dtor = {.call = socket_dtor, .arg = socket_fd}, .type = INT_DTOR};
    dtor_id_t dtor_id = add_dtor(&sock_dtor);

    socket_pool[pos] = socket_pool_entry_ctor(socket_fd, POLL_FD_EVENTS, dtor_id);
    socket_pool[pos].conn_state.fsm_state = smtp_server_fsm_step(SMTP_SERVER_FSM_ST_INIT,
                                                                 SMTP_SERVER_FSM_EV_CONN_EST,
                                                                 &socket_pool[pos].conn_state,
                                                                 NULL);
}

err_code_t accept_conn(socket_pool_t *restrict socket_pool, char *restrict err_msg_buf) {
    assert(socket_pool != NULL && err_msg_buf != NULL);

    sockaddr_in_t client_addr;
    socklen_t addr_len;
    int client_socket;
    size_t socket_pool_pos = socket_pool->size;

    if ((client_socket = accept(socket_pool->pool[0].pollfd.fd, (sockaddr_t *) &client_addr, &addr_len)) < 0) {
        log_error_code(ERR_SOCKET_ACCEPT);
    } else {
        if (socket_pool->size == socket_pool->capacity) {  // if pool is full
            int i = 0;
            for (; i < socket_pool->size && socket_pool->pool[i].pollfd.fd != -1; i++);
            if (i < socket_pool->size) { // If there is free pos in allocated pool
                socket_pool_pos = i;
            } else { // Else reallocate pool
                socket_pool->capacity += POOL_CHUNK_ALLOC;

                if ((socket_pool->pool = (socket_pool_entry_t *) realloc(
                        socket_pool->pool, sizeof(socket_pool_entry_t) * socket_pool->capacity)) == NULL) {
                    snprintf(err_msg_buf, ERROR_MESSAGE_MAX_LENGTH, "can't extend memory for client sockets pool");
                    return ERR_NOT_ALLOCATED;
                }

                socket_pool->size++;
            }
        }

        socket_init(client_socket, socket_pool_pos, socket_pool->pool);
    }
}

err_code_t serve(int server_socket, char *restrict err_msg_buf) {
    if (err_msg_buf == NULL) {
        return ERR_NULL_POINTER;
    }

    int rc = OP_SUCCESS;

    // Precompile all regex patterns
    const dtor_t regexes_dtor = {.int_dtor = {.call = free_regexes, .arg = CMD_NUM}, .type = INT_DTOR};
    compile_regexes();
    add_dtor(&regexes_dtor);

    // Initialize socket pool with server socket
    socket_pool_t socket_pool = { .pool = NULL, .size = 1, .capacity = POOL_CHUNK_ALLOC };
    if ((socket_pool.pool = (socket_pool_entry_t *) realloc(
            socket_pool.pool, sizeof(socket_pool_entry_t) * socket_pool.capacity)) == NULL) {
        snprintf(err_msg_buf, ERROR_MESSAGE_MAX_LENGTH, "can't extend memory for client sockets pool");
        return ERR_NOT_ALLOCATED;
    }
    const dtor_t server_socket_dtor = {.int_dtor = {.call = socket_dtor, .arg = server_socket}, .type = INT_DTOR};
    dtor_id_t dtor_id = add_dtor(&server_socket_dtor);
    socket_pool.pool[SERVER_SOCKET_IDX] = socket_pool_entry_ctor(server_socket, POLL_FD_EVENTS, dtor_id);

    // Infinite serve loop
    while (TRUE) {
        int ready_count = poll((pollfd_t *) socket_pool.pool, socket_pool.size, DEFAULT_POLL_TIMEOUT_MS);
        if (ready_count < 0) {
            log_error_code(ERR_SOCKET_POLL);
        }

        if (ready_count <= 0) {
            continue;
        }

        // If server socket is ready - accept new connection
        if (socket_pool.pool[SERVER_SOCKET_IDX].pollfd.revents) {
            if ((rc = accept_conn(&socket_pool, err_msg_buf)) != OP_SUCCESS) {
                return rc;
            }
            ready_count--;
        }

        // Perform corresponding actions on all ready sockets
        for (int i = 1; i < socket_pool.size && ready_count > 0; i++) {
            if (socket_pool.pool[i].pollfd.revents) {
                socket_pool_entry_t *socket_pool_entry = &socket_pool.pool[i];
                // char err_msg_buf[ERROR_MESSAGE_MAX_LENGTH] = {0};
                rc = serve_conn_event(socket_pool_entry->pollfd, &socket_pool_entry->conn_state, err_msg_buf);

                if (rc != OP_SUCCESS) {
                    log_error_message(rc, err_msg_buf);
                } else if (socket_pool_entry->conn_state.fsm_state == SMTP_SERVER_FSM_ST_DONE) {
                    if (close(socket_pool_entry->pollfd.fd) < 0) {
                        log_error_code(ERR_SOCKET_ACCEPT);
                    }

                    delete_dtor(socket_pool_entry->dtor_id);   // delete socket dtor because socket already closed
                    socket_pool_entry->pollfd.fd = -1;         // invalidate poll fd to force poll ignore this socket
                    socket_pool_entry->dtor_id = -1;           // invalidate socket dtor id to prevent id reuse
                }

                ready_count--;
            }
        }
    }
}
