/*  FILE: agt_tree.c


  NETCONF Subtree Filtering Implementation for NCX
	
  Step 1) val_parse_nc will parse the entire subtree filter 
          as type NCX_BT_ANY

       A filter is parsed as type 'any' as follows:

        NCX_BT_CONTAINER   -- container node
        NCX_BT_STRING   -- content match node
	NCX_BT_EMPTY    -- select node
        val->metaQ      -- attribute match expressions


  Step 2) agt_tree_prune_filter will traverse the filter val
          and compare it to the target config, figuring out
          if a node is TRUE or FALSE.  

          An ncx_filptr_t tree is built as this is done, to
          optimize node removal and rendering later on.

          The filter val_value_t is no longer used after this is done.
 
       - If a filter node has any child nodes, they must
         all be TRUE, for the node itself to be TRUE.
         If not, the filter node and all its children are removed

       - If access control is requested, then the scb->username
         will be checked against at 3 levels:
           - (owner, application)
           - (owner, application, parmset)
           - (owner, application, parmset, parm)
    
       - Each ncx_filptr_t node indicates 1 matching 
         instance at that level.  It should be possible
         to optimize away all the ncx_filptr_t records
         for all 'container of container' nodes.  Unless
         the child nodes contain 1 or more content select
         or attribute select expressions, all or none of
         the child nodes will match in this special case.
         (Left for a future project.)

   Step 3) agt_tree_output_filter will traverse the altered
           filter value node, and output the cached node
           instances from the target, if the node is not
           marked as deleted
           
   
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
16jun06      abb      begun; split out from agt_util.c

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

#ifndef _H_agt_tree
#include "agt_tree.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val
#include "agt_val.h"
#endif

#ifndef _H_b64
#include "b64.h"
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

#ifndef _H_tk
#include  "tk.h"
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


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define AGT_TREE_DEBUG 1


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION save_filptr
*
* create ncx_filptr_t struct and save it in the parent->childQ
* 
* INPUTS:
*    parent == filptr struct to save the nodeptr in
*    valnode == value node to save
*
* RETURNS:
*    pointer to new ncx_filptr_t struct that was saved
*    NULL if malloc error
*********************************************************************/
static ncx_filptr_t *
    save_filptr (ncx_filptr_t *parent,
		 val_value_t *valnode)
{
    ncx_filptr_t  *filptr;

    filptr = ncx_new_filptr();

    if (filptr) {
	filptr->node = valnode;
	dlq_enque(filptr, &parent->childQ);
    }

    return filptr;

} /* save_filptr */


/********************************************************************
* FUNCTION find_filptr
*
* find an ncx_filptr_t struct
* 
* INPUTS:
*    parent == parent with the Q of ncx_filptr_t to search
*    valnode == value node to find
*
* RETURNS:
*    pointer to found ncx_filptr_t struct
*    NULL if not found
*********************************************************************/
static ncx_filptr_t *
    find_filptr (ncx_filptr_t *parent,
		 val_value_t *valnode)
{
    ncx_filptr_t  *filptr;

    for (filptr = (ncx_filptr_t *)dlq_firstEntry(&parent->childQ);
	 filptr != NULL;
	 filptr = (ncx_filptr_t *)dlq_nextEntry(filptr)) {

	if (filptr->node == valnode) {
	    return filptr;
	}
    }

    return NULL;

} /* find_filptr */


