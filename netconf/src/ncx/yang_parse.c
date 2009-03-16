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

#ifndef _H_obj
#include "obj.h"
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

static status_t 
    consume_revision_date (tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   xmlChar **revstring);


/********************************************************************
* FUNCTION resolve_mod_appinfo
* 
* Validate the ncx_appinfo_t (extension usage) within
* the include, import, and feature clauses for this module
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
    resolve_mod_appinfo (tk_chain_t  *tkc,
			 ncx_module_t *mod)
{
    ncx_import_t    *imp;
    ncx_include_t   *inc;
    ncx_feature_t   *feature;
    status_t         res, retres;

    retres = NO_ERR;

    for (imp = (ncx_import_t *)dlq_firstEntry(&mod->importQ);
	 imp != NULL;
	 imp = (ncx_import_t *)dlq_nextEntry(imp)) {

	res = ncx_resolve_appinfoQ(tkc, mod, &imp->appinfoQ);
	CHK_EXIT(res, retres);
    }

    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
	 inc != NULL;
	 inc = (ncx_include_t *)dlq_nextEntry(inc)) {

	res = ncx_resolve_appinfoQ(tkc, mod, &inc->appinfoQ);
	CHK_EXIT(res, retres);
    }

    for (feature = (ncx_feature_t *)dlq_firstEntry(&mod->featureQ);
	 feature != NULL;
	 feature = (ncx_feature_t *)dlq_nextEntry(feature)) {

	res = ncx_resolve_appinfoQ(tkc, mod, &feature->appinfoQ);
	CHK_EXIT(res, retres);
    }

    return retres;

}  /* resolve_mod_appinfo */


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
    xmlChar       *str;
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
	    res = yang_consume_string(tkc, mod, &str);
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
	    if (str) {
		m__free(str);
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
* FUNCTION consume_belongs_to
* 
* Parse the next N tokens as a belongs-to clause
* Set the submodule prefix and belongs string
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'belongs-to' keyword
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get updated
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_belongs_to (tk_chain_t *tkc,
			ncx_module_t  *mod)
{
    const xmlChar      *val;
    const char         *expstr;
    xmlChar            *str;
    tk_type_t           tktyp;
    boolean             done, pfixdone;
    status_t            res, retres;

    val = NULL;
    str = NULL;
    expstr = "module name";
    done = FALSE;
    pfixdone = FALSE;
    retres = NO_ERR;

    /* Get the mandatory module name */
    res = yang_consume_id_string(tkc, mod, &mod->belongs);
    if (res != NO_ERR) {
	return res;
    }
	
    /* Get the starting left brace for the sub-clauses
     * or a semi-colon to end the belongs-to statement
     */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
	done = TRUE;
	break;
    case TK_TT_LBRACE:
	break;
    default:
	retres = ERR_NCX_WRONG_TKTYPE;
	expstr = "semi-colon or left brace";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
	done = TRUE;
    }

    /* get the prefix clause and any appinfo extensions */
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
	    CHK_EXIT(res, retres);
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
	    res = yang_consume_strclause(tkc, mod, &mod->prefix,
					 &pfixdone, &mod->appinfoQ);
	    CHK_EXIT(res, retres);
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	}
    }

    return retres;

}  /* consume_belongs_to */


/********************************************************************
* FUNCTION consume_feature
* 
* Parse the next N tokens as a feature statement
* Create an ncx_feature_t struct and add it to the
* module or submodule featureQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'feature' keyword
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get updated
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_feature (tk_chain_t *tkc,
		     ncx_module_t  *mod)
{
    const xmlChar      *val;
    const char         *expstr;
    ncx_feature_t      *feature, *testfeature;
    tk_type_t           tktyp;
    boolean             done, stat, desc, ref, keep;
    status_t            res, retres;

    val = NULL;
    expstr = "feature name";
    done = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    keep = TRUE;
    retres = NO_ERR;

    feature = ncx_new_feature();
    if (!feature) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }
    feature->tk = TK_CUR(tkc);

    /* Get the mandatory feature name */
    res = yang_consume_id_string(tkc, mod, &feature->name);
    if (res != NO_ERR) {
	/* do not keep -- must have a name field */
	retres = res;
	keep = FALSE;
    }
	
    /* Get the starting left brace for the sub-clauses
     * or a semi-colon to end the feature statement
     */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	ncx_free_feature(feature);
	return res;
    }
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
	done = TRUE;
	break;
    case TK_TT_LBRACE:
	break;
    default:
	retres = ERR_NCX_WRONG_TKTYPE;
	expstr = "semi-colon or left brace";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
	done = TRUE;
    }

    expstr = "feature sub-statement";

    /* get the prefix clause and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    retres = res;
	    done = TRUE;
	    continue;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    done = TRUE;
	    continue;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT(res)) {
		    done = TRUE;
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
	if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
	    res = yang_consume_iffeature(tkc, mod, 
					 &feature->iffeatureQ,
					 &feature->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &feature->status,
				      &stat, &feature->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &feature->descr,
				     &desc, &feature->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &feature->ref,
				     &ref, &feature->appinfoQ);
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}
	if (res != NO_ERR) {
	    retres = res;
	    if (NEED_EXIT(res)) {
		done = TRUE;
	    }
	}
    }

    /* check if feature already exists in this module */
    if (keep) {
	testfeature = ncx_find_feature(mod, feature->name);
	if (testfeature) {
	    retres = ERR_NCX_DUP_ENTRY;
	    log_error("\nError: feature '%s' already defined "
		      "in this module", feature->name);
	    ncx_print_errormsg(tkc, mod, retres);
	}
	feature->res = retres;
	dlq_enque(feature, &mod->featureQ);
    } else {
	ncx_free_feature(feature);
    }

    return retres;

}  /* consume_feature */


