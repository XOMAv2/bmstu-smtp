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

#define LOG_MESSAGE_MAX_LENGTH 1024

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
    WARN,
    INFO,
    DEBUG,
    TRACE
} log_level_te;

typedef struct {
    const char *logger_name;
    const char *logs_dir;
    const char *fd_queue_path;
    log_level_te current_log_level;
    int queue_id;
} logger_state_t;

int init_logger(const logger_state_t* state);
int start_logger(key_t queue_key);

int log_info(const char *format, ...);
int log_debug(const char *format, ...);
int log_warn(const char *format, ...);
int log_error(const char *format, ...);

#endif //SMTP_CLIENT_LOG_H
