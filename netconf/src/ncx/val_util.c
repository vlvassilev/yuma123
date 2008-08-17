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
	case OBJ_TYP_CASE:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
    return res;

} /* add_defaults */


/*************** E X T E R N A L    F U N C T I O N S  *************/

#if 0   /*   ***** TBD ******/
/********************************************************************
 * FUNCTION val_set_canonical_order
 * 
 * Change the child XML nodes throughout an entire subtree
 * to the canonical order defined in the object template
 *
 * Note that there is no canonical order defined for 
 * the contents of an ncx:root container
 *
 * Leaf objects will not be processed, if val is OBJ_TYP_LEAF
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
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_LIST:
    case OBJ_TYP_RPCIO:
    case OBJ_TYP_NOTIF:
	for (chobj = obj_get_first_child(obj);
	     chobj != NULL;
	     chobj = obj_get_next_child(chobj)) {

	    if (!obj_has_name(chobj)) {
		continue;
	    }

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
		case OBJ_TYP_CHOICE:
		    chval = val_get_choice_first_set(val, chval->obj);


		    break;
		case OBJ_TYP_CONTAINER:
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
    }


    for (chval = (val_value_t *)dlq_firstEntry(&tempQ);
	 chval

}  /* val_set_canonical_order */
#endif


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
    val_index_t       *valin;
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
	    valin = new_index(chval);
	    if (!valin) {
		res = ERR_INTERNAL_MEM;
	    } else {
		/* save the index marker record */
		chval->index = valin;
		dlq_enque(valin, &val->indexQ);
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


/* END file val_util.c */
