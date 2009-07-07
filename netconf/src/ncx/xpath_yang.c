/*  FILE: xpath_yang.c

    Schema and data model Xpath search support for
    YANG specific 'XPath-like' expressions.
    This module provides validation only.

    Once this module validates that the YANG-specific
    syntax is followed, then xpath1.c can be
    used to evaluate the expression and generate
    a node-set result.

   The following YANG features are supported:

    - augment target
    - refine target
    - leafref path target
    - key attribute for insert operation

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
13nov08      abb      begun; split out from xpath.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
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

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xpath_yang
#include "xpath_yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                         V A R I A B L E S                         *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION set_next_objnode
* 
* Get the object identifier associated with
* QName in an leafref XPath expression
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*    prefix == prefix value used if any
*    nodename == node name to find (must be present)
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    set_next_objnode (xpath_pcb_t *pcb,
		      const xmlChar *prefix,
		      xmlns_id_t  nsid,
		      const xmlChar *nodename)
{
    const obj_template_t  **useobj, *foundobj;
    const xmlChar          *modname;
    ncx_module_t           *targmod;
    status_t                res;
    boolean                 laxnamespaces;

    /* TBD: change this to an agt_profile 'namespaces' */
    laxnamespaces = TRUE;
    targmod = NULL;
    foundobj = NULL;
    res = NO_ERR;
    useobj = NULL;

    /* set the pcb target to get the result */
    switch (pcb->curmode) {
    case XP_CM_TARGET:
	useobj = &pcb->targobj;
	break;
    case XP_CM_ALT:
	useobj = &pcb->altobj;
	break;
    case XP_CM_KEYVAR:
	useobj = &pcb->varobj;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* get the module from the NSID or the prefix */
    if (res == NO_ERR) {
	if (pcb->source == XP_SRC_XML || pcb->mod==NULL) {
	    if (nsid) {
		modname = xmlns_get_module(nsid);
		if (modname) {
		    targmod = ncx_find_module(modname, NULL);
		}
		if (!targmod) {
		    res = ERR_NCX_DEF_NOT_FOUND;
		    if (pcb->logerrors) {
			log_error("\nError: module not found in expr '%s'",
				  pcb->exprstr);
		    }
		}
	    } else if (!laxnamespaces) {
		res = ERR_NCX_UNKNOWN_NAMESPACE;
		if (pcb->logerrors) {
		    log_error("\nError: no namespace in expr '%s'",
			      pcb->exprstr);
		}
	    }
	} else {
	    res = xpath_get_curmod_from_prefix(prefix,
					       pcb->mod,
					       &targmod);
	    if (res != NO_ERR) {
		if (!prefix && laxnamespaces) {
		    res = NO_ERR;
		} else if (pcb->logerrors) {
		    log_error("\nError: Module for prefix '%s' not found",
			      (prefix) ? prefix : EMPTY_STRING);
		}
	    }
        }
    }

    /* check if no NSID or prefix used: instead of rejecting
     * the request, check any top-level object, if allowed
     * by the laxnamespaces parameter
     */
    if (!targmod && laxnamespaces && 
	res == NO_ERR && (!nsid || !prefix)) {

	if (!pcb->targobj) {
	    pcb->targobj = pcb->obj;
	}

	if (pcb->targobj) {
	    if (obj_is_root(pcb->targobj)) {
		foundobj = ncx_find_any_object(nodename);
		if (foundobj && nsid && 
		    obj_get_nsid(foundobj) != nsid) {
		    foundobj = NULL;
		}
            } else if (obj_get_nsid(pcb->targobj) == xmlns_nc_id() &&
                       (!xml_strcmp(obj_get_name(pcb->targobj),
                                    NCX_EL_RPC))) {
                foundobj = NULL;

                /* find an RPC method with the nodename */
                if (prefix && nsid == 0) {
                    nsid = xmlns_find_ns_by_prefix(prefix);
                }
                if (nsid == 0) {
                    foundobj = ncx_find_any_object(nodename);
                } else {
                    modname = xmlns_get_module(nsid);
                    if (modname) {
                        targmod = ncx_find_module(modname, NULL);
                        if (targmod) {
                            foundobj = ncx_find_object(targmod,
                                                       nodename);
                        }
                    }
                }

                if (foundobj && foundobj->objtype != OBJ_TYP_RPC) {
                    foundobj = NULL;
                }
	    } else {
		foundobj = obj_find_child(pcb->targobj,
					  obj_get_mod_name(pcb->targobj),
					  nodename);
	    }
	}

	if (!foundobj) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	    if (pcb->logerrors) {
		log_error("\nError: No object match for node '%s' "
			  "in expr '%s'",
			  nodename, pcb->exprstr);
	    }
	}
    }

    /* finish off any error and exit */
    if (res != NO_ERR) {
	if (pcb->logerrors) {
	    ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	}
	return res;
    }

    /* get the object from the module (if not already done) */
    if (foundobj) {
	/* already set in the wildcard search above */	;
    } else if (*useobj) {
	if (obj_is_root(*useobj)) {
	    foundobj = 
		obj_find_template_top(targmod,
				      ncx_get_modname(targmod),
				      nodename);
        } else if (obj_get_nsid(*useobj) == xmlns_nc_id() &&
                   !xml_strcmp(obj_get_name(*useobj), NCX_EL_RPC)) {
	    foundobj = 
		obj_find_template_top(targmod,
				      ncx_get_modname(targmod),
				      nodename);
            if (foundobj && foundobj->objtype != OBJ_TYP_RPC) {
                foundobj = NULL;
            }
	} else {
	    /* get child node of this object */
	    foundobj = 
		obj_find_child(*useobj,
			       ncx_get_modname(targmod),
			       nodename);
	}
    } else if (pcb->curmode == XP_CM_KEYVAR ||
	       pcb->curmode == XP_CM_ALT) {

	/* setting object for the first time
	 * get child node of the current context object
	 */
	if (pcb->targobj) {
	    foundobj = 
		obj_find_child(pcb->targobj,
			       ncx_get_modname(targmod),
			       nodename);
	}
    } else if (pcb->flags & XP_FL_ABSPATH) {
	/* setting object for the first time
	 * get top-level object from object module 
         */
	foundobj = 
	    obj_find_template_top(targmod,
				  ncx_get_modname(targmod),
				  nodename);
    } else {
        /* setting object for the first time
	 * but the context node is a leaf, so there
	 * is no possible child node of the the start object 
	 */
	;
    }

    if (!foundobj) {
	res = ERR_NCX_DEF_NOT_FOUND;
	if (pcb->logerrors) {
	    if (prefix) {
		log_error("\nError: object not found '%s:%s'",
			  prefix, nodename);
	    } else {
		log_error("\nError: object not found '%s'",
			  nodename);
	    }
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	}
    } else {
	*useobj = foundobj;
    }

    return res;

} /* set_next_objnode */


