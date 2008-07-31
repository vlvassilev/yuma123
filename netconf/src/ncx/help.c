/*  FILE: help.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
05oct07      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_help
#include "help.h"
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

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define HELP_DEBUG 1
#endif


/********************************************************************
 * FUNCTION dump_appinfoQ (sub-mode of show RPC)
 * 
 * show module=mod-name
 *
 * INPUTS:
 *    hdr == appinfo Q to show (initialized but may be empty)
 *    indent == start indent count
 *********************************************************************/
static void
    dump_appinfoQ (const dlq_hdr_t *hdr,
		   uint32 indent)
{
    const ncx_appinfo_t *appinfo;
    boolean   first;

    first = TRUE;
    for (appinfo = (const ncx_appinfo_t *)dlq_firstEntry(hdr);
	 appinfo != NULL;
	 appinfo = (const ncx_appinfo_t *)dlq_nextEntry(appinfo)) {

	if (first) {
	    help_write_lines((const xmlChar *)"Appinfo Queue:\n", indent, TRUE);
	    first = FALSE;
	}

	help_write_lines((const xmlChar *)"", indent+2, TRUE);
	if (appinfo->value) {
	    log_stdout("%s = %s", appinfo->name, appinfo->value);
	} else {
	    log_stdout("%s", appinfo->name);
	}
    }
    if (!first) {
	log_stdout("\n");
    }

} /* dump_appinfoQ */


/********************************************************************
 * FUNCTION dump_rpcQ (sub-mode of show RPC)
 * 
 * List all the RPC methods defined in the specified module
 *
 * INPUTS:
 *    hdr == Queue header of RPC methods to show
 *    fullreport == TRUE for full dump of each RPC
 *               == FALSE for 1 line report on each RPC
 *    indent == start indent count
 *********************************************************************/
static void
    dump_rpcQ (const dlq_hdr_t *hdr,
	       boolean fullreport,
	       uint32 indent)
{

    const obj_template_t *rpc;
    boolean               anyout;

    anyout = FALSE;
    for (rpc = (const obj_template_t *)dlq_firstEntry(hdr);
	 rpc != NULL;
	 rpc = (const obj_template_t *)dlq_nextEntry(rpc)) {

	if (rpc->objtype == OBJ_TYP_RPC) {
	    anyout = TRUE;
	    obj_dump_template(rpc, fullreport, 
			      (uint32)
			      ((fullreport) ? 0 : NCX_DEF_DUMP_LEVEL),
			      indent);
	}
    }

    if (anyout) {
	log_stdout("\n");
    }

} /* dump_rpcQ */


/********************************************************************
 * FUNCTION dump_mod_hdr (sub-mode of do_show_module(s)
 * 
 * show module=mod-name header info
 *
 * INPUTS:
 *    mod == module to show
 *
 *********************************************************************/
static void
    dump_mod_hdr  (const ncx_module_t *mod)
{

    /* dump some header info */
    log_stdout("\n\nModule: %s (%s)", mod->name, mod->version);
    log_stdout("\nNamespace: %s", (mod->ns) ?
	       (const char *)mod->ns : "(none)");
    log_stdout("\nSource: %s", mod->source);

} /* dump_mod_hdr */


/**********  E X T E R N A L   F U N C T I O N S  ****************/


