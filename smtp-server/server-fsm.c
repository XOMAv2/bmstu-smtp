/*   -*- buffer-read-only: t -*- vi: set ro:
 *
 *  DO NOT EDIT THIS FILE   (server-fsm.c)
 *
 *  It has been AutoGen-ed
 *  From the definitions    server.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (C) 1992-2018 Bruce Korb - all rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Bruce Korb'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * AutoFSM IS PROVIDED BY Bruce Korb ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bruce Korb OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define DEFINE_FSM

#include <stdio.h>
#include <stdlib.h>

#include "server-fsm.h"
#include "defines.h"
#include "log.h"

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
/* START === USER HEADERS === DO NOT CHANGE THIS COMMENT */
/* END   === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#ifndef NULL
#  define NULL 0
#endif

/**
 *  Callback routine prototype.  They return the next state.  Normally, that
 *  should be the value of the "maybe_next" argument.
 */
typedef te_smtp_server_fsm_state (smtp_server_fsm_callback_t)(
    command_t *restrict command,
    te_smtp_server_fsm_state maybe_next);

static smtp_server_fsm_callback_t
    smtp_server_fsm_do_accepted,
    smtp_server_fsm_do_close,
    smtp_server_fsm_do_data,
    smtp_server_fsm_do_ehlo,
    smtp_server_fsm_do_helo,
    smtp_server_fsm_do_invalid,
    smtp_server_fsm_do_mail,
    smtp_server_fsm_do_mail_received,
    smtp_server_fsm_do_quit,
    smtp_server_fsm_do_rcpt,
    smtp_server_fsm_do_rset,
    smtp_server_fsm_do_timeout,
    smtp_server_fsm_do_vrfy;

/**
 *  Declare all the state transition handling routines.
 */
typedef struct transition t_smtp_server_fsm_transition;
struct transition {
    te_smtp_server_fsm_state      next_state;
    smtp_server_fsm_callback_t *  trans_proc;
};

/**
 *  State transition maps.  Map the enumeration and the event enumeration
 *  to the new state and the transition enumeration code (in that order).
 *  It is indexed by first the current state and then the event code.
 */
static const t_smtp_server_fsm_transition
smtp_server_fsm_trans_table[ SMTP_SERVER_FSM_STATE_CT ][ SMTP_SERVER_FSM_EVENT_CT ] = {

  /* STATE 0:  SMTP_SERVER_FSM_ST_INIT */
  { { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_READY, smtp_server_fsm_do_accepted }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid } /* EVT:  TERM_SEQ */
  },

  /* STATE 1:  SMTP_SERVER_FSM_ST_READY */
  { { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_helo }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_ehlo }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_READY, smtp_server_fsm_do_rset }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_QUIT, smtp_server_fsm_do_quit }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_READY, smtp_server_fsm_do_vrfy }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_TIMEOUT, smtp_server_fsm_do_timeout }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_DONE, smtp_server_fsm_do_close }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid } /* EVT:  TERM_SEQ */
  },

  /* STATE 2:  SMTP_SERVER_FSM_ST_HELO */
  { { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_helo }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_ehlo }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_MAIL, smtp_server_fsm_do_mail }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_rset }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_QUIT, smtp_server_fsm_do_quit }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_vrfy }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_TIMEOUT, smtp_server_fsm_do_timeout }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_DONE, smtp_server_fsm_do_close }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid } /* EVT:  TERM_SEQ */
  },

  /* STATE 3:  SMTP_SERVER_FSM_ST_MAIL */
  { { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_helo }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_ehlo }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_RCPT, smtp_server_fsm_do_rcpt }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_rset }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_QUIT, smtp_server_fsm_do_quit }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_MAIL, smtp_server_fsm_do_vrfy }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_TIMEOUT, smtp_server_fsm_do_timeout }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_DONE, smtp_server_fsm_do_close }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid } /* EVT:  TERM_SEQ */
  },

  /* STATE 4:  SMTP_SERVER_FSM_ST_RCPT */
  { { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_helo }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_ehlo }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_RCPT, smtp_server_fsm_do_rcpt }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_rset }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_DATA, smtp_server_fsm_do_data }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_QUIT, smtp_server_fsm_do_quit }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_RCPT, smtp_server_fsm_do_vrfy }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_TIMEOUT, smtp_server_fsm_do_timeout }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_DONE, smtp_server_fsm_do_close }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid } /* EVT:  TERM_SEQ */
  },

  /* STATE 5:  SMTP_SERVER_FSM_ST_DATA */
  { { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_DATA, smtp_server_fsm_do_data }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_TIMEOUT, smtp_server_fsm_do_timeout }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_DONE, smtp_server_fsm_do_close }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_HELO, smtp_server_fsm_do_mail_received } /* EVT:  TERM_SEQ */
  },

  /* STATE 6:  SMTP_SERVER_FSM_ST_SEND */
  { { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid } /* EVT:  TERM_SEQ */
  },

  /* STATE 7:  SMTP_SERVER_FSM_ST_QUIT */
  { { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_DONE, smtp_server_fsm_do_close }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid } /* EVT:  TERM_SEQ */
  },

  /* STATE 8:  SMTP_SERVER_FSM_ST_TIMEOUT */
  { { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_HELO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_EHLO */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_MAIL */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RCPT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_RSET */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_DATA_INT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_QUIT */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_NOOP */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_VRFY */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  RECV_UNKNOWN */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_EST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  CONN_TIMEOUT */
    { SMTP_SERVER_FSM_ST_DONE, smtp_server_fsm_do_close }, /* EVT:  CONN_LOST */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid }, /* EVT:  MSG_SAVED */
    { SMTP_SERVER_FSM_ST_INVALID, smtp_server_fsm_do_invalid } /* EVT:  TERM_SEQ */
  }
};


