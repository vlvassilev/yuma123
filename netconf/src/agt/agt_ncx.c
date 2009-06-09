/*  FILE: agt_ncx.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
04feb06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_cap
#include "agt_cap.h"
#endif

#ifndef _H_agt_cb
#include "agt_cb.h"
#endif

#ifndef _H_agt_cli
#include "agt_cli.h"
#endif

#ifndef _H_agt_ncx
#include "agt_ncx.h"
#endif

#ifndef _H_agt_rpc
#include "agt_rpc.h"
#endif

#ifndef _H_agt_rpcerr
#include "agt_rpcerr.h"
#endif

#ifndef _H_agt_ses
#include "agt_ses.h"
#endif

#ifndef _H_agt_state
#include "agt_state.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val
#include "agt_val.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
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

#ifndef _H_ses
#include "ses.h"
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

#define AGT_NCX_DEBUG 1

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean agt_ncx_init_done = FALSE;


/********************************************************************
* FUNCTION get_validate
*
* get : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_validate (ses_cb_t *scb,
		  rpc_msg_t *msg,
		  xml_node_t *methnode)
{
    cfg_template_t *source;
    val_value_t    *parm;
    status_t        res;

    /* check if the <running> config is ready to read */
    source = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!source) {
	res = ERR_NCX_OPERATION_FAILED;
    } else {
	res = cfg_ok_to_read(source);
    }
    if (res != NO_ERR) {
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res,
			 methnode, 
			 NCX_NT_NONE, 
			 NULL, 
			 NCX_NT_NONE, 
			 NULL);
	return res;
    }

    /* check the with-defaults parameter */
    parm = val_find_child(msg->rpc_input,
			  NULL, NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
	msg->mhdr.withdef = 
	    ncx_get_withdefaults_enum(VAL_ENUM_NAME(parm));
    }

    /* check if the optional filter parameter is ok */
    res = agt_validate_filter(scb, msg);
    if (res != NO_ERR) {
	return res;   /* error already recorded */
    }

    /* cache the 2 parameters and the data output callback function 
     * There is no invoke function -- it is handled automatically
     * by the agt_rpc module
     */
    msg->rpc_user1 = source;
    msg->rpc_data_type = RPC_DATA_STD;
    msg->rpc_datacb = agt_output_filter;

    return NO_ERR;

} /* get_validate */


/********************************************************************
* FUNCTION get_config_validate
*
* get-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_config_validate (ses_cb_t *scb,
			 rpc_msg_t *msg,
			 xml_node_t *methnode)
{
    cfg_template_t *source;
    val_value_t    *parm;
    status_t        res;

    /* check if the source config database exists */
    res = agt_get_cfg_from_parm(NCX_EL_SOURCE, 
				msg, 
				methnode, 
				&source);
    if (res != NO_ERR) {
	return res;  /* error already recorded */
    } 

    /* check if this is the startup config
     * filtered retrieval of this config is not supported
     */
    if (source->cfg_id == NCX_CFGID_STARTUP) {
	res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res,
			 methnode, 
			 NCX_NT_NONE, 
			 NULL, 
			 NCX_NT_NONE, 
			 NULL);
	return res;
    }

    /* check if this config can be read right now */
    res = cfg_ok_to_read(source);
    if (res != NO_ERR) {
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res,
			 methnode, 
			 NCX_NT_NONE, 
			 NULL, 
			 NCX_NT_NONE, 
			 NULL);
	return res;
    }

    /* check the with-defaults parameter */
    parm = val_find_child(msg->rpc_input,
			  NULL, NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
	msg->mhdr.withdef = 
	    ncx_get_withdefaults_enum(VAL_ENUM_NAME(parm));
    }

    /* check if the optional filter parameter is ok */
    res = agt_validate_filter(scb, msg);
    if (res != NO_ERR) {
	return res;   /* error already recorded */
    }

    /* cache the 2 parameters and the data output callback function 
     * There is no invoke function -- it is handled automatically
     * by the agt_rpc module
     */
    msg->rpc_user1 = source;
    msg->rpc_data_type = RPC_DATA_STD;
    msg->rpc_datacb = agt_output_filter;

    return NO_ERR;

} /* get_config_validate */


