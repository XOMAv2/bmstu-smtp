//
// Created by Karpukhin Aleksandr on 25.02.2022.
//

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "parse.h"
#include "defines.h"
#include "destruction.h"

err_code_t cmd_code_to_cmd_event(const char *restrict message, cmd_te *restrict cmd, te_smtp_server_fsm_event *restrict event) {
    assert(message != NULL && cmd != NULL && event != NULL);
    assert(strlen(message) >= EMPTY_CMD_LENGTH);

    char cmd_code[CMD_CODE_LENGTH + 1] = { 0 };

    strncpy(cmd_code, message, CMD_CODE_LENGTH);

    if (!strcmp(cmd_code, SMTP_HELO_CMD_CODE)) {
        *cmd = CMD_HELO;
    } else if (!strcmp(cmd_code, SMTP_EHLO_CMD_CODE)) {
        *cmd = CMD_EHLO;
    } else if (!strcmp(cmd_code, SMTP_MAIL_CMD_CODE)) {
        *cmd = CMD_MAIL;
    } else if (!strcmp(cmd_code, SMTP_RCPT_CMD_CODE)) {
        *cmd = CMD_RCPT;
    } else if (!strcmp(cmd_code, SMTP_DATA_CMD_CODE)) {
        *cmd = CMD_DATA;
    } else if (!strcmp(cmd_code, SMTP_RSET_CMD_CODE)) {
        *cmd = CMD_RSET;
    } else if (!strcmp(cmd_code, SMTP_NOOP_CMD_CODE)) {
        *cmd = CMD_NOOP;
    } else if (!strcmp(cmd_code, SMTP_VRFY_CMD_CODE)) {
        *cmd = CMD_VRFY;
    } else if (!strcmp(cmd_code, SMTP_QUIT_CMD_CODE)) {
        *cmd = CMD_QUIT;
    } else {
        return ERR_SMTP_INVALID_COMMAND;
    }

    *event = cmd_to_event(*cmd);

    return OP_SUCCESS;
}

// Order and quantity must match cmd_te enum.
static te_smtp_server_fsm_event cmd_events[CMD_NUM] = {
        SMTP_SERVER_FSM_EV_RECV_HELO,
        SMTP_SERVER_FSM_EV_RECV_EHLO,
        SMTP_SERVER_FSM_EV_RECV_MAIL,
        SMTP_SERVER_FSM_EV_RECV_RCPT,
        SMTP_SERVER_FSM_EV_RECV_DATA,
        SMTP_SERVER_FSM_EV_RECV_DATA_INT,
        SMTP_SERVER_FSM_EV_RECV_RSET,
        SMTP_SERVER_FSM_EV_RECV_NOOP,
        SMTP_SERVER_FSM_EV_RECV_VRFY,
        SMTP_SERVER_FSM_EV_RECV_QUIT
};

te_smtp_server_fsm_event cmd_to_event(cmd_te cmd) {
    return cmd < CMD_NUM ? cmd_events[cmd] : -1;
}

// Order and quantity must match cmd_te enum.
static char* cmd_codes[CMD_NUM] = {
        "HELO", "EHLO", "MAIL",
        "RCPT", "DATA", "DATA_INT",
        "RSET", "NOOP", "VRFY", "QUIT"
};

const char* cmd_to_str(cmd_te cmd) {
    return cmd < CMD_NUM ? cmd_codes[cmd] : NULL;
}

// Order and quantity must match cmd_te enum.
static rgx_te cmd_rgxs[CMD_NUM] = {
        RGX_STR, RGX_STR, RGX_MAIL,
        RGX_RCPT, RGX_STR, RGX_STR,
        RGX_STR, RGX_STR, RGX_STR, RGX_STR
};

rgx_te cmd_to_rgx(cmd_te cmd) {
    return cmd < CMD_NUM ? cmd_rgxs[cmd] : -1;
}

// Patterns are specified without command codes (drop <CMD SP> part) because codes are used and
// discarded at parser identification stage before any certain parser call.
// CAUTION: this POSIX regex implementation fails in endless search if regular
// expression consists only of optional matching groups (i.e. <(smth)?>).
// Any expression must contain at least one obligatory group, in our case it is <CLRF> sequence.
// So if regexec() call fails it means than no <CLRF> sequence at the end of the message was found.
static const char *cmd_regex_patterns[RGX_NUM] = {
        // helo = "HELO" SP Domain CRLF
        // ehlo = "EHLO" SP ( Domain / address-literal ) CRLF
        // data = ( "DATA" / some_string / "." ) CRLF
        // noop = "NOOP" [ SP String ] CRLF
        // vrfy = "VRFY" SP String CRLF
        "^(( )?(.+)?)(\r\n)$",
        // mail = "MAIL FROM:" Reverse-path [SP Mail-parameters] CRLF
        // use (.*:) instead of (.+:) for adl part to prevent parsing ':' as part of mailbox in case of empty adl
        "^ [Ff][Rr][Oo][Mm]:<(.*:)?(.+@.+)?>( .+=.+)*(\r\n)$",
        // rcpt = "RCPT TO:" ( "<Postmaster@" Domain ">" / "<Postmaster>" / Forward-path ) [SP Rcpt-parameters] CRLF
        "^ [Tt][Oo]:<(.*:)?(.+)?>( .+=.+)*(\r\n)$"
};

