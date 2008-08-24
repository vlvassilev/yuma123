/*  FILE: agt_val.c

   Manage Agent callbacks for typedef-based config manipulation

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
20may06      abb      begun

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

#ifndef _H_agt_acm
#include "agt_acm.h"
#endif

#ifndef _H_agt_cap
#include "agt_cap.h"
#endif

#ifndef _H_agt_cb
#include "agt_cb.h"
#endif

#ifndef _H_agt_ncx
#include "agt_ncx.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val
#include "agt_val.h"
#endif

#ifndef _H_agt_val_parse
#include "agt_val_parse.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
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
#include  "status.h"
#endif

#ifndef _H_typ
#include  "typ.h"
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

#ifndef _H_yangconst
#include  "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_VAL_DEBUG 1
#endif


/* recursive callback function foward decl */
static status_t
    invoke_btype_cb (agt_cbtyp_t cbtyp,
		     op_editop_t editop,
		     ses_cb_t  *scb,
		     rpc_msg_t  *msg,
		     cfg_template_t *target,
		     val_value_t  *newval,
		     val_value_t  *curval,
		     boolean done);


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION handle_audit_record
* 
* Create and store a change-audit record, if needed
*
* INPUTS:
*   editop == edit operation requested
*   scb == session control block
*   target == config database target 
*   node == top value node involved in edit
*   res == result of edit operation
*
* OUTPUTS:
*   log message generated if log level set to LOG_INFO or higher
*********************************************************************/
static void
    handle_audit_record (op_editop_t editop,
			 ses_cb_t  *scb,
			 cfg_template_t *target,
			 val_value_t *node,
			 status_t res)
{
    xmlChar  *ibuff, tbuff[TSTAMP_MIN_SIZE+1];

    if (editop == OP_EDITOP_LOAD) {
	return;
    }

    ibuff = NULL;
    tstamp_datetime(tbuff);

    if (node) {
	(void)val_gen_instance_id(NULL, node, NCX_IFMT_XPATH1, 
				  &ibuff);
    }

    log_info("\nedit-config: operation %s on session %d by %s@%s"
	     "\n  at %s on target %s with result (%s)"
	     "\n  data: %s",
	     op_editop_name(editop),
	     scb->sid, 
	     scb->username,
	     scb->peeraddr,
	     tbuff,
	     target->name,
	     get_error_string(res),
	     (ibuff) ? (const char *)ibuff : "--");

    if (ibuff) {
	m__free(ibuff);
    }
	     
} /* handle_audit_record */


/********************************************************************
* FUNCTION handle_user_callback
* 
* Find the correct user callback function and invoke it
*
* INPUTS:
*    cbtyp == agent callback type
*    editop == edit operation applied to newnode oand/or curnode
*    scb == session control block invoking the callback
*    msg == RPC message in progress
*    newnode == new node in operation
*    curnode == current node in operation
*
* RETURNS:
*   status of the operation (usually returned from the callback)
*   NO USER CALLBACK FOUND == NO_ERR
*********************************************************************/
static status_t
    handle_user_callback (agt_cbtyp_t cbtyp,
			  op_editop_t editop,
			  ses_cb_t  *scb,
			  rpc_msg_t  *msg,
			  val_value_t *newnode,
			  val_value_t *curnode)
{
    agt_cb_fnset_t    *cbset;
    val_value_t       *val;
    status_t           res;

    val = newnode;
    if (val->obj->cbset) {
	cbset = val->obj->cbset;
	if (cbset->cbfn[cbtyp]) {
	    res = (*cbset->cbfn[cbtyp])
		(scb, msg, cbtyp, editop, newnode, curnode);
	    val->res = res;
	    return res;
	}
    }
    return NO_ERR;

} /* handle_user_callback */


/********************************************************************
* FUNCTION add_valnode
* 
* Add a val node to the parent val node
*
* INPUTS:
*    valnode == val_value_t struct to add to the parent val
*    parent == val_value_t parent to add the valnode as a child
*********************************************************************/
static void
    add_valnode (val_value_t *valnode,
		 val_value_t *parent)
{
    val_add_child(valnode, parent);
} /* add_valnode */


/********************************************************************
* FUNCTION swap_valnode
* 
* Swap a value node in the parent value node
*
* INPUTS:
*    newval == val_valuet struct to add to the parent value node
*    curval == current value node to replace
*********************************************************************/
static void
    swap_valnode (val_value_t *newval,
		  val_value_t *curval)
{
    val_swap_child(newval, curval);
} /* swap_valnode */


/********************************************************************
* FUNCTION merge_valnode
* 
* Merge a value node to the parent value node
*
* INPUTS:
*    newval == val_value_t struct to merge into the parent node
*    curval == (first) current value found to merge with
* RETURNS:
*    TRUE if src should now be freed
*    FALSE if source should not be freed
*********************************************************************/
static boolean
    merge_valnode (val_value_t *newval,
		   val_value_t *curval)
{
    return val_merge(newval, curval);

} /* merge_valnode */


/********************************************************************
* FUNCTION add_undo_node
* 
* Add an undo node to the msg->undoQ
*
* INPUTS:
*    appnode 
*
* OUTPUTS:
*    rpc_undo_rec_t struct added to msg->undoQ
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    add_undo_node (rpc_msg_t *msg,
		   op_editop_t editop,
		   val_value_t *newnode,
		   val_value_t *curnode,
		   val_value_t *parentnode,
		   status_t  res)
{
    rpc_undo_rec_t *undo;

    /* create an undo record for this merge */
    undo = rpc_new_undorec();
    if (!undo) {
	return ERR_INTERNAL_MEM;
    }
    undo->ismeta = FALSE;
    undo->editop = editop;
    undo->newnode = newnode;

    /* save a copy of the current value in case it gets modified
     * in a merge operation
     */
    if (curnode) {
	undo->curnode = val_clone(curnode);
	if (!undo->curnode) {
	    rpc_free_undorec(undo);
	    return ERR_INTERNAL_MEM;
	}
    }
    undo->parentnode = parentnode;
    undo->res = res;

    dlq_enque(undo, &msg->rpc_undoQ);
    return NO_ERR;

} /* add_undo_node */


/********************************************************************
* FUNCTION apply_this_node
* 
* Check if the write operation applies to the current node
*
* INPUTS:
*    editop == edit operation value
*    curnode == pointer to current value node 
*                   (just used to check if non-NULL)
*
* RETURNS:
*    TRUE if the current node needs the write operation applied
*    FALSE if this is a NO=OP node (either explicit or special merge)
*********************************************************************/
static boolean
    apply_this_node (op_editop_t editop,
		     const val_value_t *curnode)
{
    boolean retval;
    const val_value_t *val;

    retval = FALSE;
    switch (editop) {
    case OP_EDITOP_NONE:
	/* never apply here when operation is none */
	break;
    case OP_EDITOP_MERGE:
	/* if no current node then always merge here
	 * merge child nodes into an existing complex type
	 * except for the index nodes, which are kept
	 */
	if (!curnode) {
	    retval = TRUE;
	} else {
	    val = (const val_value_t *)curnode;

	    /* if this is a leaf and not an index leaf, then
	     * apply the merge here
	     */
	    if (val && !val->index) {
		retval = typ_is_simple(obj_get_basetype(val->obj));
	    }
	}
	break;
    case OP_EDITOP_CREATE:
    case OP_EDITOP_REPLACE:
    case OP_EDITOP_LOAD:
    case OP_EDITOP_DELETE:
	retval = TRUE;
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }
    return retval;

} /* apply_this_node */


