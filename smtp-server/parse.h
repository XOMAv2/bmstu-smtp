//
// Created by Karpukhin Aleksandr on 25.02.2022.
//

#ifndef NETWORK_PROTOCOLS_CP_PARSE_H
#define NETWORK_PROTOCOLS_CP_PARSE_H

#include "server-fsm.h"
#include "error.h"

#define SP " "
#define CRLF "\r\n"

#define CMD_PLUS_SP_LENGTH 5

#define SMTP_HELO_CMD_CODE "HELO"
#define SMTP_EHLO_CMD_CODE "EHLO"
#define SMTP_MAIL_CMD_CODE "MAIL"
#define SMTP_RCPT_CMD_CODE "RCPT"
#define SMTP_DATA_CMD_CODE "DATA"
#define SMTP_RSET_CMD_CODE "RSET"
#define SMTP_NOOP_CMD_CODE "NOOP"
#define SMTP_VRFY_CMD_CODE "VRFY"
#define SMTP_QUIT_CMD_CODE "QUIT"

#define PARSE_MAIL_ADL_IDX       1
#define PARSE_MAIL_MAILBOX_IDX   2
#define PARSE_MAIL_PARAMS_IDX    3

#define PARSE_RCPT_PM_IDX        2
#define PARSE_RCPT_PMD_IDX       3
#define PARSE_RCPT_ADL_IDX       4
#define PARSE_RCPT_MAILBOX_IDX   5
#define PARSE_RCPT_PARAMS_IDX    6

#define CMD_NUM                  9
#define MAIL_REGEX_NUM           3
#define MAX_REGEX_MATCHES        10

// Command indices enum (mostly for extracting regex parsers).
typedef enum {
    HELO, EHLO, MAIL,
    RCPT, DATA, RSET,
    NOOP, VRFY, QUIT
} cmd_te;

// Client command data structure each received client message is wrapped in.
typedef struct {
    // Command code which using for extracting
    // message parser from precompiled parsers list.
    cmd_te cmd;
    // Event that will be defined according to
    // message content and passed to connection FSM.
    te_smtp_server_fsm_event event;
    // Command data (structure is command-dependent).
    void *data;
} command_t;

// Optional MAIL param structure (simply a key-value pair).
typedef struct {
    char *key;
    char *value;
} mail_param_t;

// Data for path containing command (MAIL or RCPT) receiving
// by server from client (according to RFC-5321).
typedef struct {
    // At-domain list (can be used for source
    // routing, but highly not recommended).
    char *adl;
    // Sender mailbox address.
    char *mailbox;
    // Optional MAIL parameters (can be ignored and for now it is).
    mail_param_t *params;
    // Optional MAIL parameters number.
    size_t params_cnt;
} path_data_t;

void compile_regexes(void);

void free_regexes(regex_t *restrict regexes, int until);

err_code_t parse_smtp_message(char *restrict message, command_t *restrict command);

#endif //NETWORK_PROTOCOLS_CP_PARSE_H
