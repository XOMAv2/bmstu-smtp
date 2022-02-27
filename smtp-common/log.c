//
// Created by Никита Иксарица on 21.02.2022.
//

#include "log.h"

static logger_state_t logger_state;

int save_log(const char *logs_dir, const char *message);
int log_message(log_level_te log_level, const char *format, va_list va_args);

/**
 * При передаче состояния логгера в виде структуры нужно, чтобы оно было инициализировано как в родителе, так и в
 * потомке, так как state используется и там и там, поэтому схема такая:
 * init_logger (в parent) -> fork (в parent) -> start_logger (в child) -> log (в parent)
 * Для логера, работающего в отдельном потоке, раздельная инициализация не требуетя, так как у всех
 * потоков одно общее адресное пространство, и все они ссылаются на один и тот же state
 */
key_t init_logger(logger_state_t state) {
    logger_state = state;

    if (access(logger_state.fd_queue_path, F_OK) < 0) {
        printf("Queue path doesn't exist: <%s>.\n", logger_state.fd_queue_path);
        return -1;
    } else if (access(logger_state.logs_dir, F_OK) < 0) {
        printf("Logs directory doesn't exist: <%s>.\n", logger_state.logs_dir);
        return -1;
    }

    printf("Log path: <%s>.\n", logger_state.logs_dir);

    return ftok(logger_state.fd_queue_path, logger_state.queue_id);
}

int start_logger(key_t queue_key) {
    int log_queue_id = msgget(queue_key, 0666);
    queue_msg_t cur_msg;

    while (true) {
        if (msgrcv(log_queue_id, &cur_msg, sizeof(cur_msg), 1, MSG_NOERROR) != -1) {
            if (save_log(logger_state.logs_dir, cur_msg.mtext) < 0) {
                printf("Error in save_log function call.\n");
                return -1;
            }
        }
    }
}

int log_info(const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    int rc = log_message(INFO, format, va_args);
    va_end(va_args);
    return rc;
}

int log_debug(const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    int rc = log_message(DEBUG, format, va_args);
    va_end(va_args);
    return rc;
}

int log_warn(const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    int rc = log_message(WARN, format, va_args);
    va_end(va_args);
    return rc;
}

int log_error(const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    int rc = log_message(ERROR, format, va_args);
    va_end(va_args);
    return rc;
}

int log_trace(const char *format, ...) {
    va_list va_args;
    va_start(va_args, format);
    int rc = log_message(TRACE, format, va_args);
    va_end(va_args);
    return rc;
}

int save_log(const char *logs_dir, const char *message) {
    size_t message_length = strlen(message);

    if (message_length == 0) {
        return 0;
    }

    time_t now = time(NULL);
    struct tm *timenow = gmtime(&now);

    char *log_path = malloc(strlen(logs_dir) + 50);

    if (log_path == NULL) {
        printf("Can't allocate memory for log_path char array.\n");
        exit(-1);
    }

    strcpy(log_path, logs_dir);
    ensure_trailing_slash(log_path);
    char filename[40];
    strftime(filename, sizeof(filename), "log_%Y-%m-%d", timenow);
    strcat(log_path, filename);
    strcat(log_path, ".log");

    FILE *f = fopen(log_path, "a+");

    if (f == NULL) {
        printf("Can't open or create log file: <%s>.\n", log_path);
        return -1;
    }

    char timeString[12];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", timenow);
    char *full_log_msg = malloc(strlen(timeString) + 2 + message_length);

    if (full_log_msg == NULL) {
        printf("Can't allocate memory for full_log_msg char array.\n");
        exit(-1);
    }

    strcpy(full_log_msg, timeString);
    strcat(full_log_msg, " ");
    strcat(full_log_msg, message);
    strcat(full_log_msg, "\n");
    fputs(full_log_msg, f);

    fclose(f);
    free(full_log_msg);
    free(log_path);

    return 0;
}

int send_log_message(const char *message) {
    if (message == NULL) {
        return -1;
    }

    key_t key = ftok(logger_state.fd_queue_path, logger_state.queue_id);
    int log_queue_id = msgget(key, 0666 | IPC_CREAT);
    queue_msg_t new_msg;
    new_msg.mtype = 1;
    strcpy(new_msg.mtext, message);
    msgsnd(log_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);

    return 0;
}

/**
 * Чтобы скрыть state логгера, от макросов придется избавиться, так как они объявляются в хедере
 * и используют глобальные значения state, т.е. state должен быть виден в хедере и так его вообще не скроешь.
 * Проблема с передачей vargs решается использованием va_list.
 */
int log_message(log_level_te log_level, const char *format, va_list va_args) {
    if (log_level <= logger_state.current_log_level) {
        char message[QUEUE_MESSAGE_MAX_LENGTH];

        switch (log_level) {
            case ERROR:
                snprintf(message, QUEUE_MESSAGE_MAX_LENGTH, "%s ERROR ", logger_state.logger_name);
                break;
            case WARN:
                snprintf(message, QUEUE_MESSAGE_MAX_LENGTH, "%s WARN ", logger_state.logger_name);
                break;
            case INFO:
                snprintf(message, QUEUE_MESSAGE_MAX_LENGTH, "%s INFO ", logger_state.logger_name);
                break;
            case DEBUG:
                snprintf(message, QUEUE_MESSAGE_MAX_LENGTH, "%s DEBUG ", logger_state.logger_name);
                break;
            case TRACE:
                snprintf(message, QUEUE_MESSAGE_MAX_LENGTH, "%s TRACE ", logger_state.logger_name);
                break;
            default:
                abort();
        }

        size_t prefix_len = strlen(message);
        vsnprintf(message + prefix_len, QUEUE_MESSAGE_MAX_LENGTH - prefix_len, format, va_args);
        send_log_message(message);
    }

    return 0;
}
