#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <pthread.h>
#include <assert.h>

#include "config.h"
#include "error.h"
#include "defines.h"
#include "protocol.h"

int rcompile(regex_t *r, const char *regex_text) {
    int status = regcomp(r, regex_text, REG_EXTENDED | REG_NEWLINE);

    if (status != 0) {
        char error_message[ERROR_MESSAGE_MAX_LENGTH];
        regerror(status, r, error_message, ERROR_MESSAGE_MAX_LENGTH);
        exit_with_error_message(ERR_REGEX_COMPILE, error_message);
    }

    return OP_SUCCESS;
}

static int match_regex(regex_t *r, const char *to_match) {
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char *p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 20;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    while (1) {
        int nomatch = regexec(r, p, n_matches, m, 0);

        if (nomatch) {
            printf("No more matches.\n");
            return nomatch;
        }

        for (int i = 0; i < n_matches; i++) {
            if (m[i].rm_so == -1) {
                continue;
            }

            int start = m[i].rm_so + (p - to_match);
            int finish = m[i].rm_eo + (p - to_match);

            if (i == 0) {
                printf("$& is ");
            } else {
                printf("$%d is ", i);
            }

            printf("'%.*s' (bytes %d:%d)\n", (finish - start),
                   to_match + start, start, finish);
        }

        p += m[0].rm_eo;
    }

    return 0;
}

void *logger(void *data) {
    assert(data);

    logger_config_t *logger_config = (logger_config_t *)data;
    logger_state_t logger_state = {
            .current_log_level = logger_config->log_level,
            .queue_id = logger_config->queue_id,
            .type = THREAD
    };

    strncpy(logger_state.logger_name, logger_config->name, LOGGER_NAME_MAX_LENGTH);
    strncpy(logger_state.logs_dir, logger_config->log_dir, PATH_MAX_LENGTH);
    strncpy(logger_state.fd_queue_path, logger_config->queue_path, PATH_MAX_LENGTH);

    if (init_logger(logger_state) > 0) {
        start_logger();
    }

    return data;
}

int main(int argc, char *argv[]) {
    app_config_t app_config;

    config_t config = parse_app_config(&app_config, argc, argv);

    pthread_t logger_thread;

    pthread_create(&logger_thread, NULL, logger, (void*)&app_config.logger_config);

    sleep(1);

    log_app_config(&app_config);

    /*regex_t r;
    const char *regex_text = "^ [Tt][Oo]:<(.*:)?(.+)?>( .+=.+)*(\r\n)$";
    const char *find_text = " TO:<reverse.path:> param_1=value_1\r\n";

    printf("Trying to find '%s' in '%s'\n", regex_text, find_text);
    rcompile(&r, regex_text);
    match_regex(&r, find_text);
*/
    int rc = OP_SUCCESS;
    int server_socket;

    if ((rc = server_init(&app_config.server_config, &server_socket)) != OP_SUCCESS) {
        exit_with_error_code(rc);
    }

    serve(server_socket);

    pthread_kill(logger_thread, SIGTERM);
    config_destroy(&config);

    exit(0);
}
