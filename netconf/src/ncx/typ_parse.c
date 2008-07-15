/*  FILE: typ_parse.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
23nov05      abb      begun; split out from typ.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <sys/types.h>
#include <regex.h>

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include "procdefs.h"
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

#ifndef _H_ncx_parse
#include "ncx_parse.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_typ_parse
#include "typ_parse.h"
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
/* #define TYP_PARSE_DEBUG 1 */
#endif

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/
/* tags for insertion modes */
typedef enum typ_insmode_t_ {
    INS_TEST,
    INS_REAL
} typ_insmode_t;


/* forward reference needed for recursive parsing of complex types */
static status_t 
consume_btype (ncx_module_t *mod, 
               tk_chain_t *tkc,
               typ_def_t *typdef,
               xmlChar **namebuff,
               typ_pmode_t pmode);



/********************************************************************
* FUNCTION set_name_from_sname
* 
* Set a const pointer to the name portion of a scoped name
*
* INPUTS:
*     sname == scoped name string
* 
* RETURNS:
*    const pointer to the first name char
*********************************************************************/
static const xmlChar *
    set_name_from_sname (const xmlChar *sname)
{
    const xmlChar *s;
    uint32         len;

    len = xml_strlen(sname);
    if (!len) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    
    /* look backwards for the first dot */
    s = sname+len-1;
    while (s >= sname && *s != NCX_SCOPE_CH) {
	s--;
    }
    /* make sure we found the dot */
    if (*s != NCX_SCOPE_CH) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return sname;
    }
    return s+1;
    
}  /* set_name_from_sname */


/********************************************************************
* FUNCTION ok_for_named_index
* 
* Check if the named type is okay to use in an named index decl
*
* INPUTS:
*     typ == typdef to  check
* RETURNS:
*     TRUE if okay, FALSE if not
*********************************************************************/
static boolean
    ok_for_named_index (typ_def_t  *typdef)
{
    switch (typdef->class) {
    case NCX_CL_NONE:
        return FALSE;
    case NCX_CL_BASE:
        return typ_ok_for_inline_index(typdef->def.base);
    case NCX_CL_SIMPLE:
        return typ_ok_for_index(typdef); 
    case NCX_CL_COMPLEX:
	return FALSE;
    case NCX_CL_NAMED:
        return ok_for_named_index(&typdef->def.named.typ->typdef);
    case NCX_CL_REF:
        return ok_for_named_index(typdef->def.ref.typdef);
    default:
        return FALSE;
    }
}  /* ok_for_named_index */


/********************************************************************
* FUNCTION ok_for_inline_metadata
* 
* Check if the base type is okay to use in an inline metadata decl
*
* INPUTS:
*     btyp == base type enum
* RETURNS:
*     TRUE if okay, FALSE if not
*********************************************************************/
static boolean
    ok_for_inline_metadata (ncx_btype_t btyp)
{
    switch (btyp) {
    case NCX_BT_ENUM:
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
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_UNION:
        return TRUE;
    default:
        return FALSE;
    }
}  /* ok_for_inline_metadata */


/********************************************************************
* FUNCTION ok_for_named_metadata
* 
* Check if the named type is okay to use in an metadata decl
*
* INPUTS:
*     typ == typdef to  check
* RETURNS:
*     TRUE if okay, FALSE if not
*********************************************************************/
static boolean
    ok_for_named_metadata (typ_def_t  *typ)
{
    switch (typ->class) {
    case NCX_CL_NONE:
        return FALSE;
    case NCX_CL_BASE:
        return ok_for_inline_metadata(typ->def.base);
    case NCX_CL_SIMPLE:
        return ok_for_inline_metadata(typ->def.simple.btyp); 
    case NCX_CL_COMPLEX:
	return FALSE;
    case NCX_CL_NAMED:
        return ok_for_named_metadata(&typ->def.named.typ->typdef);
    case NCX_CL_REF:
        return ok_for_named_metadata(typ->def.ref.typdef);
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
        return FALSE;
    }
}  /* ok_for_named_metadata */


/********************************************************************
* FUNCTION ok_for_list  
* 
* Check if the base type is okay to use in a list declaration
* (NCX_BT_SLIST)
*
* INPUTS:
*     btyp == base type enum to check
* RETURNS:
*     TRUE if okay, FALSE if not
*********************************************************************/
static boolean
    ok_for_list (ncx_btype_t btyp)
{
    switch (btyp) {
    case NCX_BT_ENUM:
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
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
        return TRUE;
    default:
        return FALSE;
    }
}  /* ok_for_list */


/********************************************************************
* FUNCTION ok_for_union
* 
* Check if the base type is okay to use in a union declaration
* (NCX_BT_UNION)
*
* INPUTS:
*     btyp == base type enum to check
* RETURNS:
*     TRUE if okay, FALSE if not
*********************************************************************/
static boolean
    ok_for_union (ncx_btype_t btyp)
{
    switch (btyp) {
    case NCX_BT_ENUM:
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
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
        return TRUE;
    default:
        return FALSE;
    }
}  /* ok_for_union */




/********************************************************************
* FUNCTION insert_enum
* 
* Insert an enum in its proper order in the simple->valQ
*
* INPUTS:
*   en == typ_enum_t struct to insert in the sim->valQ
*   sim == typ_simple_t struct to contain this enum 
* RETURNS:
*   status
*********************************************************************/
static status_t
    insert_enum (typ_enum_t *en, 
		 typ_simple_t *sim)
{
    typ_enum_t *enl;

    /* check if this is an out-of-order insert */
    for (enl = (typ_enum_t *)dlq_firstEntry(&sim->valQ);
         enl != NULL;
         enl = (typ_enum_t *)dlq_nextEntry(enl)) {
	if (!xml_strcmp(en->name, enl->name) || en->val == enl->val) {
            return ERR_NCX_DUP_ENTRY;
        } else if (en->val < enl->val) {
#ifdef TYP_PARSE_DEBUG
            log_debug3("\ntyp: Warning: Out of order enum %s = %d",
		      (char *)en->name, en->val);
#endif
            dlq_insertAhead(en, enl);
            return NO_ERR;
        } /* else keep going */
    }

    /* normal case == new last entry */
    dlq_enque(en, &sim->valQ);
    return NO_ERR;

}  /* insert_enum */


/********************************************************************
* FUNCTION get_iqual_value
* 
* Check if the specified string an instance qualifier
*
* INPUTS:
*   str  == string to check
* RETURNS:
*   iqual value (including none)status
*********************************************************************/
static ncx_iqual_t
    get_iqual_value (tk_type_t tktyp)
{
    switch (tktyp) {
    case TK_TT_QMARK:
	return NCX_IQUAL_OPT;
    case TK_TT_STAR:
	return NCX_IQUAL_ZMORE;
    case TK_TT_PLUS:
	return NCX_IQUAL_1MORE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NCX_IQUAL_NONE;
    }
}  /* get_iqual_value */


/********************************************************************
* FUNCTION check_iqual
* 
* Check if the next token is an instance qualifier
*
* INPUTS:
*   tkc  == token chain
*   typdef == typ_def_t in progress 
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_iqual (tk_chain_t  *tkc,
		 typ_def_t   *typdef)

{
    status_t  res;

    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        return res;
    }
    if (TK_CUR_IQUAL(tkc)) {
        typdef->iqual = get_iqual_value(TK_CUR_TYP(tkc));
    } else {
        typdef->iqual = NCX_IQUAL_ONE;
        TK_BKUP(tkc);
    }
    return NO_ERR;

}  /* check_iqual */


/********************************************************************
* FUNCTION check_meta_iqual
* 
* Check if the next token is an instance qualifier
*
* INPUTS:
*   tkc  == token chain
*   typdef == typ_def_t in progress 
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_meta_iqual (tk_chain_t  *tkc,
		      typ_def_t   *typdef)
{
    status_t  res;

    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        return res;
    }
    if (TK_CUR_TYP(tkc) == TK_TT_QMARK) {
        typdef->iqual = get_iqual_value(TK_TT_QMARK);
    } else {
        typdef->iqual = NCX_IQUAL_ONE;
        TK_BKUP(tkc);
    }
    return NO_ERR;

}  /* check_meta_iqual */


/********************************************************************
* FUNCTION check_xlist_iqual
* 
* Check if the next token is an instance qualifier
* Special function to check list string set iqual
*
* INPUTS:
*   tkc  == token chain
*   listval == typ_listval_t in progress 
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_xlist_iqual (tk_chain_t  *tkc,
		       typ_listval_t  *listval)
{
    status_t  res;

    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        return res;
    }
    if (TK_CUR_IQUAL(tkc)) {
        listval->iqual = get_iqual_value(TK_CUR_TYP(tkc));
    } else {
        listval->iqual = NCX_IQUAL_ONE;
        TK_BKUP(tkc);
    }
    return NO_ERR;

}  /* check_xlist_iqual */


/********************************************************************
* FUNCTION consume_name
* 
* Handle the name token if this is a child node not a root node
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  tkc == token chain
*  mod == module in progress
*  namebuff == pointer to hold buffer ptr for member-name, or NULL if none
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_name (tk_chain_t *tkc,
		  ncx_module_t *mod,
		  xmlChar **namebuff)
{
    status_t   res;

    /* enum <wspace> member-name { <enum-spec>+ }; */
    if (namebuff) {
        /* get the member-name -- a TSTRING */
        res = TK_ADV(tkc);
        if (res == NO_ERR) {
	    if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
		res = ERR_NCX_WRONG_TKTYPE;
	    } else {
		*namebuff = xml_strdup(TK_CUR_VAL(tkc));
		if (!*namebuff) {
		    res = ERR_INTERNAL_MEM;
		}
	    }
	}
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}
    }
    return NO_ERR;

} /* consume_name */


/********************************************************************
* FUNCTION consume_union_contents
* 
* Parse the union spec
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain
*    mod == module in progress
*    typdef == typedef struct in progress
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    consume_union_contents (tk_chain_t *tkc,
			    ncx_module_t *mod,
			    typ_def_t *typdef)
{
    typ_unionnode_t *un;
    typ_template_t *typ;
    status_t   res;
    boolean    done;

    /* check for '{'  */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
        return res;
    }

    done = FALSE;
    while (!done) {
        /* get type name, find the type template,
	 * and save it in a new unionnode struct,
	 * added to the typdef unionQ
	 */
        res = TK_ADV(tkc);
        if (res == NO_ERR) {
	    /* must find the type template from the mod-qual type name */
	    if (res==NO_ERR) {
		res = typ_locate_template(mod, TK_CUR_MOD(tkc),
					  TK_CUR_VAL(tkc), &typ);
		/* get a template, check okay for use in a union */
		if (res == NO_ERR && 
		    !ok_for_union(typ_get_basetype(&typ->typdef))) {
		    res = ERR_NCX_WRONG_TYPE;
		}
	    }

	    if (res == NO_ERR) {
		un = typ_new_unionnode(typ);
		if (!un) {
		    res = ERR_INTERNAL_MEM;
		} else {
		    dlq_enque(un, &typdef->def.simple.unionQ);
		}
	    }
	}

	/* check any error with the mod-auqlified type name */
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}
	    
        /* peek ahead to see if we are done with the while loop */
        if (tk_next_typ(tkc) == TK_TT_RBRACE) {
            done = TRUE;
        }
    }

    /* check for '}'  */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);

    /* TBD: should really check that at least 2 types are given */

    return res;

}  /* consume_union_contents */


