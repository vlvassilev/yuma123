/*  FILE: agt_ses.c

   NETCONF Session Manager: Agent Side Support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
06jun06      abb      begun; cloned from ses_mgr.c

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

#ifndef _H_agt_cb
#include  "agt_cb.h"
#endif

#ifndef _H_agt_connect
#include  "agt_connect.h"
#endif

#ifndef _H_agt_ses
#include  "agt_ses.h"
#endif

#ifndef _H_agt_state
#include  "agt_state.h"
#endif

#ifndef _H_agt_sys
#include  "agt_sys.h"
#endif

#ifndef _H_agt_top
#include  "agt_top.h"
#endif

#ifndef _H_agt_util
#include  "agt_util.h"
#endif

#ifndef _H_cfg
#include  "cfg.h"
#endif

#ifndef _H_def_reg
#include  "def_reg.h"
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

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define AGT_SES_DEBUG 1
#endif

/* maximum number of concurrent inbound sessions 
 * Must be >= 2 since session 0 is used for the dummy scb
 */
#define AGT_SES_MAX_SESSIONS     1024

#define AGT_SES_APP         (const xmlChar *)"netconfd"

#define AGT_SES_PARMSET     (const xmlChar *)"sessionInfo"

#define AGT_SES_MY_SESSION  (const xmlChar *)"mySession"

#define AGT_SES_LINESIZE    (const xmlChar *)"linesize"

#define AGT_SES_WITHDEF_DEFAULT (const xmlChar *)"withDefDefault"

#define AGT_SES_WITHMETA_DEFAULT (const xmlChar *)"withMetaDefault"

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

static boolean    agt_ses_init_done = FALSE;

static uint32     next_sesid;

static ses_cb_t  *agtses[AGT_SES_MAX_SESSIONS];

static ses_total_stats_t *agttotals;

#if 0
/********************************************************************
* FUNCTION make_stats_val
*
*  Make a return value for a <get> operation out of the
*  supplied session stats struct.  Add the child nodes
*  to the supplied return value
*
* INPUTS:
*   stats == session stats block to use
*   typdef == the typdef from the virtual value node for the 
*             session stats node which has the correct typdef 
*             for each child node
*   retval == output struct value to hold new child nodes
*             for the counters
* OUTPUTS:
*   retval->v.childQ has nodes added for each counter in the stats
*
*********************************************************************/
static void
    make_stats_val (const ses_stats_t *stats,
		    const typ_def_t *typdef,
		    val_value_t *retval)
{
    const typ_def_t   *realdef;
    typ_child_t *typch;
    val_value_t *chval;

    realdef = typ_get_cbase_typdef(typdef);
    for (typch = typ_first_child(TYP_DEF_COMPLEX(realdef));
	 typch != NULL;
	 typch = typ_next_child(typch)) {

	chval = val_new_virtual_chval(typch->name,
				      retval->nsid,
				      &typch->typdef, retval);
	if (!chval) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	    return;
	}

	/* brute force set the child value */
	if (!xml_strcmp(typch->name, 
			(const xmlChar *)"inBytes")) {
	    VAL_UINT(chval) = stats->in_bytes;
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"inDropMsgs")) {
	    VAL_UINT(chval) = stats->in_drop_msgs;
	} else if (!xml_strcmp(typch->name,
			       (const xmlChar *)"inMsgs")) {
	    VAL_UINT(chval) = stats->in_msgs;
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"inErrMsgs")) {
	    VAL_UINT(chval) = stats->in_err_msgs;
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"outBytes")) {
	    VAL_UINT(chval) = stats->out_bytes;
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"outDropBytes")) {
	    VAL_UINT(chval) = stats->out_drop_bytes;
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"outMsgs")) {
	    VAL_UINT(chval) = stats->out_msgs;
	} else if (!xml_strcmp(typch->name,
			       (const xmlChar *)"outErrMsgs")) {
	    VAL_UINT(chval) = stats->out_err_msgs;
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

}  /* make_stats_val */


/********************************************************************
* FUNCTION make_session_val
*
*  Make a return value for a <get> operation out of the
*  supplied session struct.  Add the child nodes
*  to the supplied return value
*
* INPUTS:
*   ses_cb_t == session control block to use
*   typdef == the typdef node for the sessionInfo struct
*             which has the correct typdef for each child node
*   retval == output struct value to hold new child nodes
*             for the counters and other data
* OUTPUTS:
*   retval->v.childQ has nodes added for each counter in the stats
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    make_session_val (const ses_cb_t *scb,
		      typ_def_t *typdef,
		      val_value_t *retval)
{
    typ_def_t   *realdef;
    typ_child_t *typch;
    val_value_t *chval;

    realdef = typ_get_base_typdef(typdef);
    for (typch = typ_first_child(TYP_DEF_COMPLEX(realdef));
	 typch != NULL;
	 typch = typ_next_child(typch)) {

	chval = val_new_virtual_chval(typch->name,
				      retval->nsid,
				      &typch->typdef, retval);
	if (!chval) {
	    return SET_ERROR(ERR_INTERNAL_MEM);
	}

	/* brute force set the child value */
	if (!xml_strcmp(typch->name, 
			(const xmlChar *)"id")) {
	    VAL_UINT(chval) = scb->sid;
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"startTime")) {
	    VAL_STR(chval) = xml_strdup(scb->start_time);
	} else if (!xml_strcmp(typch->name,
			       (const xmlChar *)"userName")) {
	    VAL_STR(chval) = xml_strdup(scb->username);
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"state")) {
	    VAL_STR(chval) = xml_strdup(ses_state_name(scb->state));
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"locks")) {
	    cfg_get_lock_list(scb->sid, chval);
	} else if (!xml_strcmp(typch->name, 
			       (const xmlChar *)"stats")) {
	    make_stats_val(&scb->stats, &typch->typdef, chval);
	} else {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
    return NO_ERR;

}  /* make_session_val */