/********************************************************************
* FUNCTION merge_metadata
* 
*   Add any metadata to the curnode from the newnode
*   Generate undo and audit records records if needed
*
* INPUTS:
*    msg == rpc message in progress
*    newval == node to steal (move) attributes from
*    curval == node to add or replace the value of attributes
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    merge_metadata (rpc_msg_t *msg,
		    val_value_t *newval,
		    val_value_t *curval)
{
#ifdef NOT_YET
    status_t  res;

    res = NO_ERR;

    /* first determine if any actual XML attribute merge is
     * actually requested in the 'newval'
     */



    /* something requested, so check if an undo record is needed */
    if (msg->rpc_need_undo) {
	res = add_undo_node(msg, editop,
			    newval, curval, /*curparent*/  NO_ERR);
	if (res != NO_ERR) {
	    return res;
	}
    }

    val_merge_meta(newval, curval);
    return res;
#else
    (void)msg;
    val_merge_meta(newval, curval);
    return NO_ERR;
#endif
}   /* merge_metadata */


/*****************   V A L I D A T E    P H A S E   ****************/

#if 0
/********************************************************************
* FUNCTION check_inst_cnt
* 
* Check the 'newval' instance count for one child node 
*
* 1) Check 'cnt' againt 'typdef' instance qualifier enum
* 2) If no value instance:
*   a) check if child nodes exist in the current node value 
*   b) if a default is requested, check for a default in the 
*      typdef chain
*
* INPUTS:
*   cnt == number of instances of this child node
*   chobj == obj_template_t struct to check for this value node
*   curval == current data object in the target (or NULL if none)
*   adddef == TRUE if one child node instance should be added
*             if it is required, and a default is available
*          == FALSE if any default value should be ignored
* RETURNS:
*   status; no rpc-error is recorded
*********************************************************************/
static status_t
    check_inst_cnt (uint32 cnt,
		    const obj_template_t  *chobj,
		    val_value_t  *curval,
		    boolean adddef)
{
    status_t      res;
    ncx_iqual_t   iqual;

    res = NO_ERR;
    iqual = obj_get_iqualval(chobj);

    /* validate this child node instance count */
    switch (iqual) {
    case NCX_IQUAL_ONE:
	/* exactly 1 instance allowed */
	if (cnt > 1) {
	    res = ERR_NCX_EXTRA_VAL_INST;
	} else if (cnt == 0) {
	    res = ERR_NCX_MISSING_VAL_INST;
	}
	break;
    case NCX_IQUAL_OPT:
	/* Zero or 1 instance allowed */
	if (cnt > 1) {
	    res = ERR_NCX_EXTRA_VAL_INST;
	}
	break;
    case NCX_IQUAL_1MORE:
	/* One or more instances allowed */
	if (cnt == 0) {
	    res = ERR_NCX_MISSING_VAL_INST;
	}
	break;
    case NCX_IQUAL_ZMORE:
	/* Zero or more instances allowed */
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* if newval->child is missing, then check further */
    if (res == ERR_NCX_MISSING_VAL_INST) {
	if (curval) {
	    if (val_child_inst_cnt(curval,
				   obj_get_mod_name(chobj),
				   obj_get_name(chobj))) {
		/* this is a merge, and a current value exists */
		res = NO_ERR;
	    }
	}
	if (res != NO_ERR && adddef) {
	    /* not a valid merge so check if a default exists */
	    if (obj_get_default(chobj)) {
		res = NO_ERR;
	    }
	}
    }

    return res;

} /* check_inst_cnt */


/********************************************************************
* FUNCTION check_child_inst_cnt
* 
* Check the proposed new complex type; go through all the
* child nodes and determine if the proper number of instances
* is present
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   newval == new data object to check
*   newval == current data object to check
*             If this is non-NULL, the MERGE operation will be checked
*             If this is NULL, then a CREATE, REPLACE, or LOAD is assumed
*   adddef == TRUE if one child node instance should be added
*             if it is required, and a default is available
*          == FALSE if any default value should be ignored
* 
* OUTPUTS:
*   rpc-error records will be generated as needed
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_child_inst_cnt (ses_cb_t  *scb,
			  rpc_msg_t  *msg,
			  val_value_t *newval,
			  val_value_t *curval,
			  boolean adddef)
{
    const obj_template_t  *chobj;
    xmlns_qname_t          qname;
    status_t               res, retres;
    uint32                 cnt;

    retres = NO_ERR;

    /* go through each child node in the typdef to see how many
     * instances of each child are present
     */
    for (chobj = obj_first_child(newval->obj);
	 chobj != NULL;
	 chobj = obj_next_child(chobj)) {

	cnt = val_child_inst_cnt(newval,
				 obj_get_mod_name(chobj),
				 obj_get_name(chobj));

	switch (obj_get_basetype(chobj)) {
	case NCX_BT_LIST:
	    /* tables are always allowed to have zero or more instances 
	     * any invalid instance IDs will be checked seperately
	     */
	    continue;
	case NCX_BT_EMPTY:
	    /* The flag datatype is allowed to be missing as FALSE */
	    if (!cnt) {
		continue;
	    }
	    break;
	default:
	    break;
	}

	/* need to validate this child node instance count */
	res = check_inst_cnt(cnt, chobj, curval, adddef);
	    
	/* check any errors for this child node */
	if (res != NO_ERR) {
	    qname.nsid = newval->nsid;
	    qname.name = obj_get_name(chobj);
	    agt_record_error(scb, &msg->mhdr, NCX_LAYER_CONTENT, 
			     res, NULL, NCX_NT_QNAME, &qname, 
			     NCX_NT_VAL, newval);
	    retres = res;
	}
    }

    return retres;

} /* check_child_inst_cnt */


/********************************************************************
* FUNCTION check_choice_group
* 
* Check the proposed new or merged NCX_NT_CHOICE; a node was found 
* that is part of a group; make sure these other group members
* are present or optionally would be with a default added.
*
* INPUTS:
*   newval == new data object to check
*   newval == current data object to check
*             If this is non-NULL, the MERGE operation will be checked
*             If this is NULL, then a CREATE, REPLACE, or LOAD is assumed
*   chobj == the child object template for chval, contains the group info
*   adddef == TRUE if one child node instance should be added
*             if it is required, and a default is available
*          == FALSE if any default value should be ignored
*   total == total number of child nodes
* RETURNS:
*   status; no rpc-error is recorded
*********************************************************************/
static status_t
    check_choice_group (val_value_t *newval,
			val_value_t *curval,
			const obj_template_t *chobj,
			boolean adddef,
			uint32  total)
{
    const obj_template_t  *grch;
    status_t               res;
    uint32                 cnt, chtotal;


#if 0
    /* Since groups cannot be nested, it is okay to assume
     * none of these child nodes will be another nested group
     */
    chtotal = 0;
    for (grch = (typ_child_t *)dlq_firstEntry(&chtyp->grouptop->groupQ);
	 grch != NULL;
	 grch = (typ_child_t *)dlq_nextEntry(grch)) {

	/* get the number of instances of this child node */
	cnt = val_child_inst_cnt(newval, grch->name);
	res = check_inst_cnt(cnt, chtyp, curval, adddef);
	if (res != NO_ERR) {
	    return res;
	} else {
	    chtotal += cnt;
	}
    }

    /* make sure the choice group accounts for all the child nodes */
    if (cnt != total) {
	return ERR_NCX_EXTRA_CHOICE;
    } else {
	return NO_ERR;
    }

#endif

    return SET_ERROR(ERR_INTERNAL_VAL);

} /* check_choice_group */


