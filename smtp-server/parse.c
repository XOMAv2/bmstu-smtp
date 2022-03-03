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
#include "destruction.h"

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
// So if regexec() call fails it means than no <CLRF> sequence at the end of the message was found
static const char *cmd_regex_patterns[CMD_NUM] = {
        // helo = "HELO" SP Domain CRLF
        "^(.+)?(\r\n)$",
        // ehlo = "EHLO" SP ( Domain / address-literal ) CRLF
        "^(.+)?(\r\n)$",
        // mail = "MAIL FROM:" Reverse-path [SP Mail-parameters] CRLF
        // use (.*:) instead of (.+:) for adl part to prevent parsing ':' as part of mailbox in case of empty adl
        "^[Ff][Rr][Oo][Mm]:<(.*:)?(.+@.+)>( .+=.+)*(\r\n)$",
        // rcpt = "RCPT TO:" ( "<Postmaster@" Domain ">" / "<Postmaster>" / Forward-path ) [SP Rcpt-parameters] CRLF
        "^[Tt][Oo]:<(.*:)?(.+)>( .+=.+)*(\r\n)$",
        "^[Dd][Aa][Tt][Aa]command->cmd(\r\n)$",
        "^[Rr][Ss][Ee][Tt](\r\n)$",
        "^[Nn][Oo][Oo][Pp](\r\n)$",
        "^[Vv][Rr][Ff][Yy] (.+?)(\r\n)$",
        "^[Qq][Uu][Ii][Tt](\r\n)$"
};

static regex_t cmd_regexes[CMD_NUM] = {0};

#define PARSE_HELO_MESSAGE_IDX 1

err_code_t parse_helo_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];
    const regmatch_t *message_match = matches + PARSE_HELO_MESSAGE_IDX;

    if (regexec(&cmd_regexes[HELO], message, MAX_REGEX_MATCHES, matches, 0)) {
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
    return parse_helo_message(message, data);
}

// Parse optional at-domain list from string matched by '(.*:)?' pattern.
char *parse_path_adl(char *restrict message, const regmatch_t *adl_match, const char *cmd_code) {
    assert(message != NULL && adl_match != NULL && cmd_code != NULL);

    if (adl_match->rm_so == -1) {
        return NULL;
    }

    char *adl = NULL;
    size_t adl_len = adl_match->rm_eo - adl_match->rm_so;  // excluding ':' and including '\0'

    if (adl_len > 0) {
        adl = malloc(adl_len + 1);
        if (adl == NULL) {
            exit_with_error_message(ERR_NOT_ALLOCATED, "%s:adl", cmd_code);
        }
        strncpy(adl, message + adl_match->rm_so, adl_len);
    }

    return adl;
}

// Parse mandatory mailbox part of path from string matched by '.+@.+' pattern.
char *parse_path_mailbox(char *restrict message, const regmatch_t *mailbox_match, const char *cmd_code) {
    assert(message != NULL && mailbox_match != NULL && cmd_code != NULL);

    size_t mailbox_len = mailbox_match->rm_eo - mailbox_match->rm_so + 1; // including '\0'
    char *mailbox = malloc(mailbox_len);

    if (mailbox == NULL) {
        exit_with_error_message(ERR_NOT_ALLOCATED, "%s:mailbox", cmd_code);
    }

    strncpy(mailbox, message + mailbox_match->rm_so, mailbox_len);

    return mailbox;
}

// Parse optional command parameters list (used for MAIL and RCPT).
size_t parse_cmd_params(char *restrict message,
                        const regmatch_t *params_match,
                        const char *cmd_code,
                        mail_param_t **restrict params_out) {
    assert(message != NULL && params_match != NULL && cmd_code != NULL && params_out != NULL);

    if (params_match->rm_so == -1) {
        return 0;
    }

    mail_param_t *params = NULL;
    size_t params_cnt = 0;

    char *param_part = strtok(message + params_match->rm_so, SP);
    while (param_part) {
        params = realloc(params, sizeof(mail_param_t) * (params_cnt + 1));
        if (params == NULL) {
            exit_with_error_message(ERR_NOT_ALLOCATED, "%s:params", cmd_code);
        }

        size_t key_len = strspn(param_part, "=");
        size_t value_len = strlen(param_part) - key_len - 1;

        params[params_cnt].key = malloc(key_len);
        if (params[params_cnt].key == NULL) {
            exit_with_error_message(ERR_NOT_ALLOCATED, "%s:param_key", cmd_code);
        }

        params[params_cnt].value = malloc(value_len);
        if (params[params_cnt].value == NULL) {
            exit_with_error_message(ERR_NOT_ALLOCATED, "%s:param_value", cmd_code);
        }

        strncpy(params[params_cnt].key, param_part, key_len);
        strncpy(params[params_cnt].value, param_part + key_len + 1, value_len);

        params_cnt++;
        param_part = strtok(NULL, SP);
    }

    *params_out = params;

    return params_cnt;
}