/********************************************************************
* FUNCTION get_activeSessions
*
* <get> operation handler for the activeSessions parm
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_activeSessions (const ses_cb_t *scb,
			getcb_mode_t cbmode,
			ncx_filptr_t *fil,
			val_value_t *virval,
			val_value_t  *dstval)
{
    (void)scb;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_ULONG(dstval) = totals.active_sessions;
    }

    return NO_ERR;

} /* get_activeSessions */


/********************************************************************
* FUNCTION get_closedSessions
*
* <get> operation handler for the closedSessions parm
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_closedSessions (const ses_cb_t *scb,
			getcb_mode_t cbmode,
			ncx_filptr_t *fil,
			val_value_t *virval,
			val_value_t  *dstval)
{
    (void)scb;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_ULONG(dstval) = totals.closed_sessions;
    }

    return NO_ERR;

} /* get_closedSessions */


/********************************************************************
* FUNCTION get_failedSessions
*
* <get> operation handler for the failedSessions parm
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_failedSessions (const ses_cb_t *scb,
			getcb_mode_t cbmode,
			ncx_filptr_t *fil,
			val_value_t *virval,
			val_value_t  *dstval)
{
    (void)scb;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_ULONG(dstval) = totals.failed_sessions;
    }

    return NO_ERR;

} /* get_failedSessions */


/********************************************************************
* FUNCTION get_sessionTotals
*
* <get> operation handler for the sessionTotals parm
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_sessionTotals (const ses_cb_t *scb,
		       getcb_mode_t cbmode,
		       ncx_filptr_t *fil,
		       val_value_t *virval,
		       val_value_t  *dstval)
{
    ses_stats_t sum, *cur;

    (void)scb;

    /* check if this is a supported operation */
    if (cbmode != GETCB_GET_VALUE) {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

    /* setup the running totals with the deleted sessions totals */
    memcpy(&sum, &totals.stats, sizeof(ses_stats_t));

    /* check if a filter is supplied, and prune if so */
    if (fil != NULL) {
	;  /***/
    } else {
	/* create a complete instance of the sessionTotals 
	 * the retval is already initialized as a struct
	 */
	make_stats_val(&sum, virval->typdef, dstval);
    }

    return NO_ERR;

} /* get_sessionTotals */


/********************************************************************
* FUNCTION get_sessions
*
* <get> operation handler for the sessions parm
*
* INPUTS:
*    scb == session control block
*    cbmode == get callback mode
*    fil == optional filter
*    virval == virtual value node with the typedef struct 
*       for the 'sessions' data structure
*    dstval == initialized return val for the 'sessions' element
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_sessions (const ses_cb_t *scb,
		  getcb_mode_t cbmode,
		  ncx_filptr_t *fil,
		  val_value_t  *virval,
		  val_value_t  *dstval)
{
    const typ_def_t   *realdef;
    typ_child_t *typch;
    val_value_t *chval;
    uint32       i;
    status_t     res;

    (void)scb;

    /* check if this is a supported operation */
    if (cbmode != GETCB_GET_VALUE) {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

    /* get the 'session' typdef */
    realdef = typ_get_cbase_typdef(virval->typdef);
    typch = typ_find_child((const xmlChar *)"session",
			   TYP_DEF_COMPLEX(realdef));
    if (!typch) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    for (i = 1; i < AGT_SES_MAX_SESSIONS; i++) {
	if (agtses[i]) {
	    /* make a 'session' container */
	    chval = val_new_virtual_chval(typch->name,
					  dstval->nsid,
					  &typch->typdef, 
					  dstval);
	    if (!chval) {
		return SET_ERROR(ERR_INTERNAL_MEM);
	    }

	    /* fill in the contrainer */
	    res = make_session_val(agtses[i], &typch->typdef, chval);
	    if (res != NO_ERR) {
		return res;
	    }
	}
    }

    return NO_ERR;

} /* get_sessions */