/********************************************************************
* FUNCTION move_up_obj
* 
* Get the parent object identifier
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    move_up_obj (xpath_pcb_t *pcb)
{
    const obj_template_t  **useobj, *foundobj;
    status_t                res;

    foundobj = NULL;
    res = NO_ERR;

    switch (pcb->curmode) {
    case XP_CM_TARGET:
	useobj = &pcb->targobj;
	break;
    case XP_CM_ALT:
	useobj = &pcb->altobj;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
	ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	return res;
    }

    if (*useobj) {
	/* get child node of this object */
	if ((*useobj)->parent) {
	    foundobj = (*useobj)->parent;
	} else if (*useobj != pcb->docroot) {
	    foundobj = pcb->docroot;
	}
    } else {
	/* setting object for the first time
	 * get parent node of the the start object 
	 */
	foundobj = pcb->obj->parent;
	if (!foundobj) {
	    foundobj = pcb->docroot;
	}
    }

    if (!foundobj) {
	res = ERR_NCX_DEF_NOT_FOUND;
	if (pcb->logerrors) {
	    log_error("\nError: parent not found for object '%s'",
		      obj_get_name(*useobj));
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	}
    } else {
	*useobj = foundobj;
    }

    return res;

} /* move_up_obj */


/********************************************************************
* FUNCTION parse_node_identifier
* 
* Parse the leafref node-identifier string
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* node-identifier        = [prefix ":"] identifier
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_node_identifier (xpath_pcb_t *pcb)
{
    const xmlChar  *prefix, *nodename;
    ncx_import_t   *import;
    ncx_module_t   *testmod;
    status_t        res;
    xmlns_id_t      nsid;

    prefix = NULL;
    nodename = NULL;
    import = NULL;
    res = NO_ERR;
    nsid = 0;

    /* get the next token in the step, node-identifier */
    res = TK_ADV(pcb->tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	return res;
    }

    switch (TK_CUR_TYP(pcb->tkc)) {
    case TK_TT_PERIOD:
	if (pcb->flags & XP_FL_INSTANCEID) {
	    if (!(pcb->targobj && 
		  pcb->targobj->objtype == OBJ_TYP_LEAF_LIST)) {
		res = ERR_NCX_INVALID_VALUE;
	    } else {
		nodename = obj_get_name(pcb->targobj);
	    }
	} else {
	    res = ERR_NCX_WRONG_TKTYPE;
	    if (pcb->logerrors) {
		log_error("\nError: '.' not allowed here");
		ncx_print_errormsg(pcb->tkc, NULL, res);
	    }
	}
	break;
    case TK_TT_MSTRING:
	/* pfix:identifier */
	prefix = TK_CUR_MOD(pcb->tkc);

	if (pcb->source != XP_SRC_XML) {
	    if (pcb->mod) {
                if (xml_strcmp(pcb->mod->prefix, prefix)) {
                    import = ncx_find_pre_import(pcb->mod, prefix);
                    if (!import) {
                        res = ERR_NCX_PREFIX_NOT_FOUND;
                        if (pcb->logerrors) {
                            log_error("\nError: import for "
                                      "prefix '%s' not found",
                                      prefix);
                            ncx_print_errormsg(pcb->tkc, pcb->mod, res);
                        }
                        break;
                    } else {
                        testmod = ncx_find_module(import->module,
                                                  import->revision);
                        if (testmod) {
                            nsid = testmod->nsid;
                        }
                    }
                }
            } else {
                nsid = xmlns_find_ns_by_prefix(prefix);
            }
	} else {
	    res = xml_get_namespace_id(pcb->reader,
				       prefix,
				       TK_CUR_MODLEN(pcb->tkc),
				       &nsid);
	    if (res != NO_ERR) {
		if (pcb->logerrors) {
		    log_error("\nError: unknown XML prefix '%s'",  prefix);
		    ncx_print_errormsg(pcb->tkc, NULL, res);
		}
		break;
	    }
	}

        /* save the NSID in the token for printing later */
        TK_CUR_NSID(pcb->tkc) = nsid;
	/* fall through to check QName */
    case TK_TT_TSTRING:
	nodename = TK_CUR_VAL(pcb->tkc);
	if (pcb->obj) {
	    res = set_next_objnode(pcb, prefix, nsid, nodename);
	} /* else identifier not checked here */
	break;
    default:
	res = ERR_NCX_WRONG_TKTYPE;
	ncx_mod_exp_err(pcb->tkc, 
                        pcb->mod, 
                        res,
			tk_get_token_name(TK_CUR_TYP(pcb->tkc)));
    }

    return res;

}  /* parse_node_identifier */


/********************************************************************
* FUNCTION parse_current_fn
* 
* Parse the leafref current-function token sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* current-function-invocation = 'current()'
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_current_fn (xpath_pcb_t *pcb)
{
    status_t     res;

    /* get the function name 'current' */
    res = xpath_parse_token(pcb, TK_TT_TSTRING);
    if (res != NO_ERR) {
	return res;
    }
    if (xml_strcmp(TK_CUR_VAL(pcb->tkc),
		   (const xmlChar *)"current")) {
	res = ERR_NCX_WRONG_VAL;
	ncx_mod_exp_err(pcb->tkc, pcb->mod, res,
			"current() function");
	return res;
    }

    /* get the left paren '(' */
    res = xpath_parse_token(pcb, TK_TT_LPAREN);
    if (res != NO_ERR) {
	return res;
    }

    /* get the right paren ')' */
    res = xpath_parse_token(pcb, TK_TT_RPAREN);
    if (res != NO_ERR) {
	return res;
    }

    /* assign the context node to the start object node */
    if (pcb->obj) {
	switch (pcb->curmode) {
	case XP_CM_TARGET:
	    pcb->targobj = pcb->obj;
	    break;
	case XP_CM_ALT:
	    pcb->altobj = pcb->obj;
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    return NO_ERR;

}  /* parse_current_fn */


/********************************************************************
* FUNCTION parse_path_key_expr
* 
* Parse the leafref *path-key-expr token sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* path-key-expr          = current-function-invocation "/"
*                          rel-path-keyexpr
*
* rel-path-keyexpr       = 1*(".." "/") *(node-identifier "/")
*                          node-identifier
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_path_key_expr (xpath_pcb_t *pcb)
{
    tk_type_t    nexttyp;
    status_t     res;
    boolean      done;

    res = NO_ERR;
    done = FALSE;

    pcb->altobj = NULL;
    pcb->curmode = XP_CM_ALT;

    /* get the current function call 'current()' */
    res = parse_current_fn(pcb);
    if (res != NO_ERR) {
	return res;
    }

    /* get the path separator '/' */
    res = xpath_parse_token(pcb, TK_TT_FSLASH);
    if (res != NO_ERR) {
	return res;
    }

    /* make one loop for each step of the first part of
     * the rel-path-keyexpr; the '../' token pairs
     */
    while (!done) {
	/* get the parent marker '..' */
	res = xpath_parse_token(pcb, TK_TT_RANGESEP);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* get the path separator '/' */
	res = xpath_parse_token(pcb, TK_TT_FSLASH);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* move the context node up one level */
	if (pcb->obj) {
	    res = move_up_obj(pcb);
	    if (res != NO_ERR) {
		done = TRUE;
		continue;
	    }
	}

	/* check the next token;
	 * it may be the start of another '../' pair
	 */
	nexttyp = tk_next_typ(pcb->tkc);
	if (nexttyp != TK_TT_RANGESEP) {
	    done = TRUE;
	} /* else keep going to the next '../' pair */
    }

    if (res != NO_ERR) {
	return res;
    }

    /* get the node-identifier sequence */
    done = FALSE;
    while (!done) {
	res = parse_node_identifier(pcb);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* check the next token;
	 * it may be the fwd slash to start another step
	 */
	nexttyp = tk_next_typ(pcb->tkc);
	if (nexttyp != TK_TT_FSLASH) {
	    done = TRUE;
	} else {
	    res = xpath_parse_token(pcb, TK_TT_FSLASH);
	    if (res != NO_ERR) {
		done = TRUE;
	    }
	}
    }

    return res;

}  /* parse_path_key_expr */


