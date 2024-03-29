//
// Created by Никита Иксарица on 21.02.2022.
//

#include <pthread.h>

#include "log.h"
#include "errno.h"

static logger_state_t logger_state;

void save_log(const char *logs_dir, const char *message);
int log_message(log_level_te log_level, const char *format, va_list va_args);

/**
 * При передаче состояния логгера в виде структуры нужно, чтобы оно было инициализировано как в
 * родителе, так и в потомке, так как state используется и там и там, поэтому схема такая:
 * init_logger (в parent) -> fork (в parent) -> start_logger (в child) -> log (в parent)
 * Для логера, работающего в отдельном потоке, раздельная инициализация не требуетя, так как у всех
 * потоков одно общее адресное пространство, и все они ссылаются на один и тот же state
 */
key_t init_logger(logger_state_t state) {
    logger_state = state;

    if (access(logger_state.fd_queue_path, F_OK) < 0) {
        printf("Queue path doesn't exist: <%s>.\n", logger_state.fd_queue_path);
        exit(-1);
    } else if (access(logger_state.logs_dir, F_OK) < 0) {
        printf("Logs directory doesn't exist: <%s>.\n", logger_state.logs_dir);
        exit(-1);
    }

    printf("Log path: <%s>.\n", logger_state.logs_dir);

    return (logger_state.ipc_key = ftok(logger_state.fd_queue_path, logger_state.queue_id));
}

void sigterm_handler(int sig) {
    if (sig == SIGTERM) {
        save_log(logger_state.logs_dir, "SIGTERM RECEIVED, TERMINATE");

        switch (logger_state.type) {
            case THREAD:
                pthread_exit(NULL);
            case PROCESS:
                exit(0);
        }
    }
}

int start_logger(void) {
    int log_queue_id = msgget(logger_state.ipc_key, IPC_CREAT | 0666);

    struct msqid_ds qstatus = {0};
    msgctl(log_queue_id, IPC_STAT, &qstatus);
    qstatus.msg_qbytes = MAX_QUEUE_SIZE; // Здесь выставляется новый максимальный размер очереди в 16 Кб.

    // To raise the value of the msg_qbytes field, the effective user ID of the current process
    // must have root user authority.
    // https://www.ibm.com/docs/en/aix/7.2?topic=m-msgctl-subroutine
    if (msgctl(log_queue_id, IPC_SET, &qstatus) < 0) {
        printf("Call to msgctl failed.\n");
        exit(-1);
    }

    // Чтобы можно было завершить логер, послав ему сигнал SIGTERM.
    // Это будет работать как для логера в отдельном потоке (использовать pthread_kill),
    // так и для логера в отдельном процессе (использовать kill).
    if (signal(SIGTERM, sigterm_handler) == SIG_ERR) {
        printf("Can't set signal handler.\n");
        exit(-1);
    }

    queue_msg_t cur_msg;

    while (true) {
        if (msgrcv(log_queue_id, &cur_msg, sizeof(cur_msg), 1, MSG_NOERROR) != -1) {
            printf("message received: %s\n", cur_msg.mtext);
            save_log(logger_state.logs_dir, cur_msg.mtext);
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

void save_log(const char *logs_dir, const char *message) {
    size_t message_length = strlen(message);
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
        exit(-1);
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
}

int send_log_message(const char *message) {
    if (message == NULL) {
        return -1;
    }

    int log_queue_id = msgget(logger_state.ipc_key, 0666 | IPC_CREAT);

    /*struct msqid_ds qstatus;
    if(msgctl(log_queue_id, IPC_STAT, &qstatus) < 0){
        perror("msgctl failed");
        exit(1);
    }

    printf("Current number of bytes on queue %ld\n",qstatus.msg_cbytes);
    printf("Maximum number of bytes allowed on the queue %ld\n",qstatus.msg_qbytes);*/

    queue_msg_t new_msg;
    new_msg.mtype = 1;
    strcpy(new_msg.mtext, message);

    // Даже при стандартном размере очереди в 2 Кб данный примитывный механизм ретраев
    // позволяет корректно отсылать сколько нужно сообщений подряд с небольшой задержкой,
    // пока msgctl не работает, это решает проблему с отправкой в неблокирующем вызове.
    int i = 0, rc = -1;
    errno = EAGAIN;
    
    for (; rc == -1 && errno == EAGAIN && i < MAX_SEND_RETRIES; i++) {
        printf("retrying: retry %d\n", i);
        rc = msgsnd(log_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);
        usleep(WAIT_BEFORE_RETRY);
    }

    return rc;
}

/**
 * Чтобы скрыть state логгера, от макросов придется избавиться, так как они объявляются в хедере
 * и используют глобальные значения state, т.е. state должен быть виден в хедере и так его вообще не скроешь.
 * Проблема с передачей vargs решается использованием va_list.
 */
int log_message(log_level_te log_level, const char *format, va_list va_args) {
    if (log_level <= logger_state.current_log_level) {
        char message[QUEUE_MESSAGE_MAX_LENGTH];
        snprintf(message, QUEUE_MESSAGE_MAX_LENGTH, "%s %s ", log_level_to_str(log_level), logger_state.logger_name);

        size_t prefix_len = strlen(message);
        vsnprintf(message + prefix_len, QUEUE_MESSAGE_MAX_LENGTH - prefix_len, format, va_args);
        return send_log_message(message);
    }

    return -1;
}

log_level_te str_to_log_level(const char *restrict str) {
    if (strcmp(str, "ERROR") == 0) {
        return ERROR;
    } else if (strcmp(str, "WARN") == 0) {
        return WARN;
    } else if (strcmp(str, "INFO") == 0) {
        return INFO;
    } else if (strcmp(str, "DEBUG") == 0) {
        return DEBUG;
    } else if (strcmp(str, "TRACE") == 0) {
        return TRACE;
    }

    return -1;
}

static const char *log_levels[LOG_LEVELS_NUM] = {
        "ERROR", "WARN", "INFO",
        "DEBUG", "TRACE"
};

const char *log_level_to_str(log_level_te log_level) {
    return (log_level < LOG_LEVELS_NUM) ? log_levels[log_level] : NULL;
}