/********************************************************************
* FUNCTION check_choice_inst_cnt
* 
* Check the proposed new or merged NCX_NT_CHOICE; go through all the
* child nodes and determine if one choice or choice group is selected
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   newval == new data object to check
*   newval == current data object to check
*             If this is non-NULL, the MERGE operation will be checked
*             If this is NULL, then a CREATE, REPLACE, or LOAD is assumed
*   adddef == TRUE if one child node instance should be added
*             if it is required, and a default is available
*          == FALSE if any default value should be ignored
* 
* OUTPUTS:
*   rpc-error records will be generated as needed
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_choice_inst_cnt (ses_cb_t  *scb,
			   rpc_msg_t  *msg,
			   val_value_t *newval,
			   val_value_t *curval,
			   boolean adddef)
{
    status_t      res;
    uint32        cnt, chcnt;
    const typ_def_t    *typdef;
    typ_child_t  *chtyp;
    val_value_t  *chval;

    res = NO_ERR;

    /* first check the total number of child nodes present */
    cnt = val_child_cnt(newval);
    if (!cnt) {
	/* zero child nodes present is an error
	 * check if this is a merge operation with a valid choice 
	 */
	if (curval) {
	    cnt = val_child_cnt(curval);
	}

	/* make sure current value has at least 1 child node */
	if (!cnt) {
	    /* nothing present, which is an error 
	     * TBD: There are no defvals for complex types yet
	     */
	    res = ERR_NCX_MISSING_CHOICE;
	}
    } 

    /* at this point the total child count is > 0
     * make sure the choice instance is properly constructed
     */
    if (res==NO_ERR) {
	/* need to check for a single choice or a choice group 
	 * go through each child node in the value instance
	 * and compare it to the typdef to see how many
	 * instances of each child should be present
	 *
	 * chval is set to the first child instance;
	 * Picking the first child node as the one that is valid.
	 * This is just an arbitrary convention; If extra
	 * choices are present, there is no way to pick one
	 * as correct, and the rest as errors
	 */
	chval = val_get_first_child(newval);
	if (!chval) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}

	/* get the typdef that actually has the child Q */
	typdef = typ_get_cbase_typdef(newval->typdef);

	/* get the child type node for this child instance name */
	chtyp = typ_find_child(chval->name, &typdef->def.complex);
	if (!chtyp) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}

	/* check if this child is part of a group */
	if (chtyp->grouptop) {
	    /* This choice is part of a choice group 
	     * Make sure all the members of the group
	     * are accounted for; 
	     */
	    res = check_choice_group(newval, curval, chtyp, adddef, cnt);
	} else {
	    /* get the total instance count for this child node */
	    chcnt = val_child_inst_cnt(newval, chtyp->name);

	    /* validate this child node instance count */
	    res = check_inst_cnt(chcnt, chtyp, curval, adddef);

	    /* check this child instance count accounts for the total */
	    if (res == NO_ERR && chcnt != cnt) {
		res = ERR_NCX_EXTRA_CHOICE;
	    }
	}
    }

    /* check any errors for this child node */
    if (res != NO_ERR) {
	agt_record_error(scb, &msg->mhdr, NCX_LAYER_CONTENT, 
			 res, NULL, NCX_NT_STRING, chtyp->name, 
			 NCX_NT_VAL, newval);
    }

    return res;

} /* check_choice_inst_cnt */


/********************************************************************
* FUNCTION check_edit_instance
* 
* Check the complex data instance against the edit operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   newval == new data object
*   curval == current data object in the target (or NULL if none)
*   adddef == TRUE if one child node instance should be added
*             if it is required, and a default is available
*          == FALSE if any default value should be ignored
* 
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_edit_instance (ses_cb_t  *scb,
			 rpc_msg_t  *msg,
			 val_value_t  *newval,
			 val_value_t *curval,
			 boolean      adddef)
{
    status_t   res;

#ifdef DEBUG
    if (!newval) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    switch (newval->btyp) {
    case NCX_BT_CONTAINER:
	res = check_child_inst_cnt(scb, msg, newval, curval, adddef);
	break;
    case NCX_BT_CHOICE:
	/* check that 1 choice or choice group is selected */
	/* res = check_choice_inst_cnt(scb, msg, newval, curval, adddef); */
	res = SET_ERROR(ERR_INTERNAL_VAL);
	break;
    case NCX_BT_LIST:
	/* check that table instance and non-instance columns 
	 * are present 
	 */	    
	res = check_child_inst_cnt(scb, msg, newval, curval, adddef);
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

} /* check_edit_instance */


/********************************************************************
* FUNCTION validate_write_val
* 
* Invoke all the AGT_CB_VALIDATE callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   pop == parent requested edit op
*   newval == parm to apply
*   curvalset == current valset containing this parm from target (if any)
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    validate_write_val (ses_cb_t  *scb,
			rpc_msg_t  *msg,
			cfg_template_t *target,
			op_editop_t  pop,
			val_value_t    *newval,
			val_value_t  *curvalset)
{
    val_value_t     *curval, *v_val;
    status_t         res;
    ncx_iqual_t      iqual;

#ifdef AGT_VAL_DEBUG
    log_debug2("\nvalidate_write_val: %s start", newval->name);
#endif

    curval = NULL;
    v_val = NULL;
    res = NO_ERR;

    /* try to find the current parm in the target config */
    if (curvalset) {
	curval = val_find_child(curvalset,
				obj_get_mod_prefix(newval->obj),
				newval->name);
	if (curval && val_is_virtual(curval)) {
	    v_val = val_get_virtual_value(scb, curval, &res);
	}
    }

    if (res == NO_ERR) {
	/* check and adjust the operation attribute */
	iqual = val_get_iqualval(newval);

	/* check edit-op does not need to check the
	 * parm value, so pass the curparm, not curval or v_val
	 */
	res = agt_check_editop(pop, &newval->editop,
			       newval, curval, iqual);

	/* check the max-access for this param */
	if (res == NO_ERR) {
	    res = agt_check_max_access(newval->editop, 
				       obj_get_max_access(newval->obj), 
				       (curval != NULL));
	}

	if (res==NO_ERR && newval->obj->objtype==OBJ_TYP_LIST) {
	    /**** unique test ****/
	}

    }

    /* record any error so far */
    if (res != NO_ERR) {
	agt_record_error(scb, &msg->mhdr, NCX_LAYER_CONTENT, res,
			 NULL, NCX_NT_VAL, newval, 
			 NCX_NT_VAL, newval);
    } else {
	/* check for any typedef callbacks for this param;
	 * process the edit operation along the way
	 */
	res = invoke_btype_cb(AGT_CB_VALIDATE, pop, scb, msg, 
			      target, newval, 
			      (v_val) ? v_val : curval, FALSE);
    }

    if (v_val) {
	val_free_value(v_val);
    }

    return res;

}  /* validate_write_val */
#endif


