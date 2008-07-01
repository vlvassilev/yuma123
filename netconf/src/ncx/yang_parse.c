/*  FILE: yang_parse.c


   YANG module parser

    YANG modules are parsed in the following steps:

    1) basic tokenization and string processing (tk.c)
    2) first pass module processing, reject bad syntax
    3) resolve ranges, forward references, etc in typedefs
    4) resolve data model definitions (objects, rpcs, notifications)
    5) convert/copy obj_template_t data specifications to 
       typ_def_t representation


*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
26oct07      abb      begun; start from ncx_parse.c


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

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yang_ext
#include "yang_ext.h"
#endif

#ifndef _H_yang_grp
#include "yang_grp.h"
#endif

#ifndef _H_yang_obj
#include "yang_obj.h"
#endif

#ifndef _H_yang_parse
#include "yang_parse.h"
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
#define YANG_PARSE_DEBUG 1
/* #define YANG_PARSE_TK_DEBUG 1 */
/* #define YANG_PARSE_RDLN_DEBUG 1 */
/* #define YANG_PARSE_DEBUG_TRACE 1 */
#endif


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/********************************************************************
* FUNCTION resolve_iappinfo
* 
* Validate the ncx_appinfo_t (extension usage) within
* the includes and imports clauses for this module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc    == token chain
*   mod    == module in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_iappinfo (tk_chain_t  *tkc,
		      ncx_module_t *mod)
{
    ncx_import_t    *imp;
    ncx_include_t   *inc;
    status_t         res, retres;

    retres = NO_ERR;

    for (imp = (ncx_import_t *)dlq_firstEntry(&mod->importQ);
	 imp != NULL;
	 imp = (ncx_import_t *)dlq_nextEntry(imp)) {

	res = ncx_resolve_appinfoQ(tkc, mod, &imp->appinfoQ);
	CHK_EXIT;
    }

    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
	 inc != NULL;
	 inc = (ncx_include_t *)dlq_nextEntry(inc)) {

	res = ncx_resolve_appinfoQ(tkc, mod, &inc->appinfoQ);
	CHK_EXIT;
    }

    return retres;

}  /* resolve_iappinfo */


/********************************************************************
* FUNCTION consume_mod_hdr
* 
* Parse the module header statements
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc    == token chain
*   mod    == module in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_mod_hdr (tk_chain_t  *tkc,
		     ncx_module_t *mod)
{
    const xmlChar *val;
    const char    *expstr;
    tk_type_t      tktyp;
    status_t       res, retres;
    boolean        done, ver, ns, pfix;


    expstr = "module header statement";
    ver = FALSE;
    ns = FALSE;
    pfix = FALSE;
    res = NO_ERR;
    retres = NO_ERR;
    done = FALSE;

    while (!done) {

	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		res = NO_ERR;
	    }
	    continue;
	case TK_TT_RBRACE:
	    TK_BKUP(tkc);
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_YANG_VERSION)) {
	    /* Optional 'yang-version' field is present */
	    if (ver) {
		retres = ERR_NCX_ENTRY_EXISTS;
		ncx_print_errormsg(tkc, mod, retres);
	    }
	    ver = TRUE;

	    /* get the version number */
	    res = ncx_consume_token(tkc, mod, TK_TT_DNUM);
	    if (res != NO_ERR) {
		retres = res;
	    } else {
		if (xml_strcmp(TK_CUR_VAL(tkc), YANG_VERSION_STR)) {
		    retres = ERR_NCX_WRONG_VERSION;
		    ncx_print_errormsg(tkc, mod, retres);
		} else if (!mod->langver) {
		    mod->langver = YANG_VERSION_NUM;
		}
	    }

	    res = yang_consume_semiapp(tkc, mod, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
	    }
        } else if (!xml_strcmp(val, YANG_K_NAMESPACE)) {
	    res = yang_consume_strclause(tkc, mod, &mod->ns,
					 &ns, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
	    }
	} else if (!xml_strcmp(val, YANG_K_PREFIX)) {
	    res = yang_consume_strclause(tkc, mod, &mod->prefix,
					 &pfix, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
	    }
	} else if (!yang_top_keyword(val)) {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	} else {
	    /* assume we reached the end of this sub-section
	     * if there are simply clauses out of order, then
	     * the compiler will not detect that.
	     */
	    TK_BKUP(tkc);
	    done = TRUE;
	}
    }

    /* check missing mandatory sub-clauses */
    if (!mod->ns) {
	retres = ERR_NCX_DATA_MISSING;
	expstr = "namespace statement";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }
    if (!mod->prefix) {
	retres = ERR_NCX_DATA_MISSING;
	expstr = "prefix statement";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }

    return retres;

}  /* consume_mod_hdr */


/********************************************************************
* FUNCTION consume_submod_hdr
* 
* Parse the sub-module header statements
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc    == token chain
*   mod    == module in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_submod_hdr (tk_chain_t  *tkc,
			ncx_module_t *mod)
{
    const xmlChar *val;
    const char    *expstr;
    tk_type_t      tktyp;
    status_t       res, retres;
    boolean        done, ver, blong;

    expstr = "submodule header statement";
    ver = FALSE;
    blong = FALSE;
    res = NO_ERR;
    retres = NO_ERR;
    done = FALSE;

    while (!done) {

	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    CHK_EXIT;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	case TK_TT_RBRACE:
	    TK_BKUP(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_YANG_VERSION)) {
	    /* Optional 'yang-version' field is present */
	    if (ver) {
		retres = ERR_NCX_ENTRY_EXISTS;
		ncx_print_errormsg(tkc, mod, retres);
	    }
	    ver = TRUE;

	    /* get the version number */
	    res = ncx_consume_token(tkc, mod, TK_TT_DNUM);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT) {
		    return res;
		}
	    } else {
		if (xml_strcmp(TK_CUR_VAL(tkc), YANG_VERSION_STR)) {
		    retres = ERR_NCX_WRONG_VERSION;
		    ncx_print_errormsg(tkc, mod, retres);
		} else {
		    mod->langver = YANG_VERSION_NUM;
		}
	    }

	    res = yang_consume_semiapp(tkc, mod, &mod->appinfoQ);
	    CHK_EXIT;
        } else if (!xml_strcmp(val, YANG_K_BELONGS_TO)) {
	    res = yang_consume_strclause(tkc, mod, &mod->belongs,
					 &blong, &mod->appinfoQ);
	    CHK_EXIT;
	} else if (!yang_top_keyword(val)) {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	} else {
	    /* assume we reached the end of this sub-section
	     * if there are simply clauses out of order, then
	     * the compiler will not detect that.
	     */
	    TK_BKUP(tkc);
	    done = TRUE;
	}
    }

    /* check missing mandatory sub-clause */
    if (!mod->belongs) {
	retres = ERR_NCX_DATA_MISSING;
	expstr = "belongs-to statement";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }

    return retres;

}  /* consume_submod_hdr */