/********************************************************************
* FUNCTION help_program_module
*
* Print the full help text for an entire program module to STDOUT
*
* INPUTS:
*    modname == module name without file suffix
*    cliname == name of CLI parmset within the modname module
*
*********************************************************************/
void
    help_program_module (const xmlChar *modname,
			 const xmlChar *cliname,
			 boolean full)
{
    ncx_module_t   *mod;
    obj_template_t *cli;
    ncx_node_t      dtyp;

    mod = ncx_find_module(modname);
    if (!mod) {
	log_error("\nhelp: Module '%s' not found", modname);
	SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
	return;
    }

    log_stdout("\n\n  Program %s", mod->name);
    log_stdout("\n\n  Usage:");
    log_stdout("\n\n    %s [parameters]", mod->name);
    log_stdout("\n\n  Parameters can be entered in any order, and have ");
    log_stdout("the form:");
    log_stdout("\n\n    [start] name separator [value]");
    log_stdout("\n\n  where:");
    log_stdout("\n\n    start == 0, 1, or 2 dashes (foo, -foo, --foo)");
    log_stdout("\n\n    name == parameter name"
	       "\n         Parameter name completion "
	       "will be attempted "
	       "\n         if a partial name is entered.");
    log_stdout("\n\n    separator == whitespace or equals sign "
	       "(foo=bar, foo bar)");
    log_stdout("\n\n    value == string value for the parameter.");
    log_stdout("\n         Strings with whitespace need to be double quoted "
	       "\n         (--foo=\"some string\")");

    log_stdout("\n\n  Description:\n");
    if (mod->descr) {
	help_write_lines(mod->descr, 4, TRUE);
    }

    if (cliname) {
	dtyp = NCX_NT_OBJ;
	cli = (obj_template_t *)
	    def_reg_find_moddef(mod->name, cliname, &dtyp);
	if (!cli) {
	    log_error("\nhelp: CLI Object %s not found", cliname);
	    SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
	    return;
	} else if (cli->objtype == OBJ_TYP_CONTAINER) {
	    log_stdout("\n\n  Command Line Parameters");
	    log_stdout("\n\n    Key:  type name [built-in-type] [default]");
	    log_stdout("\n          built-in YANG type is present for "
		       "derived types only\n");

	    obj_dump_datadefQ(obj_get_datadefQ(cli), full, 
			      (uint32)(full ? 0 : NCX_DEF_DUMP_LEVEL), 4);
	}
    }

    if (obj_any_rpcs(&mod->datadefQ)) {
	log_stdout("\n\n  Local Commands\n");
	dump_rpcQ(&mod->datadefQ, PARTIAL, 4);
    }

}  /* help_program_module */


/********************************************************************
* FUNCTION help_data_module
*
* Print the full help text for an entire data module to STDOUT
*
* INPUTS:
*    mod     == data module struct
*    full    == TRUE if a full report desired
*               FALSE if a partial report desired
*********************************************************************/
void
    help_data_module (const ncx_module_t *mod,
		      boolean full)
{

    dump_mod_hdr(mod);

    if (!full) {
	return;
    }

    if (mod->last_update) {
	log_stdout("\nLast Update: %s", mod->last_update);
    }
    if (mod->descr) {
	log_stdout("\nDescription:\n %s", mod->descr);
    }
    dump_rpcQ(&mod->datadefQ, full, 2);

    if (full) {
	dump_appinfoQ(&mod->appinfoQ, 2);
    }

}  /* help_data_module */


/********************************************************************
* FUNCTION help_type
*
* Print the full help text for a data type to STDOUT
*
* INPUTS:
*    typ     == type template struct
*    full    == TRUE if a full report desired
*               FALSE if a partial report desired
*********************************************************************/
void
    help_type (const typ_template_t *typ,
		      boolean full)
{
    log_stdout("\n  Type: %s", typ->name);
    log_stdout(" (%s)",
	       tk_get_btype_sym(typ_get_basetype
				((const typ_def_t *)&typ->typdef)));

    if (typ->descr) {
	log_stdout("\n Description: %s", typ->descr);
    }

    if (typ->condition) {
	log_stdout("\n Condition: %s", typ->condition);
    }

    if (typ->defval) {
	log_stdout("\n Default: %s", typ->defval);
    }

    if (typ->units) {
	log_stdout("\n Units: %s", typ->units);
    }

    /***/

    if (full) {
	dump_appinfoQ(&typ->typdef.appinfoQ, 1);
    }

}  /* help_type */


/********************************************************************
* FUNCTION help_object
*
* Print the full help text for a RPC method template to STDOUT
*
* INPUTS:
*    obj     == object template
*    full    == TRUE if a full report desired
*               FALSE if a partial report desired
*********************************************************************/
void
    help_object (const obj_template_t *obj,
		 boolean full)
{
    obj_dump_template(obj, full, 
		      (uint32)(full ? 0 : NCX_DEF_DUMP_LEVEL), 0);

}  /* help_object */


/********************************************************************
 * FUNCTION help_write_lines
 * 
 * write some indented output to STDOUT
 *
 * INPUTS:
 *    str == string to print; 'indent' number of spaces
 *           will be added to each new line
 *    indent == indent count
 *    startnl == TRUE if start with a newline, FALSE otherwise
 *********************************************************************/
void
    help_write_lines (const xmlChar *str,
		      uint32 indent,
		      boolean startnl)
{
    uint32  i;

    if (startnl) {
	log_stdout("\n");
	for (i=0; i<indent; i++) {
	    log_stdout(" ");
	}
    }

    while (*str) {
	log_stdout("%c", *str);
	if (*str++ == '\n') {
	    for (i=0; i<indent; i++) {
		log_stdout(" ");
	    }
	}
    }

} /* help_write_lines */


/* END file help.c */
