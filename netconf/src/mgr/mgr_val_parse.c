/*  FILE: mgr_val_parse.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
11feb06      abb      begun; hack, clone agent code and remove
                      all the rpc-error handling code; later a proper
                      libxml2 docPtr interface will be used instead

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
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

#ifndef _H_mgr_val_parse
#include "mgr_val_parse.h"
#endif

#ifndef _H_mgr_xml
#include "mgr_xml.h"
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

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tk
#include "tk.h"
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

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
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
#define MGR_VAL_PARSE_DEBUG 1
#endif


/* forward declaration for recursive calls */
static status_t 
    parse_btype (ses_cb_t  *scb,
		 const obj_template_t *obj,
		 const xml_node_t *startnode,
		 val_value_t  *retval);


static status_t 
    parse_one_btype (ses_cb_t  *scb,
		     const obj_template_t *obj,
		     ncx_btype_t  btyp,
		     const xml_node_t *startnode,
		     val_value_t  *retval);



/********************************************************************
 * FUNCTION get_xml_node
 * 
 * Get the next (or maybe current) XML node in the reader stream
 * This hack needed because xmlTextReader cannot look ahead or
 * back up during processing.
 * 
 * The YANG leaf-list is really a collection of sibling nodes
 * and there is no way to tell where it ends without reading
 * past the end of it.
 *
 * This hack relies on the fact that a top-levelleaf-list could
 * never show up in a real NETCONF PDU
 *
 * INPUTS:
 *   scb == session control block
 *   xmlnode == xml_node_t to fill in
 *
 * OUTPUTS:
 *   *xmlnode filled in
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    get_xml_node (ses_cb_t  *scb,
		  xml_node_t *xmlnode)
{
    status_t   res;

    if (scb->xmladvance) {
	res = mgr_xml_consume_node(scb->reader, xmlnode);
    } else {
	res = mgr_xml_consume_node_noadv(scb->reader, xmlnode);
    }
    scb->xmladvance = TRUE;
    return res;

}   /* get_xml_node */


/********************************************************************
 * FUNCTION gen_index_chain
 * 
 * Create an index chain for the just-parsed table or container struct
 *
 * INPUTS:
 *   instart == first obj_key_t in the chain to process
 *   val == the just parsed list entry with the childQ containing
 *          nodes to check as index nodes
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    gen_index_chain (const obj_key_t *instart,
		     val_value_t *val)
{
    const obj_key_t    *key;
    status_t            res;

    res = NO_ERR;

    /* 0 or more index components expected */
    for (key = instart; 
	 key != NULL;
	 key = (const obj_key_t *)dlq_nextEntry(key)) {

	res = val_gen_index_comp(key, val);
	if (res != NO_ERR) {
	    return res;
	}
    }

    return res;

}   /* gen_index_chain */


/********************************************************************
 * FUNCTION get_editop
 * 
 * Check the node for operation="foo" attribute
 * and convert its value to an op_editop_t enum
 *
 * INPUTS:
 *   node == xml_node_t to check
 * RETURNS:
 *   editop == will be OP_EDITOP_NONE if explicitly set,
 *             not-present, or error
 *********************************************************************/
static op_editop_t
    get_editop (const xml_node_t  *node)
{
    const xml_attr_t  *attr;

    attr = xml_find_ro_attr(node, xmlns_nc_id(), NC_OPERATION_ATTR_NAME);
    if (!attr) {
	return OP_EDITOP_NONE;
    }
    return op_editop_id(attr->attr_val);

} /* get_editop */