/********************************************************************
* FUNCTION get_sessionInfo
*
* <get> operation handler for the sessionInfo parmset
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_sessionInfo (const ses_cb_t *scb,
		     getcb_mode_t cbmode,
		     ncx_filptr_t *fil,
		     val_value_t *virval,
		     val_value_t  *dstval)
{

    val_setup_virtual_retval(virval, dstval);

    /* determine which callback mode is being used */
    if (cbmode == GETCB_GET_METAQ) {
	/* no session parameters have meta-vars, quick exit */
	return NO_ERR;
    }

    /* determine which node is being requested */
    if (!xml_strcmp(virval->name, 
		    (const xmlChar *)"activeSessions")) {
	return get_activeSessions(scb, cbmode, fil, virval, dstval);
    } else if (!xml_strcmp(virval->name, 
			   (const xmlChar *)"closedSessions")) {
	return get_closedSessions(scb, cbmode, fil, virval, dstval);
    } else if (!xml_strcmp(virval->name, 
		    (const xmlChar *)"failedSessions")) {
	return get_failedSessions(scb, cbmode, fil, virval, dstval);
    } else if (!xml_strcmp(virval->name, 
		    (const xmlChar *)"sessionTotals")) {
	return get_sessionTotals(scb, cbmode, fil, virval, dstval);
    } else if (!xml_strcmp(virval->name, 
		    (const xmlChar *)"sessions")) {
	return get_sessions(scb, cbmode, fil, virval, dstval);
    } else {
	return ERR_NCX_OPERATION_FAILED;
    }

}   /* get_sessionInfo */


/********************************************************************
* FUNCTION get_linesize
*
* <get> operation handler for the linesize parm
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_linesize (const ses_cb_t *scb,
		  getcb_mode_t cbmode,
		  ncx_filptr_t *fil,
		  val_value_t *virval,
		  val_value_t  *dstval)
{
    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = scb->linesize;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* get_linesize */


/********************************************************************
* FUNCTION set_linesize
*
* <edit-config> operation handler for the linesize parm
*
* INPUTS:
*    see agt/agt_.h agt_cb_pcb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    set_linesize (ses_cb_t  *scb,
		  rpc_msg_t *msg,
		  agt_cbtyp_t cbtyp,
		  op_editop_t  editop,
		  ps_parm_t *newp,
		  ps_parm_t *curp)
{
    status_t  res;

    (void)msg;
    (void)curp;

#ifdef AGT_SES_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nagt_ses: set linesize for session %d", 
		   scb->sid);
    }
#endif

    res = NO_ERR;

    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef REMOVED
	switch (editop) {
	case OP_EDITOP_NONE:
	    /* treat as a no-op, not an error!! */
	    break;
	case OP_EDITOP_CREATE:
	    /* should not happen ... */
	    res = ERR_NCX_DATA_EXISTS;
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, NULL,
			     NCX_NT_PARM, newp, NCX_NT_PARM, newp);
	    break;
	case OP_EDITOP_DELETE:
	    /* should not happen since max-access exceeded */
	    res = ERR_NCX_NO_ACCESS_MAX;
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, NULL,
			     NCX_NT_PARM, newp, NCX_NT_PARM, newp);
	    break;
	case OP_EDITOP_MERGE:
	case OP_EDITOP_REPLACE:
	    break;
	case OP_EDITOP_LOAD:
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
#endif

	break;
    case AGT_CB_APPLY:
	if (editop != OP_EDITOP_NONE) {
	    scb->linesize = VAL_UINT(newp->val);
	}
	break;
    case AGT_CB_COMMIT:
	/* nothing to do */
	break;
    case AGT_CB_ROLLBACK:
	/***/
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* set_linesize */


/********************************************************************
* FUNCTION get_withDefDefault
*
* <get> operation handler for the withDefDefault parm
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_withDefDefault (const ses_cb_t *scb,
			getcb_mode_t cbmode,
			ncx_filptr_t *fil,
			val_value_t *virval,
			val_value_t  *dstval)
{
    if (cbmode == GETCB_GET_VALUE) {
	VAL_ENUM(dstval) = (scb->withdef) ? 1 : 0;
	VAL_ENUM_NAME(dstval) = (scb->withdef) ? 
	    NCX_EL_TRUE : NCX_EL_FALSE;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* get_withDefDefault */


