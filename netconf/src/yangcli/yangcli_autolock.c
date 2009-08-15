/*  FILE: yangcli_autolock.c

   NETCONF YANG-based CLI Tool

   autolock support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
13-aug-09    abb      begun; started from yangcli.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libssh2.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include "libtecla.h"

#ifndef _H_procdefs
#include "procdefs.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_mgr
#include "mgr.h"
#endif

#ifndef _H_mgr_ses
#include "mgr_ses.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val_util
#include "val_util.h"
#endif

#ifndef _H_var
#include "var.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xml_val
#include "xml_val.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yangcli
#include "yangcli.h"
#endif

#ifndef _H_yangcli_autolock
#include "yangcli_autolock.h"
#endif

#ifndef _H_yangcli_cmd
#include "yangcli_cmd.h"
#endif

#ifndef _H_yangcli_util
#include "yangcli_util.h"
#endif


/********************************************************************
* FUNCTION send_lock_pdu_to_agent
* 
* Send a <lock> or <unlock> operation to the agent
*
* INPUTS:
*   agent_cb == agent control block to use
*   lockcb == lock control block to use within agent_cb
*   islock == TRUE for lock; FALSE for unlock
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_lock_pdu_to_agent (agent_cb_t *agent_cb,
                            lock_cb_t *lockcb,
                            boolean islock)
{
    const obj_template_t  *rpc, *input;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *targetval, *parmval;
    ses_cb_t              *scb;
    status_t               res;
    ncx_cfg_t              cfg_id;
    xmlns_id_t             obj_nsid;

    req = NULL;
    reqdata = NULL;
    res = NO_ERR;
    cfg_id = lockcb->config_id;

    if (islock) {
        rpc = ncx_find_object(get_netconf_mod(), 
                              NCX_EL_LOCK);
    } else {
        rpc = ncx_find_object(get_netconf_mod(), 
                              NCX_EL_UNLOCK);
    }
    if (!rpc) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    obj_nsid = obj_get_nsid(rpc);

    /* get the 'input' section container */
    input = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!input) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* construct a method + parameter tree */
    reqdata = xml_val_new_struct(obj_get_name(rpc), obj_nsid);
    if (!reqdata) {
	log_error("\nError allocating a new RPC request");
	return ERR_INTERNAL_MEM;
    }

    /* set the [un]lock/input/target node to 'cfg_id' */
    targetval = xml_val_new_struct(NCX_EL_TARGET, obj_nsid);
    if (!targetval) {
	log_error("\nError allocating a new RPC request");
        val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    } else {
        val_add_child(targetval, reqdata);
    }

    parmval = xml_val_new_flag(lockcb->config_name,
                               obj_nsid);
    if (!parmval) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    } else {
        val_add_child(parmval, targetval);
    }

    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    } else {
	req = mgr_rpc_new_request(scb);
	if (!req) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nError allocating a new RPC request");
	} else {
	    req->data = reqdata;
	    req->rpc = rpc;
	    req->timeout = agent_cb->timeout;
	}
    }
	
    /* if all OK, send the RPC request */
    if (res == NO_ERR) {
	if (LOGDEBUG2) {
	    log_debug2("\nabout to send RPC request with reqdata:");
	    val_dump_value_ex(reqdata, 
                              NCX_DEF_INDENT,
                              agent_cb->display_mode);
	}

	/* the request will be stored if this returns NO_ERR */
	res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
        if (res == NO_ERR) {
            if (islock) {
                lockcb->lock_state = LOCK_STATE_REQUEST_SENT;
            } else {
                lockcb->lock_state = LOCK_STATE_RELEASE_SENT;
            }
            (void)time(&lockcb->last_msg_time);
            agent_cb->locks_cur_cfg = lockcb->config_id;
        }
    }

    /* cleanup and set next state */
    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_lock_pdu_to_agent */


