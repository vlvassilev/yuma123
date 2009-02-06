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

#ifndef _H_xpath
#include  "xpath.h"
#endif

#ifndef _H_xpath1
#include  "xpath1.h"
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

    if (cbtyp > AGT_CB_ROLLBACK) {
	return NO_ERR;
    }

    if (newnode) {
	val = newnode;
    } else if (curnode) {
	val = curnode;
    } else {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (val->obj->cbset) {
	cbset = val->obj->cbset;
	if (cbset->cbfn[cbtyp]) {
	    res = (*cbset->cbfn[cbtyp])
		(scb, msg, cbtyp, editop, newnode, curnode);
	    if (val->res == NO_ERR) {
		val->res = res;
	    }
	    return res;
	}
    }
    return NO_ERR;

} /* handle_user_callback */


/********************************************************************
* FUNCTION add_undo_node
* 
* Add an undo node to the msg->undoQ
*
* INPUTS:
*    msg == RPC message in progress
*    editop == edit-config operation attribute value
*    newnode == node from PDU
*    curnode == node from database (if any)
*    parentnode == parent of curnode (or would be)
*    res == result of edit operation
*    result == address of return status
* OUTPUTS:
*    rpc_undo_rec_t struct added to msg->undoQ
*   *result set to resturn status
*
* RETURNS:
*   pointer to new undo record, in case any extra_deleteQ
*   items need to be added; NULL on error
*********************************************************************/
static rpc_undo_rec_t *
    add_undo_node (rpc_msg_t *msg,
		   op_editop_t editop,
		   val_value_t *newnode,
		   val_value_t *curnode,
		   val_value_t *parentnode,
		   status_t  res,
		   status_t  *result)
{
    rpc_undo_rec_t *undo;

    /* create an undo record for this merge */
    undo = rpc_new_undorec();
    if (!undo) {
	*result = ERR_INTERNAL_MEM;
	return NULL;
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
	    *result = ERR_INTERNAL_MEM;
	    return NULL;
	}
    }
    undo->parentnode = parentnode;
    undo->res = res;

    dlq_enque(undo, &msg->rpc_undoQ);
    *result = NO_ERR;
    return undo;

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
	    /* if this is a leaf and not an index leaf, then
	     * apply the merge here
	     */
	    if (curnode && !curnode->index) {
		retval = typ_is_simple
		    (obj_get_basetype(curnode->obj));
	    }
	}
	break;
    case OP_EDITOP_COMMIT:
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
* FUNCTION add_child
* 
* Add a child node
*
* INPUTS:
*   child == child value to add
*   parent == parent value to add child to
*   undo == undo record in progress (may be NULL)
*
* OUTPUTS:
*    child added to parent->v.childQ
*    any other cases removed and either added to undo node or deleted
*
*********************************************************************/
static void
    add_child_node (val_value_t  *child,
		    val_value_t  *parent,
		    rpc_undo_rec_t *undo)
{
    val_value_t  *val;
    dlq_hdr_t     cleanQ;

    log_debug3("\nAdd child '%s' to parent '%s'",
	       child->name, parent->name);

    dlq_createSQue(&cleanQ);

    val_add_child_clean(child, parent, &cleanQ);
    if (undo) {
	dlq_block_enque(&cleanQ, &undo->extra_deleteQ);
    } else {
	while (!dlq_empty(&cleanQ)) {
	    val = (val_value_t *)dlq_deque(&cleanQ);
	    val_free_value(val);
	}
    }

    
}  /* add_child_node */


