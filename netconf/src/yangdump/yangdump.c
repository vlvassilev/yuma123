/*  FILE: yangdump.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
01-nov-06    abb      begun

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

#ifndef _H_cyang
#include  "cyang.h"
#endif

#ifndef _H_def_reg
#include  "def_reg.h"
#endif

#ifndef _H_help
#include  "help.h"
#endif

#ifndef _H_html
#include  "html.h"
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

#ifndef _H_sql
#include  "sql.h"
#endif

#ifndef _H_status
#include  "status.h"
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

#ifndef _H_xsd
#include  "xsd.h"
#endif

#ifndef _H_xsd_util
#include  "xsd_util.h"
#endif

#ifndef _H_yang
#include  "yang.h"
#endif

#ifndef _H_yangconst
#include  "yangconst.h"
#endif

#ifndef _H_yangdump
#include  "yangdump.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
/* #define YANGDUMP_DEBUG   1 */

/* this should match the buffer size in ncx/tk.h */
#define YANGDUMP_BUFFSIZE   0xffff

#define YANGDUMP_DEF_OUTPUT   "stdout"

#define YANGDUMP_DEF_CONFIG   (const xmlChar *)"/etc/yangdump.conf"

#define YANGDUMP_DEF_TOC      (const xmlChar *)"menu"

#define YANGDUMP_DEF_OBJVIEW  OBJVIEW_RAW

#define YANGDUMP_MOD          (const xmlChar *)"yangdump"
#define YANGDUMP_PARMSET      (const xmlChar *)"yangdump"

#define YANGDUMP_PARM_CONFIG        (const xmlChar *)"config"
#define YANGDUMP_PARM_DEFNAMES      (const xmlChar *)"defnames"
#define YANGDUMP_PARM_DEPENDENCIES  (const xmlChar *)"dependencies"
#define YANGDUMP_PARM_EXPORTS       (const xmlChar *)"exports"
#define YANGDUMP_PARM_FORMAT        (const xmlChar *)"format"
#define YANGDUMP_PARM_HTML_DIV      (const xmlChar *)"html-div"
#define YANGDUMP_PARM_HTML_TOC      (const xmlChar *)"html-toc"
#define YANGDUMP_PARM_IDENTIFIERS   (const xmlChar *)"identifiers"
#define YANGDUMP_PARM_INDENT        (const xmlChar *)"indent"
#define YANGDUMP_PARM_MODULE        (const xmlChar *)"module"
#define YANGDUMP_PARM_MODVERSION    (const xmlChar *)"modversion"
#define YANGDUMP_PARM_NO_SUBDIRS    (const xmlChar *)"no-subdirs"
#define YANGDUMP_PARM_NO_VERSIONNAMES (const xmlChar *)"no-versionnames"
#define YANGDUMP_PARM_OUTPUT        (const xmlChar *)"output"
#define YANGDUMP_PARM_OBJVIEW       (const xmlChar *)"objview"
#define YANGDUMP_PARM_XSD_SCHEMALOC (const xmlChar *)"xsd-schemaloc"
#define YANGDUMP_PARM_SIMURLS       (const xmlChar *)"simurls"
#define YANGDUMP_PARM_SUBTREE       (const xmlChar *)"subtree"
#define YANGDUMP_PARM_URLSTART      (const xmlChar *)"urlstart"
#define YANGDUMP_PARM_UNIFIED       (const xmlChar *)"unified"


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static ps_parmset_t       *cli_ps;
static yangdump_cvtparms_t   cvtparms;


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
    log_error("\n%s: Error exit (%s)\n", PROGNAME, msg);

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
	      "\nTry '%s --help' for usage details\n", PROGNAME);

} /* pr_usage */