/********************************************************************
* FUNCTION edit_config_validate
*
* edit-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    edit_config_validate (ses_cb_t *scb,
			  rpc_msg_t *msg,
			  xml_node_t *methnode)
{
    cfg_template_t *target;
    val_value_t    *val;
    op_editop_t     defop;
    op_errop_t      errop;
    op_testop_t     testop;
    status_t        res;

    /* check if the source config database exists */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &target);
    if (res != NO_ERR) {
	return res;  /* error already recorded */
    } 

    /* get the default-operation parameter */
    val = val_find_child(msg->rpc_input, 
			 NC_MODULE,
			 NCX_EL_DEFAULT_OPERATION);
    if (!val || val->res != NO_ERR) {
	/* set to the default if any error */
	defop = OP_EDITOP_MERGE;
    } else {
	defop = op_defop_id(VAL_STR(val));
    }

    /* get the error-option parameter */
    val = val_find_child(msg->rpc_input, 
			 NC_MODULE,
			 NCX_EL_ERROR_OPTION);
    if (!val || val->res != NO_ERR) {
	/* set to the default if any error */
	errop = OP_ERROP_STOP;
    } else {
	errop = op_errop_id(VAL_STR(val));
    }

    /* the internal processing needs to know if rollback is
     * requested to optimize the undo-prep and cleanup code path
     */
    msg->rpc_err_option = errop;
    if (errop==OP_ERROP_ROLLBACK) {
	msg->rpc_need_undo = TRUE;
    }

    /* Get the test-option parameter:
     *
     * This implementation always runs the validation tests in 
     * the same order, even if the value 'set' is used.
     * The validation stage is never bypassed, even if 'set'
     * is used instead of 'test-then-set'.
     *
     * Get the value to check for the test-only extension
     */
    val = val_find_child(msg->rpc_input, 
			 NC_MODULE,
			 NCX_EL_TEST_OPTION);
    if (!val || val->res != NO_ERR) {
	/* set to the default if any error */
	testop = OP_TESTOP_SET;
    } else {
	testop = op_testop_enum(VAL_STR(val));
    }

    /* get the config parameter */
    val = val_find_child(msg->rpc_input, 
			 NC_MODULE,
			 NCX_EL_CONFIG);
    if (val && val->res == NO_ERR) {
	/* validate the <config> element (wrt/ embedded operation
	 * attributes) against the existing data model.
	 * <rpc-error> records will be added as needed 
	 */
	res = agt_val_validate_write(scb, msg, target, val, defop);

	/* for continue-on-error, ignore the validate return value
	 * in case there are multiple parmsets and not all of them
	 * had errors.  Force a NO_ERR return.
	 */
	if (!NEED_EXIT(res)) {
	    if (errop == OP_ERROP_CONTINUE) {
		res = NO_ERR;
	    }
	}

	if (target->cfg_id == NCX_CFGID_RUNNING && res==NO_ERR) {
	    res = agt_val_split_root_check(scb, 
					   msg, 
					   val, 
					   target->root, 
					   defop);
	}
    } else if (!val) {
	/* this is reported in agt_val_parse phase */
	res = ERR_NCX_DATA_MISSING;
    } else {
	res = val->res;
    }

    /* save the default operation in 'user1' */
    msg->rpc_user1 = (void *)defop;

    /* save the test option in 'user2' */
    msg->rpc_user2 = (void *)testop;

    return res;

} /* edit_config_validate */


/********************************************************************
* FUNCTION edit_config_invoke
*
* edit-config : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    edit_config_invoke (ses_cb_t *scb,
			rpc_msg_t *msg,
			xml_node_t *methnode)
{
    cfg_template_t *target;
    val_value_t    *val;
    op_editop_t     defop;
    op_testop_t     testop;
    status_t        res;

    /* get the cached options */
    testop = (op_errop_t)msg->rpc_user2;

    /* quick exit if this is a test-only request */
    if (testop == OP_TESTOP_TESTONLY) {
	return NO_ERR;
    }
    defop = (op_defop_t)msg->rpc_user1;

    /* get the config to write */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &target);
    if (res != NO_ERR) {
	return res;  /* error already recorded */
    } 

    /* get pointer to the config parameter */
    val = val_find_child(msg->rpc_input, 
			 NC_MODULE,
			 NCX_EL_CONFIG);
    if (!val || val->res != NO_ERR) {
	/* set to the default if any error */
	return SET_ERROR(ERR_NCX_OPERATION_FAILED);
    }

    /* apply the <config> into the target config */
    res = agt_val_apply_write(scb, msg, target, val, defop);
    return res;

} /* edit_config_invoke */


/********************************************************************
* FUNCTION copy_config_validate
*
* copy-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    copy_config_validate (ses_cb_t *scb,
			  rpc_msg_t *msg,
			  xml_node_t *methnode)
{
    cfg_template_t     *srccfg, *destcfg;
    const cap_list_t   *mycaps;
    val_value_t        *parm;
    status_t            res;

    destcfg = NULL;

    /* get the agent capabilities */
    mycaps = agt_cap_get_caps();
    if (!mycaps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* check if neither distinct startup or url capability supported */
    if (!cap_std_set(mycaps, CAP_STDID_STARTUP) &&
	!cap_std_set(mycaps, CAP_STDID_URL)) {
	/* operation not supported 
         * *** update in the future if copy to <running> ever supported
	 */
	res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res,
			 methnode, 
			 NCX_NT_NONE,
			 NULL,
			 NCX_NT_NONE,
			 NULL);
	return res;
    }

    /* get the config to copy from */
    res = agt_get_cfg_from_parm(NCX_EL_SOURCE, msg, methnode, &srccfg);
    if (res != NO_ERR) {
	return res;
    }

    /* get the config to copy to */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &destcfg);
    if (res == NO_ERR) {
	if (destcfg->cfg_id != NCX_CFGID_STARTUP) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	}
    } else {
	return res;
    }

    /* get the config state; check if lock can be granted
     * based on the current config state
     */
    if (res == NO_ERR) {
	res = cfg_ok_to_write(destcfg, SES_MY_SID(scb));
    }

    if (res == NO_ERR) {
	parm = val_find_child(msg->rpc_input,
			      NULL,
			      NCX_EL_WITH_DEFAULTS);
	if (parm && parm->res == NO_ERR) {
	    msg->mhdr.withdef = VAL_BOOL(parm);
	}
    }

    if (res != NO_ERR) {
	/* cannot write to this configuration datastore */
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res,
			 methnode, 
			 NCX_NT_CFG, 
			 (const void *)destcfg, 
			 NCX_NT_NONE, 
			 NULL);
    } else {
	/* save the source and destination config */
	msg->rpc_user1 = srccfg;
	msg->rpc_user2 = destcfg;
    }

    return res;

} /* copy_config_validate */


