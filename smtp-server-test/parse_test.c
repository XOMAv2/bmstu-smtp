//
// Created by Karpukhin Aleksandr on 06.03.2022.
//

#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "parse_test.h"
#include "parse.h"
#include "defines.h"

void test_parse_str_scenario(cmd_te cmd, const char *restrict message, const char *restrict crlf, _Bool is_parsed) {
    assert(crlf != NULL);

    char message_buf[MAX_MESSAGE_LENGTH] = {0};

    if (message) {
        snprintf(message_buf, MAX_MESSAGE_LENGTH, "%s %s%s", cmd_to_str(cmd), message, crlf);
    } else {
        snprintf(message_buf, MAX_MESSAGE_LENGTH, "%s%s", cmd_to_str(cmd), crlf);
    }

    conn_state_t conn_state = { .fsm_state = SMTP_SERVER_FSM_ST_INIT };
    command_t command = { .message = message_buf, .conn_state = &conn_state };

    if (!is_parsed) {
        assert(parse_smtp_message(&command) != OP_SUCCESS);
        return;
    }

    assert(parse_smtp_message(&command) == OP_SUCCESS);
    assert(command.cmd == cmd);
    assert(command.event == cmd_to_event(cmd));
    assert(command.conn_state->fsm_state == SMTP_SERVER_FSM_ST_INIT);
    if (message) {
        assert(strcmp((char *) command.data, message) == 0);
    } else {
        assert(command.data == NULL);
    }
}

void test_parse_data_int_scenario(const char *restrict message, const char *restrict crlf, _Bool is_parsed) {
    assert(crlf != NULL);

    char message_buf[MAX_MESSAGE_LENGTH] = {0};
    if (message) {
        snprintf(message_buf, MAX_MESSAGE_LENGTH, "%s%s", message, crlf);
    } else {
        snprintf(message_buf, MAX_MESSAGE_LENGTH, "%s", crlf);
    }

    conn_state_t conn_state = { .fsm_state = SMTP_SERVER_FSM_ST_DATA };
    command_t command = { .message = message_buf, .conn_state = &conn_state };

    if (!is_parsed) {
        assert(parse_smtp_message(&command) != OP_SUCCESS);
        return;
    }

    assert(parse_smtp_message(&command) == OP_SUCCESS);
    assert(command.cmd == CMD_DATA_INT);
    assert(command.event == SMTP_SERVER_FSM_EV_RECV_DATA_INT);
    assert(command.conn_state->fsm_state == SMTP_SERVER_FSM_ST_DATA);
    if (message) {
        assert(strcmp((char *) command.data, message) == 0);
    } else {
        assert(command.data == NULL);
    }
}

void test_parse_path_scenario(const char *restrict prefix,
                              cmd_te cmd,
                              const char *restrict adl,
                              const char *restrict mailbox,
                              const cmd_param_t *restrict params,
                              size_t params_cnt,
                              const char *restrict crlf,
                              _Bool is_parsed) {
    assert(prefix && mailbox && crlf);

    char message[MAX_MESSAGE_LENGTH] = {0};

    if (adl) {
        snprintf(message,
                 MAX_MESSAGE_LENGTH,
                 "%s %s:<%s:%s>",
                 cmd_to_str(cmd), prefix, adl, mailbox);
    } else {
        snprintf(message,
                 MAX_MESSAGE_LENGTH,
                 "%s %s:<%s>",
                 cmd_to_str(cmd), prefix, mailbox);
    }

    size_t message_len, i;
    for (message_len = strlen(message), i = 0; params && i < params_cnt; message_len = strlen(message), i++) {
        snprintf(message + message_len, MAX_MESSAGE_LENGTH - message_len, " %s=%s", params[i].key, params[i].value);
    }
    snprintf(message + message_len, MAX_MESSAGE_LENGTH - message_len, "%s", crlf);

    conn_state_t conn_state = { .fsm_state = SMTP_SERVER_FSM_ST_INIT };
    command_t command = { .message = message, .conn_state = &conn_state };

    if (!is_parsed) {
        assert(parse_smtp_message(&command) != OP_SUCCESS);
        return;
    }

    // Assert command metadata.
    assert(parse_smtp_message(&command) == OP_SUCCESS);
    assert(command.cmd == cmd);
    assert(command.event == cmd_to_event(cmd));
    assert(command.conn_state->fsm_state == SMTP_SERVER_FSM_ST_INIT);

    // Assert command data.
    path_data_t *path_data = (path_data_t *) command.data;
    assert(path_data != NULL);

    if (adl && strlen(adl)) {
        assert(strcmp(path_data->adl, adl) == 0);
    } else {
        assert(path_data->adl == NULL);
    }

    assert(strcmp(path_data->mailbox, mailbox) == 0);
    assert(path_data->params_cnt == params_cnt);

    if (params_cnt) {
        for (i = 0; i < params_cnt; i++) {
            assert(strcmp(path_data->params[i].key, params[i].key) == 0);
            assert(strcmp(path_data->params[i].value, params[i].value) == 0);
        }
    } else {
        assert(path_data->params == NULL);
    }
}