err_code_t parse_path_message(cmd_te cmd, char *restrict message, void **restrict data) {
    assert(message != NULL && data != NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    // Parse message body.
    if (regexec(&cmd_regexes[cmd], message, MAX_REGEX_MATCHES, matches, 0)) {
        return ERR_REGEX_INVALID_MAIL;
    }

    const regmatch_t *adl_match = matches + PARSE_MAIL_ADL_IDX;
    const regmatch_t *mailbox_match = matches + PARSE_MAIL_MAILBOX_IDX;
    const regmatch_t *params_match = matches + PARSE_MAIL_PARAMS_IDX;

    const char *cmd_code = cmd == MAIL ? "MAIL" : "RCPT";

    char *adl = parse_path_adl(message, adl_match, cmd_code);

    mail_param_t *params = NULL;
    size_t params_cnt = parse_cmd_params(message, params_match, cmd_code, &params);

    // Parse mailbox (mandatory).
    char *mailbox = parse_path_mailbox(message, mailbox_match, cmd_code);

    // Store all message data to output structure.
    path_data_t *path_data = (path_data_t *) calloc(1, sizeof(path_data_t));
    if (path_data == NULL) {

        exit_with_error_message(ERR_NOT_ALLOCATED, "%s:data", cmd_code);
    }
    path_data->adl = adl;
    path_data->params = params;
    path_data->params_cnt = params_cnt;
    path_data->mailbox = mailbox;
    *data = path_data;

    return OP_SUCCESS;
}

err_code_t parse_mail_message(char *restrict message, void **restrict data) {
    return parse_path_message(MAIL, message, data);
}

err_code_t parse_rcpt_message(char *restrict message, void **restrict data) {
    return parse_path_message(RCPT, message, data);
}

err_code_t parse_data_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&cmd_regexes[DATA], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_rset_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&cmd_regexes[RSET], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_noop_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&cmd_regexes[NOOP], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_vrfy_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&cmd_regexes[VRFY], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

err_code_t parse_quit_message(char *restrict message, void **restrict data) {
    assert(data = NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    if (regexec(&cmd_regexes[QUIT], message, MAX_REGEX_MATCHES, matches, 0)) {
        ERR_REGEX_NO_CRLF;
    }

    return OP_SUCCESS;
}

static err_code_t (*const parsers[CMD_NUM])(char *restrict, void **restrict) = {
        parse_helo_message, parse_ehlo_message, parse_mail_message,
        parse_rcpt_message, parse_data_message, parse_rset_message,
        parse_noop_message, parse_vrfy_message, parse_quit_message
};

err_code_t compile_regex(regex_t *regex, const char *restrict pattern, char *restrict error_message) {
    int status = regcomp(regex, pattern, REG_EXTENDED | REG_NEWLINE);

    if (status != 0) {
        regerror(status, regex, error_message, ERROR_MESSAGE_MAX_LENGTH);
        return ERR_REGEX_COMPILE;
    }

    return OP_SUCCESS;
}

void compile_regexes(void) {
    int i;
    char error_message[ERROR_MESSAGE_MAX_LENGTH];

    for (i = 0; i < CMD_NUM && compile_regex(&cmd_regexes[i], cmd_regex_patterns[i], error_message) == OP_SUCCESS; i++);
    if (i < CMD_NUM) {
        exit_with_error_message(ERR_REGEX_COMPILE, error_message);
    }
    const dtor_t cmd_regexes_dtor = {
            .regex_dtor = {.call = free_regexes, .regexes = cmd_regexes, .cnt = CMD_NUM},
            .type = REGEX_DTOR
    };
    add_dtor(&cmd_regexes_dtor);

    /*for (i = 0; i < MAIL_REGEX_NUM && compile_regex(&mail_regexes[i], mail_regex_patterns[i], error_message) == OP_SUCCESS; i++);
    if (i < MAIL_REGEX_NUM) {
        exit_with_error_message(ERR_REGEX_COMPILE, error_message);
    }
    const dtor_t mail_regexes_dtor = {
            .regex_dtor = {.call = free_regexes, .regexes = mail_regexes, .cnt = MAIL_REGEX_NUM},
            .type = REGEX_DTOR
    };
    add_dtor(&mail_regexes_dtor);*/
}

void free_regexes(regex_t *restrict regexes, int until) {
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
