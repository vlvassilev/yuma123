/*  FILE: xpath_keyref.c

    Schema and data model Xpath search support
		
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

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_grp
#include "grp.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
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

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xpath_keyref
#include "xpath_keyref.h"
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
* FUNCTION parse_token
* 
* Parse the keyref token sequence for a specific token type
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*    tktyp == expected token type
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_token (xpath_pcb_t *pcb,
		 tk_type_t  tktype)
{
    status_t     res;

    /* get the next token */
    res = TK_ADV(pcb->tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	return res;
    }

    if (TK_CUR_TYP(pcb->tkc) != tktype) {
	res = ERR_NCX_WRONG_TKTYPE;
	ncx_mod_exp_err(pcb->tkc, pcb->mod, res,
			tk_get_token_name(tktype));
	return res;
    }

    return NO_ERR;

}  /* parse_token */


/********************************************************************
* FUNCTION set_next_objnode
* 
* Get the object identifier associated with
* QName in an keyref XPath expression
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
		      const xmlChar *nodename)
{
    const obj_template_t  **useobj, *foundobj;
    ncx_module_t           *targmod;
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
    case XP_CM_KEYVAR:
	useobj = &pcb->varobj;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res == NO_ERR) {
	res = xpath_get_curmod_from_prefix(prefix,
					   pcb->objmod,
					   &targmod);
	if (res != NO_ERR) {
	    log_error("\nError: Module for prefix '%s' not found",
		      (prefix) ? prefix : (const xmlChar *)"");
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	return res;
    }

    if (*useobj) {
	if (obj_is_root(*useobj)) {
	    foundobj = 
		obj_find_template_top(targmod,
				      ncx_get_modname(targmod),
				      nodename);
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
    } else if (pcb->abspath) {
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
	if (prefix) {
	    log_error("\nError: object not found '%s:%s'",
		      prefix, nodename);
	} else {
	    log_error("\nError: object not found '%s'",
		      nodename);
	}
	ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
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
	log_error("\nError: parent not found for object '%s'",
		  obj_get_name(*useobj));
	ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
    } else {
	*useobj = foundobj;
    }

    return res;

} /* move_up_obj */


/********************************************************************
* FUNCTION parse_node_identifier
* 
* Parse the keyref node-identifier string
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
    const xmlChar  *prefix;
    ncx_import_t   *import;
    status_t        res;

    prefix = NULL;
    import = NULL;
    res = NO_ERR;

    /* get the next token in the step, node-identifier */
    res = TK_ADV(pcb->tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	return res;
    }

    switch (TK_CUR_TYP(pcb->tkc)) {
    case TK_TT_MSTRING:
	/* pfix:identifier */
	prefix = TK_CUR_MOD(pcb->tkc);
	if (xml_strcmp(pcb->mod->prefix, prefix)) {
	    import = ncx_find_pre_import(pcb->mod, prefix);
	    if (!import) {
		res = ERR_NCX_PREFIX_NOT_FOUND;
		log_error("\nError: import for prefix '%s' not found",
			  prefix);
		ncx_print_errormsg(pcb->tkc, pcb->mod, res);
		break;
	    }
	}
	/* fall through to check QName */
    case TK_TT_TSTRING:
	if (pcb->obj) {
	    res = set_next_objnode(pcb, prefix,
				   TK_CUR_VAL(pcb->tkc));
	} /* else identifier not checked here */
	break;
    default:
	res = ERR_NCX_WRONG_TKTYPE;
	ncx_mod_exp_err(pcb->tkc, pcb->mod, res,
			tk_get_token_name(TK_CUR_TYP(pcb->tkc)));
    }

    return res;

}  /* parse_node_identifier */


