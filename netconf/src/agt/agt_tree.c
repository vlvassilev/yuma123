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


static boolean
    prune_subtree (const ses_cb_t *scb,
		   val_value_t *filval,
		   boolean getop,
		   ncx_filptr_t *parent);

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define AGT_TREE_DEBUG 1 */


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

#ifdef AGT_TREE_DEBUG
static uint32 debugcnt = 0;
#endif


/********************************************************************
* FUNCTION save_filptr
*
* create ncx_filptr_t struct and save it in the filptr->childQ
* 
* INPUTS:
*    parent == filptr struct to save the nodeptr in
*    filval == the filter node that matched
*    nodetyp == enumerated type of 'node'
*    node == node to save
*
* RETURNS:
*    pointer to new ncx_filptr_t struct that was savedstatus
*********************************************************************/
static ncx_filptr_t *
    save_filptr (ncx_filptr_t *parent,
		 val_value_t  *filval,
		 ncx_node_t  nodetyp,
		 void *node)
{
    ncx_filptr_t  *filptr;

    filptr = ncx_new_filptr();
    if (!filptr) {
	return NULL;
    }
    filptr->btyp = filval->btyp;
    filptr->nsid = filval->nsid;
    filptr->nodetyp = nodetyp;
    filptr->node = node;
    dlq_enque(filptr, &parent->childQ);
    return filptr;

} /* save_filptr */


