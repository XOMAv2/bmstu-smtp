//
// Created by Никита Иксарица on 21.02.2022.
//

#include "log.h"

int start_logger(char *logs_dir) {
    if (access(fd_queue_path, F_OK) < 0) {
        printf("Queue path doesn't exist: <%s>.\n", fd_queue_path);
        return -1;
    } else if (access(logs_dir, F_OK) < 0) {
        printf("Logs directory doesn't exist: <%s>.\n", logs_dir);
        return -1;
    }

    printf("Log path: <%s>.\n", logs_dir);
    key_t key = ftok(fd_queue_path, queue_id);
    int log_queue_id = msgget(key, 0666);
    queue_msg_t cur_msg;

    while (true) {
        if (msgrcv(log_queue_id, &cur_msg, sizeof(cur_msg), 1, MSG_NOERROR) != -1) {
            if (save_log(logs_dir, cur_msg.mtext) < 0) {
                printf("Error in save_log function call.\n");
                return -1;
            }
        }
    }
}

int save_log(char *logs_dir, char *message) {
    size_t message_length = strlen(message);

    if (message_length == 0) {
        return 0;
    }

    time_t now = time(NULL);
    struct tm *timenow = gmtime(&now);

    char *log_path = malloc(strlen(logs_dir) + 50);
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
    strcpy(full_log_msg, timeString);
    strcat(full_log_msg, " ");
    strcat(full_log_msg, message);
    fputs(full_log_msg, f);

    fclose(f);
    free(full_log_msg);
    free(log_path);

    return 0;
}

int send_log_message(char *message) {
    key_t key = ftok(fd_queue_path, queue_id);
    int log_queue_id = msgget(key, 0666 | IPC_CREAT);
    queue_msg_t new_msg;
    new_msg.mtype = 1;
    strcpy(new_msg.mtext, message);
    msgsnd(log_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);

    return 0;
}
