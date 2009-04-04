/*  FILE: ps_parse.c

  Parse an XML representation of a parameter set instance,
  and create a ps_parmset_t struct that contains the data found.

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
21oct05      abb      begun
19dec05      abb      restart after refining NCX
10feb06      abb      change raw xmlTextReader interface to use
                      ses_cbt_t instead
10feb07      abb      split common routines from agt_ps_parse
                      so mgr code can use it

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_ps_parse
#include "ps_parse.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_var
#include "var.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define PS_PARSE_DEBUG 1 */

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION parse_complex_parm
* 
* Create a ps_parm_t struct for the specified parm value,
* and insert it into the parmset (complex)
*
* Called only by parse_parm
*
* INPUTS:
*   parm == psd_parm_t descriptor for the missing parm
*   strval == string representation of the parm value
*             (may be NULL if parm btype is NCX_BT_EMPTY
*   new_parm == initialized return parm struct to be
*               filled in and added to the parmset (ps)
*   script == TRUE if parsing a script (in the manager)
*          == FALSE if parsing XML (in the agent)
*
* OUTPUTS:
*   If the specified parm is mandatory w/defval defined, then a 
*   new ps_parm_t will be inserted in the ps->parmQ as required
*   to fill in the parmset.
*
* RETURNS:
*   status 
*********************************************************************/
static status_t
    parse_complex_parm (psd_parm_t *parm,
			const xmlChar *strval,
			ps_parm_t  *new_parm,
			boolean script)
{
    val_value_t    *chval;
    typ_template_t *typ;
    typ_def_t      *typdef;
    typ_child_t    *chtyp;
    ncx_btype_t     btyp, chbtyp;
    status_t        res;
    boolean         done;
    
    /* get the default value */
    typ = (typ_template_t *)parm->pdef;
    typdef = &typ->typdef;
    btyp = typ_get_basetype(typdef);
    res = NO_ERR;

    switch (btyp) {
    case NCX_BT_ANY:
    case NCX_BT_ROOT:
	if (script) {
	    (void)var_get_script_val(typdef,
				     new_parm->val,
				     psd_get_parm_nsid(parm),
				     parm->name,
				     strval,
				     ISPARM,
				     &res);
	} else {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	}
	break;
    case NCX_BT_CONTAINER:
	/* other complex types not supported yet */
	res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	break;
    case NCX_BT_CHOICE:
	val_init_complex(new_parm->val, NCX_BT_CHOICE);

	chval = val_new_value();
	if (!chval) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    /* look for a valid 'choice' match */
	    done = FALSE;
	    for (chtyp = typ_first_child(&typdef->def.complex);
		 chtyp != NULL && !done;
		 chtyp = typ_next_child(chtyp)) {

		chbtyp = typ_get_basetype(&chtyp->typdef);

		/* keep trying nodes in the choice until one matches */
		if (chbtyp == NCX_BT_EMPTY) {
		    if (!xml_strcmp(strval, chtyp->name)) {
			res = val_set_simval(chval,
					     &chtyp->typdef,
					     psd_get_parm_nsid(parm),
					     chtyp->name,
					     strval);
			done = TRUE;
		    } else {
			continue;
		    }
		} else if (typ_is_simple(chbtyp)) {
		    if (script) {
			(void)var_get_script_val(&chtyp->typdef,
						 chval,
						 psd_get_parm_nsid(parm),
						 chtyp->name,
						 strval,
						 ISPARM,
						 &res);
		    } else {
			res = val_set_simval(chval,
					     &chtyp->typdef,  
					     psd_get_parm_nsid(parm), 
					     chtyp->name,
					     strval);
		    }

		    /* check if the string parsed okay for this choice */
		    if (res==NO_ERR && chval->res==NO_ERR) {
			done = TRUE;
		    } else {
			val_clean_value(chval);
		    }
		}  /* else complex child nodes skipped !!! */
	    }

	    if (!done) {
		/* didn't finnd any simvals that accepted or no sim types */
		res = ERR_NCX_OPERATION_NOT_SUPPORTED;
		val_free_value(chval);
	    } else {
		res = NO_ERR;
		val_add_child(chval, new_parm->val);
	    }
	}
	break;
    case NCX_BT_LIST:
    case NCX_BT_XCONTAINER:
    default:
	/* other complex types not supported yet */
	res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

    return res;

}  /* parse_complex_parm */


/********************************************************************
* FUNCTION parse_parm
* 
* Create a ps_parm_t struct for the specified parm value,
* and insert it into the parmset (extended)
*
* INPUTS:
*   ps == ps_parmset_t to adjust
*   parm == psd_parm_t descriptor for the missing parm
*   strval == string representation of the parm value
*             (may be NULL if parm btype is NCX_BT_EMPTY
*   script == TRUE if parsing a script (in the manager)
*          == FALSE if parsing XML (in the agent)
*
* OUTPUTS:
*   If the specified parm is mandatory w/defval defined, then a 
*   new ps_parm_t will be inserted in the ps->parmQ as required
*   to fill in the parmset.
*
* RETURNS:
*   status 
*********************************************************************/
static status_t
    parse_parm (ps_parmset_t *ps,
		psd_parm_t *parm,
		const xmlChar *strval,
		boolean script)
{
    ps_parm_t      *new_parm;
    typ_template_t *typ;
    typ_def_t      *typdef;
    ncx_btype_t     btyp;
    status_t        res;
    
#ifdef DEBUG
    if (!ps || !parm) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* create a new parm and fill it in */
    new_parm = ps_new_parm();
    if (!new_parm) {
        return ERR_INTERNAL_MEM;
    }
    ps_setup_parm(new_parm, ps, parm);

    /* get the base type value */
    typ = (typ_template_t *)parm->pdef;
    typdef = &typ->typdef;
    btyp = typ_get_basetype(typdef);

    if (typ_is_simple(btyp)) {
	res = val_simval_ok(typdef, strval);
	if (res == NO_ERR) {
	    if (script) {
		(void)var_get_script_val(typdef,
					 new_parm->val,
					 psd_get_parm_nsid(parm),
					 parm->name,
					 strval,
					 ISPARM,
					 &res);
	    } else {
		res = val_set_simval(new_parm->val,
				     typdef,  
				     psd_get_parm_nsid(parm),
				     parm->name,
				     strval);
	    }
	}
    } else {
	res = parse_complex_parm(parm, strval, new_parm, script);
    }

    if (res != NO_ERR) {
	if (new_parm) {
	    ps_free_parm(new_parm);
	}
	return res;
    } else {
	ps_add_parm(new_parm, ps, NCX_MERGE_FIRST);
	return NO_ERR;
    }
    /*NOTREACHED*/

}  /* parse_parm */


/********************************************************************
* FUNCTION check_cli_block_err
* 
* Check for errors in the choice block
* Generate RPC errors as needed if errQ non-NULL
*
* INPUTS:
*   setval == TRUE if block supposed to be set
*       check for any missing parameters
*          == FALSE if block supposed to be not set
*       check for any set parameters
*   ps == parmset to check
*   block == block to check

* RETURNS:
*   status
*********************************************************************/
static status_t
    check_cli_block_err (boolean setval,
			 const ps_parmset_t *ps,
			 const psd_block_t  *block)
{
    const psd_parm_t *parm;

    /* check all the parms in this block for missing or extra error */
    for (parm = (const psd_parm_t *)dlq_firstEntry(&block->blockQ);
	 parm != NULL;
	 parm = (const psd_parm_t *)dlq_nextEntry(parm)) {
	if (setval) {
	    /* check for missing parm */
	    if (psd_parm_required(parm) && 
		!ps_parmnum_set(ps, parm->parm_id)) {
		/* parm is supposed to be set but is not */
		return ERR_NCX_MISSING_PARM;
	    }
	} else {
	    /* check for extra parm */
	    if (ps_parmnum_set(ps, parm->parm_id)) {
		/* parm is supposed to be missing but it is set */
		return ERR_NCX_EXTRA_CHOICE;
	    }
	}
    }

    return NO_ERR;
    
}  /* check_cli_block_err */


/********************************************************************
* FUNCTION check_insert_default
* 
* Create a ps_parm_t struct if the specified parm type
* has a default value, and insert it into the parmset
*
* Defvals are allowed for simple types only at this time.
*
* If the parameter is defined within a choice, then no default
* will be added, since no default choice (within a PSD or parmset)
* is supported at this time.
*
* INPUTS:
*   ps == ps_parmset_t to adjust
*   parm == psd_parm_t descriptor for the missing parm
*
* OUTPUTS:
*   If the specified parm is mandatory w/defval defined, then a 
*   new ps_parm_t will be inserted in the ps->parmQ as required
*   to fill in the parmset.
*
* RETURNS:
*   status -- only internal or malloc errors can occur. 
*   SET_ERROR() will be called if any internal errors are encountered
*********************************************************************/
static status_t
    check_insert_default (ps_parmset_t *ps,
			  psd_parm_t *parm)
{
    typ_template_t *typ;
    const xmlChar  *defval;

    /* check if this is mandatory first */
    if (parm->usage != PSD_UT_MANDATORY) {
	return NO_ERR;
    }

    /* get the default value */
    typ = (typ_template_t *)parm->pdef;
    defval = typ_get_defval(typ);
    if (!defval) {
	return NO_ERR;
    }

    return parse_parm(ps, parm, defval, FALSE);

}  /* check_insert_default */


/********************************************************************
* FUNCTION cli_choice_check
* 
* Check a ps_parmset_t struct against its expected PSD 
* for instance validation:
*
*    - choice validation: 
*      only one member (or block) allowed if the data type is choice
*      Only issue errors based on the instance test qualifiers
*
*    - instance qualifiers: 
*      FULL: validate expected choices for any nested parmsets
*      TOP:  validate expected choices for the top level only
*      PARTIAL: only check for too many choice members. Do not check
*            for missing choice members
*
* The input is checked against the specified PSD.
*
* Any generated <rpc-error> elements will be added to the errQ
*
* INPUTS:
*   ps == ps_parmset_t to check
*
* OUTPUTS:
*   print to STDOUT if any errors encountered
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    cli_choice_check (const ps_parmset_t *ps)
{
    const psd_hdronly_t  *psd_hdr, *choice_hdr;
    const psd_choice_t   *psd_choice;
    const psd_block_t    *psd_block;
    const psd_parm_t     *psd_parm, *set_parm;
    status_t              res, retres;
    boolean               parmreq, choicereq;

    /* check if there is any work to do at all */
    if (!ps->psd->choicecnt) {
	return NO_ERR;
    }

    retres = NO_ERR;

    /* Go through all the entries in the PSD parmQ and check
     * the choices against the parmset to see if each 
     * choice parm or choice block is present in the correct 
     * number of instances.
     *
     * The parmQ contains PSD_NT_PARM and PSD_NT_CHOICE entries
     */
    for (psd_hdr = (const psd_hdronly_t *)dlq_firstEntry(&ps->psd->parmQ);
	 psd_hdr != NULL;
	 psd_hdr = (const psd_hdronly_t *)dlq_nextEntry(psd_hdr)) {

	if (psd_hdr->ntyp != PSD_NT_CHOICE) {
	    /* skip PSD_NT_PARM entry */
	    continue;
	}

	psd_choice = (const psd_choice_t *)psd_hdr;
	set_parm = NULL;
	choicereq = TRUE;

	/* go through all the entries in this choice and validate them 
	 * The choiceQ contains PSD_NT_BLOCK or PSD_NT_PARM entries
	 */
	for (choice_hdr = (const psd_hdronly_t *)
		 dlq_firstEntry(&psd_choice->choiceQ);
	     choice_hdr != NULL;
	     choice_hdr = (const psd_hdronly_t *)
		 dlq_nextEntry(choice_hdr)) {
	    if (choice_hdr->ntyp==PSD_NT_BLOCK) {
		/* this entire block should be present or absent
		 * unless the parameter is optional
		 */
		psd_block = (const psd_block_t *)choice_hdr;

		if (ps_check_block_set(ps, psd_block)) {
		    /* at least 1 parm in the block is set */
		    if (set_parm) {
			/* this is an error for all the parms set */
			res = check_cli_block_err(FALSE, ps, psd_block);
			if (res != NO_ERR) {
			    retres = res;
			}
		    } else {
			/* this becomes the selected choice member */
			set_parm = (const psd_parm_t *)
			    dlq_firstEntry(&psd_block->blockQ);

			/* check for any parms that should be set 
			 * but are not
			 */
			res = check_cli_block_err(TRUE, ps, psd_block);
			if (res != NO_ERR) {
			    retres = res;
			}
		    }
		} else {
		    /* nothing in this block was set */
		    continue;
		}
	    } else {
		/* this choice member is a simple parm */
		psd_parm = (const psd_parm_t *)choice_hdr;

		/* if any choice member is optional, then
		 * the entire choice is optional
		 */
		parmreq = psd_parm_required(psd_parm);
		if (!parmreq) {
		    choicereq = FALSE;
		}

		/* check too many choices set error */
		if (ps_parmnum_set(ps, psd_parm->parm_id)) {
		    /* this parm is set */
		    if (set_parm) {
			/* choice already set elsewhere */
			retres = ERR_NCX_EXTRA_CHOICE;

			log_error("\nError: "
			       "extra parameter '%s' selected "
				  "in a choice", psd_parm->name);

		    } else {
			/* make this the selected choice */
			set_parm = psd_parm;
		    }
		} 
	    }
	}

	/* check for 'no choice made' error
	 * Do not generate error if any choice members are optional
	 */
	if (retres == NO_ERR) {
	    if (choicereq && !set_parm) {
		retres = ERR_NCX_MISSING_CHOICE;
		log_error("\nps_parse: Error: missing parameter (%s) "
			  "within a choice", psd_parm->name);
	    }
	}
    }  /* end loop per parm */

    return retres;

}  /* cli_choice_check */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION ps_parse_cli
* 
* Generate 1 ps_parmset_t struct from a Unix Command Line,
* which should conform to the specified psd_template_t definition.
* For CLI interfaces, only one parmset can be specified.
*
* CLI Syntax Supported
*
* [prefix] parmname [separator] [value]
*
*    prefix ==  0 to 2 dashes    foo  -foo --foo
*    parmname == any valid NCX identifier string
*    separator ==  - equals char (=)
*                  - whitespace (sp, ht)
*    value == any NCX number or NCX string
*          == 'enum' data type
*          == not present (only for 'flag' data type)
*          == extended
*    This value: string will converted to appropriate 
*    simple type in val_value_t format.
*
* The format "--foo=bar" must be processed within one argv segment.
* If the separator is a whitespace char, then the next string
* in the argv array must contain the expected value.
* 
*
* DESIGN NOTES:
*
*   1) parse the (argc, argv) input against the specified PSD
*   2) add any missing mandatory and condition-met parameters
*      to the parmset. Defaults within a choice group are not
*      checked during this phase (unless valonly is TRUE)
*   3) Set the instance sequence IDs to distinguish duplicates
*   4) Validate any 'choice' constructs within the parmset
*   5) Validate the proper number of instances (missing or extra)
*      (unless valonly is TRUE)
*
* The 'value' string cannot be split across multiple argv segments.
* Use quotation chars within the CLI shell to pass a string
* containing whitespace to the CLI parser:
*
*   --foo="quoted string if whitespace needed"
*   --foo="quoted string if setting a variable \
*         as a top-level assignment"
*   --foo=unquoted-string-without-whitespace
*
* The input is parsed against the specified PSD.
* A ps_parmset_t tree is built as the input is read.
* Each parameter is syntax checked as is is parsed.
* This function will attempt to parse the input as fully as
* possible to generate rpc_err_rec_t structs for all identified errors.
* If possible, the parser will skip to next parmameter in the parmset,
* in order to support 'continue-on-error' type of operations.
*
* Any agent callback functions that might be registered for
* the specified parmset (or its sub-nodes) are not called
* by this function.  This must be done after this function
* has been called, and returns NO_ERR.
* 
* !!! TO-DO !!!
*  - There are no 1-char aliases for NCX parameters.
*  - The PSD 'order' parameter is not checked during CLI parsing
*  - Only 'loose' parameter order is supported.
*  - Position-dependent, unnamed parameters are not supported
*    at this time.  This bad API practice is discouraged anyway.
*
* INPUTS:
*   argc == number of strings passed in 'argv'
*   argv == array of command line argument strings
*   psd == psd_template_t that should be used to validate the input
*   valonly == TRUE if only the values presented should
*             be checked, no defaults, missing parms (Step 1&3 only)
*           == FALSE if all the tests and procedures should be done
*   autocomp == TRUE if parameter auto-completion should be
*               tried if any specified parameters are not matches
*               for the specified parmset
*            == FALSE if exact match only is desired
*   status == pointer to status_t to get the return value
*
* OUTPUTS:
*   *status == the final function return status