#define Smtp_Server_FsmFsmErr_off     19
#define Smtp_Server_FsmEvInvalid_off  75
#define Smtp_Server_FsmStInit_off     83


static char const zSmtp_Server_FsmStrings[300] =
/*     0 */ "** OUT-OF-RANGE **\0"
/*    19 */ "FSM Error:  in state %d (%s), event %d (%s) is invalid\n\0"
/*    75 */ "invalid\0"
/*    83 */ "init\0"
/*    88 */ "ready\0"
/*    94 */ "helo\0"
/*    99 */ "mail\0"
/*   104 */ "rcpt\0"
/*   109 */ "data\0"
/*   114 */ "send\0"
/*   119 */ "quit\0"
/*   124 */ "timeout\0"
/*   132 */ "recv_helo\0"
/*   142 */ "recv_ehlo\0"
/*   152 */ "recv_mail\0"
/*   162 */ "recv_rcpt\0"
/*   172 */ "recv_rset\0"
/*   182 */ "recv_data\0"
/*   192 */ "recv_data_int\0"
/*   206 */ "recv_quit\0"
/*   216 */ "recv_noop\0"
/*   226 */ "recv_vrfy\0"
/*   236 */ "recv_unknown\0"
/*   249 */ "conn_est\0"
/*   258 */ "conn_timeout\0"
/*   271 */ "conn_lost\0"
/*   281 */ "msg_saved\0"
/*   291 */ "term_seq";

static const size_t aszSmtp_Server_FsmStates[9] = {
    83,  88,  94,  99,  104, 109, 114, 119, 124 };

static const size_t aszSmtp_Server_FsmEvents[17] = {
    132, 142, 152, 162, 172, 182, 192, 206, 216, 226, 236, 249, 258, 271, 281,
    291, 75 };


#define SMTP_SERVER_FSM_EVT_NAME(t)   ( (((unsigned)(t)) >= 17) \
    ? zSmtp_Server_FsmStrings : zSmtp_Server_FsmStrings + aszSmtp_Server_FsmEvents[t])

#define SMTP_SERVER_FSM_STATE_NAME(s) ( (((unsigned)(s)) >= 9) \
    ? zSmtp_Server_FsmStrings : zSmtp_Server_FsmStrings + aszSmtp_Server_FsmStates[s])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