/********************************************************************
* FUNCTION parse_current_fn
* 
* Parse the keyref current-function token sequence
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
    res = parse_token(pcb, TK_TT_TSTRING);
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
    res = parse_token(pcb, TK_TT_LPAREN);
    if (res != NO_ERR) {
	return res;
    }

    /* get the right paren ')' */
    res = parse_token(pcb, TK_TT_RPAREN);
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
* Parse the keyref *path-key-expr token sequence
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
    res = parse_token(pcb, TK_TT_FSLASH);
    if (res != NO_ERR) {
	return res;
    }

    /* make one loop for each step of the first part of
     * the rel-path-keyexpr; the '../' token pairs
     */
    while (!done) {
	/* get the parent marker '..' */
	res = parse_token(pcb, TK_TT_RANGESEP);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* get the path separator '/' */
	res = parse_token(pcb, TK_TT_FSLASH);
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
	    res = parse_token(pcb, TK_TT_FSLASH);
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
* Parse the keyref *path-predicate token sequence
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
* INPUTS:
*    pcb == parser control block in progress
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_path_predicate (xpath_pcb_t *pcb)
{
    tk_type_t              nexttyp;
    status_t               res;
    boolean                done;
    uint64                 keyflags, keybit;
    uint32                 keytotal, keycount, loopcount;
    int32                  keynum;

    res = NO_ERR;
    done = FALSE;
    keyflags = 0;
    keytotal = 0;
    keycount = 0;
    loopcount = 0;

    if (pcb->targobj && pcb->targobj->objtype == OBJ_TYP_LIST) {
	keytotal = obj_key_count(pcb->targobj);
	if (keytotal > 64) {
	    log_warn("\nWarning: Only first 64 keys in list '%s'"
		     " can be checked in XPath expression", 
		     obj_get_name(pcb->obj));
	}
    }
    
    /* make one loop for each step */
    while (!done) {

	/* only used if pcb->obj set and validating expr */
	loopcount++;

	/* get the left bracket '[' */
	res = parse_token(pcb, TK_TT_LBRACK);
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
	    if (pcb->varobj->objtype != OBJ_TYP_LEAF) {
		res = ERR_NCX_WRONG_TYPE;
		log_error("\nError: path predicate found is %s '%s'",
			  obj_get_typestr(pcb->varobj),
			  obj_get_name(pcb->varobj));
		ncx_mod_exp_err(pcb->tkc, 
				pcb->objmod, 
				res, "leaf");
		done = TRUE;
		continue;
	    }

	    switch (pcb->source) {
	    case XP_SRC_KEYREF:
		if (!obj_is_key(pcb->varobj)) {
		    res = ERR_NCX_TYPE_NOT_INDEX;
		    log_error("\nError: path predicate '%s' is "
			      "not a key leaf",
			      obj_get_name(pcb->varobj));
		    ncx_mod_exp_err(pcb->tkc, 
				    pcb->objmod, 
				    res, "key leaf");
		    done = TRUE;
		    continue;
		}
		break;
	    case XP_SRC_LEAFREF:
		break;
	    default:
		res = SET_ERROR(ERR_INTERNAL_VAL);
		ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		done = TRUE;
		continue;
	    }

	    keynum = get_key_number(pcb->targobj, pcb->varobj);
	    if (keynum == -1) {
		SET_ERROR(ERR_INTERNAL_VAL);
	    } else if (keynum < 64) {
		keybit = (uint64)(1 << keynum);
		if (keyflags & keybit) {
		    res = ERR_NCX_EXTRA_PARM;
		    log_error("\nError: key '%s' already specified "
			      "in XPath expression for object '%s'",
			      obj_get_name(pcb->varobj),
			      obj_get_name(pcb->targobj));
		    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		} else {
		    keycount++;
		    keyflags |= keybit;
		}
	    } else {
		log_warn("\nWarning: Key '%s' skipped in validation test",
			 obj_get_name(pcb->varobj));
	    }
	} 

	/* get the equals sign '=' */
	res = parse_token(pcb, TK_TT_EQUAL);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* get the path-key-expr next */
	res = parse_path_key_expr(pcb);
	if (res != NO_ERR) {
	    done = TRUE;
	    continue;
	}

	/* check the object specified in the key expression */
	if (pcb->obj && pcb->altobj) {
	    if (pcb->altobj->objtype != OBJ_TYP_LEAF) {
		res = ERR_NCX_WRONG_TYPE;
		log_error("\nError: path target found is %s '%s'",
			  obj_get_typestr(pcb->altobj),
			  obj_get_name(pcb->altobj));
		ncx_mod_exp_err(pcb->tkc, 
				pcb->objmod, 
				res, "leaf");
		done = TRUE;
		continue;
	    } else {
		/* check the predicate for errors */
		if (pcb->altobj == pcb->obj) {
		    res = ERR_NCX_DEF_LOOP;
		    log_error("\nError: path target '%s' is set to "
			      "the target object",
			      obj_get_name(pcb->altobj));
		    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		    done = TRUE;
		    continue;
		} else if (pcb->altobj == pcb->varobj) {
		    res = ERR_NCX_DEF_LOOP;
		    log_error("\nError: path target '%s' is set to "
			      "the key leaf object",
			      obj_get_name(pcb->altobj));
		    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
		    done = TRUE;
		    continue;
		}
	    }
	}

	/* get the right bracket ']' */
	res = parse_token(pcb, TK_TT_RBRACK);
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
		res = ERR_NCX_MISSING_INDEX;
		log_error("\nError: missing key components in"
			  " XPath expression for list '%s'",
			  obj_get_name(pcb->targobj));
	    } else {
		res = ERR_NCX_EXTRA_VAL_INST;
		log_error("\nError: extra key components in"
			  " XPath expression for list '%s'",
			  obj_get_name(pcb->targobj));
	    }
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	}
    }

    return res;

}  /* parse_path_predicate */


