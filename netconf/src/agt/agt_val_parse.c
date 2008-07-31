/*  FILE: agt_val_parse.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
11feb06      abb      begun

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

#ifndef _H_agt_ps_parse
#include "agt_ps_parse.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val_parse
#include "agt_val_parse.h"
#endif

#ifndef _H_agt_xml
#include "agt_xml.h"
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

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_VAL_PARSE_DEBUG 1
#endif


/* forward declaration for recursive calls */
static status_t 
    parse_btype_nc (ses_cb_t  *scb,
		    xml_msg_hdr_t *msg,
		    typ_def_t *typdef,
		    const xml_node_t *startnode,
		    ncx_data_class_t parentdc,
		    val_value_t  *retval);



/********************************************************************
 * FUNCTION gen_index_chain
 * 
 * Create an index chain for the just-parsed table or container struct
 *
 * INPUTS:
 *   scb == session control block (NULL means don't record errors)
 *   msg == xml_msg_hdr_t in progress (NULL means don't record errors)
 *   instart == first typ_index_t in the chain to process
 *   val == the just parsed table row with the childQ containing
 *          nodes to check as index nodes
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    gen_index_chain (ses_cb_t  *scb,
		     xml_msg_hdr_t *msg,
		     const typ_index_t *instart,
		     val_value_t *val)
{
    status_t            res, retres;
    const typ_index_t  *in;

    retres = NO_ERR;

    /* 1 or more index components expected */
    for (in = instart; 
	 in != NULL;
	 in = (const typ_index_t *)dlq_nextEntry(in)) {
	res = val_gen_index_comp(in, val);
	if (res != NO_ERR) {
	    if (msg) {
		agt_record_error(scb, &msg->errQ, 
				 NCX_LAYER_OPERATION, res, 
				 NULL, NCX_NT_INDEX, in, 
				 NCX_NT_VAL, val);
	    }
	    retres = res;
	}
    }

    return retres;

}   /* gen_index_chain */

/********************************************************************
 * FUNCTION new_child_val
 * 
 * INPUTS:
 *   nsid == namespace ID of name
 *   name == name string (direct or strdup, based on copyname)
 *   copyname == TRUE is dname strdup should be used
 *   parent == parent node
 *   editop == requested edit operation
 *   
 * RETURNS:
 *   status
 *********************************************************************/
static val_value_t *
    new_child_val (xmlns_id_t   nsid,
		   const xmlChar *name,
		   boolean copyname,
		   val_value_t *parent,
		   op_editop_t editop)
{
    val_value_t *chval;

    chval = val_new_value();
    if (!chval) {
	SET_ERROR(ERR_INTERNAL_MEM);
	return NULL;
    }

    /* save a const pointer to the name of this field */
    if (copyname) {
	chval->dname = xml_strdup(name);
	if (chval->dname) {
	    chval->name = chval->dname;
	} else {
	    SET_ERROR(ERR_INTERNAL_MEM);
	    val_free_value(chval);
	    return NULL;
	}
    } else {
	chval->name = name;
    }

    chval->parent_typ = NCX_NT_VAL;
    chval->parent = parent;
    chval->editop = editop;
    chval->nsid = nsid;

    return chval;

} /* new_child_val */


/********************************************************************
 * FUNCTION find_next_child
 * 
 * Check the instance qualifiers and see if the specified node
 * is a valid (subsequent) child node.
 *
 * Example:
 *  
 *  struct foo { 
 *    int a?;
 *    int b?;
 *    int c*;
 *
 * Since a, b, and c are optional, all of them have to be
 * checked, even while node 'a' is expected
 * The caller will save the current child in case the pointer
 * needs to be backed up.
 *
 * INPUTS:
 *   ch   == current typ_child_t
 *   nsid == expected namespace ID
 *   chnode == xml_node_t of start element 
 *   useiqual == TRUE if the instance qualifiers
 *                   should be used
 *                == FALSE == if they should be honored
 * RETURNS:
 *   pointer to child that matched or NULL if no valid next child
 *********************************************************************/
static typ_child_t *
    find_next_child (typ_child_t *ch,
		     xmlns_id_t nsid,
		     const xml_node_t *chnode,
		     boolean useiqual)
{

    typ_child_t *chnext;
    status_t     res;

    chnext = ch;

    for (;;) {
	switch (chnext->typdef.iqual) {
	case NCX_IQUAL_ONE:
	case NCX_IQUAL_1MORE:
	    /* the current child is mandatory; this is an error */
	    if (useiqual) {
		return NULL;
	    }
	    /* else fall through to next case */
	case NCX_IQUAL_OPT:
	case NCX_IQUAL_ZMORE:
	    /* the current child is optional; keep trying
	     * try to get the next child in the complex type 
	     */
	    chnext = typ_next_child(chnext);
	    if (!chnext) {
		return NULL;
	    } else {
		res = xml_node_match(chnode, nsid, chnext->name, 
		     XML_NT_NONE);
		if (res == NO_ERR) {
		    return chnext;
		}
	    }
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return NULL;
	}
    }
    /*NOTREACHED*/

} /* find_next_child */


/********************************************************************
 * FUNCTION index_node_match
 * 
 * INPUTS:
 *   in   == current typ_index_t
 *   nsid == current namespace ID
 *   chnode == xml_node_t of start element 
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    index_node_match (typ_index_t *in,
		      xmlns_id_t   nsid,
		      const xml_node_t *chnode)
{
    status_t  res;

    res = xml_node_match(chnode, nsid, in->typch.name, XML_NT_NONE);
    if (res != NO_ERR) {
	if (in->sname) {
	    res = xml_node_match(chnode, nsid, in->sname, XML_NT_NONE);
	}
    }
    return res;

} /* index_node_match */


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
 * FUNCTION pick_dataclass
 * 
 * Pick the correct data class for this value node
 *
 * INPUTS:
 *   parentdc == parent data class
 *   typdef == type definition struct for the value node
 *
 * RETURNS:
 *   data class for this value node
 *********************************************************************/
static ncx_data_class_t
    pick_dataclass (ncx_data_class_t parentdc,
		    const typ_def_t *typdef)
{
    ncx_data_class_t  dc;

    dc = typ_get_dataclass(typdef);
    if (dc == NCX_DC_NONE) {
	dc = parentdc;
    }
    return dc;

} /* pick_dataclass */