/********************************************************************
* FUNCTION consume_import
* 
* Parse the next N tokens as an import clause
* Create a ncx_import struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'import' keyword
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get the ncx_import_t 
*   pcb == parser control block
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_import (tk_chain_t *tkc,
		    ncx_module_t  *mod,
		    yang_pcb_t *pcb)
{
    ncx_import_t       *imp, *testimp;
    const xmlChar      *val;
    const char         *expstr;
    yang_node_t        *node;
    yang_import_ptr_t  *impptr;
    xmlChar            *str;
    tk_token_t         *savetk;
    tk_type_t           tktyp;
    boolean             done, pfixdone;
    status_t            res, retres;

    val = NULL;
    str = NULL;
    expstr = "module name";
    done = FALSE;
    pfixdone = FALSE;
    retres = NO_ERR;

    /* Get a new ncx_import_t to fill in */
    imp = ncx_new_import();
    if (!imp) {
	retres = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, retres);
	return retres;
    } else {
	imp->tk = TK_CUR(tkc);
	imp->usexsd = TRUE;
    }

    /* Get the mandatory module name */
    res = yang_consume_id_string(tkc, mod, &imp->module);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT) {
	    ncx_free_import(imp);
	    return res;
	}
    }
	
    /* Get the starting left brace for the sub-clauses */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT) {
	    ncx_free_import(imp);
	    return res;
	}
    }

    /* get the prefix clause and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    ncx_free_import(imp);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    ncx_free_import(imp);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &imp->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT) {
		    ncx_free_import(imp);
		    return res;
		}
	    }
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}

	/* Got a token string so check the value, should be 'prefix' */
	if (!xml_strcmp(val, YANG_K_PREFIX)) {
	    res = yang_consume_strclause(tkc, mod, &imp->prefix,
					 &pfixdone, &imp->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT) {
		    ncx_free_import(imp);
		    return res;
		}
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	}
    }

    savetk = tkc->cur;
    tkc->cur = imp->tk;

    /* check all the mandatory clauses are present */
    if (!imp->prefix) {
	retres = ERR_NCX_DATA_MISSING;
	expstr = "prefix string";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }

    /* check if the import is already present */
    if (imp->module && imp->prefix) {

	/* check if module already present */
	testimp = ncx_find_import_test(mod, imp->module);
	if (testimp) {
	    imp->usexsd = FALSE;
	    if (!xml_strcmp(testimp->prefix, imp->prefix)) {
		/* warning for exact duplicate import */
		log_warn("\nWarning: duplicate import found on line %u",
			 testimp->tk->linenum);
		res = ERR_NCX_DUP_IMPORT;
		ncx_print_errormsg(tkc, mod, res);
	    } else {
		/* warning for dup. import w/ different prefix */
		log_warn("\nWarning: same import with different prefix"
			 " found on line %u", testimp->tk->linenum);
		res = ERR_NCX_INVALID_DUP_IMPORT;
		ncx_print_errormsg(tkc, mod, res);
	    }
	}

	/* check simple module loop with itself */
	if (imp->usexsd && !xml_strcmp(imp->module, mod->name)) {
	    log_error("\nError: import '%s' for current module",
		      mod->name);
	    retres = ERR_NCX_IMPORT_LOOP;
	    ncx_print_errormsg(tkc, mod, retres);
	}

	/* check simple module loop with the top-level file */
	if (imp->usexsd && xml_strcmp(mod->name, pcb->top->name) &&
	    !xml_strcmp(imp->module, pcb->top->name)) {
	    log_error("\nError: import loop for top-level %smodule '%s'",
		      (pcb->top->ismod) ? "" : "sub", imp->module);
	    retres = ERR_NCX_IMPORT_LOOP;
	    ncx_print_errormsg(tkc, mod, retres);
	}

	/* check simple submodule importing its parent module */
	if (imp->usexsd &&
	    mod->belongs && !xml_strcmp(mod->belongs, imp->module)) {
	    log_error("\nError: submodule '%s' cannot import its"
		      " parent module '%s'", mod->name, imp->module);
	    retres = ERR_NCX_IMPORT_LOOP;
	    ncx_print_errormsg(tkc, mod, retres);
	}

	/* check prefix for this module corner-case */
	if (mod->ismod && !xml_strcmp(imp->prefix, mod->prefix)) {
	    log_error("\nError: import '%s' using "
		      "prefix for current module (%s)",
		      imp->module, imp->prefix);
	    retres = ERR_NCX_IN_USE;
	    ncx_print_errormsg(tkc, mod, retres);
	}
	    
	/* check if prefix already used in other imports */
	testimp = ncx_find_pre_import_test(mod, imp->prefix);
	if (testimp) {
	    if (xml_strcmp(testimp->module, imp->module)) {
		retres = ERR_NCX_IN_USE;
		log_error("\nImport %s on line %u already using prefix %s",
			  testimp->module, testimp->tk->linenum,
			  testimp->prefix);
		ncx_print_errormsg(tkc, mod, retres);
	    }
	}
	
	/* check for import loop */
	node = yang_find_node(&pcb->impchainQ, imp->module);
	if (node) {
	    log_error("\nError: loop created by import '%s'"
		      " from module '%s', line %u",
		      imp->module, node->mod->name,
		      node->tk->linenum);
	    retres = ERR_NCX_IMPORT_LOOP;
	    ncx_print_errormsg(tkc, mod, retres);
	}
    }

    /* save or delete the import struct */
    if (retres == NO_ERR  && imp->usexsd) {
	node = yang_new_node();
	if (!node) {
	    retres = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, retres);
	    ncx_free_import(imp);
	} else {
	    impptr = yang_new_import_ptr(imp->module, imp->prefix);
	    if (!impptr) {
		retres = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, retres);
		ncx_free_import(imp);
		yang_free_node(node);
	    } else {
		/* save the import used record and the import */
		node->name = imp->module;
		node->mod = mod;
		node->tk = imp->tk;

		dlq_enque(node, &pcb->impchainQ);
		dlq_enque(imp, &mod->importQ);
		dlq_enque(impptr, &pcb->allimpQ);
	    
		/* load the module now instead of later for validation */
		res = ncxmod_load_imodule(imp->module, pcb, YANG_PT_IMPORT);
		if (res != NO_ERR) {
		    retres = ERR_NCX_IMPORT_ERRORS;
		    log_error("\nError: '%s' import of module '%s' failed",
			      mod->sourcefn, imp->module);
		    ncx_print_errormsg(tkc, mod, retres);
		}

		/* remove the node in the import chain that 
		 * was added before the module was loaded
		 */
		node = (yang_node_t *)dlq_lastEntry(&pcb->impchainQ);
		if (node) {
		    dlq_remove(node);
		    yang_free_node(node);
		} else {
		    retres = SET_ERROR(ERR_INTERNAL_VAL);
		}
	    }
	}
    } else {
	ncx_free_import(imp);
    }

    tkc->cur = savetk;

    return retres;

}  /* consume_import */