/********************************************************************
* FUNCTION check_insert_attr
* 
* Check the YANG insert attribute
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   newval == val_value_t from the PDU
*   
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_insert_attr (ses_cb_t  *scb,
		       rpc_msg_t  *msg,
		       val_value_t  *newval)
{
    val_value_t     *testval, *simval;
    const xmlChar   *modname, *badval;
    status_t         res;

    res = NO_ERR;
    badval = NULL;
    modname = obj_get_mod_name(newval->obj);

    if (newval->editop == OP_EDITOP_DELETE) {
	/* this error already checked in agt_val_parse */
	return NO_ERR;
    }

    if (newval->obj->objtype==OBJ_TYP_LEAF_LIST) {

	/* OK to check insertstr, otherwise errors
	 * should already be recorded by agt_val_parse
	 */
	if (!newval->insertstr) {
	    /* insert op already checked in agt_val_parse */
	    return NO_ERR;
	}

	/* make sure the insert attr is on a node with a parent */
	if (!newval->curparent) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	} else {
	    /* validate the insert string against siblings */
	    testval = 
		val_find_child(newval->curparent,
			       modname, newval->name);
	    if (!testval) {
		res = ERR_NCX_INSERT_MISSING_INSTANCE;
	    } else if (!ncx_valid_name(newval->insertstr,
				       xml_strlen(newval->insertstr))) {
		res = ERR_NCX_INVALID_VALUE;
		badval = newval->insertstr;
	    } else {
		/* make a value node to compare in the
		 * value space instead of the lexicographical space
		 */
		simval = val_make_simval(newval->typdef,
					 newval->nsid,
					 newval->name,
					 newval->insertstr,
					 &res);
		if (res != NO_ERR) {
		    badval = newval->insertstr;
		} else {
		    testval = 
			val_first_child_match(newval->curparent,
					      simval);
		    if (!testval) {
			/* sibling leaf-list with the specified
			 * value was not found
			 */
			res = ERR_NCX_INSERT_MISSING_INSTANCE;
		    }
		}
		    
		if (simval) {
		    val_free_value(simval);
		}
	    }
	}
    } else if (newval->obj->objtype == OBJ_TYP_LIST) {
	/***/;
    } else {
	return NO_ERR;
    }

	     
    /* record any errors so far */
    if (res != NO_ERR) {
	if (badval) {
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT,
			     res, NULL,
			     NCX_NT_STRING, badval,
			     NCX_NT_VAL, newval);
	} else {
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT,
			     res, NULL, 
			     NCX_NT_VAL, newval, 
			     NCX_NT_VAL, newval);
	}
    }

    return res;

}  /* check_insert_attr */


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
    const xmlChar   *name;
    rpc_undo_rec_t   *undo;
    status_t          res;
    op_editop_t       cur_editop;
    boolean           applyhere, mergehere, freenew;
    

    res = NO_ERR;
    mergehere = FALSE;
    freenew = FALSE;
    undo = NULL;

    if (newval) {
	cur_editop = newval->editop;
	name = newval->name;
    } else if (curval) {
	cur_editop = editop;
	name = curval->name;
    } else {
	*done = TRUE;
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

#ifdef AGT_VAL_DEBUG
	log_debug3("\napply_write_val: %s start", name);
#endif

    /* check if this node needs the edit operation applied */
    if (*done) {
	applyhere = FALSE;
    } else if (editop == OP_EDITOP_COMMIT) {
	applyhere = val_get_dirty_flag(newval);
	*done = applyhere;
    } else if (editop == OP_EDITOP_DELETE) {
	applyhere = TRUE;
	*done = TRUE;
    } else {
	applyhere = apply_this_node(cur_editop, curval);
	*done = applyhere;
    }

    /* apply the requested edit operation */
    if (applyhere) {

#ifdef AGT_VAL_DEBUG
	log_debug2("\napply_write_val: %s applyhere", name);
#endif

	if (msg->rpc_need_undo) {
	    undo = add_undo_node(msg, editop, newval,
				 curval, parent, NO_ERR, &res);
	    if (res != NO_ERR) {
		return res;
	    }
	}

	if (target->cfg_id == NCX_CFGID_RUNNING) {
	    handle_audit_record(cur_editop, scb, target, 
				(curval) ? curval : newval, res);
	}

	if (editop != OP_EDITOP_LOAD) {
	    cfg_set_dirty_flag(target);

	    if (editop == OP_EDITOP_DELETE) {
		if (parent) {
		    val_set_dirty_flag(parent);
		}
	    } else if (curval) {
		val_set_dirty_flag(curval);
	    } else if (parent) {
		val_set_dirty_flag(parent);
	    }
	}

	/* make sure the node is not a virtual value */
	if (curval && val_is_virtual(curval)) {
	    return NO_ERR;   /*** freenew?? ***/
	}

	switch (cur_editop) {
	case OP_EDITOP_MERGE:
	    val_remove_child(newval);
	    if (curval) {
		freenew = val_merge(newval, curval);
	    } else {
		add_child_node(newval, parent, undo);
		if (target->cfg_id == NCX_CFGID_RUNNING) { 
		    val_clear_editvars(newval);
		}
	    }

	    /**** NEEDS OPTIMIZED INSERT : TEMP REORDER ****/
	    if (!freenew) {
		val_set_canonical_order(parent);
	    }
	    break;
	case OP_EDITOP_REPLACE:
	case OP_EDITOP_COMMIT:
	    val_remove_child(newval);
	    if (curval) {
		val_set_canonical_order(newval);
		val_swap_child(newval, curval);
		if (!msg->rpc_need_undo) {
		    val_free_value(curval);
		} /* else curval not freed yet, hold in undo record */
	    } else {
		add_child_node(newval, parent, undo);
		if (target->cfg_id == NCX_CFGID_RUNNING) { 
		    val_clear_editvars(newval);
		}

		/**** NEEDS OPTIMIZED INSERT : TEMP REORDER ****/
		val_set_canonical_order(parent);
	    }
	    break;
	case OP_EDITOP_CREATE:
	    val_remove_child(newval);
	    add_child_node(newval, parent, undo);
	    if (target->cfg_id == NCX_CFGID_RUNNING) { 
		val_clear_editvars(newval);
	    }

	    /**** NEEDS OPTIMIZED INSERT : TEMP REORDER ****/
	    val_set_canonical_order(parent);
	    break;
	case OP_EDITOP_LOAD:
	    val_remove_child(newval);
	    val_set_canonical_order(newval);

	    /*** DOES NOT ALLOW FOR NESTED LOAD OPERATIONS
	     *** such as loading a module at runtime with an
	     *** augment of a nested object as the root
	     ***/
	    res = cfg_apply_load_root(target, newval);
	    break;
	case OP_EDITOP_DELETE:
	    if (curval) {
		val_remove_child(curval);
		if (!msg->rpc_need_undo) {
		    val_free_value(curval);
		} /* else curval not freed yet, hold in undo record */
	    }
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    if (freenew) {
	val_free_value(newval);
    }

    return res;

}  /* apply_write_val */


/********************************************************************
* FUNCTION test_apply_write_val
* 
* Execute the AGT_CB_TEST_APPLY phase
*
* INPUTS:
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
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    test_apply_write_val (val_value_t  *parent,
			  val_value_t  *newval,
			  val_value_t  *curval,
			  boolean      *done)
{
    val_value_t     *testval;
    status_t         res;
    boolean          applyhere, mergehere, freenew;
    
    res = NO_ERR;
    mergehere = FALSE;
    freenew = FALSE;

    /* check if this node needs the edit operation applied */
    if (*done) {
	applyhere = FALSE;
    } else if (newval->editop == OP_EDITOP_COMMIT) {
	applyhere = val_get_dirty_flag(newval);
	*done = applyhere;
    } else {
	applyhere = apply_this_node(newval->editop, curval);
	*done = applyhere;
    }

    /* apply the requested edit operation */
    if (applyhere) {

#ifdef AGT_VAL_DEBUG
	log_debug3("\ntest_apply_write_val: %s start", newval->name);
#endif

	/* make sure the node is not a virtual value */
	if (curval && val_is_virtual(curval)) {
	    return NO_ERR;   /*** freenew?? ***/
	}

	switch (newval->editop) {
	case OP_EDITOP_MERGE:
	    testval = val_clone(newval);
	    if (!testval) {
		res = ERR_INTERNAL_MEM;
	    } else {
		if (curval) {
		    freenew = val_merge(testval, curval);
		} else {
		    add_child_node(testval, parent, NULL);
		}
	    }

	    /**** NEEDS OPTIMIZED INSERT : TEMP REORDER ****/
	    if (!freenew) {
		; // val_set_canonical_order(parent);
	    }
	    break;
	case OP_EDITOP_REPLACE:
	case OP_EDITOP_COMMIT:
	    testval = val_clone(newval);
	    if (!testval) {
		res = ERR_INTERNAL_MEM;
	    } else {
		if (curval) {
		    // val_set_canonical_order(testval);
		    val_swap_child(testval, curval);
		    val_free_value(curval);
		} else {
		    add_child_node(testval, parent, NULL);
		    
		    /**** NEEDS OPTIMIZED INSERT : TEMP REORDER ****/
		    // val_set_canonical_order(parent);
		}
	    }
	    break;
	case OP_EDITOP_CREATE:
	    testval = val_clone(newval);
	    if (!testval) {
		res = ERR_INTERNAL_MEM;
	    } else {
		add_child_node(testval, parent, NULL);

		/**** NEEDS OPTIMIZED INSERT : TEMP REORDER ****/
		// val_set_canonical_order(parent);
	    }
	    break;
	case OP_EDITOP_LOAD:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    break;
	case OP_EDITOP_DELETE:
	    if (curval) {
		val_remove_child(curval);
		val_free_value(curval);
	    }
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    } /* else ignore metadata merge */

    if (freenew) {
	val_free_value(testval);
    }

    return res;

}  /* test_apply_write_val */


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
    val_value_t     *curparent;
    status_t         res;
    ncx_iqual_t      iqual;
    op_editop_t      cureditop;

    res = NO_ERR;

    /* check the 'operation' attribute in VALIDATE phase */
    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef AGT_VAL_DEBUG
	log_debug3("\ninvoke_simval:validate: %s start", newval->name);
#endif

	/* check and adjust the operation attribute */
	iqual = val_get_iqualval(newval);
	res = agt_check_editop(editop, &newval->editop, 
			       newval, curval, iqual);

	/* check the operation against the object definition
	 * and whether or not the entry currently exists
	 */
	if (res == NO_ERR) {
	    res = agt_check_max_access(newval->editop, 
				       obj_get_max_access(newval->obj), 
				       (curval != NULL));
	}

	/* check the insert operation, if any */
	if (res == NO_ERR) {
	    res = check_insert_attr(scb, msg, newval);
	    /* any errors already recorded */
	} else {
	    /* record error that happened above */
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_CONTENT, res, 
			     NULL, NCX_NT_VAL, newval, 
			     NCX_NT_VAL, newval);
	}
	break;
    case AGT_CB_TEST_APPLY:
	if (newval) {
	    curparent = newval->curparent;
	} else if (curval) {
	    curparent = curval->parent;
	} else {
	    curparent = NULL;
	}
	res = test_apply_write_val(curparent, newval, 
				   curval, &done);
	break;
    case AGT_CB_APPLY:
	if (newval) {
	    curparent = newval->curparent;
	    cureditop = newval->editop;
	} else {
	    curparent = NULL;
	    cureditop = editop;
	    if (curval) {
		curparent = curval->parent;
		if (cureditop == OP_EDITOP_NONE) {
		    cureditop = curval->editop;
		}
	    }
	}
	res = apply_write_val(cureditop, scb, msg, target, 
			      curparent, newval, curval, &done);
	break;
    case AGT_CB_COMMIT:
    case AGT_CB_ROLLBACK:
	break;
    default:
	/* nothing to do for commit or rollback at this time */
	;  
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
    val_value_t      *chval, *curch, *nextch, *curparent;
    status_t          res, retres;
    ncx_iqual_t       iqual;
    op_editop_t       cur_editop;
    boolean           initialdone;

    retres = NO_ERR;
    initialdone = done;

    /* check the 'operation' attribute in VALIDATE phase */
    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef AGT_VAL_DEBUG
    log_debug3("\ninvoke_cpxval:validate: %s start", newval->name);
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
    case AGT_CB_TEST_APPLY:
	retres = test_apply_write_val(newval->curparent, 
				      newval, curval, &done);
	break;
    case AGT_CB_APPLY:
	if (newval) {
	    cur_editop = newval->editop;
	    curparent = newval->curparent;
	} else if (curval) {
	    cur_editop = editop;
	    curparent = curval->parent;
	} else {
	    retres = SET_ERROR(ERR_INTERNAL_VAL);
	}

	if (retres == NO_ERR) {
	    retres = apply_write_val(cur_editop, scb, msg, target,
				     curparent, newval, 
				     curval, &done);
	}
	break;
    case AGT_CB_COMMIT:
    case AGT_CB_ROLLBACK:
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (newval) {
	cur_editop = newval->editop;
	curparent = newval;
    } else if (curval) {
	cur_editop = editop;
	curparent = curval;
    } else {
	retres = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check all the child nodes next */
    if (retres == NO_ERR && !done && curparent) {
	for (chval = val_get_first_child(curparent);
	     chval != NULL && retres == NO_ERR;
	     chval = nextch) {

	    nextch = val_get_next_child(chval);

	    chval->curparent = curval;
	    if (curval) {
		curch = val_first_child_match(curval, chval);
	    } else {
		curch = NULL;
	    }
	    res = invoke_btype_cb(cbtyp, cur_editop, scb, msg, 
				  target, chval, curch, FALSE);
	    if (chval->res == NO_ERR) {
		chval->res = res;
	    }
	    if (res != NO_ERR) {
		retres = res;
	    }
	}
    }

    /* check if the typdef for this value has a callback
     * only call if the operation was applied here
     */
    if (retres == NO_ERR && !initialdone && done) {
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
*          == NULL if cbtyp == AGT_CB_TEST_APPLY
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
    val_value_t   *v_val;
    status_t       res;
    obj_type_t     objtype;
    
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

    if (newval && newval->obj) {
	objtype = newval->obj->objtype;
    } else if (curval && curval->obj) {
	objtype = curval->obj->objtype;
    } else {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* first traverse all the nodes until leaf nodes are reached */
    switch (objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	res = invoke_simval_cb(cbtyp, editop, scb, msg,
			       target, newval, 
			       (v_val) ? v_val : curval, done);
	break;
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
	val_remove_child(undo->newnode);
	val_free_value(undo->newnode);
	undo->newnode = NULL;
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
	add_child_node(undo->curnode, undo->parentnode, NULL);
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
	    val_remove_child(undo->newnode);
	    if (undo->curnode) {
		val_swap_child(undo->curnode, undo->newnode);
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
		    val_free_value(undo->curnode);
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

    switch (cbtyp) {
    case AGT_CB_VALIDATE:
    case AGT_CB_APPLY:
    case AGT_CB_TEST_APPLY:
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


#ifdef NOT_YET
/********************************************************************
* FUNCTION unique_check
* 
* Check for the proper number of unique tuples within
* the list instances for the specified list entry
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   obj == object template for child node in valset to check
*          This must represent an OBJ_TYP_LIST object
*          If the unique Q is empty, return NO_ERR
*   valset == val_value_t of the parent of the 'obj' child type
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
    unique_check (ses_cb_t *scb,
		  xml_msg_hdr_t *msg,
		  const obj_template_t *obj,
		  val_value_t *valset,
		  ncx_layer_t   layer)
{
    const obj_template_t  *chobj;
    const obj_unique_t    *uni;
    const xmlChar         *modname, objname;
    val_value_t           *compset1, *compset2, *startchild, *curchild;
    uint32                 uninum, listcnt;
    status_t               res, retres;

    uni = obj_get_first_unique(obj);
    if (!uni) {
	return NO_ERR;
    }

    listcnt = val_child_inst_cnt(valset,
				 obj_get_mod_name(obj),
				 obj_get_name(obj));
    if (listcnt < 2) {
	return NO_ERR;
    }

    compset1 = NULL;
    compset2 = NULL;
    res = NO_ERR;
    retres = NO_ERR;
    uninum = 1;
    modname = obj_get_mod_name(obj);
    objname = obj_get_name(obj);

    compset1 = val_new_value();
    if (!compset) {
	res = ERR_INTERNAL_MEM;
    } else {
	val_init_from_template(compset1, obj);
    }

    compset2 = val_new_value();
    if (!compset) {
	res = ERR_INTERNAL_MEM;
    } else {
	val_init_from_template(compset2, obj);
    }

    if (res != NO_ERR) {
	agt_record_error(scb, msg, layer, res, 
			 NULL, NCX_NT_OBJ, obj, 
			 NCX_NT_VAL, valset);
	retres = res;
	uni = NULL;
    }

    /* go through each unique test for the list
     * they do not have names, so just give them numbers
     * in the unique test 'bad-value' clause
     */
    while (uni) {
	res = NO_ERR;
	cnt = 0;

#ifdef AGT_VAL_DEBUG
	log_debug3("\nunique_check '%s' against '%s'",
		   objname, valset->name);
#endif

	/* save the values in a temp list entry of the same type as
	 * the real list being checked.
	 */
	firstchild = val_find_child(valset, modname, objname);



	if (res != NO_ERR) {
	    agt_record_error(scb, msg, layer, res, 
			     NULL, NCX_NT_OBJ, obj, 
			     NCX_NT_VAL, val);
	}

	uni = obj_next_unique(uni);
	uninum++;
    }


    if (compset1) {
	val_free_value(compset1);
    }
    if (compset2) {
	val_free_value(compset1);
    }

    return res;
    
}  /* unique_check */
#endif


/********************************************************************
* FUNCTION instance_check
* 
* Check for the proper number of object instances for
* the specified value struct. Checks the direct accessible
* children of 'val' only!!!
* 
* Also check 'require-instance' for the NCX_BT_LEAFREF
* and NCX_BT_INSTANCEID data types
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
*   root == config root for 'val'
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
		    val_value_t *root,
		    ncx_layer_t layer)
{
    val_value_t         *errval;
    xmlChar             *instbuff;
    xpath_result_t      *result;
    const xpath_pcb_t   *xpcb;
    xpath_pcb_t         *xpcbclone;
    const ncx_errinfo_t *errinfo;
    const typ_def_t     *typdef;
    ncx_iqual_t          iqual;
    uint32               cnt, i, minelems, maxelems;
    boolean              minset, maxset, minerr, maxerr;
    boolean              constrained, fnresult;
    status_t             res, res2;
    char                 buff[NCX_MAX_NUMLEN];

    res = NO_ERR;
    res2 = NO_ERR;
    errinfo = NULL;
    iqual = obj_get_iqualval(obj);
    minerr = FALSE;
    maxerr = FALSE;
    minelems = 0;
    maxelems = 0;
    minset = obj_get_min_elements(obj, &minelems);
    maxset = obj_get_max_elements(obj, &maxelems);

    cnt = val_instance_count(val, obj_get_mod_name(obj),
			     obj_get_name(obj));

#ifdef AGT_VAL_DEBUG
    if (LOGDEBUG3) {
	if (!minset) {
	    switch (iqual) {
	    case NCX_IQUAL_ONE:
	    case NCX_IQUAL_1MORE:
		minelems = 1;
		break;
	    case NCX_IQUAL_ZMORE:
	    case NCX_IQUAL_OPT:
		minelems = 0;
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}

	if (!maxset) {
	    switch (iqual) {
	    case NCX_IQUAL_ONE:
	    case NCX_IQUAL_OPT:
		maxelems = 1;
		break;
	    case NCX_IQUAL_1MORE:
	    case NCX_IQUAL_ZMORE:
		maxelems = 0;
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}

	if (maxelems) {
	    sprintf(buff, "%u", maxelems);
	}

	log_debug3("\ninstance_check '%s' against '%s' "
		   "(cnt=%u, min=%u, max=%s)",
		   obj_get_name(obj), val->name, cnt, 
		   minelems, maxelems ? buff : "unbounded");
    }
#endif

    if (minset) {
	if (cnt < minelems) {
	    /* not enough instances error */
	    minerr = TRUE;
	    res = ERR_NCX_MIN_ELEMS_VIOLATION;
	    val->res = res;
	    if (cnt) {
		/* use the first child instance as the
		 * value node for the error-path
		 */
		errval = val_find_child(val,
					obj_get_mod_name(obj),
					obj_get_name(obj));
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_NONE, NULL, 
				 NCX_NT_VAL, errval);
	    } else {
		/* need to construct a string error-path */
		instbuff = NULL;
		res2 = val_gen_split_instance_id(msg, val,
						 NCX_IFMT_XPATH1,
						 obj_get_nsid(obj),
						 obj_get_name(obj),
						 &instbuff);
		if (res2 == NO_ERR) {
		    agt_record_error(scb, msg, layer, res, 
				     NULL, NCX_NT_NONE, NULL, 
				     NCX_NT_STRING, instbuff);
		} else {
		    agt_record_error(scb, msg, layer, res, 
				     NULL, NCX_NT_OBJ, obj, 
				     NCX_NT_VAL, val);
		}
		if (instbuff) {
		    m__free(instbuff);
		}
	    }
	}
    }

    if (maxset) {
	if (cnt > maxelems) {
	    maxerr = TRUE;
	    res = ERR_NCX_MAX_ELEMS_VIOLATION;
	    val->res = res;

	    /* too many instances error
	     * need to find all the extra instances
	     * and mark the extras as errors or they will
	     * not get removed later
	     */
	    val_set_extra_instance_errors(val, 
					  obj_get_mod_name(obj),
					  obj_get_name(obj),
					  maxelems);
	    /* use the first extra child instance as the
	     * value node for the error-path
	     */
	    errval = val_find_child(val,
				    obj_get_mod_name(obj),
				    obj_get_name(obj));
	    i = 1;
	    while (errval && i <= maxelems) {
		errval = val_get_next_child(errval);
		i++;
	    }
	    if (errval) {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_VAL, errval);
	    } else {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_VAL, val);
	    }
	}
    }

    switch (iqual) {
    case NCX_IQUAL_ONE:
    case NCX_IQUAL_1MORE:
	if (cnt < 1 && !minerr) {
	    /* missing single parameter */
	    res = ERR_NCX_MISSING_VAL_INST;
	    val->res = res;

	    /* need to construct a string error-path */
	    instbuff = NULL;
	    res2 = val_gen_split_instance_id(msg, val,
					     NCX_IFMT_XPATH1,
					     obj_get_nsid(obj),
					     obj_get_name(obj),
					     &instbuff);
	    if (res2 == NO_ERR) {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_STRING, instbuff);
	    } else {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_VAL, val);
	    }
	    if (instbuff) {
		m__free(instbuff);
	    }
	}
	if (iqual == NCX_IQUAL_1MORE) {
	    break;
	}
	/* else fall through */
    case NCX_IQUAL_OPT:
	if (cnt > 1 && !maxerr) {
	    /* too many parameters */
	    val_set_extra_instance_errors(val, 
					  obj_get_mod_name(obj),
					  obj_get_name(obj), 1);
	    res = ERR_NCX_EXTRA_VAL_INST;
	    val->res = res;

	    /* use the first extra child instance as the
	     * value node for the error-path
	     */
	    errval = val_find_child(val,
				    obj_get_mod_name(obj),
				    obj_get_name(obj));
	    i = 1;
	    while (errval && i < maxelems) {
		errval = val_get_next_child(errval);
		i++;
	    }
	    if (errval) {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_VAL, errval);
	    } else {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_VAL, val);
	    }
	}
	break;
    case NCX_IQUAL_ZMORE:
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
	val->res = res;
	agt_record_error(scb, msg, layer, res, 
			 NULL, NCX_NT_OBJ, obj, 
			 NCX_NT_VAL, val);

    }

    switch (val->btyp) {
    case NCX_BT_LEAFREF:
	/* do a complete parsing to retrieve the
	 * instance that matched, just checking the
	 * require-instance flag
	 */
	typdef = obj_get_ctypdef(val->obj);
	constrained = typ_get_constrained(typdef);
	xpcb = typ_get_leafref_pcb(typdef);

	if (constrained) {
	    xpcbclone = xpath_clone_pcb(xpcb);
	    if (!xpcbclone) {
		res2 = ERR_INTERNAL_MEM;
	    } 

	    if (res2 == NO_ERR) {
		result = xpath1_eval_xmlexpr
		    (scb->reader, xpcbclone, val, root,
		     FALSE, TRUE, &res2);

		if (res2 == NO_ERR) {
		    /* check result: the string value in 'val'
		     * must match one of the values in the
		     * result set
		     */
		    fnresult = xpath1_compare_result_to_string
			(xpcbclone, result, VAL_STR(val), &res2);

		    if (res2 == NO_ERR && !fnresult) {
			/* did not match any of the current instances */
			res2 = ERR_NCX_MISSING_INSTANCE;
		    }
		}

		if (result) {
		    xpath_free_result(result);
		}
	    } else {
		/* just use the leafref xrefdef */
		errinfo = NULL;
		res2 = val_simval_ok_errinfo
		    (typ_get_xref_typdef(typdef),
		     VAL_STR(val),
		     &errinfo);
	    }

	    if (xpcbclone) {
		xpath_free_pcb(xpcbclone);
	    }
	}

	if (res2 != NO_ERR) {
	    val->res = res2;
	    agt_record_error_errinfo(scb, msg, layer, res2, 
				     NULL, NCX_NT_OBJ, obj, 
				     NCX_NT_VAL, val, errinfo);
	}

	if (res == NO_ERR) {
	    res = res2;
	}
	break;
    case NCX_BT_INSTANCE_ID:
	/* do a complete parsing to retrieve the
	 * instance that matched, just checking the
	 * require-instance flag
	 */
	result = 
	    xpath1_eval_xml_instanceid(scb->reader,
				       val->xpathpcb,
				       val,
				       root,
				       FALSE,
				       &res2);
	if (result) {
	    xpath_free_result(result);
	}

	if (res2 != NO_ERR) {
	    val->res = res2;
	    agt_record_error(scb, msg, layer, res2, 
			     NULL, NCX_NT_OBJ, obj, 
			     NCX_NT_VAL, val);
	}
	if (res == NO_ERR) {
	    res = res2;
	}
	break;
    default:
	;
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
*   root == database root of 'val'
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
		      val_value_t *root,
		      ncx_layer_t   layer)
{
    const obj_template_t  *testobj;
    val_value_t           *chval, *testval;
    status_t               res, retres;

    res = NO_ERR;
    retres = NO_ERR;

#ifdef AGT_VAL_DEBUG
    log_debug3("\nichoice_check_agt: check '%s' against '%s'",
	       obj_get_name(choicobj), val->name);
#endif

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
    for (testobj = obj_first_child(chval->casobj);
	 testobj != NULL;
	 testobj = obj_next_child(testobj)) {

	res = instance_check(scb, msg, testobj, 
			     val, root, layer);
	CHK_EXIT(res, retres);
	/* errors already recorded if other than NO_ERR */
    }

    /* check if any objects from other cases are present */
    testval = val_get_choice_next_set(val, choicobj, chval);
    while (testval) {
	if (testval->casobj != chval->casobj) {
	    /* error: extra case object in this choice */
	    retres = res = ERR_NCX_EXTRA_CHOICE;
	    if (msg) {
		agt_record_error(scb, msg, layer, res, 
				 NULL, NCX_NT_OBJ, choicobj, 
				 NCX_NT_VAL, testval);
	    }
	}
	testval = val_get_choice_next_set(val, choicobj, testval);
    }

    if (val->res == NO_ERR) {
	val->res = retres;
    }

    return retres;

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
		     val_value_t *curval)
{
    const obj_template_t  *obj;
    const dlq_hdr_t       *mustQ;
    xpath_pcb_t           *must;
    xpath_result_t        *result;
    val_value_t           *chval;
    status_t               res, retres;


    retres = NO_ERR;

    obj = curval->obj;

    if (!obj_has_name(obj)) {
	return NO_ERR;
    }

    /* execute all the must tests top down, so
     * foo/bar errors are reported before /foo/bar/child
     */
    mustQ = obj_get_mustQ(obj);
    if (mustQ && !dlq_empty(mustQ)) {

	log_debug3("\nmst_stmt_check: %s start", curval->name);

	for (must = (xpath_pcb_t *)dlq_firstEntry(mustQ);
	     must != NULL;
	     must = (xpath_pcb_t *)dlq_nextEntry(must)) {

	    res = NO_ERR;
	    result = xpath1_eval_expr(must, curval, root, 
				      FALSE, TRUE, &res);
	    if (!result || res != NO_ERR) {
		log_debug2("\nagt_val: must XPath failed for "
			   "%s %s (%s)",
			   obj_get_typestr(obj),
			   obj_get_name(obj),
			   get_error_string(res));

		if (res == NO_ERR) {
		    res = SET_ERROR(ERR_INTERNAL_VAL);
		}
		agt_record_error_errinfo(scb, msg,
					 NCX_LAYER_CONTENT,
					 res, NULL,
					 NCX_NT_STRING,
					 must->exprstr,
					 NCX_NT_VAL,
					 curval,
					 (ncx_errinfo_set
					  (&must->errinfo)) ?
					 &must->errinfo : NULL);
		CHK_EXIT(res, retres);
	    } else if (!xpath_cvt_boolean(result)) {
		res = ERR_NCX_MUST_TEST_FAILED;
		agt_record_error_errinfo(scb, msg,
					 NCX_LAYER_CONTENT,
					 res,
					 NULL,
					 NCX_NT_STRING,
					 must->exprstr,
					 NCX_NT_VAL,
					 curval,
					 (ncx_errinfo_set
					  (&must->errinfo)) ?
					 &must->errinfo : NULL);
		CHK_EXIT(res, retres);
	    } else {
		log_debug2("\nmust OK '%s'", must->exprstr);
	    }

	    if (result) {
		xpath_free_result(result);
	    }

	    if (res != NO_ERR) {
		curval->res = res;
	    }
	}
    }

    /* recurse for every child node until leafs are hit */
    for (chval = val_get_first_child(curval);
	 chval != NULL && retres == NO_ERR;
	 chval = val_get_next_child(chval)) {

	res = must_stmt_check(scb, msg, root, chval);
	CHK_EXIT(res, retres);
    }

    return retres;
    
}  /* must_stmt_check */


