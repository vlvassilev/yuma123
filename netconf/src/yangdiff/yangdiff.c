/*  FILE: yangdiff.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
10-jun-08    abb      begun; used yangdump.c as a template

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_conf
#include  "conf.h"
#endif

#ifndef _H_def_reg
#include  "def_reg.h"
#endif

#ifndef _H_ext
#include  "ext.h"
#endif

#ifndef _H_help
#include  "help.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include  "ncxmod.h"
#endif

#ifndef _H_ps
#include  "ps.h"
#endif

#ifndef _H_ps_parse
#include  "ps_parse.h"
#endif

#ifndef _H_psd
#include  "psd.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_xml_wr
#include  "xml_wr.h"
#endif

#ifndef _H_yang
#include  "yang.h"
#endif

#ifndef _H_yangconst
#include  "yangconst.h"
#endif

#ifndef _H_yangdiff
#include  "yangdiff.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define YANGDIFF_DEBUG   1

/* this should match the buffer size in ncx/tk.h */
#define YANGDIFF_BUFFSIZE           0xffff

#define YANGDIFF_DIFFTYPE_TERSE    (const xmlChar *)"terse"
#define YANGDIFF_DIFFTYPE_BRIEF    (const xmlChar *)"brief"
#define YANGDIFF_DIFFTYPE_NORMAL   (const xmlChar *)"normal"
#define YANGDIFF_DIFFTYPE_REVISION (const xmlChar *)"revision"

#define YANGDIFF_DEF_OUTPUT        (const xmlChar *)"stdout"
#define YANGDIFF_DEF_DIFFTYPE      (const xmlChar *)"summary"
#define YANGDIFF_DEF_DT            YANGDIFF_DT_NORMAL
#define YANGDIFF_DEF_CONFIG        (const xmlChar *)"/etc/yangdiff.conf"
#define YANGDIFF_DEF_FILENAME      (const xmlChar *)"yangdiff.log"

#define YANGDIFF_MOD               (const xmlChar *)"yangdiff"
#define YANGDIFF_PARMSET           (const xmlChar *)"yangdiff"

#define YANGDIFF_PARM_CONFIG        (const xmlChar *)"config"
#define YANGDIFF_PARM_OLD           (const xmlChar *)"old"
#define YANGDIFF_PARM_NEW           (const xmlChar *)"new"
#define YANGDIFF_PARM_DIFFTYPE      (const xmlChar *)"difftype"
#define YANGDIFF_PARM_OUTPUT        (const xmlChar *)"output"
#define YANGDIFF_PARM_INDENT        (const xmlChar *)"indent"
#define YANGDIFF_PARM_NO_SUBDIRS    (const xmlChar *)"no-subdirs"
#define YANGDIFF_PARM_NO_HEADER     (const xmlChar *)"no-header"
#define YANGDIFF_LINE (const xmlChar *)\
"\n==================================================================="

/* output_line isnew parameter */
#define ISOLD  FALSE
#define ISNEW  TRUE


#define TO_STR (const xmlChar *)"to "
#define A_STR (const xmlChar *)"A "
#define D_STR (const xmlChar *)"D "
#define M_STR (const xmlChar *)"M "
#define ADD_STR (const xmlChar *)"- Added "
#define DEL_STR (const xmlChar *)"- Removed "
#define MOD_STR (const xmlChar *)"- Changed "


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static ps_parmset_t          *cli_ps;
static yangdiff_diffparms_t   diffparms;


static uint32
    type_changed (yangdiff_diffparms_t *cp,
		  typ_def_t *oldtypdef,
		  typ_def_t *newtypdef);


/********************************************************************
* FUNCTION pr_err
*
* Print an error message
*
* INPUTS:
*    res == error result
*
*********************************************************************/
static void
    pr_err (status_t  res)
{
    const char *msg;

    msg = get_error_string(res);
    log_error("\n%s: Error exit (%s)\n", YANGDIFF_PROGNAME, msg);

} /* pr_err */


/********************************************************************
* FUNCTION pr_usage
*
* Print a usage message
*
*********************************************************************/
static void
    pr_usage (void)
{
    log_error("\nError: No parameters entered."
	      "\nTry '%s --help' for usage details\n",
	      YANGDIFF_PROGNAME);

} /* pr_usage */


/********************************************************************
* FUNCTION indent_in
*
* Move curindent to the right
*
* INPUTS:
*   cp == compare paraeters to use
*
*********************************************************************/
static void
    indent_in (yangdiff_diffparms_t *cp)
{
    if (cp->indent) {
	cp->curindent += cp->indent;
	ses_set_indent(cp->scb, cp->curindent);
    }
    
} /* indent_in */


/********************************************************************
* FUNCTION indent_out
*
* Move curindent to the left
*
* INPUTS:
*   cp == compare paraeters to use
*
*********************************************************************/
static void
    indent_out (yangdiff_diffparms_t *cp)
{
    if (cp->indent) {
	if (cp->curindent >= cp->indent) {
	    cp->curindent -= cp->indent;
	} else {
	    cp->curindent = 0;
	}
	ses_set_indent(cp->scb, cp->curindent);
    }
    
} /* indent_out */


/********************************************************************
* FUNCTION str_field_changed
*
* Check if the string field changed at all
*
* INPUTS:
*   fieldname == fieldname to use (may be NULL)
*   oldstr == old string to check (may be NULL)
*   newstr == new string to check (may be NULL)
*   isrev == TRUE if revision mode, FALSE otherwise
*   cdb == pointer to change description block to fill in (may be NULL)
*
* OUTPUTS:
*   *cdb is filled in, if cdb is non-NULL
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*   
*********************************************************************/
static uint32
    str_field_changed (const xmlChar *fieldname,
		       const xmlChar *oldstr,
		       const xmlChar *newstr,
		       boolean isrev,
		       yangdiff_cdb_t *cdb)
{
    uint32  ret;

    if (cdb) {
	cdb->fieldname = fieldname;
	cdb->oldval = oldstr;
	cdb->newval = newstr;
    }

    if (!oldstr && newstr) {
	ret = 1;
	if (cdb) {
	    cdb->changed = TRUE;
	    cdb->useval = newstr;
	    cdb->chtyp = (isrev) ? ADD_STR : A_STR;
	}
    } else if (oldstr && !newstr) {
	ret = 1;
	if (cdb) {
	    cdb->changed = TRUE;
	    cdb->useval = oldstr;
	    cdb->chtyp = (isrev) ? DEL_STR : D_STR;
	}
    } else if (oldstr && newstr &&
	       xml_strcmp(oldstr, newstr)) {
	ret = 1;
	if (cdb) {
	    cdb->changed = TRUE;
	    cdb->useval = newstr;
	    cdb->chtyp = (isrev) ? MOD_STR : M_STR;
	}
    } else {
	ret = 0;
	if (cdb) {
	    cdb->changed = FALSE;
	    cdb->useval = NULL;
	    cdb->chtyp = NULL;
	}
    }
    return ret;

} /* str_field_changed */


/********************************************************************
* FUNCTION bool_field_changed
*
* Check if the boolean field changed at all
*
* INPUTS:
*   oldbool == old boolean to check
*   newbool == new boolean to check
*   isrev == TRUE if revision mode, FALSE otherwise
*   cdb == pointer to change description block to fill in
*
* OUTPUTS:
*   *cdb is filled in
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*   
*********************************************************************/
static uint32
    bool_field_changed (const xmlChar *fieldname,
		       boolean oldbool,
		       boolean newbool,
		       boolean isrev,
		       yangdiff_cdb_t *cdb)
{
    cdb->fieldname = fieldname;
    cdb->oldval = oldbool ? NCX_EL_TRUE : NCX_EL_FALSE;
    cdb->newval = newbool ? NCX_EL_TRUE : NCX_EL_FALSE;

    if (oldbool != newbool) {
	cdb->changed = TRUE;
	cdb->useval = cdb->newval;
	cdb->chtyp = (isrev) ? MOD_STR : M_STR;
    } else {
	cdb->changed = FALSE;
	cdb->useval = NULL;
	cdb->chtyp = NULL;
    }
    return cdb->changed ? 1 : 0;

} /* bool_field_changed */


/********************************************************************
* FUNCTION status_field_changed
*
* Check if the status field changed at all
*
* INPUTS:
*   oldstat == old status to check
*   newstat == new status to check
*   isrev == TRUE if revision mode, FALSE otherwise
*   cdb == pointer to change description block to fill in
*
* OUTPUTS:
*   *cdb is filled in
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*********************************************************************/
static uint32
    status_field_changed (const xmlChar *fieldname,
			  ncx_status_t oldstat,
			  ncx_status_t newstat,
			  boolean isrev,
			  yangdiff_cdb_t *cdb)
{
    cdb->fieldname = fieldname;
    cdb->oldval = ncx_get_status_string(oldstat);
    cdb->newval = ncx_get_status_string(newstat);

    if (oldstat != newstat) {
	cdb->changed = TRUE;
	cdb->useval = cdb->newval;
	cdb->chtyp = (isrev) ? MOD_STR : M_STR;
    } else {
	cdb->changed = FALSE;
	cdb->useval = NULL;
	cdb->chtyp = NULL;
    }
    return cdb->changed ? 1 : 0;

} /* status_field_changed */


/********************************************************************
* FUNCTION prefix_field_changed
*
* Check if the status field changed at all
*
* INPUTS:
*   oldmod == old module to check
*   newmod == new module to check
*   oldprefix == old prefix to check
*   newprefix == new prefix to check
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*********************************************************************/
static uint32
    prefix_field_changed (const ncx_module_t *oldmod,
			  const ncx_module_t *newmod,
			  const xmlChar *oldprefix,
			  const xmlChar *newprefix)
{
    const ncx_import_t *oldimp, *newimp;

    if (str_field_changed(NULL, oldprefix, 
			  newprefix, FALSE, NULL)) {
	if (oldprefix && newprefix) {
	    oldimp = ncx_find_pre_import(oldmod, oldprefix);
	    newimp = ncx_find_pre_import(newmod, newprefix);
	    if (oldimp && newimp &&
		!xml_strcmp(oldimp->module, newimp->module)) {
		return 0;
	    } else {
		return 1;
	    }
	} else {
	    return 1;
	}
    } else {
	return 0;
    }
    /*NOTREACHED*/

} /* prefix_field_changed */


/********************************************************************
 * FUNCTION output_line
 * 
 *  Output one line for a comparison
 *
 * INPUTS:
 *    cp == parameter block to use
 *    isnew == TRUE if the 'new' line 
 *             FALSE if this is the 'old' line
 *    fieldname == fieldname that is different to print (may be NULL)
 *    fieldval == string value to print (may be NULL)
 *    isid == TRUE if identifier
 *         == FALSE if a string value
 *
 *********************************************************************/
