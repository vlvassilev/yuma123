/*  FILE: conf.c

    This module parses an NCX Text config file

    The text input format matches the output
    format of vaL-dump_value and val_stdout_value.

    In addition to that syntax, comments may also be present.
    The '#' char starts a comments and all chars up until
    the next newline char (but not including the newline)
    end a comment.

    A comment cannot be inside a string.
    
   Commands in a config file are not whitespace sensitive,
   except within string values.

   Unless inside a string, a newline will end a command line,
   so <name> value pairs must appear on the same line.

     foo 42
     bar fred
     baz fred flintstone
     goo "multi line
          value entered for goo
          must start on the same line as goo"

   Containers are represented by curly braces

   myvars {
     foo 42
     bar fred
     baz fred flintstone
   }

   The left brace must appear on the same line as foo
   The right brace must be after the newline for the
   preceding (baz) command.

   Objects with index values are listed sequentially
   after the list name.  Whitespace is significant,
   so string index values with whitespace or newlines
   must be in double quotes.  

   The nodes within the array which represent the
   index will not be repreated inside the curly braces.
  
   For example, the array 'myarray' is indexed by 'foo',
   so it does not appear again within the entry.

   myarray 42 {
     bar fred
     baz fred flintstone
   }
 
   point 22 478 {
     red 6
     green 17
     blue 9
   }

   
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
20oct07      abb      begun

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

#ifndef _H_conf
#include "conf.h"
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

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tk
#include  "tk.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define CONF_DEBUG 1
#endif


/********************************************************************
*                                                                   *
*                              T Y P E S                            *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION adv_tk
* 
* Advance to the next token
* Print error message if EOF found instead
*
* INPUTS:
*   tkc == token chain
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    adv_tk (tk_chain_t  *tkc)
{
    status_t  res;

    res = TK_ADV(tkc);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, NULL, res);
    }
    return res;
	    
}   /* adv_tk */



/********************************************************************
* FUNCTION get_tk
* 
* Get the next token
* Skip over TK_TT_NEWLINE until a different type is found
*
* INPUTS:
*   tkc == token chain
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_tk (tk_chain_t  *tkc)
{
    boolean done;
    status_t  res;

    res = NO_ERR;
    done = FALSE;
    while (!done) {
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    done = TRUE;
	} else if (TK_CUR_TYP(tkc) != TK_TT_NEWLINE) {
	    done = TRUE;
	}
    }
    return res;
	    
} /* get_tk */




/********************************************************************
* FUNCTION consume_tk
* 
* Consume a specified token type
* Skip over TK_TT_NEWLINE until a different type is found
*
* INPUTS:
*   tkc == token chain
*   ttyp == expected token type
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    consume_tk (tk_chain_t  *tkc,
		tk_type_t  ttyp)
{
    status_t  res;

    res = get_tk(tkc);
    if (res != NO_ERR) {
	return res;
    } else {
	return (TK_CUR_TYP(tkc) == ttyp) ? 
	    NO_ERR : ERR_NCX_WRONG_TKTYPE;
    }
    /*NOTREACHED*/
	    
} /* consume_tk */


/********************************************************************
* FUNCTION match_name
* 
* Get the next token which must be a string which matches name
*
* INPUTS:
*   tkc == token chain
*   name  == name to match
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    match_name (tk_chain_t  *tkc,
		const xmlChar *name)
{
    status_t res;

    /* get the next token */
    res = consume_tk(tkc, TK_TT_TSTRING);
    if (res == NO_ERR) {
	if (xml_strcmp(TK_CUR_VAL(tkc), name)) {
	    res = ERR_NCX_WRONG_VAL;
	}
    }
    return res;

} /* match_name */


/********************************************************************
* FUNCTION skip_parmset
* 
* Skip until the end of the parmset
* current token is the parmset name
*
* INPUTS:
*   tkc == token chain
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    skip_parmset (tk_chain_t  *tkc)
{
    status_t res;
    uint32   brace_count;
    boolean  done;

    brace_count = 0;
    done = FALSE;

    /* get the next token */
    while (!done) {
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    return res;
	}
	switch (TK_CUR_TYP(tkc)) {
	case TK_TT_LBRACE:
	    brace_count++;
	    break;
	case TK_TT_RBRACE:
	    if (brace_count <= 1) {
		return NO_ERR;
	    } else {
		brace_count--;
	    }
	default:
	    ;
	}
    }
    return NO_ERR;

} /* skip_parmset */