/********************************************************************
* FUNCTION apply_write_val
* 
* Invoke all the AGT_CB_APPLY callbacks for a 
* source and target and write operation
*
* INPUTS:
*   editop == edit operation in effect on the current node
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == config database target 
*   parent == parent value of curval and newval
*   newval == new value to apply
*   curval == current instance of value (may be NULL if none)
*   *done  == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*
* OUTPUTS:
*   newval and curval may be moved around to
*       different queues, or get modified
*   *done may be changed from FALSE to TRUE if node is applied here
* RETURNS:
*   status
*********************************************************************/
static status_t
    apply_write_val (op_editop_t  editop,
		     ses_cb_t  *scb,
		     rpc_msg_t  *msg,
		     cfg_template_t *target,
		     val_value_t  *parent,
		     val_value_t  *newval,
		     val_value_t  *curval,
		     boolean      *done)
{
    status_t         res;
    boolean          applyhere, mergehere, freenew;
    
#ifdef AGT_VAL_DEBUG
    log_debug2("\napply_write_val: %s start", newval->name);
#endif

    res = NO_ERR;
    mergehere = FALSE;
    freenew = FALSE;

    /* check if this node needs the edit operation applied */
    if (*done) {
	applyhere = FALSE;
    } else {
	applyhere = apply_this_node(newval->editop, curval);
	*done = applyhere;
    }

    /* apply the requested edit operation */
    if (applyhere) {
	if (msg->rpc_need_undo) {
	    res = add_undo_node(msg, editop, newval,
				curval, parent, NO_ERR);
	    if (res != NO_ERR) {
		return res;
	    }
	}

	handle_audit_record(newval->editop, scb, target, 
			    (curval) ? curval : newval, res);

	/* make sure the node is not a virtual value */
	if (curval && val_is_virtual(curval)) {
	    return NO_ERR;   /*** freenew?? ***/
	}

	switch (newval->editop) {
	case OP_EDITOP_MERGE:
	    dlq_remove(newval);
	    if (curval) {
		freenew = merge_valnode(newval, curval);
	    } else {
		add_valnode(newval, parent);
	    }
	    break;
	case OP_EDITOP_REPLACE:
	    dlq_remove(newval);
	    if (curval) {
		swap_valnode(newval, curval);
		if (!msg->rpc_need_undo) {
		    val_free_value(curval);
		} /* else curval not freed yet, hold in undo record */
	    } else {
		add_valnode(newval, parent);
	    }
	    break;
	case OP_EDITOP_CREATE:
	    dlq_remove(newval);
	    add_valnode(newval, parent);
	    break;
	case OP_EDITOP_LOAD:
	    dlq_remove(newval);
	    res = cfg_apply_load_root(target, newval);
	    break;
	case OP_EDITOP_DELETE:
	    if (curval) {
		dlq_remove(curval);
		if (!msg->rpc_need_undo) {
		    val_free_value(curval);
		} /* else curval not freed yet, hold in undo record */
	    }
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    } else if (newval->editop == OP_EDITOP_MERGE && curval) {
	merge_metadata(msg, newval, curval);
    }

    if (freenew) {
	val_free_value(newval);
    }

    return res;

}  /* apply_write_val */


/********************************************************************
* FUNCTION invoke_simval_cb
* 
* Invoke all the specified agent simple type callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type being invoked
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newval == val_value_t from the PDU
*   curval == current value (if any) from the target config
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*   
* RETURNS:
*   status
*********************************************************************/
static status_t
    invoke_simval_cb (agt_cbtyp_t cbtyp,
		      op_editop_t editop,
		      ses_cb_t  *scb,
		      rpc_msg_t  *msg,
		      cfg_template_t *target,		      
		      val_value_t  *newval,
		      val_value_t  *curval,
		      boolean  done)
{
    status_t         res;
    ncx_iqual_t      iqual;

    res = NO_ERR;

    /* check the 'operation' attribute in VALIDATE phase */
    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef AGT_VAL_DEBUG
    log_debug2("\ninvoke_simval:validate: %s start", newval->name);
#endif

	/* check and adjust the operation attribute */
	iqual = val_get_iqualval(newval);
	res = agt_check_editop(editop, &newval->editop, 
			       newval, curval, iqual);

	if (res == NO_ERR) {
	    res = agt_check_max_access(newval->editop, 
				       obj_get_max_access(newval->obj), 
				       (curval != NULL));
	}

	if (res != NO_ERR) {
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, 
			     NULL, NCX_NT_VAL, newval, 
			     NCX_NT_VAL, newval);
	}
	break;
    case AGT_CB_APPLY:
	res = apply_write_val(newval->editop, scb, msg, target, 
			      newval->curparent, newval, curval, &done);
	break;
    default:
	;  /***/
    }

    /* check if the typdef for this value has a callback */
    if (res == NO_ERR) {
	res = handle_user_callback(cbtyp, editop, scb, msg, 
				   newval, curval);
    }

    return res;

}  /* invoke_simval_cb */


/********************************************************************
* FUNCTION invoke_cpxval_cb
* 
* Invoke all the specified agent complex type callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type being invoked
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newval == val_value_t from the PDU
*   curval == current value (if any) from the target config
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    invoke_cpxval_cb (agt_cbtyp_t cbtyp,
		      op_editop_t  editop,
		      ses_cb_t  *scb,
		      rpc_msg_t  *msg,
		      cfg_template_t *target,
		      val_value_t  *newval,
		      val_value_t  *curval,
		      boolean done)
{
    val_value_t      *chval, *curch, *nextch;
    status_t          res, retres;
    ncx_iqual_t       iqual;
    
    retres = NO_ERR;

    /* check the 'operation' attribute in VALIDATE phase */
    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef AGT_VAL_DEBUG
    log_debug2("\ninvoke_cpxval:validate: %s start", newval->name);
#endif

	/* check and adjust the operation attribute */
	iqual = val_get_iqualval(newval);
	res = agt_check_editop(editop, &newval->editop, 
			       newval, curval, iqual);
	if (res == NO_ERR) {
	    res = agt_check_max_access(newval->editop, 
				       obj_get_max_access(newval->obj), 
				       (curval != NULL));
	}

	if (res==NO_ERR && newval->obj->objtype==OBJ_TYP_LIST) {
	    /**** unique test ****/
	}

	if (res != NO_ERR) {
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, 
			     NULL, NCX_NT_VAL, newval, 
			     NCX_NT_VAL, newval);
	    return res;
	}
	retres = res;
	break;
    case AGT_CB_APPLY:
	retres = apply_write_val(newval->editop, scb, msg, target,
				 newval->curparent, newval, curval, &done);
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check all the child nodes next */
    if (retres == NO_ERR) {
	for (chval = val_get_first_child(newval);
	     chval != NULL;
	     chval = nextch) {

	    nextch = val_get_next_child(chval);

	    chval->curparent = curval;
	    if (curval) {
		curch = val_first_child(curval, chval);
	    } else {
		curch = NULL;
	    }
	    res = invoke_btype_cb(cbtyp, newval->editop, scb, msg, 
				  target, chval, curch, done);
	    chval->res = res;
	    if (res != NO_ERR) {
		retres = res;
	    }
	}
    }

    /* check if the typdef for this value has a callback */
    if (retres == NO_ERR) {
	retres = handle_user_callback(cbtyp, editop, scb, msg,
				      newval, curval);
    }
	
    return retres;

}  /* invoke_cpxval_cb */


