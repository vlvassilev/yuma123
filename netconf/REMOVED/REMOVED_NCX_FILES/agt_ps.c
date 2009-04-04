/*  FILE: agt_ps.c

   Manage Agent callbacks for parmset manipulation

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
28apr06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include <memory.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_ps
#include "agt_ps.h"
#endif

#ifndef _H_agt_rpc
#include "agt_rpc.h"
#endif

#ifndef _H_agt_rpcerr
#include "agt_rpcerr.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val
#include "agt_val.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_getcb
#include "getcb.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_ps_parse
#include "ps_parse.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
********************************************************************/

#define AGT_PS_DEBUG 1


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION create_parmset
* 
* Create a data parmset for the specified PSD
* Choices are skipped and their parms must be added manually!!!
*
* INPUTS:
*   psd == parmset template to use
*   cfg == address of output cfg pointer
*   res == address of output status value
*
* OUTPUTS:
*   *cfg == <running> config 
*   *res == status result, should be NO_ERR if return value non-NULL
*   parmset is added to <running> config and registered in the 
*   def-reg if NO_ERR
*
* RETURNS:
*   pointer to the new parmset that was just created, or NULL if errors
*********************************************************************/
static ps_parmset_t *
    create_parmset (const psd_template_t *psd,
		    cfg_template_t **cfg,
		    status_t *res)
{
    ps_parmset_t   *ps;

    *res = NO_ERR;

    /* create a data parmset instance for this PSD */
    ps = ps_new_parmset();
    if (!ps) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    ps_setup_parmset(ps, psd, PSD_TYP_DATA);

    ps->name = xml_strdup(psd->name);
    if (!ps->name) {
	*res = ERR_INTERNAL_MEM;
    }

    /* get the <running> config */
    if (*res == NO_ERR) {
	*cfg = cfg_get_config_id(NCX_CFGID_RUNNING);
	if (!*cfg) {
	    *res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* set the parmset instance ID */
    if (*res == NO_ERR) {
	*res = ps_gen_parmset_instance_id(ps);
    }

    /* make sure the parmset is not already loaded */
    if (*res == NO_ERR && cfg_get_parmset(*cfg, ps)) {
	*res = ERR_NCX_DATA_EXISTS;
    }

    if (*res != NO_ERR) {
	ps_free_parmset(ps);
	ps = NULL;
    } else {
	*res = NO_ERR;
    }

    return ps;

} /* create_parmset */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_ps_validate_write
* 
* Validate the requested <edit-config> write operation
*
* Check all the embedded operation attributes against
* the default-operation and maintained current operation.
*
* Invoke all the user AGT_CB_VALIDATE callbacks for a 
* 'new value' and 'existing value' pairs, for a given write operation, 
*
* These callbacks are invoked bottom-up, so the first step is to
* step through all the application parmsets and traverse the
* 'new' data model (from the PDU) all the way to the leaf nodes
*
* The operation attribute is checked against the real data model
* on the way down the tree, and the user callbacks are invoked
* bottom-up on the way back.  This way, the user callbacks can
* share sub-tree validation routines, and perhaps add additional
* <rpc-error> information, based on the context and specific errors
* reported from 'below'.
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   valroot == the val_value_t struct containing the NCX_BT_ROOT
*              datatype representing the config root with
*              proposed changes to the target
*   editop == requested start-state write operation
*             (usually from the default-operation parameter)
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added to the msg->rpc_errQ
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t
    agt_ps_validate_write (ses_cb_t  *scb,
			   rpc_msg_t  *msg,
			   cfg_template_t *target,
			   val_value_t  *valroot,
			   op_editop_t  editop)
{
    status_t        res;

#ifdef DEBUG
    if (!scb || !msg || !target || !valroot) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (valroot->btyp != NCX_BT_ROOT) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }	
#endif

    /* check the lock first */
    res = cfg_ok_to_write(target, scb->sid);
    if (res != NO_ERR) {
	agt_record_error(scb, &msg->mhdr.errQ, NCX_LAYER_CONTENT, res, NULL,
		 NCX_NT_NONE, NULL, NCX_NT_VAL, valroot);
	return res;
    }

    /* the <config> root is just a value node of type 'root'
     * traverse all nodes and check the <edit-config> request
     */
    res = agt_val_handle_callback(AGT_CB_VALIDATE, editop, scb, 
				  msg, target, valroot, target->root);

    return res;

}  /* agt_ps_validate_write */


/********************************************************************
* FUNCTION agt_ps_apply_write
* 
* Apply the requested write operation
*
* Invoke all the AGT_CB_APPLY callbacks for a 
* source and target and write operation
*
* TBD: support for handling nested parmsets independently
*      of the parent parmset is not supported.  This means
*      that independent parmset instances nested within the
*      parent parmset all have to be applied, or none applied
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   pducfg == the 'root' value struct that represents the
*             tree of changes to apply to the target
*   editop == requested start-state write operation
*             (usually from the default-operation parameter)
*   errop == requested error handling
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added to the msg->mhsr.errQ
*
* RETURNS:
*   none
*********************************************************************/
status_t
    agt_ps_apply_write (ses_cb_t  *scb,
			rpc_msg_t  *msg,
			cfg_template_t *target,
			val_value_t    *pducfg,
			op_editop_t  editop,
			op_errop_t  errop)
{
    status_t        res;

#ifdef DEBUG
    if (!scb || !msg || !target || !pducfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (pducfg->btyp != NCX_BT_ROOT) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }	
#endif

    /* check the lock first */
    res = cfg_ok_to_write(target, scb->sid);
    if (res != NO_ERR) {
	agt_record_error(scb, &msg->mhdr.errQ, NCX_LAYER_CONTENT, 
			 res, NULL,
			 NCX_NT_NONE, NULL, NCX_NT_VAL, pducfg);
	return res;
    }

    /* make sure the target root is not NULL */
    if (!target->root) {
	target->root = val_new_value();
	if (!target->root) {
	    return SET_ERROR(ERR_INTERNAL_MEM);
	}
	/* init with an empty queue of applications */
	val_init_complex(target->root, NCX_BT_ROOT);
	target->root->dataclass = NCX_DC_CONFIG;
    }

    /* start with the config root, which is a val_value_t node */
    res = agt_val_handle_callback(AGT_CB_APPLY, editop, scb, 
				  msg, target, pducfg, target->root);

    /* check if undo was in effect */
    if (msg->rpc_need_undo) {
	if (res==NO_ERR) {
	    /* complete the operation */
	    res = agt_val_handle_callback(AGT_CB_COMMIT, editop, scb, 
					  msg, target, pducfg, target->root);
	} else {
	    /* rollback the operation */
	    res = agt_val_handle_callback(AGT_CB_ROLLBACK, editop, scb, 
					  msg, target, pducfg, target->root);
	}
    }  /* else there was no rollback, so APPLY is the final phase */

    /* handle the LOAD operation here based on start state edit mode */
    if (editop==OP_EDITOP_LOAD && (res==NO_ERR || errop==OP_ERROP_CONTINUE)) {
	/* fast-track API for the internal load command 
	 * The pducfg struct will be handed off to cfg->root
	 */
	res = cfg_load_root(target);
	if (res != NO_ERR) {
	    agt_record_error(scb, &msg->mhdr.errQ, 
			     NCX_LAYER_OPERATION, res, NULL,
			     NCX_NT_NONE, NULL, NCX_NT_VAL, pducfg);
	}
    }

    return res;

}  /* agt_ps_apply_write */


#if 0
/********************************************************************
* FUNCTION agt_ps_invoke_cb
* 
* Invoke all the specified callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   ps     == parmset from the PDU
*   curps  == parmset from the target config (may be NULL)
*   conterr == TRUE if the callbacks should continue on error
*           == FALSE if they should not continue on error
*
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added to the msg->mhdr.errQ
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_ps_invoke_cb (agt_cbtyp_t  cbtyp,
		      op_editop_t  editop,
		      ses_cb_t  *scb,
		      rpc_msg_t  *msg,
		      cfg_template_t *target,
		      ps_parmset_t *ps,
		      ps_parmset_t *curps,
		      boolean       conterr)
{
    ps_parm_t  *parm, *curparm;
    status_t    res, retres;
    agt_ps_pcbset_t  *cbset;
    agt_ps_pscbset_t *pscbset;

    
#ifdef DEBUG
    if (!scb || !msg || !target || !ps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    retres = NO_ERR;

    /* first invoke all the typedef, then parm callbacks */
    for (parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (ps_parm_t *)dlq_nextEntry(parm)) {

	/* find the current parm, if any */
	if (curps) {
	    curparm = ps_find_parmcopy(curps, parm);
	} else {
	    curparm = NULL;
	}

	/* call the typedef callback */
	res = agt_val_handle_callback(cbtyp, editop, scb, msg, 
				      target, parm->val, 
				      (curparm) ? curparm->val : NULL);
	if (res != NO_ERR) {
	    if (conterr) {
		retres = res;
	    } else {
		return res;
	    }
	}

	/* call the parm typedef user callback function, if any */
	if (parm->parm->cbset) {
	    cbset = (agt_ps_pcbset_t *)parm->parm->cbset;
	    if (cbset->pcb[cbtyp]) {
		res = (*cbset->pcb[cbtyp])
		    (scb, msg, cbtyp, ps->editop, parm, curparm);
		if (res != NO_ERR) {
		    if (conterr) {
			retres = res;
		    } else {
			return res;
		    }
		}
	    }
	}
    }

    /* invoke the parmset callback */
    if (ps->psd->cbset) {
	pscbset = (agt_ps_pscbset_t *)ps->psd->cbset;
	if (pscbset->pscb[cbtyp]) {
	    retres = (*pscbset->pscb[cbtyp])
		(scb, msg, cbtyp, editop, ps, curps);
	}
    }

    return retres;

}  /* agt_ps_invoke_cb */
#endif