static regex_t cmd_regexes[RGX_NUM] = {0};

_Bool is_empty_cmd(cmd_te cmd) {
    return (cmd == CMD_DATA || cmd == CMD_RSET || cmd == CMD_QUIT);
}

_Bool is_may_be_empty_cmd(cmd_te cmd) {
    return (cmd == CMD_DATA_INT || cmd == CMD_NOOP);
}

char* parse_str_data(const regmatch_t *restrict data_match, const char *restrict message, cmd_te cmd) {
    assert(message != NULL && data_match != NULL);

    size_t str_len = data_match->rm_eo - data_match->rm_so;
    char *str = calloc(str_len + 1, sizeof(char));

    if (str == NULL) {
        exit_with_error_message(ERR_NOT_ALLOCATED, "%s:message", cmd_to_str(cmd));
    }

    strncpy(str, message + data_match->rm_so, str_len);

    return str;
}

err_code_t parse_str_message(command_t *restrict command) {
    assert(command != NULL && command->message != NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];
    const regmatch_t *payload_match = matches + PARSE_STR_PAYLOAD_IDX;
    const regmatch_t *sp_match = matches + PARSE_STR_SP_IDX;
    const regmatch_t *str_match = matches + PARSE_STR_DATA_IDX;

    if (regexec(&cmd_regexes[cmd_to_rgx(command->cmd)], command->message, MAX_REGEX_MATCHES, matches, 0)) {
        return ERR_REGEX_NOMATCH;
    }

    _Bool is_empty = is_empty_cmd(command->cmd);
    _Bool is_may_be_empty = is_may_be_empty_cmd(command->cmd);

    // Process empty or may be empty commands
    if (is_empty || is_may_be_empty) {
        if (sp_match->rm_so == -1 && str_match->rm_so == -1) {
            command->data = NULL;
            return OP_SUCCESS;
        } else if (!is_may_be_empty) {
            return ERR_REGEX_NOMATCH;
        }
    }

    // Process intermediate data command (whole message except CRLF is interpreted as payload).
    if (command->cmd == CMD_DATA_INT) {
        command->data = parse_str_data(payload_match, command->message, command->cmd);
        return OP_SUCCESS;
    }

    // Process rest of nonempty or maybe nonempty commands that starts with command code.
    if (sp_match->rm_so != -1 && str_match->rm_so != -1) {
        command->data = parse_str_data(str_match, command->message, command->cmd);
        return OP_SUCCESS;
    }

    return ERR_REGEX_NOMATCH;
}

// Parse optional at-domain list from string matched by '(.*:)?' pattern.
char *parse_path_adl(char *restrict message, const regmatch_t *restrict adl_match, cmd_te cmd) {
    assert(message != NULL && adl_match != NULL);

    if (adl_match->rm_so == -1) {
        return NULL;
    }

    char *adl = NULL;
    size_t adl_len = adl_match->rm_eo - adl_match->rm_so - 1;  // excluding ':'

    if (adl_len > 0) {
        adl = calloc(adl_len + 1, sizeof(char));
        if (adl == NULL) {
            exit_with_error_message(ERR_NOT_ALLOCATED, "%s:adl", cmd_to_str(cmd));
        }
        strncpy(adl, message + adl_match->rm_so, adl_len);
    }

    return adl;
}

// Parse optional command parameters list (used for MAIL and RCPT).
size_t parse_cmd_params(char *restrict message,
                        const regmatch_t *restrict params_match,
                        cmd_te cmd,
                        cmd_param_t **restrict params_out) {
    assert(message != NULL && params_match != NULL && params_out != NULL);

    if (params_match->rm_so == -1) {
        return 0;
    }

    cmd_param_t *params = NULL;
    size_t params_cnt = 0;

    *(message + params_match->rm_eo) = '\0'; // dirty hack, it was too lazy to invent something more elegant

    char *last;
    char *param_part = strtok_r(message + params_match->rm_so, SP, &last);
    while (param_part) {
        params = realloc(params, sizeof(cmd_param_t) * (params_cnt + 1));
        if (params == NULL) {
            exit_with_error_message(ERR_NOT_ALLOCATED, "%s:params", cmd_to_str(cmd));
        }

        size_t key_len = strcspn(param_part, "=");
        size_t value_len = strlen(param_part) - key_len - 1;

        char *key = calloc(key_len + 1, sizeof(char));
        if (key == NULL) {
            exit_with_error_message(ERR_NOT_ALLOCATED, "%s:param_key", cmd_to_str(cmd));
        }

        char *value = calloc(value_len + 1, sizeof(char));
        if (value == NULL) {
            exit_with_error_message(ERR_NOT_ALLOCATED, "%s:param_value", cmd_to_str(cmd));
        }

        strncpy(key, param_part, key_len);
        strncpy(value, param_part + key_len + 1, value_len);

        params[params_cnt].key = key;
        params[params_cnt].value = value;

        params_cnt++;
        param_part = strtok_r(last, SP, &last);
    }

    *params_out = params;

    return params_cnt;
}

