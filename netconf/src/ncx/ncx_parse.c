/*  FILE: ncx_parse.c


    This module parses an NCX Module as defined in the 
    following ABNF:

          --- see netconf/doc/syntax.txt ---

    The NCX file is converted to a C ncx_module_t
    data structure during parsing before the definitions
    are finally registered with the def_reg module.

    TWO PASS PARSER
      1) Convert module text to a token chain (tk_chain_t)
      2) Convert a token chain to a module (ncx_module_t)

    Parser Convention Notes:
    - Token advancement
      - Consuming a token means advancing the token chain to 
        that token, validating and processing that token.
      - The token pointer (tkc->cur) is left on the last 
        token consumed by any particular sub-function.
      - All parser functions must follow this convention


*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
12nov05      abb      begun

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

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ncx_parse
#include "ncx_parse.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_typ_parse
#include  "typ_parse.h"
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
/* #define NCX_PARSE_DEBUG 1 */
/* #define NCX_PARSE_TK_DEBUG 1 */
/* #define NCX_PARSE_RDLN_DEBUG 1 */
#endif

/* bitmask for header fields to know if entered */
#define B_HDR_DESCRIPTION  bit0
#define B_HDR_VERSION      bit1
#define B_HDR_OWNER        bit2
#define B_HDR_APPLICATION  bit3
#define B_HDR_COPYRIGHT    bit4
#define B_HDR_CONTACT_INFO bit5
#define B_HDR_NAMESPACE    bit6
#define B_HDR_LAST_UPDATE  bit7
#define B_HDR_REV_HISTORY  bit8
#define B_HDR_APPINFO      bit9

#define M_HDR_MANDATORY  (B_HDR_VERSION | B_HDR_OWNER | B_HDR_APPLICATION)

/* bitmask for type fields to know if entered */
#define B_TYP_DESCRIPTION  bit0
#define B_TYP_CONDITION    bit1
#define B_TYP_SYNTAX       bit2
#define B_TYP_METADATA     bit3
#define B_TYP_DEFAULT      bit4
#define B_TYP_MAX_ACCESS   bit5
#define B_TYP_DATA_CLASS   bit6
#define B_TYP_UNITS        bit7
#define B_TYP_APPINFO      bit8

#define M_TYP_MANDATORY    B_TYP_SYNTAX

/* bitmask for parmset fields to know if entered */
#define B_PSD_DESCRIPTION  bit0
#define B_PSD_CONDITION    bit1
#define B_PSD_ORDER        bit2
#define B_PSD_TYPE         bit3
#define B_PSD_DATA_CLASS   bit4
#define B_PSD_APPINFO      bit5
#define B_PSD_PARMS        bit6

#define M_PSD_MANDATORY    B_PSD_PARMS

/* bitmask for parm fields to know if entered */
#define B_PAR_DESCRIPTION  bit0
#define B_PAR_CONDITION    bit1
#define B_PAR_TYPE         bit2
#define B_PAR_MAX_ACCESS   bit3
#define B_PAR_USAGE        bit4
#define B_PAR_DATA_CLASS   bit5
#define B_PAR_APPINFO      bit6

#define M_PAR_MANDATORY    B_PAR_TYPE

/* bitmask for rpc fields to know if entered */
#define B_RPC_DESCRIPTION  bit0
#define B_RPC_CONDITION    bit1
#define B_RPC_TYPE         bit2
#define B_RPC_INPUT        bit3
#define B_RPC_PARMS        bit4
#define B_RPC_OUTPUT       bit5
#define B_RPC_APPINFO      bit6

#define M_RPC_MANDATORY    B_RPC_TYPE


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* callback function for consume_container function */
typedef status_t (*ncx_containedFn_t) 
    (tk_chain_t *tkc, ncx_module_t  *mod);


/********************************************************************
* FUNCTION consume_version_val
* 
* Consume a version value string
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   verstr == addr of the string to get the version value
*          == NULL if the string should not be saved
* OUTPUTS:
*   *verstr points to a malloces string containing the version
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_version_val (tk_chain_t *tkc,
			 ncx_module_t *mod,
			 xmlChar **verstr)
{
    status_t      res;

    res = TK_ADV(tkc);
    if (res == NO_ERR) {
	if (TK_CUR_LEN(tkc) && TK_CUR_LEN(tkc) <= NCX_MAX_NLEN) {
	    if (verstr) {
		*verstr = xml_strdup(TK_CUR_VAL(tkc));
		if (!*verstr) {
		    res = ERR_INTERNAL_MEM;
		}
	    }
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
    }
    return res;

}  /* consume_version_val */


/********************************************************************
* FUNCTION consume_version
* 
* Consume a version string
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   verstr == addr of the string to get the version value
*
* OUTPUTS:
*   *verstr points to a malloces string containing the version
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_version (tk_chain_t *tkc,
		     ncx_module_t  *mod,
		     xmlChar **verstr)
{
    status_t      res;

    /* Get the mandatory 'version' field 
     * Cannot use normal ncx_consume_name or ncx_consume_string
     * because version field has no restrictions on content
     * and likely to be a number
     */
    res = ncx_consume_tstring(tkc, mod, NCX_EL_VERSION, FALSE);
    if (res != NO_ERR) {
        return res;
    }

    res = consume_version_val(tkc, mod, verstr);
    if (res != NO_ERR) {
        return res;
    }

    res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    if (res != NO_ERR) {
	if (*verstr) {
	    m__free(*verstr);
	    *verstr = NULL;
	}
    }

    return res;

}  /* consume_version */


/********************************************************************
* FUNCTION consume_revhist_entry
* 
* Check if a revision-history sub-clause is present
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_revhist_entry (tk_chain_t *tkc,
			   ncx_module_t  *mod)
{
    status_t       res;
    ncx_revhist_t *revhist;

    /* right brace means revision-history is done */
    if (tk_next_typ(tkc)==TK_TT_RBRACE) {
	return ERR_NCX_SKIPPED;
    }

    revhist = NULL;
    res = NO_ERR;

    /* only malloc a new record if descr strings are being saved */
    if (ncx_save_descr()) {
	revhist = ncx_new_revhist();
	if (!revhist) {
	    res = ERR_INTERNAL_MEM;
	}
    }

    /* get the version identifier */
    if (res == NO_ERR) {
	res = consume_version_val(tkc, mod, revhist ? 
				  &revhist->version : NULL);
    }

    /* get the description string */
    if (res==NO_ERR) {
	res = TK_ADV(tkc);
	if (res==NO_ERR) {
	    if (!TK_CUR_WSTR(tkc)) {
		res = ERR_NCX_WRONG_TKTYPE;
	    }
	}
    }

    /* save a copy of the description string */
    if (res==NO_ERR && revhist) {
	revhist->descr = xml_strdup(TK_CUR_VAL(tkc));
	if (!revhist->descr) {
	    res = ERR_INTERNAL_MEM;
	}
    }

    /* check any errors so far */
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	if (revhist) {
	    ncx_free_revhist(revhist);
	}
    } else {
	/* terminating semi-colon expected */
	res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    }

    /* save the entry even if the semicol is missing */
    if (revhist) {
	dlq_enque(revhist, &mod->revhistQ);
    }

    return res;

}  /* consume_revhist_entry */


/********************************************************************
* FUNCTION consume_revision_history
* 
* Check if a revision-history clause is present
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_revision_history (tk_chain_t *tkc,
			      ncx_module_t  *mod)
{
    status_t       res;
    boolean        done;

    /* check if optional revision-history keyword is present */
    res = ncx_consume_tstring(tkc, mod, NCX_EL_REVISION_HISTORY, TRUE);
    if (res!=NO_ERR) {
	/* may be a real error or just skipped */
        return res;
    }

    /* got the token, so the left brace must be present */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	return res;
    }

    done = FALSE;
    while (!done) {
	res = consume_revhist_entry(tkc, mod);
	if (res != NO_ERR) {
	    done = TRUE;
	}
    }

    /* get the closing right brace */
    if (res==NO_ERR || res==ERR_NCX_SKIPPED) {
	res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    }

    return res;

}  /* consume_revision_history */