/********************************************************************
* FUNCTION copy_config_invoke
*
* copy-config : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    copy_config_invoke (ses_cb_t *scb,
			rpc_msg_t *msg,
			xml_node_t *methnode)
{
    cfg_template_t *source, *target;
    status_t        res;

    res = NO_ERR;
    source = (cfg_template_t *)msg->rpc_user1;
    target = (cfg_template_t *)msg->rpc_user2;

    /* check write running to startup */
    if (source->cfg_id == NCX_CFGID_RUNNING &&
	target->cfg_id == NCX_CFGID_STARTUP) {
	res = agt_ncx_cfg_save(source, FALSE);
	if (res != NO_ERR) {
	    /* config save failed */
	    agt_record_error(scb, 
			     &msg->mhdr, 
			     NCX_LAYER_OPERATION,
			     res, 
			     methnode,
			     NCX_NT_CFG, 
			     target, 
			     NCX_NT_NONE, 
			     NULL);
	}
    }

    /* all other copy operations are optional
     * and are not supported at this time
     */
    return res;

} /* copy_config_invoke */


/********************************************************************
* FUNCTION delete_config_validate
*
* delete-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    delete_config_validate (ses_cb_t *scb,
			    rpc_msg_t *msg,
			    xml_node_t *methnode)
{
    const agt_profile_t  *prof;
    cfg_template_t       *target;
    status_t              res;
    const void           *errval;
    ncx_node_t            errtyp;

#ifdef NOT_YET
    cap_list_t      *mycaps;
#endif

    /* get the config to delete */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &target);
    if (res != NO_ERR) {
	return res;  /* error already recorded */
    } 

    /* get the agent profile */
    prof = agt_get_profile();
    if (!prof) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* check the value provided -- only <startup> supported !!! */
    if (res==NO_ERR) {
	/* check if the startup config is allowed to be deleted
	 * and that is the config to be deleted
	 */
	if (!prof->agt_del_startup) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	} else if (xml_strcmp(target->name, NCX_EL_STARTUP)) {
	    res = ERR_NCX_WRONG_VAL;
	}
    }

    /* check if okay to delete this config now */
    if (res == NO_ERR) {
	res = cfg_ok_to_write(target, SES_MY_SID(scb));
    }

#ifdef NOT_YET
    if (res == NO_ERR) {
	/* get the agent capabilities */    
	mycaps = agt_cap_get_caps();
	if (!mycaps) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}
    }

    /* check if the url capability is supported 
     * it is not mandatory to support deletion of any config db 
     */
    if (res == NO_ERR) {
	if (!cap_std_set(mycaps, CAP_STDID_URL)) {
	    /* none of the hardwired configs can be deleted */
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	} else {
	    /*** TBD: add URL check ***/
	    res = NCX_ERR_OPERATION_NOT_SUPPORTED;  
	}
    }
#endif

    if (res != NO_ERR) {
	errval = agt_get_parmval(NCX_EL_TARGET, msg);
	if (errval) {
	    errtyp = NCX_NT_VAL;
	} else {
	    errval = NCX_EL_TARGET;
	    errtyp = NCX_NT_STRING;
	}
	agt_record_error(scb, 
			 &msg->mhdr,
			 NCX_LAYER_OPERATION, 
			 res, 
			 methnode, 
			 errtyp, 
			 errval,
			 NCX_NT_STRING, 
			 "/rpc/delete-config/target");
    }
    return res;

} /* delete_config_validate */


/********************************************************************
* FUNCTION delete_config_invoke
*
* delete-config : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    delete_config_invoke (ses_cb_t *scb,
			  rpc_msg_t *msg,
			  xml_node_t *methnode)
{
    (void)scb;
    (void)msg;
    (void)methnode;

    /*** NEED TO ADD SUPPORT FOR DELETE startup ***/
    return NO_ERR;

} /* delete_config_invoke */


/********************************************************************
* FUNCTION lock_validate
*
* lock : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    lock_validate (ses_cb_t *scb,
		   rpc_msg_t *msg,
		   xml_node_t *methnode)
{

    status_t         res;
    cfg_template_t  *cfg;

    /* get the config to lock */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &cfg);
    if (res != NO_ERR) {
	return res;
    }

    /* get the config state; check if lock can be granted
     * based on the current config state
     */
    res = cfg_ok_to_lock(cfg);
    if (res == NO_ERR) {
	/* lock can be granted
	 * setup the user1 scratchpad with the cfg to lock 
	 */
	msg->rpc_user1 = (void *)cfg;
    } else {
	/* lock probably already held */
	agt_record_error(scb,
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION,
			 res,
			 methnode,
			 NCX_NT_CFG, 
			 (const void *)cfg,
			 NCX_NT_NONE,
			 NULL);
    }
    return res;

} /* lock_validate */