/********************************************************************
* FUNCTION consume_union
* 
* Current token is a list (NCX_BT_UNION)
* Process the token chain and gather the list definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  tkc == token chain
*  mod == module in progress
*  typdef == typedef struct in progress
*  namebuff == ptr to buffer ptr for member-name, of NULL if none
*
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_union (tk_chain_t *tkc,
		   ncx_module_t *mod,
		   typ_def_t *typdef,
		   xmlChar **namebuff)
{
    status_t     res;

    /* c-union-type :== "union" <wspace> [member-name] 
     *    <wspace> "{" <typename>+ "}"  ";" 
     */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    /* setup the typedef as a simple type */
    typ_init_simple(typdef, NCX_BT_UNION);

    res = consume_union_contents(tkc, mod, typdef);
    if (res != NO_ERR) {
	return res;
    }

    /* check for closing ';'  */
    res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);

    return res;

}  /* consume_union */



/********************************************************************
* FUNCTION consume_enum_contents
* 
* Parse the enum spec starting with the  '=' or '+=' tokens
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain
*    mod == module in progress
*    typdef == typedef struct in progress
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    consume_enum_contents (tk_chain_t *tkc,
			   ncx_module_t *mod,
			   typ_def_t *typdef)
{
    status_t   res;
    boolean    done;
    int32      defval;
    ncx_num_t  num;
    typ_enum_t *ev;

    /* check for '='  or '+=' */
    res = TK_ADV(tkc);
    if (res == NO_ERR) {
	switch (TK_CUR_TYP(tkc)) {
	case TK_TT_EQUAL:
	    typdef->def.simple.flags |= TYP_FL_REPLACE;
	    break;
	case TK_TT_PLUSEQ:
	    break;
	default:
	    res = ERR_NCX_WRONG_TKTYPE;
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* check for '{'  */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
        return res;
    }

    defval = 0;
    done = FALSE;
    while (!done) {
        /* get enum name and save it in a new enum struct */
        res = TK_ADV(tkc);
        if (res == NO_ERR) {
	    if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
		res = ERR_NCX_WRONG_TKTYPE;
	    } else {
		ev = typ_new_enum(TK_CUR_VAL(tkc));
		if (!ev) {
		    res = ERR_INTERNAL_MEM;
		} 
	    }
	}

	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}
	    
        /* check for enum explicit dec-num value */
        if (tk_next_typ(tkc) == TK_TT_EQUAL) {
            /* token has explicit value set */
            res = TK_ADV(tkc);    /* skip past the equals sign */
            if (res == NO_ERR) {
		/* get a decimal-number */
		res = TK_ADV(tkc);
		if (res == NO_ERR) {
		    if (TK_CUR_TYP(tkc) != TK_TT_DNUM) {
			res = ERR_NCX_WRONG_TKTYPE;
		    } else {
			/* convert the number string to an int */
			res = ncx_convert_tkcnum(tkc, NCX_BT_INT32, &num);
		    }
		}
            }
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                typ_free_enum(ev);
                return res;
            }

	    ev->val = num.i;
            ev->flags |= TYP_FL_ESET;   /* mark explicit set val */
            /* update the defval for subsequent enums */
            defval = ev->val + 1;
        } else {
            /* token does not have explicit value set */
            ev->val = defval++;
        }

        /* insert this new enum */
        res = insert_enum(ev, &typdef->def.simple);
        if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
            typ_free_enum(ev);
            return res;
        }

        /* peek ahead to see if we are done with the while loop */
        if (tk_next_typ(tkc) == TK_TT_RBRACE) {
            done = TRUE;
        }
    }

    /* check for '}'  */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    return res;

}  /* consume_enum_contents */


/********************************************************************
* FUNCTION consume_enum_bits
* 
* Current token is 'enum' or 'bits'
* Process the token chain and gather the enum definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  tkc == token chain
*  typdef == typedef struct in progress
*  btyp == base type (NCX_BT_ENUM or NCX_BT_BITS)
*  namebuff == pointer to hold buffer ptr for member-name, or NULL if none
*  pmode == TYP_PM_NORMAL if final TK_TT_SEMICOL should be parsed
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_enum_bits (tk_chain_t *tkc,
		       ncx_module_t *mod,
		       typ_def_t *typdef,
		       ncx_btype_t btyp,
		       xmlChar **namebuff,
		       typ_pmode_t  pmode)
{
    status_t   res;
    boolean    getsemi;

    /* enum <wspace> member-name { <enum-spec>+ }; */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    /* setup the typedef as a simple type */
    typ_init_simple(typdef, btyp);

    res = consume_enum_contents(tkc, mod, typdef);
    if (res != NO_ERR) {
        return res;
    }

    /* check if the instance qualifier is allowed to be next */
    switch (pmode) {
    case TYP_PM_INDEX:
        getsemi = FALSE;
	res = NO_ERR;
        break;
    case TYP_PM_NORMAL:
        getsemi = TRUE;
        res = check_iqual(tkc, typdef);
        break;
    case TYP_PM_MDATA:
        getsemi = TRUE;
        res = check_meta_iqual(tkc, typdef);
        break;
    case TYP_PM_NONE:
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* check for final semi-colon */
    if (getsemi) {
        res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    }
    return res;

}  /* consume_enum_bits */


/********************************************************************
* FUNCTION consume_plain_base
* 
* Current token is 'any', 'root', 'flag'
* Process the token chain and gather the plain base type
* definition in typdef
* Handles base types:
*   NCX_BT_ANY
*   NCX_BT_ROOT
*   NCX_BT_EMPTY
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   btype == base type
*   typdef == typedef struct in progress
*   namebuff == non-NULL if member-name expected
*   pmode == TYP_PM_NORMAL if final TK_TT_SEMICOL should be parsed
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    consume_plain_base (tk_chain_t *tkc,
			ncx_module_t *mod,
			ncx_btype_t  btyp,
			typ_def_t *typdef,
			xmlChar **namebuff,
			typ_pmode_t pmode)
{
    status_t   res;
    boolean    getsemi;

    /* finish this type : foo <member-name> ; */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    switch (btyp) {
    case NCX_BT_ANY:
    case NCX_BT_ROOT:
	typ_init_complex(typdef, btyp);
	break;
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
	typ_init_simple(typdef, btyp);
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check if the instance qualifier is allowed to be next */
    switch (pmode) {
    case TYP_PM_INDEX:
        getsemi = FALSE;
	res = NO_ERR;
        break;
    case TYP_PM_NORMAL:
        getsemi = TRUE;
        res = check_iqual(tkc, typdef);
        break;
    case TYP_PM_MDATA:
        getsemi = TRUE;
        res = check_meta_iqual(tkc, typdef);
        break;
    case TYP_PM_NONE:
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);	    
	return res;
    }

    if (getsemi) {
        res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    }

    return res;

}  /* consume_plain_base */