/********************************************************************
* FUNCTION get_key_number
* 
* Get the key number for the specified key
* It has already been tokenized
*
* INPUTS:
*    obj == list object to check
*    keyobj == key leaf object to find 
*
* RETURNS:
*   int32   [0..N-1] key number or -1 if not a key in this obj
*********************************************************************/
static int32
    get_key_number (const obj_template_t *obj,
		    const obj_template_t *keyobj)
{
    const obj_key_t       *key;
    int32                  keynum;

    keynum = 0;

    key = obj_first_ckey(obj);
    while (key) {
	if (key->keyobj == keyobj) {
	    return keynum;
	}

	keynum++;
	key = obj_next_ckey(key);
    }

    return -1;

} /* get_key_number */


/********************************************************************
* FUNCTION parse_path_predicate
* 
* Parse the leafref *path-predicate token sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* path-predicate         = "[" *WSP path-equality-expr *WSP "]"
*
* path-equality-expr     = node-identifier *WSP "=" *WSP path-key-expr
*
* path-key-expr          = current-function-invocation "/"
*                          rel-path-keyexpr
*
* node-identifier        = [prefix ":"] identifier
*
* current-function-invocation = 'current()'
*
* For instance-identifiers (XP_FL_INSTANCEID) the 
* following syntax is used:
*
* path-predicate        = "[" *WSP instpath-equality-expr *WSP "]"
*
* instpath-equality-expr = node-identifier *WSP "=" *WSP quoted-string
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_path_predicate (xpath_pcb_t *pcb)
{
#define MAX_KEYS   64
    tk_type_t              nexttyp;
    status_t               res;
    boolean                done, leaflist;
    uint64                 keyflags, keybit;
    uint32                 keytotal, keycount, loopcount;
    int32                  keynum;

    res = NO_ERR;
    done = FALSE;
    keyflags = 0;
    keytotal = 0;
    keycount = 0;
    loopcount = 0;
    leaflist = FALSE;

    if (pcb->targobj && pcb->targobj->objtype == OBJ_TYP_LIST) {
	keytotal = obj_key_count(pcb->targobj);
	if (keytotal > MAX_KEYS) {
	    if (pcb->logerrors && 
                ncx_warning_enabled(ERR_NCX_MAX_KEY_CHECK)) {
		log_warn("\nWarning: Only first %u keys in list '%s'"
			 " can be checked in XPath expression", 
			 MAX_KEYS,
			 obj_get_name(pcb->obj));
	    }
	}
    } else if ((pcb->flags & XP_FL_INSTANCEID) && 
               pcb->targobj && 
	       pcb->targobj->objtype == OBJ_TYP_LEAF_LIST) {
	keytotal = 1;
    }
    
    /* make one loop for each step */
    while (!done) {

	leaflist = FALSE;

	/* only used if pcb->obj set and validating expr */
	loopcount++;

	/* get the left bracket '[' */
	res = xpath_parse_token(pcb, TK_TT_LBRACK);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	pcb->varobj = NULL;
	pcb->curmode = XP_CM_KEYVAR;

	/* get the node-identifier next */
	res = parse_node_identifier(pcb);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* validate the variable object foo in [foo = expr] */
	if (pcb->obj && pcb->varobj) {
	    if ((pcb->flags & XP_FL_INSTANCEID) &&
		pcb->varobj->objtype == OBJ_TYP_LEAF_LIST) {
		if (!(pcb->targobj && pcb->targobj == pcb->varobj)) {
		    res = ERR_NCX_WRONG_INDEX_TYPE;
		    if (pcb->logerrors) {
			log_error("\nError: wrong index type '%s' for "
				  "leaf-list '%s'",
				  obj_get_typestr(pcb->varobj),
				  obj_get_name(pcb->targobj));
			ncx_print_errormsg(pcb->tkc, NULL, res);
		    }
		} else {
		    leaflist = TRUE;
		}
	    } else if (pcb->varobj->objtype != OBJ_TYP_LEAF) {
		res = ERR_NCX_WRONG_TYPE;
		if (pcb->logerrors) {
		    log_error("\nError: path predicate found is %s '%s'",
			      obj_get_typestr(pcb->varobj),
			      obj_get_name(pcb->varobj));
		    ncx_mod_exp_err(pcb->tkc, 
				    pcb->objmod, 
				    res, "leaf");
		}
		done = TRUE;
		continue;
	    }

	    if (leaflist) {
		keynum = 0;
	    } else {
		keynum = get_key_number(pcb->targobj, pcb->varobj);
	    }
	    if (keynum == -1) {
		SET_ERROR(ERR_INTERNAL_VAL);
	    } else if (keynum < MAX_KEYS) {
		keybit = (uint64)(1 << keynum);
		if (keyflags & keybit) {
		    res = ERR_NCX_EXTRA_PARM;
		    if (pcb->logerrors) {
			log_error("\nError: key '%s' already specified "
				  "in XPath expression for object '%s'",
				  obj_get_name(pcb->varobj),
				  obj_get_name(pcb->targobj));
			ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		    }
		} else {
		    keycount++;
		    keyflags |= keybit;
		}
	    } else {
		if (pcb->logerrors &&
                    ncx_warning_enabled(ERR_NCX_MAX_KEY_CHECK)) {
		    log_warn("\nWarning: Key '%s' skipped "
                             "in validation test",
			     obj_get_name(pcb->varobj));
		}
	    }
	} 

	/* get the equals sign '=' */
	res = xpath_parse_token(pcb, TK_TT_EQUAL);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	if (pcb->flags & XP_FL_INSTANCEID) {
	    pcb->altobj = NULL;

	    /* parse literal */
	    res = TK_ADV(pcb->tkc);
	    if (res == NO_ERR) {
		if (!(TK_CUR_TYP(pcb->tkc) == TK_TT_QSTRING ||
		      TK_CUR_TYP(pcb->tkc) == TK_TT_SQSTRING)) {
		    res = ERR_NCX_WRONG_TKTYPE;
		}
	    }

	    if (res != NO_ERR) {
		if (pcb->logerrors) {
		    log_error("\nError: invalid predicate in "
			      "expression '%s'", pcb->exprstr);
		    ncx_print_errormsg(pcb->tkc, NULL, res);
		}
		done = TRUE;
		continue;
	    }
	} else {
	    /* get the path-key-expr next */
	    res = parse_path_key_expr(pcb);
	    if (res != NO_ERR) {
		done = TRUE;
		continue;
	    }
	}

	/* check the object specified in the key expression */
	if (pcb->obj && pcb->altobj) {
	    if (pcb->altobj->objtype != OBJ_TYP_LEAF) {
		res = ERR_NCX_WRONG_TYPE;
		if (pcb->logerrors) {
		    log_error("\nError: path target found is %s '%s'",
			      obj_get_typestr(pcb->altobj),
			      obj_get_name(pcb->altobj));
		    ncx_mod_exp_err(pcb->tkc, 
				    pcb->objmod, 
				    res, 
                                    "leaf");
		}
		done = TRUE;
		continue;
	    } else {
		/* check the predicate for errors */
		if (pcb->altobj == pcb->obj) {
		    res = ERR_NCX_DEF_LOOP;
		    if (pcb->logerrors) {
			log_error("\nError: path target '%s' is set to "
				  "the target object",
				  obj_get_name(pcb->altobj));
			ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		    }
		    done = TRUE;
		    continue;
		} else if (pcb->altobj == pcb->varobj) {
		    res = ERR_NCX_DEF_LOOP;
		    if (pcb->logerrors) {
			log_error("\nError: path target '%s' is set to "
				  "the key leaf object",
				  obj_get_name(pcb->altobj));
			ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		    }
		    done = TRUE;
		    continue;
		}
	    }
	}

	/* get the right bracket ']' */
	res = xpath_parse_token(pcb, TK_TT_RBRACK);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	pcb->curmode = XP_CM_TARGET;

	/* check the next token;
	 * it may be the start of another path-predicate '['
	 */
	nexttyp = tk_next_typ(pcb->tkc);
	if (nexttyp != TK_TT_LBRACK) {
	    done = TRUE;
	} /* else keep going to the next path-predicate */
    }

    if (pcb->obj) {
	if (loopcount != keytotal) {
	    if (keycount < keytotal) {
		if (pcb->flags & XP_FL_SCHEMA_INSTANCEID) {
		    /* schema-instance allowed to skip keys */
		    ;   
		} else {
		    /* regular instance-identifier must have all keys */
		    res = ERR_NCX_MISSING_INDEX;
		    if (pcb->logerrors) {
			log_error("\nError: missing key components in"
				  " XPath expression for list '%s'",
				  obj_get_name(pcb->targobj));
			ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		    }
		}
	    } else {
		res = ERR_NCX_EXTRA_VAL_INST;
		if (pcb->logerrors) {
		    log_error("\nError: extra key components in"
			      " XPath expression for list '%s'",
			      obj_get_name(pcb->targobj));
		    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		}
	    }
	}
    }

    return res;

}  /* parse_path_predicate */


