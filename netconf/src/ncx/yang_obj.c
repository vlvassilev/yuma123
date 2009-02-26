/*  FILE: yang_obj.c

    YANG module parser, data-def-stmt support

    YANG data is (currently) much different than NCX parmsets
    because no complexTypes exist in YANG, and no groupings
    or augments exist in NCX.

    Currently migrating from NCX data format:
 
         /ns:application/parmset/parm/value/value/...

    to YANG object format, based on the obj_template_t struct

        /ns1:value/ns2:value/ns3:value/...

    An obj_template_t is essentially a QName + typ_def_t within the code

    Every definition node has a typ_def_t, and every value
    instance node has a val_value_t struct.  This allows
    engine callbacks to process arbitrarily complex
    data structues with the same code.

    There are 12 types of objects:

      OBJ_TYP_CONTAINER
      OBJ_TYP_LEAF
      OBJ_TYP_LEAF_LIST
      OBJ_TYP_LIST
      OBJ_TYP_CHOICE
      OBJ_TYP_CASE
      OBJ_TYP_USES
      OBJ_TYP_REFINE
      OBJ_TYP_AUGMENT
      OBJ_TYP_RPC
      OBJ_TYP_RPCIO
      OBJ_TYP_NOTIF

   These objects are grouped as follows:
      * concrete data node objects (container - list)
      * meta grouping constructs (choice, case) 
        and (uses, refine, augment)
      * RPC method objects (rpc, input, output)
      * notification objects (notification)

    5 Pass Validation Process

    In pass 1, the source file is parsed into YANG tokens.
    String concatentation are quoted string adjustment are
    handled in this pass.

    In pass 2, the objects are parsed via yang_obj_consume_datadef.
    Syntax errors and any other static errors are reported

    In pass 3, the object definitions are validated for correctness,
    via te yang_obj_resolve_datadefs function.  This is mixed with
    calls to yang_typ_resolve_typedefs and yang_grp_resolve_groupings.

    Uses and augments are not expanded in pass 3, so some details
    like key validation for a list cannot be done, since the
    contents may depend on the expanded uses or descendant form
    augment statement.
   
    In pass 4, groupings are completed with yang_grp_resolve_complete.
    Then all the uses-based data is cloned and placed into
    the tree, via yang_obj_resolve_uses

    In pass 5, all the augment-based data is cloned and placed into
    the tree, via yang_obj_resolve_augments

    The uses and augment objects are kept for
    XSD and other translation, and needed for internal data sharing.
    In a cloned object, a minimal amount of data is copied,
    and the rest is shadowed with back-pointers.

    For the 'uses' statement, refined objects are merged into
    the cloned tree as specified by the grouping and any
    refine statements within the uses statement.

    For the 'augment' statement, one exact clone of each augmenting
    node is placed in the target, based on the schema node target
    for the augment clause.


*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
09dec07      abb      begun; start from yang_typ.c
29nov08      abb      added when-stmt support as per yang-02

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_grp
#include  "grp.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xpath1
#include "xpath1.h"
#endif

#ifndef _H_xpath_yang
#include "xpath_yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yang_grp
#include "yang_grp.h"
#endif

#ifndef _H_yang_obj
#include "yang_obj.h"
#endif

#ifndef _H_yang_typ
#include "yang_typ.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define YANG_OBJ_DEBUG 1
#endif


/********************************************************************
*                                                                   *
*                          M A C R O S                              *
*                                                                   *
*********************************************************************/

/* used in parser routines to decide if processing can continue
 * will exit the function if critical error or continue if not
 * In all uses, there is a new object being constructed,
 * called 'obj', which must be freed before exit
 *
 * Unless the error is considered fatal, processing
 * continues in order to validate as much of the input
 * module as possible
 */
#define CHK_OBJ_EXIT(obj, res, retres)			  \
    if (res != NO_ERR) {				  \
	if (res < ERR_LAST_SYS_ERR || res==ERR_NCX_EOF) { \
	    obj_free_template(obj);			  \
	    return res;					  \
	} else {					  \
	    retres = res;				  \
	}						  \
    }


#define CHK_DEV_EXIT(dev, res, retres)			  \
    if (res != NO_ERR) {				  \
	if (res < ERR_LAST_SYS_ERR || res==ERR_NCX_EOF) { \
	    obj_free_deviation(dev);			  \
	    return res;					  \
	} else {					  \
	    retres = res;				  \
	}						  \
    }


#define CHK_DEVI_EXIT(devi, res, retres)		  \
    if (res != NO_ERR) {				  \
	if (res < ERR_LAST_SYS_ERR || res==ERR_NCX_EOF) { \
	    obj_free_deviate(devi);			  \
	    return res;					  \
	} else {					  \
	    retres = res;				  \
	}						  \
    }


/********************************************************************
*                                                                   *
*              F O R W A R D    D E C L A R A T I O N S             *
*                                                                   *
*********************************************************************/

/* local functions called recursively */
static status_t 
    consume_datadef (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     dlq_hdr_t *que,
		     obj_template_t *parent,
		     grp_template_t *grp);

static status_t 
    consume_case_datadef (tk_chain_t *tkc,
			  ncx_module_t  *mod,
			  dlq_hdr_t *que,
			  obj_template_t *parent);

static status_t 
    consume_refine (tk_chain_t *tkc,
		    ncx_module_t  *mod,
		    dlq_hdr_t *que,
		    obj_template_t *parent);

static status_t 
    consume_augment (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     dlq_hdr_t *que,
		     obj_template_t *parent,
		     grp_template_t *grp);

static status_t 
    expand_augment (tk_chain_t *tkc,
		    ncx_module_t  *mod,
		    obj_template_t *obj,
		    dlq_hdr_t *datadefQ);

/*************    P A R S E   F U N C T I O N S    *************/


/********************************************************************
* FUNCTION finish_config_flag
* 
* Finish the internal settings for the config-stmt
*
* INPUTS:
*   obj == object to process
*
*********************************************************************/
static void
    finish_config_flag (obj_template_t *obj)
{
    boolean flag;

    if (!(obj->flags & OBJ_FL_CONFSET)) {
	if (obj->parent && !obj_is_root(obj->parent)) {
	    flag = obj_get_config_flag(obj->parent);
	    if (flag) {
		obj->flags |= OBJ_FL_CONFIG;
	    } else {
		obj->flags &= ~OBJ_FL_CONFIG;
	    }
	} else if (OBJ_DEF_CONFIG) {
	    obj->flags |= OBJ_FL_CONFIG;
	}
    }

} /* finish_config_flag */


/********************************************************************
* FUNCTION add_object
* 
* Check if an object already exists, and add it if not
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* 'obj' is either deleted or added at the end of this fn
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   obj == object to add
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    add_object (tk_chain_t *tkc,
		ncx_module_t  *mod,
		dlq_hdr_t  *que,
		obj_template_t *obj)
{
    obj_template_t  *testobj;
    const xmlChar   *name;
    yang_stmt_t     *stmt;
    status_t         res;

    res = NO_ERR;
    name = obj_get_name(obj);

    if (que == &mod->datadefQ) {
	testobj = obj_find_template_top(mod, 
					obj_get_mod_name(obj), 
					name);
    } else {
	testobj = obj_find_template_test(que, 
					 obj_get_mod_name(obj), 
					 name);
    }
    if (testobj) {
	if (testobj->mod != mod) {
	    log_error("\nError: object '%s' already defined "
		      "in submodule '%s' at line %u",
		      name, mod->name, testobj->linenum);
	} else {
	    log_error("\nError: object '%s' already defined at line %u",
		      name, testobj->linenum);
	}
	res = ERR_NCX_DUP_ENTRY;
	ncx_print_errormsg(tkc, mod, res);
	obj_free_template(obj);
    } else {
	obj_set_ncx_flags(obj);
	dlq_enque(obj, que);  /* may have some errors */
	if (mod->stmtmode && que==&mod->datadefQ) {
	    /* save top-level object order only */
	    stmt = yang_new_obj_stmt(obj);
	    if (stmt) {
		dlq_enque(stmt, &mod->stmtQ);
	    } else {
		log_error("\nError: malloc failure for obj_stmt");
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
	    }
	}
    }
    return res;

}  /* add_object */


/********************************************************************
* FUNCTION consume_semi_lbrace
* 
* Parse the next token as a semi-colon or a left brace
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'anyxml' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == object in progress
*   done == address of boolean done flag
*
*
OUTPUTS:
*  *done will be set on exit to TRUE or FALSE
*
* RETURNS:
*   status of the operation;
*********************************************************************/
static status_t 
    consume_semi_lbrace (tk_chain_t *tkc,
			 ncx_module_t  *mod,
			 obj_template_t *obj,
			 boolean *done)
{
    const char *expstr;
    status_t res;

    /* Get the starting left brace for the sub-clauses
     * or a semi-colon to end the case-stmt
     */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	*done = TRUE;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
	obj->flags |= OBJ_FL_EMPTY;
	*done = TRUE;
	break;
    case TK_TT_LBRACE:
	*done = FALSE;
	break;
    default:
	res = ERR_NCX_WRONG_TKTYPE;
	expstr = "semi-colon or left brace";
	ncx_mod_exp_err(tkc, mod, res, expstr);
	*done = TRUE;
    }
    return res;

}  /* consume_semi_lbrace */


