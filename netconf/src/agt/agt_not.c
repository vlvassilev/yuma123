/*  FILE: agt_state.c

   NETCONF State Data Model implementation: Agent Side Support

identifiers:
container /netconf
container /netconf/streams
list /netconf/streams/stream
leaf /netconf/streams/stream/name
leaf /netconf/streams/stream/description
leaf /netconf/streams/stream/replaySupport
leaf /netconf/streams/stream/replayLogCreationTime
notification /replayComplete
notification /notificationComplete


*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
24feb09      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>
#include  <unistd.h>
#include  <errno.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include  "agt.h"
#endif

#ifndef _H_agt_cap
#include  "agt_cap.h"
#endif

#ifndef _H_agt_cb
#include  "agt_cb.h"
#endif

#ifndef _H_agt_not
#include  "agt_not.h"
#endif

#ifndef _H_agt_rpc
#include  "agt_rpc.h"
#endif

#ifndef _H_agt_ses
#include  "agt_ses.h"
#endif

#ifndef _H_agt_state
#include  "agt_state.h"
#endif

#ifndef _H_agt_util
#include  "agt_util.h"
#endif

#ifndef _H_cfg
#include  "cfg.h"
#endif

#ifndef _H_getcb
#include  "getcb.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncxmod
#include  "ncxmod.h"
#endif

#ifndef _H_ncxtypes
#include  "ncxtypes.h"
#endif

#ifndef _H_rpc
#include  "rpc.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_ses_msg
#include  "ses_msg.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_val_util
#include  "val_util.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_xml_wr
#include  "xml_wr.h"
#endif

#ifndef _H_yangconst
#include  "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define AGT_NOT_DEBUG 1
#endif

#define notifications_N_create_subscription \
    (const xmlChar *)"create-subscription"
#define notifications_N_eventTime (const xmlChar *)"eventTime"
#define notifications_N_create_subscription_stream \
    (const xmlChar *)"stream"
#define notifications_N_create_subscription_filter \
    (const xmlChar *)"filter"
#define notifications_N_create_subscription_startTime \
    (const xmlChar *)"startTime"
#define notifications_N_create_subscription_stopTime \
    (const xmlChar *)"stopTime"
#define nc_notifications_N_netconf (const xmlChar *)"netconf"
#define nc_notifications_N_streams (const xmlChar *)"streams"
#define nc_notifications_N_stream (const xmlChar *)"stream"
#define nc_notifications_N_name (const xmlChar *)"name"
#define nc_notifications_N_description \
    (const xmlChar *)"description"
#define nc_notifications_N_replaySupport \
    (const xmlChar *)"replaySupport"
#define nc_notifications_N_replayLogCreationTime \
    (const xmlChar *)"replayLogCreationTime"
#define nc_notifications_N_replayComplete \
    (const xmlChar *)"replayComplete"
#define nc_notifications_N_notificationComplete \
    (const xmlChar *)"notificationComplete"


/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static boolean              agt_not_init_done = FALSE;

/* notifications.yang */
static ncx_module_t         *notifmod;

/* nc-notifications.yang */
static ncx_module_t         *ncnotifmod;

/* Q of agt_not_subscription_t 
 * one entry is created for each create-subscription call
 * if stopTime is given then the subscription will be
 * automatically deleted when all the replay is complete.
 * Otherwise, a subscription is deleted when the session
 * is terminated
 */
static dlq_hdr_t             subscriptionQ;

/* Q of agt_not_msg_t 
 * these are the messages that represent the replay buffer
 * only system-wide notifications are stored in this Q
 * the replayComplete and notificationComplete events are
 * generated special-case, and not stored for replay
 */
static dlq_hdr_t             notificationQ;

/* cached pointer to the <notification> element template */
static const obj_template_t *notificationobj;

/* cached pointer to the /notification/eventTime element template */
static const obj_template_t *eventTimeobj;

/* cached pointer to the /ncn:replayComplete element template */
static const obj_template_t *replayCompleteobj;

/* cached pointer to the /ncn:notificationComplete element template */
static const obj_template_t *notificationCompleteobj;

/* flag to signal quick exit */
static boolean               anySubscriptions;

/* auto-increment message index */
static uint64                msgid;


/********************************************************************
* FUNCTION free_subscription
*
* Clean and free a subscription control block
*
* INPUTS:
*    sub == subscription to delete
*
*********************************************************************/
static void
    free_subscription (agt_not_subscription_t *sub)
{
    if (sub->stream) {
	m__free(sub->stream);
    }
    if (sub->startTime) {
	m__free(sub->startTime);
    }
    if (sub->stopTime) {
	m__free(sub->stopTime);
    }
    if (sub->filter) {
	val_free_value(sub->filter);
    }
    m__free(sub);

}  /* free_subscription */