/********************************************************************
* FUNCTION process_cli_input
*
* Process the param line parameters against the hardwired
* parmset for the yangdump program
*
* get all the parms and store them in the cvtparms struct
*
* INPUTS:
*    argc == argument count
*    argv == array of command line argument strings
*    cp == address of returned values
*
* OUTPUTS:
*    parmset values will be stored in *cvtparms if NO_ERR
*    errors will be written to STDOUT
*
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
static status_t
    process_cli_input (int argc,
		       const char *argv[],
		       yangdump_cvtparms_t  *cp)
{
    psd_template_t  *psd;
    ps_parmset_t    *ps;
    ps_parm_t       *parm;
    val_value_t     *val;
    ncx_node_t       dtyp;
    status_t         res;

    res = NO_ERR;
    ps = NULL;

    cp->buff = m__getMem(YANGDUMP_BUFFSIZE);
    if (!cp->buff) {
	return ERR_INTERNAL_MEM;
    }
    cp->bufflen = YANGDUMP_BUFFSIZE;

    /* find the parmset definition in the registry */
    dtyp = NCX_NT_PSD;
    psd = (psd_template_t *)
	def_reg_find_moddef(YANGDUMP_MOD, YANGDUMP_PARMSET, &dtyp);
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
    res = ps_get_parmval(ps, YANGDUMP_PARM_CONFIG, &val);
    if (res == NO_ERR) {
	/* try the specified config location */
	cp->config = VAL_STR(val);
	res = conf_parse_ps_from_filespec(cp->config, ps, TRUE, TRUE);
	if (res != NO_ERR) {
	    return res;
	}
    } else {
	/* try default config location */
	res = conf_parse_ps_from_filespec(YANGDUMP_DEF_CONFIG,
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
	cp->full_logfilename = ncx_get_source(VAL_STR(val));
	if (!cp->full_logfilename) {
	    return ERR_INTERNAL_MEM;
	}
    }
    if (ps_find_parm(ps, NCX_EL_LOGAPPEND)) {
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

    /* defnames parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_DEFNAMES);
    if (parm) {
	cp->defnames = TRUE;
    }

    /* dependencies parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_DEPENDENCIES);
    if (parm) {
	cp->dependencies = TRUE;
    }

    /* exports parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_EXPORTS);
    if (parm) {
	cp->exports = TRUE;
    }

    /* format parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_FORMAT);
    if (parm) {
	/* format -- use string provided */
	cp->format = ncx_get_cvttyp_enum((char *)VAL_STR(parm->val));
    } else {
	cp->format = NCX_CVTTYP_NONE;
    }

    /* identifiers parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_IDENTIFIERS);
    if (parm) {
	cp->identifiers = TRUE;
    }

    /* indent parameter */
    res = ps_get_parmval(ps, YANGDUMP_PARM_INDENT, &val);
    if (res == NO_ERR) {
	cp->indent = (int32)VAL_UINT(val);
    } else {
	cp->indent = NCX_DEF_INDENT;
    }

    /* help parameter */
    if (ps_find_parm(ps, NCX_EL_HELP)) {
	cp->helpmode = TRUE;
    }

    /* html-div parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_HTML_DIV);
    if (parm) {
	cp->html_div = TRUE;
    }

    /* html-toc parameter */
    res = ps_get_parmval(ps, YANGDUMP_PARM_HTML_TOC, &val);
    if (res == NO_ERR) {
	cp->html_toc = VAL_STR(val);
    } else {
	cp->html_toc = YANGDUMP_DEF_TOC;
    }

    /* modpath parameter */
    res = ps_get_parmval(ps, NCX_EL_MODPATH, &val);
    if (res == NO_ERR) {
	ncxmod_set_modpath(VAL_STR(val));
    }

    /* module parameter */
    cp->modcount = ps_parm_count(ps, YANGDUMP_PARM_MODULE);
    if (cp->modcount == 1) {
	res = ps_get_parmval(ps, YANGDUMP_PARM_MODULE, &val);
	if (res == NO_ERR) {
	    cp->module = strdup((const char *)VAL_STR(val));
	    if (!cp->module) {
		return ERR_INTERNAL_MEM;
	    }
	}
    }

    /* modversion parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_MODVERSION);
    if (parm) {
	cp->modversion = TRUE;
    }

    /* no-subdirs parameter */
    if (ps_find_parm(ps, YANGDUMP_PARM_NO_SUBDIRS)) {
	cp->nosubdirs = TRUE;
    }

    /* no-versionnames */
    if (ps_find_parm(ps, YANGDUMP_PARM_NO_VERSIONNAMES)) {
	cp->noversionnames = TRUE;
    }

    /* objview parameter */
    res = ps_get_parmval(ps, YANGDUMP_PARM_OBJVIEW, &val);
    if (res == NO_ERR) {
	cp->objview = (const char *)VAL_STR(val);
    } else {
	cp->objview = YANGDUMP_DEF_OBJVIEW;
    }

    /* output parameter */
    res = ps_get_parmval(ps, YANGDUMP_PARM_OUTPUT, &val);
    if (res == NO_ERR) {
	/* output -- use filename provided */
	cp->output = (const char *)VAL_STR(val);
	cp->full_output = ncx_get_source(VAL_STR(val));
	if (!cp->full_output) {
	    return ERR_INTERNAL_MEM;
	}
	cp->output_isdir = ncxmod_test_subdir(cp->full_output);
    }

    /* simurls parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_SIMURLS);
    if (parm) {
	cp->simurls = TRUE;
    }

    /* subtree parameter */
    res = ps_get_parmval(ps, YANGDUMP_PARM_SUBTREE, &val);
    if (res == NO_ERR) {
	cp->subtree = (const char *)VAL_STR(val);
    }

    /* unified parameter */
    parm = ps_find_parm(ps, YANGDUMP_PARM_UNIFIED);
    if (parm) {
	cp->unified = TRUE;
    }

    /* urlstart parameter */
    res = ps_get_parmval(ps, YANGDUMP_PARM_URLSTART, &val);
    if (res == NO_ERR) {
	cp->urlstart = VAL_STR(val);
    }

    /* version parameter */
    if (ps_find_parm(ps, NCX_EL_VERSION)) {
	cp->versionmode = TRUE;
    }

    /* xsd-schemaloc parameter */
    res = ps_get_parmval(ps, YANGDUMP_PARM_XSD_SCHEMALOC, &val);
    if (res == NO_ERR) {
	cp->schemaloc = VAL_STR(val);
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
    memset(&cvtparms, 0x0, sizeof(yangdump_cvtparms_t));
    cli_ps = NULL;

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
	res = ncxmod_load_module(YANGDUMP_MOD);
	if (res == NO_ERR) {
	    res = process_cli_input(argc, argv, &cvtparms);
	}
    }

    if (res != NO_ERR && res != ERR_NCX_SKIPPED) {
	pr_err(res);
    }
    return res;

}  /* main_init */