/********************************************************************
* FUNCTION consume_anyxml
* 
* Parse the next N tokens as an anyxml-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'anyxml' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level anyxml-stmt
*   grp == parent grp_template_t or NULL if not child of grp
*   
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_anyxml (tk_chain_t *tkc,
		    ncx_module_t  *mod,
		    dlq_hdr_t  *que,
		    obj_template_t *parent,
		    grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_leaf_t      *leaf;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, conf, flagset, mand, stat, desc, ref;
    status_t         res, retres;

    obj = NULL;
    leaf = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    conf = FALSE;
    mand = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_LEAF);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;

    leaf = obj->def.leaf;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    if (leaf->typdef) {
	typ_free_typdef(leaf->typdef);
    }
    leaf->typdef = typ_get_basetype_typdef(NCX_BT_ANY);

    /* Get the mandatory anyxml name */
    res = yang_consume_id_string(tkc, mod, &leaf->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the anyxml statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);
	flagset = FALSE;

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_CONFIG)) {
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &conf, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_CONFSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_CONFIG;
	    } else {
		obj->flags &= ~OBJ_FL_CONFIG;
	    }
	} else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &mand, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_MANDSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_MANDATORY;
	    }
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &leaf->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &leaf->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &leaf->ref,
				     &ref, &obj->appinfoQ);

	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (leaf->name && ncx_valid_name2(leaf->name)) {
	res = add_object(tkc, mod, que, obj);
	CHK_EXIT(res, retres);
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_anyxml */


/********************************************************************
* FUNCTION consume_container
* 
* Parse the next N tokens as a container-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'container' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_container (tk_chain_t *tkc,
		       ncx_module_t  *mod,
		       dlq_hdr_t  *que,
		       obj_template_t *parent,
		       grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_container_t *con;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, pres, conf, flagset, stat, desc, ref;
    status_t         res, retres;

    obj = NULL;
    con = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    pres = FALSE;
    conf = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_CONTAINER);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;

    con = obj->def.container;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }
	
    /* Get the mandatory container name */
    res = yang_consume_id_string(tkc, mod, &con->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);
	flagset = FALSE;

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
	    res = yang_typ_consume_typedef(tkc, mod,
					   con->typedefQ);
	} else if (!xml_strcmp(val, YANG_K_GROUPING)) {
	    res = yang_grp_consume_grouping(tkc, mod, 
					    con->groupingQ, obj);
	} else if (!xml_strcmp(val, YANG_K_MUST)) {
	    res = yang_consume_must(tkc, mod, &con->mustQ,
				    &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_PRESENCE)) {
	    res = yang_consume_strclause(tkc, mod, 
					 &con->presence,
					 &pres, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_CONFIG)) {
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &conf, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_CONFSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_CONFIG;
	    } else {
		obj->flags &= ~OBJ_FL_CONFIG;
	    }
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &con->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &con->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &con->ref,
				     &ref, &obj->appinfoQ);
	} else {
	    res = yang_obj_consume_datadef(tkc, mod,
					   con->datadefQ, obj);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (con->name && ncx_valid_name2(con->name)) {
	res = add_object(tkc, mod, que, obj);
	CHK_EXIT(res, retres);
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_container */


/********************************************************************
* FUNCTION consume_leaf
* 
* Parse the next N tokens as a leaf-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'leaf' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == parent grp_template_t or NULL if not child of grp
*   
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_leaf (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  dlq_hdr_t  *que,
		  obj_template_t *parent,
		  grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_leaf_t      *leaf;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, typ, units, def, conf;
    boolean          mand, stat, desc, ref, typeok, flagset;
    status_t         res, retres;

    obj = NULL;
    leaf = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    typ = FALSE;
    units = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    typeok = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_LEAF);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;

    leaf = obj->def.leaf;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    /* Get the mandatory leaf name */
    res = yang_consume_id_string(tkc, mod, &leaf->name);
    CHK_OBJ_EXIT(obj, res, retres);

    /* Get the mandatory left brace */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the leaf statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);
	flagset = FALSE;

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_TYPE)) {
	    if (typ) {
		retres = ERR_NCX_DUP_ENTRY;
		typeok = FALSE;
		ncx_print_errormsg(tkc, mod, retres);

		/* toss the old typdef because this is a fatal
		 * error anyway, and need to skip past the new
		 * typedef; replace the old typedef!
		 */
		typ_clean_typdef(leaf->typdef);
		res = yang_typ_consume_type(tkc, mod, leaf->typdef);
	    } else {
		typ = TRUE;
		res = yang_typ_consume_type(tkc, mod, leaf->typdef);
		if (res == NO_ERR) {
		    typeok = TRUE;
		}
	    }
	} else if (!xml_strcmp(val, YANG_K_UNITS)) {
	    res = yang_consume_strclause(tkc, mod, &leaf->units,
					 &units, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MUST)) {
	    res = yang_consume_must(tkc, mod, &leaf->mustQ,
				    &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DEFAULT)) {
	    res = yang_consume_strclause(tkc, mod, 
					 &leaf->defval,
					 &def, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_CONFIG)) {
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &conf, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_CONFSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_CONFIG;
	    } else {
		obj->flags &= ~OBJ_FL_CONFIG;
	    }
	} else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &mand, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_MANDSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_MANDATORY;
	    }
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &leaf->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &leaf->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &leaf->ref,
				     &ref, &obj->appinfoQ);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* check mandatory params */
    if (!typ) {
	expstr = "mandatory type statement";
	retres = ERR_NCX_DATA_MISSING;
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }

    /* save or delete the obj_template_t struct */
    if (leaf->name && ncx_valid_name2(leaf->name) && typeok) {
	res = add_object(tkc, mod, que, obj);
	CHK_EXIT(res, retres);
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_leaf */


/********************************************************************
* FUNCTION consume_leaflist
* 
* Parse the next N tokens as a leaf-list-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'leaf-list' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_leaflist (tk_chain_t *tkc,
		      ncx_module_t  *mod,
		      dlq_hdr_t  *que,
		      obj_template_t *parent,
		      grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_leaflist_t  *llist;
    const xmlChar   *val;
    const char      *expstr;
    xmlChar         *str;
    tk_type_t        tktyp;
    boolean          done, when, typ, units, conf;
    boolean          minel, maxel, ord, stat, desc, ref, typeok, flagset;
    status_t         res, retres;

    obj = NULL;
    llist = NULL;
    val = NULL;
    expstr = "keyword";
    str = NULL;
    done = FALSE;
    when = FALSE;
    typ = FALSE;
    units = FALSE;
    conf = FALSE;
    minel = FALSE;
    maxel = FALSE;
    ord = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    typeok = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_LEAF_LIST);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;

    llist = obj->def.leaflist;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    /* Get the mandatory leaf-list name */
    res = yang_consume_id_string(tkc, mod, &llist->name);
    CHK_OBJ_EXIT(obj, res, retres);

    /* Get the mandatory left brace */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the leaf-list statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_TYPE)) {
	    if (typ) {
		retres = ERR_NCX_DUP_ENTRY;
		typeok = FALSE;
		ncx_print_errormsg(tkc, mod, retres);
		typ_clean_typdef(llist->typdef);
		res = yang_typ_consume_type(tkc, mod, llist->typdef);
	    } else {
		typ = TRUE;
		res = yang_typ_consume_type(tkc, mod, llist->typdef);
		if (res == NO_ERR) {
		    typeok = TRUE;
		}
	    }
	} else if (!xml_strcmp(val, YANG_K_UNITS)) {
	    res = yang_consume_strclause(tkc, mod, &llist->units,
					 &units, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MUST)) {
	    res = yang_consume_must(tkc, mod, &llist->mustQ,
				    &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_CONFIG)) {
	    flagset = FALSE;
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &conf, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_CONFSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_CONFIG;
	    } else {
		obj->flags &= ~OBJ_FL_CONFIG;
	    }
	} else if (!xml_strcmp(val, YANG_K_MIN_ELEMENTS)) {
	    res = yang_consume_uint32(tkc, mod,
				      &llist->minelems,
				      &minel, &obj->appinfoQ);
	    llist->minset = TRUE;
	} else if (!xml_strcmp(val, YANG_K_MAX_ELEMENTS)) {
	    res = yang_consume_max_elements(tkc, mod,
					    &llist->maxelems,
					    &maxel, &obj->appinfoQ);
	    llist->maxset = TRUE;
	} else if (!xml_strcmp(val, YANG_K_ORDERED_BY)) {
	    res = yang_consume_ordered_by(tkc, mod, 
					  &llist->ordersys,
					  &ord, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &llist->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &llist->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &llist->ref,
				     &ref, &obj->appinfoQ);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* check mandatory params */
    if (!typ) {
	expstr = "mandatory type-stmt";
	retres = ERR_NCX_DATA_MISSING;
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }

    /* save or delete the obj_template_t struct */
    if (llist->name && ncx_valid_name2(llist->name) && typeok) {
	res = add_object(tkc, mod, que, obj);
	CHK_EXIT(res, retres);
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_leaflist */


/********************************************************************
* FUNCTION consume_list
* 
* Parse the next N tokens as a list-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'list' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_list (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  dlq_hdr_t  *que,
		  obj_template_t *parent,
		  grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_list_t      *list;
    obj_unique_t    *objuniq;
    const xmlChar   *val;
    const char      *expstr;
    xmlChar         *str;
    tk_token_t      *savetk;
    tk_type_t        tktyp;
    boolean          done, when, key, conf;
    boolean          minel, maxel, ord, stat, desc, ref, flagset;
    status_t         res, retres;

    obj = NULL;
    list = NULL;
    val = NULL;
    expstr = "keyword";
    str = NULL;
    done = FALSE;
    when = FALSE;
    key = FALSE;
    conf = FALSE;
    minel = FALSE;
    maxel = FALSE;
    ord = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_LIST);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;

    list = obj->def.list;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    /* Get the mandatory list name */
    res = yang_consume_id_string(tkc, mod, &list->name);
    CHK_OBJ_EXIT(obj, res, retres);

    /* Get the mandatory left brace */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the list statements and any appinfo extensions */
    while (!done) {

	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
	    res = yang_typ_consume_typedef(tkc, mod,
					   list->typedefQ);
	} else if (!xml_strcmp(val, YANG_K_GROUPING)) {
	    res = yang_grp_consume_grouping(tkc, mod,
					    list->groupingQ, obj);
	} else if (!xml_strcmp(val, YANG_K_MUST)) {
	    res = yang_consume_must(tkc, mod, &list->mustQ,
				    &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_KEY)) {
	    savetk = TK_CUR(tkc);
	    res = yang_consume_strclause(tkc, mod, &list->keystr,
					 &key, &obj->appinfoQ);
	    if (res == NO_ERR) {
		list->keytk = savetk;
		list->keylinenum = savetk->linenum;
	    }
	} else if (!xml_strcmp(val, YANG_K_UNIQUE)) {
	    objuniq = obj_new_unique();
	    if (!objuniq) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
		obj_free_template(obj);
		return res;
	    }

	    objuniq->tk = TK_CUR(tkc);

	    res = yang_consume_strclause(tkc, mod,
					 &objuniq->xpath,
					 NULL, &obj->appinfoQ);
	    if (res == NO_ERR) {
		dlq_enque(objuniq, list->uniqueQ);
	    } else {
		obj_free_unique(objuniq);
	    }
	} else if (!xml_strcmp(val, YANG_K_CONFIG)) {
	    flagset = FALSE;
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &conf, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_CONFSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_CONFIG;
	    } else {
		obj->flags &= ~OBJ_FL_CONFIG;
	    }
	} else if (!xml_strcmp(val, YANG_K_MIN_ELEMENTS)) {
	    res = yang_consume_uint32(tkc, mod,
				      &list->minelems,
				      &minel, &obj->appinfoQ);
	    list->minset = TRUE;
	} else if (!xml_strcmp(val, YANG_K_MAX_ELEMENTS)) {
	    res = yang_consume_max_elements(tkc, mod,
					    &list->maxelems,
					    &maxel, &obj->appinfoQ);
	    list->maxset = TRUE;
	} else if (!xml_strcmp(val, YANG_K_ORDERED_BY)) {
	    res = yang_consume_ordered_by(tkc, mod,
					  &list->ordersys,
					  &ord, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &list->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &list->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &list->ref,
				     &ref, &obj->appinfoQ);
	} else {
	    res = yang_obj_consume_datadef(tkc, mod,
					   list->datadefQ, obj);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    if (!list->keystr && (obj->flags & OBJ_FL_CONFIG)) {
	log_error("\nError: No key entered for list '%s' on line %u",
		  list->name, obj->linenum);
	retres = ERR_NCX_DATA_MISSING;
	ncx_print_errormsg(tkc, mod, retres);
    }

    if (dlq_empty(list->datadefQ)) {
	expstr = "mandatory data-def statement";
	retres = ERR_NCX_DATA_MISSING;
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }

    /* save or delete the obj_template_t struct */
    if (list->name && ncx_valid_name2(list->name)) {
	res = add_object(tkc, mod, que, obj);
	CHK_EXIT(res, retres);
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_list */


/********************************************************************
* FUNCTION consume_case
* 
* Parse the next N tokens as a case-stmt
* Create and fill in an obj_template_t struct
* and add it to the caseQ for a choice, in progress
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'case' keyword (if withcase==TRUE)
* Current token is a data-def keyword (if withcase==FALSE)
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   choic == obj_choice_t in progress, add case arm to this caseQ
*   caseQ == Que to store the obj_template_t generated
*   parent == the obj_template_t containing the 'choic' param
*             In YANG, a top-level object cannot be a 'choice',
*             so this param should not be NULL
*   withcase == TRUE if a case arm was entered and the normal
*               (full) syntax for a case arm is used
*            == FALSE if the case arm is the shorthand implied
*               kind. The start token is the limited data-def-stmt
*               keyword and only one data-def-stmt will be parsed
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_case (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  dlq_hdr_t *caseQ,
		  obj_template_t *parent,
		  boolean withcase)
{
    obj_case_t      *cas, *testcas;
    obj_template_t  *obj, *testobj, *test2obj, *casobj;
    const xmlChar   *val, *namestr;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, stat, desc, ref, anydone;
    status_t         res, retres;

    obj = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    anydone = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_CASE);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;

    cas = obj->def.cas;

    /* Get the mandatory case name */
    if (withcase) {
	res = yang_consume_id_string(tkc, mod, &cas->name);
	CHK_OBJ_EXIT(obj, res, retres);

	res = consume_semi_lbrace(tkc, mod, obj, &done);
	CHK_OBJ_EXIT(obj, res, retres);
    } else {
	/* shoarthand version, just 1 data-def-stmt per case */
	anydone = TRUE;
	res = consume_case_datadef(tkc, mod,
				   cas->datadefQ, obj);
	CHK_OBJ_EXIT(obj, res, retres);
	done = TRUE;
    }

    /* get the case statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &cas->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &cas->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &cas->ref,
				     &ref, &obj->appinfoQ);
	} else {
	    res = consume_case_datadef(tkc, mod,
				       cas->datadefQ, obj);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* if shorthand version, copy leaf name to case name */
    if (!withcase && retres==NO_ERR) {
	res = NO_ERR;
	testobj = (obj_template_t *)dlq_firstEntry(cas->datadefQ);
	if (testobj) {
	    val = obj_get_name(testobj);
	    if (val) {
		cas->name = xml_strdup(val);
		if (!cas->name) {
		    res = ERR_INTERNAL_MEM;
		}
	    } else {
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }
	} else {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}
    }

    /* check case arm already defined
     * if not, check if any objects in the new case
     * are already defined in a different case
     */
    if (retres == NO_ERR) {
	res = NO_ERR;
	for (casobj = (obj_template_t *)dlq_firstEntry(caseQ);
	     casobj != NULL && res==NO_ERR;
	     casobj = (obj_template_t *)dlq_nextEntry(casobj)) {

	    testcas = casobj->def.cas;

	    if (!xml_strcmp(cas->name, testcas->name)) {
		/* case arm name already used  error */
		res = retres = ERR_NCX_DUP_ENTRY;
		log_error("\nError: case name '%s' already used"
			  " on line %u", testcas->name,
			  casobj->linenum);
		ncx_print_errormsg(tkc, mod, retres);
	    } else {
		/* check object named within case arm already used */
		for (testobj = (obj_template_t *)
			 dlq_firstEntry(cas->datadefQ);
		     testobj != NULL;
		     testobj = (obj_template_t *)
			 dlq_nextEntry(testobj)) {

		    namestr = obj_get_name(testobj);
		    test2obj = 
			obj_find_template_test(testcas->datadefQ,
					       obj_get_mod_name(testobj),
					       namestr);
		    if (test2obj) {
			/* duplicate in another case arm error */
			res = retres = ERR_NCX_DUP_ENTRY;
			log_error("\nError: object name '%s' already used"
				  " in case '%s', on line %u", 
				  namestr, testcas->name, test2obj->linenum);
			ncx_print_errormsg(tkc, mod, retres);
		    } 
		}
	    }
	}
    }
	
    /* save or delete the obj_template_t struct */
    if (res==NO_ERR && cas->name && ncx_valid_name2(cas->name)) {
	obj_set_ncx_flags(obj);
	dlq_enque(obj, caseQ);
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_case */


/********************************************************************
* FUNCTION consume_choice
* 
* Parse the next N tokens as a choice-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'choice' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_choice (tk_chain_t *tkc,
		    ncx_module_t  *mod,
		    dlq_hdr_t  *que,
		    obj_template_t *parent,
		    grp_template_t *grp)
{
    obj_template_t  *obj, *testobj, *test2obj, *casobj;
    obj_choice_t    *choic;
    obj_case_t      *testcas;
    const xmlChar   *val, *namestr;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, def, mand, conf, stat, desc, ref, flagset;
    status_t         res, retres;

    obj = NULL;
    choic = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    def = FALSE;
    mand = FALSE;
    conf = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_CHOICE);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;

    choic = obj->def.choic;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    /* Get the mandatory choice name */
    res = yang_consume_id_string(tkc, mod, &choic->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);
	flagset = FALSE;

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DEFAULT)) {
	    res = yang_consume_strclause(tkc, mod, &choic->defval,
					 &def, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &mand, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_MANDSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_MANDATORY;
	    }
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &choic->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &choic->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &choic->ref,
				     &ref, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_CONFIG)) {
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &conf, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_CONFSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_CONFIG;
	    } else {
		obj->flags &= OBJ_FL_CONFIG;
	    }
	} else if (!xml_strcmp(val, YANG_K_CASE)) {
	    res = consume_case(tkc, mod,
				    choic->caseQ,
				    obj, TRUE);
	} else if (!xml_strcmp(val, YANG_K_ANYXML) ||
		   !xml_strcmp(val, YANG_K_CONTAINER) ||
		   !xml_strcmp(val, YANG_K_LEAF) ||
		   !xml_strcmp(val, YANG_K_LEAF_LIST) ||
		   !xml_strcmp(val, YANG_K_LIST)) {
	    /* create an inline 1-obj case statement */
	    res = consume_case(tkc, mod,
				    choic->caseQ,
				    obj, FALSE);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (choic->name && ncx_valid_name2(choic->name)) {
	res = NO_ERR;

	/* make sure sibling choice is not already defined with same name */
	if (que == &mod->datadefQ) {
	    testobj = obj_find_template_top(mod, 
					    obj_get_mod_name(obj), 
					    choic->name);
	} else {
	    testobj = obj_find_template_test(que, 
					     obj_get_mod_name(obj),
					     choic->name);
	}
	if (testobj) {
	    if (testobj->mod != mod) {
		log_error("\nError: object '%s' already defined "
			  "in submodule '%s' at line %u",
			  choic->name, testobj->mod->name,
			  testobj->linenum);
	    } else {
		log_error("\nError: choice '%s' already defined at line %u",
			  choic->name, testobj->linenum);
	    }
	    res = retres = ERR_NCX_DUP_ENTRY;
	    ncx_print_errormsg(tkc, mod, retres);
	}

	/* since the choice and case nodes do not really exist,
	 * the objects within each case datadefQ must not conflict
	 * with any sibling nodes of the choice itself
	 */
	for (casobj = (obj_template_t *)dlq_firstEntry(choic->caseQ);
	     casobj != NULL && res==NO_ERR;
	     casobj = (obj_template_t *)dlq_nextEntry(casobj)) {

	    testcas = casobj->def.cas;

	    /* check object named within choice sibling objects */
	    for (testobj = (obj_template_t *)
		     dlq_firstEntry(testcas->datadefQ);
		 testobj != NULL;
		 testobj = (obj_template_t *)dlq_nextEntry(testobj)) {
		    
		namestr = obj_get_name(testobj);
		test2obj = 
		    obj_find_template_test(que, 
					   obj_get_mod_name(testobj), 
					   namestr);
		if (test2obj) {
		    /* duplicate in the same Q as the choice */
		    res = retres = ERR_NCX_DUP_ENTRY;
		    log_error("\nError: object name '%s' in case '%s'"
			      " already used in sibling node, on line %u", 
			      namestr, test2obj->linenum);
		    ncx_print_errormsg(tkc, mod, retres);
		} 
	    }
	}

	if (res==NO_ERR) {
	    obj_set_ncx_flags(obj);
	    dlq_enque(obj, que);  /* may have some errors */	    
	} else {
	    obj_free_template(obj);
	}
    } else {
	/* choice name was not valid */
	obj_free_template(obj);
    }

    return retres;

}  /* consume_choice */


/********************************************************************
* FUNCTION consume_refine
* 
* Parse the next N tokens as a refinement-stmt
* Create and fill in an obj_template_t struct
* and add it to the datadefQ for a uses in progress
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'refine' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t struct that get created
*   parent == the obj_template_t containing the 'uses' param
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_refine (tk_chain_t *tkc,
		    ncx_module_t  *mod,
		    dlq_hdr_t *que,
		    obj_template_t *parent)
{
    obj_template_t  *obj;
    obj_refine_t    *refine;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, targ, desc, ref, pres, def;
    boolean          conf, mand, minel, maxel, flagset;
    status_t         res, retres;

    obj = NULL;
    refine = NULL;
    val = NULL;
    expstr = "refine target";
    done = FALSE;
    targ = FALSE;
    desc = FALSE;
    ref = FALSE;
    pres = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    minel = FALSE;
    maxel = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_REFINE);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }
    refine = obj->def.refine;

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;

    /* Get the mandatory refine target */
    res = yang_consume_string(tkc, mod, &refine->target);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);
	flagset = FALSE;

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a token string so check the value */
	if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    refine->descr_tk = TK_CUR(tkc);
	    res = yang_consume_descr(tkc, mod, &refine->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    refine->ref_tk = TK_CUR(tkc);
	    res = yang_consume_descr(tkc, mod, &refine->ref,
				     &ref, &obj->appinfoQ);

	} else if (!xml_strcmp(val, YANG_K_PRESENCE)) {
	    refine->presence_tk = TK_CUR(tkc);
	    res = yang_consume_strclause(tkc, mod, 
					 &refine->presence,
					 &pres, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DEFAULT)) {
	    refine->def_tk = TK_CUR(tkc);
	    res = yang_consume_strclause(tkc, mod, 
					 &refine->def,
					 &def, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_CONFIG)) {
	    refine->config_tk = TK_CUR(tkc);
	    res = yang_consume_boolean(tkc, mod, &flagset,
				       &conf, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_CONFSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_CONFIG;
	    } else {
		obj->flags &= ~OBJ_FL_CONFIG;
	    }
	} else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
	    refine->mandatory_tk = TK_CUR(tkc);
	    res = yang_consume_boolean(tkc, mod,
				       &flagset,
				       &mand, &obj->appinfoQ);
	    obj->flags |= OBJ_FL_MANDSET;
	    if (flagset) {
		obj->flags |= OBJ_FL_MANDATORY;
	    }
	} else if (!xml_strcmp(val, YANG_K_MIN_ELEMENTS)) {
	    refine->minelems_tk = TK_CUR(tkc);
	    res = yang_consume_uint32(tkc, mod,
				      &refine->minelems,
				      &minel, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MAX_ELEMENTS)) {
	    refine->maxelems_tk = TK_CUR(tkc);
	    res = yang_consume_max_elements(tkc, mod,
					    &refine->maxelems,
					    &maxel, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MUST)) {
	    res = yang_consume_must(tkc, mod, &refine->mustQ,
				    &obj->appinfoQ);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (refine->target) {
	res = add_object(tkc, mod, que, obj);
	CHK_EXIT(res, retres);
    } else {
	obj_free_template(obj);
    }

    return res;

}  /* consume_refine */


/********************************************************************
* FUNCTION consume_uses
* 
* Parse the next N tokens as a uses-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'uses' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_uses (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  dlq_hdr_t  *que,
		  obj_template_t *parent,
		  grp_template_t *grp)
{
    obj_template_t  *obj, *testobj;
    obj_uses_t      *uses;
    grp_template_t  *impgrp;
    const xmlChar   *val;
    const char      *expstr;
    xmlChar         *str;
    yang_stmt_t     *stmt;
    tk_type_t        tktyp;
    boolean          done, when, stat, desc, ref;
    status_t         res, retres;

    obj = NULL;
    uses = NULL;
    val = NULL;
    expstr = "keyword";
    str = NULL;
    done = FALSE;
    when = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_USES);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    uses = obj->def.uses;

    /* Get the mandatory uses target [prefix:]name */
    res = yang_consume_pid_string(tkc, mod,
				  &uses->prefix,
				  &uses->name);
    CHK_OBJ_EXIT(obj, res, retres);

    /* attempt to find grouping only if it is from another module */
    if (uses->prefix && xml_strcmp(uses->prefix, mod->prefix)) {
	res = yang_find_imp_grouping(tkc, mod, uses->prefix,
				     uses->name, obj->tk, &impgrp);
	CHK_OBJ_EXIT(obj, res, retres);
	uses->grp = impgrp;
    }

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    expstr = "uses sub-statement";

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &uses->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &uses->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &uses->ref,
				     &ref, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_AUGMENT)) {
	    res = consume_augment(tkc, mod, uses->datadefQ,
				       obj, NULL);
	} else if (!xml_strcmp(val, YANG_K_REFINE)) {
	    res = consume_refine(tkc, mod, uses->datadefQ, obj);
	} else {
	   res = ERR_NCX_WRONG_TKVAL;
	   ncx_mod_exp_err(tkc, mod, res, expstr);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (uses->name && ncx_valid_name2(uses->name)) {
	testobj = obj_find_template_test(que, 
					 obj_get_mod_name(obj),
					 uses->name);
	if (testobj) {
	    log_error("\nError: object '%s' already defined at line %u",
		      uses->name, testobj->linenum);
	    retres = ERR_NCX_DUP_ENTRY;
	    ncx_print_errormsg(tkc, mod, retres);
	    obj_free_template(obj);
	} else {
	    obj_set_ncx_flags(obj);
	    dlq_enque(obj, que);  /* may have some errors */
	    if (mod->stmtmode && que==&mod->datadefQ) {
		/* save top-level object order only */
		stmt = yang_new_obj_stmt(obj);
		if (stmt) {
		    dlq_enque(stmt, &mod->stmtQ);
		} else {
		    log_error("\nError: malloc failure for obj_stmt");
		    res = ERR_INTERNAL_MEM;
		    ncx_print_errormsg(tkc, mod, res);
		}
	    }
	}
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_uses */


/********************************************************************
* FUNCTION consume_rpcio
* 
* Parse the next N tokens as an input-stmt or output-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'input' or 'output' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent RPC object
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_rpcio (tk_chain_t *tkc,
		   ncx_module_t  *mod,
		   dlq_hdr_t  *que,
		   obj_template_t *parent)
{
    obj_template_t  *obj, *testobj;
    obj_rpcio_t     *rpcio;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, anydone;
    status_t         res, retres;

    obj = NULL;
    rpcio = NULL;
    val = NULL;
    expstr = "typedef, grouping, or data-def keyword";
    done = FALSE;
    anydone = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_RPCIO);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;

    rpcio = obj->def.rpcio;
	
    /* Get the mandatory RPC method name */
    rpcio->name = xml_strdup(TK_CUR_VAL(tkc));
    if (!rpcio->name) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	obj_free_template(obj);
	return res;
    }

    /* Get the starting left brace for the sub-clauses */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a token string so check the value */
	if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
	    res = yang_typ_consume_typedef(tkc, mod, &rpcio->typedefQ);
	} else if (!xml_strcmp(val, YANG_K_GROUPING)) {
	    res = yang_grp_consume_grouping(tkc, mod, 
					    &rpcio->groupingQ, obj);
	} else {
	    res = yang_obj_consume_datadef(tkc, mod,
					   &rpcio->datadefQ, obj);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    testobj = obj_find_template_test(que, 
				     obj_get_mod_name(obj),
				     rpcio->name);
    if (testobj) {
	log_error("\nError: '%s' statement already defined at line %u",
		  rpcio->name, testobj->linenum);
	retres = ERR_NCX_DUP_ENTRY;
	ncx_print_errormsg(tkc, mod, retres);
	obj_free_template(obj);
    } else {
	obj_set_ncx_flags(obj);
	dlq_enque(obj, que);  /* may have some errors */
    }

    return retres;

}  /* consume_rpcio */


