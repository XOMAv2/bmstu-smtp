#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <pthread.h>

#include "config.h"
#include "error.h"
#include "defines.h"

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
    logger_state_t *logger_state = (logger_state_t *)data;
    logger_state->type = THREAD;

    if (init_logger(*logger_state) > 0) {
        start_logger();
    }

    return data;
}

int main(int argc, char *argv[]) {
    server_config_t server_config;
    logger_state_t logger_state;

    parse_app_config(&server_config, &logger_state, argc, argv);

    pthread_t logger_thread;

    pthread_create(&logger_thread, NULL, logger, (void*)&logger_state);

    sleep(1);

    log_info("server config:\n");
    log_info("host: %s\n", server_config.host);
    log_info("port: %d\n", server_config.port);
    log_info("user: %s\n", server_config.user);
    log_info("group: %s\n", server_config.group);
    log_info("domain: %s\n", server_config.domain);
    log_info("maildir: %s\n", server_config.maildir);

    regex_t r;
    const char *regex_text = "^[Tt][Oo]:<((Postmaster)|(Postmaster@.+)|(.*:)?(.+@.+))>( .+=.+)*(\r\n)$";
    const char *find_text = "TO:<reverse:Postmaster@domain> param1=1 param2=3\r\n";

    printf("Trying to find '%s' in '%s'\n", regex_text, find_text);
    rcompile(&r, regex_text);
    match_regex(&r, find_text);

/*    int rc = OP_SUCCESS;
    int server_socket;

    if ((rc = server_init(&server_config, &server_socket)) != OP_SUCCESS) {
        exit_with_error_code(rc);
    }

    char err_msg[ERROR_MESSAGE_MAX_LENGTH];

    if ((rc = serve(server_socket, err_msg)) != OP_SUCCESS) {
        exit_with_error_message(rc, err_msg);
    }*/

    pthread_kill(logger_thread, SIGTERM);

    exit(0);
}