/********************************************************************
* FUNCTION consume_include
* 
* Parse the next N tokens as an include clause
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'include' keyword
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get the ncx_import_t 
*   pcb == parser control block
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_include (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     yang_pcb_t *pcb)
{

    ncx_include_t  *inc, *testinc;
    const char     *expstr;
    yang_node_t    *node, *testnode;
    tk_token_t     *savetk;
    status_t        res, retres;

    expstr = "submodule name";
    retres = NO_ERR;

    /* Get a new ncx_include_t to fill in */
    inc = ncx_new_include();
    if (!inc) {
	retres = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, retres);
	return retres;
    } else {
	inc->tk = TK_CUR(tkc);
	inc->usexsd = TRUE;
    }

    /* Get the mandatory submodule name */
    res = yang_consume_id_string(tkc, mod, &inc->submodule);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT) {
	    ncx_free_include(inc);
	    return res;
	}
    }

    /* Get the end of the statement */
    res = yang_consume_semiapp(tkc, mod, &inc->appinfoQ);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT) {
	    ncx_free_include(inc);
	    return res;
	}
    }

    /* set up tkc for potential errors */
    savetk = tkc->cur;
    tkc->cur = inc->tk;

    /* check if the mandatory submodule name is valid
     * and if the include is already present 
     */
    if (!inc->submodule) {
	retres = ERR_NCX_DATA_MISSING;
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    } else if (!ncx_valid_name2(inc->submodule)) {
	retres = ERR_NCX_INVALID_NAME;
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    } else {
	/* check if submodule already present */
	testinc = ncx_find_include(mod, inc->submodule);
	if (testinc) {
	    /* warning for same duplicate already in the
	     * include statements
	     */
	    inc->usexsd = FALSE;
	    log_warn("\nWarning: duplicate include found on line %u",
		     testinc->tk->linenum);
	    res = ERR_NCX_DUP_INCLUDE;
	    ncx_print_errormsg(tkc, mod, res);
	} else {

	    /* check simple submodule loop with itself */
	    if (!xml_strcmp(inc->submodule, mod->name)) {
		log_error("\nError: include '%s' for current submodule",
			  mod->name);
		retres = ERR_NCX_INCLUDE_LOOP;
		ncx_print_errormsg(tkc, mod, retres);
	    }

	    /* check simple submodule loop with the top-level file */
	    if (xml_strcmp(mod->name, pcb->top->name) &&
		!xml_strcmp(inc->submodule, pcb->top->name)) {
		log_error("\nError: include loop for top-level %smodule '%s'",
			  (pcb->top->ismod) ? "" : "sub", inc->submodule);
		retres = ERR_NCX_INCLUDE_LOOP;
		ncx_print_errormsg(tkc, mod, retres);
	    }

	    /* check simple submodule including its parent module */
	    if (mod->belongs && !xml_strcmp(mod->belongs, inc->submodule)) {
		log_error("\nError: submodule '%s' cannot include its"
			  " parent module '%s'", mod->name, inc->submodule);
		retres = ERR_NCX_INCLUDE_LOOP;
		ncx_print_errormsg(tkc, mod, retres);
	    }

	    /* check for include loop */
	    node = yang_find_node(&pcb->incchainQ, inc->submodule);
	    if (node) {
		log_error("\nError: loop created by include '%s'"
			  " from %smodule '%s', line %u",
			  inc->submodule, (node->mod->ismod) ? "" : "sub",
			  node->mod->name, node->tk->linenum);
		retres = ERR_NCX_INCLUDE_LOOP;
		ncx_print_errormsg(tkc, mod, retres);
	    }
	}
    }

    /* save or delete the include struct */
    if (retres == NO_ERR && inc->usexsd) {
	node = yang_new_node();
	if (!node) {
	    retres = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, retres);
	    ncx_free_include(inc);
	} else {
	    /* save the import used record and the include */
	    node->name = inc->submodule;
	    node->mod = mod;
	    node->tk = inc->tk;

	    dlq_enque(inc, &mod->includeQ);

	    /* check if already parsed, and in the allincQ */
	    testnode = yang_find_node(&pcb->allincQ, inc->submodule);
	    if (!testnode) {
		dlq_enque(node, &pcb->incchainQ);

		/* load the module now instead of later for validation */
		retres = ncxmod_load_imodule(inc->submodule, pcb,
					     YANG_PT_INCLUDE);

		/* remove the node in the include chain that 
		 * was added before the submodule was loaded
		 */
		node = (yang_node_t *)dlq_lastEntry(&pcb->incchainQ);
		if (node) {
		    dlq_remove(node);
		    yang_free_node(node);
		} else {
		    SET_ERROR(ERR_INTERNAL_VAL);
		}
	    } else {
		yang_free_node(node);
	    }
	}
    }

    tkc->cur = savetk;

    return retres;
    
}  /* consume_include */