/********************************************************************
* FUNCTION parse_index
* 
* Parse, and fill the indexQ for one val_value_t struct during
* processing of a text config file
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* The value name is the current token.
* Based on the value typdef, the res of the tokens
* comprising the value statement will be processed
*
* INPUTS:
*   tkc == token chain
*   typdef == the typdef struct to use for filling in 'val'
*   val == initialized value struct, without any value,
*          which will be filled in by this function
*   nsid == namespace ID to use for this value
*
* OUTPUTS:
*   indexQ filled in as tokens representing the index components
*   are parsed.  NEWLINE tokens are skipped as needed
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    parse_index (tk_chain_t  *tkc,
		 typ_def_t *typdef,
		 val_value_t *val,
		 xmlns_id_t  nsid)
{
    typ_index_t   *indef, *infirst;
    val_value_t   *inval;
    status_t       res;

    infirst = typ_first_index(TYP_DEF_COMPLEX(typdef));

    /* first make value nodes for all the index values */
    for (indef = infirst; indef != NULL; indef = typ_next_index(indef)) {

	/* advance to the next non-NEWLINE token */
	res = get_tk(tkc);
	if (res != NO_ERR) {
	    ncx_conf_exp_err(tkc, res, "index value");
	    return res;
	}

	/* check if a valid token is given for the index value */
	if (TK_CUR_TEXT(tkc)) {
	    inval = val_make_simval(typ_get_index_typdef(indef),
				    nsid,
				    typ_get_index_name(indef),
				    TK_CUR_VAL(tkc), &res);
	    if (!inval) {
		ncx_conf_exp_err(tkc, res, "index value");
		return res;
	    } else {
		val_add_child(inval, val);
	    }
	} else {
	    res = ERR_NCX_WRONG_TKTYPE;
	    ncx_conf_exp_err(tkc, res, "index value");
	    return res;
	}
    }

    /* generate the index chain in the indexQ */
    res = val_gen_index_chain(infirst, val);
    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, NULL, res);
    }

    return res;
	
} /* parse_index */


/********************************************************************
* FUNCTION parse_val
* 
* Parse, and fill one val_value_t struct during
* processing of a text config file
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* The value name is the current token.
* Based on the value typdef, the res of the tokens
* comprising the value statement will be processed
*
* INPUTS:
*   tkc == token chain
*   typdef == the typdef struct to use for filling in 'val'
*   val == initialized value struct, without any value,
*          which will be filled in by this function
*   nsid == namespace ID to use for this value
*   valname == name of the value struct
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    parse_val (tk_chain_t  *tkc,
	       typ_def_t *typdef,
	       val_value_t *val,
	       xmlns_id_t  nsid,
	       const xmlChar *valname)
{
    typ_def_t       *chdef;
    val_value_t     *chval;
    status_t          res;
    ncx_btype_t       btyp;
    boolean           done;

    btyp = typ_get_basetype(typdef);

    /* check if there is an index clause expected */
    if (typ_has_index(btyp)) {
	res = parse_index(tkc, typdef, val, nsid);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get next token, NEWLINE is significant at this point */
    res = adv_tk(tkc);
    if (res != NO_ERR) {
	return res;
    }

    /* the current token should be the value for a leaf
     * or a left brace for the start of a complex type
     * A NEWLINE is treated as if the user entered a
     * zero-length string for the value.  (Unless the
     * base type is NCX_BT_EMPTY, in which case the NEWLINE
     * is the expected token
     */
    if (typ_is_simple(btyp)) {
	/* form for a leaf is: foo [value] NEWLINE  */
	if (TK_CUR_TEXT(tkc)) {
	    res = val_set_simval(val, typdef, nsid, valname,
				 TK_CUR_VAL(tkc));
	} else if (TK_CUR_TYP(tkc)==TK_TT_NEWLINE) {
	    res = val_set_simval(val, typdef, nsid, valname, NULL);
	} else {
	    res = ERR_NCX_WRONG_TKTYPE;
	}
	if (res != NO_ERR) {
	    ncx_conf_exp_err(tkc, res, "simple value string");
	    return res;
	}

	/* get a NEWLINE unless current token is already a NEWLINE */
	if (TK_CUR_TYP(tkc) != TK_TT_NEWLINE) {
	    res = adv_tk(tkc);
	    if (res != NO_ERR) {
		return res;
	    }
	    if (TK_CUR_TYP(tkc) != TK_TT_NEWLINE) {
		res = ERR_NCX_WRONG_TKTYPE;
		ncx_conf_exp_err(tkc, res, "\\n");
	    }
	}
    } else {
	/* complex type is foo {  ... } or
	 * foo index1 index2 { ... }
	 * If there is an index, it was already parsed
	 */
	res = consume_tk(tkc, TK_TT_LBRACE);
	if (res != NO_ERR) {
	    ncx_conf_exp_err(tkc, res, "left brace");
	    return res;
	}

	/* get all the child nodes specified for this complex type */
	res = NO_ERR;
	done = FALSE;
	while (!done && res==NO_ERR) {
	    /* start out looking for a child node name or a
	     * right brace to end the sub-section
	     */
	    if (tk_next_typ(tkc)==TK_TT_NEWLINE) {
		/* skip the NEWLINE token */
		(void)adv_tk(tkc);
	    } else if (tk_next_typ(tkc)==TK_TT_RBRACE) {
		/* found end of sub-section */
		done = TRUE;
	    } else {
		/* get the next token */
		res = adv_tk(tkc);
		if (res != NO_ERR) {
		    continue;
		}

		/* make sure cur token is an identifier string
		 * if so, find the child node and call this function
		 * recursively to fill it in and add it to
		 * the parent 'val'
		 */
		if (TK_CUR_ID(tkc)) {
		    /* parent 'typdef' must have a child with a name
		     * that matches the current token vale
		     */
		    chdef = typ_find_child_typdef(TK_CUR_VAL(tkc),
						  typdef);
		    if (chdef) {
			chval = val_new_value();
			if (!chval) {
			    res = ERR_INTERNAL_MEM;
			    ncx_print_errormsg(tkc, NULL, res);
			} else {
			    res = parse_val(tkc, chdef,
					chval, nsid,
					TK_CUR_VAL(tkc));
			    if (res == NO_ERR) {
				val_add_child(chval, val);
			    } else {
				val_free_value(chval);
			    }
			}
		    } else {
			/* string is not a child name in this typdef */
			res = ERR_NCX_DEF_NOT_FOUND;
			ncx_conf_exp_err(tkc, res, "identifier string");
		    }
		} else {
		    /* token is not an identifier string */
		    res = ERR_NCX_WRONG_TKTYPE;
		    ncx_conf_exp_err(tkc, res, "identifier string");
		}
	    }
	}  /* end loop through all the child nodes */

	/* expecting a right brace to finish the complex value */
	if (res == NO_ERR) {
	    res = consume_tk(tkc, TK_TT_RBRACE);
	    if (res != NO_ERR) {
		ncx_conf_exp_err(tkc, res, "right brace");
		return res;
	    }
	}
    }

    return res;

}  /* parse_val */