/********************************************************************
* FUNCTION parse_absolute_path
* 
* Parse the leafref path-arg string
* It has already been tokenized
*
* The exact syntax below is not followed!!!
* The first '/' to start the absolute path
* is allowed to be omitted in XPath, so
* allow that here as well
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* absolute-path          = 1*("/" (node-identifier *path-predicate))
*
* path-predicate         = "[" *WSP path-equality-expr *WSP "]"
*
* path-equality-expr     = node-identifier *WSP "=" *WSP path-key-expr
*
* path-key-expr          = current-function-invocation "/"
*                          rel-path-keyexpr
*
* node-identifier        = [prefix ":"] identifier
*
* current-function-invocation = 'current()'
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_absolute_path (xpath_pcb_t *pcb)
{
    tk_type_t    nexttyp;
    status_t     res;
    boolean      done, first;

    res = NO_ERR;
    done = FALSE;
    first = TRUE;

    /* make one loop for each step */
    while (!done) {
        /* get  the first token in the step, '/' */
        if (first) {
            /* allow the first '/' to be omitted */
            if (tk_next_typ(pcb->tkc) == TK_TT_FSLASH) {
                res = xpath_parse_token(pcb, TK_TT_FSLASH);
            }
            first = FALSE;
        } else {
            /* get  the first token in the step, '/' */
            res = xpath_parse_token(pcb, TK_TT_FSLASH);
        }
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* get the node-identifier next */
	res = parse_node_identifier(pcb);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* check the next token;
	 * it may be the start of a path-predicate '['
	 * or the start of another step '/'
	 */
	nexttyp = tk_next_typ(pcb->tkc);
	if (nexttyp == TK_TT_LBRACK) {
	    res = parse_path_predicate(pcb);
	    if (res != NO_ERR) {
		done = TRUE;
		continue;
	    }

	    nexttyp = tk_next_typ(pcb->tkc);
	    if (nexttyp != TK_TT_FSLASH) {
		done = TRUE;
	    }
	} else if (nexttyp != TK_TT_FSLASH) {
	    done = TRUE;
	} /* else keep going to the next step */
    }

    /* check that the string ended properly */
    if (res == NO_ERR) {
	nexttyp = tk_next_typ(pcb->tkc);
	if (nexttyp != TK_TT_NONE) {
	    res = ERR_NCX_INVALID_TOKEN;
	    if (pcb->logerrors) {
		log_error("\nError: wrong token at end of absolute-path '%s'",
			  tk_get_token_name(nexttyp));
		ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	    }
	}
    }

    return res;

}  /* parse_absolute_path */