/********************************************************************
* FUNCTION consume_linkage_stmts
* 
* Parse the next N tokens as N import or include clauses
* Create ncx_import structs and add them to the specified module
*
* Recusively parse any include submodule statement
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get the ncx_import_t 
*   pcb == parser control block
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_linkage_stmts (tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   yang_pcb_t *pcb)
{
    const xmlChar *val;
    const char    *expstr;
    tk_type_t      tktyp;
    status_t       res, retres;
    boolean        done;

    expstr = "import or include keyword";
    res = NO_ERR;
    retres = NO_ERR;
    done = FALSE;

    while (!done) {

	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    CHK_EXIT;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	case TK_TT_RBRACE:
	    TK_BKUP(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_IMPORT)) {
	    res = consume_import(tkc, mod, pcb);
	    CHK_EXIT;
        } else if (!xml_strcmp(val, YANG_K_INCLUDE)) {
	    res = consume_include(tkc, mod, pcb);
	    CHK_EXIT;
	} else if (!yang_top_keyword(val)) {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	} else {
	    /* assume we reached the end of this sub-section
	     * if there are simply clauses out of order, then
	     * the compiler will not detect that.
	     */
	    TK_BKUP(tkc);
	    done = TRUE;
	}
    }

    return retres;

}  /* consume_linkage_stmts */


/********************************************************************
* FUNCTION consume_meta_stmts
* 
* Parse the meta-stmts and submodule-meta-stmts constructs
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc    == token chain
*   mod    == module in progress
*   ismain == TRUE for meta-stmts
*             FALSE for submodule-meta-stmts
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_meta_stmts (tk_chain_t  *tkc,
			ncx_module_t *mod,
			boolean ismain)
{
    const xmlChar *val;
    const char    *expstr;
    tk_type_t      tktyp;
    status_t       res, retres;
    boolean        done, org, contact, descr, ref;

    expstr = "meta-statement";
    done = FALSE;
    org = FALSE;
    contact = FALSE;
    descr = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    while (!done) {

	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    CHK_EXIT;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	case TK_TT_RBRACE:
	    TK_BKUP(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_ORGANIZATION)) {
	    /* 'organization' field is present */
	    res = yang_consume_strclause(tkc, mod, &mod->organization,
					 &org, &mod->appinfoQ);
	    CHK_EXIT;
        } else if (!xml_strcmp(val, YANG_K_CONTACT)) {
	    res = yang_consume_descr(tkc, mod, &mod->contact_info,
				     &contact, &mod->appinfoQ);
	    CHK_EXIT;
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &mod->descr,
				     &descr, &mod->appinfoQ);
	    CHK_EXIT;
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &mod->ref,
				     &ref, &mod->appinfoQ);
	    CHK_EXIT;
	} else if (!yang_top_keyword(val)) {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	} else {
	    /* assume we reached the end of this sub-section
	     * if there are simply clauses out of order, then
	     * the compiler will not detect that.
	     */
	    TK_BKUP(tkc);
	    done = TRUE;
	}
    }

#ifdef WAS_REMOVED_FROM_SPEC
    if (ismain) {
	if (!mod->organization) {
	    retres = ERR_NCX_DATA_MISSING;
	    expstr = "organization clause";
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	}
    }
    if (!descr) {
	retres = ERR_NCX_DATA_MISSING;
	expstr = "description clause";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }
#else
    ismain=FALSE;
#endif

    return retres;

}  /* consume_meta_stmts */