/********************************************************************
* FUNCTION parse_absolute_path
* 
* Parse the keyref path-arg string
* It has already been tokenized
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
    boolean      done;

    res = NO_ERR;
    done = FALSE;

    /* make one loop for each step */
    while (!done) {
	/* get  the first token in the step, '/' */
	res = parse_token(pcb, TK_TT_FSLASH);
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
	    log_error("\nError: wrong token at end of absolute-path '%s'",
		      tk_get_token_name(nexttyp));
	    ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	}
    }

    return res;

}  /* parse_absolute_path */


/********************************************************************
* FUNCTION parse_relative_path
* 
* Parse the keyref relative-path string
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
	res = parse_token(pcb, TK_TT_RANGESEP);
	if (res == NO_ERR) {
	    res = parse_token(pcb, TK_TT_FSLASH);
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
* Parse the keyref path-arg string
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
	pcb->abspath = TRUE;
	return parse_absolute_path(pcb);
    } else {
	return parse_relative_path(pcb);
    }

}  /* parse_path_arg */


/************    E X T E R N A L   F U N C T I O N S    ************/


/*******    X P A T H   and   K E Y R E F    S U P P O R T   *******/


/********************************************************************
* FUNCTION xpath_keyref_parse_path
* 
* Parse the keyref path as a keyref path
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
*
* INPUTS:
*    tkc == parent token chain
*    mod == module in progress
*    pcb == initialized xpath parser control block
*           for the keyref path; use xpath_new_pcb
*           to initialize before calling this fn
*
* OUTPUTS:
*   pcb->tkc is filled and then validated
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_keyref_parse_path (tk_chain_t *tkc,
			     ncx_module_t *mod,
			     xpath_pcb_t *pcb)
{
    status_t       res;

#ifdef DEBUG
    if (!tkc || !mod || !pcb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* before all objects are known, only simple validation
     * is done, and the token chain is saved for reuse
     * each time the expression is evaluated
     */
    pcb->tkc = tk_tokenize_xpath_string(mod, pcb->exprstr, 
					TK_CUR_LNUM(tkc),
					TK_CUR_LPOS(tkc),
					&res);
    if (!pcb->tkc || res != NO_ERR) {
	log_error("\nError: Invalid path string '%s'",
		  pcb->exprstr);
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* the module that contains the keyref is the one
     * that will always be used to resolve prefixes
     * within the XPath expression
     */
    pcb->mod = mod;
    pcb->source = XP_SRC_KEYREF;

    /* since the pcb->obj is not set, this validation
     * phase will skip identifier tests, predicate tests
     * and completeness tests
     */
    pcb->parseres = parse_path_arg(pcb);

    /* the expression will not be processed further if the
     * parseres is other than NO_ERR
     */
    return pcb->parseres;

}  /* xpath_keyref_parse_path */


