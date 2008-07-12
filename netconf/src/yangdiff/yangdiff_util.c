/*  FILE: yangdiff_util.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
10-jul-08    abb      split out from yangdiff.c

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

#ifndef _H_yangdiff_typ
#include  "yangdiff_typ.h"
#endif

#ifndef _H_yangdiff_util
#include  "yangdiff_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define YANGDIFF_UTIL_DEBUG   1


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION indent_in
*
* Move curindent to the right
*
* INPUTS:
*   cp == compare paraeters to use
*
*********************************************************************/
void
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
void
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
uint32
    str_field_changed (const xmlChar *fieldname,
		       const xmlChar *oldstr,
		       const xmlChar *newstr,
		       boolean isrev,
		       yangdiff_cdb_t *cdb)
{
    uint32  ret;

    ret = 0;

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
    } else if (cdb) {
	cdb->changed = FALSE;
	cdb->useval = NULL;
	cdb->chtyp = NULL;
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
uint32
    bool_field_changed (const xmlChar *fieldname,
		       boolean oldbool,
		       boolean newbool,
		       boolean isrev,
		       yangdiff_cdb_t *cdb)
{
    uint32 ret;

    ret = 0;
    if (cdb) {
	cdb->fieldname = fieldname;
	cdb->oldval = oldbool ? NCX_EL_TRUE : NCX_EL_FALSE;
	cdb->newval = newbool ? NCX_EL_TRUE : NCX_EL_FALSE;
    }

    if (oldbool != newbool) {
	ret = 1;
	if (cdb) {
	    cdb->changed = TRUE;
	    cdb->useval = cdb->newval;
	    cdb->chtyp = (isrev) ? MOD_STR : M_STR;
	}
    } else if (cdb) {
	cdb->changed = FALSE;
	cdb->useval = NULL;
	cdb->chtyp = NULL;
    }
    return ret;

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
uint32
    status_field_changed (const xmlChar *fieldname,
			  ncx_status_t oldstat,
			  ncx_status_t newstat,
			  boolean isrev,
			  yangdiff_cdb_t *cdb)
{
    uint32 ret;

    ret = 0;

    if (cdb) {
	cdb->fieldname = fieldname;
	cdb->oldval = ncx_get_status_string(oldstat);
	cdb->newval = ncx_get_status_string(newstat);
    }
    if (oldstat != newstat) {
	ret = 1;
	if (cdb) {
	    cdb->changed = TRUE;
	    cdb->useval = cdb->newval;
	    cdb->chtyp = (isrev) ? MOD_STR : M_STR;
	}
    } else if (cdb) {
	cdb->changed = FALSE;
	cdb->useval = NULL;
	cdb->chtyp = NULL;
    }
    return ret;

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
uint32
    prefix_field_changed (const ncx_module_t *oldmod,
			  const ncx_module_t *newmod,
			  const xmlChar *oldprefix,
			  const xmlChar *newprefix)
{
    const ncx_import_t *oldimp, *newimp;

    if (str_field_changed(NULL, oldprefix, 
			  newprefix, FALSE, NULL)) {
	if (oldprefix && newprefix) {
	    /* check if the different prefix values reference
	     * the same module name
	     */
	    oldimp = ncx_find_pre_import(oldmod, oldprefix);
	    newimp = ncx_find_pre_import(newmod, newprefix);
	    if (oldimp && newimp &&
		!xml_strcmp(oldimp->module, newimp->module)) {
		return 0;
	    } else {
		return 1;
	    }
	} else if (oldprefix && 
		   !xml_strcmp(oldprefix, oldmod->prefix)) {
	    return 0;  /* using own module prefix */
	} else if (newprefix && 
		   !xml_strcmp(newprefix, newmod->prefix)) {
	    return 0;  /* using own module prefix */
	} else {
	    return 1;
	}
    } else {
	return 0;
    }
    /*NOTREACHED*/

} /* prefix_field_changed */


/********************************************************************
 * FUNCTION output_val
 * 
 *  Output one value; will add truncate!
 *
 * INPUTS:
 *    cp == parameter block to use
 *    strval == str value to output
 *********************************************************************/
void
    output_val (yangdiff_diffparms_t *cp,
		const xmlChar *strval)
{
    uint32 i;

    ses_putchar(cp->scb, '\'');
    if (xml_strlen(strval) > cp->maxlen) {
	for (i=0; i<cp->maxlen; i++) {
	    ses_putchar(cp->scb, strval[i]);
	}
	ses_putstr(cp->scb, (const xmlChar *)"...");
    } else {
	ses_putstr(cp->scb, strval);
    }
    ses_putchar(cp->scb, '\'');

}  /* output_val */


/********************************************************************
 * FUNCTION output_mstart_line
 * 
 *  Output one line for a comparison where the change is 'modify'
 *  Just print the field name and identifier
 *
 * INPUTS:
 *    cp == parameter block to use
 *    fieldname == fieldname that is different to print
 *    val == string value to print (may be NULL)
 *    isid == TRUE if val is an ID
 *         == FALSE if val should be single-quoted
 *         (ignored in val == NULL)
 *********************************************************************/
void
    output_mstart_line (yangdiff_diffparms_t *cp,
			const xmlChar *fieldname,
			const xmlChar *val,
			boolean isid)
{
    ses_putstr_indent(cp->scb, 
		      (cp->edifftype==YANGDIFF_DT_REVISION) 
		      ? MOD_STR : M_STR, cp->curindent);
    ses_putstr(cp->scb, fieldname);
    if (val) {
	ses_putchar(cp->scb, ' ');
	if (isid) {
	    ses_putstr(cp->scb, val);
	} else {
	    output_val(cp, val);
	}
    }

}  /* output_mstart_line */


/********************************************************************
 * FUNCTION output_cdb_line
 * 
 *  Output one line for a comparison, from a change description block
 *
 * INPUTS:
 *    cp == parameter block to use
 *    cdb == change descriptor block to use
 *********************************************************************/
void
    output_cdb_line (yangdiff_diffparms_t *cp,
		     const yangdiff_cdb_t *cdb)
{
    boolean showval;

    if (!cdb->changed) {
	return;
    }

    showval = TRUE;

    switch (cp->edifftype) {
    case YANGDIFF_DT_TERSE:
	showval = FALSE;
	/* fall through */
    case YANGDIFF_DT_NORMAL:
    case YANGDIFF_DT_REVISION:
	ses_putstr_indent(cp->scb, cdb->chtyp, cp->curindent);
	ses_putstr(cp->scb, cdb->fieldname);
	if (showval) {
	    ses_putchar(cp->scb, ' ');
	    if (cdb->oldval && cdb->newval) {
		ses_putstr(cp->scb, FROM_STR);
		output_val(cp, cdb->oldval);
		ses_putchar(cp->scb, ' ');
		ses_putstr(cp->scb, TO_STR);
		output_val(cp, cdb->newval);
	    } else if (cdb->useval) {
		output_val(cp, cdb->useval);
	    } else {
		ses_putstr(cp->scb, (const xmlChar *)" ''");
	    }
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* output_cdb_line */


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
void
    output_diff (yangdiff_diffparms_t *cp,
		 const xmlChar *fieldname,
		 const xmlChar *oldval,
		 const xmlChar *newval,
		 boolean isid)
{
    const xmlChar *useval, *astr, *dstr, *mstr;
    boolean finish, useto;

    useval = NULL;
    useto = FALSE;
    finish = TRUE;

    if (cp->edifftype==YANGDIFF_DT_REVISION) {
	astr = ADD_STR;
	dstr = DEL_STR;
	mstr = MOD_STR;
    } else {
	astr = A_STR;
	dstr = D_STR;
	mstr = M_STR;
    }

    if (!oldval && newval) {
	ses_putstr_indent(cp->scb, astr, cp->curindent);
	useval = newval;
    } else if (oldval && !newval) {
	ses_putstr_indent(cp->scb, dstr, cp->curindent);
	useval = oldval;
    } else if (oldval && newval) {
	if (xml_strcmp(oldval, newval)) {
	    ses_putstr_indent(cp->scb, mstr, cp->curindent);
	    useto = TRUE;
	} else {
	    finish = FALSE;
	}
    } else {
	finish = FALSE;
    }

    if (finish) {
	ses_putstr(cp->scb, fieldname);
	if (useto) {
	    if (xml_strncmp(oldval, newval, cp->maxlen)) {
		ses_putchar(cp->scb, ' ');
		ses_putstr(cp->scb, FROM_STR);
		output_val(cp, oldval);
		ses_putchar(cp->scb, ' ');
		ses_putstr(cp->scb, TO_STR);
		output_val(cp, newval);
	    }
	} else {
	    ses_putchar(cp->scb, ' ');
	    if (isid) {
		ses_putstr(cp->scb, useval);
	    } else {
		output_val(cp, useval);
	    }
	}
    }

}  /* output_diff */


/********************************************************************
 * FUNCTION output_errinfo_diff
 * 
 *  Output the differences report for one ncx_errinfo_t struct
 *  Used for may clauses, such as must, range, length, pattern;
 *  within a leaf, leaf-list, or typedef definition
 *
 * INPUTS:
 *    cp == parameter block to use
 *    olderr == old err info struct (may be NULL)
 *    newerr == new err info struct (may be NULL)
 *
 *********************************************************************/
void
    output_errinfo_diff (yangdiff_diffparms_t *cp,
			 const ncx_errinfo_t *olderr,
			 const ncx_errinfo_t *newerr)
{
    if (!olderr && newerr) {
	/* errinfo added in new revision */
	output_diff(cp, YANG_K_ERROR_MESSAGE, NULL,
		    newerr->error_message, FALSE);
	output_diff(cp, YANG_K_ERROR_APP_TAG, NULL, 
		    newerr->error_app_tag, FALSE);
	output_diff(cp, YANG_K_DESCRIPTION, NULL, 
		    newerr->descr, FALSE);
	output_diff(cp, YANG_K_REFERENCE, NULL, 
		    newerr->ref, FALSE);
    } else if (olderr && !newerr) {
	/* errinfo removed in new revision */
	output_diff(cp, YANG_K_ERROR_MESSAGE,
		    olderr->error_message, NULL, FALSE);
	output_diff(cp, YANG_K_ERROR_APP_TAG,
		    olderr->error_app_tag, NULL, FALSE);
	output_diff(cp, YANG_K_DESCRIPTION,
		    olderr->descr, NULL, FALSE);
	output_diff(cp, YANG_K_REFERENCE,
		    newerr->ref, NULL, FALSE);
    } else if (olderr && newerr) {
	/* errinfo maybe changed in new revision */
	output_diff(cp, YANG_K_ERROR_MESSAGE,
		    olderr->error_message,
		    newerr->error_message, FALSE);
	output_diff(cp, YANG_K_ERROR_APP_TAG,
		    olderr->error_app_tag,
		    newerr->error_app_tag, FALSE);
	output_diff(cp, YANG_K_DESCRIPTION,
		    olderr->descr, newerr->descr, FALSE);
	output_diff(cp, YANG_K_REFERENCE,
		    olderr->ref, newerr->ref, FALSE);
    }
} /* output_errinfo_diff */


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
uint32
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

/* END yangdiff_util.c */