/********************************************************************
* FUNCTION invoke_btype_cb
* 
* Invoke all the specified agent typedef callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type being invoked
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newval == val_value_t from the PDU
*   curval == current value (if any) from the target config
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    invoke_btype_cb (agt_cbtyp_t cbtyp,
		     op_editop_t editop,
		     ses_cb_t  *scb,
		     rpc_msg_t  *msg,
		     cfg_template_t *target,
		     val_value_t  *newval,
		     val_value_t  *curval,
		     boolean done)
{
    status_t       res;
    val_value_t   *v_val;

    res = NO_ERR;
    v_val = NULL;

    if (cbtyp==AGT_CB_VALIDATE) {
	if (curval && val_is_virtual(curval)) {
	    v_val = val_get_virtual_value(scb, curval, &res);
	}

	if (res != NO_ERR) {
	    return res;
	}
    }

    /* first traverse all the nodes until leaf nodes are reached */
    switch (newval->obj->objtype) {
    case OBJ_TYP_LEAF:
	res = invoke_simval_cb(cbtyp, editop, scb, msg,
			       target, newval, 
			       (v_val) ? v_val : curval, done);
	break;
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_LIST:
    case OBJ_TYP_RPC:
    case OBJ_TYP_RPCIO:
    case OBJ_TYP_NOTIF:
	res = invoke_cpxval_cb(cbtyp, editop, scb, msg, 
			       target, newval, 
			       (v_val) ? v_val : curval, done);
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (v_val) {
	val_free_value(v_val);
    }

    return res;

}  /* invoke_btype_cb */


/********************************************************************
* FUNCTION process_undo_entry   AGT_CB_ROLLBACK
* 
* Attempt to rollback an edit that requested rollback-on-error
*
* INPUTS:
*   undo == rpc_undo_rec_t struct to process
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*
*********************************************************************/
static void
    process_undo_entry (rpc_undo_rec_t *undo,
			ses_cb_t  *scb,
			rpc_msg_t  *msg,
			cfg_template_t *target)
{
    status_t   res;

    (void)target;

    switch (undo->editop) {
    case OP_EDITOP_LOAD:
	/* not supported for internal load operation */
	break;    
    case OP_EDITOP_CREATE:
	/* Since 'create' cannot apply to an attribute,
	 * the metadata is not checked
	 */
	res = handle_user_callback(AGT_CB_ROLLBACK, undo->editop,
				   scb, msg, undo->newnode, undo->curnode);

#ifdef NOT_YET
	/**** SET done !!! ***/
	if (!done) {
	    /* no rollback callback, so apply a delete operation */
	    res = handle_user_callback(AGT_CB_APPLY, OP_EDITOP_DELETE,
				       scb, msg, undo->newnode, NULL);
	}
#endif

	/* delete the node from the tree */
	dlq_remove(undo->newnode);
	val_free_value(undo->newnode);
	break;
    case OP_EDITOP_DELETE:
	/* Since 'delete' cannot apply to an attribute,
	 * the metadata is not checked
	 */
	res = handle_user_callback(AGT_CB_ROLLBACK, 
				   undo->editop, scb, msg, 
				   undo->newnode, undo->curnode);

#ifdef NOT_YET
	/**** SET done !!! ***/
	if (!done) {
	    /* no rollback callback, so apply a create operation */
	    res = handle_user_callback(AGT_CB_APPLY, OP_EDITOP_CREATE,
				       scb, msg, undo->curnode, NULL);
	}
#endif

	/* add the node back in the tree */
	add_valnode(undo->curnode, undo->parentnode);
	break;
    case OP_EDITOP_MERGE:
    case OP_EDITOP_REPLACE:
	/*** NEED TO CHECK FOR MERGED META DATA FOR MERGE ***/

	/* call the user rollback handler, if any */
	res = handle_user_callback(AGT_CB_ROLLBACK, 
				   undo->editop, scb, msg, 
				   undo->newnode, undo->curnode);
#ifdef NOT_YET
	/**** SET done !!! ***/
	if (!done) {
	    /* no rollback callback, so apply a create operation */
	    res = handle_user_callback(AGT_CB_APPLY, OP_EDITOP_DELETE,
				       scb, msg, undo->newnode,
				       undo->curnode);
	}
#endif

	/* check if the old node needs to be swapped back
	 * of if the new node is just removed
	 */
	if (undo->newnode) {
	    dlq_remove(undo->newnode);
	    if (undo->curnode) {
		swap_valnode(undo->curnode, undo->newnode);
	    } 
	    /* remove new node */
	    val_free_value(undo->newnode);

	} /* else should not happen */
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    /***** !!!!!  ****
    handle_undo_audit_record(undo->editop,
                             scb, target,
  			     undo->curnode,
			     undo->res);
    ***/

}  /* process_undo_entry */


/********************************************************************
* FUNCTION process_undo_list
* 
* Either complete or rollback an edit that requested rollback-on-error
*
* INPUTS:
*   cbtyp == callback type  (AGT_CB_COMMIT or AGT_CB_ROLLBACK)
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*
* OUTPUTS:
*   if errors are encountered (such as undo-failed) then they
*   will be added to the msg->mhdr.errQ
*
* RETURNS:
*   void
*********************************************************************/
static void
    process_undo_list (agt_cbtyp_t cbtyp,
		       ses_cb_t  *scb,
		       rpc_msg_t  *msg,
		       cfg_template_t *target)

{
    rpc_undo_rec_t  *undo;
    status_t         res;

    while (!dlq_empty(&msg->rpc_undoQ)) {
	undo = (rpc_undo_rec_t *)dlq_deque(&msg->rpc_undoQ);
	if (cbtyp==AGT_CB_COMMIT) {
	    res = handle_user_callback(AGT_CB_COMMIT, undo->editop, 
				       scb, msg, undo->newnode, 
				       undo->curnode);

	    /* just clean up 'curnode' if it was held instead of deleted */
	    switch (undo->editop) {
	    case OP_EDITOP_REPLACE:
	    case OP_EDITOP_DELETE:
		/* finish deleting 'curnode' */
		if (undo->curnode) {
		    ncx_free_node(undo->curnodetyp, undo->curnode);
		}
		break;
	    case OP_EDITOP_LOAD:
	    case OP_EDITOP_CREATE:
	    case OP_EDITOP_MERGE:
		/* no nodes in the undo rec need to be freed */
		break;
	    default:
		/* should not happen */
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	} else {
	    /* rollback the edit operation */
	    process_undo_entry(undo, scb, msg, target);
	}
	rpc_free_undorec(undo);
    }

}  /* process_undo_list */


/********************************************************************
* FUNCTION handle_callback
* 
* Invoke all the specified agent typedef callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type being invoked
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newval == val_value_t from the PDU
*   curval == current value (if any) from the target config
* RETURNS:
*   status
*********************************************************************/
static status_t
    handle_callback (agt_cbtyp_t cbtyp,
		     op_editop_t editop,
		     ses_cb_t  *scb,
		     rpc_msg_t  *msg,
		     cfg_template_t *target,
		     val_value_t  *newval,
		     val_value_t  *curval)
{
    status_t  res;

#ifdef DEBUG
    if (!scb || !msg || !target || !newval) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    switch (cbtyp) {
    case AGT_CB_VALIDATE:
    case AGT_CB_APPLY:
	/* keep checking until all the child nodes have been processed */
	res = invoke_btype_cb(cbtyp, editop, scb, msg, 
			      target, newval, curval, FALSE);
	break;
    case AGT_CB_COMMIT:
    case AGT_CB_ROLLBACK:
	process_undo_list(cbtyp, scb, msg, target);
	res = NO_ERR;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* handle_callback */



/********************************************************************
* FUNCTION instance_check
* 
* Check for the proper number of object instances for
* the specified value struct. Checks the direct accessible
* children of 'val' only!!!
* 
* The top-level value set passed cannot represent a choice
* or a case within a choice.
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   obj == object template for child node in valset to check
*   val == val_value_t list, leaf-list, or container to check
*   layer == NCX layer calling this function (for error purposes only)
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 

*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    instance_check (ses_cb_t *scb,
		    xml_msg_hdr_t *msg,
		    const obj_template_t *obj,
		    val_value_t *val,
		    ncx_layer_t   layer)
{
    ncx_iqual_t            iqual;
    uint32                 cnt, minelems, maxelems;
    boolean                minset, maxset, minerr, maxerr;
    status_t               res;

    res = NO_ERR;
    iqual = obj_get_iqualval(obj);
    minerr = FALSE;
    maxerr = FALSE;
    minelems = 0;
    maxelems = 0;

    minset = obj_get_min_elements(obj, &minelems);
    maxset = obj_get_max_elements(obj, &maxelems);

    cnt = val_instance_count(val, obj_get_mod_name(obj),
			     obj_get_name(obj));

    if (LOGDEBUG2) {
	log_debug2("\ninstance_check '%s' against '%s' (cnt=%u, min=%u, max=%u)",
		   obj_get_name(obj), val->name, cnt, minelems, maxelems);
    }

    if (minset) {
	if (cnt < minelems) {
	    /* not enough instances error */
	    minerr = TRUE;
	    res = ERR_NCX_MIN_ELEMS_VIOLATION;
	    if (msg) {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_VAL, val);
	    }
	    res = NO_ERR;
	}
    }

    if (maxset) {
	if (cnt > maxelems) {
	    /* too many instances error
	     * need to find all the extra instances
	     * and mark the extras as errors or they will
	     * not get removed later
	     */
	    val_set_extra_instance_errors(val, 
					  obj_get_mod_name(obj),
					  obj_get_name(obj),
					  maxelems);

	    maxerr = TRUE;
	    res = ERR_NCX_MAX_ELEMS_VIOLATION;
	    if (msg) {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_VAL, val);
	    }
	    res = NO_ERR;
	}
    }

    switch (iqual) {
    case NCX_IQUAL_ONE:
	if (cnt < 1 && !minerr) {
	    /* missing single parameter */
	    res = ERR_NCX_MISSING_VAL_INST;
	}
	/* fall through */
    case NCX_IQUAL_OPT:
	if (cnt > 1 && !maxerr) {
	    /* too many parameters */
	    val_set_extra_instance_errors(val, 
					  obj_get_mod_name(obj),
					  obj_get_name(obj), 1);
	    res = ERR_NCX_EXTRA_VAL_INST;
	}
	break;
    case NCX_IQUAL_1MORE:
	if (cnt < 1 && !minerr) {
	    /* missing parameter error */
	    res = ERR_NCX_MISSING_VAL_INST;

	}
	break;
    case NCX_IQUAL_ZMORE:
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res != NO_ERR && msg) {
	agt_record_error(scb, msg, layer, res, 
			 NULL, NCX_NT_OBJ, obj, 
			 NCX_NT_VAL, val);
    }

    return res;
    
}  /* instance_check */


