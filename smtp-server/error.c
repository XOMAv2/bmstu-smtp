//
// Created by Karpukhin Aleksandr on 30.01.2022.
//

#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <regex.h>
#include "errno.h"

#include "defines.h"
#include "destruction.h"

void print_to_buf(char *restrict buf, const char *prefix, const char *format, const va_list* va_args) {
    const char *new_prefix = (prefix == NULL ? ERR_NO_ERR_MESSAGE_MESSAGE : prefix);

    if (format == NULL) {
        snprintf(buf, ERROR_MESSAGE_MAX_LENGTH, "%s", new_prefix);
    } else if (va_args == NULL) {
        snprintf(buf, ERROR_MESSAGE_MAX_LENGTH, "%s: %s", new_prefix, format);
    } else {
        snprintf(buf, ERROR_MESSAGE_MAX_LENGTH, "%s: ", new_prefix);
        vsnprintf(buf + strlen(buf), ERROR_MESSAGE_MAX_LENGTH - strlen(buf), format, *va_args);
    }
}

void log_error(err_code_t code, const char *format, const va_list* va_args) {
    char buf[ERROR_MESSAGE_MAX_LENGTH];

    switch (code) {
        case ERR_NOT_ALLOCATED:
            print_to_buf(buf, ERR_NOT_ALLOCATED_MESSAGE, format, va_args);
            break;
        case ERR_OUT_OF_RANGE:
            print_to_buf(buf, ERR_OUT_OF_RANGE_MESSAGE, format, va_args);
            break;
        case ERR_INVALID_ARGV:
            print_to_buf(buf, ERR_INVALID_ARGV_MESSAGE, format, va_args);
            break;
        case ERR_NULL_POINTER:
            print_to_buf(buf, ERR_NULL_POINTER_MESSAGE, format, va_args);
            break;
        case ERR_PARSE_CONFIG:
            print_to_buf(buf, ERR_PARSE_CONFIG_MESSAGE, format, va_args);
            break;
        case ERR_PARSE_PORT:
            print_to_buf(buf, ERR_PARSE_PORT_MESSAGE, format, va_args);
            break;
        case ERR_PARSE_PROC_NUM:
            print_to_buf(buf, ERR_PARSE_PROC_NUM_MESSAGE, format, va_args);
            break;
        case ERR_PARSE_BACKLOG:
            print_to_buf(buf, ERR_PARSE_BACKLOG_MESSAGE, format, va_args);
            break;
        case ERR_REGEX_COMPILE:
            print_to_buf(buf, ERR_REGEX_COMPILE_MESSAGE, format, va_args);
            break;
        case ERR_REGEX_NOMATCH:
            print_to_buf(buf, ERR_REGEX_NOMATCH_MESSAGE, format, va_args);
            break;
        case ERR_REGEX_NO_DOMAIN:
            print_to_buf(buf, ERR_REGEX_NO_DOMAIN_MESSAGE, format, va_args);
            break;
        case ERR_REGEX_NO_CRLF:
            print_to_buf(buf, ERR_REGEX_NO_CRLF_MESSAGE, format, va_args);
            break;
        case ERR_REGEX_NO_FROM:
            print_to_buf(buf, ERR_REGEX_NO_FROM_MESSAGE, format, va_args);
            break;
        case ERR_REGEX_NO_REVERSE_PATH:
            print_to_buf(buf, ERR_REGEX_NO_REVERSE_PATH_MESSAGE, format, va_args);
            break;
        case ERR_REGEX_INVALID_MAIL_PARAMS:
            print_to_buf(buf, ERR_REGEX_INVALID_MAIL_PARAMS_MESSAGE, format, va_args);
            break;
        case ERR_SOCKET_INIT:
            print_to_buf(buf, ERR_SOCKET_INIT_MESSAGE, format, va_args);
            break;
        case ERR_SOCKET_BIND:
            print_to_buf(buf, ERR_SOCKET_BIND_MESSAGE, format, va_args);
            break;
        case ERR_SOCKET_LISTEN:
            print_to_buf(buf, ERR_SOCKET_LISTEN_MESSAGE, format, va_args);
            break;
        case ERR_SOCKET_ACCEPT:
            print_to_buf(buf, ERR_SOCKET_ACCEPT_MESSAGE, format, va_args);
            break;
        case ERR_SOCKET_POLL:
            print_to_buf(buf, ERR_SOCKET_POLL_MESSAGE, format, va_args);
        case ERR_SOCKET_POLL_POLLERR:
            print_to_buf(buf, ERR_SOCKET_POLL_POLLERR_MESSAGE, format, va_args);
        case ERR_SOCKET_RECV:
            print_to_buf(buf ,ERR_SOCKET_RECV_MESSAGE, format, va_args);
        case ERR_SMTP_INVALID_COMMAND:
            print_to_buf(buf, ERR_SMTP_INVALID_COMMAND_MESSAGE, format, va_args);
        case ERR_UNKNOWN:
            print_to_buf(buf, ERR_UNKNOWN_MESSAGE, format, va_args);
            break;
        default:
            print_to_buf(buf, format, NULL,  NULL);
    }

    errno = code;
    fprintf(stderr, "\n%s\n", buf);
}

void log_error_code(err_code_t code) {
    log_error(code, NULL, NULL);
}

void log_error_message(err_code_t code, const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    log_error(code, format, &va_args);
    va_end(va_args);
}

void exit_with_error_message(err_code_t code, const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    log_error(code, format, &va_args);
    va_end(va_args);

    exec_all_dtors();
    exit(code);
}

void exit_with_error_code(err_code_t code) {
    exit_with_error_message(code, NULL);
}

void exit_with_config_error(const config_t *config, const char * filename) {
    const char *text = config_error_text(config);
    int line = config_error_line(config);
    char buf[ERROR_MESSAGE_MAX_LENGTH];

    snprintf(buf, ERROR_MESSAGE_MAX_LENGTH, "%s:%d - %s", filename, line, text);
    exit_with_error_message(ERR_PARSE_CONFIG, buf);
}