/********************************************************************
* FUNCTION consume_revision
* 
* Parse the next N tokens as a revision clause
* Create a ncx_revhist entry and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'revision' keyword
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get the ncx_import_t 
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_revision (tk_chain_t *tkc,
		      ncx_module_t  *mod)
{
    ncx_revhist_t *rev, *testrev;
    const xmlChar *val;
    const char    *expstr;
    xmlChar       *str, *p;
    tk_type_t      tktyp;
    boolean        done, descrdone;
    status_t       res, retres;

    rev = NULL;
    val = NULL;
    str = NULL;
    expstr = "module name";
    done = FALSE;
    descrdone = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* only malloc a new record if descr strings are being saved */
    rev = ncx_new_revhist();
    if (!rev) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    } else {
	rev->tk = TK_CUR(tkc);
    }

    /* get the mandatory version identifier date string */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	ncx_free_revhist(rev);
	return res;
    }

    if (TK_CUR_STR(tkc)) {
	rev->version = xml_strdup(TK_CUR_VAL(tkc));
	if (!rev->version) {
	    res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
	    ncx_free_revhist(rev);
	    return res;
	}
    } else if (TK_CUR_TYP(tkc)==TK_TT_DNUM) {
	/* assume this is an unquoted date string
	 * this code does not detect corner-cases
	 * like 2007 -11 -20 because an unquoted
	 * dateTime string is the same as 3 integers
	 * and if there are spaces between them
	 * this will be parsed ok by tk.c
	 */
	str = m__getMem(64);
	if (!str) {
	    res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
	    ncx_free_revhist(rev);
	    return res;
	} else {
	    p = str;
	    p += xml_strcpy(p, TK_CUR_VAL(tkc));
	    res = ncx_consume_token(tkc, mod, TK_TT_DNUM);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT) {
		    ncx_free_revhist(rev);
		    m__free(str);
		    return res;
		}
	    } else {
		p += xml_strcpy(p, TK_CUR_VAL(tkc));
		res = ncx_consume_token(tkc, mod, TK_TT_DNUM);
		if (res != NO_ERR) {
		    retres = res;
		    if (NEED_EXIT) {
			ncx_free_revhist(rev);
			m__free(str);
			return res;
		    }
		} else {
		    xml_strcpy(p, TK_CUR_VAL(tkc));
		    rev->version = str;
		}
	    }
	}
    } else {
	retres = ERR_NCX_WRONG_TKTYPE;
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }

    /* Get the starting left brace for the sub-clauses */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT) {
	    ncx_free_revhist(rev);
	    return res;
	}
    }

    /* get the description clause and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    ncx_free_revhist(rev);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    ncx_free_revhist(rev);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT) {
		    ncx_free_revhist(rev);
		    return res;
		}
	    }
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	case TK_TT_RBRACE:
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}

	/* Got a token str so check the value, should be 'description' */
	if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    /* Mandatory 'description' field is present */
	    res = yang_consume_descr(tkc, mod, &rev->descr,
				     &descrdone, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT) {
		    ncx_free_revhist(rev);
		    return res;
		}
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    continue;
	}
    }

    /* check all the mandatory clauses are present */
    if (!descrdone) {
	retres = ERR_NCX_DATA_MISSING;
	expstr = "description clause";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
    }


    if (rev->version) {
	/* check if the version string is valid */
	res = yang_validate_date_string(tkc, mod, rev->tk, rev->version);
	CHK_EXIT;
	    
	/* check if the revision is already present */
	testrev = ncx_find_revhist(mod, rev->version);
	if (testrev) {
	    /* error for dup. revision with same version */
	    retres = ERR_NCX_DUP_ENTRY;
	    ncx_print_errormsg(tkc, mod, retres);
	}
    }

    /* save or delete the revision struct */
    if (rev->version) {
	rev->res = retres;
	dlq_enque(rev, &mod->revhistQ);
    } else {
	ncx_free_revhist(rev);
    }

    return retres;

}  /* consume_revision */


/********************************************************************
* FUNCTION consume_revision_stmts
* 
* Parse the next N tokens as N revision statements
* Create a ncx_revhist struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module struct that will get the ncx_revhist_t entries
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_revision_stmts (tk_chain_t *tkc,
			    ncx_module_t  *mod)
{
    const xmlChar *val;
    xmlChar       *str;
    const char    *expstr;
    ncx_revhist_t *rev;
    tk_type_t      tktyp;
    status_t       res, retres;
    boolean        done;
    int            ret;

    expstr = "description keyword";
    res = NO_ERR;
    retres = NO_ERR;
    done = FALSE;

    while (!done) {

	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    CHK_EXIT;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	case TK_TT_RBRACE:
	    TK_BKUP(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_REVISION)) {
	    res = consume_revision(tkc, mod);
	    CHK_EXIT;
	} else if (!yang_top_keyword(val)) {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	} else {
	    /* assume we reached the end of this sub-section
	     * if there are simply clauses out of order, then
	     * the compiler will not detect that.
	     */
	    TK_BKUP(tkc);
	    done = TRUE;
	}
    }

    /* search the revision date strings, find the
     * highest value date string
     */
    val = NULL;
    for (rev = (ncx_revhist_t *)dlq_firstEntry(&mod->revhistQ);
	 rev != NULL;
	 rev = (ncx_revhist_t *)dlq_nextEntry(rev)) {

	if (rev->res == NO_ERR) {
	    if (!val) {
		val = rev->version;
	    } else {
		ret = xml_strcmp(rev->version, val);
		if (ret > 0) {
		    val = rev->version;
		}
	    }
	}
    }

    /* assign the module version string */
    if (val) {
	/* valid date string found */
	mod->version = xml_strdup(val);
    } else if (!dlq_empty(&mod->revhistQ)) {
	/* use the first (invalid format) date string */
	rev = (ncx_revhist_t *)dlq_firstEntry(&mod->revhistQ);
	if (rev->version) {
	    mod->version = xml_strdup(rev->version);
	}
    }

    if (!mod->version) {
	/* hard-wire the version to current date if no revision clauses */
	str = m__getMem(TSTAMP_DATE_SIZE);
	if (str) {
	    tstamp_date(str);
	    mod->version = str;
	}
    }

    /* check if malloc worked */
    if (!mod->version) {
	retres = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, retres);
    } else if (val) {
	;
    } else if (!dlq_empty(&mod->revhistQ)) {
	log_warn("\nWarning: no valid revision statements, setting "
		 "version to '%s' for %smodule '%s'",
		 mod->version, (mod->ismod) ? "" : "sub", mod->name);
	mod->warnings++;
    } else {
	log_warn("\nWarning: no revision statements, setting "
		 "version to '%s' for %smodule '%s'",
		 mod->version, (mod->ismod) ? "" : "sub", mod->name);
	mod->warnings++;
    }

    return retres;

}  /* consume_revision_stmts */