/********************************************************************
* FUNCTION check_range
* 
* Check if a number is in the specified range
* This function used to validate the range setup, 
* DO NOT USE FOR RUNTIME RANGE CHECKING.
*
* INPUTS:
*   num == ncx_num_t to check if it is in range
*   rv == range spec to check against
*   btyp == base type of the values
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_range (ncx_num_t *num,
		 typ_rangedef_t *rv,
		 ncx_btype_t  btyp)
{
    int32    cmp;
    status_t res;

    /* check if range definition overlaps 
     * Have 
     *   Range.lo .. Range.hi (rv->lb .. rv->ub)
     *   Value (num)
     *
     * Make sure num not in the List range
     */
    res = NO_ERR;
    cmp = ncx_compare_nums(num, &rv->lb, btyp);
    switch (cmp) {
    case 0:
        /* num is in the range */
        res = ERR_NCX_OVERLAP_RANGE;
	break;
    case 1:
        /* num > LB, need to check upper bound */
        cmp = ncx_compare_nums(num, &rv->ub, btyp);
        switch (cmp) {
        case 0:
        case -1:
            /* num is in the range */
            res = ERR_NCX_OVERLAP_RANGE;
	    break;
        case 1:
            /* num is greater than the range spec */
            break;    /* NO_ERR */
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
	break;
    case -1:
        /* num is lower the the range spec */
        break;  /* NO_ERR */
    default: 
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* check_range */


/********************************************************************
* FUNCTION insert_rangedef
* 
* Insert a typ_rangedef_t in its proper order in the simple->rangeQ
*
* INPUTS:
*   rv == typ_rangedef_t struct to insert in the sim->rangeQ
*   sim == typ_simple_t struct to contain this range spec fragment
* RETURNS:
*   status
*********************************************************************/
static status_t
    insert_rangedef (typ_rangedef_t *rv, 
		     typ_simple_t *sim)
{
    typ_rangedef_t *rvl;
    ncx_btype_t     btyp;
    status_t        res;

    /* get range number type */
    btyp = typ_get_range_type(sim->btyp);

    /* make sure the rangeval is valid to begin with */
    if (ncx_compare_nums(&rv->lb, &rv->ub, btyp) == 1) {
        /* lower bound larger than upper bound */
        return ERR_NCX_INVALID_RANGE;
    }
        
    /* check if this is an over-lapping insert */
    for (rvl = (typ_rangedef_t *)dlq_firstEntry(&sim->rangeQ);
         rvl != NULL;
         rvl = (typ_rangedef_t *)dlq_nextEntry(rvl)) {
        /* check if range definition overlaps 
         * Have 
         *   List.lo .. list.hi (rvl->lb .. rvl->ub)
         *   New.lo .. New.hi    (rv->lb .. rv->ub)
         *
         * Not sure it there is a more clever algorithm,
         * but this one simply brute-force makes sure
         * that the New endpoints are not in the List set
         * and the List endpoints are not in the New set.
         * If so, there is overlap.
         */
        if (rv->flags & TYP_FL_LBINF) {
            if (rvl->flags & TYP_FL_LBINF) {
                return ERR_NCX_OVERLAP_RANGE;
            } /* else LB can't overlap this range, keep going */
        } else {
            res = check_range(&rv->lb, rvl, btyp);
            if (res != NO_ERR) {
                return res;
            }
        }

        if (rv->flags & TYP_FL_UBINF) {
            if (rvl->flags & TYP_FL_UBINF) {
                return ERR_NCX_OVERLAP_RANGE;
            }
        } else {
            res = check_range(&rv->ub, rvl, btyp);
            if (res != NO_ERR) {
                return res;
            }
        }
        if (rvl->flags & TYP_FL_LBINF) {
            if (rv->flags & TYP_FL_LBINF) {
                return ERR_NCX_OVERLAP_RANGE;
            }
        } else {
            res = check_range(&rvl->lb, rv, btyp);
            if (res != NO_ERR) {
                return res;
            }
        }
        if (rvl->flags & TYP_FL_UBINF) {
            if (rv->flags & TYP_FL_UBINF) {
                return ERR_NCX_OVERLAP_RANGE;
            }
        } else {
            res = check_range(&rvl->ub, rv, btyp);
            if (res != NO_ERR) {
                return res;
            }
        }
    }

    if (ncx_is_min(&rv->lb, btyp)) {
	rv->flags |= TYP_FL_LBMIN;
    }

    if (ncx_is_max(&rv->ub, btyp)) {
	rv->flags |= TYP_FL_UBMAX;
    }

    /* range is okay, store in the order specified */
    dlq_enque(rv, &sim->rangeQ);
    return NO_ERR;

}  /* insert_rangedef */


/********************************************************************
* FUNCTION consume_rangedef
* 
* Current token is the start of range fragment
* Process the token chain and gather the range fragment
* in a pre-allocated typ_rangedef_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* !!! YANG version is more correct
* !!! This code does not handle min and max evaluation
* !!! correctly.  The parent range min and max are not used
* !!! as defined, but rather the absolute min and max for
* !!! the base data type.
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   rv == rangeval in progress (just preallocated)
*   btyp == base type of range vals
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    consume_rangedef (tk_chain_t *tkc,
		      ncx_module_t *mod,
		      typ_rangedef_t *rv,
		      ncx_btype_t btyp)
{
    status_t   res;
    boolean    sep;

    res = NO_ERR;

    /* save the base type */
    rv->btyp = btyp;

    /* check special case that shows up a lot */
    if (TK_CUR_TYP(tkc) == TK_TT_RANGESEP) {
        /* set lower bound to lowest value for the base type */
        switch (btyp) {
        case NCX_BT_INT8:
            rv->lb.i = NCX_MIN_INT8;
            break;
        case NCX_BT_INT16:
            rv->lb.i = NCX_MIN_INT16;
            break;
        case NCX_BT_INT32:
            rv->lb.i = NCX_MIN_INT;
            break;
        case NCX_BT_INT64:
            rv->lb.l = NCX_MIN_LONG;
            break;
        case NCX_BT_UINT8:
        case NCX_BT_UINT16:
        case NCX_BT_UINT32:
            rv->lb.u = NCX_MIN_UINT;
            break;
        case NCX_BT_UINT64:
            rv->lb.ul = NCX_MIN_ULONG;
            break;
        case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
            rv->lb.f = 0;
            rv->flags |= TYP_FL_LBINF;
#else
            rv->lb.f = xml_strdup((const xmlChar *) NCX_MIN_FLOAT);
            if (!rv->lb.f) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }
#endif
            break;
        case NCX_BT_FLOAT64:
            rv->flags |= TYP_FL_LBINF;
#ifdef HAS_FLOAT
            rv->lb.d = 0;
#else
            rv->lb.d = xml_strdup((const xmlChar *) NCX_MIN_DOUBLE);
            if (!rv->lb.d) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }
#endif
            break;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        /* move past the rangesep token */
        res = TK_ADV(tkc);
        sep = TRUE;
    } else {
        sep = FALSE;
    }

    /* current token must be a number */
    if (res == NO_ERR) {
	if (!TK_CUR_NUM(tkc)) {
	    res = ERR_NCX_WRONG_TKTYPE;
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* save the number */
    if (sep) {
        /* save the upper bound number, advance the token, and exit */
        res = ncx_convert_tkcnum(tkc, btyp, &rv->ub);
        if (res == NO_ERR) {
	    res = TK_ADV(tkc);   /* skip past number token */
	}
    } else {
        /* save the lower bound number */
        res = ncx_convert_tkcnum(tkc, btyp, &rv->lb);
        if (res == NO_ERR) {
	    /* move past the number token */
	    res = TK_ADV(tkc);
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* if we get here, then a lower bound is all that is parsed,
     * and the current token can be a bar, rangesep, or rparen
     */
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_RPAREN:
    case TK_TT_BAR:
        /* BAR or right paren token ends the rangeval */
        res = ncx_copy_num(&rv->lb, &rv->ub, btyp);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	}
        /* DO NOT move past the BAR or RPAREN token, just exit */
        return res;
    case TK_TT_RANGESEP:
        /* skip past the rangesep token */
        res = TK_ADV(tkc);
        break;
    default:
        res = ERR_NCX_WRONG_TKTYPE;
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }
    
    /* if we get here, current token can be number or bar or rparen */
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_DNUM:
    case TK_TT_HNUM:
    case TK_TT_RNUM:
        /* save the upper bound number */
        res = ncx_convert_tkcnum(tkc, btyp, &rv->ub);
        if (res == NO_ERR) {
	    /* move past the number token and exit */
	    res = TK_ADV(tkc);
	}
	break;
    case TK_TT_RPAREN:
    case TK_TT_BAR:
        /* BAR or right paren token ends the rangeval
         * set lower bound to lowest value for the base type 
         */
        switch (btyp) {
        case NCX_BT_INT8:
            rv->ub.i = NCX_MAX_INT8;
            break;
        case NCX_BT_INT16:
            rv->ub.i = NCX_MAX_INT16;
            break;
        case NCX_BT_INT32:
            rv->ub.i = NCX_MAX_INT;
            break;
        case NCX_BT_INT64:
            rv->ub.l = NCX_MAX_LONG;
            break;
        case NCX_BT_UINT8:
            rv->ub.u = NCX_MAX_UINT8;
            break;
        case NCX_BT_UINT16:
            rv->ub.u = NCX_MAX_UINT16;
            break;
        case NCX_BT_UINT32:
            rv->ub.u = NCX_MAX_UINT;
            break;
        case NCX_BT_UINT64:
            rv->ub.ul = NCX_MAX_ULONG;
            break;
        case NCX_BT_FLOAT32:
            rv->flags |= TYP_FL_UBINF;
#ifdef HAS_FLOAT
            rv->ub.f = 0;
#else
            rv->ub.f = xml_strdup((const xmlChar *) NCX_MAX_FLOAT);
            if (!rv->ub.f) {
		res = ERR_INTERNAL_MEM;
            }
#endif
            break;
        case NCX_BT_FLOAT64:
            rv->flags |= TYP_FL_UBINF;
#ifdef HAS_FLOAT
            rv->ub.d = 0;
#else
            rv->ub.d = xml_strdup((const xmlChar *) NCX_MAX_DOUBLE);
            if (!rv->ub.d) {
                res = ERR_INTERNAL_MEM;
            }
#endif
            break;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

        /* DO NOT move past the BAR or RPAREN token, just exit */
        break;
    default:
        res = ERR_NCX_WRONG_TKTYPE;
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
    }

    return res;

}  /* consume_rangedef */


/********************************************************************
* FUNCTION consume_range
* 
* Current token is the start of range token 
* Process the token chain and gather the range definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   typdef == typedef struct in progress (already setup as SIMPLE)
*            simple.btyp == enum for the specific builtin number type
* RETURNS:
  *  status
*********************************************************************/
static status_t 
    consume_range (tk_chain_t *tkc,
		   ncx_module_t *mod,
		   typ_def_t *typdef)
{
    status_t   res;
    boolean    done;
    typ_rangedef_t *rv;
    ncx_btype_t  btyp;

    /* get the correct number type in the range specification */
    btyp = typ_get_range_type(typdef->def.simple.btyp);
    
    /* move past start-of-range tag '(' */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    done = FALSE;
    while (!done) {
        /* get a new range value struct */
        rv = typ_new_rangedef();
        if (!rv) {
            return ERR_INTERNAL_MEM;
        }

        /* get one range spec */
        res = consume_rangedef(tkc, mod, rv, btyp);
        if (res != NO_ERR) {
            typ_free_rangedef(rv, btyp);
            return res;
        }

        /* insert this new rangedef */
        res = insert_rangedef(rv, &typdef->def.simple);
        if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
            typ_free_rangedef(rv, btyp);
            return res;
        }

        /* Current token is either a BAR or RPAREN 
         * Move past itif BAR and keep going if BAR
         * If RPAREN then exit pointing at this token
         */
        switch (TK_CUR_TYP(tkc)) {
        case TK_TT_RPAREN:
            done = TRUE;
            break;
        case TK_TT_BAR:
            res = TK_ADV(tkc);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }
            break;
        default:
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }
    }
    return NO_ERR;

}  /* consume_range */


/********************************************************************
* FUNCTION consume_num_contents
* 
* Process the token chain and gather the number definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain
*    mod == module in progress
*    typdef == typedef struct in progress
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    consume_num_contents (tk_chain_t *tkc,
			  ncx_module_t *mod,
			  typ_def_t *typdef)
{
    status_t   res;

    /* check for start of range '('  */
    res = NO_ERR;
    if (tk_next_typ(tkc) == TK_TT_LPAREN) {
        res = TK_ADV(tkc);
        if (res == NO_ERR) {
            res = consume_range(tkc, mod, typdef);
        } else {
	    ncx_print_errormsg(tkc, mod, res);
	}
    }
    return res;

}  /* consume_num_contents */


/********************************************************************
* FUNCTION consume_num
* 
* Current token is a number (e.g., int, uint, real)
* Process the token chain and gather the number definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain
*    mod == module in progress
*    btyp == enum for the specific builtin number type
*    typdef == typedef struct in progress
*    namebuff == pointer to buffer ptr to hold member-name, of NULL if none
*    pmode == TYP_PM_NORMAL if final TK_TT_SEMICOL should be parsed
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    consume_num (tk_chain_t *tkc,
		 ncx_module_t *mod,
		 ncx_btype_t btyp,
		 typ_def_t *typdef,
		 xmlChar **namebuff,
		 typ_pmode_t  pmode)
{
    status_t   res;
    boolean    getsemi;

    /* (int | uint | real) <wspace> [member-name] <range-spec>+ ; */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    /* setup the typedef as a simple type */
    typ_init_simple(typdef, btyp);

    res = consume_num_contents(tkc, mod, typdef);
    if (res != NO_ERR) {
	return res;
    }

    /* check if the instance qualifier is allowed to be next */
    switch (pmode) {
    case TYP_PM_INDEX:
        getsemi = FALSE;
	res = NO_ERR;
        break;
    case TYP_PM_NORMAL:
        getsemi = TRUE;
        res = check_iqual(tkc, typdef);
        break;
    case TYP_PM_MDATA:
        getsemi = TRUE;
        res = check_meta_iqual(tkc, typdef);
        break;
    case TYP_PM_NONE:
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* check if a semi-colon is supposed to finish this off */
    if (getsemi) {
        /* check for ';'  */
        res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    } 

    return res;

}  /* consume_num */


/********************************************************************
* FUNCTION insert_sval
* 
* Insert a string value in the simple->valQ if not already there
*
* INPUTS:
*   sv == typ_sval_t struct to insert in the sim->valQ
*   valQ == que header to contain this string
* RETURNS:
*   status
*********************************************************************/
static status_t
    insert_sval (typ_sval_t *sv, 
		 dlq_hdr_t   *qhdr)
{
    typ_sval_t *svl;

    /* check if this is a duplicate insert */
    for (svl = (typ_sval_t *)dlq_firstEntry(qhdr);
         svl != NULL;
         svl = (typ_sval_t *)dlq_nextEntry(svl)) {
	if (!xml_strcmp(sv->val, svl->val)) {
	    return ERR_NCX_DUP_ENTRY;
	} 
    }

    /* not a duplicate;  add as new last entry */
    dlq_enque(sv, qhdr);
    return NO_ERR;

}  /* insert_sval */


/********************************************************************
* FUNCTION consume_string_list
* 
* form: { listmem1 listmem2 listmem3 }
*
* Current token is the left brace before a list of string values
* Process the token chain and gather the string definitions
*  in the simple typdef provided
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain
*    module in progress
*    sim == simple typedef to hold info (in the sim->valQ)
*    pat == TRUE if this sval contains a pattern that needs
*             to be compiled into a regex
*        == FALSE if this is not being used as a pattern
*
* RETURNS:
*   status
*********************************************************************/
static status_t 
    consume_string_list (tk_chain_t *tkc,
			 ncx_module_t *mod,
			 typ_simple_t *sim,
			 boolean pat)
{
    status_t   res;
    typ_sval_t  *sv;

    /* start with the left brace */
    for (;;) {
        /* move past left brace or string */
        res = TK_ADV(tkc);
        if (res == NO_ERR) {
	    /* check current token: string or right brace is allowed */
	    if (TK_CUR_WSTR(tkc)) {
		/* save the string in the valQ */
		sv = typ_new_sval(TK_CUR_VAL(tkc), sim->btyp);
		if (!sv) {
		    res = ERR_INTERNAL_MEM;
		}

		if (res == NO_ERR) {
		    if (pat) {
			res = typ_compile_pattern(sim->btyp, sv);
		    }
		    if (res==NO_ERR) {
			res = insert_sval(sv, &sim->valQ);
		    }
		    if (res != NO_ERR) {
			typ_free_sval(sv);
		    }
		}
	    } else if (TK_CUR_TYP(tkc) == TK_TT_RBRACE) {
		/* end of list, backup current token */
		TK_BKUP(tkc);
		res = dlq_empty(&sim->valQ) ?
		    ERR_NCX_EMPTY_VAL : NO_ERR;
		if (res != NO_ERR) {
		    ncx_print_errormsg(tkc, mod, res);
		}
		return res;
	    } else {
		res = ERR_NCX_WRONG_TKTYPE;
	    }
	}
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}
    }
    /*NOTREACHED*/

} /* consume_string_list */


