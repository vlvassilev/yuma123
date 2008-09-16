/*  FILE: val_util.c

   val_value_t struct utilities for object validateion support
		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
19dec05      abb      begun
21jul08      abb      start obj-based rewrite

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

#ifndef _H_cli
#include "cli.h"
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

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define VAL_UTIL_DEBUG  1



/********************************************************************
* FUNCTION new_index
* 
* Malloc and initialize the fields in a val_index_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static val_index_t * 
    new_index (val_value_t *valnode)
{
    val_index_t  *in;

    in = m__getObj(val_index_t);
    if (!in) {
	return NULL;
    }
    in->val = valnode;
    return in;

}  /* new_index */




/********************************************************************
* FUNCTION choice_check
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
*   val == parent val_value_t to check
*   choicobj == object template for the choice to check
*
* OUTPUTS:
*   log any errors encountered
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    choice_check (val_value_t *val,
		  const obj_template_t *choicobj)
{
    val_value_t           *chval, *testval;
    status_t               res, retres;

    retres = NO_ERR;

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
	    retres = ERR_NCX_MISSING_CHOICE;
	    log_error("\nError: Nothing selected for "
		      "mandatory choice '%s'",
		      obj_get_name(choicobj));
	    ncx_print_errormsg(NULL, NULL, retres);
	}
	return retres;
    }

    /* else a choice was selected
     * first make sure all the mandatory case 
     * objects are present
     */
    res = val_instance_check(val, chval->casobj);
    if (res != NO_ERR) {
	retres = res;
    }

    /* check if any objects from other cases are present */
    testval = val_get_choice_next_set(val, choicobj, chval);
    while (testval) {
	if (testval->casobj != chval->casobj) {
	    /* error: extra case object in this choice */
	    retres = ERR_NCX_EXTRA_CHOICE;
	    log_error("\nError: Extra object '%s' "
		      "in choice '%s'; Case '%s' already selected", 
		      testval->name,
		      obj_get_name(choicobj),
		      obj_get_name(chval->casobj));
	    ncx_print_errormsg(NULL, NULL, retres);
	}
	testval = val_get_choice_next_set(val, choicobj, testval);
    }
    return retres;

}  /* choice_check */


