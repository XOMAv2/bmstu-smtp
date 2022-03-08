//
// Created by Karpukhin Aleksandr on 08.03.2022.
//

#ifndef BMSTU_SMTP_DATA_H
#define BMSTU_SMTP_DATA_H

#include <poll.h>
#include <stddef.h>

#include "destruction.h"

// Command indices enum for extracting regex parsers.
typedef enum {
    CMD_HELO, CMD_EHLO, CMD_MAIL,
    CMD_RCPT, CMD_DATA, CMD_DATA_INT,
    CMD_RSET, CMD_NOOP, CMD_VRFY, CMD_QUIT
} cmd_te;

/**
 *  Finite State machine States
 *
 *  Count of non-terminal states.  The generated states INVALID and DONE
 *  are terminal, but INIT is not  :-).
 */

#define SMTP_SERVER_FSM_STATE_CT  9
typedef enum {
    SMTP_SERVER_FSM_ST_INIT,    SMTP_SERVER_FSM_ST_READY,
    SMTP_SERVER_FSM_ST_HELO,    SMTP_SERVER_FSM_ST_MAIL,
    SMTP_SERVER_FSM_ST_RCPT,    SMTP_SERVER_FSM_ST_DATA,
    SMTP_SERVER_FSM_ST_SEND,    SMTP_SERVER_FSM_ST_QUIT,
    SMTP_SERVER_FSM_ST_TIMEOUT, SMTP_SERVER_FSM_ST_INVALID,
    SMTP_SERVER_FSM_ST_DONE
} te_smtp_server_fsm_state;

/**
 *  Finite State machine transition Events.
 *
 *  Count of the valid transition events
 */
#define SMTP_SERVER_FSM_EVENT_CT 16
typedef enum {
    SMTP_SERVER_FSM_EV_RECV_HELO,     SMTP_SERVER_FSM_EV_RECV_EHLO,
    SMTP_SERVER_FSM_EV_RECV_MAIL,     SMTP_SERVER_FSM_EV_RECV_RCPT,
    SMTP_SERVER_FSM_EV_RECV_RSET,     SMTP_SERVER_FSM_EV_RECV_DATA,
    SMTP_SERVER_FSM_EV_RECV_DATA_INT, SMTP_SERVER_FSM_EV_RECV_QUIT,
    SMTP_SERVER_FSM_EV_RECV_NOOP,     SMTP_SERVER_FSM_EV_RECV_VRFY,
    SMTP_SERVER_FSM_EV_RECV_UNKNOWN,  SMTP_SERVER_FSM_EV_CONN_EST,
    SMTP_SERVER_FSM_EV_CONN_TIMEOUT,  SMTP_SERVER_FSM_EV_CONN_LOST,
    SMTP_SERVER_FSM_EV_MSG_SAVED,     SMTP_SERVER_FSM_EV_TERM_SEQ,
    SMTP_SERVER_FSM_EV_INVALID
} te_smtp_server_fsm_event;

typedef struct pollfd pollfd_t;

typedef struct {
    pollfd_t pollfd;
    dtor_id_t dtor_id;
    te_smtp_server_fsm_state fsm_state;
} conn_state_t;

// Client command data structure each received client message is wrapped in.
// It passes to parsers and then to the FSM, so it carries all processing pipeline data.
typedef struct {
    // Command code which using for extracting
    // message parser from precompiled parsers list.
    // Set at the beginning of parsing stage.
    cmd_te cmd;

    // Message received from connection socket.
    // Set at the beginning of parsing stage and may be changed during parsing stage.
    char *restrict message;

    // Current connection state in which command was received.
    // Set at the creation and may be changed at the FSM processing stage.
    // It is supposed that this is pointer to the existing record in the connection state pool,
    // so all changes in the connection state will be in the pool immediately.
    conn_state_t *restrict conn_state;

    // Event that will be defined according to
    // message content and passed to connection FSM.
    // Set at the beginning of the parsing stage.
    te_smtp_server_fsm_event event;

    // Command data (structure is command-dependent).
    // Set at the end of parsing stage.
    const void *data;
} command_t;

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

#endif //BMSTU_SMTP_DATA_H