/********************************************************************
* FUNCTION choice_check_agt
* 
* Agent version of ncx/val_util.c/choice_check
*
* Check a val_value_t struct against its expected OBJ
* for instance validation:
*
*    - choice validation: 
*      only one case allowed if the data type is choice
*      Only issue errors based on the instance test qualifiers
*
* The input is checked against the specified obj_template_t.
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   choicobj == object template for the choice to check
*   val == parent val_value_t list or container to check
*   layer == NCX layer calling this function (for error purposes only)
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    choice_check_agt (ses_cb_t  *scb,
		      xml_msg_hdr_t *msg,
		      const obj_template_t *choicobj,
		      val_value_t *val,
		      ncx_layer_t   layer)
{
    val_value_t           *chval, *testval;
    status_t               res;

    res = NO_ERR;

    if (LOGDEBUG2) {
	log_debug2("\nichoice_check_agt: check '%s' against '%s'",
		   obj_get_name(choicobj), val->name);
    }

    /* Go through all the child nodes for this object
     * and look for choices against the value set to see if each 
     * a choice case is present in the correct number of instances.
     *
     * The current value could never be a OBJ_TYP_CHOICE since
     * those nodes are not stored in the val_value_t tree
     * Instead, it is the parent of the choice object,
     * and the accessible case nodes will be child nodes
     * of that complex parent type
     */
    chval = val_get_choice_first_set(val, choicobj);
    if (!chval) {
	if (obj_is_mandatory(choicobj)) {
	    /* error missing choice */
	    res = ERR_NCX_MISSING_CHOICE;
	    if (msg) {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, choicobj, 
				 NCX_NT_VAL, val);
	    }
	}
	return res;
    }

    /* else a choice was selected
     * first make sure all the mandatory case 
     * objects are present
     */
    res = instance_check(scb, msg, chval->casobj, val, layer);
    /* errors already recorded if other than NO_ERR */

    /* check if any objects from other cases are present */
    testval = val_get_choice_next_set(val, choicobj, chval);
    while (testval) {
	if (testval->casobj != chval->casobj) {
	    /* error: extra case object in this choice */
	    res = ERR_NCX_EXTRA_CHOICE;
	    if (msg) {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, choicobj, 
				 NCX_NT_VAL, testval);
	    }
	}
	testval = val_get_choice_next_set(val, choicobj, testval);
    }

    val->res = res;

    return res;

}  /* choice_check_agt */


/********************************************************************
* FUNCTION must_stmt_check
* 
* Check for any must-stmts in the object tree and validate the Xpath
* expression against the complete database 'root' and current
* context node 'curval'
* 
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   root == val_value_t or the target database root to validate
*   curval == val_value_t for the current context node in the tree
*   layer == NCX layer calling this function (for error purposes only)
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    must_stmt_check (ses_cb_t *scb,
		     xml_msg_hdr_t *msg,
		     val_value_t *root,
		     val_value_t *curval,
		     ncx_layer_t   layer)
{
    const obj_template_t  *obj;
    const dlq_hdr_t       *mustQ;
    const ncx_errinfo_t   *errinfo;
    val_value_t           *chval;
    status_t               res, retres;

#ifdef AGT_VAL_DEBUG
    log_debug2("\nmst_stmt_check: %s start", curval->name);
#endif

    retres = NO_ERR;

    obj = curval->obj;

    for (chval = val_get_first_child(curval);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {

	res = must_stmt_check(scb, msg, root, chval, layer);
	CHK_EXIT;
    }

    mustQ = obj_get_mustQ(obj);
    if (mustQ) {
	for (errinfo = (const ncx_errinfo_t *)dlq_firstEntry(mustQ);
	     errinfo != NULL;
	     errinfo = (const ncx_errinfo_t *)dlq_nextEntry(errinfo)) {
	    /*** agt_xpath_eval(scb, msg, root, curval, errinfo, layer);  ***/
	}

    }

    return retres;
    
}  /* must_stmt_check */


/******************* E X T E R N   F U N C T I O N S ***************/


