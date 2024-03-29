//
// Created by Никита Иксарица on 21.02.2022.
//

#ifndef SMTP_COMMON_LOG_H
#define SMTP_COMMON_LOG_H

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
#include "queue_msg.h"

typedef enum log_level {
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
} log_level_te;

#define LOGGER_NAME_MAX_LENGTH 1024
#define MAX_QUEUE_SIZE         16384
#define MAX_SEND_RETRIES       5
#define WAIT_BEFORE_RETRY      1000
#define LOG_LEVELS_NUM         5

typedef enum {
    THREAD,
    PROCESS
} logger_type_te;

typedef struct logger_state {
    char logger_name[LOGGER_NAME_MAX_LENGTH];
    char logs_dir[PATH_MAX_LENGTH];
    char fd_queue_path[PATH_MAX_LENGTH];
    log_level_te current_log_level;
    int queue_id;
    key_t ipc_key;
    logger_type_te type;
} logger_state_t;

int init_logger(logger_state_t state);
int start_logger(void);

int log_info(const char *format, ...);
int log_debug(const char *format, ...);
int log_warn(const char *format, ...);
int log_error(const char *format, ...);
int log_trace(const char *format, ...);

log_level_te str_to_log_level(const char *restrict str);
const char *log_level_to_str(log_level_te log_level);

#endif //SMTP_COMMON_LOG_H
