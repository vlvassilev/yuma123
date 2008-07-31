/*  FILE: yangdiff.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
10-jun-08    abb      begun; used yangdump.c as a template
31-jul-08    abb      convert from NCX based CLI to YANG based CLI

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

#ifndef _H_cli
#include  "cli.h"
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

#ifndef _H_xml_util
#include  "xml_util.h"
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

#ifndef _H_yangdiff_grp
#include  "yangdiff_grp.h"
#endif

#ifndef _H_yangdiff_obj
#include  "yangdiff_obj.h"
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
#define YANGDIFF_DEBUG   1


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static val_value_t           *cli_val;
static yangdiff_diffparms_t   diffparms;


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
		output_mstart_line(cp, YANG_K_IMPORT, oldimp->module, TRUE);
		if (cp->edifftype != YANGDIFF_DT_TERSE) {
		    indent_in(cp);
		    output_diff(cp, YANG_K_PREFIX,
				oldimp->prefix, newimp->prefix, TRUE);
		    indent_out(cp);
		}
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
		output_mstart_line(cp, YANG_K_REVISION, oldrev->version, FALSE);
		if (cp->edifftype != YANGDIFF_DT_TERSE) {
		    indent_in(cp);
		    output_cdb_line(cp, &cdb);
		    indent_out(cp);
		}
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
    case YANGDIFF_DT_NORMAL:
    case YANGDIFF_DT_REVISION:
	output_mstart_line(cp, YANG_K_EXTENSION, oldext->name, TRUE);
	if (cp->edifftype != YANGDIFF_DT_TERSE) {
	    indent_in(cp);
	    for (i=0; i<5; i++) {
		if (extcdb[i].changed) {
		    output_cdb_line(cp, &extcdb[i]);
		}
	    }
	    indent_out(cp);
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
	output_diff(cp, oldpcb->top->ismod ? 
		    YANG_K_MODULE : YANG_K_SUBMODULE, 
		    oldpcb->top->name, newpcb->top->name, TRUE);
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

    ses_putstr(cp->scb, (const xmlChar *)"\n// old: ");
    ses_putstr(cp->scb, oldpcb->top->name);
    ses_putchar(cp->scb, ' ');
    ses_putchar(cp->scb, '(');
    ses_putstr(cp->scb, oldpcb->top->version);
    ses_putchar(cp->scb, ')');
    ses_putchar(cp->scb, ' ');
    ses_putstr(cp->scb,
	       (cp->old_isdir) ? oldpcb->top->source 
	       : oldpcb->top->sourcefn);

    ses_putstr(cp->scb, (const xmlChar *)"\n// new: ");
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

    /* all extensions */
    output_extension_diff(cp, oldpcb, newpcb);

    /* global typedefs */
    output_typedefQ_diff(cp, &oldpcb->top->typeQ, 
			 &newpcb->top->typeQ);

    /* global groupings */
    output_groupingQ_diff(cp, &oldpcb->top->groupingQ, 
			  &newpcb->top->groupingQ);

    /* global data definitions */
    output_datadefQ_diff(cp, &oldpcb->top->datadefQ, 
			 &newpcb->top->datadefQ);

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

    ses_putchar(cp->scb, '\n');

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
    const xmlChar   *newstart;
    uint32           len, pslen;

    pslen = 0;

    /* setup source filespec pointer */
    newstart = newfn;
    if (cp->new_isdir) {
	newstart += xml_strlen(cp->full_new);
    }

    /* check length: subtree mode vs 1 file mode */
    if (cp->old_isdir) {
	len = xml_strlen(cp->full_old);
	if ((cp->full_old[len-1] != NCXMOD_PSCHAR) && 
	    (*newstart != NCXMOD_PSCHAR)) {
	    pslen = 1;
	}
	len += xml_strlen(newstart);
    } else {
	len = xml_strlen(cp->full_old);
    }

    /* get a buffer */
    buff = m__getMem(len+pslen+1);
    if (!buff) {
	return NULL;
    }

    /* fill in buffer: subtree mode vs 1 file mode */
    if (cp->old_isdir) {
	p = buff;
	p += xml_strcpy(p, cp->full_old);
	if (pslen) {
	    *p++ = NCXMOD_PSCHAR;
	}
	xml_strcpy(p, newstart);
    } else {
	xml_strcpy(buff, cp->full_old);
    }

    return buff;

}   /* make_curold_filename */


