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

#ifndef _H_agt_ps_parse
#include "agt_ps_parse.h"
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

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
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
*   nodetyp == type of node of the start of imparted data
*   node == start of imparted data
*   res == result of edit operation
*
* OUTPUTS:
*   log message generated if log level set to LOG_INFO or higher
*********************************************************************/
static void
    handle_audit_record (op_editop_t editop,
			 ses_cb_t  *scb,
			 cfg_template_t *target,
			 ncx_node_t nodetyp,
			 void *node,
			 status_t res)
{
    xmlChar  *ibuff, tbuff[TSTAMP_MIN_SIZE+1];

    if (editop == OP_EDITOP_LOAD) {
	return;
    }

    ibuff = NULL;
    tstamp_datetime(tbuff);

    if (node) {
	(void)val_gen_instance_id(nodetyp, node, 
				  NCX_IFMT_XPATH1, FALSE, &ibuff);
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
*    newnodetyp == NCX node enumeration for newnode
*                 curnode must be the same type as newnode
*    newnode == new node in operation
*     curnode == current node in operation
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
			  ncx_node_t newnodetyp,
			  void *newnode,
			  void *curnode)
{
    agt_cb_pscbset_t  *pscbset;    /* parmset callback set */
    agt_cb_pcbset_t   *pcbset;     /* parm callback set */
    agt_cb_tcbset_t   *tcbset;     /* type callback set */
    ps_parmset_t      *ps;
    ps_parm_t         *parm;
    val_value_t       *val;
    status_t           res;

    switch (newnodetyp) {
    case NCX_NT_VAL:
	val = (val_value_t *)newnode;
	if (val->typdef->cbset) {
	    tcbset = val->typdef->cbset;
	    if (tcbset->tcb[cbtyp]) {
		res = (*tcbset->tcb[cbtyp])
		    (scb, msg, cbtyp, editop, newnode, curnode);
		val->res = res;
		return res;
	    }
	}
	return NO_ERR;
    case NCX_NT_PARM:
	parm = (ps_parm_t *)newnode;
	if (parm->parm->cbset) {
	    pcbset = parm->parm->cbset;
	    if (pcbset->pcb[cbtyp]) {
		res = (*pcbset->pcb[cbtyp])
		    (scb, msg, cbtyp, editop, newnode, curnode);
		parm->res = res;
		return res;
	    }
	}
	return NO_ERR;   /* did not execute a callback */
    case NCX_NT_PARMSET:
	if (newnodetyp==NCX_NT_PARMSET) {
	    ps = (ps_parmset_t *)newnode;
	}
	if (ps->psd->cbset) {
	    pscbset = ps->psd->cbset;
	    if (pscbset->pscb[cbtyp]) {
		res = (*pscbset->pscb[cbtyp])
		    (scb, msg, cbtyp, editop, newnode, curnode);
		ps->res = res;
		return res;
	    }
	}
	return NO_ERR;  /* did not execute a callback */
    case NCX_NT_APP:
	/* app nodes do not have callback handlers and there
	 * cannot be a parent callback across this application 
	 * node boundary -- just exist, no-op
	 */
	return NO_ERR;   /* did not execute a callback */
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    /*NOTREACHED*/

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
* FUNCTION add_parmnode
* 
* Add a parm node to the parent parmset node
*
* INPUTS:
*    parmnode == ps_parm_t struct to add to the parent app
*    parent == ps_parmset_t to add the parmnode as a child
*********************************************************************/
static void
    add_parmnode (ps_parm_t *parmnode,
		  ps_parmset_t *parent)
{

    parmnode->parent = parent;
    dlq_enque(parmnode, &parent->parmQ);

} /* add_parmnode */


/********************************************************************
* FUNCTION swap_parmnode
* 
* Swap a parm node in the parent parmset node
*
* INPUTS:
*    newparm == ps_parm_t struct to add to the parent parmset
*    curparm == current parm node to replace
*********************************************************************/
static void
    swap_parmnode (ps_parm_t *newparm,
		   ps_parm_t *curparm)
{
    newparm->parent = curparm->parent;
    dlq_swap(newparm, curparm);

} /* swap_parmnode */


/********************************************************************
* FUNCTION merge_parmnode
* 
* Merge a parm node to the parent parmset node
*
* INPUTS:
*    newparm == ps_parm_t struct to merge into the parent parmset
*    curparm == (first) current value found to merge with
*
* RETURNS:
*    TRUE if src should now be freed
*    FALSE if source should not be freed
*********************************************************************/
static boolean
    merge_parmnode (ps_parm_t *newparm,
		    ps_parm_t *curparm)
{
    return ps_merge_parm(newparm, curparm);

} /* merge_parmnode */


/********************************************************************
* FUNCTION add_psnode
* 
* Add a parmset node to the parent app node
*
* INPUTS:
*    psnode == ps_parmset_t struct to add to the parent app
*    parent == cfg_app_t to add the psnode as a child
*********************************************************************/
static void
    add_psnode (ps_parmset_t *psnode,
		cfg_app_t *parent)
{
    psnode->parent = parent;
    dlq_enque(psnode, &parent->parmsetQ);

} /* add_psnode */


/********************************************************************
* FUNCTION swap_psnode
* 
* Swap a parmset node in the parent app header node
*
* INPUTS:
*    newps == ps_parmset_t struct to add to the parent app
*    curps == current parmset node to replace
*********************************************************************/
static void
    swap_psnode (ps_parmset_t *newps,
		 ps_parmset_t *curps)
{
    newps->parent = curps->parent;
    dlq_swap(newps, curps);

} /* swap_psnode */


/********************************************************************
* FUNCTION merge_psnode
* 
* Merge a parmset node to the current parmset node
*
* INPUTS:
*    newps == ps_parmset_t struct to merge into the current one
*    curps == current parmset struct to merge with
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    merge_psnode (ps_parmset_t *newps,
		  ps_parmset_t *curps)
{
    return ps_merge_parmset(newps, curps);

} /* merge_psnode */


/********************************************************************
* FUNCTION add_appnode
* 
* Add an appnode to the parent value node
*
* INPUTS:
*    appnode == cfg_app_t struct to add to the parent value
*    parent == complex value node with a childQ
*********************************************************************/
static void
    add_appnode (cfg_app_t *appnode,
		 val_value_t *parent)
{
    appnode->parent = parent;
    dlq_enque(appnode, &parent->v.appQ);

} /* add_appnode */


/********************************************************************
* FUNCTION swap_appnode
* 
* Swap an appnode to the parent value node
*
* INPUTS:
*    newapp == cfg_app_t struct to add to the parent value
*    curapp == current cfg_app_t node to replace
*********************************************************************/
static void
    swap_appnode (cfg_app_t *newapp,
		  cfg_app_t *curapp)
{
    newapp->parent = curapp->parent;
    dlq_swap(newapp, curapp);

} /* swap_appnode */


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
		   ncx_node_t  newnodetyp,
		   void *newnode,
		   ncx_node_t curnodetyp,
		   void *curnode,
		   ncx_node_t parenttyp,
		   void *parentnode,
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
    undo->newnodetyp = newnodetyp;
    undo->newnode = newnode;
    undo->curnodetyp = curnodetyp;

    /* save a copy of the current value in case it gets modified
     * in a merge operation
     */
    if (curnode) {
	switch (curnodetyp) {
	case NCX_NT_VAL:
	    undo->curnode = val_clone((val_value_t *)curnode);
	    break;
	case NCX_NT_PARM:
	    undo->curnode = ps_clone_parm((ps_parm_t *)curnode);
	    break;
	default:
	    undo->curnode = curnode;
	}
    }
    undo->parentnodetyp = parenttyp;
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
*    nodetyp == type of node in 'curnode' 
*    curnode == pointer to current node (just used to check if non-NULL)
*
* RETURNS:
*    TRUE if the current node needs the write operation applied
*    FALSE if this is a NO=OP node (either explicit or special merge)
*********************************************************************/
static boolean
    apply_this_node (op_editop_t editop,
		     ncx_node_t nodetyp,
		     const void *curnode)
{
    boolean retval;
    const val_value_t *val;
    const ps_parm_t   *parm;

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
	    switch (nodetyp) {
	    case NCX_NT_PARM:
		parm = (const ps_parm_t *)curnode;
		val = parm->val;
		break;
	    case NCX_NT_VAL:
		val = (const val_value_t *)curnode;
		break;
	    default:
		/* all parmset and app node merges keep the 
		 * current parmset container 
		 */
		val = NULL;
	    }

	    /* if this is a leaf and not an index leaf, then
	     * apply the merge here
	     */
	    if (val && !val->index) {
		retval = typ_is_simple(typ_get_basetype(val->typdef));
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
			    NCX_NT_VAL, newval,
			    NCX_NT_VAL, curval,
			    NCX_NT_VAL, curparent, NO_ERR);
	if (res != NO_ERR) {
	    return res;
	}
    }

    val_merge_meta(newval, curval);
    return res;
#else
    val_merge_meta(newval, curval);
    return NO_ERR;
#endif
}   /* merge_metadata */


/********************************************************************
* FUNCTION get_parmset_node
* 
* Find the specified parmset node in the parent
*
* INPUTS:
*    newps == parmset to match in the parent node
*    curapp == current cfg_app_t node (or NULL if none)
*
* RETURNS:
*    pointer to current parmset from parent or NULL if none
*********************************************************************/
static ps_parmset_t *
    get_parmset_node (ps_parmset_t *newps,
		      cfg_app_t *curapp)
{
    ps_parmset_t *ps;

    if (!curapp) {
	return NULL;
    }

    for (ps = (ps_parmset_t *)dlq_firstEntry(&curapp->parmsetQ);
	 ps != NULL;
	 ps = (ps_parmset_t *)dlq_nextEntry(ps)) {
	if (!xml_strcmp(ps->name, newps->name)) {
	    return ps;
	}
    }
    return NULL;

} /* get_parmset_node */


/********************************************************************
* FUNCTION get_app_node
* 
* Find the appnode in the appQ of the parent value
*
* INPUTS:
*   val == data root node to check for cfg_app_t nodes
*   owner == owner name to find
*   appname == application node name to find
*
* RETURNS:
*   pointer to found node, or NULL if not found
*********************************************************************/
static cfg_app_t *
    get_app_node (val_value_t  *val,
		  const xmlChar *owner,
		  const xmlChar *appname)
{
    cfg_app_t  *app;

    for (app = (cfg_app_t *)dlq_firstEntry(&val->v.appQ);
	 app != NULL;
	 app = (cfg_app_t *)dlq_nextEntry(app)) {
	if (!xml_strcmp(owner, app->appdef->owner) &&
	    !xml_strcmp(appname, app->appdef->appname)) {
	    return app;
	}
    }
    return NULL;

} /* get_app_node */


/*****************   V A L I D A T E    P H A S E   ****************/


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
*   chtyp == typ_child_t struct to check for this value node
*   curval == current data object in the target (or NULL if none)
*   adddef == TRUE if one child node instance should be added
*             if it is required, and a default is available
*          == FALSE if any default value should be ignored
* RETURNS:
*   status; no rpc-error is recorded
*********************************************************************/
static status_t
    check_inst_cnt (uint32 cnt,
		    typ_child_t  *chtyp,
		    val_value_t  *curval,
		    boolean adddef)
{
    status_t      res;
    ncx_iqual_t   iqual;

    res = NO_ERR;
    iqual = typ_get_iqualval_def(&chtyp->typdef);

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
	    if (val_child_inst_cnt(curval, chtyp->name)) {
		/* this is a merge, and a current value exists */
		res = NO_ERR;
	    }
	}
	if (res != NO_ERR && adddef) {
	    /* not a valid merge so check if a default exists */
	    if (typ_get_default(&chtyp->typdef)) {
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
    status_t      res, retres;
    uint32        cnt;
    const typ_def_t    *typdef;
    typ_child_t  *chtyp;
    xmlns_qname_t qname;

    retres = NO_ERR;

    /* get the real typdef in case this is a named type */
    typdef = typ_get_cbase_typdef(newval->typdef);

    /* go through each child node in the typdef to see how many
     * instances of each child are present
     */
    for (chtyp = typ_first_child(&typdef->def.complex);
	 chtyp != NULL;
	 chtyp = typ_next_child(chtyp)) {

	/* TBD:
	 * this doesn't allow for child nodes in different namespaces 
	 */
	cnt = val_child_inst_cnt(newval, chtyp->name);

	switch (typ_get_basetype(&chtyp->typdef)) {
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
	    /* if the parent is a container, then the one and only
	     * child node can have zero or more instances
	     */
	    if (newval->btyp==NCX_BT_XCONTAINER) {
		continue;
	    }
	    break;
	}

	/* need to validate this child node instance count */
	res = check_inst_cnt(cnt, chtyp, curval, adddef);
	    
	/* check any errors for this child node */
	if (res != NO_ERR) {
	    qname.nsid = newval->nsid;
	    qname.name = chtyp->name;
	    agt_record_error(scb, &msg->mhdr.errQ, NCX_LAYER_CONTENT, 
		res, NULL, NCX_NT_QNAME, &qname, NCX_NT_VAL, newval);
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
*   chtyp == the child type node for chval, contains the group info
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
			typ_child_t *chtyp,
			boolean adddef,
			uint32  total)
{
    status_t      res;
    uint32        cnt, chtotal;
    typ_child_t  *grch;

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
	agt_record_error(scb, &msg->mhdr.errQ, NCX_LAYER_CONTENT, 
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
	res = check_choice_inst_cnt(scb, msg, newval, curval, adddef);
	break;
    case NCX_BT_LIST:
	/* check that table instance and non-instance columns 
	 * are present 
	 */	    
	res = check_child_inst_cnt(scb, msg, newval, curval, adddef);
	break;
    case NCX_BT_XCONTAINER:
	res = NO_ERR;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

} /* check_edit_instance */


/********************************************************************
* FUNCTION validate_write_parm
* 
* Invoke all the AGT_CB_VALIDATE callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   pop == parent requested edit op
*   newparm == parm to apply
*   curps == current parmset containing this parm from target (if any)
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    validate_write_parm (ses_cb_t  *scb,
			 rpc_msg_t  *msg,
			 cfg_template_t *target,
			 op_editop_t  pop,
			 ps_parm_t     *newparm,
			 ps_parmset_t  *curps)
{
    ps_parm_t       *curparm;
    val_value_t     *curval, *v_val;
    status_t         res;
    ncx_iqual_t      iqual;

#ifdef AGT_VAL_DEBUG
    log_debug2("\nagt_val_parm: %s start", newparm->parm->name);
#endif

    curparm = NULL;
    curval = NULL;
    v_val = NULL;
    res = NO_ERR;

    /* try to find the current parm in the target config */
    if (curps) {
	curparm = ps_find_parm(curps, newparm->parm->name);
	if (curparm) {
	    /* check if the hdr is a parm or a nested parmset */
	    curval = curparm->val;
	    if (curval && val_is_virtual(curval)) {
		v_val = val_get_virtual_value(scb, curval, &res);
	    }
	}
    }

    if (res == NO_ERR) {
	/* check and adjust the operation attribute */
	iqual = val_get_iqualval(newparm->val);

	/* check edit-op does not need to check the
	 * parm value, so pass the curparm, not curval or v_val
	 */
	res = agt_check_editop(pop, &newparm->val->editop,
			       NCX_NT_PARM, newparm, curparm, iqual);

	/* check the max-access for this param */
	if (res == NO_ERR) {
	    res = agt_check_max_access(newparm->val->editop, 
				       newparm->parm->access, 
				       (curparm != NULL));
	}
    }

    /* record any error so far */
    if (res != NO_ERR) {
	agt_record_error(scb, &msg->mhdr.errQ, NCX_LAYER_CONTENT, res,
		 NULL, NCX_NT_PARM, newparm, NCX_NT_PARM, newparm);
    } else {
	/* check for any typedef callbacks for this param;
	 * process the edit operation along the way
	 */
	res = invoke_btype_cb(AGT_CB_VALIDATE, pop, scb, msg, 
			      target, newparm->val, 
			      (v_val) ? v_val : curval, FALSE);
    }

    /* if there are no errors so far in the parm, then
     * try to call the validate function for this parm 
     */
    if (res == NO_ERR) {
	res = handle_user_callback(AGT_CB_VALIDATE,
				   pop, scb, msg, NCX_NT_PARM,
				   newparm, curparm);
    }

    if (v_val) {
	val_free_value(v_val);
    }

    return res;

}  /* validate_write_parm */


/********************************************************************
* FUNCTION validate_write_ps
* 
* Invoke all the AGT_CB_VALIDATE callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   editop == requested write operation
*   newps == parmset to validate
*   curps == corresponding parrmset from the target (may be NULL)
*
* RETURNS:
*   status; rpc-errors are recorded for any errors found
*********************************************************************/
static status_t
    validate_write_ps (ses_cb_t  *scb,
		       rpc_msg_t  *msg,
		       cfg_template_t *target,
		       op_editop_t  editop,
		       ps_parmset_t  *newps,
		       ps_parmset_t  *curps)  
{
    ps_parm_t        *newparm;
    status_t          res, retres;
    boolean           errdone; 

#ifdef AGT_VAL_DEBUG
    log_debug2("\nagt_val_ps: %s start", newps->name);
#endif

    errdone = FALSE;

    /* check and adjust the operation attribute */
    res = agt_check_editop(editop, &newps->editop, NCX_NT_PARMSET,
			   newps, curps, NCX_IQUAL_ONE);
    if (res == NO_ERR) {
	/* first check access control, only if it is a data model
	 * parameter set; RPC access has already been checked
	 */
	if (newps->psd->psd_type == PSD_TYP_DATA) {
	    if (!agt_acm_ps_write_allowed(scb->username, newps)) {
		res = ERR_NCX_ACCESS_DENIED;
	    }
	}
    }

    /* try to call the user validate function for this parmset */
    if (res==NO_ERR) {
	res = handle_user_callback(AGT_CB_VALIDATE, editop,
				   scb, msg, NCX_NT_PARMSET,
				   newps, curps);
	errdone = TRUE;
    }

    /* record any errors and exit if so */
    if (res != NO_ERR) {
	if (!errdone) {
	    agt_record_error(scb, &msg->mhdr.errQ, 
			     NCX_LAYER_CONTENT, res,
			     NULL, NCX_NT_PARMSET, newps, 
			     NCX_NT_PARMSET, newps);
	}

	return res;
    }

    /* go through each parameter and call all the validate
     * callback functions that exist.  Check the editop as well.
     */
    retres = NO_ERR;
    for (newparm = (ps_parm_t *)dlq_firstEntry(&newps->parmQ);
	 newparm != NULL;
	 newparm = (ps_parm_t *)dlq_nextEntry(newparm)) {

	res = validate_write_parm(scb, msg, target, 
				  editop, newparm, curps);
	newparm->res = res;
	if (res != NO_ERR) {
	    retres = res;
	    errdone = TRUE;
	}
    } 

    /* check the editop against the current node 
     * and add defaults if needed (create or load)
     */
    if (retres == NO_ERR) {
	errdone = FALSE;
	switch (newps->editop) {
	case OP_EDITOP_CREATE:
	    if (curps) {
		retres = ERR_NCX_DATA_EXISTS;
	    } else {
		retres = ps_parse_add_defaults(newps);
	    }
	    break;
	case OP_EDITOP_DELETE:
	    if (!curps) {
		retres = ERR_NCX_DATA_MISSING;
	    }
	    break;
	case OP_EDITOP_MERGE:
	    if (!curps) {
		retres = ps_parse_add_defaults(newps);
	    }
	    break;
	case OP_EDITOP_REPLACE:
	case OP_EDITOP_LOAD:
	    retres = ps_parse_add_defaults(newps);
	    break;
	default:
	    ;
	}
    }

    /* check if the operation is leaving a valid choices */
    if (retres == NO_ERR) {
	errdone = FALSE;
	switch (newps->editop) {
	case OP_EDITOP_MERGE:
	    if (curps) {
		/*** not checking valid choices after merge !!! ***/
		break;   
	    } /* else drop through */
	case OP_EDITOP_CREATE:
	case OP_EDITOP_REPLACE:
	case OP_EDITOP_LOAD:
	    retres = agt_ps_parse_choice_check(scb, &msg->mhdr,
					       newps, NCX_LAYER_CONTENT);
	    errdone = TRUE;
	    break;
	default:
	    ;
	}
    }

    /* check if the operation is leaving a full parmset */
    if (retres == NO_ERR) {
	errdone = FALSE;
	switch (newps->editop) {
	case OP_EDITOP_MERGE:
	    if (curps) {
		/*** not checking full parmset after merge !!! ***/
		break;   
	    } /* else drop through */
	case OP_EDITOP_CREATE:
	case OP_EDITOP_REPLACE:
	case OP_EDITOP_LOAD:
	    retres = agt_ps_parse_instance_check(scb, &msg->mhdr,
						 newps, NCX_LAYER_CONTENT);
	    errdone = TRUE;
	    break;
	default:
	    ;
	}
    }


    /* check if error output needed */
    if (retres != NO_ERR && !errdone) {
	agt_record_error(scb, &msg->mhdr.errQ, NCX_LAYER_CONTENT, 
			 retres, NULL, NCX_NT_PARMSET, newps, 
			 NCX_NT_PARMSET, newps);
    }

    return retres;

}  /* validate_write_ps */


/********************************************************************
* FUNCTION validate_write_app
* 
* Invoke all the AGT_CB_VALIDATE callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   editop == requested write operation
*   app == application node to validate
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    validate_write_app (ses_cb_t  *scb,
			rpc_msg_t  *msg,
			cfg_template_t *target,
			op_editop_t  editop,
			cfg_app_t  *newapp,
			cfg_app_t  *curapp)  
{
    ps_parmset_t    *newps, *curps;
    status_t        res, retres;

    retres = NO_ERR;

#ifdef AGT_VAL_DEBUG
    log_debug2("\nagt_val_app: %s start", newapp->appdef->appname);
#endif

    /* go through all the parmsets specified in the new config 
     * data.  This code will not recognize monitoring
     * data objects in the <edit-config> RPC method.
     * This is consistent with the <get-config> RPC method
     * which also filters out all non-config data models
     */
    for (newps = (ps_parmset_t *)dlq_firstEntry(&newapp->parmsetQ);
	 newps != NULL;
	 newps = (ps_parmset_t *)dlq_nextEntry(newps)) {

	res = NO_ERR;
	if (curapp) {
	    /* try to find this parmset in the target config */
	    curps = cfg_get_parmset(target, newps);
	} else {
	    curps = NULL;
	}

	res = validate_write_ps(scb, msg, target, 
				editop, newps, curps);
	newps->res = res;

	if (res != NO_ERR) {
	    retres = res;
	}
    }

    return retres;

}  /* validate_write_app */


/*******************   A P P L Y    P H A S E   ********************/


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
*   curparent, newval, and curval may be moved around to
*   different queues, or get modified
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

    /* call the user callback apply function for this parm */
    res = handle_user_callback(AGT_CB_APPLY,
			       newval->editop, scb, msg,
			       NCX_NT_VAL,
			       newval, curval);
    if (res != NO_ERR) {
	return res;
    }

    /* check if this node needs the edit operation applied */
    if (*done) {
	applyhere = FALSE;
    } else {
	applyhere = apply_this_node(newval->editop, NCX_NT_VAL, curval);
	*done = applyhere;
    }

    /* apply the requested edit operation */
    if (applyhere) {
	if (msg->rpc_need_undo) {
	    res = add_undo_node(msg, editop,
				NCX_NT_VAL, newval,
				NCX_NT_VAL, curval,
				NCX_NT_VAL, parent, NO_ERR);
	    if (res != NO_ERR) {
		return res;
	    }
	}

	handle_audit_record(newval->editop, scb, target, 
			    NCX_NT_VAL,
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
	case OP_EDITOP_LOAD:
	    dlq_remove(newval);
	    add_valnode(newval, parent);
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
* FUNCTION apply_write_parm
* 
* Invoke all the AGT_CB_APPLY callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newps == parent of new parmset to apply
*   curps == current parent parmset of parm
*   newparm == new parm to apply
*   curparm == current instance of parm (may be NULL if none)
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*         Processing continues to the leaf nodes in case there
*         are nested user callbacks related to embedded named types
*
* OUTPUTS:
*   newps, curps, newparm, and curparm may be moved around to
*   different queues, get modified
* RETURNS:
*   status
*********************************************************************/
static status_t
    apply_write_parm (ses_cb_t  *scb,
		      rpc_msg_t  *msg,
		      cfg_template_t *target,
		      ps_parmset_t  *newps,
		      ps_parmset_t  *curps,
		      ps_parm_t     *newparm,
		      ps_parm_t     *curparm,
		      boolean        done)
{
    status_t          res;
    boolean           applyhere, freenew;
    op_editop_t       editop;

#ifdef AGT_VAL_DEBUG
    log_debug2("\nagt_write_parm: %s start", newparm->parm->name);
#endif

    res = NO_ERR;
    freenew = FALSE;
    editop = newparm->val->editop;
    newparm->val->curparent = curparm;

    /* call the user callback apply function for this parm */
    res = handle_user_callback(AGT_CB_APPLY,
			       newparm->editop, scb, msg,
			       NCX_NT_PARM,
			       newparm, curparm);
    if (res != NO_ERR) {
	return res;
    }

    /* check if this node needs the edit operation applied */
    applyhere = (done) ? FALSE : 
	apply_this_node(editop, NCX_NT_VAL, curparm->val);

    /* try to call the apply function for this parm */
    if (!applyhere) {
	/* process all the way to the leaf */
	res = invoke_btype_cb(AGT_CB_APPLY, newps->editop, 
			      scb, msg, target, 
			      newparm->val, 
			      (curparm) ? curparm->val : NULL, done);
    } else {
	/* apply the requested edit operation */
	if (msg->rpc_need_undo) {
	    res = add_undo_node(msg, editop,
				NCX_NT_PARM, newparm,
				NCX_NT_PARM, curparm,
				NCX_NT_PARMSET, curps, NO_ERR);
	    if (res != NO_ERR) {
		return res;
	    }
	}

	handle_audit_record(editop, scb, target, 
			    NCX_NT_PARM, 
			    (curparm) ? curparm : newparm, res);

	res = handle_user_callback(AGT_CB_APPLY,
				   editop, scb, msg,
				   NCX_NT_PARM,
				   newparm, curparm);

	if (curparm && val_is_virtual(curparm->val)) {
	    freenew = FALSE;
	} else {
	    switch (editop) {
	    case OP_EDITOP_MERGE:
		dlq_remove(newparm);
		if (curparm) {
		    freenew = merge_parmnode(newparm, curparm);
		} else {
		    add_parmnode(newparm, curps);
		}
		break;
	    case OP_EDITOP_REPLACE:
		if (curparm) {
		    dlq_remove(newparm);
		    swap_parmnode(newparm, curparm);
		    if (!msg->rpc_need_undo) {
			ps_free_parm(curparm);
		    } /* else curparm not freed yet, hold in undo record */
		} else {
		    dlq_remove(newparm);
		    add_parmnode(newparm, curps);
		}
		break;
	    case OP_EDITOP_CREATE:
	    case OP_EDITOP_LOAD:
		dlq_remove(newparm);
		add_parmnode(newparm, curps);
		break;
	    case OP_EDITOP_DELETE:
		if (curparm) {
		    dlq_remove(curparm);
		    if (!msg->rpc_need_undo) {
			ps_free_parm(curparm);
		    } /* else curparm not freed yet, hold in undo record */
		}
		done = TRUE;
		break;
	    default:
		freenew = TRUE;
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
    }

    if (freenew) {
	ps_free_parm(newparm);
    }

    return res;

}  /* apply_write_parm */


/********************************************************************
* FUNCTION apply_write_ps
* 
* Invoke all the AGT_CB_APPLY callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   curapp == parent node of the current instance of parmset
*             This must not be NULL, even if curps is NULL
*   newps == new parmset to apply
*   curps == current instance of parmset (may be NULL if none)
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*
* OUTPUTS:
*   newapp, curapp, newps, and curps may be moved around to
*   different queues, get modified
* RETURNS:
*   status
*********************************************************************/
static status_t
    apply_write_ps (ses_cb_t  *scb,
		    rpc_msg_t  *msg,
		    cfg_template_t *target,
		    cfg_app_t     *curapp,
		    ps_parmset_t  *newps,
		    ps_parmset_t  *curps,
		    boolean done)
{
    ps_parm_t        *newparm, *curparm, *nextparm;
    status_t          res, retres;
    boolean           applyhere;

#ifdef AGT_VAL_DEBUG
    log_debug2("\nagt_apply_ps: %s start", newps->name);
#endif

    retres = NO_ERR;
    res = NO_ERR;

    /* call the user callback apply function for this parm */
    res = handle_user_callback(AGT_CB_APPLY,
			       newps->editop, scb, msg,
			       NCX_NT_PARMSET,
			       newps, curps);
    if (res != NO_ERR) {
	return res;
    }

    /* check if this node needs to apply the requested edit operation */
    applyhere = (done) ? FALSE : 
	apply_this_node(newps->editop, NCX_NT_PARMSET, curps);

    /* check user callback 
    * !!! what if there is no parmset handler 
    */
    if (!applyhere) {
	/* go through all the parameters */
	newparm = (ps_parm_t *)dlq_firstEntry(&newps->parmQ);
	while (newparm) {
	    nextparm = (ps_parm_t *)dlq_nextEntry(newparm);

	    /* find current value */
	    if (curps) {
		curparm = ps_find_parm(curps, newparm->parm->name);
	    } else {
		curparm = NULL;
	    }

	    /* apply this parameter */
	    res = apply_write_parm(scb, msg, target, 
				   newps, curps, 
				   newparm, curparm, done);
	    if (res != NO_ERR) {
		if (msg->rpc_err_option != OP_ERROP_CONTINUE) {
		    return res;
		} else {
		    retres = res;
		}
	    }
	    newparm = nextparm;
	}
    }

    /* check if continue on error is requested */
    if (retres != NO_ERR && msg->rpc_err_option==OP_ERROP_CONTINUE) {
	retres = NO_ERR;
    }

    /* apply the edit operation to this node if needed */
    if (retres == NO_ERR && applyhere) {
	if (msg->rpc_need_undo) {
	    res = add_undo_node(msg, newps->editop,
				NCX_NT_PARMSET, newps,
				NCX_NT_PARMSET, curps,
				NCX_NT_APP, curapp, NO_ERR);
	    if (res != NO_ERR) {
		return res;
	    }
	}

	handle_audit_record(newps->editop, scb, target, 
			    NCX_NT_PARMSET, 
			    (curps) ? curps : newps, retres);

	switch (newps->editop) {
	case OP_EDITOP_MERGE:
	    if (curps) {
		/* leave newps queued and it will be freed
		 * along with the rest of the request PDU
		 */
		res = merge_psnode(newps, curps);
		if (res != NO_ERR) {
		    SET_ERROR(res);
		}
	    } else {
		dlq_remove(newps);
		add_psnode(newps, curapp);
	    }
	    break;
	case OP_EDITOP_REPLACE:
	    if (curps) {
		if (ps_is_vparmset(curps)) {
		    /* leave newps queued and it will be freed
		     * along with the rest of the request PDU
		     */
		    ps_replace_vparmset(newps, curps);
		} else {
		    dlq_remove(newps);
		    swap_psnode(newps, curps);
		    if (!msg->rpc_need_undo) {
			ps_free_parmset(curps);
		    } /* else curps is not deleted yet -- hold in undo */
		}
	    } else {
		dlq_remove(newps);
		add_psnode(newps, curapp);
	    }
	    break;
	case OP_EDITOP_CREATE:
	case OP_EDITOP_LOAD:
	    dlq_remove(newps);
	    add_psnode(newps, curapp);
	    break;
	case OP_EDITOP_DELETE:
	    if (curps) {
		dlq_remove(curps);
		if (!msg->rpc_need_undo) {
		    ps_free_parmset(curps);
		} /* else curps is not deleted yet -- hold in undo */
	    }
	    break;
	default:
	    retres = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    return retres;

}  /* apply_write_ps */


/********************************************************************
* FUNCTION apply_write_app
* 
* Invoke all the AGT_CB_APPLY callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newapp == application node to apply
*   curval == parent node of 'curapp' node
*   curapp == current application node from target (may be NULL)
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
* RETURNS:
*   status
*********************************************************************/
static status_t
    apply_write_app (ses_cb_t  *scb,
		     rpc_msg_t  *msg,
		     cfg_template_t *target,
		     cfg_app_t  *newapp,
		     val_value_t *curval,
		     cfg_app_t  *curapp,
		     boolean     done)
{
    ps_parmset_t    *newps, *curps, *nextps;
    status_t        res, retres;
    boolean         applyhere;

#ifdef AGT_VAL_DEBUG
    log_debug2("\nagt_apply_app: %s start", newapp->appdef->appname);
#endif

    retres = NO_ERR;
    res = NO_ERR;

    /* check if this node needs the edit operation applied */
    applyhere = (done) ? FALSE :
	apply_this_node(newapp->editop, NCX_NT_APP, curapp);


    /* Go through all the parmsets specified in the new config 
     * data. Since app nodes do not have callback functions
     * each parmset must be handled individually
     */
    newps = (ps_parmset_t *)dlq_firstEntry(&newapp->parmsetQ);
    while (newps) {
	/* save the next parmset */
	nextps = (ps_parmset_t *)dlq_nextEntry(newps);

	/* skip any parmsets that failed the validate phase */
	if (newps->res != NO_ERR) {
	    newps = nextps;
	    continue;
	}

	/* try to find this parmset in the target config */
	curps = get_parmset_node(newps, curapp);

	/* newapp may get moved to a different queue */
	res = apply_write_ps(scb, msg, target, 
			     curapp, newps, curps,
			     applyhere || done);
	if (res != NO_ERR) {
	    retres = res;
	}
	newps = nextps;
    }

    /* check if continue on error is requested */
    if (retres != NO_ERR && msg->rpc_err_option==OP_ERROP_CONTINUE) {
	retres = NO_ERR;
    }

    /* now apply the edit operation to this node if needed */
    if (retres == NO_ERR && applyhere) {
	if (msg->rpc_need_undo) {
	    res = add_undo_node(msg, newapp->editop,
				NCX_NT_APP, newapp,
				NCX_NT_APP, curapp,
				NCX_NT_VAL, curval, NO_ERR);
	    if (res != NO_ERR) {
		return res;
	    }
	}

	handle_audit_record(newapp->editop, scb, target, 
			    NCX_NT_APP, 
			    (curapp) ? curapp : newapp, retres);

	switch (newapp->editop) {
	case OP_EDITOP_MERGE:
	    /* no current app node
	     * this new app is being inserted in the target
	     * the parent of curapp should not be NULL 
	     */
	    dlq_remove(newapp);
	    add_appnode(newapp, curval);
	    break;
	case OP_EDITOP_REPLACE:
	    /* grab the new val for replacement of the app in the target */
	    if (curapp) {
		dlq_remove(newapp);
		swap_appnode(newapp, curapp);
		if (!msg->rpc_need_undo) {
		    cfg_free_appnode(curapp);
		} /* else curapp is not deleted yet -- hold in undo */
	    } else {
		dlq_remove(newapp);
		add_appnode(newapp, curval);
	    }
	    break;
	case OP_EDITOP_CREATE:
	case OP_EDITOP_LOAD:
	    dlq_remove(newapp);
	    add_appnode(newapp, curval);
	    break;
	case OP_EDITOP_DELETE:
	    if (curapp) {
		dlq_remove(curapp);
		if (!msg->rpc_need_undo) {
		    cfg_free_appnode(curapp);
		} /* else curapp is not deleted yet -- hold in undo */
	    }
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    return retres;

}  /* apply_write_app */


/*********   B A S E   T Y P E   S P E C I F I C    ***************/


/********************************************************************
* FUNCTION invoke_root_cb
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
    invoke_root_cb (agt_cbtyp_t cbtyp,
		    op_editop_t editop,
		    ses_cb_t  *scb,
		    rpc_msg_t  *msg,
		    cfg_template_t *target,
		    val_value_t  *newval,
		    val_value_t  *curval,
		    boolean done)
{
    cfg_app_t        *newapp, *curapp, *nextapp;
    status_t          res, retres;

    retres = NO_ERR;

    /* go through the root value and invoke any callbacks that
     * need to process the requested write operation internally
     */

    newapp = (cfg_app_t *)dlq_firstEntry(&newval->v.appQ);
    while (newapp) {

	nextapp = (cfg_app_t *)dlq_nextEntry(newapp);

	/* find out if this application exists in the parent root node */
	if (curval) {
	    curapp = get_app_node(curval,
				  newapp->appdef->owner,
				  newapp->appdef->appname);
	} else {
	    curapp = NULL;
	}

	switch (cbtyp) {
	case AGT_CB_VALIDATE:
	    /* check and adjust the operation attribute */
	    res = agt_check_editop(editop, &newapp->editop, NCX_NT_APP,
				   newapp, curapp, NCX_IQUAL_ONE);
	    if (res != NO_ERR) {
		agt_record_error(scb, &msg->mhdr.errQ, 
				 NCX_LAYER_CONTENT, res, 
				 NULL, NCX_NT_VAL, newval, 
				 NCX_NT_VAL, newval);
	    } else {
		res = validate_write_app(scb, msg, target, 
					 editop, newapp, curapp);
	    }
	    break;
	case AGT_CB_APPLY:
	    res = apply_write_app(scb, msg, target, 
				  newapp, curval, curapp, done);
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}

	if (res != NO_ERR) {
	    retres = res;
	}

	newapp = nextapp;
    }
	
    return retres;

}  /* invoke_root_cb */


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
	/* check and adjust the operation attribute */
	iqual = val_get_iqualval(newval);
	res = agt_check_editop(editop, &newval->editop, 
			       NCX_NT_VAL, newval, curval, iqual);
	if (res != NO_ERR) {
	    agt_record_error(scb, &msg->mhdr.errQ, 
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
				   NCX_NT_VAL, newval, curval);
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
	/* check and adjust the operation attribute */
	iqual = val_get_iqualval(newval);
	res = agt_check_editop(editop, &newval->editop, 
			       NCX_NT_VAL, newval, curval, iqual);
	if (res != NO_ERR) {
	    agt_record_error(scb, &msg->mhdr.errQ, 
			     NCX_LAYER_CONTENT, res, 
			     NULL, NCX_NT_VAL, newval, 
			     NCX_NT_VAL, newval);
	    return res;
	}

	switch (newval->editop) {
	case OP_EDITOP_CREATE:
	case OP_EDITOP_LOAD:
	    res = check_edit_instance(scb, msg, newval, NULL, TRUE);
	    break;
	case OP_EDITOP_REPLACE:
	    res = check_edit_instance(scb, msg, newval, NULL, FALSE);
	    break;
	case OP_EDITOP_MERGE:
	    res = check_edit_instance(scb, msg, newval, curval, FALSE);
	    break;
	default:
	    res = NO_ERR;
	    break;
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
				      NCX_NT_VAL, newval, curval);
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
    switch (newval->btyp) {
    case NCX_BT_ROOT:
	res = invoke_root_cb(cbtyp, editop, scb, msg, 
			     target, newval, 
			     (v_val) ? v_val : curval, done);
	break;
    case NCX_BT_ANY:
    case NCX_BT_ENAME:
    case NCX_BT_ENUM:
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
    case NCX_BT_INT64:
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
    case NCX_BT_UINT64:
    case NCX_BT_FLOAT32:
    case NCX_BT_FLOAT64:
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_SLIST:
    case NCX_BT_XLIST:
	res = invoke_simval_cb(cbtyp, editop, scb, msg,
			       target, newval, 
			       (v_val) ? v_val : curval, done);
	break;
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_LIST:
    case NCX_BT_XCONTAINER:
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

    switch (undo->editop) {
    case OP_EDITOP_LOAD:
	/* not supported for internal load operation */
	break;    
    case OP_EDITOP_CREATE:
	/* Since 'create' cannot apply to an attribute,
	 * the metadata is not checked
	 */
	res = handle_user_callback(AGT_CB_ROLLBACK, undo->editop,
				   scb, msg, undo->newnodetyp, 
				   undo->newnode, undo->curnode);

#ifdef NOT_YET
	/**** SET done !!! ***/
	if (!done) {
	    /* no rollback callback, so apply a delete operation */
	    res = handle_user_callback(AGT_CB_APPLY, OP_EDITOP_DELETE,
				       scb, msg, undo->newnodetyp, 
				       undo->newnode, NULL);
	}
#endif

	/* delete the node from the tree */
	dlq_remove(undo->newnode);
	ncx_free_node(undo->newnodetyp, undo->newnode);
	break;
    case OP_EDITOP_DELETE:
	/* Since 'delete' cannot apply to an attribute,
	 * the metadata is not checked
	 */
	res = handle_user_callback(AGT_CB_ROLLBACK, 
				   undo->editop, scb, msg, 
				   undo->newnodetyp, undo->newnode,
				   undo->curnode);
#ifdef NOT_YET
	/**** SET done !!! ***/
	if (!done) {
	    /* no rollback callback, so apply a create operation */
	    res = handle_user_callback(AGT_CB_APPLY, OP_EDITOP_CREATE,
				       scb, msg, undo->curnodetyp, 
				       undo->curnode, NULL);
	}
#endif

	/* add the node back in the tree */
	switch (undo->curnodetyp) {
	case NCX_NT_VAL:
	    add_valnode(undo->curnode, undo->parentnode);
	    break;
	case NCX_NT_PARM:
	    add_parmnode(undo->curnode, undo->parentnode);
	    break;
	case NCX_NT_PARMSET:
	    add_psnode(undo->curnode, undo->parentnode);
	    break;
	case NCX_NT_APP:
	    add_appnode(undo->curnode, undo->parentnode);
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
	break;
    case OP_EDITOP_MERGE:
    case OP_EDITOP_REPLACE:
	/*** NEED TO CHECK FOR MERGED META DATA FOR MERGE ***/

	/* call the user rollback handler, if any */
	res = handle_user_callback(AGT_CB_ROLLBACK, 
				   undo->editop, scb, msg, 
				   undo->newnodetyp, undo->newnode,
				   undo->curnode);
#ifdef NOT_YET
	/**** SET done !!! ***/
	if (!done) {
	    /* no rollback callback, so apply a create operation */
	    res = handle_user_callback(AGT_CB_APPLY, OP_EDITOP_DELETE,
				       scb, msg, undo->newnodetyp,
				       undo->newnode, undo->curnode);
	}
#endif

	/* check if the old node needs to be swapped back
	 * of if the new node is just removed
	 */
	if (undo->newnode) {
	    dlq_remove(undo->newnode);
	    if (undo->curnode) {
		/* remove new node and put back old node */
		switch (undo->newnodetyp) {
		case NCX_NT_VAL:
		    swap_valnode(undo->curnode, undo->newnode);
		    break;
		case NCX_NT_PARM:
		    swap_parmnode(undo->curnode, undo->newnode);
		    break;
		case NCX_NT_PARMSET:
		    swap_psnode(undo->curnode, undo->newnode);
		    break;
		case NCX_NT_APP:
		    swap_appnode(undo->curnode, undo->newnode);
		    break;
		default:
		    SET_ERROR(ERR_INTERNAL_VAL);
		} 
	    } 
	    /* remove new node */
	    ncx_free_node(undo->newnodetyp, undo->newnode);

	} /* else should not happen */
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    /****
    handle_undo_audit_record(undo->editop,
                             scb, target,
			     undo->curnodetyp,
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
				       scb, msg, undo->newnodetyp, 
				       undo->newnode, undo->curnode);

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


/******************* E X T E R N   F U N C T I O N S ***************/


/********************************************************************
* FUNCTION agt_val_handle_callback
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
status_t
    agt_val_handle_callback (agt_cbtyp_t cbtyp,
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
    case AGT_CB_LOAD:
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

}  /* agt_val_handle_callback */


/* END file agt_val.c */