err_code_t parse_path_message(command_t *restrict command) {
    assert(command != NULL && command->message != NULL);

    regmatch_t matches[MAX_REGEX_MATCHES];

    // Parse message body.
    if (regexec(&cmd_regexes[cmd_to_rgx(command->cmd)], command->message, MAX_REGEX_MATCHES, matches, 0)) {
        return ERR_REGEX_NOMATCH;
    }

    const regmatch_t *adl_match = matches + PARSE_MAIL_ADL_IDX;
    const regmatch_t *mailbox_match = matches + PARSE_MAIL_MAILBOX_IDX;
    const regmatch_t *params_match = matches + PARSE_MAIL_PARAMS_IDX;

    if (mailbox_match->rm_so == -1) {
        return ERR_REGEX_NOMATCH;
    }

    // Parse mailbox (mandatory).
    char *mailbox = parse_str_data(mailbox_match, command->message, command->cmd);

    // Parse adl (optional).
    char *adl = parse_path_adl(command->message, adl_match, command->cmd);

    // Parse params (optional).
    cmd_param_t *params = NULL;
    size_t params_cnt = parse_cmd_params(command->message, params_match, command->cmd, &params);

    // Store all message data to output structure.
    path_data_t *path_data = (path_data_t *) calloc(1, sizeof(path_data_t));
    if (path_data == NULL) {
        exit_with_error_message(ERR_NOT_ALLOCATED, "%s:data", cmd_to_str(command->cmd));
    }
    path_data->adl = adl;
    path_data->params = params;
    path_data->params_cnt = params_cnt;
    path_data->mailbox = mailbox;
    command->data = path_data;

    return OP_SUCCESS;
}

err_code_t parse_message_data(command_t *restrict command) {
    assert(command != NULL && command->message != NULL);

    switch (command->cmd) {
    case CMD_MAIL: case CMD_RCPT:
            return parse_path_message(command);
        default:
            return parse_str_message(command);
    }
}

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

    for (i = 0; i < RGX_NUM && compile_regex(&cmd_regexes[i], cmd_regex_patterns[i], error_message) == OP_SUCCESS; i++);
    if (i < RGX_NUM) {
        exit_with_error_message(ERR_REGEX_COMPILE, error_message);
    }
    const dtor_t cmd_regexes_dtor = {
            .regex_dtor = {.call = free_regexes, .regexes = cmd_regexes, .cnt = RGX_NUM},
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

err_code_t parse_smtp_message(command_t *restrict command) {
    assert(command != NULL && command->message != NULL);

    size_t message_len = strlen(command->message);

    // Parse command code
    if (message_len < EMPTY_CMD_LENGTH ||
        cmd_code_to_cmd_event(command->message, &command->cmd, &command->event) < 0) {

        if (command->conn_state->fsm_state == SMTP_SERVER_FSM_ST_DATA) {
            command->cmd = CMD_DATA_INT;
            command->event = SMTP_SERVER_FSM_EV_RECV_DATA_INT;

            return parse_message_data(command);
        } else {
            return ERR_SMTP_INVALID_COMMAND;
        }
    }

    command->message += CMD_CODE_LENGTH;

    return parse_message_data(command);
}

// Destructor command_t structure. Tried to free all
// available data if not null except command.message,
// because it is supposed to be always allocated on stack.
void command_dtor(command_t *restrict command) {
    if (command == NULL) return;

    if (command->data != NULL) {
        if (command->cmd == CMD_MAIL || command->cmd == CMD_RCPT) {
            path_data_t *path_data = (path_data_t*)command->data;
            if (path_data->adl) {
                free((void*)path_data->adl);
            }
            if (path_data->mailbox) {
                free((void*)path_data->mailbox);
            }
            if (path_data->params) {
                for (int i = 0; i < path_data->params_cnt; i++) {
                    free((void*)path_data->params[i].key);
                    free((void*)path_data->params[i].value);
                }
                free((void*)path_data->params);
            }
        }

        free((void*)command->data);
        command->data = NULL;
    }
}

char *heap_str(const char *restrict str) {
    assert(str);

    size_t str_len = strlen(str);
    char *heap_data = calloc(str_len + 1, sizeof(char));
    if (heap_data == NULL) {
        exit_with_error_message(ERR_NOT_ALLOCATED, "copy_str_to_heap");
    }

    strncpy(heap_data, str, str_len);

    return heap_data;
}