/********************************************************************
* FUNCTION new_subscription
*
* Malloc and fill in a new subscription control block
*
* INPUTS:
*     scb == session this subscription is for
*     stream == requested stream ID
*     curTime == current time for creation time
*     startTime == replay start time (may be NULL)
*     stopTime == replayStopTime (may be NULL)
*     futurestop == TRUE if stopTime in the future
*                   FALSE if not set or not in the future
*     filter == filter value node passed from PDU
*           !!! THIS MALLOCED NODE IS FREED LATER !!!!
*
* RETURNS:
*    pointer to malloced struct or NULL if no memoryerror
*********************************************************************/
static agt_not_subscription_t *
    new_subscription (ses_cb_t *scb,
		      const xmlChar *stream,
		      const xmlChar *curTime,
		      const xmlChar *startTime,
		      const xmlChar *stopTime,
		      boolean futurestop,
		      val_value_t *filter)
{
    agt_not_subscription_t  *sub;
    agt_not_stream_t         streamid;

    if (!xml_strcmp(stream, NCX_DEF_STREAM_NAME)) {
	streamid = AGT_NOT_STREAM_NETCONF;
    } else {
	/*** !!! ***/
	SET_ERROR(ERR_INTERNAL_VAL);
	streamid = AGT_NOT_STREAM_NETCONF;
    }

    sub = m__getObj(agt_not_subscription_t);
    if (!sub) {
	return NULL;
    }
    memset(sub, 0x0, sizeof(agt_not_subscription_t));

    sub->stream = xml_strdup(stream);
    if (!sub->stream) {
	free_subscription(sub);
	return NULL;
    }

    if (startTime) {
	sub->startTime = xml_strdup(startTime);
	if (!sub->startTime) {
	    free_subscription(sub);
	    return NULL;
	}
    }

    if (stopTime) {
	sub->stopTime = xml_strdup(stopTime);
	if (!sub->stopTime) {
	    free_subscription(sub);
	    return NULL;
	}
    }

    sub->scb = scb;
    sub->sid = scb->sid;
    sub->streamid = streamid;
    xml_strcpy(sub->createTime, curTime);
    sub->filter = filter;
    if (futurestop) {
	sub->flags = AGT_NOT_FL_FUTURESTOP;
    }
    sub->state = AGT_NOT_STATE_INIT;

    return sub;

}  /* new_subscription */


/********************************************************************
* FUNCTION create_subscription_validate
*
* create-subscription : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    create_subscription_validate (ses_cb_t *scb,
				  rpc_msg_t *msg,
				  xml_node_t *methnode)
{
    const xmlChar           *stream, *startTime, *stopTime;
    val_value_t             *valstream, *valfilter;
    val_value_t             *valstartTime, *valstopTime;
    agt_not_subscription_t  *sub;
    xmlChar                  tstampbuff[TSTAMP_MIN_SIZE];
    int                      ret;
    status_t                 res;
    agt_not_stream_t         streamid;
    boolean                  futurestop;

    res = NO_ERR;
    valstream = NULL;
    valfilter = NULL;
    valstartTime = NULL;
    valstopTime = NULL;
    startTime = NULL;
    stopTime = NULL;
    stream = NCX_DEF_STREAM_NAME;
    streamid = AGT_NOT_STREAM_NETCONF;
    tstamp_datetime(tstampbuff);
    futurestop = FALSE;

    /* get the stream parameter */
    valstream = 
	val_find_child(msg->rpc_input, 
		       AGT_NOT_MODULE1,
		       notifications_N_create_subscription_stream);
    if (valstream) {
	if (valstream->res == NO_ERR) {
	    stream = VAL_STR(valstream);
	    if (xml_strcmp(NCX_DEF_STREAM_NAME, stream)) {
		/* not the hard-wired NETCONF stream and
		 * no other strams supported at this time
		 * report the error
		 */
		res = ERR_NCX_NOT_FOUND;
		agt_record_error(scb, 
				 &msg->mhdr, 
				 NCX_LAYER_OPERATION, 
				 res, 
				 methnode, 
				 NCX_NT_STRING, 
				 stream,
				 NCX_NT_VAL, 
				 valstream);
	    }  /* else name==NETCONF: OK */
	} /* else error already reported */
    } /* else default NETCONF stream is going to be used */

    /* get the filter parameter */
    valfilter = 
	val_find_child(msg->rpc_input, 
		       AGT_NOT_MODULE1,
		       notifications_N_create_subscription_filter);
    if (valfilter) {
	if (valfilter->res == NO_ERR) {
	    /* check if the optional filter parameter is ok */
	    res = agt_validate_filter_ex(scb, msg, valfilter);
	    /* error already recorded if not NO_ERR */
	}
    }

    /* get the startTime parameter */
    valstartTime = 
	val_find_child(msg->rpc_input, 
		       AGT_NOT_MODULE1,
		       notifications_N_create_subscription_startTime);
    if (valstartTime) {
	if (valstartTime->res == NO_ERR) {
	    startTime = VAL_STR(valstartTime);
	}
    } 

    /* get the stopTime parameter */
    valstartTime = 
	val_find_child(msg->rpc_input, 
		       AGT_NOT_MODULE1,
		       notifications_N_create_subscription_stopTime);
    if (valstopTime) {
	if (valstopTime->res == NO_ERR) {
	    stopTime = VAL_STR(valstopTime);
	}
    } 

    if (startTime || stopTime) {
	/*** NEED TO NORMALIZE THE xsd:dateTime STRINGS SO
	 *** ALL THE TIMEZONES ARE THE SAME  (TBD)
	 ***/


	if (startTime) {
	    ret = xml_strcmp(startTime, tstampbuff);
	    if (ret > 0) {
		res = ERR_NCX_BAD_ELEMENT;
		agt_record_error(scb, 
				 &msg->mhdr, 
				 NCX_LAYER_OPERATION, 
				 res, 
				 methnode, 
				 NCX_NT_STRING, 
				 startTime,
				 NCX_NT_VAL, 
				 valstartTime);
	    }  /* else startTime before now (OK) */

	    if (stopTime) {
		ret = xml_strcmp(startTime, stopTime);
		if (ret > 0) {
		    res = ERR_NCX_BAD_ELEMENT;
		    agt_record_error(scb, 
				     &msg->mhdr, 
				     NCX_LAYER_OPERATION, 
				     res, 
				     methnode, 
				     NCX_NT_STRING, 
				     stopTime,
				     NCX_NT_VAL, 
				     valstopTime);
		}  /* else startTime before stopTime (OK) */
	    }
	}

	if (stopTime) {
	    if (!startTime) {
		res = ERR_NCX_MISSING_ELEMENT;
		agt_record_error(scb, 
				 &msg->mhdr, 
				 NCX_LAYER_OPERATION, 
				 res, 
				 methnode, 
				 NCX_NT_NONE, 
				 NULL,
				 NCX_NT_VAL, 
				 valstartTime);
	    }

	    /* treat stopTime in the future as an error */
	    ret = xml_strcmp(stopTime, tstampbuff);
	    if (ret > 0) {
		futurestop = TRUE;
	    }
	}
    }

    if (res == NO_ERR) {
	/* create a new subscription control block */
	sub = new_subscription(scb,
			       stream,
			       tstampbuff,
			       startTime,
			       stopTime,
			       futurestop,
			       valfilter);
	if (!sub) {
	    res = ERR_INTERNAL_MEM;
	    agt_record_error(scb, 
			     &msg->mhdr, 
			     NCX_LAYER_OPERATION, 
			     res, 
			     methnode, 
			     NCX_NT_NONE, 
			     NULL,
			     NCX_NT_NONE, 
			     NULL);
	} else {
	    if (valfilter) {
		/* passing off this memory now !! */
		val_remove_child(valfilter);
	    }
	    msg->rpc_user1 = sub;
	}
    }

    return res;

} /* create_subscription_validate */


