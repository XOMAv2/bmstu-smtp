//
// Created by Karpukhin Aleksandr on 20.02.2022.
//

#ifndef NETWORK_PROTOCOLS_CP_PROTOCOL_H
#define NETWORK_PROTOCOLS_CP_PROTOCOL_H

#include <poll.h>

#include "server-fsm.h"
#include "config.h"
#include "error.h"
#include "destruction.h"

#define SERVER_SOCKET_IDX 0

typedef struct pollfd pollfd_t;
typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;

typedef struct {
    pollfd_t pollfd;
    dtor_id_t dtor_id;
    conn_state_t conn_state;
} socket_pool_entry_t;

typedef struct {
    socket_pool_entry_t *pool;
    size_t size;
    size_t capacity;
} socket_pool_t;

// Serve new external event on specified connection and return new connection state
err_code_t serve_conn_event(pollfd_t pollfd, conn_state_t *restrict conn_state, char * restrict err_msg_buf);

// Create new server socket, bind with address specified in server config and listen for incoming client connections
err_code_t server_init(const server_config_t *server_config, int *restrict server_socket);

// Main server function - poll all sockets (client and server) for events and perform corresponding actions:
// - for server socket - establish new connection with client and init all corresponding service data structures
// - for each client socket - receive data from existing connection and perform necessary actions with/without response
err_code_t serve(int server_socket, char *restrict err_msg);

#endif //NETWORK_PROTOCOLS_CP_PROTOCOL_H