/********************************************************************
* FUNCTION apply_commit_deletes
* 
* Apply the requested commit delete operations
*
* Invoke all the AGT_CB_COMMIT callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming commit rpc_msg_t in progress
*   target == target database (NCX_CFGID_RUNNING)
*   candval == value struct from the candidate config
*   runval == value struct from the running config
*
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added 
*   to the msg->mhsr.errQ
*
* RETURNS:
*   none
*********************************************************************/
static status_t
    apply_commit_deletes (ses_cb_t  *scb,
			  rpc_msg_t  *msg,
			  cfg_template_t *target,
			  val_value_t *candval,
			  val_value_t *runval)
{
    val_value_t      *curval, *nextval, *matchval;
    status_t          res;

    res = NO_ERR;

    /* go through running config
     * if the matching node is not in the candidate,
     * then delete that node in the running config as well
     */
    for (curval = val_get_first_child(runval);
	 curval != NULL && res == NO_ERR; 
	 curval = nextval) {

	nextval = val_get_next_child(curval);

	/* check only database config nodes */
	if (obj_is_data_db(curval->obj) &&
	    obj_is_config(curval->obj)) {

	    /* check if node deleted in source */
	    matchval = val_first_child_match(candval, curval);
	    if (!matchval) {
		/* prevent the agt_val code from ignoring this node */
		val_set_dirty_flag(curval);

		/* deleted in the source, so delete in the target */
		res = handle_callback(AGT_CB_APPLY,
				      OP_EDITOP_DELETE, scb, 
				      msg, target, 
				      NULL, curval);
	    } else {
		/* else keep this node in target config
		 * but check any child nodes for deletion
		 */
		res = apply_commit_deletes(scb, msg, target,
					   matchval, curval);
	    }
	}  /* else skip non-config database node */
    }

    return res;

}   /* apply_commit_deletes */


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
*   root == database root of 'valset'
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
			    val_value_t *root,
			    ncx_layer_t layer)
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
    log_debug3("\nagt_val_instchk: %s start", valset->name);