/********************************************************************
 * FUNCTION add_defaults
 * 
 * Go through the specified value struct and add in any defaults
 * for missing leaf and choice nodes, that have defaults.
 *
 * !!! Only the child nodes will be checked for missing defaults
 * !!! The top-level value passed to this function is assumed to
 * !!! be already set
 *
 * This function does not handle top-level choice object subtrees.
 * This special case must be handled with the datadefQ
 * for the module.  If a top-level leaf value is passed in,
 * which is from a top-level choice case-arm, then the
 * rest of the case-arm objects will not get added by
 * this function.
 *
 * It is assumed that even top-level data-def-stmts will
 * be handled within a <config> container, so the top-level
 * object should always a container.
 *
 * INPUTS:
 *   val == the value struct to modify
 *   scriptmode == TRUE if the value is a script object access
 *              == FALSE for normal val_get_simval access instead
 *   addcas == obj_template for OBJ_TYP_CASE when adding defaults
 *             to a case
 *
 * OUTPUTS:
 *   *val and any sub-nodes are set to the default value as requested
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    add_defaults (val_value_t *val,
		  boolean scriptmode,
		  const obj_template_t *addcas)
{
    const obj_template_t *obj, *chobj, *casobj;
    const xmlChar        *defval;
    val_value_t          *chval, *testval;
    status_t              res;

#ifdef DEBUG
    /* test in static fn because it is so recursive */
    if (!val || !val->obj) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (addcas) {
	obj = addcas;
    } else {
	obj = val->obj;
    }

    res = NO_ERR;

    /* skip any uses or augment nodes */
    if (!obj_has_name(obj)) {
	return NO_ERR;
    }

    /* go through each child object node and determine
     * if a child value node exists for it or not
     *
     * If not, then determine if a default is needed
     * and available
     *
     * Objects without children will drop through the loop
     * All the uses and augment nodes will be skipped,
     */
    for (chobj = obj_first_child(obj);
	 chobj != NULL && res == NO_ERR;
	 chobj = obj_next_child(chobj)) {

	switch (chobj->objtype) {
	case OBJ_TYP_LEAF:
	    /* If the child leaf is required then it is marked
	     * as mandatory and no default exists
	     * If mandatory, then default is ignored
	     */
	    if (!obj_is_mandatory(chobj)) {
		chval = val_find_child(val, obj_get_mod_name(chobj),
				       obj_get_name(chobj));
		if (!chval) {
		    defval = obj_get_default(chobj);
		    if (defval) {
			chval = val_new_value();
			if (!chval) {
			    return ERR_INTERNAL_MEM;
			}
			val_init_from_template(chval, chobj);
			res = cli_parse_parm(val, chobj,
					    defval, scriptmode);
			if (res==NO_ERR) {
			    val->flags |= VAL_FL_DEFSET;
			}
		    }
		}
	    }
	    break;
	case OBJ_TYP_CHOICE:
	    /* if the choice is not set, and it has a default
	     * case, then add that case to the val struct
	     * If a partial case is present, then try to fill in 
	     * any of the nodes with defaults
	     */
	    if (obj_is_mandatory(chobj)) {
		break;
	    }

	    /* get the default case for this choice (if any) */
	    casobj = obj_get_default_case(chobj);

	    /* check if the choice has been set at all */
	    testval = val_get_choice_first_set(val, chobj);
	    if (testval) {
		/* use the selected case instead of the default case */
		casobj = testval->casobj;
		if (!casobj) {
		    res = SET_ERROR(ERR_INTERNAL_VAL);
		}
	    }
	    if (casobj) {
		/* add all the default nodes in the default case
		 * or selected case 
		 */
		res = add_defaults(val, scriptmode, casobj);
	    }
	    break;
	case OBJ_TYP_LEAF_LIST:
	    /* these object types never gets default entries */
	    break;
	case OBJ_TYP_CONTAINER:
	case OBJ_TYP_RPCIO:
	case OBJ_TYP_LIST:
	case OBJ_TYP_CASE:
	    /* add defaults to the subtrees of existing
	     * complex nodes, but do not add any new ones
	     */
	    chval = val_find_child(val, obj_get_mod_name(chobj),
				   obj_get_name(chobj));
	    if (chval) {
		res = add_defaults(chval, scriptmode, NULL);		
	    }

	    if (chobj->objtype == OBJ_TYP_LIST) {
		while (res == NO_ERR && chval) {
		    chval = val_find_next_child(val,
						obj_get_mod_prefix(chobj),
						obj_get_name(chobj),
						chval);
		    if (chval) {
			res = add_defaults(chval, scriptmode, NULL);
		    }
		}
	    }
	    break;
	case OBJ_TYP_RPC:
	case OBJ_TYP_NOTIF:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
    return res;

} /* add_defaults */


/********************************************************************
* FUNCTION get_index_comp
* 
* Get the index component string for the specified value
* 
* INPUTS:
*   mhdr == message hdr w/ prefix map or NULL to just use
*           the internal prefix mappings
*   format == desired output format
*   val == val_value_t for table or container
*   buff = buffer to hold result; NULL == get length only
*   
* OUTPUTS:
*   mhdr.pmap may have entries added if prefixes used
*      in the instance identifier which are not already in the pmap
*   *len = number of bytes that were (or would have been) written 
*          to buff
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_index_comp (xml_msg_hdr_t *mhdr,
		    ncx_instfmt_t format,
		    const val_value_t *val,
		    xmlChar *buff,
		    uint32  *len)
{
    const xmlChar      *prefix;
    uint32              cnt, total;
    status_t            res;
    boolean             quotes;

    total = 0;

    /* get the data type to determine if a quoted string is needed */
    switch (val->btyp) {
    case NCX_BT_ENUM:
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_BOOLEAN:
    case NCX_BT_KEYREF:
    case NCX_BT_UNION:
	quotes = (format==NCX_IFMT_CLI) ?
	    val_need_quotes(VAL_STR(val)) : TRUE;
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
	quotes = FALSE;
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check if foo:parmname='parmval' format is needed */
    if (format==NCX_IFMT_XPATH1 || format==NCX_IFMT_XPATH2) {
	if (mhdr) {
	    prefix = xml_msg_get_prefix_xpath(mhdr, val->nsid);
	} else {
	    prefix = xmlns_get_ns_prefix(val->nsid);
	}
	if (!prefix) {
	    return ERR_INTERNAL_MEM;
	}

	if (buff) {
	    buff += xml_strcpy(buff, prefix);
	    *buff++ = ':';
	}
	total += xml_strlen(prefix);
	total++;

	if (buff) {
	    buff += xml_strcpy(buff, val->name);
	}
	total += xml_strlen(val->name);

	if (buff) {
	    *buff++ = VAL_EQUAL_CH;
	}
	total++;

    }

    if (quotes) {
	if (buff) {
	    *buff++ = (xmlChar)((format==NCX_IFMT_XPATH1) ?
				VAL_QUOTE_CH : VAL_DBLQUOTE_CH);
	}
	total++;  
    }

    res = val_sprintf_simval_nc(buff, val, &cnt);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }
    if (buff) {
	buff += cnt;
    }
    total += cnt;

    if (quotes) {
	if (buff) {
	    *buff++ = (xmlChar)((format==NCX_IFMT_XPATH1) ?
				VAL_QUOTE_CH : VAL_DBLQUOTE_CH);
	}
	total++;  
    }

    /* end the string */
    if (buff) {
	*buff = 0;
    }

    *len = total;
    return NO_ERR;

}  /* get_index_comp */