/********************************************************************
* FUNCTION consume_augdata
* 
* Parse the next N tokens as a case-stmt
* Create and fill in an obj_template_t struct
* and add it to the datadefQ for the augment in progress
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the starting keyword of an object
* that can be refined in a uses statement:
*
*   container
*   leaf
*   leaf-list
*   list
*   choice
*   uses
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t struct that get created
*   parent == the obj_template_t containing the 'augment' param
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_augdata (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     dlq_hdr_t *que,
		     obj_template_t *parent)
{
    const xmlChar   *val;
    tk_type_t        tktyp;
    status_t         res;
    boolean          errdone;

    errdone = TRUE;
    res = NO_ERR;
    tktyp = TK_CUR_TYP(tkc);
    val = TK_CUR_VAL(tkc);

    /* check the current token type */
    if (tktyp != TK_TT_TSTRING) {
	errdone = FALSE;
	res = ERR_NCX_WRONG_TKTYPE;
    } else {
	/* Got a token string so check the value */
	if (!xml_strcmp(val, YANG_K_ANYXML)) {
	    res = consume_anyxml(tkc, mod, que, parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_CONTAINER)) {
	    res = consume_container(tkc, mod, que, parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_LEAF)) {
	    res = consume_leaf(tkc, mod, que, parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_LEAF_LIST)) {
	    res = consume_leaflist(tkc, mod, que, parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_LIST)) {
	    res = consume_list(tkc, mod, que, parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_CHOICE)) {
	    res = consume_choice(tkc, mod, que, parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_USES)) {
	    res = consume_uses(tkc, mod, que, parent, NULL);
	} else {
	    errdone = FALSE;
	    res = ERR_NCX_WRONG_TKVAL;
	}
    }

    if (res != NO_ERR && !errdone) {
	ncx_mod_exp_err(tkc, mod, res,
			"container, leaf, leaf-list, list, "
			"choice, or uses keyword");
    }

    return res;

}  /* consume_augdata */


/********************************************************************
* FUNCTION consume_augment
* 
* Parse the next N tokens as an augment-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'augment' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level augment
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_augment (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     dlq_hdr_t *que,
		     obj_template_t *parent,
		     grp_template_t *grp)
{

    obj_template_t  *obj;
    obj_augment_t   *aug;
    const xmlChar   *val;
    const char      *expstr;
    xmlChar         *str;
    yang_stmt_t     *stmt;
    tk_type_t        tktyp;
    boolean          done, when, stat, desc, ref;
    boolean          rpcin, rpcout;
    status_t         res, retres;

    obj = NULL;
    aug = NULL;
    val = NULL;
    expstr = "keyword";
    str = NULL;
    done = FALSE;
    when = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    rpcin = FALSE;
    rpcout = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_AUGMENT);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    aug = obj->def.augment;

    /* Get the mandatory augment target */
    res = yang_consume_string(tkc, mod, &aug->target);
    CHK_OBJ_EXIT(obj, res, retres);

    /* Get the semi-colon or starting left brace for the sub-clauses */
    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_WHEN)) {
	    res = yang_consume_when(tkc, mod, obj, &when);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &aug->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &aug->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &aug->ref,
				     &ref, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_CASE)) {
	    res = consume_case(tkc, mod, &aug->datadefQ,
				    obj, TRUE);
	} else {
	    res = consume_augdata(tkc, mod, &aug->datadefQ, obj);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (aug->target) {
	obj_set_ncx_flags(obj);
	dlq_enque(obj, que);
	if (mod->stmtmode && que==&mod->datadefQ) {
	    /* save top-level object order only */
	    stmt = yang_new_obj_stmt(obj);
	    if (stmt) {
		dlq_enque(stmt, &mod->stmtQ);
	    } else {
		log_error("\nError: malloc failure for obj_stmt");
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
	    }
	}
    } else {
	obj_free_template(obj);
    }

    return retres;

} /* consume_augment */


/********************************************************************
* FUNCTION consume_case_datadef
* 
* Parse the next N tokens as a case-data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_case_datadef (tk_chain_t *tkc,
			  ncx_module_t  *mod,
			  dlq_hdr_t *que,
			  obj_template_t *parent)
{
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          errdone;
    status_t         res;

    expstr = "container, leaf, leaf-list, list, uses,"
	"or augment keyword";
    errdone = TRUE;
    res = NO_ERR;
    tktyp = TK_CUR_TYP(tkc);
    val = TK_CUR_VAL(tkc);

    /* check the current token type */
    if (tktyp != TK_TT_TSTRING) {
	res = ERR_NCX_WRONG_TKTYPE;
	errdone = FALSE;
    } else {
	/* Got a token string so check the value */
	if (!xml_strcmp(val, YANG_K_ANYXML)) {
	    res = consume_anyxml(tkc, mod, que,
				      parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_CONTAINER)) {
	    res = consume_container(tkc, mod, que,
					 parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_LEAF)) {
	    res = consume_leaf(tkc, mod, que,
				    parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_LEAF_LIST)) {
	    res = consume_leaflist(tkc, mod, que,
					parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_LIST)) {
	    res = consume_list(tkc, mod, que,
				    parent, NULL);
	} else if (!xml_strcmp(val, YANG_K_USES)) {
	    res = consume_uses(tkc, mod, que, parent, NULL);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    errdone = FALSE;
	}
    }

    if (res != NO_ERR && !errdone) {
	ncx_mod_exp_err(tkc, mod, res, expstr);
    }

    return res;

}  /* consume_case_datadef */


/********************************************************************
* FUNCTION consume_rpc
* 
* Parse the next N tokens as an rpc-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'rpc' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_rpc (tk_chain_t *tkc,
		 ncx_module_t  *mod,
		 dlq_hdr_t  *que,
		 obj_template_t *parent,
		 grp_template_t *grp)
{
    obj_template_t        *obj, *chobj;
    const obj_template_t  *testobj;
    obj_rpc_t             *rpc;
    const xmlChar         *val;
    const char            *expstr;
    tk_type_t              tktyp;
    boolean                done, stat, desc, ref;
    status_t               res, retres;

    obj = NULL;
    rpc = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_RPC);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    rpc = obj->def.rpc;
	
    /* Get the mandatory RPC method name */
    res = yang_consume_id_string(tkc, mod, &rpc->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a token string so check the value */
	if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
	    res = yang_typ_consume_typedef(tkc, mod, &rpc->typedefQ);
	} else if (!xml_strcmp(val, YANG_K_GROUPING)) {
	    res = yang_grp_consume_grouping(tkc, mod, 
					    &rpc->groupingQ, obj);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &rpc->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &rpc->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &rpc->ref,
				     &ref, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_INPUT) ||
		   !xml_strcmp(val, YANG_K_OUTPUT)) {
	    res = consume_rpcio(tkc, mod, &rpc->datadefQ, obj);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (rpc->name && ncx_valid_name2(rpc->name)) {

	/* make sure the rpc node has an input and output node
	 * for augment purposes
	 */
	testobj = obj_find_child(obj, NULL, YANG_K_INPUT);
	if (!testobj) {
	    chobj = obj_new_template(OBJ_TYP_RPCIO);
	    if (!chobj) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
		obj_free_template(obj);
		return res;
	    }

	    chobj->mod = mod;
	    chobj->tk = obj->tk;
	    chobj->linenum = obj->linenum;
	    chobj->parent = obj;

	    chobj->def.rpcio->name = xml_strdup(YANG_K_INPUT);
	    if (!chobj->def.rpcio->name) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
		obj_free_template(chobj);
		obj_free_template(obj);
		return res;
	    }
	    obj_set_ncx_flags(chobj);
	    dlq_enque(chobj, &obj->def.rpc->datadefQ);
	}

	testobj = obj_find_child(obj, NULL, YANG_K_OUTPUT);
	if (!testobj) {
	    chobj = obj_new_template(OBJ_TYP_RPCIO);
	    if (!chobj) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
		obj_free_template(obj);
		return res;
	    }

	    chobj->mod = mod;
	    chobj->tk = obj->tk;
	    chobj->linenum = obj->linenum;
	    chobj->parent = obj;

	    chobj->def.rpcio->name = xml_strdup(YANG_K_OUTPUT);
	    if (!chobj->def.rpcio->name) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
		obj_free_template(chobj);
		obj_free_template(obj);
		return res;
	    }
	    obj_set_ncx_flags(chobj);
	    dlq_enque(chobj, &obj->def.rpc->datadefQ);
	}
	
	res = add_object(tkc, mod, que, obj);
	CHK_EXIT(res, retres);
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_rpc */


/********************************************************************
* FUNCTION consume_notif
* 
* Parse the next N tokens as a notification-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'notification' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_notif (tk_chain_t *tkc,
		   ncx_module_t  *mod,
		   dlq_hdr_t  *que,
		   obj_template_t *parent,
		   grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_notif_t     *notif;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, stat, desc, ref;
    status_t         res, retres;

    obj = NULL;
    notif = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_NOTIF);
    if (!obj) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    obj->mod = mod;
    obj->tk = TK_CUR(tkc);
    obj->linenum = obj->tk->linenum;
    obj->parent = parent;
    obj->grp = grp;
    if (que == &mod->datadefQ) {
	obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    notif = obj->def.notif;
	
    /* Get the mandatory RPC method name */
    res = yang_consume_id_string(tkc, mod, &notif->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_template(obj);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
	    CHK_OBJ_EXIT(obj, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a token string so check the value */
	if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
	    res = yang_typ_consume_typedef(tkc, mod, &notif->typedefQ);
	} else if (!xml_strcmp(val, YANG_K_GROUPING)) {
	    res = yang_grp_consume_grouping(tkc, mod, 
					    &notif->groupingQ, obj);
	} else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &obj->iffeatureQ,
					 &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &notif->status,
				      &stat, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &notif->descr,
				     &desc, &obj->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &notif->ref,
				     &ref, &obj->appinfoQ);
	} else {
	    res = consume_datadef(tkc, mod, &notif->datadefQ,
				  obj, NULL);
	}
	CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (notif->name && ncx_valid_name2(notif->name)) {
	res = add_object(tkc, mod, que, obj);
	CHK_EXIT(res, retres);
    } else {
	obj_free_template(obj);
    }

    return retres;

}  /* consume_notif */


/********************************************************************
* FUNCTION consume_deviate
* 
* Parse the next N tokens as a deviate-stmt
* Create a obj_deviate_t struct and add it to the 
* specified deviation->deviateQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'deviate' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   deviation == parent obj_deviation_t to hold the new obj_deviate_t
*                created by this function
*
* OUTPUTS:
*   new deviate struct added to deviation deviateQ
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_deviate (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     obj_deviation_t *deviation)
{
    obj_deviate_t    *devi;
    obj_unique_t     *uniq;
    typ_def_t        *dummy;
    const xmlChar    *val;
    const char       *expstr;
    xmlChar          *str;
    tk_type_t         tktyp;
    boolean           done, type, units, def, conf;
    boolean           mand, minel, maxel;
    status_t          res, retres;

    /* Get a new obj_deviation_t to fill in */
    devi = obj_new_deviate();
    if (!devi) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    val = NULL;
    expstr = "add, replace, delete, or not-supported";
    str = NULL;
    done = FALSE;
    type = FALSE;
    units = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    minel = FALSE;
    maxel = FALSE;
    retres = NO_ERR;

    devi->tk = TK_CUR(tkc);
    devi->linenum = devi->tk->linenum;

    /* Get the mandatory deviation argument */
    res = yang_consume_string(tkc, mod, &str);
    CHK_DEVI_EXIT(devi, res, retres);

    if (res == NO_ERR) {
	/* check the value */
	if (!xml_strcmp(str, YANG_K_ADD)) {
	    devi->arg = OBJ_DARG_ADD;
	} else if (!xml_strcmp(str, YANG_K_DELETE)) {
	    devi->arg = OBJ_DARG_DELETE;
	} else if (!xml_strcmp(str, YANG_K_REPLACE)) {
	    devi->arg = OBJ_DARG_REPLACE;
	} else if (!xml_strcmp(str, YANG_K_NOT_SUPPORTED)) {
	    devi->arg = OBJ_DARG_NOT_SUPPORTED;
	} else {
	    log_error("\nError: invalid deviate-stmt "
		      "argument '%s'", str);
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	}
    }

    if (str) {
	m__free(str);
	str = NULL;
    }

    /* Get the starting left brace or semi-colon */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	obj_free_deviate(devi);
	return res;
    }

    /* check for semi-colon or left brace */
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
	/* only 1 arg type allowed to be empty */
	if (devi->arg == OBJ_DARG_NOT_SUPPORTED) {
	    devi->empty = TRUE;
	} else {
	    retres = ERR_NCX_WRONG_TKTYPE;
	    expstr = "left brace";
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	}
	done = TRUE;
	break;
    case TK_TT_LBRACE:
	done = FALSE;
	break;
    default:
	retres = ERR_NCX_WRONG_TKTYPE;
	expstr = "semi-colon or left brace";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
	done = TRUE;
    }

    expstr = "deviate-stmt sub-clause";

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_deviate(devi);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_deviate(devi);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &devi->appinfoQ);
	    CHK_DEVI_EXIT(devi, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_TYPE)) {
	    devi->type_tk = TK_CUR(tkc);

	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: type-stmt cannot be added");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_DELETE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: type-stmt cannot be deleted");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_REPLACE:
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    if (type) {
		log_error("\nError: type-stmt already entered");
		retres = ERR_NCX_ENTRY_EXISTS;		
		dummy = typ_new_typdef();
		if (!dummy) {
		    res = ERR_INTERNAL_MEM;
		    ncx_print_errormsg(tkc, mod, res);
		    obj_free_deviate(devi);
		    return res;
		} else {
		    res = yang_typ_consume_type(tkc, 
						mod, 
						dummy);
		    typ_free_typdef(dummy);
		}
	    } else {

		type = TRUE;
		devi->typdef = typ_new_typdef();
		if (!devi->typdef) {
		    res = ERR_INTERNAL_MEM;
		    ncx_print_errormsg(tkc, mod, res);
		    obj_free_deviate(devi);
		    return res;
		} else {
		    res = yang_typ_consume_type(tkc, 
						mod, 
						devi->typdef);
		}
	    }
	} else if (!xml_strcmp(val, YANG_K_UNITS)) {
	    devi->units_tk = TK_CUR(tkc);

	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		break;
	    case OBJ_DARG_DELETE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: units-stmt cannot be deleted");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_REPLACE:
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    res = yang_consume_strclause(tkc, mod, &devi->units,
					 &units, &devi->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DEFAULT)) {
	    devi->default_tk = TK_CUR(tkc);

	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		break;
	    case OBJ_DARG_DELETE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: default-stmt cannot be deleted");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_REPLACE:
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    res = yang_consume_strclause(tkc, mod, 
					 &devi->defval,
					 &def, &devi->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_CONFIG)) {
	    devi->config_tk = TK_CUR(tkc);

	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		break;
	    case OBJ_DARG_DELETE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: config-stmt cannot be deleted");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_REPLACE:
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    res = yang_consume_boolean(tkc, mod,
				       &devi->config,
				       &conf, &devi->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
	    devi->mandatory_tk = TK_CUR(tkc);

	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		break;
	    case OBJ_DARG_DELETE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: mandatory-stmt cannot be deleted");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_REPLACE:
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    res = yang_consume_boolean(tkc, mod,
				       &devi->mandatory,
				       &mand, &devi->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MIN_ELEMENTS)) {
	    devi->minelems_tk = TK_CUR(tkc);

	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		break;
	    case OBJ_DARG_DELETE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: min-elements-stmt cannot be deleted");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_REPLACE:
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    res = yang_consume_uint32(tkc, mod,
				      &devi->minelems,
				      &minel, &devi->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MAX_ELEMENTS)) {
	    devi->maxelems_tk = TK_CUR(tkc);

	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		break;
	    case OBJ_DARG_DELETE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: max-elements-stmt cannot be deleted");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_REPLACE:
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    res = yang_consume_max_elements(tkc, mod,
					    &devi->maxelems,
					    &maxel, &devi->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_MUST)) {
	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		break;
	    case OBJ_DARG_DELETE:
		break;
	    case OBJ_DARG_REPLACE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: must-stmt cannot be replaced");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    res = yang_consume_must(tkc, mod, &devi->mustQ,
				    &devi->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_UNIQUE)) {
	    uniq = obj_new_unique();
	    if (!uniq) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
		obj_free_deviate(devi);
		return res;
	    }

	    uniq->tk = TK_CUR(tkc);

	    switch (devi->arg) {
	    case OBJ_DARG_NONE:
		/* argument was invalid so cannot check it further */
		break;
	    case OBJ_DARG_ADD:
		break;
	    case OBJ_DARG_DELETE:
		break;
	    case OBJ_DARG_REPLACE:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: unique-stmt cannot be replaced");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_DARG_NOT_SUPPORTED:
		retres = ERR_NCX_INVALID_DEV_STMT;
		log_error("\nError: sub-clauses not allowed "
			  "for 'not-supported'");
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    default:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    res = yang_consume_strclause(tkc, mod,
					 &uniq->xpath,
					 NULL, &devi->appinfoQ);
	    CHK_DEVI_EXIT(devi, res, retres);
	    if (res == NO_ERR) {
		dlq_enque(uniq, &devi->uniqueQ);
	    } else {
		obj_free_unique(uniq);
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	}
	CHK_DEVI_EXIT(devi, res, retres);
    }

    devi->res = retres;
    dlq_enque(devi, &deviation->deviateQ);

    return retres;

}  /* consume_deviate */