/********************************************************************
* FUNCTION consume_header
* 
* Parse the next N tokens as an NCX module header
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_header (tk_chain_t *tkc,
                    ncx_module_t  *mod)
{
    const xmlChar *val;
    tk_type_t      tktyp;
    uint32         flags;
    status_t       res, retres;
    boolean        done;

    /* Process the 'header' token string */
    res = ncx_consume_tstring(tkc, mod, NCX_EL_HEADER, NCX_REQ);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* Process the opening left brace token */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    flags = 0;
    done = FALSE;
    retres = NO_ERR;

    /* loop through all the sub-clauses present */
    while (!done) {
	/* look ahead to the next token */
	tktyp = tk_next_typ(tkc);
	val = tk_next_val(tkc);

	/* check the next token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* sub-clause assumed */
	case TK_TT_RBRACE:
	    retres = TK_ADV(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    done = TRUE;
	    continue;
 	}
	    
	/* Got a token string so check the value */
        if (!xml_strcmp(val, NCX_EL_DESCRIPTION)) {
	    if (flags & B_HDR_DESCRIPTION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'description' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DESCRIPTION, 
					 ncx_save_descr() ? 
					 &mod->descr : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_DESCRIPTION;
	    }
	} else if (!xml_strcmp(val, NCX_EL_VERSION)) {
	    if (flags & B_HDR_VERSION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the mandatory 'version' field */
	    res = consume_version(tkc, mod, &mod->version);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_VERSION;
	    }
	} else if (!xml_strcmp(val, NCX_EL_OWNER)) {
	    if (flags & B_HDR_OWNER) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the mandatory 'owner' name */
	    res = ncx_consume_name(tkc, mod, NCX_EL_OWNER, &mod->owner, 
				   NCX_REQ, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_OWNER;
	    }
	} else if (!xml_strcmp(val, NCX_EL_APPLICATION)) {
	    if (flags & B_HDR_APPLICATION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the mandatory 'application' name */
	    res = ncx_consume_name(tkc, mod, NCX_EL_APPLICATION, 
				   &mod->app, NCX_REQ, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_APPLICATION;
	    }
	} else if (!xml_strcmp(val, NCX_EL_COPYRIGHT)) {
	    if (flags & B_HDR_COPYRIGHT) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'copyright' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_COPYRIGHT, 
					 ncx_save_descr() ? 
					 &mod->copyright : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
		continue;
	    } else {
		flags |= B_HDR_COPYRIGHT;
	    }
	} else if (!xml_strcmp(val, NCX_EL_CONTACT_INFO)) {
	    if (flags & B_HDR_CONTACT_INFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'contact-info' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_CONTACT_INFO, 
					 ncx_save_descr() ? 
					 &mod->contact_info : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_CONTACT_INFO;
	    }

	} else if (!xml_strcmp(val, NCX_EL_NAMESPACE)) {
	    if (flags & B_HDR_NAMESPACE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'namespace' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_NAMESPACE, 
					 &mod->ns, NCX_OPT, 
					 NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_NAMESPACE;
	    }
	} else if (!xml_strcmp(val, NCX_EL_LAST_UPDATE)) {
	    if (flags & B_HDR_LAST_UPDATE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'last-update' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_LAST_UPDATE, 
					 &mod->last_update, NCX_OPT, 
					 NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_LAST_UPDATE;
	    }
	} else if (!xml_strcmp(val, NCX_EL_REVISION_HISTORY)) {
	    if (flags & B_HDR_REV_HISTORY) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'revision-history' clause is present */
	    res = consume_revision_history(tkc, mod);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_REV_HISTORY;
	    }
	} else if (!xml_strcmp(val, NCX_EL_APPINFO)) {
	    if (flags & B_HDR_APPINFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'appinfo' clause is present */
	    res = ncx_consume_appinfo(tkc, mod, &mod->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_HDR_APPINFO;
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    done = TRUE;
	}
    }

    /* check if the header was parsed okay */
    if (retres == ERR_NCX_ENTRY_EXISTS) {
	res = TK_ADV(tkc);
    } else if (retres == NO_ERR) {
	if ((flags & M_HDR_MANDATORY) != M_HDR_MANDATORY) {
	    retres = ERR_NCX_DATA_MISSING;
	}
    }

    if (retres != NO_ERR) {
	ncx_print_errormsg(tkc, mod, retres);
    }

    return retres;

}  /* consume_header */


/********************************************************************
* FUNCTION consume_import_items
* 
* Parse the import.items token chain as a list of item names
* Create an ncx_import_item_t for each one and add it
* to the import->itemQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   import == ncx_import_t struct in progress
*   tkc    == token chain
*   mod == module in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_import_items (ncx_import_t  *import,
                          tk_chain_t  *tkc,
			  ncx_module_t *mod)
{
    ncx_import_item_t *item;
    boolean            done;
    status_t           res;

    res = NO_ERR;
    done = FALSE;
    while (!done) {
        if (tk_next_typ(tkc)==TK_TT_RBRACE) {
            done = TRUE;
        } else {
            /* get the TSTRING item name */
            res = TK_ADV(tkc);
            if (res == NO_ERR) {
		if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
		    res = ERR_NCX_WRONG_TKTYPE;
		}
	    }

	    if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
		return res;
	    }

            /* save the TSTRING value */
            item = ncx_new_import_item();
            if (!item) {
                res = ERR_INTERNAL_MEM;
            } else {
		item->name = xml_strdup(TK_CUR_VAL(tkc));
		if (!item->name) {
		    ncx_free_import_item(item);
		    res = ERR_INTERNAL_MEM;
		} else {
		    dlq_enque(item, &import->itemQ);
		}
	    }

	    if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
		return res;
	    }
	}
    }
    return NO_ERR;

}  /* consume_import_items */


/********************************************************************
* FUNCTION consume_import
* 
* Parse the next N tokens as an <import> subtree
* Create a ncx_import struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod   == module struct that will get the ncx_import_t 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_import (tk_chain_t *tkc,
                    ncx_module_t  *mod)
{
    ncx_import_t    *imp;
    status_t         res;

    /* peek ahead to see if the section is done */
    if (tk_next_typ(tkc)==TK_TT_RBRACE) {
	return ERR_PARS_SECDONE;  
    }

    /* Get a new ncx_import_t to fill in */
    imp = ncx_new_import();
    if (!imp) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* Get the mandatory module name */
    res = ncx_consume_name(tkc, mod, NCX_EL_IMPORT, &imp->module, 
        NCX_REQ, TK_TT_NONE);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        ncx_free_import(imp);
        return res;
    }

    /* peek ahead to see if the section is done */
    if (tk_next_typ(tkc)==TK_TT_SEMICOL) {
        /* no items specified */
        dlq_enque(imp, &mod->importQ);
        TK_ADV(tkc);
	return NO_ERR;
    }

    /* else there must be an items list */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        ncx_free_import(imp);
        return res;
    }
    
    /* get all the import items */
    res = consume_import_items(imp, tkc, mod);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        ncx_free_import(imp);
        return res;
    }

    /* get the 'import' end tag */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    if (res == NO_ERR) {
        res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    }
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	ncx_free_import(imp);
	return res;
    }	

    /* success, add the entry at the end of the import queue */
    dlq_enque(imp, &mod->importQ);
    return NO_ERR;

}  /* consume_import */