/********************************************************************
 * FUNCTION compare_one
 * 
 *  Validate and then compare one module to another
 *
 * load in the requested 'new' module to compare
 * if this is a subtree call, then the curnew pointer
 * will be set, otherwise the 'new' pointer must be set
 *
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
    const xmlChar     *logsource;
    status_t           res;
    boolean            skipreport;

    res = NO_ERR;
    skipreport = FALSE;
    cp->curindent = 0;

    /* force modules to be reloaded */
    ncx_set_cur_modQ(&cp->newmodQ);

    newpcb = ncxmod_load_module_diff((cp->curnew) ? cp->curnew : cp->new,
				     /* subtree mode */
				     (cp->curnew) ? TRUE : FALSE,
				     FALSE  /* cp->unified */, 
				     (cp->curnew) ? cp->full_new : NULL,
				     &res);
    if (res == ERR_NCX_SKIPPED) {
	if (newpcb) {
	    yang_free_pcb(newpcb);
	}
	return NO_ERR;
    } else if (res != NO_ERR) {
	if (newpcb && newpcb->top) {
	    logsource = (LOGDEBUG) ? newpcb->top->source 
		: newpcb->top->sourcefn;
	    if (newpcb->top->errors) {
		log_error("\n*** %s: %u Errors, %u Warnings\n", 
			  logsource, newpcb->top->errors, 
			  newpcb->top->warnings);
	    } else if (newpcb->top->warnings) {
		log_warn("\n*** %s: %u Errors, %u Warnings\n", 
			 logsource, newpcb->top->errors, 
			 newpcb->top->warnings);
	    }
	} else {
	    /* make sure next task starts on a newline */
	    log_error("\n");   
	}

	if (!newpcb || !newpcb->top || newpcb->top->errors) {
	    if (newpcb) {
		yang_free_pcb(newpcb);
	    }
	    return res;
	}  else {
	    /* just warnings reported */
	    res = NO_ERR;
	}
    } else if (LOGDEBUG && newpcb && newpcb->top) {
	log_debug("\n*** %s: %u Errors, %u Warnings\n", 
		   newpcb->top->source,
		   newpcb->top->errors, newpcb->top->warnings);
    }

    /* figure out where to get the requested 'old' file */
    cp->curold = make_curold_filename((cp->new_isdir) ? 
				      cp->curnew : newpcb->top->sourcefn, cp);
    if (!cp->curold) {
	res = ERR_INTERNAL_MEM;
	ncx_print_errormsg(NULL, NULL, res);
	yang_free_pcb(newpcb);
	return res;
    }

    /* force modules to be reloaded */
    ncx_set_cur_modQ(&cp->oldmodQ);

    /* load in the requested 'old' module to compare
     * if this is a subtree call, then the curnew pointer
     * will be set, otherwise the 'new' pointer must be set
     */
    oldpcb = ncxmod_load_module_diff(cp->curold, 
				     (cp->new_isdir) ? TRUE : FALSE,
				     FALSE  /* cp->unified */,
				     (cp->old_isdir) ? cp->full_old : NULL,
				     &res);
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
	if (oldpcb && oldpcb->top) {
	    logsource = (LOGDEBUG) ? oldpcb->top->source 
		: oldpcb->top->sourcefn;

	    if (oldpcb->top->errors) {
		log_error("\n*** %s: %u Errors, %u Warnings\n", 
			  logsource, oldpcb->top->errors, 
			  oldpcb->top->warnings);
	    } else if (oldpcb->top->warnings) {
		log_warn("\n*** %s: %u Errors, %u Warnings\n", 
			 logsource, oldpcb->top->errors, 
			 oldpcb->top->warnings);
	    }
	} else {
	    /* make sure next task starts on a newline */
	    log_error("\n");
	}

	if (!oldpcb || !oldpcb->top || oldpcb->top->errors) {
	    if (newpcb) {
		yang_free_pcb(newpcb);
	    }
	    if (oldpcb) {
		yang_free_pcb(oldpcb);
	    }
	    return res;
	}  else {
	    /* just warnings reported */
	    res = NO_ERR;
	}
	return res;
    } else if (LOGDEBUG && oldpcb && oldpcb->top) {
	log_debug("\n*** %s: %u Errors, %u Warnings\n", 
		  oldpcb->top->source,
		  oldpcb->top->errors, oldpcb->top->warnings);
    }

    /* allow new modules to be available fore ncx_find_module */
    ncx_set_cur_modQ(&cp->newmodQ);

    /* check if old and new files parsed okay */
    if (ncx_any_dependency_errors(newpcb->top)) {
	log_error("\nError: one or more modules imported into new '%s' "
		  "had errors", newpcb->top->sourcefn);
	skipreport = TRUE;
    } else {
	cp->newmod = newpcb->top;
    }

    /* allow old modules to be available for ncx_find_module */
    ncx_set_cur_modQ(&cp->oldmodQ);

    if (ncx_any_dependency_errors(oldpcb->top)) {
	log_error("\nError: one or more modules imported into old '%s' "
		  "had errors", oldpcb->top->sourcefn);
	skipreport = TRUE;
    } else {
	cp->oldmod = oldpcb->top;
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
    cp->curnew = ncx_get_source((const xmlChar *)fullspec);
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
	help_program_module(YANGDIFF_MOD, YANGDIFF_CONTAINER, FULL);
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
* !!! Relies on CLI parmset 'cp' remaining valid until program cleanup !!!
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
    const obj_template_t  *obj;
    val_value_t           *valset, *val;
    ncx_node_t             dtyp;
    status_t               res;

    res = NO_ERR;
    valset = NULL;

    cp->buff = m__getMem(YANGDIFF_BUFFSIZE);
    if (!cp->buff) {
	return ERR_INTERNAL_MEM;
    }
    cp->bufflen = YANGDIFF_BUFFSIZE;

    cp->maxlen = YANGDIFF_DEF_MAXSIZE;

    /* find the parmset definition in the registry */
    dtyp = NCX_NT_OBJ;
    obj = (const obj_template_t *)
	def_reg_find_moddef(YANGDIFF_MOD, YANGDIFF_CONTAINER, &dtyp);
    if (!obj) {
	res = ERR_NCX_NOT_FOUND;
    }

    /* parse the command line against the PSD */
    if (res == NO_ERR) {
	valset = cli_parse(argc, argv, obj,
			   FULLTEST, PLAINMODE, TRUE, &res);
    }
    if (res != NO_ERR) {
	return res;
    } else if (!valset) {
	pr_usage();
	return ERR_NCX_SKIPPED;
    } else {
	cli_val = valset;
    }

    /* next get any params from the conf file */
    val = val_find_child(valset, NULL, YANGDIFF_PARM_CONFIG);
    if (val) {
	/* try the specified config location */
	cp->config = VAL_STR(val);
	res = conf_parse_val_from_filespec(cp->config, 
					   valset, TRUE, TRUE);
	if (res != NO_ERR) {
	    return res;
	}
    } else {
	/* try default config location */
	res = conf_parse_val_from_filespec(YANGDIFF_DEF_CONFIG,
					   valset, TRUE, FALSE);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the log-level parameter */
    val = val_find_child(valset, NULL, NCX_EL_LOGLEVEL);
    if (val) {
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
    val = val_find_child(valset, NULL, NCX_EL_LOG);
    if (val) {
	cp->logfilename = VAL_STR(val);
	cp->full_logfilename = ncx_get_source(VAL_STR(val));
	if (!cp->full_logfilename) {
	    return ERR_INTERNAL_MEM;
	}
    }
    val = val_find_child(valset, NULL, NCX_EL_LOGAPPEND);
    if (val) {
	cp->logappend = TRUE;
    }

    /* try to open the log file if requested */
    if (cp->full_logfilename) {
	res = log_open((const char *)cp->full_logfilename,
		       cp->logappend, FALSE);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /*** ORDER DOES NOT MATTER FOR REST OF PARAMETERS ***/

    /* difftype parameter */
    val = val_find_child(valset, NULL, YANGDIFF_PARM_DIFFTYPE);
    if (val) {
	cp->difftype = VAL_STR(val);
	if (!xml_strcmp(cp->difftype, YANGDIFF_DIFFTYPE_TERSE)) {
	    cp->edifftype = YANGDIFF_DT_TERSE;
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
    val = val_find_child(valset, NULL, YANGDIFF_PARM_INDENT);
    if (val) {
	cp->indent = (int32)VAL_UINT(val);
    } else {
	cp->indent = NCX_DEF_INDENT;
    }

    /* help parameter */
    val = val_find_child(valset, NULL, NCX_EL_HELP);
    if (val) {
	cp->helpmode = TRUE;
    }

    /* modpath parameter */
    val = val_find_child(valset, NULL, NCX_EL_MODPATH);
    if (val) {
	ncxmod_set_modpath(VAL_STR(val));
    }

    /* old parameter */
    val = val_find_child(valset, NULL, YANGDIFF_PARM_OLD);
    if (val) {
	cp->old = VAL_STR(val);
	cp->full_old = ncx_get_source(VAL_STR(val));
	if (!cp->full_old) {
	    return ERR_INTERNAL_MEM;
	} else {
	    cp->old_isdir = ncxmod_test_subdir(cp->full_old); 
	}
    }

    /* new parameter */
    val = val_find_child(valset, NULL, YANGDIFF_PARM_NEW);
    if (val) {
	cp->new = VAL_STR(val);
	cp->full_new = ncx_get_source(VAL_STR(val));
	if (!cp->full_new) {
	    return ERR_INTERNAL_MEM;
	} else {
	    cp->new_isdir = ncxmod_test_subdir(cp->full_new);
	}
    }

    /* no-header parameter */
    val = val_find_child(valset, NULL, YANGDIFF_PARM_NO_HEADER);
    if (val) {
	cp->noheader = TRUE;
    }

    /* no-subdirs parameter */
    val = val_find_child(valset, NULL, YANGDIFF_PARM_NO_SUBDIRS);
    if (val) {
	cp->nosubdirs = TRUE;
    }

    /* output parameter */
    val = val_find_child(valset, NULL, YANGDIFF_PARM_OUTPUT);
    if (val) {
	cp->output = VAL_STR(val);
	cp->full_output = ncx_get_source(VAL_STR(val));
	if (!cp->full_output) {
	    return ERR_INTERNAL_MEM;
	} else {
	    cp->output_isdir = ncxmod_test_subdir(cp->full_output);
	}
    } /* else use default output -- STDOUT */

    /* version parameter */
    val = val_find_child(valset, NULL, NCX_EL_VERSION);
    if (val) {
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
    dlq_createSQue(&diffparms.oldmodQ);
    dlq_createSQue(&diffparms.newmodQ);

    cli_val = NULL;

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
	/* load in the YANG converter CLI definition file */
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
    ncx_module_t  *mod;

    if (cli_val) {
	val_free_value(cli_val);
    }

    while (!dlq_empty(&diffparms.oldmodQ)) {
	mod = dlq_deque(&diffparms.oldmodQ);
	ncx_free_module(mod);
    }

    while (!dlq_empty(&diffparms.newmodQ)) {
	mod = dlq_deque(&diffparms.newmodQ);
	ncx_free_module(mod);
    }

    /* free the input parameters */
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
    if (diffparms.full_old) {
	m__free(diffparms.full_old);
    }
    if (diffparms.full_new) {
	m__free(diffparms.full_new);
    }
    if (diffparms.full_output) {
	m__free(diffparms.full_output);
    }
    if (diffparms.full_logfilename) {
	m__free(diffparms.full_logfilename);
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
    } else {
	pr_err(res);
    }

    print_errors();
    main_cleanup();

    return (res == NO_ERR) ? 0 : 1;

} /* main */

/* END yangdiff.c */