/********************************************************************
* FUNCTION lock_invoke
*
* lock : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    lock_invoke (ses_cb_t *scb,
		 rpc_msg_t *msg,
		 xml_node_t *methnode)
{
    cfg_template_t   *cfg;
    status_t          res;

    cfg = (cfg_template_t *)msg->rpc_user1;
    res = cfg_lock(cfg, SES_MY_SID(scb), CFG_SRC_NETCONF);
    if (res != NO_ERR) {
	/* config is in a state where locks cannot be granted */
	agt_record_error(scb,
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION,
			 res, 
			 methnode,
			 NCX_NT_NONE, 
			 NULL,
			 NCX_NT_NONE, 
			 NULL);
    }

    return res;

} /* lock_invoke */


/********************************************************************
* FUNCTION unlock_validate
*
* unlock : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    unlock_validate (ses_cb_t *scb,
		     rpc_msg_t *msg,
		     xml_node_t *methnode)
{
    status_t         res;
    cfg_template_t  *cfg;

    /* get the config to lock */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &cfg);
    if (res != NO_ERR) {
	return res;
    }

    /* get the config state; check if lock is already granted
     * based on the current config state
     */
    res = cfg_ok_to_unlock(cfg, SES_MY_SID(scb));
    if (res == NO_ERR) {
	msg->rpc_user1 = (void *)cfg;
    } else {
	agt_record_error(scb,
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION,
			 res, 
			 methnode,
			 NCX_NT_NONE, 
			 NULL, 
			 NCX_NT_NONE,
			 NULL);
    }

    return res;

} /* unlock_validate */


/********************************************************************
* FUNCTION unlock_invoke
*
* unlock : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    unlock_invoke (ses_cb_t *scb,
		   rpc_msg_t *msg,
		   xml_node_t *methnode)
{
    cfg_template_t  *cfg;
    status_t         res;

    cfg = (cfg_template_t *)msg->rpc_user1;
    res = cfg_unlock(cfg, SES_MY_SID(scb));
    if (res != NO_ERR) {
	agt_record_error(scb,
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION,
			 res, 
			 methnode,
			 NCX_NT_NONE, 
			 NULL, 
			 NCX_NT_NONE,
			 NULL);
    }
    return res;

} /* unlock_invoke */


/********************************************************************
* FUNCTION close_session_invoke
*
* close-session : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    close_session_invoke (ses_cb_t *scb,
			  rpc_msg_t *msg,
			  xml_node_t *methnode)
{

    (void)msg;
    (void)methnode;
    agt_ses_request_close(SES_MY_SID(scb), 
			  SES_MY_SID(scb),
			  SES_TR_CLOSED);
    return NO_ERR;

} /* close_session_invoke */


/********************************************************************
* FUNCTION kill_session_validate
*
* kill-session : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    kill_session_validate (ses_cb_t *scb,
			   rpc_msg_t *msg,
			   xml_node_t *methnode)
{
    status_t         res;
    val_value_t     *val;

    res = NO_ERR;

    /* get the session-id parameter */
    val = val_find_child(msg->rpc_input, 
			 NC_MODULE,
			 NCX_EL_SESSION_ID);
    if (!val || val->res != NO_ERR) {
	/* error already recorded in parse phase */
	if (val) {
	    return val->res;
	} else {
	    return ERR_NCX_OPERATION_FAILED;
	}
    }

    /* make sure the session-id is valid 
     * The RFC forces a kill-session of the current
     * session to be an error, even though agt_ses.c
     * supports this corner-case
     */
    if (VAL_UINT(val) == scb->sid
	|| !agt_ses_session_id_valid(VAL_UINT(val))) {
	res = ERR_NCX_INVALID_VALUE;
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
	/* save the session-id to kill */
	msg->rpc_user1 = (void *)VAL_UINT(val);
    }

    return res;

} /* kill_session_validate */


/********************************************************************
* FUNCTION kill_session_invoke
*
* kill-session : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    kill_session_invoke (ses_cb_t *scb,
			 rpc_msg_t *msg,
			 xml_node_t *methnode)
{
    ses_id_t  sid;

    (void)methnode;

    sid = (ses_id_t)msg->rpc_user1;
    if (sid==scb->sid) {
	/* zapping the current session */
	agt_ses_request_close(sid, 
			      scb->sid,
			      SES_TR_KILLED);
    } else {
	/* zapping another session */
	agt_ses_kill_session(sid, 
			     scb->sid,
			     SES_TR_KILLED);
    }
    return NO_ERR;

} /* kill_session_invoke */