/********************************************************************
* FUNCTION agt_ps_new_vparmset
* 
* Create a virtual (read-only) parmset for the specified PSD
* Choices are skipped and their parms must be added manually!!!
*
* INPUTS:
*   module == module name
*   psdname == parmset name
*   cbfn == get callback function to use for the entire parmset
*   full == TRUE if all relevant parameters should be created:
*           MANDATORY, OPTIONAL, and CONDITIONAL if condition is true
*        == FALSE if only mandatory parameters should be created
*   res == address of output status value
*
* OUTPUTS:
*   *res == status result, should be NO_ERR if return value non-NULL
*   parmset is added to <running> config and registered in the 
*   def-reg if NO_ERR
*
* RETURNS:
*   pointer to the new parmset that was just created, or NULL if errors
*********************************************************************/
ps_parmset_t *
    agt_ps_new_vparmset (const xmlChar *module,
			 const xmlChar *psdname,
			 getcb_fn_t cbfn,
			 boolean  full,
			 status_t *res)
{
    ps_parmset_t         *ps;
    const psd_template_t *psd;
    const psd_parm_t     *parmdef;
    ps_parm_t            *parm;
    cfg_template_t       *cfg;
    uint32                numparms, i;
    ncx_node_t            dtyp;
    

#ifdef DEBUG
    if (!module || !psdname || !cbfn || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* find the PSD template for this callback */
    dtyp = NCX_NT_PSD;
    psd = (const psd_template_t *)
	def_reg_find_moddef(module, psdname, &dtyp);
    if (!psd) {
	*res = ERR_NCX_UNKNOWN_PSD;
	return NULL;
    }

    /* check any errors in parmset setup */
    ps = create_parmset(psd, &cfg, res);
    if (!ps) {
	return NULL;
    }

    /* go through the parmQ
     * check the plain parameters and skip any choices
     */
    numparms = psd_parm_count(psd);
    for (i=1; i <= numparms; i++) {

	parmdef = psd_find_parmnum(psd, i);
	if (!parmdef) {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    continue;
	}

	/* check if this parameter needs to be setup */
	switch (parmdef->usage) {
	case PSD_UT_CONDITIONAL:
	    if (!psd_condition_met(parmdef->condition)) {
		continue;
	    }
	    break;
	case PSD_UT_MANDATORY:
	    break;
	case PSD_UT_OPTIONAL:
	    if (!full) {
		continue;
	    }
	    break;
	default:
	    *res = SET_ERROR(ERR_INTERNAL_VAL);
	    ps_free_parmset(ps);
	    return NULL;
	}

	/* create and setup the new virtual parameter */
	parm = agt_ps_new_vparm(ps, parmdef, cbfn, res);
	if (!parm) {
	    ps_free_parmset(ps);
	    return NULL;
	}
    }

    /* add this parmset to the <running> config */
    *res = cfg_add_parmset(cfg, ps, SES_NULL_SID);
    if (*res != NO_ERR) {
	ps_free_parmset(ps);
	return NULL;
    } else {
	*res = NO_ERR;
	return ps;
    }

} /* agt_ps_new_vparmset */


/********************************************************************
* FUNCTION agt_ps_new_vparm
* 
* Create a virtual (read-only) parm for the specified PSD
* Choices are skipped and their parms must be added manually!!!
*
* INPUTS:
*   ps == virtual parmset to create a new parm for
*   parmdef == parm definition template to use
*   cbfn == get callback function to use for the entire parmset
*   res == address of output status value
*
* OUTPUTS:
*   *res == status result, should be NO_ERR if return value non-NULL
*   parm is added to the parmset if NO_ERR
*
* RETURNS:
*   pointer to the new parm that was just created, or NULL if errors
*********************************************************************/
ps_parm_t *
    agt_ps_new_vparm (ps_parmset_t *ps,
		      const psd_parm_t *parmdef,
		      getcb_fn_t cbfn,
		      status_t *res)
{
    ps_parm_t  *parm;

#ifdef DEBUG
    if (!ps || !parmdef || !cbfn || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    parm = ps_new_parm();
    if (!parm) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    ps_setup_parm(parm, ps, parmdef);
    val_init_virtual(parm->val, cbfn, parmdef->pdef);
    parm->val->dataclass = NCX_DC_TCONFIG;
    ps_add_parm(parm, ps, NCX_MERGE_FIRST);

    *res = NO_ERR;
    return parm;
    
} /* agt_ps_new_vparm */


/********************************************************************
* FUNCTION agt_ps_set_lastchange
* 
* Set the last changed time for a parmset instance
*
* INPUTS:
*   ps == parmset to set
*
*********************************************************************/
void
    agt_ps_set_lastchange (ps_parmset_t *ps)
{

#ifdef DEBUG
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (!ps->lastchange) {
	ps->lastchange = m__getMem(TSTAMP_MIN_SIZE);
	if (!ps->lastchange) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	    return;
	}
    }
    tstamp_datetime(ps->lastchange);

} /* agt_ps_set_lastchange */


/* END file agt_ps.c */