/********************************************************************
* FUNCTION parse_parm
* 
* Parse, and fill one ps_parm_t struct during
* processing of a parmset
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
*
* INPUTS:
*   tkc == token chain
*   ps == parmset to fill in
*   keepvals == TRUE to save existing parms in 'ps', as needed
*               FALSE to overwrite old parms in 'ps', as needed
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    parse_parm (tk_chain_t  *tkc,
		ps_parmset_t *ps,
		boolean keepvals)
{
    const psd_parm_t *psd_parm;
    typ_template_t   *typ;
    ps_parm_t        *curparm, *newparm;
    status_t          res;
    ncx_iqual_t       iqual;
    boolean           match;

    /* get the next token, which must be a TSTRING
     * representing the parameter name 
     */
    if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
	ncx_conf_exp_err(tkc, res, "parameter name");
	return res;
    }

    /* check if this TSTRING is a parameter in this parmset */
    curparm = ps_find_parm(ps, TK_CUR_VAL(tkc));
    if (curparm) {
	psd_parm = curparm->parm;
    } else {
	psd_parm = psd_find_parm(ps->psd, TK_CUR_VAL(tkc));
    }
    if (!psd_parm) {
	res = ERR_NCX_UNKNOWN_PARM;
	ncx_conf_exp_err(tkc, res, "parameter name");
	return res;
    }

    /* got a valid parameter name, now create a new parm
     * even if it may not be kept.  There are corner-cases
     * that require the new value be parsed before knowing
     * if a parm value is a duplicate or not
     */
    newparm = ps_new_parm();
    if (!newparm) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(tkc, NULL, res);
	return res;
    }
    ps_setup_parm(newparm, ps, psd_parm);
    typ = psd_get_parm_template(psd_parm);

    /* parse the parameter value */
    res = parse_val(tkc, &typ->typdef,
		    newparm->val, psd_get_parm_nsid(psd_parm),
		    psd_parm->name);
    if (res != NO_ERR) {
	return res;
    }

    /* check if a potential current value exists, or just
     * add the newparm to the parmset
     */
    if (curparm) {
	iqual = typ_get_iqualval(typ);
	if (iqual == NCX_IQUAL_ONE || iqual == NCX_IQUAL_OPT) {
	    /* only one allowed, check really a match */
	    match = TRUE;
	    if (val_has_index(curparm->val) &&
		!val_index_match(newparm->val, curparm->val)) {
		match = FALSE;
	    }

	    if (!match) {
		ps_add_parm(newparm, ps, NCX_MERGE_LAST);
	    } else if (keepvals) {
		/* keep current value and toss new value */
		log_warn("\nconf: Parameter %s already exists. "
			 "Not using new value", curparm->parm->name);
		ps_free_parm(newparm);
	    } else {
		/* replace current value and warn old value tossed */
		log_warn("\nconf: Parameter %s already exists. "
			 "Overwriting with new value",
			 curparm->parm->name);
		ps_remove_parm(curparm);
		ps_free_parm(curparm);
		ps_add_parm(newparm, ps, NCX_MERGE_LAST);
	    }
	} else {
	    /* mutliple instances allowed */
	    ps_add_parm(newparm, ps, NCX_MERGE_LAST);
	}
    } else {
	ps_add_parm(newparm, ps, NCX_MERGE_LAST);
    }

    return NO_ERR;

}  /* parse_parm */