/********************************************************************
 * FUNCTION do_get_locks (local RPC)
 * 
 * get all the locks on the agent
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the history command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t
    do_get_locks (agent_cb_t *agent_cb,
                  const obj_template_t *rpc,
                  const xmlChar *line,
                  uint32  len)
{
    ses_cb_t      *scb;
    val_value_t   *valset, *parm;
    uint32         locks_timeout, retry_interval;
    boolean        cleanup, done;
    status_t       res;

    if (agent_cb->locks_active) {
        log_error("\nError: locks are already active");
        return ERR_NCX_OPERATION_FAILED;
    }
    if (agent_cb->state != MGR_IO_ST_CONN_IDLE) {
        log_error("\nError: no active session to lock");
        return ERR_NCX_OPERATION_FAILED;
    }

    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (scb == NULL) {
        log_error("\nError: active session dropped, cannot lock");
        return ERR_NCX_OPERATION_FAILED;
    }

    locks_timeout = agent_cb->locks_timeout;
    retry_interval = agent_cb->locks_retry_interval;
    cleanup = TRUE;

    res = NO_ERR;

    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (valset && res == NO_ERR) {
        /* get the overall lock timeout */
	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      YANGCLI_LOCK_TIMEOUT);
	if (parm && parm->res == NO_ERR) {
	    locks_timeout = VAL_UINT(parm);
        }

        /* get the retry interval between failed locks */
        parm = val_find_child(valset, 
                              YANGCLI_MOD, 
                              YANGCLI_RETRY_INTERVAL);
	if (parm && parm->res == NO_ERR) {
	    retry_interval = VAL_UINT(parm);
        }

        /* get the auto-cleanup flag */
        parm = val_find_child(valset, 
                              YANGCLI_MOD, 
                              YANGCLI_CLEANUP);
	if (parm && parm->res == NO_ERR) {
	    cleanup = VAL_BOOL(parm);
        }
    }

    /* start the auto-lock procedure */
    setup_lock_cbs(agent_cb);
    agent_cb->locks_timeout = locks_timeout;
    agent_cb->locks_retry_interval = retry_interval;
    agent_cb->locks_cleanup = cleanup;

    done = FALSE;
    if (LOGINFO) {
        log_info("\nSending <lock> operations for get-locks...\n");
    }
    res = handle_get_locks_request_to_agent(agent_cb,
                                            TRUE,
                                            &done);
    if (res != NO_ERR && done) {
        /* need to undo the whole thing; whatever got done */

    }

    if (valset != NULL) {
        val_free_value(valset);
    }

    return res;

}  /* do_get_locks */


/********************************************************************
 * FUNCTION do_release_locks (local RPC)
 * 
 * release all the locks on the agent
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the history command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t
    do_release_locks (agent_cb_t *agent_cb,
                  const obj_template_t *rpc,
                  const xmlChar *line,
                  uint32  len)
{
    ses_cb_t      *scb;
    val_value_t   *valset;
    uint32         locks_timeout, retry_interval;
    boolean        cleanup, done, needed;
    status_t       res;

    if (!agent_cb->locks_active) {
        log_error("\nError: locks are not active");
        return ERR_NCX_OPERATION_FAILED;
    }
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (scb == NULL) {
        log_error("\nError: active session dropped, cannot lock");
        return ERR_NCX_OPERATION_FAILED;
    }

    locks_timeout = agent_cb->locks_timeout;
    retry_interval = agent_cb->locks_retry_interval;
    cleanup = TRUE;

    res = NO_ERR;
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (res == NO_ERR || res == ERR_NCX_SKIPPED) {

        /* start the auto-unlock procedure */
        agent_cb->locks_timeout = locks_timeout;
        agent_cb->locks_retry_interval = retry_interval;
        agent_cb->locks_cleanup = cleanup;
        needed = setup_unlock_cbs(agent_cb);

        if (LOGINFO && needed) {
            log_info("\nSending <unlock> operations for release-locks...\n");
        }

        if (needed) {
            done = FALSE;
            res = handle_release_locks_request_to_agent(agent_cb,
                                                        TRUE,
                                                        &done);
            if (done) {
                /* need to close the session or report fatal error */
                clear_lock_cbs(agent_cb);
            }
        }
    }

    if (valset != NULL) {
        val_free_value(valset);
    }

    return res;

}  /* do_release_locks */