/********************************************************************
* FUNCTION parse_relative_path
* 
* Parse the leafref relative-path string
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* relative-path          = descendant-path /
*                          (".." "/"
*                          *relative-path)
*
* descendant-path        = node-identifier *path-predicate
*                          absolute-path
*
* Real implementation:
*
* relative-path          = *(".." "/") descendant-path
*
* descendant-path        = node-identifier *path-predicate
*                          [absolute-path]
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_relative_path (xpath_pcb_t *pcb)
{
    tk_type_t    nexttyp;
    status_t     res;

    res = NO_ERR;

    /* check the next token;
     * it may be the start of a '../' pair or a
     * descendant-path (node-identifier)
     */
    nexttyp = tk_next_typ(pcb->tkc);
    while (nexttyp == TK_TT_RANGESEP && res == NO_ERR) {
	res = xpath_parse_token(pcb, TK_TT_RANGESEP);
	if (res == NO_ERR) {
	    res = xpath_parse_token(pcb, TK_TT_FSLASH);
	    if (res == NO_ERR) {
		if (pcb->obj) {
		    res = move_up_obj(pcb);
		}

		/* check the next token;
		 * it may be the start of another ../ pair
		 * or a node identifier
		 */
		nexttyp = tk_next_typ(pcb->tkc);
	    }
	}
    }

    if (res == NO_ERR) {
	/* expect a node identifier first */
	res = parse_node_identifier(pcb);
	if (res == NO_ERR) {
	    /* check the next token;
	     * it may be the start of a path-predicate '['
	     * or the start of another step '/'
	     */
	    nexttyp = tk_next_typ(pcb->tkc);
	    if (nexttyp == TK_TT_LBRACK) {
		res = parse_path_predicate(pcb);
	    }

	    if (res == NO_ERR) {
		/* check the next token;
		 * it may be the start of another step '/'
		 */
		nexttyp = tk_next_typ(pcb->tkc);
		if (nexttyp == TK_TT_FSLASH) {
		    res = parse_absolute_path(pcb);
		}
	    }
	}
    }
	    
    return res;

}  /* parse_relative_path */


/********************************************************************
* FUNCTION parse_path_arg
* 
* Parse the leafref path-arg string
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* path-arg-str           = < a string which matches the rule
*                            path-arg >
*
* path-arg               = absolute-path / relative-path
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_path_arg (xpath_pcb_t *pcb)
{
    tk_type_t  nexttyp;

    nexttyp = tk_next_typ(pcb->tkc);
    if (nexttyp == TK_TT_FSLASH) {
	pcb->flags |= XP_FL_ABSPATH;
	return parse_absolute_path(pcb);
    } else {
	return parse_relative_path(pcb);
    }

}  /* parse_path_arg */

/********************************************************************
* FUNCTION parse_keyexpr
* 
* Parse the key attribute expression
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* keyexprr          = 1*path-predicate
*
* path-predicate         = "[" *WSP path-equality-expr *WSP "]"
*
* path-equality-expr     = node-identifier *WSP "=" *WSP quoted-string
*
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_keyexpr (xpath_pcb_t *pcb)
{
    tk_type_t    nexttyp;
    status_t     res;
    boolean      done;

    res = NO_ERR;
    done = FALSE;

    /* make one loop for each step */
    while (!done) {
	/* check the next token;
	 * it may be the start of a path-predicate '['
	 * or the start of another step '/'
	 */
	nexttyp = tk_next_typ(pcb->tkc);
	if (nexttyp == TK_TT_LBRACK) {
	    res = parse_path_predicate(pcb);
	    if (res != NO_ERR) {
		done = TRUE;
	    }
	} else if (nexttyp == TK_TT_NONE) {
	    done = TRUE;
	    continue;
	} else {
	    res = ERR_NCX_INVALID_VALUE;
	    done = TRUE;
	    if (pcb->logerrors) {
		log_error("\nError: wrong token in key-expr '%s'",
			  pcb->exprstr);
		ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	    }
	}
    }

    return res;

}  /* parse_keyexpr */


/************    E X T E R N A L   F U N C T I O N S    ************/


/********************************************************************
* FUNCTION xpath_yang_parse_path
* 
* Parse the leafref path as a leafref path
*
* DOES NOT VALIDATE PATH NODES USED IN THIS PHASE
* A 2-pass validation is used in case the path expression
* is defined within a grouping.  This pass is
* used on all objects, even in groupings
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
*
* INPUTS:
*    tkc == parent token chain (may be NULL)
*    mod == module in progress
*    source == context for this expression
*              XP_SRC_LEAFREF or XP_SRC_INSTANCEID
*    pcb == initialized xpath parser control block
*           for the leafref path; use xpath_new_pcb
*           to initialize before calling this fn.
*           The pcb->exprstr field must be set
*
* OUTPUTS:
*   pcb->tkc is filled and then validated for well-formed
*   leafref or instance-identifier syntax
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_yang_parse_path (tk_chain_t *tkc,
			   ncx_module_t *mod,
			   xpath_source_t source,
			   xpath_pcb_t *pcb)
{
    status_t       res;
    uint32         linenum, linepos;

#ifdef DEBUG
    if (!pcb || !pcb->exprstr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (source != XP_SRC_LEAFREF && source != XP_SRC_INSTANCEID) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    pcb->logerrors = TRUE;
    linenum = (tkc) ? TK_CUR_LNUM(tkc) : 1;
    linepos = (tkc) ? TK_CUR_LPOS(tkc) : 1;

    /* before all objects are known, only simple validation
     * is done, and the token chain is saved for reuse
     * each time the expression is evaluated
     */
    pcb->tkc = tk_tokenize_xpath_string(mod, 
					pcb->exprstr, 
					linenum,
					linepos,
					&res);
    if (!pcb->tkc || res != NO_ERR) {
	if (pcb->logerrors) {
	    log_error("\nError: Invalid path string '%s'",
		      pcb->exprstr);
	    ncx_print_errormsg(tkc, mod, res);
	}
	return res;
    }

    /* the module that contains the leafref is the one
     * that will always be used to resolve prefixes
     * within the XPath expression
     */
    pcb->mod = mod;
    pcb->source = source;
    if (source == XP_SRC_INSTANCEID) {
	pcb->flags |= XP_FL_INSTANCEID;
    }
	
    /* since the pcb->obj is not set, this validation
     * phase will skip identifier tests, predicate tests
     * and completeness tests
     */
    pcb->parseres = parse_path_arg(pcb);

    /* the expression will not be processed further if the
     * parseres is other than NO_ERR
     */
    return pcb->parseres;

}  /* xpath_yang_parse_path */