/********************************************************************
* FUNCTION set_withDefDefault
*
* <edit-config> operation handler for the withDefDefault parm
*
* INPUTS:
*    see agt/agt_.h agt_cb_pcb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    set_withDefDefault (ses_cb_t  *scb,
			rpc_msg_t *msg,
			agt_cbtyp_t cbtyp,
			op_editop_t  editop,
			ps_parm_t *newp,
			ps_parm_t *curp)
{
    status_t  res;

    (void)msg;
    (void)curp;

#ifdef AGT_SES_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nagt_ses: set withdef for session %d", scb->sid);
    }
#endif

    res = NO_ERR;

    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef REMOVED

	switch (editop) {
	case OP_EDITOP_NONE:
	    /* treat as a no-op, not an error!! */
	    break;
	case OP_EDITOP_CREATE:
	    /* should not happen ... */
	    res = ERR_NCX_DATA_EXISTS;
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, NULL,
			     NCX_NT_PARM, newp, NCX_NT_PARM, newp);
	    break;
	case OP_EDITOP_DELETE:
	    /* should not happen since max-access exceeded */
	    res = ERR_NCX_NO_ACCESS_MAX;
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, NULL,
			     NCX_NT_PARM, newp, NCX_NT_PARM, newp);
	    break;
	case OP_EDITOP_MERGE:
	case OP_EDITOP_REPLACE:
	    break;
	case OP_EDITOP_LOAD:
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
#endif

	break;
    case AGT_CB_APPLY:
	if (editop != OP_EDITOP_NONE) {
	    if (VAL_ENUM(newp->val)) {
		scb->withdef = TRUE;
	    } else {
		scb->withdef = FALSE;
	    }
	}
	break;
    case AGT_CB_COMMIT:
	/* nothing to do */
	break;
    case AGT_CB_ROLLBACK:
	/***/
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* set_withDefDefault */


/********************************************************************
* FUNCTION get_withMetaDefault
*
* <get> operation handler for the withMetaDefault parm
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_withMetaDefault (const ses_cb_t *scb,
			 getcb_mode_t cbmode,
			 ncx_filptr_t *fil,
			 val_value_t *virval,
			 val_value_t  *dstval)
{
    if (cbmode == GETCB_GET_VALUE) {
	VAL_ENUM(dstval) = (scb->withmeta) ? 1 : 0;
	VAL_ENUM_NAME(dstval) = (scb->withmeta) ? 
	    NCX_EL_TRUE : NCX_EL_FALSE;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* get_withMetaDefault */


/********************************************************************
* FUNCTION set_withMetaDefault
*
* <edit-config> operation handler for the withMetaDefault parm
*
* INPUTS:
*    see agt/agt_.h agt_cb_pcb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    set_withMetaDefault (ses_cb_t  *scb,
			 rpc_msg_t *msg,
			 agt_cbtyp_t cbtyp,
			 op_editop_t  editop,
			 ps_parm_t *newp,
			 ps_parm_t *curp)
{
    status_t  res;

    (void)msg;
    (void)curp;

#ifdef AGT_SES_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nagt_ses: set withmeta for session %d", 
		   scb->sid);
    }
#endif

    res = NO_ERR;

    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef REMOVED
	switch (editop) {
	case OP_EDITOP_NONE:
	    /* treat as a no-op, not an error!! */
	    break;
	case OP_EDITOP_CREATE:
	    /* should not happen ... */
	    res = ERR_NCX_DATA_EXISTS;
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, NULL,
			     NCX_NT_PARM, newp, NCX_NT_PARM, newp);
	    break;
	case OP_EDITOP_DELETE:
	    /* should not happen since max-access exceeded */
	    res = ERR_NCX_NO_ACCESS_MAX;
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, NULL,
			     NCX_NT_PARM, newp, NCX_NT_PARM, newp);
	    break;
	case OP_EDITOP_MERGE:
	case OP_EDITOP_REPLACE:
	    break;
	case OP_EDITOP_LOAD:
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
#endif
	break;
    case AGT_CB_APPLY:
	if (editop != OP_EDITOP_NONE) {
	    if (VAL_ENUM(newp->val)) {
		scb->withmeta = TRUE;
	    } else {
		scb->withmeta = FALSE;
	    }
	}
	break;
    case AGT_CB_COMMIT:
	/* nothing to do */
	break;
    case AGT_CB_ROLLBACK:
	/***/
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* set_withMetaDefault */


/********************************************************************
* FUNCTION get_mySession
*
* <get> operation handler for the mySession parmset
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_mySession (const ses_cb_t *scb,
		   getcb_mode_t cbmode,
		   ncx_filptr_t *fil,
		   val_value_t *virval,
		   val_value_t  *dstval)
{

    val_setup_virtual_retval(virval, dstval);

    /* determine which callback mode is being used */
    if (cbmode == GETCB_GET_METAQ) {
	/* no session parameters have meta-vars, quick exit */
	return NO_ERR;
    }

    /* determine which node is being requested */
    if (!xml_strcmp(virval->name, AGT_SES_LINESIZE)) {
	return get_linesize(scb, cbmode, fil, virval, dstval);
    } else if (!xml_strcmp(virval->name, AGT_SES_WITHDEF_DEFAULT)) {
	return get_withDefDefault(scb, cbmode, fil, virval, dstval);
    } else if (!xml_strcmp(virval->name, 
		    (const xmlChar *)"withMetaDefault")) {
	return get_withMetaDefault(scb, cbmode, fil, virval, dstval);
    } else {
	return ERR_NCX_OPERATION_FAILED;
    }

}   /* get_mySession */

#endif  /* 0 */


/************* E X T E R N A L    F U N C T I O N S ***************/


