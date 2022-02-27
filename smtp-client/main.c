#include <stdio.h>
#include "../smtp-common/log.h"

int main(void) {

    // Вот это нужно загрузить из конфига
    logger_state_t logger_stte = {
            .logger_name = "CLIENT",
            .logs_dir = "./logs_dir",
            .fd_queue_path = "/tmp",
            .queue_id = 100,
            .current_log_level = TRACE
    };

    init_logger(logger_stte);

    if (fork() == 0) { // Child process.
        start_logger();
    } else {
        log_info("Success!");
        log_debug("Success!");
        log_warn("Success!");
        log_error("Success!");
        log_trace("Success!");
    }

    return 0;
}
