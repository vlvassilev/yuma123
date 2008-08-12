/*  FILE: agt_val_util.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
19dec05      abb      begun
21jul08      abb      start obj-based rewrite
08aug08      abb      clone for agent <rpc-error> reporting version

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>
#include  <string.h>
#include  <ctype.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val_util
#include "agt_val_util.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_val_util
#include "val_util.h"
#endif

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_VAL_UTIL_DEBUG  1
#endif


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

    minset = obj_get_min_elements(obj, &minelems);
    maxset = obj_get_max_elements(obj, &maxelems);

    cnt = val_instance_count(val, obj_get_mod_name(obj),
			     obj_get_name(obj));

    if (minset) {
	if (cnt < minelems) {
	    /* not enough instances error */
	    minerr = TRUE;
	    res = ERR_NCX_MISSING_VAL_INST;
	    if (msg) {
		agt_record_error(scb, &msg->errQ, layer, res, 
				 NULL, NCX_NT_OBJ, obj, 
				 NCX_NT_VAL, val);
	    }
	    res = NO_ERR;
	}
    }

    if (maxset) {
	if (cnt > maxelems) {
	    /* too many instances error */
	    maxerr = TRUE;
	    res = ERR_NCX_EXTRA_VAL_INST;
	    if (msg) {
		agt_record_error(scb, &msg->errQ, layer, res, 
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
	} else if (cnt > 1 && !maxerr) {
	    /* too many parameters */
	    res = ERR_NCX_EXTRA_VAL_INST;
	}
	break;
    case NCX_IQUAL_OPT:
	if (cnt > 1 && !maxerr) {
	    /* too many parameters */
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
	agt_record_error(scb, &msg->errQ, layer, res, 
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
		agt_record_error(scb, &msg->errQ, layer, res, 
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
		agt_record_error(scb, &msg->errQ, layer, res, 
				 NULL, NCX_NT_OBJ, choicobj, 
				 NCX_NT_VAL, testval);
	    }
	}
	testval = val_get_choice_next_set(val, choicobj, testval);
    }

    val->res = res;

    return res;

}  /* choice_check_agt */


/*************** E X T E R N A L    F U N C T I O N S  *************/


/********************************************************************
* FUNCTION agt_val_instance_check
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
    status_t               res, retres;

#ifdef DEBUG
    if (!valset) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    retres = NO_ERR;

    obj = valset->obj;

    if (obj->objtype == OBJ_TYP_LEAF_LIST) {
	retres = instance_check(scb, msg, obj, valset, layer);
    } else {
	/* check all the child nodes for correct number of instances */
	for (chobj = obj_first_child(obj);
	     chobj != NULL;
	     chobj = obj_next_child(chobj)) {

	    switch (chobj->objtype) {
	    case OBJ_TYP_CHOICE:
		res = choice_check_agt(scb, msg, chobj, valset, layer);
		break;
	    case OBJ_TYP_CASE:
		res = SET_ERROR(ERR_INTERNAL_VAL);
		break;
	    default:
		res = instance_check(scb, msg, chobj, valset, layer);
	    }
	    CHK_EXIT;
	}
    }

    return retres;
    
}  /* agt_val_instance_check */


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
	agt_record_error(scb, &msg->errQ, NCX_LAYER_CONTENT, res, 
	     NULL, NCX_NT_STRING, NC_OPERATION_ATTR_NAME, NCX_NT_VAL, val);
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
		agt_record_error(scb, &msg->errQ, 
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


/* END file agt_val_util.c */
