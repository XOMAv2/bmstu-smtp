//
// Created by Karpukhin Aleksandr on 30.01.2022.
//
// Wrapper for logger defined "log.h" that allows to
// log some predefined errors with additional text data.
// ALso provide additional interface for critical errors
// processing. Such functions starts with 'exit' prefix and
// perform exit() call with specified error code and preliminary
// logging to standard logger. If logger is unavailable,
// corresponding data will be written to the stderr.
//

#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <regex.h>
#include "errno.h"

#include "log.h"
#include "defines.h"
#include "destruction.h"

void print_to_buf(char *restrict buf, const char *prefix, const char *format, va_list* va_args) {
    const char *new_prefix = (prefix == NULL ? ERR_NO_ERR_MESSAGE_MESSAGE : prefix);

    if (format == NULL) {
        snprintf(buf, ERROR_MESSAGE_MAX_LENGTH, "%d %s", errno, new_prefix);
    } else if (va_args == NULL) {
        snprintf(buf, ERROR_MESSAGE_MAX_LENGTH, "%d %s: %s", errno, new_prefix, format);
    } else {
        snprintf(buf, ERROR_MESSAGE_MAX_LENGTH, "%d %s: ", errno, new_prefix);
        vsnprintf(buf + strlen(buf), ERROR_MESSAGE_MAX_LENGTH - strlen(buf), format, *va_args);
    }
}

const char *log_error_raw(err_code_t code, const char *format, va_list* va_args) {
    char *buf = calloc(ERROR_MESSAGE_MAX_LENGTH, sizeof(char));
    if (buf == NULL) {
        exit(ERR_NOT_ALLOCATED);
    }

    errno = code;

    switch (code) {
        case ERR_NOT_ALLOCATED:
            print_to_buf(buf, ERR_NOT_ALLOCATED_MESSAGE, format, va_args);
            break;
        case ERR_OUT_OF_RANGE:
            print_to_buf(buf, ERR_OUT_OF_RANGE_MESSAGE, format, va_args);
            break;
        case ERR_NULL_POINTER:
            print_to_buf(buf, ERR_NULL_POINTER_MESSAGE, format, va_args);
            break;
        case ERR_CREATE_THREAD:
            print_to_buf(buf, ERR_CREATE_THREAD_MESSAGE, format, va_args);
            break;
        case ERR_INIT_MUTEX:
            print_to_buf(buf, ERR_INIT_MUTEX_MESSAGE, format, va_args);
            break;
        case ERR_PARSE_CONFIG:
            print_to_buf(buf, ERR_PARSE_CONFIG_MESSAGE, format, va_args);
            break;
        case ERR_PARSE_SERVER_CONFIG:
            print_to_buf(buf, ERR_PARSE_SERVER_CONFIG_MESSAGE, format, va_args);
            break;
        case ERR_PARSE_LOGGER_CONFIG:
            print_to_buf(buf, ERR_PARSE_LOGGER_CONFIG_MESSAGE, format, va_args);
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
        case ERR_REGEX_INVALID_MAIL:
            print_to_buf(buf, ERR_REGEX_INVALID_MAIL_MESSAGE, format, va_args);
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
            break;
        case ERR_SOCKET_POLL_POLLERR:
            print_to_buf(buf, ERR_SOCKET_POLL_POLLERR_MESSAGE, format, va_args);
            break;
        case ERR_SOCKET_RECV:
            print_to_buf(buf ,ERR_SOCKET_RECV_MESSAGE, format, va_args);
            break;
        case ERR_SMTP_INVALID_COMMAND:
            print_to_buf(buf, ERR_SMTP_INVALID_COMMAND_MESSAGE, format, va_args);
            break;
        case ERR_INVALID_ARGV:
            print_to_buf(buf, ERR_INVALID_ARGV_MESSAGE, format, va_args);
            break;
        case ERR_UNKNOWN_ENUM_VALUE:
            print_to_buf(buf, ERR_UNKNOWN_ENUM_VALUE_MESSAGE, format, va_args);
            break;
        case ERR_SMTP_BAD_SEQ:
            print_to_buf(buf, ERR_SMTP_BAD_SEQ_MESSAGE, format, va_args);
            break;
        case ERR_UNKNOWN:
            print_to_buf(buf, ERR_UNKNOWN_MESSAGE, format, va_args);
            break;
        default:
            print_to_buf(buf, format, NULL,  NULL);
    }

    return buf;
}

void log_error_code(err_code_t code) {
    log_error_raw(code, NULL, NULL);
}

void log_error_message(err_code_t code, const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    const char *err_msg = log_error_raw(code, format, &va_args);

    if (log_error("%s", err_msg) < 0) {
        fprintf(stderr, "\n%s\n", err_msg);
    }

    va_end(va_args);
}

void exit_with_error_message(err_code_t code, const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    const char *err_msg = log_error_raw(code, format, &va_args);

    if (log_error("%s", err_msg) < 0) {
        fprintf(stderr, "\n%s\n", err_msg);
    }

    va_end(va_args);
    exec_all_dtors();
    exit(code);
}

const char *form_error_message(err_code_t code, const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    const char *err_msg = log_error_raw(code, format, &va_args);
    va_end(va_args);

    return err_msg;
}

const char *form_error_code(err_code_t code) {
    return form_error_message(code, NULL);
}

void exit_with_error_code(err_code_t code) {
    exit_with_error_message(code, NULL);
}

void exit_with_config_error(err_code_t err_code, const config_t *config, const char * filename) {
    const char *text = config_error_text(config);
    int line = config_error_line(config);
    char buf[ERROR_MESSAGE_MAX_LENGTH];

    snprintf(buf, ERROR_MESSAGE_MAX_LENGTH, "%s:%d - %s", filename, line, text);
    exit_with_error_message(err_code, buf);
}

