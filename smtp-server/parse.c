//
// Created by Karpukhin Aleksandr on 25.02.2022.
//

#include <assert.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>

#include "parse.h"
#include "error.h"
#include "defines.h"

err_code_t cmd_code_to_cmd_event(const char *cmd_code, cmd_te *restrict cmd, te_smtp_server_fsm_event *restrict event) {
    assert(cmd_code != NULL && cmd != NULL && event != NULL);

    if (!strcmp(cmd_code, SMTP_HELO_CMD_CODE)) {
        *cmd = HELO;
        *event = SMTP_SERVER_FSM_EV_RECV_HELO;
    } else if (!strcmp(cmd_code, SMTP_EHLO_CMD_CODE)) {
        *cmd = EHLO;
        *event = SMTP_SERVER_FSM_EV_RECV_EHLO;
    } else if (!strcmp(cmd_code, SMTP_MAIL_CMD_CODE)) {
        *cmd = MAIL;
        *event = SMTP_SERVER_FSM_EV_RECV_MAIL;
    } else if (!strcmp(cmd_code, SMTP_RCPT_CMD_CODE)) {
        *cmd = RCPT;
        *event = SMTP_SERVER_FSM_EV_RECV_RCPT;
    } else if (!strcmp(cmd_code, SMTP_DATA_CMD_CODE)) {
        *cmd = DATA;
        *event = SMTP_SERVER_FSM_EV_RECV_DATA;
    } else if (!strcmp(cmd_code, SMTP_RSET_CMD_CODE)) {
        *cmd = RSET;
        *event = SMTP_SERVER_FSM_EV_RECV_RSET;
    } else if (!strcmp(cmd_code, SMTP_NOOP_CMD_CODE)) {
        *cmd = NOOP;
        *event = SMTP_SERVER_FSM_EV_RECV_NOOP;
    } else if (!strcmp(cmd_code, SMTP_VRFY_CMD_CODE)) {
        *cmd = VRFY;
        *event = SMTP_SERVER_FSM_EV_RECV_VRFY;
    } else if (!strcmp(cmd_code, SMTP_QUIT_CMD_CODE)) {
        *cmd = QUIT;
        *event = SMTP_SERVER_FSM_EV_RECV_QUIT;
    }

    return ERR_SMTP_INVALID_COMMAND;
}

// Patterns are specified without command codes (drop <CMD SP> part) because codes are used and
// discarded at parser identification stage before any certain parser call.
// CAUTION: this POSIX regex implementation fails in endless search if regular
// expression consists only of optional matching groups (i.e. <(smth)?>).
// Any expression must contain at least one obligatory group, in our case it is <CLRF> sequence.
// So if regexec() call fails it means than no <CLRF> sequence at the end of the message was found.
static const char *regex_patterns[CMD_NUM] = {
        "^(.+)?(\r\n)$",                                  // helo = "HELO" SP Domain CRLF
        "^(.+)?(\r\n)$",                                  // ehlo = "EHLO" SP ( Domain / address-literal ) CRLF
        "^([Ff][Rr][Oo][Mm]:)?(<.*>)?( )?(.+)?(\r\n)$",   // mail = "MAIL FROM:" Reverse-path [SP Mail-parameters] CRLF
        "^[Rr][Cc][Pp][Tt]\\s[Tt][Oo]:\\s?[<]?(.+?)[>]?(\r\n)$",
        "^[Dd][Aa][Tt][Aa]command->cmd(\r\n)$",
        "^[Rr][Ss][Ee][Tt](\r\n)$",
        "^[Nn][Oo][Oo][Pp](\r\n)$",
        "^[Vv][Rr][Ff][Yy] (.+?)(\r\n)$",
        "^[Qq][Uu][Ii][Tt](\r\n)$"
};

static regex_t regexes[CMD_NUM] = {0};

#define PARSE_HELO_MESSAGE_IDX 1