/********************************************************************
* FUNCTION agt_ses_init
*
* INIT 1:
*   Initialize the session manager module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_ses_init (void)
{
    uint32             i;

    if (agt_ses_init_done) {
	return ERR_INTERNAL_INIT_SEQ;
    }

    for (i=0; i<AGT_SES_MAX_SESSIONS; i++) {
	agtses[i] = NULL;
    }
    next_sesid = 1;

    agttotals = ses_get_total_stats();
    memset(agttotals, 0x0, sizeof(ses_total_stats_t));
    tstamp_datetime(agttotals->startTime);

    agt_ses_init_done = TRUE;
    return NO_ERR;

}  /* agt_ses_init */


/********************************************************************
* FUNCTION agt_ses_init2
*
* INIT 2:
*   Initialize the virtual parmsets and their callbacks
*   This must be done after the <running> config is loaded
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_ses_init2 (void)
{
#if 0
    status_t  res;

    if (!agt_ses_init_done) {
	return ERR_INTERNAL_INIT_SEQ;
    }

    /* create a virtual parmset for the sessionInfo node */
    sesinfo = agt_ps_new_vparmset(AGT_SES_MODULE,
				  AGT_SES_PARMSET,
				  get_sessionInfo, FULL, &res);
    if (!sesinfo || res != NO_ERR) {
	return res;
    }

    /* create a transient parmset for the mySession node */
    mySession = agt_ps_new_vparmset(AGT_SES_MODULE,
				    AGT_SES_MY_SESSION, 
				    get_mySession, FULL, &res);
    if (!mySession || res != NO_ERR) {
	return res;
    }

    /* register callback functions for the parameters */
    res = agt_cb_register_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_LINESIZE,
					FORALL,
					AGT_CB_APPLY,
					set_linesize);
    if (res != NO_ERR) {
	return res;
    }

    res = agt_cb_register_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_WITHDEF_DEFAULT,
					FORALL,
					AGT_CB_APPLY,
					set_withDefDefault);
    if (res != NO_ERR) {
	return res;
    }

    res = agt_cb_register_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_WITHMETA_DEFAULT,
					FORALL,
					AGT_CB_APPLY,
					set_withMetaDefault);

    return res;
#endif
    return NO_ERR;

}  /* agt_ses_init2 */


/********************************************************************
* FUNCTION agt_ses_cleanup
*
* Cleanup the session manager module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    agt_ses_cleanup (void)
{
    uint32 i;

    if (agt_ses_init_done) {
	for (i=0; i<AGT_SES_MAX_SESSIONS; i++) {
	    if (agtses[i]) {
		agt_ses_free_session(agtses[i]);
	    }
	}

	next_sesid = 0;

#if 0
	if (sesinfo) {
	    /* ps_free_parmset(sesinfo); */
	    sesinfo = NULL;
	}

	if (mySession) {
	    /* ps_free_parmset(mySession); */
	    mySession = NULL;
	}

	/* unregister callback functions for the parameters */
	agt_cb_unregister_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_LINESIZE);
	agt_cb_unregister_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_WITHDEF_DEFAULT);
	agt_cb_unregister_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_WITHMETA_DEFAULT);

#endif

	agt_ses_init_done = FALSE;
    }

}  /* agt_ses_cleanup */


/********************************************************************
* FUNCTION agt_ses_new_dummy_session
*
* Create a dummy session control block
*
* INPUTS:
*   none
* RETURNS:
*   pointer to initialized dummy SCB, or NULL if malloc error
*********************************************************************/
ses_cb_t *
    agt_ses_new_dummy_session (void)
{
    ses_cb_t  *scb;

    if (!agt_ses_init_done) {
	agt_ses_init();
    }

    /* check if dummy session already cached */
    if (agtses[0]) {
	SET_ERROR(ERR_INTERNAL_INIT_SEQ);
	return NULL;
    }

    /* no, so create it */
    scb = ses_new_dummy_scb();
    if (!scb) {
	return NULL;
    }

    agtses[0] = scb;
    
    return scb;

}  /* agt_ses_new_dummy_session */


/********************************************************************
* FUNCTION agt_ses_free_dummy_session
*
* Free a dummy session control block
*
* INPUTS:
*   scb == session control block to free
*
*********************************************************************/
void
    agt_ses_free_dummy_session (ses_cb_t *scb)
{
#ifdef DEBUG
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    if (!agt_ses_init_done) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    if (scb->sid != 0 || !agtses[0]) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    ses_free_scb(scb);
    agtses[0] = NULL;

}  /* agt_ses_free_dummy_session */