/********************************************************************
* FUNCTION validate_validate
*
* validate : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    validate_validate (ses_cb_t *scb,
		       rpc_msg_t *msg,
		       xml_node_t *methnode)
{
    cfg_template_t       *target;
    val_value_t          *val, *child, *rootval;
    const xmlChar        *errstr;
    const agt_profile_t  *profile;
    status_t              res;

    target = NULL;
    res = NO_ERR;
    rootval = NULL;
    errstr = NULL;
    child = NULL;

    /* get the source parameter */
    val = val_find_child(msg->rpc_input, NC_MODULE,
			 NCX_EL_SOURCE);
    if (!val || val->res != NO_ERR) {
	if (val) {
	    res = val->res;
	} else {
	    res = ERR_NCX_OPERATION_FAILED;
	}
	errstr = NCX_EL_SOURCE;
    }

    /* determine which variant of the input parameter is present */
    if (res == NO_ERR) {
	child = val_get_first_child(val);
	if (!child || child->res != NO_ERR) {
	    if (child) {
		res = child->res;
		errstr = child->name;
	    } else {
		res = ERR_NCX_OPERATION_FAILED;
		errstr = NCX_EL_SOURCE;
	    }
	}
    }

    if (res == NO_ERR) {
	if (!xml_strcmp(child->name, NCX_EL_RUNNING)) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    errstr = child->name;
	} else if (!xml_strcmp(child->name, NCX_EL_CANDIDATE)) {
	    profile = agt_get_profile();
	    if (profile->agt_targ != NCX_AGT_TARG_CANDIDATE) {
		res = ERR_NCX_OPERATION_NOT_SUPPORTED;
		errstr = child->name;
	    } else {
		target = cfg_get_config_id(NCX_CFGID_CANDIDATE);
		if (target) {
		    rootval = target->root;
		    if (!rootval) {
			res = ERR_NCX_OPERATION_FAILED;
			errstr = child->name;
		    }
		} else {
		    res = ERR_NCX_OPERATION_FAILED;
		    errstr = child->name;
		}
	    }
	} else if (!xml_strcmp(child->name, NCX_EL_STARTUP)) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    errstr = child->name;
	} else if (!xml_strcmp(child->name, NCX_EL_URL)) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    errstr = child->name;
	} else if (!xml_strcmp(child->name, NCX_EL_CONFIG)) {
	    rootval = child;
	}
    }

    if (res != NO_ERR) {
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res, 
			 methnode,
			 (errstr) ? NCX_NT_STRING : NCX_NT_NONE,
			 (errstr) ? errstr : NULL,
			 (rootval) ? NCX_NT_VAL : NCX_NT_NONE, 
			 (rootval) ? rootval : NULL);
	return res;
    }


    /* set the error parameter to gather the most errors */
    msg->rpc_err_option = OP_ERROP_CONTINUE;

    /* validate the <config> element (wrt/ embedded operation
     * attributes) against the existing data model.
     * <rpc-error> records will be added as needed 
     */
    res = agt_val_validate_write(scb, 
				 msg, 
				 NULL, 
				 rootval, 
				 OP_EDITOP_MERGE);

    if (res == NO_ERR) {
	res = agt_val_root_check(scb, &msg->mhdr, rootval);
    }

    return res;

} /* validate_validate */


/********************************************************************
* FUNCTION commit_validate
*
* commit : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    commit_validate (ses_cb_t *scb,
		     rpc_msg_t *msg,
		     xml_node_t *methnode)
{
    cfg_template_t       *candidate, *running;
    const agt_profile_t  *profile;
    status_t              res;
    boolean               errdone;


    res = NO_ERR;
    errdone = FALSE;

    profile = agt_get_profile();

    if (profile->agt_targ != NCX_AGT_TARG_CANDIDATE) {
	res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    } else {
	/* get the candidate config */
	candidate = cfg_get_config_id(NCX_CFGID_CANDIDATE);
	running = cfg_get_config_id(NCX_CFGID_RUNNING);
	if (!candidate || !running) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	} else {
	    /* check if this session is allowed to clear the
	     * candidate config now
	     */
	    res = cfg_ok_to_write(candidate, SES_MY_SID(scb));

	    if (res == NO_ERR) {
		/* check if the running config can be written */
		res = cfg_ok_to_write(running, SES_MY_SID(scb));
	    }

	    if (res == NO_ERR) {
		res = agt_val_root_check(scb, 
					 &msg->mhdr, 
					 candidate->root);
		if (res != NO_ERR) {
		    errdone = TRUE;
		}
	    }
	}
    }

    if (res != NO_ERR && !errdone) {
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res, 
			 methnode,
			 NCX_NT_NONE, 
			 NULL, 
			 NCX_NT_NONE, 
			 NULL);
    }

    return res;

} /* commit_validate */


/********************************************************************
* FUNCTION commit_invoke
*
* commit : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    commit_invoke (ses_cb_t *scb,
		   rpc_msg_t *msg,
		   xml_node_t *methnode)
{
    cfg_template_t *candidate, *running;
    status_t        res;

    res = NO_ERR;

    candidate = cfg_get_config_id(NCX_CFGID_CANDIDATE);
    running = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!candidate || !running) {
	res = SET_ERROR(ERR_INTERNAL_VAL);
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
	res = agt_val_apply_commit(scb, msg, candidate, running);
	if (res == NO_ERR) {
	    res = cfg_fill_candidate_from_running();
	    if (res != NO_ERR) {
		agt_record_error(scb,
				 &msg->mhdr, 
				 NCX_LAYER_OPERATION,
				 res,
				 methnode,
				 NCX_NT_NONE, 
				 NULL, 
				 NCX_NT_VAL,
				 candidate->root);
	    }
	}  /* else errors already recorded */
    }

    return res;

} /* commit_invoke */


/********************************************************************
* FUNCTION discard_changes_validate
*
* discard-changes : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    discard_changes_validate (ses_cb_t *scb,
			      rpc_msg_t *msg,
			      xml_node_t *methnode)
{
    cfg_template_t       *candidate;
    const agt_profile_t  *profile;
    status_t              res;

    res = NO_ERR;
    profile = agt_get_profile();

    if (profile->agt_targ != NCX_AGT_TARG_CANDIDATE) {
	res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    } else {
	/* get the candidate config */
	candidate = cfg_get_config_id(NCX_CFGID_CANDIDATE);
	if (!candidate) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	} else {
	    /* check if this session is allowed to invoke now */
	    res = cfg_ok_to_write(candidate, SES_MY_SID(scb));
	}
    }

    if (res != NO_ERR) {
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res, 
			 methnode,
			 NCX_NT_NONE, 
			 NULL, 
			 NCX_NT_NONE, 
			 NULL);
	return res;
    }

    return res;

} /* discard_changes_validate */


