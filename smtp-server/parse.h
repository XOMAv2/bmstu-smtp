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

#define CMD_NUM 9

#define MAX_REGEX_MATCHES 10

typedef enum {
    HELO, EHLO, MAIL,
    RCPT, DATA, RSET,
    NOOP, VRFY, QUIT
} cmd_te;

// Client command data structure each received client message is wrapped in
typedef struct {
    cmd_te cmd;
    te_smtp_server_fsm_event event;
    void *data;
} command_t;

void compile_regexes(void);

void free_regexes(int until);

err_code_t parse_smtp_message(char *restrict message, command_t *restrict command);

#endif //NETWORK_PROTOCOLS_CP_PARSE_H