/********************************************************************
* FUNCTION agt_val_instance_check
* 
* Check for the proper number of object instances for
* the specified value struct.
* 
* The top-level value set passed cannot represent a choice
* or a case within a choice.
*
* This function is intended for validating PDUs (RPC requests)
* during the PDU processing.  It does not check the instance
* count or must-stmt expressions for any <config> (ncx:root)
* container.  This must be dome with the agt_val_root_check function.
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   valset == val_value_t list, leaf-list, or container to check
*   layer == NCX layer calling this function (for error purposes only)
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
status_t 
    agt_val_instance_check (ses_cb_t *scb,
			    xml_msg_hdr_t *msg,
			    val_value_t *valset,
			    ncx_layer_t   layer)
{
    const obj_template_t  *obj, *chobj;
    val_value_t           *chval;
    status_t               res, retres;

#ifdef DEBUG
    if (!valset) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef AGT_VAL_DEBUG
    log_debug2("\nagt_val_instchk: %s start", valset->name);
#endif

    retres = NO_ERR;

    obj = valset->obj;

    if (obj_is_cli(obj) || obj_is_abstract(obj)) {
	return NO_ERR;
    }

    if (obj->objtype == OBJ_TYP_LEAF_LIST) {
	retres = instance_check(scb, msg, obj, valset, layer);
    } else if (val_child_cnt(valset)) {
	for (chval = val_get_first_child(valset);
	     chval != NULL;
	     chval = val_get_next_child(chval)) {

	    if (obj_is_root(chval->obj)) {
		continue;
	    } else if (chval->obj->objtype != OBJ_TYP_LEAF) {
		res = agt_val_instance_check(scb, msg, chval, layer);
		CHK_EXIT;
	    }
	}
    }

    /* check all the child nodes for correct number of instances */
    for (chobj = obj_first_child(obj);
	 chobj != NULL;
	 chobj = obj_next_child(chobj)) {
	
	if (obj_is_root(chobj)) {
	    continue;
	}

	if (chobj->objtype == OBJ_TYP_CHOICE) {
	    res = choice_check_agt(scb, msg, chobj, valset, layer);
	} else {
	    res = instance_check(scb, msg, chobj, valset, layer);
	}
	if (res != NO_ERR) {
	    valset->res = res;
	}
	CHK_EXIT;
    }

    return retres;
    
}  /* agt_val_instance_check */


/********************************************************************
* FUNCTION agt_val_root_check
* 
* Check for the proper number of object instances for
* the specified configuration database
* 
* The top-level value set passed cannot represent a choice
* or a case within a choice.
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   root == val_value_t for the target config being checked
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
status_t 
    agt_val_root_check (ses_cb_t *scb,
			xml_msg_hdr_t *msg,
			val_value_t *root)
{
    const ncx_module_t    *mod;
    const obj_template_t  *obj, *chobj;
    val_value_t           *chval;
    status_t               res, retres;

#ifdef DEBUG
    if (!root) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(root->obj)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    retres = NO_ERR;

    obj = root->obj;

    /* check the instance counts for the subtrees that are present */
    res = agt_val_instance_check(scb, msg, root, NCX_LAYER_CONTENT);
    CHK_EXIT;

    /* check the must-stmt expressions for the subtrees that are present */
    for (chval = val_get_first_child(root);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {

	res = must_stmt_check(scb, msg, root, chval, NCX_LAYER_CONTENT);
	CHK_EXIT;
    }

    /* check all the modules in the system for top-level objects and
     * check the instance count and any missing mandatory top-level nodes
     * this is CPU intensive if the agent has a lot of objects
     *
     */
    for (mod = ncx_get_first_module();
	 mod != NULL;
	 mod = ncx_get_next_module(mod)) {

	/* hack: skip the NCX extensions module that defines objects
	 * just for the XSD generation usage
	 */
	if (mod->nsid == xmlns_ncx_id()) {
	    continue;
	}

	for (chobj = ncx_get_first_object(mod);
	     chobj != NULL;
	     chobj = ncx_get_next_object(mod, chobj)) {

	    if (obj_is_cli(chobj) || obj_is_abstract(chobj)) {
		continue;
	    }

	    if (chobj->objtype == OBJ_TYP_CHOICE) {
		res = choice_check_agt(scb, msg, chobj, 
				       root, NCX_LAYER_CONTENT);
	    } else {
		res = instance_check(scb, msg, chobj, 
				     root, NCX_LAYER_CONTENT);
	    }
	    CHK_EXIT;
	}
    }

    return retres;
    
}  /* agt_val_root_check */


/********************************************************************
* FUNCTION agt_val_split_root_check
* 
* Check for the proper number of object instances for
* the specified configuration database.  Conceptually
* combine the newroot and root and check that.
* 
* The top-level value set passed cannot represent a choice
* or a case within a choice.
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   newroot == val_value_t for the edit-config config contents
*   root == val_value_t for the target config being checked
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
status_t 
    agt_val_split_root_check (ses_cb_t *scb,
			      xml_msg_hdr_t *msg,
			      val_value_t *newroot,
			      val_value_t *root)
{

    return ERR_NCX_OPERATION_NOT_SUPPORTED;

#if 0
    const obj_template_t  *obj, *chobj;
    val_value_t           *chval;
    status_t               res, retres;

#ifdef DEBUG
    if (!newroot || !root) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(root->obj)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    retres = NO_ERR;

    obj = root->obj;

    /* check the instance counts for the subtrees that are present */
    res = agt_val_instance_check(scb, msg, root, NCX_LAYER_CONTENT);
    CHK_EXIT;

    /* check the must-stmt expressions for the subtrees that are present */
    for (chval = val_get_first_child(root);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {

	res = must_stmt_check(scb, msg, root, chval, NCX_LAYER_CONTENT);
	CHK_EXIT;
    }

    /* check all the modules in the system for top-level objects and
     * check the instance count and any missing mandatory top-level nodes
     * this is CPU intensive if the agent has a lot of objects
     *
     */
    for (mod = ncx_get_first_module();
	 mod != NULL;
	 mod = ncx_get_next_module(mod)) {

	for (chobj = ncx_get_first_object(mod);
	     chobj != NULL;
	     chobj = ncx_get_next_object(mod, chobj)) {

	    if (chobj->objtype == OBJ_TYP_CHOICE) {
		res = choice_check_agt(scb, msg, chobj, root, layer);
	    } else {
		res = instance_test(scb, msg, chobj, root, layer);
	    }
	    CHK_EXIT;
	}
    }

    return retres;
#endif
    
}  /* agt_val_split_root_check */


/********************************************************************
* FUNCTION agt_val_validate_write
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
* step through all the child nodes and traverse the
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
*   valroot == the val_value_t struct containing the root
*              (NCX_BT_CONTAINER, ncx:root)
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
    agt_val_validate_write (ses_cb_t  *scb,
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
    if (!obj_is_root(valroot->obj)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }	
#endif

    /* check the lock first */
    res = cfg_ok_to_write(target, scb->sid);
    if (res != NO_ERR) {
	agt_record_error(scb, &msg->mhdr, NCX_LAYER_CONTENT, res, NULL,
			 NCX_NT_NONE, NULL, 
			 NCX_NT_VAL, valroot);
	return res;
    }

    /* the <config> root is just a value node of type 'root'
     * traverse all nodes and check the <edit-config> request
     */
    res = handle_callback(AGT_CB_VALIDATE, editop, scb, 
			  msg, target, valroot, target->root);

    return res;

}  /* agt_val_validate_write */


/********************************************************************
* FUNCTION agt_val_apply_write
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
    agt_val_apply_write (ses_cb_t  *scb,
			 rpc_msg_t  *msg,
			 cfg_template_t *target,
			 val_value_t    *pducfg,
			 op_editop_t  editop,
			 op_errop_t  errop)
{
#if 0
    const obj_template_t *obj;
    ncx_node_t            dtyp;
#endif
    status_t              res;


#ifdef DEBUG
    if (!scb || !msg || !target || !pducfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(pducfg->obj)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }	
#endif

    /* check the lock first */
    res = cfg_ok_to_write(target, scb->sid);
    if (res != NO_ERR) {
	agt_record_error(scb, &msg->mhdr, NCX_LAYER_CONTENT, 
			 res, NULL,
			 NCX_NT_NONE, NULL, 
			 NCX_NT_VAL, pducfg);
	return res;
    }