/********************************************************************
* FUNCTION agt_ses_new_session
*
* Create a real agent session control block
*
* INPUTS:
*   transport == the transport type
*   fd == file descriptor number to use for IO 
* RETURNS:
*   pointer to initialized SCB, or NULL if some error
*   This pointer is stored in the session table, so it does
*   not have to be saved by the caller
*********************************************************************/
ses_cb_t *
    agt_ses_new_session (ses_transport_t transport,
			 int fd)
{
    ses_cb_t  *scb;
    uint32     i, slot;
    status_t   res;

    if (!agt_ses_init_done) {
	agt_ses_init();
    }

    res = NO_ERR;
    slot = 0;
    scb = NULL;

    /* check if any sessions are available */
    if (!next_sesid) {
	/* end has already been reached, so now in brute force
	 * session reclaim mode
	 */
	slot = 0;
	for (i=1; i<AGT_SES_MAX_SESSIONS && !slot; i++) {
	    if (!agtses[i]) {
		slot = i;
	    }
	}
    } else {
	slot = next_sesid;
    }

    if (slot) {
	/* make sure there is memory for a session control block */
	scb = ses_new_scb();
	if (scb) {
	    /* initialize the static vars */
	    scb->type = SES_TYP_NETCONF;
	    scb->transport = transport;
	    scb->state = SES_ST_INIT;
	    scb->mode = SES_MODE_XML;
	    scb->sid = slot;
	    scb->inready.sid = slot;
	    scb->outready.sid = slot;
	    scb->state = SES_ST_INIT;
	    scb->fd = fd;
	    scb->instate = SES_INST_IDLE;
	    res = ses_msg_new_buff(scb, &scb->outbuff);
	} else {
	    res = ERR_INTERNAL_MEM;
	}
    } else {
	res = ERR_NCX_RESOURCE_DENIED;
    }

    /* add the FD to SCB mapping in the definition registry */
    if (res == NO_ERR) {
	res = def_reg_add_scb(scb->fd, scb);
    }

    /* check result and add to session array if NO_ERR */
    if (res == NO_ERR) {
	agtses[slot] = scb;

	/* update the next slot now */
	if (next_sesid) {
	    if (++next_sesid==AGT_SES_MAX_SESSIONS) {
		/* reached the end */
		next_sesid = 0;
	    }
	}
	if (LOGINFO) {
	    log_info("\nNew session %d created OK", slot);
	}
	agttotals->inSessions++;
	agttotals->active_sessions++;
    } else {
	if (scb) {
	    ses_free_scb(scb);
	    scb = NULL;
	}
	if (LOGINFO) {
	    log_info("\nNew session request failed (%s)",
		     get_error_string(res));
	}
    }

    return scb;

}  /* agt_ses_new_session */


/********************************************************************
* FUNCTION agt_ses_free_session
*
* Free a real session control block
*
* INPUTS:
*   scb == session control block to free
*
*********************************************************************/
void
    agt_ses_free_session (ses_cb_t *scb)
{
    ses_id_t  slot;


#ifdef DEBUG
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    if (!agt_ses_init_done) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    slot = scb->sid;

    if (scb->fd) {
	def_reg_del_scb(scb->fd);
    }

    cfg_release_locks(slot);

    agt_state_remove_session(slot);
    agt_not_remove_subscriptions(slot);

    /* add this session to ses stats */
    agttotals->active_sessions--;
    if (scb->active) {
	agttotals->closed_sessions++;
    } else {
	agttotals->failed_sessions++;
    }

    /* this will close the socket if it is still open */
    ses_free_scb(scb);

    agtses[slot] = NULL;

    if (LOGINFO) {
	log_info("\nSession %d closed", slot);
    }

}  /* agt_ses_free_session */


/********************************************************************
* FUNCTION agt_ses_session_id_valid
*
* Check if a session-id is for an active session
*
* INPUTS:
*   sid == session ID to check
* RETURNS:
*   TRUE if active session; FALSE otherwise
*********************************************************************/
boolean
    agt_ses_session_id_valid (ses_id_t  sid)
{
    if (sid < AGT_SES_MAX_SESSIONS && agtses[sid]) {
	return TRUE;
    } else {
	return FALSE;
    }
} /* agt_ses_session_id_valid */


/********************************************************************
* FUNCTION agt_ses_request_close
*
* Start the close of the specified session
*
* INPUTS:
*   sid == session ID to close
*   killedby == session ID executing the kill-session or close-session
*   termreason == termination reason code
*
* RETURNS:
*   none
*********************************************************************/
void
    agt_ses_request_close (ses_id_t sid,
			   ses_id_t killedby,
			   ses_term_reason_t termreason)
{
    ses_cb_t *scb;

    /* check the session ID */
    if (!sid || sid > AGT_SES_MAX_SESSIONS) {
	return;
    }

    /* check the session control block */
    scb = agtses[sid];
    if (!scb) {
	return;
    }

    /* N/A for dummy sessions */
    if (scb->type==SES_TYP_DUMMY) {
	return;
    }

    scb->killedbysid = killedby;
    scb->termreason = termreason;

    /* change the session control block state */
    switch (scb->state) {
    case SES_ST_IDLE:
    case SES_ST_SHUTDOWN:
    case SES_ST_SHUTDOWN_REQ:
	agt_ses_kill_session(scb->sid, 
			     killedby,
			     termreason);
	break;
    case SES_ST_IN_MSG:
	scb->state = SES_ST_SHUTDOWN_REQ;
	break;
    default:
	if (dlq_empty(&scb->outQ)) {
	    agt_ses_kill_session(scb->sid, 
				 killedby,
				 termreason);
	} else {
	    scb->state = SES_ST_SHUTDOWN_REQ;
	}
    }

}  /* agt_ses_request_close */


