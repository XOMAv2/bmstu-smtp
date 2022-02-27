#include <stdio.h>
#include "../smtp-common/log.h"

int main(void) {

    // Вот это нужно загрузить из конфига
    logger_state_t logger_stte = {
            .logger_name = "CLIENT",
            .logs_dir = "./logs_dir",
            .fd_queue_path = "/tmp",
            .queue_id = 100,
            .current_log_level = DEBUG
    };

    key_t queue_key = init_logger(logger_stte);

    if (fork() == 0) { // Child process.
        start_logger(queue_key);
    } else {
        log_info("Success!");
        log_debug("Success!");
    }

    return 0;
}
