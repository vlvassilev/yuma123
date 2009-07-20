#ifndef _H_ses
#define _H_ses
/*  FILE: ses.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   NETCONF Session Common definitions module

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
30-dec-05    abb      Begun.
*/

/* used by applications to generate FILE output */
#include <stdio.h>

/* used by the agent for the xmlTextReader interface */
#include <xmlreader.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tstamp
#include "tstamp.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

#define SES_MY_SID(S)   ((S)->sid)

#define SES_REQ_KILL(S) agt_ses_request_close((S)->sid)

#define SES_KILLREQ_SET(S) ((S)->state >= SES_ST_SHUTDOWN_REQ)

#define SES_ACK_KILLREQ(S) ((S)->state = SES_ST_SHUTDOWN)

#define SES_OUT_BYTES(S) (S)->stats.out_bytes

#define SES_LINELEN(S) (S)->stats.out_line

#define SES_LINESIZE(S) (S)->linesize

#define SES_NULL_SID  0

/* controls the size of each buffer chuck */
#define SES_MSG_BUFFSIZE  2000

/* max number of buffer chunks a session can have allocated at once  */
#define SES_MAX_BUFFERS  4096

/* max number of buffers a session is allowed to cache in its freeQ */
#define SES_MAX_FREE_BUFFERS  32

/* max number of buffers to try to send in one call to the write fn */
#define SES_MAX_BUFFSEND   32

/* max number of bytes to try to send in one call to the write_fn */
#define SES_MAX_BYTESEND   0xffff

#define SES_DEF_LINESIZE   72

/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/

/* Session ID */
typedef uint32 ses_id_t;

/* Session Types */
typedef enum ses_type_t_ {
    SES_TYP_NONE,
    SES_TYP_NETCONF,
    SES_TYP_NCX,
    SES_TYP_DUMMY
} ses_type_t;


/* Transport Types */
typedef enum ses_transport_t_ {
    SES_TRANSPORT_NONE,
    SES_TRANSPORT_SSH,
    SES_TRANSPORT_BEEP,
    SES_TRANSPORT_SOAP
} ses_transport_t;


/* Session States */
typedef enum ses_state_t_ {
    SES_ST_NONE,
    SES_ST_INIT,
    SES_ST_HELLO_WAIT,
    SES_ST_IDLE,
    SES_ST_IN_MSG,
    SES_ST_SHUTDOWN_REQ,
    SES_ST_SHUTDOWN
} ses_state_t;


/* Session Input Handler States */
typedef enum ses_instate_t_ {
    SES_INST_NONE,
    SES_INST_IDLE,
    SES_INST_INMSG,
    SES_INST_INEND
} ses_instate_t;


/* Session Output Mode */
typedef enum ses_mode_t_ {
    SES_MODE_NONE,
    SES_MODE_XML,
    SES_MODE_XMLDOC,
    SES_MODE_HTML,
    SES_MODE_TEXT
} ses_mode_t;


/* Session Termination reason */
typedef enum ses_term_reason_t_ {
    SES_TR_NONE,
    SES_TR_CLOSED,
    SES_TR_KILLED,
    SES_TR_DROPPED,
    SES_TR_TIMEOUT,
    SES_TR_OTHER,
    SES_TR_NOSTART,
    SES_TR_BAD_HELLO
} ses_term_reason_t;


/*** using uint32 instead of uint64 because the netconf-state
 *** data model is specified that way
 ***/

/* Per Session Statistics */
typedef struct ses_stats_t_ {
    /* extra original internal byte counters */
    uint32            in_bytes;
    uint32            in_drop_msgs;
    uint32            out_bytes;
    uint32            out_drop_bytes;

    /* hack: bytes since '\n', pretty-print */
    uint32            out_line;    

    /* netconf-state counters */
    uint32            inXMLParseErrors;
    uint32            inBadHellos;
    uint32            inRpcs;
    uint32            inBadRpcs;
    uint32            inNotSupportedRpcs;
    uint32            outRpcReplies;
    uint32            outRpcErrors;
    uint32            outNotifications;
} ses_stats_t;


/* Session Total Statistics */
typedef struct ses_total_stats_t_ {
    uint32            active_sessions;
    uint32            closed_sessions;
    uint32            failed_sessions;
    uint32            inSessions;
    ses_stats_t       stats;
    xmlChar           startTime[TSTAMP_MIN_SIZE];
} ses_total_stats_t;


/* Session Message Buffer */
typedef struct ses_msg_buff_t_ {
    dlq_hdr_t        qhdr;
    size_t           bufflen;      /* buff actual size */
    size_t           buffpos;      /* buff cur position */
    xmlChar          buff[SES_MSG_BUFFSIZE];   
} ses_msg_buff_t;


