//
// Created by Karpukhin Aleksandr on 25.02.2022.
//

#ifndef NETWORK_PROTOCOLS_CP_PARSE_H
#define NETWORK_PROTOCOLS_CP_PARSE_H

#include <regex.h>

#include "server-fsm.h"
#include "error.h"

#define SP " "
#define POINT '.'
#define CRLF "\r\n"

#define CMD_CODE_LENGTH    4
#define EMPTY_CMD_LENGTH   6    // 4 code chars + \r\n

#define SMTP_HELO_CMD_CODE "HELO"
#define SMTP_EHLO_CMD_CODE "EHLO"
#define SMTP_MAIL_CMD_CODE "MAIL"
#define SMTP_RCPT_CMD_CODE "RCPT"
#define SMTP_DATA_CMD_CODE "DATA"
#define SMTP_RSET_CMD_CODE "RSET"
#define SMTP_NOOP_CMD_CODE "NOOP"
#define SMTP_VRFY_CMD_CODE "VRFY"
#define SMTP_QUIT_CMD_CODE "QUIT"

#define PARSE_STR_PAYLOAD_IDX    1
#define PARSE_STR_SP_IDX         2
#define PARSE_STR_DATA_IDX       3

#define PARSE_MAIL_ADL_IDX       1
#define PARSE_MAIL_MAILBOX_IDX   2
#define PARSE_MAIL_PARAMS_IDX    3

#define PARSE_RCPT_PM_IDX        2
#define PARSE_RCPT_PMD_IDX       3
#define PARSE_RCPT_ADL_IDX       4
#define PARSE_RCPT_MAILBOX_IDX   5
#define PARSE_RCPT_PARAMS_IDX    6

#define CMD_NUM                  10
#define RGX_NUM                  3
#define MAIL_REGEX_NUM           3
#define MAX_REGEX_MATCHES        10

// Command indices enum for extracting regex parsers.
typedef enum {
    CMD_HELO, CMD_EHLO, CMD_MAIL,
    CMD_RCPT, CMD_DATA, CMD_DATA_INT,
    CMD_RSET, CMD_NOOP, CMD_VRFY, CMD_QUIT
} cmd_te;

// Regex enum for extracting predefined command data regex patterns.
typedef enum {
    RGX_STR = 0,   // single string message data pattern (HELO/EHLO/DATA/NOOP/VRFY)
    RGX_MAIL,  // MAIL message data pattern
    RGX_RCPT,  // RCPT message data pattern
} rgx_te;

// Optional MAIL param structure (simply a key-value pair).
typedef struct {
    const char *key;
    const char *value;
} cmd_param_t;

// Data for path containing command (MAIL or RCPT) receiving
// by server from client (according to RFC-5321).
typedef struct {
    // At-domain list (can be used for source
    // routing, but highly not recommended).
    const char *adl;
    // Sender mailbox address.
    const char *mailbox;
    // Optional command parameters (can be ignored and for now it is).
    const cmd_param_t *params;
    // Optional command parameters number.
    size_t params_cnt;
} path_data_t;

// Client command data structure each received client message is wrapped in.
// It passes to parsers and then to the FSM, so it carries all processing pipeline data.
typedef struct {
    // Command code which using for extracting
    // message parser from precompiled parsers list.
    cmd_te cmd;
    // Message received from connection socket.
    char *restrict message;
    // Current connection state in which command was received.
    te_smtp_server_fsm_state state;
    // Event that will be defined according to
    // message content and passed to connection FSM.
    te_smtp_server_fsm_event event;
    // Command data (structure is command-dependent).
    const void *data;
} command_t;

// Compile all predefined regexes in 'parse' module.
// It stores in static fields not accessible from outside of module
// and can be modified only through destruction.
void compile_regexes(void);

// Single regex collection destructor.
void free_regexes(regex_t *restrict regexes, int until);

// Parse incoming client message wrapped in command_t structure and fill in all
// fields in command that are necessary for next processing pipeline stages (FSM).
// parse_smtp_message fills in command.cmd and command.event fields and then calls
// message parser that fills in command.data field.
err_code_t parse_smtp_message(command_t *restrict command);

// Get string command code by cmd_te enum value.
const char* cmd_to_str(cmd_te cmd);

// Get FSM event enum value by cmd_te enum value.
te_smtp_server_fsm_event cmd_to_event(cmd_te cmd);

// Get regex pattern index in static regex pool by cmd_te enum value.
rgx_te cmd_to_rgx(cmd_te cmd);

// Check if command with specified numeric code must have empty payload.
_Bool is_empty_cmd(cmd_te cmd);

// Check if command with specified numeric code can have empty payload.
_Bool is_may_be_empty_cmd(cmd_te cmd);

// Copy nonempty null terminated string to preliminarily allocated heap memory.
// New string will be null terminated too.
// Allocate singe char with zero code in case of empty input string.
// Call exit() in case memory allocation error.
char *heap_str(const char *restrict str);

#endif //NETWORK_PROTOCOLS_CP_PARSE_H
