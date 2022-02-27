#include <stdio.h>
#include "../smtp-common/log.h"

int main(void) {

    // Вот это нужно загрузить из конфига
    logger_state_t logger_stte = {
            .logger_name = "CLIENT",
            .logs_dir = "./log",
            .fd_queue_path = "./log",
            .queue_id = 100,
            .current_log_level = DEBUG
    };

    key_t queue_key = init_logger(&logger_stte);

    if (fork() == 0) { // Child process.
        start_logger(queue_key);
    } else {
        log_info("%s", "Success!");
        log_debug("%s", "Success!");
    }

    return 0;
}