/************   R E S O L V E    F U N C T I O N S   ***************/


/********************************************************************
* FUNCTION check_parent
* 
* Check the node against its parent
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == object to check, only if obj != NULL
*          and obj->parent != NULL
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_parent (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  obj_template_t *obj)
{
    status_t      res;
    ncx_status_t  stat, parentstat;
    boolean       conf, parentconf;

    res = NO_ERR;

    /* check status stmt against the parent, if any */
    if (obj && obj->parent && !obj_is_root(obj->parent)) {
	if (!obj_is_refine(obj)) {
	    stat = obj_get_status(obj);
	    parentstat = obj_get_status(obj->parent);

	    /* check invalid status warning */
	    if (stat < parentstat) {
		log_warn("\nWarning: Invalid status: "
			 "child node '%s' = '%s' and"
			 " parent node '%s' = '%s'",
			 obj_get_name(obj),
			 ncx_get_status_string(stat),
			 obj_get_name(obj->parent),
			 ncx_get_status_string(parentstat));
		tkc->cur = obj->tk;
		ncx_print_errormsg(tkc, mod, ERR_NCX_INVALID_STATUS);
	    }
	}

	/* check invalid config flag error for real object only */
	if (obj->objtype <= OBJ_TYP_CASE &&
	    obj->parent->objtype <= OBJ_TYP_CASE) {
	    conf = obj_get_config_flag(obj);
	    parentconf = obj_get_config_flag(obj->parent);
	    if (!parentconf && conf) {
		if (obj_is_data(obj)) {
		    log_error("\nError: Node '%s' is marked as configuration, "
			      "but parent node '%s' is not",
			      obj_get_name(obj),
			      obj_get_name(obj->parent));
		    tkc->cur = obj->tk;
		    res = ERR_NCX_INVALID_VALUE;
		    ncx_print_errormsg(tkc, mod, res);
		} else {
		    log_info("\nInfo: Non-data node '%s' "
			     "is marked as configuration : statement ignored",
			     obj_get_name(obj));
		    tkc->cur = obj->tk;
		    res = ERR_NCX_STMT_IGNORED;
		    ncx_print_errormsg(tkc, mod, res);
		}
	    }
	}
    }

    return res;

}  /* check_parent */


/********************************************************************
* FUNCTION resolve_default_parm
* 
* Check the rpc input or container object type
* to see if a CLI default-parm was defined.  If so,
* find the target object.
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == parent object for 'rpcio' or 'list'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_default_parm (tk_chain_t *tkc,
		   ncx_module_t  *mod,
		   obj_template_t *obj)
{
    const obj_template_t *targobj;
    const ncx_appinfo_t  *appinfo;
    status_t              res;

    res = NO_ERR;

    if (obj->objtype == OBJ_TYP_CONTAINER ||
	(obj->objtype == OBJ_TYP_RPCIO &&
	 !xml_strcmp(obj_get_name(obj), YANG_K_INPUT))) {

	appinfo = ncx_find_appinfo(&obj->appinfoQ,
				   NCX_PREFIX,
				   NCX_EL_DEFAULT_PARM);
	if (appinfo) {
	    if (appinfo->value) {
		targobj = obj_find_child(obj,
					 obj_get_mod_name(obj),
					 appinfo->value);
		if (targobj) {
		    if (obj->objtype == OBJ_TYP_CONTAINER) {
			obj->def.container->defaultparm = targobj;
		    } else {
			obj->def.rpcio->defaultparm = targobj;
		    }
		} else {
		    res = ERR_NCX_UNKNOWN_OBJECT;
		}
	    } else {
		res = ERR_NCX_MISSING_PARM;
	    }

	    if (res != NO_ERR) {
		log_error("\nError: invalid 'default-parm' extension");
		ncx_print_errormsg(tkc, mod, res);
	    }
	}
    }

    return res;
				    
}  /* resolve_default_parm */


/********************************************************************
* FUNCTION resolve_mustQ
* 
* Check any must-stmts for this node
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == module in progress containing obj
*   obj == object to check (from the cooked module,
*                           not from any grouping or augment)
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_mustQ (ncx_module_t  *mod,
		   obj_template_t *obj)
{
    xpath_pcb_t    *must;
    dlq_hdr_t      *mustQ;
    status_t        res, retres;

    mustQ = obj_get_mustQ(obj);
    if (!mustQ) {
	return NO_ERR;
    }

    retres = NO_ERR;
    for (must = (xpath_pcb_t *)dlq_firstEntry(mustQ);
	 must != NULL;
	 must = (xpath_pcb_t *)dlq_nextEntry(must)) {

	if (must->parseres != NO_ERR) {
	    /* some errors already reported so do not
	     * duplicate messages; just skip 2nd pass
	     */
	    continue;
	}

	res = xpath1_validate_expr(mod, obj, must);
	CHK_EXIT(res, retres);
    }
    return retres;

}  /* resolve_mustQ */


/********************************************************************
* FUNCTION resolve_when
* 
* Check any when-stmt for this node
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == module in progress containing obj
*   when == XPath control block to use
*   obj == object to check (from the cooked module,
*                           not from any grouping or augment)
*
* OUTPUTS:
*   pcb->validateres is set
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_when (ncx_module_t  *mod,
		  xpath_pcb_t *when,
		  obj_template_t *obj)
{
    if (when->parseres != NO_ERR) {
	/* some errors already reported so do not
	 * duplicate messages; just skip 2nd pass
	 */
	return NO_ERR;
    }

    return xpath1_validate_expr(mod, obj, when);

}  /* resolve_when */


/********************************************************************
* FUNCTION resolve_metadata
* 
* Check the object for ncx:metadata definitions
* Convert any clauses to metadata nodes within
* the the object struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == object to check for ncx:metadata clauses
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_metadata (tk_chain_t *tkc,
		      ncx_module_t  *mod,
		      obj_template_t *obj)
{
    const dlq_hdr_t      *que;
    const ncx_appinfo_t  *appinfo;
    tk_chain_t           *newchain;
    obj_metadata_t       *meta;
    status_t              res, retres;

    retres = NO_ERR;
    
    que = obj_get_appinfoQ(obj);
    if (!que) {
	return NO_ERR;
    }

    for (appinfo = 
	     ncx_find_appinfo(que, NCX_PREFIX, NCX_EL_METADATA);
	 appinfo != NULL;
	 appinfo = ncx_find_next_appinfo(appinfo, NCX_PREFIX,
					 NCX_EL_METADATA)) {

	/* parse the value string into 2 or 3 fields */
	newchain = NULL;
	meta = NULL;
	res = NO_ERR;

	/* turn the string into a Q of tokens */
	newchain = tk_tokenize_metadata_string(mod,
					       appinfo->value,
					       &res);
	if (res != NO_ERR) {
	    log_error("\nError: Invalid metadata value string");
	} else {
	    meta = obj_new_metadata();
	    if (!meta) {
		res = ERR_INTERNAL_MEM;
	    } else {
		/* check the tokens that are in the chain for
		 * a YANG QName for the datatype and a YANG
		 * identifier for the XML attribute name
		 */
		res = yang_typ_consume_metadata_type(newchain, mod, 
						     meta->typdef);
		if (res == NO_ERR) {
		    /* make sure type OK for XML attribute */
		    if (!typ_ok_for_metadata
			(typ_get_basetype(meta->typdef))) {
			log_error("\nError: Builtin type %s not "
				  "allowed for metadata in object %s",
				  tk_get_btype_sym
				  (typ_get_basetype(meta->typdef)),
				  obj_get_name(obj));
			res = ERR_NCX_WRONG_TYPE;
		    }
		}

		if (res == NO_ERR) {
		    /* got a type for the attribute
		     * now need to get a valid name
		     */
		    res = yang_consume_id_string(newchain,
						 mod,
						 &meta->name);
		    if (res == NO_ERR) {
			/* check if the name clashes
			 * with any standard attributes
			 */
			if (!xml_strcmp(meta->name, 
					NC_OPERATION_ATTR_NAME)) {
			    log_warn("\nWarning: metadata using "
				     "reserved name 'operation' "
				     "for object %s",
				     obj_get_name(obj));
			} else if (!xml_strcmp(meta->name, 
					       YANG_K_KEY)) {
			    log_warn("\nWarning: metadata using "
				     "reserved name 'key' "
				     "for object %s",
				     obj_get_name(obj));
			} else if (!xml_strcmp(meta->name, 
					       YANG_K_INSERT)) {
			    log_warn("\nWarning: metadata using "
				     "reserved name 'insert' "
				     "for object %s",
				     obj_get_name(obj));
			} else if (!xml_strcmp(meta->name, 
					       YANG_K_VALUE)) {
			    log_warn("\nWarning: metadata using "
				     "reserved name 'value' "
				     "for object %s",
				     obj_get_name(obj));
			}

			/* save the metadata even if the name clashes
			 * because it is supposed to be used
			 * with a namespace;  However, the
			 * standard attribbutes are often used
			 * without any prefix
			 */
			res = obj_add_metadata(meta, obj);
		    }
		}
	    }
	}

	if (res != NO_ERR) {
	    log_error("\nError: Invalid ncx:metadata string");
	    res = ERR_NCX_INVALID_VALUE;
	    tkc->cur = appinfo->tk;
	    ncx_print_errormsg(tkc, mod, res);
	    retres = res;
	}

	if (newchain) {
	    tk_free_chain(newchain);
	}

	if (res != NO_ERR && meta) {
	    obj_free_metadata(meta);
	}
    }

    return retres;


} /* resolve_metadata */


/********************************************************************
* FUNCTION resolve_container
* 
* Check the container object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   con == obj_container_t to check
*   obj == parent object for 'con'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_container (tk_chain_t *tkc,
		       ncx_module_t  *mod,
		       obj_container_t *con,
		       obj_template_t *obj)
{
    status_t              res, retres;

    retres = NO_ERR;

    res = resolve_metadata(tkc, mod, obj);
    CHK_EXIT(res, retres);

    if (!obj_is_refine(obj)) {
	res = yang_typ_resolve_typedefs(tkc, mod, con->typedefQ, obj);
	CHK_EXIT(res, retres);

	res = yang_grp_resolve_groupings(tkc, mod, con->groupingQ, obj);
	CHK_EXIT(res, retres);
    }

    finish_config_flag(obj);

    res = yang_obj_resolve_datadefs(tkc, mod, con->datadefQ);
    CHK_EXIT(res, retres);

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    return retres;
				    
}  /* resolve_container */


/********************************************************************
* FUNCTION resolve_leaf
* 
* Check the leaf object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   leaf == obj_leaf_t to check
*   obj == parent object for 'leaf'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_leaf (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  obj_leaf_t *leaf,
		  obj_template_t *obj)
{
    status_t res, retres;

    retres = NO_ERR;

    res = resolve_metadata(tkc, mod, obj);
    CHK_EXIT(res, retres);

    if (!obj_is_refine(obj)) {
	res = yang_typ_resolve_type(tkc, mod, leaf->typdef,
				    leaf->defval, obj);
	if (res == NO_ERR) {
	    obj_set_xpath_flags(obj);
	}
	CHK_EXIT(res, retres);
    } 

    finish_config_flag(obj);

    if ((obj->flags & OBJ_FL_MANDATORY) && leaf->defval) {
	log_error("\nError: both mandatory and default statements present"
		  "'%s'", obj_get_name(obj));
	retres = ERR_NCX_INVALID_VALUE;
	tkc->cur = obj->tk;
	ncx_print_errormsg(tkc, mod, retres);
    }

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    return retres;
				    
}  /* resolve_leaf */


/********************************************************************
* FUNCTION resolve_leaflist
* 
* Check the leaf-list object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   llist == obj_leaflist_t to check
*   obj == parent object for 'llist'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_leaflist (tk_chain_t *tkc,
		      ncx_module_t  *mod,
		      obj_leaflist_t *llist,
		      obj_template_t *obj)
{
    status_t res, retres;

    retres = NO_ERR;

    res = resolve_metadata(tkc, mod, obj);
    CHK_EXIT(res, retres);

    if (!obj_is_refine(obj)) {
	res = yang_typ_resolve_type(tkc, mod,
				    llist->typdef, NULL, obj);
	if (res == NO_ERR) {
	    obj_set_xpath_flags(obj);
	}
	CHK_EXIT(res, retres);
    }

    /* mark default as zero or more entries
     * the min-elements and max-elements will override
     * this property at runtime
     */
    llist->typdef->iqual = NCX_IQUAL_ZMORE;

    finish_config_flag(obj);

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* check if minelems and maxelems are valid */
    if (llist->minelems && llist->maxelems) {
	if (llist->minelems > llist->maxelems) {
	    log_error("\nError: leaf-list '%s' min-elements > max-elements",
		      obj_get_name(obj));
	    retres = ERR_NCX_INVALID_VALUE;
	    tkc->cur = obj->tk;
	    ncx_print_errormsg(tkc, mod, retres);
	}
    }

    return retres;
				    
}  /* resolve_leaflist */


/********************************************************************
* FUNCTION resolve_list
* 
* Check the list object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   list == obj_list_t to check
*   obj == parent object for 'list'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_list (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  obj_list_t *list,
		  obj_template_t *obj)
{
    status_t res, retres;

    retres = NO_ERR;

    res = resolve_metadata(tkc, mod, obj);
    CHK_EXIT(res, retres);

    if (!obj_is_refine(obj)) {
	res = yang_typ_resolve_typedefs(tkc, mod, list->typedefQ, obj);
	CHK_EXIT(res, retres);

	res = yang_grp_resolve_groupings(tkc, mod, list->groupingQ, obj);
	CHK_EXIT(res, retres);
    }

    finish_config_flag(obj);

    res = yang_obj_resolve_datadefs(tkc, mod, list->datadefQ);
    CHK_EXIT(res, retres);

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* check if minelems and maxelems are valid */
    if (list->minelems && list->maxelems) {
	if (list->minelems > list->maxelems) {
	    log_error("\nError: list '%s' min-elements > max-elements",
		      obj_get_name(obj));
	    retres = ERR_NCX_INVALID_VALUE;
	    tkc->cur = obj->tk;
	    ncx_print_errormsg(tkc, mod, retres);
	}
    }

    return retres;
				    
}  /* resolve_list */


/********************************************************************
* FUNCTION get_list_key
* 
* Get the key components and validate, save them

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   list == obj_list_t to check
*   obj == parent object for 'list'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    get_list_key (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  obj_list_t    *list,
		  obj_template_t *obj)
{
    obj_template_t    *key;
    xmlChar           *str, *p, savech;
    tk_token_t        *errtk;
    obj_key_t         *objkey;
    status_t           retres;
    ncx_btype_t        btyp;

    retres = NO_ERR;
    errtk = (list->keytk) ? list->keytk : obj->tk;

    /* skip all leading whitespace */
    p = list->keystr;
    while (*p && xml_isspace(*p)) {
	p++;
    }
    if (!*p) {
	log_error("\nError: no identifiers entered in key '%s'",
		  list->keystr);
	retres = ERR_NCX_INVALID_VALUE;
	tkc->cur = errtk;
	ncx_print_errormsg(tkc, mod, retres);
	return retres;
    }

    /* find end of non-whitespace string */
    while (*p) {
	str = p;
	while (*p && !xml_isspace(*p)) {
	    p++;
	}

	/* check for a valid identifier name
	 * YANG does not currently allow deep keys
	 * so do not look for descendant-schema-nodeid
	 */
	if (!ncx_valid_name(str, (uint32)(p-str))) {
	    savech = *p;
	    *p = 0;
	    log_error("\nError: invalid identifier in key"
		      " for list '%s' (%s)", list->name, str);
	    retres = ERR_NCX_INVALID_NAME;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, retres);
	    *p = savech;
	    while (*p && xml_isspace(*p)) {
		p++;
	    }
	    continue;
	}

	/* got a valid identifier so find it as a
	 * child node in the obj_list_t datadefQ
	 */
	savech = *p;
	*p = 0;
	key = obj_find_template(list->datadefQ, 
				obj_get_mod_name(obj), 
				str);
	if (!key) {
	    log_error("\nError: node %s not found in key "
		      "for list '%s'", str, list->name);
	    retres = ERR_NCX_DEF_NOT_FOUND;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, retres);
	    *p = savech;
	    while (*p && xml_isspace(*p)) {
		p++;
	    }
	    continue;
	} else {
	    key->flags |= OBJ_FL_KEY;
	}

	/* skip any whitespace between key components */
	*p = savech;
	while (*p && xml_isspace(*p)) {
	    p++;
	}

	/* make sure the key is a leaf */
	if (key->objtype != OBJ_TYP_LEAF) {
	    /* found the key node, but it is not a leaf */
	    log_error("\nError: node '%s' on line %u not a leaf in key"
		      " for list '%s' (%s)",
		      obj_get_name(key), 
		      key->linenum,
		      list->name, obj_get_typestr(key));
	    retres = ERR_NCX_TYPE_NOT_INDEX;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, retres);
	    continue;
	} 

	/* make sure the base type is OK for an index */
	if (!typ_ok_for_index(key->def.leaf->typdef)) {
	    btyp = typ_get_basetype(key->def.leaf->typdef);
	    if (btyp != NCX_BT_NONE) {
		log_error("\nError: leaf node '%s' on line %u not valid type "
			  "in key, for list '%s' (%s)",
			  obj_get_name(key),
			  key->linenum,
			  list->name,
			  tk_get_btype_sym(btyp));
		retres = ERR_NCX_TYPE_NOT_INDEX;
		tkc->cur = errtk;
		ncx_print_errormsg(tkc, mod, retres);
	    }
	}

	/* make sure madatory=false is not set for the key leaf */
	if ((key->flags & OBJ_FL_MANDSET) && !(key->flags & OBJ_FL_MANDATORY)) {
	    log_warn("\nWarning: 'mandatory false;' ignored in leaf '%s' "
		      "on line %u for list '%s'",
		      obj_get_name(key), key->linenum, list->name);
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, ERR_NCX_STMT_IGNORED);
	}

	/* make sure key component not already used (YANG CLR) */
	objkey = obj_find_key2(list->keyQ, key);
	if (objkey) {
	    log_error("\nError: duplicate key node '%s' on line %u "
		      "for list '%s'",
		      obj_get_name(key), key->linenum, list->name);
	    retres = ERR_NCX_DUP_ENTRY;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, retres);
	    continue;
	}

	/* get a new key record struct */
	objkey = obj_new_key();
	if (!objkey) {
	    retres = ERR_INTERNAL_MEM;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, retres);
	    return retres;
	}

	/* everything OK so save the key
	 * a backptr to 'objkey' in 'key' cannot be maintained
	 * because 'key' may be inside a grouping, and a simple
	 * uses foo; will cause the groupingQ to be used directly
	 */
	objkey->keyobj = key;
	dlq_enque(objkey, list->keyQ);
    }

    return retres;

}  /* get_list_key */