/********************************************************************
 * FUNCTION get_output_session
 * 
 * Malloc and init an output session.
 * Open the correct output file if needed
 *
 * INPUTS:
 *    pcb == parser control block
 *    cp == conversion parameter struct to use
 *    res == address of return status
 *
 * OUTPUTS:
 *  *res == return status
 *
 * RETURNS:
 *   pointer to new session control block, or NULL if some error
 *********************************************************************/
static ses_cb_t *
    get_output_session (yang_pcb_t *pcb,
			yangdump_cvtparms_t *cp,
			status_t  *res)
{
    ncx_module_t    *mod;
    FILE            *fp;
    ses_cb_t        *scb;
    xmlChar         *namebuff;

    fp = NULL;
    scb = NULL;
    namebuff = NULL;
    *res = NO_ERR;

    mod = pcb->top;
    if (!mod) {
	*res = SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
    }

    /* open the output file if not STDOUT */
    if (*res == NO_ERR) {
	if ((cp->defnames && cp->format != NCX_CVTTYP_NONE) || 
	    (cp->output && cp->output_isdir)) {
	    namebuff = xsd_make_output_filename(mod, cp);
	    if (!namebuff) {
		*res = ERR_INTERNAL_MEM;
	    } else {
		fp = fopen((const char *)namebuff, "w");
		if (!fp) {
		    *res = ERR_FIL_OPEN;
		}
	    }
	} else if (cp->output) {
	    if (!cp->firstdone) {
		fp = fopen(cp->output, "w");
		cp->firstdone = TRUE;
	    } else if (cp->subtree) {
		fp = fopen(cp->output, "a");
	    } else {
		fp = fopen(cp->output, "w");
	    }
	    if (!fp) {
		*res = ERR_FIL_OPEN;
	    }
	}
    }

    /* get a dummy session control block */
    if (*res == NO_ERR) {
	scb = ses_new_dummy_scb();
	if (!scb) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    scb->fp = fp;
	    ses_set_mode(scb, SES_MODE_TEXT);
	}
    } 

    if (namebuff) {
	m__free(namebuff);
    }

    return scb;

}  /* get_output_session */


/********************************************************************
 * FUNCTION output_one_module_exports
 * 
 * Generate a list of top-level symbols for a module
 *
 * INPUTS:
 *    pcb == parser control block with module to use
 *    scb == session control block to use for output
 *    buff == scratch buffer to use 
 *********************************************************************/
static void
    output_one_module_exports (ncx_module_t *mod,
			       ses_cb_t *scb,
			       char *buff)
{
    typ_template_t    *typ;
    grp_template_t    *grp;
    obj_template_t    *obj;
    ext_template_t    *ext;
    rpc_template_t    *rpc;
    psd_template_t    *psd;

    if (mod->ismod) {
	sprintf(buff, "\nnamespace %s", mod->ns);
	ses_putstr(scb, (const xmlChar *)buff);
	if (mod->prefix) {
	    sprintf(buff, "\nprefix %s", mod->prefix);
	    ses_putstr(scb, (const xmlChar *)buff);
	} else {
	    sprintf(buff, "\nprefix %s", mod->name);
	    ses_putstr(scb, (const xmlChar *)buff);
	}
    } else {
	sprintf(buff, "\nbelongs-to %s", mod->belongs);
	ses_putstr(scb, (const xmlChar *)buff);
    }

    for (typ = (typ_template_t *)dlq_firstEntry(&mod->typeQ);
	 typ != NULL;
	 typ = (typ_template_t *)dlq_nextEntry(typ)) {
	sprintf(buff, "\ntypedef %s", typ->name);
	ses_putstr(scb, (const xmlChar *)buff);
    }

    for (grp = (grp_template_t *)dlq_firstEntry(&mod->groupingQ);
	 grp != NULL;
	 grp = (grp_template_t *)dlq_nextEntry(grp)) {
	sprintf(buff, "\ngrouping %s", grp->name);
	ses_putstr(scb, (const xmlChar *)buff);
    }

    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
	 obj != NULL;
	 obj = (obj_template_t *)dlq_nextEntry(obj)) {
	switch (obj->objtype) {
	case OBJ_TYP_AUGMENT:
	case OBJ_TYP_USES:
	    break;
	default:
	    if (obj_is_cloned(obj) && !obj_is_augclone(obj)) {
		break;
	    }
	    sprintf(buff, "\n%s %s", obj_get_typestr(obj),
		    obj_get_name(obj));
	    ses_putstr(scb, (const xmlChar *)buff);
	}
    }

    for (psd = (psd_template_t *)dlq_firstEntry(&mod->psdQ);
	 psd != NULL;
	 psd = (psd_template_t *)dlq_nextEntry(psd)) {
	sprintf(buff, "\nparmset %s", psd->name);
	ses_putstr(scb, (const xmlChar *)buff);
    }

    for (rpc = (rpc_template_t *)dlq_firstEntry(&mod->rpcQ);
	 rpc != NULL;
	 rpc = (rpc_template_t *)dlq_nextEntry(rpc)) {
	sprintf(buff, "\nrpc %s", rpc->name);
	ses_putstr(scb, (const xmlChar *)buff);
    }

    for (ext = (ext_template_t *)dlq_firstEntry(&mod->extensionQ);
	 ext != NULL;
	 ext = (ext_template_t *)dlq_nextEntry(ext)) {
	sprintf(buff, "\nextension %s", ext->name);
	ses_putstr(scb, (const xmlChar *)buff);
    }

    ses_putchar(scb, '\n');

}   /* output_one_module_exports */