/********************************************************************
* FUNCTION xpath_keyref_validate_path
* 
* Validate the previously parsed keyref path
*   - QNames are valid
*   - object structure referenced is valid
*   - objects are all 'config true'
*   - target object is a leaf
*   - keyref represents a single instance
*
* Called after all 'uses' and 'augment' expansion
* so validation against cooked object tree can be done
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    mod == module containing the 'obj' (in progress)
*    obj == object using the keyref data type
*    pcb == the keyref parser control block from the typdef
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_keyref_validate_path (ncx_module_t *mod,
				const obj_template_t *obj,
				xpath_pcb_t *pcb)
{
    status_t  res;

#ifdef DEBUG
    if (!mod || !obj || !pcb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!pcb->tkc) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

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
    pcb->source = XP_SRC_KEYREF;
    pcb->targobj = NULL;
    pcb->altobj = NULL;
    pcb->varobj = NULL;
    pcb->curmode = XP_CM_TARGET;

    /* validate the XPath expression against the 
     * full cooked object tree
     */
    pcb->validateres = parse_path_arg(pcb);

    /* check keyref is config but target is not */
    if (pcb->validateres == NO_ERR && pcb->targobj) {
	if (obj_get_config_flag(obj) &&
	    !obj_get_config_flag(pcb->targobj)) {
	    res = ERR_NCX_NOT_CONFIG;
	    log_error("\nError: XPath target '%s' for keyref '%s' must be "
		      "a config object",
		      obj_get_name(pcb->targobj),
		      obj_get_name(obj));
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	    pcb->validateres = res;
	}

	switch (pcb->source) {
	case XP_SRC_KEYREF:
	    if (!obj_is_key(pcb->targobj)) {
		res = ERR_NCX_TYPE_NOT_INDEX;
		log_error("\nError: path target '%s' is "
			  "not a key leaf",
			  obj_get_name(pcb->targobj));
		ncx_mod_exp_err(pcb->tkc, 
				pcb->objmod, 
				res, "key leaf");
		pcb->validateres = res;
	    }
	    break;
	case XP_SRC_LEAFREF:
	    break;
	default:
	    ;
	}

	if (pcb->targobj == pcb->obj) {
	    res = ERR_NCX_DEF_LOOP;
	    log_error("\nError: path target '%s' is set to "
		      "the target object",
		      obj_get_name(pcb->targobj));
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	}
    }

    return pcb->validateres;

}  /* xpath_keyref_validate_path */


/********************************************************************
* FUNCTION xpath_keyref_get_value
* 
* Get a pointer to the keyref target value node
*
* INPUTS:
*    mod == module in progress
*    obj == object initiating search, which contains the keyref type
*    pcb == XPath parser control block to use
*    targval == address of return target value (may be NULL)
*
* OUTPUTS:
*   if non-NULL:
*      *targval == pointer to return value node target
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_keyref_get_value (ncx_module_t *mod,
			    obj_template_t *obj,
			    xpath_pcb_t *pcb,
			    val_value_t **targval)
{

#ifdef DEBUG
    if (!mod || !obj || !pcb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /*****/

    return NO_ERR;

}  /* xpath_keyref_get_value */


/* END xpath.c */