/********************************************************************
* FUNCTION attr_test
*
* Check any attribute match expressions
*
* INPUTS:
*    filval == filter node value
*    targval == corresponding node from the target
*
* RETURNS:
*    TRUE if no tests failed
*    FALSE if any tests fail (exits at first failure)
*********************************************************************/
static boolean
    attr_test (const val_value_t *filval,
	       const val_value_t  *targval)
{
    const val_value_t  *m1;
    const dlq_hdr_t    *metaQ;
    xmlns_id_t          invid;
    boolean             done, ret;

    invid = xmlns_inv_id();

    metaQ = val_get_metaQ(filval);
    if (!metaQ) {
	return TRUE;
    }

    ret = TRUE;
    done = FALSE;

    for (m1 = val_get_first_meta(metaQ);
	 m1 != NULL && !done;
	 m1 = val_get_next_meta(m1)) {
	if (m1->nsid == invid) {
	    ret = FALSE;
	    done = TRUE;
	} else if (!val_meta_match(targval, m1)) {
	    ret = FALSE;
	    done = TRUE;
	}
    }
    return ret;

}  /* attr_test */


/********************************************************************
* FUNCTION content_match_test
*
* Check a content match node against the corresponding 
* node in the target.
*
* INPUTS:
*    scb == session control block
*    testval == string to compare with
*    curval == target node to compare against
*
* RETURNS:
*    TRUE if content match test OK
*    FALSE if content different or target is a complex data
*          type and not a simple type
*********************************************************************/
static boolean
    content_match_test (ses_cb_t *scb,
			const xmlChar *testval, 
			val_value_t *curval)
{
    const val_value_t  *cmpval;
    val_value_t        *v_val;
    xmlChar            *binbuff;
    boolean             testres;
    status_t            res;
    uint32              binlen, retlen, testlen;
    ncx_num_t           num;

    v_val = NULL;

    /* skip matches of any password object */
    if (obj_is_password(curval->obj)) {
	return FALSE;
    }

    /* handle virtual compare differently */
    if (val_is_virtual(curval)) {
	/* get temp value to store virtual value */
	v_val = val_get_virtual_value(scb, curval, &res);
	if (!v_val) {
	    log_error("\agt_tree: get virtual failed %s",
		      get_error_string(res));
	    return FALSE;
	}
    }

    testres = FALSE;
    cmpval = (v_val) ? v_val : curval;

    switch (cmpval->btyp) {
    case NCX_BT_BOOLEAN:
	if (!xml_strcmp(testval, NCX_EL_TRUE) ||
	    !xml_strcmp(testval, (const xmlChar *)"1")) {
	    testres = cmpval->v.bool;
	} else if (!xml_strcmp(testval, NCX_EL_FALSE) ||
		   !xml_strcmp(testval, (const xmlChar *)"0")) {
	    testres = !cmpval->v.bool;
	}
	break;
    case NCX_BT_BINARY:
	binlen = xml_strlen(testval);
	binbuff = m__getMem(binlen);
	if (!binbuff) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	    return 0;
	}
	res = b64_decode(testval, binlen,
			 binbuff, binlen, &retlen);
	if (res == NO_ERR) {
	    testres = !memcmp(binbuff, cmpval->v.binary.ustr, retlen);
	} else {
	    SET_ERROR(res);
	    testres = 0;
	}
	m__free(binbuff);
	break;
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
	ncx_init_num(&num);
	res = ncx_decode_num(testval, cmpval->btyp, &num);
	if (res == NO_ERR) {
	    testres = !ncx_compare_nums(&num, &cmpval->v.num, 
					curval->btyp) ? TRUE : FALSE;
	}
	ncx_clean_num(cmpval->btyp, &num);
	break;
    case NCX_BT_ENUM:
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_IDREF:
    case NCX_BT_LEAFREF: /****/
	testlen = xml_strlen(testval);
	retlen = 0;
	res = val_sprintf_simval_nc(NULL, cmpval, &retlen);
	if (res == NO_ERR && retlen == testlen) {
	    binbuff = m__getMem(retlen+1);
	    if (binbuff) {
		res = val_sprintf_simval_nc(binbuff, cmpval, &retlen);
		if (res == NO_ERR) {
		    testres = (xml_strcmp(binbuff, testval)) 
			? FALSE : TRUE;
		}
		m__free(binbuff);
	    }
	}
	break;
    case NCX_BT_BITS:
    case NCX_BT_SLIST:
    case NCX_BT_EMPTY:
    case NCX_BT_CONTAINER:
    case NCX_BT_LIST:
    default:
	;   /* test is automatically FALSE for these data types */
    }

    if (v_val) {
	val_free_value(v_val);
    }

    return testres;

} /* content_match_test */