/********************************************************************
* FUNCTION create_subscription_invoke
*
* create-subscription : invoke callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    create_subscription_invoke (ses_cb_t *scb,
				rpc_msg_t *msg,
				xml_node_t *methnode)
{
    agt_not_subscription_t *sub;
    agt_not_msg_t          *not, *nextnot;
    int                     ret;
    boolean                 done;

    (void)scb;
    (void)methnode;
    sub = (agt_not_subscription_t *)msg->rpc_user1;

    agt_state_add_subscription(sub);

    if (sub->startTime) {
	/* this subscription has requested replay
	 * go through the notificationQ and set
	 * the start replay pointer
	 */
	sub->state = AGT_NOT_STATE_REPLAY;
	done = FALSE;
	for (not = (agt_not_msg_t *)
		 dlq_firstEntry(&notificationQ);
	     not != NULL && !done;
	     not = (agt_not_msg_t *)dlq_nextEntry(not)) {

	    ret = xml_strcmp(sub->startTime, not->eventTime);
	    if (ret <= 0) {
		sub->firstreplaymsg = not;
		sub->firstreplaymsgid = not->msgid;
		done = TRUE;
	    }
	}

	if (!done) {
	    /* the startTime is after the last available
	     * notification eventTime, so replay is over
	     */
	    sub->flags |= AGT_NOT_FL_RC_READY;
	} else if (sub->stopTime) {
	    /* the sub->firstreplaymsg was set;
	     * the subscription has requested to be
	     * terminated after a specific time
	     */
	    if (sub->flags & AGT_NOT_FL_FUTURESTOP) {
		/* just use the last replay buffer entry
		 * as the end-of-replay marker
		 */
		sub->lastreplaymsg = (agt_not_msg_t *)
		    dlq_lastEntry(&notificationQ);
		sub->lastreplaymsgid = 
		    sub->lastreplaymsg->msgid;
	    } else {
		/* first check that the start notification
		 * is not already past the requested stopTime
		 */
		ret = xml_strcmp(sub->stopTime,
				 sub->firstreplaymsg->eventTime);
		if (ret <= 0) {
		    sub->firstreplaymsg = NULL;
		    sub->firstreplaymsgid = 0;
		    sub->flags |= AGT_NOT_FL_RC_READY;
		} else {
		    /* check the notifications after the
		     * start replay node, to find out
		     * which one should be the last replay 
		     */
		    done = FALSE;
		    for (not = sub->firstreplaymsg;
			 not != NULL && !done;
			 not = nextnot) {
		 
			nextnot = (agt_not_msg_t *)dlq_nextEntry(not);
			if (nextnot) {
			    ret = xml_strcmp(sub->stopTime, 
					     nextnot->eventTime);
			} else {
			    ret = -1;
			}
			if (ret < 0) {
			    /* the previous one checked was the winner */
			    sub->lastreplaymsg = not;
			    sub->lastreplaymsgid = not->msgid;
			    done = TRUE;
			}
		    }

		    if (!done) {
			sub->lastreplaymsg = (agt_not_msg_t *)
			    dlq_lastEntry(&notificationQ);
			sub->lastreplaymsgid = 
			    sub->lastreplaymsg->msgid;
		    }
		}			
	    }
	}
    } else {
	sub->state = AGT_NOT_STATE_LIVE;
    }

    dlq_enque(sub, &subscriptionQ);
    anySubscriptions = TRUE;

    if (LOGDEBUG) {
	log_debug("\nagt_not: Started %s subscription on stream "
		  "'%s' for session '%u'",
		  (sub->startTime) ? "replay" : "live",
		  sub->stream, 
		  sub->scb->sid);
    }
    return NO_ERR;

} /* create_subscription_invoke */