/********************************************************************
* FUNCTION consume_type
* 
* Parse an NCX module 'type' section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod  == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_type (tk_chain_t *tkc,
                  ncx_module_t   *mod)
{
    typ_template_t  *typ;
    xmlChar         *str;
    const xmlChar   *val;
    tk_type_t        tktyp;
    uint32           flags;
    status_t         res, retres;
    boolean          done, doerr;
    ncx_access_t     maxacc;
    ncx_data_class_t dataclass;
    dlq_hdr_t        appinfoQ;

    /* peek ahead to see if the section is done */
    if (!(tk_next_typ(tkc)==TK_TT_TSTRING && tk_next_val(tkc) &&
          !xml_strcmp(NCX_EL_TYPE, tk_next_val(tkc)))) {
	return ERR_PARS_SECDONE;  
    }

    /* malloc a new type struct */
    typ = typ_new_template();
    if (!typ) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    typ->mod = mod;
    typ->tk = TK_CUR(tkc);   /* only valid during parsing */
    typ->linenum = typ->tk->linenum;

    /* Already checked for a 'type' start tag, so this should work */
    res = ncx_consume_name(tkc, mod, NCX_EL_TYPE, &typ->name, 
			   NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        typ_free_template(typ);
        return res;
    }

    flags = 0;
    done = FALSE;
    doerr = FALSE;
    retres = NO_ERR;
    dlq_createSQue(&appinfoQ);

    /* loop through all the sub-clauses present */
    while (!done) {
	/* look ahead to the next token */
	tktyp = tk_next_typ(tkc);
	val = tk_next_val(tkc);

	/* check the next token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    done = TRUE;
	    doerr = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* sub-clause assumed */
	case TK_TT_RBRACE:
	    (void)TK_ADV(tkc);
	    done = TRUE;
	    continue;
	default:
	    (void)TK_ADV(tkc);
	    retres = ERR_NCX_WRONG_TKTYPE;
	    done = TRUE;
	    doerr = TRUE;
	    continue;
 	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, NCX_EL_DESCRIPTION)) {
	    if (flags & B_TYP_DESCRIPTION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'description' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DESCRIPTION, 
					 ncx_save_descr() ? 
					 &typ->descr : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_DESCRIPTION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_CONDITION)) {
	    if (flags & B_TYP_CONDITION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'condition' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_CONDITION, 
					 ncx_save_descr() ? 
					 &typ->condition : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_CONDITION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_SYNTAX)) {
	    if (flags & B_TYP_SYNTAX) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* get start of syntax clause */
	    res = ncx_consume_tstring(tkc, mod, NCX_EL_SYNTAX, NCX_REQ);
	    if (res==NO_ERR) {
		res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
		if (res == NO_ERR) {
		    /* convert the syntax contents to internal format */
		    res = typ_parse_syntax_contents(mod, tkc, typ);
		    if (res == NO_ERR) {
			/* get the 'syntax' end tag */
			res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
		    }
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_SYNTAX;
	    }
        } else if (!xml_strcmp(val, NCX_EL_METADATA)) {
	    if (flags & B_TYP_METADATA) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* check for an optional metadata clause */
	    res = ncx_consume_tstring(tkc, mod, NCX_EL_METADATA, NCX_OPT);
	    if (res == NO_ERR) {
		res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
		if (res == NO_ERR) {
		    /* convert the metadata contents to internal format */
		    res = typ_parse_metadata_contents(mod, tkc, typ);
		    if (res == NO_ERR) {
			/* get the 'metadata' end tag */
			res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
		    }
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_METADATA;
	    }
        } else if (!xml_strcmp(val, NCX_EL_DEFAULT)) {
	    if (flags & B_TYP_DEFAULT) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Check if the optional 'default' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DEFAULT, 
					 &typ->defval, NCX_OPT, 
					 NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_DEFAULT;
	    }
        } else if (!xml_strcmp(val, NCX_EL_MAX_ACCESS)) {
	    if (flags & B_TYP_MAX_ACCESS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Check for the optional 'max-access' field
	     * If missing, then leave as 'not set', and the
	     * parent node will be used to determine the access
	     *
	     * This must be after the syntax clause is parsed and the 
	     * typ.typdef struct has been filled in, 
	     * so saved in 'maxacc' for now
	     */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_MAX_ACCESS, &str, 
					 NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		maxacc = ncx_get_access_enum(str);
		m__free(str);
		if (maxacc == NCX_ACCESS_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		    doerr = TRUE;
		}
	    }

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_MAX_ACCESS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_DATA_CLASS)) {
	    if (flags & B_TYP_DATA_CLASS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Get the optional 'data-class' field */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DATA_CLASS,
					 &str, NCX_OPT, NCX_NO_WSP, 
					 TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		dataclass = ncx_get_data_class_enum(str);
		m__free(str);
		if (dataclass==NCX_DC_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		    doerr = TRUE;
		}
	    }

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_DATA_CLASS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_UNITS)) {
	    if (flags & B_TYP_UNITS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* check if the optional 'units' clause is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_UNITS, 
					 &typ->units, NCX_OPT, NCX_WSP, 
					 TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_UNITS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_APPINFO)) {
	    if (flags & B_TYP_APPINFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* check if the optional 'appinfo' clause is present 
	     * This must be after the syntax clause is parsed and the 
	     * typ.typdef struct has been filled in, so it is saved in
	     * the temp 'appinfoQ'
	     */
	    res = ncx_consume_appinfo(tkc, mod, &appinfoQ);

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_TYP_APPINFO;
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    doerr = TRUE;
	    done = TRUE;
	}
    }

    /* check if the type was parsed okay */
    if (retres == ERR_NCX_ENTRY_EXISTS) {
	doerr = TRUE;
	(void)TK_ADV(tkc);
    } else if (retres == NO_ERR) {
	if ((flags & M_TYP_MANDATORY) != M_TYP_MANDATORY) {
	    doerr = TRUE;
	    retres = ERR_NCX_DATA_MISSING;
	}
    }

    /* check for a duplicate entry error in this module */
    if (retres == NO_ERR && ncx_is_duplicate(mod, typ->name)) {
	doerr = TRUE;
	retres = ERR_NCX_DUP_ENTRY;
    }

    /* finish up the clauses that were cached locally */
    if (retres == NO_ERR) {
	/* check max-access set or not */
	if (flags & B_TYP_MAX_ACCESS) {
	    typ->typdef.maxaccess = maxacc;
	} else {
	    typ->typdef.maxaccess = NCX_ACCESS_NONE;
	}

	/* check data-class set or not */
	if (flags & B_TYP_DATA_CLASS) {
	    typ->typdef.dataclass = dataclass;
	} else {
	    if (typ->typdef.maxaccess == NCX_ACCESS_RO) {
		typ->typdef.dataclass = NCX_DC_STATE;
	    } else {
		typ->typdef.dataclass = NCX_DC_NONE;  /* not set */
	    }
	}

	/* check appinfoQ set or not */
	if (flags & B_TYP_APPINFO) {
	    dlq_block_enque(&appinfoQ, &typ->typdef.appinfoQ);
	}
    } else {
	if (doerr) {
	    ncx_print_errormsg(tkc, mod, retres);
	}
	if (!dlq_empty(&appinfoQ)) {
	    ncx_clean_appinfoQ(&appinfoQ);
	}
	typ_free_template(typ);
	return retres;
    }

#ifdef NCX_PARSE_DEBUG
    log_debug3("\nncx_parse: adding type (%s) to mod (%s)", 
	    typ->name, mod->name);
#endif

    /* add the type to the module type Que */
    dlq_enque(typ, &mod->typeQ);
    return NO_ERR;

}  /* consume_type */