/********************************************************************
* FUNCTION xpath_yang_validate_path
* 
* Validate the previously parsed leafref path
*   - QNames are valid
*   - object structure referenced is valid
*   - objects are all 'config true'
*   - target object is a leaf
*   - leafref represents a single instance
*
* A 2-pass validation is used in case the path expression
* is defined within a grouping.  This pass is
* used only on cooked (real) objects
*
* Called after all 'uses' and 'augment' expansion
* so validation against cooked object tree can be done
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    mod == module containing the 'obj' (in progress)
*        == NULL if no object in progress
*    obj == object using the leafref data type
*    pcb == the leafref parser control block, possibly
*           cloned from from the typdef
*    schemainst == TRUE if ncx:schema-instance string
*               == FALSE to use the pcb->source field 
*                  to determine the exact parse mode
*    leafobj == address of the return target object
*
* OUTPUTS:
*   *leafobj == the target leaf found by parsing the path (NO_ERR)
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_yang_validate_path (ncx_module_t *mod,
			      const obj_template_t *obj,
			      xpath_pcb_t *pcb,
			      boolean schemainst,
			      const obj_template_t **leafobj)
{
    status_t          res;
    boolean           doerror;
    ncx_btype_t       btyp;

#ifdef DEBUG
    if (!obj || !pcb || !leafobj || !pcb->exprstr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!pcb->tkc) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    if (schemainst) {
	/* relax errors for missing keys in instance-identifier */
	pcb->flags |= XP_FL_SCHEMA_INSTANCEID;
    }

    pcb->logerrors = TRUE;
    *leafobj = NULL;

    if (pcb->parseres != NO_ERR) {
	/* errors already reported, skip this one */
	return NO_ERR;
    }

    pcb->docroot = ncx_get_gen_root();
    if (!pcb->docroot) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    tk_reset_chain(pcb->tkc);
    pcb->objmod = mod;
    pcb->obj = obj;
    pcb->targobj = NULL;
    pcb->altobj = NULL;
    pcb->varobj = NULL;
    pcb->curmode = XP_CM_TARGET;

    /* validate the XPath expression against the 
     * full cooked object tree
     */
    pcb->validateres = parse_path_arg(pcb);

    /* check leafref is config but target is not */
    if (pcb->validateres == NO_ERR && pcb->targobj) {
	/* return this object even if errors
	 * it will get ignored unless return NO_ERR
	 */
	*leafobj = pcb->targobj;

	/* make sure the config vs. non-config rules are followed */
	if (obj_get_config_flag(obj) &&
	    !obj_get_config_flag(pcb->targobj)) {

	    doerror = TRUE;

	    btyp = obj_get_basetype(obj);

	    /* only some instance-identifier and leafref 
	     * objects will ever return TRUE 
	     */
	    if ((btyp == NCX_BT_INSTANCE_ID || 
		 btyp == NCX_BT_LEAFREF) && 
		!typ_get_constrained(obj_get_ctypdef(obj))) {
		doerror = FALSE;
	    }

	    /* yangcli_util/get_instanceid_parm sets the
	     * object field to the config root instead
	     * of the leafref object, so this hack is used
	     * to suppress the error intended for leafrefs
	     */
	    if (obj_is_root(obj)) {
		doerror = FALSE;
	    }

	    if (doerror) {
		res = ERR_NCX_NOT_CONFIG;
		if (pcb->logerrors) {
		    log_error("\nError: XPath target '%s' for leafref '%s'"
			      " must be a config object",
			      obj_get_name(pcb->targobj),
			      obj_get_name(obj));
		    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		}
		pcb->validateres = res;
	    }
	}

	/* check for a leaf, then if that is OK */
	if (pcb->source == XP_SRC_LEAFREF) {
	    if (!obj_get_ctypdef(pcb->targobj) ||  
		pcb->targobj->objtype != OBJ_TYP_LEAF) {
		res = ERR_NCX_INVALID_VALUE;
		if (pcb->logerrors) {
		    log_error("\nError: invalid path target anyxml '%s'",
			      obj_get_name(pcb->targobj));
		    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		}
		pcb->validateres = res;
	    }
	}

	/* this test is probably not worth doing,
	 * but check for the loop anyway
	 */
	if (pcb->targobj == pcb->obj) {
	    res = ERR_NCX_DEF_LOOP;
	    if (pcb->logerrors) {
		log_error("\nError: path target '%s' is set to "
			  "the target object",
			  obj_get_name(pcb->targobj));
		ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	    }
	    pcb->validateres = res;
	}

	/**** NEED TO CHECK THE CORNER CASE WHERE A LEAFREF
	 **** AND/OR INSTANCE-IDENTIFIER COMBOS CAUSE A LOOP
	 **** THAT WILL PREVENT AGENT VALIDATION FROM COMPLETING
	 ****/
    }

    return pcb->validateres;

}  /* xpath_yang_validate_path */


/********************************************************************
* FUNCTION xpath_yang_validate_xmlpath
* 
* Validate an instance-identifier expression
* within an XML PDU context
*
* INPUTS:
*    reader == XML reader to use
*    pcb == initialized XPath parser control block
*           with a possibly unchecked pcb->exprstr.
*           This function will  call tk_tokenize_xpath_string
*           if it has not already been called.
*    logerrors == TRUE if log_error and ncx_print_errormsg
*                  should be used to log XPath errors and warnings
*                 FALSE if internal error info should be recorded
*                 in the xpath_result_t struct instead
*                !!! use FALSE unless DEBUG mode !!!
*    targobj == address of return target object
*     
* OUTPUTS:
*   *targobj is set to the object that this instance-identifier
*    references, if NO_ERR
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_yang_validate_xmlpath (xmlTextReaderPtr reader,
				 xpath_pcb_t *pcb,
				 const obj_template_t *pathobj,
				 boolean logerrors,
				 const obj_template_t **targobj)
{
    status_t  res;

#ifdef DEBUG
    if (!reader || !pcb || !targobj || !pcb->exprstr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    *targobj = NULL;
    pcb->logerrors = logerrors;

    if (pcb->tkc) {
	tk_reset_chain(pcb->tkc);
    } else {
	pcb->tkc = tk_tokenize_xpath_string(NULL, 
					    pcb->exprstr, 
					    0, 
					    0, 
					    &res);
    }

    if (!pcb->tkc || res != NO_ERR) {
	if (pcb->logerrors) {
	    log_error("\nError: Invalid path string '%s'",
		      pcb->exprstr);
	}
	pcb->parseres = res;
	return res;
    }

    pcb->docroot = ncx_get_gen_root();
    if (!pcb->docroot) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (pathobj && obj_is_schema_instance_string(pathobj)) {
	pcb->flags |= XP_FL_SCHEMA_INSTANCEID;
    } else {
	pcb->flags |= XP_FL_INSTANCEID;
    }

    pcb->obj = pcb->docroot;
    pcb->reader = reader;

    pcb->source = XP_SRC_XML;
    pcb->objmod = NULL;
    pcb->val = NULL;
    pcb->val_docroot = NULL;
    pcb->targobj = NULL;
    pcb->altobj = NULL;
    pcb->varobj = NULL;
    pcb->curmode = XP_CM_TARGET;
    pcb->flags |= XP_FL_ABSPATH;

    /* validate the XPath expression against the 
     * full cooked object tree
     */
    pcb->validateres = parse_absolute_path(pcb);

    /* check leafref is config but target is not */
    if (pcb->validateres == NO_ERR && pcb->targobj) {
	*targobj = pcb->targobj;
    }

    pcb->reader = NULL;
    return pcb->validateres;

}  /* xpath_yang_validate_xmlpath */