static void
    output_line (yangdiff_diffparms_t *cp,
		 boolean isnew,
		 const xmlChar *fieldname,
		 const xmlChar *fieldval,
		 boolean isid)
{
    int32 curin;

    curin = cp->curindent;

    if (isnew) {
	ses_putstr_indent(cp->scb, (const xmlChar *)"+  ", curin);
    } else {
	ses_putstr_indent(cp->scb, (const xmlChar *)"-  ", curin);
    }
    if (fieldname) {
	ses_putstr(cp->scb, fieldname);
	if (fieldval) {
	    ses_putchar(cp->scb, ' ');
	    if (!isid) {
		ses_putchar(cp->scb, '\'');
	    }
	    ses_putstr(cp->scb, fieldval);
	    if (!isid) {
		ses_putchar(cp->scb, '\'');
	    }
	} else {
	    ses_putstr(cp->scb, (const xmlChar *)" ''");
	}
    }

}  /* output_line */


/********************************************************************
 * FUNCTION output_cdb_line
 * 
 *  Output one line for a comparison, from a change description block
 *
 * INPUTS:
 *    cp == parameter block to use
 *    cdb == change descriptor block to use
 *********************************************************************/
static void
    output_cdb_line (yangdiff_diffparms_t *cp,
		     yangdiff_cdb_t *cdb)
{
    boolean showval;

    showval = TRUE;

    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
	showval = FALSE;
	/* fall through */
    case YANGDIFF_DT_BRIEF:
    case YANGDIFF_DT_REVISION:
	ses_putstr_indent(cp->scb, cdb->chtyp, cp->curindent);
	ses_putstr(cp->scb, cdb->fieldname);
	if (cdb->useval) {
	    ses_putchar(cp->scb, ' ');
	    if (showval) {
		if (cp->edifftype==YANGDIFF_DT_REVISION &&
		    cdb->oldval && cdb->newval) {
		    ses_putstr(cp->scb, TO_STR);
		}
		ses_putchar(cp->scb, '\'');
		ses_putstr(cp->scb, cdb->useval);
		ses_putchar(cp->scb, '\'');
	    }
	} else if (showval) {
	    ses_putstr(cp->scb, (const xmlChar *)" ''");
	}
	break;
    case YANGDIFF_DT_NORMAL:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* output_cdb_line */


/********************************************************************
 * FUNCTION output_line2
 * 
 *  Output one line for a comparison with a subfield
 *
 * INPUTS:
 *    cp == parameter block to use
 *    isnew == TRUE if the 'new' line 
 *             FALSE if this is the 'old' line
 *    fieldname == fieldname that is different to print (may be NULL)
 *    fieldval == field string value to print (may be NULL)
 *    subfieldname == sub-fieldname that is different to print (may be NULL)
 *    subfieldval == sub-field string value to print (may be NULL)
 *
 *********************************************************************/
static void
    output_line2 (yangdiff_diffparms_t *cp,
		  boolean isnew,
		  const xmlChar *fieldname,
		  const xmlChar *fieldval,
		  const xmlChar *subfieldname,
		  const xmlChar *subfieldval)
{
    int32 curin;

    curin = cp->curindent;

    if (isnew) {
	ses_putstr_indent(cp->scb, (const xmlChar *)"+  ", curin);
    } else {
	ses_putstr_indent(cp->scb, (const xmlChar *)"-  ", curin);
    }
    if (fieldname) {
	ses_putstr(cp->scb, fieldname);
	if (subfieldname) {
	    if (fieldval) {
		ses_putchar(cp->scb, ' ');
		ses_putstr(cp->scb, fieldval);
	    }
	    ses_putchar(cp->scb, '/');
	    ses_putstr(cp->scb, subfieldname); 
	    if (subfieldval) {
		ses_putchar(cp->scb, ' ');
		ses_putchar(cp->scb, '\'');
		ses_putstr(cp->scb, subfieldval);
		ses_putchar(cp->scb, '\'');
	    }
	} else if (fieldval) {
	    ses_putchar(cp->scb, ' ');
	    ses_putchar(cp->scb, '\'');
	    ses_putstr(cp->scb, fieldval);
	    ses_putchar(cp->scb, '\'');
	}
    }

}  /* output_line2 */


/********************************************************************
 * FUNCTION output_diff
 * 
 *  Output one detailed difference
 *
 * INPUTS:
 *    cp == parameter block to use
 *    fieldname == fieldname that is different to print
 *    oldval == string value for old rev (may be NULL)
 *    newval == string value for new rev (may be NULL)
 *    isid == TRUE if value is really an identifier, not just a string
 *
 *********************************************************************/
static void
    output_diff (yangdiff_diffparms_t *cp,
		 const xmlChar *fieldname,
		 const xmlChar *oldval,
		 const xmlChar *newval,
		 boolean isid)
{
    const xmlChar *useval;
    boolean finish, useto;

    finish = TRUE;
    useval = NULL;
    useto = FALSE;

    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
    case YANGDIFF_DT_BRIEF:
	if (!oldval && newval) {
	    ses_putstr_indent(cp->scb, A_STR, cp->curindent);
	    useval = newval;
	} else if (oldval && !newval) {
	    ses_putstr_indent(cp->scb, D_STR, cp->curindent);
	    useval = oldval;
	} else if (oldval && newval) {
	    if (xml_strcmp(oldval, newval)) {
		ses_putstr_indent(cp->scb, M_STR, cp->curindent);
		useval = newval;
	    } else {
		finish = FALSE;
	    }
	} else {
	    finish = FALSE;
	}

	if (finish) {
	    ses_putstr(cp->scb, fieldname);
	    ses_putchar(cp->scb, ' ');
	    if (!isid) {
		ses_putchar(cp->scb, '\'');
	    }
	    ses_putstr(cp->scb, useval);
	    if (!isid) {
		ses_putchar(cp->scb, '\'');
	    }
	}
	break;
    case YANGDIFF_DT_NORMAL:
	if (!oldval && newval) {
	    output_line(cp, ISOLD, NULL, NULL, FALSE);
	    output_line(cp, ISNEW, fieldname, newval, isid);
	    ses_putchar(cp->scb, '\n');
	} else if (oldval && !newval) {
	    output_line(cp, ISOLD, fieldname, oldval, isid);
	    output_line(cp, ISNEW, NULL, NULL, FALSE);
	    ses_putchar(cp->scb, '\n');
	} else if (oldval && newval) {
	    if (xml_strcmp(oldval, newval)) {
		output_line(cp, ISOLD, fieldname, oldval, isid);
		output_line(cp, ISNEW, fieldname, newval, isid);
		ses_putchar(cp->scb, '\n');
	    }
	}
	break;
    case YANGDIFF_DT_REVISION:
	if (!oldval && newval) {
	    ses_putstr_indent(cp->scb, ADD_STR, cp->curindent);
	    useval = newval;
	} else if (oldval && !newval) {
	    ses_putstr_indent(cp->scb, DEL_STR, cp->curindent);
	    useval = oldval;
	} else if (oldval && newval) {
	    if (xml_strcmp(oldval, newval)) {
		ses_putstr_indent(cp->scb, MOD_STR, cp->curindent);
		useval = newval;
		useto = TRUE;
	    } else {
		finish = FALSE;
	    }
	} else {
	    finish = FALSE;
	}
	if (finish) {
	    ses_putstr(cp->scb, fieldname);
	    ses_putchar(cp->scb, ' ');
	    if (useto) {
		ses_putstr(cp->scb, TO_STR);
	    }
	    if (!isid) {
		ses_putchar(cp->scb, '\'');
	    }
	    ses_putstr(cp->scb, useval);
	    if (!isid) {
		ses_putchar(cp->scb, '\'');
	    }
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* output_diff */


/********************************************************************
 * FUNCTION output_diff2
 * 
 *  Output one detailed difference with 1 sub-field
 *
 * INPUTS:
 *    cp == parameter block to use
 *    fieldname == fieldname that is different to print
 *    fieldval == fieldname that is different to print
 *    subfieldname == sub-fieldname that is different to print (may be NULL)
 *    oldval == sub-field string value for old rev (may be NULL)
 *    newval == sub-field string value for new rev (may be NULL)
 *
 *********************************************************************/
static void
    output_diff2 (yangdiff_diffparms_t *cp,
		  const xmlChar *fieldname,
		  const xmlChar *fieldval,
		  const xmlChar *subfieldname,
		  const xmlChar *oldval,
		  const xmlChar *newval)
{
    const xmlChar *useval;
    boolean finish, useto;

    finish = TRUE;
    useval = NULL;
    useto = FALSE;

    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
	if (!oldval && newval) {
	    ses_putstr_indent(cp->scb, A_STR, cp->curindent);
	} else if (oldval && !newval) {
	    ses_putstr_indent(cp->scb, D_STR, cp->curindent);
	} else if (oldval && newval) {
	    if (xml_strcmp(oldval, newval)) {
		ses_putstr_indent(cp->scb, M_STR, cp->curindent);
	    } else {
		finish = FALSE;
	    }
	} else {
	    finish = FALSE;
	}
	if (finish) {
	    ses_putstr(cp->scb, fieldname);
	    if (fieldval) {
		ses_putchar(cp->scb, ' ');
		/* ses_putchar(cp->scb, '\''); */
		ses_putstr(cp->scb, fieldval); 
		/* ses_putchar(cp->scb, '\''); */
	    }
	}
	break;
    case YANGDIFF_DT_BRIEF:
	if (!oldval && newval) {
	    ses_putstr_indent(cp->scb, A_STR, cp->curindent);
	    useval = newval;
	} else if (oldval && !newval) {
	    ses_putstr_indent(cp->scb, D_STR, cp->curindent);
	    useval = oldval;
	} else if (oldval && newval) {
	    if (xml_strcmp(oldval, newval)) {
		ses_putstr_indent(cp->scb, M_STR, cp->curindent);
		useval = newval;
	    } else {
		finish = FALSE;
	    }
	} else {
	    finish = FALSE;
	}

	if (finish) {
	    ses_putstr(cp->scb, fieldname);
	    if (fieldval) {
		ses_putchar(cp->scb, ' ');
		if (!subfieldname) {
		    ses_putchar(cp->scb, '\'');
		}
		ses_putstr(cp->scb, fieldval);
		if (!subfieldname) {
		    ses_putchar(cp->scb, '\'');
		}
	    }
	    if (subfieldname) {
		ses_putchar(cp->scb, '/');
		ses_putstr(cp->scb, subfieldname);
	    }
	    ses_putchar(cp->scb, ' ');
	    ses_putchar(cp->scb, '\'');
	    ses_putstr(cp->scb, useval);
	    ses_putchar(cp->scb, '\'');
	}
	break;
    case YANGDIFF_DT_NORMAL:
	if (!oldval && newval) {
	    output_line(cp, ISOLD, NULL, NULL, FALSE);
	    output_line2(cp, ISNEW, fieldname, fieldval, 
			 subfieldname, newval);
	    ses_putchar(cp->scb, '\n');
	} else if (oldval && !newval) {
	    output_line2(cp, ISOLD, fieldname, fieldval,
			 subfieldname, oldval);
	    output_line(cp, ISNEW, NULL, NULL, FALSE);
	    ses_putchar(cp->scb, '\n');
	} else if (oldval && newval) {
	    if (xml_strcmp(oldval, newval)) {
		output_line2(cp, ISOLD, fieldname, fieldval,
			     subfieldname, oldval);
		output_line2(cp, ISNEW, fieldname, fieldval,
			     subfieldname, newval);
		ses_putchar(cp->scb, '\n');
	    }
	}
	break;
    case YANGDIFF_DT_REVISION:
	if (!oldval && newval) {
	    ses_putstr_indent(cp->scb, ADD_STR, cp->curindent);
	    useval = newval;
	} else if (oldval && !newval) {
	    ses_putstr_indent(cp->scb, DEL_STR, cp->curindent);
	    useval = oldval;
	} else if (oldval && newval) {
	    if (xml_strcmp(oldval, newval)) {
		ses_putstr_indent(cp->scb, MOD_STR, cp->curindent);
		useval = newval;
		useto = TRUE;
	    } else {
		finish = FALSE;
	    }
	} else {
	    finish = FALSE;
	}

	if (finish) {
	    ses_putstr(cp->scb, fieldname);
	    if (fieldval) {
		ses_putchar(cp->scb, ' ');
		if (!subfieldname) {
		    if (useto) {
			ses_putstr(cp->scb, TO_STR);
		    }
		    ses_putchar(cp->scb, '\'');
		}
		ses_putstr(cp->scb, fieldval);
		if (!subfieldname) {
		    ses_putchar(cp->scb, '\'');
		}
	    }
	    if (subfieldname) {
		ses_putchar(cp->scb, '/');
		ses_putstr(cp->scb, subfieldname);
	    }
	    ses_putchar(cp->scb, ' ');
	    if (useto) {
		ses_putstr(cp->scb, TO_STR);
	    }
	    ses_putchar(cp->scb, '\'');
	    ses_putstr(cp->scb, useval);
	    ses_putchar(cp->scb, '\'');
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* output_diff */


/********************************************************************
 * FUNCTION output_cdb_diff
 * 
 *  Output one detailed difference with 1 sub-field
 *  from a change description block
 *
 * INPUTS:
 *    cp == parameter block to use
 *    fieldname == fieldname that is different to print
 *    fieldval == fieldname that is different to print
 *    cdb == descriptor for subfield
 *
 *********************************************************************/
static void
    output_cdb_diff (yangdiff_diffparms_t *cp,
		     const xmlChar *fieldname,
		     const xmlChar *fieldval,
		     yangdiff_cdb_t *cdb)
{
    if (!cdb->changed) {
	return;
    }

    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
	break;
    case YANGDIFF_DT_BRIEF:
	break;
    case YANGDIFF_DT_NORMAL:
	if (!cdb->oldval) {
	    output_line(cp, ISOLD, NULL, NULL, FALSE);
	} else {
	    output_line2(cp, ISOLD, fieldname, fieldval,
			 cdb->fieldname, cdb->oldval);
	}
	if (!cdb->newval) {
	    output_line(cp, ISNEW, NULL, NULL, FALSE);
	} else {
	    output_line2(cp, ISNEW, fieldname, fieldval,
			 cdb->fieldname, cdb->newval);
	}
	ses_putchar(cp->scb, '\n');
	break;
    case YANGDIFF_DT_REVISION:
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* output_cdb_diff */


/********************************************************************
 * FUNCTION output_import_diff
 * 
 *  Output the differences report for the imports portion
 *  of two 2 parsed modules.  import-stmt order is not
 *  a semantic change, so it is not checked
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldpcb == old module
 *    newpcb == new module
 *
 *********************************************************************/
static void
    output_import_diff (yangdiff_diffparms_t *cp,
			yang_pcb_t *oldpcb,
			yang_pcb_t *newpcb)
{
    ncx_import_t *oldimp, *newimp;

    /* clear the used flag, for yangdiff reuse */
    for (newimp = (ncx_import_t *)dlq_firstEntry(&newpcb->top->importQ);
	 newimp != NULL;
	 newimp = (ncx_import_t *)dlq_nextEntry(newimp)) {
	newimp->used = FALSE;
    }

    /* look for matching imports */
    for (oldimp = (ncx_import_t *)dlq_firstEntry(&oldpcb->top->importQ);
	 oldimp != NULL;
	 oldimp = (ncx_import_t *)dlq_nextEntry(oldimp)) {

	/* find this import in the new module */
	newimp = ncx_find_import(newpcb->top, oldimp->module);
	if (newimp) {
	    if (xml_strcmp(oldimp->prefix, newimp->prefix)) {
		/* prefix was changed in the new module */
		output_diff2(cp, YANG_K_IMPORT, oldimp->module,
			     YANG_K_PREFIX,  oldimp->prefix, 
			     newimp->prefix);
	    }
	    newimp->used = TRUE;
	} else {
	    /* import was removed from the new module */
	    output_diff(cp, YANG_K_IMPORT, oldimp->module, NULL, TRUE);
	}
    }

    for (newimp = (ncx_import_t *)dlq_firstEntry(&newpcb->top->importQ);
	 newimp != NULL;
	 newimp = (ncx_import_t *)dlq_nextEntry(newimp)) {

	if (!newimp->used) {
	    /* this import was added in the new revision */
	    output_diff(cp, YANG_K_IMPORT, NULL, newimp->module, TRUE);
	}
    }

} /* output_import_diff */


/********************************************************************
 * FUNCTION output_include_diff
 * 
 *  Output the differences report for the includes portion
 *  of two 2 parsed modules
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldpcb == old module
 *    newpcb == new module
 *
 *********************************************************************/
static void
    output_include_diff (yangdiff_diffparms_t *cp,
			 yang_pcb_t *oldpcb,
			 yang_pcb_t *newpcb)
{
    ncx_include_t *oldinc, *newinc;

    /* clear usexsd field for yangdiff reuse */
    for (newinc = (ncx_include_t *)dlq_firstEntry(&newpcb->top->includeQ);
	 newinc != NULL;
	 newinc = (ncx_include_t *)dlq_nextEntry(newinc)) {
	newinc->usexsd = FALSE;
    }

    /* look for matchine entries */
    for (oldinc = (ncx_include_t *)dlq_firstEntry(&oldpcb->top->includeQ);
	 oldinc != NULL;
	 oldinc = (ncx_include_t *)dlq_nextEntry(oldinc)) {

	/* find this include in the new module */
	newinc = ncx_find_include(newpcb->top, oldinc->submodule);
	if (newinc) {
	    newinc->usexsd = TRUE;
	} else {
	    /* include was removed from the new module */
	    output_diff(cp, YANG_K_INCLUDE, oldinc->submodule, NULL, TRUE);
	}
    }

    for (newinc = (ncx_include_t *)dlq_firstEntry(&newpcb->top->includeQ);
	 newinc != NULL;
	 newinc = (ncx_include_t *)dlq_nextEntry(newinc)) {

	if (!newinc->usexsd) {
	    /* this include was added in the new revision */
	    output_diff(cp, YANG_K_INCLUDE, NULL, newinc->submodule, TRUE);
	}
    }

} /* output_include_diff */


/********************************************************************
 * FUNCTION output_revision_diff
 * 
 *  Output the differences report for the revisions list portion
 *  of two 2 parsed modules
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldpcb == old module
 *    newpcb == new module
 *
 *********************************************************************/
static void
    output_revision_diff (yangdiff_diffparms_t *cp,
			 yang_pcb_t *oldpcb,
			 yang_pcb_t *newpcb)
{
    ncx_revhist_t *oldrev, *newrev;
    yangdiff_cdb_t  cdb;
    boolean isrev;

    isrev = (cp->edifftype == YANGDIFF_DT_REVISION) ? TRUE : FALSE;

    /* clear res field for yangdiff reuse */
    for (newrev = (ncx_revhist_t *)dlq_firstEntry(&newpcb->top->revhistQ);
	 newrev != NULL;
	 newrev = (ncx_revhist_t *)dlq_nextEntry(newrev)) {
	newrev->res = ERR_NCX_INVALID_STATUS;
    }

    for (oldrev = (ncx_revhist_t *)dlq_firstEntry(&oldpcb->top->revhistQ);
	 oldrev != NULL;
	 oldrev = (ncx_revhist_t *)dlq_nextEntry(oldrev)) {

	/* find this revision in the new module */
	newrev = ncx_find_revhist(newpcb->top, oldrev->version);
	if (newrev) {
	    if (str_field_changed(YANG_K_DESCRIPTION,
				  oldrev->descr, newrev->descr,
				  isrev, &cdb)) {
		/* description was changed in the new module */
		output_diff2(cp, YANG_K_REVISION, oldrev->version,
			     YANG_K_DESCRIPTION,  oldrev->descr, 
			     newrev->descr);
	    }
	    newrev->res = NO_ERR;
	} else {
	    /* revision was removed from the new module */
	    output_diff(cp, YANG_K_REVISION, oldrev->version, NULL, FALSE);
	}
    }

    for (newrev = (ncx_revhist_t *)dlq_firstEntry(&newpcb->top->revhistQ);
	 newrev != NULL;
	 newrev = (ncx_revhist_t *)dlq_nextEntry(newrev)) {

	if (newrev->res != NO_ERR) {
	    /* this revision-stmt was added in the new version */
	    output_diff(cp, YANG_K_REVISION, NULL, newrev->version, FALSE);
	}
    }

} /* output_revision_diff */


/********************************************************************
 * FUNCTION output_one_extension_diff
 * 
 *  Output the differences report for an extension
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldext == old extension
 *    newext == new extension
 *
 *********************************************************************/
static void
    output_one_extension_diff (yangdiff_diffparms_t *cp,
			       ext_template_t *oldext,
			       ext_template_t *newext)
{
    yangdiff_cdb_t  extcdb[5];
    uint32          changecnt, i;
    boolean         isrev;

    isrev = (cp->edifftype==YANGDIFF_DT_REVISION) ? TRUE : FALSE;

    /* figure out what changed */
    changecnt = 0;
    changecnt += str_field_changed(YANG_K_DESCRIPTION,
				   oldext->descr, newext->descr, 
				   isrev, &extcdb[0]);
    changecnt += str_field_changed(YANG_K_REFERENCE,
				   oldext->ref, newext->ref, 
				   isrev, &extcdb[1]);
    changecnt += str_field_changed(YANG_K_ARGUMENT,
				   oldext->arg, newext->arg, 
				   isrev, &extcdb[2]);
    changecnt += bool_field_changed(YANG_K_YIN_ELEMENT,
				    oldext->argel, newext->argel, 
				    isrev, &extcdb[3]);
    changecnt += status_field_changed(YANG_K_STATUS,
				      oldext->status, newext->status, 
				      isrev, &extcdb[4]);
    if (changecnt == 0) {
	return;
    }

    /* generate the diff output, based on the requested format */
    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
    case YANGDIFF_DT_BRIEF:
    case YANGDIFF_DT_REVISION:
	ses_putstr_indent(cp->scb, 
			  (isrev) ? MOD_STR : M_STR, 
			  cp->curindent);
	ses_putstr(cp->scb, YANG_K_EXTENSION);
	ses_putchar(cp->scb, ' ');
	ses_putstr(cp->scb, oldext->name);
	if (cp->edifftype == YANGDIFF_DT_TERSE) {
	    return;
	}

	indent_in(cp);
	for (i=0; i<5; i++) {
	    if (extcdb[i].changed) {
		output_cdb_line(cp, &extcdb[i]);
	    }
	}
	indent_out(cp);
	break;
    case YANGDIFF_DT_NORMAL:
	for (i=0; i<5; i++) {
	    if (extcdb[i].changed) {
		output_cdb_diff(cp, YANG_K_EXTENSION,
				oldext->name, &extcdb[i]);
	    }
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* output_one_extension_diff */


/********************************************************************
 * FUNCTION output_extension_diff
 * 
 *  Output the differences report for the extensions portion
 *  of two 2 parsed modules
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldpcb == old module
 *    newpcb == new module
 *
 *********************************************************************/
static void
    output_extension_diff (yangdiff_diffparms_t *cp,
			   yang_pcb_t *oldpcb,
			   yang_pcb_t *newpcb)
{
    ext_template_t *oldext, *newext;

    /* make sure the new 'used' flags are cleared */
    for (newext = (ext_template_t *)dlq_firstEntry(&newpcb->top->extensionQ);
	 newext != NULL;
	 newext = (ext_template_t *)dlq_nextEntry(newext)) {
	newext->used = FALSE;
    }

    for (oldext = (ext_template_t *)dlq_firstEntry(&oldpcb->top->extensionQ);
	 oldext != NULL;
	 oldext = (ext_template_t *)dlq_nextEntry(oldext)) {

	/* find this extension in the new module */
	newext = ext_find_extension(&newpcb->top->extensionQ, oldext->name);
	if (newext) {
	    output_one_extension_diff(cp, oldext, newext);
	    newext->used = TRUE;
	} else {
	    /* extension was removed from the new module */
	    output_diff(cp, YANG_K_EXTENSION, oldext->name, NULL, TRUE);
	}
    }

    for (newext = (ext_template_t *)dlq_firstEntry(&newpcb->top->extensionQ);
	 newext != NULL;
	 newext = (ext_template_t *)dlq_nextEntry(newext)) {
	if (!newext->used) {
	    /* this extension-stmt was added in the new version */
	    output_diff(cp, YANG_K_EXTENSION, NULL, newext->name, TRUE);
	}
    }

} /* output_extension_diff */


/********************************************************************
 * FUNCTION output_one_type_diff
 * 
 *  Output the differences report for one type section
 *  within a leaf, leaf-list, or typedef definition
 *
 * type_changed should be called first to determine
 * if the type actually changed.  Otherwise a 'M typedef foo'
 * output line will result and be a false positive
 *
 * INPUTS:
 *    cp == parameter block to use
 *    name == type nameto use
 *    oldmod == old module
 *    newmod == new module
 *    oldtypdef == old internal typedef
 *    newtypdef == new internal typedef
 *
 *********************************************************************/
static void
    output_one_type_diff (yangdiff_diffparms_t *cp,
			  const xmlChar *name,
			  const typ_def_t *oldtypdef,
			  const typ_def_t *newtypdef)
{
    xmlChar        *p, *oldp, *newp;
    yangdiff_cdb_t  typcdb[5];
    uint32          changecnt, i;
    boolean         isrev;

    typ_def_t         *oldtest, *newtest;
    const xmlChar     *oldpath, *newpath;
    const ncx_import_t *oldimp, *newimp;
    ncx_btype_t        oldbtyp, newbtyp;

    /* check if there is a module prefix involved
     * in the change.  This may be a false positive
     * if the prefix simply changed
     */
    if (prefix_field_changed(cp->oldmod, cp->newmod,
			     oldtypdef->prefix, 
			     newtypdef->prefix)) {
	oldp = p = cp->buff;
	if (oldtypdef->prefix) {
	    p += xml_strcpy(p, oldtypdef->prefix);
	    *p++ = ':';
	}
	p += xml_strcpy(p, oldtypdef->name);
	newp = ++p;
	if (newtypdef->prefix) {
	    p += xml_strcpy(p, newtypdef->prefix);
	    *p++ = ':';
	}
	p += xml_strcpy(p, newtypdef->name);	    
	(void)str_field_changed(YANG_K_TYPE, oldp, newp,
				TRUE, &typcdb[0]);
    } else if (str_field_changed(NULL, oldtypdef->typename, 
			  newtypdef->typename, FALSE, NULL)) {

    }




    ses_putstr_indent(cp->scb, isrev ? MOD_STR : M_STR, 
		      cp->curindent);
    ses_putstr(cp->scb, YANG_K_TYPE);
    ses_putchar(cp->scb, ' ');
    if (oldtyp->prefix) {
	ses_putstr(cp->scb, oldtyp->prefix);
	ses_putchar(cp->scb, ':');
    }
    ses_putstr(cp->scb, oldtyp->name);

    if (cp->edifftype == YANGDIFF_DT_TERSE) {
	return;
    }

    indent_in(cp);



    if (oldtypdef->class != newtypdef->class) {
	return 1;
    }

    switch (oldtypdef->class) {
    case NCX_CL_BASE:
	return (oldtypdef->def.base == newtypdef->def.base) ? 0 : 1;
    case NCX_CL_SIMPLE:
	oldbtyp = typ_get_basetype(oldtypdef);
	newbtyp = typ_get_basetype(newtypdef);
	if (oldbtyp != newbtyp) {
	    return 1;
	}
	if (oldbtyp == NCX_BT_KEYREF) {
	    oldpath = typ_get_keyref_path(oldtypdef);
	    newpath = typ_get_keyref_path(newtypdef);
	    return str_field_changed(YANG_K_PATH, oldpath, newpath, 
				     FALSE, NULL);
	} else if (typ_is_string(oldbtyp)) {
	    if (pattern_changed(oldtypdef, newtypdef)) {
		return 1;
	    }
	    if (range_changed(oldtypdef, newtypdef)) {
		return 1;
	    }
	} else if (typ_is_number(oldbtyp)) {
	    if (range_changed(oldtypdef, newtypdef)) {
		return 1;
	    }
	} else {
	    switch (oldbtyp) {
	    case NCX_BT_BITS:
		return ebQ_changed(&oldtypdef->def.simple.valQ,
				   &newtypdef->def.simple.valQ, TRUE);
	    case NCX_BT_ENUM:
		return ebQ_changed(&oldtypdef->def.simple.valQ,
				   &newtypdef->def.simple.valQ, FALSE);
	    case NCX_BT_UNION:
		return unQ_changed(&oldtypdef->def.simple.unionQ,
				   &newtypdef->def.simple.unionQ);
	    default:
		;
	    }
	}
	return 0;
    case NCX_CL_COMPLEX:
	/* not supported for NCX complex types!!! */
	return 0;     
    case NCX_CL_NAMED:
	oldtest = typ_get_new_named(oldtypdef);
	newtest = typ_get_new_named(newtypdef);
	if ((!oldtest && newtest) || (oldtest && !newtest)) {
	    return 1;
	}
	if (oldtest && newtest) {
	    if (type_changed(cp, oldtest, newtest)) {
	    }
	}

	return 0;
    case NCX_CL_REF:
	return type_changed(cp, oldtypdef->def.ref.typdef,
			    newtypdef->def.ref.typdef);
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return 0;
    }
    /*NOTREACHED*/



    isrev = (cp->edifftype==YANGDIFF_DT_REVISION) ? TRUE : FALSE;

    /* figure out what changed */
    changecnt = 0;
    changecnt += str_field_changed(YANG_K_DESCRIPTION,
				   oldext->descr, newext->descr, 
				   isrev, &extcdb[0]);
    changecnt += str_field_changed(YANG_K_REFERENCE,
				   oldext->ref, newext->ref, 
				   isrev, &extcdb[1]);
    changecnt += str_field_changed(YANG_K_ARGUMENT,
				   oldext->arg, newext->arg, 
				   isrev, &extcdb[2]);
    changecnt += bool_field_changed(YANG_K_YIN_ELEMENT,
				    oldext->argel, newext->argel, 
				    isrev, &extcdb[3]);
    changecnt += status_field_changed(YANG_K_STATUS,
				      oldext->status, newext->status, 
				      isrev, &extcdb[4]);
    if (changecnt == 0) {
	return;
    }

    /* generate the diff output, based on the requested format */
    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
    case YANGDIFF_DT_BRIEF:
    case YANGDIFF_DT_REVISION:
	ses_putstr_indent(cp->scb, 
			  (cp->edifftype==YANGDIFF_DT_REVISION) ? 
			  MOD_STR : M_STR, 
			  cp->curindent);
	ses_putstr(cp->scb, YANG_K_EXTENSION);
	ses_putchar(cp->scb, ' ');
	ses_putstr(cp->scb, oldext->name);
	if (cp->edifftype == YANGDIFF_DT_TERSE) {
	    return;
	}

	indent_in(cp);
	for (i=0; i<5; i++) {
	    if (extcdb[i].changed) {
		output_cdb_line(cp, &extcdb[i]);
	    }
	}
	indent_out(cp);
	break;
    case YANGDIFF_DT_NORMAL:
	for (i=0; i<5; i++) {
	    if (extcdb[i].changed) {
		output_cdb_diff(cp, YANG_K_EXTENSION,
				oldext->name, &extcdb[i]);
	    }
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* output_one_type_diff */


/********************************************************************
* FUNCTION errinfo_changed
*
* Check if the ncx_errinfo_t struct changed at all
*
* INPUTS:
*   olderr == old error struct to check
*   newerr == new error struct to check
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*********************************************************************/
static uint32
    errinfo_changed (const ncx_errinfo_t *olderr,
		     const ncx_errinfo_t *newerr)
{
    if ((!olderr && newerr) || (olderr && !newerr)) {
	return 1;
    } else if (olderr && newerr) {
	if (str_field_changed(YANG_K_DESCRIPTION,
			      olderr->descr, newerr->descr,
			      FALSE, NULL)) {
	    return 1;
	}
	if (str_field_changed(YANG_K_REFERENCE,
			      olderr->ref, newerr->ref,
			      FALSE, NULL)) {
	    return 1;
	}
	if (str_field_changed(YANG_K_ERROR_APP_TAG,
			      olderr->error_app_tag,
			      newerr->error_app_tag,
			      FALSE, NULL)) {
	    return 1;
	}
	if (str_field_changed(YANG_K_ERROR_MESSAGE,
			      olderr->error_message,
			      newerr->error_message,
			      FALSE, NULL)) {
	    return 1;
	}
	return 0;
    } else {
	return 0;
    }
    /*NOTREACHED*/
    
}  /* errinfo_changed */


/********************************************************************
* FUNCTION pattern_changed
*
* Check if the pattern-stmt changed at all
*
* INPUTS:
*   oldtypdef == old type def struct to check
*   newtypdef == new type def struct to check
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*********************************************************************/
static uint32
    pattern_changed (const typ_def_t *oldtypdef,
		     const typ_def_t *newtypdef)
{
    const xmlChar  *oldpat, *newpat;

    oldpat = typ_get_pattern(oldtypdef);
    newpat = typ_get_pattern(newtypdef);

    /* check pattern string is the same */
    if ((!oldpat && newpat) || (oldpat && !newpat)) {
	return 1;
    } else if (oldpat && newpat) {
	if (xml_strcmp(oldpat, newpat)) {
	    return 1;
	}
    } else {
	return 0;    	/* no pattern defined */
    }

    /* pattern string is the same, check error stuff */
    return errinfo_changed(oldtypdef->pat_errinfo,
			   newtypdef->pat_errinfo);
    
}  /* pattern_changed */


/********************************************************************
* FUNCTION range_changed
*
* Check if the range-stmt or length-stmt changed at all
*
* INPUTS:
*   oldtypdef == old type def struct to check
*   newtypdef == new type def struct to check
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*********************************************************************/
static uint32
    range_changed (const typ_def_t *oldtypdef,
		   const typ_def_t *newtypdef)
{
    const typ_range_t     *oldr, *newr;

    oldr = typ_get_crange_con(oldtypdef);
    newr = typ_get_crange_con(newtypdef);

    /* check range string is the same */
    if ((!oldr && newr) || (oldr && !newr)) {
	return 1;
    } else if (oldr && newr) {
	if (xml_strcmp(oldr->rangestr, newr->rangestr)) {
	    return 1;
	}
    } else {
	return 0;    	/* no range defined */
    }

    /* range string is the same, check error stuff */
    return errinfo_changed(oldtypdef->range_errinfo,
			   newtypdef->range_errinfo);
    
}  /* range_changed */


/********************************************************************
* FUNCTION ebQ_changed
*
* Check if the enum/bits Q in the typdef has changed, used for 
* enums and bits
*
* INPUTS:
*   oldQ == Q of old typ_enum_t structs to check
*   newQ == Q of new typ_enum_t structs to check
*   isbits == TRUE if checking for a bits datatype
*             FALSE if checking for an enum datatype
* RETURNS:
*   1 if field changed
*   0 if field not changed
*********************************************************************/
static uint32
    ebQ_changed (dlq_hdr_t *oldQ,
		 dlq_hdr_t *newQ,
		 boolean isbits)
{
    typ_enum_t     *oldval, *newval;


    if (dlq_count(oldQ) != dlq_count(newQ)) {
	return 1;
    }

    /* clear the seen flag to be safe */
    for (newval = (typ_enum_t *)dlq_firstEntry(newQ);
	 newval != NULL;
	 newval = (typ_enum_t *)dlq_nextEntry(newval)) {
	newval->flags &= ~TYP_FL_SEEN;
    }

    /* check for matching entries */
    for (oldval = (typ_enum_t *)dlq_firstEntry(oldQ);
	 oldval != NULL;
	 oldval = (typ_enum_t *)dlq_nextEntry(oldval)) {

	newval = typ_find_enumdef(newQ, oldval->name);
	if (newval) {
	    if (isbits) {
		if (oldval->pos != newval->pos) {
		    return 1;
		}
	    } else {
		if (oldval->val != newval->val) {
		    return 1;
		}
	    }
	    if (str_field_changed(YANG_K_DESCRIPTION,
				  oldval->descr, newval->descr,
				  FALSE, NULL)) {
		return 1;
	    }
	    if (str_field_changed(YANG_K_REFERENCE,
				  oldval->ref, newval->ref,
				  FALSE, NULL)) {
		return 1;
	    }
	    if (status_field_changed(YANG_K_STATUS,
				     oldval->status, newval->status,
				     FALSE, NULL)) {
		return 1;
	    }
	    /* mark new enum as used */
	    newval->flags |= TYP_FL_SEEN;
	} else {
	    /* removed name in new version */
	    return 1;
	}
    }

    /* check for new entries */
    for (newval = (typ_enum_t *)dlq_firstEntry(newQ);
	 newval != NULL;
	 newval = (typ_enum_t *)dlq_nextEntry(newval)) {
	if ((newval->flags & TYP_FL_SEEN) == 0) {
	    return 1;
	}
    }

    return 0;
    
}  /* ebQ_changed */


/********************************************************************
* FUNCTION unQ_changed
*
* Check if the union Q in the typdef has changed
*
* INPUTS:
*   oldQ == Q of old typ_unionnode_t structs to check
*   newQ == Q of new typ_unionnode_t structs to check
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*********************************************************************/
static uint32
    unQ_changed (dlq_hdr_t *oldQ,
		 dlq_hdr_t *newQ)
{
    typ_unionnode_t     *oldval, *newval;
    boolean              done;


    if (dlq_count(oldQ) != dlq_count(newQ)) {
	return 1;
    }

    /* clear the seen flag to be safe */
    for (newval = (typ_unionnode_t *)dlq_firstEntry(newQ);
	 newval != NULL;
	 newval = (typ_unionnode_t *)dlq_nextEntry(newval)) {
	newval->seen = FALSE;
    }

    oldval = (typ_unionnode_t *)dlq_firstEntry(oldQ);
    newval = (typ_unionnode_t *)dlq_firstEntry(newQ);


    /* check for matching entries */
    done = FALSE;
    while (!done) {
	if (!oldval || !newval) {
	    return 0;
	}

	if (oldval->typdef && newval->typdef) {
	    if (type_changed(cp, oldval->typdef, newval->typdef)) {
		return 1;
	    }
	} else if (oldval->typ && newval->typ) {
	    if (type_changed(cp, &oldval->typ->typdef,
			     &newval->typ->typdef)) {
		return 1;
	    }
	}

	newval->seen = TRUE;

	oldval = (typ_unionnode_t *)dlq_nextEntry(oldval);
	newval = (typ_unionnode_t *)dlq_nextEntry(newval);

    }

    /* check for new entries */
    for (newval = (typ_unionnode_t *)dlq_firstEntry(newQ);
	 newval != NULL;
	 newval = (typ_unionnode_t *)dlq_nextEntry(newval)) {
	if (!newval->seen) {
	    return 1;
	}
    }

    return 0;
    
}  /* unQ_changed */


/********************************************************************
* FUNCTION type_changed
*
* Check if the type clause and sub-clauses changed at all
*
* INPUTS:
*   cp == compare parameters to use
*   oldtypdef == old type def struct to check
*   newtypdef == new type def struct to check
*
* RETURNS:
*   1 if field changed
*   0 if field not changed
*********************************************************************/
static uint32
    type_changed (yangdiff_diffparms_t *cp,
		  typ_def_t *oldtypdef,
		  typ_def_t *newtypdef)
{
    typ_def_t         *oldtest, *newtest;
    const xmlChar     *oldpath, *newpath;
    ncx_btype_t        oldbtyp, newbtyp;

#ifdef DEBUG
    if (!oldtypdef || !newtypdef) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 1;
    }
#endif

    if (oldtypdef->class != newtypdef->class) {
	return 1;
    }
    if (prefix_field_changed(cp->oldmod, cp->newmod,
			     oldtypdef->prefix, 
			     newtypdef->prefix)) {
	return 1;
    }
    if (str_field_changed(NULL, oldtypdef->typename, 
			  newtypdef->typename, FALSE, NULL)) {
	return 1;
    }

    switch (oldtypdef->class) {
    case NCX_CL_BASE:
	return (oldtypdef->def.base == newtypdef->def.base) ? 0 : 1;
    case NCX_CL_SIMPLE:
	oldbtyp = typ_get_basetype(oldtypdef);
	newbtyp = typ_get_basetype(newtypdef);
	if (oldbtyp != newbtyp) {
	    return 1;
	}
	if (oldbtyp == NCX_BT_KEYREF) {
	    oldpath = typ_get_keyref_path(oldtypdef);
	    newpath = typ_get_keyref_path(newtypdef);
	    return str_field_changed(YANG_K_PATH, oldpath, newpath, 
				     FALSE, NULL);
	} else if (typ_is_string(oldbtyp)) {
	    if (pattern_changed(oldtypdef, newtypdef)) {
		return 1;
	    }
	    if (range_changed(oldtypdef, newtypdef)) {
		return 1;
	    }
	} else if (typ_is_number(oldbtyp)) {
	    if (range_changed(oldtypdef, newtypdef)) {
		return 1;
	    }
	} else {
	    switch (oldbtyp) {
	    case NCX_BT_BITS:
		return ebQ_changed(&oldtypdef->def.simple.valQ,
				   &newtypdef->def.simple.valQ, TRUE);
	    case NCX_BT_ENUM:
		return ebQ_changed(&oldtypdef->def.simple.valQ,
				   &newtypdef->def.simple.valQ, FALSE);
	    case NCX_BT_UNION:
		return unQ_changed(&oldtypdef->def.simple.unionQ,
				   &newtypdef->def.simple.unionQ);
	    default:
		;
	    }
	}
	return 0;
    case NCX_CL_COMPLEX:
	/* not supported for NCX complex types!!! */
	return 0;     
    case NCX_CL_NAMED:
	oldtest = typ_get_new_named(oldtypdef);
	newtest = typ_get_new_named(newtypdef);
	if ((!oldtest && newtest) || (oldtest && !newtest)) {
	    return 1;
	} else if (oldtest && newtest) {
	    if (type_changed(cp, oldtest, newtest)) {
		return 1;
	    }
	}
	return type_changed(cp, oldtypdef->def.named.typ,
			    newtypdef->def.named.typ);
    case NCX_CL_REF:
	return type_changed(cp, oldtypdef->def.ref.typdef,
			    newtypdef->def.ref.typdef);
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return 0;
    }
    /*NOTREACHED*/

} /* type_changed */


/********************************************************************
 * FUNCTION output_one_typedef_diff
 * 
 *  Output the differences report for one typedef definition
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldtyp == old typedef
 *    newtyp == new typedef
 *
 *********************************************************************/
static void
    output_one_typedef_diff (yangdiff_diffparms_t *cp,
			     typ_template_t *oldtyp,
			     typ_template_t *newtyp)
{
    yangdiff_cdb_t  typcdb[5];
    uint32          changecnt, i;
    boolean         isrev, tchanged;

    isrev = (cp->edifftype==YANGDIFF_DT_REVISION) ? TRUE : FALSE;

    /* figure out what changed */
    tchanged = FALSE;
    changecnt = 0;

    if (type_changed(cp, &oldtyp->typdef, &newtyp->typdef)) {
	tchanged = TRUE;
	changecnt++;
    }

    changecnt += str_field_changed(YANG_K_UNITS,
				   oldtyp->units, newtyp->units, 
				   isrev, &typcdb[0]);
    changecnt += str_field_changed(YANG_K_DEFAULT,
				   oldtyp->defval, newtyp->defval, 
				   isrev, &typcdb[1]);
    changecnt += status_field_changed(YANG_K_STATUS,
				      oldtyp->status, newtyp->status, 
				      isrev, &typcdb[2]);
    changecnt += str_field_changed(YANG_K_DESCRIPTION,
				   oldtyp->descr, newtyp->descr, 
				   isrev, &typcdb[3]);
    changecnt += str_field_changed(YANG_K_REFERENCE,
				   oldtyp->ref, newtyp->ref, 
				   isrev, &typcdb[4]);
    if (changecnt == 0) {
	return;
    }

    /* generate the diff output, based on the requested format */
    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
    case YANGDIFF_DT_BRIEF:
    case YANGDIFF_DT_REVISION:
	ses_putstr_indent(cp->scb, isrev ? MOD_STR : M_STR, 
			  cp->curindent);
	ses_putstr(cp->scb, YANG_K_TYPEDEF);
	ses_putchar(cp->scb, ' ');
	ses_putstr(cp->scb, oldtyp->name);
	if (cp->edifftype == YANGDIFF_DT_TERSE) {
	    return;
	}

	indent_in(cp);
	if (tchanged) {
	    output_one_type_diff(cp, oldtyp->name,
				 &oldtyp->typdef, &newtyp->typdef);
	}
	for (i=0; i<5; i++) {
	    if (typcdb[i].changed) {
		output_cdb_line(cp, &typcdb[i]);
	    }
	}
	indent_out(cp);
	break;
    case YANGDIFF_DT_NORMAL:
	if (tchanged) {
	    output_one_type_diff(cp, oldtyp->name,
				 &oldtyp->typdef, &newtyp->typdef);
	}
	for (i=0; i<5; i++) {
	    if (typcdb[i].changed) {
		output_cdb_diff(cp, YANG_K_TYPEDEF,
				oldtyp->name, &typcdb[i]);
	    }
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* output_one_typedef_diff */


/********************************************************************
 * FUNCTION output_typedef_diff
 * 
 *  Output the differences report for a typedef definition
 *  Not always called for top-level typedefs; Can be called
 *  for nested typedefs
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldpcb == old module
 *    newpcb == new module
 *
 *********************************************************************/
static void
    output_typedef_diff (yangdiff_diffparms_t *cp,
			 yang_pcb_t *oldpcb,
			 yang_pcb_t *newpcb)
{
    typ_template_t *oldtyp, *newtyp;

    /* borrowing the 'used' flag for marking matched typedefs
     * first set all these flags to FALSE
     */
    for (newtyp = (typ_template_t *)dlq_firstEntry(&newpcb->top->typeQ);
	 newtyp != NULL;
	 newtyp = (typ_template_t *)dlq_nextEntry(newtyp)) {
	newtyp->used = FALSE;
    }

    /* look through the old type Q for matching types in the new type Q */
    for (oldtyp = (typ_template_t *)dlq_firstEntry(&oldpcb->top->typeQ);
	 oldtyp != NULL;
	 oldtyp = (typ_template_t *)dlq_nextEntry(oldtyp)) {

	/* find this revision in the new module */
	newtyp = ncx_find_type(newpcb->top, oldtyp->name);
	if (newtyp) {
	    output_one_typedef_diff(cp, oldtyp, newtyp);
	    newtyp->used = TRUE;
	} else {
	    /* typedef was removed from the new module */
	    output_diff(cp, YANG_K_TYPEDEF, oldtyp->name, NULL, TRUE);
	}
    }

    /* look for typedefs that were added in the new module */
    for (newtyp = (typ_template_t *)dlq_firstEntry(&newpcb->top->typeQ);
	 newtyp != NULL;
	 newtyp = (typ_template_t *)dlq_nextEntry(newtyp)) {
	if (!newtyp->used) {
	    /* this typedef was added in the new version */
	    output_diff(cp, YANG_K_TYPEDEF, NULL, newtyp->name, TRUE);
	}
    }

} /* output_typedef_diff */


/********************************************************************
 * FUNCTION output_hdr_diff
 * 
 *  Output the differences report for the header portion
 *  of two 2 parsed modules
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldpcb == old module
 *    newpcb == new module
 *
 *********************************************************************/
static void
    output_hdr_diff (yangdiff_diffparms_t *cp,
		     yang_pcb_t *oldpcb,
		     yang_pcb_t *newpcb)
{

    char  oldnumbuff[NCX_MAX_NLEN];
    char  newnumbuff[NCX_MAX_NLEN];

    /* module name, module/submodule mismatch */
    if (oldpcb->top->ismod != newpcb->top->ismod ||
	xml_strcmp(oldpcb->top->name, newpcb->top->name)) {
	output_line(cp, ISOLD, 
		    oldpcb->top->ismod ? 
		    YANG_K_MODULE : YANG_K_SUBMODULE, 
		    oldpcb->top->name, TRUE);
	output_line(cp, ISNEW, 
		    newpcb->top->ismod ? 
		    YANG_K_MODULE : YANG_K_SUBMODULE, 
		    newpcb->top->name, TRUE);
	ses_putchar(cp->scb, '\n');
    }

    /* yang-version */
    if (oldpcb->top->langver != newpcb->top->langver) {
	sprintf(oldnumbuff, "%u", oldpcb->top->langver);
	sprintf(newnumbuff, "%u", newpcb->top->langver);
	output_diff(cp, YANG_K_YANG_VERSION,
		    (const xmlChar *)oldnumbuff,
		    (const xmlChar *)newnumbuff, TRUE);
    }

    /* namespace */
    output_diff(cp, YANG_K_NAMESPACE,
		oldpcb->top->ns, newpcb->top->ns, FALSE);

    /* prefix */
    output_diff(cp, YANG_K_PREFIX,
		oldpcb->top->prefix, newpcb->top->prefix, FALSE);

    /* imports */
    output_import_diff(cp, oldpcb, newpcb);

    /* includes */
    output_include_diff(cp, oldpcb, newpcb);

    /* organization */
    output_diff(cp, YANG_K_ORGANIZATION,
		oldpcb->top->organization,
		newpcb->top->organization, FALSE);

    /* contact */
    output_diff(cp, YANG_K_CONTACT,
		oldpcb->top->contact_info,
		newpcb->top->contact_info, FALSE);

    /* description */
    output_diff(cp, YANG_K_DESCRIPTION,
		oldpcb->top->descr, newpcb->top->descr, FALSE);

    /* reference */
    output_diff(cp, YANG_K_REFERENCE,
		oldpcb->top->ref, newpcb->top->ref, FALSE);

    /* revisions */
    output_revision_diff(cp, oldpcb, newpcb);

}  /* output_hdr_diff */


/********************************************************************
 * FUNCTION output_diff_banner
 * 
 *  Output the banner for the start of a diff report
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldpcb == old module
 *    newpcb == new module
 *
 *********************************************************************/
static void
    output_diff_banner (yangdiff_diffparms_t *cp,
			yang_pcb_t *oldpcb,
			yang_pcb_t *newpcb)
{
    if (!cp->firstdone) {
	ses_putstr(cp->scb, (const xmlChar *)"\n// Generated by ");
	ses_putstr(cp->scb, YANGDIFF_PROGNAME);
	ses_putchar(cp->scb, ' ');
	ses_putstr(cp->scb, YANGDIFF_PROGVER);
	cp->firstdone = TRUE;
    }

#ifdef ADD_SEP_LINE
    if (cp->new_isdir) {
	ses_putstr(cp->scb, YANGDIFF_LINE);
    }
#endif

    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
    case YANGDIFF_DT_BRIEF:
    case YANGDIFF_DT_REVISION:
	ses_putstr(cp->scb, (const xmlChar *)"\n// old: ");
	break;
    case YANGDIFF_DT_NORMAL:
	ses_putstr(cp->scb, (const xmlChar *)"\n---  ");
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    ses_putstr(cp->scb, oldpcb->top->name);
    ses_putchar(cp->scb, ' ');
    ses_putchar(cp->scb, '(');
    ses_putstr(cp->scb, oldpcb->top->version);
    ses_putchar(cp->scb, ')');
    ses_putchar(cp->scb, ' ');
    ses_putstr(cp->scb,
	       (cp->old_isdir) ? oldpcb->top->source 
	       : oldpcb->top->sourcefn);

    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
    case YANGDIFF_DT_BRIEF:
    case YANGDIFF_DT_REVISION:
	ses_putstr(cp->scb, (const xmlChar *)"\n// new: ");
	break;
    case YANGDIFF_DT_NORMAL:
	ses_putstr(cp->scb, (const xmlChar *)"\n+++  ");
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    ses_putstr(cp->scb, newpcb->top->name);
    ses_putchar(cp->scb, ' ');
	ses_putchar(cp->scb, '(');
    ses_putstr(cp->scb, newpcb->top->version);
	ses_putchar(cp->scb, ')');
    ses_putchar(cp->scb, ' ');
    ses_putstr(cp->scb,
	       (cp->new_isdir) ? newpcb->top->source 
	       : newpcb->top->sourcefn);
    ses_putchar(cp->scb, '\n');

    if (cp->edifftype == YANGDIFF_DT_REVISION) {
	indent_in(cp);
	ses_putstr_indent(cp->scb, (const xmlChar *)"revision ",
			  cp->curindent);
	tstamp_date(cp->buff);
	ses_putstr(cp->scb, cp->buff);
	ses_putstr(cp->scb, (const xmlChar *)" {");
	indent_in(cp);
	ses_putstr_indent(cp->scb, (const xmlChar *)"description \"",
			  cp->curindent);
	indent_in(cp);
    }

}  /* output_diff_banner */


/********************************************************************
 * FUNCTION generate_diff_report
 * 
 *  Generate the differences report for 2 parsed modules
 *
 * INPUTS:
 *    cp == parameter block to use
 *    oldpcb == old module
 *    newpcb == new module
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    generate_diff_report (yangdiff_diffparms_t *cp,
			  yang_pcb_t *oldpcb,
			  yang_pcb_t *newpcb)
{
    status_t   res;

    res = NO_ERR;

    /* generate the banner indicating the start of diff report */
    output_diff_banner(cp, oldpcb, newpcb);

    /* header fields */
    if (!cp->noheader) {
	output_hdr_diff(cp, oldpcb, newpcb);
    }

    /* extensions */
    output_extension_diff(cp, oldpcb, newpcb);

    /* global typedefs */
    output_typedef_diff(cp, oldpcb, newpcb);







    /* finish off revision statement if that is the diff mode */
    if (cp->edifftype == YANGDIFF_DT_REVISION) {
	/* end description clause */
	indent_out(cp);
	ses_putstr_indent(cp->scb, (const xmlChar *)"\";",
			  cp->curindent);
	/* end revision clause */
	indent_out(cp);
	ses_putstr_indent(cp->scb, (const xmlChar *)"}",
			  cp->curindent);
	indent_out(cp);
    }

    return res;

}  /* generate_diff_report */


/********************************************************************
* FUNCTION make_curold_filename
* 
* Construct an output filename spec, based on the 
* comparison parameters
*
* INPUTS:
*    newfn == new filename -- just the module.ext part
*    cp == comparison parameters to use
*
* RETURNS:
*   malloced string or NULL if malloc error
*********************************************************************/
static xmlChar *
    make_curold_filename (const xmlChar *newfn,
			  const yangdiff_diffparms_t *cp)
{
    xmlChar         *buff, *p;
    uint32           len;


    if (cp->old_isdir) {
	len = xml_strlen(cp->old);
	if (cp->old[len-1] != NCXMOD_PSCHAR) {
	    len++;
	}
	len += xml_strlen(newfn);
    } else {
	len = xml_strlen(cp->old);
    }

    buff = m__getMem(len+1);
    if (!buff) {
	return NULL;
    }

    if (cp->old_isdir) {
	p = buff;
	p += xml_strcpy(p, cp->old);
	if (*(p-1) != NCXMOD_PSCHAR) {
	    *p++ = NCXMOD_PSCHAR;
	}
	xml_strcpy(p, newfn);
    } else {
	xml_strcpy(buff, cp->old);
    }

    return buff;

}   /* make_curold_filename */


/********************************************************************
 * FUNCTION compare_one
 * 
 *  Validate and then compare one module to another
 *
 * INPUTS:
 *    cp == parameter block to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    compare_one (yangdiff_diffparms_t *cp)
{
    yang_pcb_t        *oldpcb, *newpcb;
    status_t           res;
    boolean            skipreport;

    res = NO_ERR;
    skipreport = FALSE;
    cp->curindent = 0;

    /* load in the requested 'new' module to compare
     * if this is a subtree call, then the curnew pointer
     * will be set, otherwise the 'new' pointer must be set
     */
    newpcb = ncxmod_load_module_xsd((cp->curnew) ? 
				    cp->curnew : cp->new,
				    (cp->curnew) ? TRUE : FALSE,
				    TRUE  /* cp->unified */, &res);
    if (res == ERR_NCX_SKIPPED) {
	/* this is probably a submodule being skipped in subtree mode */
	if (newpcb) {
	    yang_free_pcb(newpcb);
	}
	return NO_ERR;
    } else if (res != NO_ERR) {
	if (newpcb && newpcb->top && 
	    (newpcb->top->errors || newpcb->top->warnings)) {
	    log_write("\n*** %s: %u Errors, %u Warnings\n", 
		      newpcb->top->sourcefn,
		      newpcb->top->errors, newpcb->top->warnings);
	} else {
	    log_write("\n");   /***/
	}
	if (newpcb) {
	    yang_free_pcb(newpcb);
	}
	return res;
    } else if (LOGDEBUG2 && newpcb && newpcb->top) {
	log_debug2("\n*** %s: %u Errors, %u Warnings\n", 
		   newpcb->top->sourcefn,
		   newpcb->top->errors, newpcb->top->warnings);
    }

    /* figure out where to get the requested 'old' file */
    cp->curold = make_curold_filename(newpcb->top->sourcefn, cp);
    if (!cp->curold) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(NULL, NULL, res);
	yang_free_pcb(newpcb);
	return res;
    }

    /* load in the requested 'old' module to compare
     * if this is a subtree call, then the curnew pointer
     * will be set, otherwise the 'new' pointer must be set
     */
    oldpcb = ncxmod_load_module_diff(cp->curold, 
				     (cp->curnew) ? TRUE : FALSE,
				     TRUE  /* cp->unified */,
				     NULL /* modpath */,  &res);
    if (res == ERR_NCX_SKIPPED) {
	/* this is probably a submodule being skipped in subtree mode */
	log_debug("\nyangdiff: New PCB OK but old PCB skipped (%s)",
		  newpcb->top->sourcefn);
	if (oldpcb) {
	    yang_free_pcb(oldpcb);
	}
	yang_free_pcb(newpcb);
	return NO_ERR;
    } else if (res != NO_ERR) {
	if (oldpcb && oldpcb->top &&
	    (LOGINFO ||
	     (oldpcb->top->errors || oldpcb->top->warnings))) {
	    log_info("\n*** %s: %u Errors, %u Warnings\n", 
		      oldpcb->top->sourcefn,
		      oldpcb->top->errors, oldpcb->top->warnings);
	} else {
	    log_write("\n");   /***/
	}
	if (oldpcb) {
	    yang_free_pcb(oldpcb);
	}
	yang_free_pcb(newpcb);
	return res;
    } else if (LOGDEBUG2 && oldpcb && oldpcb->top) {
	log_debug2("\n*** %s: %u Errors, %u Warnings\n", 
		   oldpcb->top->sourcefn,
		   oldpcb->top->errors, oldpcb->top->warnings);
    }


    /* check if old and new files parsed okay */
    if (ncx_any_dependency_errors(newpcb->top)) {
	log_error("\nError: one or more modules imported into new '%s' "
		  "had errors", newpcb->mod->sourcefn);
	skipreport = TRUE;
    } else {
	cp->newmod = newpcb->top;
    }

    if (ncx_any_dependency_errors(oldpcb->top)) {
	log_error("\nError: one or more modules imported into old '%s' "
		  "had errors", oldpcb->mod->sourcefn);
	skipreport = TRUE;
    } else {
	cp->oldmod = oldpcb->top;
    }

    /* skip NCX files */
    if (!oldpcb->top->isyang) {
	log_error("\nError: NCX modules not supported (%s)", 
		  oldpcb->mod->sourcefn);
	skipreport = TRUE;
    } else if (!newpcb->top->isyang) {
	log_error("\nError: NCX modules not supported (%s)", 
		  newpcb->mod->sourcefn);
	skipreport = TRUE;
    }

    /* generate compare output to the dummy session */
    if (!skipreport) {
	res = generate_diff_report(cp, oldpcb, newpcb);
    } else {
	res = ERR_NCX_IMPORT_ERRORS;
	ncx_print_errormsg(NULL, NULL, res);
    }

    /* clean up the parser control blocks */
    if (newpcb) {
	yang_free_pcb(newpcb);
    }
    if (oldpcb) {
	yang_free_pcb(oldpcb);
    }

    return res;

}  /* compare_one */


/********************************************************************
 * FUNCTION subtree_callback
 * 
 * Handle the current filename in the subtree traversal
 * Parse the module and generate.
 *
 * Follows ncxmod_callback_fn_t template
 *
 * INPUTS:
 *   fullspec == absolute or relative path spec, with filename and ext.
 *               this regular file exists, but has not been checked for
 *               read access of 
 *   cookie == NOT USED
 *
 * RETURNS:
 *    status
 *
 *    Return fatal error to stop the traversal or NO_ERR to
 *    keep the traversal going.  Do not return any warning or
 *    recoverable error, just log and move on
 *********************************************************************/
static status_t
    subtree_callback (const char *fullspec,
		      void *cookie)
{
    yangdiff_diffparms_t *cp;
    status_t    res;
    
    cp = cookie;
    res = NO_ERR;

    if (cp->curnew) {
	m__free(cp->curnew);
    }
    cp->curnew = xml_strdup((const xmlChar *)fullspec);
    if (!cp->curnew) {
	return ERR_INTERNAL_MEM;
    }

    log_debug2("\nStart subtree file:\n%s\n", fullspec);
    res = compare_one(cp);
    if (res != NO_ERR) {
	if (!NEED_EXIT) {
	    res = NO_ERR;
	}
    }
    return res;

}  /* subtree_callback */


/********************************************************************
* FUNCTION make_output_filename
* 
* Construct an output filename spec, based on the 
* comparison parameters
*
* INPUTS:
*    cp == comparison parameters to use
*
* RETURNS:
*   malloced string or NULL if malloc error
*********************************************************************/
static xmlChar *
    make_output_filename (const yangdiff_diffparms_t *cp)
{
    xmlChar        *buff, *p;
    uint32          len;

    if (cp->output && *cp->output) {
	len = xml_strlen(cp->output);
	if (cp->output_isdir) {
	    if (cp->output[len-1] != NCXMOD_PSCHAR) {
		len++;
	    }
	    len += xml_strlen(YANGDIFF_DEF_FILENAME);
	}
    } else {
	len = xml_strlen(YANGDIFF_DEF_FILENAME);
    }

    buff = m__getMem(len+1);
    if (!buff) {
	return NULL;
    }

    if (cp->output && *cp->output) {
	p = buff;
	p += xml_strcpy(p, cp->output);
	if (cp->output_isdir) {
	    if (*(p-1) != NCXMOD_PSCHAR) {
		*p++ = NCXMOD_PSCHAR;
	    }
	    xml_strcpy(p, YANGDIFF_DEF_FILENAME);
	}
    } else {
	xml_strcpy(buff, YANGDIFF_DEF_FILENAME);
    }

    return buff;

}   /* make_output_filename */


/********************************************************************
 * FUNCTION get_output_session
 * 
 * Malloc and init an output session.
 * Open the correct output file if needed
 *
 * INPUTS:
 *    cp == compare parameters to use
 *    res == address of return status
 *
 * OUTPUTS:
 *  *res == return status
 *
 * RETURNS:
 *   pointer to new session control block, or NULL if some error
 *********************************************************************/
static ses_cb_t *
    get_output_session (yangdiff_diffparms_t *cp,
			status_t  *res)
{
    FILE            *fp;
    ses_cb_t        *scb;
    xmlChar         *namebuff;

    fp = NULL;
    scb = NULL;
    namebuff = NULL;
    *res = NO_ERR;

    /* open the output file if not STDOUT */
    if (cp->output && *cp->output) {
	namebuff = make_output_filename(cp);
	if (!namebuff) {
	    *res = ERR_INTERNAL_MEM;
	    return NULL;
	}

	fp = fopen((const char *)namebuff, "w");
	if (!fp) {
	    *res = ERR_FIL_OPEN;
	    return NULL;
	}
    }

    /* get a dummy session control block */
    scb = ses_new_dummy_scb();
    if (!scb) {
	*res = ERR_INTERNAL_MEM;
    } else {
	scb->fp = fp;
	ses_set_mode(scb, SES_MODE_TEXT);
    }

    if (namebuff) {
	m__free(namebuff);
    }

    return scb;

}  /* get_output_session */


/********************************************************************
* FUNCTION main_run
*
*    Run the compare program, based on the input parameters
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    main_run (void)
{

    status_t  res;

    res = NO_ERR;

    if (diffparms.versionmode) {
	log_write("yangdiff %s\n", YANGDIFF_PROGVER);
    }
    if (diffparms.helpmode) {
	help_program_module(YANGDIFF_MOD, YANGDIFF_PARMSET, FULL);
    }
    if ((diffparms.helpmode || diffparms.versionmode)) {
	return res;
    }

    /* check if subdir search suppression is requested */
    if (diffparms.nosubdirs) {
	ncxmod_set_subdirs(FALSE);
    }

    /* setup the output session to a file or STDOUT */
    diffparms.scb = get_output_session(&diffparms, &res);
    if (!diffparms.scb || res != NO_ERR) {
	return res;
    }

    /* reset the current indent from default (3) to 0 */
    /* ses_set_indent(diffparms.scb, 0); */
    
    /* make sure the mandatory parameters are set */
    if (!diffparms.old) {
	log_error("\nError: The 'old' parameter is required.");
	res = ERR_NCX_MISSING_PARM;
	ncx_print_errormsg(NULL, NULL, res);
    }
    if (!diffparms.new) {
	log_error("\nError: The 'new' parameter is required.");
	res = ERR_NCX_MISSING_PARM;
	ncx_print_errormsg(NULL, NULL, res);
    }
    if (!diffparms.difftype) {
	log_error("\nError: The 'difftype' parameter is required.");
	res = ERR_NCX_MISSING_PARM;
	ncx_print_errormsg(NULL, NULL, res);
    }
    if (diffparms.edifftype==YANGDIFF_DT_NONE) {
	log_error("\nError: Invalid 'difftype' parameter value.");
	res = ERR_NCX_INVALID_VALUE;
	ncx_print_errormsg(NULL, NULL, res);
    }
    if (diffparms.new_isdir && !diffparms.old_isdir) {
	log_error("\nError: The 'old' parameter must identify a directory.");
	res = ERR_NCX_INVALID_VALUE;
	ncx_print_errormsg(NULL, NULL, res);
    }
    if (!xml_strcmp(diffparms.old, diffparms.new)) {
	log_error("\nError: The 'old' and 'new' parameters must be different.");
	res = ERR_NCX_INVALID_VALUE;
	ncx_print_errormsg(NULL, NULL, res);
    }


    if (res == NO_ERR) {

	/* compare one file to another or 1 subtree to another */
	if (diffparms.new_isdir) {
	    res = ncxmod_process_subtree((const char *)diffparms.new,
					 subtree_callback,
					 &diffparms);
	} else {
	    res = compare_one(&diffparms);
	}
    }

    return res;

} /* main_run */


/********************************************************************
* FUNCTION process_cli_input
*
* Process the param line parameters against the hardwired
* parmset for the yangdiff program
*
* get all the parms and store them in the diffparms struct
*
* INPUTS:
*    argc == argument count
*    argv == array of command line argument strings
*    cp == address of returned values
*
* OUTPUTS:
*    parmset values will be stored in *diffparms if NO_ERR
*    errors will be written to STDOUT
*
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
static status_t
    process_cli_input (int argc,
		       const char *argv[],
		       yangdiff_diffparms_t  *cp)
{
    psd_template_t  *psd;
    ps_parmset_t    *ps;
    ps_parm_t       *parm;
    val_value_t     *val;
    ncx_node_t       dtyp;
    status_t         res;

    res = NO_ERR;
    ps = NULL;

    cp->buff = m__getMem(YANGDIFF_BUFFSIZE);
    if (!cp->buff) {
	return ERR_INTERNAL_MEM;
    }
    cp->bufflen = YANGDIFF_BUFFSIZE;

    /* find the parmset definition in the registry */
    dtyp = NCX_NT_PSD;
    psd = (psd_template_t *)
	def_reg_find_moddef(YANGDIFF_MOD, YANGDIFF_PARMSET, &dtyp);
    if (!psd) {
	res = ERR_NCX_NOT_FOUND;
    }

    /* parse the command line against the PSD */
    if (res == NO_ERR) {
	ps = ps_parse_cli(argc, argv, psd,
			  FULLTEST, PLAINMODE, TRUE, &res);
    }
    if (res != NO_ERR) {
	return res;
    } else if (!ps) {
	pr_usage();
	return ERR_NCX_SKIPPED;
    } else {
	cli_ps = ps;
    }

    /* next get any params from the conf file */
    res = ps_get_parmval(ps, YANGDIFF_PARM_CONFIG, &val);
    if (res == NO_ERR) {
	/* try the specified config location */
	cp->config = VAL_STR(val);
	res = conf_parse_ps_from_filespec(cp->config, ps, TRUE, TRUE);
	if (res != NO_ERR) {
	    return res;
	}
    } else {
	/* try default config location */
	res = conf_parse_ps_from_filespec(YANGDIFF_DEF_CONFIG,
					  ps, TRUE, FALSE);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the log-level parameter */
    res = ps_get_parmval(ps, NCX_EL_LOGLEVEL, &val);
    if (res == NO_ERR) {
	cp->log_level = 
	    log_get_debug_level_enum((const char *)VAL_STR(val));
	if (cp->log_level == LOG_DEBUG_NONE) {
	    log_error("\nError: invalid log-level value (%s)",
		      (const char *)VAL_STR(val));
	    return ERR_NCX_INVALID_VALUE;
	} else {
	    log_set_debug_level(cp->log_level);
	}
    }

    /* get the logging parameters */
    res = ps_get_parmval(ps, NCX_EL_LOG, &val);
    if (res == NO_ERR) {
	cp->logfilename = VAL_STR(val);
    }
    if (ps_find_parm(ps, NCX_EL_LOGAPPEND)) {
	cp->logappend = TRUE;
    }

    /* try to open the log file if requested */
    if (cp->logfilename) {
	res = log_open((const char *)cp->logfilename,
		       cp->logappend, FALSE);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /*** ORDER DOES NOT MATTER FOR REST OF PARAMETERS ***/

    /* difftype parameter */
    res = ps_get_parmval(ps, YANGDIFF_PARM_DIFFTYPE, &val);
    if (res == NO_ERR) {
	cp->difftype = VAL_STR(val);
	if (!xml_strcmp(cp->difftype, YANGDIFF_DIFFTYPE_TERSE)) {
	    cp->edifftype = YANGDIFF_DT_TERSE;
	} else if (!xml_strcmp(cp->difftype, YANGDIFF_DIFFTYPE_BRIEF)) {
	    cp->edifftype = YANGDIFF_DT_BRIEF;
	} else if (!xml_strcmp(cp->difftype, YANGDIFF_DIFFTYPE_NORMAL)) {
	    cp->edifftype = YANGDIFF_DT_NORMAL;
	} else if (!xml_strcmp(cp->difftype, YANGDIFF_DIFFTYPE_REVISION)) {
	    cp->edifftype = YANGDIFF_DT_REVISION;
	} else {
	    cp->edifftype = YANGDIFF_DT_NONE;
	}
    } else {
	cp->difftype = YANGDIFF_DEF_DIFFTYPE;
	cp->edifftype = YANGDIFF_DEF_DT;
    }

    /* indent parameter */
    res = ps_get_parmval(ps, YANGDIFF_PARM_INDENT, &val);
    if (res == NO_ERR) {
	cp->indent = (int32)VAL_UINT(val);
    } else {
	cp->indent = NCX_DEF_INDENT;
    }

    /* help parameter */
    if (ps_find_parm(ps, NCX_EL_HELP)) {
	cp->helpmode = TRUE;
    }

    /* modpath parameter */
    res = ps_get_parmval(ps, NCX_EL_MODPATH, &val);
    if (res == NO_ERR) {
	ncxmod_set_modpath(VAL_STR(val));
    }

    /* old parameter */
    res = ps_get_parmval(ps, YANGDIFF_PARM_OLD, &val);
    if (res == NO_ERR) {
	cp->old = xml_strdup(VAL_STR(val));
	if (!cp->old) {
	    return ERR_INTERNAL_MEM;
	}
	cp->old_isdir = ncxmod_test_subdir((const char *)cp->old);	
    }

    /* new parameter */
    res = ps_get_parmval(ps, YANGDIFF_PARM_NEW, &val);
    if (res == NO_ERR) {
	cp->new = xml_strdup(VAL_STR(val));
	if (!cp->new) {
	    return ERR_INTERNAL_MEM;
	}
	cp->new_isdir = ncxmod_test_subdir((const char *)cp->new);
    }

    /* no-header parameter */
    if (ps_find_parm(ps, YANGDIFF_PARM_NO_HEADER)) {
	cp->noheader = TRUE;
    }

    /* no-subdirs parameter */
    if (ps_find_parm(ps, YANGDIFF_PARM_NO_SUBDIRS)) {
	cp->nosubdirs = TRUE;
    }

    /* output parameter */
    parm = ps_find_parm(ps, YANGDIFF_PARM_OUTPUT);
    if (parm) {
	/* output -- use filename provided */
	cp->output = VAL_STR(parm->val);
	cp->output_isdir = ncxmod_test_subdir((const char *)cp->output);
    } else {
	/* use default output -- STDOUT */
	cp->output = NULL;
    }

    /* version parameter */
    if (ps_find_parm(ps, NCX_EL_VERSION)) {
	cp->versionmode = TRUE;
    }

    return NO_ERR;

} /* process_cli_input */


/********************************************************************
 * FUNCTION main_init
 * 
 * 
 * 
 *********************************************************************/
static status_t 
    main_init (int argc,
	       const char *argv[])
{
    status_t       res;

    /* init module static variables */
    memset(&diffparms, 0x0, sizeof(yangdiff_diffparms_t));
    cli_ps = NULL;

    diffparms.buff = m__getMem(YANGDIFF_BUFFSIZE);
    if (!diffparms.buff) {
	return ERR_INTERNAL_MEM;
    } else {
	diffparms.bufflen = YANGDIFF_BUFFSIZE;
    }

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     * parm(TRUE) indicates all description clauses should be saved
     * Set debug cutoff filter to user errors
     */
    res = ncx_init(TRUE,
#ifdef DEBUG
		   LOG_DEBUG_INFO,
#else
		   LOG_DEBUG_WARN,
#endif
		   NULL);
    if (res == NO_ERR) {
	/* load in the NCX converter parmset definition file */
	res = ncxmod_load_module(YANGDIFF_MOD);
	if (res == NO_ERR) {
	    res = process_cli_input(argc, argv, &diffparms);
	}
    }

    if (res != NO_ERR && res != ERR_NCX_SKIPPED) {
	pr_err(res);
    }
    return res;

}  /* main_init */


/********************************************************************
 * FUNCTION main_cleanup
 * 
 * 
 * 
 *********************************************************************/
static void
    main_cleanup (void)
{
    if (cli_ps) {
	ps_free_parmset(cli_ps);
    }

    /* free the input parameters */
    if (diffparms.old) {
	m__free(diffparms.old);
    }
    if (diffparms.new) {
	m__free(diffparms.new);
    }
    if (diffparms.mod) {
	ncx_free_module(diffparms.mod);
    }
    if (diffparms.scb) {
	ses_free_scb(diffparms.scb);
    }
    if (diffparms.curold) {
	m__free(diffparms.curold);
    }
    if (diffparms.curnew) {
	m__free(diffparms.curnew);
    }
    if (diffparms.buff) {
	m__free(diffparms.buff);
    }

    /* cleanup the NCX engine and registries */
    ncx_cleanup();

    log_close();

}  /* main_cleanup */


/********************************************************************
*                                                                   *
*			FUNCTION main				    *
*                                                                   *
*********************************************************************/
int 
    main (int argc, 
	  const char *argv[])
{
    status_t    res;

    res = main_init(argc, argv);

    if (res == NO_ERR) {
	res = main_run();
    }

    /* if warnings+ are enabled, then res could be NO_ERR and still
     * have output to STDOUT
     */
    if (res == NO_ERR) {
	log_warn("\n");   /*** producing extra blank lines ***/
    }

    print_errors();
    main_cleanup();

    return (res == NO_ERR) ? 0 : 1;

} /* main */

/* END yangdiff.c */