/********************************************************************
* FUNCTION get_unique_comps
* 
* Get the unique-stmt components and validate, save them

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   list == obj_list_t to check
*   obj == parent object for 'list'
*   uni == unique statement collected in this struct
*          needs to be validated and finalized
*
* OUTPUTS:
*   uni->compQ is filled with obj_unique_comp_t structs
*         each one represents one leaf in the unique tuple
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    get_unique_comps (tk_chain_t *tkc,
		      ncx_module_t  *mod,
		      obj_list_t *list,
		      obj_template_t *obj,
		      obj_unique_t *uni)
{
    obj_template_t    *uniobj, *testobj;
    xmlChar           *str, *p, *savestr, savech;
    tk_token_t        *errtk;
    obj_unique_comp_t *unicomp, *testcomp;
    status_t           res, retres;

    savestr = NULL;
    retres = NO_ERR;
    errtk = (uni->tk) ? uni->tk : obj->tk;

    /* skip all leading whitespace */
    p = uni->xpath;
    while (*p && xml_isspace(*p)) {
	p++;
    }
    if (!*p) {
	log_error("\nError: no identifiers entered in unique statement '%s'",
		  uni->xpath);
	retres = ERR_NCX_INVALID_VALUE;
	tkc->cur = errtk;
	ncx_print_errormsg(tkc, mod, retres);
	return retres;
    }

    /* keep parsing Xpath strings until EOS reached */
    while (*p) {
	/* find end of non-whitespace string */
	str = p;
	while (*p && !xml_isspace(*p)) {
	    p++;
	}
	savech = *p;
	*p = 0;

	/* check for a valid descendant-schema-nodeid string */
	res = xpath_find_schema_target_err(tkc, mod, obj,
					   list->datadefQ,
					   str, &uniobj, NULL, errtk);
	CHK_EXIT(res, retres);
	if (res == NO_ERR) {
	    savestr = xml_strdup(str);
	    if (!savestr) {
		retres = ERR_INTERNAL_MEM;
		tkc->cur = errtk;
		ncx_print_errormsg(tkc, mod, retres);
		return retres;
	    }
	}

	*p = savech;
	while (*p && xml_isspace(*p)) {
	    p++;   /* skip whitespace between strings */
	}
	if (res != NO_ERR) {
	    continue;
	}

	/* got a valid Xpath expression which points to a
	 * child node in the obj_list_t datadefQ
	 * make sure the unique target is a leaf
	 */
	if (uniobj->objtype != OBJ_TYP_LEAF ||
	    obj_get_basetype(uniobj) == NCX_BT_ANY) {
	    log_error("\nError: node '%s' on line %u not leaf in "
		      "list '%s' unique-stmt",
		      obj_get_name(uniobj),
		      uniobj->linenum,
		      list->name);
	    retres = ERR_NCX_INVALID_UNIQUE_NODE;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, retres);
	    m__free(savestr);
	    continue;
	} 

	/* make sure there is a no config mismatch */
	if (obj_is_config(obj) && !obj_is_config(uniobj)) {
	    log_error("\nError: leaf '%s' on line %u not config in "
		      "list '%s' unique-stmt",
		      obj_get_name(uniobj),
		      uniobj->linenum,
		      list->name);
	    retres = ERR_NCX_INVALID_UNIQUE_NODE;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, retres);
	    m__free(savestr);
	    continue;
	}

	/* the final target seems to be a valid leaf
	 * so check that its path back to the original
	 * object is all static object types
	 *   container, leaf, choice, case
	 */
	testobj = uniobj->parent;
	res = NO_ERR;
	while (testobj && (testobj != obj) && (res == NO_ERR)) {
	    switch (testobj->objtype) {
	    case OBJ_TYP_CONTAINER:
	    case OBJ_TYP_CHOICE:
	    case OBJ_TYP_CASE:
		break;
	    default:
		res = ERR_NCX_INVALID_UNIQUE_NODE;
		log_error("\nError: multi-instance node (%s) "
			  "within unique stmt '%s'",
			  obj_get_typestr(testobj),
			  uni->xpath);
		tkc->cur = errtk;
		ncx_print_errormsg(tkc, mod, res);
	    }

	    testobj = testobj->parent;
	}
	if (res != NO_ERR) {
	    m__free(savestr);
	    CHK_EXIT(res, retres);
	    continue;
	}

	uniobj->flags |= OBJ_FL_UNIQUE;

	/* make sure this leaf component not already used */
	for (testcomp = (obj_unique_comp_t *)dlq_firstEntry(&uni->compQ);
	     testcomp != NULL && res==NO_ERR;
	     testcomp = (obj_unique_comp_t *)dlq_nextEntry(testcomp)) {
	    if (testcomp->unobj == uniobj) {
		log_warn("\nWarning: duplicate unique node '%s' on line %u "
			  "for list '%s'",
			  obj_get_name(uniobj),
			  uniobj->linenum, list->name);
		tkc->cur = errtk;
		ncx_print_errormsg(tkc, mod,
				   ERR_NCX_DUP_UNIQUE_COMP);
	    }
	}

	/* try to save the info in a new unicomp struct */
	if (retres == NO_ERR) {
	    /* get a new unique component struct */
	    unicomp = obj_new_unique_comp();
	    if (!unicomp) {
		retres = ERR_INTERNAL_MEM;
		tkc->cur = errtk;
		ncx_print_errormsg(tkc, mod, retres);
		m__free(savestr);
		return retres;
	    } else {
		/* everything OK so save the unique component
		 * pass off the malloced savestr to the
		 * unicomp record
		 */
		unicomp->unobj = uniobj;
		unicomp->xpath = savestr;
		dlq_enque(unicomp, &uni->compQ);
	    }
	} else {
	    m__free(savestr);
	}
    }

    return retres;

}  /* get_unique_comps */


/********************************************************************
* FUNCTION resolve_list_final
* 
* Check the list object type after all uses and augments are expanded

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   list == obj_list_t to check
*   obj == parent object for 'list'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_list_final (tk_chain_t *tkc,
			ncx_module_t  *mod,
			obj_list_t *list,
			obj_template_t *obj)
{
    obj_unique_t    *uni;
    status_t         res, retres;

    retres = NO_ERR;

    /* validate key clause */
    if (list->keystr) {
	res = get_list_key(tkc, mod, list, obj);
	CHK_EXIT(res, retres);
    }

    /* validate Q of unique clauses */
    for (uni = (obj_unique_t *)dlq_firstEntry(list->uniqueQ);
	 uni != NULL;
	 uni = (obj_unique_t *)dlq_nextEntry(uni)) {
	res = get_unique_comps(tkc, mod, list, obj, uni);
	CHK_EXIT(res, retres);
    }

    return retres;

}  /* resolve_list_final */


/********************************************************************
* FUNCTION resolve_case
* 
* Check the case object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   cas == obj_case_t to check
*   obj == parent object for 'cas'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_case (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  obj_case_t *cas,
		  obj_template_t *obj)
{
    status_t res, retres;

    retres = NO_ERR;

    res = yang_obj_resolve_datadefs(tkc, mod, cas->datadefQ);
    CHK_EXIT(res, retres);

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    return retres;
				    
}  /* resolve_case */


/********************************************************************
* FUNCTION resolve_choice
* 
* Check the choice object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   choic == obj_choice_t to check
*   obj == parent object for 'choic'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_choice (tk_chain_t *tkc,
		    ncx_module_t  *mod,
		    obj_choice_t *choic,
		    obj_template_t *obj)
{
    obj_case_t *cas;
    obj_template_t *cobj;
    status_t res, retres;

    retres = NO_ERR;

    /* not in draft yet !!! */
    finish_config_flag(obj);

    if ((obj->flags & OBJ_FL_MANDATORY) && choic->defval) {
	log_error("\nError: both mandatory and default statements present"
		  "'%s'", obj_get_name(obj));
	retres = ERR_NCX_INVALID_VALUE;
	tkc->cur = obj->tk;
	ncx_print_errormsg(tkc, mod, retres);
    }

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* finish up the data-def-stmts in each case arm */
    res = yang_obj_resolve_datadefs(tkc, mod, choic->caseQ);
    CHK_EXIT(res, retres);

    /* check defval is valid case name */
    if (choic->defval) {
	cas = obj_find_case(choic, obj_get_mod_name(obj), 
			    choic->defval);
	if (!cas) {
	    /* default is not a valid case name */
	    tkc->cur = obj->tk;
	    retres = ERR_NCX_INVALID_VALUE;
	    log_error("\nError: Choice default '%s' "
		      "not a valid case name", choic->defval);
	    ncx_print_errormsg(tkc, mod, retres);
	} else {
	    /* valid case name, 
	     * make sure 'cas' contains only optional data nodes
	     */
	    for (cobj = (obj_template_t *)dlq_firstEntry(cas->datadefQ);
		 cobj != NULL;
		 cobj = (obj_template_t *)dlq_nextEntry(cobj)) {
		if (obj_is_mandatory(cobj)) {
		    tkc->cur = cobj->tk;		    
		    retres = ERR_NCX_DEFCHOICE_NOT_OPTIONAL;
		    ncx_print_errormsg(tkc, mod, retres);
		}
	    }
	}
    }
	
    return retres;
				    
}  /* resolve_choice */


/********************************************************************
* FUNCTION check_refine_allowed
* 
* Check the uses object type against the target node found
*
* Only checks if extra refine clauses are present
* which are not allowed for that 
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   refineobj == refine object to check
*   targobj == target object to check against
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_refine_allowed (tk_chain_t *tkc,
			  ncx_module_t  *mod,
			  obj_template_t *refineobj,
			  obj_template_t *targobj)
{
    obj_refine_t   *refine;
    xpath_pcb_t    *must;
    boolean         pres, def, conf, mand, minel, maxel, bmust;
    status_t        res;

    refine = refineobj->def.refine;

    pres = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    minel = FALSE;
    maxel = FALSE;
    bmust = FALSE;

    res = NO_ERR;

    switch (targobj->objtype) {
    case OBJ_TYP_LEAF:
	conf = TRUE;
	mand = TRUE;
	if (obj_get_basetype(targobj) != NCX_BT_ANY) {
	    bmust = TRUE;
	    def = TRUE;
	}
	break;
    case OBJ_TYP_LEAF_LIST:
	conf = TRUE;
	if (obj_get_basetype(targobj) == NCX_BT_ANY) {
	    mand = TRUE;
	} else {
	    bmust = TRUE;
	    minel = TRUE;
	    maxel = TRUE;
	}
	break;
    case OBJ_TYP_CONTAINER:
	bmust = TRUE;
	pres = TRUE;
	conf = TRUE;
	break;
    case OBJ_TYP_LIST:
	bmust = TRUE;
	conf = TRUE;
	minel = TRUE;
	maxel = TRUE;
	break;
    case OBJ_TYP_CHOICE:
	def = TRUE;
	mand = TRUE;
	break;
    case OBJ_TYP_CASE:
	break;
    default:
	return NO_ERR;  /* error: should already be reported */
    }

    /* check all the fields except description and reference
     * since they are allowed to appear in every variant
     */
    if (refine->presence && !pres) {
	res = ERR_NCX_REFINE_NOT_ALLOWED;
	log_error("\nError: 'presence' refinement on %s '%s'",
		  obj_get_typestr(targobj),
		  obj_get_name(targobj));
	tkc->cur = refine->presence_tk;
	ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->def && !def) {
	res = ERR_NCX_REFINE_NOT_ALLOWED;
	log_error("\nError: 'default' refinement on %s '%s'",
		  obj_get_typestr(targobj),
		  obj_get_name(targobj));
	tkc->cur = refine->def_tk;
	ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->config_tk && !conf) {
	res = ERR_NCX_REFINE_NOT_ALLOWED;
	log_error("\nError: 'config' refinement on %s '%s'",
		  obj_get_typestr(targobj),
		  obj_get_name(targobj));
	tkc->cur = refine->config_tk;
	ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->mandatory_tk && !mand) {
	res = ERR_NCX_REFINE_NOT_ALLOWED;
	log_error("\nError: 'mandatory' refinement on %s '%s'",
		  obj_get_typestr(targobj),
		  obj_get_name(targobj));
	tkc->cur = refine->config_tk;
	ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->minelems_tk && !minel) {
	res = ERR_NCX_REFINE_NOT_ALLOWED;
	log_error("\nError: 'min-elements' refinement on %s '%s'",
		  obj_get_typestr(targobj),
		  obj_get_name(targobj));
	tkc->cur = refine->minelems_tk;
	ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->maxelems_tk && !maxel) {
	res = ERR_NCX_REFINE_NOT_ALLOWED;
	log_error("\nError: 'max-elements' refinement on %s '%s'",
		  obj_get_typestr(targobj),
		  obj_get_name(targobj));
	tkc->cur = refine->maxelems_tk;
	ncx_print_errormsg(tkc, mod, res);
    }

    if (!dlq_empty(&refine->mustQ) && !bmust) {
	res = ERR_NCX_REFINE_NOT_ALLOWED;
	for (must = (xpath_pcb_t *)dlq_firstEntry(&refine->mustQ);
	     must != NULL;
	     must = (xpath_pcb_t *)dlq_nextEntry(must)) {
	    log_error("\nError: 'must' refinement on %s '%s'",
		      obj_get_typestr(targobj),
		      obj_get_name(targobj));
	    tkc->cur = must->tk;
	    ncx_print_errormsg(tkc, mod, res);
	}
    }

    return res;

}  /* check_refine_allowed */