/********************************************************************
 * FUNCTION output_module_exports
 * 
 * Generate a list of top-level symbols for a module
 *
 * INPUTS:
 *    pcb == parser control block with module to use
 *    cp == coversion param struct to use
 *    scb == session control block to use for output
 *********************************************************************/
static void
    output_module_exports (yang_pcb_t *pcb,
			   yangdump_cvtparms_t *cp,
			   ses_cb_t *scb)
{
    ncx_module_t      *mod;
    yang_node_t       *node;

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
	SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
	return;
    }


    ses_putstr(scb, (const xmlChar *)"\nexports:");

    if (pcb->subtree_mode) {
	if (mod->ismod) {
	    sprintf(cp->buff, "\nmodule %s", mod->name);
	} else {
	    sprintf(cp->buff, "\nsubmodule %s", mod->name);
	}
	ses_putstr(scb, (const xmlChar *)cp->buff);
	ses_putchar(scb, ' ');
	ses_putstr(scb, mod->version);
    }

    output_one_module_exports(mod, scb, cp->buff);

    if (cp->unified && mod->ismod) {
	for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		output_one_module_exports(node->submod, scb, cp->buff);
	    }
	}
    }

}   /* output_module_exports */


/********************************************************************
 * FUNCTION output_one_module_dependencies
 * 
 * Generate a list of dependencies for a module
 *
 * INPUTS:
 *    mod == module in progress
 *    scb == session control block to use for output
 *    cp == conversion parameters in use
 *********************************************************************/
static void
    output_one_module_dependencies (ncx_module_t *mod,
				    ses_cb_t *scb,
				    yangdump_cvtparms_t *cp)
{
    ncx_module_t      *testmod;
    ncx_import_t      *imp;
    yang_import_ptr_t *impptr;
    yang_node_t       *node; 

    if (mod->isyang) {
	for (impptr = (yang_import_ptr_t *)dlq_firstEntry(&mod->saveimpQ);
	     impptr != NULL;
	     impptr = (yang_import_ptr_t *)dlq_nextEntry(impptr)) {

	    testmod = def_reg_find_module(impptr->modname);
	    sprintf(cp->buff, "\nimport %s", impptr->modname);
	    ses_putstr(scb,(const xmlChar *)cp->buff);
	    if (testmod) {
		sprintf(cp->buff, " %s", testmod->version);
		ses_putstr(scb,(const xmlChar *)cp->buff);		
	    }
	}
	if (!cp->unified) {
	    for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
		 node != NULL;
		 node = (yang_node_t *)dlq_nextEntry(node)) {

		sprintf(cp->buff, "\ninclude %s", node->name);
		ses_putstr(scb,(const xmlChar *)cp->buff);
		if (node->submod) {
		    sprintf(cp->buff, " %s", node->submod->version);
		    ses_putstr(scb,(const xmlChar *)cp->buff);
		}
	    }
	}
    } else {
	for (imp = (ncx_import_t *)dlq_firstEntry(&mod->importQ);
	     imp != NULL;
	     imp = (ncx_import_t *)dlq_nextEntry(imp)) {

	    testmod = def_reg_find_module(imp->module);
	    sprintf(cp->buff, "\nimport %s", imp->module);
	    ses_putstr(scb,(const xmlChar *)cp->buff);	    
	    if (testmod) {
		sprintf(cp->buff, " %s", testmod->version);
		ses_putstr(scb,(const xmlChar *)cp->buff);
	    }
	}
    }

    ses_putchar(scb, '\n');

}   /* output_one_module_dependencies */