/********************************************************************
* FUNCTION resolve_feature
* 
* Validate all the if-feature clauses present in 
* the specified feature
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
*   feature == ncx_feature_t to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_feature (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     ncx_feature_t *feature)
{
    ncx_feature_t    *testfeature;
    ncx_iffeature_t  *iff;
    status_t          res, retres;
    boolean           errdone;

    retres = NO_ERR;

    /* check if there are any if-feature statements inside
     * this feature that need to be resolved
     */
    for (iff = (ncx_iffeature_t *)
	     dlq_firstEntry(&feature->iffeatureQ);
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
	} else if (!xml_strcmp(iff->name, feature->name)) {
	    /* error: if-feature foo inside feature foo */
	    res = retres = ERR_NCX_DEF_LOOP;
	    log_error("\nError: 'if-feature %s' inside feature '%s'",
		      iff->name, iff->name);
	    tkc->cur = iff->tk;
	    ncx_print_errormsg(tkc, mod, retres);
	    errdone = TRUE;
	} else {
	    testfeature = ncx_find_feature(mod, iff->name);
	}

	if (!testfeature && !errdone) {
	    log_error("\nError: Feature '%s' not found "
		      "for if-feature statement",
		      iff->name);
	    res = retres = ERR_NCX_DEF_NOT_FOUND;
	    tkc->cur = iff->tk;
	    ncx_print_errormsg(tkc, mod, retres);
	}

	if (testfeature) {
	    iff->feature = testfeature;
	}
    }

    return retres;

}  /* resolve_feature */


/********************************************************************
* FUNCTION check_feature_loop
* 
* Validate all the if-feature clauses present in 
* the specified feature, after all if-features have
* been resolved (or at least attempted)
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
*   feature == ncx_feature_t to check now
*   startfeature == feature that started this off, so if this
*                   is reached again, it will trigger an error
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_feature_loop (tk_chain_t *tkc,
			ncx_module_t  *mod,
			ncx_feature_t *feature,
			ncx_feature_t *startfeature)
{
    ncx_iffeature_t  *iff;
    status_t          res, retres;

    retres = NO_ERR;

    /* check if there are any if-feature statements inside
     * this feature that need to be resolved
     */
    for (iff = (ncx_iffeature_t *)
	     dlq_firstEntry(&feature->iffeatureQ);
	 iff != NULL;
	 iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

	if (!iff->feature) {
	    continue;
	}

	if (iff->feature == startfeature) {
	    retres = res = ERR_NCX_DEF_LOOP;
	    startfeature->res = res;
	    log_error("\nError: if-feature loop detected for '%s' "
		      "in feature '%s'",
		      startfeature->name,
		      feature->name);
	    tkc->cur = startfeature->tk;
	    ncx_print_errormsg(tkc, mod, res);
	} else if (iff->feature->res != ERR_NCX_DEF_LOOP) {
	    res = check_feature_loop(tkc, mod, iff->feature,
				     startfeature);
	    if (res != NO_ERR) {
		retres = res;
	    }
	}
    }

    return retres;

}  /* check_feature_loop */