/********************************************************************
* FUNCTION combine_refine_objects
* 
* Combine two refine objects with the same target
* Add fields from the mergeobj into the keepobj.
*
* The mergeobj should be freed after this call
*
* Only checks if extra refine clauses are present
* which are not allowed for that 
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   keepobj == refine object to keep
*   mergeobj == refine object to merge into 'keepobj'
*   targobj == refine object target to check type
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    combine_refine_objects (tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    obj_template_t *keepobj,
			    obj_template_t *mergeobj,
			    obj_template_t *targobj)
{
    obj_refine_t   *krefine, *mrefine;
    boolean         pres, def, conf, mand, minel, maxel, bmust;
    status_t        res;

    krefine = keepobj->def.refine;
    mrefine = mergeobj->def.refine;

    pres = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    minel = FALSE;
    maxel = FALSE;
    bmust = FALSE;

    res = NO_ERR;

    switch (targobj->objtype) {
    case OBJ_TYP_LEAF:
	conf = TRUE;
	mand = TRUE;
	if (obj_get_basetype(targobj) != NCX_BT_ANY) {
	    bmust = TRUE;
	    def = TRUE;
	}
	break;
    case OBJ_TYP_LEAF_LIST:
	conf = TRUE;
	if (obj_get_basetype(targobj) == NCX_BT_ANY) {
	    mand = TRUE;
	} else {
	    bmust = TRUE;
	    minel = TRUE;
	    maxel = TRUE;
	}
	break;
    case OBJ_TYP_CONTAINER:
	bmust = TRUE;
	pres = TRUE;
	conf = TRUE;
	break;
    case OBJ_TYP_LIST:
	bmust = TRUE;
	conf = TRUE;
	minel = TRUE;
	maxel = TRUE;
	break;
    case OBJ_TYP_CHOICE:
	def = TRUE;
	mand = TRUE;
	break;
    case OBJ_TYP_CASE:
	break;
    default:
	return NO_ERR;  /* error: should already be reported */
    }


    if (mrefine->descr) {
	if (krefine->descr) {
	    res = ERR_NCX_DUP_REFINE_STMT;
	    log_error("\nError: description-stmt set in refine on line %u",
		      krefine->descr_tk->linenum);
	    tkc->cur = mrefine->descr_tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else {
	    krefine->descr = mrefine->descr;
	    krefine->descr_tk = mrefine->descr_tk;
	    mrefine->descr = NULL;
	}
    }

    if (mrefine->ref) {
	if (krefine->ref) {
	    res = ERR_NCX_DUP_REFINE_STMT;
	    log_error("\nError: reference-stmt set in refine on line %u",
		      krefine->ref_tk->linenum);
	    tkc->cur = mrefine->ref_tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else {
	    krefine->ref = mrefine->ref;
	    krefine->ref_tk = mrefine->ref_tk;
	    mrefine->ref = NULL;
	}
    }

    if (mrefine->presence && pres) {
	if (krefine->presence) {
	    res = ERR_NCX_DUP_REFINE_STMT;
	    log_error("\nError: presence-stmt set in refine on line %u",
		      krefine->presence_tk->linenum);
	    tkc->cur = mrefine->presence_tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else {
	    krefine->presence = mrefine->presence;
	    krefine->presence_tk = mrefine->presence_tk;
	    mrefine->presence = NULL;
	}
    }

    if (mrefine->def && def) {
	if (krefine->def) {
	    res = ERR_NCX_DUP_REFINE_STMT;
	    log_error("\nError: default-stmt set in refine on line %u",
		      krefine->def_tk->linenum);
	    tkc->cur = mrefine->def_tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else {
	    krefine->def = mrefine->def;
	    krefine->def_tk = mrefine->def_tk;
	    mrefine->def = NULL;
	}
    }

    if (mrefine->config_tk && conf) {
	if (krefine->config_tk) {
	    res = ERR_NCX_DUP_REFINE_STMT;
	    log_error("\nError: config-stmt set in refine on line %u",
		      krefine->config_tk->linenum);
	    tkc->cur = mrefine->config_tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else {
	    krefine->config_tk = mrefine->config_tk;
	    keepobj->flags |= OBJ_FL_CONFSET;
	    if (mergeobj->flags & OBJ_FL_CONFIG) {
		keepobj->flags |= OBJ_FL_CONFIG;
	    } else {
		keepobj->flags &= ~OBJ_FL_CONFIG;
	    }
	}
    }

    if (mrefine->mandatory_tk && mand) {
	if (krefine->mandatory_tk) {
	    res = ERR_NCX_DUP_REFINE_STMT;
	    log_error("\nError: mandatory-stmt set in refine on line %u",
		      krefine->mandatory_tk->linenum);
	    tkc->cur = mrefine->mandatory_tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else {
	    krefine->mandatory_tk = mrefine->mandatory_tk;
	    keepobj->flags |= OBJ_FL_MANDSET;
	    if (mergeobj->flags & OBJ_FL_MANDATORY) {
		keepobj->flags |= OBJ_FL_MANDATORY;
	    }
	}
    }

    if (mrefine->minelems_tk && minel) {
	if (krefine->minelems_tk) {
	    res = ERR_NCX_DUP_REFINE_STMT;
	    log_error("\nError: min-elements-stmt set in refine on line %u",
		      krefine->minelems_tk->linenum);
	    tkc->cur = mrefine->minelems_tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else {
	    krefine->minelems = mrefine->minelems;
	    krefine->minelems_tk = mrefine->minelems_tk;
	}
    }

    if (mrefine->maxelems_tk && maxel) {
	if (krefine->maxelems_tk) {
	    res = ERR_NCX_DUP_REFINE_STMT;
	    log_error("\nError: max-elements-stmt set in refine on line %u",
		      krefine->maxelems_tk->linenum);
	    tkc->cur = mrefine->maxelems_tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else {
	    krefine->maxelems = mrefine->maxelems;
	    krefine->minelems_tk = mrefine->minelems_tk;
	}
    }

    if (!dlq_empty(&mrefine->mustQ) && bmust) {
	dlq_block_enque(&mrefine->mustQ, &krefine->mustQ);
    }

    if (!dlq_empty(&mergeobj->appinfoQ)) {
	dlq_block_enque(&mergeobj->appinfoQ, &keepobj->appinfoQ);
    }

    return res;

}  /* combine_refine_objects */


/********************************************************************
* FUNCTION resolve_uses
* 
* Check the uses object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   uses == obj_uses_t to check
*   obj == parent object for 'uses'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_uses (tk_chain_t *tkc,
		  ncx_module_t  *mod,
		  obj_uses_t *uses,
		  obj_template_t *obj)
{
    obj_template_t *chobj, *testobj, *cobj, *targobj, *nextobj;
    obj_case_t     *cas;
    obj_refine_t   *refine;
    status_t        res, retres;

    retres = NO_ERR;

    /* find the grouping that this uses references if this is
     * a local grouping, and grp has not been set yet
     */
    if (!uses->grp) {
	uses->grp = obj_find_grouping(obj, uses->name);
	if (!uses->grp) {
	    uses->grp = ncx_find_grouping(mod, uses->name);
	    if (!uses->grp) {
		log_error("\nError: grouping '%s' not found",
			  uses->name);
		retres = ERR_NCX_DEF_NOT_FOUND;
		tkc->cur = obj->tk;
		ncx_print_errormsg(tkc, mod, retres);
	    }
	}
    }

    /* check for nested uses -- a uses AA within grouping AA */
    if (uses->grp) {
	uses->grp->used = TRUE;
	res = yang_grp_check_nest_loop(tkc, mod, obj, uses->grp);
	CHK_EXIT(res, retres);
	if (res != NO_ERR) {
	    uses->grp = NULL;   /* prevent recursive crash later */
	}
    }


    /* resolve all the grouping augments, skip the refines */
    res = yang_obj_resolve_datadefs(tkc, mod, uses->datadefQ);
    if (res != NO_ERR) {
	retres = res;
    }
    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* make sure all the refinements really match a child
     * in the grouping
     */
    for (chobj = (obj_template_t *)dlq_firstEntry(uses->datadefQ);
	 chobj != NULL;
	 chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

	if (chobj->objtype != OBJ_TYP_REFINE) {
	    continue;
	}

	refine = chobj->def.refine;

	/* find schema-nodeid target
	 * the node being refined MUST exist in the grouping
	 */
	res = xpath_find_schema_target(tkc, mod, obj,
				       &uses->grp->datadefQ,
				       refine->target, 
				       &targobj, NULL);
	if (res != NO_ERR || !targobj) {
	    /* warning: refined obj not in the grouping */
	    log_warn("\nWarning: refinement node '%s' not found"
		      " in grouping '%s'", obj_get_name(chobj),
		      uses->grp->name);
	    retres = ERR_NCX_MISSING_REFTARGET;
	    tkc->cur = chobj->tk;
	    ncx_print_errormsg(tkc, mod, retres);
	} else {
	    /* refine target is valid, so save it */
	    refine->targobj = targobj;

	    /* check any extra refinements not allowed for
	     * the target object type
	     */
	    res = check_refine_allowed(tkc, mod,
				       chobj, targobj);
	    CHK_EXIT(res, retres);

	    /* check if any default statements are present,
	     * and if they are OK for the target data type
	     */
	    if (targobj->objtype == OBJ_TYP_LEAF) {
		if (refine->def) {
		    res = val_simval_ok(targobj->def.leaf->typdef,
					refine->def);
		    if (res != NO_ERR) {
			retres = res;
			log_error("\nError: Leaf refinement '%s' has "
				  "invalid default value (%s)",
				  obj_get_name(targobj),
				  refine->def);
			tkc->cur = refine->def_tk;
			ncx_print_errormsg(tkc, mod, retres);
		    }
		}
	    } else if (targobj->objtype == OBJ_TYP_CHOICE) {
		if (refine->def) {
		    cas = obj_find_case(targobj->def.choic,
					obj_get_mod_name(targobj),
					refine->def);
		    if (!cas) {
			/* default is not a valid case name */
			tkc->cur = refine->def_tk;
			retres = ERR_NCX_INVALID_VALUE;
			log_error("\nError: Refined choice default '%s' "
				  "is not a valid case name",
				  refine->def);
			ncx_print_errormsg(tkc, mod, retres);
		    } else {
			/* valid case name, 
			 * make sure 'cas' contains only optional data nodes
			 */
			for (cobj = (obj_template_t *)
				 dlq_firstEntry(cas->datadefQ);
			     cobj != NULL;
			     cobj = (obj_template_t *)dlq_nextEntry(cobj)) {
			    if (obj_has_name(cobj) &&
				obj_is_mandatory(cobj)) {
				tkc->cur = cobj->tk;		    
				retres = ERR_NCX_DEFCHOICE_NOT_OPTIONAL;
				ncx_print_errormsg(tkc, mod, retres);
			    }
			}
		    }
		}
	    }
	}
    }

    /* go through the refinement objects one more time
     * and combine the ones with the same target (if any)
     * Generate an error for duplicate sub-clauses entered
     */
    for (chobj = (obj_template_t *)dlq_firstEntry(uses->datadefQ);
	 chobj != NULL;
	 chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

	if (chobj->objtype != OBJ_TYP_REFINE) {
	    continue;
	}

	cobj = chobj->def.refine->targobj;
	if (!cobj) {
	    continue;
	}

	/* look through rest of Q for any refine w/ same target */
	for (testobj = (obj_template_t *)dlq_nextEntry(chobj);
	     testobj != NULL; 
	     testobj = nextobj) {

	    nextobj = (obj_template_t *)dlq_nextEntry(testobj);

	    if (testobj->objtype != OBJ_TYP_REFINE) {
		continue;
	    }

	    if (testobj->def.refine->targobj == cobj) {
		/* duplicate refine found, remove and combine */
		dlq_remove(testobj);
		res = combine_refine_objects(tkc, mod,
					     chobj, testobj, cobj);
		obj_free_template(testobj);
		CHK_EXIT(res, retres);
	    }
	}
    }

    /* at this point there should be one refine object for
     * each target specified, and no duplicates, unless fatal
     * error so will not continue anyways
     */
    return retres;
				    
}  /* resolve_uses */


/********************************************************************
* FUNCTION expand_uses
* 
* Expand the indicated grouping inline, inserted into 
* datadefQ just before the uses object node

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == obj_template_t that contains the obj_uses_t to check
*   datadefQ == Q that obj is stored in (needed to check for dup. err)
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    expand_uses (tk_chain_t *tkc,
		 ncx_module_t  *mod,
		 obj_template_t *obj,
		 dlq_hdr_t *datadefQ)
{
    obj_uses_t     *uses;
    obj_template_t *chobj, *newobj, *testobj;
    const xmlChar  *name;
    status_t        res, retres;

    retres = NO_ERR;
    uses = obj->def.uses;

    if (!uses->grp) {
	/* this node has errors, currently proccessing for
	 * errors only, so just keep going
	 */
	return NO_ERR;
    }

    /* go through each node in the grouping
     * make sure it is not already in the same datadefQ
     * as the uses; don't check the module name since
     * augments has not been expanded yet
     * clone the object and add it inline to the datadefQ
     */
    for (chobj = (obj_template_t *)
	     dlq_firstEntry(&uses->grp->datadefQ);
	 chobj != NULL;
	 chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

#ifdef YANG_OBJ_DEBUG
	log_debug3("\nexpand_uses: mod %s, object %s, on line %u",
		   mod->name, obj_get_name(chobj),
		   chobj->linenum);
#endif

	switch (chobj->objtype) {
	case OBJ_TYP_USES:    /* expand should already be done */
	case OBJ_TYP_AUGMENT:
	case OBJ_TYP_REFINE:
	    break;
	default:
	    name = obj_get_name(chobj);
	    testobj = obj_find_template_test(datadefQ, NULL, name);
	    if (testobj) {
		log_error("\nError: object '%s' already defined at line %u",
			  name, testobj->linenum);
		retres = ERR_NCX_DUP_ENTRY;
		tkc->cur = chobj->tk;
		ncx_print_errormsg(tkc, mod, retres);
	    } else {
		newobj = obj_clone_template(mod, chobj,
					    uses->datadefQ);
		if (!newobj) {
		    retres = ERR_INTERNAL_MEM;
		    tkc->cur = chobj->tk;
		    ncx_print_errormsg(tkc, mod, res);
		    return res;
		} else {
		    /* set the object module (and namespace)
		     * to the target, not the module w/ grouping
		     */
		    newobj->mod = obj->mod;
		    newobj->parent = obj->parent;
		    newobj->usesobj = obj;
		    dlq_insertAhead(newobj, obj);

#ifdef YANG_OBJ_DEBUG
		    log_debug3("\nexpand_uses: add new obj %s to parent %s,"
			       " uses.%u",
			       obj_get_name(newobj),
			       (obj->grp) ? obj->grp->name :
			       ((obj->parent) ? 
				obj_get_name(obj->parent) : NCX_EL_NONE),
			       obj->linenum);
#endif
		}
	    }
	}
    }

    /* go through each node in the uses datadefQ
     * looking for augments within the uses
     * expand the augment within the same Q
     * as the uses
     */
    for (chobj = (obj_template_t *)
	     dlq_firstEntry(uses->datadefQ);
	 chobj != NULL;
	 chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

	if (chobj->objtype != OBJ_TYP_AUGMENT) {
	    continue;
	}

#ifdef YANG_OBJ_DEBUG
	log_debug3("\nexpand_uses_augment: mod %s, augment on line %u",
		   mod->name, chobj->linenum);
#endif

	res = expand_augment(tkc, mod, chobj, datadefQ);
	CHK_EXIT(res, retres);
    }

    return retres;
				    
}  /* expand_uses */


/********************************************************************
* FUNCTION resolve_augment
* 
* Check the augment object type

* Error messages are printed by this function!!
 Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   aug == obj_augment_t to check
*   obj == parent object for 'aug'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_augment (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     obj_augment_t *aug,
		     obj_template_t *obj)
{
    status_t       res, retres;

    
    retres = NO_ERR;

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* figure out augment target later */

    /* check if correct target Xpath string form is present */
    if (obj->parent && !obj_is_root(obj->parent) && 
	aug->target && *aug->target == '/') {
	/* absolute-schema-nodeid target not allowed */
	log_error("\nError: absolute schema-nodeid form"
		  " not allowed in nested augment statement");
	retres = ERR_NCX_INVALID_VALUE;
	tkc->cur = obj->tk;
	ncx_print_errormsg(tkc, mod, retres);
    }

    /* check if correct target Xpath string form is present */
    if ((!obj->parent || obj_is_root(obj->parent)) && 
	(aug->target && *aug->target != '/')) {
	/* absolute-schema-nodeid target must be used */
	log_error("\nError: descendant schema-nodeid form"
		  " not allowed in top-level augment statement");
	retres = ERR_NCX_INVALID_AUGTARGET;
	tkc->cur = obj->tk;
	ncx_print_errormsg(tkc, mod, retres);
    }

    /* resolve augment contents */
    res = yang_obj_resolve_datadefs(tkc, mod, &aug->datadefQ);
    CHK_EXIT(res, retres);

    return retres;
				    
}  /* resolve_augment */


/********************************************************************
* FUNCTION expand_augment
* 
* Expand the indicated top-level or nested augment inline,
* inserted into the tree at the specified node
*
* Note that nested augment clauses are only allowed to
* use the descendant-schema-nodeid form of Xpath expression,
* and the target must therefore be within the sibling sub-trees
* contained in the datadefQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == obj_template_t that contains the obj_augment_t to check
*   datadefQ == Q of obj_template_t that contains 'obj'
*        
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    expand_augment (tk_chain_t *tkc,
		    ncx_module_t  *mod,
		    obj_template_t *obj,
		    dlq_hdr_t *datadefQ)
{
    obj_augment_t     *aug;
    obj_template_t    *targobj, *chobj, *newobj, *testobj;
    const xmlChar     *name;
    dlq_hdr_t         *targQ, *augQ;
    status_t           res, retres;
    boolean            augextern, augdefcase, config, confset;
    
    aug = obj->def.augment;
    if (!aug->target) {
	/* this node has errors, currently proccessing for
	 * errors only, so just keep going
	 */
	return NO_ERR;
    }

    augQ = &aug->datadefQ;
    retres = NO_ERR;
    targobj = NULL;
    targQ = NULL;

    /* find schema-nodeid target
     * the node being augmented MUST exist to be valid
     */
    res = xpath_find_schema_target(tkc, mod, obj, datadefQ,
				   aug->target, &targobj, NULL);
    if (res != NO_ERR) {
	return res;
    }
	
    aug->targobj = targobj;

    augextern = xml_strcmp(obj_get_mod_name(obj),
			   obj_get_mod_name(targobj));

    augdefcase = (targobj->objtype == OBJ_TYP_CASE && targobj->parent
		  && targobj->parent->def.choic->defval &&
		  !xml_strcmp(obj_get_name(targobj),
			      targobj->parent->def.choic->defval)) 
	? TRUE : FALSE;
    
    /* check external augment for mandatory nodes */
    if (augextern || augdefcase) {

	/* check that all the augment nodes are optional */
	for (testobj = (obj_template_t *)dlq_firstEntry(augQ);
	     testobj != NO_ERR;
	     testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

	    if (!obj_has_name(testobj)) {
		continue;
	    }
	
	    if (obj_is_mandatory(testobj)) {
		if (augextern) {
		    log_error("\nError: Mandatory object '%s' not allowed "
			      "in external augment statement",
			      obj_get_name(testobj));
		} else {
		    log_error("\nError: Mandatory object '%s' not allowed "
			      "in default case '%s'",
			      obj_get_name(testobj), 
			      obj_get_name(targobj));
		}
		
		retres = ERR_NCX_MANDATORY_NOT_ALLOWED;
		tkc->cur = testobj->tk;
		ncx_print_errormsg(tkc, mod, retres);
	    }
	}
    }

    /* make sure the objects augmented the target node
     * are OK for that object type
     */
    switch (targobj->objtype) {
    case OBJ_TYP_RPC:
	retres = ERR_NCX_INVALID_AUGTARGET;
	log_error("\nError: cannot augment rpc node '%s'; use 'input' "
		  "or 'output' instead", 
		  obj_get_name(targobj));
	tkc->cur = obj->tk;
	ncx_print_errormsg(tkc, mod, retres);
	break;
    case OBJ_TYP_CHOICE:
	for (testobj = (obj_template_t *)dlq_firstEntry(augQ);
	     testobj != NULL;
	     testobj = (obj_template_t *)dlq_nextEntry(testobj)) {
	    switch (testobj->objtype) {
	    case OBJ_TYP_RPC:
	    case OBJ_TYP_RPCIO:
	    case OBJ_TYP_NOTIF:
	    case OBJ_TYP_CHOICE:
		retres = ERR_NCX_INVALID_AUGTARGET;
		log_error("\nError: invalid %s '%s' augmenting choice node",
			  obj_get_typestr(testobj),
			  obj_get_name(testobj));
		tkc->cur = obj->tk;
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_TYP_NONE:
	    case OBJ_TYP_USES:
	    case OBJ_TYP_AUGMENT:
	    case OBJ_TYP_REFINE:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
		break;
	    default:
		;
	    }
	}
	break;
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	/* in this implementation, anyxml is tagged as a base type
	 * for a leaf, since it originally was a builtin type
	 * TBD: make anyxml a separate object type
	 * until then, make sure the basetype is not anyxml
	 */
	if (obj_get_basetype(targobj) == NCX_BT_ANY) {
	    retres = ERR_NCX_INVALID_AUGTARGET;
	    log_error("\nError: cannot augment anyxml node '%s'",
		      obj_get_name(testobj));
	    tkc->cur = obj->tk;
	    ncx_print_errormsg(tkc, mod, retres);
	}
	break;
    default:
	for (testobj = (obj_template_t *)dlq_firstEntry(augQ);
	     testobj != NULL;
	     testobj = (obj_template_t *)dlq_nextEntry(testobj)) {
	    switch (testobj->objtype) {
	    case OBJ_TYP_RPC:
	    case OBJ_TYP_RPCIO:
	    case OBJ_TYP_NOTIF:
	    case OBJ_TYP_CASE:
		retres = ERR_NCX_INVALID_AUGTARGET;
		log_error("\nError: invalid %s '%s' augmenting data node",
			  obj_get_typestr(testobj),
			  obj_get_name(testobj));
		tkc->cur = obj->tk;
		ncx_print_errormsg(tkc, mod, retres);
		break;
	    case OBJ_TYP_NONE:
	    case OBJ_TYP_AUGMENT:
	    case OBJ_TYP_REFINE:
		retres = SET_ERROR(ERR_INTERNAL_VAL);
		break;
	    default:
		;
	    }
	}
    }

    /* get the augment target datadefQ */
    targQ = obj_get_datadefQ(targobj);
    if (!targQ) {
	log_error("\nError: %s '%s' cannot be augmented",
		  obj_get_typestr(targobj),
		  obj_get_name(targobj));
	retres = ERR_NCX_INVALID_AUGTARGET;
	tkc->cur = targobj->tk;
	ncx_print_errormsg(tkc, mod, retres);
	return retres;
    }

    /* go through each node in the augment
     * make sure it is not already in the same datadefQ
     * if not, then clone the grouping object and add it
     * to the augment target
     */
    for (chobj = (obj_template_t *)dlq_firstEntry(augQ);
	 chobj != NULL;
	 chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

#ifdef YANG_OBJ_DEBUG
	log_debug3("\nexpand_aug: mod %s, object %s, on line %u",
		   mod->name, obj_get_name(chobj),
		   chobj->linenum);
#endif

	switch (chobj->objtype) {
	case OBJ_TYP_USES:    /* expand should already be done */
	    break;
	case OBJ_TYP_AUGMENT:
	    res = expand_augment(tkc, mod, chobj, &aug->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	default:
	    name = obj_get_name(chobj);

	    /* try to find the node in any namespace (warning) */
	    testobj = obj_find_template_test(targQ, NULL, name);
	    if (testobj && xml_strcmp(testobj->mod->name,
				      chobj->mod->name)) {
		log_warn("\nWarning: sibling object '%s' "
			 "already defined "
			 "in module %s at line %u",
			 name, testobj->mod->name,
			 testobj->linenum);
		res = ERR_NCX_DUP_AUGNODE;
		tkc->cur = chobj->tk;
		ncx_print_errormsg(tkc, mod, res);
	    }

	    /* try to find the node in the target namespace (error) */
	    testobj = obj_find_template_test(targQ, targobj->mod->name,
					     name);
	    if (testobj) {
		log_error("\nError: object '%s' already defined at line %u",
			  name, testobj->linenum);
		retres = ERR_NCX_DUP_ENTRY;
		tkc->cur = chobj->tk;
		ncx_print_errormsg(tkc, mod, retres);
	    } else {
		/* OK to create the new name
		 * make a clone of the augment object in
		 * case this augment is inside a grouping
		 */
		if (targobj->objtype == OBJ_TYP_CHOICE) {
		    /* make sure all the child nodes are wrapped
		     * in a OBJ_TYP_CASE node -- this has not
		     * been checked yet
		     */
		    newobj = obj_clone_template_case(mod, chobj, NULL);
		} else {
		    /* create a cloned object with the namespace of the
		     * module defining the augment
		     */
		    newobj = obj_clone_template(mod, chobj, NULL);
		}
		if (!newobj) {
		    res = ERR_INTERNAL_MEM;
		    tkc->cur = chobj->tk;
		    ncx_print_errormsg(tkc, mod, res);
		    return res;
		} else {
		    newobj->parent = targobj;
		    newobj->flags |= OBJ_FL_AUGCLONE;
		    newobj->augobj = obj;
		    obj_set_ncx_flags(newobj);
		    dlq_enque(newobj, targQ);

		    /* may need to set the config flag now, under the context
		     * of the actual target, not within the grouping
		     */
		    config = obj_get_config_flag2(newobj, &confset);
		    if (!confset) {
			obj_set_config_flag(newobj);
		    }

#ifdef YANG_OBJ_DEBUG
		    log_debug3("\nexpand_aug: add new obj %s to target %s.%u,"
			       " aug.%u",
			       obj_get_name(newobj),
			       obj_get_name(targobj),
			       targobj->linenum,
			       obj->linenum);
#endif
		}
	    }
	}
    }

    return retres;
				    
}  /* expand_augment */


/********************************************************************
* FUNCTION resolve_rpc
* 
* Check the rpc method object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   rpc == obj_rpc_t to check
*   obj == parent object for 'rpc'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_rpc (tk_chain_t *tkc,
		 ncx_module_t  *mod,
		 obj_rpc_t *rpc,
		 obj_template_t *obj)
{
    status_t          res, retres;

    retres = NO_ERR;

    res = yang_typ_resolve_typedefs(tkc, mod, &rpc->typedefQ, obj);
    CHK_EXIT(res, retres);

    res = yang_grp_resolve_groupings(tkc, mod, &rpc->groupingQ, obj);
    CHK_EXIT(res, retres);

    res = yang_obj_resolve_datadefs(tkc, mod, &rpc->datadefQ);
    CHK_EXIT(res, retres);

    return retres;
				    
}  /* resolve_rpc */


/********************************************************************
* FUNCTION resolve_rpcio
* 
* Check the rpc input or output object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   rpcio == obj_rpcio_t to check
*   obj == parent object for 'rpcio'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_rpcio (tk_chain_t *tkc,
		   ncx_module_t  *mod,
		   obj_rpcio_t *rpcio,
		   obj_template_t *obj)
{
    status_t              res, retres;

    retres = NO_ERR;

    res = yang_typ_resolve_typedefs(tkc, mod, &rpcio->typedefQ, obj);
    CHK_EXIT(res, retres);

    res = yang_grp_resolve_groupings(tkc, mod, &rpcio->groupingQ, obj);
    CHK_EXIT(res, retres);

    res = yang_obj_resolve_datadefs(tkc, mod, &rpcio->datadefQ);
    CHK_EXIT(res, retres);

    return retres;
				    
}  /* resolve_rpcio */


/********************************************************************
* FUNCTION resolve_notif
* 
* Check the notification object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   notif == obj_notif_t to check
*   obj == parent object for 'notif'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_notif (tk_chain_t *tkc,
		   ncx_module_t  *mod,
		   obj_notif_t *notif,
		   obj_template_t *obj)
{
    status_t          res, retres;

    retres = NO_ERR;

    res = yang_typ_resolve_typedefs(tkc, mod, &notif->typedefQ, obj);
    CHK_EXIT(res, retres);

    res = yang_grp_resolve_groupings(tkc, mod, &notif->groupingQ, obj);
    CHK_EXIT(res, retres);

    /* resolve notification contents */
    res = yang_obj_resolve_datadefs(tkc, mod, &notif->datadefQ);
    CHK_EXIT(res, retres);

    return retres;
				    
}  /* resolve_notif */


/********************************************************************
* FUNCTION consume_datadef
* 
* Parse the next N tokens as a data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
*   
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == grp_template_t parent or NULL if parent is not a grouping
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_datadef (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     dlq_hdr_t *que,
		     obj_template_t *parent,
		     grp_template_t *grp)
{
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          errdone;
    status_t         res;

    expstr = "anyxml, container, leaf, leaf-list, list, choice, uses,"
	"or augment keyword";
    errdone = TRUE;
    res = NO_ERR;
    tktyp = TK_CUR_TYP(tkc);
    val = TK_CUR_VAL(tkc);

    /* check the current token type */
    if (tktyp != TK_TT_TSTRING) {
	res = ERR_NCX_WRONG_TKTYPE;
	errdone = FALSE;
    } else {
	/* Got a token string so check the value */
	if (!xml_strcmp(val, YANG_K_ANYXML)) {
	    res = consume_anyxml(tkc, mod, que, parent, grp);
	} else if (!xml_strcmp(val, YANG_K_CONTAINER)) {
	    res = consume_container(tkc, mod, que,
					 parent, grp);
	} else if (!xml_strcmp(val, YANG_K_LEAF)) {
	    res = consume_leaf(tkc, mod, que,
				    parent, grp);
	} else if (!xml_strcmp(val, YANG_K_LEAF_LIST)) {
	    res = consume_leaflist(tkc, mod, que,
					parent, grp);
	} else if (!xml_strcmp(val, YANG_K_LIST)) {
	    res = consume_list(tkc, mod, que,
				    parent, grp);
	} else if (!xml_strcmp(val, YANG_K_CHOICE)) {
	    res = consume_choice(tkc, mod, que,
				      parent, grp);
	} else if (!xml_strcmp(val, YANG_K_USES)) {
	    res = consume_uses(tkc, mod, que, parent, grp);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    errdone = FALSE;
	}
    }

    if (res != NO_ERR && !errdone) {
	log_error("\nError: '%s' token not allowed here", val);
	ncx_mod_exp_err(tkc, mod, res, expstr);
	yang_skip_statement(tkc, mod);
    }

    return res;

}  /* consume_datadef */