/********************************************************************
 * FUNCTION output_module_dependencies
 * 
 * Generate a list of dependencies for a module
 *
 * INPUTS:
 *    pcb == parser control block with module to use
 *    cp == conversion parameters to use
 *    scb == session control block to write output
 *********************************************************************/
static void
    output_module_dependencies (yang_pcb_t *pcb,
				yangdump_cvtparms_t *cp,
				ses_cb_t *scb)
{
    ncx_module_t      *mod;
    yang_node_t       *node; 

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
	SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
	return;
    }

    ses_putstr(scb, (const xmlChar *)"\ndependencies:");

    if (pcb->subtree_mode) {
	if (mod->ismod) {
	    sprintf(cp->buff, "\nmodule %s", mod->name);
	} else {
	    sprintf(cp->buff, "\nsubmodule %s", mod->name);
	}
	ses_putstr(scb,(const xmlChar *)cp->buff);
	ses_putchar(scb, ' ');
	ses_putstr(scb, mod->version);
    }

    output_one_module_dependencies(mod, scb, cp);

    if (cp->unified && mod->ismod) {
	for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		output_one_module_dependencies(node->submod, scb, cp);
	    }
	}
    }

}   /* output_module_dependencies */


/********************************************************************
 * FUNCTION output_identifiers
 * 
 * Generate a list of object identifiers for an object
 * YANG support only!!!
 *
 * INPUTS:
 *    datadefQ == Q of obj_template_t to use
 *    scb == session control block for output
 *    buff == scratch buffer to use 
 *    bufflen == size of buff in bytes
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    output_identifiers (dlq_hdr_t *datadefQ,
			ses_cb_t *scb,
			xmlChar *buff,
			uint32 bufflen)
{
    obj_template_t    *obj;
    dlq_hdr_t         *childQ;
    uint32             reallen;
    status_t           res;

    for (obj = (obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (obj_template_t *)dlq_nextEntry(obj)) {

	if (!obj_has_name(obj)) {
	    continue;  /* skip uses and augment */
	}

	res = obj_copy_object_id(obj, buff, bufflen, &reallen);
	if (res != NO_ERR) {
	    log_error("\nError: copy object ID failed (%s)",
		      get_error_string(res));
	    return res;
	}

	ses_putchar(scb, '\n');
	ses_putstr(scb, obj_get_typestr(obj));
	ses_putchar(scb, ' ');
	ses_putstr(scb, buff);

	childQ = obj_get_datadefQ(obj);
	if (childQ) {
	    res = output_identifiers(childQ, scb, buff, bufflen);
	    if (res != NO_ERR) {
		return res;
	    }
	}
    }

    return NO_ERR;

}   /* output_identifiers */


/********************************************************************
 * FUNCTION output_one_module_identifiers
 * 
 * Generate a list of object identifiers for a module
 * YANG support only!!!
 *
 * INPUTS:
 *    pcb == parser control block with module to use
 *    scb == session control block to use for output
 *    buff == scratch buffer to use 
 *    bufflen == size of buff in bytes
 *********************************************************************/
static void
    output_one_module_identifiers (ncx_module_t *mod,
				   ses_cb_t *scb,
				   xmlChar *buff,
				   uint32 bufflen)
{
    (void)output_identifiers(&mod->datadefQ, scb, buff, bufflen);
    ses_putchar(scb, '\n');

}   /* output_one_module_identifiers */


/********************************************************************
 * FUNCTION output_module_identifiers
 * 
 * Generate a list of object identifiers for a module
 * YANG support only!!!
 *
 * INPUTS:
 *    pcb == parser control block with module to use
 *    cp == conversion parameters to use
 *    scb == session control block for output
 *
 *********************************************************************/
static void
    output_module_identifiers (yang_pcb_t *pcb,
			       yangdump_cvtparms_t *cp,
			       ses_cb_t *scb)

{
    ncx_module_t      *mod;
    yang_node_t       *node; 

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
	SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
	return;
    }

    ses_putstr(scb, (const xmlChar *)"\nidentifiers:");

    if (pcb->subtree_mode) {
	if (mod->ismod) {
	    sprintf((char *)cp->buff, "\nmodule %s", mod->name);
	} else {
	    sprintf((char *)cp->buff, "\nsubmodule %s", mod->name);
	}
	ses_putstr(scb, (const xmlChar *)cp->buff);

	sprintf((char *)cp->buff, " %s", mod->version);
	ses_putstr(scb, (const xmlChar *)cp->buff);
    }

    output_one_module_identifiers(mod, scb,
				  (xmlChar *)cp->buff,
				  cp->bufflen);

    if (cp->unified && mod->ismod) {
	for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		output_one_module_identifiers(node->submod, scb, 
					      (xmlChar *)cp->buff,
					      cp->bufflen);
	    }
	}
    }

}   /* output_module_identifiers */


/********************************************************************
 * FUNCTION output_one_module_version
 * 
 * Generate the module or submodule version
 *
 * INPUTS:
 *    pcb == parser control block with module to use
 *    scb == session control block to use for output
 *    buff == scratch buffer to use 
 *********************************************************************/