#endif

    retres = NO_ERR;

    obj = valset->obj;

    if (obj_is_cli(obj) || obj_is_abstract(obj)) {
	return NO_ERR;
    }

    if (val_child_cnt(valset)) {
	for (chval = val_get_first_child(valset);
	     chval != NULL;
	     chval = val_get_next_child(chval)) {

	    if (obj_is_root(chval->obj)) {
		continue;
	    } else if (chval->obj->objtype != OBJ_TYP_LEAF) {
		res = agt_val_instance_check(scb, msg, 
					     chval, root, layer);
		CHK_EXIT(res, retres);
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
	    res = choice_check_agt(scb, msg, chobj, 
				   valset, root, layer);
	} else {
	    res = instance_check(scb, msg, chobj, 
				 valset, root, layer);
	}
	if (res != NO_ERR && valset->res == NO_ERR) {	    
	    valset->res = res;
	}
	CHK_EXIT(res, retres);
    }

    return retres;
    
}  /* agt_val_instance_check */


/********************************************************************
* FUNCTION agt_val_root_check
* 
* Check for the proper number of object instances for
* the specified configuration database
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
    xmlns_id_t             ncxid;

#ifdef DEBUG
    if (!root) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(root->obj)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    retres = NO_ERR;
    ncxid = xmlns_ncx_id();
    obj = root->obj;

    /* check the instance counts for the subtrees that are present */
    res = agt_val_instance_check(scb, msg, root, 
				 root, NCX_LAYER_CONTENT);
    CHK_EXIT(res, retres);

    /* check the must-stmt expressions for the subtrees that are present */
    for (chval = val_get_first_child(root);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {

	res = must_stmt_check(scb, msg, root, chval);
	CHK_EXIT(res, retres);
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
	if (mod->nsid == ncxid) {
	    continue;
	}

	for (chobj = ncx_get_first_data_object(mod);
	     chobj != NULL;
	     chobj = ncx_get_next_data_object(mod, chobj)) {

	    if (obj_is_cli(chobj) || obj_is_abstract(chobj)) {
		continue;
	    }

	    if (chobj->objtype == OBJ_TYP_CHOICE) {
		res = choice_check_agt(scb, msg, chobj, 
				       root, root, NCX_LAYER_CONTENT);
	    } else {
		res = instance_check(scb, msg, chobj, 
				     root, root, NCX_LAYER_CONTENT);
	    }
	    CHK_EXIT(res, retres);
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
* This function is only used if the cfg target is RUNNING
* The CANDIDATE cfg should use the agt_val_root_check
* instead for a pre-commit test
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == RPC message in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   newroot == val_value_t for the edit-config config contents
*   root == val_value_t for the target config being checked
*   defop == the starting default-operation value
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
			      rpc_msg_t  *msg,
			      val_value_t *newroot,
			      val_value_t *root,
			      op_editop_t  defop)
{
    val_value_t     *copyroot;
    status_t         res;

#ifdef DEBUG
    if (!newroot || !root) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(root->obj)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* create a temporary root config clone and do
     * a test apply to that copy.  This will create
     * the proper config for must, instance, choice,
     * and unique tests.  
     *
     * Resource errors due to user callback code
     * can still happen during the real apply
     */
    res = NO_ERR;
    copyroot = val_clone_config_data(root, &res);
    if (!copyroot) {
	agt_record_error(scb, &msg->mhdr, NCX_LAYER_CONTENT,
			 ERR_NCX_OPERATION_FAILED, NULL,
			 NCX_NT_STRING, get_error_string(res), 
			 NCX_NT_VAL, root);
	return res;
    }

    /* do a dummy test apply to the partial config
     * start with the config root, which is a val_value_t node 
     */
    res = handle_callback(AGT_CB_TEST_APPLY, defop, scb, 
			  msg, NULL, newroot, copyroot);

    if (res == NO_ERR) {
	res = agt_val_root_check(scb, &msg->mhdr, copyroot);
    }

    val_free_value(copyroot);

    return res;
    
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
*          == NULL for no actual write acess (validate only)
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
    if (!scb || !msg || !valroot) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(valroot->obj)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }	
#endif

    if (target) {
	/* check the lock first */
	res = cfg_ok_to_write(target, scb->sid);
	if (res != NO_ERR) {
	    agt_record_error(scb, &msg->mhdr, NCX_LAYER_CONTENT, res, NULL,
			     NCX_NT_NONE, NULL, 
			     NCX_NT_VAL, valroot);
	    return res;
	}
    }

    /* the <config> root is just a value node of type 'root'
     * traverse all nodes and check the <edit-config> request
     */
    res = handle_callback(AGT_CB_VALIDATE, editop, scb, 
			  msg, target, valroot,
			  (target) ? target->root : NULL);

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
*
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
			 op_editop_t  editop)
{
    status_t              res;

#ifdef DEBUG
    if (!scb || !msg || !target || !pducfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(pducfg->obj)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
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

    return res;

}  /* agt_val_apply_write */


/********************************************************************
* FUNCTION agt_val_apply_commit
* 
* Apply the requested commit operation
*
* Invoke all the AGT_CB_COMMIT callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming commit rpc_msg_t in progress
*   source == cfg_template_t for the source (candidate)
*   target == cfg_template_t for the config database to 
*             write (running)
*
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added 
*   to the msg->mhsr.errQ
*
* RETURNS:
*   none
*********************************************************************/
status_t
    agt_val_apply_commit (ses_cb_t  *scb,
			  rpc_msg_t  *msg,
			  cfg_template_t *source,
			  cfg_template_t *target)
{
    val_value_t      *newval, *nextval, *matchval;
    status_t          res;

#ifdef DEBUG
    if (!scb || !msg || !source || !target) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    /* only save if the source config was touched */
    if (!cfg_get_dirty_flag(source)) {
	return NO_ERR;
    }

    /* check if any config nodes have been deleted in the target */
    res = apply_commit_deletes(scb, msg, target,
			       source->root,
			       target->root);

    /* check if any config nodes have been changed in the target */
    for (newval = val_get_first_child(source->root);
	 newval != NULL && res == NO_ERR; 
	 newval = nextval) {

	nextval = val_get_next_child(newval);

	if (obj_is_data_db(newval->obj) &&
	    obj_is_config(newval->obj)) {

	    matchval = val_first_child_match(target->root, newval);

	    newval->curparent = target->root;
	    
	    res = handle_callback(AGT_CB_APPLY,
				  OP_EDITOP_COMMIT, scb, 
				  msg, target, 
				  newval, matchval);
	}
    }

    /* check if undo was in effect */
    if (msg->rpc_need_undo) {
	if (res==NO_ERR) {
	    /* complete the operation */
	    res = handle_callback(AGT_CB_COMMIT,
				  OP_EDITOP_COMMIT, scb, 
				  msg, target, 
				  source->root,
				  target->root);
	} else {
	    /* rollback the operation */
	    res = handle_callback(AGT_CB_ROLLBACK,
				  OP_EDITOP_COMMIT, scb, 
				  msg, target, 
				  source->root,
				  target->root);
	}
    }  /* else there was no rollback, so APPLY is the final phase */

    if (res == NO_ERR) {
	res = agt_ncx_cfg_save(target, FALSE);
	if (res != NO_ERR) {
	    /* config save failed */
	    agt_record_error(scb, &msg->mhdr, 
			     NCX_LAYER_OPERATION, 
			     res, NULL, 
			     NCX_NT_CFG, target,
			     NCX_NT_NONE, NULL);
	} else {
	    val_clean_tree(target->root);
	}
    }

    return res;

}  /* agt_val_apply_commit */


/* END file agt_val.c */