/********************************************************************
* FUNCTION consume_parm_contents
* 
* Parse the next N tokens as the contents of a 'parm' clause
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* The 'parm' keyword and opening left brace have already been parsed
* This function will consume up to and including the matching right
* brace, if no errors
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   parm   == psd_parm_t  in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_parm_contents (ncx_module_t *mod,
			   tk_chain_t  *tkc,
                           psd_parm_t  *parm)
{

    xmlChar         *str;
    const xmlChar   *val;
    typ_template_t  *typ;
    tk_type_t        tktyp;
    uint32           flags;
    boolean          done, doerr;
    status_t         res, retres;

    flags = 0;
    done = FALSE;
    doerr = FALSE;
    retres = NO_ERR;

    /* loop through all the sub-clauses present */
    while (!done) {
	/* look ahead to the next token */
	tktyp = tk_next_typ(tkc);
	val = tk_next_val(tkc);

	/* check the next token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    done = TRUE;
	    doerr = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* sub-clause assumed */
	case TK_TT_RBRACE:
	    (void)TK_ADV(tkc);
	    done = TRUE;
	    continue;
	default:
	    (void)TK_ADV(tkc);
	    retres = ERR_NCX_WRONG_TKTYPE;
	    done = TRUE;
	    doerr = TRUE;
	    continue;
 	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, NCX_EL_DESCRIPTION)) {
	    if (flags & B_PAR_DESCRIPTION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'description' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DESCRIPTION, 
					 ncx_save_descr() ? 
					 &parm->descr : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PAR_DESCRIPTION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_CONDITION)) {
	    if (flags & B_PAR_CONDITION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'condition' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_CONDITION, 
					 ncx_save_descr() ? 
					 &parm->condition : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PAR_CONDITION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_TYPE)) {
	    if (flags & B_PAR_TYPE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Get the mandatory 'type' field */
	    res = ncx_consume_mname(tkc, mod, NCX_EL_TYPE, &parm->typname, 
				    &parm->modstr, NCX_REQ, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		/* find the type template */
		res = typ_locate_template(mod, parm->modstr, parm->typname,
					  (typ_template_t **)&parm->pdef);
	    }
	    if (res != NO_ERR) {
		retres = res;
		doerr = TRUE;
		done = TRUE;
	    } else {
		flags |= B_PAR_TYPE;
	    }
        } else if (!xml_strcmp(val, NCX_EL_MAX_ACCESS)) {
	    if (flags & B_PAR_MAX_ACCESS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Check for the optional 'max-access' field */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_MAX_ACCESS, &str, 
					 NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		parm->access = ncx_get_access_enum(str);
		m__free(str);
		if (parm->access==NCX_ACCESS_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		    doerr = TRUE;
		}
	    }

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PAR_MAX_ACCESS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_USAGE)) {
	    if (flags & B_PAR_USAGE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Check for optional 'usage' clause */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_USAGE, &str, 
					 NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		parm->usage = psd_get_usage_enum(str);
		m__free(str);
		if (parm->usage==PSD_UT_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		    doerr = TRUE;
		}
	    }

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PAR_USAGE;
	    }
        } else if (!xml_strcmp(val, NCX_EL_DATA_CLASS)) {
	    if (flags & B_PAR_DATA_CLASS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Get the optional 'data-class' field */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DATA_CLASS, &str, 
					 NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		parm->dataclass = ncx_get_data_class_enum(str);
		m__free(str);
		if (parm->dataclass==NCX_DC_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		    doerr = TRUE;
		}
	    }

	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PAR_DATA_CLASS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_APPINFO)) {
	    if (flags & B_PAR_APPINFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Check if the optional 'appinfo' clause is present */
	    res = ncx_consume_appinfo(tkc, mod, &parm->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PAR_APPINFO;
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    done = TRUE;
	    doerr = TRUE;
	}
    }

    /* check if the parm was parsed okay */
    if (retres == ERR_NCX_ENTRY_EXISTS) {
	doerr = TRUE;
	(void)TK_ADV(tkc);
    } else if (retres == NO_ERR) {
	if ((flags & M_PAR_MANDATORY) != M_PAR_MANDATORY) {
	    doerr = TRUE;
	    retres = ERR_NCX_DATA_MISSING;
	}
    }

    /* check if any defaults need to be set */
    if (retres == NO_ERR) {
	/* set usage if not provided */
	if (!(flags & B_PAR_USAGE)) {
	    /* if condition clause present then default is conditional */
	    if (parm->condition) {
		parm->usage = PSD_UT_CONDITIONAL;
	    } else {
		parm->usage = PSD_UT_MANDATORY;
	    }
	}

	/* set data-class if not provided */
	if (!(flags & B_PAR_DATA_CLASS)) {
	    /* if the max-access is read-only, default is PSD_PC_STATE
	     * otherwise the default is PSD_PC_CONFIG
	     */
	    typ = parm->pdef;
	    if (typ->typdef.maxaccess == NCX_ACCESS_RO) {
		parm->dataclass = NCX_DC_STATE;
	    } else {
		parm->dataclass = NCX_DC_NONE;  /* still not set */
	    }
	}

	/* set max-access if not provided */
	if (!(flags & B_PAR_MAX_ACCESS)) {
	    if (parm->dataclass==NCX_DC_STATE) {
		parm->access = NCX_ACCESS_RO;
	    } /* else still not set */
	}
    } else if (doerr) {
	ncx_print_errormsg(tkc, mod, res);
    }

    return retres;

}  /* consume_parm_contents */


/********************************************************************
* FUNCTION consume_parm_cmn
* 
* Common sub-parser for regular parms or choice parms
* Parse the next N nodes as a 'parm' subtree
* Create a psd_parm struct and add it to the specified psd
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
* OUTPUTS:
*   *ret == pointer to new parm, if NO_ERR; NULL otherwise
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
	consume_parm_cmn (ncx_module_t *mod,
			  tk_chain_t *tkc,
			  psd_parm_t  **ret)
{
    psd_parm_t      *parm;
    status_t        res;

    /* get a new parameter */
    parm = psd_new_parm();
    if (!parm) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    /* the next nodes should be a <parm> start tag, name, left brace */
    res = ncx_consume_name(tkc, mod, NCX_EL_PARM, &parm->name, 
                           NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
        psd_free_parm(parm);
        return res;
    }

    /* get the parm contents and end tag */
    res = consume_parm_contents(mod, tkc, parm);
    if (res != NO_ERR) {
        psd_free_parm(parm);
        return res;
    }

    *ret = parm;
    return NO_ERR;

}  /* consume_parm_cmn */


/********************************************************************
* FUNCTION consume_parm
* 
* Parse the next N nodes as a 'parm' subtree
* Create a psd_parm struct and add it to the specified psd
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   psd    == parmsetdef struct that will get the psd_parm_t 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_parm (ncx_module_t  *mod,
		  tk_chain_t *tkc,
		  psd_template_t  *psd)
{
    psd_parm_t      *parm;
    status_t        res;

    res = consume_parm_cmn(mod, tkc, &parm);
    if (res != NO_ERR) {
        return res;
    }

    /* check if this is a duplicate */
    if (psd_find_parm(psd, parm->name)) {
	res = ERR_NCX_DUP_ENTRY;
	ncx_print_errormsg(tkc, mod, res);
	psd_free_parm(parm);
	return res;
    }

    /* set the parm ID to the next ID and bump the parm count */
    parm->parm_id = ++psd->parmcnt;
    parm->parent = psd;

    /* check data class mismatch */
    if (parm->dataclass==NCX_DC_NONE) {
	parm->dataclass = psd->dataclass;
    }

    /* 1st pass parsing success, add the entry at the end of the queue */
    dlq_enque(parm, &psd->parmQ);
    return NO_ERR;

}  /* consume_parm */