/********************************************************************
* FUNCTION consume_xlist_list
* 
* form: { list1 list2 list3 }
*
* Current token should be the left brace of a set of strings
* Process the token chain and gather the string definitions
*  in the simple typdef provided
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  tkc == token chain
* mod == module in progress
*  sim == simple typedef to hold info (in the sim->valQ)
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_xlist_list (tk_chain_t *tkc,
		       ncx_module_t *mod,
		       typ_simple_t *sim)
{
    status_t   res;
    typ_sval_t  *sv;
    typ_listval_t  *lv;
    boolean   done;

    /* loop until a final right brace or error is seen */
    for (;;) {
        /* get a new list header */
        lv = typ_new_listval();
        if (!lv) {
	    res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }

        /* start with a left brace for a string value set */
        res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
        if (res != NO_ERR) {
            typ_free_listval(lv);
            return res;
        }

        done = FALSE;
        while (!done) {
            res = TK_ADV(tkc);     /* move past left brace or string */
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
		typ_free_listval(lv);
		return res;
	    }

	    /* current token must be a string or right brace */
	    if (TK_CUR_WSTR(tkc)) {
		/* save the whitespace-allowed string in the list header */
		sv = typ_new_sval(TK_CUR_VAL(tkc), sim->btyp);
		if (!sv) {
		    res = ERR_INTERNAL_MEM;
		    typ_free_listval(lv);
		    ncx_print_errormsg(tkc, mod, res);
		    return res;
		} else {
		    /* this sval does not have a pattern
		     * it has a value set instead
		     */
		    res = insert_sval(sv, &lv->strQ);
		    if (res != NO_ERR) {
			ncx_print_errormsg(tkc, mod, res);
			typ_free_listval(lv);
			typ_free_sval(sv);
			return res;
		    }
                }
            } else if (TK_CUR_TYP(tkc) == TK_TT_RBRACE) {
                /* end of list -- do not move past right brace */ 
                done = TRUE;
            } else {
                /* wrong token type */
		res = ERR_NCX_WRONG_TKTYPE;
		ncx_print_errormsg(tkc, mod, res);
                typ_free_listval(lv);
                return res;
            }
        }

	/* check if an instance qualifier is after the string set */
	res = check_xlist_iqual(tkc, lv);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    typ_free_listval(lv);
	    return res;
	}

        /* don't need to check this, as each string list
         * represents the allowed values for a different
         * string position in that list, and duplicates 
         * are allowed across columns
         */
        dlq_enque(lv, &sim->valQ);

        /* check if there are more lists */
        if (tk_next_typ(tkc) == TK_TT_RBRACE) {
            return NO_ERR;
        }  /* else should be left brace to start a new list */
    }
    /*NOTREACHED*/

} /* consume_xlist_list */


/********************************************************************
* FUNCTION consume_string_contents
* 
* Process the token chain and gather the string definition in typdef
* Handles base types:
*   NCX_BT_STRING
*   NCX_BT_BINARY
*   NCX_BT_XLIST
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  tkc == token chain
*  mod == module in progress
*  btyp == enum for the specific builtin string type
*  typdef == typedef struct in progress
*  pmode == TYP_PM_NORMAL if final TK_TT_SEMICOL should be parsed
*  iqual_done == address of the output value holder
*
* OUTPUTS:
*  iqual_done = TRUE if instance qualifier already seen
*
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_string_contents (tk_chain_t *tkc,
			     ncx_module_t *mod,
			     ncx_btype_t btyp,
			     typ_def_t *typdef,
			     typ_pmode_t  pmode,
			     boolean *iqual_done)
{
    status_t     res;
    typ_sval_t  *sv;
    boolean      pat;

    *iqual_done = FALSE;
    pat = FALSE;

    /* The next token can be a lparen (start a range)
     * or a keyword (pattern)
     * or an equals sign (start a value set)
     * or a semi-colon to end a simple string or list typdef
     * IF pmode is not TYP_PM_NORMAL then a comma or right brace
     * instead of a semi-colon can end the string definition
     */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_LPAREN:
        res = consume_range(tkc, mod, typdef);
        if (res != NO_ERR) {
            return res;
        }

        /* check the next token to see if we are done or there
         * are other restrictions (pattern)
         */
        if (tk_next_typ(tkc) == TK_TT_TSTRING) {
            res = TK_ADV(tkc);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }
            /* fall through to next case arm */
        } else { 
            break;
        }
    case TK_TT_TSTRING:
        /* this token can be 'pattern' */
        if (!xml_strcmp(TK_CUR_VAL(tkc), NCX_EL_PATTERN)) {
	    pat = TRUE;
            typdef->def.simple.strrest = NCX_SR_PATTERN;
        } else {
            res = ERR_NCX_WRONG_TKVAL;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
        /* the next token must be an equals or pluseq token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }
        if (!(TK_CUR_TYP(tkc) == TK_TT_EQUAL ||
	      TK_CUR_TYP(tkc) == TK_TT_PLUSEQ)) {
            res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
        /* fall through */
    case TK_TT_EQUAL:
    case TK_TT_PLUSEQ:
	if (TK_CUR_TYP(tkc) == TK_TT_EQUAL) {
	    typdef->def.simple.flags |= TYP_FL_REPLACE;
	}

        /* a simple string or a value set is expected 
	 * after the '=' or '+=' token 
	 */
        if (typdef->def.simple.strrest != NCX_SR_NONE) {
	    /* this was just set to pattern */
            if (btyp==NCX_BT_XLIST) {

		/* do not allow '+=' on list types */
		if (TK_CUR_TYP(tkc)==TK_TT_PLUSEQ) {
		    res = ERR_NCX_WRONG_TKTYPE;
		    ncx_print_errormsg(tkc, mod, res);
		    return res;
		}
		    
                /* form 1: xlist foo pattern = { x y z }; */
		res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
		if (res != NO_ERR) {
		    return res;
		}

                res = consume_string_list(tkc, mod, 
					  &typdef->def.simple, pat);
                if (res != NO_ERR) {
                    return res;
                }
                /* Get the close-set token '}' */
                res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
                if (res != NO_ERR) {
                    return res;
                }
            } else {
                /* form: string foo pattern = x;
                 * btyp = a regular string, not a list
                 * next token should be any STRING, save it in the valQ 
                 */
                res = TK_ADV(tkc);

                if (res == NO_ERR && !TK_CUR_WSTR(tkc)) {
                    res = ERR_NCX_WRONG_TKTYPE;
                }

		if (res == NO_ERR) {
		    sv = typ_new_sval(TK_CUR_VAL(tkc), btyp);
		    if (!sv) {
			res = ERR_INTERNAL_MEM;
		    }
		}

		if (res != NO_ERR) {
		    ncx_print_errormsg(tkc, mod, res);
		    return res;
                }

		/* check if the pattern string needs to be compiled */
		if (pat) {
		    res = typ_compile_pattern(btyp, sv);
		}
		if (res==NO_ERR) {
		    res = insert_sval(sv, &typdef->def.simple.valQ);
		}
                if (res != NO_ERR) {
		    /*** TBD: Get extended error info from regcomp result */
		    ncx_print_errormsg(tkc, mod, res);		    
                    typ_free_sval(sv);
                    return res;
                }
            }
        } else {
            /* a value set is expected */
            typdef->def.simple.strrest = NCX_SR_VALSET;
            if (btyp==NCX_BT_XLIST) {

		/* do not allow '+=' on list types */
		if (TK_CUR_TYP(tkc)==TK_TT_PLUSEQ) {
		    res = ERR_NCX_WRONG_TKTYPE;
		    ncx_print_errormsg(tkc, mod, res);
		    return res;
		}

                /* form: xlist foo = { {a b c} {x y z} }; */
                res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
                if (res != NO_ERR) {
                    return res;
                }
                res = consume_xlist_list(tkc, mod, &typdef->def.simple);
                if (res != NO_ERR) {
                    return res;
                }
                /* Get the close-set token '}' */
                res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
                if (res != NO_ERR) {
                    return res;
                }
            } else {
                /* form: string foo = { a b c };
                 * This is a plain string exact-match value set 
                 * Get a list of whitespace-delimited strings
                 */
                res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
                if (res != NO_ERR) {
                    return res;
                }
                res = consume_string_list(tkc, mod, 
					  &typdef->def.simple, pat);
                if (res != NO_ERR) {
                    return res;
                }
                /* Get the close-set token '}' */
                res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
                if (res != NO_ERR) {
                    return res;
                }
            }
        }
        break;
    case TK_TT_SEMICOL:
        if (pmode==TYP_PM_NORMAL || pmode==TYP_PM_MDATA) {
            TK_BKUP(tkc);
            return NO_ERR;
        } else {
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }
    case TK_TT_COMMA:
        /* This case arm is here only for inline index calls */
        if (pmode!=TYP_PM_NORMAL) {
            TK_BKUP(tkc);
            return NO_ERR;
        } else {
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }
    case TK_TT_RBRACK:
        /* This case arm is here only for inline index calls */
        if (pmode==TYP_PM_INDEX) {
            TK_BKUP(tkc);
            return NO_ERR;
        } else {
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }
    case TK_TT_QMARK:
        if (pmode==TYP_PM_INDEX) {
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        } 
        typdef->iqual = NCX_IQUAL_OPT;
        *iqual_done = TRUE;
        break;
    case TK_TT_STAR:
        if (pmode==TYP_PM_INDEX || pmode==TYP_PM_MDATA) {
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        } 
        typdef->iqual = NCX_IQUAL_ZMORE;
        *iqual_done = TRUE;
        break;
    case TK_TT_PLUS:
        if (pmode==TYP_PM_INDEX || pmode==TYP_PM_MDATA) {
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        } 
        typdef->iqual = NCX_IQUAL_1MORE;
        *iqual_done = TRUE;
        break;
    default:
	res = ERR_NCX_WRONG_TKTYPE;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    return NO_ERR;

}  /* consume_string_contents */


/********************************************************************
* FUNCTION consume_list_contents
* 
* Process the token chain and gather the list definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  tkc == token chain
*  mod == module in progress
*  typdef == typedef struct in progress
*
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_list_contents (tk_chain_t *tkc,
			   ncx_module_t *mod,
			   typ_def_t *typdef)
{
    status_t   res;
    boolean    doerr;

    doerr = FALSE;
    res = NO_ERR;

    /* The next token can be a lparen (start a range) or a left brace */
    if (tk_next_typ(tkc) == TK_TT_LPAREN) {
	/* advance to the left paren */
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    doerr = TRUE;
	}

	/* get the range definition */
	if (res==NO_ERR) {
	    res = consume_range(tkc, mod, typdef);
	}
    }

    /* The left brace must be next */
    if (res==NO_ERR) {
	res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    }

    /* the next token must be an ID name */
    if (res==NO_ERR) {
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    doerr = TRUE;
	}
    }
    if (res==NO_ERR && !TK_CUR_ID(tkc)) {
	res = ERR_NCX_WRONG_TKTYPE;
	doerr = TRUE;
    }
	
    /* must find the type template from the mod-qual type name */
    if (res==NO_ERR) {
	res = typ_locate_template(mod, TK_CUR_MOD(tkc),
				  TK_CUR_VAL(tkc), 
				  &typdef->def.simple.listtyp);
	if (res == NO_ERR) {
	    /* get a template, check okay for list contents */
	    if (!ok_for_list(typ_get_basetype
			     (&typdef->def.simple.listtyp->typdef))) {
		res = ERR_NCX_WRONG_TYPE;
	    }
	}
	if (res != NO_ERR) {
	    doerr = TRUE;
	}
    }

    /* The right brace must be next */
    if (res==NO_ERR) {
	res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    }

    /* print error only it did not come from a ncx_consume_ fn */
    if (doerr) {
	ncx_print_errormsg(tkc, mod, res);
    }

    return res;

}  /* consume_list_contents */