/* embedded Q header for the message ready Q */
typedef struct ses_ready_t_ {
    dlq_hdr_t hdr;
    ses_id_t  sid;
    boolean   inq;
} ses_ready_t;


/* Session Message */
typedef struct ses_msg_t_ {
    dlq_hdr_t        qhdr;        /* Q header for buffcb->msgQ */
    boolean          ready;               /* ready for parsing */
    ses_msg_buff_t  *curbuff;         /* cur position in buffQ */
    dlq_hdr_t        buffQ;             /* Q of ses_msg_buff_t */
} ses_msg_t;

/* optional read function for the session */
typedef ssize_t (*ses_read_fn_t) (void *s,
				  char *buff,
				  size_t bufflen,
                                  boolean *erragain);

/* optional write function for the session */
typedef status_t (*ses_write_fn_t) (void *s);

/* Session Control Block */
typedef struct ses_cb_t_ {
    dlq_hdr_t        qhdr;           /* queued by manager only */
    ses_type_t       type;                      /* session type */
    ses_transport_t  transport;               /* transport type */
    ses_state_t      state;                    /* session state */
    ses_mode_t       mode;                      /* session mode */
    ses_id_t         sid;                         /* session ID */
    ses_id_t         killedbysid;       /* killed-by session ID */
    ses_term_reason_t termreason;
    xmlChar         *start_time;         /* dateTime start time */
    xmlChar         *username;                       /* user ID */
    xmlChar         *peeraddr;           /* Inet address string */
    boolean          active;            /* <hello> completed ok */
    boolean          xmladvance;    /* reader hack for leaflist */
    xmlTextReaderPtr reader;             /* input stream reader */
    FILE            *fp;             /* set if output to a file */
    int              fd;           /* set if output to a socket */
    ses_read_fn_t    rdfn;          /* set if external write fn */
    ses_write_fn_t   wrfn;           /* set if external read fn */
    uint32           inendpos;          /* inside EOM directive */
    ses_instate_t    instate;               /* input state enum */
    uint32           buffcnt;           /* current buffer count */
    uint32           freecnt;            /* current freeQ count */
    dlq_hdr_t        msgQ;              /* Q of ses_msg_t input */
    dlq_hdr_t        freeQ;              /* Q of ses_msg_buff_t */
    dlq_hdr_t        outQ;               /* Q of ses_msg_buff_t */
    ses_msg_buff_t  *outbuff;          /* current output buffer */
    ses_ready_t      inready;            /* header for inreadyQ */
    ses_ready_t      outready;          /* header for outreadyQ */
    ses_stats_t      stats;           /* per-session statistics */
    void            *mgrcb;    /* if manager session, mgr_scb_t */
    /*** user preferences ***/
    int32            indent;          /* indent N spaces (0..9) */
    uint32           linesize;              /* TERM line length */
    ncx_withdefaults_t  withdef;       /* with-defaults default */
    boolean          withmeta;         /* with-metadata default */
} ses_cb_t;


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern ses_cb_t *
    ses_new_scb (void);

extern ses_cb_t *
    ses_new_dummy_scb (void);

extern void
    ses_free_scb (ses_cb_t *scb);

extern void
    ses_putchar (ses_cb_t *scb,
		 uint32    ch);

extern void
    ses_putstr (ses_cb_t *scb,
		const xmlChar *str);

extern void
    ses_putstr_indent (ses_cb_t *scb,
		       const xmlChar *str,
		       int32 indent);

extern void
    ses_putcstr (ses_cb_t *scb,
		 const xmlChar *str,
		 int32 indent);

extern void
    ses_indent (ses_cb_t *scb,
		int32 indent);

extern int32
    ses_indent_count (const ses_cb_t *scb);

extern void
    ses_set_indent (ses_cb_t *scb,
		    int32 indent);

extern void
    ses_set_mode (ses_cb_t *scb,
		  ses_mode_t mode);

extern ses_mode_t
    ses_get_mode (ses_cb_t *scb);

extern status_t
    ses_start_msg (ses_cb_t *scb);

extern void
    ses_finish_msg (ses_cb_t *scb);

extern int
    ses_read_cb (void *context,
		 char *buffer,
		 int len);

extern status_t
    ses_accept_input (ses_cb_t *scb);

extern const xmlChar *
    ses_state_name (ses_state_t state);

extern ncx_withdefaults_t
    ses_withdef (const ses_cb_t *scb);

extern boolean
    ses_withmeta (const ses_cb_t *scb);

extern uint32
    ses_line_left (const ses_cb_t *scb);


extern void
    ses_put_extern (ses_cb_t *scb,
		    const xmlChar *fname);

extern ses_total_stats_t *
    ses_get_total_stats (void);

extern const xmlChar *
    ses_get_transport_name (ses_transport_t transport);

#endif            /* _H_ses */