#if 0
    /* make sure the target root is not NULL */
    if (!target->root) {
	dtyp = NCX_NT_OBJ;
	obj = (const obj_template_t *)
	    def_reg_find_moddef(NC_MODULE, NCX_EL_CONFIG, &dtyp);
	if (!obj) {
	    return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
	}
							  
	target->root = val_new_value();
	if (!target->root) {
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(target->root, obj);
    }
#endif

    /* start with the config root, which is a val_value_t node */
    res = handle_callback(AGT_CB_APPLY, editop, scb, 
			  msg, target, pducfg, target->root);

    /* check if undo was in effect */
    if (msg->rpc_need_undo) {
	if (res==NO_ERR) {
	    /* complete the operation */
	    res = handle_callback(AGT_CB_COMMIT, editop, scb, 
				  msg, target, pducfg, target->root);
	} else {
	    /* rollback the operation */
	    res = handle_callback(AGT_CB_ROLLBACK, editop, scb, 
				  msg, target, pducfg, target->root);
	}
    }  /* else there was no rollback, so APPLY is the final phase */


    /* handle the LOAD operation here based on start state edit mode */
    if (editop==OP_EDITOP_LOAD && (res==NO_ERR || errop==OP_ERROP_CONTINUE)) {
#if 0
	/* fast-track API for the internal load command 
	 * The pducfg struct will be handed off to cfg->root
	 */
	res = cfg_load_root(target);
	if (res != NO_ERR) {
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_OPERATION, res, NULL,
			     NCX_NT_NONE, NULL, NCX_NT_VAL, pducfg);
	}
#endif
    }

    return res;

}  /* agt_val_apply_write */

#if 0
/********************************************************************
* FUNCTION metaerr_count
* 
* Count the number of the specified meta error records
*
* INPUTS:
*     val == value to check
*     nsid == mamespace ID to match against
*     name == attribute name to match against
*
* RETURNS:
*     number of matching records found
*********************************************************************/
static uint32
    metaerr_count (const val_value_t *val,
		   xmlns_id_t  nsid,
		   const xmlChar *name)
{
    const val_metaerr_t *merr;
    uint32               cnt;

    cnt = 0;
    for (merr = (const val_metaerr_t *)dlq_firstEntry(&val->metaerrQ);
	 merr != NULL;
	 merr = (const val_metaerr_t *)dlq_nextEntry(merr)) {
	if (xml_strcmp(merr->name, name)) {
	    continue;
	}
	if (nsid) {
	    if (merr->nsid == nsid) {
		cnt++;
	    }
	} else {
	    cnt++;
	}
    }
    return cnt;

} /* metaerr_count */


/********************************************************************
* FUNCTION match_metaval
* 
* Match the specific attribute value and namespace ID
*
* INPUTS:
*     attr == attr to check
*     nsid == mamespace ID to match against
*     name == attribute name to match against
*
* RETURNS:
*     TRUE if attr is a match; FALSE otherwise
*********************************************************************/
static boolean
    match_metaval (const xml_attr_t *attr,
		   xmlns_id_t  nsid,
		   const xmlChar *name)
{
    if (xml_strcmp(attr->attr_name, name)) {
	return FALSE;
    }
    if (attr->attr_ns) {
	return (attr->attr_ns==nsid);
    } else {
	/* unqualified match */
	return TRUE;
    }
} /* match_metaval */


/********************************************************************
* FUNCTION clean_metaerrs
* 
* Clean the val->metaerrQ
*
* INPUTS:
*     val == value to check
*
*********************************************************************/
static void
    clean_metaerrs (val_value_t *val)
{
    val_metaerr_t *merr;

    while (!dlq_empty(&val->metaerrQ)) {
	merr = (val_metaerr_t *)dlq_deque(&val->metaerrQ);
	val_free_metaerr(merr);
    }
} /* clean_metaerrs */


/********************************************************************
* FUNCTION metadata_inst_check
* 
* Validate that all the XML attributes in the specified 
* xml_node_t struct are pesent in appropriate numbers
*
* Since attributes are unordered, they all have to be parsed
* before they can be checked for instance count
*
* INPUTS:
*     scb == session control block
*     msg == incoming RPC message
*            Errors are appended to msg->errQ
*     val == value to check for metadata errors
*     
* OUTPUTS:
*    msg->errQ may be appended with new errors or warnings
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    metadata_inst_check (ses_cb_t *scb,
			 xml_msg_hdr_t *msg,
			 val_value_t  *val)
{
    const typ_def_t   *typdef;
    typ_child_t       *metadef;
    uint32             cnt;
    status_t           res, retres;
    boolean            first;
    xmlns_qname_t      qname;

    retres = NO_ERR;

    /* first check the inst count of the operation attribute */
    cnt = val_metadata_inst_count(val, xmlns_nc_id(), NC_OPERATION_ATTR_NAME);
    cnt += metaerr_count(val, xmlns_nc_id(), NC_OPERATION_ATTR_NAME);
    if (cnt > 1) {
	res = ERR_NCX_EXTRA_ATTR;
	agt_record_error(scb, msg, NCX_LAYER_CONTENT, res, 
			 NULL, NCX_NT_STRING, NC_OPERATION_ATTR_NAME, 
			 NCX_NT_VAL, val);
    }

    /* get the typdef for the first in the chain with 
     * some meta data defined; may be NULL, in which
     * case just the operation attribute will be checked
     */
    typdef = typ_get_cqual_typdef(val->typdef, NCX_SQUAL_META);

    /* go through the entire typdef chain checking proper
     * attribute instance count, and record errors
     */
    first = TRUE;
    while (typdef) {
	if (first) {
	    metadef = typ_first_meta(typdef);
	    first = FALSE;
	} else {
	    metadef = typ_next_meta(metadef);
	}
	if (!metadef) {
	    typdef = typ_get_cparent_typdef(typdef);
	    first = TRUE;
	} else {
	    /* got something to check 
	     * 
	     * limitation for now!!!
	     * attribute namespace must be the same as the
	     * value that holds it, except for the netconf
	     * operation attribute
	     */
	    res = NO_ERR;
	    cnt = val_metadata_inst_count(val, val->nsid, metadef->name);
	    cnt += metaerr_count(val, val->nsid, metadef->name);

	    /* check the instance qualifier from the typdef 
	     * continue the loop if there is no error
	     */
	    switch (metadef->typdef.iqual) {
	    case NCX_IQUAL_ONE:
		if (!cnt) {
		    res = ERR_NCX_MISSING_ATTR;
		} else if (cnt > 1) {
		    res = ERR_NCX_EXTRA_ATTR;
		}
		break;
	    case NCX_IQUAL_OPT:
		if (cnt > 1) {
		    res = ERR_NCX_EXTRA_ATTR;
		}
		break;
	    case NCX_IQUAL_1MORE:
		if (!cnt) {
		    res = ERR_NCX_MISSING_ATTR;
		}
		break;
	    case NCX_IQUAL_ZMORE:
		break;
	    default:
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    if (res != NO_ERR) {
		qname.nsid = val->nsid;
		qname.name = metadef->name;
		agt_record_error(scb, msg, 
				 NCX_LAYER_CONTENT, res,
				 (const xml_node_t *)val->name,
				 NCX_NT_QNAME, &qname, 
				 NCX_NT_VAL, val);
	    }
	}
    }
    return retres;

} /* metadata_inst_check */
#endif


/* END file agt_val.c */
