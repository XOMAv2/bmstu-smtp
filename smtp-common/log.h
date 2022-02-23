//
// Created by Никита Иксарица on 21.02.2022.
//

#ifndef SMTP_CLIENT_LOG_H
#define SMTP_CLIENT_LOG_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include "helpers.h"

typedef struct queue_msg
{
    // Тип сообщения. Должен принимать только положительные значения, используемые процессом-получателем для выбора
    // сообщений через функцию msgrcv.
    long mtype;
    // Содержание сообщения.
    char mtext[1024];
} queue_msg_t;

typedef enum log_level {
    ERROR,
    INFO,
    WARN,
    DEBUG
} log_level_te;

extern log_level_te current_log_level;
extern char fd_queue_path[];
extern int queue_id;

#define LOG(log_level, format_, ...) {                                        \
    int total_size = 1024;                                                    \
    char *prefix;                                                             \
                                                                              \
    if ((log_level) <= current_log_level) {                                   \
        switch ((log_level)) {                                                \
            case INFO:                                                        \
                prefix = "INFO";                                              \
                break;                                                        \
            case WARN:                                                        \
                prefix = "WARN";                                              \
                break;                                                        \
            case ERROR:                                                       \
                prefix = "ERROR";                                             \
                break;                                                        \
            case DEBUG:                                                       \
                prefix = "DEBUG";                                             \
                break;                                                        \
            default:                                                          \
                abort();                                                      \
        };                                                                    \
                                                                              \
        char *message = malloc(sizeof(char) * total_size);                    \
        snprintf(message, total_size, "%s "format_"\n", prefix, __VA_ARGS__); \
        send_log_message(message);                                            \
        free(message);                                                        \
    }                                                                         \
}

#define log_i(format, ...) LOG(INFO, format, __VA_ARGS__);
#define log_d(format, ...) LOG(DEBUG, format, __VA_ARGS__);
#define log_w(format, ...) LOG(WARN, format, __VA_ARGS__);
#define log_e(format, ...) LOG(ERROR, format, __VA_ARGS__);
int start_logger(char *logs_dir);
int save_log(char *logs_dir, char *message);
int send_log_message(char *message);

#endif //SMTP_CLIENT_LOG_H