/********************************************************************
* FUNCTION handle_get_locks_request_to_agent
* 
* Send the first <lock> operation to the agent
* in a get-locks command
*
* INPUTS:
*   agent_cb == agent control block to use
*   first == TRUE if this is the first call; FALSE otherwise
*   done == address of return final done flag
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*    *done == TRUE when the return code is NO_ERR and all
*                 the locks are granted
*             FALSE otherwise
* RETURNS:
*    status; if NO_ERR then check *done flag
*            otherwise done is true on any error
*********************************************************************/
status_t
    handle_get_locks_request_to_agent (agent_cb_t *agent_cb,
                                       boolean first,
                                       boolean *done)
{
    lock_cb_t      *lockcb;
    ncx_cfg_t       cfg_id;
    time_t          timenow;
    double          timediff;
    status_t        res;
    boolean         finddone, stillwaiting;

#ifdef DEBUG
    if (!agent_cb || !done) {
        return  SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    *done = FALSE;
    finddone = FALSE;
    stillwaiting = FALSE;
    lockcb = NULL;
    (void)time(&timenow);

    if (first) {
        /* this is the first try, start the overall timer */
        (void)time(&agent_cb->locks_start_time);
    } else if (check_locks_timeout(agent_cb)) {
        log_error("\nError: get-locks timeout");
        handle_locks_cleanup(agent_cb);
        return ERR_NCX_TIMEOUT;
    }

    /* find a config that needs a lock PDU sent
     * first check for a config that has not sent
     * any request yet
     */
    for (cfg_id = NCX_CFGID_RUNNING;
         cfg_id <= NCX_CFGID_STARTUP && !finddone;
         cfg_id++) {

        lockcb = &agent_cb->lock_cb[cfg_id];

        if (lockcb->lock_used) {
            if (lockcb->lock_state == LOCK_STATE_IDLE) {
                finddone = TRUE;
            } else if (lockcb->lock_state == LOCK_STATE_FATAL_ERROR) {
                log_error("\nError: fatal error getting lock"
                          " on the %s config",
                          lockcb->config_name);
                return ERR_NCX_OPERATION_FAILED;
            }
        }
    }

    if (!finddone) {
        /* all entries that need to send a lock
         * request have tried at least once to do that
         * now go through all the states and see if any
         * retries are needed; exit if a fatal error is found
         */
        stillwaiting = FALSE;
        for (cfg_id = NCX_CFGID_RUNNING;
             cfg_id <= NCX_CFGID_STARTUP && !finddone;
             cfg_id++) {

            lockcb = &agent_cb->lock_cb[cfg_id];
            if (lockcb->lock_used) {
                if (lockcb->lock_state == LOCK_STATE_TEMP_ERROR) {
                    timediff = difftime(timenow, lockcb->last_msg_time);
                    if (timediff >= 
                        (double)agent_cb->locks_retry_interval) {
                        finddone = TRUE;
                    } else {
                        agent_cb->locks_waiting = TRUE;
                        stillwaiting = TRUE;
                    }
                }
            }
        }

        /* check if there is more work to do after this 
         * function exits
         */
        if (!finddone && !stillwaiting) {
            *done = TRUE;
        }
    }

    /* check if a <lock> request needs to be sent */
    if (finddone && lockcb) {
        agent_cb->command_mode = CMD_MODE_AUTOLOCK;
        res = send_lock_pdu_to_agent(agent_cb,
                                     lockcb,
                                     TRUE);
    }

    return res;
    
}  /* handle_get_locks_request_to_agent */


/********************************************************************
* FUNCTION handle_release_locks_request_to_agent
* 
* Send an <unlock> operation to the agent
* in a get-locks command teardown or a release-locks
* operation
*
* INPUTS:
*   agent_cb == agent control block to use
*   first == TRUE if this is the first call; FALSE otherwise
*   done == address of return final done flag
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*    *done == TRUE when the return code is NO_ERR and all
*                 the locks are granted
*             FALSE otherwise
* RETURNS:
*    status; if NO_ERR then check *done flag
*            otherwise done is true on any error
*********************************************************************/
status_t
    handle_release_locks_request_to_agent (agent_cb_t *agent_cb,
                                           boolean first,
                                           boolean *done)
{
    lock_cb_t      *lockcb;
    ncx_cfg_t       cfg_id;
    status_t        res;
    boolean         finddone;

#ifdef DEBUG
    if (!agent_cb || !done) {
        return  SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    lockcb = NULL;
    res = NO_ERR;
    finddone = FALSE;
    *done = FALSE;

    if (first) {
        agent_cb->command_mode = CMD_MODE_AUTOUNLOCK;
        (void)time(&agent_cb->locks_start_time);
    } else if (check_locks_timeout(agent_cb)) {
        log_error("\nError: release-locks timeout");
        clear_lock_cbs(agent_cb);
        return ERR_NCX_TIMEOUT;
    }

    /* do these in order because there are no
     * temporary errors for unlock
     */
    for (cfg_id = NCX_CFGID_RUNNING;
         cfg_id <= NCX_CFGID_STARTUP && !finddone;
         cfg_id++) {

        lockcb = &agent_cb->lock_cb[cfg_id];
        if (lockcb->lock_used &&
            lockcb->lock_state == LOCK_STATE_ACTIVE) {
            finddone = TRUE;
        }
    }

    if (!finddone) {
        /* nothing to do */
        if (first) {
            log_info("\nNo locks to release");
        }
        agent_cb->state = MGR_IO_ST_CONN_IDLE;
        clear_lock_cbs(agent_cb);
        *done = TRUE;
    } else {
        res = send_lock_pdu_to_agent(agent_cb,
                                     lockcb,
                                     FALSE);
    }

    return res;
    
}  /* handle_release_locks_request_to_agent */


/********************************************************************
* FUNCTION handle_locks_cleanup
* 
* Deal with the cleanup for the get-locks or release-locks
*
* INPUTS:
*   agent_cb == agent control block to use
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*
*********************************************************************/
void
    handle_locks_cleanup (agent_cb_t *agent_cb)
{
    status_t        res;
    boolean         done;

#ifdef DEBUG
    if (!agent_cb) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (!use_agentcb(agent_cb)) {
        log_error("\nError: connection lost, canceling release-locks");
        clear_lock_cbs(agent_cb);
        return;
    }

    if (agent_cb->locks_cleanup) {
        agent_cb->command_mode = CMD_MODE_AUTOUNLOCK;
        done = FALSE;
        res = handle_release_locks_request_to_agent(agent_cb,
                                                    TRUE,
                                                    &done);
        if (done) {
            clear_lock_cbs(agent_cb);
        }
    } else {
        clear_lock_cbs(agent_cb);
    }

}  /* handle_locks_cleanup */


/********************************************************************
* FUNCTION check_locks_timeout
* 
* Check if the locks_timeout is active and if it expired yet
*
* INPUTS:
*   agent_cb == agent control block to use
*
* RETURNS:
*   TRUE if locks_timeout expired
*   FALSE if no timeout has occurred
*********************************************************************/
boolean
    check_locks_timeout (agent_cb_t *agent_cb)
{
    time_t          timenow;
    double          timediff;

#ifdef DEBUG
    if (!agent_cb) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (agent_cb->locks_timeout) {
        /* check if there is an overall timeout yet */
        (void)time(&timenow);
        timediff = difftime(timenow,
                            agent_cb->locks_start_time);
        if (timediff >= (double)agent_cb->locks_timeout) {
            log_debug("\nlock timeout");
            return TRUE;
        }
    }
    return FALSE;

}  /* check_locks_timeout */


/********************************************************************
* FUNCTION send_discard_changes_pdu_to_agent
* 
* Send a <discard-changes> operation to the agent
*
* INPUTS:
*   agent_cb == agent control block to use
*
* RETURNS:
*    status
*********************************************************************/
status_t
    send_discard_changes_pdu_to_agent (agent_cb_t *agent_cb)
{
    const obj_template_t  *rpc;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata;
    ses_cb_t              *scb;
    status_t               res;
    xmlns_id_t             obj_nsid;

    req = NULL;
    reqdata = NULL;
    res = NO_ERR;

    rpc = ncx_find_object(get_netconf_mod(), 
                          NCX_EL_DISCARD_CHANGES);
    if (!rpc) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    obj_nsid = obj_get_nsid(rpc);

    /* construct a method node */
    reqdata = xml_val_new_flag(obj_get_name(rpc), 
                               obj_nsid);
    if (!reqdata) {
	log_error("\nError allocating a new RPC request");
	return ERR_INTERNAL_MEM;
    }

    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    } else {
	req = mgr_rpc_new_request(scb);
	if (!req) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nError allocating a new RPC request");
	} else {
	    req->data = reqdata;
	    req->rpc = rpc;
	    req->timeout = agent_cb->timeout;
	}
    }
	
    /* if all OK, send the RPC request */
    if (res == NO_ERR) {
	if (LOGDEBUG2) {
	    log_debug2("\nabout to send RPC request with reqdata:");
	    val_dump_value_ex(reqdata, 
                              NCX_DEF_INDENT,
                              agent_cb->display_mode);
	}

	/* the request will be stored if this returns NO_ERR */
	res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
        if (res == NO_ERR) {
            agent_cb->command_mode = CMD_MODE_AUTODISCARD;
        }
    }

    /* cleanup and set next state */
    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_discard_changes_pdu_to_agent */


/* END yangcli_autolock.c */