static cmd_param_t valid_params[2] = {
        {.key = "param_1", .value = "value_1" },
        {.key = "param_2", .value = "value_2" },
};

static cmd_param_t invalid_key_params[1] = {
        {.key = "", .value = "value_1" },
};

static cmd_param_t invalid_value_params[1] = {
        {.key = "param_1", .value = "" },
};

void before_test(void) {
    compile_regexes();
}

void after_test(void) {
}

void test_parse_helo(void) {
    test_parse_str_scenario(CMD_HELO, "domain", CRLF, true);   // valid, nonempty domain
    test_parse_str_scenario(CMD_HELO, "", CRLF, false);        // invalid, empty domain with SP
    test_parse_str_scenario(CMD_HELO, NULL, CRLF, false);      // invalid, empty domain

    test_parse_str_scenario(CMD_HELO, "domain", "\r", false);   // invalid crlf
    test_parse_str_scenario(CMD_HELO, "domain", "\n", false);   // invalid crlf
    test_parse_str_scenario(CMD_HELO, "domain", "", false);     // invalid crlf

    printf("\ntest_parse_helo successful\n");
}

void test_parse_ehlo(void) {
    test_parse_str_scenario(CMD_EHLO, "domain", CRLF, true);  // valid, nonempty domain
    test_parse_str_scenario(CMD_EHLO, "", CRLF, false);       // invalid, empty domain with SP
    test_parse_str_scenario(CMD_EHLO, NULL, CRLF, false);     // invalid, empty domain

    test_parse_str_scenario(CMD_EHLO, "domain", "\r", false);   // invalid crlf
    test_parse_str_scenario(CMD_EHLO, "domain", "\n", false);   // invalid crlf
    test_parse_str_scenario(CMD_EHLO, "domain", "", false);     // invalid crlf

    printf("\ntest_parse_ehlo successful\n");
}