/********************************************************************
* FUNCTION consume_list
* 
* Current token is a list (NCX_BT_SLIST)
* Process the token chain and gather the list definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  tkc == token chain
*  mod == module in progress
*  typdef == typedef struct in progress
*  namebuff == ptr to buffer ptr for member-name, of NULL if none
*
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_list (tk_chain_t *tkc,
		  ncx_module_t *mod,
		  typ_def_t *typdef,
		  xmlChar **namebuff)
{
    status_t     res;

    /* c-list-type :== "list" [uint-range-def] <wspace> type-id ";" */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    /* setup the typedef as a simple type */
    typ_init_simple(typdef, NCX_BT_SLIST);

    res = consume_list_contents(tkc, mod, typdef);
    if (res != NO_ERR) {
	return res;
    }

    /* check for any instance qualifier */
    res = check_iqual(tkc, typdef);
    
    /* check for closing ';'  */
    if (res == NO_ERR) {
	res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    }
    
    return res;

}  /* consume_list */


/********************************************************************
* FUNCTION consume_string
* 
* Current token is a string or list (e.g., string, binary, xlist)
* Process the token chain and gather the string definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  tkc == token chain
*  mod == module in progress
*  btyp == enum for the specific builtin string type
*  typdef == typedef struct in progress
*  namebuff == ptr to buffer ptr for member-name, of NULL if none
*  pmode == TYP_PM_NORMAL if final TK_TT_SEMICOL should be parsed
*
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_string (tk_chain_t *tkc,
		    ncx_module_t *mod,
		    ncx_btype_t btyp,
		    typ_def_t *typdef,
		    xmlChar **namebuff,
		    typ_pmode_t  pmode)
{
    status_t     res;
    boolean      iqual_done, getsemi;

    /* c-string-type :== "string" <wspace> member-name */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    /* setup the typedef as a simple type */
    typ_init_simple(typdef, btyp);
    iqual_done = FALSE;

    res = consume_string_contents(tkc, mod, btyp, typdef, pmode, &iqual_done);
    if (res != NO_ERR) {
	return res;
    }

    /* check if the instance qualifier is allowed to be next */
    switch (pmode) {
    case TYP_PM_INDEX:
        getsemi = FALSE;
        break;
    case TYP_PM_NORMAL:
        getsemi = TRUE;
        if (!iqual_done) {
            res = check_iqual(tkc, typdef);
        }
        break;
    case TYP_PM_MDATA:
        getsemi = TRUE;
        if (!iqual_done) {
            res = check_meta_iqual(tkc, typdef);
        }
	break;
    case TYP_PM_NONE:
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    if (getsemi) {
        /* check for closing ';'  */
        res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    }

    return res;

}  /* consume_string */


/********************************************************************
* FUNCTION insert_child
* 
* Insert a child node in a complex->childQ
*
* INPUTS:
*   ch == typ_child_t struct to insert in the cpx->childQ
*   typdef == complex typdef to contain this child node
*   insmode == INS_TEST for test only, INS_REAL for test and insert
* RETURNS:
*   status
*********************************************************************/
static status_t
    insert_child (typ_child_t *ch, 
		  typ_def_t *typdef,
		  typ_insmode_t  insmode)
{
    typ_index_t   *in;
    typ_child_t   *chl;
    typ_complex_t *cpx;


    cpx = &typdef->def.complex;

    /* check if this is a group header, if so, it has already
     * been screened, so just save it and exit
     */
    if (!dlq_empty(&ch->groupQ)) {
        dlq_enque(ch, &cpx->childQ);
        return NO_ERR;
    }

    /* if this is a table, then check the index first */
    if (cpx->btyp==NCX_BT_LIST) {
        for (in = (typ_index_t *)dlq_firstEntry(&cpx->indexQ);
             in != NULL;
             in = (typ_index_t *)dlq_nextEntry(in)) {
            if (in->ityp==NCX_IT_INLINE || in->ityp==NCX_IT_REMOTE) {
                /* these index types are not pointers to child members */
                if (!xml_strcmp(in->typch.name, ch->name)) {
                    return ERR_NCX_DUP_ENTRY;
                }
            }
        }
    }

    /* check the member list so far */
    chl = typ_find_child(ch->name, cpx);
    if (chl) {
        return ERR_NCX_DUP_ENTRY;
    }

#ifdef TYP_PARSE_DEBUG
    log_debug3("\ntyp_parse: Adding child %s to %s",
	      ch->name, tk_get_btype_sym(cpx->btyp));
#endif

    if (insmode==INS_REAL) {
        /* no error, so make new last entry */
	ch->parent = typdef;
        dlq_enque(ch, &cpx->childQ);
    }

    return NO_ERR;

}  /* insert_child */


/********************************************************************
* FUNCTION insert_metadata
* 
* Insert a child node in a simple->metaQ or complex->metaQ
*
* INPUTS:
*   ch == typ_child_t struct to insert in the cpx->childQ
*   metaQ == Q header to contain this metadata child node
* RETURNS:
*   status
*********************************************************************/
static status_t
    insert_metadata (typ_child_t *ch, 
		     dlq_hdr_t *metaQ)
{
    typ_child_t *chl;

    /* check the metadata list so far */
    for (chl = (typ_child_t *)dlq_firstEntry(metaQ);
         chl != NULL;
         chl = (typ_child_t *)dlq_nextEntry(chl)) {
        if (!xml_strcmp(ch->name, chl->name)) {
            return ERR_NCX_DUP_ENTRY;
        }
    }

#ifdef TYP_PARSE_DEBUG
    log_debug3("\ntyp_parse: Adding metadata %s", ch->name);
#endif

    /* no error, so make new last entry */
    dlq_enque(ch, metaQ);
    ch->parent = NULL;
    return NO_ERR;

}  /* insert_metadata */


/********************************************************************
* FUNCTION move_past_index_sep
* 
* Current token should be end of an index construct
* Move past the construct to a comma or right bracket
* Set the done flag if right bracket, then move past this token
*
* INPUTS:
*  tkc == token chain
* OUTPUTS:
*  *done set to TRUE if right bracket seen
* RETURNS:
*  status
*********************************************************************/
static status_t 
    move_past_index_sep (tk_chain_t *tkc,
			 boolean  *done)
{
    status_t   res;

    /* move past the definition to a comma of right bracket */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        return res;
    }
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_COMMA:
        *done = FALSE;
        return TK_ADV(tkc);          /* move past comma */
    case TK_TT_RBRACK:
        *done = TRUE;         
        return NO_ERR;       /* do not move past rbrack */
    default:
        return ERR_NCX_WRONG_TKTYPE;
    }
    /*NOTREACHED*/
} /* move_past_index_sep */


/********************************************************************
* FUNCTION search_module
* 
* Search the module for the specified identifier
* Brute force search only used in this case, before the
* module has been added to the definition registry.
*
* Searches only Types and PSD Queues
*
* INPUTS:
*    mod == ncx_module_t in progress to search
*    name == name string to find
* OUTPUTS:
*    *dtyp == type of data pointer returned 
* RETURNS:
*    void * cast of the data structure found or NULL if not found
*********************************************************************/
static void *
    search_module (ncx_module_t *mod,
		   const xmlChar *name,
		   ncx_node_t *dtyp)
{
    typ_template_t *typ;
    psd_template_t *psd;

    /* search the type Queue */
    typ = ncx_find_type(mod, name);
    if (typ) {
        *dtyp = NCX_NT_TYP;
        return (void *)typ;
    }

    /* search the PSD Queue */
    psd = ncx_find_psd(mod, name);
    if (psd) {
        *dtyp = NCX_NT_PSD;
        return (void *)psd;
    }

    *dtyp = NCX_NT_NONE;
    return NULL;

} /* search_module */