/********************************************************************
* FUNCTION consume_cparm
* 
* Parse the next N nodes as a parm within a choice or
* choice block decl
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   psd  == parmsetdef struct in progress to verify against
*   pch == psd_choice_t in progress, that will get the psd_parm_t
*          if the pb parm is NULL
*   pb == psd_block_t that will get the psd_parm_t struct
*      == NULL if this is not a choice block decl in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_cparm (ncx_module_t  *mod,
		   tk_chain_t *tkc,
		   psd_template_t  *psd,
		   psd_choice_t  *pch,
		   psd_block_t *pb)
{
    psd_parm_t      *parm, *test;
    status_t         res;

    res = consume_parm_cmn(mod, tkc, &parm);
    if (res != NO_ERR) {
        return res;
    }

    /* check if this is a duplicate in the PSD in progress */
    if (psd_find_parm(psd, parm->name)) {
	res = ERR_NCX_DUP_ENTRY;
    }

    /* make sure this parm is not already in the choice in progress */
    if (res==NO_ERR) {
	test = psd_search_choice(pch, parm->name);
	if (test) {
	    res = ERR_NCX_DUP_ENTRY;
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	psd_free_parm(parm);
	return res;
    }

    /* make sure this parm is not already in the block in progress */
    if (pb) {
        test = psd_search_block(pb, parm->name);
        if (test) {
	    res = ERR_NCX_DUP_ENTRY;
	    ncx_print_errormsg(tkc, mod, res);
            psd_free_parm(parm);
            return res;
        }
	parm->block_id = pb->block_id;
        dlq_enque(parm, &pb->blockQ);
    }

    /* set the choice ID, parm ID and bump the parm count */
    parm->choice_id = pch->choice_id;
    parm->parm_id = ++psd->parmcnt;
    parm->parent = psd;

    /* check if this is not part of a block */
    if (!pb) {
	dlq_enque(parm, &pch->choiceQ);
    }

    return NO_ERR;

}  /* consume_cparm */


/********************************************************************
* FUNCTION consume_pblock
* 
* Parse the next N nodes as a choice block decl
* Create a psd_parm_t struct for each block member 
* and add it to a new psd_block_t, then add that
* to the specified psd_choice_t
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   psd == parmsetdef struct to verify against
*   pch == psd_choice_t that will get the psd_block_t struct
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_pblock (ncx_module_t *mod,
		    tk_chain_t *tkc,
		    psd_template_t  *psd,
		    psd_choice_t *pch)
{
    psd_block_t  *pb;
    status_t      res;
    boolean       done;

    /* consume the opening left bracket of the block decl */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACK);
    if (res != NO_ERR) {
        return res;
    }

    /* get a new choice block header */
    pb = psd_new_block();
    if (!pb) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    /* set the block start_parm */
    pb->start_parm = psd->parmcnt;
    pb->choice_id = pch->choice_id;
    pb->block_id = pch->blockcnt+1;

    /* get all the parms in this choice block */
    done = FALSE;
    while (!done) {
        res = consume_cparm(mod, tkc, psd, pch, pb);
        if (res != NO_ERR) {
            psd_free_block(pb);
            return res;
        }

        if (tk_next_typ(tkc) == TK_TT_RBRACK) {
            done = TRUE;
        }
    }

    /* consume the closing right bracket of the block decl */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACK);
    if (res != NO_ERR) {
        psd_free_block(pb);
        return res;
    }

    /* set the block end_parm */
    pb->end_parm = psd->parmcnt;
    pch->blockcnt++;

    /* add this block to the choice in progress */
    dlq_enque(pb, &pch->choiceQ);
    return NO_ERR;

}  /* consume_pblock */


/********************************************************************
* FUNCTION consume_psection
* 
* Parse the next N nodes as a 'parm' or 'choice' subtree
* Create a psd_parm struct and add it to the specified psd
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
*   psd    == parmsetdef struct that will get the psd_parm_t 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_psection (tk_chain_t *tkc,
		      ncx_module_t *mod,
                      psd_template_t  *psd)
{
    psd_choice_t    *pch;
    status_t         res;
    boolean          done;

    /* move to the first token */
    res = TK_ADV(tkc);
    if (res == NO_ERR) {
	/* this should be an identifier token */
	if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
	    res = ERR_NCX_WRONG_TKTYPE;
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    /* this identifier should be a 'parm' or 'choice' token */
    if (!xml_strcmp(TK_CUR_VAL(tkc), NCX_EL_PARM)) {
        /* this is a regular parm */
        TK_BKUP(tkc);
        return consume_parm(mod, tkc, psd);
    } else if (!xml_strcmp(TK_CUR_VAL(tkc), NCX_EL_CHOICE)) {
        /* consume the starting left brace of the choice */
        res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
        if (res != NO_ERR) {
            return res;
        }
        
        /* get a new parm choice header */
        pch = psd_new_choice();
        if (!pch) {
	    res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }

	/* the first parm in the choice will get the next value */
	pch->start_parm = psd->parmcnt+1;
	pch->choice_id = psd->choicecnt+1;

        /* get all the parm choices */
        done = FALSE;
        while (!done) {
            /* next token can be a '[' to start a choice block decl
             * or a 'parm' token to start a regular parm decl
             */
            if (tk_next_typ(tkc) == TK_TT_LBRACK) {
                res = consume_pblock(mod, tkc, psd, pch);
            } else {
                res = consume_cparm(mod, tkc, psd, pch, NULL);
            }
            if (res != NO_ERR) {
                psd_free_choice(pch);
                return res;
            }

            if (tk_next_typ(tkc) == TK_TT_RBRACE) {
                done = TRUE;
            }
        }

        /* consume the ending right brace of the choice */
        res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
        if (res != NO_ERR) {
            psd_free_choice(pch);
        }

	/* set the end_parm value -- at least one parm was found */
	pch->end_parm = psd->parmcnt;
	psd->choicecnt++;

        /* save the parm choice in the PSD parm Q */
        dlq_enque(pch, &psd->parmQ);
        return NO_ERR;
    } else {
	res = ERR_NCX_WRONG_TKVAL;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }        
    /*NOTREACHED*/

}  /* consume_psection */


/********************************************************************
* FUNCTION consume_parms
* 
* Parse the next N nodes as a 'parms' clause
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   psd == parmset template to contain the PSD parms section
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_parms (tk_chain_t *tkc, 
		   ncx_module_t  *mod,
		   psd_template_t *psd)
{
    boolean         parmsdone;
    status_t        res;

    /* Get the parms start tag and left brace */
    res = ncx_consume_tstring(tkc, mod, NCX_EL_PARMS, NCX_REQ);
    if (res==NO_ERR) {
	res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    }
    if (res != NO_ERR) {
	return res;
    }

    /* get each 'parm' or 'choice' subsection */
    parmsdone = FALSE;
    while (!parmsdone) {
	res = consume_psection(tkc, mod, psd);
	if (res != NO_ERR) {
	    parmsdone = TRUE;
	} else if (tk_next_typ(tkc) == TK_TT_RBRACE) {
	    parmsdone = TRUE;
	}
    }

    if (res == NO_ERR) {
	/* get the mandatory 'parms' end tag */
	res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    }

    return res;

}  /* consume_parms */