/********************************************************************
* FUNCTION resolve_iffeatureQ
* 
* Check the Q of if-feature statements for the specified object

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == object to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_iffeatureQ (tk_chain_t *tkc,
			ncx_module_t  *mod,
			obj_template_t *obj)
{
    ncx_feature_t    *testfeature;
    ncx_iffeature_t  *iff;
    status_t          res, retres;
    boolean           errdone;

    retres = NO_ERR;

    /* check if there are any if-feature statements inside
     * this object that need to be resolved
     */
    for (iff = (ncx_iffeature_t *)
	     dlq_firstEntry(&obj->iffeatureQ);
	 iff != NULL;
	 iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

	testfeature = NULL;
	errdone = FALSE;
	res = NO_ERR;

	if (iff->prefix &&
	    xml_strcmp(iff->prefix, mod->prefix)) {
	    /* find the feature in another module */
	    res = yang_find_imp_feature(tkc, mod, iff->prefix,
					iff->name, iff->tk,
					&testfeature);
	    if (res != NO_ERR) {
		retres = res;
		errdone = TRUE;
	    }
	} else {
	    testfeature = ncx_find_feature(mod, iff->name);
	}

	if (!testfeature && !errdone) {
	    log_error("\nError: Feature '%s' not found "
		      "for if-feature statement in object '%s'",
		      iff->name, obj_get_name(obj));
	    res = retres = ERR_NCX_DEF_NOT_FOUND;
	    tkc->cur = iff->tk;
	    ncx_print_errormsg(tkc, mod, retres);
	}

	if (testfeature) {
	    iff->feature = testfeature;
	}
    }

    /* check the feature mismatch corner cases later,
     * after the OBJ_FL_KEY flags have been set
     */
    return retres;
				    
}  /* resolve_iffeatureQ */


/********************************************************************
* FUNCTION check_iffeature_mismatch
* 
* Check the child object against the ancestor node for 1 if-feature
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   ancestor == ancestor node of child to compare against
*               and stop the check
*   testobj == current object being checked
*   iff == current if-feature record within the testobj to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_iffeature_mismatch (tk_chain_t *tkc,
			      ncx_module_t  *mod,
			      obj_template_t *ancestor,
			      obj_template_t *testobj,
			      ncx_iffeature_t *iff)

{
    status_t  res;

    res = NO_ERR;

    if (!ncx_find_iffeature(&ancestor->iffeatureQ,
			    iff->prefix,
			    iff->name,
			    mod->prefix)) {
	res = ERR_NCX_INVALID_CONDITIONAL;
	log_error("\nError: 'if-feature %s' present for "
		  "%s %s, but not in list %s",
		  iff->name, 
		  obj_get_typestr(testobj),
		  obj_get_name(testobj),
		  obj_get_name(ancestor));
	tkc->cur = iff->tk;
	ncx_print_errormsg(tkc, mod, res);
    }
    return res;
}  /* check_iffeature_mismatch */


/********************************************************************
* FUNCTION check_one_when_mismatch
* 
* Check one object when clause(s) against another
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   test1 == node to check
*   test2 == node to check
* 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_one_when_mismatch (tk_chain_t *tkc,
			     ncx_module_t  *mod,
			     obj_template_t *test1,
			     obj_template_t *test2)
{
    if (!test1->when) {
	return NO_ERR;
    }

    if (test2->when) {
	if (!xml_strcmp(test1->when->exprstr,
			test2->when->exprstr)) {
	    return NO_ERR;
	}
    }
	
    if (test2->usesobj && test2->usesobj->when) {
	if (!xml_strcmp(test1->when->exprstr,
			test2->usesobj->when->exprstr)) {
	    return NO_ERR;
	}
    }
	
    if (test2->augobj && test2->augobj->when) {
	if (!xml_strcmp(test1->when->exprstr,
			test2->augobj->when->exprstr)) {
	    return NO_ERR;
	}
    }

    log_error("\nError: when-stmt '%s' not in affect "
	      "for list %s",
	      test1->when, obj_get_name(test2));
    tkc->cur = test1->when->tk;
    ncx_print_errormsg(tkc, mod, ERR_NCX_INVALID_CONDITIONAL);

    return ERR_NCX_INVALID_CONDITIONAL;
				    
}  /* check_one_conditional_mismatch */


/********************************************************************
* FUNCTION check_conditional_mismatch
* 
* Check the child object against the ancestor node to see
* if any child conditionals are present that are not
* present in the path to the ancestor.  Treat this
* as an error
*
* Do not call for every node!
*    - (parent-list, key-leaf)
*    - (unique-list, unique-node)
*    - (parent-choice, default-case)
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   ancestor == ancestor node of child to compare against
*               and stop the check
*   child == child object to check
* 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_conditional_mismatch (tk_chain_t *tkc,
				ncx_module_t  *mod,
				obj_template_t *ancestor,
				obj_template_t *child)
{
    obj_template_t   *testobj;
    ncx_iffeature_t  *iff;
    status_t          res, retres;
    boolean           done;

    retres = NO_ERR;

    testobj = child->parent;
    if (!testobj) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (ancestor->objtype==OBJ_TYP_CHOICE && 
	child->objtype==OBJ_TYP_CASE) {


    } else {
	/* make sure that the child node does not introduce
	 * any if-features that are not in the ancestor node
	 */
	testobj = child;
	done = FALSE;

	/* go up the tree from the child to the level before
	 * the ancestor
	 */
	while (!done) {
	    /* check all possible when clauses in the testobj
	     * against the ancestor node
	     */
	    res = check_one_when_mismatch(tkc, mod, 
					  testobj,
					  ancestor);
	    CHK_EXIT(res, retres);

	    if (testobj->usesobj) {
		res = check_one_when_mismatch(tkc, mod, 
					      testobj->usesobj,
					      ancestor);
		CHK_EXIT(res, retres);
	    }

	    if (testobj->augobj) {
		res = check_one_when_mismatch(tkc, mod, 
					      testobj->augobj,
					      ancestor);
		CHK_EXIT(res, retres);
	    }

	    /* check all the if-features in the testnode
	     * against the ones in the ancestor;
	     * a missing if-feature in the ancestor is an error
	     */
	    for (iff = (ncx_iffeature_t *)
		     dlq_firstEntry(&testobj->iffeatureQ);
		 iff != NULL;
		 iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

		res = check_iffeature_mismatch(tkc, mod, ancestor,
					       testobj, iff);
		CHK_EXIT(res, retres);
	    }


	    /* check any extra if-features from the uses object */
	    if (testobj->usesobj) {
		for (iff = (ncx_iffeature_t *)
			 dlq_firstEntry(&testobj->usesobj->iffeatureQ);
		     iff != NULL;
		     iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

		    if (!ncx_find_iffeature(&testobj->iffeatureQ,
					    iff->prefix,
					    iff->name,
					    mod->prefix)) {

			res = check_iffeature_mismatch(tkc, mod, ancestor,
						       testobj, iff);
			CHK_EXIT(res, retres);
		    }
		}
	    }

	    /* check any extra if-features from the augment object */
	    if (testobj->augobj) {
		for (iff = (ncx_iffeature_t *)
			 dlq_firstEntry(&testobj->augobj->iffeatureQ);
		     iff != NULL;
		     iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

		    if (!ncx_find_iffeature(&testobj->iffeatureQ,
					    iff->prefix,
					    iff->name,
					    mod->prefix)) {

			res = check_iffeature_mismatch(tkc, mod, ancestor,
						       testobj, iff);
			CHK_EXIT(res, retres);
		    }
		}
	    }

	    testobj = testobj->parent;
	    if (!testobj) {
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	    if (testobj == ancestor) {
		done = TRUE;
	    }
	}
    }

    return retres;
				    
}  /* check_conditional_mismatch */


/************   E X T E R N A L   F U N C T I O N S   ***************/


/********************************************************************
* FUNCTION yang_obj_consume_datadef
* 
* Parse the next N tokens as a data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
*   
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_datadef (tk_chain_t *tkc,
			      ncx_module_t  *mod,
			      dlq_hdr_t *que,
			      obj_template_t *parent)
{
    status_t         res;

#ifdef DEBUG
    if (!tkc || !mod || !que) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_datadef(tkc, mod, que, parent, NULL);
    return res;

}  /* yang_obj_consume_datadef */


/********************************************************************
* FUNCTION yang_obj_consume_datadef_grp
* 
* Parse the next N tokens as a data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == grp_template_t containing 'que'
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_datadef_grp (tk_chain_t *tkc,
				  ncx_module_t  *mod,
				  dlq_hdr_t *que,
				  obj_template_t *parent,
				  grp_template_t *grp)
{
    status_t         res;

#ifdef DEBUG
    if (!tkc || !mod || !que || !grp) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_datadef(tkc, mod, que, parent, grp);
    return res;

}  /* yang_obj_consume_datadef_grp */


/********************************************************************
* FUNCTION yang_obj_consume_rpc
* 
* Parse the next N tokens as a rpc-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'rpc' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new RPC added to module
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_rpc (tk_chain_t *tkc,
			  ncx_module_t  *mod)
{
    status_t         res;

#ifdef DEBUG
    if (!tkc || !mod) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_rpc(tkc, mod, &mod->datadefQ, NULL, NULL);
    return res;

}  /* yang_obj_consume_rpc */