/********************************************************************
 * FUNCTION parse_any
 * 
 * Parse the XML input as an 'any' type 
 *
 * INPUTS:
 *   scb == session in progress
 *   startnode == XML to start processing at
 *   retval == address of value struct to fill in
 *
 * OUTPUTS:
 *   *retval filled in
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_any (ses_cb_t  *scb,
	       const xml_node_t *startnode,
	       val_value_t  *retval)
{
    val_value_t             *chval;
    status_t                 res, res2;
    boolean                  done, getstrend;
    xml_node_t               nextnode;

    /* init local vars */
    chval = NULL;
    res = NO_ERR;
    res2 = NO_ERR;
    done = FALSE;
    getstrend = FALSE;

    xml_init_node(&nextnode);

    /* make sure the startnode is correct */
    switch (startnode->nodetyp) {
    case XML_NT_START:
	/* do not set the object template yet, in case
	 * there is a <foo></foo> form of empty element
	 * or another start (child) node
	 */
	break;
    case XML_NT_EMPTY:
	/* treat this 'any' is an 'empty' data type  */
	val_init_from_template(retval, ncx_get_gen_empty());
	retval->v.bool = TRUE;
	return NO_ERR;
    default:
	res = ERR_NCX_WRONG_NODETYP;
    }

    if (res == NO_ERR) {
	/* at this point have either a simple type or a complex type
	 * get the next node which could be any type 
	 */
	res = mgr_xml_consume_node_nons(scb->reader, &nextnode);
    }

    if (res == NO_ERR) {

#ifdef MGR_VAL_PARSE_DEBUG
	log_debug3("\nparse_any: expecting any node type");
	if (LOGDEBUG3) {
	    xml_dump_node(&nextnode);
	}
#endif

	/* decide the base type from the child node type */
	switch (nextnode.nodetyp) {
	case XML_NT_START:
	case XML_NT_EMPTY:
	    /* A nested start or empty element means the parent is
	     * treated as a 'container' data type
	     */
	    val_init_from_template(retval, ncx_get_gen_container());
	    break;
	case XML_NT_STRING:
	    /* treat this string child node as the string
	     * content for the parent node
	     */
	    val_init_from_template(retval, ncx_get_gen_string());
	    retval->v.str = xml_strdup(nextnode.simval);
	    res = (retval->v.str) ? NO_ERR : ERR_INTERNAL_MEM;
	    getstrend = TRUE;
	    break;
	case XML_NT_END:
	    res = xml_endnode_match(startnode, &nextnode);
	    if (res == NO_ERR) {
		/* treat this start + end pair as an 'empty' data type */
		val_init_from_template(retval, ncx_get_gen_empty());
		retval->v.bool = TRUE;
		return NO_ERR;
	    }
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* check if processing a simple type as a string */
    if (getstrend) {
	/* need to get the endnode for startnode then exit */
	xml_clean_node(&nextnode);
	res2 = mgr_xml_consume_node_nons(scb->reader, &nextnode);
	if (res2 == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	    log_debug3("\nparse_any: expecting end node for %s", 
		   startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(&nextnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &nextnode);
	}
    }

    /* check if there were any errors in the startnode */
    if (res != NO_ERR || res2 != NO_ERR) {
	xml_clean_node(&nextnode);
	return (res==NO_ERR) ? res2 : res;
    }

    if (getstrend) {
	return NO_ERR;
    }

    /* if we get here, then the startnode is a container */
    while (!done) {
	/* At this point have a nested start node
	 *  Allocate a new val_value_t for the child value node 
	 */
	res = NO_ERR;
	chval = val_new_child_val(nextnode.nsid, nextnode.elname, 
				  TRUE, retval, get_editop(&nextnode));
	if (!chval) {
	    res = ERR_INTERNAL_MEM;
	}

	/* check any error setting up the child node */
	if (res == NO_ERR) {
	    /* recurse through and get whatever nodes are present
	     * in the child node; call it an 'any' type
	     * make sure this function gets called again
	     * so the namespace errors can be ignored properly ;-)
	     *
	     * Cannot call this function directly or the
	     * XML attributes will not get processed
	     */
	    res = parse_btype(scb, ncx_get_gen_anyxml(), 
			      &nextnode, chval);
	    chval->res = res;
	    xml_clean_node(&nextnode);
	    val_add_child(chval, retval);
	    chval = NULL;
	}

	/* record any error, if not already done */
	if (res != NO_ERR) {
	    xml_clean_node(&nextnode);
	    if (chval) {
		val_free_value(chval);
	    }
	    return res;
	}

	/* get the next node */
	res = mgr_xml_consume_node_nons(scb->reader, &nextnode);
	if (res == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	    log_debug3("\nparse_any: expecting start, empty, or end node");
	    if (LOGDEBUG3) {
		xml_dump_node(&nextnode);
	    }
#endif
	    res = xml_endnode_match(startnode, &nextnode);
	    if (res == NO_ERR) {
		done = TRUE;
	    }	    
	} else {
	    /* error already recorded */
	    done = TRUE;
	}
    }

    xml_clean_node(&nextnode);
    return res;

} /* parse_any */


/********************************************************************
 * FUNCTION parse_enum
 * 
 * Parse the XML input as a 'enumeration' type 
 * e.g..
 *
 * <foo>fred</foo>
 * 
 * These NCX variants are no longer supported:
 *    <foo>11</foo>
 *    <foo>fred(11)</foo>
 *
 * INPUTS:
 *   scb == session in progress
 *   obj == object template to parse against
 *   startnode == XML to start processing at
 *   retval == address of value struct to fill in
 *
 * OUTPUTS:
 *   *retval filled in
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_enum (ses_cb_t  *scb,
		const obj_template_t *obj,
		const xml_node_t *startnode,
		val_value_t  *retval)
{
    xml_node_t         valnode, endnode;
    status_t           res, res2;

    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    res2 = NO_ERR;

    val_init_from_template(retval, obj);

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, obj_get_nsid(obj), 
			 NULL, XML_NT_START); 
    if (res == NO_ERR) {
	/* get the next node which should be a string node */
	res = get_xml_node(scb, &valnode);
    }

    if (res == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	log_debug3("\nparse_enum: expecting string node");
	if (LOGDEBUG3) {
	    xml_dump_node(&valnode);
	}
#endif

	/* validate the node type and enum string or number content */
	switch (valnode.nodetyp) {
	case XML_NT_START:
	    res = ERR_NCX_WRONG_NODETYP_CPX;
	    break;
	case XML_NT_STRING:
	    /* get the non-whitespace string here */
	    res = val_enum_ok(obj_get_ctypdef(obj), 
			      valnode.simval, 
			      &retval->v.enu.val, 
			      &retval->v.enu.name);
	    if (res == NO_ERR) {
		/* record the value even if there are errors after this */
		retval->btyp = NCX_BT_ENUM;
	    }
	    break;
	default:
	    res = ERR_NCX_WRONG_NODETYP;
	}

	/* get the matching end node for startnode */
	res2 = get_xml_node(scb, &endnode);
	if (res2 == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	    log_debug3("\nparse_enum: expecting end for %s", 
		       startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(&endnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &endnode);
	}
    }

    if (res == NO_ERR) {
	res = res2;
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_enum */


/********************************************************************
 * FUNCTION parse_empty
 * For NCX_BT_EMPTY
 *
 * Parse the XML input as an 'empty' or 'boolean' type 
 * e.g.:
 *
 *  <foo/>
 * <foo></foo>
 * <foo>   </foo>
 *
 * INPUTS:
 *   scb == session in progress
 *   obj == object template to parse against
 *   startnode == XML to start processing at
 *   retval == address of value struct to fill in
 *
 * OUTPUTS:
 *   *retval filled in
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_empty (ses_cb_t  *scb,
		 const obj_template_t *obj,
		 const xml_node_t *startnode,
		 val_value_t  *retval)
{
    xml_node_t        endnode;
    status_t          res;

    /* init local vars */
    xml_init_node(&endnode);

    val_init_from_template(retval, obj);

    /* validate the node type and enum string or number content */
    switch (startnode->nodetyp) {
    case XML_NT_EMPTY:
	res = xml_node_match(startnode, obj_get_nsid(obj),
	     NULL, XML_NT_NONE);
	break;
    case XML_NT_START:
	res = xml_node_match(startnode, obj_get_nsid(obj),
	     NULL, XML_NT_NONE);
	if (res == NO_ERR) {
	    res = get_xml_node(scb, &endnode);
	    if (res == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
		log_debug3("\nparse_empty: expecting end for %s", 
		       startnode->qname);
		if (LOGDEBUG3) {
		    xml_dump_node(&endnode);
		}
#endif
		res = xml_endnode_match(startnode, &endnode);
		if (res != NO_ERR) {
		    if (endnode.nodetyp != XML_NT_STRING ||
			!xml_isspace_str(endnode.simval)) {
			res = ERR_NCX_WRONG_NODETYP;
		    } else {
			/* that was an empty string -- try again */
			xml_clean_node(&endnode);
			res = get_xml_node(scb, &endnode);
			if (res == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
			    log_debug3("\nparse_empty: expecting end for %s", 
				   startnode->qname);
			    if (LOGDEBUG3) {
				xml_dump_node(&endnode);
			    }
#endif
			    res = xml_endnode_match(startnode, &endnode);
			}
		    }
		}
	    }
	}
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
    }

    /* record the value if no errors */
    if (res == NO_ERR) {
	retval->v.bool = TRUE;
    }

    xml_clean_node(&endnode);
    return res;

} /* parse_empty */


/********************************************************************
 * FUNCTION parse_boolean
 * 
 * Parse the XML input as a 'boolean' type 
 * e.g..
 *
 * <foo>true</foo>
 * <foo>false</foo>
 * <foo>1</foo>
 * <foo>0</foo>
 *
 * INPUTS:
 *   scb == session in progress
 *   obj == object template to parse against
 *   startnode == XML to start processing at
 *   retval == address of value struct to fill in
 *
 * OUTPUTS:
 *   *retval filled in
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_boolean (ses_cb_t  *scb,
		   const obj_template_t *obj,
		   const xml_node_t *startnode,
		   val_value_t  *retval)
{
    xml_node_t         valnode, endnode;
    status_t           res, res2;

    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    res2 = NO_ERR;

    val_init_from_template(retval, obj);

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, obj_get_nsid(obj), 
			 NULL, XML_NT_START); 
    if (res == NO_ERR) {
	/* get the next node which should be a string node */
	res = get_xml_node(scb, &valnode);
    }

    if (res == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	log_debug3("\nparse_boolean: expecting string node.");
	if (LOGDEBUG3) {
	    xml_dump_node(&valnode);
	}
#endif

	/* validate the node type and enum string or number content */
	switch (valnode.nodetyp) {
	case XML_NT_START:
	    res = ERR_NCX_WRONG_NODETYP_CPX;
	    break;
	case XML_NT_STRING:
	    /* get the non-whitespace string here */
	    if (ncx_is_true(valnode.simval)) {
		retval->v.bool = TRUE;
	    } else if (ncx_is_false(valnode.simval)) {
		retval->v.bool = FALSE;
	    } else {
		res = ERR_NCX_INVALID_VALUE;
	    }
	    break;
	default:
	    res = ERR_NCX_WRONG_NODETYP;
	}

	/* get the matching end node for startnode */
	res2 = get_xml_node(scb, &endnode);
	if (res2 == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	    log_debug3("\nparse_boolean: expecting end for %s", 
		       startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(&endnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &endnode);
	}
    }

    if (res == NO_ERR) {
	res = res2;
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_boolean */


/********************************************************************
* FUNCTION parse_num
* 
* Parse the XML input as a numeric data type 
*
* INPUTS:
*     scb == session control block
*            Input is read from scb->reader.
*     obj == object template for this number type
*     btyp == base type of the expected ordinal number type
*     startnode == top node of the parameter to be parsed
*            Parser function will attempt to consume all the
*            nodes until the matching endnode is reached
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*    msg->errQ may be appended with new errors or warnings
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    parse_num (ses_cb_t  *scb,
	       const obj_template_t *obj,
	       ncx_btype_t  btyp,
	       const xml_node_t *startnode,
	       val_value_t  *retval)
{
    const ncx_errinfo_t *errinfo;
    xml_node_t           valnode, endnode;
    status_t             res, res2;

    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    errinfo = NULL;

    val_init_from_template(retval, obj);
    
    /* make sure the startnode is correct */
    res = xml_node_match(startnode, obj_get_nsid(obj),
			 NULL, XML_NT_START); 
    if (res == NO_ERR) {
	/* get the next node which should be a string node */
	res = get_xml_node(scb, &valnode);
    }

    if (res != NO_ERR) {
	/* fatal error */
	xml_clean_node(&valnode);
	return res;
    }

#ifdef MGR_VAL_PARSE_DEBUG
    log_debug3("\nparse_num: expecting string node.");
    if (LOGDEBUG3) {
	xml_dump_node(&valnode);
    }
#endif

    /* validate the number content */
    switch (valnode.nodetyp) {
    case XML_NT_START:
	res = ERR_NCX_WRONG_NODETYP_CPX;
	break;
    case XML_NT_STRING:
	/* get the non-whitespace string here */
	res = ncx_decode_num(valnode.simval, btyp, &retval->v.num);
	if (res == NO_ERR) {
	    res = val_range_ok_errinfo(obj_get_ctypdef(obj), btyp, 
				       &retval->v.num, &errinfo);
	}
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
    }

    /* get the matching end node for startnode */
    res2 = get_xml_node(scb, &endnode);
    if (res2 == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	log_debug3("\nparse_num: expecting end for %s", startnode->qname);
	if (LOGDEBUG3) {
	    xml_dump_node(&endnode);
	}
#endif
	res2 = xml_endnode_match(startnode, &endnode);
    }

    if (res == NO_ERR) {
	res = res2;
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_num */


/********************************************************************
* FUNCTION parse_string
* 
* Parse the XML input as a numeric data type 
*
* INPUTS:
*     scb == session control block
*            Input is read from scb->reader.
*     obj == object template for this string type
*     btyp == base type of the expected ordinal number type
*     startnode == top node of the parameter to be parsed
*            Parser function will attempt to consume all the
*            nodes until the matching endnode is reached
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*    msg->errQ may be appended with new errors or warnings
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    parse_string (ses_cb_t  *scb,
		  const obj_template_t *obj,
		  ncx_btype_t  btyp,
		  const xml_node_t *startnode,
		  val_value_t  *retval)
{
    const ncx_errinfo_t  *errinfo;
    const typ_template_t *listtyp;
    xml_node_t            valnode, endnode;
    status_t              res, res2;
    boolean               empty;
    ncx_btype_t           listbtyp;

    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    empty = FALSE;
    errinfo = NULL;

    val_init_from_template(retval, obj);

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, obj_get_nsid(obj),
			 NULL, XML_NT_START); 
    if (res != NO_ERR) {
	res = xml_node_match(startnode, obj_get_nsid(obj),
			     NULL, XML_NT_EMPTY); 
	if (res == NO_ERR) {
	    empty = TRUE;
	}
    }

    /* get the value string node */
    if (res == NO_ERR && !empty) {
	/* get the next node which should be a string node */
	res = get_xml_node(scb, &valnode);
	if (res == NO_ERR && valnode.nodetyp == XML_NT_END) {
	    empty = TRUE;
	}
    }
  
    /* check empty string corner case */
    if (empty) {
	if (btyp==NCX_BT_SLIST || btyp==NCX_BT_BITS) {
	    /* check the empty list */
	    res = val_list_ok_errinfo(obj_get_ctypdef(obj), 
				      &retval->v.list,
				      &errinfo);
	} else {
	    /* check the empty string */
	    res = val_string_ok_errinfo(obj_get_ctypdef(obj), 
					btyp, (const xmlChar *)"",
					&errinfo);
	    retval->v.str = xml_strdup((const xmlChar *)"");
	    if (!retval->v.str) {
		res = ERR_INTERNAL_MEM;
	    }
	}
    }

    if (res != NO_ERR) {
	xml_clean_node(&valnode);
	return res;
    }

    if (empty) {
	return NO_ERR;
    }

#ifdef MGR_VAL_PARSE_DEBUG
    log_debug3("\nparse_string: expecting string node.");
    if (LOGDEBUG3) {
	xml_dump_node(&valnode);
    }
#endif

    /* validate the number content */
    switch (valnode.nodetyp) {
    case XML_NT_START:
	res = ERR_NCX_WRONG_NODETYP_CPX;
	break;
    case XML_NT_STRING:
	if (btyp==NCX_BT_SLIST || btyp==NCX_BT_BITS) {
	    /* get the list of strings, then check them */
	    listtyp = typ_get_listtyp(obj_get_ctypdef(obj));
	    listbtyp = typ_get_basetype(&listtyp->typdef);

	    res = ncx_set_list(listbtyp, valnode.simval, 
			       &retval->v.list);
	    if (res == NO_ERR) {
		res = ncx_finish_list(&listtyp->typdef, &retval->v.list);
	    }

	    if (res == NO_ERR) {
		res = val_list_ok_errinfo(obj_get_ctypdef(obj), 
					  &retval->v.list, &errinfo);
	    }
	} else {
	    /* check the non-whitespace string */
	    res = val_string_ok_errinfo(obj_get_ctypdef(obj), 
					btyp, valnode.simval, &errinfo);
	}

	/* record the value even if there are errors */
	switch (btyp) {
	case NCX_BT_STRING:
	case NCX_BT_BINARY:
	case NCX_BT_INSTANCE_ID:
	case NCX_BT_KEYREF:
	    retval->v.str = xml_strdup(valnode.simval);
	    if (!retval->v.str) {
		res = ERR_INTERNAL_MEM;
	    }
	    break;
	case NCX_BT_SLIST:
	    break;   /* value already set */
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
    }

    /* get the matching end node for startnode */
    res2 = get_xml_node(scb, &endnode);
    if (res2 == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	log_debug3("\nparse_string: expecting end for %s", 
		   startnode->qname);
	if (LOGDEBUG3) {
	    xml_dump_node(&endnode);
	}
#endif
	res2 = xml_endnode_match(startnode, &endnode);
	
    }

    if (res == NO_ERR) {
	res = res2;
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_string */


/********************************************************************
 * FUNCTION parse_union
 * 
 * Parse the XML input as a 'union' type 
 * e.g..
 *
 * <foo>fred</foo>
 * <foo>11</foo>
 *
 * INPUTS:
 *     scb == session control block
 *            Input is read from scb->reader.
 *     obj == object template for this string type
 *     startnode == top node of the parameter to be parsed
 *            Parser function will attempt to consume all the
 *            nodes until the matching endnode is reached
 *     retval ==  val_value_t that should get the results of the parsing
 *     
 * OUTPUTS:
 *    *retval will be filled in
 *    msg->errQ may be appended with new errors or warnings
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_union (ses_cb_t  *scb,
		 const obj_template_t *obj,
		 const xml_node_t *startnode,
		 val_value_t  *retval)
{
    const ncx_errinfo_t *errinfo;
    xml_node_t           valnode, endnode;
    status_t             res, res2;

    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    res2 = NO_ERR;
    errinfo = NULL;

    val_init_from_template(retval, obj);

    /* make sure the startnode is correct */
    if (res == NO_ERR) {
	res = xml_node_match(startnode, obj_get_nsid(obj), 
			     NULL, XML_NT_START); 
    }
    if (res == NO_ERR) {
	/* get the next node which should be a string node */
	res = get_xml_node(scb, &valnode);
    }

    if (res == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	log_debug3("\nparse_union: expecting string or number node.");
	if (LOGDEBUG3) {
	    xml_dump_node(&valnode);
	}
#endif

	/* validate the node type and union node content */
	switch (valnode.nodetyp) {
	case XML_NT_START:
	    res = ERR_NCX_WRONG_NODETYP_CPX;
	    break;
	case XML_NT_STRING:
	    /* get the non-whitespace string here */
	    res = val_union_ok_errinfo(obj_get_ctypdef(obj), 
				       valnode.simval, retval, &errinfo);
	    break;
	default:
	    res = ERR_NCX_WRONG_NODETYP;
	}

	/* get the matching end node for startnode */
	res2 = get_xml_node(scb, &endnode);
	if (res2 == NO_ERR) {
#ifdef MGR_VAL_PARSE_DEBUG
	    log_debug3("\nparse_union: expecting end for %s", 
		       startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(&endnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &endnode);
	}
    }

    if (res == NO_ERR) {
	res = res2;
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_union */


/********************************************************************
 * FUNCTION parse_leaflist
 * 
 * Parse the XML input as a pseudo-complex type
 *
 * Handles the following base types:
 *   NCX_BT_LEAF_LIST
 *
 *
 * <foo>fred</foo>
 * <foo>barney</foo>
 * <foo>wilma</foo>
 *
 * INPUTS:
 *     scb == session control block
 *            Input is read from scb->reader.
 *     obj == object template for this string type
 *     startnode == top node of the parameter to be parsed
 *            Parser function will attempt to consume all the
 *            nodes until the matching endnode is reached
 *     retval ==  val_value_t that should get the results of the parsing
 *     
 * OUTPUTS:
 *    *retval will be filled in
 *    msg->errQ may be appended with new errors or warnings
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_leaflist (ses_cb_t  *scb,
		    const obj_template_t *obj,
		    const xml_node_t *startnode,
		    val_value_t  *retval)
{
    val_value_t          *chval;
    xml_node_t            testnode;
    status_t              res, retres;
    boolean               done, empty;
    ncx_btype_t           btyp;

    /* setup local vars */
    res = NO_ERR;
    retres = NO_ERR;
    done = FALSE;
    empty = FALSE;

    /* first set up a complex type (NCX_BT_LEAF_LIST
     * that will hold all the simple leafs
     */
    val_init_from_template_primary(retval, obj);
    retval->editop = get_editop(startnode);

    switch (startnode->nodetyp) {
    case XML_NT_START:
	break;
    case XML_NT_EMPTY:
	empty = TRUE;
	break;
    case XML_NT_STRING:
    case XML_NT_END:
	res = ERR_NCX_WRONG_NODETYP_SIM;
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
    }

    if (res != NO_ERR) {
	return res;
    }

    xml_init_node(&testnode);

    /* get the basetype of the individual leafs */
    btyp = obj_get_basetype(obj);

    /* go through sibling nodes until a different element shows up
     * indicating the end of the leaf list
     * gather all the leafs as children of the leaf-list
     */
    while (!done) {
	/* init per-loop vars */
	res = NO_ERR;

	/* try to create a new child node for the leaf-list */
	chval = val_new_child_val(obj_get_nsid(obj),
				  obj_get_name(obj), 
				  FALSE, retval, 
				  get_editop(startnode));
	if (!chval) {
	    res = ERR_INTERNAL_MEM;
	} else if (empty) {
	    val_init_from_template(chval, obj);
	    val_add_child(chval, retval);
	    done = TRUE;
	    continue;
	} else {
	    /* get one leaf parsed */
	    res = parse_one_btype(scb, obj, btyp, startnode, chval);
	    chval->res = res;
	    val_add_child(chval, retval);
	}

	/* check any errors in setting up the child or index nodes */
	if (res != NO_ERR) {
	    retres = res;
	    if (NEED_EXIT) {
		done = TRUE;
		continue;
	    }
	}

	/* peek ahead at the next node to see if it matches this 
	 * leaflist or not 
	 */
	res = get_xml_node(scb, &testnode);
	if (res == NO_ERR) {
	    if (testnode.nodetyp==XML_NT_START ||
		testnode.nodetyp==XML_NT_EMPTY) {
		res = xml_node_match(&testnode, 
				     obj_get_nsid(obj), 
				     obj_get_name(obj), 
				     XML_NT_NONE);
	    } else {
		/* not really an error */
		res = ERR_NCX_WRONG_ELEMENT;
	    }

	    if (res != NO_ERR) {
		/* this is part of the next object, or maybe the
		 * end of a parent object
		 */
		scb->xmladvance = FALSE;
		done = TRUE;
		continue;
	    }
	}
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}
	xml_clean_node(&testnode);
    }

    xml_clean_node(&testnode);
    return retres;

} /* parse_leaflist */


/********************************************************************
 * FUNCTION parse_complex
 * 
 * Parse the XML input as a complex type
 *
 * Handles the following base types:
 *   NCX_BT_CONTAINER
 *   NCX_BT_LIST
 *
 * E.g., container:
 *
 * <foo>
 *   <a>blah</a>
 *   <b>7</b>
 *   <c/>
 * </foo>
 *
 * In an instance document, containers and lists look 
 * the same.  The validation is different of course, but the
 * parsing is basically the same.
 *
 * INPUTS:
 *     scb == session control block
 *            Input is read from scb->reader.
 *     obj == object template for this complex type
 *     btyp == base type of the expected complex type
 *     startnode == top node of the parameter to be parsed
 *            Parser function will attempt to consume all the
 *            nodes until the matching endnode is reached
 *     retval ==  val_value_t that should get the results of the parsing
 *     
 * OUTPUTS:
 *    *retval will be filled in
 *    msg->errQ may be appended with new errors or warnings
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_complex (ses_cb_t  *scb,
		   const obj_template_t *obj,
		   ncx_btype_t btyp,
		   const xml_node_t *startnode,
		   val_value_t  *retval)
{
    const obj_template_t *chobj, *curchild, *curtop, *nextchobj;
    val_value_t          *chval;
    xml_node_t            chnode;
    status_t              res, res2, retres;
    boolean               done, empty;
    ncx_btype_t           chbtyp;

    /* setup local vars */
    chobj = NULL;
    curtop = NULL;
    curchild = NULL;
    nextchobj = NULL;
    res = NO_ERR;
    res2 = NO_ERR;
    retres = NO_ERR;
    done = FALSE;
    empty = FALSE;

    val_init_from_template(retval, obj);

    /* do not really need to validate the start node type
     * since it is probably a virtual container at the
     * very start, and after that, a node match must have
     * occurred to call this function recursively
     */
    switch (startnode->nodetyp) {
    case XML_NT_START:
	break;
    case XML_NT_EMPTY:
	empty = TRUE;
	break;
    case XML_NT_STRING:
    case XML_NT_END:
	res = ERR_NCX_WRONG_NODETYP_SIM;
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
    }

    if (res == NO_ERR) {
	/* start setting up the return value */
	retval->editop = get_editop(startnode);

	/* setup the first child in the complex object
	 * Allowed be NULL in some cases so do not check
	 * the result yet
	 */
	chobj = obj_first_child(obj);
    } else {
	return res;
    }

    if (empty) {
	return NO_ERR;
    }

    xml_init_node(&chnode);

    /* go through each child node until the parent end node */
    while (!done) {
	/* init per-loop vars */
	res2 = NO_ERR;
	empty = FALSE;

	/* get the next node which should be a child or end node */
	res = get_xml_node(scb, &chnode);
	if (res != NO_ERR) {
	    done = TRUE;
	} else {
#ifdef MGR_VAL_PARSE_DEBUG
	    log_debug3("\nparse_complex: expecting start-child or end node.");
	    if (LOGDEBUG3) {
		xml_dump_node(&chnode);
	    }
#endif
	    /* validate the child member node type */
	    switch (chnode.nodetyp) {
	    case XML_NT_START:
	    case XML_NT_EMPTY:
		/* any namespace OK for now, check in obj_get_child_node */
		break;
	    case XML_NT_STRING:
		res = ERR_NCX_WRONG_NODETYP_SIM;
		break;
	    case XML_NT_END:
		res = xml_endnode_match(startnode, &chnode);
		if (res == NO_ERR) {
		    /* no error exit */
		    done = TRUE;
		    continue;
		}
		break;
	    default:
		res = ERR_NCX_WRONG_NODETYP;
	    }
	}

	/* if we get here, there is a START or EMPTY node
	 * that could be a valid child node
	 *
	 * if xmlorder enforced then check if the 
	 * node is the correct child node
	 *
	 * if no xmlorder, then check for any valid child
	 */
	if (res==NO_ERR) {
	    res = obj_get_child_node(obj, chobj, &chnode, FALSE,
				     &curtop, &curchild);
	}

	/* try to setup a new child node */
	if (res == NO_ERR) {
	    /* save the child base type */
	    chbtyp = obj_get_basetype(curchild);

	    /* at this point, the 'curchild' template matches the
	     * 'chnode' namespace and name;
	     * Allocate a new val_value_t for the child value node
	     */
	    chval = val_new_child_val(obj_get_nsid(curchild),
				      obj_get_name(curchild), 
				      FALSE, retval, 
				      get_editop(&chnode));
	    if (!chval) {
		res = ERR_INTERNAL_MEM;
	    }
	}

	/* check any errors in setting up the child node */
	if (res != NO_ERR) {

	    /* try to skip just the child node sub-tree */
	    xml_clean_node(&chnode);
	    if (res2 != NO_ERR) {
		/* skip child didn't work, now skip the entire value subtree */
		(void)mgr_xml_skip_subtree(scb->reader, startnode);
		return res;
	    } else {
		/* skip child worked, go on to next child, parse for errors */
		retres = res;
		continue;
	    }
	}

	/* recurse through and get whatever nodes are present
	 * in the child node
	 */
	res = parse_btype(scb, curchild, &chnode, chval);
	chval->res = res;
	val_add_child(chval, retval);
	if (res == NO_ERR) {
	    /* setup the next child unless the current child
	     * is a list
	     */
	    if (chbtyp != NCX_BT_LIST) {
		/* setup next child if the cur child is 0 - 1 instance */
		switch (obj_get_iqualval(curchild)) {
		case NCX_IQUAL_ONE:
		case NCX_IQUAL_OPT:
		    if (chobj) {
			chobj = obj_next_child(chobj);
		    }
		    break;
		default:
		    break;
		}
	    }
	} else {
	    /* did not parse the child node correctly */
	    retres = res;
	}
	xml_clean_node(&chnode);

	/* check if this is the special internal load-config RPC method,
	 * in which case there will not be an end tag for the <load-config>
	 * start tag passed to this function.  Need to quick exit
	 * to prevent reading past the end of the XML config file,
	 * which just contains the <config> element
	 *
	 * know there is just one parameter named <config> so just
	 * go through the loop once because the </load-config> tag
	 * will not be present
	 */
	if ((startnode->nsid == xmlns_ncx_id() || startnode->nsid == 0) && 
	    !xml_strcmp(startnode->elname, NCX_EL_LOAD_CONFIG)) {
	    done = TRUE;
	}
    }

    /* check if the index ID needs to be set */
    if (btyp==NCX_BT_LIST) {
	res = gen_index_chain(obj_first_key(obj), retval);
	if (res != NO_ERR) {
	    retres = res;
	}
    }

    xml_clean_node(&chnode);
    return retres;

} /* parse_complex */


/********************************************************************
* FUNCTION parse_one_btype
* 
* Switch to dispatch to specific base type handler
* for one element; needed so leaf-list can be handled properly
*
* INPUTS:
*     scb == session control block
*            Input is read from scb->reader.
*     obj == object template for expected node
*     btyp == forced base type to use
*     startnode == top node of the parameter to be parsed
*            Parser function will attempt to consume all the
*            nodes until the matching endnode is reached
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*    msg->errQ may be appended with new errors or warnings
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    parse_one_btype (ses_cb_t  *scb,
		     const obj_template_t *obj,
		     ncx_btype_t  btyp,
		     const xml_node_t *startnode,
		     val_value_t  *retval)
{
    status_t     res, res2, res3;
    op_editop_t  editop;
    boolean      nserr;

    /* get the attribute values from the start node */
    editop = OP_EDITOP_NONE;
    retval->nsid = startnode->nsid;

    /* check namespace errors except if the type is ANY */
    nserr = (btyp != NCX_BT_ANY);

#if 0  /**** TEMP!!! GETTING STARTED !!! ****/
    /* parse the attributes, if any; do not quick exit on this error */
    res2 = parse_metadata_nc(scb, msg, obj, startnode, 
	   nserr, retval, &editop);
#else
    res2 = NO_ERR;
#endif


    /* continue to parse the startnode depending on the base type 
     * to record as many errors as possible
     */
    switch (btyp) {
    case NCX_BT_ANY:
	res = parse_any(scb, startnode, retval);
	break;
    case NCX_BT_ENUM:
	res = parse_enum(scb, obj, startnode, retval);
	break;
    case NCX_BT_EMPTY:
	res = parse_empty(scb, obj, startnode, retval);
	break;
    case NCX_BT_BOOLEAN:
	res = parse_boolean(scb, obj, startnode, retval);
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
	res = parse_num(scb, obj, btyp, startnode, retval);
	break;
    case NCX_BT_KEYREF:
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
    case NCX_BT_INSTANCE_ID:
	res = parse_string(scb, obj, btyp, startnode, retval);
	break;
    case NCX_BT_UNION:
	res = parse_union(scb, obj, startnode, retval);
	break;
    case NCX_BT_LEAF_LIST:
	res = parse_leaflist(scb, obj, startnode, retval);
	break;
    case NCX_BT_CONTAINER:
    case NCX_BT_LIST:
	res = parse_complex(scb, obj, btyp, startnode, retval);
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* this will only be non-zero if the operation attribute
     * was seen in XML subtree for the value
     */
    retval->editop = editop;

    /* set the config flag for this value */
    res3 = NO_ERR;

#if 0   /**** TEMP ****/
    if (res == NO_ERR) {
	/* this has to be after the retval typdef is set */
	res3 = metadata_inst_check(scb, msg, retval);
	clean_metaerrs(retval);
    }
#endif

    if (res != NO_ERR) {
	retval->res = res;
	return res;
    } else if (res2 != NO_ERR) {
	retval->res = res2;
	return res2;
    }

    retval->res = res3;
    return res3;

} /* parse_one_btype */


/********************************************************************
* FUNCTION parse_btype
* 
* Switch to dispatch to specific base type handler
*
* INPUTS:
*     scb == session control block
*            Input is read from scb->reader.
*     obj == object template to use for parsing
*     startnode == top node of the parameter to be parsed
*            Parser function will attempt to consume all the
*            nodes until the matching endnode is reached
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*    msg->errQ may be appended with new errors or warnings
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    parse_btype (ses_cb_t  *scb,
		 const obj_template_t *obj,
		 const xml_node_t *startnode,
		 val_value_t  *retval)
{
    ncx_btype_t  btyp;

    btyp = obj_get_primary_basetype(obj);
    return parse_one_btype(scb, obj, btyp, startnode, retval);

} /* parse_btype */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION mgr_val_parse
* 
* parse a value for a YANG type from a NETCONF PDU XML stream 
*
* Parse NETCONF PDU sub-contents into value fields
* This module does not enforce complex type completeness.
* Different subsets of configuration data are permitted
* in several standard (and any proprietary) RPC methods
*
* A seperate parsing phase is used to validate the input
* contained in the returned val_value_t struct.
*
* This parsing phase checks that simple types are complete
* and child members of complex types are valid (but maybe 
* missing or incomplete child nodes.
*
* INPUTS:
*     scb == session control block
*     msg == incoming message
*     typ == typ_template_t for the parm or object type to parse
*     startnode == top node of the parameter to be parsed
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    mgr_val_parse (ses_cb_t  *scb,
		   const obj_template_t *obj,
		   const xml_node_t *startnode,
		   val_value_t  *retval)
{
    status_t  res;

#ifdef DEBUG
    if (!scb || !obj || !startnode || !retval) {
	/* non-recoverable error */
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef MGR_VAL_PARSE_DEBUG
    log_debug3("\nmgr_val_parse: %s:%s btyp:%s", 
	       obj_get_mod_prefix(obj),
	       obj_get_name(obj), 
	       tk_get_btype_sym(obj_get_basetype(obj)));
#endif

    /* get the element values */
    res = parse_btype(scb, obj, startnode, retval);
    return res;

}  /* mgr_val_parse */


#if 0
/********************************************************************
* FUNCTION find_metadef
* 
* Find the metadata descriptor, if any, in the typdef chain
*
* INPUTS:
*     typdef == typdef to check
*     name == attribute name to find
*
* RETURNS:
*   pointer to metadata descriptor or NULL of not found
*********************************************************************/
static typ_child_t *
    find_metadef (typ_def_t *typdef,
		  const xmlChar *name)
{
    typ_child_t  *meta;

    for (;;) {
	meta = typ_find_meta(typdef, name);
	if (meta) {
	    return meta;
	}
	
	typdef = typ_get_parent_typdef(typdef);
	if (!typdef) {
	    return NULL;
	}
    }
    /*NOTREACHED*/

} /* find_metadef */



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
*     val == value to check for metadata errors
*     
* RETURNS:
*   status
*********************************************************************/
static status_t 
    metadata_inst_check (val_value_t  *val)
{
    const typ_def_t   *typdef;
    typ_child_t       *metadef;
    uint32             cnt;
    status_t           res;
    boolean            first;

    /* first check the inst count of the operation attribute */
    cnt = val_metadata_inst_count(val, xmlns_nc_id(), 
				  NC_OPERATION_ATTR_NAME);
    if (cnt > 1) {
	return ERR_NCX_EXTRA_ATTR;
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
		return res;
	    }
	}
    }
    return NO_ERR;

} /* metadata_inst_check */


/********************************************************************
* FUNCTION parse_metadata
* 
* Parse all the XML attributes in the specified xml_node_t struct
*
* Only XML_NT_START or XML_NT_EMPTY nodes will have attributes
*
* INPUTS:
*     typdef == first non-ptr-only typdef for this type
*     nserr == TRUE if namespace errors should be checked
*           == FALSE if not, and any attribute is accepted 
*     node == node of the parameter maybe with attributes to be parsed
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*    *editop contains the last value of the operation attribute
*      seen, if any; will be OP_EDITOP_NONE if not set
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    parse_metadata (typ_def_t *typdef,
		    const xml_node_t *node,
		    boolean nserr,
		    val_value_t  *retval,
		    op_editop_t  *editop)
{
    typ_child_t    *oper, *meta;
    xml_attr_t     *attr;
    typ_def_t      *metadef;
    val_value_t    *metaval;
    xmlns_id_t      ncid;
    status_t        res;
    boolean         isoper;

    /* setup the NETCONF operation attribute */
    oper = ncx_get_operation_attr();
    ncid =  xmlns_nc_id();

    res = NO_ERR;

    /* go through all the attributes in the node and convert
     * to val_value_t structs
     * find the correct typedef to match each attribute
     */
    for (attr = xml_get_first_attr(node);
	 attr != NULL;
	 attr = xml_next_attr(attr)) {

	meta = NULL;
	metadef = NULL;
	isoper = FALSE;

	/* check qualified and unqualified operation attribute,
	 * then the 'xmlns' attribute, then a defined attribute
	 */
	if (match_metaval(attr, ncid, NC_OPERATION_ATTR_NAME)) {
	    metadef = &oper->typdef;
	    isoper = TRUE;
	} else if (attr->attr_dname &&
		   !xml_strncmp(attr->attr_dname, 
				XMLNS, xml_strlen(XMLNS))) {
	    continue;   /* skip this 'xmlns' attribute */
	} else {
	    /* find the attribute definition in this typdef */
	    meta = find_metadef(typdef, attr->attr_name);
	    if (meta) {
		metadef = &meta->typdef;
	    } else if (!nserr) {
		metadef = typ_get_basetype_typdef(NCX_BT_STRING);
	    }
	}

	if (metadef) {
	    /* alloc a new value struct for rhe attribute */
	    metaval = val_new_value();
	    if (!metaval) {
		res = ERR_INTERNAL_MEM;
	    } else {
		/* parse the attribute string against the typdef */
		res = val_parse_meta(metadef, attr->attr_ns,
				     attr->attr_name, 
				     attr->attr_val, metaval);
		if (res == NO_ERR) {
		    if (isoper) {
			*editop = op_editop_id(attr->attr_val);
			/* don't save operation attr */
			val_free_value(metaval);
		    } else {
			dlq_enque(metaval, &retval->metaQ);
		    }
		} else {
		    val_free_value(metaval);
		}
	    }
	} else {
	    res = ERR_NCX_UNKNOWN_ATTRIBUTE;
	}

	if (res != NO_ERR) {
	    return res;
	}
    }
    return NO_ERR;

} /* parse_metadata */
#endif  /* 0 */


/* END file mgr_val_parse.c */