/********************************************************************
* FUNCTION xpath_yang_validate_xmlkey
* 
* Validate a key XML attribute value given in
* an <edit-config> operation with an 'insert' attribute
* Check that a complete set of predicates is present
* for the specified list of leaf-list
*
* INPUTS:
*    reader == XML reader to use
*    pcb == initialized XPath parser control block
*           with a possibly unchecked pcb->exprstr.
*           This function will  call tk_tokenize_xpath_string
*           if it has not already been called.
*    obj == list or leaf-list object associated with
*           the pcb->exprstr predicate expression
*           (MAY be NULL if first-pass parsing and
*           object is not known yet -- parsed in XML attribute)
*    logerrors == TRUE if log_error and ncx_print_errormsg
*                  should be used to log XPath errors and warnings
*                 FALSE if internal error info should be recorded
*                 in the xpath_result_t struct instead
*                !!! use FALSE unless DEBUG mode !!!
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_yang_validate_xmlkey (xmlTextReaderPtr reader,
				xpath_pcb_t *pcb,
				const obj_template_t *obj,
				boolean logerrors)
{
    status_t  res;

#ifdef DEBUG
    if (!reader || !pcb || !pcb->exprstr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    pcb->logerrors = logerrors;

    if (pcb->tkc) {
	tk_reset_chain(pcb->tkc);
    } else {
	pcb->tkc = tk_tokenize_xpath_string(NULL, 
					    pcb->exprstr, 
					    0, 
					    0, 
					    &res);
    }

    if (!pcb->tkc || res != NO_ERR) {
	if (pcb->logerrors) {
	    log_error("\nError: Invalid path string '%s'",
		      pcb->exprstr);
	}
	pcb->parseres = res;
	return res;
    }

    pcb->docroot = ncx_get_gen_root();
    if (!pcb->docroot) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    pcb->obj = obj;
    pcb->reader = reader;
    pcb->flags = XP_FL_INSTANCEID;
    pcb->source = XP_SRC_XML;
    pcb->objmod = NULL;
    pcb->val = NULL;
    pcb->val_docroot = NULL;
    pcb->targobj = obj;
    pcb->altobj = NULL;
    pcb->varobj = NULL;
    pcb->curmode = XP_CM_TARGET;

    /* validate the XPath expression against the 
     * full cooked object tree
     */
    pcb->validateres = parse_keyexpr(pcb);

    pcb->reader = NULL;

    return pcb->validateres;

}  /* xpath_yang_validate_xmlkey */