/********************************************************************
* FUNCTION agt_ses_kill_session
*
* Kill the specified session
*
* INPUTS:
*   sid == session ID to kill
* RETURNS:
*   none
*********************************************************************/
void
    agt_ses_kill_session (ses_id_t sid,
			  ses_id_t killedby,
			  ses_term_reason_t termreason)
{
    ses_cb_t *scb;

    if (!sid || sid > AGT_SES_MAX_SESSIONS) {
	return;
    }

    scb = agtses[sid];
    if (!scb) {
	return;    /* no session found for this ID */
    }

    /* N/A for dummy sessions */
    if (scb->type==SES_TYP_DUMMY) {
	return;
    }

    agt_sys_send_sysSessionEnd(scb,
			       termreason,
			       killedby);

    /* clear all resources and delete the session control block */
    agt_ses_free_session(scb);

} /* agt_ses_kill_session */


/********************************************************************
* FUNCTION agt_ses_process_first_ready
*
* Check the readyQ and process the first message, if any
*
* RETURNS:
*     TRUE if a message was processed
*     FALSE if the readyQ was empty
*********************************************************************/
boolean
    agt_ses_process_first_ready (void)
{
    ses_cb_t     *scb;
    ses_ready_t  *rdy;
    ses_msg_t    *msg;
    status_t      res;
    ses_id_t      mysid;

#ifdef AGT_SES_DEBUG
    xmlChar       buff[32];
    uint32        cnt;
#endif

    rdy = ses_msg_get_first_inready();
    if (!rdy) {
	return FALSE;
    }

    /* get the session control block that rdy is embedded into */
    scb = agtses[rdy->sid];
    mysid = scb->sid;

#ifdef AGT_SES_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nagt_ses msg ready for session %d", scb->sid);
    }
#endif

    /* check the session control block state */
    if (scb->state >= SES_ST_SHUTDOWN_REQ) {
	/* don't process the message or even it mark it
	 * It will be cleaned up when the session is freed
	 */
#ifdef AGT_SES_DEBUG
	if (LOGDEBUG) {
	    log_debug("\nagt_ses drop input, session %d shutting down", 
		      scb->sid);
	}
#endif

	return TRUE;
    }

#ifdef AGT_SES_DEBUG
    /* make sure a message is really there */
    msg = (ses_msg_t *)dlq_firstEntry(&scb->msgQ);
    if (!msg || !msg->ready) {
	SET_ERROR(ERR_INTERNAL_PTR);
	log_error("\nagt_ses ready Q message not correct");
	return FALSE;
    } else if (LOGDEBUG2) {
	cnt = xml_strcpy(buff, 
			 (const xmlChar *)"Incoming msg for session ");
	sprintf((char *)(&buff[cnt]), "%u", scb->sid);
	ses_msg_dump(msg, buff);
    }
#endif

    /* setup the XML parser */
    if (scb->reader) {
	    /* reset the xmlreader */
	res = xml_reset_reader_for_session(ses_read_cb,
					   NULL, 
					   scb, 
					   scb->reader);
    } else {
	res = xml_get_reader_for_session(ses_read_cb,
					 NULL, 
					 scb, 
					 &scb->reader);
    }

    /* process the message */
    if (res == NO_ERR) {
	/* process the message */
	agt_top_dispatch_msg(scb);
    } else {
	if (LOGINFO) {
	    log_info("\nReset xmlreader failed for session %d (%s)",
		     scb->sid, 
		     get_error_string(res));
	}
	scb->state = SES_ST_SHUTDOWN_REQ;
    }

    /* get the session control block again to make sure it was not
     * removed due to invalid <ncxconnect> that caused the session
     * to be deleted
     */
    scb = agtses[mysid];

    if (scb) {
	/* free the message that was just processed */
	msg = (ses_msg_t *)dlq_deque(&scb->msgQ);
	if (msg) {
	    ses_msg_free_msg(scb, msg);
	}

	/* check if any messages left for this session */
	if (!dlq_empty(&scb->msgQ)) {
	    ses_msg_make_inready(scb);
	}
    }

    return TRUE;
    
}  /* agt_ses_process_first_ready */