* Just as the NETCONF parser does, the CLI parser will not add a parameter
* to the ps_parmset if any errors occur related to the initial parsing.
* Since CHOICE and Instance Qualifer validation is done after initial
* parsing.
*
* RETURNS:
*   pointer to the malloced and filled in ps_parmset_t
*********************************************************************/
ps_parmset_t *
    ps_parse_cli (int argc, 
		  const char *argv[],
		  const psd_template_t *psd,
		  boolean valonly,
		  boolean script,
		  boolean autocomp,
		  status_t  *status)
{
    ps_parmset_t   *ps;
    psd_parm_t     *psd_parm;
    typ_template_t *psd_type;
    const char     *msg;
    char           *parmname, *parmval, *str, *buff;
    int32           parmnum;
    uint32          parmnamelen, buffpos, bufflen, parmid;
    ncx_iqual_t     iqual;
    ncx_btype_t     btyp;
    status_t        res, retres;
    xmlChar         errbuff[NCX_MAX_NLEN];

#ifdef DEBUG
    if (!status) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (!argv || !psd) {
	*status = SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* check if there are any parameters at all to parse */
    if (argc < 2) {
	*status = NO_ERR;
	return NULL;
    }

    /* allocate a new parmset */
    ps = ps_new_parmset();
    if (!psd) {
	*status = ERR_INTERNAL_MEM;
	return NULL;
    }

    if (script) {
	/* set the parmset name to the application pathname */
	ps->name = xml_strdup((const xmlChar *)argv[0]);
    } else {
	/* set the parmset name to the PSD static name */
	ps->name = xml_strdup(psd->name);
    }

    /* check for a malloc error */
    if (!ps->name) {
	*status = ERR_INTERNAL_MEM;
	ps_free_parmset(ps);
	return NULL;
    }

    /* set the parmset type to RPC; this hack is done to
     * prevent making the caller do this same test
     * CLI does not have its own parser mode, but
     * the RPC PSD type matches the flat structure
     * of the CLI data input
     */
    res = ps_setup_parmset(ps, psd, PSD_TYP_RPC);
    if (res != NO_ERR) {
	/* non-recoverable error */
	*status = res;
	ps_free_parmset(ps);
	return NULL;
    }

    /* gather all the argv strings into one buffer for easier parsing
     * need to copy because the buffer is written during parsing
     * and the argv parameter is always a 'const char **' data type
     *
     * first determine the buffer length
     */
    bufflen = 0;
    for (parmnum=1; parmnum < argc; parmnum++) {
	bufflen += (uint32)(strlen(argv[parmnum]) + 1);
    }

    /* adjust extra space */
    if (bufflen) {
	bufflen--;
    }

    buff = m__getMem(bufflen+1);
    if (!buff) {
	/* non-recoverable error */
	*status = ERR_INTERNAL_MEM;
	ps_free_parmset(ps);
	return NULL;
    }

    /* copy the argv strings into the buffer */
    buffpos = 0;
    for (parmnum=1; parmnum < argc; parmnum++) {
	buffpos += xml_strcpy((xmlChar *)&buff[buffpos], 
			      (const xmlChar *)argv[parmnum]);
	if (parmnum+1 < argc) {
	    buff[buffpos++] = ' ';
	}
    }
    buff[buffpos] = 0;

    /* setup parm loop */
    buffpos = 0;

    /* 1) go through all the command line strings
     * setup parmname and parmval based on strings found
     * and the PSD for these parameters
     * save each parm value in a ps_parm_t struct
     */
    while (buffpos < bufflen) {

	res = NO_ERR;

	/* first skip starting whitespace */
	while (buff[buffpos] && isspace(buff[buffpos])) {
	    buffpos++;
	}

	/* check start of parameter name conventions 
	 * allow zero, one, or two dashes before parmname
         *          foo   -foo   --foo
	 */
	if (!buff[buffpos]) {
	    res = ERR_NCX_WRONG_LEN;
	} else if (buff[buffpos] == NCX_CLI_START_CH) {
	    if (!buff[buffpos+1]) {
		res = ERR_NCX_WRONG_LEN;
	    } else if (buff[buffpos+1] == NCX_CLI_START_CH) {
		if (!buff[buffpos+2]) {
		    res = ERR_NCX_WRONG_LEN;
		} else {
		    buffpos += 2;  /* skip past 2 dashes */
		}
	    } else {
		buffpos++;    /* skip past 1 dash */
	    }
	} /* else no dashes, leave parmname pointer alone */

	/* check for the end of the parm name, wsp or equal sign
	 * get the parm template, and get ready to parse it
	 */
	if (res == NO_ERR) {
	    /* check the parmname string for a terminating char */
	    parmname = &buff[buffpos];
	    str = &parmname[1];
	    while (*str && !isspace(*str) && *str != NCX_ASSIGN_CH) {
		str++;
	    }

	    parmnamelen = (uint32)(str - parmname);
	    buffpos += parmnamelen;

	    /* check if this parameter name is in the parmset def */
	    psd_parm = psd_find_parm_str(psd, 
					 (const xmlChar *)parmname,
					 parmnamelen);

	    /* check if parm was found, try partial name if not */
	    if (!psd_parm && autocomp) {
		psd_parm = 
		    psd_match_parm_str(psd, 
				       (const xmlChar *)parmname,
				       parmnamelen);
	    }
	    
	    if (!psd_parm) {
		res = ERR_NCX_UNKNOWN_PARM;
	    } else {
		/* do not check parameter order for CLI */
		ps->curparm = psd_parm->parm_id;
		psd_type = (typ_template_t *)psd_parm->pdef;
		btyp = typ_get_basetype(&psd_type->typdef);
		parmval = NULL;

		/* check if ended on space of EOLN */
		if (btyp == NCX_BT_EMPTY) {
		    buffpos++;   /* skip past WSP or equal or EOLN */
		} else if (buffpos < bufflen) {
		    buffpos++;   /* skip past WSP or equal */
		    /* value expected, so zero-terminate the string
		     * that represents the parameter value
		     */

		    /* skip any whitespace */
		    while (buff[buffpos] && isspace(buff[buffpos])) {
			buffpos++;
		    }

		    /* if any chars left in buffer, get the parmval */
		    if (buffpos < bufflen) {
			if (buff[buffpos] == NCX_QUOTE_CH) {
			    if (script) {
				/* set the start at quote */
				parmval = &buff[buffpos];
			    } else {
				/* set the start after quote */
				parmval = &buff[++buffpos];
			    }

			    /* find the end of the quoted string */
			    str = &parmval[1];
			    while (*str && *str != NCX_QUOTE_CH) {
				str++;
			    }
			    /* if script mode keep ending quote */
			    if (script && *str == NCX_QUOTE_CH) {
				str++;
			    }
			} else if (script && (buffpos+1 < bufflen) &&
				   (buff[buffpos] == NCX_XML1a_CH) &&
				   (buff[buffpos+1] == NCX_XML1b_CH)) {
			    /* set the start of the XML parmval to the [ */
			    parmval = &buff[buffpos];
			    
			    /* find the end of the inline XML */
			    str = parmval+1;
			    while (*str && !((*str==NCX_XML2a_CH) 
					     && (str[1]==NCX_XML2b_CH))) {
				str++;
			    }

			    if (!*str) {
				/* did not find end of XML string */
				res = ERR_NCX_DATA_MISSING;
			    } else {
				/* setup after the ] char to be zeroed */
				str += 2;
			    }
			} else {
			    /* set the start of the parmval */
			    parmval = &buff[buffpos];

			    /* find the end of the unquoted string */
			    str = &parmval[1];
			    while (*str && !isspace(*str)) {
				str++;
			    }
			}

			/* terminate string */
			*str = 0;

			/* skip buffpos past eo-string */
			buffpos += (uint32)((str - parmval) + 1);  
		    }
		}

		/* make sure value entered if expected */
		if (res==NO_ERR && !parmval && btyp != NCX_BT_EMPTY) {
		    res = ERR_NCX_EMPTY_VAL;
		}
	    }
        }

	/* create a new ps_parm struct and set the value */
	if (res == NO_ERR) {
	    res = parse_parm(ps, psd_parm, (const xmlChar *)parmval, script);
	}

	/* check any errors in the parm name or value */
	if (res != NO_ERR) {
	    msg = get_error_string(res);
	    xml_strncpy(errbuff, (const xmlChar *)parmname, parmnamelen);
	    switch (res) {
	    case ERR_NCX_UNKNOWN_PARM:
		log_error("\nError: Unknown parameter (%s)", errbuff);
		break;
	    default:
		if (buffpos < bufflen) {
		    log_error("\nError: %s (%s = %s)", msg, errbuff, &buff[buffpos]);
		} else if (parmval) {
		    log_error("\nError: %s (%s = %s)", msg, errbuff, parmval);
		} else {
		    log_error("\nError: %s (%s)", msg, errbuff);
		}
	    }
	    m__free(buff);
	    *status = res;
	    return ps;
	}
    }

    /* cleanup after loop */
    m__free(buff);
    buff = NULL;
    res = NO_ERR;

    /* 2) add any defaults for mandatory parms that are not set */
    if (!valonly) {
	res = ps_parse_add_defaults(ps);
    }

    /* 3) go through each parm node and set its name-specific 
     * sequence number
     * this is done afterwards to support loose paramter order
     * Done even if some parms had errors!
     */
    ps_set_instseq(ps);

    if (valonly) {
	*status = res;
	return ps;
    }

    /* 4) choice check must be before instance check */
    if (res == NO_ERR) {
	res = cli_choice_check(ps);
    }

    if (res != NO_ERR) {
	*status = res;
	return ps;
    }

    /* 5) CLI Instance Check
     * Go through all the parameters in the PSD and check
     * the parmset to see if each parm
     * is present the correct number of instances
     * The PSD node can be a choice or a simple parameter 
     */
    retres = NO_ERR;
    for (parmid = 1; parmid <= ps->psd->parmcnt; parmid++) {
	res = NO_ERR;

	/* get the parameter descriptor */
	psd_parm = psd_find_parmnum(ps->psd, parmid);
	if (!psd_parm) {
	    *status = SET_ERROR(ERR_INTERNAL_PTR);
	    return ps;
	}

	/* check for missing parameter if it is not in a choice */
	if (!psd_parm->choice_id) {
	    if (psd_parm_required(psd_parm)) {
		if (!ps_parmnum_seterr(ps, parmid)) {
		    /* param is not set */
		    res = ERR_NCX_MISSING_PARM;

		    log_error("\nps_parse Error: missing parameter (%s)", 
			   psd_parm->name);

		    retres = res;
		}
	    }
	}

	/* check too many instances of the parameter */
	if (res == NO_ERR) {
	    psd_type = (typ_template_t *)psd_parm->pdef;
	    iqual = typ_get_iqualval(psd_type);
	    if ((iqual==NCX_IQUAL_ONE || iqual==NCX_IQUAL_OPT) &&
		ps_parmnum_mset(ps, parmid)) {
		/* max 1 allowed, but more than 1 set 
		 * generate an error 
		 */
		res = ERR_NCX_EXTRA_PARMINST;

		log_error("\nError:  extra parameter instance for '%s'", 
		       psd_parm->name);
	    }
	}

	/* record any errors in the instance checking */
	if (res != NO_ERR) {
	    retres = res;
	}
    }

    *status = retres;
    return ps;

}  /* ps_parse_cli */


/********************************************************************
* FUNCTION ps_parse_cli_parm
* 
* Create a ps_parm_t struct for the specified parm value,
* and insert it into the parmset
*
* ONLY CALLED FROM CLI PARSING FUNCTIONS IN ncxcli.c
* ALLOWS SCRIPT EXTENSIONS TO BE PRESENT
*
* INPUTS:
*   ps == ps_parmset_t to adjust
*   parm == psd_parm_t descriptor for the missing parm
*   strval == string representation of the parm value
*             (may be NULL if parm btype is NCX_BT_EMPTY
*   script == TRUE if CLI script mode
*          == FALSE if CLI plain mode
* 
* OUTPUTS:
*   If the specified parm is mandatory w/defval defined, then a 
*   new ps_parm_t will be inserted in the ps->parmQ as required
*   to fill in the parmset.
*
* RETURNS:
*   status 
*********************************************************************/
status_t
    ps_parse_cli_parm (ps_parmset_t *ps,
		       psd_parm_t *parm,
		       const xmlChar *strval,
		       boolean script)
{
    return parse_parm(ps, parm, strval, script);

}  /* ps_parse_cli_parm */


/********************************************************************
* FUNCTION ps_parse_add_defaults
* 
* Check a ps_parmset_t struct against its expected PSD 
* and add any default values for mandatory parameters
* which have defvals, but are missing in the parmset.
*
* Defvals are allowed for simple types only at this time.
*
* If a parameter is encountered that itself is a parmset, then
* this function will be called recursively, if the toponly
* parameter is FALSE.
*
* INPUTS:
*   ps == ps_parmset_t to adjust
*
* OUTPUTS:
*   If missing mandatory params w/defval are found, then new
*   ps_parm_t will be inserted in the ps->parmQ as required
*   to fill in the parmset.
*
* RETURNS:
*   status -- only internal or malloc errors can occur. 
*   SET_ERROR() will be called if any internal errors are encountered
*********************************************************************/
status_t
    ps_parse_add_defaults (ps_parmset_t *ps)
{
    uint32              i;
    psd_parm_t         *psd_parm;
    const psd_block_t  *psd_block;
    status_t            res;
    
#ifdef DEBUG
    if (!ps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check each parm flag to see if that parm has been set */
    for (i=1; i <= ps->psd->parmcnt; i++) {
	if (!ps_parmnum_seterr(ps, i)) {
	    /* this parm was not set, check if it needs to be */
	    psd_parm = psd_find_parmnum(ps->psd, i);
	    if (!psd_parm) {
		return SET_ERROR(ERR_INTERNAL_PTR);
	    }

	    /* check if this param is part of a choice */
	    if (psd_parm->choice_id) {
		/* only set the default if this is part of a block
		 * and other parms in the block are already set
		 */
		if (!psd_parm->block_id) {
		    continue;
		}

		/* this is parm within a choice block
		 * only set it if others in the block are already set
		 */
		psd_block = psd_find_blocknum(ps->psd,
			psd_parm->choice_id, psd_parm->block_id);
		if (!psd_block) {
		    return SET_ERROR(ERR_INTERNAL_PTR);
		}
		if (!ps_check_block_set(ps, psd_block)) {
		    continue;
		} /* else set the default for this parm */
	    } /* else parm is not in a choice */

	    /* need to check this parm to see if it has a default */
	    res = check_insert_default(ps, psd_parm);
	    if (res != NO_ERR) {
		return res;
	    }
	}
    }
    
    return NO_ERR;

}  /* ps_parse_add_defaults */


/********************************************************************
* FUNCTION ps_parse_add_clone
* 
* Create a clone of the specified ps_parm_t struct 
* and insert it into the specified parmset
*
* INPUTS:
*   ps == ps_parmset_t to adjust
*   curparm == current ps_parm_t value to clone
*
* OUTPUTS:
*   A clone of the curparm struct will be made and
*   inserted into 'ps'
*
* RETURNS:
*   status 
*********************************************************************/
status_t
    ps_parse_add_clone (ps_parmset_t *ps,
			const ps_parm_t *curparm)
{
    ps_parm_t      *newparm, *ps_parm;
    
#ifdef DEBUG
    if (!ps || !curparm) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif
    newparm = ps_clone_parm(curparm);
    if (!newparm) {
	return ERR_INTERNAL_MEM;
    }

    ps_setup_parm(newparm, ps, curparm->parm);
    ps_mark_pflag_set(ps, newparm->parm->parm_id);

    /* need to find the right place to insert the value */
    for (ps_parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 ps_parm != NULL;
	 ps_parm = (ps_parm_t *)dlq_nextEntry(ps_parm)) {
	if (ps_parm->parm->parm_id > newparm->parm->parm_id) {
	    /* insert the new parm before this higher numbered parm */
	    dlq_insertAhead(newparm, ps_parm);
	    return NO_ERR;
	}
    }

    /* if we get here then new last entry is needed */
    dlq_enque(newparm, &ps->parmQ);
    return NO_ERR;

}  /* ps_parse_add_clone */


/* END file ps_parse.c */