static int smtp_server_fsm_invalid_transition( te_smtp_server_fsm_state st, te_smtp_server_fsm_event evt );

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * */
/**
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
static int
smtp_server_fsm_invalid_transition( te_smtp_server_fsm_state st, te_smtp_server_fsm_event evt )
{
    /* START == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */
    char const *fmt = zSmtp_Server_FsmStrings + Smtp_Server_FsmFsmErr_off;
    log_error(fmt, st, SMTP_SERVER_FSM_STATE_NAME(st), evt, SMTP_SERVER_FSM_EVT_NAME(evt));
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return ERR_SMTP_BAD_SEQ;
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_accepted(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == ACCEPTED == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_accepted");
    return maybe_next;
/*  END   == ACCEPTED == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_close(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == CLOSE == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_close");
    return maybe_next;
/*  END   == CLOSE == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_data(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == DATA == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_data");
    return maybe_next;
/*  END   == DATA == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_ehlo(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == EHLO == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_ehlo");
    return maybe_next;
/*  END   == EHLO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_helo(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == HELO == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_helo");
    return maybe_next;
/*  END   == HELO == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_invalid(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == INVALID == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_invalid");
    return smtp_server_fsm_invalid_transition(command->conn_state->fsm_state, command->event);
/*  END   == INVALID == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_mail(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == MAIL == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_mail");
    return maybe_next;
/*  END   == MAIL == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_mail_received(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == MAIL RECEIVED == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_mail_received");
    return maybe_next;
/*  END   == MAIL RECEIVED == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_quit(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == QUIT == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_quit");
    return maybe_next;
/*  END   == QUIT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_rcpt(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == RCPT == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_rcpt");
    return maybe_next;
/*  END   == RCPT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_rset(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == RSET == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_rset");
    return maybe_next;
/*  END   == RSET == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_timeout(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == TIMEOUT == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_timeout");
    return maybe_next;
/*  END   == TIMEOUT == DO NOT CHANGE THIS COMMENT  */
}

static te_smtp_server_fsm_state
smtp_server_fsm_do_vrfy(command_t *restrict command, te_smtp_server_fsm_state maybe_next)
{
/*  START == VRFY == DO NOT CHANGE THIS COMMENT  */
    log_debug("smtp_server_fsm_do_vrfy");
    return maybe_next;
/*  END   == VRFY == DO NOT CHANGE THIS COMMENT  */
}

/**
 *  Step the FSM.  Returns the resulting state.  If the current state is
 *  SMTP_SERVER_FSM_ST_DONE or SMTP_SERVER_FSM_ST_INVALID, it resets to
 *  SMTP_SERVER_FSM_ST_INIT and returns SMTP_SERVER_FSM_ST_INIT.
 */
err_code_t
smtp_server_fsm_step(command_t *restrict command)
{
    te_smtp_server_fsm_state nxtSt;
    smtp_server_fsm_callback_t * pT;

    if ((unsigned)command->conn_state->fsm_state >= SMTP_SERVER_FSM_ST_INVALID) {
        command->conn_state->fsm_state = SMTP_SERVER_FSM_ST_INIT;
        return OP_SUCCESS;
    }

#ifndef __COVERITY__
    if (command->event >= SMTP_SERVER_FSM_EV_INVALID) {
        nxtSt = SMTP_SERVER_FSM_ST_INVALID;
        pT    = smtp_server_fsm_do_invalid;
    } else
#endif /* __COVERITY__ */
    {
        const t_smtp_server_fsm_transition * ttbl =
            smtp_server_fsm_trans_table[ command->conn_state->fsm_state ] + command->event;
        nxtSt = ttbl->next_state;
        pT    = ttbl->trans_proc;
    }

    if (pT != NULL) {
        nxtSt = (*pT)(command, nxtSt);
    }

    if (nxtSt == ERR_SMTP_BAD_SEQ) {
        return ERR_SMTP_BAD_SEQ;
    }

    command->conn_state->fsm_state = nxtSt;

    /* START == FINISH STEP == DO NOT CHANGE THIS COMMENT */
    /* END   == FINISH STEP == DO NOT CHANGE THIS COMMENT */

    return OP_SUCCESS;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of server-fsm.c */