/********************************************************************
* FUNCTION new_notification
* 
* Malloc and initialize the fields in an agt_not_msg_t
*
* INPUTS:
*   eventType == object template of the event type
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static agt_not_msg_t * 
    new_notification (const obj_template_t *eventType)
{
    agt_not_msg_t  *not;

    not = m__getObj(agt_not_msg_t);
    if (!not) {
	return NULL;
    }
    (void)memset(not, 0x0, sizeof(agt_not_msg_t));
    dlq_createSQue(&not->payloadQ);
    not->msgid = ++msgid;
    if (msgid == 0) {
	/* msgid is wrapping!!! */
	SET_ERROR(ERR_INTERNAL_VAL);
    }
    tstamp_datetime(not->eventTime);
    not->notobj = eventType;
    return not;

}  /* new_notification */


/********************************************************************
* FUNCTION free_notification
* 
* Scrub the memory in an agt_not_template_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    not == agt_not_template_t to delete
*********************************************************************/
static void 
    free_notification (agt_not_msg_t *not)
{
    val_value_t *val;

#ifdef DEBUG
    if (!not) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(&not->payloadQ)) {
	val = (val_value_t *)dlq_deque(&not->payloadQ);
	val_free_value(val);
    }

    if (not->msg) {
	val_free_value(not->msg);
    }

    m__free(not);

}  /* free_notification */


/********************************************************************
* FUNCTION expire_subscription
*
* Expire a removed subscription because it has a stopTime
* and no more replay notifications are left to deliver
*
* MUST REMOVE FROM subscriptionQ FIRST !!!
*
* INPUTS:
*    sub == subscription to expire
*
*********************************************************************/
static void
    expire_subscription (agt_not_subscription_t *sub)
{
    agt_state_remove_subscription(sub->scb->sid);
    free_subscription(sub);
    anySubscriptions = (dlq_empty(&subscriptionQ)) ? FALSE : TRUE;

    if (LOGDEBUG) {
	log_debug("\nagt_not: Removed %s subscription "
		  "for session '%u'",
		  (sub->startTime) ? "replay" : "live",
		  sub->scb->sid);
    }

}  /* expire_subscription */