/********************************************************************
* FUNCTION yang_obj_consume_notification
* 
* Parse the next N tokens as a notification-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'notification' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new notification added to module
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_notification (tk_chain_t *tkc,
				   ncx_module_t  *mod)
{
    status_t         res;

#ifdef DEBUG
    if (!tkc || !mod) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_notif(tkc, mod, &mod->datadefQ, NULL, NULL);
    return res;

}  /* yang_obj_consume_notification */


/********************************************************************
* FUNCTION yang_obj_consume_augment
* 
* Parse the next N tokens as a top-level augment-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'augment' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new augment object added to module 
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_augment (tk_chain_t *tkc,
			      ncx_module_t  *mod)
{
    status_t         res;

#ifdef DEBUG
    if (!tkc || !mod) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_augment(tkc, mod, &mod->datadefQ, NULL, NULL);
    return res;

}  /* yang_obj_consume_augment */


/********************************************************************
* FUNCTION yang_obj_consume_deviation
* 
* Parse the next N tokens as a top-level deviation-stmt
* Create a obj_deviation_t struct and add it to the 
* specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'deviation' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new deviation struct added to module deviationQ
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_deviation (tk_chain_t *tkc,
				ncx_module_t  *mod)
{
    obj_deviation_t  *dev;
    const xmlChar    *val;
    const char       *expstr;
    xmlChar          *str;
    tk_type_t         tktyp;
    boolean           done, desc, ref;
    status_t          res, retres;

#ifdef DEBUG
    if (!tkc || !mod) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* Get a new obj_deviation_t to fill in */
    dev = obj_new_deviation();
    if (!dev) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    val = NULL;
    expstr = "absolute schema node";
    str = NULL;
    done = FALSE;
    desc = FALSE;
    ref = FALSE;
    retres = NO_ERR;

    dev->tk = TK_CUR(tkc);
    dev->linenum = dev->tk->linenum;

    /* Get the mandatory deviation target */
    res = yang_consume_string(tkc, mod, &dev->target);
    CHK_DEV_EXIT(dev, res, retres);

    /* Get the starting left brace or semi-colon */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	obj_free_deviation(dev);
	return res;
    }

    /* check for semi-colon or left brace */
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
	dev->empty = TRUE;
	done = TRUE;
	break;
    case TK_TT_LBRACE:
	done = FALSE;
	break;
    default:
	res = ERR_NCX_WRONG_TKTYPE;
	expstr = "semi-colon or left brace";
	ncx_mod_exp_err(tkc, mod, res, expstr);
	done = TRUE;
    }

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_deviation(dev);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    obj_free_deviation(dev);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &dev->appinfoQ);
	    CHK_DEV_EXIT(dev, res, retres);
	    continue;
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}

	/* Got a keyword token string so check the value */
	if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &dev->descr,
				     &desc, &dev->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &dev->ref,
				     &ref, &dev->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DEVIATE)) {
	    res = consume_deviate(tkc, mod, dev);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    expstr = "description, reference, or deviate";
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	}
	CHK_DEV_EXIT(dev, res, retres);
    }

    /* save or delete the obj_deviation_t struct */
    if (dev->target) {
	dlq_enque(dev, &mod->deviationQ);
    } else {
	obj_free_deviation(dev);
    }

    return retres;

}  /* yang_obj_consume_deviation */


/********************************************************************
* FUNCTION yang_obj_resolve_datadefs
* 
* First pass object validation
*
* Analyze the entire datadefQ within the module struct
* Finish all the clauses within this struct that
* may have been defered because of possible forward references
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_datadefs (tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj;
    status_t         res, retres;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;


    /* first resolve all the local type names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
	 testobj != NULL;
	 testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

	res = ncx_resolve_appinfoQ(tkc, mod, &testobj->appinfoQ);
	CHK_EXIT(res, retres);

	res = resolve_iffeatureQ(tkc, mod, testobj);
	CHK_EXIT(res, retres);

	switch (testobj->objtype) {
	case OBJ_TYP_CONTAINER:
	    res = resolve_container(tkc, mod,
				    testobj->def.container, testobj);
	    break;
	case OBJ_TYP_LEAF:
	    res = resolve_leaf(tkc, mod,
			       testobj->def.leaf, testobj);
	    break;
	case OBJ_TYP_LEAF_LIST:
	    res = resolve_leaflist(tkc, mod,
				   testobj->def.leaflist, testobj);
	    break;
	case OBJ_TYP_LIST:
	    res = resolve_list(tkc, mod,
			       testobj->def.list, testobj);
	    break;
	case OBJ_TYP_CHOICE:
	    res = resolve_choice(tkc, mod,
				 testobj->def.choic, testobj);
	    break;
	case OBJ_TYP_CASE:
	    res = resolve_case(tkc, mod,
			       testobj->def.cas, testobj);
	    break;
	case OBJ_TYP_USES:
	    res = resolve_uses(tkc, mod,
			       testobj->def.uses, testobj);
	    break;
	case OBJ_TYP_AUGMENT:
	    res = resolve_augment(tkc, mod,
				  testobj->def.augment, testobj);
	    break;
	case OBJ_TYP_RPC:
	    res = resolve_rpc(tkc, mod,
			      testobj->def.rpc, testobj);
	    break;
	case OBJ_TYP_RPCIO:
	    res = resolve_rpcio(tkc, mod,
			      testobj->def.rpcio, testobj);
	    break;
	case OBJ_TYP_NOTIF:
	    res = resolve_notif(tkc, mod,
				testobj->def.notif, testobj);
	    break;
	case OBJ_TYP_REFINE:
	    res = NO_ERR;
	    break;
	case OBJ_TYP_NONE:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	CHK_EXIT(res, retres);
    }

    return retres;

}  /* yang_obj_resolve_datadefs */


/********************************************************************
* FUNCTION yang_obj_resolve_uses
* 
* Second pass object validation
*
* Expand and validate any uses clauses within any objects
* within the datadefQ
*
* Search through the entire datadefQ tree(s) and call
* the expand_uses() function
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_uses (tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj, *casobj;
    obj_case_t      *cas;
    obj_augment_t   *aug;
    status_t         res, retres;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* first resolve all the local type names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
	 testobj != NULL;
	 testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

#ifdef YANG_OBJ_DEBUG
	log_debug3("\nresolve_uses: mod %s, object %s, on line %u",
		   mod->name, obj_get_name(testobj),
		   testobj->linenum);
#endif

	switch (testobj->objtype) {
	case OBJ_TYP_CONTAINER:
	    res = yang_grp_resolve_complete(tkc, mod,
					    testobj->def.container->groupingQ,
					    testobj);
	    CHK_EXIT(res, retres);

	    res = yang_obj_resolve_uses(tkc, mod,
					testobj->def.container->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    break;
	case OBJ_TYP_LIST:
	    res = yang_grp_resolve_complete(tkc, mod,
					    testobj->def.list->groupingQ,
					    testobj);
	    CHK_EXIT(res, retres);

	    res = yang_obj_resolve_uses(tkc, mod,
					testobj->def.list->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_CHOICE:
	    for (casobj = (obj_template_t *)
		     dlq_firstEntry(testobj->def.choic->caseQ);
		 casobj != NULL;
		 casobj = (obj_template_t *)dlq_nextEntry(casobj)) {
		cas = casobj->def.cas;
		res = yang_obj_resolve_uses(tkc, mod, cas->datadefQ);
		CHK_EXIT(res, retres);
	    }
	    break;
	case OBJ_TYP_CASE:
	    cas = testobj->def.cas;
	    res = yang_obj_resolve_uses(tkc, mod, cas->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_USES:
	    res = expand_uses(tkc, mod, testobj, datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_AUGMENT:
	    aug = testobj->def.augment;
	    res = yang_obj_resolve_uses(tkc, mod, &aug->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_RPC:
	    res = yang_grp_resolve_complete(tkc, mod,
					    &testobj->def.rpc->groupingQ,
					    testobj);
	    CHK_EXIT(res, retres);

	    res = yang_obj_resolve_uses(tkc, mod,
					&testobj->def.rpc->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_RPCIO:
	    res = yang_grp_resolve_complete(tkc, mod,
					    &testobj->def.rpcio->groupingQ,
					    testobj);
	    CHK_EXIT(res, retres);

	    res = yang_obj_resolve_uses(tkc, mod,
					&testobj->def.rpcio->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_NOTIF:
	    res = yang_grp_resolve_complete(tkc, mod,
					    &testobj->def.notif->groupingQ,
					    testobj);
	    CHK_EXIT(res, retres);

	    res = yang_obj_resolve_uses(tkc, mod,
					&testobj->def.notif->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_NONE:
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	CHK_EXIT(res, retres);
    }

    return retres;

}  /* yang_obj_resolve_uses */


/********************************************************************
* FUNCTION yang_obj_resolve_augments
* 
*
* Third pass object validation
*
* Expand and validate any augment clauses within any objects
* within the datadefQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_augments (tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj;
    status_t         res, retres;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* first resolve all the local type names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
	 testobj != NULL;
	 testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

	if (testobj->objtype == OBJ_TYP_AUGMENT) {
	    res = expand_augment(tkc, mod, testobj, datadefQ);
	    CHK_EXIT(res, retres);
	}
    }

    return retres;

}  /* yang_obj_resolve_augments */


/********************************************************************
* FUNCTION yang_obj_resolve_final
* 
* Fourth pass object validation
*
* Check various final stage errors and warnings
* within a single file
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_final (tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj;
    status_t         res, retres;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* first resolve all the local object names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
	 testobj != NULL;
	 testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

	/* skip all cloned nodes */
	if (obj_is_cloned(testobj)) {
	    continue;
	}

#ifdef YANG_OBJ_DEBUG
	log_debug3("\nresolve_final: mod %s, object %s, on line %u",
		   mod->name, obj_get_name(testobj), testobj->linenum);
#endif
	
	switch (testobj->objtype) {
	case OBJ_TYP_CONTAINER:
	    res = yang_grp_resolve_final(tkc, mod,
					 testobj->def.container->groupingQ);
	    CHK_EXIT(res, retres);
	    res = yang_obj_resolve_final(tkc, mod, 
					 testobj->def.container->datadefQ);
	    CHK_EXIT(res, retres);
	    res = resolve_default_parm(tkc, mod, testobj);
	    yang_check_obj_used(tkc, mod,
				testobj->def.container->typedefQ,
				testobj->def.container->groupingQ);
	    break;
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    break;
	case OBJ_TYP_LIST:
	    res = yang_grp_resolve_final(tkc, mod,
					 testobj->def.list->groupingQ);
	    CHK_EXIT(res, retres);
	    res = yang_obj_resolve_final(tkc, mod, 
					 testobj->def.list->datadefQ);
	    CHK_EXIT(res, retres);
	    yang_check_obj_used(tkc, mod,
				testobj->def.list->typedefQ,
				testobj->def.list->groupingQ);
	    res = resolve_list_final(tkc, mod, testobj->def.list, testobj);
	    break;
	case OBJ_TYP_CHOICE:
	    res = yang_obj_resolve_final(tkc, mod, 
					 testobj->def.choic->caseQ);
	    break;
	case OBJ_TYP_CASE:
	    res = yang_obj_resolve_final(tkc, mod, 
					 testobj->def.cas->datadefQ);
	    break;
	case OBJ_TYP_USES:
	    res = yang_obj_resolve_final(tkc, mod, 
					 testobj->def.uses->datadefQ);
	    break;
	case OBJ_TYP_AUGMENT:
	    res = yang_obj_resolve_final(tkc, mod, 
					 &testobj->def.augment->datadefQ);
	    break;
	case OBJ_TYP_RPC:
	    res = yang_grp_resolve_final(tkc, mod,
					 &testobj->def.rpc->groupingQ);
	    CHK_EXIT(res, retres);
	    res = yang_obj_resolve_final(tkc, mod, 
					 &testobj->def.rpc->datadefQ);
	    yang_check_obj_used(tkc, mod,
				&testobj->def.rpc->typedefQ,
				&testobj->def.rpc->groupingQ);
	    break;
	case OBJ_TYP_RPCIO:
	    res = yang_grp_resolve_final(tkc, mod,
					 &testobj->def.rpcio->groupingQ);
	    CHK_EXIT(res, retres);
	    res = yang_obj_resolve_final(tkc, mod, 
					 &testobj->def.rpcio->datadefQ);
	    CHK_EXIT(res, retres);
	    res = resolve_default_parm(tkc, mod, testobj);
	    yang_check_obj_used(tkc, mod,
				&testobj->def.rpcio->typedefQ,
				&testobj->def.rpcio->groupingQ);
	    break;
	case OBJ_TYP_NOTIF:
	    res = yang_grp_resolve_final(tkc, mod,
					 &testobj->def.notif->groupingQ);
	    CHK_EXIT(res, retres);
	    res = yang_obj_resolve_final(tkc, mod, 
					 &testobj->def.notif->datadefQ);
	    yang_check_obj_used(tkc, mod,
				&testobj->def.notif->typedefQ,
				&testobj->def.notif->groupingQ);
	    break;
	case OBJ_TYP_REFINE:
	    break;
	case OBJ_TYP_NONE:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	CHK_EXIT(res, retres);
    }

    return retres;

}  /* yang_obj_resolve_final */


/********************************************************************
* FUNCTION yang_obj_resolve_xpath
* 
* Fifth (and final) pass object validation
*
* Check all leafref, must, and when XPath expressions
* to make sure they are well-formed
*
* Checks the cooked objects, and skips all groupings
* uses, and augment nodes
*
* MUST BE CALLED AFTER yang_obj_resolve_final
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_xpath (tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    dlq_hdr_t *datadefQ)
{
    obj_template_t        *testobj;
    const obj_template_t  *leafobj;
    obj_key_t             *key;
    const obj_unique_t    *uniq;
    obj_unique_comp_t     *uncomp;
    typ_def_t             *typdef;
    const xpath_pcb_t     *pcb;
    xpath_pcb_t           *pcbclone;
    status_t               res, retres;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* first resolve all the local object names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
	 testobj != NULL;
	 testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

	if (!obj_has_name(testobj)) {
	    /* skip augment and uses */
	    continue;
	}

	/* check the when-stmt in the object itself */
	if (testobj->when) {
	    res = resolve_when(mod, testobj->when, testobj);
	    CHK_EXIT(res, retres);
	}

	/* check the when-stmt in the uses object carried fwd */
	if (testobj->usesobj && testobj->usesobj->when) {
	    res = resolve_when(mod,
			       testobj->usesobj->when,
			       testobj);
	    CHK_EXIT(res, retres);
	}

	/* check the when-stmt in the augment object carried fwd */
	if (testobj->augobj && testobj->augobj->when) {
	    res = resolve_when(mod,
			       testobj->augobj->when,
			       testobj);
	    CHK_EXIT(res, retres);
	}

	/* validate correct Xpath in must clauses */
	res = resolve_mustQ(mod, testobj);
	CHK_EXIT(res, retres);
	
	switch (testobj->objtype) {
	case OBJ_TYP_CONTAINER:
	    /* check container children */
	    res = 
		yang_obj_resolve_xpath(tkc, mod, 
				       testobj->def.container->datadefQ);
	    break;
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    if (obj_get_basetype(testobj) == NCX_BT_LEAFREF) {
#ifdef YANG_OBJ_DEBUG
		log_debug3("\nresolve_xpath: mod %s, object %s, on line %u",
			   mod->name, obj_get_name(testobj), 
			   testobj->linenum);
#endif

		/* need to make a copy of the XPath PCB because
		 * typedefs are treated as read-only when referenced
		 * from a val_value_t node
		 */
		typdef = obj_get_typdef(testobj);
		pcb = typ_get_leafref_pcb(typdef);
		pcbclone = xpath_clone_pcb(pcb);
		if (!pcbclone) {
		    res = ERR_INTERNAL_MEM;
		} else {
		    tkc->cur = pcb->tk;
		    res = xpath_yang_parse_path(tkc, mod, pcbclone);
		    if (res == NO_ERR) {
			leafobj = NULL;
			res = xpath_yang_validate_path(mod, 
						       testobj, 
						       pcbclone, 
						       &leafobj);
			if (res == NO_ERR && leafobj) {
			    typ_set_xref_typdef(typdef, 
						obj_get_ctypdef(leafobj));
			}
		    }
		    xpath_free_pcb(pcbclone);
		}
	    }
	    break;
	case OBJ_TYP_LIST:
	    /* check that none of the key leafs have more
	     * conditionals than their list parent
	     */
	    for (key = obj_first_key(testobj);
		 key != NULL;
		 key = obj_next_key(key)) {

		if (key->keyobj) {
		    res = check_conditional_mismatch(tkc, mod,
						     testobj,
						     key->keyobj);
		    CHK_EXIT(res, retres);
		}
	    }

	    /* check that none of the unique set leafs have more
	     * conditionals than their list parent
	     */
	    for (uniq = obj_first_unique(testobj);
		 uniq != NULL;
		 uniq = obj_next_unique(uniq)) {

		for (uncomp = obj_first_unique_comp(uniq);
		     uncomp != NULL;
		     uncomp = obj_next_unique_comp(uncomp)) {

		    if (uncomp->unobj) {
			res = check_conditional_mismatch(tkc, mod,
							 testobj,
							 uncomp->unobj);
			CHK_EXIT(res, retres);
		    }
		}
	    }

	    /* check list children */
	    res = yang_obj_resolve_xpath(tkc, mod, 
					 testobj->def.list->datadefQ);
	    CHK_EXIT(res, retres);
	    break;
	case OBJ_TYP_CHOICE:
	    res = yang_obj_resolve_xpath(tkc, mod, 
					 testobj->def.choic->caseQ);
	    break;
	case OBJ_TYP_CASE:
	    res = yang_obj_resolve_xpath(tkc, mod, 
					 testobj->def.cas->datadefQ);
	    break;
	case OBJ_TYP_RPC:
	    res = yang_obj_resolve_xpath(tkc, mod, 
					 &testobj->def.rpc->datadefQ);
	    break;
	case OBJ_TYP_RPCIO:
	    res = yang_obj_resolve_xpath(tkc, mod, 
					 &testobj->def.rpcio->datadefQ);
	    break;
	case OBJ_TYP_NOTIF:
	    res = yang_obj_resolve_xpath(tkc, mod, 
					 &testobj->def.notif->datadefQ);
	    break;
	case OBJ_TYP_NONE:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	CHK_EXIT(res, retres);
    }

	return retres;

}  /* yang_obj_resolve_xpath */


/* END file yang_obj.c */
