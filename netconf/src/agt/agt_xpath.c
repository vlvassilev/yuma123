/*  FILE: agt_tree.c


  NETCONF XPath Filtering Implementation
	
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
27jan09      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>
#include  <string.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_acm
#include "agt_acm.h"
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

#ifndef _H_agt_xpath
#include "agt_xpath.h"
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
#include  "log.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_obj
#include  "obj.h"
#endif

#ifndef _H_op
#include  "op.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
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

#ifndef _H_xpath
#include  "xpath.h"
#endif

#ifndef _H_xpath1
#include  "xpath1.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define AGT_XPATH_DEBUG 1


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION find_val_resnode
* 
* Check if the specified resnode ptr is already in the Q
*
* INPUTS:
*    resultQ == Q of xpath_resnode_t structs to check
*               DOES NOT HAVE TO BE WITHIN A RESULT NODE Q
*    valptr   == pointer value to find
*
* RETURNS:
*    found resnode or NULL if not found
*********************************************************************/
static xpath_resnode_t *
    find_val_resnode (dlq_hdr_t *resultQ,
		      val_value_t *valptr)
{
    xpath_resnode_t  *resnode;

    for (resnode = (xpath_resnode_t *)dlq_firstEntry(resultQ);
	 resnode != NULL;
	 resnode = (xpath_resnode_t *)dlq_nextEntry(resnode)) {

	if (resnode->node.valptr == valptr) {
	    return resnode;
	}
    }
    return NULL;

}  /* find_val_resnode */


/********************************************************************
* FUNCTION output_resnode
*
* Out the result node in NETCONF format
* Collapse all the subtrees together whenever possible
* Add key leafs in lists if the filter left them out
*
* INPUTS:
*    scb == session control block
*    msg == rpc_msg_t in progress
*    pcb == XPath parser control block to use
*    resultQ == current Q of xpath_resnode_t to use for the
*               pool of potential nodes to gather into
*               the curval output
*    curval == current resnode value to output w/ path to root
*    ceilingval == current root of the tree
*    getop == TRUE for <get>; FALSE for <get-config>
*    indent == start indent amount
*
*********************************************************************/
static void
    output_resnode (ses_cb_t *scb,
		    rpc_msg_t *msg,
		    xpath_pcb_t *pcb,
		    dlq_hdr_t *resultQ,
		    val_value_t *curval,
		    val_value_t *ceilingval,
		    boolean getop,
		    int32 indent)
{
    val_value_t       *topval;
    val_index_t       *valindex;
    xpath_resnode_t   *testnode, *nextnode, dummynode;
    dlq_hdr_t          dummyQ, descendantQ;
    int32              indentamount;
    boolean            dowrite;

    dlq_createSQue(&dummyQ);
    dlq_createSQue(&descendantQ);
    indentamount = ses_indent_count(scb);

    /* find the top-level node for this result node */
    topval = curval;
    while (topval->parent &&
	   topval->parent != ceilingval) {
	topval = topval->parent;
    }

    if (topval == curval) {
	/* no other result nodes are going to be
	 * present which are descendants of this node
	 * since it was explicitly set in the result
	 *
	 * special check to make sure not duplicating
	 * a key leaf; cannot use the obj_is_key()
	 * function because the object is a generic string
	 */
	if (!curval->index) {
	    if (getop) {
		xml_wr_full_val(scb, 
				&msg->mhdr, 
				curval, 
				indent);
	    } else {
		xml_wr_full_check_val(scb, 
				      &msg->mhdr, curval, 
				      indent, 
				      agt_check_config);
	    }
	}
	return;
    }

    /* have the top-level node to output which
     * is different than the selected result node
     * make a dummy node to check all the
     * resnodes for this common ancestor
     */
    xpath_init_resnode(&dummynode);
    dummynode.node.valptr = topval;
    dlq_enque(&dummynode, &dummyQ);

    /* gather all the remaining resnodes
     * that also have this value node as its top
     * level parent
     */
    for (testnode = (xpath_resnode_t *)dlq_firstEntry(resultQ);
	 testnode != NULL;
	 testnode = nextnode) {

	nextnode = (xpath_resnode_t *)dlq_nextEntry(testnode);

	if (xpath1_check_node_exists(pcb, 
				     &dummyQ,
				     testnode->node.valptr)) {
	    dlq_remove(testnode);
	    dlq_enque(testnode, &descendantQ);
	}
    }

    dlq_remove(&dummynode);
    xpath_clean_resnode(&dummynode);

    /* check if access control is allowing this user
     * to retrieve this value node
     */
    dowrite = agt_acm_val_read_allowed(&msg->mhdr,
				      scb->username,
				      topval);
	    
    /* have all the nodes required for this subtree
     * start with the top and keep going until
     * curval and all the dummyQ contents
     * have been output
     */
    if (dowrite) {
	xml_wr_begin_elem_ex(scb, 
			     &msg->mhdr,
			     ceilingval->nsid,
			     topval->nsid,
			     topval->name, 
			     &topval->metaQ, 
			     FALSE, 
			     indent, 
			     FALSE);
    }

    if (indent >= 0) {
	indent += indentamount;
    }

    /* check special case for lists; generate key leafs first */
    if (val_has_index(topval)) {
	/* write all key leafs in order
	 * remove any of them that happen to be
	 * in the descendantQ
	 */
	for (valindex = val_get_first_index(topval);
	     valindex != NULL;
	     valindex = val_get_next_index(valindex)) {

	    if (dowrite) {
		xml_wr_full_val(scb, 
				&msg->mhdr, 
				valindex->val, 
				indent);
	    }

	    testnode = find_val_resnode(&descendantQ,
					valindex->val);
	    if (testnode) {
		dlq_remove(testnode);
		xpath_free_resnode(testnode);
	    }
	}
    }

    /* clear any nodes from the descendantQ which are child
     * nodes of 'topval'
     */
    for (testnode = (xpath_resnode_t *)dlq_firstEntry(&descendantQ);
	 testnode != NULL;
	 testnode = nextnode) {
	    
	nextnode = (xpath_resnode_t *)dlq_nextEntry(testnode);

	if (testnode->node.valptr->parent == topval) {
	    /* this descendant is a sibling or higher-up
	     * descendant of the topval, so output it now
	     */
	    dlq_remove(testnode);
	    if (getop) {
		if (dowrite) {
		    xml_wr_full_val(scb, 
				    &msg->mhdr, 
				    testnode->node.valptr, 
				    indent);
		}
	    } else {
		if (dowrite) {
		    xml_wr_full_check_val(scb, 
					  &msg->mhdr,
					  testnode->node.valptr, 
					  indent, 
					  agt_check_config);
		}
		xpath_free_resnode(testnode);
	    }
	}
    }

    if (dowrite) {
	/* move the ceiling closer to the curval and try again
	 * with the current value
	 */
	output_resnode(scb, 
		       msg, 
		       pcb, 
		       &descendantQ, 
		       curval, 
		       topval, 
		       getop, 
		       indent);
    }


    /* need to clear the descendant nodes before 
     * generating the topval end tag
     */
    testnode = (xpath_resnode_t *)dlq_deque(&descendantQ);
    while (testnode) {
	if (dowrite) {
	    output_resnode(scb, 
			   msg, 
			   pcb, 
			   &descendantQ, 
			   testnode->node.valptr, 
			   topval, 
			   getop, 
			   indent);
	}
	xpath_free_resnode(testnode);
	testnode = (xpath_resnode_t *)dlq_deque(&descendantQ);
    }

    if (indent >= 0) {
	indent -= indentamount;
    }

    /* finish off the topval node */
    if (dowrite) {
	xml_wr_end_elem(scb, 
			&msg->mhdr, 
			topval->nsid,
			topval->name, 
			indent);
    }

} /* output_resnode */


