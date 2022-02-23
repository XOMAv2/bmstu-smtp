#include <stdio.h>
#include "../smtp-common/log.h"

log_level_te current_log_level = DEBUG;
char fd_queue_path[] = "/tmp";
int queue_id = 100;

int main(int argc, char **argv) {
    (void)argc; // TODO: delete (here bacause of compiler warning).

    if (fork() == 0) { // Child process.
        start_logger(argv[1]);
    } else {
        log_i("%s", "Success!");
    }

    return 0;
}