static void
    output_one_module_version (ncx_module_t *mod,
			       ses_cb_t *scb,
			       char *buff)
{
    if (mod->ismod) {
	sprintf(buff, "\nmodule %s", mod->name);
    } else {
	sprintf(buff, "\nsubmodule %s", mod->name);
    }
    ses_putstr(scb, (const xmlChar *)buff);

    sprintf(buff, " %s", (mod->version) ? mod->version : NCX_EL_NONE);
    ses_putstr(scb, (const xmlChar *)buff);

    if (mod->source && LOGDEBUG2) {
	ses_putchar(scb, ' ');
	ses_putstr(scb, mod->source);
    }

}   /* output_one_module_version */


/********************************************************************
 * FUNCTION output_module_version
 * 
 * Generate the module version
 * YANG support only!!!
 *
 * INPUTS:
 *    pcb == parser control block with module to use
 *    cp == conversion parameters to use
 *    scb == session control block for output
 *********************************************************************/
static void
    output_module_version (yang_pcb_t *pcb,
			   yangdump_cvtparms_t *cp,
			   ses_cb_t *scb)
{
    ncx_module_t      *mod;
    yang_node_t       *node; 

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
	SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
	return;
    }

    ses_putstr(scb, (const xmlChar *)"\nmodversion:");
    output_one_module_version(mod, scb, cp->buff);

    if (cp->unified && mod->ismod) {
	for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		output_one_module_version(node->submod, scb, cp->buff);
	    }
	}
    }
    ses_putchar(scb, '\n');

}   /* output_module_version */


/********************************************************************
 * FUNCTION copy_module
 * 
 * Copy the source file to a different location and possibly
 * change the filename
 *
 * INPUTS:
 *    pcb == parser control block with module to use
 *    cp == conversion parameters to use
 *    scb == session control block for output
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    copy_module (yang_pcb_t *pcb,
		 yangdump_cvtparms_t *cp,
		 ses_cb_t *scb)
{
    ncx_module_t      *mod;
    FILE              *srcfile;
    boolean            done;

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
	return SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
    }

    /* open the YANG source file for reading */
    srcfile = fopen((const char *)mod->source, "r");
    if (!srcfile) {
	return SET_ERROR(ERR_NCX_MISSING_FILE);
    }

    done = FALSE;
    while (!done) {
	if (!fgets((char *)cp->buff, (int)cp->bufflen, srcfile)) {
	    /* read line failed, not an error */
	    done = TRUE;
	    continue;
	}
	ses_putstr(scb, (const xmlChar *)cp->buff);
    }

    fclose(srcfile);
    return NO_ERR;

}   /* copy_module */