/********************************************************************
* FUNCTION consume_psd
* 
* Parse the next N nodes as a <psd> subtree
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module to contain the PSD
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_psd (tk_chain_t *tkc, 
		 ncx_module_t  *mod)
{
    psd_template_t *psd;
    psd_parm_t     *parm;
    xmlChar        *str;
    const xmlChar  *val;
    boolean         done;
    status_t        res, retres;
    tk_type_t       tktyp;
    uint32          flags, i;

    /* peek ahead to see if the section is done */
    if (!(tk_next_typ(tkc)==TK_TT_TSTRING && tk_next_val(tkc) &&
          !xml_strcmp(NCX_EL_PARMSET, tk_next_val(tkc)))) {
	return ERR_PARS_SECDONE;  
    }

    /* malloc a new PSD struct */
    psd = psd_new_template();
    if (!psd) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* Already checked for a 'parmset' start node, so this should work */
    res = ncx_consume_name(tkc, mod, NCX_EL_PARMSET, &psd->name, 
            NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
	psd_free_template(psd);
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    psd->linenum = tkc->cur->linenum;  /* close enough for NCX */

    flags = 0;
    done = FALSE;
    retres = NO_ERR;

    /* loop through all the sub-clauses present */
    while (!done) {
	/* look ahead to the next token */
	tktyp = tk_next_typ(tkc);
	val = tk_next_val(tkc);

	/* check the next token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* sub-clause assumed */
	case TK_TT_RBRACE:
	    retres = TK_ADV(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    done = TRUE;
	    continue;
	}

	/* Got a token string so check the value */
        if (!xml_strcmp(val, NCX_EL_DESCRIPTION)) {
	    if (flags & B_PSD_DESCRIPTION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'description' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DESCRIPTION, 
					 ncx_save_descr() ? 
					 &psd->descr : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PSD_DESCRIPTION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_CONDITION)) {
	    if (flags & B_PSD_CONDITION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'condition' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_CONDITION, 
					 ncx_save_descr() ? 
					 &psd->condition : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PSD_CONDITION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_ORDER)) {
	    if (flags & B_PSD_ORDER) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the optional <order> field */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_ORDER, &str, 
					 NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		psd->order = psd_get_order_enum(str);
		m__free(str);
		if (psd->order==PSD_OT_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PSD_ORDER;
	    }
        } else if (!xml_strcmp(val, NCX_EL_TYPE)) {
	    if (flags & B_PSD_TYPE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the optional 'type' field */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_TYPE, &psd->pstype, 
					 NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		if (!xml_strcmp(psd->pstype, NCX_PSTYP_DATA)) {
		    psd->psd_type = PSD_TYP_DATA;
		} else if (!xml_strcmp(psd->pstype, NCX_PSTYP_RPC)) {
		    psd->psd_type = PSD_TYP_RPC;
		} else if (!xml_strcmp(psd->pstype, NCX_PSTYP_CLI)) {
		    psd->psd_type = PSD_TYP_CLI;
		} else {
		    res = ERR_NCX_WRONG_VAL;
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PSD_TYPE;
	    }
        } else if (!xml_strcmp(val, NCX_EL_DATA_CLASS)) {
	    if (flags & B_PSD_DATA_CLASS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the optional 'data-class' field  */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DATA_CLASS, &str, 
					 NCX_OPT, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		psd->dataclass = ncx_get_data_class_enum(str);
		m__free(str);
		if (psd->dataclass==NCX_DC_NONE) {
		    res = ERR_NCX_WRONG_VAL;
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PSD_DATA_CLASS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_APPINFO)) {
	    if (flags & B_PSD_APPINFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* check if the optional appinfo clause is present */
	    res = ncx_consume_appinfo(tkc, mod, &psd->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PSD_APPINFO;
	    }
        } else if (!xml_strcmp(val, NCX_EL_PARMS)) {
	    if (flags & B_PSD_PARMS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }

	    /* Get the parms section */
	    res = consume_parms(tkc, mod, psd);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_PSD_PARMS;
	    } 
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    done = TRUE;
	}
    }	    

    /* check if the parmset construct was parsed okay */
    if (retres == ERR_NCX_ENTRY_EXISTS) {
	res = TK_ADV(tkc);
    } else if (retres == NO_ERR) {
	if ((flags & M_PSD_MANDATORY) != M_PSD_MANDATORY) {
	    retres = ERR_NCX_DATA_MISSING;
	}
    }

    /* make sure this PSD is not a duplicate */
    if (retres == NO_ERR && ncx_is_duplicate(mod, psd->name)) {
	retres = ERR_NCX_DUP_ENTRY;
    }

    /* check missing parameters with defaults */
    if (retres == NO_ERR) {
	/* set the order parameter to the default if not provided */
	if (!(flags & B_PSD_ORDER)) {
	    psd->order = PSD_OT_LOOSE;
	}

	/* set the data-class parameter to the default if not provided */
	if (!(flags & B_PSD_DATA_CLASS)) {
	    psd->dataclass = NCX_DC_CONFIG;
	}

	/* check all the access and dataclass fields in the parms
	 * which may be wrong because of corner-cases in clause ordering
	 */
	for (i=1; i <= psd->parmcnt; i++) {
	    parm = psd_find_parmnum(psd, i);
	    if (parm) {
		if (parm->dataclass == NCX_DC_NONE) {
		    if (parm->access == NCX_ACCESS_RO) {
			parm->dataclass = NCX_DC_STATE;
		    } else {
			parm->dataclass = psd->dataclass;
		    }
		}

		if (psd->psd_type == PSD_TYP_RPC) {
		    parm->access = NCX_ACCESS_RW;
		} else if (parm->access == NCX_ACCESS_NONE) {
		    switch (parm->dataclass) {
		    case NCX_DC_STATE:
			parm->access = NCX_ACCESS_RO;
			break;
		    case NCX_DC_TCONFIG:
			parm->access = NCX_ACCESS_RW;
			break;
		    case NCX_DC_CONFIG:
			parm->access = NCX_ACCESS_RC;
			break;
		    default:
			SET_ERROR(ERR_INTERNAL_VAL);
		    }
		}
	    }
	}

	/* set the type parameter to the default if not provided */
	if (!(flags & B_PSD_TYPE)) {
	    psd->pstype = xml_strdup(PSD_DEF_PSTYPE);
	    if (!psd->pstype) {
		retres = ERR_INTERNAL_MEM;
	    }
	    psd->psd_type = PSD_DEF_PSTYPE_ENUM;
	}
    }

    /* check any errors at all in the parmset definition */
    if (retres != NO_ERR) {
	psd_free_template(psd);
	ncx_print_errormsg(tkc, mod, retres);
	return retres;
    }

#ifdef NCX_PARSE_DEBUG
    log_debug3("\nncx_parse: adding PSD (%s) to module (%s)", 
	      psd->name, mod->name);
#endif

    /* Got a valid entry, add it to the mod->psdQ */
    psd->module = mod->name;
    psd->app = mod->app;
    dlq_enque(psd, &mod->psdQ);
    return NO_ERR;

}  /* consume_psd */


/********************************************************************
* FUNCTION consume_int_psd
* 
* Handle creation of an internal RPC parmset
* and parse the next N nodes as a parms section for
* the internal PSD
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module to contain the PSD
*   rpc == RPC method template to get the internal parmset
*
* OUTPUTS:
*  rpc->in_psd set with new internal parmset if NO_ERR
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_int_psd (tk_chain_t *tkc, 
		     ncx_module_t  *mod,
		     rpc_template_t *rpc)
{
    psd_template_t *psd;
    psd_parm_t     *parm;
    status_t        res;
    uint32          i;

    /* malloc a new PSD struct */
    psd = psd_new_template();
    if (!psd) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }
    
    psd->name = xml_strdup(rpc->name);
    psd->pstype = xml_strdup(NCX_PSTYP_RPC);

    if (!psd->name || !psd->pstype) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	psd_free_template(psd);
	return res;
    }

    psd->order = PSD_OT_STRICT;
    psd->psd_type = PSD_TYP_RPC;
    psd->nsid = rpc->nsid;
    psd->module = mod->name;
    psd->app = mod->app;
    psd->dataclass = NCX_DC_CONFIG;
    
    /* Get the parms section */
    res = consume_parms(tkc, mod, psd);

    /* check missing parameters with defaults */
    if (res == NO_ERR) {

	/* check all the access and dataclass fields in the parms
	 * which may be wrong because of corner-cases in clause ordering
	 */
	for (i=1; i <= psd->parmcnt; i++) {
	    parm = psd_find_parmnum(psd, i);
	    if (parm) {
		parm->dataclass = NCX_DC_CONFIG;
		parm->access = NCX_ACCESS_RW;

	    }
	}
    }

    /* check any errors at all in the parmset definition */
    if (res != NO_ERR) {
	psd_free_template(psd);
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

#ifdef NCX_PARSE_DEBUG
    log_debug3("\nncx_parse: adding PSD (%s) to module (%s)", 
	      psd->name, mod->name);
#endif

    rpc->in_psd_name = NULL;
    rpc->in_psd = psd;

    return NO_ERR;

}  /* consume_int_psd */