/********************************************************************
* FUNCTION discard_changes_invoke
*
* discard-changes : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    discard_changes_invoke (ses_cb_t *scb,
			   rpc_msg_t *msg,
			   xml_node_t *methnode)
{
    cfg_template_t *candidate;
    status_t        res;

    res = NO_ERR;


    /* get the candidate config */
    candidate = cfg_get_config_id(NCX_CFGID_CANDIDATE);
    if (!candidate) {
	res = SET_ERROR(ERR_INTERNAL_VAL);
    } else if (cfg_get_dirty_flag(candidate)) {
	res = cfg_fill_candidate_from_running();
    }

    if (res != NO_ERR) {
	agt_record_error(scb, 
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION, 
			 res, 
			 methnode,
			 NCX_NT_NONE, 
			 NULL, 
			 NCX_NT_NONE, 
			 NULL);
    }

    return res;

} /* discard_changes_invoke */


/********************************************************************
* FUNCTION load_config_validate
*
* load-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    load_config_validate (ses_cb_t *scb,
			  rpc_msg_t *msg,
			  xml_node_t *methnode)
{
    cfg_template_t  *target;
    val_value_t     *val;
    status_t         res;

    (void)methnode;

    /* This special callback is used by internal NCX functions
     * to load the initial configuration.  The msg->rpc_user1 parameter 
     * has already been set to the address of the cfg_template_t
     * to fill in.
     *
     * NOTE: HACK DEPENDS ON THE agt_rpc_load_config_file to setup
     * the rpc->rpc_user1 parameter
     */
    target = (cfg_template_t *)msg->rpc_user1;
    if (!target) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* get the config parameter */
    val = val_find_child(msg->rpc_input, NULL, NCX_EL_CONFIG);
    if (!val) {
	/* we shouldn't get here if the config param is missing */
	return SET_ERROR(ERR_NCX_OPERATION_FAILED);
    }

    /* Startup config load mode is always continue-on-error */
    msg->rpc_err_option = OP_ERROP_CONTINUE;

    /* errors will be added as needed */
    res = agt_val_validate_write(scb, 
				 msg, 
				 target, 
				 val, 
				 OP_EDITOP_LOAD);

    if (res == NO_ERR) {
	res = agt_val_root_check(scb, &msg->mhdr, val);
    }
    msg->rpc_user2 = val;

    return res;

} /* load_config_validate */


/********************************************************************
* FUNCTION load_config_invoke
*
* load-config : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    load_config_invoke (ses_cb_t *scb,
			rpc_msg_t *msg,
			xml_node_t *methnode)
{
    cfg_template_t  *target;
    val_value_t     *val;
    status_t         res;

    (void)methnode;

    /* This special callback is used by internal NCX functions
     * to load the initial configuration. 
     */

    res = NO_ERR;
    target = (cfg_template_t *)msg->rpc_user1;
    val = (val_value_t *)msg->rpc_user2;

    if (!target || !val) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* load the <config> into the target config */
    if (res == NO_ERR) {
	res = agt_val_apply_write(scb, 
				  msg, 
				  target, 
				  val, 
				  OP_EDITOP_LOAD);
    }

    val_clean_tree(target->root);

    return res;

} /* load_config_invoke */