/********************************************************************
 * FUNCTION convert_one
 * 
 *  Validate and then perhaps convert one module to the specified format
 *  The global params in cvtparms are used (should change that!)
 *
 * INPUTS:
 *    cp == parameter block to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    convert_one (yangdump_cvtparms_t *cp)
{
    ses_cb_t          *scb;
    val_value_t       *val;
    yang_pcb_t        *pcb;
    xmlChar           *namebuff;
    const xmlChar     *modname;
    xml_attrs_t        attrs;
    status_t           res;

    scb = NULL;
    res = NO_ERR;


    if (cp->modcount > 1) {
	modname = (const xmlChar *)cp->curmodule;
    } else {
	modname = (const xmlChar *)cp->module;
    }

    /* load in the requested module to convert */
    pcb = ncxmod_load_module_xsd(modname,
				 cp->subtree ? TRUE : FALSE,
				 cp->unified, &res);
    if (res == ERR_NCX_SKIPPED) {
	if (pcb) {
	    yang_free_pcb(pcb);
	}
	return NO_ERR;
    } else if (res != NO_ERR) {
	if (pcb && pcb->top) {
	    if (pcb->top->errors) {
		log_error("\n*** %s: %u Errors, %u Warnings\n", 
			  pcb->top->sourcefn,
			  pcb->top->errors, pcb->top->warnings);
	    } else if (pcb->top->warnings) {
		log_warn("\n*** %s: %u Errors, %u Warnings\n", 
			 pcb->top->sourcefn,
			 pcb->top->errors, pcb->top->warnings);
	    }
	} else {
	    /* make sure next task starts on a newline */
	    log_error("\n");
	}
	if (!pcb || !pcb->top || pcb->top->errors) {
	    if (pcb) {
		yang_free_pcb(pcb);
	    }
	    return res;
	} else {
	    /* just warnings reported */
	    res = NO_ERR;
	}
    } else if (LOGINFO && pcb && pcb->top) {
	log_info("\n*** %s: %u Errors, %u Warnings\n", 
		 pcb->top->sourcefn,
		 pcb->top->errors, pcb->top->warnings);
    }

    /* check if output session needed, any reports requestd or
     * any format except XSD and NONE means a session will be used
     * to write output.
     */
    if (cp->modversion || cp->exports || cp->dependencies || cp->identifiers
	|| (pcb->top->isyang &&
	    !(cp->format==NCX_CVTTYP_NONE || cp->format==NCX_CVTTYP_XSD))) {

	scb = get_output_session(pcb, cp, &res);
	if (!scb || res != NO_ERR) {
	    return res;
	}

	if (cp->indent != ses_indent_count(scb)) {
	    ses_set_indent(scb, cp->indent);
	}
    }

    /* check if modversion and other report modes */
    if (scb) {
	if (cp->modversion) {
	    output_module_version(pcb, cp, scb);
	}

	if (cp->exports) {
	    output_module_exports(pcb, cp, scb);
	}

	if (cp->dependencies) {
	    output_module_dependencies(pcb, cp, scb);
	}

	if (cp->identifiers) {
	    output_module_identifiers(pcb, cp, scb);
	}
    }

    /* check the type of translation requested */
    switch (cp->format) {
    case NCX_CVTTYP_NONE:
	if (ncx_any_dependency_errors(pcb->top)) {
	    log_warn("\nWarning: one or more modules imported into '%s' "
		     "had errors", pcb->top->sourcefn);
	}
	log_debug2("\nFile '%s' compiled without errors",
		   pcb->top->sourcefn);
	break;
    case NCX_CVTTYP_XSD:
	if (ncx_any_dependency_errors(pcb->top)) {
	    log_error("\nError: one or more imported modules had errors."
		      "\n       XSD conversion of '%s' terminated.",
		      pcb->top->sourcefn);
	    res = ERR_NCX_IMPORT_ERRORS;
	    ncx_print_errormsg(NULL, pcb->top, res);
	} else {
	    val = NULL;
	    xml_init_attrs(&attrs);
	    res = xsd_convert_module(pcb, cp, &val, &attrs);
	    if (res == NO_ERR) {
		if (cp->defnames || (cp->output && cp->output_isdir)) {
		    namebuff = xsd_make_output_filename(pcb->top, cp);
		    if (!namebuff) {
			res = ERR_INTERNAL_MEM;
		    } else {
			/* output to the specified file or STDOUT */
			res = xml_wr_file(namebuff, val, &attrs, 
					  DOCMODE, WITHHDR, cp->indent);
			m__free(namebuff);
		    }
		} else {
		    /* output to the specified file or STDOUT */
		    res = xml_wr_file((const xmlChar *)cp->output, 
				      val, &attrs, DOCMODE, WITHHDR, 
				      cp->indent);
		}
	    }
	    if (res != NO_ERR) {
		pr_err(res);
	    }
	    xml_clean_attrs(&attrs);
	}
	break;
    case NCX_CVTTYP_SQL:
	res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	pr_err(res);
	break;
    case NCX_CVTTYP_SQLDB:
	if (ncx_any_dependency_errors(pcb->top)) {
	    log_error("\nError: one or more imported modules had errors."
		      "\n       SQL object database conversion of '%s' terminated.",
		      pcb->top->sourcefn);
	    res = ERR_NCX_IMPORT_ERRORS;
	    ncx_print_errormsg(NULL, pcb->top, res);
	} else {
	    if (pcb->top->isyang) {
		res = sql_convert_module(pcb, cp, scb);
		if (res != NO_ERR) {
		    pr_err(res);
		}
	    } else {
		log_error("\nError: '%s' skipped, "
			 "SQL translation not enabled",
			 pcb->top->sourcefn);
		res = ERR_NCX_OPERATION_NOT_SUPPORTED;
		ncx_print_errormsg(NULL, pcb->top, res);
	    }
	}
	break;
    case NCX_CVTTYP_COPY:
	if (ncx_any_dependency_errors(pcb->top)) {
	    log_error("\nError: one or more imported modules had errors."
		      "\n       Copy source for '%s' terminated.",
		      pcb->top->sourcefn);
	    res = ERR_NCX_IMPORT_ERRORS;
	    ncx_print_errormsg(NULL, pcb->top, res);
	} else {
	    if (pcb->top->isyang) {
		res = copy_module(pcb, cp, scb);
		if (res != NO_ERR) {
		    pr_err(res);
		}
	    } else {
		log_error("\nError: '%s' skipped, "
			 "Copy mode not enabled for NCX files",
			 pcb->top->sourcefn);
		res = ERR_NCX_OPERATION_NOT_SUPPORTED;
		ncx_print_errormsg(NULL, pcb->top, res);
	    }
	}
	break;
    case NCX_CVTTYP_HTML:
	if (ncx_any_dependency_errors(pcb->top)) {
	    log_error("\nError: one or more imported modules had errors."
		      "\n       HTML conversion of '%s' terminated.",
		      pcb->top->sourcefn);
	    res = ERR_NCX_IMPORT_ERRORS;
	    ncx_print_errormsg(NULL, pcb->top, res);
	} else {
	    if (pcb->top->isyang) {
		res = html_convert_module(pcb, cp, scb);
		if (res != NO_ERR) {
		    pr_err(res);
		}
	    } else {
		log_error("\nError: '%s' skipped, HTML translation not supported",
			 pcb->top->sourcefn);
		res = ERR_NCX_OPERATION_NOT_SUPPORTED;
		ncx_print_errormsg(NULL, pcb->top, res);
	    }
	}
	break;
    case NCX_CVTTYP_H:
	res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	pr_err(res);
	break;
    case NCX_CVTTYP_YANG:
	if (ncx_any_dependency_errors(pcb->top)) {
	    log_error("\nError: one or more imported modules had errors."
		      "\n       YANG conversion of '%s' terminated.",
		      pcb->top->sourcefn);
	    res = ERR_NCX_IMPORT_ERRORS;
	    ncx_print_errormsg(NULL, pcb->top, res);
	} else {
	    if (pcb->top->isyang) {
		res = cyang_convert_module(pcb, cp, scb);
		if (res != NO_ERR) {
		    pr_err(res);
		}
	    } else {
		log_error("\nError: '%s' skipped, YANG translation "
			  "not supported", pcb->top->sourcefn);
		res = ERR_NCX_OPERATION_NOT_SUPPORTED;
		ncx_print_errormsg(NULL, pcb->top, res);
	    }
	}
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
	pr_err(res);
    }

    if (res != NO_ERR || LOGDEBUG2) {
	if (pcb && pcb->top) {
	    log_write("\n*** %s: %u Errors, %u Warnings\n", 
		      pcb->top->sourcefn,
		      pcb->top->errors, pcb->top->warnings);
	} else if (res != NO_ERR) {
	    log_write("\n");
	}
    }

    if (pcb) {
	yang_free_pcb(pcb);
    }

    if (scb) {
	ses_free_scb(scb);
    }

    return res;

}  /* convert_one */


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
    yangdump_cvtparms_t *cp;
    status_t    res;
    
    cp = cookie;

    if (cp->module) {
	m__free(cp->module);
    }
    cp->module = (char *)ncx_get_source((const xmlChar *)fullspec);
    if (!cp->module) {
	return ERR_INTERNAL_MEM;
    }

    log_debug2("\nStart subtree file:\n%s\n", fullspec);
    res = convert_one(cp);
    if (res != NO_ERR) {
	if (!NEED_EXIT) {
	    res = NO_ERR;
	}
    }

    return res;

}  /* subtree_callback */


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
    if (cvtparms.mod) {
	ncx_free_module(cvtparms.mod);
    }
    if (cvtparms.srcfile) {
	m__free(cvtparms.srcfile);
    }
    if (cvtparms.module) {
	m__free(cvtparms.module);
    }
    if (cvtparms.full_output) {
	m__free(cvtparms.full_output);
    }
    if (cvtparms.full_logfilename) {
	m__free(cvtparms.full_logfilename);
    }
    if (cvtparms.buff) {
	m__free(cvtparms.buff);
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
    ps_parm_t  *parm;
    status_t    res;

    res = main_init(argc, argv);

    if (res == NO_ERR) {
	if (cvtparms.versionmode) {
	    log_write("yangdump %s\n", YANGDUMP_PROGVER);
	}
	if (cvtparms.helpmode) {
	    help_program_module(YANGDUMP_MOD, YANGDUMP_PARMSET, FULL);
	}
	if (!(cvtparms.helpmode || cvtparms.versionmode)) {
	    /* check if subdir search suppression is requested */
	    if (cvtparms.nosubdirs) {
		ncxmod_set_subdirs(FALSE);
	    }

	    /* convert one file or N files or 1 subtree */
	    if (cvtparms.modcount==1 && cvtparms.module) {
		res = convert_one(&cvtparms);
	    } else if (cvtparms.modcount > 1) {
		res = NO_ERR;
		parm = ps_find_parm(cli_ps, YANGDUMP_PARM_MODULE);
		while (parm && res==NO_ERR) {
		    cvtparms.curmodule = (const char *)VAL_STR(parm->val);
		    res = convert_one(&cvtparms);
		    if (!NEED_EXIT) {
			parm = ps_next_parm_instance(parm);
		    } else {
			parm = NULL;
		    }
		}
	    } else if (cvtparms.subtree) {
		if (cvtparms.format == NCX_CVTTYP_XSD ||
		    cvtparms.format == NCX_CVTTYP_HTML ||
		    cvtparms.format == NCX_CVTTYP_YANG ||
		    cvtparms.format == NCX_CVTTYP_COPY) {
		    /* force separate file names in subtree mode */
		    cvtparms.defnames = TRUE;
		}
		res = ncxmod_process_subtree(cvtparms.subtree,
					     subtree_callback,
					     &cvtparms);
	    } else {
		res = ERR_NCX_MISSING_PARM;
		log_error("\nyangdump: Error: missing parameter (%s or %s)\n",
			  NCX_EL_MODULE, NCX_EL_SUBTREE);
	    }
	}
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

/* END yangdump.c */