/********************************************************************
* FUNCTION next_filptr
*
* go to the next ncx_filptr_t struct in the Q
* remove the current node and free it if requested
* 
* INPUTS:
*    filptr == current filptr node
*    discard == TRUE if the current should be removed from the
*               the Q and discarded
*               FALSE if it should be left in the Q
* RETURNS:
*    pointer to the next filptr or NULL if no next
*********************************************************************/
static ncx_filptr_t *
    next_filptr (ncx_filptr_t *filptr,
		 boolean discard)
{
    ncx_filptr_t  *fp;

    fp = (ncx_filptr_t *)dlq_nextEntry(filptr);
    if (discard) {
	dlq_remove(filptr);
	ncx_free_filptr(filptr);
    }
    return fp;

} /* next_filptr */


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
    content_match_test (const ses_cb_t *scb,
			const xmlChar *testval, 
			val_value_t *curval)
{
    const val_value_t  *cmpval;
    val_value_t  *v_val;
    xmlChar     *binbuff;
    ncx_num_t  num;
    ncx_enum_t  enu1;
    boolean   testres;
    status_t  res;
    uint32    binlen, retlen;

    v_val = NULL;

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

    switch (curval->btyp) {
    case NCX_BT_BOOLEAN:
	if (!xml_strcmp(testval, NCX_EL_TRUE) ||
	    !xml_strcmp(testval, (const xmlChar *)"1")) {
	    testres = cmpval->v.bool;
	} else if (!xml_strcmp(testval, NCX_EL_FALSE) ||
		   !xml_strcmp(testval, (const xmlChar *)"0")) {
	    testres = !cmpval->v.bool;
	}
	break;
    case NCX_BT_ENUM:
	/* convert the test string to an enum */
	res = ncx_set_enum(testval, &enu1);
	if (res != NO_ERR) {
	    break;
	}
	testres = (!ncx_compare_enums(&enu1, &cmpval->v.enu))
			? TRUE : FALSE;
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
	res = ncx_decode_num(testval, curval->btyp, &num);
	if (res == NO_ERR) {
	    testres = !ncx_compare_nums(&num, &cmpval->v.num, 
					curval->btyp) ? TRUE : FALSE;
	}
	break;
    case NCX_BT_STRING:
	testres = !xml_strcmp((const xmlChar *)VAL_STR(cmpval), testval);
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
    case NCX_BT_SLIST:
	SET_ERROR(ERR_NCX_OPERATION_NOT_SUPPORTED);
	break;
    case NCX_BT_EMPTY:
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
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
* FUNCTION check_sibling_set
*
* First stage evaluation of the sibling nodes within a subtree filter
* to see what mix is present.
*
* INPUTS:
*    filval == filter node value
*
* OUTPUTS:
*    *anycon == TRUE if any container nodes found
*    *anycm  == TRUE if any content match nodes found
*    *anysel == TRUE if any select nodes found
*
* RETURNS:
*    none
*********************************************************************/
static void
    check_sibling_set (const val_value_t *filval,
		       boolean *anycon,
		       boolean *anycm,
		       boolean *anysel)
{
    const val_value_t  *chval;

    *anycon = FALSE;
    *anycm = FALSE;
    *anysel = FALSE;

    for (chval = val_get_first_child(filval);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {
	switch (chval->btyp) {
	case NCX_BT_EMPTY:
	    *anysel = TRUE;
	    break;
	case NCX_BT_STRING:
	    *anycm = TRUE;
	    break;
	case NCX_BT_CONTAINER:
	    *anycon = TRUE;
	    break;
	case NCX_BT_NONE:
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
}  /* check_sibling_set */


/********************************************************************
* FUNCTION check_fp_sibling_set
*
* First stage evaluation of the sibling nodes within a subtree filter
* to see what mix is present. Run test on ncx_filptr_t tree
* instead of a val_avlue_t tree
*
* INPUTS:
*    filptr == filter node value
*
* OUTPUTS:
*    *anycon == TRUE if any container nodes found
*    *anycm  == TRUE if any content match nodes found
*    *anysel == TRUE if any select nodes found
*
* RETURNS:
*    none
*********************************************************************/
static void
    check_fp_sibling_set (const ncx_filptr_t *filptr,
			  boolean *anycon,
			  boolean *anycm,
			  boolean *anysel)
{
    const ncx_filptr_t  *fp;

    *anycon = FALSE;
    *anycm = FALSE;
    *anysel = FALSE;

    for (fp = (const ncx_filptr_t *)dlq_firstEntry(&filptr->childQ);
	 fp != NULL;
	 fp = (const ncx_filptr_t *)dlq_nextEntry(fp)) {
	switch (fp->btyp) {
	case NCX_BT_EMPTY:
	    *anysel = TRUE;
	    break;
	case NCX_BT_STRING:
	    *anycm = TRUE;
	    break;
	case NCX_BT_CONTAINER:
	    *anycon = TRUE;
	    break;
	case NCX_BT_NONE:
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
}  /* check_fp_sibling_set */


/********************************************************************
* FUNCTION match_val
*
* Match the request filter node to a nested child node in the 
* specified target parm
*
* INPUTS:
*    filval == current filter value node, which should
*            represent a parm node
*    curval == current value node from the target
*    parent == filptr parent to save this child filptr in
* 
* OUTPUTS:
*    a ncx_filptr_t struct is malloced and added to the 
*    parent->childQ for each instance of the found val
*
* RETURNS:
*    number of matches found
*********************************************************************/
static uint32
    match_val (val_value_t *filval, 
	       const val_value_t *curval,
	       ncx_filptr_t  *parent)
{
    val_value_t  *chcurval;
    ncx_filptr_t       *fp;
    status_t            res;
    uint32              found;

    found = 0;
    /* make sure the current target node is a complex type */
    if (!typ_has_children(curval->btyp)) {
	return 0;
    }

    /* check all child nodes and save all the ones that match */
    for (chcurval = val_get_first_child(curval);
	 chcurval != NULL;
	 chcurval = val_get_next_child(chcurval)) {
	
	if (!xml_strcmp(chcurval->name, filval->name)) {
	    if (attr_test(filval, chcurval)) {
		fp = save_filptr(parent, filval, NCX_NT_VAL, chcurval);
		if (!fp) {
		    SET_ERROR(res);
		    return found;
		} else {
		    found++;
		}
	    } 
	} 
    }
    return found;

} /* match_val */


/********************************************************************
* FUNCTION content_match_val
*
* Match the request content match node to a nested child node in the 
* specified target parm.  If the nested target node is a simple
* type and it passes the content match test
*
* INPUTS:
*    scb == session control block
*    filval == current filter value node, which should
*            represent a parm node
*    curval == current value node from the target
*    parent == filptr parent to save this child filptr in
* 
* OUTPUTS:
*    a ncx_filptr_t struct is malloced and added to the 
*    parent->childQ for each instance of the found matching val
*
* RETURNS:
*    number of matches found
*********************************************************************/
static uint32
    content_match_val (const ses_cb_t *scb,
		       val_value_t *filval, 
		       val_value_t *curval,
		       ncx_filptr_t *parent)
{
    val_value_t  *chcurval;
    ncx_filptr_t       *fp;
    uint32              found;

    found = 0;

    /* make sure the current target node is a simple type */
    if (!typ_has_children(curval->btyp)) {
	return 0;
    }

    for (chcurval = val_get_first_child(curval);
	 chcurval != NULL;
	 chcurval = val_get_next_child(chcurval)) {

	if (!xml_strcmp(chcurval->name, filval->name)) {
	    if (attr_test(filval, chcurval)) {
		if (content_match_test(scb, VAL_STR(filval), chcurval)) {
		    fp = save_filptr(parent, filval, NCX_NT_VAL, chcurval);
		    if (!fp) {
			SET_ERROR(ERR_INTERNAL_MEM);
			return found;
		    } else {
			found++;
		    }
		} 
	    } 
	}
    }
    return found;

} /* content_match_val */


/********************************************************************
* FUNCTION prune_val
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
*    filval == filter value (w/ indexQ of ncx_filptr_t nodes)
*    getop  == TRUE to include all data
*           == FALSE to include just writable data
*
* OUTPUTS:
*    fil is pruned as needed
*    only 'true' result filter nodes should be remaining 
*
* RETURNS:
*      TRUE if the filval node itself is a TRUE filter, because
*           at least one of its child node paths is TRUE
*      FALSE if the filval node and all its childen are a FALSE
*        filter and should be removed from the output
*********************************************************************/
static boolean
    prune_val (const ses_cb_t *scb,
	       val_value_t *filval,
	       boolean getop,
	       ncx_filptr_t  *parent)
{
    val_value_t  *val;
    val_value_t        *chval;
    ncx_filptr_t       *filptr, *fp, *chfp;
    boolean             test, done;

    /* The childQ contains all possible filptr instance matches 
     * from the target data model for the 'filval'.
     *
     * Go through and try to eliminate all the instances
     * that do not match nested nodes in the filter.
     *
     * Mark the node as filtered if there are no instances
     * remaining at the end of the process
     */
    filptr = (ncx_filptr_t *)dlq_firstEntry(&parent->childQ);
    while (filptr) {
	/* The filter node can be any type of node
	 * It matches the 'val' node.  Any child
	 * nodes in the filter must match child
	 * nodes in the target node
	 */
	if (filptr->nodetyp != NCX_NT_VAL) {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return FALSE;
	} else {
	    val = (val_value_t *)filptr->node;
	}

#ifdef AGT_TREE_DEBUG
	log_debug("\nagt_tree_prune_val: %s %u", filval->name, debugcnt++);
	/* val_dump_value(val, NCX_DEF_INDENT); */
#endif

	/* first check if this is a get-config operation 
	 * and if so, if it fails the config test 
	 */
	if (!getop && !agt_check_config(ses_withdef(scb), 
					NCX_NT_VAL, val)) {
	    /* not a config node so skip it */
	    filptr = next_filptr(filptr, TRUE);
	    continue;
	}
	
	/* next check any attr-match tests */
	if (!attr_test(filval, val)) {
	    /* failed an attr-match test so skip it */
	    filptr = next_filptr(filptr, TRUE);
	    continue;
	}

	switch (filval->btyp) {
	case NCX_BT_EMPTY:
	    /* this is a select node which has already been 
	     * matched to the current node; nothing else to do
	     */
	    filptr = next_filptr(filptr, FALSE);
	    break;
	case NCX_BT_STRING:
	    /* This is a content select node 
	     * Need to compare it to the current node
	     * based on the target data type.
	     * The filter data type is always going to
	     * be string and this needs to be 
	     * compared to a sprintf of the target value
	     */
	    test = content_match_test(scb, VAL_STR(filval), val);
	    filptr = next_filptr(filptr, !test);
	    break;
	case NCX_BT_CONTAINER:
	    /* this is a container node, so the current node
	     * in the target must be a complex type; not a leaf node
	     */
	    if (!obj_is_root(val->obj)) {
		if (!typ_has_children(val->btyp)) {
		    filptr = next_filptr(filptr, TRUE);
		    break;
		}
	    }

	    /* go through the child nodes of the filter
	     * and compare to the complex target 
	     */
	    done = FALSE;
	    for (chval = val_get_first_child(filval);
		 chval != NULL && !done;
		 chval = val_get_next_child(chval)) {

		/* check namespace */
		if (!obj_is_root(val->obj) &&
		    chval->nsid != filval->nsid) {
		    continue;
		}

		/* add another layer to the filptr chain
		 * for the child node header
		 ********************************* !!!!!!!!!!! ***************************
		 */
		fp = save_filptr(filptr, chval, NCX_NT_CHILD, NULL);
		if (!fp) {
		    SET_ERROR(ERR_INTERNAL_MEM);
		    return !dlq_empty(&parent->childQ);
		}

		switch (chval->btyp) {
		case NCX_BT_EMPTY:
		    if (!match_val(chval, val, fp)) {
			dlq_remove(fp);
			ncx_free_filptr(fp);
		    }
		    break;
		case NCX_BT_STRING:
		    if (!content_match_val(scb, chval, val, fp)) {
			/* this whole filptr is removed
			 * if a child select node fails
			 */
			filptr = next_filptr(filptr, TRUE);
			done = TRUE;
		    }
		    break;
		case NCX_BT_CONTAINER:
		    if (match_val(chval, val, fp)) {
			chfp = (ncx_filptr_t *)dlq_firstEntry(&fp->childQ);
			while (chfp) {
			    test = prune_subtree(scb, chval, getop, chfp);
			    chfp = next_filptr(chfp, !test);
			}
		    }
		    if (dlq_empty(&fp->childQ)) {
			dlq_remove(fp);
			ncx_free_filptr(fp);
		    }
		    break;
		default:
		    SET_ERROR(ERR_INTERNAL_VAL);
		    return !dlq_empty(&parent->childQ);
		}
	    }
	    if (!done) {
		filptr = next_filptr(filptr, dlq_empty(&filptr->childQ));
	    }
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    return !dlq_empty(&parent->childQ);

} /* prune_val */



/********************************************************************
* FUNCTION prune_subtree
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
*    filval == filter value (w/ indexQ of ncx_filptr_t nodes)
*    getop  == TRUE to include monitor sets and parmsets
*           == FALSE to include just parmsets
*    parent == ncx_filptr_t that will contain any found nodes
*
* OUTPUTS:
*    fil is pruned as needed
*    only 'true' result filter nodes should be remaining 
*
* RETURNS:
*      TRUE if the filval node itself is a TRUE filter, because
*           at least one of its child node paths is TRUE
*      FALSE if the filval node and all its childen are a FALSE
*        filter and should be removed from the output
*********************************************************************/
static boolean
    prune_subtree (const ses_cb_t *scb,
		   val_value_t *filval,
		   boolean getop,
		   ncx_filptr_t *parent)
{
    boolean       retval;

    switch (parent->nodetyp) {
    case NCX_NT_VAL:
	retval = prune_val(scb, filval, getop, parent);
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	retval = FALSE;
    }    
    return retval;

} /* prune_subtree */


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
*
* RETURNS:
*    none
*********************************************************************/
static void
    output_node (ses_cb_t *scb, 
		 rpc_msg_t *msg, 
		 ncx_filptr_t  *parent,
		 int32 indent)
{
    ncx_filptr_t  *filptr;
    val_value_t   *val;
    boolean        empty, anycon, anycm, anysel;

    for (filptr = (ncx_filptr_t *)dlq_firstEntry(&parent->childQ);
	 filptr != NULL;
	 filptr = (ncx_filptr_t *)dlq_nextEntry(filptr)) {
	
	switch (filptr->nodetyp) {
	case NCX_NT_VAL:
	    val = (val_value_t *)filptr->node;

	    if (filptr->btyp != NCX_BT_CONTAINER) {
		xml_wr_value_elem(scb, &msg->mhdr, val, parent->nsid,
				  filptr->nsid,  val->name, 
				  &val->metaQ, FALSE, indent);
		break;
	    } else {
		/* check the child nodes in the filter
		 * If there are container nodes or select nodes
		 * then only those specific nodes will be output
		 *
		 * If there are only content match nodes then
		 * the entire filval node is supposed to be output
		 */
		check_fp_sibling_set(filptr, &anycon, &anycm, &anysel);
		if (!anycon && !anysel) {
		    xml_wr_value_elem(scb, &msg->mhdr, val, 
				      parent->nsid, filptr->nsid,
				      val->name, &val->metaQ, FALSE, indent);
		    break;
		}

		/* else 1 or more containers and/or select nodes */
		empty = dlq_empty(&filptr->childQ);
		xml_wr_begin_elem_ex(scb, &msg->mhdr, 
				     parent->nsid, filptr->nsid,
				     val->name, &val->metaQ, FALSE, 
				     indent, empty);
 		if (empty) {
		    break;
		}
	    }

	    /* non-empty filter struct */
	    if (indent >= 0) {
		indent += NCX_DEF_INDENT;
	    }
	    output_node(scb, msg, filptr, indent);	    
	    if (indent >= 0) {
		indent -= NCX_DEF_INDENT;
	    }
	    xml_wr_end_elem(scb, &msg->mhdr, filptr->nsid, val->name, indent);
	    break;
	case NCX_NT_CHILD:
	    output_node(scb, msg, filptr, indent);
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return;
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

    log_debug('\n');
    for (i=0; i<indent; i++) {
	log_debug(' ');
    }

    log_debug("filptr: %u %u %u",
	   filptr->btyp, filptr->nsid, filptr->nodetyp);

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
    val_value_t       *fil;
    ncx_filptr_t      *top;
    boolean            anycon, anycm, anysel;

#ifdef DEBUG
    if (!msg || !cfg || !msg->rpc_filter.op_filter) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* the msg filter should be non-NULL */
    fil = msg->rpc_filter.op_filter;
    
    /* make sure the config has some data in it */
    if (!cfg->root) {
	return NULL;
    }

    /* start at the top with <filter> itself */
    switch (fil->btyp) {
    case NCX_BT_EMPTY:
	/* This is an empty filter element; 
	 * This is allowed, but the result is the empty set
	 */
	return NULL;
    case NCX_BT_STRING:
	/* This is a mixed mode request, which is supposed to
	 * be invalid; In this case it is simply interpreted
	 * as 'not a match', because all NCX data is in XML.
	 * This is not really allowed, and the result is the empty set
	 */
	return NULL;
    case NCX_BT_CONTAINER:

	/* This is the normal case - a container node
	 * Go through the child nodes.
	 * There should be only container nodes at this level
	 *
	 * This must match an application node in the 
	 * target configuration.  
	 */
	check_sibling_set(fil, &anycon, &anycm, &anysel);

#if 0
	if (anycm) {
	    /* A content match node at this level cancels out
	     * the entire sibling set, since there are no simple
	     * types to compare against yet
	     */
	    return NULL;
	}
#endif

	top = ncx_new_filptr();
	if (!top) {
	    return NULL;
	}
	top->nodetyp = NCX_NT_TOP;
	
	/* Go through all the child nodes of <filter>
	 * There are only independent container nodes at this level
	 * for each one, match to a val_value_t node and prune the
	 * subtree from there
	 */

	if (!prune_val(scb, cfg->root, getop, top)) {
	    ncx_free_filptr(top);
	    return NULL;
	} else {

#ifdef AGT_TREE_DEBUG
	    dump_filptr_node(top, 0);
#endif

	    return top;
	}
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

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
*
* RETURNS:
*    none
*********************************************************************/
void
    agt_tree_output_filter (ses_cb_t *scb,
			    rpc_msg_t *msg,
			    ncx_filptr_t *top,
			    int32 indent)
{
#ifdef DEBUG
    if (!scb || !msg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    output_node(scb, msg, top, indent);
    
} /* agt_tree_output_filter */


/* END file agt_tree.c */