/********************************************************************
* FUNCTION consume_body_stmts
* 
* Parse the next N tokens as N body statements
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module struct in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_body_stmts (tk_chain_t *tkc,
			ncx_module_t  *mod)
{
    const xmlChar *val;
    const char    *expstr;
    tk_type_t      tktyp;
    status_t       res, retres;
    boolean        done;

    expstr = "body statement";
    res = NO_ERR;
    retres = NO_ERR;
    done = FALSE;

    while (!done) {

	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    return res;
	case TK_TT_RBRACE:
	    /* found end of module */
	    TK_BKUP(tkc);
	    return retres;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    CHK_EXIT;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* YANG clause assumed */
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}

	/* Got a token string so check the keyword value
	 * separate the top-level only statements from
	 * the data-def-stmts, which can appear within
	 * groupings and nested objects
	 */
        if (!xml_strcmp(val, YANG_K_EXTENSION)) {
	    res = yang_ext_consume_extension(tkc, mod);
	} else if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
	    res = yang_typ_consume_typedef(tkc, mod, &mod->typeQ);
	} else if (!xml_strcmp(val, YANG_K_GROUPING)) {
	    res = yang_grp_consume_grouping(tkc, mod,
					    &mod->groupingQ, NULL);
	} else if (!xml_strcmp(val, YANG_K_RPC)) {
	    res = yang_obj_consume_rpc(tkc, mod);
	} else if (!xml_strcmp(val, YANG_K_NOTIFICATION)) {
	    res = yang_obj_consume_notification(tkc, mod);
	} else {
	    res = yang_obj_consume_datadef(tkc, mod,
					   &mod->datadefQ, NULL);
	}
	CHK_EXIT;
    }

    return retres;

}  /* consume_body_stmts */