/********************************************************************
* FUNCTION resolve_scoped_name
* 
* Find the scoped identifier in the symbol path
*
* INPUTS:
*    mod == ncx_module_t in progress
*    tkc == token chain; current token is the scoped name string
*    forindex == TRUE if index type restrictions should be enforced
* OUTPUTS:
*    *namebuff is filled in with the final scoped name segment
*    typdef is filled in with the type definition for the 
*        specified scoped identifier (if NO_ERR)
* RETURNS:
*   status
*********************************************************************/
static status_t 
    resolve_scoped_name (ncx_module_t  *mod,
			 tk_chain_t *tkc,
			 boolean forindex,
			 xmlChar  **namebuff,
			 typ_def_t *typdef)
{
    xmlChar        *buff;
    const xmlChar  *next;
    void           *dptr;
    ncx_node_t      dtyp;
    typ_template_t *typ;
    typ_def_t      *chtyp;
    psd_template_t *psd;
    psd_parm_t     *parm;
    status_t        res;

    /* get the top-level definition name and look for it
     * in the current module first
     */
    dtyp = NCX_NT_NONE;

    buff = m__getMem(NCX_MAX_NLEN+1);
    if (!buff) {
	return SET_ERROR(ERR_INTERNAL_MEM);
    }

    next = ncx_get_name_segment(TK_CUR_VAL(tkc), buff);

    if (TK_CUR_MOD(tkc) != NULL) {
        /* look in the specified module for the template */
        dptr = ncx_locate_modqual_import(TK_CUR_MOD(tkc), 
					 TK_CUR_VAL(tkc), 
					 mod->diffmode, &dtyp);
        if (!dptr) {
	    m__free(buff);
            return ERR_NCX_DEF_NOT_FOUND;
        }
    } else {
        dptr = search_module(mod, buff, &dtyp);
        if (!dptr) {
            /* try the import path for the identifier */
            dptr = ncx_locate_import(mod, buff, &dtyp);
            if (!dptr) {
		m__free(buff);
                return ERR_NCX_DEF_NOT_FOUND;
            }
        }
    }

    /* May have something to return, now keep getting the 
     * next segment in the scoped name and looking for 
     * a matching node in the TYP, or PSD that was found
     * 
     * Get the first child member name because it is handled
     * differently for each definition class
     */
    if (!*next) {
	m__free(buff);
        return ERR_NCX_INVALID_TOKEN;
    }
    next = ncx_get_name_segment(++next, buff);

    /* (dptr, dtyp) indicate parent, now find child-name in buff */
    switch (dtyp) {
    case NCX_NT_TYP:
        typ = (typ_template_t *)dptr;
        chtyp = typ_find_type_member(&typ->typdef, buff);
        break;
    case NCX_NT_PSD:
        psd = (psd_template_t *)dptr;
        parm = psd_find_parm(psd, buff);
        if (!parm) {
	    m__free(buff);
            return ERR_NCX_DEFSEG_NOT_FOUND;
        }
        res = typ_locate_template(mod, parm->modstr, parm->typname, &typ);
        if (res != NO_ERR) {
	    m__free(buff);
            return res;
        }
        chtyp = &typ->typdef;
        break;
    default:
	m__free(buff);
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    
    /* 'chtyp' now points to the typ_def_t of the first tier 
     * child node, and can be handled the same the rest
     * of the way.
     *
     * Keep looping until there are no more name segments left
     * or an error occurs
     *
     * Each time get_name_segment is called, the next pointer
     * will be a dot or end of string.
     */
    while (*next) {
        /* get the next segment in 'buff' */
        next = ncx_get_name_segment(++next, buff);

        /* look for the ID in buff in the member names */
        chtyp = typ_find_type_member(chtyp, buff);
        if (!chtyp) {
	    m__free(buff);
            return ERR_NCX_DEFSEG_NOT_FOUND;
        }
    }

    /* chtyp points to the final segment, named in buff
     * make sure very last segment is valid for an index
     * if the forindex flag is set
     * Save the final name segment if requested
     * Setup and return the typdef for a named type
     */
    if (forindex && !ok_for_named_index(chtyp)) {
	m__free(buff);
        return ERR_NCX_TYPE_NOT_INDEX;
    }

    if (namebuff) {
	/* pass buff off to theoutput parameter */
        *namebuff = buff;
    } else {
	m__free(buff);
    }

    typdef->class = NCX_CL_REF;
    typdef->def.ref.typdef = chtyp;

    return NO_ERR;

} /* resolve_scoped_name */


/********************************************************************
* FUNCTION resolve_local_scoped_name
* 
* Find the scoped identifier in the specified complex typedef
* 
* E.g.: foo.bar.baz
*
* INPUTS:
*    cpx == complex type to check
*    name == scoped name string to find
* OUTPUTS:
*    *td is set to the typdef of the found local scoped member
* RETURNS:
*   status
*********************************************************************/
static status_t 
    resolve_local_scoped_name (typ_complex_t *cpx,
			       const xmlChar *name,
			       typ_def_t **td)
{
    xmlChar        *buff;
    const xmlChar  *next;
    typ_child_t    *ch;
    typ_def_t      *typdef;
    ncx_btype_t    btyp;

    buff = m__getMem(NCX_MAX_NLEN+1);
    if (!buff) {
	return ERR_INTERNAL_MEM;
    }

    /* get the top-level definition name and look for it
     * in the child queue.  This is going to work because
     * the token was already parsed as a scoped token string
     */
    next = ncx_get_name_segment(name, buff);

    /* one of the child names must match the name in buff */
    ch = typ_find_child(buff, cpx);
    if (!ch) {
	m__free(buff);
        return ERR_NCX_INDEX_TYPE_NOT_FOUND;
    }        

    /* 'ch' now points to the first tier child node
     * matching the first name companent
     *
     * Each time get_name_segment is called, the next pointer
     * will be a dot or end of string.
     *
     * Keep looping until there are no more name segments left
     * or an error occurs.  The first time the loop is entered
     * the *next char should be non-zero.
     */
    typdef = &ch->typdef;
    while (*next) {
        /* there is a next child, this better be a complex 
         * setup the real typdef if needed (named and ref)
         */
        switch (typdef->class) {
        case NCX_CL_NONE:
	    m__free(buff);
            return SET_ERROR(ERR_INTERNAL_VAL);
        case NCX_CL_BASE:
        case NCX_CL_SIMPLE:
	    m__free(buff);
            return ERR_NCX_DEFSEG_NOT_FOUND;
	case NCX_CL_COMPLEX:
	    break;
        case NCX_CL_NAMED:
            btyp = typ_get_basetype(typdef);
            if (ncx_get_tclass(btyp) != NCX_CL_COMPLEX) {
		m__free(buff);
                return ERR_NCX_DEFSEG_NOT_FOUND;
            }
            typdef = typ_get_next_typdef(typdef);
            break;
        case NCX_CL_REF:
            typdef = typ_get_next_typdef(typdef);
            if (typdef->class != NCX_CL_COMPLEX) {
		m__free(buff);
                return ERR_NCX_DEFSEG_NOT_FOUND;
            }
            break;
        default:
	    m__free(buff);
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

        /* typdef is now set to real typdef struct
         * get the next segment in 'buff' 
         */
        next = ncx_get_name_segment(++next, buff);

        /* find the specified child of this child */
        ch = typ_find_child(buff, &typdef->def.complex);
        if (ch) {
            typdef = &ch->typdef;
        } else {
	    m__free(buff);
            return ERR_NCX_DEFSEG_NOT_FOUND;
        }
    }

    /* get the final typedef to return */
    *td = typ_get_next_typdef(typdef);

    m__free(buff);

    return NO_ERR;

} /* resolve_local_scoped_name */


/********************************************************************
* FUNCTION consume_named
* 
* Current token is supposed to be a type name
* Check through the import processing algorithm
* looking for the module that has this type defined.
*
* Fill in typdef if a type is found
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
*
* INPUTS:
*  mod == ncx_module_t in progress
*  tkc == token chain
*  typdef == typedef struct in progress
*  namebuff == pointer to hold buffer ptr for member-name, or NULL if none
*  pmode == TYP_PM_NORMAL if final TK_TT_SEMICOL should be parsed
*  chdeferr == TRUE if ERR_DEF_NOT_FOUND error msg should be generated
*           == FALSE if ERR_DEF_NOT_FOUND error msg should not be generated
*              
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_named (ncx_module_t *mod, 
		   tk_chain_t *tkc,
		   typ_def_t *typdef,
		   xmlChar **namebuff,
		   typ_pmode_t  pmode,
		   boolean chdeferr)
{
    status_t     res;
    boolean      getsemi, iqual_done;
    ncx_btype_t  btyp;
    ncx_tclass_t tclass;
    tk_type_t    nexttyp;

    /* generic form: 
     * 
     * typeName [member-name] [type-extensions] [iqual] ;
     *
     * The current token is a TSTRING which should be a type name
     */

    iqual_done = FALSE;

    /* init named typedef */
    typdef->class = NCX_CL_NAMED;
    typdef->def.named.typ = NULL;

    /* The next token must be a type name (token string) */
    if (!TK_CUR_ID(tkc)) {
	res = ERR_NCX_WRONG_TKTYPE;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* look in this module and then the import path for the template */
    res = typ_locate_template(mod, TK_CUR_MOD(tkc), TK_CUR_VAL(tkc), 
                              &typdef->def.named.typ);
    if (res != NO_ERR) {
	if (res != ERR_NCX_DEF_NOT_FOUND || chdeferr) {
	    ncx_print_errormsg(tkc, mod, res);
	}
        return res;
    }

    /* these fields used by yangdump -- ignoring malloc failure!! */
    if (typdef->def.named.typ) {
	typdef->linenum = typdef->def.named.typ->linenum;
	typdef->typename = xml_strdup(typdef->def.named.typ->name);
	if (typdef->def.named.typ->mod &&
	    typdef->def.named.typ->mod != mod) {
	    typdef->prefix = xml_strdup(typdef->def.named.typ->mod->name);
	}
    }

    /* get the member-name in namebuff if present */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    /* check if a type extension is specified
     *  - range, value set, or pattern
     */
    nexttyp = tk_next_typ(tkc);
    if (nexttyp==TK_TT_LPAREN || nexttyp==TK_TT_EQUAL ||
	nexttyp==TK_TT_PLUSEQ || nexttyp==TK_TT_TSTRING) {

	/* only allowed to extend base or simple types */
	tclass = typ_get_base_class(&typdef->def.named.typ->typdef);
	switch (tclass) {
	case NCX_CL_BASE:
	case NCX_CL_SIMPLE:
	    break;
	default:
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}

	/* allocate the typ_def_t used to extend or replace the
	 * restrictions of the parent typdef
	 */
	typdef->def.named.newtyp = typ_new_typdef();
	if (!typdef->def.named.newtyp) {
	    res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}

	/* initialize the new typdef with the parent base type */
	btyp = typ_get_basetype(&typdef->def.named.typ->typdef);
	typ_init_simple(typdef->def.named.newtyp, btyp);

	switch (btyp) {
	case NCX_BT_ENUM:
	    res = consume_enum_contents(tkc, mod, typdef->def.named.newtyp);
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
	    res = consume_num_contents(tkc, mod, typdef->def.named.newtyp);
	    break;
	case NCX_BT_STRING:
	case NCX_BT_BINARY:
	    res = consume_string_contents(tkc, mod, btyp,
		  typdef->def.named.newtyp, pmode, &iqual_done);
	    break;
	case NCX_BT_ENAME:
	    if (pmode==TYP_PM_NORMAL || pmode==TYP_PM_INDEX) {
		res = consume_string_contents(tkc, mod, btyp, 
		     typdef->def.named.newtyp, pmode, &iqual_done);
	    } else {
		res = ERR_NCX_WRONG_TKTYPE;
		ncx_print_errormsg(tkc, mod, res);
		return res;
	    }
	    break;
	case NCX_BT_SLIST:
	    if (pmode == TYP_PM_NORMAL) {
		res = consume_list_contents(tkc, mod,  
		    typdef->def.named.newtyp);
	    } else {
		res = ERR_NCX_WRONG_TKTYPE;
		ncx_print_errormsg(tkc, mod, res);
		return res;
	    }
	    break;
	case NCX_BT_XLIST:
	    if (pmode == TYP_PM_NORMAL) {
		res = consume_string_contents(tkc, mod, btyp, 
		      typdef->def.named.newtyp, pmode, &iqual_done);
	    } else {
		res = ERR_NCX_WRONG_TKTYPE;
		ncx_print_errormsg(tkc, mod, res);
		return res;
	    }
	    break;
	default:
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}		
    }

    /* check if the instance qualifier is allowed to be next */
    switch (pmode) {
    case TYP_PM_INDEX:
        if (!ok_for_named_index(&typdef->def.named.typ->typdef)) {
            res = ERR_NCX_TYPE_NOT_INDEX;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
        getsemi = FALSE;
        break;
    case TYP_PM_MDATA:
        if (!ok_for_named_metadata(&typdef->def.named.typ->typdef)) {
            res = ERR_NCX_TYPE_NOT_MDATA;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
        getsemi = TRUE;
	if (!iqual_done) {
	    res = check_meta_iqual(tkc, typdef);
	    if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
		return res;
	    }
	}
	break;
    case TYP_PM_NORMAL:
        getsemi = TRUE;
	if (!iqual_done) {
	    res = check_iqual(tkc, typdef);
	    if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
		return res;
	    }
	}
        break;
    case TYP_PM_NONE:
    default:
        return ERR_INTERNAL_VAL;
    }

    if (getsemi) {
        /* get the closing semi-colon */
        res = ncx_consume_token(tkc, mod, TK_TT_SEMICOL);
    }
    return res;

}  /* consume_named */


/********************************************************************
* FUNCTION consume_index
* 
* Current token should be start of index (left bracket)
* Process the token chain and gather the index definition in cpx->indexQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  mod == ncx_module_t in progress
*  tkc == token chain
*  cpx == complex typedef struct in progress
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_index (ncx_module_t  *mod,
		   tk_chain_t *tkc,
		   typ_complex_t *cpx)
{
    status_t        res;
    typ_index_t    *in;
    boolean         done, modqual;
    ncx_btype_t     btyp;

    /* form: [index1, index2, index3] 
     *
     * Need to create a typ_index_t struct for each index component
     */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACK);
    if (res != NO_ERR) {
        return res;
    }

    /* move past left bracket */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    /* loop through the comma separated list of index components */
    modqual = FALSE;
    done = FALSE;
    while (!done) {
        /* current token is the start of an index component */
        switch (TK_CUR_TYP(tkc)) {
        case TK_TT_RBRACK:
            /* Do not move past the right bracket -- just exit OK 
             * EVEN IF THE cpx->indexQ IS STILL EMPTY
             */
            done = TRUE;
            break;
        case TK_TT_TSTRING:
            /* (A) try this is as a base type name first */
            btyp = tk_get_btype_id(TK_CUR_VAL(tkc), TK_CUR_LEN(tkc));
            if (btyp != NCX_BT_NONE) {

                /* check if this is ok for an index definition */
                if (!typ_ok_for_inline_index(btyp)) {
                    /* base type not allowed in index (like choice) */
		    res = ERR_NCX_TYPE_NOT_INDEX;
		    ncx_print_errormsg(tkc, mod, res);
                    return res;
                }

                /* this is a valid type; get a new index record */
                in = typ_new_index();
                if (!in) {
		    res = ERR_INTERNAL_MEM;
		    ncx_print_errormsg(tkc, mod, res);
                    return res;
                }

                /* try to parse an inline base type definition 
                 * consume_btype expects to start out advancing the
                 * token to the first keyword, so backup the cur pointer
                 */
                TK_BKUP(tkc);    
                res = consume_btype(mod, tkc, &in->typch.typdef,
                       &in->typch.name, TYP_PM_INDEX);
                if (res != NO_ERR) {
                    typ_free_index(in);
                    return res;
                }
                in->ityp = NCX_IT_INLINE;
                dlq_enque(in, &cpx->indexQ);
                res = move_past_index_sep(tkc, &done);
                if (res != NO_ERR) {
		    ncx_print_errormsg(tkc, mod, res);		    
                    return res;
                }
            } else {
                /* (B) (C) string is not a base type, try a named type */
                in = typ_new_index();
                if (!in) {
		    res = ERR_INTERNAL_MEM;
		    ncx_print_errormsg(tkc, mod, res);
                    return res;		    
                }
                res = consume_named(mod, tkc, &in->typch.typdef, 
				    &in->typch.name, TYP_PM_INDEX, FALSE);
                if (res == NO_ERR) {
                    /* add this valid index and move to next index */
                    in->ityp = NCX_IT_NAMED;
                    dlq_enque(in, &cpx->indexQ);
                    res = move_past_index_sep(tkc, &done);
                    if (res != NO_ERR) {
			ncx_print_errormsg(tkc, mod, res);
			return res;		    
                    }
                } else if (res != ERR_NCX_DEF_NOT_FOUND) {
		    return res;		    
                } else {
                    /* (D) string is not a named type 
                     * assume for now it is a local table member name
                     * This will get checked after the table members
                     * are parsed. This fwd reference is allowed.
                     */
                    if (TK_CUR_LEN(tkc) && (TK_CUR_LEN(tkc)<NCX_MAX_NLEN)) {
                        in = typ_new_index();
                        if (!in) {
			    res = ERR_INTERNAL_MEM;
			    ncx_print_errormsg(tkc, mod, res);		    
			    return res;    
                        }
                        in->typch.name = xml_strdup(TK_CUR_VAL(tkc));
			if (!in->typch.name) {
			    res = ERR_INTERNAL_MEM;
			    ncx_print_errormsg(tkc, mod, res);
			    typ_free_index(in);
			    return res;
			}
                        in->ityp = NCX_IT_LOCAL;

                        /* the typdef is untouched and will be set 
                         * when finish_index is called
                         */
                        dlq_enque(in, &cpx->indexQ);
                        
                        /* move past the definition and the comma or
                         * right bracket that follows
                         */
                        res = move_past_index_sep(tkc, &done);
                        if (res != NO_ERR) {
			    ncx_print_errormsg(tkc, mod, res);
                            return res;
                        }
                    } else {
			res = ERR_NCX_WRONG_LEN;
			ncx_print_errormsg(tkc, mod, res);
			return res;
                    }
                }
            }
            break;
        case TK_TT_MSSTRING:
            modqual = TRUE;
            /* fall through */
        case TK_TT_SSTRING:
            /* (E) this is a scoped token string, which
             * can be either a local nested member-name
             * or a remote nested member name
             */
            in = typ_new_index();
            if (!in) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
		return res;
            }
            in->sname = xml_strdup(TK_CUR_VAL(tkc));
            if (!in->sname) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
                typ_free_index(in);
                return res;
            }

	    res = resolve_scoped_name(mod, tkc,
		      TRUE, &in->typch.name, &in->typch.typdef);
	    if (res == NO_ERR) {
		/* this is a remote scoped identifier */
		in->ityp = NCX_IT_SREMOTE;
	    } else if (modqual) {
		res = ERR_NCX_DEF_NOT_FOUND;
		ncx_print_errormsg(tkc, mod, res);
                typ_free_index(in);
                return res;
	    } else {
                /* assume it is a scoped local.
                 * the typdef is untouched and will be set 
                 * when finish_index is called
                 */
                in->ityp = NCX_IT_SLOCAL;
		in->typch.name = xml_strdup(set_name_from_sname(in->sname));
            }
            dlq_enque(in, &cpx->indexQ);
            res = move_past_index_sep(tkc, &done);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }
            break;
        case TK_TT_MSTRING:
            /* (B) (C) mod-qual: must be a named type */
            in = typ_new_index();
            if (!in) {
		res = ERR_INTERNAL_MEM;
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }
            res = consume_named(mod, tkc, &in->typch.typdef, 
                                &in->typch.name, TYP_PM_INDEX, TRUE);
            if (res == NO_ERR) {
                /* add this valid index and move to next index */
                in->ityp = NCX_IT_NAMED;
                dlq_enque(in, &cpx->indexQ);
                res = move_past_index_sep(tkc, &done);
                if (res != NO_ERR) {
		    ncx_print_errormsg(tkc, mod, res);
		    return res;
                }
            }
            return res;
        default:
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
    }
    return NO_ERR;

}  /* consume_index */