/********************************************************************
* FUNCTION parse_parmset
* 
* Parse, and fill one ps_parmset_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
*
* INPUTS:
*   tkc == token chain
*   ps == parmset iniitalized and could be already filled in
*   keepvals == TRUE to save existing parms in 'ps', as needed
*               FALSE to overwrite old parms in 'ps', as needed
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    parse_parmset (tk_chain_t  *tkc,
		   ps_parmset_t *ps,
		   boolean keepvals)
{
    status_t       res;
    boolean        done;

    /* get the parmset name */
    done = FALSE;
    while (!done) {
	res = match_name(tkc, ps->psd->name);
	if (res == ERR_NCX_EOF) {
	    log_debug("\nconf: parmset '%s' not found in file '%s'",
		      ps->psd->name, tkc->filename);
	    return NO_ERR;
	} else if (res != NO_ERR) {
	    res = skip_parmset(tkc);
	    if (res != NO_ERR) {
		return res;
	    }
	} else {
	    done = TRUE;
	}
    }

    /* get a left brace */
    res = consume_tk(tkc, TK_TT_LBRACE);
    if (res != NO_ERR) {
	ncx_conf_exp_err(tkc, res, "left brace to start parmset");
	return res;
    }

    done = FALSE;
    while (!done) {

	res = get_tk(tkc);
	if (res == ERR_NCX_EOF) {
	    return NO_ERR;
	} else if (res != NO_ERR) {
	    return res;
	}
	
	/* allow an empty parmset */
	if (TK_CUR_TYP(tkc)==TK_TT_RBRACE) {
	    done = TRUE;
	} else {
	    res = parse_parm(tkc, ps, keepvals);
	    if (res != NO_ERR) {
		done = TRUE;
	    }
	}
    }

    return NO_ERR;

}  /* parse_parmset */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION conf_parse_from_filespec
* 
* Parse a file as an NCX text config file against
* a specific parmset definition.  Fill in an
* initialized parmset (and could be partially filled in)
*
* Error messages are printed by this function!!
*
* If a value is already set, and only one value is allowed
* then the 'keepvals' parameter will control whether that
* value will be kept or overwitten
*
* INPUTS:
*   filespec == absolute path or relative path
*               This string is used as-is without adjustment.
*   ps       == parmset to fill in, must be initialized
*               already with ps_setup_parmset
*   keepvals == TRUE if old values should always be kept
*               FALSE if old vals should be overwritten
*   fileerr == TRUE to generate a missing file error
*              FALSE to return NO_ERR instead, if file not found
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    conf_parse_ps_from_filespec (const xmlChar *filespec,
				 ps_parmset_t *ps,
				 boolean keepvals,
				 boolean fileerr)
{
    tk_chain_t    *tkc;
    FILE          *fp;
    status_t       res;

#ifdef DEBUG
    if (!filespec || !ps) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 
#endif

    fp = fopen((const char *)filespec, "r");
    if (!fp) {
	if (fileerr) {
	    log_error("\nError: config file '%s' could not be opened",
		      filespec);
	    return ERR_FIL_OPEN;
	} else {
	    return NO_ERR;
	}
    }

    /* get a new token chain */
    res = NO_ERR;
    tkc = tk_new_chain();
    if (!tkc) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(NULL, NULL, res);
	fclose(fp);
        return res;
    }

    /* else setup the token chain and parse this config file */
    tk_setup_chain_conf(tkc, fp, filespec);

    res = tk_tokenize_input(tkc, NULL);

#ifdef CONF_TK_DEBUG
    if (LOGDEBUG3) {
	tk_dump_chain(tkc);
    }
#endif

    if (res == NO_ERR) {
	res = parse_parmset(tkc, ps, keepvals);
    }

    fclose(fp);
    tkc->fp = NULL;
    tk_free_chain(tkc);

    return res;

}  /* conf_parse_ps_from_filespec */


/* END file conf.c */