/********************************************************************
* FUNCTION agt_ses_ssh_port_allowed
*
* Check if the port number used for SSH connect is okay
*
* RETURNS:
*     TRUE if port allowed
*     FALSE if port not allowed
*********************************************************************/
boolean
    agt_ses_ssh_port_allowed (uint16 port)
{
    const agt_profile_t *profile;
    uint32         i;

    if (port == 0) {
	return FALSE;
    }

    profile = agt_get_profile();
    if (!profile) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    if (profile->agt_ports[0]) {
	/* something configured, so use only that list */
	for (i = 0; i < AGT_MAX_PORTS; i++) {
	    if (port == profile->agt_ports[i]) {
		return TRUE;
	    }
	}
	return FALSE;
    } else {
	/* no ports configured so allow 830 */
	if (port==NCX_NCSSH_PORT) {
	    return TRUE;
	} else {
#ifdef MACOSX
	    /* there is a bug in the libssh2 and even though
	     * connections on port 830 are accepted, they
	     * are reported as port 22 (SSH) in the
	     * SSH_CONNECTION environment variable
	     */
	    if (port==22) {
		return TRUE;
	    }
#endif
	    return FALSE;
	}
    }
    /*NOTREACHED*/

}  /* agt_ses_ssh_port_allowed */


/********************************************************************
* FUNCTION agt_ses_fill_writeset
*
* Drain the ses_msg outreadyQ and set the specified fdset
* Used by agt_ncxserver write_fd_set
*
* INPUTS:
*    fdset == pointer to fd_set to fill
*    maxfdnum == pointer to max fd int to fill in
*
* OUTPUTS:
*    *fdset is updated in
*    *maxfdnum may be updated
*********************************************************************/
void
    agt_ses_fill_writeset (fd_set *fdset,
			   int *maxfdnum)
{
    ses_ready_t *rdy;
    ses_cb_t *scb;
    boolean done;

    FD_ZERO(fdset);
    done = FALSE;
    while (!done) {
	rdy = ses_msg_get_first_outready();
	if (!rdy) {
	    done = TRUE;
	} else {
	    scb = agtses[rdy->sid];
	    if (scb && scb->state <= SES_ST_SHUTDOWN_REQ) {
		FD_SET(scb->fd, fdset);
		if (scb->fd > *maxfdnum) {
		    *maxfdnum = scb->fd;
		}
	    }
	}
    }

}  /* agt_ses_fill_writeset */


/********************************************************************
* FUNCTION agt_ses_get_inSessions
*
* <get> operation handler for the inSessions counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_inSessions (ses_cb_t *scb,
			    getcb_mode_t cbmode,
			    val_value_t *virval,
			    val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->inSessions;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_inSessions */


/********************************************************************
* FUNCTION agt_ses_get_inXMLParseErrors
*
* <get> operation handler for the inXMLParseErrors counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_inXMLParseErrors (ses_cb_t *scb,
				  getcb_mode_t cbmode,
				  val_value_t *virval,
				  val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->stats.inXMLParseErrors;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_inXMLParseErrors */


/********************************************************************
* FUNCTION agt_ses_get_inBadHellos
*
* <get> operation handler for the inBadHellos counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_inBadHellos (ses_cb_t *scb,
			     getcb_mode_t cbmode,
			     val_value_t *virval,
			     val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->stats.inBadHellos;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_inBadHellos */


/********************************************************************
* FUNCTION agt_ses_get_inRpcs
*
* <get> operation handler for the inRpcs counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_inRpcs (ses_cb_t *scb,
			getcb_mode_t cbmode,
			val_value_t *virval,
			val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->stats.inRpcs;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_inRpcs */


/********************************************************************
* FUNCTION agt_ses_get_inBadRpcs
*
* <get> operation handler for the inBadRpcs counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_inBadRpcs (ses_cb_t *scb,
			   getcb_mode_t cbmode,
			   val_value_t *virval,
			   val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->stats.inBadRpcs;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_inBadRpcs */


/********************************************************************
* FUNCTION agt_ses_get_inNotSupportedRpcs
*
* <get> operation handler for the inNotSupportedRpcs counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_inNotSupportedRpcs (ses_cb_t *scb,
				    getcb_mode_t cbmode,
				    val_value_t *virval,
				    val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->stats.inNotSupportedRpcs;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_inNotSupportedRpcs */


/********************************************************************
* FUNCTION agt_ses_get_outRpcReplies
*
* <get> operation handler for the outRpcReplies counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_outRpcReplies (ses_cb_t *scb,
			       getcb_mode_t cbmode,
			       val_value_t *virval,
			       val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->stats.outRpcReplies;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_outRpcReplies */


/********************************************************************
* FUNCTION agt_ses_get_outRpcErrors
*
* <get> operation handler for the outRpcErrors counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_outRpcErrors (ses_cb_t *scb,
			      getcb_mode_t cbmode,
			      val_value_t *virval,
			      val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->stats.outRpcErrors;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_outRpcErrors */


/********************************************************************
* FUNCTION agt_ses_get_outNotifications
*
* <get> operation handler for the outNotifications counter
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    agt_ses_get_outNotifications (ses_cb_t *scb,
				  getcb_mode_t cbmode,
				  val_value_t *virval,
				  val_value_t  *dstval)
{
    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
	VAL_UINT(dstval) = agttotals->stats.outNotifications;
	return NO_ERR;
    } else {
	return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* agt_ses_get_outNotifications */


/* END file agt_ses.c */
