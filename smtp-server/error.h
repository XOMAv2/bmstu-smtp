//
// Created by Karpukhin Aleksandr on 30.01.2022.
//

#ifndef LAB01_ERROR_H
#define LAB01_ERROR_H

#include <libconfig.h>

// Resources access and allocation errors
#define ERR_NOT_ALLOCATED             -100
#define ERR_OUT_OF_RANGE              -101
#define ERR_NULL_POINTER              -102
#define ERR_CREATE_THREAD             -103
#define ERR_INIT_MUTEX                -104

// Parsing errors
#define ERR_PARSE_CONFIG              -200
#define ERR_PARSE_PORT                -201
#define ERR_PARSE_PROC_NUM            -202
#define ERR_PARSE_BACKLOG             -203
#define ERR_REGEX_COMPILE             -204
#define ERR_REGEX_NOMATCH             -205
#define ERR_REGEX_NO_DOMAIN           -206
#define ERR_REGEX_NO_CRLF             -207
#define ERR_REGEX_NO_FROM             -208
#define ERR_REGEX_NO_REVERSE_PATH     -209
#define ERR_REGEX_INVALID_MAIL_PARAMS -210

// Socket ops errors
#define ERR_SOCKET_INIT               -300
#define ERR_SOCKET_BIND               -301
#define ERR_SOCKET_LISTEN             -302
#define ERR_SOCKET_ACCEPT             -303
#define ERR_SOCKET_POLL               -304
#define ERR_SOCKET_POLL_POLLERR       -305
#define ERR_SOCKET_RECV               -306

// SMTP errors
#define ERR_SMTP_INVALID_COMMAND      -400


#define ERR_INVALID_ARGV              -500
#define ERR_NO_ERR_MESSAGE            -501

#define ERR_UNKNOWN                   -600

#define ERR_NOT_ALLOCATED_MESSAGE               "error during memory (re)allocation"
#define ERR_OUT_OF_RANGE_MESSAGE                "going beyond the memory area bound during indexing"
#define ERR_NULL_POINTER_MESSAGE                "attempt to access memory by null pointer"
#define ERR_CREATE_THREAD_MESSAGE               "error during new thread creation"
#define ERR_INIT_MUTEX_MESSAGE                  "error during mutex initialization"

#define ERR_PARSE_CONFIG_MESSAGE                "error during application configuration parsing"
#define ERR_PARSE_PORT_MESSAGE                  "socket port must be in range [0, 65353]"
#define ERR_PARSE_PROC_NUM_MESSAGE              "proc num must be in range [0, 1024]"
#define ERR_PARSE_BACKLOG_MESSAGE               "backlog size must be strict positive int value"
#define ERR_REGEX_COMPILE_MESSAGE               "error during regular expression compilation"
#define ERR_REGEX_NOMATCH_MESSAGE               "no any regular expression matches found"
#define ERR_REGEX_NO_DOMAIN_MESSAGE             "no domain found in received message"
#define ERR_REGEX_NO_CRLF_MESSAGE               "no CRLF sequence found at the end of the message"
#define ERR_REGEX_NO_FROM_MESSAGE               "no FROM section in MAIL message"
#define ERR_REGEX_NO_REVERSE_PATH_MESSAGE       "no reverse-path section in MAIL message"
#define ERR_REGEX_INVALID_MAIL_PARAMS_MESSAGE   "mail params sequence not matches with pattern"

#define ERR_SOCKET_INIT_MESSAGE                 "can't create server socket"
#define ERR_SOCKET_BIND_MESSAGE                 "can't bind server socket"
#define ERR_SOCKET_LISTEN_MESSAGE               "can't listen on server socket"
#define ERR_SOCKET_ACCEPT_MESSAGE               "error during server socket accept call"
#define ERR_SOCKET_POLL_MESSAGE                 "error during poll call on client sockets"
#define ERR_SOCKET_POLL_POLLERR_MESSAGE         "poll call set POLLERR in revents"
#define ERR_SOCKET_RECV_MESSAGE                 "error while reading message from client socket"

#define ERR_SMTP_INVALID_COMMAND_MESSAGE        "invalid smtp command code received"

#define ERR_INVALID_ARGV_MESSAGE                "invalid command line arguments"
#define ERR_NO_ERR_MESSAGE_MESSAGE              "no error message was specified"

#define ERR_UNKNOWN_MESSAGE                     "unknown error code received"

typedef int err_code_t;

void log_error_code(err_code_t code);
void log_error_message(err_code_t code, const char *format, ...);

void exit_with_error_code(err_code_t code);
void exit_with_error_message(err_code_t code, const char *format, ...);
void exit_with_config_error(const config_t *config, const char * filename);

#endif //LAB01_ERROR_H