/********************************************************************
* FUNCTION send_notification
*
* Send the specified notification to the specified subscription
* If checkfilter==TRUE, then check the filter if any, 
* and only send if it passes
*
* INPUTS:
*   sub == subscription to use
*   notif == notification to use
*   checkfilter == TRUE to check the filter if any
*                  FALSE to bypass the filtering 
*                  (e.g., replayComplete, notificationComplete)
*
* OUTPUTS:
*   message sent to sub->scb if filter passes
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    send_notification (agt_not_subscription_t *sub,
		       agt_not_msg_t *notif,
		       boolean checkfilter)
{
    val_value_t        *topval, *eventTime;
    val_value_t        *eventType, *payloadval;
    ses_total_stats_t  *totalstats;
    xml_msg_hdr_t       msghdr;
    status_t            res;

    totalstats = ses_get_total_stats();

    if (!notif->msg) {
	/* need to construct the notification msg */
	topval = val_new_value();
	if (!topval) {
	    log_error("\nError: malloc failed: cannot send notification");
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(topval, notificationobj);

	eventTime = val_make_simval(obj_get_ctypdef(eventTimeobj),
				    obj_get_nsid(eventTimeobj),
				    obj_get_name(eventTimeobj),
				    notif->eventTime,
				    &res);
	if (!eventTime) {
	    log_error("\nError: make simval failed (%s): cannot "
		      "send notification", 
		      get_error_string(res));
	    val_free_value(topval);
	    sub->scb->stats.out_drop_bytes++;
	    totalstats->stats.out_drop_bytes++;
	    return res;
	}
	val_add_child(eventTime, topval);

	eventType = val_new_value();
	if (!eventType) {
	    log_error("\nError: malloc failed: cannot send notification");
	    val_free_value(topval);
	    sub->scb->stats.out_drop_bytes++;
	    totalstats->stats.out_drop_bytes++;
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(eventType, notif->notobj);
	val_add_child(eventType, topval);
	notif->event = eventType;

	while (!dlq_empty(&notif->payloadQ)) {
	    payloadval = (val_value_t *)
		dlq_deque(&notif->payloadQ);
	    val_add_child(payloadval, eventType);
	}

	notif->msg = topval;
    }

    /* check if any filtering is needed */
    if (checkfilter && sub->filter) {


    }

    /* create an RPC message header struct */
    xml_msg_init_hdr(&msghdr);

    res = ses_start_msg(sub->scb);
    if (res != NO_ERR) {
	log_error("\nError: cannot start notification");
	sub->scb->stats.out_drop_bytes++;
	totalstats->stats.out_drop_bytes++;
	xml_msg_clean_hdr(&msghdr);
	return res;
    }

    xml_wr_full_val(sub->scb, &msghdr, notif->msg, 0);
		    
    ses_finish_msg(sub->scb);

    sub->scb->stats.outNotifications++;
    totalstats->stats.outNotifications++;

    if (LOGDEBUG) {
	log_debug("\nagt_not: Sent <%s> (%u) on '%s' stream "
		  "for session '%u'",
		  obj_get_name(notif->notobj),
		  notif->msgid,
		  sub->stream,
		  sub->scb->sid);
    }
    if (LOGDEBUG2) {
	log_debug2("\nNotification contents:");
	if (notif->msg) {
	    val_dump_value(notif->msg, NCX_DEF_INDENT);
	}
	log_debug2("\n");
    }

    xml_msg_clean_hdr(&msghdr);
    return NO_ERR;

}  /* send_notification */


/********************************************************************
* FUNCTION get_entry_after
*
* Get the entry after the specified msgid
*
* INPUTS:
*    thismsgid == get the first msg with an ID higher than this value
*
* RETURNS:
*    pointer to an notification to use
*    NULL if none found
*********************************************************************/
static agt_not_msg_t *
    get_entry_after (uint32 thismsgid)
{
    agt_not_msg_t *not;

    for (not = (agt_not_msg_t *)dlq_firstEntry(&notificationQ);
	 not != NULL;
	 not = (agt_not_msg_t *)dlq_nextEntry(not)) {

	if (not->msgid > thismsgid) {
	    return not;
	}
    }

    return NULL;

} /* get_entry_after */


/********************************************************************
* FUNCTION send_replayComplete
*
* Send the <replayComplate> notification
*
* INPUTS:
*    sub == subscription to use
*********************************************************************/
static void
    send_replayComplete (agt_not_subscription_t *sub)
{
    agt_not_msg_t  *not;
    status_t        res;

    not = new_notification(replayCompleteobj);
    if (!not) {
	log_error("\nError: malloc failed; cannot "
		  "send <replayComplete>");
	return;
    }

    res = send_notification(sub, not, FALSE);
    if (res != NO_ERR && NEED_EXIT(res)) {
	sub->state = AGT_NOT_STATE_SHUTDOWN;
    }

    free_notification(not);

} /* send_replayComplete */


/********************************************************************
* FUNCTION send_notificationComplete
*
* Send the <notificationComplate> notification
*
* INPUTS:
*    sub == subscription to use
*********************************************************************/
static void
    send_notificationComplete (agt_not_subscription_t *sub)
{
    agt_not_msg_t  *not;
    status_t        res;

    not = new_notification(notificationCompleteobj);
    if (!not) {
	log_error("\nError: malloc failed; cannot "
		  "send <notificationComplete>");
	return;
    }

    res = send_notification(sub, not, FALSE);
    if (res != NO_ERR && NEED_EXIT(res)) {
	sub->state = AGT_NOT_STATE_SHUTDOWN;
    }

    free_notification(not);

} /* send_notificationComplete */


/********************************************************************
* FUNCTION init_static_vars
*
* Init the static vars
*
*********************************************************************/
static void
    init_static_vars (void)
{
    notifmod = NULL;
    ncnotifmod = NULL;
    notificationobj = NULL;
    eventTimeobj = NULL;
    replayCompleteobj = NULL;
    notificationCompleteobj = NULL;
    anySubscriptions = FALSE;
    msgid = 0;

} /* init_static_vars */


/************* E X T E R N A L    F U N C T I O N S ***************/