/********************************************************************
* FUNCTION consume_rpc_contents
* 
* Parse contents of one NCX module <rpc> section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain
*   rpc == rpc_template_t to fill in
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_rpc_contents (ncx_module_t *mod,
			  tk_chain_t *tkc,
                          rpc_template_t   *rpc)
{
    const xmlChar *val;
    xmlChar       *str;
    tk_type_t      tktyp;
    uint32         flags;
    status_t       res, retres;
    boolean        done;

    flags = 0;
    done = FALSE;
    retres = NO_ERR;

    while (!done) {
	/* look ahead to the next token */
	tktyp = tk_next_typ(tkc);
	val = tk_next_val(tkc);

	/* check the next token type */
	switch (tktyp) {
	case TK_TT_NONE:
	    retres = ERR_NCX_EOF;
	    done = TRUE;
	    continue;
	case TK_TT_TSTRING:
	    break;  /* sub-clause assumed */
	case TK_TT_RBRACE:
	    retres = TK_ADV(tkc);
	    done = TRUE;
	    continue;
	default:
	    retres = ERR_NCX_WRONG_TKTYPE;
	    done = TRUE;
	    continue;
	}
	    
	/* Got a token string so check the value */
        if (!xml_strcmp(val, NCX_EL_DESCRIPTION)) {
	    if (flags & B_RPC_DESCRIPTION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'description' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_DESCRIPTION, 
					 ncx_save_descr() ? 
					 &rpc->descr : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_DESCRIPTION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_CONDITION)) {
	    if (flags & B_RPC_CONDITION) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Check if the optional 'condition' field is present */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_CONDITION, 
					 ncx_save_descr() ? 
					 &rpc->condition : NULL, 
					 NCX_OPT, NCX_WSP, TK_TT_SEMICOL);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_CONDITION;
	    }
        } else if (!xml_strcmp(val, NCX_EL_RPC_TYPE)) {
	    if (flags & B_RPC_TYPE) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the mandatory <rpc-type> field */
	    res = ncx_consume_dyn_string(tkc, mod, NCX_EL_RPC_TYPE, &str,
					 NCX_REQ, NCX_NO_WSP, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		if (!xml_strcmp(NCX_EL_OTHER, str)) {
		    rpc->rpc_typ = RPC_TYP_OTHER;
		} else if (!xml_strcmp(NCX_EL_CONFIG, str)) {
		    rpc->rpc_typ = RPC_TYP_CONFIG;
		} else if (!xml_strcmp(NCX_EL_EXEC, str)) {
		    rpc->rpc_typ = RPC_TYP_EXEC;
		} else if (!xml_strcmp(NCX_EL_MONITOR, str)) {
		    rpc->rpc_typ = RPC_TYP_MONITOR;
		} else if (!xml_strcmp(NCX_EL_DEBUG, str)) {
		    rpc->rpc_typ = RPC_TYP_DEBUG;
		} else {
		    res = ERR_NCX_WRONG_VAL;
		}
		m__free(str);
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_TYPE;
	    }
        } else if (!xml_strcmp(val, NCX_EL_INPUT)) {
	    if (flags & B_RPC_INPUT) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the optional 'input' field */
	    res = ncx_consume_mname(tkc, mod, NCX_EL_INPUT, &rpc->in_psd_name,
				    &rpc->in_modstr, NCX_OPT, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		/* check the input PSD name */
		res = psd_locate_template(mod, rpc->in_modstr, 
					  rpc->in_psd_name, &rpc->in_psd);
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_INPUT;
	    }
        } else if (!xml_strcmp(val, NCX_EL_PARMS)) {
	    if (flags & B_RPC_PARMS) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the other form of the input PSD, direct parms clause */
	    res = consume_int_psd(tkc, mod, rpc);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_PARMS;
	    }
        } else if (!xml_strcmp(val, NCX_EL_OUTPUT)) {
	    if (flags & B_RPC_OUTPUT) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* Get the optional 'output' field */
	    res = ncx_consume_mname(tkc, mod, NCX_EL_OUTPUT, 
				    &rpc->out_data_name,
				    &rpc->out_modstr, 
				    NCX_OPT, TK_TT_SEMICOL);
	    if (res == NO_ERR) {
		/* check the output data name as a type first */
		res = typ_locate_template(mod, rpc->out_modstr, 
					  rpc->out_data_name, 
					  (typ_template_t **)&rpc->out_data);
		if (res == NO_ERR) {
		    rpc->out_datatyp = RPC_OT_TYPE;
		} else {
		    /* check the output data name as a PSD 2nd */
		    res = psd_locate_template(mod, rpc->out_modstr, 
				      rpc->out_data_name, 
				      (psd_template_t **)&rpc->out_data);
		    if (res == NO_ERR) {
			rpc->out_datatyp = RPC_OT_PSD;
		    }
		}
	    }
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_OUTPUT;
	    }
        } else if (!xml_strcmp(val, NCX_EL_APPINFO)) {
	    if (flags & B_RPC_APPINFO) {
		retres = ERR_NCX_ENTRY_EXISTS;
		done = TRUE;
		continue;
	    }
	    /* check if the optional appinfo clause is present */
	    res = ncx_consume_appinfo(tkc, mod, &rpc->appinfoQ);
	    if (res != NO_ERR) {
		retres = res;
		done = TRUE;
	    } else {
		flags |= B_RPC_APPINFO;
	    }
	} else {
	    retres = ERR_NCX_WRONG_TKVAL;
	    done = TRUE;
	}
    }

    /* check if the rpc construct was parsed okay */
    if (retres == ERR_NCX_ENTRY_EXISTS) {
	res = TK_ADV(tkc);
    } else if (retres == NO_ERR) {
	if ((flags & M_RPC_MANDATORY) != M_RPC_MANDATORY) {
	    retres = ERR_NCX_DATA_MISSING;
	} else if ((flags & B_RPC_INPUT) && (flags & B_RPC_PARMS)) {
	    retres = ERR_NCX_EXTRA_CHOICE;
	}
    }

    if (retres == NO_ERR) {
	/* check if output skipped */
	if (!(flags & B_RPC_OUTPUT)) {
	    /* set to the RpcOkReplyType default */
	    rpc->out_modstr = xml_strdup(NC_MODULE);
	    rpc->out_data_name = xml_strdup(NC_OK_REPLY);
	    rpc->out_datatyp = RPC_OT_TYPE;
	}
    } else {
	ncx_print_errormsg(tkc, mod, retres);
    }

    return retres;

} /* consume_rpc_contents */