/********************************************************************
* FUNCTION consume_identity
* 
* Parse the next N tokens as an identity statement
* Create an ncx_identity_t struct and add it to the
* module or submodule identityQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'identity' keyword
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get updated
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_identity (tk_chain_t *tkc,
		      ncx_module_t  *mod)
{
    const xmlChar      *val;
    const char         *expstr;
    ncx_identity_t     *identity, *testidentity;
    tk_type_t           tktyp;
    boolean             done, base, stat, desc, ref, keep;
    status_t            res, retres;

    val = NULL;
    expstr = "identity name";
    done = FALSE;
    base = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    keep = TRUE;
    retres = NO_ERR;

    identity = ncx_new_identity();
    if (!identity) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }
    identity->tk = TK_CUR(tkc);
    identity->isroot = TRUE;
    identity->mod = mod;

    /* Get the mandatory identity name */
    res = yang_consume_id_string(tkc, mod, &identity->name);
    if (res != NO_ERR) {
	/* do not keep -- must have a name field */
	retres = res;
	keep = FALSE;
    }
	
    /* Get the starting left brace for the sub-clauses
     * or a semi-colon to end the identity statement
     */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	ncx_free_identity(identity);
	return res;
    }
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
	done = TRUE;
	break;
    case TK_TT_LBRACE:
	break;
    default:
	retres = ERR_NCX_WRONG_TKTYPE;
	expstr = "semi-colon or left brace";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
	done = TRUE;
    }

    expstr = "identity sub-statement";

    /* get the prefix clause and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    retres = res;
	    done = TRUE;
	    continue;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    done = TRUE;
	    continue;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT(res)) {
		    done = TRUE;
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
	if (!xml_strcmp(val, YANG_K_BASE)) {
	    identity->isroot = FALSE;
	    res = yang_consume_pid(tkc, mod, 
				   &identity->baseprefix,
				   &identity->basename,
				   &base, &identity->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_STATUS)) {
	    res = yang_consume_status(tkc, mod, &identity->status,
				      &stat, &identity->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &identity->descr,
				     &desc, &identity->appinfoQ);
	} else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &identity->ref,
				     &ref, &identity->appinfoQ);
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	    continue;
	}
	if (res != NO_ERR) {
	    retres = res;
	    if (NEED_EXIT(res)) {
		done = TRUE;
	    }
	}
    }

    /* check if feature already exists in this module */
    if (keep) {
	testidentity = ncx_find_identity(mod, identity->name);
	if (testidentity) {
	    retres = ERR_NCX_DUP_ENTRY;
	    log_error("\nError: identity '%s' already defined "
		      "in this module", identity->name);
	    ncx_print_errormsg(tkc, mod, retres);
	}
	identity->res = retres;
	dlq_enque(identity, &mod->identityQ);
    } else {
	ncx_free_identity(identity);
    }

    return retres;

}  /* consume_identity */


/********************************************************************
* FUNCTION resolve_identity
* 
* Validate the identity statement base clause, if any
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
*   identity == ncx_identity_t to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_identity (tk_chain_t *tkc,
		      ncx_module_t  *mod,
		      ncx_identity_t *identity)
{
    ncx_identity_t   *testidentity;
    status_t          res;
    boolean           errdone;

    if (identity->isroot) {
	return NO_ERR;
    }

    res = NO_ERR;
    testidentity = NULL;
    errdone = FALSE;

    if (identity->baseprefix &&
	xml_strcmp(identity->baseprefix, mod->prefix)) {

	/* find the identity in another module */
       	res = yang_find_imp_identity(tkc, mod, 
				     identity->baseprefix,
				     identity->basename, 
				     identity->tk,
				     &testidentity);
	if (res != NO_ERR) {
	    errdone = TRUE;
	}
    } else if (!xml_strcmp(identity->name, identity->basename)) {
	/* error: 'base foo' inside 'identity foo' */
	res = ERR_NCX_DEF_LOOP;
	log_error("\nError: 'base %s' inside identity '%s'",
		  identity->basename, identity->name);
	tkc->cur = identity->tk;
	ncx_print_errormsg(tkc, mod, res);
	errdone = TRUE;
    } else {
	testidentity = 
	    ncx_find_identity(mod, 
			      identity->basename);
    }

    if (!testidentity && !errdone) {
	log_error("\nError: Base '%s%s%s' not found "
		  "for identity statement '%s'",
		  (identity->baseprefix) ? 
		  identity->baseprefix : EMPTY_STRING,
		  (identity->baseprefix) ? "?" : "",
		  identity->basename,
		  identity->name);
	res = ERR_NCX_DEF_NOT_FOUND;
	tkc->cur = identity->tk;
	ncx_print_errormsg(tkc, mod, res);
    }

    if (testidentity) {
	identity->base = testidentity;
    }

    return res;

}  /* resolve_identity */