/********************************************************************
* FUNCTION process_val
*
* Evaluate the subtree and remove nodes
* which are not in the result set
*
* The filval is a NCX_BT_CONTAINER, and already matched 
* to the 'curnode'.  This function evaluates the child nodes
* recursively as more container nodes are matched to the target
*
* INPUTS:
*    scb == session control block
*        == NULL if no read access control is desired
*    getop  == TRUE if this is a <get> and not a <get-config>
*              The target is expected to be the <running> 
*              config, and all state data will be available for the
*              filter output.
*              FALSE if this is a <get-config> and only the 
*              specified target in available for filter output
*    filval == filter node
*    curval == current database node
*    result == filptr tree result to fill in
*    keepempty == address of return keepempty flag
*
* OUTPUTS:
*    *result is filled in as needed
*     only 'true' result filter nodes should be remaining 
*    *keepempty is set to TRUE if 
* RETURNS:
*     status, NO_ERR or malloc error
*********************************************************************/
static status_t
    process_val (ses_cb_t *scb,
		 boolean getop,
		 val_value_t *filval,
		 val_value_t *curval,
		 ncx_filptr_t *result,
		 boolean *keepempty)
{
    val_value_t      *filchild, *curchild;
    val_index_t      *valindex;
    ncx_filptr_t     *filptr;
    boolean           test, anycon, anycm, anysel, mykeepempty;
    xmlns_id_t        ncid;
    status_t          res;

    res = NO_ERR;
    *keepempty = FALSE;

    /* little hack: treat a filter node with the 
     * NETCONF namespace as if it were set to 0
     * this will happen if the manager is lazy
     * and just set the NETCONF namespace at
     * the top level, and uses it for everything
     */
    ncid = xmlns_nc_id();

    /* The filval is the same level as the curval
     *
     * Go through and check all the child nodes of the
     * filval (sibling set) against all the children
     * of the curval.  Determine which nodes to keep
     * and save ncx_filptr_t structs for those nodes
     */
    anycon = FALSE;
    anycm = FALSE;
    anysel = FALSE;

    /* check any content match nodes first
     * they must all be true or this entire sibling
     * set is rejected
     */
    for (filchild = val_get_first_child(filval);
	 filchild != NULL;
	 filchild = val_get_next_child(filchild)) {

	if (filchild->nsid == ncid) {
	    /* no content in NETCONF namespace, so assume
	     * that no namespace was used and the filter NSID
	     * has been passed down to this point
	     */
	    filchild->nsid = 0;
	}

	/* skip all but content match nodes */
	switch (filchild->btyp) {
	case NCX_BT_STRING:
	    anycm = TRUE;
	    break;
	case NCX_BT_EMPTY:
	    anysel = TRUE;
	    continue;
	case NCX_BT_CONTAINER:
	    anycon = TRUE;
	    continue;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}

	/* check corner case not caught by XML pares */
	if (val_all_whitespace(VAL_STR(filchild))) {
	    /* should not happen! */
	    SET_ERROR(ERR_INTERNAL_VAL);
	    continue;
	}

	/* This is a valid content select node 
	 * Need to compare it to the current node
	 * based on the target data type.
	 * The filter data type is always going to
	 * be string and this needs to be 
	 * compared to a sprintf of the target value
	 * Compare the string value to all instances
	 * of the 'filchild' node; need just 1 match
	 */
	test = FALSE;
	for (curchild = 
		 val_first_child_qname(curval, 
				       filchild->nsid,
				       filchild->name);
	     curchild != NULL && !test;
	     curchild = val_next_child_qname(curval,
					     filchild->nsid,
					     filchild->name,
					     curchild)) {

#ifdef NOT_YET	
	    /* check access control if scb is non-NULL */
	    if (scb && !agt_acm_val_read_allowed(scb->username, 
						 curchild)) {
		/* treat an access-failed on a content match
		 * test as a termination trigger
		 */
		return NO_ERR;
	    }
#endif
	    test = content_match_test(scb, 
				      VAL_STR(filchild), 
				      curchild);
	}

	if (!test) {
#ifdef AGT_TREE_DEBUG
	    log_debug2("\nagt_tree_process_val: %s "
		       "sibling set pruned; CM not found for '%s'", 
		       filval->name, filchild->name);
#endif
	    return NO_ERR;
	}
    }

    /* at this point any content match node tests have passed
     * except any attribute match tests in the content nodes
     * have not been tested yet.  They are AND tests.
     * RFC 4741 is not that clear, but this means the content
     * match is not ever skipped because of failed match tests
     *
     * Check if there are no more tests; If not, all the
     * child nodes of curval are selected
     */
    if (!anycon && !anysel) {
	*keepempty = TRUE;
	return NO_ERR;
    }

    /* Go through the filval child nodes again and this
     * time select nodes for real
     */
    for (filchild = val_get_first_child(filval);
	 filchild != NULL;
	 filchild = val_get_next_child(filchild)) {

	/* first check if this is a get-config operation 
	 * and if so, if the test node fails the config test 
	 */
	if (!getop && !agt_check_config(ses_withdef(scb), 
					NCX_NT_VAL, 
					filchild)) {
	    continue;
	}

	/* go through all the actual instances of 'filchild'
	 * within the child nodes of 'curval'
	 */
	for (curchild = val_first_child_qname(curval, 
					      filchild->nsid,
					      filchild->name);
	     curchild != NULL;
	     curchild = val_next_child_qname(curval,
					     filchild->nsid,
					     filchild->name,
					     curchild)) {
	    
	    filptr = NULL;

#ifdef NOT_YET	
	    /* check access control if scb is non-NULL */
	    if (scb && !agt_acm_val_read_allowed(scb->username, 
						 testval)) {
		continue;
	    }
#endif

	    /* check any attr-match tests */
	    if (!attr_test(filchild, curchild)) {
		/* failed an attr-match test so skip it */
		continue;
	    }

	    /* process the filter child node based on its type */
	    switch (filchild->btyp) {
	    case NCX_BT_STRING:
		/* This is a content select node */
		test = content_match_test(scb, 
					  VAL_STR(filchild), 
					  curchild);
		if (!test) {
		    break;
		} /* else fall through and add node */
	    case NCX_BT_EMPTY:
		/* this is a select node  matched to the 
		 * current node, so save it
		 */
		filptr = save_filptr(result, curchild);
		if (!filptr) {
		    return ERR_INTERNAL_MEM;
		}
		break;
	    case NCX_BT_CONTAINER:
		/* this is a container node, so the current node
		 * in the target must be a complex type; not a leaf node
		 */
		if (!typ_has_children(curchild->btyp)) {
		    break;
		}

		/* save this node for now */
		filptr = save_filptr(result, curchild);
		if (!filptr) {
		    return ERR_INTERNAL_MEM;
		}

		/* go through the child nodes of the filter
		 * and compare to the complex target 
		 */
		res = process_val(scb, getop, filchild,
				  curchild, filptr, 
				  &mykeepempty);
		if (res != NO_ERR) {
		    return res;
		}

		if (!mykeepempty && dlq_empty(&filptr->childQ)) {
		    dlq_remove(filptr);
		    ncx_free_filptr(filptr);
		    filptr = NULL;
		}
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    if (filptr && 
		filptr->node->btyp == NCX_BT_LIST &&
		!dlq_empty(&filptr->childQ)) {

		/* make sure all the list keys (if any)
		 * are present, since specific nodes
		 * are requested, and the keys could
		 * get filtered out
		 */
		for (valindex = val_get_first_index(filptr->node);
		     valindex != NULL;
		     valindex = val_get_next_index(valindex)) {
		    
		    if (!find_filptr(filptr, valindex->val)) {
			if (!save_filptr(filptr, valindex->val)) {
			    return ERR_INTERNAL_MEM;
			}
		    }
		}
	    }
	}
    }

    return NO_ERR;

} /* process_val */


/********************************************************************
* FUNCTION output_node
*
* Output the current node to the specified session
*
* INPUTS:
*    scb == session control block
*    msg == rpc_msg_t in progress
*    parent == parent filter chain node
*    indent == start indent amount
*    getop == TRUE if <get>, FALSE if <get-config>
*
* RETURNS:
*    none
*********************************************************************/
static void
    output_node (ses_cb_t *scb, 
		 rpc_msg_t *msg, 
		 ncx_filptr_t *parent,
		 int32 indent,
		 boolean getop)
{
    ncx_filptr_t  *filptr;
    val_value_t   *val;
    xmlns_id_t     parentnsid, valnsid;
    int32          indentamount;

    indentamount = ses_indent_count(scb);

    for (filptr = (ncx_filptr_t *)dlq_firstEntry(&parent->childQ);
	 filptr != NULL;
	 filptr = (ncx_filptr_t *)dlq_nextEntry(filptr)) {
	
	val = filptr->node;
	valnsid = obj_get_nsid(val->obj);
	if (val->parent) {
	    parentnsid = obj_get_nsid(val->parent->obj);
	} else {
	    parentnsid = 0;
	}

	if (dlq_empty(&filptr->childQ)) {
	    if (getop) {
		xml_wr_full_val(scb, &msg->mhdr, val, indent);
	    } else {
		xml_wr_full_check_val(scb, &msg->mhdr, 
				      val, indent,
				      agt_check_config);
	    }
	} else {
	    /* check the child nodes in the filter
	     * If there are container nodes or select nodes
	     * then only those specific nodes will be output
	     *
	     * If there are only content match nodes then
	     * the entire filval node is supposed to be output
	     */

	    /* else 1 or more containers and/or select nodes */
	    xml_wr_begin_elem_ex(scb, 
				 &msg->mhdr,
				 parentnsid,
				 valnsid,
				 val->name, 
				 &val->metaQ, 
				 FALSE, 
				 indent, 
				 FALSE);

	    if (indent >= 0) {
		indent += indentamount;
	    }

	    output_node(scb, msg, filptr, indent, getop);	    

	    if (indent >= 0) {
		indent -= indentamount;
	    }

	    xml_wr_end_elem(scb, 
			    &msg->mhdr, 
			    valnsid,
			    val->name, 
			    indent);
	}
    }
}  /* output_node */


#ifdef AGT_TREE_DEBUG
/********************************************************************
* FUNCTION dump_filptr_node
*
* Output the ncx_filptr_t node to STDOUT
*
* INPUTS:
*  filptr == filptr node to dump
*
* RETURNS:
*    none
*********************************************************************/
static void
    dump_filptr_node (ncx_filptr_t  *filptr,
		      int32 indent)
{
    ncx_filptr_t        *fp;
    int32                i;

    log_debug2("\n");
    for (i=0; i<indent; i++) {
	log_debug2(" ");
    }

    log_debug2("filptr: %u:%s %s",  
	       filptr->node->nsid,
	       filptr->node->name,
	       tk_get_btype_sym(filptr->node->btyp));


    for (fp = (ncx_filptr_t *)dlq_firstEntry(&filptr->childQ);
	 fp != NULL;
	 fp = (ncx_filptr_t *)dlq_nextEntry(fp)) {
	dump_filptr_node(fp, indent+2);
    }
    
}  /* dump_filptr_node */
#endif


/************  E X T E R N A L    F U N C T I O N S    **************/


/********************************************************************
* FUNCTION agt_tree_prune_filter
*
* Need to evaluate the entire subtree filter and remove nodes
* which are not in the result set
*
* The filter subtree usually starts out as type NCX_BT_CONTAINER
* but a single select node could be present instead.
*
* The <filter> subtree starts out as type NCX_BT_CONTAINER (root)
* and is converted by val_parse as follows:
*
*   Subtree Filter:
*     NCX_BT_ANY -- start type
*   changed to real types during parsing
*     NCX_BT_CONTAINER   -->  container node
*     NCX_BT_EMPTY    -->  select node
*     NCX_BT_STRING  -->  content select node
*
* INPUTS:
*    scb == session control block
*         == NULL if access control should not be applied
*    msg == rpc_msg_t in progress
*    cfg == config target to check against
*    getop  == TRUE if this is a <get> and not a <get-config>
*              The target is expected to be the <running> 
*              config, and all state data will be available for the
*              filter output.
*              FALSE if this is a <get-config> and only the 
*              specified target in available for filter output
*
* OUTPUTS:
*    msg->rpc_filter.op_filter is pruned as needed by setting
*    the VAL_FL_FILTERED bit.in the val->flags field
*    for the start of subtrees which failed the filter test.
*    Only nodes which result in non-NULL output should
*    remain unchanged at the end of this procedure.
*
* RETURNS:
*    pointer to generated tree of matching nodes
*    NULL if no match
*********************************************************************/
ncx_filptr_t *
    agt_tree_prune_filter (ses_cb_t *scb,
			   rpc_msg_t *msg,
			   const cfg_template_t *cfg,
			   boolean getop)
{
    val_value_t       *filter;
    ncx_filptr_t      *top;
    status_t           res;
    boolean            keepempty;

#ifdef DEBUG
    if (!msg || !cfg || !msg->rpc_filter.op_filter) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* make sure the config has some data in it */
    if (!cfg->root) {
	return NULL;
    }

    /* the msg filter should be non-NULL */
    filter = msg->rpc_filter.op_filter;
    
    /* start at the top with <filter> itself */
    switch (filter->btyp) {
    case NCX_BT_EMPTY:
	/* This is an empty filter element; 
	 * This is allowed, but the result is the empty set
	 */
	break;
    case NCX_BT_STRING:
	/* This is a mixed mode request, which is supposed to
	 * be invalid; In this case it is simply interpreted
	 * as 'not a match', because all NCX data is in XML.
	 * This is not really allowed, and the result is the empty set
	 */
	break;
    case NCX_BT_CONTAINER:
	/* This is the normal case - a container node
	 * Go through the child nodes.
	 */
	top = ncx_new_filptr();
	if (!top) {
	    return NULL;
	}
	top->node = cfg->root;
	
	res = process_val(scb, getop, filter, 
			  cfg->root, top, &keepempty);
	if (res != NO_ERR || dlq_empty(&top->childQ)) {
	    /* ignore keepempty because the result will
	     * be the same w/NULL return, just faster
	     */
	    ncx_free_filptr(top);
	} else {
#ifdef AGT_TREE_DEBUG
	    dump_filptr_node(top, 0);
#endif
	    return top;
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    return NULL;

} /* agt_tree_prune_filter */


/********************************************************************
* FUNCTION agt_tree_output_filter
*
* Output the pruned subtree filter to the specified session
*
* INPUTS:
*    scb == session control block
*         == NULL if access control should not be applied
*    msg == rpc_msg_t in progress
*    top == ncx_filptr tree to output
*    indent == start indent amount
*    getop == TRUE if <get>, FALSE if <get-config>
*
* RETURNS:
*    none
*********************************************************************/
void
    agt_tree_output_filter (ses_cb_t *scb,
			    rpc_msg_t *msg,
			    ncx_filptr_t *top,
			    int32 indent,
			    boolean getop)
{
#ifdef DEBUG
    if (!scb || !msg || !top) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    output_node(scb, msg, top, indent, getop);
    
} /* agt_tree_output_filter */


/* END file agt_tree.c */
