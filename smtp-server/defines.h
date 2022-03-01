//
// Created by Karpukhin Aleksandr on 14.02.2022.
//

#ifndef NETWORK_PROTOCOLS_CP_DEFINES_H
#define NETWORK_PROTOCOLS_CP_DEFINES_H

#include <sys/poll.h>

#define OP_SUCCESS 0

#define ERROR_MESSAGE_MAX_LENGTH 1000
#define MAX_MESSAGE_LENGTH 1024
#define DEFAULT_CONFIG_PATH "../../smtp-server/server.cfg"

#define TRUE 1
#define FALSE 0

#define POOL_CHUNK_ALLOC 10

#define POLL_FD_EVENTS POLLIN | POLLPRI | POLLOUT | POLLERR

#define DEFAULT_POLL_TIMEOUT_MS 500

#define MAX_RETRIES 10
#define SLEEP_BEFORE_RETRY_MS 200

#endif //NETWORK_PROTOCOLS_CP_DEFINES_H