/********************************************************************
* FUNCTION consume_rpc
* 
* Parse one NCX module <rpc> section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_rpc (tk_chain_t *tkc,
		 ncx_module_t   *mod)
{
    rpc_template_t  *rpc;
    status_t       res;

    /* peek ahead to see if the section is done */
    if (!(tk_next_typ(tkc)==TK_TT_TSTRING && tk_next_val(tkc) &&
          !xml_strcmp(NCX_EL_RPC, tk_next_val(tkc)))) {
	return ERR_PARS_SECDONE;  
    }

    /* malloc a new RPC struct */
    rpc = rpc_new_template();
    if (!rpc) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);	
	return res;
    }

    /* Already checked for an <rpc> start node, so this should work */
    res = ncx_consume_name(tkc, mod, NCX_EL_RPC, &rpc->name, 
			   NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
        rpc_free_template(rpc);
        return res;
    }

    rpc->linenum = tkc->cur->linenum;  /* close enough for NCX */

    /* get the rest of the rpc, including the end tag */
    res = consume_rpc_contents(mod, tkc, rpc);
    if (res != NO_ERR) {
	rpc_free_template(rpc);
        return res;
    }

    /* make sure this is not a duplicate */
    if (ncx_is_duplicate(mod, rpc->name)) {
	res = ERR_NCX_DUP_ENTRY;
	ncx_print_errormsg(tkc, mod, res);	
	rpc_free_template(rpc);
	return res;
    }

#ifdef NCX_PARSE_DEBUG
    log_debug3("\nncx_parse: adding RPC (%s) to module (%s)", 
	      rpc->name, mod->name);
#endif

    /* add the rpc to the module RPC Que */
    dlq_enque(rpc, &mod->rpcQ);
    return NO_ERR;

}  /* consume_rpc */


/********************************************************************
* FUNCTION consume_definition
* 
* Parse an NCX module definition section construct, such as <type>
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module_t in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t
    consume_definition (tk_chain_t *tkc, 
			ncx_module_t *mod)
{
    status_t res;

    /* peek ahead to see if the section is done */
    if (tk_next_typ(tkc)==TK_TT_RBRACE) {
        return ERR_PARS_SECDONE;
    }

    /* brute force through all the options */
    res = consume_type(tkc, mod);
    if (res == NO_ERR || res != ERR_PARS_SECDONE) {
        return res;
    }

    res = consume_psd(tkc, mod);
    if (res == NO_ERR || res != ERR_PARS_SECDONE) {
        return res;
    }

    res = consume_rpc(tkc, mod);
    if (res == NO_ERR || res != ERR_PARS_SECDONE) {
        return res;
    }

    res = ERR_NCX_WRONG_TKTYPE;
    ncx_print_errormsg(tkc, mod, res);
    return res;

} /* consume_definition */


/********************************************************************
* FUNCTION consume_container
* 
* Parse the NCX module contain section, such as <imports> section
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc  == token chain
*   mod == ncx_module_t in progress
*   elname == start of container name
*   cbfn == pointer to callback function for contained items
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_container (tk_chain_t  *tkc,
                       ncx_module_t   *mod,
                       const xmlChar *elname,
                       ncx_containedFn_t  cbfn)
{
    status_t  res;
    boolean   done;

    /* Check for a start node and '{' */
    res = ncx_consume_tstring(tkc, mod, elname, NCX_OPT);
    if (res==NO_ERR) {
        res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    }
    if (res != NO_ERR) {
	return (res == ERR_NCX_SKIPPED) ? NO_ERR : res;
    }

    /* process each child subtree */
    done = FALSE;
    while (!done) {
	res = (*cbfn)(tkc, mod);
	switch (res) {
	case NO_ERR:
	    break;
	case ERR_PARS_SECDONE:
	    done = TRUE;
	    break;
	default:
	    return res;
	}
    }

    /* get the end tag */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    return res;

}  /* consume_container */


/********************************************************************
* FUNCTION parse_module
* 
* Parse, generate and register one ncx_module_t struct
* from an NCX instance document stream containing one NCX module,
* which conforms to the NcxModule ABNF.
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
*
* INPUTS:
*   tkc == token chain
*   mod == ncx_module in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    parse_module (tk_chain_t  *tkc,
                  ncx_module_t *mod)
{
    status_t       res;

    /* consume 'ncx-module' module-name '{' */
    res = ncx_consume_name(tkc, mod, NCX_EL_NCXMODULE, 
         &mod->name, NCX_REQ, TK_TT_LBRACE);
    if (res != NO_ERR) {
	return res;
    }

    /* check if this module is already loaded */
    if (def_reg_find_module(mod->name)) {
        return ERR_NCX_SKIPPED;
    }

    /* the first section is the mandatory module 'header' */
    res = consume_header(tkc, mod);
    if (res != NO_ERR) {
	return res;
    }

    /* Check for an optional 'imports' section */
    res = consume_container(tkc, mod, 
         NCX_EL_IMPORTS, consume_import);
    if (res != NO_ERR) {
	return res;
    }

    /* Check for an optional 'definitions' section */
    res = consume_container(tkc, mod, 
        NCX_EL_DEFINITIONS, consume_definition);
    if (res != NO_ERR) {
	return res;
    }

    /* the next node should be the 'ncx-module' end node */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    if (res != NO_ERR) {
        return res;
    }

    /* add the definitions to the def_reg hash table */
    res = ncx_add_to_registry(mod);
    if (res != NO_ERR) {
	ncx_print_errormsg(NULL, mod, res);
    }
    return res;

}  /* parse_module */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION ncx_parse_from_filespec
* 
* Parse a file as an NCX module in compact syntax
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   filespec == absolute path or relative path
*               This string is used as-is without adjustment.
*   res == address of return status
*
* OUTPUTS:
*   *res == status of the operation
*
* RETURNS:
*   pointer to completed ncx_module_t
*   This is a copy of the malloced pointer -- DO NOT FREE

*********************************************************************/
ncx_module_t *
    ncx_parse_from_filespec (const xmlChar *filespec,
			     status_t  *res)
{
    tk_chain_t     *tkc;
    ncx_module_t   *mod, *testmod;
    FILE           *fp;
    xmlChar        *str;

#ifdef DEBUG
    if (!filespec || !res) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    } 
#endif

    *res = NO_ERR;
    mod = NULL;

    fp = fopen((const char *)filespec, "r");
    if (!fp) {
	*res = ERR_NCX_MISSING_FILE;
	return NULL;
    }

    log_debug2("\nLoading NCX module from file %s", filespec);

    /* get a new token chain */

    tkc = tk_new_chain();
    if (!tkc) {
	*res = ERR_INTERNAL_MEM;
	log_error("\nncx_parse malloc error");
	fclose(fp);
        return NULL;
    }

    str = ncx_get_source(filespec);
    if (!str) {
	*res = ERR_INTERNAL_MEM;
    } else {
	/* else setup the token chain to parse this NCX file */
	tk_setup_chain_ncx(tkc, fp, str);

	/* start a new ncx_module_t struct */
	mod = ncx_new_module();
	if (!mod) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    /* save the source of this ncx-module for monitor / debug */
	    mod->source = str;
	    mod->ismod = TRUE;

	    /* find the start of the file name */
	    mod->sourcefn = &str[xml_strlen(str)];
	    while (mod->sourcefn > str &&
		   *mod->sourcefn != NCX_PATHSEP_CH) {
		mod->sourcefn--;
	    }
	    if (*mod->sourcefn == NCX_PATHSEP_CH) {
		mod->sourcefn++;
	    }
	}
    }

    if (*res == NO_ERR) {
	*res = tk_tokenize_input(tkc, mod);

#ifdef NCX_PARSE_TK_DEBUG
	tk_dump_chain(tkc);
#endif
    }

    if (*res == NO_ERR) {
	*res = parse_module(tkc, mod);
    }

    if (*res == ERR_NCX_SKIPPED) {
	testmod = ncx_find_module(mod->name);
	ncx_free_module(mod);
	mod = testmod; 
	if (testmod) {
	    *res = NO_ERR;
	} else {
	    *res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    fclose(fp);
    tkc->fp = NULL;
    tk_free_chain(tkc);

    return mod;

}  /* ncx_parse_from_filespec */


/* END file ncx_parse.c */