void test_parse_mail(void) {
    test_parse_path_scenario("FROM", CMD_MAIL, "some.adl", "domain@main", valid_params, 2, CRLF, true);    // valid
    test_parse_path_scenario("FROM", CMD_MAIL, "some.adl", "domain@main", NULL, 0, CRLF, true);            // valid, empty params
    test_parse_path_scenario("FROM", CMD_MAIL, "", "domain@main", valid_params, 2, CRLF, true);            // valid, empty adl
    test_parse_path_scenario("FROM", CMD_MAIL, "", "domain@main", NULL, 0, CRLF, true);                    // valid, empty adl and params
    test_parse_path_scenario("FROM", CMD_MAIL, NULL, "domain@main", NULL, 0, CRLF, true);

    test_parse_path_scenario("FROM", CMD_MAIL, NULL, "domain@main", invalid_key_params, 1, CRLF, false);    // invalid param key
    test_parse_path_scenario("FROM", CMD_MAIL, NULL, "domain@main", invalid_value_params, 1, CRLF, false);  // invalid param value

    test_parse_path_scenario("INVALID", CMD_MAIL, NULL, "domain@main", NULL, 0, CRLF, false);   // invalid prefix
    test_parse_path_scenario("FROM", CMD_MAIL, NULL, "Postmaster", NULL, 0, CRLF, false);      // invalid mailbox
    test_parse_path_scenario("FROM", CMD_MAIL, NULL, "", NULL, 0, CRLF, false);                // empty mailbox

    test_parse_path_scenario("FROM", CMD_MAIL, NULL, "domain@main", NULL, 0, "\r", false);     // invalid crlf
    test_parse_path_scenario("FROM", CMD_MAIL, NULL, "domain@main", NULL, 0, "\n", false);     // invalid crlf
    test_parse_path_scenario("FROM", CMD_MAIL, NULL, "domain@main", NULL, 0, "", false);       // invalid crlf

    printf("\ntest_parse_mail successful\n");
}



void test_parse_rcpt(void) {
    test_parse_path_scenario("TO", CMD_RCPT, "some.adl", "domain@main", valid_params, 2, CRLF, true);    // valid
    test_parse_path_scenario("TO", CMD_RCPT, "some.adl", "Postmaster", valid_params, 2, CRLF, true);     // valid, mailbox special case
    test_parse_path_scenario("TO", CMD_RCPT, "", "Postmaster", valid_params, 2, CRLF, true);             // valid, empty adl, ':' is present
    test_parse_path_scenario("TO", CMD_RCPT, NULL, "Postmaster", valid_params, 2, CRLF, true);           // valid, empty adl
    test_parse_path_scenario("TO", CMD_RCPT, "some.adl", "Postmaster", NULL, 0, CRLF, true);             // valid, empty params
    test_parse_path_scenario("TO", CMD_RCPT, NULL, "Postmaster", NULL, 0, CRLF, true);                   // valid, empty adl and params

    test_parse_path_scenario("INVALID", CMD_RCPT, NULL, "Postmaster", NULL, 0, CRLF, false);             // invalid prefix
    test_parse_path_scenario("TO", CMD_RCPT, NULL, "", NULL, 0, CRLF, false);                            // empty mailbox
    test_parse_path_scenario("TO", CMD_RCPT, NULL, "domain@main", invalid_key_params, 1, CRLF, false);   // invalid param key
    test_parse_path_scenario("TO", CMD_RCPT, NULL, "domain@main", invalid_value_params, 1, CRLF, false); // invalid param value

    test_parse_path_scenario("TO", CMD_RCPT, NULL, "domain@main", NULL, 0, "\r", false);     // invalid crlf
    test_parse_path_scenario("TO", CMD_RCPT, NULL, "domain@main", NULL, 0, "\n", false);     // invalid crlf
    test_parse_path_scenario("TO", CMD_RCPT, NULL, "domain@main", NULL, 0, "", false);       // invalid crlf

    printf("\ntest_parse_rcpt successful\n");
}

void test_parse_data(void) {
    test_parse_str_scenario(CMD_DATA, NULL, CRLF, true);      // valid, empty payload
    test_parse_str_scenario(CMD_DATA, "", CRLF, false);       // valid, empty payload with SP
    test_parse_str_scenario(CMD_DATA, "smth", CRLF, false);   // invalid, nonempty payload

    test_parse_str_scenario(CMD_DATA, NULL, "\r", false);      // invalid crlf
    test_parse_str_scenario(CMD_DATA, NULL, "\n", false);      // invalid crlf
    test_parse_str_scenario(CMD_DATA, NULL, "", false);        // invalid crlf

    printf("\ntest_parse_data successful\n");
}

