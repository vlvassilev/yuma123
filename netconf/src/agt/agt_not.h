#ifndef _H_agt_not
#define _H_agt_not
/*  FILE: agt_not.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   NETCONF Notifications DM module support

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
30-may-09    abb      Begun.
*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_ses
#include "ses.h"
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

#define AGT_NOT_MODULE1     (const xmlChar *)"notifications"
#define AGT_NOT_MODULE2     (const xmlChar *)"nc-notifications"

/* agt_not_subscription_t flags */


/* if set, stopTime is in the future */
#define AGT_NOT_FL_FUTURESTOP  bit0

/* if set, replayComplete is ready to be sent */
#define AGT_NOT_FL_RC_READY    bit1

/* if set replayComplete has been sent */
#define AGT_NOT_FL_RC_DONE     bit2

/* if set, notificationComplete is ready to be sent */
#define AGT_NOT_FL_NC_READY    bit3

/* if set, notificationComplete has been sent */
#define AGT_NOT_FL_NC_DONE     bit4


/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/

/* per-subscription state */
typedef enum agt_not_state_t_ {
    /* none == cleared memory; not set */
    AGT_NOT_STATE_NONE,

    /* init == during initialization */
    AGT_NOT_STATE_INIT,

    /* replay == during sending of selected replay buffer */
    AGT_NOT_STATE_REPLAY,

    /* timed == stopTime in future, after replayComplete
     * but before notificationComplete
     */
    AGT_NOT_STATE_TIMED,

    /* live == live delivery mode (no stopTime given) */
    AGT_NOT_STATE_LIVE,

    /* shutdown == forced shutdown mode; 
     * notificationComplete is being sent
     */
    AGT_NOT_STATE_SHUTDOWN
} agt_not_state_t;


/* agent supported stream types */
typedef enum agt_not_stream_t_ {
    AGT_NOT_STREAM_NONE,
    AGT_NOT_STREAM_NETCONF
} agt_not_stream_t;


/* one notification message that will be sent to all
 * subscriptions and kept in the replay buffer (notificationQ)
 */
typedef struct agt_not_msg_t_ {
    dlq_hdr_t                qhdr;
    const obj_template_t    *notobj;
    dlq_hdr_t                payloadQ;
    uint32                   msgid;
    xmlChar                  eventTime[TSTAMP_MIN_SIZE];
    val_value_t             *msg;     /* /notification element */
    val_value_t             *event;  /* ptr inside msg for filter */
} agt_not_msg_t;


/* one subscription that will be kept in the subscriptionQ */
typedef struct agt_not_subscription_t_ {
    dlq_hdr_t             qhdr;
    ses_cb_t             *scb;              /* back-ptr to session */
    ses_id_t              sid;              /* used when scb deleted */
    xmlChar              *stream;
    agt_not_stream_t      streamid;
    op_filtertyp_t        filtertyp;
    val_value_t          *filterval;
    val_value_t          *selectval;
    xmlChar               createTime[TSTAMP_MIN_SIZE];
    xmlChar              *startTime;       /* converted to UTC */
    xmlChar              *stopTime;        /* converted to UTC */
    uint32                flags;
    agt_not_msg_t        *firstreplaymsg;   /* back-ptr; could be zapped */
    agt_not_msg_t        *lastreplaymsg;    /* back-ptr; could be zapped */
    agt_not_msg_t        *lastmsg;          /* back-ptr; could be zapped */
    uint32                firstreplaymsgid; /* w/firstreplaymsg is deleted */
    uint32                lastreplaymsgid;  /* w/lastreplaymsg is deleted */
    uint32                lastmsgid;        /* w/ lastmsg is deleted */
    agt_not_state_t       state;
} agt_not_subscription_t;


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern status_t
    agt_not_init (void);

extern status_t
    agt_not_init2 (void);

extern void 
    agt_not_cleanup (void);

extern uint32
    agt_not_send_notifications (void);

extern void
    agt_not_clean_eventlog (void);

extern void
    agt_not_remove_subscription (ses_id_t sid);

extern agt_not_msg_t * 
    agt_not_new_notification (const obj_template_t *eventType);

extern void 
    agt_not_free_notification (agt_not_msg_t *not);

/* consumes 'val'; will be freed later */
extern void
    agt_not_add_to_payload (agt_not_msg_t *notif,
			    val_value_t *val);


/* consumes 'notif'; will be freed later */
extern void
    agt_not_queue_notification (agt_not_msg_t *notif);


#endif            /* _H_agt_not */