/********************************************************************
* FUNCTION parse_yang_module
* 
* Parse, generate and register one ncx_module_t struct
* from an NCX instance document stream containing one NCX module,
* which conforms to the NcxModule ABNF.
*
* This is just the first pass parser.
* Many constructs are not validated or internal data structures
* completed yet, after this pass is done.
* This allows support for syntax error checking first,
* and support for forward references i the DML
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module in progress
*   pcb == YANG parser control block
*   ptyp == yang parse type
*   wasadded == pointer to return registry-added flag
*
* OUTPUTS:
*   *wasadded == TRUE if the module was addeed to the NCX moduleQ
*             == FALSE if error-exit, not added and ncx_free_module
*                should be called instead of ncx_remove_module
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    parse_yang_module (tk_chain_t  *tkc,
		       ncx_module_t *mod,
		       yang_pcb_t *pcb,
		       yang_parsetype_t ptyp,
		       boolean *wasadded)
{
    yang_node_t   *node;
    boolean        ismain, loaded;
    status_t       res, retres;

#ifdef YANG_PARSE_DEBUG_TRACE
    log_debug3("\nEnter parse_yang_module");
    if (mod->sourcefn) {
	log_debug3(" %s", mod->sourcefn);
    }
    if (LOGDEBUG3) {
	yang_dump_nodeQ(&pcb->impchainQ, "impchainQ");
	yang_dump_nodeQ(&pcb->incchainQ, "incchainQ");
	yang_dump_nodeQ(&pcb->allincQ, "allincQ");
    }
#endif

    loaded = FALSE;
    ismain = TRUE;
    retres = NO_ERR;
    *wasadded = FALSE;

    mod->isyang = TRUE;

    /* set all YANG owner strings to the same value */
    mod->owner = xml_strdup(NCX_OWNER);
    if (!mod->owner) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* set all YANG application strings to the same value */
    mod->app = xml_strdup((const xmlChar *)"yang");
    if (!mod->app) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* could be module or submodule -- get the first keyword */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
    } else if (TK_CUR_ID(tkc)) {
	/* got an ID token, make sure it is correct to continue */
	if (!xml_strcmp(TK_CUR_VAL(tkc), YANG_K_MODULE)) {
	    ismain = TRUE;
	    TK_BKUP(tkc);
	    res = ncx_consume_name(tkc, mod, YANG_K_MODULE, 
				   &mod->name, NCX_REQ, TK_TT_LBRACE);
	} else if (!xml_strcmp(TK_CUR_VAL(tkc), YANG_K_SUBMODULE)) {
	    ismain = FALSE;
	    TK_BKUP(tkc);
	    res = ncx_consume_name(tkc, mod, YANG_K_SUBMODULE, 
				   &mod->name, NCX_REQ, TK_TT_LBRACE);
	} else {
	    res = ERR_NCX_WRONG_TKVAL;
	    ncx_print_errormsg(tkc, mod, res);
	}
    } else {
	res = ERR_NCX_WRONG_TKTYPE;
	ncx_print_errormsg(tkc, mod, res);
    }
    CHK_EXIT;

    /* exit on all errors, since this is probably not a YANG file */
    if (retres != NO_ERR) {
	return res;
    }

    /* got a start of [sub]module OK, check the parse type */
    switch (ptyp) {
    case YANG_PT_TOP:
	if (pcb->with_submods && !ismain) {
	    return ERR_NCX_SKIPPED;
	} else {
	    pcb->top = mod;
	}
	break;
    case YANG_PT_INCLUDE:
	if (ismain) {
	    log_error("\nError: including module '%s', "
		      "should be submodule", mod->name);
	    retres = ERR_NCX_EXP_SUBMODULE;
	    ncx_print_errormsg(tkc, mod, retres);
	}
	break;
    case YANG_PT_IMPORT:
	if (!ismain) {
	    log_error("\nError: importing submodule '%s', "
		      "should be module", mod->name);
	    retres = ERR_NCX_EXP_MODULE;
	    ncx_print_errormsg(tkc, mod, retres);
	}
	break;
    case YANG_PT_TOP_INCL:
	if (!ismain) {
	    log_error("\nError: belongs-to (submodule) '%s': "
		      "must identify a module", mod->name);
	    retres = ERR_NCX_EXP_MODULE;
	    ncx_print_errormsg(tkc, mod, retres);
	}
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    mod->ismod = ismain;

    if (ismain) {
	/* consume module-header-stmts */
	res = consume_mod_hdr(tkc, mod);
	CHK_EXIT;
    } else {
	/* consume submodule-header-stmts */
	res = consume_submod_hdr(tkc, mod);
	CHK_EXIT;
    }

    /* check if only parsing the module header */
    if (ptyp == YANG_PT_TOP_INCL) {
	if (pcb->subtree_mode) {
	    /* don't waste the one def_reg add on this partial module
	     * it is not really needed
	     */
	    return NO_ERR;
	} else {
	    /* clear these backpointers because the yang_pcb_t
	     * will get destroyed soon
	     */
	    mod->allincQ = NULL;
	    mod->allimpQ = NULL;

	    /* add the module name and namespace to the registry */
	    res = ncx_add_to_registry(mod);
	    if (res != NO_ERR) {
		retres = res;
	    } else {
		pcb->mod = mod;
		*wasadded = TRUE;
	    }
	    return retres;
	}
    }

    /* check if this module is already loaded, except in diff mode */
    if (mod->ismod && mod->name && !pcb->diffmode &&
	def_reg_find_module(mod->name)) {
	switch (ptyp) {
	case YANG_PT_TOP:
	    loaded = TRUE;
	    break;
	case YANG_PT_IMPORT:
	    return NO_ERR;
	default:
	    ;
	}
    }

    /* Check if this is a top-level submodule parse
     * and get the module that it belongs-to
     * so the prefix and namespace can be copied
     *
     * If the belongs clause is not set then the submodule
     * prefix will remain NULL
     */
    if ((ptyp==YANG_PT_TOP) && !ismain && 
	mod->belongs && ncx_valid_name2(mod->belongs)) {
	res = ncxmod_load_imodule(mod->belongs, pcb,
				  YANG_PT_TOP_INCL);
	CHK_EXIT;

	/* set the submodule prefix to the main module prefix */
	if (pcb->mod && pcb->mod->prefix) {
	    mod->prefix = pcb->mod->prefix;
	    mod->belongsver = pcb->mod->version;
	}
    }

    /* set the prefix if this is a submodule being included */
    if (ptyp==YANG_PT_INCLUDE && pcb->top) {
	mod->prefix = pcb->top->prefix;
    }

    /* Get the linkage statements (imports, include) */
    res = consume_linkage_stmts(tkc, mod, pcb);
    CHK_EXIT;

    /* Get the meta statements (organization, etc.) */
    res = consume_meta_stmts(tkc, mod, ismain);
    CHK_EXIT;

    /* Get the revision statements */
    res = consume_revision_stmts(tkc, mod);
    CHK_EXIT;

    /* make sure there is at least name and prefix to continue */
    if (!mod->name || !mod->prefix || !*mod->name || !*mod->prefix) {
	return retres;
    }

    /* Get the definition statements */
    res = consume_body_stmts(tkc, mod);
    CHK_EXIT;

    /* the next node should be the '(sub)module' end node */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    CHK_EXIT;

    /* check extra tokens left over */
    res = TK_ADV(tkc);
    if (res == NO_ERR) {
	retres = ERR_NCX_EXTRA_NODE;
	log_error("\nError: Extra input after end of module"
		  " starting on line %u", TK_CUR_LNUM(tkc));
	ncx_print_errormsg(tkc, mod, retres);
    }

    /**************** Module Validation *************************/

    /* check all the module level extension statements */
    res = ncx_resolve_appinfoQ(tkc, mod, &mod->appinfoQ);
    CHK_EXIT;

    /* check all the module level extension statements
     * within the include and import statements
     */
    res = resolve_iappinfo(tkc, mod);
    CHK_EXIT;

    /* Validate any module-level typedefs */
    res = yang_typ_resolve_typedefs(tkc, mod, &mod->typeQ, NULL);
    CHK_EXIT;

    /* Validate any module-level groupings */
    res = yang_grp_resolve_groupings(tkc, mod, &mod->groupingQ, NULL);
    CHK_EXIT;

    /* Validate any module-level data-def-stmts */
    res = yang_obj_resolve_datadefs(tkc, mod, &mod->datadefQ);
    CHK_EXIT;

    /* Expand and validate any uses-stmts within module-level groupings */
    res = yang_grp_resolve_complete(tkc, mod, &mod->groupingQ, NULL);
    CHK_EXIT;

    /* Expand and validate any uses-stmts within module-level datadefs */
    res = yang_obj_resolve_uses(tkc, mod, &mod->datadefQ);
    CHK_EXIT;

    /* Expand and validate any augment-stmts within module-level datadefs */
    res = yang_obj_resolve_augments(tkc, mod, &mod->datadefQ);
    CHK_EXIT;

    /* Check for imports not used warnings */
    yang_check_imports_used(tkc, mod);

    /* One final check for grouping integrity */
    res = yang_grp_resolve_final(tkc, mod, &mod->groupingQ);
    CHK_EXIT;

    /* One final check for object integrity */
    res = yang_obj_resolve_final(tkc, mod, &mod->datadefQ);
    CHK_EXIT;

    /* save the module parse status */
    mod->status = retres;

    /* make sure there is at least name and prefix to continue */
    if (!((mod->name && ncx_valid_name2(mod->name)) &&
	  (mod->prefix && ncx_valid_name2(mod->prefix)))) {
	return retres;
    }

    /* add the definitions to the def_reg hash table;
     * check the parse type before adding to registry
     */
    switch (ptyp) {
    case YANG_PT_TOP:
    case YANG_PT_IMPORT:
	/* add this regular module to the registry */
	if (mod->ismod || pcb->top == mod) {
	    if (!mod->ismod) {
		mod->nsid = pcb->mod->nsid;
	    }

	    dlq_block_enque(mod->allimpQ, &mod->saveimpQ);
	    dlq_block_enque(mod->allincQ, &mod->saveincQ);
	    mod->allincQ = NULL;
	    mod->allimpQ = NULL;

	    if (!loaded) {
		if (!pcb->diffmode) {
		    res = ncx_add_to_registry(mod);
		    if (res != NO_ERR) {
			retres = res;
		    } else {
			/* if mod==top, and top is a submodule, then 
			 * yang_free_pcb will delete the submodule later
			 */
			*wasadded = TRUE;
		    }
		}
	    }
	} else {
	    mod->nsid = xmlns_find_ns_by_name(mod->ns);
	}
	break;
    case YANG_PT_TOP_INCL:
	SET_ERROR(ERR_INTERNAL_VAL);
	break;
    case YANG_PT_INCLUDE:
	/* create an entry in the allinc Q to cache this entry
	 * and keep only one copy for all includes of the same
	 * submodule
	 */
	node = yang_new_node();
	if (!node) {
	    retres = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, retres);
	} else {
	    node->name = mod->name;
	    node->mod = NULL;
	    node->tk = NULL;
	    node->tkc = tkc;
	    node->submod = mod;
	    mod->allimpQ = NULL;
	    mod->allincQ = NULL;
	    node->res = retres;
	    dlq_enque(node, &pcb->allincQ);
	    *wasadded = TRUE;
	}
	break;
    default:
	retres = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return retres;

}  /* parse_yang_module */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION yang_parse_from_filespec
* 
* Parse a file as a YANG module
*
* Error messages are printed by this function!!
*
* INPUTS:
*   filespec == absolute path or relative path
*               This string is used as-is without adjustment.
*   pcb == parser control block used as very top-level struct
*   ptyp == parser call type
*            YANG_PT_TOP == called from top-level file
*            YANG_PT_INCLUDE == called from an include-stmt in a file
*            YANG_PT_IMPORT == called from an import-stmt in a file
*
* OUTPUTS:
*   an ncx_module is filled out and validated as the file
*   is parsed.  If no errors:
*     TOP, IMPORT:
*        the module is loaded into the definition registry with 
*        the ncx_add_to_registry function
*     INCLUDE:
*        the submodule is loaded into the top-level module,
*        specified in the pcb
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_parse_from_filespec (const xmlChar *filespec,
			      yang_pcb_t *pcb,
			      yang_parsetype_t ptyp)
{
    tk_chain_t     *tkc;
    ncx_module_t   *mod;
    FILE           *fp;
    xmlChar        *str;
    status_t        res;
    boolean         wasadd;

#ifdef DEBUG
    if (!filespec || !pcb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    fp = NULL;
    tkc = NULL;
    mod = NULL;
    res = NO_ERR;

    /* open the YANG source file for reading */
    fp = fopen((const char *)filespec, "r");
    if (!fp) {
	return ERR_NCX_MISSING_FILE;
    }

    log_debug2("\nLoading YANG module from file %s", filespec);

    /* get a new token chain */
    if (res == NO_ERR) {
	tkc = tk_new_chain();
	if (!tkc) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nyang_parse malloc error");
	}
    }

    /* copy the filespec */
    if (res == NO_ERR) {
	str = ncx_get_source(filespec);
	if (!str) {
	    res = ERR_INTERNAL_MEM;
	}
    }

    /* setup the token chain to parse this YANG file */
    if (res == NO_ERR) {
	tk_setup_chain_yang(tkc, fp, str);

	/* start a new ncx_module_t struct */
	mod = ncx_new_module();
	if (!mod) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    /* save the source of this ncx-module for monitor / debug */
	    mod->source = str;

	    /* find the start of the file name */
	    mod->sourcefn = &str[xml_strlen(str)];
	    while (mod->sourcefn > str &&
		   *mod->sourcefn != NCX_PATHSEP_CH) {
		mod->sourcefn--;
	    }
	    if (*mod->sourcefn == NCX_PATHSEP_CH) {
		mod->sourcefn++;
	    }

	    /* set the back-ptr to Q of all the import files */
	    mod->allimpQ = &pcb->allimpQ;

	    /* set the back-ptr to Q of all the include files */
	    mod->allincQ = &pcb->allincQ;

	    /* set the stmt-track mode flag if needed */
	    if (ptyp==YANG_PT_TOP && pcb->stmtmode) {
		mod->stmtmode = TRUE;
	    }
	}
    }

    if (res == NO_ERR) {
	/* serialize the file into language tokens
	 * !!! need to change this later because it may use too
	 * !!! much memory in embedded parsers
	 */
	res = tk_tokenize_input(tkc, mod);

#ifdef YANG_PARSE_TK_DEBUG
	tk_dump_chain(tkc);
#endif
    }

    /* parse the module and validate it only if a token chain
     * was properly parsed
     */
    if (res == NO_ERR) {
	res = parse_yang_module(tkc, mod, pcb, ptyp, &wasadd);
	if (res != NO_ERR) {
	    if (!wasadd) {
		ncx_free_module(mod);
	    }
	} else if (!wasadd && !pcb->diffmode) {
	    if (mod->ismod) {
		if (pcb->top == mod) {
		    /* swap with the real module already done */
		    pcb->top = ncx_find_module(mod->name);
		}
	    } else if (!pcb->with_submods) {
		/* subtree parsing mode can cause top-level to already
		 * be loaded into the registry, swap out the new dummy
		 * module with the real one
		 */
		if (pcb->top == mod) {
		    pcb->top = ncx_find_module(mod->belongs);
		}
	    }
	    ncx_free_module(mod);
	    if (!pcb->top && !pcb->with_submods) {
		res = ERR_NCX_MOD_NOT_FOUND;
	    }
	}
    }

    fclose(fp);
    if (tkc) {
	tkc->fp = NULL;
    }

    if (tkc && ptyp != YANG_PT_INCLUDE) {
	tk_free_chain(tkc);
    }

    return res;

}  /* yang_parse_from_filespec */


/* END file yang_parse.c */