/********************************************************************
* FUNCTION xpath_yang_make_instanceid_val
* 
* Make a value subtree out of an instance-identifier
* Used by yangcli to send PDUs from CLI target parameters
*
* The XPath pcb must be previously parsed and found valid
* It must be an instance-identifier value, 
* not a leafref path
*
* INPUTS:
*    pcb == the leafref parser control block, possibly
*           cloned from from the typdef
*    retres == address of return status (may be NULL)
*    deepest == address of return deepest node created (may be NULL)
*
* OUTPUTS:
*   if (non-NULL)
*       *retres == return status
*       *deepest == pointer to end of instance-id chain node
* RETURNS:
*   malloced value subtree representing the instance-identifier
*   in internal val_value_t data structures
*********************************************************************/
val_value_t *
    xpath_yang_make_instanceid_val (xpath_pcb_t *pcb,
				    status_t *retres,
				    val_value_t **deepest)
{
    val_value_t          *childval, *top, *curtop;
    const obj_template_t *curobj, *childobj;
    const xmlChar        *objprefix, *objname, *modname;
    status_t              res;
    boolean               done, done2;

#ifdef DEBUG
    if (!pcb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (!pcb->tkc) {
	if (retres) {
	    *retres = SET_ERROR(ERR_INTERNAL_VAL);
	}
	return NULL;
    }
#endif

    if (retres) {
	*retres = NO_ERR;
    }
    if (deepest) {
	*deepest = NULL;
    }

    top = NULL;
    curtop = NULL;

    if (pcb->parseres != NO_ERR) {
	/* errors already reported, skip this one */
	if (retres) {
	    *retres = pcb->parseres;
	}
	return NULL;
    }

    /* get first token */
    tk_reset_chain(pcb->tkc);
    res = TK_ADV(pcb->tkc);
    if (res != NO_ERR) {
	if (retres) {
	    *retres = res;
	}
	return NULL;
    }

    /* get the start object */
    if (TK_CUR_TYP(pcb->tkc) == TK_TT_FSLASH) {
	/* absolute path, use objroot to start */
	curobj = pcb->docroot;
	res = TK_ADV(pcb->tkc);
	if (res != NO_ERR) {
	    if (retres) {
		*retres = res;
	    }
	    return NULL;
	}
    } else {
	/* relative path, use context object to start */
	curobj = pcb->obj;
    }
    if (!curobj) {
	SET_ERROR(ERR_INTERNAL_VAL);
	if (retres) {
	    *retres = ERR_INTERNAL_VAL;
	}
	return NULL;
    }

    /* get all the path steps */
    res = NO_ERR;
    done = FALSE;
    while (!done && res == NO_ERR) {
	/* this is expected to be a QName or LCName identifier */
	objprefix = TK_CUR_MOD(pcb->tkc);
	if (objprefix) {
	    modname = xmlns_get_module
		(xmlns_find_ns_by_prefix(objprefix));
	} else {
	    modname = NULL;
	}
	objname = TK_CUR_VAL(pcb->tkc);

	/* find the child node and add it to the curtop */
	if (obj_is_root(curobj)) {
	    if (modname) {
		childobj = 
		    ncx_find_object(ncx_find_module(modname, NULL), 
				    objname);
	    } else {
		childobj = ncx_find_any_object(objname);
	    }
	} else {
	    childobj = obj_find_child(curobj, modname, objname);
	}
	if (!childobj) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	    continue;
	}

	childval = val_new_value();
	if (!childval) {
	    res = ERR_INTERNAL_MEM;
	    continue;
	}
	val_init_from_template(childval, childobj);

	if (curtop) {
	    val_add_child(childval, curtop);
	} else {
	    top = childval;
	}
	curtop = childval;
	curobj = childobj;
	if (deepest) {
	    *deepest = curtop;
	}

	/* move on to the next token */
	res = TK_ADV(pcb->tkc);
	if (res != NO_ERR) {
	    /* reached end of the line at an OK point */
	    res = NO_ERR;
	    done = TRUE;
	    continue;
	}

	switch (TK_CUR_TYP(pcb->tkc)) {
	case TK_TT_FSLASH:
	    /* normal identifier expected next, go back to start */
	    res = TK_ADV(pcb->tkc);
	    continue;
	case TK_TT_LBRACK:
	    /* predicate expected next, keep going */
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    continue;
	}

	done2 = FALSE;
	while (!done2 && res == NO_ERR) {
	    /* got a start of predicate, get the key leaf name */
	    res = TK_ADV(pcb->tkc);
	    if (res != NO_ERR) {
		continue;
	    }

	    /* this is expected to be a QName or LCName identifier */
	    objprefix = TK_CUR_MOD(pcb->tkc);
	    if (objprefix) {
		modname = xmlns_get_module
		    (xmlns_find_ns_by_prefix(objprefix));
	    } else {
		modname = NULL;
	    }
	    objname = TK_CUR_VAL(pcb->tkc);

	    /* find the child node and add it to the curtop */
	    childobj = obj_find_child(curobj, modname, objname);
	    if (!childobj) {
		res = ERR_NCX_DEF_NOT_FOUND;
		continue;
	    }
	    childval = val_new_value();
	    if (!childval) {
		res = ERR_INTERNAL_MEM;
		continue;
	    }
	    val_init_from_template(childval, childobj);
	    val_add_child(childval, curtop);

	    /* get the = sign */
	    res = TK_ADV(pcb->tkc);
	    if (res != NO_ERR) {
		continue;
	    }
	    if (TK_CUR_TYP(pcb->tkc) != TK_TT_EQUAL) {
		res = SET_ERROR(ERR_INTERNAL_VAL);
		continue;
	    }

	    /* get the key leaf value */
	    res = TK_ADV(pcb->tkc);
	    if (res != NO_ERR) {
		continue;
	    }
	    if (!TK_CUR_STR(pcb->tkc)) {
		res = SET_ERROR(ERR_INTERNAL_VAL);
		continue;
	    }

	    /* set the new leaf with the value */
	    res = val_set_simval(childval,
				 obj_get_ctypdef(childobj),
				 obj_get_nsid(childobj),
				 obj_get_name(childobj),
				 TK_CUR_VAL(pcb->tkc));
	    if (res != NO_ERR) {
		continue;
	    }

	    /* get the closing predicate bracket */
	    res = TK_ADV(pcb->tkc);
	    if (res != NO_ERR) {
		continue;
	    }
	    if (TK_CUR_TYP(pcb->tkc) != TK_TT_RBRACK) {
		res = SET_ERROR(ERR_INTERNAL_VAL);
		continue;
	    }

	    /* check any more predicates or path steps */
	    res = TK_ADV(pcb->tkc);
	    if (res != NO_ERR) {
		/* no more steps, stopped at an OK spot */
		done = TRUE;
		done2 = TRUE;
		res = NO_ERR;
		continue;
	    }

	    switch (TK_CUR_TYP(pcb->tkc)) {
	    case TK_TT_LBRACK:
		/* get another predicate */
		break;
	    case TK_TT_FSLASH:
		/* done with predicates for now 
		 * setup the next path step
		 */
		res = TK_ADV(pcb->tkc);
		if (res != NO_ERR) {
		    continue;
		}
		done2 = TRUE;
		break;
	    default:
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
    }


    /* cleanup and exit */
    if (res != NO_ERR && top) {
	val_free_value(top);
	top = NULL;
    }

    if (LOGDEBUG2 && top) {
	log_debug2("\nval_inst for expr '%s'\n", 
		   pcb->exprstr);
	val_dump_value(top, NCX_DEF_INDENT);
    }

    return top;

}  /* xpath_yang_make_instanceid_val */


/********************************************************************
* FUNCTION xpath_yang_get_namespaces
* 
* Get the namespace URI IDs used in the specified
* XPath expression;
* 
* usually an instance-identifier or schema-instance node
* but this function simply reports all the TK_TT_MSTRING
* tokens that have an nsid set
*
* The XPath pcb must be previously parsed and found valid
*
* INPUTS:
*    pcb == the XPath parser control block to use
*    nsid_array == address of return array of xmlns_id_t
*    max_nsids == number of NSIDs that can be held
*    num_nsids == address of return number of NSIDs
*                 written to the buffer. No duplicates
*                 will be present in the buffer
*
* OUTPUTS:
*    nsid_array[0..*num_nsids] == returned NSIDs used
*       in the XPath expression
*    *num_nsids == number of NSIDs written to the array
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_yang_get_namespaces (const xpath_pcb_t *pcb,
			       xmlns_id_t *nsid_array,
			       uint32 max_nsids,
			       uint32 *num_nsids)
{
    const tk_token_t     *tk;
    boolean               done, found;
    uint32                i, next;
    xmlns_id_t            cur_nsid;
    status_t              res;

#ifdef DEBUG
    if (!pcb || !nsid_array || !num_nsids) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!pcb->tkc) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    if (max_nsids == 0) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    if (pcb->parseres != NO_ERR) {
	return  pcb->parseres;
    }

    res = NO_ERR;
    next = 0;
    *num_nsids = 0;

    done = FALSE;
    for (tk = (const tk_token_t *)dlq_firstEntry(&pcb->tkc->tkQ);
         tk != NULL && !done;
         tk = (const tk_token_t *)dlq_nextEntry(tk)) {

	/* only MSTRING and QVARBIND tokens have relevant NSID fields
	 * as used in instance-identifier or schema-instance
	 * path syntax
	 */
	switch (tk->typ) {
	case TK_TT_MSTRING:
	case TK_TT_QVARBIND:
        case TK_TT_NCNAME_STAR:
	    break;
	default:
	    continue;
	}

	cur_nsid = tk->nsid;
	if (cur_nsid == 0) {
	    /* this token never had an NSID set */
	    continue;
	}

	/* check if this NSID already recorded */
	found = FALSE;
	for (i = 0; i < next && !found; i++) {
	    if (nsid_array[i] == cur_nsid) {
		found = TRUE;
	    }
	}
	if (found) {
	    continue;
	}

	/* need to add this entry
	 * check if there would be a buffer overflow
	 */
	if (next >= max_nsids) {
	    res = ERR_BUFF_OVFL;
	    done = TRUE;
	} else {
	    nsid_array[next++] = cur_nsid;
	}
    }

    *num_nsids = next;

    return res;

}  /* xpath_yang_get_namespaces */


/* END xpath_yang.c */