/********************************************************************
* FUNCTION load_invoke
*
* load module : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    load_invoke (ses_cb_t *scb,
		 rpc_msg_t *msg,
		 xml_node_t *methnode)
{
    val_value_t           *val, *newval;
    ncx_module_t          *mod;
    status_t               res;

    res = NO_ERR;
    newval = NULL;

    val = val_find_child(msg->rpc_input, 
			 NCXMOD_NETCONFD,
			 NCX_EL_MODULE);
    if (!val || val->res != NO_ERR) {
	return ERR_NCX_OPERATION_FAILED;
    }

    /**** TBD: revision parameter support ****/
    mod = ncx_find_module(VAL_STR(val), NULL);
    if (!mod) {
	res = ncxmod_load_module(VAL_STR(val), NULL, NULL);
	if (res != NO_ERR) {
	    agt_record_error(scb, 
			     &msg->mhdr, 
			     NCX_LAYER_OPERATION, 
			     res,
			     methnode, 
			     NCX_NT_NONE, 
			     NULL, 
			     NCX_NT_VAL, 
			     val);
	    return res;
	} else {
	    mod = ncx_find_module(VAL_STR(val), NULL);
	    if (!mod) {
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
    }

    /* generate the return value */
    if (mod->version) {
	newval = val_make_string(val->nsid,
				 NCX_EL_MOD_REVISION,
				 mod->version);				 
	if (!newval) {
	    res = ERR_INTERNAL_MEM;
	}
    }

    if (res == NO_ERR) {
	res = agt_state_add_module_schema(mod);
    }

    if (res != NO_ERR) {
	if (newval) {
	    val_free_value(newval);
	}
	agt_record_error(scb,
			 &msg->mhdr, 
			 NCX_LAYER_OPERATION,
			 res,
			 methnode,
			 NCX_NT_NONE,
			 NULL, 
			 NCX_NT_VAL,
			 val);
    } else {
	msg->rpc_data_type = RPC_DATA_YANG;
	dlq_enque(newval, &msg->rpc_dataQ);
    }

    return res;

} /* load_invoke */


/********************************************************************
* FUNCTION restart_invoke
*
* restart : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    restart_invoke (ses_cb_t *scb,
		    rpc_msg_t *msg,
		    xml_node_t *methnode)
{
    xmlChar   timebuff[TSTAMP_MIN_SIZE];

    (void)msg;
    (void)methnode;

    tstamp_datetime(timebuff);

    log_write("\n\n**************"
	      "\nNotice: restart requested\n   by %s "
	      "on session %u at %s\n\n",
	      scb->username,
	      scb->sid,
	      timebuff);

    agt_request_shutdown(NCX_SHUT_RESTART);
    return NO_ERR;

} /* restart_invoke */


/********************************************************************
* FUNCTION shutdown_invoke
*
* shutdown : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    shutdown_invoke (ses_cb_t *scb,
		     rpc_msg_t *msg,
		     xml_node_t *methnode)
{
    xmlChar   timebuff[TSTAMP_MIN_SIZE];

    (void)msg;
    (void)methnode;

    tstamp_datetime(timebuff);

    log_write("\n\n*****************************"
	      "\nNotice: shutdown requested\n    by %s "
	      "on session %u at %s\n\n",
	      scb->username,
	      scb->sid,
	      timebuff);

    agt_request_shutdown(NCX_SHUT_EXIT);
    return NO_ERR;

} /* shutdown_invoke */


/********************************************************************
* FUNCTION register_nc_callbacks
*
* Register the agent callback functions for the NETCONF RPC methods 
*
* RETURNS:
*    status, NO_ERR if all registered okay
*********************************************************************/
static status_t 
    register_nc_callbacks (void)
{
    status_t  res;

    /* get */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_GET),
				  AGT_RPC_PH_VALIDATE,
				  get_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }


    /* get-config */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_GET_CONFIG),
				  AGT_RPC_PH_VALIDATE,
				  get_config_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* edit-config */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_EDIT_CONFIG),
				  AGT_RPC_PH_VALIDATE,
				  edit_config_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_EDIT_CONFIG),
				  AGT_RPC_PH_INVOKE,
				  edit_config_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }


    /* copy-config */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_COPY_CONFIG),
				  AGT_RPC_PH_VALIDATE,
				  copy_config_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_COPY_CONFIG),
				  AGT_RPC_PH_INVOKE,
				  copy_config_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* delete-config */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_DELETE_CONFIG),
				  AGT_RPC_PH_VALIDATE,
				  delete_config_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_DELETE_CONFIG),
				  AGT_RPC_PH_INVOKE,
				  delete_config_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* lock */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_LOCK),
				  AGT_RPC_PH_VALIDATE,
				  lock_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_LOCK),
				  AGT_RPC_PH_INVOKE,
				  lock_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }


    /* unlock */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_UNLOCK),
				  AGT_RPC_PH_VALIDATE,
				  unlock_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_UNLOCK),
				  AGT_RPC_PH_INVOKE,
				  unlock_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* close-session
     * no validate for close-session 
     */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_CLOSE_SESSION),
				  AGT_RPC_PH_INVOKE,
				  close_session_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* kill-session */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_KILL_SESSION),
				  AGT_RPC_PH_VALIDATE,
				  kill_session_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_KILL_SESSION),
				  AGT_RPC_PH_INVOKE,
				  kill_session_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* validate :validate capability */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_VALIDATE),
				  AGT_RPC_PH_VALIDATE,
				  validate_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* commit :candidate capability */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_COMMIT),
				  AGT_RPC_PH_VALIDATE,
				  commit_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_COMMIT),
				  AGT_RPC_PH_INVOKE,
				  commit_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* discard-changes :candidate capability */
    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_DISCARD_CHANGES),
				  AGT_RPC_PH_VALIDATE,
				  discard_changes_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
				  op_method_name(OP_DISCARD_CHANGES),
				  AGT_RPC_PH_INVOKE,
				  discard_changes_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* load-config extension */
    res = agt_rpc_register_method(AGT_CLI_MODULE, 
				  NCX_EL_LOAD_CONFIG,
				  AGT_RPC_PH_VALIDATE, 
				  load_config_validate);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    res = agt_rpc_register_method(AGT_CLI_MODULE, 
				  NCX_EL_LOAD_CONFIG,
				  AGT_RPC_PH_INVOKE,
				  load_config_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* load module extension */
    res = agt_rpc_register_method(AGT_CLI_MODULE, 
				  NCX_EL_LOAD,
				  AGT_RPC_PH_INVOKE,  
				  load_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* restart extension */
    res = agt_rpc_register_method(AGT_CLI_MODULE, 
				  NCX_EL_RESTART,
				  AGT_RPC_PH_INVOKE,  
				  restart_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }


    /* shutdown extension */
    res = agt_rpc_register_method(AGT_CLI_MODULE, 
				  NCX_EL_SHUTDOWN,
				  AGT_RPC_PH_INVOKE,  
				  shutdown_invoke);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }


    return NO_ERR;

} /* register_nc_callbacks */