/********************************************************************
* FUNCTION finish_index
* 
* Resolve any NCX_IT_LOCAL and NCX_IT_SLOCAL index component types
*
* INPUTS:
*  cpx == complex typedef for a table to check
* RETURNS:
*  status
*********************************************************************/
static status_t 
    finish_index (typ_complex_t *cpx)
{
    typ_index_t    *in;
    typ_child_t    *ch;
    typ_def_t      *typdef;
    boolean         done;
    status_t        res;

    /* check each index component */
    for (in = (typ_index_t *)dlq_firstEntry(&cpx->indexQ);
         in != NULL;
         in = (typ_index_t *)dlq_nextEntry(in)) {
        if (in->ityp==NCX_IT_LOCAL) {
            /* one of the child names must match this index name */
            done = FALSE;
            ch = typ_find_child(in->typch.name, cpx);
            if (ch) {
                if (ok_for_named_index(&ch->typdef)) {
                    in->typch.typdef.class = NCX_CL_REF;
                    in->typch.typdef.def.ref.typdef = &ch->typdef;
                } else {
                    return ERR_NCX_TYPE_NOT_INDEX;
                }
            } else {
                return ERR_NCX_INDEX_TYPE_NOT_FOUND;
            }
        } else if (in->ityp==NCX_IT_SLOCAL) {
            res = resolve_local_scoped_name(cpx, in->sname, &typdef);
            if (res != NO_ERR) {
		return res;
	    }
	    if (ok_for_named_index(typdef)) {
		in->typch.typdef.class = NCX_CL_REF;
		in->typch.typdef.def.ref.typdef = typdef;
	    }
	} 
    }

    return NO_ERR;

}  /* finish_index */


/********************************************************************
* FUNCTION consume_maxrows
* 
* Consume a maxrows clause if present
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Example: 40 instances allowed, indexed by fooName string
*
*          table [fooName] (40) {  .... }
*
* INPUTS:
*   mod == module in progress
*   tkc == token chain
*   cpx == typ_complex_t struct to contain this child node
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    consume_maxrows (ncx_module_t *mod,
		     tk_chain_t *tkc,
		     typ_complex_t *cpx)
{
    status_t  res;
    boolean   doerr;
    ncx_num_t num;

    if (tk_next_typ(tkc) != TK_TT_LPAREN) {
	return ERR_NCX_SKIPPED;
    }

    doerr = FALSE;
    res = TK_ADV(tkc);    /* move to '[' */
    if (res == NO_ERR) {
	res = TK_ADV(tkc);   /* move to uint number */
    }
    if (res == NO_ERR) {
	if (!TK_CUR_INUM(tkc)) {
	    res = ERR_NCX_WRONG_TKTYPE;
	} else {
	    res = ncx_convert_tkcnum(tkc, NCX_BT_UINT32, &num);
	}
	if (res != NO_ERR) {
	    doerr = TRUE;
	} else {
	    cpx->maxrows = num.u;
	}
    }
    if (res == NO_ERR) {
	res = ncx_consume_token(tkc, mod, TK_TT_RPAREN);
    }
    if (doerr) {
	ncx_print_errormsg(tkc, mod, res);
    }
    return res;

}  /* consume_maxrows */


/********************************************************************
* FUNCTION consume_complex
* 
* Current token is a complex type (e.g., struct, choice, table)
* Process the token chain and gather the complex definition in typdef
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*  mod == ncx_module_t in progress
*  tkc == token chain
*  btyp == enum for the specific builtin complex type
*  typdef == typedef struct in progress
*  namebuff == ptr to buffer ptr for member-name, of NULL if none
*  pmode == parser mode
*
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_complex (ncx_module_t *mod,
		     tk_chain_t *tkc,
		     ncx_btype_t btyp,
		     typ_def_t *typdef,
		     xmlChar **namebuff,
		     typ_pmode_t  pmode)
{
    status_t     res;
    boolean      done, grdone;
    uint32       choicenum;
    typ_child_t *ch, *grch, *testch;

    choicenum = 0;

    /* setup the typedef as a complex type */
    typ_init_complex(typdef, btyp);

    /* (struct | choice | table) [member-name] { ... } */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    /* get the index if this is a table */
    if (btyp==NCX_BT_LIST) {
        res = consume_index(mod, tkc, &typdef->def.complex);
        if (res != NO_ERR) {
            return res;
        }
	res = consume_maxrows(mod, tkc, &typdef->def.complex);
        if (res != NO_ERR && res != ERR_NCX_SKIPPED) {
            return res;
        }
    }


    /* get the opening left brace of the child section */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
        return res;
    }

    /* get the child members */
    done = FALSE;
    while (!done) {
        /* get a new child node */
        ch = typ_new_child();
        if (!ch) {
            res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }

	/* number the choice members so merge operation can 
	 * tell them apart later
	 */
	if (btyp == NCX_BT_CHOICE) {
	    choicenum++;
	}

        /* check if this is choice and a group is being declared */
        if (btyp==NCX_BT_CHOICE && tk_next_typ(tkc)==TK_TT_LBRACK) {
            res = ncx_consume_token(tkc, mod, TK_TT_LBRACK);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                typ_free_child(ch);
                return res;
            }
            grdone = FALSE;
            while (!grdone) {
                /* get a new group child node */
                grch = typ_new_child();
                if (!grch) {
                    res = ERR_INTERNAL_MEM;
		    ncx_print_errormsg(tkc, mod, res);
                    typ_free_child(ch);
		    return res;
                }

                /* recurse through and get a type decl */
                res = consume_btype(mod, tkc, &grch->typdef, 
                                    &grch->name, pmode);
                if (res != NO_ERR) {
                    typ_free_child(grch);
                    typ_free_child(ch);
                    return res;
                }

                /* check for duplicate names, not a real insert */
                res = insert_child(grch, typdef, INS_TEST);
                if (res != NO_ERR) {
		    ncx_print_errormsg(tkc, mod, res);
                    typ_free_child(grch);
                    typ_free_child(ch);
                    return res;
                }

                /* check for duplicate names in the list we have so far */
                for (testch = (typ_child_t *)dlq_firstEntry(&ch->groupQ);
                     testch != NULL;
                     testch = (typ_child_t *)dlq_nextEntry(testch)) {
                    if (!xml_strcmp(testch->name, grch->name)) {
                        res = ERR_NCX_DUP_ENTRY;
			ncx_print_errormsg(tkc, mod, res);
                        typ_free_child(grch);
                        typ_free_child(ch);
			return res;
                    }
                }

                /* finally add the new node to the child group Q */
		grch->grouptop = ch;
		grch->typdef.choicenum = choicenum;
                dlq_enque(grch, &ch->groupQ);

                /* check if the group is ending */
                if (tk_next_typ(tkc) == TK_TT_RBRACK) {
                    res = TK_ADV(tkc);
                    if (res != NO_ERR) {
			ncx_print_errormsg(tkc, mod, res);
                        typ_free_child(ch);
                        return res;
                    }
                    grdone = TRUE;
                }
            }
            
            /* save the group header child -- the ch->typdef struct
	     * is left NCX_CL_NONE for this kind of typ_child node
	     */
            res = insert_child(ch, typdef, INS_REAL);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                typ_free_child(ch);
                return res;
            }
        } else {
            /* regular parm, not a choice group decl;
             * recurse through and get this type decl 
             */
            res = consume_btype(mod, tkc, &ch->typdef, &ch->name, pmode);
            if (res != NO_ERR) {
                typ_free_child(ch);
                return res;
            }

	    if (btyp == NCX_BT_CHOICE) {
		ch->typdef.choicenum = choicenum;
	    }

            /* save this child node; check for duplicate names */
            res = insert_child(ch, typdef, INS_REAL);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                typ_free_child(ch);
                return res;
            }
        }

        /* move to right brace if it is next, or just keep going */
        if (tk_next_typ(tkc) == TK_TT_RBRACE) {
            res = TK_ADV(tkc);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }

            /* the instance qualifier is allowed to be next */
            res = check_iqual(tkc, typdef);
            if (res != NO_ERR) {
		ncx_print_errormsg(tkc, mod, res);
                return res;
            }
            done = TRUE;
        }
    }

    /* if this is a table, then the index clause needs to be 
     * finished up if it contains any forward references
     */
    if (btyp == NCX_BT_LIST) {
        res = finish_index(&typdef->def.complex);
        if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
            return res;
        }
    }

    return NO_ERR;

}  /* consume_complex */


