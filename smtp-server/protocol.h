//
// Created by Karpukhin Aleksandr on 20.02.2022.
//

#ifndef NETWORK_PROTOCOLS_CP_PROTOCOL_H
#define NETWORK_PROTOCOLS_CP_PROTOCOL_H

#include <poll.h>

#include "data.h"
#include "config.h"
#include "error.h"

#define SERVER_SOCKET_IDX 0

typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;

typedef struct {
    conn_state_t *pool;
    size_t size;
    size_t capacity;
} conn_pool_t;

// Serve new external event on specified connection and return new connection state
void serve_conn_event(conn_state_t *restrict conn_state);

// Create new server socket, bind with address specified in server config and listen for incoming client connections
err_code_t server_init(const server_config_t *server_config, int *restrict server_socket);

// Main server function - poll all sockets (client and server) for events and perform corresponding actions:
// - for server socket - establish new connection with client and init all corresponding service data structures
// - for each client socket - receive data from existing connection and perform necessary actions with/without response
// Endless loop that never returns, all not critical errors are logged, all critical errors in serve() and all child called
// functions lead to exit() with error code (this is primarily allocation errors, that can't be fixed).
void serve(int server_socket);

#endif //NETWORK_PROTOCOLS_CP_PROTOCOL_H