/********************************************************************
* FUNCTION output_result
*
* Out the result nodeset in NETCONF format
* Collapse all the subtrees together whenever possible
* Add key leafs in lists if the filter left them out
*
* INPUTS:
*    scb == session control block
*    msg == rpc_msg_t in progress
*    pcb == XPath parser control block to use
*    result == XPath result to use
*    getop == TRUE for <get>; FALSE for <get-config>
*    indent == start indent amount
*
*********************************************************************/
static void
    output_result (ses_cb_t *scb,
		   rpc_msg_t *msg,
		   xpath_pcb_t *pcb,
		   xpath_result_t *result,
		   boolean getop,
		   int32 indent)
{
    val_value_t       *curval;
    xpath_resnode_t   *resnode;

    resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);
    while (resnode) {
	curval = resnode->node.valptr;

	/* check corner case, output entire tree */
	if (curval == pcb->val_docroot) {
	    if (getop) {
		xml_wr_val(scb, &msg->mhdr, curval, indent);
	    } else {
		xml_wr_check_val(scb, &msg->mhdr, curval, 
				 indent, agt_check_config);
	    }
	    xpath_free_resnode(resnode);
	    return;
	}

	output_resnode(scb, msg, pcb,
		       &result->r.nodeQ,
		       curval, 
		       pcb->val_docroot,
		       getop, indent);

	xpath_free_resnode(resnode);

	resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);
    }

} /* output_result */


/************  E X T E R N A L    F U N C T I O N S    **************/


/********************************************************************
* FUNCTION agt_xpath_eval_filter
*
* Evaluate the XPath filter against the specified 
* config root, and output the result of the
* <get> or <get-config> operation to the specified session
*
* INPUTS:
*    scb == session control block
*    msg == rpc_msg_t in progress
*    cfg == config target to check against
*    getop  == TRUE if this is a <get> and not a <get-config>
*              The target is expected to be the <running> 
*              config, and all state data will be available for the
*              filter output.
*              FALSE if this is a <get-config> and only the 
*              specified target in available for filter output
*    indent == start indent amount
*
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_xpath_output_filter (ses_cb_t *scb,
			     rpc_msg_t *msg,
			     const cfg_template_t *cfg,
			     boolean getop,
			     int32 indent)
{
    val_value_t       *selectval;
    xpath_result_t    *result;
    status_t           res;

#ifdef DEBUG
    if (!scb || !msg || !cfg || !msg->rpc_filter.op_filter) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* make sure the config has some data in it */
    if (!cfg->root) {
	return NO_ERR;
    }

    res = NO_ERR;

    /* the msg filter should be non-NULL */
    selectval = msg->rpc_filter.op_filter;
    
    result = xpath1_eval_xmlexpr(scb->reader,
				 selectval->xpathpcb,
				 cfg->root,
				 cfg->root,
				 FALSE,
				 !getop,
				 &res);

    if (result && (res == NO_ERR) && 
	(result->restype == XP_RT_NODESET)) {

	/* prune result of redundant nodes */
	xpath1_prune_nodeset(selectval->xpathpcb, result);

	/* output filter */
	output_result(scb, 
		      msg, 
		      selectval->xpathpcb,
		      result, 
		      getop,
		      indent);
    }

    if (result) {
	xpath_free_result(result);
    }

    return res;

} /* agt_xpath_output_filter */


/* END file agt_xpath.c */