err_code_t parse_helo_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];
    const regmatch_t *message_match = matches + PARSE_HELO_MESSAGE_IDX;

    if (regexec(&regexes[HELO], message, MAX_REGEX_MATCHES, matches, 0)) {
        return ERR_REGEX_NO_CRLF;
    } else if (message_match->rm_so == -1) {
        return ERR_REGEX_NO_DOMAIN;
    }

    size_t domain_len = message_match->rm_eo - message_match->rm_so;
    char *domain = calloc(domain_len, sizeof(char));
    if (domain == NULL) {
        exit_with_error_code(ERR_NOT_ALLOCATED);
    }
    strncpy(domain, message + message_match->rm_so, domain_len);

    return OP_SUCCESS;
}

err_code_t parse_ehlo_message(char *restrict message, void **restrict data) {
    assert(data = NULL);
    return parse_helo_message(message, data);
}

#define PARSE_MAIL_FROM_IDX         1
#define PARSE_MAIL_REVERSE_PATH_IDX 2
#define PARSE_MAIL_SP_IDX           3
#define PARSE_MAIL_PARAMS_IDX       4

err_code_t parse_mail_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    const char *at_domain_regex = "";
    const char *mailbox_regex = "";

    regmatch_t matches[MAX_REGEX_MATCHES];
    const regmatch_t * from_match = matches + PARSE_MAIL_FROM_IDX;
    const regmatch_t * reverse_path_match = matches + PARSE_MAIL_REVERSE_PATH_IDX;
    const regmatch_t * sp_match = matches + PARSE_MAIL_SP_IDX;
    const regmatch_t * params_match = matches + PARSE_MAIL_PARAMS_IDX;

    // parse message body
    if (regexec(&regexes[MAIL], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    } else if (from_match->rm_so == -1) {
        return ERR_REGEX_NO_FROM;
    } else if (reverse_path_match->rm_so == -1) {
        return ERR_REGEX_NO_REVERSE_PATH;
    } else if (sp_match->rm_so != params_match->rm_so && (sp_match->rm_so == -1 || params_match->rm_so == -1)) {
        return ERR_REGEX_INVALID_MAIL_PARAMS;
    }

    // parse reverse-path section

    // parse params section

    return OP_SUCCESS;
}

err_code_t parse_rcpt_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&regexes[RCPT], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_data_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&regexes[DATA], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_rset_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&regexes[RSET], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_noop_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&regexes[NOOP], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_vrfy_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&regexes[VRFY], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_quit_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&regexes[QUIT], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

static err_code_t (* const parsers[CMD_NUM])(char *restrict, void **restrict) = {
        parse_helo_message, parse_ehlo_message, parse_mail_message,
        parse_rcpt_message, parse_data_message, parse_rset_message,
        parse_noop_message, parse_vrfy_message, parse_quit_message
};

void compile_regexes(void) {
    for (int i = 0; i < CMD_NUM; i++) {
        int status = regcomp(&regexes[i], regex_patterns[i], REG_EXTENDED | REG_NEWLINE);

        if (status != 0) {
            char error_message[ERROR_MESSAGE_MAX_LENGTH];
            regerror(status, &regexes[i], error_message, ERROR_MESSAGE_MAX_LENGTH);
            free_regexes(i);
            exit_with_error_message(ERR_REGEX_COMPILE, error_message);
        }
    }
}

void free_regexes(int until) {
    for (int i = 0; i < until; i++) {
        regfree(&regexes[i]);
    }
}

err_code_t match_regex(regex_t *regex, const char *to_match, regmatch_t matches[MAX_REGEX_MATCHES]) {
    if (regexec(regex, to_match, MAX_REGEX_MATCHES, matches, 0)) {
        printf("No more matches.\n");
        return ERR_REGEX_NOMATCH;
    }

    return OP_SUCCESS;
}

err_code_t parse_smtp_message(char *restrict message, command_t *restrict command) {
    assert(message != NULL && command != NULL);

    char *cmd_code;

    if ((cmd_code = strtok(message, SP)) == NULL ||
        cmd_code_to_cmd_event(cmd_code, &command->cmd, &command->event) < 0) {
        return ERR_SMTP_INVALID_COMMAND;
    }

    return parsers[command->cmd](strtok(NULL, ""), &(command->data));
}