/********************************************************************
* FUNCTION unregister_nc_callbacks
*
* Unregister the agent callback functions for the NETCONF RPC methods 
*
*********************************************************************/
static void
    unregister_nc_callbacks (void)
{
    /* get */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_GET));

    /* get-config */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_GET_CONFIG));

    /* edit-config */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_EDIT_CONFIG));

    /* copy-config */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_COPY_CONFIG));

    /* delete-config */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_DELETE_CONFIG));

    /* lock */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_LOCK));

    /* unlock */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_UNLOCK));

    /* close-session */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_CLOSE_SESSION));

    /* kill-session */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_KILL_SESSION));

    /* validate */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_VALIDATE));

    /* commit */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_COMMIT));

    /* discard-changes */
    agt_rpc_unregister_method(NC_MODULE, 
			      op_method_name(OP_DISCARD_CHANGES));

    /* load-config extension */
    agt_rpc_unregister_method(AGT_CLI_MODULE, NCX_EL_LOAD_CONFIG);

    /* load module extension */
    agt_rpc_unregister_method(AGT_CLI_MODULE, NCX_EL_LOAD);

    /* restart extension */
    agt_rpc_unregister_method(AGT_CLI_MODULE, NCX_EL_RESTART);

    /* shutdown extension */
    agt_rpc_unregister_method(AGT_CLI_MODULE, NCX_EL_SHUTDOWN);

} /* unregister_nc_callbacks */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_ncx_init
* 
* Initialize the NCX Agent standard method routines
* 
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    agt_ncx_init (void)
{
    status_t  res;

    if (!agt_ncx_init_done) {

	res = register_nc_callbacks();
	if (res != NO_ERR) {
	    unregister_nc_callbacks();
	    return res;
	}

        agt_ncx_init_done = TRUE;

    }
    return NO_ERR;

}  /* agt_ncx_init */


/********************************************************************
* FUNCTION agt_ncx_cleanup
*
* Cleanup the NCX Agent standard method routines
* 
* TBD -- put platform-specific agent cleanup here
*
*********************************************************************/
void
    agt_ncx_cleanup (void)
{
    if (agt_ncx_init_done) {

	unregister_nc_callbacks();
	agt_ncx_init_done = FALSE;
    }
}   /* agt_ncx_cleanup */


/********************************************************************
* FUNCTION agt_ncx_cfg_load
*
* Load the specifed config from the indicated source
*
* This function should only be used to load an empty config
* in CFG_ST_INIT state
*
* INPUTS:
*    cfg = Config template to load data into
*    cfgloc == enum for the config source location
*    cfgparm == string parameter used in different ways 
*               depending on the cfgloc value
*     For cfgloc==CFG_LOC_FILE, this is a system-dependent filespec
* 
* OUTPUTS:
*    errQ contains any rpc_err_rec_t structs (if non-NULL)
*
* RETURNS:
*    overall status; may be the last of multiple error conditions
*********************************************************************/
status_t
    agt_ncx_cfg_load (cfg_template_t *cfg,
		      cfg_location_t cfgloc,
		      const xmlChar *cfgparm)
{
    status_t  res;

#ifdef DEBUG
    if (!cfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (cfg->cfg_state != CFG_ST_INIT) {
	return SET_ERROR(ERR_NCX_CFG_STATE);
    }
#endif

    cfg->cfg_loc = cfgloc;
    if (cfgparm) {
	cfg->src_url = xml_strdup(cfgparm);
	if (!cfg->src_url) {
	    return ERR_INTERNAL_MEM;
	}
    }

    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    switch (cfgloc) {
    case CFG_LOC_INTERNAL:
	break;
    case CFG_LOC_FILE:
	if (!cfg->src_url) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    /* the cfgparm should be a filespec of an XML config file */
	    res = agt_rpc_load_config_file(cfgparm, cfg);
	}
	break;
    case CFG_LOC_NAMED:
	break;
    case CFG_LOC_LOCAL_URL:
	break;
    case CFG_LOC_REMOTE_URL:
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

} /* agt_ncx_cfg_load */


/********************************************************************
* FUNCTION agt_ncx_cfg_save
*
* Save the specified cfg to the its startup source, which should
* be stored in the cfg struct
*
* INPUTS:
*    cfg  = Config template to save from
*    bkup = TRUE if the current startup config should
*           be saved before it is overwritten
*         = FALSE to just overwrite the old startup cfg
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_ncx_cfg_save (cfg_template_t *cfg,
		      boolean bkup)
{
    status_t     res;
    xml_attrs_t  attrs;

#ifdef DEBUG
    if (!cfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!cfg->root) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif
	
    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    switch (cfg->cfg_loc) {
    case CFG_LOC_INTERNAL:
	break;
    case CFG_LOC_FILE:
	if (bkup) {
	    /* remove any existing backup */
	    /****/

	    /* rename the current startup to the backup */
	    /****/
	} 

	/* write the new startup config */
	xml_init_attrs(&attrs);

	/* output to the specified file or STDOUT */
	res = xml_wr_check_file(cfg->src_url, cfg->root, &attrs, 
				XMLMODE, WITHHDR, NCX_DEF_INDENT,
				agt_check_save);

	xml_clean_attrs(&attrs);
	break;
    case CFG_LOC_NAMED:

	break;
    case CFG_LOC_LOCAL_URL:

	break;
    case CFG_LOC_REMOTE_URL:

	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

} /* agt_ncx_cfg_save */


/* END file agt_ncx.c */