/********************************************************************
* FUNCTION check_identity_loop
* 
* Validate the base identity chain for loops
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
*   identity == ncx_identity_t to check now
*   startidentity == identity that started this off, so if this
*                    is reached again, it will trigger an error
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_identity_loop (tk_chain_t *tkc,
			 ncx_module_t  *mod,
			 ncx_identity_t *identity,
			 ncx_identity_t *startidentity)
{
    status_t          res;

    /* check if there is a base statement, and if it leads
     * back to startidentity or not
     */
    if (!identity->base) {
	res = NO_ERR;
    } else if (identity->base == startidentity) {
	res = ERR_NCX_DEF_LOOP;
	startidentity->res = res;
	log_error("\nError: identity base loop detected for '%s' "
		  "in identity '%s'",
		  startidentity->name,
		  identity->name);
	tkc->cur = startidentity->tk;
	ncx_print_errormsg(tkc, mod, res);
    } else if (identity->base->res != ERR_NCX_DEF_LOOP) {
	res = check_identity_loop(tkc, mod, 
				  identity->base,
				  startidentity);
	if (res == NO_ERR) {
	    /* thread the idlink into the base identifier */
	    dlq_enque(&identity->idlink, &identity->base->childQ);
	    identity->idlink.inq = TRUE;
	}
    }

    return res;

}  /* check_identity_loop */


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
	    CHK_EXIT(res, retres);
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
		if (NEED_EXIT(res)) {
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
	    CHK_EXIT(res, retres);
        } else if (!xml_strcmp(val, YANG_K_BELONGS_TO)) {
	    res = consume_belongs_to(tkc, mod);
	    CHK_EXIT(res, retres);
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
    tk_token_t         *savetk, *revtk;
    tk_type_t           tktyp;
    boolean             done, pfixdone, revdone;
    status_t            res, retres;

    val = NULL;
    str = NULL;
    expstr = "module name";
    done = FALSE;
    pfixdone = FALSE;
    revdone = FALSE;
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
	if (NEED_EXIT(res)) {
	    ncx_free_import(imp);
	    return res;
	}
    }
	
    /* Get the starting left brace for the sub-clauses */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT(res)) {
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
		if (NEED_EXIT(res)) {
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
		if (NEED_EXIT(res)) {
		    ncx_free_import(imp);
		    return res;
		}
	    }
	} else if (!xml_strcmp(val, YANG_K_REVISION_DATE)) {
	    revtk = TK_CUR(tkc);
	    if (revdone) {
		res = retres = ERR_NCX_ENTRY_EXISTS;
		ncx_print_errormsg(tkc, mod, retres);
		if (imp->revision) {
		    m__free(imp->revision);
		    imp->revision = NULL;
		}
	    } else {
		res = NO_ERR;
		revdone = TRUE;
	    }

	    res = consume_revision_date(tkc, mod, &imp->revision);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT(res)) {
		    ncx_free_import(imp);
		    return res;
		}
	    }

	    res = yang_consume_semiapp(tkc, mod, &imp->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT(res)) {
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

    if (imp->revision) {
	/* validate the revision date */
	res = yang_validate_date_string(tkc, mod, revtk, imp->revision);
	if (res != NO_ERR) {
	    retres = res;
	    if (NEED_EXIT(res)) {
		ncx_free_import(imp);
		return res;
	    }
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
	    if (((testimp->revision && imp->revision) ||
		 (!testimp->revision && !imp->revision)) &&
		yang_compare_revision_dates(testimp->revision, 
					    imp->revision)) {
		
		log_error("\nError: invalid duplicate import found on line %u",
			  testimp->tk->linenum);
		retres = res = ERR_NCX_INVALID_DUP_IMPORT;
		ncx_print_errormsg(tkc, mod, res);
	    } else {
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
		    res = ERR_NCX_PREFIX_DUP_IMPORT;
		    ncx_print_errormsg(tkc, mod, res);
		}
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
	node = yang_find_node(&pcb->impchainQ, 
			      imp->module,
			      imp->revision);
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
	    impptr = yang_new_import_ptr(imp->module, 
					 imp->prefix,
					 imp->revision);
	    if (!impptr) {
		retres = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, retres);
		ncx_free_import(imp);
		yang_free_node(node);
	    } else {
		/* save the import used record and the import */
		node->name = imp->module;
		node->revision = imp->revision;
		node->mod = mod;
		node->tk = imp->tk;

		/* save the import on the impchain stack */
		dlq_enque(node, &pcb->impchainQ);

		/* save the import for prefix translkation */
		dlq_enque(imp, &mod->importQ);

		/* save the import marker to keep a list
		 * of all the imports with no duplicates
		 * regardless of recursion or submodules
		 */
		dlq_enque(impptr, &pcb->allimpQ);

		/* load the module now instead of later for validation
		 * it may not get used, but assume it will
		 */
		res = ncxmod_load_imodule(imp->module, 
					  imp->revision,
					  pcb, YANG_PT_IMPORT);
		if (res != NO_ERR) {
		    /* skip error if module has just warnings */
		    if (get_errtyp(res) < ERR_TYP_WARN) {
			retres = ERR_NCX_IMPORT_ERRORS;
			if (imp->revision) {
			    log_error("\nError: '%s' import of "
				      "module '%s' revision '%s' failed",
				      mod->sourcefn, 
				      imp->module,
				      imp->revision);
			} else {
			    log_error("\nError: '%s' import of "
				      "module '%s' failed",
				      mod->sourcefn, imp->module);
			}
			ncx_print_errormsg(tkc, mod, res);
		    }
		} /* else ignore the warnings */

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
    const xmlChar  *val;
    yang_node_t    *node, *testnode;
    tk_token_t     *savetk, *revtk;
    tk_type_t       tktyp;
    status_t        res, retres;
    boolean         done, revdone;

    done = FALSE;
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
	if (NEED_EXIT(res)) {
	    ncx_free_include(inc);
	    return res;
	}
    }

    /* Get the starting left brace for the sub-clauses
     * or a semi-colon to end the include statement
     */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_free_include(inc);
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
	done = TRUE;
	break;
    case TK_TT_LBRACE:
	break;
    default:
	retres = ERR_NCX_WRONG_TKTYPE;
	expstr = "semi-colon or left brace";
	ncx_mod_exp_err(tkc, mod, retres, expstr);
	done = TRUE;
    }

    /* get the revision clause and any appinfo extensions */
    while (!done) {
	/* get the next token */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    ncx_free_include(inc);
	    return res;
	}

	tktyp = TK_CUR_TYP(tkc);
	val = TK_CUR_VAL(tkc);

	/* check the current token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    res = ERR_NCX_EOF;
	    ncx_mod_exp_err(tkc, mod, res, expstr);
	    ncx_free_include(inc);
	    return res;
	case TK_TT_MSTRING:
	    /* vendor-specific clause found instead */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT(res)) {
		    ncx_free_include(inc);
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
	if (!xml_strcmp(val, YANG_K_REVISION_DATE)) {
	    revtk = TK_CUR(tkc);
	    if (revdone) {
		res = retres = ERR_NCX_ENTRY_EXISTS;
		ncx_print_errormsg(tkc, mod, retres);
		if (inc->revision) {
		    m__free(inc->revision);
		    inc->revision = NULL;
		}
	    } else {
		res = NO_ERR;
		revdone = TRUE;
	    }

	    res = consume_revision_date(tkc, mod, &inc->revision);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT(res)) {
		    ncx_free_include(inc);
		    return res;
		}
	    }

	    res = yang_consume_semiapp(tkc, mod, &inc->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		if (NEED_EXIT(res)) {
		    ncx_free_include(inc);
		    return res;
		}
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    ncx_mod_exp_err(tkc, mod, retres, expstr);
	    yang_skip_statement(tkc, mod);
	}
    }

    if (inc->revision) {
	/* validate the revision date */
	res = yang_validate_date_string(tkc, mod, revtk, inc->revision);
	if (res != NO_ERR) {
	    retres = res;
	    if (NEED_EXIT(res)) {
		ncx_free_include(inc);
		return res;
	    }
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
	    /* check if there is a revision conflict */
	    if (((testinc->revision && inc->revision) ||
		 (!testinc->revision && !inc->revision)) &&
		yang_compare_revision_dates(testinc->revision, 
					    inc->revision)) {
		log_error("\nError: invalid duplicate include found on line %u",
			  testinc->tk->linenum);
		retres = res = ERR_NCX_INVALID_DUP_INCLUDE;
		ncx_print_errormsg(tkc, mod, res);
	    } else {
		/* warning for same duplicate already in the
		 * include statements
		 */
		inc->usexsd = FALSE;
		log_warn("\nWarning: duplicate include found on line %u",
			 testinc->tk->linenum);
		res = ERR_NCX_DUP_INCLUDE;
		ncx_print_errormsg(tkc, mod, res);
	    }
	} else {
	    /* check simple submodule loop with itself */
	    if (!xml_strcmp(inc->submodule, mod->name)) {
		log_error("\nError: include '%s' for current submodule",
			  mod->name);
		retres = ERR_NCX_INCLUDE_LOOP;
		ncx_print_errormsg(tkc, mod, retres);
	    }

	    /* check simple submodule loop with the top-level file */
	    if (retres == NO_ERR &&
		xml_strcmp(mod->name, pcb->top->name) &&
		!xml_strcmp(inc->submodule, pcb->top->name)) {
		log_error("\nError: include loop for top-level %smodule '%s'",
			  (pcb->top->ismod) ? "" : "sub", inc->submodule);
		retres = ERR_NCX_INCLUDE_LOOP;
		ncx_print_errormsg(tkc, mod, retres);
	    }

	    /* check simple submodule including its parent module */
	    if (retres == NO_ERR &&
		mod->belongs && !xml_strcmp(mod->belongs, inc->submodule)) {
		log_error("\nError: submodule '%s' cannot include its"
			  " parent module '%s'", mod->name, inc->submodule);
		retres = ERR_NCX_INCLUDE_LOOP;
		ncx_print_errormsg(tkc, mod, retres);
	    }

	    /* check for include loop */
	    if (retres == NO_ERR) {
		node = yang_find_node(&pcb->incchainQ, 
				      inc->submodule,
				      inc->revision);
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
	    node->revision = inc->revision;
	    node->mod = mod;
	    node->tk = inc->tk;

	    /* hand off malloced 'inc' struct here */
	    dlq_enque(inc, &mod->includeQ);

	    /* check if already parsed, and in the allincQ */
	    testnode = yang_find_node(&pcb->allincQ, 
				      inc->submodule,
				      inc->revision);
	    if (!testnode) {
		dlq_enque(node, &pcb->incchainQ);

		/* load the module now instead of later for validation */
		retres = ncxmod_load_imodule(inc->submodule, 
					     inc->revision,
					     pcb, YANG_PT_INCLUDE);

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
    } else {
	ncx_free_include(inc);
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
	    CHK_EXIT(res, retres);
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
	    CHK_EXIT(res, retres);
        } else if (!xml_strcmp(val, YANG_K_INCLUDE)) {
	    res = consume_include(tkc, mod, pcb);
	    CHK_EXIT(res, retres);
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
	    CHK_EXIT(res, retres);
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
	    CHK_EXIT(res, retres);
        } else if (!xml_strcmp(val, YANG_K_CONTACT)) {
	    res = yang_consume_descr(tkc, mod, &mod->contact_info,
				     &contact, &mod->appinfoQ);
	    CHK_EXIT(res, retres);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
	    res = yang_consume_descr(tkc, mod, &mod->descr,
				     &descr, &mod->appinfoQ);
	    CHK_EXIT(res, retres);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
	    res = yang_consume_descr(tkc, mod, &mod->ref,
				     &ref, &mod->appinfoQ);
	    CHK_EXIT(res, retres);
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
* FUNCTION consume_revision_date
* 
* Parse the next N tokens as a date-arg-str clause
* Adds the string form to the specified string object
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* NEXT token is the start of the 'date-arg-str' clause
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get the ncx_import_t 
*   revstring == address of return revision date string
*
* OUTPUTS:
*   *revstring malloced and set to date-arg-str value
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_revision_date (tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   xmlChar **revstring)
{
    xmlChar       *str, *p;
    const char    *expstr;
    status_t       res;

    str = NULL;
    expstr = "date-arg-str";
    res = NO_ERR;

    /* get the mandatory version identifier date string */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    if (TK_CUR_STR(tkc)) {
	*revstring = xml_strdup(TK_CUR_VAL(tkc));
	if (!*revstring) {
	    res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
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
	    return res;
	} else {
	    p = str;
	    p += xml_strcpy(p, TK_CUR_VAL(tkc));
	    res = ncx_consume_token(tkc, mod, TK_TT_DNUM);
	    if (res != NO_ERR) {
		if (NEED_EXIT(res)) {
		    m__free(str);
		    return res;
		}
	    } else {
		p += xml_strcpy(p, TK_CUR_VAL(tkc));
		res = ncx_consume_token(tkc, mod, TK_TT_DNUM);
		if (res != NO_ERR) {
		    m__free(str);
		    return res;
		} else {
		    xml_strcpy(p, TK_CUR_VAL(tkc));
		    *revstring = str;
		}
	    }
	}
    } else {
	res = ERR_NCX_WRONG_TKTYPE;
	ncx_mod_exp_err(tkc, mod, res, expstr);
    }

    return res;

}  /* consume_revision_date */


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
    tk_type_t      tktyp;
    boolean        done, descrdone;
    status_t       res, retres;

    val = NULL;
    expstr = "module name";
    done = FALSE;
    descrdone = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    rev = ncx_new_revhist();
    if (!rev) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    } else {
	rev->tk = TK_CUR(tkc);
    }

    /* get the mandatory version identifier date string */
    res = consume_revision_date(tkc, mod, &rev->version);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT(res)) {
	    ncx_free_revhist(rev);
	    return res;
	}
    }

    /* Get the starting left brace for the sub-clauses */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT(res)) {
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
		if (NEED_EXIT(res)) {
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
		if (NEED_EXIT(res)) {
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
	CHK_EXIT(res, retres);
	    
	/* check if the revision is already present */
	testrev = ncx_find_revhist(mod, rev->version);
	if (testrev) {
	    /* error for dup. revision with same version */
	    retres = ERR_NCX_DUP_ENTRY;
	    log_error("\nError: revision with same date on line %u",
		      testrev->tk->linenum);
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
*   pcb == parser control block to use to check rev-date
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_revision_stmts (tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    yang_pcb_t *pcb)
{
    const xmlChar *val;
    const char    *expstr;
    ncx_revhist_t *rev;
    tk_token_t    *savetk;
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
	    CHK_EXIT(res, retres);
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
	    CHK_EXIT(res, retres);
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
		/* first pass through loop */
		val = rev->version;
	    } else {
		ret = yang_compare_revision_dates(rev->version, val);
		if (ret > 0) {
		    log_warn("\nWarning: revision dates not in "
			     "descending order");
		    savetk = TK_CUR(tkc);
		    TK_CUR(tkc) = rev->tk;
		    ncx_print_errormsg(tkc, mod, 
				       ERR_NCX_BAD_REV_ORDER);
		    TK_CUR(tkc) = savetk;
		    val = rev->version;
		}
	    }
	}
    }

    /* assign the module version string */
    if (val) {
	/* valid date string found */
	mod->version = xml_strdup(val);
    }

#if 0
    /**** WAS USED TO SET REVISION TO CUR-DATE ****/
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
    }

    if (val) {
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
#else
    /* leave the version NULL if no good revision dates found */
    if (!val) {
	log_warn("\nWarning: no revision statements "
		 "for %smodule '%s'",
		 (mod->ismod) ? "" : "sub", mod->name);
	mod->warnings++;
    } else if (pcb->revision && 
	       yang_compare_revision_dates(val, pcb->revision)) {

	log_error("\nError: found version '%s' instead of "
		  "requested version '%s",
		  (val) ? val : EMPTY_STRING,
		  pcb->revision);
	retres = ERR_NCX_WRONG_VERSION;
	ncx_print_errormsg(tkc, mod, retres);
    }
#endif

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
	    CHK_EXIT(res, retres);
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
	} else if (!xml_strcmp(val, YANG_K_FEATURE)) {
	    res = consume_feature(tkc, mod);
	} else if (!xml_strcmp(val, YANG_K_IDENTITY)) {
	    res = consume_identity(tkc, mod);
	} else if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
	    res = yang_typ_consume_typedef(tkc, mod, &mod->typeQ);
	} else if (!xml_strcmp(val, YANG_K_GROUPING)) {
	    res = yang_grp_consume_grouping(tkc, mod,
					    &mod->groupingQ, NULL);
	} else if (!xml_strcmp(val, YANG_K_RPC)) {
	    res = yang_obj_consume_rpc(tkc, mod);
	} else if (!xml_strcmp(val, YANG_K_NOTIFICATION)) {
	    res = yang_obj_consume_notification(tkc, mod);
	} else if (!xml_strcmp(val, YANG_K_AUGMENT)) {
	    res = yang_obj_consume_augment(tkc, mod);
	} else if (!xml_strcmp(val, YANG_K_DEVIATION)) {
	    res = yang_obj_consume_deviation(tkc, mod);
	} else {
	    res = yang_obj_consume_datadef(tkc, mod,
					   &mod->datadefQ, NULL);
	}
	CHK_EXIT(res, retres);
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
*             == FALSE if error-exit, not added
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
    yang_node_t    *node;
    ncx_feature_t  *feature;
    ncx_identity_t *identity;
    boolean         ismain, loaded, otherversion;
    status_t        res, retres;

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
    otherversion = FALSE;

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
    CHK_EXIT(res, retres);

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
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    mod->ismod = ismain;

    if (ismain) {
	/* consume module-header-stmts */
	res = consume_mod_hdr(tkc, mod);
	CHK_EXIT(res, retres);
    } else {
	/* consume submodule-header-stmts */
	res = consume_submod_hdr(tkc, mod);
	CHK_EXIT(res, retres);
    }

    /* Get the linkage statements (imports, include) */
    res = consume_linkage_stmts(tkc, mod, pcb);
    CHK_EXIT(res, retres);

    /* Get the meta statements (organization, etc.) */
    res = consume_meta_stmts(tkc, mod, ismain);
    CHK_EXIT(res, retres);

    /* Get the revision statements */
    res = consume_revision_stmts(tkc, mod, pcb);
    CHK_EXIT(res, retres);

    /* make sure there is at least name and prefix to continue
     * do not continue if requested version does not match
     */
    if (retres == ERR_NCX_WRONG_VERSION ||
	!mod->name || (mod->ismod && !mod->prefix) || 
	!*mod->name || (mod->ismod && !*mod->prefix)) {
	return retres;
    }

    /* check if this module is already loaded, except in diff mode */
    if (mod->ismod && !pcb->diffmode &&
	ncx_find_module(mod->name, mod->version)) {
	switch (ptyp) {
	case YANG_PT_TOP:
	    loaded = TRUE;
	    if (ncx_find_module(mod->name, NULL)) {
		/* do not re-register the namespace */
		otherversion = TRUE;
	    }
	    break;
	case YANG_PT_IMPORT:
	    return NO_ERR;
	default:
	    ;
	}
    }

    /* Get the definition statements */
    res = consume_body_stmts(tkc, mod);
    CHK_EXIT(res, retres);

    /* the next node should be the '(sub)module' end node */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    CHK_EXIT(res, retres);

    /* check extra tokens left over */
    res = TK_ADV(tkc);
    if (res == NO_ERR) {
	retres = ERR_NCX_EXTRA_NODE;
	log_error("\nError: Extra input after end of module"
		  " starting on line %u", TK_CUR_LNUM(tkc));
	ncx_print_errormsg(tkc, mod, retres);
    }

    /**************** Module Validation *************************/

    /* check all the module level extension usage */
    res = ncx_resolve_appinfoQ(tkc, mod, &mod->appinfoQ);
    CHK_EXIT(res, retres);

    /* check all the module level extension usage
     * within the include, import, and feature statements
     */
    res = resolve_mod_appinfo(tkc, mod);
    CHK_EXIT(res, retres);

    /* resolve any if-feature statements within the featureQ */
    for (feature = (ncx_feature_t *)dlq_firstEntry(&mod->featureQ);
	 feature != NULL;
	 feature = (ncx_feature_t *)dlq_nextEntry(feature)) {

	res = resolve_feature(tkc, mod, feature);
	CHK_EXIT(res, retres);
    }

    /* check for any if-feature loops caused by this module */
    for (feature = (ncx_feature_t *)dlq_firstEntry(&mod->featureQ);
	 feature != NULL;
	 feature = (ncx_feature_t *)dlq_nextEntry(feature)) {

	res = check_feature_loop(tkc, mod, feature, feature);
	CHK_EXIT(res, retres);
    }

    /* resolve the base-stmt within any identity statements 
     * within the identityQ 
     */
    for (identity = (ncx_identity_t *)dlq_firstEntry(&mod->identityQ);
	 identity != NULL;
	 identity = (ncx_identity_t *)dlq_nextEntry(identity)) {

	res = resolve_identity(tkc, mod, identity);
	CHK_EXIT(res, retres);
    }

    /* resolve any identity base loops within the identityQ */
    for (identity = (ncx_identity_t *)dlq_firstEntry(&mod->identityQ);
	 identity != NULL;
	 identity = (ncx_identity_t *)dlq_nextEntry(identity)) {

	res = check_identity_loop(tkc, mod, identity, identity);
	CHK_EXIT(res, retres);
    }

    /* Validate any module-level typedefs */
    res = yang_typ_resolve_typedefs(tkc, mod, &mod->typeQ, NULL);
    CHK_EXIT(res, retres);

    /* Validate any module-level groupings */
    res = yang_grp_resolve_groupings(tkc, mod, &mod->groupingQ, NULL);
    CHK_EXIT(res, retres);

    /* Validate any module-level data-def-stmts */
    res = yang_obj_resolve_datadefs(tkc, mod, &mod->datadefQ);
    CHK_EXIT(res, retres);

    /* Expand and validate any uses-stmts within module-level groupings */
    res = yang_grp_resolve_complete(tkc, mod, &mod->groupingQ, NULL);
    CHK_EXIT(res, retres);

    /* Expand and validate any uses-stmts within module-level datadefs */
    res = yang_obj_resolve_uses(tkc, mod, &mod->datadefQ);
    CHK_EXIT(res, retres);

    /* Expand and validate any augment-stmts within module-level datadefs */
    res = yang_obj_resolve_augments(tkc, mod, &mod->datadefQ);
    CHK_EXIT(res, retres);

    /* Expand and validate any deviation-stmts within the module
     * Only expand if source mode is XML
     * Or if it is XMLDOC mode, but 'cooked-mode' output
     */
    res = yang_obj_resolve_deviations(pcb, tkc, mod);
    CHK_EXIT(res, retres);

    /* One final check for grouping integrity */
    res = yang_grp_resolve_final(tkc, mod, &mod->groupingQ);
    CHK_EXIT(res, retres);

    /* One final check for object integrity */
    res = yang_obj_resolve_final(tkc, mod, &mod->datadefQ);
    CHK_EXIT(res, retres);

    /* Validate all the XPath expressions within all cooked objects */
    res = yang_obj_resolve_xpath(tkc, mod, &mod->datadefQ);
    CHK_EXIT(res, retres);

    /* check for loops in any leafref XPath targets */
    res = yang_obj_check_leafref_loops(tkc, mod, &mod->datadefQ);
    CHK_EXIT(res, retres);

    /* Check for imports not used warnings */
    yang_check_imports_used(tkc, mod);

    /* save the module parse status */
    mod->status = retres;

    /* make sure there is at least name and prefix to continue */
    if (!mod->name || !ncx_valid_name2(mod->name)) {
	return retres;
    }
    if (mod->ismod) {
	if (!mod->prefix || !ncx_valid_name2(mod->prefix)) {
	    return retres;
	}
    } else if (mod->prefix) {
	if (!ncx_valid_name2(mod->prefix)) {
	    return retres;
	}
    } else {
	mod->prefix = xml_strdup(EMPTY_STRING);
	if (!mod->prefix) {
	    return ERR_INTERNAL_MEM;
	}
    }

    /* add the NS definitions to the registry;
     * check the parse type first
     */
    switch (ptyp) {
    case YANG_PT_TOP:
    case YANG_PT_IMPORT:
	/* add this regular module to the registry */
	if (mod->ismod || pcb->top == mod) {

	    if (pcb->top == mod) {
		dlq_block_enque(mod->allimpQ, &mod->saveimpQ);
		dlq_block_enque(mod->allincQ, &mod->saveincQ);
	    }  /* else leave the pcb->allimpQ and pcb->allincQ alone */
	    mod->allincQ = NULL;
	    mod->allimpQ = NULL;

	    if (!loaded) {
		if (pcb->diffmode) {
		    res = ncx_add_to_modQ(mod);
		} else {
		    res = ncx_add_to_registry(mod, otherversion);
		}
		if (res != NO_ERR) {
		    retres = res;
		} else {
		    /* if mod==top, and top is a submodule, then 
		     * yang_free_pcb will delete the submodule later
		     */
		    *wasadded = TRUE;
		}
	    }
	} else {
	    mod->nsid = xmlns_find_ns_by_name(mod->ns);
	}
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
	    node->revision = mod->version;
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
    str = NULL;

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
	    m__free(str);
	    str = NULL;
	} else {
	    /* save the source of this ncx-module for monitor / debug 
	     * hand off malloced src string here
	     */
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

	    /* set the stmt-track mode flag to the master flag in the PCB */
	    mod->stmtmode = pcb->stmtmode;

	    /* set the diff-mode flag */
	    mod->diffmode = pcb->diffmode;
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
		if (pcb->top == mod) {
		    pcb->top = NULL;
		}
		ncx_free_module(mod);
	    }
	} else if (!wasadd && !pcb->diffmode) {
	    if (mod->ismod) {
		if (pcb->top == mod) {
		    /* swap with the real module already done */
		    pcb->top = ncx_find_module(mod->name,
					       mod->version);
		}
	    } else if (!pcb->with_submods) {
		/* subtree parsing mode can cause top-level to already
		 * be loaded into the registry, swap out the new dummy
		 * module with the real one
		 */
		if (pcb->top == mod) {
		    pcb->top = ncx_find_module(mod->belongs,
					       mod->version);
		}
	    } else if (pcb->top == mod) {
		pcb->top = NULL;
	    }
	    ncx_free_module(mod);
	    if (!pcb->top && !pcb->with_submods) {
		res = ERR_NCX_MOD_NOT_FOUND;
	    }
	}  /* else the module went into an alternate mod Q */
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