/********************************************************************
* FUNCTION get_instance_string
* 
* Get the instance ID string for the specified value
* 
* INPUTS:
*   mhdr == message hdr w/ prefix map or NULL to just use
*           the internal prefix mappings
*   format == desired output format
*   val == value node to generate instance string for
*   buff = buffer to hold result; NULL == get length only
*   len == address of return length
*
* OUTPUTS:
*   mhdr.pmap may have entries added if prefixes used
*      in the instance identifier which are not already in the pmap
*   *len = number of bytes that were (or would have been) written 
*          to buff
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_instance_string (xml_msg_hdr_t *mhdr,
			 ncx_instfmt_t format,
			 const val_value_t *val,
			 xmlChar *buff,
			 uint32  *len)
{
    const xmlChar      *name, *prefix, *ncprefix;
    xmlChar             numbuff[NCX_MAX_NUMLEN+1];
    uint32              cnt, childcnt, total;
    status_t            res;
    boolean             root;
    xmlns_id_t          rpcid;

    /* init local vars */
    res = NO_ERR;
    total = 0;
    cnt = 0;
    root = FALSE;
    prefix = NULL;
    name = NULL;

    /* process the specific node type 
     * Recurively find the top node and start there
     */
    if (val->parent) {
	res = get_instance_string(mhdr, format, val->parent, 
				  buff, &cnt);
    } else {
	root = TRUE;
    }

    *len = 0;

    if (res != NO_ERR) {
	return res;
    }

    /* move the buffer pointer to the end to append */
    if (buff) {
	buff += cnt;
    }

    if (val->obj->objtype == OBJ_TYP_RPCIO) {
	/* get the prefix and name of the RPC method 
	 * instead of this node named 'input'
	 */
	rpcid = obj_get_nsid(val->obj->parent);
	if (rpcid) {
	    if (mhdr) {
		prefix = xml_msg_get_prefix_xpath(mhdr, rpcid);
	    } else {
		prefix = xmlns_get_ns_prefix(rpcid);
	    }
	}
	name = obj_get_name(val->obj->parent);
    } else {
	/* make sure the prefix is in the message header so
	 * an xmlns directive will be generated for this prefix
	 */
	if (mhdr) {
	    prefix = xml_msg_get_prefix_xpath(mhdr, val->nsid);
	} else {
	    prefix = xmlns_get_ns_prefix(val->nsid);
	}
	name = val->name;
    }