/********************************************************************
 * FUNCTION parse_any_nc
 * 
 * Parse the XML input as an 'any' type 
 *
 * INPUTS:
 *   see parse_btype_nc parameter list
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_any_nc (ses_cb_t  *scb,
		  xml_msg_hdr_t *msg,
		  const xml_node_t *startnode,
		  ncx_data_class_t parentdc,
		  val_value_t  *retval)
{
    xml_node_t         nextnode;
    const xml_node_t  *errnode;
    val_value_t       *chval, *lastval;
    status_t           res, res2;
    boolean            done, getstrend, errdone;
    typ_def_t         *anytypdef;

    /* init local vars */
    getstrend = FALSE;
    errnode = startnode;
    done = FALSE;
    errdone = FALSE;
    anytypdef = typ_get_basetype_typdef(NCX_BT_ANY);
    xml_init_node(&nextnode);
    res = NO_ERR;
    res2 = NO_ERR;
    lastval = NULL;
    retval->dataclass = parentdc;

    /* make sure the startnode is correct */
    switch (startnode->nodetyp) {
    case XML_NT_START:
	break;
    case XML_NT_EMPTY:
	/* treat this 'any' is a 'flag' data type  */
	retval->btyp = NCX_BT_EMPTY;
	retval->typdef = typ_get_basetype_typdef(NCX_BT_EMPTY);
	retval->v.bool = TRUE;
	return NO_ERR;
    default:
	res = ERR_NCX_WRONG_NODETYP;
    }

    if (res == NO_ERR) {
	/* at this point have either a simple type or a complex type
	 * get the next node which could be any type 
	 */
	res = agt_xml_consume_node_nons(scb->reader, &nextnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	}
    }

    if (res == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
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
	     * treated as a 'struct' data type
	     */
	    retval->btyp = NCX_BT_CONTAINER;
	    retval->typdef = typ_get_basetype_typdef(NCX_BT_CONTAINER);
	    break;
	case XML_NT_STRING:
	    /* treat this string child node as the string
	     * content for the parent node
	     */
	    retval->btyp = NCX_BT_STRING;
	    retval->typdef = typ_get_basetype_typdef(NCX_BT_STRING);
	    retval->v.str = xml_strdup(nextnode.simval);
	    res = (retval->v.str) ? NO_ERR : ERR_INTERNAL_MEM;
	    getstrend = TRUE;
	    break;
	case XML_NT_END:
	    res = xml_endnode_match(startnode, &nextnode);
	    if (res == NO_ERR) {
		/* treat this start + end pair as a 'flag' data type */
		retval->btyp = NCX_BT_EMPTY;
		retval->typdef = typ_get_basetype_typdef(NCX_BT_EMPTY);
		retval->v.bool = TRUE;
		return NO_ERR;
	    } else {
		errnode = &nextnode;
	    }
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    errnode = &nextnode;
	}
    }

    /* check if processing a simple type as a string */
    if (getstrend) {
	/* need to get the endnode for startnode then exit */
	xml_clean_node(&nextnode);
	res2 = agt_xml_consume_node_nons(scb->reader, &nextnode,
		NCX_LAYER_OPERATION, &msg->errQ);
	if (res2 == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	    log_debug3("\nparse_any: expecting end node for %s", 
		   startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(&nextnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &nextnode);
	} else {
	    errdone = TRUE;
	}
	if (res2 != NO_ERR) {
	    errnode = &nextnode;
	}
    }

    /* check if there were any errors in the startnode */
    if (res != NO_ERR || res2 != NO_ERR) {
	if (!errdone) {
	    /* add rpc-error to msg->errQ */
	    (void)agt_ps_parse_error_subtree(scb, msg, startnode,
		 errnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	}
	xml_clean_node(&nextnode);
	return (res==NO_ERR) ? res2 : res;
    }

    if (getstrend) {
	return NO_ERR;
    }

    /* if we get here, then the startnode is a struct */
    val_init_complex(retval, NCX_BT_CONTAINER);

    while (!done) {
	/* At this point have a nested start node
	 *  Allocate a new val_value_t for the child value node 
	 */
	res = NO_ERR;
	chval = new_child_val(nextnode.nsid, nextnode.elname, 
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
	     */
	    res = parse_btype_nc(scb, msg, anytypdef, &nextnode, 
				 retval->dataclass, chval);
	    xml_clean_node(&nextnode);
	    if (res == NO_ERR) {
		/* success - save the child value node 
		 * check the lastval and set the seqid if needed
		 */
		if (lastval) {
		    if (!xml_strcmp(lastval->name, chval->name)) {
			if (lastval->seqid == 0) {
			    lastval->seqid = 1;
			}
			chval->seqid = lastval->seqid+1;
		    } 
		}
		lastval = chval;

		val_add_child(chval, retval);
	    } else {
		errdone = TRUE;
	    }
	}

	/* record any error, if not already done */
	if (res != NO_ERR) {
	    if (!errdone) {
		/* add rpc-error to msg->errQ */
		(void)agt_ps_parse_error_subtree(scb, msg, startnode,
		     errnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	    }
	    xml_clean_node(&nextnode);
	    if (chval) {
		val_free_value(chval);
	    }
	    return res;
	}

	/* get the next node */
	res = agt_xml_consume_node_nons(scb->reader, &nextnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
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

} /* parse_any_nc */


/********************************************************************
 * FUNCTION parse_root_appnode_nc
 * 
 * Parse the XML input as an 'root' appnode 
 *
 *   <appname>
 *     <parmset-name> ... </parmset-name>
 *     <parmset-name> ... </parmset-name>
 *   </appname>
 *
 * INPUTS:
 *   see parse_btype_nc parameter list
 *
 *   startnode == <app-name> container start node
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_root_appnode_nc (ses_cb_t  *scb,
			   xml_msg_hdr_t *msg,
			   const xml_node_t *startnode,
			   const ncx_appnode_t *appdefnode,
			   val_value_t  *retval)
{
    xml_node_t           chnode;
    boolean              psdone, empty;
    cfg_app_t           *app;
    psd_template_t      *psd;
    ps_parmset_t        *ps;
    status_t             res, retres;
    ncx_node_t           deftyp;

    /* make sure the node is a start node 
     * this should be the application container
     */
    if (startnode->nodetyp != XML_NT_START) {
	/* add rpc-error to msg->errQ */
	res = ERR_NCX_WRONG_NODETYP;
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     startnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	return res;
    }

    /* get a new config application node */
    app = cfg_new_appnode();
    if (!app) {
	/* add rpc-error to msg->errQ */
	res = ERR_INTERNAL_MEM;
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
		     startnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	return res;
    }
    app->appdef = appdefnode;
    app->editop = get_editop(startnode);
    app->parent = retval;

    /* save the current expected namespace for nested complex types 
     * don't need to reset, since it only matters on the way down
     * the XML tree during parsing
     */
    msg->cur_appns = appdefnode->nsid;

    /* queue the appnode now, even if errors, and even if empty */
    dlq_enque(app, &retval->v.appQ);

    /* loop through all the parmset nodes and create ps_parmset_t nodes */
    psdone = FALSE;
    retres = NO_ERR;
    xml_init_node(&chnode);
    while (!psdone) {
	/* get the next node which should be an parmset
	 * start node or the application container end node 
	 *
	 * hard-wiring the layer to 'content' 
	 * since all usage of this data type is for
	 * nesting content-level data models
	 */
	xml_clean_node(&chnode);
	res = agt_xml_consume_node(scb->reader, &chnode,
	       NCX_LAYER_CONTENT, &msg->errQ);
	if (res != NO_ERR) {
	    (void)agt_ps_parse_error_subtree(scb, msg, startnode,
		     &chnode, res, NCX_NT_NONE, NULL, NCX_NT_APP, app);
	    xml_clean_node(&chnode);
	    return res;
	} 

	/* else got an XML node to examine */
#ifdef AGT_VAL_PARSE_DEBUG
	log_debug3("\nparse_root: expecting parmset-start or app-end node.");
	if (LOGDEBUG3) {
	    xml_dump_node(&chnode);
	}
#endif
	/* validate the node type and namespace */
	switch (chnode.nodetyp) {
	case XML_NT_START:
	    empty = FALSE;
	    res = xml_node_match(&chnode, msg->cur_appns, 
				 NULL, XML_NT_NONE);
	    break;
	case XML_NT_EMPTY:
	    /* empty parmset node is allowed; exit */
	    empty = TRUE;
	    res = xml_node_match(&chnode, msg->cur_appns, 
				 NULL, XML_NT_NONE);
	    break;
	case XML_NT_STRING:
	    res = ERR_NCX_WRONG_NODETYP_SIM;
	    break;
	case XML_NT_END:
	    res = xml_endnode_match(startnode, &chnode);
	    if (res == NO_ERR) {
		/* no error exit */
		psdone = TRUE;
		continue;
	    }
	    break;
	default:
	    res = ERR_NCX_WRONG_NODETYP;
	}

	/* if node ok,
	 * look for a parmset definition for this (owner, elname) 
	 */
	if (res == NO_ERR) {
	    deftyp = NCX_NT_PSD;
	    psd = def_reg_find_moddef(chnode.module, 
		      chnode.elname, &deftyp);
	    res = (psd) ? NO_ERR : ERR_NCX_UNKNOWN_PSD;
	}

	/* check any errors with the app container node so far */
	if (res != NO_ERR) {
	    /* try to skip just this application node and continue */
	    (void)agt_ps_parse_error_subtree(scb, msg, 
			 (empty) ? NULL : &chnode,
			 &chnode, res, NCX_NT_NONE, NULL, NCX_NT_APP, app);
	    /* try the next app node */
	    retres = res;
	    continue;  
	}

	/* have a parmset definition node 
	 * malloc a new parmset 
	 */
	ps = ps_new_parmset();
	if (!ps) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    res = NO_ERR;
	    ps->editop = get_editop(&chnode);
	    ps->parent = app;
	    if (!empty) {
		res = agt_ps_parse_val(scb, msg, psd, &chnode, ps);
	    } else {
		ps_setup_parmset(ps, psd, psd->psd_type);
	    }
	}
	if (res != NO_ERR) {
	    retres = res;
	}

	/* set the parmset instance ID even if there are errors */
	res = val_gen_instance_id(NCX_NT_PARMSET, ps, 
		  NCX_IFMT_C, FALSE, &ps->instance);
	if (res != NO_ERR && res != ERR_NCX_NO_INSTANCE) {
	    retres = res;
	}

	/* don't know what kind of error processing requested, 
	 * so save the PS even though there are errors.
	 * This could also be an empty parmset, or a
	 * filled in parmset (normal case)
	 */
	ps->res = retres;
	dlq_enque(ps, &app->parmsetQ);
	
    }  /* psnode loop */

    /* it is okay to clean an already clean xml_node_t */
    xml_clean_node(&chnode);
    return retres;

} /* parse_root_appnode_nc */


/********************************************************************
 * FUNCTION parse_root_nc
 * 
 * Parse the XML input as an 'root' type , E.g.:
 *
 * <config>
 *   <appname>
 *     <parmset-name> ... </parmset-name>
 *     <parmset-name> ... </parmset-name>
 *   </appname>
 *   <appname>
 *     <parmset-name> ... </parmset-name>
 *     <parmset-name> ... </parmset-name>
 *   </appname>
 * </config>
 *
 * INPUTS:
 *   see parse_btype_nc parameter list
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_root_nc (ses_cb_t  *scb,
		   xml_msg_hdr_t *msg,
		   const xml_node_t *startnode,
		   ncx_data_class_t parentdc,
		   val_value_t  *retval)
{
    xml_node_t           chnode;
    boolean              appdone, empty;
    const ncx_appnode_t *appdefnode;
    status_t             res, retres;
    cfg_app_t           *app;

    /* make sure the node is a start node 
     * this should be the <config> or <data> container, 
     * or other container nested inside the data model
     */
    res = xml_node_match(startnode, msg->cur_appns,
			 NULL, XML_NT_START); 
    if (res != NO_ERR) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     startnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	return res;
    }

    /* setup the return value as a NCX_BT_ROOT */
    retval->btyp = NCX_BT_ROOT;
    retval->typdef = typ_get_basetype_typdef(NCX_BT_ROOT);
    retval->name = startnode->elname;
    retval->dname = NULL;
    retval->editop = get_editop(startnode);

    /* avoid a compiler warning */
    if (parentdc == NCX_DC_CONFIG) {
	retval->dataclass = parentdc;
    } else {
	retval->dataclass = NCX_DC_CONFIG;
    }	

    dlq_createSQue(&retval->v.appQ);
    
    /* loop through all the application nodes and create
     * N cfg_app_t nodes, each containing M parmsets
     */
    appdone = FALSE;
    retres = NO_ERR;
    xml_init_node(&chnode);
    while (!appdone) {
	/* get the next node which should be an application 
	 * start node or the container end node 
	 *
	 * hard-wiring the layer to 'content' is a not a hack
	 * since all usage of this data type is for
	 * nesting content-level data models
	 */
	xml_clean_node(&chnode);
	res = agt_xml_consume_node(scb->reader, &chnode,
	       NCX_LAYER_CONTENT, &msg->errQ);
	if (res != NO_ERR) {
	    (void)agt_ps_parse_error_subtree(scb, msg, startnode,
		 &chnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	    xml_clean_node(&chnode);
	    return res;
	} 

	/* else got an XML node to examine 
	 * this is the application header node, which should declare
	 * a namespace for the application
	 */

#ifdef AGT_VAL_PARSE_DEBUG
	log_debug3("\nparse_root: expecting app-start or end node.");
	if (LOGDEBUG3) {
	    xml_dump_node(&chnode);
	}
#endif

	/* validate the child node type and namespace */
	switch (chnode.nodetyp) {
	case XML_NT_START:
	    empty = FALSE;
	    break;
	case XML_NT_EMPTY:
	    /* special case -- empty application node
	     * this could be an edit-config 'delete'
	     * or a get-config subtree filter
	     */
	    empty = TRUE;
	    break;
	case XML_NT_STRING:
	    res = ERR_NCX_WRONG_NODETYP_SIM;
	    break;
	case XML_NT_END:
	    res = xml_endnode_match(startnode, &chnode);
	    if (res == NO_ERR) {
		/* no error exit */
		appdone = TRUE;
		continue;
	    } 
	    break;
	default:
	    res = ERR_NCX_WRONG_NODETYP;
	}

	/* if node ok,
	 * look for an application node for this (owner, elname) 
	 */
	if (res == NO_ERR) {
	    appdefnode = def_reg_find_cfgapp(chnode.module, 
			  chnode.elname, NCX_CFGID_RUNNING);
	    res = (appdefnode) ? NO_ERR : ERR_NCX_UNKNOWN_APP;
	}

	/* check any errors with the app container node so far */
	if (res != NO_ERR) {
	    /* try to skip just this application node and continue */
	    (void)agt_ps_parse_error_subtree(scb, msg, 
		 (empty) ? NULL : &chnode,
		 &chnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	    /* try the next app node */
	    retres = res;
	    continue;  
	}

	/* consume all the parmsets in this application container */
	if (!empty) {
	    res = parse_root_appnode_nc(scb, msg, &chnode, 
					appdefnode, retval);
	    if (res != NO_ERR) {
		retres = res;
	    }
	} else {
	    /* create and store an empty cfg_app_t node */
	    app = cfg_new_appnode();
	    app->appdef = appdefnode;
	    app->editop = get_editop(&chnode);
	    app->parent = retval;
	    dlq_enque(app, &retval->v.appQ);
	}
    } /* appnode loop */

    xml_clean_node(&chnode);
    return retres;

} /* parse_root_nc */


/********************************************************************
* FUNCTION parse_ename_nc
* 
* Parse the XML input as a 'ename' type 
* e.g..
*
* <foo><bar/></foo>
* <foo><bar></bar></foo>
* <foo><bar>    </bar></foo>
*
* INPUTS:
*   see parse_btype_nc parameter list
* RETURNS:
*   status
*********************************************************************/
static status_t 
    parse_ename_nc (ses_cb_t  *scb,
		    xml_msg_hdr_t *msg,
		    typ_def_t *typdef,
		    const xml_node_t *startnode,
		    ncx_data_class_t parentdc,
		    val_value_t  *retval)
{
    xml_node_t ename, endnode;
    const xml_node_t *errnode;
    const xmlChar *badval;
    status_t   res, res2;
    boolean errdone;

    /* init local vars */
    xml_init_node(&ename);
    xml_init_node(&endnode);
    errnode = startnode;
    badval = NULL;
    errdone = FALSE;
    res = NO_ERR;
    res2 = NO_ERR;

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, msg->cur_appns,
			 NULL, XML_NT_START); 
    if (res == NO_ERR) {
	/* get the next node which should be an empty element */
	res = agt_xml_consume_node(scb->reader, &ename, 
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	    errnode = &ename;
	}
    }

    if (res != NO_ERR) {
	if (!errdone) {
	    /* add rpc-error to msg->errQ */
	    (void)agt_ps_parse_error_subtree(scb, msg, startnode,
		 errnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	}
	xml_clean_node(&ename);
	return res;
    }

#ifdef AGT_VAL_PARSE_DEBUG
    log_debug3("\nparse_ename: expecting empty node.");
    if (LOGDEBUG3) {
	xml_dump_node(&ename);
    }
#endif

    /* validate the node type and empty content */
    switch (ename.nodetyp) {
    case XML_NT_START:
	/* check for alternate version with end node */
	res = agt_xml_consume_node(scb->reader, &endnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	} else {
#ifdef AGT_VAL_PARSE_DEBUG
	    log_debug3("\nparse_ename: expecting end node");
	    if (LOGDEBUG3) {
		xml_dump_node(&endnode);
	    }
#endif
	    switch (endnode.nodetyp) {
	    case XML_NT_END:
		res = xml_endnode_match(&ename, &endnode);
		if (res != NO_ERR) {
		    errnode = &endnode;
		} /* else success, consider this an empty el */
		break;
	    case XML_NT_STRING:
		/* allow an empty whitespace-only string here;
		 * if any non-whitespace then got a simple type
		 * instead of an empty element
		 */
		res = (xml_isspace_str(endnode.simval)) ?
		    NO_ERR : ERR_NCX_WRONG_NODETYP_SIM;
		if (res != NO_ERR) {
		    errnode = &ename;
		} else {
		    /* got an empty string; try again for the endnode */
		    xml_clean_node(&endnode);
		    res = agt_xml_consume_node(scb->reader, &endnode,
			   NCX_LAYER_OPERATION, &msg->errQ);
		    if (res != NO_ERR) {
			errdone = TRUE;
		    } else {
#ifdef AGT_VAL_PARSE_DEBUG
			log_debug3("\nparse_ename: expecting end node2");
			if (LOGDEBUG3) {
			    xml_dump_node(&endnode);
			}
#endif

			/* got a node; is it the matching endnode */
			res = xml_endnode_match(&ename, &endnode);
			if (res != NO_ERR) {
			    errnode = &endnode;
			}  /* else success, consider this an empty el */
		    }
		}
		break;
	    case XML_NT_START:
		/* got a start of a nested struct instead of
		 * an empty element 
		 */
		res = ERR_NCX_WRONG_NODETYP_CPX;
		errnode = &endnode;
		break;
	    default:
		res = ERR_NCX_WRONG_NODETYP;
		errnode = &endnode;
	    }
	}
	break;
    case XML_NT_EMPTY:
	/* got the simple case;  a real empty element 
	 * check the namespace
	 */
	res = xml_node_match(&ename, msg->cur_appns, 
			     NULL, XML_NT_NONE);
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
	errnode = &ename;
    }

    /* check if ename content matched */
    if (res == NO_ERR) {
	/* check if the ename element is one of the correct values */
	res = val_string_ok(typdef, NCX_BT_ENAME, ename.elname);
	if (res == NO_ERR) {
	    /* record the value even if there are errors after this */
	    retval->btyp = NCX_BT_ENAME;
	    retval->typdef = typdef;
	    retval->dataclass = pick_dataclass(parentdc, typdef);
	    retval->v.str = xml_strdup(ename.elname);
	    if (!retval->v.str) {
		res = ERR_INTERNAL_MEM;
	    }
	} else {
	    badval = ename.elname;
	}
    } 

    if (res == NO_ERR) {
	/* get the matching end node for startnode */
	xml_clean_node(&endnode);
	res2 = agt_xml_consume_node(scb->reader, &endnode,
		NCX_LAYER_OPERATION, &msg->errQ);
	if (res2 == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	    log_debug3("\nparse_ename: expecting end for %s",
		       startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(startnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &endnode);
	    if (res2 != NO_ERR) {
		errnode = &endnode;
	    }
	} else {
	    errdone = TRUE;
	}
    }

    if (res == NO_ERR) {
	res = res2;
    }

    /* check if any errors; record the first error */
    if (res != NO_ERR)  {
	if (!errdone) {
	    /* add rpc-error to msg->errQ */
	    (void)agt_ps_parse_error_subtree(scb, msg, startnode,
		 errnode, res, NCX_NT_STRING, badval, NCX_NT_VAL, retval);
	}
    }

    xml_clean_node(&ename);
    xml_clean_node(&endnode);

    return res;

} /* parse_ename_nc */


/********************************************************************
 * FUNCTION parse_enum_nc
 * 
 * Parse the XML input as a 'enum' type 
 * e.g..
 *
 * <foo>fred</foo>
 * <foo>11</foo>
 * <foo>fred(11)</foo>
 *
 * INPUTS:
 *   see parse_btype_nc parameter list
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_enum_nc (ses_cb_t  *scb,
		   xml_msg_hdr_t *msg,
		   typ_def_t *typdef,
		   const xml_node_t *startnode,
		   ncx_data_class_t parentdc,
		   val_value_t  *retval)
{
    xml_node_t valnode, endnode;
    const xml_node_t  *errnode;
    const xmlChar *badval;
    status_t   res, res2;
    boolean    errdone;


    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    errnode = startnode;
    errdone = FALSE;
    badval = NULL;
    res2 = NO_ERR;

    retval->typdef = typdef;
    retval->dataclass = pick_dataclass(parentdc, typdef);

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, msg->cur_appns, 
			 NULL, XML_NT_START); 
    if (res == NO_ERR) {
	/* get the next node which should be a string node */
	res = agt_xml_consume_node(scb->reader, &valnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	}
    }

    if (res == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	log_debug3("\nparse_enum: expecting string node.");
	if (LOGDEBUG3) {
	    xml_dump_node(&valnode);
	}
#endif

	/* validate the node type and enum string or number content */
	switch (valnode.nodetyp) {
	case XML_NT_START:
	    res = ERR_NCX_WRONG_NODETYP_CPX;
	    errnode = &valnode;
	    break;
	case XML_NT_STRING:
	    /* get the non-whitespace string here */
	    res = val_enum_ok(typdef, valnode.simval, 
		      &retval->v.enu.val, &retval->v.enu.name);
	    if (res == NO_ERR) {
		/* record the value even if there are errors after this */
		retval->btyp = NCX_BT_ENUM;
	    } else {
		badval = valnode.simval;
	    }
	    break;
	default:
	    res = ERR_NCX_WRONG_NODETYP;
	    errnode = &valnode;
	}

	/* get the matching end node for startnode */
	res2 = agt_xml_consume_node(scb->reader, &endnode,
		NCX_LAYER_OPERATION, &msg->errQ);
	if (res2 == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	    log_debug3("\nparse_ename: expecting end for %s", 
		       startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(&endnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &endnode);
	    if (res2 != NO_ERR) {
		errnode = &endnode;
	    }
	} else {
	    errdone = TRUE;
	}
    }

    if (res == NO_ERR) {
	res = res2;
    }

    /* check if any errors; record the first error */
    if ((res != NO_ERR) && !errdone) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     errnode, res, NCX_NT_STRING, badval, NCX_NT_VAL, retval);
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_enum_nc */


/********************************************************************
 * FUNCTION parse_empty_nc
 * For NCX_BT_EMPTY
 *
 * Parse the XML input as an 'empty' or 'boolean' type 
 * e.g.:
 *
 *  <foo/>
 * <foo></foo>
 * <foo>   </foo>
 *
 *
 * INPUTS:
 *   see parse_btype_nc parameter list
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_empty_nc (ses_cb_t  *scb,
		    xml_msg_hdr_t *msg,
		    typ_def_t *typdef,
		    const xml_node_t *startnode,
		    ncx_data_class_t parentdc,
		    val_value_t  *retval)
{
    xml_node_t        endnode;
    const xml_node_t *errnode;
    status_t          res;
    boolean           errdone;

    /* init local vars */
    xml_init_node(&endnode);
    errnode = startnode;
    errdone = FALSE;

    /* validate the node type and enum string or number content */
    switch (startnode->nodetyp) {
    case XML_NT_EMPTY:
	res = xml_node_match(startnode, msg->cur_appns,
	     NULL, XML_NT_NONE);
	break;
    case XML_NT_START:
	res = xml_node_match(startnode, msg->cur_appns,
	     NULL, XML_NT_NONE);
	if (res == NO_ERR) {
	    res = agt_xml_consume_node(scb->reader, &endnode,
		   NCX_LAYER_OPERATION, &msg->errQ);
	    if (res != NO_ERR) {
		errdone = TRUE;
	    } else {
#ifdef AGT_VAL_PARSE_DEBUG
		log_debug3("\nparse_ename: expecting end for %s", 
		       startnode->qname);
		if (LOGDEBUG3) {
		    xml_dump_node(&endnode);
		}
#endif
		res = xml_endnode_match(startnode, &endnode);
		if (res != NO_ERR) {
		    if (endnode.nodetyp != XML_NT_STRING ||
			!xml_isspace_str(endnode.simval)) {
			errnode = &endnode;
			res = ERR_NCX_WRONG_NODETYP;
		    } else {
			/* that was an empty string -- try again */
			xml_clean_node(&endnode);
			res = agt_xml_consume_node(scb->reader, &endnode,
	                    NCX_LAYER_OPERATION, &msg->errQ);
			if (res == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
			    log_debug3("\nparse_enum: expecting end for %s", 
				   startnode->qname);
			    if (LOGDEBUG3) {
				xml_dump_node(&endnode);
			    }
#endif
			    res = xml_endnode_match(startnode, &endnode);
			    if (res != NO_ERR) {
				errnode = &endnode;
			    }
			} else {
			    errdone = TRUE;
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
	retval->btyp = NCX_BT_EMPTY;
	retval->typdef = typdef;
	retval->v.bool = TRUE;
	retval->dataclass = pick_dataclass(parentdc, typdef);
    } else if (!errdone) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     errnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
    }

    xml_clean_node(&endnode);
    return res;

} /* parse_empty_nc */


/********************************************************************
 * FUNCTION parse_boolean_nc
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
 *   see parse_btype_nc parameter list
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_boolean_nc (ses_cb_t  *scb,
		      xml_msg_hdr_t *msg,
		      typ_def_t *typdef,
		      const xml_node_t *startnode,
		      ncx_data_class_t parentdc,
		      val_value_t  *retval)
{
    xml_node_t valnode, endnode;
    const xml_node_t  *errnode;
    const xmlChar *badval;
    status_t   res, res2;
    boolean    errdone;


    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    errnode = startnode;
    errdone = FALSE;
    badval = NULL;
    res2 = NO_ERR;

    retval->typdef = typdef;
    retval->dataclass = pick_dataclass(parentdc, typdef);

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, msg->cur_appns, 
			 NULL, XML_NT_START); 
    if (res == NO_ERR) {
	/* get the next node which should be a string node */
	res = agt_xml_consume_node(scb->reader, &valnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	}
    }

    if (res == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	log_debug3("\nparse_boolean: expecting string node.");
	if (LOGDEBUG3) {
	    xml_dump_node(&valnode);
	}
#endif

	/* validate the node type and enum string or number content */
	switch (valnode.nodetyp) {
	case XML_NT_START:
	    res = ERR_NCX_WRONG_NODETYP_CPX;
	    errnode = &valnode;
	    break;
	case XML_NT_STRING:
	    /* get the non-whitespace string here */
	    retval->btyp = NCX_BT_BOOLEAN;
	    if (ncx_is_true(valnode.simval)) {
		retval->v.bool = TRUE;
	    } else if (ncx_is_false(valnode.simval)) {
		retval->v.bool = FALSE;
	    } else {
		res = ERR_NCX_INVALID_VALUE;
		badval = valnode.simval;
	    }
	    break;
	default:
	    res = ERR_NCX_WRONG_NODETYP;
	    errnode = &valnode;
	}

	/* get the matching end node for startnode */
	res2 = agt_xml_consume_node(scb->reader, &endnode,
		NCX_LAYER_OPERATION, &msg->errQ);
	if (res2 == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	    log_debug3("\nparse_ename: expecting end for %s", 
		       startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(&endnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &endnode);
	    if (res2 != NO_ERR) {
		errnode = &endnode;
	    }
	} else {
	    errdone = TRUE;
	}
    }

    if (res == NO_ERR) {
	res = res2;
    }

    /* check if any errors; record the first error */
    if ((res != NO_ERR) && !errdone) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     errnode, res, NCX_NT_STRING, badval, NCX_NT_VAL, retval);
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_boolean_nc */


/********************************************************************
* FUNCTION parse_num_nc
* 
* Parse the XML input as a numeric data type 
*
* INPUTS:
*     scb == session control block
*            Input is read from scb->reader.
*     msg == incoming RPC message
*            Errors are appended to msg->errQ
*     typdef == first non-ptr-only typdef for this type
*     btyp == base type of the expected ordinal number type
*     startnode == top node of the parameter to be parsed
*            Parser function will attempt to consume all the
*            nodes until the matching endnode is reached
*     parentdc == data class of the parent node
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
    parse_num_nc (ses_cb_t  *scb,
		  xml_msg_hdr_t *msg,
		  typ_def_t *typdef,
		  ncx_btype_t  btyp,
		  const xml_node_t *startnode,
		  ncx_data_class_t parentdc,
		  val_value_t  *retval)
{
    xml_node_t valnode, endnode;
    const xml_node_t  *errnode;
    const xmlChar *badval;
    status_t   res, res2;
    boolean   errdone;

    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    badval = NULL;
    errnode = startnode;
    errdone = FALSE;

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, msg->cur_appns,
			 NULL, XML_NT_START); 
    if (res == NO_ERR) {
	/* get the next node which should be a string node */
	res = agt_xml_consume_node(scb->reader, &valnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	}
    }

    if (res != NO_ERR) {
	/* fatal error */
	if (!errdone) {
	    (void)agt_ps_parse_error_subtree(scb, msg, startnode,
		 errnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	}
	xml_clean_node(&valnode);
	return res;
    }

#ifdef AGT_VAL_PARSE_DEBUG
    log_debug3("\nparse_num: expecting string node.");
    if (LOGDEBUG3) {
	xml_dump_node(&valnode);
    }
#endif

    /* validate the number content */
    switch (valnode.nodetyp) {
    case XML_NT_START:
	res = ERR_NCX_WRONG_NODETYP_CPX;
	errnode = &valnode;
	break;
    case XML_NT_STRING:
	/* get the non-whitespace string here */
	res = ncx_decode_num(valnode.simval, btyp, &retval->v.num);
	if (res == NO_ERR) {
	    res = val_range_ok(typdef, btyp, &retval->v.num);
	    if (res == NO_ERR) {
		/* record the value even if there are errors after this */
		retval->btyp = btyp;
		retval->typdef = typdef;
		retval->dataclass = pick_dataclass(parentdc, typdef);
	    }
	}
	if (res != NO_ERR) {
	    badval = valnode.simval;
	}
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
	errnode = &valnode;
    }

    /* get the matching end node for startnode */
    res2 = agt_xml_consume_node(scb->reader, &endnode,
	    NCX_LAYER_OPERATION, &msg->errQ);
    if (res2 == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	log_debug3("\nparse_num: expecting end for %s", startnode->qname);
	if (LOGDEBUG3) {
	    xml_dump_node(&endnode);
	}
#endif
	res2 = xml_endnode_match(startnode, &endnode);
	if (res2 != NO_ERR) {
	    errnode = &endnode;
	}
    } else {
	errdone = TRUE;
    }

    if (res == NO_ERR) {
	res = res2;
    }

    /* check if any errors; record the first error */
    if ((res != NO_ERR) && !errdone) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     errnode, res, NCX_NT_STRING, badval, NCX_NT_VAL, retval);
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_num_nc */


/********************************************************************
* FUNCTION parse_string_nc
* 
* Parse the XML input as a numeric data type 
*
* INPUTS:
*     scb == session control block
*            Input is read from scb->reader.
*     msg == incoming RPC message
*            Errors are appended to msg->errQ
*     typdef == first non-ptr-only typdef for this type
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
    parse_string_nc (ses_cb_t  *scb,
		     xml_msg_hdr_t *msg,
		     typ_def_t *typdef,
		     ncx_btype_t  btyp,
		     const xml_node_t *startnode,
		     ncx_data_class_t parentdc,
		     val_value_t  *retval)
{
    xml_node_t         valnode, endnode;
    const xml_node_t  *errnode;
    const xmlChar     *badval;
    const typ_template_t *listtyp;
    status_t           res, res2;
    boolean            errdone, empty;
    ncx_btype_t        listbtyp;


    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    errnode = startnode;
    badval = NULL;
    errdone = FALSE;
    empty = FALSE;
    retval->btyp = btyp;
    retval->typdef = typdef;
    retval->dataclass = pick_dataclass(parentdc, typdef);

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, msg->cur_appns,
			 NULL, XML_NT_START); 
    if (res != NO_ERR) {
	res = xml_node_match(startnode, msg->cur_appns,
			     NULL, XML_NT_EMPTY); 
	if (res == NO_ERR) {
	    empty = TRUE;
	}
    }

    /* get the value string node */
    if (res == NO_ERR && !empty) {
	/* get the next node which should be a string node */
	res = agt_xml_consume_node(scb->reader, &valnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	}
    }

    /* check empty string corner case */
    if (empty) {
	if (btyp==NCX_BT_XLIST) {
	    /* check the empty xlist */
	    ncx_init_xlist(&retval->v.xlist);
	    res = val_xlist_ok(typdef, &retval->v.xlist);
	} else if (btyp==NCX_BT_SLIST) {
	    /* check the empty list */
	    listtyp = typ_get_listtyp(typdef);
	    listbtyp = typ_get_basetype(&listtyp->typdef);
	    ncx_init_list(&retval->v.list, listbtyp);
	    res = val_list_ok(typdef, &retval->v.list);
	} else {
	    /* check the empty string */
	    res = val_string_ok(typdef, btyp, (const xmlChar *)"");
	    if (res == NO_ERR) {
		retval->v.str = xml_strdup((const xmlChar *)"");
		if (!retval->v.str) {
		    res = ERR_INTERNAL_MEM;
		}
	    }
	}
    }

    if (res != NO_ERR) {
	if (!errdone) {
	    /* add rpc-error to msg->errQ */
	    (void)agt_ps_parse_error_subtree(scb, msg, startnode,
		 errnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	}
	xml_clean_node(&valnode);
	return res;
    }

    if (empty) {
	return NO_ERR;
    }

#ifdef AGT_VAL_PARSE_DEBUG
    log_debug3("\nparse_string: expecting string node.");
    if (LOGDEBUG3) {
	xml_dump_node(&valnode);
    }
#endif

    /* validate the number content */
    switch (valnode.nodetyp) {
    case XML_NT_START:
	res = ERR_NCX_WRONG_NODETYP_CPX;
	errnode = &valnode;
	break;
    case XML_NT_STRING:
	if (btyp==NCX_BT_XLIST) {
	    /* get the list of strings, then check them */
	    ncx_init_xlist(&retval->v.xlist);
	    res = ncx_set_xlist(valnode.simval, &retval->v.xlist);
	    if (res==NO_ERR) {
		res = val_xlist_ok(typdef, &retval->v.xlist);
	    }
	} else if (btyp==NCX_BT_SLIST) {
	    /* get the list of strings, then check them */
	    listtyp = typ_get_listtyp(typdef);
	    listbtyp = typ_get_basetype(&listtyp->typdef);
	    ncx_init_list(&retval->v.list, listbtyp);
	    res = ncx_set_list(listbtyp, valnode.simval, 
			       &retval->v.list);
	    if (res == NO_ERR) {
		res = ncx_finish_list(&listtyp->typdef, &retval->v.list);
	    }
	    if (res == NO_ERR) {
		res = val_list_ok(typdef, &retval->v.list);
	    }
	} else {
	    /* check the non-whitespace string */
	    res = val_string_ok(typdef, btyp, valnode.simval);
	}

	if (res != NO_ERR) {
	    badval = valnode.simval;
	} else {
	    /* record the value even if there are errors after this */
	    switch (btyp) {
	    case NCX_BT_STRING:
	    case NCX_BT_BINARY:
	    case NCX_BT_ENAME:
		retval->v.str = xml_strdup(valnode.simval);
		if (!retval->v.str) {
		    res = ERR_INTERNAL_MEM;
		}
		break;
	    case NCX_BT_SLIST:
	    case NCX_BT_XLIST:
		break;   /* value already set */
	    default:
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
	errnode = &valnode;
    }

    /* get the matching end node for startnode */
    res2 = agt_xml_consume_node(scb->reader, &endnode,
	    NCX_LAYER_OPERATION, &msg->errQ);
    if (res2 == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	log_debug3("\nparse_string: expecting end for %s", startnode->qname);
	if (LOGDEBUG3) {
	    xml_dump_node(&endnode);
	}
#endif
	res2 = xml_endnode_match(startnode, &endnode);
	
    } else {
	errdone = TRUE;
    }

    if (res == NO_ERR) {
	res = res2;
    }

    /* check if any errors; record the first error */
    if ((res != NO_ERR)	&& !errdone) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     errnode, res, NCX_NT_STRING, badval, NCX_NT_VAL, retval);
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_string_nc */


/********************************************************************
 * FUNCTION parse_union_nc
 * 
 * Parse the XML input as a 'union' type 
 * e.g..
 *
 * <foo>fred</foo>
 * <foo>11</foo>
 *
 * INPUTS:
 *   see parse_btype_nc parameter list
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_union_nc (ses_cb_t  *scb,
		    xml_msg_hdr_t *msg,
		    typ_def_t *typdef,
		    const xml_node_t *startnode,
		    ncx_data_class_t parentdc,
		    val_value_t  *retval)
{
    xml_node_t valnode, endnode;
    const xml_node_t  *errnode;
    const xmlChar *badval;
    status_t   res, res2;
    boolean    errdone;


    /* init local vars */
    xml_init_node(&valnode);
    xml_init_node(&endnode);
    errnode = startnode;
    errdone = FALSE;
    badval = NULL;
    res2 = NO_ERR;

    retval->btyp = NCX_BT_UNION;
    retval->typdef = typdef;
    retval->dataclass = pick_dataclass(parentdc, typdef);

    /* make sure the startnode is correct */
    if (res == NO_ERR) {
	res = xml_node_match(startnode, msg->cur_appns, 
			     NULL, XML_NT_START); 
    }
    if (res == NO_ERR) {
	/* get the next node which should be a string node */
	res = agt_xml_consume_node(scb->reader, &valnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	}
    }

    if (res == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	log_debug3("\nparse_union: expecting string or number node.");
	if (LOGDEBUG3) {
	    xml_dump_node(&valnode);
	}
#endif

	/* validate the node type and union node content */
	switch (valnode.nodetyp) {
	case XML_NT_START:
	    res = ERR_NCX_WRONG_NODETYP_CPX;
	    errnode = &valnode;
	    break;
	case XML_NT_STRING:
	    /* get the non-whitespace string here */
	    res = val_union_ok(typdef, valnode.simval, retval);
	    if (res != NO_ERR) {
		badval = valnode.simval;
	    }
	    break;
	default:
	    res = ERR_NCX_WRONG_NODETYP;
	    errnode = &valnode;
	}

	/* get the matching end node for startnode */
	res2 = agt_xml_consume_node(scb->reader, &endnode,
		NCX_LAYER_OPERATION, &msg->errQ);
	if (res2 == NO_ERR) {
#ifdef AGT_VAL_PARSE_DEBUG
	    log_debug3("\nparse_ename: expecting end for %s", 
		       startnode->qname);
	    if (LOGDEBUG3) {
		xml_dump_node(&endnode);
	    }
#endif
	    res2 = xml_endnode_match(startnode, &endnode);
	    if (res2 != NO_ERR) {
		errnode = &endnode;
	    }
	} else {
	    errdone = TRUE;
	}
    }

    if (res == NO_ERR) {
	res = res2;
    }

    /* check if any errors; record the first error */
    if ((res != NO_ERR) && !errdone) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     errnode, res, NCX_NT_STRING, badval, NCX_NT_VAL, retval);
    }

    xml_clean_node(&valnode);
    xml_clean_node(&endnode);
    return res;

} /* parse_union_nc */


/********************************************************************
 * FUNCTION finish_table_index
 * 
 * Create an index chain for the just-parsed table
 *
 * INPUTS:
 *   scb == session control block
 *   msg == xml_msg_hdr_t in progress
 *   typdef == typdef of the table 
 *   val == the just parsed table row with the childQ containing
 *          nodes to check as index nodes
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    finish_table_index (ses_cb_t  *scb,
			xml_msg_hdr_t *msg,
			typ_def_t *typdef,
			val_value_t *val)
{
    status_t           res;

    if (dlq_empty(&typdef->def.complex.indexQ)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    res = gen_index_chain(scb, msg,
			  typ_first_index(TYP_DEF_COMPLEX(typdef)),
			  val);

    return res;

} /* finish_table_index */


/********************************************************************
 * FUNCTION parse_complex_nc
 * 
 * Parse the XML input as a complex type
 *
 * Handles the following base types:
 *   NCX_BT_CONTAINER
 *   NCX_BT_CHOICE
 *   NCX_BT_LIST
 *
 * E.g., struct:
 *
 * <foo>
 *   <a>blah</a>
 *   <b>7</b>
 *   <c/>
 * </foo>
 *
 * In an instance document, structs, choices, and tables look 
 * the same.  The validation is different of course, but the
 * parsing is basically the same.
 *
 * INPUTS:
 *   see parse_btype_nc parameter list
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_complex_nc (ses_cb_t  *scb,
		      xml_msg_hdr_t *msg,
		      typ_def_t *typdef,
		      ncx_btype_t btyp,
		      const xml_node_t *startnode,
		      ncx_data_class_t parentdc,
		      val_value_t  *retval)
{
    xml_node_t         chnode;
    const xml_node_t  *errnode;
    val_value_t       *chval, *lastval;
    typ_child_t       *ch, *nextch;
    typ_index_t       *instart, *infind, *in;
    typ_def_t         *realtypdef;
    status_t           res, res2, retres;
    boolean            done, usein, errdone;
    ncx_btype_t        chbtyp;

    /* setup local vars */
    xml_init_node(&chnode);
    errnode = startnode;
    in = NULL;
    instart = NULL;
    ch = NULL;
    lastval = NULL;
    res = NO_ERR;
    res2 = NO_ERR;
    retres = NO_ERR;
    done = FALSE;

    /* make sure the startnode is correct */
    res = xml_node_match(startnode, msg->cur_appns,
			 NULL, XML_NT_START); 
    if (res == NO_ERR) {
	/* start setting up the return value */
	val_init_complex(retval, btyp);
	retval->typdef = typdef;
	retval->name = startnode->elname;
	retval->dname = NULL;
	retval->editop = get_editop(startnode);
	retval->dataclass = pick_dataclass(parentdc, typdef);
	realtypdef = typ_get_base_typdef(typdef);
	

	/* check if an index pointer needs to be setup */
	if (btyp==NCX_BT_LIST) {
	    instart = typ_first_index(&realtypdef->def.complex);
	}

	/* setup the first child in the struct typedef */
	ch = typ_first_child(&realtypdef->def.complex);

	/* make sure there is at least 1 index or child node defined */
	if (!instart && !ch) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* check any errors in the startnode */
    if (res != NO_ERR) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     errnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	return res;
    }

    /* go through each child node until the parent end node */
    while (!done) {
	/* init per-loop vars */
	usein = FALSE;
	res2 = NO_ERR;
	errdone = FALSE;

	/* get the next node which should be a child or end node */
	res = agt_xml_consume_node(scb->reader, &chnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	    done = TRUE;
	} else {
#ifdef AGT_VAL_PARSE_DEBUG
	    log_debug3("\nparse_complex: expecting start-child or end node.");
	    if (LOGDEBUG3) {
		xml_dump_node(&chnode);
	    }
#endif
	    /* validate the struct member node type and namespace */
	    switch (chnode.nodetyp) {
	    case XML_NT_START:
	    case XML_NT_EMPTY:
		res = xml_node_match(&chnode, msg->cur_appns,
				     NULL, XML_NT_NONE);
		break;
	    case XML_NT_STRING:
		res = ERR_NCX_WRONG_NODETYP_SIM;
		errnode = &chnode;
		break;
	    case XML_NT_END:
		res = xml_endnode_match(startnode, &chnode);
		if (res != NO_ERR) {
		    errnode = &chnode;
		} else {
		    /* no error exit */
		    done = TRUE;
		    continue;
		}
		break;
	    default:
		res = ERR_NCX_WRONG_NODETYP;
		errnode = &chnode;
	    }
	}

	/* check if a child member should be used */
	if (res==NO_ERR) {
	    if (ch) {
		res = xml_node_match(&chnode, msg->cur_appns, 
				     ch->name, XML_NT_NONE);
		if (res != NO_ERR) {
		    /* check if there are other child nodes that could
		     * match, due to instance qualifiers 
		     */
		    nextch = find_next_child(ch, msg->cur_appns, 
				     &chnode, FALSE);
		    if (nextch) {
			/* found a valid next child */
			res = NO_ERR;
			ch = nextch;
		    }
		}
	    }
	    if (res != NO_ERR) {
		/* try to find a matching index */
		for (infind = instart;
		     infind != NULL && !usein;
		     infind = typ_next_index(infind)) {
		    res = index_node_match(infind, msg->cur_appns, 
				   &chnode);
		    if (res == NO_ERR) {
			in = infind;
			usein = TRUE;
		    }
		}
	    }
	}

	/* try to setup a new child node */
	if (res == NO_ERR) {
	    /* save the child base type */
	    chbtyp = typ_get_basetype((usein) ? 
				      &in->typch.typdef : &ch->typdef);

	    /* at this point, the 'in' or 'ch' template matches the
	     * 'chnode' owner and name;
	     * Allocate a new val_value_t for the child value node
	     */
	    chval = new_child_val(chnode.nsid,
		  (usein) ? in->typch.name : ch->name, FALSE,
		  retval, get_editop(&chnode));
	    if (!chval) {
		res = ERR_INTERNAL_MEM;
	    }
	}

	/* check any errors in setting up the child or index nodes */
	if (res != NO_ERR) {
	    /* try to skip just the child node sub-tree */
	    if (!errdone) {
		/* add rpc-error to msg->errQ */
		res2 = agt_ps_parse_error_subtree(scb, msg, &chnode,
		      errnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	    }
	    xml_clean_node(&chnode);
	    if (res2 != NO_ERR) {
		/* skip child didn't work, now skip the entire value subtree */
		(void)agt_xml_skip_subtree(scb->reader, startnode);
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
	
	res = parse_btype_nc(scb, msg, (usein) 
			     ? &in->typch.typdef : &ch->typdef, 
			     &chnode, retval->dataclass, chval);
	if (res == NO_ERR) {
	    /* success - save the child value node 
	     * check the lastval and set the seqid if needed;
	     * unnamed table entries and instance qualifiers create
	     * duplicates which need to be identified by ordinal value
	     */
	    if (lastval) {
		if (!xml_strcmp(lastval->name, chval->name)) {
		    if (lastval->seqid == 0) {
			lastval->seqid = 1;
		    }
		    chval->seqid = lastval->seqid+1;
		} 
	    }
	    lastval = chval;
	    val_add_child(chval, retval);

	    if (chbtyp != NCX_BT_LIST) {
		/* setup next child if the cur child is 0 - 1 instance */
		switch (ch->typdef.iqual) {
		case NCX_IQUAL_ONE:
		case NCX_IQUAL_OPT:
		    ch = typ_next_child(ch);
		    break;
		default:
		    break;
		}
	    }
	} else {
	    /* did not parse the child node correctly */
	    val_free_value(chval);
	    retres = res;
	}
	xml_clean_node(&chnode);
    }

    /* check if the index ID needs to be set */
    if (btyp==NCX_BT_LIST) {
	res = finish_table_index(scb, msg, realtypdef, retval);
	if (res != NO_ERR) {
	    retres = res;
	}
    }

    xml_clean_node(&chnode);
    return retres;

} /* parse_complex_nc */


/********************************************************************
 * FUNCTION finish_container_index
 * 
 * Create an index chain for the just-parsed container child node
 *
 * INPUTS:
 *   scb == session control block
 *   msg == xml_msg_hdr_t in progress
 *   typdef == typdef of the table 
 *   contab == the just parsed container tree that should contain the
 *            index components if complex type; 
 *            The childQ contains 0 - N nodes, each representing
 *            one row in the container.
 *            This child node could be a simple or complex type
 *
 * OUTPUTS:
 *   The indexQ in each child node is set
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    finish_container_index (ses_cb_t  *scb,
			   xml_msg_hdr_t *msg,
			   typ_def_t *typdef,
			   val_value_t *contab)
{
    typ_index_t       *instart;
    val_value_t       *chval;
    status_t           res, retres;
    uint32             ordindex;

    /* check what kind of index is present */
    instart = typ_first_index(TYP_DEF_COMPLEX(typdef));

    /* Set the contab flags in all child nodes
     * For noindex:
     *    treat the order of each entry as an index
     *    duplicates are allowed so the type or value of 
     *    the child node does not matter
     * For other index types, the ordindex will be ignored
     */
    ordindex = 0;
    for (chval = val_get_first_child(contab);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {
	chval->seqid = ordindex++;
	chval->flags |= VAL_FL_CONTAB;
    }

    if (!instart) {
	return NO_ERR;
    }

    /* only structs can even have more than one index here so
     * just generate index chains for them
     */
    retres = NO_ERR;
    for (chval = val_get_first_child(contab);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {
	if (chval->btyp==NCX_BT_CONTAINER) {
	    res = gen_index_chain(scb, msg, instart, chval);
	    if (res != NO_ERR) {
		retres = res;
	    }
	}
    }

    return retres;

} /* finish_container_index */


/********************************************************************
 * FUNCTION parse_container_nc
 * 
 * Parse the XML input as an 'container' type 
 *
 * INPUTS:
 *   see parse_btype_nc parameter list
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    parse_container_nc (ses_cb_t  *scb,
			xml_msg_hdr_t *msg,
			typ_def_t *typdef,
			const xml_node_t *startnode,
			ncx_data_class_t  parentdc,
			val_value_t  *retval)
{
    xml_node_t         chnode;
    status_t           res, res2, retres;
    boolean            empty, done, errdone;
    typ_child_t       *ch;
    typ_def_t         *realtypdef;
    val_value_t       *chval, *lastval;

    /* make sure the startnode is correct */
    switch (startnode->nodetyp) {
    case XML_NT_EMPTY:
	/* OK special case -- treat as an empty container */
	empty = TRUE;
	res = xml_node_match(startnode, msg->cur_appns,
			     NULL, XML_NT_NONE);
	break;
    case XML_NT_START:
	empty = FALSE;
	res = xml_node_match(startnode, msg->cur_appns,
			     NULL, XML_NT_NONE);
	break;
    default:
	res = ERR_NCX_WRONG_NODETYP;
    }

    /* check any errors in the startnode */
    if (res != NO_ERR) {
	/* add rpc-error to msg->errQ */
	(void)agt_ps_parse_error_subtree(scb, msg, startnode,
	     startnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	return res;
    }

    /* start setting up the return value */
    val_init_complex(retval, NCX_BT_XCONTAINER);
    retval->typdef = typdef;
    retval->name = startnode->elname;
    retval->dname = NULL;
    retval->editop = get_editop(startnode);
    retval->dataclass = pick_dataclass(parentdc, typdef);

    /* if this is an empty node, then nothing left to parse */
    if (empty) {
	return NO_ERR;
    }

    /* setup local vars */
    lastval = NULL;
    retres = NO_ERR;
    done = FALSE;
    xml_init_node(&chnode);

    realtypdef = typ_get_base_typdef(typdef);

    ch = (typ_child_t *)dlq_firstEntry(&realtypdef->def.complex.childQ);

    /* go through each child node until the parent end node */
    while (!done) {
	errdone = FALSE;
	xml_clean_node(&chnode);

	/* get the next node which should be the child or end node 
	 * There should be 0 to N instances of a single type of
	 * child node present, indicated by ch->typdef
	 */
	res = agt_xml_consume_node(scb->reader, &chnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res != NO_ERR) {
	    errdone = TRUE;
	} else {
#ifdef AGT_VAL_PARSE_DEBUG
	    log_debug3("\nparse_container:expecting start-child or end node.");
	    if (LOGDEBUG3) {
		xml_dump_node(&chnode);
	    }
#endif
	    /* validate the node type */
	    switch (chnode.nodetyp) {
	    case XML_NT_START:
		break;
	    case XML_NT_EMPTY:
		res = ERR_NCX_WRONG_NODETYP;
		break;
	    case XML_NT_STRING:
		res = ERR_NCX_WRONG_NODETYP_SIM;
		break;
	    case XML_NT_END:
		res = xml_endnode_match(startnode, &chnode);
		if (res == NO_ERR) {
		    done = TRUE;
		    continue;
		}
		break;
	    default:
		res = ERR_NCX_WRONG_NODETYP;
	    }
	}

	/* check if the child node is the correct name and namespace */
	if (res==NO_ERR) {
	    res = xml_node_match(&chnode, msg->cur_appns, 
			 ch->name, XML_NT_NONE);
	}

	/* start a child node if it was the correct name */
	if (res == NO_ERR) {
	    chval = val_new_value();
	    if (!chval) {
		res = ERR_INTERNAL_MEM;
	    } else {
		/* save a const pointer to the name of this field */
		chval->name = ch->name;
		chval->dname = NULL;
		chval->editop = get_editop(&chnode);
		chval->btyp = typ_get_basetype(&ch->typdef);
		chval->parent_typ = NCX_NT_VAL;
		chval->parent = retval;
	    }
	}

	/* if any errors so far, try to skip to the next sibling */
	if (res != NO_ERR) {
	    if (!errdone) {
		/* add rpc-error to msg->errQ */
		res2 = agt_ps_parse_error_subtree(scb, msg, &chnode,
		      &chnode, res, NCX_NT_NONE, NULL, NCX_NT_VAL, retval);
	    }
	    if (res2 != NO_ERR) {
		(void)agt_xml_skip_subtree(scb->reader, startnode);
		xml_clean_node(&chnode);
		return res;
	    } else {
		retres = res;
		continue;
	    }
	}

	/* recurse through and get whatever nodes are present
	 * in the child node
	 */
	res = parse_btype_nc(scb, msg, &ch->typdef, &chnode, 
			     retval->dataclass, chval);
	if (res == NO_ERR) {
	    /* success - save the child value node 
	     * check the lastval and set the seqid if needed
	     */
	    if (lastval) {
		if (!xml_strcmp(lastval->name, chval->name)) {
		    if (lastval->seqid == 0) {
			lastval->seqid = 1;
		    }
		    chval->seqid = lastval->seqid+1;
		} 
	    }
	    lastval = chval;
	    val_add_child(chval, retval);
	} else {
	    /* do not save the child node if there are parse errors */
	    val_free_value(chval);
	    retres = res;
	}


	/* Note that instance qualifiers on the container child type
	 * are not allowed, and ignored if the named type allows
	 * multiple or optional instances.
	 * By definition, a table or container with index components 
	 * already allows for 0 - N instances
	 */

    } /* loop through container item */

    /* Check the index and set the val_index_t chains as needed */
    res = finish_container_index(scb, msg, realtypdef, retval);
    if (res != NO_ERR) {
	retres = res;
    }

    xml_clean_node(&chnode);
    return retres;

} /* parse_container_nc */


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


/********************************************************************
* FUNCTION parse_metadata_nc
* 
* Parse all the XML attributes in the specified xml_node_t struct
*
* Only XML_NT_START or XML_NT_EMPTY nodes will have attributes
*
* INPUTS:
*     scb == session control block
*     msg == incoming RPC message
*            Errors are appended to msg->errQ
*     obj == object template containing meta-data definition
*     nserr == TRUE if namespace errors should be checked
*           == FALSE if not, and any attribute is accepted 
*     node == node of the parameter maybe with attributes to be parsed
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*    msg->errQ may be appended with new errors or warnings
*    *editop contains the last value of the operation attribute
*      seen, if any; will be OP_EDITOP_NONE if not set
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    parse_metadata_nc (ses_cb_t *scb,
		       xml_msg_hdr_t *msg,
		       typ_def_t *typdef,
		       const xml_node_t *node,
		       boolean nserr,
		       val_value_t  *retval,
		       op_editop_t  *editop)
{
    typ_child_t    *oper, *meta;
    xml_attr_t     *attr;
    typ_def_t      *metadef;
    val_value_t    *metaval;
    val_metaerr_t  *merr;
    xmlns_id_t      ncid;
    status_t        res, retres;
    boolean         isoper, copy;
    const xmlChar  *errname;

    /* setup the NETCONF operation attribute */
    oper = ncx_get_operation_attr();
    retres = NO_ERR;
    ncid =  xmlns_nc_id();

    /* go through all the attributes in the node and convert
     * to val_value_t structs
     * find the correct typedef to match each attribute
     */
    for (attr = xml_get_first_attr(node);
	 attr != NULL;
	 attr = xml_next_attr(attr)) {

	res = NO_ERR;
	meta = NULL;
	metadef = NULL;
	isoper = FALSE;
	errname = NULL;

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
		    if (isoper) {
			errname = NC_OPERATION_ATTR_NAME;
		    } else {
			errname = meta->name;
		    }
		    copy = FALSE;
		    val_free_value(metaval);
		}
	    }
	} else {
	    errname = attr->attr_name;
	    copy = TRUE;
	    res = ERR_NCX_UNKNOWN_ATTRIBUTE;
	}

	if (res != NO_ERR) {
	    if (errname) {
		merr = val_new_metaerr(retval->nsid, errname, copy);
		if (!merr) {
		    /* continue on without it */
		    SET_ERROR(ERR_INTERNAL_MEM);
		} else {
		    dlq_enque(merr, &retval->metaerrQ);
		}
	    } else {
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	    agt_ps_parse_error_attr(scb, msg, attr, node, 
		res, NCX_NT_VAL, retval);
	    retres = res;
	}
    }
    return retres;

} /* parse_metadata_nc */


/********************************************************************
* FUNCTION parse_btype_nc
* 
* Switch to dispatch to specific base type handler
*
* INPUTS:
*     scb == session control block
*            Input is read from scb->reader.
*     msg == incoming RPC message
*            Errors are appended to msg->errQ
*     typdef == first non-ptr-only typdef for this type
*     startnode == top node of the parameter to be parsed
*            Parser function will attempt to consume all the
*            nodes until the matching endnode is reached
*     parentdc == data class of the parent node, which will get
*                 used if it is not explicitly set for the typdef
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
    parse_btype_nc (ses_cb_t  *scb,
		    xml_msg_hdr_t *msg,
		    obj_template_t *obj,
		    const xml_node_t *startnode,
		    ncx_data_class_t parentdc,
		    val_value_t  *retval)
{
    typ_def_t   *typdef;
    ncx_btype_t  btyp;
    status_t     res, res2, res3;
    op_editop_t  editop;
    boolean      nserr;

    /* get the attribute values from the start node */
    editop = OP_EDITOP_NONE;
    retval->nsid = startnode->nsid;

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_LIST:
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
    case OBJ_TYP_RPCIO:
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* get the base type */
    btyp = typ_get_basetype(typdef);

    /* check namespace errors except if the type is ANY */
    nserr = (btyp != NCX_BT_ANY);

    /* parse the attributes, if any; do not quick exit on this error */
    res2 = parse_metadata_nc(scb, msg, typdef, startnode, 
	   nserr, retval, &editop);

    /* continue to parse the startnode depending on the base type 
     * to record as many errors as possible
     */
    switch (btyp) {
    case NCX_BT_ANY:
	res = parse_any_nc(scb, msg, startnode, parentdc, retval);
	break;
    case NCX_BT_ROOT:
	res = parse_root_nc(scb, msg, startnode, parentdc, retval);
	break;
    case NCX_BT_ENAME:
	res = parse_ename_nc(scb, msg, typdef, startnode, parentdc, retval);
	break;
    case NCX_BT_ENUM:
	res = parse_enum_nc(scb, msg, typdef, startnode, parentdc, retval);
	break;
    case NCX_BT_EMPTY:
	res = parse_empty_nc(scb, msg, typdef, startnode, parentdc, retval);
	break;
    case NCX_BT_BOOLEAN:
	res = parse_boolean_nc(scb, msg, typdef, startnode, parentdc, retval);
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
	res = parse_num_nc(scb, msg, typdef, btyp, startnode, 
			   parentdc, retval);
	break;
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_SLIST:
    case NCX_BT_XLIST:
    case NCX_BT_INSTANCE_ID:
	res = parse_string_nc(scb, msg, typdef, btyp, startnode, 
			      parentdc, retval);
	break;
    case NCX_BT_UNION:
	res = parse_union_nc(scb, msg, typdef, startnode, parentdc, retval);
	break;
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_LIST:
	res = parse_complex_nc(scb, msg, typdef, btyp, startnode, 
			       parentdc, retval);
	break;
    case NCX_BT_XCONTAINER:
	res = parse_container_nc(scb, msg, typdef, startnode, 
				 parentdc, retval);
	break;
    case NCX_BT_KEYREF:
	res = SET_ERROR(ERR_NCX_OPERATION_NOT_SUPPORTED);
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
    if (res == NO_ERR) {
	/* this has to be after the retval typdef is set */
	res3 = metadata_inst_check(scb, msg, retval);
	clean_metaerrs(retval);
    }

    if (res != NO_ERR) {
	return res;
    } else if (res2 != NO_ERR) {
	return res2;
    } else {
	return res3;
    }

} /* parse_btype_nc */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_val_parse
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
* Note that the NETCONF inlineFilterType will be parsed
* correctly because it is type 'anyxml'.  This type is
* parsed as a struct, and no validation other well-formed
* XML is done.
*
* INPUTS:
*     scb == session control block
*     msg == incoming RPC message
*     typ == typ_template_t for the parm or object type to parse
*     startnode == top node of the parameter to be parsed
*     parentdc == parent data class
*                 For the first call to this function, this will
*                 be the data class of a parameter.
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*    msg->errQ may be appended with new errors
*
* RETURNS:
*    status
*********************************************************************/
/* parse a value for an NCX type from a NETCONF PDU XML stream */
status_t 
    agt_val_parse (ses_cb_t  *scb,
		   xml_msg_hdr_t *msg,
		   typ_template_t *typ,
		   const xml_node_t *startnode,
		   ncx_data_class_t  parentdc,
		   val_value_t  *retval)
{
    typ_def_t    *typdef;

#ifdef DEBUG
    if (!scb || !msg || !typ || !startnode || !retval) {
	/* non-recoverable error */
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    typdef = &typ->typdef;

#ifdef AGT_VAL_PARSE_DEBUG
    log_debug3("\nparse_btype: name:%s btyp:%s", 
	   typ->name, tk_get_btype_sym(typ_get_basetype(typdef)));
#endif

    /* get the element values */
    return parse_btype_nc(scb, msg, typdef, startnode, parentdc, retval);

}  /* agt_val_parse */




/* END file agt_val_parse.c */