/********************************************************************
* FUNCTION consume_container_type
* 
* Current token is a container type
* Process the token chain and gather the definition in typdef
*
* INPUTS:
*  mod == ncx_module_t in progress
*  tkc == token chain
*  typdef == typedef struct in progress
*  namebuff == ptr to buffer ptr for member-name, of NULL if none
*  pmode == parser mode
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_container_type (ncx_module_t *mod,
			    tk_chain_t *tkc,
			    typ_def_t *typdef,
			    xmlChar **namebuff,
			    typ_pmode_t  pmode)
{
    status_t      res;
    typ_child_t  *ch;
    typ_index_t  *in;

    /* setup the return typedef as a container type */
    typ_init_complex(typdef, NCX_BT_XCONTAINER);

    /* container [member-name] [index] { mname name } */
    res = consume_name(tkc, mod, namebuff);
    if (res != NO_ERR) {
	return res;
    }

    /* get the index clause */
    res = consume_index(mod, tkc, &typdef->def.complex);
    if (res != NO_ERR) {
	return res;
    }

    /* get the maxrows clause container foo [a, b, c] (27) { } */
    res = consume_maxrows(mod, tkc, &typdef->def.complex);
    if (res != NO_ERR && res != ERR_NCX_SKIPPED) {
	return res;
    }
    
    /* get the opening left brace of the child section */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    if (res != NO_ERR) {
        return res;
    }

    /* move past the left brace */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    /* get the single child node */
    ch = typ_new_child();
    if (!ch) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* consume 1 named data type 
     * This works even if the type is really a base type like struct 
     */
    res = consume_named(mod, tkc, &ch->typdef, &ch->name, pmode, TRUE); 
    if (res != NO_ERR) {
	typ_free_child(ch);
	return res;
    }

    /* Check that the child data type is not a table or container
     * The use of table is conceptually incompatible, and the use
     * of container is too complicated -- wrt/ figuring out the
     * array index tuple of an arbitrary subtree
     */
    switch (typ_get_basetype(&ch->typdef)) {
    case NCX_BT_LIST:
    case NCX_BT_XCONTAINER:
	res = ERR_NCX_WRONG_DATATYP;
	ncx_print_errormsg(tkc, mod, res);
	typ_free_child(ch);
	return res;
    default:
	break;
    }

    /* get the closing left brace of the child section */
    res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    if (res == NO_ERR) {
	/* check if any instance qualifiers present */
	res = check_iqual(tkc, typdef);
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	typ_free_child(ch);
        return res;
    }

    /* save this child node; no need check for duplicate names */    
    ch->parent = typdef;
    dlq_enque(ch, &typdef->def.complex.childQ);    

    res = finish_index(&typdef->def.complex);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* check that only local or scoped-local index types are used */
    for (in = (typ_index_t *)dlq_firstEntry(&typdef->def.complex.indexQ);
	 in != NULL;
	 in = (typ_index_t *)dlq_nextEntry(in)) {
	if (in->ityp != NCX_IT_LOCAL && in->ityp != NCX_IT_SLOCAL) {
	    res = ERR_NCX_WRONG_INDEX_TYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}
    }

    return NO_ERR;

}  /* consume_container_type */


/********************************************************************
* FUNCTION consume_btype
* 
* Check the current token for a base type name or a named type
* if allowed. Fill in typdef if all OK.
*
* Current token is a container type
* Process the token chain and gather the definition in typdef
*
* INPUTS:
*  mod == ncx_module_t in progress
*  tkc == token chain
*  typdef == typedef struct in progress
*  namebuff == NON-NULL then named type OK, not OK if NULL
*           This is only NULL for top-layer type definitions,
*           which have no name associated with them until
*           used as a member in another type or as a parm name, etc.
*  pmode == TYP_PM_NORMAL if the terminating TK_TT_SEMICOL token
*           should be parsed, some other value if not. 
*           For table index parsing, set this flag to TYP_PM_INDEX
*           For metadata clause parsing, set this flag to TYP_PM_MDATA
* OUTPUTS:
*  if namebuff was non-NULL, then a member-name TSTRING is expected
*  saved in this name buffer, if NO_ERR
* RETURNS:
*  status
*********************************************************************/
static status_t 
    consume_btype (ncx_module_t *mod, 
		   tk_chain_t *tkc,
		   typ_def_t *typdef,
		   xmlChar **namebuff,
		   typ_pmode_t pmode)
{
    ncx_btype_t   btyp;
    status_t      res;

    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        return res;
    }
    if (!TK_CUR_VAL(tkc)) {
        res = ERR_NCX_WRONG_TKVAL;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }
    btyp = tk_get_btype_id(TK_CUR_VAL(tkc), TK_CUR_LEN(tkc));

    switch (btyp) {
    case NCX_BT_NONE:
        res = consume_named(mod, tkc, typdef, namebuff, pmode, TRUE);
        break;
    case NCX_BT_ENUM:
    case NCX_BT_BITS:
        res = consume_enum_bits(tkc, mod, typdef, btyp, namebuff, pmode);
        break;
    case NCX_BT_ANY:
    case NCX_BT_ROOT:
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
        res = consume_plain_base(tkc, mod, btyp, typdef, namebuff, pmode);
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
        res = consume_num(tkc, mod, btyp, typdef, namebuff, pmode);
        break;
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
        res = consume_string(tkc, mod, btyp, typdef, namebuff, pmode);
        break;
    case NCX_BT_UNION:
	if (pmode==TYP_PM_NORMAL || pmode==TYP_PM_MDATA) {
	    res = consume_union(tkc, mod, typdef, namebuff);
	} else {
            res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	}
        break;
    case NCX_BT_ENAME:
        if (pmode==TYP_PM_NORMAL || pmode==TYP_PM_INDEX) {
            res = consume_string(tkc, mod, btyp, typdef, namebuff, pmode);
        } else {
            res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
        break;
    case NCX_BT_SLIST:
        if (pmode == TYP_PM_NORMAL) {
            res = consume_list(tkc, mod, typdef, namebuff);
        } else {
            res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
        break;
    case NCX_BT_XLIST:
        if (pmode == TYP_PM_NORMAL) {
            res = consume_string(tkc, mod, btyp, typdef, namebuff, pmode);
        } else {
            res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
        break;
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_LIST:
        if (pmode == TYP_PM_NORMAL) {
            res = consume_complex(mod, tkc, btyp, typdef, namebuff, pmode);
        } else {
            res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
        break;
    case NCX_BT_XCONTAINER:
        if (pmode == TYP_PM_NORMAL) {
            res = consume_container_type(mod, tkc, typdef, namebuff, pmode);
        } else {
            res = ERR_NCX_WRONG_TKTYPE;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }
	break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    return res;

}  /* consume_btype */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION typ_parse_syntax_contents
* 
* Parse the contents of a 'type' syntax clause
*
* Current token is a container type
* Process the token chain and gather the definition in typdef
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain, pointing at the token just before
*          the start of the syntax contents (as usual)
* OUTPUTS:
*   typ->typdef is filled in, if NO_ERR
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    typ_parse_syntax_contents (ncx_module_t *mod, 
			       tk_chain_t *tkc,
			       typ_template_t *typ)

{
#ifdef DEBUG
    if (!mod || !tkc || !typ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef TYP_PARSE_DEBUG
    log_debug3("\ntyp_parse_syntax: (%s.%s)", mod->name, typ->name);
#endif

    return consume_btype(mod, tkc, &typ->typdef, NULL, TYP_PM_NORMAL);

}  /* typ_parse_syntax_contents */


/********************************************************************
* FUNCTION typ_process_metadata_contents
* 
* Parse the contents of a 'metadata' clause
*
* Current token is a container type
* Process the token chain and gather the definition in typdef
*
* INPUTS:
*   mod == ncx_module_t in progress
*   tkc == token chain, pointing at the token just before
*          the start of the syntax contents (as usual)
* OUTPUTS:
*   typ->typdef is filled in, if NO_ERR
* RETURNS:
*   status
*********************************************************************/
status_t 
    typ_parse_metadata_contents (ncx_module_t *mod, 
				 tk_chain_t *tkc,
				 typ_template_t *typ)

{
    status_t      res;
    boolean       done;
    typ_child_t  *ch;

#ifdef DEBUG
    if (!mod || !tkc || !typ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    switch (typ->typdef.class) {
    case NCX_CL_SIMPLE:
    case NCX_CL_COMPLEX:
    case NCX_CL_NAMED:
        break;
    default:
        res = ERR_NCX_MDATA_NOT_ALLOWED;
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    done = FALSE;
    while (!done) {
        /* get the metadata members */
        ch = typ_new_child();
        if (!ch) {
            res = ERR_INTERNAL_MEM;
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
        }

        /* get this base or named type decl */
        res = consume_btype(mod, tkc, &ch->typdef, &ch->name, TYP_PM_MDATA);
        if (res != NO_ERR) {
            typ_free_child(ch);
            return res;
        }

        /* check for duplicate names while saving this entry */
        switch (typ->typdef.class) {
        case NCX_CL_SIMPLE:
            res = insert_metadata(ch, &typ->typdef.def.simple.metaQ);
            break;
        case NCX_CL_COMPLEX:
            res = insert_metadata(ch, &typ->typdef.def.complex.metaQ);
            break;
        case NCX_CL_NAMED:
	    /* setup the typedef for adding metadata and restrictions 
	     * It doesn't matter what type we use here because the real
	     * data type is in the referenced typ_def_t.  
	     *
	     * This simple typ_def_t is just used for metadata
	     * Simple types can also override restrictions
	     */
	    if (!typ->typdef.def.named.newtyp) {
		typ->typdef.def.named.newtyp = typ_new_typdef();
		if (!typ->typdef.def.named.newtyp) {
		    res = ERR_INTERNAL_MEM;
		    ncx_print_errormsg(tkc, mod, res);
		    return res;
		} else {
		    typ_init_simple(typ->typdef.def.named.newtyp, 
				    NCX_BT_INT32);
		}
	    }
            res = insert_metadata(ch, 
                &typ->typdef.def.named.newtyp->def.simple.metaQ);
            break;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
            typ_free_child(ch);
            return res;
        }

        if (tk_next_typ(tkc)==TK_TT_RBRACE) {
            done = TRUE;
        }
    }
    return NO_ERR;

}  /* typ_parse_metadata_contents */


/* END file typ_parse.c */