/********************************************************************
* FUNCTION agt_not_init
*
* INIT 1:
*   Initialize the agent notification module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_not_init (void)
{
    status_t   res;

    if (agt_not_init_done) {
	return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

#ifdef AGT_NOT_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nagt_not: Loading notifications module");
    }
#endif

    dlq_createSQue(&subscriptionQ);
    dlq_createSQue(&notificationQ);
    init_static_vars();
    agt_not_init_done = TRUE;

    /* load the notifications module */
    res = ncxmod_load_module(AGT_NOT_MODULE1, NULL, &notifmod);
    if (res != NO_ERR) {
	return res;
    }

    /* load the nc-notifications module */
    res = ncxmod_load_module(AGT_NOT_MODULE2, NULL, &ncnotifmod);
    if (res != NO_ERR) {
	return res;
    }

    /* find the object definition for the notification element */
    notificationobj = ncx_find_object(notifmod,
				      NCX_EL_NOTIFICATION);

    if (!notificationobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    eventTimeobj = obj_find_child(notificationobj,
				  AGT_NOT_MODULE1,
				  notifications_N_eventTime);
    if (!eventTimeobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    replayCompleteobj = 
	ncx_find_object(ncnotifmod,
			nc_notifications_N_replayComplete);
    if (!replayCompleteobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    notificationCompleteobj = 
	ncx_find_object(ncnotifmod,
			nc_notifications_N_notificationComplete);
    if (!notificationCompleteobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    return NO_ERR;

}  /* agt_not_init */


/********************************************************************
* FUNCTION agt_not_init2
*
* INIT 2:
*   Initialize the monitoring data structures
*   This must be done after the <running> config is loaded
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_not_init2 (void)
{
    const obj_template_t  *topobj, *streamsobj, *streamobj;
    const obj_template_t  *nameobj, *descriptionobj;
    const obj_template_t  *replaySupportobj, *replayLogCreationTimeobj;
    val_value_t           *topval, *streamsval, *streamval, *childval;
    cfg_template_t        *runningcfg;
    status_t               res;
    xmlChar                tstampbuff[TSTAMP_MIN_SIZE];

    if (!agt_not_init_done) {
	return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    /* set up create-subscription RPC operation */
    res = agt_rpc_register_method(AGT_NOT_MODULE1,
				  notifications_N_create_subscription,
				  AGT_RPC_PH_VALIDATE,
				  create_subscription_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(AGT_NOT_MODULE1,
				  notifications_N_create_subscription,
				  AGT_RPC_PH_INVOKE,
				  create_subscription_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* get the running config to add some static data into */
    runningcfg = cfg_get_config(NCX_EL_RUNNING);
    if (!runningcfg || !runningcfg->root) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* get all the object nodes first */
    topobj = obj_find_template_top(ncnotifmod, 
				   AGT_NOT_MODULE2,
				   nc_notifications_N_netconf);
    if (!topobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    streamsobj = obj_find_child(topobj, 
				AGT_NOT_MODULE2, 
				nc_notifications_N_streams);
    if (!streamsobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    streamobj = obj_find_child(streamsobj,
			       AGT_NOT_MODULE2,
			       nc_notifications_N_stream);
    if (!streamobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    nameobj = obj_find_child(streamobj,
			     AGT_NOT_MODULE2,
			     nc_notifications_N_name);
    if (!nameobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    descriptionobj = obj_find_child(streamobj,
				    AGT_NOT_MODULE2,
				    nc_notifications_N_description);
    if (!descriptionobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    replaySupportobj = obj_find_child(streamobj,
				      AGT_NOT_MODULE2,
				      nc_notifications_N_replaySupport);
    if (!replaySupportobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    replayLogCreationTimeobj 
	= obj_find_child(streamobj,
			 AGT_NOT_MODULE2,
			 nc_notifications_N_replayLogCreationTime);
    if (!replayLogCreationTimeobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* add /netconf */
    topval = val_new_value();
    if (!topval) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(topval, topobj);

    /* handing off the malloced memory here */
    val_add_child(topval, runningcfg->root);

    /* add /netconf/streams */
    streamsval = val_new_value();
    if (!streamsval) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(streamsval, streamsobj);
    val_add_child(streamsval, topval);

    /* add /netconf/streams/stream
     * creating only one stram for the default NETCONF entry
     */
    streamval = val_new_value();
    if (!streamval) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(streamval, streamobj);
    val_add_child(streamval, streamsval);

    /* add /netconf/streams/stream/name */
    childval = val_make_simval(obj_get_ctypdef(nameobj),
			       obj_get_nsid(nameobj),
			       obj_get_name(nameobj),
			       NCX_DEF_STREAM_NAME,
			       &res);
    if (!childval) {
	return res;
    }
    val_add_child(childval, streamval);

    /* add /netconf/streams/stream/description */
    childval = val_make_simval(obj_get_ctypdef(descriptionobj),
			       obj_get_nsid(descriptionobj),
			       obj_get_name(descriptionobj),
			       NCX_DEF_STREAM_DESCR,
			       &res);
    if (!childval) {
	return res;
    }
    val_add_child(childval, streamval);

    /* add /netconf/streams/stream/replaySupport */
    childval = val_make_simval(obj_get_ctypdef(replaySupportobj),
			       obj_get_nsid(replaySupportobj),
			       obj_get_name(replaySupportobj),
			       NCX_EL_TRUE,
			       &res);
    if (!childval) {
	return res;
    }
    val_add_child(childval, streamval);

    /* set replay start time to now */
    tstamp_datetime(tstampbuff);

    /* add /netconf/streams/stream/replayLogCreationTime */
    childval = val_make_simval(obj_get_ctypdef(replayLogCreationTimeobj),
			       obj_get_nsid(replayLogCreationTimeobj),
			       obj_get_name(replayLogCreationTimeobj),
			       tstampbuff,
			       &res);
    if (!childval) {
	return res;
    }
    val_add_child(childval, streamval);

    /***** add coldstart or restart notification to notificationQ *****/

    return NO_ERR;

}  /* agt_not_init2 */


/********************************************************************
* FUNCTION agt_not_cleanup
*
* Cleanup the module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    agt_not_cleanup (void)
{
    agt_not_subscription_t *sub;
    agt_not_msg_t          *msg;

    if (agt_not_init_done) {
	init_static_vars();

	agt_rpc_unregister_method(AGT_NOT_MODULE1, 
				  notifications_N_create_subscription);


	/* clear the subscriptionQ */
	while (!dlq_empty(&subscriptionQ)) {
	    sub = (agt_not_subscription_t *)dlq_deque(&subscriptionQ);
	    free_subscription(sub);
	}

	/* clear the notificationQ */
	while (!dlq_empty(&notificationQ)) {
	    msg = (agt_not_msg_t *)dlq_deque(&notificationQ);
	    free_notification(msg);
	}

	agt_not_init_done = FALSE;
    }

}  /* agt_not_cleanup */


/********************************************************************
* FUNCTION agt_not_send_notifications
*
* Send out some notifications to the configured subscriptions
* if needed.
*
* Simple design:
*   go through all the subscriptions and send at most one 
*   notification to each one if needed.  This will build
*   in some throttling based on the ncxserver select loop
*   timeout (or however this function is called).
*
*  OUTPUTS:
*     notifications may be written to some active sessions
*
*********************************************************************/
void
    agt_not_send_notifications (void)
{
    agt_not_subscription_t  *sub, *nextsub;
    agt_not_msg_t           *not;
    xmlChar                  nowbuff[TSTAMP_MIN_SIZE];
    status_t                 res;
    int                      ret;

    if (!anySubscriptions) {
	return;
    }

    tstamp_datetime(nowbuff);

    for (sub = (agt_not_subscription_t *)
	     dlq_firstEntry(&subscriptionQ);
	 sub != NULL;
	 sub = nextsub) {

	nextsub = (agt_not_subscription_t *)dlq_nextEntry(sub);

	switch (sub->state) {
	case AGT_NOT_STATE_NONE:
	case AGT_NOT_STATE_INIT:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    break;
	case AGT_NOT_STATE_REPLAY:
	    /* check if replayComplete is ready */
	    if (sub->flags & AGT_NOT_FL_RC_READY) {
		/* yes, check if it has already been done */
		if (sub->flags & AGT_NOT_FL_RC_DONE) {
		    /* yes, check if <notificationComplete> is
		     * needed or if the state should be
		     * changed to AGT_NOT_STATE_LIVE
		     */
		    if (sub->stopTime) {
			/* need to terminate the subscription 
			 * at some point
			 */
			if (sub->flags & AGT_NOT_FL_FUTURESTOP) {
			    /* switch to timed mode */
			    sub->state = AGT_NOT_STATE_TIMED;
			} else {
			    /* send <notificationComplete> */
			    sub->flags |= AGT_NOT_FL_NC_READY;
			    send_notificationComplete(sub);
			    sub->flags |= AGT_NOT_FL_NC_DONE;

			    /* cleanup subscription next time
			     * through this function 
			     */
			    sub->state = AGT_NOT_STATE_SHUTDOWN;
			}
		    } else {
			/* no stopTime, so go to live mode */
			sub->state = AGT_NOT_STATE_LIVE;
		    }
		} else {
		    /* send <replayComplete> */
		    send_replayComplete(sub);
		    sub->flags |= AGT_NOT_FL_RC_DONE;
		    /* figure out the rest next time through fn */
		}
	    } else {
		/* still sending replay notifications
		 * figure out which one to send next
		 */
		if (sub->lastmsg) {
		    not = (agt_not_msg_t *)
			dlq_nextEntry(sub->lastmsg);
		} else if (sub->lastmsgid) {
		    /* use ID, back-ptr was cleared */
		    not = get_entry_after(sub->lastmsgid);
		} else {
		    /* this is the first replay being sent */
		    if (sub->firstreplaymsg) {
			not = sub->firstreplaymsg;
		    } else {
			/* use ID, back-ptr was cleared */
			not = get_entry_after(sub->firstreplaymsgid);
		    }
		}
		if (not) {
		    /* found a replay entry to send */
		    res = send_notification(sub, not, TRUE);
		    if (res != NO_ERR && NEED_EXIT(res)) {
			/* treat as a fatal error */
			sub->state = AGT_NOT_STATE_SHUTDOWN;
		    } else {
			/* msg sent OK; set up next loop through fn */
			sub->lastmsg = not;
			sub->lastmsgid = not->msgid;
			if (sub->lastreplaymsg == not) {
			    /* this was the last replay to send */
			    sub->flags |= AGT_NOT_FL_RC_READY;
			} else if (sub->lastreplaymsgid &&
				   sub->lastreplaymsgid <= not->msgid) {
			    /* this was the last replay to send */
			    sub->flags |= AGT_NOT_FL_RC_READY;
			}
		    }
		} else {
		    /* nothing left in the replay buffer */
		    sub->flags |= AGT_NOT_FL_RC_READY;
		    send_replayComplete(sub);
		    sub->flags |= AGT_NOT_FL_RC_DONE;

		    if (sub->stopTime) {
			/* need to terminate the subscription 
			 * at some point
			 */
			if (sub->flags & AGT_NOT_FL_FUTURESTOP) {
			    /* switch to timed mode */
			    sub->state = AGT_NOT_STATE_TIMED;
			} else {
			    sub->flags |= AGT_NOT_FL_NC_READY;
			}
		    } else {
			/* no stopTime, so go to live mode */
			sub->state = AGT_NOT_STATE_LIVE;
		    }
		}
	    }
	    break;
	case AGT_NOT_STATE_TIMED:
	    if (sub->lastmsg) {
		not = (agt_not_msg_t *)dlq_nextEntry(sub->lastmsg);
	    } else if (sub->lastmsgid) {
		/* use ID, back-ptr was cleared */
		not = get_entry_after(sub->lastmsgid);
	    } else {
		/* this is the first notification sent */
		not = (agt_not_msg_t *)dlq_firstEntry(&notificationQ);
	    }

	    res = NO_ERR;
	    if (not) {
		sub->lastmsg = not;
		sub->lastmsgid = not->msgid;

		ret = xml_strcmp(sub->stopTime, not->eventTime);

		res = send_notification(sub, not, TRUE);
		if (res != NO_ERR && NEED_EXIT(res)) {
		    /* treat as a fatal error */
		    sub->state = AGT_NOT_STATE_SHUTDOWN;
		}
	    } else {
		/* there is no notification to send */
		ret = xml_strcmp(sub->stopTime, nowbuff);
	    }

	    if (ret <= 0) {
		/* the future stopTime has passed;
		 * start killing the subscription next loop
		 * through this function
		 */
		sub->flags |= AGT_NOT_FL_NC_READY;
		sub->state = AGT_NOT_STATE_SHUTDOWN;
	    } /* else stopTime still in the future */
	    break;
	case AGT_NOT_STATE_LIVE:
	    if (sub->lastmsg) {
		not = (agt_not_msg_t *)dlq_nextEntry(sub->lastmsg);
	    } else if (sub->lastmsgid) {
		/* use ID, back-ptr was cleared */
		not = get_entry_after(sub->lastmsgid);
	    } else {
		/* this is the first notification sent */
		not = (agt_not_msg_t *)dlq_firstEntry(&notificationQ);
	    }
	    if (not) {
		sub->lastmsg = not;
		sub->lastmsgid = not->msgid;

		res = send_notification(sub, not, TRUE);
		if (res != NO_ERR && NEED_EXIT(res)) {
		    /* treat as a fatal error */
		    sub->state = AGT_NOT_STATE_SHUTDOWN;
		}
	    } /* else don't do anything */
	    break;
	case AGT_NOT_STATE_SHUTDOWN:
	    /* terminating the subscription after 
	     * the <notificationComplete> is sent,
	     * only if the stopTime was set
	     */
	    if (sub->stopTime) {
		if (!(sub->flags & AGT_NOT_FL_NC_DONE)) {
		    send_notificationComplete(sub);
		    sub->flags |= AGT_NOT_FL_NC_DONE;
		    break;
		}
	    }
	    dlq_remove(sub);
	    expire_subscription(sub);
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
    
}  /* agt_not_send_notifications */


/********************************************************************
* FUNCTION agt_not_remove_subscriptions
*
* Remove any expire any subscriptions with
* the specified session ID. The session is being removed
* so just zap all subscriptions with the same session ID
*
* INPUTS:
*    sid == session ID to use
*********************************************************************/
void
    agt_not_remove_subscriptions (ses_id_t sid)
{

    agt_not_subscription_t  *sub, *nextsub;

    if (!anySubscriptions) {
	return;
    }

    for (sub = (agt_not_subscription_t *)
	     dlq_firstEntry(&subscriptionQ);
	 sub != NULL;
	 sub = nextsub) {
	
	nextsub = (agt_not_subscription_t *)dlq_nextEntry(sub);

	if (sub->sid == sid) {
	    dlq_remove(sub);
	    expire_subscription(sub);
	}
    }
    
}  /* agt_not_remove_subscriptions */


/* END file agt_not.c */