#ifdef DEBUG
    if (!prefix || !*prefix || !name || !*name) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* check if a path sep char is needed */
    switch (format) {
    case NCX_IFMT_C:
	if (cnt) {
	    if (buff) {
		*buff++ = VAL_INST_SEPCH;
	    }
	    cnt++;
	}
	break;
    default:
	if (buff) {
	    *buff++ = VAL_XPATH_SEPCH;   /*   starting '/' */
	}
	cnt++;
    }

    /* check if the 'rpc' root element needs to be added here */
    if (root) {
	/* copy prefix */
	if (mhdr) {
	    ncprefix = xml_msg_get_prefix_xpath(mhdr, xmlns_nc_id());
	    if (!ncprefix) {
		return ERR_INTERNAL_MEM;
	    }
	} else {
	    ncprefix = xmlns_get_ns_prefix(xmlns_nc_id());
	    if (!ncprefix) {
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
	if (buff) {
	    buff += xml_strcpy(buff, ncprefix);
	    *buff++ = ':';
	}
	cnt += xml_strlen(ncprefix);
	cnt++;

	/* copy name */
	if (buff) {
	    buff += xml_strcpy(buff, NCX_EL_RPC);
	}
	cnt += xml_strlen(NCX_EL_RPC);

	/* add another path sep char */
	if (buff) {
	    *buff++ = (xmlChar)((format==NCX_IFMT_C) ? 
				VAL_INST_SEPCH : VAL_XPATH_SEPCH);
	}
	cnt++;
    }


    /* add prefix string for this component */
    if (buff) {
	buff += xml_strcpy(buff, prefix);
	*buff++ = ':';
    }
    cnt += xml_strlen(prefix);
    cnt++;

    /* add name string for this component */
    if (buff) {
	buff += xml_strcpy(buff, name);
    }
    cnt += xml_strlen(name);

    total = cnt;

    /* check if this is a value node with an index clause */
    if (!dlq_empty(&val->indexQ)) {
	cnt = 0;
	res = val_get_index_string(mhdr, format, val, buff, &cnt);
	if (res == NO_ERR) {
	    if (buff) {
		buff[cnt] = 0;
	    }
	    total += cnt;
	}
    } else if (val->parent) {
	childcnt = val_child_inst_cnt(val->parent,
				      obj_get_mod_name(val->obj),
				      val->name);
	if (childcnt > 1) {	
	    /* there are multiple unnamed instances, so force
	     * an instance ID on any of them
	     */
	    cnt = val_get_child_inst_id(val->parent, val);
	    if (cnt > 0) {
		/* add an instance ID like foo[3] */
		if (buff) {
		    *buff++ = '[';
		}
		total++;

		sprintf((char *)numbuff, "%u", cnt);

		if (buff) {
		    buff += xml_strcpy(buff, numbuff);
		}
		total += xml_strlen(numbuff);

		if (buff) {
		    *buff++ = ']';
		}
		total++;
	    } else {
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
    }

    /* set the length even if index error, and exit */
    *len = total;
    return res;

}  /* get_instance_string */


/*************** E X T E R N A L    F U N C T I O N S  *************/


/********************************************************************
 * FUNCTION val_set_canonical_order
 * 
 * Change the child XML nodes throughout an entire subtree
 * to the canonical order defined in the object template
 *
 * >>> IT IS ASSUMED THAT ONLY VALID NODES ARE PRESENT
 * >>> AND ALL ERROR NODES HAVE BEEN PURGED ALREADY
 *
 * There is no canonical order defined for 
 * the contents of the following nodes:
 *    - anyxml leaf
 *    - ordered-by user leaf-list
 *
 * These nodes are not ordered, but their child nodes are ordered
 *    - ncx:root container
 *    - ordered-by user list
 *      
 * Leaf objects will not be processed, if val is OBJ_TYP_LEAF
 * Leaf-list objects will not be processed, if val is 
 * OBJ_TYP_LEAF_LIST.  These object types must be processed
 * within the context of the parent object.
 *
 * List child key nodes are ordered first among all
 * of the list's child nodes.
 *
 * List nodes with system keys are not kept in sorted order
 * This is not required by YANG.  Instead the user-given
 * order servers as the canonical order.  It is up to
 * the application setting the config to pick an
 * order for the list nodes.
 *
 * Also, leaf-list order is not changed, regardless of
 * the order of
 *
 * If a child node is found to be ncx:root, then it will
 * NOT be reordered.  A separate call is needed with the
 * root as the parameter to reorder the root contents.
 *
 * A root container as a child will be treated as an anyxml,
 * and just the child node itself will be reordered.  All its
 * children will be unchanged
 *
 * INPUTS:
 *   val == value node to change to canonical order
 *
 * OUTPUTS:
 *   val->v.childQ may be reordered, for all complex types
 *   in the subtree
 *
 *********************************************************************/
void
    val_set_canonical_order (val_value_t *val)
{
    const obj_template_t  *chobj;
    const obj_key_t       *key;
    dlq_hdr_t              tempQ;
    val_value_t           *chval;

#ifdef DEBUG
    if (!val || !val->obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    dlq_createSQue(&tempQ);

    switch (val->obj->objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
    case OBJ_TYP_USES:
    case OBJ_TYP_AUGMENT:
	break;
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
    case OBJ_TYP_RPC:
	SET_ERROR(ERR_INTERNAL_VAL);
	break;
    case OBJ_TYP_LIST:
	/* for a list, put all the key leafs first */
	for (key = obj_first_ckey(val->obj);
	     key != NULL;
	     key = obj_next_ckey(key)) {

	    chval = val_find_child(val, 
				   obj_get_mod_name(key->keyobj),
				   obj_get_name(key->keyobj));
	    if (chval) {
		val_remove_child(chval);
		dlq_enque(chval, &tempQ);
	    }
	}
	dlq_block_enque(&tempQ, &val->v.childQ);
	/* fall through to do the rest of the child nodes */
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_RPCIO:
    case OBJ_TYP_NOTIF:
	for (chobj = obj_first_child_deep(val->obj);
	     chobj != NULL;
	     chobj = obj_next_child_deep(chobj)) {

	    chval = val_find_child(val, 
				   obj_get_mod_name(chobj),
				   obj_get_name(chobj));
	    while (chval) {
		val_remove_child(chval);
		dlq_enque(chval, &tempQ);

		switch (chval->obj->objtype) {
		case OBJ_TYP_LEAF:
		case OBJ_TYP_LEAF_LIST:
		    break;
		case OBJ_TYP_CONTAINER:
		    if (obj_is_root(chval->obj)) {
			break;
		    } /* else fall through */
		case OBJ_TYP_LIST:
		    val_set_canonical_order(chval);
		    break;
		default:
		    ;
		}
		chval = val_find_child(val, 
				       obj_get_mod_name(chobj),
				       obj_get_name(chobj));
	    }
	}

	dlq_block_enque(&tempQ, &val->v.childQ);
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* val_set_canonical_order */


/********************************************************************
 * FUNCTION val_gen_index_comp
 * 
 * Create an index component
 *
 * INPUTS:
 *   in == obj_key_t in the chain to process
 *   val == the just parsed table row with the childQ containing
 *          nodes to check as index nodes
 *
 * OUTPUTS:
 *   val->indexQ will get a val_index_t record added if return NO_ERR
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t 
    val_gen_index_comp  (const obj_key_t *in,
			 val_value_t *val)
{
    val_value_t       *chval;
    status_t           res;
    boolean            found;

#ifdef DEBUG
    if (!in || !val) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* 1 or more index components expected */
    found = FALSE;
    res = NO_ERR;

    for (chval = (val_value_t *)dlq_firstEntry(&val->v.childQ);
	 chval != NULL && !found && res==NO_ERR;
	 chval = (val_value_t *)dlq_nextEntry(chval)) {

	if (chval->index) {
	    continue;
	} else if (chval->obj->mod != in->keyobj->mod) {
	    continue;
	} else if (!xml_strcmp(obj_get_name(in->keyobj), 
			       chval->name)) {
	    res = val_gen_key_entry(chval);
	    if (res == NO_ERR) {
		found = TRUE;
	    }
	}
    }

    if (res == NO_ERR && !found) {
	res = ERR_NCX_MISSING_INDEX;
    }

    return res;

}  /* val_gen_index_comp */


/********************************************************************
 * FUNCTION val_gen_key_entry
 * 
 * Create a key record within an index comp
 *
 * INPUTS:
 *   in == obj_key_t in the chain to process
 *   keyval == the just parsed table row with the childQ containing
 *          nodes to check as index nodes
 *
 * OUTPUTS:
 *   val->indexQ will get a val_index_t record added if return NO_ERR
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t 
    val_gen_key_entry  (val_value_t *keyval)
{
    val_index_t       *valin;
    status_t           res;

#ifdef DEBUG
    if (!keyval || !keyval->parent) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    valin = new_index(keyval);
    if (!valin) {
	res = ERR_INTERNAL_MEM;
    } else {
	/* save the index marker record */
	keyval->index = valin;
	dlq_enque(valin, &keyval->parent->indexQ);
    }

    return res;

}  /* val_gen_key_entry */


/********************************************************************
 * FUNCTION val_gen_index_chain
 * 
 * Create an index chain for the just-parsed table or container struct
 *
 * INPUTS:
 *   obj == list object containing the keyQ
 *   val == the just parsed table row with the childQ containing
 *          nodes to check as index nodes
 *
 * OUTPUTS:
 *   *val->indexQ has entries added for each index component, if NO_ERR
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t 
    val_gen_index_chain (const obj_template_t *obj,
			 val_value_t *val)
{
    const obj_key_t       *instart, *in;
    status_t               res;

#ifdef DEBUG
    if (!obj || !val) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (obj->objtype != OBJ_TYP_LIST) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    instart = (const obj_key_t *)
	dlq_firstEntry(obj->def.list->keyQ);

    /* 1 or more index components expected */
    for (in = instart; 
	 in != NULL;
	 in = (const obj_key_t *)dlq_nextEntry(in)) {
	res = val_gen_index_comp(in, val);
	if (res != NO_ERR) {
	    return res;
	}
    }

    return NO_ERR;

} /* val_gen_index_chain */


/********************************************************************
 * FUNCTION val_add_defaults
 * 
 * Go through the specified value struct and add in any defaults
 * for missing leaf and choice nodes, that have defaults.
 *
 * !!! Only the child nodes will be checked for missing defaults
 * !!! The top-level value passed to this function is assumed to
 * !!! be already set
 *
 * This function does not handle top-level choice object subtrees.
 * This special case must be handled with the datadefQ
 * for the module.  If a top-level leaf value is passed in,
 * which is from a top-level choice case-arm, then the
 * rest of the case-arm objects will not get added by
 * this function.
 *
 * It is assumed that even top-level data-def-stmts will
 * be handled within a <config> container, so the top-level
 * object should always a container.
 *
 * INPUTS:
 *   val == the value struct to modify
 *   scriptmode == TRUE if the value is a script object access
 *              == FALSE for normal val_get_simval access instead
 *
 * OUTPUTS:
 *   *val and any sub-nodes are set to the default value as requested
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t 
    val_add_defaults (val_value_t *val,
		      boolean scriptmode)
{
    return add_defaults(val, scriptmode, NULL);

} /* val_add_defaults */


/********************************************************************
* FUNCTION val_instance_check
* 
* Check for the proper number of object instances for
* the specified value struct. Checks the direct accessible
* children of 'val' only!!!
* 
* The 'obj' parameter is usually the val->obj field
* except for choice/case processing
*
* Log errors as needed and mark val->res as needed
*
* INPUTS:
*   val == value to check
*
* RETURNS:
*   status
*********************************************************************/
status_t
    val_instance_check (val_value_t  *val,
			const obj_template_t *obj)
{
    const obj_template_t  *chobj;
    ncx_iqual_t            iqual;
    uint32                 cnt, minelems, maxelems;
    boolean                minset, maxset, minerr, maxerr;
    status_t               res, retres;

    retres = NO_ERR;

    /* check all the child nodes for correct number of instances */
    for (chobj = obj_first_child(obj);
	 chobj != NULL;
	 chobj = obj_next_child(chobj)) {

	iqual = obj_get_iqualval(chobj);
	minerr = FALSE;
	maxerr = FALSE;

	switch (chobj->objtype) {
	case OBJ_TYP_LEAF_LIST:
	    minset = chobj->def.leaflist->minset;
	    minelems = chobj->def.leaflist->minelems;
	    maxset = chobj->def.leaflist->maxset;
	    maxelems = chobj->def.leaflist->maxelems;
	    break;
	case OBJ_TYP_LIST:
	    minset = chobj->def.list->minset;
	    minelems = chobj->def.list->minelems;
	    maxset = chobj->def.list->maxset;
	    maxelems = chobj->def.list->maxelems;
	    break;
	default:
	    minset = FALSE;
	    maxset = FALSE;
	}

	switch (chobj->objtype) {
	case OBJ_TYP_CHOICE:
	    res = choice_check(val, chobj);
	    if (res != NO_ERR) {
		retres = res;
	    }
	    continue;
	case OBJ_TYP_CASE:
	    retres = SET_ERROR(ERR_INTERNAL_VAL);
	    continue;
	default:
	    cnt = val_instance_count(val, 
				     obj_get_mod_name(chobj),
				     obj_get_name(chobj));
	}

	if (minset) {
	    if (cnt < minelems) {
		/* not enough instances error */
		minerr = TRUE;
		retres = ERR_NCX_MISSING_VAL_INST;
		log_error("\nError: Not enough instances of object '%s' "
			  "Got '%u', needed '%u'",
			  obj_get_name(chobj), cnt, minelems);
		ncx_print_errormsg(NULL, NULL, retres);
	    }
	}

	if (maxset) {
	    if (cnt > maxelems) {
		/* too many instances error */
		maxerr = TRUE;
		retres = ERR_NCX_EXTRA_VAL_INST;
		log_error("\nError: Too many instances of object '%s' entered "
			  "Got '%u', allowed '%u'",
			  obj_get_name(chobj), cnt, maxelems);
		ncx_print_errormsg(NULL, NULL, retres);
	    }
	}

	switch (iqual) {
	case NCX_IQUAL_ONE:
	    if (cnt < 1 && !minerr) {
		/* missing single parameter */
		retres = ERR_NCX_MISSING_VAL_INST;
		log_error("\nError: Mandatory object '%s' is missing",
			  obj_get_name(chobj));
		ncx_print_errormsg(NULL, NULL, retres);
	    } else if (cnt > 1 && !maxerr) {
		/* too many parameters */
		retres = ERR_NCX_EXTRA_VAL_INST;
		log_error("\nError: Extra instances of object '%s' entered",
			  obj_get_name(chobj));
		ncx_print_errormsg(NULL, NULL, retres);
	    }
	    break;
	case NCX_IQUAL_OPT:
	    if (cnt > 1 && !maxerr) {
		/* too many parameters */
		retres = ERR_NCX_EXTRA_VAL_INST;
		log_error("\nError: Extra instances of object '%s' entered",
			  obj_get_name(chobj));
		ncx_print_errormsg(NULL, NULL, retres);
	    }
	    break;
	case NCX_IQUAL_1MORE:
	    if (cnt < 1 && !minerr) {
		/* missing parameter error */
		retres = ERR_NCX_MISSING_VAL_INST;
		log_error("\nError: Mandatory object '%s' is missing",
			  obj_get_name(chobj));
		ncx_print_errormsg(NULL, NULL, retres);
	    }
	    break;
	case NCX_IQUAL_ZMORE:
	    break;
	default:
	    retres = SET_ERROR(ERR_INTERNAL_VAL);
	}

    }

    return retres;
    
}  /* val_instance_check */


/********************************************************************
* FUNCTION val_get_choice_first_set
* 
* Check a val_value_t struct against its expected OBJ
* to determine if a specific choice has already been set
* Get the value struct for the first value set for
* the specified choice
*
* INPUTS:
*   val == val_value_t to check
*   obj == choice object to check
*
* RETURNS:
*   pointer to first value struct or NULL if choice not set
*********************************************************************/
val_value_t *
    val_get_choice_first_set (val_value_t *val,
			      const obj_template_t *obj)
{
    val_value_t  *chval;

#ifdef DEBUG
    if (!val || !obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (chval = val_get_first_child(val);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {

	if (chval->casobj && chval->casobj->parent==obj) {
	    return chval;
	}
    }
    return NULL;

}  /* val_get_choice_first_set */


/********************************************************************
* FUNCTION val_get_choice_next_set
* 
* Check a val_value_t struct against its expected OBJ
* to determine if a specific choice has already been set
* Get the value struct for the next value set from the
* specified choice, afvter 'curval'
*
* INPUTS:
*   val == val_value_t to check
*   obj == choice object to check
*   curchild == current child selected from this choice (obj)
*
* RETURNS:
*   pointer to first value struct or NULL if choice not set
*********************************************************************/
val_value_t *
    val_get_choice_next_set (val_value_t *val,
			     const obj_template_t *obj,
			     val_value_t *curchild)
{
    val_value_t  *chval;

#ifdef DEBUG
    if (!val || !obj || !curchild) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    for (chval = val_get_next_child(curchild);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {

	if (chval->casobj && chval->casobj->parent==obj) {
	    return chval;
	}
    }
    return NULL;

}  /* val_get_choice_next_set */


/********************************************************************
* FUNCTION val_choice_is_set
* 
* Check a val_value_t struct against its expected OBJ
* to determine if a specific choice has already been set
* Check that all the mandatory config fields in the selected
* case are set
*
* INPUTS:
*   val == parent of the choice object to check
*   obj == choice object to check
*
* RETURNS:
*   pointer to first value struct or NULL if choice not set
*********************************************************************/
boolean
    val_choice_is_set (val_value_t *val,
		       const obj_template_t *obj)
{
    const obj_template_t  *cas, *child;
    val_value_t           *testval, *chval;
    boolean                done;

#ifdef DEBUG
    if (!val || !obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    done = FALSE;
    for (testval = val_get_first_child(val);
	 testval != NULL && !done;
	 testval = val_get_next_child(testval)) {

	if (testval->casobj && testval->casobj->parent==obj) {
	    chval = testval;
	    done = TRUE;
	}
    }

    if (!done) {
	return FALSE;
    }

    cas = chval->casobj;

    /* check if all the mandatory parms are present in this case */
    for (child = obj_first_child(cas);
	 child != NULL;
	 child = obj_next_child(child)) {

	if (!obj_is_config(child)) {
	    continue;
	}
	if (!obj_is_mandatory(child)) {
	    continue;
	}
	if (!val_find_child(val, obj_get_mod_name(child),
			   obj_get_name(child))) {
	    return FALSE;
	}
    }
    return TRUE;

}  /* val_choice_is_set */


/********************************************************************
* FUNCTION val_purge_errors_from_root
* 
* Remove any error nodes under a root container
* that were saved for error recording purposes
*
* INPUTS:
*   val == root container to purge
*
*********************************************************************/
void
    val_purge_errors_from_root (val_value_t *val)
{
    val_value_t           *chval, *nextval;

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    if (!obj_is_root(val->obj)) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    for (chval = val_get_first_child(val);
	 chval != NULL; 
	 chval = nextval) {

	nextval = val_get_next_child(chval);

	if (chval->res != NO_ERR) {
	    val_remove_child(chval);
	    val_free_value(chval);
	}
    }
    val->res = NO_ERR;

}  /* val_purge_errors_from_root */


/********************************************************************
 * FUNCTION val_new_child_val
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
val_value_t *
    val_new_child_val (xmlns_id_t   nsid,
		       const xmlChar *name,
		       boolean copyname,
		       val_value_t *parent,
		       op_editop_t editop)
{
    val_value_t *chval;

    chval = val_new_value();
    if (!chval) {
	return NULL;
    }

    /* save a const pointer to the name of this field */
    if (copyname) {
	chval->dname = xml_strdup(name);
	if (chval->dname) {
	    chval->name = chval->dname;
	} else {
	    val_free_value(chval);
	    return NULL;
	}
    } else {
	chval->name = name;
    }

    chval->parent = parent;
    chval->editop = editop;
    chval->nsid = nsid;

    return chval;

} /* val_new_child_val */



/********************************************************************
* FUNCTION val_gen_instance_id
* 
* Malloc and Generate the instance ID string for this value node, 
* 
* INPUTS:
*   mhdr == message hdr w/ prefix map or NULL to just use
*           the internal prefix mappings
*   val == node to generate the instance ID for
*   format == desired output format (NCX or Xpath)
*   buff == pointer to address of buffer to use
*
* OUTPUTS
*   mhdr.pmap may have entries added if prefixes used
*      in the instance identifier which are not already in the pmap
*   *buff == malloced buffer with the instance ID
*
* RETURNS:
*   status
*********************************************************************/
status_t
    val_gen_instance_id (xml_msg_hdr_t *mhdr,
			 const val_value_t  *val, 
			 ncx_instfmt_t format,
			 xmlChar  **buff)
{
    uint32    len;
    status_t  res;

#ifdef DEBUG 
    if (!val || !buff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* figure out the length of the parmset instance ID */
    res = get_instance_string(mhdr, format, val, NULL, &len);
    if (res != NO_ERR) {
	return res;
    }

    /* check no instance ID */
    if (len==0) {
	*buff = NULL;
	return ERR_NCX_NO_INSTANCE;
    }

    /* get a buffer to fit the instance ID string */
    *buff = (xmlChar *)m__getMem(len+1);
    if (!*buff) {
	return ERR_INTERNAL_MEM;
    } else {
	memset(*buff, 0x0, len+1);
    }

    /* get the instance ID string for real this time */
    res = get_instance_string(mhdr, format, val, *buff, &len);
    if (res != NO_ERR) {
	m__free(*buff);
	*buff = NULL;
    }

    return res;

}  /* val_gen_instance_id */


/********************************************************************
* FUNCTION val_get_index_string
* 
* Get the index string for the specified table or container entry
* 
* INPUTS:
*   mhdr == message hdr w/ prefix map or NULL to just use
*           the internal prefix mappings
*   format == desired output format
*   val == val_value_t for table or container
*   buff == buffer to hold result; 
         == NULL means get length only
*   
* OUTPUTS:
*   mhdr.pmap may have entries added if prefixes used
*      in the instance identifier which are not already in the pmap
*   *len = number of bytes that were (or would have been) written 
*          to buff
*
* RETURNS:
*   status
*********************************************************************/
status_t
    val_get_index_string (xml_msg_hdr_t *mhdr,
			  ncx_instfmt_t format,
			  const val_value_t *val,
			  xmlChar *buff,
			  uint32  *len)
{
    const val_value_t  *ival, *nextival;
    const val_index_t  *valin;
    uint32              cnt, total;
    status_t            res;

#ifdef DEBUG
    if (!val) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif
	
    total = 0;
    *len = 0;

    /* get the first index node value struct */
    valin = (const val_index_t *)dlq_firstEntry(&val->indexQ);
    if (valin) {
	ival = valin->val;
    } else {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check that there is at least one index component */
    if (format != NCX_IFMT_CLI) {
	/* start with the left bracket */
	if (buff) {
	    *buff++ = (xmlChar)VAL_BINDEX_CH;
	}
	total++;
    }

    /* at least one real index component; generate entire index */
    while (ival) {
	/* generate one component, or N if it is a struct */
	res = get_index_comp(mhdr, format, ival, buff, &cnt);
	if (res != NO_ERR) {
	    return res;
	} else {
	    if (buff) {
		buff += cnt;
	    }
	    total += cnt;
	}

	/* setup next index component */
	nextival = NULL;
	valin = (const val_index_t *)dlq_nextEntry(valin);
	if (valin) {
	    nextival = valin->val;
	} 

	/* check if an index separator string is needed */
	if (nextival) {
	    switch (format) {
	    case NCX_IFMT_C:
		/* add the index separator char ',' */
		if (buff) {
		    *buff++ = VAL_INDEX_SEPCH;
		}
		total++;
		break;
	    case NCX_IFMT_XPATH1:
	    case NCX_IFMT_XPATH2:
		/* format is one of the Xpath variants 
		 * add the index separator string ' and ' 
		 */
		if (buff) {
		    buff += xml_strcpy(buff, VAL_XPATH_INDEX_SEPSTR);
		}
		total += VAL_XPATH_INDEX_SEPLEN;
		break;
	    case NCX_IFMT_CLI:
		/* add the CLI index separator char ' ' */
		if (buff) {
		    *buff++ = VAL_INDEX_CLI_SEPCH;
		}
		total++;
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}

	/* setup the next index component */
	ival = nextival;
    }

    /* add the closing right bracket */
    if (format != NCX_IFMT_CLI) {
	if (buff) {
	    *buff++ = VAL_EINDEX_CH;
	    *buff = 0;
	}
	total++;
    }

    /* return success */
    *len = total;
    return NO_ERR;

    }  /* val_get_index_string */



/* END file val_util.c */