void test_parse_data_int(void) {
    test_parse_data_int_scenario("smth", CRLF, true);    // valid, nonempty payload
    test_parse_data_int_scenario(".", CRLF, true);       // valid, ending sequence
    test_parse_data_int_scenario(NULL, CRLF, true);      // valid, empty payload

    test_parse_data_int_scenario("smth", "\r", false);    // invalid crlf
    test_parse_data_int_scenario("smth", "\n", false);    // invalid crlf
    test_parse_data_int_scenario("smth", "", false);      // invalid crlf

    test_parse_data_int_scenario(NULL, "\r", false);      // invalid crlf
    test_parse_data_int_scenario(NULL, "\n", false);      // invalid crlf
    test_parse_data_int_scenario(NULL, "", false);        // invalid crlf

    printf("\ntest_parse_data_int successful\n");
}

void test_parse_noop(void) {
    test_parse_str_scenario(CMD_NOOP, "smth", CRLF, true);    // valid, nonempty payload
    test_parse_str_scenario(CMD_NOOP, NULL, CRLF, true);      // valid, empty payload
    test_parse_str_scenario(CMD_NOOP, "", CRLF, false);       // invali, empty payload with SP

    test_parse_str_scenario(CMD_NOOP, "smth", "\r", false);      // invalid crlf
    test_parse_str_scenario(CMD_NOOP, "smth", "\n", false);      // invalid crlf
    test_parse_str_scenario(CMD_NOOP, "smth", "", false);        // invalid crlf

    test_parse_str_scenario(CMD_NOOP, NULL, "\r", false);      // invalid crlf
    test_parse_str_scenario(CMD_NOOP, NULL, "\n", false);      // invalid crlf
    test_parse_str_scenario(CMD_NOOP, NULL, "", false);        // invalid crlf

    printf("\ntest_parse_noop successful\n");
}

void test_parse_rset(void) {
    test_parse_str_scenario(CMD_RSET, NULL, CRLF, true);      // valid, empty payload
    test_parse_str_scenario(CMD_RSET, "", CRLF, false);       // valid, empty payload with SP
    test_parse_str_scenario(CMD_RSET, "smth", CRLF, false);   // invalid, nonempty payload

    test_parse_str_scenario(CMD_RSET, NULL, "\r", false);      // invalid crlf
    test_parse_str_scenario(CMD_RSET, NULL, "\n", false);      // invalid crlf
    test_parse_str_scenario(CMD_RSET, NULL, "", false);        // invalid crlf

    printf("\ntest_parse_rset successful\n");
}

void test_parse_vrfy(void) {
    test_parse_str_scenario(CMD_VRFY, "smth", CRLF, true);    // valid, nonempty payload
    test_parse_str_scenario(CMD_VRFY, "", CRLF, false);       // invalid, empty payload with SP
    test_parse_str_scenario(CMD_VRFY, NULL, CRLF, false);     // invalid, empty payload

    test_parse_str_scenario(CMD_VRFY, "smth", "\r", false);    // invalid crlf
    test_parse_str_scenario(CMD_VRFY, "smth", "\n", false);    // invalid crlf
    test_parse_str_scenario(CMD_VRFY, "smth", "", false);      // invalid crlf

    printf("\ntest_parse_vrfy successful\n");
}

void test_parse_quit(void) {
    test_parse_str_scenario(CMD_QUIT, NULL, CRLF, true);      // valid, empty payload
    test_parse_str_scenario(CMD_QUIT, "", CRLF, false);       // valid, empty payload with SP
    test_parse_str_scenario(CMD_QUIT, "smth", CRLF, false);   // invalid, nonempty payload

    test_parse_str_scenario(CMD_QUIT, NULL, "\r", false);      // invalid crlf
    test_parse_str_scenario(CMD_QUIT, NULL, "\n", false);      // invalid crlf
    test_parse_str_scenario(CMD_QUIT, NULL, "", false);        // invalid crlf

    printf("\ntest_parse_quit successful\n");
}
