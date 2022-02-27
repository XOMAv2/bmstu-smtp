#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <pthread.h>

#include "config.h"
#include "error.h"
#include "defines.h"
#include "smtp.h"

int compile_regex(regex_t *r, const char *regex_text) {
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

}

int main(int argc, char *argv[]) {
    server_config_t server_config;

    parse_app_config(&server_config, argc, argv);

/*    printf("config:\n");
    printf("host: %s\n", server_config.host);
    printf("port: %d\n", server_config.port);
    printf("user: %s\n", server_config.user);
    printf("group: %s\n", server_config.group);
    printf("proc: %d\n", server_config.proc_count);
    printf("domain: %s\n", server_config.domain);
    printf("maildir: %s\n", server_config.maildir);*/

    pthread_t log_thread;

    pthread_create(log_thread, NULL, logger, NULL);

    regex_t r;
    const char *regex_text = "^([Ff][Rr][Oo][Mm]:)?(<.+>)?( )?(.+)?(\r\n)$";
    const char *find_text = "FROM:<mailbox>params\r\n";

    printf("Trying to find '%s' in '%s'\n", regex_text, find_text);
    compile_regex(&r, regex_text);
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

    exit(0);
}
