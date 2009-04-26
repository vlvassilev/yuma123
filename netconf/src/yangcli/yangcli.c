/*  FILE: yangcli.c

   NETCONF YANG-based CLI Tool

   See ./README for details

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
01-jun-08    abb      begun; started from ncxcli.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <libssh2.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

/* #define MEMORY_DEBUG 1 */

#ifdef MEMORY_DEBUG
#include <mcheck.h>
#endif

#include "libtecla.h"

#define _C_main 1

#ifndef _H_procdefs
#include "procdefs.h"
#endif

#ifndef _H_cli
#include "cli.h"
#endif

#ifndef _H_conf
#include "conf.h"
#endif

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_mgr
#include "mgr.h"
#endif

#ifndef _H_mgr_hello
#include "mgr_hello.h"
#endif

#ifndef _H_mgr_io
#include "mgr_io.h"
#endif

#ifndef _H_mgr_rpc
#include "mgr_rpc.h"
#endif

#ifndef _H_mgr_ses
#include "mgr_ses.h"
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

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_runstack
#include "runstack.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_val_util
#include "val_util.h"
#endif

#ifndef _H_var
#include "var.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xml_wr
#include "xml_wr.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yangcli
#include "yangcli.h"
#endif

#ifndef _H_yangcli_cmd
#include "yangcli_cmd.h"
#endif

#ifndef _H_yangcli_tab
#include "yangcli_tab.h"
#endif

#ifndef _H_yangcli_util
#include "yangcli_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define YANGCLI_DEBUG   1
#endif


/********************************************************************
*                                                                   *
*                          T Y P E S                                *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*             F O R W A R D   D E C L A R A T I O N S               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

/*****************  I N T E R N A L   V A R S  ****************/

/* TBD: use multiple agent control blocks, stored in this Q */
static dlq_hdr_t      agent_cbQ;

/* hack for now instead of lookup functions to get correct
 * agent processing context; later search by session ID
 */
static agent_cb_t    *cur_agent_cb = NULL;

/* yangcli.yang file used for quicker lookups */
static ncx_module_t  *yangcli_mod;

/* netconf.yang file used for quicker lookups */
static ncx_module_t  *netconf_mod;

/* need to save CLI parameters: other vars are back-pointers */
static val_value_t   *mgr_cli_valset;

/* true if running a script from the invocation and exiting */
static boolean         batchmode;

/* true if printing program help and exiting */
static boolean         helpmode;
static help_mode_t     helpsubmode;

/* true if printing program version and exiting */
static boolean         versionmode;

/* contains list of modules entered from CLI --modules parm */
static val_value_t    *modules;

/* name of script pased at invocation to auto-run */
static xmlChar        *runscript;

/* TRUE if runscript has been completed */
static boolean         runscriptdone;


/* Q of modtrs that have been loaded with 'mgrload' */
static dlq_hdr_t       mgrloadQ;


/*****************  C O N F I G   V A R S  ****************/

/* TRUE if OK to load modules automatically
 * FALSE if --no-autoload set by user
 * when agent connection is made, and module discovery is done
 * then this var controls whether the matching modules
 * will be loaded into this application automatically
 */
static boolean         autoload;

/* TRUE if OK to check for partial command names and parameter
 * names by the user.  First match (TBD: longest match!!)
 * will be used if no exact match found
 * FALSE if only exact match should be used
 */
static boolean         autocomp;

/* NCX_BAD_DATA_IGNORE to silently accept invalid input values
 * NCX_BAD_DATA_WARN to warn and accept invalid input values
 * NCX_BAD_DATA_CHECK to prompt user to keep or re-enter value
 * NCX_BAD_DATA_ERROR to prompt user to re-enter value
 */
static ncx_bad_data_t  baddata;

/* global connect param set, copied to agent connect parmsets */
static val_value_t   *connect_valset;

/* name of external CLI config file used on invocation */
static xmlChar        *confname;

/* the module to check first when no prefix is given and there
 * is no parent node to check;
 * usually set to module 'netconf'
 */
static xmlChar        *default_module;  

/* 0 for no timeout; N for N seconds message timeout */
static uint32          default_timeout;

/* FALSE to send PDUs in manager-specified order
 * TRUE to always send in correct canonical order
 */
static boolean         fixorder;

/* global log level for all logging except for direct STDOUT output */
static log_debug_t     log_level;

/* TRUE if append to existing log; FALSE to start a new log */
static boolean         logappend;

/* optional log filespec given at invocation to store output
 * user IO from the KBD will still be done, but errors and
 * warnings will be sent to the file, not STDOUT
 */
static xmlChar        *logfilename;

/* FALSE to skip optional nodes in do_fill
 * TRUE to check optional nodes in do_fill
 */
static boolean         optional;

/* default NETCONF test-option value */
static op_testop_t     testoption;

/* default NETCONF error-option value */
static op_errop_t      erroption;

/* default NETCONF with-defaults value */
static ncx_withdefaults_t  withdefaults;




/********************************************************************
* FUNCTION do_startup_screen
* 
*  Print the startup messages to the log and stdout output
* 
*********************************************************************/
static void
    do_startup_screen (void)
{
    logfn_t             logfn;
    boolean             imode;

    imode = interactive_mode();
    if (imode) {
	logfn = log_stdout;
    } else {
	logfn = log_write;
    }

    (*logfn)("\n  yangcli version %s",  YANGCLI_PROGVER);
    (*logfn)("\n  Copyright 2009, Andy Bierman\n");

    if (!imode) {
	return;
    }

    (*logfn)("\n  Type 'help' or 'help <command-name>' to get started");
    (*logfn)("\n    e.g., 'help help' or 'help connect'");
    (*logfn)("\n  Use the <tab> key for command and value completion");
    (*logfn)("\n  Use the <enter> key to accept the default value ");
    (*logfn)("in brackets");

    (*logfn)("\n\n  These escape sequences are available ");
    (*logfn)("when filling parameter values:");
    (*logfn)("\n\n\t?\thelp");
    (*logfn)("\n\t??\tfull help");
    (*logfn)("\n\t?s\tskip current parameter");
    (*logfn)("\n\t?c\tcancel current command");

    (*logfn)("\n\n  These assignment statements are available ");
    (*logfn)("when entering commands:");
    (*logfn)("\n\n\t$<varname> = <expr>\tLocal user variable assignment");
    (*logfn)("\n\t$$<varname> = <expr>\tGlobal user variable assignment");
    (*logfn)("\n\t@<filespec> = <expr>\tFile assignment");
    (*logfn)("\n\n  Refer to the user manual for more details\n");


}  /* do_startup_screen */


/********************************************************************
* FUNCTION free_agent_cb
* 
*  Clean and free an agent control block
* 
* INPUTS:
*    agent_cb == control block to free
*                MUST BE REMOVED FROM ANY Q FIRST
*
*********************************************************************/
static void
    free_agent_cb (agent_cb_t *agent_cb)
{

    modptr_t  *modptr;

    if (agent_cb->name) {
	m__free(agent_cb->name);
    }
    if (agent_cb->address) {
	m__free(agent_cb->address);
    }
    if (agent_cb->password) {
	m__free(agent_cb->password);
    }
    if (agent_cb->result_name) {
	m__free(agent_cb->result_name);
    }
    if (agent_cb->result_filename) {
	m__free(agent_cb->result_filename);
    }

    if (agent_cb->connect_valset) {
	val_free_value(agent_cb->connect_valset);
    }

    /* cleanup the user edit buffer */
    if (agent_cb->cli_gl) {
	(void)del_GetLine(agent_cb->cli_gl);
    }

    var_clean_varQ(&agent_cb->varbindQ);

    while (!dlq_empty(&agent_cb->modptrQ)) {
	modptr = (modptr_t *)dlq_deque(&agent_cb->modptrQ);
	free_modptr(modptr);
    }

    m__free(agent_cb);

}  /* free_agent_cb */


/********************************************************************
* FUNCTION new_agent_cb
* 
*  Malloc and init a new agent control block
* 
* INPUTS:
*    name == name of agent record
*
* RETURNS:
*   malloced agent_cb struct or NULL of malloc failed
*********************************************************************/
static agent_cb_t *
    new_agent_cb (const xmlChar *name)
{
    agent_cb_t  *agent_cb;
    boolean      err;
    int          retval;

    err = FALSE;
    agent_cb = m__getObj(agent_cb_t);
    if (agent_cb == NULL) {
	return NULL;
    }

    memset(agent_cb, 0x0, sizeof(agent_cb_t));
    dlq_createSQue(&agent_cb->varbindQ);
    dlq_createSQue(&agent_cb->modptrQ);

    /* set default agent flags to current settings */
    agent_cb->state = MGR_IO_ST_INIT;
    agent_cb->baddata = baddata;
    agent_cb->log_level = log_level;
    agent_cb->autoload = autoload;
    agent_cb->fixorder = fixorder;
    agent_cb->get_optional = optional;
    agent_cb->testoption = testoption;
    agent_cb->erroption = erroption;
    agent_cb->timeout = default_timeout;
    agent_cb->withdefaults = withdefaults;

    agent_cb->name = xml_strdup(name);
    if (agent_cb->name == NULL) {
	err = TRUE;
    } else {

	/* get a read line context with a history buffer 
	 * change later to not get allocated if batch mode active
	 */
	agent_cb->cli_gl = new_GetLine(YANGCLI_LINELEN, 
				       YANGCLI_HISTLEN);
	if (agent_cb->cli_gl == NULL) {
	    err = TRUE;
	} else {
	    /* setup the yangcli tab-completion function for libtecla */
	    retval = gl_customize_completion(agent_cb->cli_gl,
					     &agent_cb->completion_state,
					     yangcli_tab_callback);
	    if (retval != 0) {
		err = TRUE;
	    }
	}
    }

    if (err) {
	free_agent_cb(agent_cb);
	agent_cb = NULL;
    }

    return agent_cb;

}  /* new_agent_cb */


/********************************************************************
 * FUNCTION handle_config_assign
 * 
 * handle a user assignment of a config variable
 *
 * INPUTS:
 *   agent_cb == agent control block to use
 *   configval == value to set
 *  use 1 of:
 *   newval == value to use for changing 'configval' 
 *   newvalstr == value to use as string form 
 * 
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    handle_config_assign (agent_cb_t *agent_cb,
			  val_value_t *configval,
			  val_value_t *newval,
			  const xmlChar *newvalstr)
{
    const xmlChar         *usestr;
    xmlChar               *dupval;
    val_value_t           *testval;
    const obj_template_t  *testobj;
    status_t               res;
    log_debug_t            testloglevel;
    ncx_bad_data_t         testbaddata;
    op_testop_t            testop;
    op_errop_t             errop;
    ncx_num_t              testnum;

    res = NO_ERR;
    if (newval) {
	if (!typ_is_string(newval->btyp)) {
	    return ERR_NCX_WRONG_TYPE;
	}
	if (VAL_STR(newval) == NULL) {
	    return ERR_NCX_INVALID_VALUE;
	}
	usestr = VAL_STR(newval);
    } else if (newvalstr) {
	usestr = newvalstr;
    } else {
	log_error("\nError: NULL value in config assignment");
	return ERR_NCX_INVALID_VALUE;
    }

    if (!xml_strcmp(configval->name, YANGCLI_AGENT)) {
	/* should check for valid IP address!!! */
	if (val_need_quotes(usestr)) {
	    /* using this dumb test as a placeholder */
	    log_error("\nError: invalid hostname");
	} else {
	    /* save or update the connnect_valset */
	    testval = val_find_child(connect_valset,
				     NULL, YANGCLI_AGENT);
	    if (testval) {
		res = val_set_simval(testval,
				     testval->typdef,
				     testval->nsid,
				     testval->name,
				     usestr);
		if (res != NO_ERR) {
		    log_error("\nError: changing 'agent' failed");
		}
	    } else {
		testobj = obj_find_child(connect_valset->obj,
					 NULL, YANGCLI_AGENT);
		testval = val_make_simval(obj_get_ctypdef(testobj),
					  obj_get_nsid(testobj),
					  YANGCLI_AGENT,
					  usestr,
					  &res);
		if (testval) {
		    val_add_child(testval, connect_valset);
		}
	    }
	}
    } else if (!xml_strcmp(configval->name, YANGCLI_AUTOCOMP)) {
	if (ncx_is_true(usestr)) {
	    autocomp = TRUE;
	} else if (ncx_is_false(usestr)) {
	    autocomp = FALSE;
	} else {
	    log_error("\nError: value must be 'true' or 'false'");
	    res = ERR_NCX_INVALID_VALUE;
	}
    } else if (!xml_strcmp(configval->name, YANGCLI_BADDATA)) {
	testbaddata = ncx_get_baddata_enum(usestr);
	if (testbaddata != NCX_BAD_DATA_NONE) {
	    agent_cb->baddata = testbaddata;
	    baddata = testbaddata;
	} else {
	    log_error("\nError: value must be 'true' or 'false'");
	    res = ERR_NCX_INVALID_VALUE;
	}
    } else if (!xml_strcmp(configval->name, YANGCLI_DEF_MODULE)) {
	if (!ncx_valid_name2(usestr)) {
	    log_error("\nError: must be a valid module name");
	    res = ERR_NCX_INVALID_VALUE;
	} else {
	    /* save a copy of the string value */
	    dupval = xml_strdup(usestr);
	    if (dupval == NULL) {
		log_error("\nError: malloc failed");
		res = ERR_INTERNAL_MEM;
	    } else {
		if (default_module) {
		    m__free(default_module);
		}
		default_module = dupval;
	    }
	}
    } else if (!xml_strcmp(configval->name, YANGCLI_USER)) {
	if (!ncx_valid_name2(usestr)) {
	    log_error("\nError: must be a valid user name");
	    res = ERR_NCX_INVALID_VALUE;
	} else {
	    /* save or update the connnect_valset */
	    testval = val_find_child(connect_valset,
				     NULL, YANGCLI_USER);
	    if (testval) {
		res = val_set_simval(testval,
				     testval->typdef,
				     testval->nsid,
				     testval->name,
				     usestr);
		if (res != NO_ERR) {
		    log_error("\nError: changing user name failed");
		}
	    } else {
		testobj = obj_find_child(connect_valset->obj,
					 NULL, YANGCLI_USER);
		testval = val_make_simval(obj_get_ctypdef(testobj),
					  obj_get_nsid(testobj),
					  YANGCLI_USER,
					  usestr,
					  &res);
		if (testval) {
		    val_add_child(testval, connect_valset);
		}
	    }
	}
    } else if (!xml_strcmp(configval->name, YANGCLI_TEST_OPTION)) {
	if (!xml_strcmp(usestr, NCX_EL_NONE)) {
	    agent_cb->testoption = OP_TESTOP_NONE;
	    testop = OP_TESTOP_NONE;
	} else {	    
	    testop = op_testop_enum(usestr);
	    if (testop != OP_TESTOP_NONE) {
		agent_cb->testoption = testop;
		testoption = testop;
	    } else {
		log_error("\nError: must be a valid 'test-option'");
		log_error("\n       (none, test-then-set, set, "
			  "test-only)\n");
		res = ERR_NCX_INVALID_VALUE;
	    }
	}
    } else if (!xml_strcmp(configval->name, YANGCLI_ERROR_OPTION)) {
	if (!xml_strcmp(usestr, NCX_EL_NONE)) {
	    agent_cb->erroption = OP_ERROP_NONE;
	    erroption = OP_ERROP_NONE;
	} else {	    
	    errop = op_errop_id(usestr);
	    if (errop != OP_ERROP_NONE) {
		agent_cb->erroption = errop;
		erroption = errop;
	    } else {
		log_error("\nError: must be a valid 'error-option'");
		log_error("\n       (none, stop-on-error, "
			  "continue-on-error, rollback-on-error)\n");
		res = ERR_NCX_INVALID_VALUE;
	    }
	}
    } else if (!xml_strcmp(configval->name, YANGCLI_TIMEOUT)) {
	ncx_init_num(&testnum);
	res = ncx_decode_num(usestr,
			     NCX_BT_UINT32,
			     &testnum);
	if (res == NO_ERR) {
	    agent_cb->timeout = testnum.u;
	    default_timeout = testnum.u;
	} else {
	    log_error("\nError: must be valid uint32 value");
	}
	ncx_clean_num(NCX_BT_UINT32, &testnum);
    } else if (!xml_strcmp(configval->name, YANGCLI_OPTIONAL)) {
	if (ncx_is_true(usestr)) {
	    optional = TRUE;
	    agent_cb->get_optional = TRUE;
	} else if (ncx_is_false(usestr)) {
	    optional = FALSE;
	    agent_cb->get_optional = FALSE;
	} else {
	    log_error("\nError: value must be 'true' or 'false'");
	    res = ERR_NCX_INVALID_VALUE;
	}
    } else if (!xml_strcmp(configval->name, NCX_EL_LOGLEVEL)) {
	testloglevel = 
	    log_get_debug_level_enum((const char *)usestr);
	if (testloglevel == LOG_DEBUG_NONE) {
	    log_error("\nError: value must be valid log-level:");
	    log_error("\n       (off, error, warn, info, debug, debug2)\n");
	    res = ERR_NCX_INVALID_VALUE;
	} else {
	    agent_cb->log_level = testloglevel;
	    log_set_debug_level(testloglevel);
	}
    } else if (!xml_strcmp(configval->name, YANGCLI_FIXORDER)) {
	if (ncx_is_true(usestr)) {
	    fixorder = TRUE;
	    agent_cb->fixorder = TRUE;
	} else if (ncx_is_false(usestr)) {
	    fixorder = FALSE;
	    agent_cb->fixorder = FALSE;
	} else {
	    log_error("\nError: value must be 'true' or 'false'");
	    res = ERR_NCX_INVALID_VALUE;
	}
    } else if (!xml_strcmp(configval->name, NCX_EL_WITH_DEFAULTS)) {
	if (!xml_strcmp(usestr, NCX_EL_NONE)) {
	    withdefaults = NCX_WITHDEF_NONE;
	} else {
	    if (ncx_get_withdefaults_enum(usestr) == NCX_WITHDEF_NONE) {
		log_error("\nError: value must be 'none', "
			  "'report-all', 'trim', or 'explicit'");
		res = ERR_NCX_INVALID_VALUE;
	    } else {
		withdefaults = ncx_get_withdefaults_enum(usestr);
		agent_cb->withdefaults = withdefaults;
	    }
	}
    } else {
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* update the variable value for user access */
    if (res == NO_ERR) {
	if (newval) {
	    res = var_set_move(configval->name, 
			       xml_strlen(configval->name),
			       VAR_TYP_CONFIG, newval);
	} else {
	    res = var_set_from_string(configval->name,
				      newvalstr, 
				      VAR_TYP_CONFIG);
	}
	if (res != NO_ERR) {
	    log_error("\nError: set result for '%s' failed (%s)",
			  agent_cb->result_name, 
			  get_error_string(res));
	}
    }

    if (res == NO_ERR) {
	log_info("\nOK\n");
    }

    return res;

} /* handle_config_assign */


/********************************************************************
* FUNCTION handle_delete_result
* 
* Delete the specified file, if it is ASCII and a regular file
*
* INPUTS:
*    agent_cb == agent control block to use
*
* OUTPUTS:
*    agent_cb->result_filename will get deleted if NO_ERR
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    handle_delete_result (agent_cb_t *agent_cb)
{
    status_t     res;
    struct stat  statbuf;
    int          statresult;

    res = NO_ERR;

    if (LOGDEBUG2) {
	log_debug2("\n*** delete file result '%s'",
		   agent_cb->result_filename);
    }

    /* see if file already exists */
    statresult = stat((const char *)agent_cb->result_filename,
		      &statbuf);
    if (statresult != 0) {
	log_error("\nError: assignment file '%s' could not be opened",
		  agent_cb->result_filename);
	res = errno_to_status();
    } else if (!S_ISREG(statbuf.st_mode)) {
	log_error("\nError: assignment file '%s' is not a regular file",
		  agent_cb->result_filename);
	res = ERR_NCX_OPERATION_FAILED;
    } else {
	statresult = remove((const char *)agent_cb->result_filename);
	if (statresult == -1) {
	    log_error("\nError: assignment file '%s' could not be deleted",
		      agent_cb->result_filename);
	    res = errno_to_status();
	}
    }

    clear_result(agent_cb);

    return res;

}  /* handle_delete_result */


/********************************************************************
* FUNCTION output_file_result
* 
* Check the filespec string for a file assignment statement
* Save it if it si good
*
* INPUTS:
*    agent_cb == agent control block to use
* use 1 of these 2 parms:
*    resultval == result to output to file
*    resultstr == result to output as string
*
* OUTPUTS:
*    agent_cb->result_filename will get set if NO_ERR
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    output_file_result (agent_cb_t *agent_cb,
			val_value_t *resultval,
			const xmlChar *resultstr)
{
    FILE        *fil;
    status_t     res;
    xml_attrs_t  attrs;
    struct stat  statbuf;
    int          statresult;

    res = NO_ERR;

    if (LOGDEBUG2) {
	log_debug2("\n*** output file result to '%s'",
		   agent_cb->result_filename);
    }

    /* see if file already exists */
    statresult = stat((const char *)agent_cb->result_filename,
		      &statbuf);
    if (statresult == 0) {
	log_error("\nError: assignment file '%s' already exists",
		  agent_cb->result_filename);
	clear_result(agent_cb);
	return ERR_NCX_DATA_EXISTS;
    }
    
    if (resultval) {
	if (file_is_text(agent_cb->result_filename)) {
	    /* output in text format to the specified file */
	    res = log_alt_open((const char *)
			       agent_cb->result_filename);
	    if (res != NO_ERR) {
		log_error("\nError: assignment file '%s' could "
			  "not be opened (%s)",
			  agent_cb->result_filename,
			  get_error_string(res));
	    } else {
		val_dump_alt_value(resultval, 0);
		log_alt_close();
	    }
	} else {
	    /* output in XML format to the specified file */
	    xml_init_attrs(&attrs);
	    res = xml_wr_check_file(agent_cb->result_filename,
				    resultval,
				    &attrs, 
				    XMLMODE, 
				    WITHHDR, 
				    NCX_DEF_INDENT,
				    NULL);
	    xml_clean_attrs(&attrs);
	}
    } else {
	fil = fopen((const char *)agent_cb->result_filename, "w");
	if (fil == NULL) {
	    log_error("\nError: assignment file '%s' could "
		      "not be opened",
		      agent_cb->result_filename);
	    res = errno_to_status();
	} else {
	    statresult = fputs((const char *)resultstr, fil);
	    if (statresult == EOF) {
		log_error("\nError: assignment file '%s' could "
			  "not be written",
			  agent_cb->result_filename);
		res = errno_to_status();
	    } else {
		statresult = fputc('\n', fil);	
		if (statresult == EOF) {
		    log_error("\nError: assignment file '%s' could "
			      "not be written",
			      agent_cb->result_filename);
		    res = errno_to_status();
		}
	    }
	}
	statresult = fclose(fil);
	if (statresult == EOF) {
	    log_error("\nError: assignment file '%s' could "
		      "not be closed",
		      agent_cb->result_filename);
	    res = errno_to_status();
	}
    }

    if (res == NO_ERR) {
	log_info("\nOK\n");
    }

    clear_result(agent_cb);

    return res;

}  /* output_file_result */



/********************************************************************
* FUNCTION check_assign_statement
* 
* Check if the command line is an assignment statement
* 
* E.g.,
*
*   $foo = $bar
*   $foo = get-config filter=@filter.xml
*   $foo = "quoted string literal"
*   $foo = [<inline><xml/></inline>]
*   $foo = @datafile.xml
*
* INPUTS:
*   agent_cb == agent control block to use
*   line == command line string to expand
*   len  == address of number chars parsed so far in line
*   getrpc == address of return flag to parse and execute an
*            RPC, which will be assigned to the var found
*            in the 'result' module variable
*   fileassign == address of file assign flag
*
* OUTPUTS:
*   *len == number chars parsed in the assignment statement
*   *getrpc == TRUE if the rest of the line represent an
*              RPC command that needs to be evaluated
*   *fileassign == TRUE if the assignment is for a
*                  file output (starts with @)
*                  FALSE if not a file assignment
*
* SIDE EFFECTS:
*    If this is an 'rpc' assignment statement, 
*    (*dorpc == TRUE && *len > 0 && return NO_ERR),
*    then the global result and result_pending variables will
*    be set upon return.  For the other (direct) assignment
*    statement variants, the statement is completely handled
*    here.
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    check_assign_statement (agent_cb_t *agent_cb,
			    const xmlChar *line,
			    uint32 *len,
			    boolean *getrpc,
			    boolean *fileassign)
{
    const xmlChar         *str, *name, *filespec;
    val_value_t           *curval;
    const obj_template_t  *obj;
    val_value_t           *val;
    xmlChar               *tempstr;
    uint32                 nlen, tlen;
    var_type_t             vartype;
    status_t               res;

    /* save start point in line */
    str = line;
    *len = 0;
    *getrpc = FALSE;
    *fileassign = FALSE;
    vartype = VAR_TYP_NONE;
    curval = NULL;

    /* skip leading whitespace */
    while (*str && isspace(*str)) {
	str++;
    }

    if (*str == '@') {
	/* check if valid file assignment is being made */
	*fileassign = TRUE;
	str++;
    }

    if (*str == '$') {
	/* check if a valid variable assignment is being made */
	res = var_check_ref(str, ISLEFT, &tlen, 
			    &vartype, &name, &nlen);
	if (res != NO_ERR) {
	    /* error in the varref */
	    return res;
	} else if (tlen == 0) {
	    /* should not happen: returned not a varref */
	    *getrpc = TRUE;
	    return NO_ERR;
	} else if (*fileassign) {
	    /* file assignment complex form:
	     *
	     *    @$foo = bar or @$$foo = bar
	     *
	     * get the var reference for real because
	     * it is supposed to contain the filespec
	     * for the output file
	     */
	    curval = var_get_str(name, nlen, vartype);
	    if (curval == NULL) {
		log_error("\nError: file assignment variable "
			  "not found");
		return ERR_NCX_VAR_NOT_FOUND;
	    }

	    /* variable must be a string */
	    if (!typ_is_string(curval->btyp)) {
		log_error("\nError: file assignment variable '%s' "
			  "is wrong type '%s'",
			  curval->name,
			  tk_get_btype_sym(curval->btyp));
		return ERR_NCX_VAR_NOT_FOUND;
	    }
	    filespec = VAL_STR(curval);
	    res = check_filespec(agent_cb, filespec, curval->name);
	    if (res != NO_ERR) {
		return res;
	    }
	} else {
	    /* variable regerence:
	     *
	     *     $foo or $$foo
	     *
	     * check for a valid varref, get the data type, which
	     * will also indicate if the variable exists yet
	     */
	    switch (vartype) {
	    case VAR_TYP_SYSTEM:
		log_error("\nError: system variables "
			  "are read-only");
		return ERR_NCX_VAR_READ_ONLY;
	    case VAR_TYP_GLOBAL:
	    case VAR_TYP_CONFIG:
		curval = var_get_str(name, nlen, vartype);
		break;
	    case VAR_TYP_LOCAL:
	    case VAR_TYP_SESSION:
		curval = var_get_local_str(name, nlen);
		break;
	    default:
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
	/* move the str pointer past the variable name */
	str += tlen;
    } else if (*fileassign) {
	/* file assignment, simple form:
	 *
	 *     @foo.txt = bar
	 *
	 * get the length of the filename 
	 */
	name = str;
	while (*str && !isspace(*str) && *str != '=') {
	    str++;
	}
	nlen = (uint32)(str-name);

	/* save the filename in a temp string */
	tempstr = xml_strndup(name, nlen);
	if (tempstr == NULL) {
	    return ERR_INTERNAL_MEM;
	}

	/* check filespec and save filename for real */
	res = check_filespec(agent_cb, tempstr, NULL);

	m__free(tempstr);

	if (res != NO_ERR) {
	    return res;
	}
    } else {
	/* not an assignment statement at all */
	*getrpc = TRUE;
	return NO_ERR;
    }

    /* skip any more whitespace, after the LHS term */
    while (*str && xml_isspace(*str)) {
	str++;
    }

    /* check end of string */
    if (!*str) {
	log_error("\nError: truncated assignment statement");
	clear_result(agent_cb);
	return ERR_NCX_DATA_MISSING;
    }

    /* check for the equals sign assignment char */
    if (*str == NCX_ASSIGN_CH) {
	/* move past assignment char */
	str++;
    } else {
	log_error("\nError: equals sign '=' expected");
	clear_result(agent_cb);
	return ERR_NCX_WRONG_TKTYPE;
    }

    /* skip any more whitespace */
    while (*str && xml_isspace(*str)) {
	str++;
    }

    /* check end of string */
    if (!*str) {
	if (*fileassign) {
	    /* got file assignment (@foo) =  EOLN
	     * treat this as a request to delete the file
	     */
	    res = handle_delete_result(agent_cb);
	} else {
	    /* got $foo =  EOLN
	     * treat this as a request to unset the variable
	     */
	    if (vartype == VAR_TYP_SYSTEM ||
		vartype == VAR_TYP_CONFIG) {
		log_error("\nError: cannot remove system variables");
		clear_result(agent_cb);
		return ERR_NCX_OPERATION_FAILED;
	    }

	    /* else try to unset this variable */
	    res = var_unset(name, nlen, vartype);
	}
	*len = str - line;
	return res;
    }

    /* the variable name and equals sign is parsed
     * and the current char is either '$', '"', '<',
     * or a valid first name
     *
     *      $foo = blah
     *             ^
     */
    if (*fileassign) {
	obj = NULL;
    } else {
	obj = (curval) ? curval->obj : NULL;
    }

    /* get the script or CLI input as a new val_value_t struct */
    val = var_check_script_val(obj, str, ISTOP, &res);
    if (val) {
	/* a script value reference was found */
	if (obj==NULL || !xml_strcmp(val->name, NCX_EL_STRING)) {
	    /* the generic name needs to be overwritten */
	    val_set_name(val, name, nlen);
	}

	if (*fileassign) {
	    /* file assignment of a variable value 
	     *   @foo.txt=$bar  or @$foo=$bar
	     */
	    res = output_file_result(agent_cb, val, NULL);
	    val_free_value(val);
	} else {
	    /* this is a plain assignment statement
	     * first check if the input is VAR_TYP_CONFIG
	     */
	    if (vartype == VAR_TYP_CONFIG) {
		if (curval==NULL) {
		    res = SET_ERROR(ERR_INTERNAL_VAL);
		} else {
		    res = handle_config_assign(agent_cb,
					       curval, 
					       val,
					       NULL);
		}
	    } else {
		/* val is a malloced struct, pass it over to the
		 * var struct instead of cloning it
		 */
		res = var_set_move(name, nlen, vartype, val);
	    }
	    if (res != NO_ERR) {
		val_free_value(val);
	    }
	}
    } else if (res==NO_ERR) {
	/* this is as assignment to the results
	 * of an RPC function call 
	 */
	if (agent_cb->result_name) {
	    log_warn("\nWarning: result already pending for %s",
		     agent_cb->result_name);
	    m__free(agent_cb->result_name);
	    agent_cb->result_name = NULL;
	}

	if (!*fileassign) {
	    /* save the variable result name */
	    agent_cb->result_name = xml_strndup(name, nlen);
	    if (agent_cb->result_name == NULL) {
		*len = 0;
		res = ERR_INTERNAL_MEM;
	    } else {
		agent_cb->result_vartype = vartype;
	    }
	}

	if (res == NO_ERR) {
	    *len = str - line;
	    *getrpc = TRUE;
	}
    } else {
	/* there was some error in the statement processing */
	*len = 0;
	clear_result(agent_cb);
    }

    return res;

}  /* check_assign_statement */


/********************************************************************
 * FUNCTION create_system_var
 * 
 * create a read-only system variable
 *
 * INPUTS:
 *   varname == variable name
 *   varval  == variable string value (may be NULL)
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    create_system_var (const char *varname,
		       const char *varval)
{
    status_t  res;

    res = var_set_from_string((const xmlChar *)varname,
			      (const xmlChar *)varval,
			      VAR_TYP_SYSTEM);
    return res;

} /* create_system_var */

/********************************************************************
 * FUNCTION create_config_var
 * 
 * create a read-write system variable
 *
 * INPUTS:
 *   varname == variable name
 *   varval  == variable string value (may be NULL)
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    create_config_var (const xmlChar *varname,
		       const xmlChar *varval)
{
    status_t  res;

    res = var_set_from_string(varname, varval,
			      VAR_TYP_CONFIG);
    return res;

} /* create_config_var */


/********************************************************************
 * FUNCTION init_system_vars
 * 
 * create the read-only system variables
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    init_system_vars (void)
{
    const char *envstr;
    status_t    res;

    envstr = getenv(NCXMOD_PWD);
    res = create_system_var(NCXMOD_PWD, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(USER_HOME);
    res = create_system_var(USER_HOME, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(ENV_HOST);
    res = create_system_var(ENV_HOST, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(ENV_SHELL);
    res = create_system_var(ENV_SHELL, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(ENV_USER);
    res = create_system_var(ENV_USER, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(ENV_LANG);
    res = create_system_var(ENV_LANG, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(NCXMOD_HOME);
    res = create_system_var(NCXMOD_HOME, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(NCXMOD_MODPATH);
    res = create_system_var(NCXMOD_MODPATH, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(NCXMOD_DATAPATH);
    res = create_system_var(NCXMOD_DATAPATH, envstr);
    if (res != NO_ERR) {
	return res;
    }

    envstr = getenv(NCXMOD_RUNPATH);
    res = create_system_var(NCXMOD_RUNPATH, envstr);
    if (res != NO_ERR) {
	return res;
    }

    return NO_ERR;

} /* init_system_vars */


/********************************************************************
 * FUNCTION init_config_vars
 * 
 * create the read-write global variables
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    init_config_vars (void)
{
    val_value_t    *parm;
    const xmlChar  *strval;
    status_t        res;
    xmlChar         numbuff[NCX_MAX_NUMLEN];

    strval = NULL;
    parm = val_find_child(mgr_cli_valset, NULL, YANGCLI_AGENT);
    if (parm) {
	strval = VAL_STR(parm);
    }
    res = create_config_var(YANGCLI_AGENT, strval);
    if (res != NO_ERR) {
	return res;
    }

    res = create_config_var(YANGCLI_AUTOCOMP, 
			    (autocomp) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
	return res;
    }

    res = create_config_var(YANGCLI_BADDATA, 
			    ncx_get_baddata_string(baddata));
    if (res != NO_ERR) {
	return res;
    }

    res = create_config_var(YANGCLI_DEF_MODULE, default_module);
    if (res != NO_ERR) {
	return res;
    }

    strval = NULL;
    parm = val_find_child(mgr_cli_valset, NULL, YANGCLI_USER);
    if (parm) {
	strval = VAL_STR(parm);
    } else {
	strval = (const xmlChar *)getenv(ENV_USER);
    }

    res = create_config_var(YANGCLI_USER, strval);
    if (res != NO_ERR) {
	return res;
    }

    res = create_config_var(YANGCLI_TEST_OPTION, NCX_EL_NONE);
    if (res != NO_ERR) {
	return res;
    }

    res = create_config_var(YANGCLI_ERROR_OPTION, NCX_EL_NONE); 
    if (res != NO_ERR) {
	return res;
    }

    sprintf((char *)numbuff, "%u", default_timeout);
    res = create_config_var(YANGCLI_TIMEOUT, numbuff);
    if (res != NO_ERR) {
	return res;
    }

    res = create_config_var(YANGCLI_OPTIONAL, 
			    (cur_agent_cb->get_optional) 
			    ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
	return res;
    }


    res = create_config_var(NCX_EL_LOGLEVEL, 
			    log_get_debug_level_string(log_level));
    if (res != NO_ERR) {
	return res;
    }

    res = create_config_var(YANGCLI_FIXORDER, 
			    (fixorder) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
	return res;
    }

    res = create_config_var(YANGCLI_WITH_DEFAULTS, NCX_EL_NONE); 
    if (res != NO_ERR) {
	return res;
    }

    return NO_ERR;

} /* init_config_vars */



/********************************************************************
* FUNCTION process_cli_input
*
* Process the param line parameters against the hardwired
* parmset for the ncxmgr program
*
* INPUTS:
*    argc == argument count
*    argv == array of command line argument strings
*
* OUTPUTS:
*    global vars that are present are filled in, with parms 
*    gathered or defaults
*
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
static status_t
    process_cli_input (int argc,
		       const char *argv[])
{
    const obj_template_t  *obj;
    val_value_t           *parm;
    status_t               res;

    res = NO_ERR;
    mgr_cli_valset = NULL;

    /* find the parmset definition in the registry */
    obj = ncx_find_object(yangcli_mod, YANGCLI_BOOT);
    if (obj == NULL) {
	res = ERR_NCX_NOT_FOUND;
    }

    if (res == NO_ERR) {
	/* check no command line parms */
	if (argc <= 1) {
	    mgr_cli_valset = val_new_value();
	    if (mgr_cli_valset == NULL) {
		res = ERR_INTERNAL_MEM;
	    } else {
		res = NO_ERR;
		val_init_from_template(mgr_cli_valset, obj);
	    }
	} else {
	    /* parse the command line against the PSD */    
	    mgr_cli_valset = cli_parse(argc, argv, obj,
				       FULLTEST, PLAINMODE,
				       autocomp, &res);
	}
    }

    if (res != NO_ERR) {
	if (mgr_cli_valset) {
	    val_free_value(mgr_cli_valset);
	    mgr_cli_valset = NULL;
	}
	return res;
    }

    /* next get any params from the conf file */
    confname = get_strparm(mgr_cli_valset, 
			   YANGCLI_MOD, YANGCLI_CONFIG);
    if (confname) {
	res = conf_parse_val_from_filespec(confname, 
					   mgr_cli_valset,
					   TRUE, TRUE);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /****************************************************
     * go through the yangcli params in order,
     * after setting up the logging parameters
     ****************************************************/

    /* get the agent parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AGENT);
    if (parm && parm->res == NO_ERR) {
	/* save to the connect_valset parmset */
	res = add_clone_parm(parm, connect_valset);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the autocomp parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AUTOCOMP);
    if (parm && parm->res == NO_ERR) {
	autocomp = VAL_BOOL(parm);
    } else {
	autocomp = TRUE;
    }

    /* get the autoload parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AUTOLOAD);
    if (parm && parm->res == NO_ERR) {
	autoload = VAL_BOOL(parm);
    } else {
	autoload = TRUE;
    }

    /* get the baddata parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, 
			  YANGCLI_BADDATA);
    if (parm && parm->res == NO_ERR) {
	baddata = ncx_get_baddata_enum(VAL_ENUM_NAME(parm));
	if (baddata == NCX_BAD_DATA_NONE) {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    baddata = BAD_DATA_DEFAULT;
	}
    } else {
	baddata = BAD_DATA_DEFAULT;
    }

    /* get the batch-mode parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_BATCHMODE);
    if (parm && parm->res == NO_ERR) {
	batchmode = TRUE;
    }

    /* get the default module for unqualified module addesses */
    default_module = get_strparm(mgr_cli_valset, 
				 YANGCLI_MOD, 
				 YANGCLI_DEF_MODULE);

    /* get the fixorder parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_FIXORDER);
    if (parm && parm->res == NO_ERR) {
	fixorder = VAL_BOOL(parm);
    } else {
	fixorder = TRUE;
    }

    /* get the help flag */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_HELP);
    if (parm && parm->res == NO_ERR) {
	helpmode = TRUE;
    }

    /* help submode parameter (brief/normal/full) */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_BRIEF);
    if (parm) {
	helpsubmode = HELP_MODE_BRIEF;
    } else {
	/* full parameter */
	parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_FULL);
	if (parm) {
	    helpsubmode = HELP_MODE_FULL;
	} else {
	    helpsubmode = HELP_MODE_NORMAL;
	}
    }

    /* get the password parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_PASSWORD);
    if (parm && parm->res == NO_ERR) {
	/* save to the connect_valset parmset */
	res = add_clone_parm(parm, connect_valset);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the modules parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_MODULES);
    if (parm && parm->res == NO_ERR) {
	modules = val_clone(parm);
	if (modules == NULL) {
	    return ERR_INTERNAL_MEM;
	}
    }

    /* get the run-script parameter */
    runscript = get_strparm(mgr_cli_valset, YANGCLI_MOD, YANGCLI_RUN_SCRIPT);

    /* get the timeout parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	default_timeout = VAL_UINT(parm);

	/* save to the connect_valset parmset */
	res = add_clone_parm(parm, connect_valset);
	if (res != NO_ERR) {
	    return res;
	}
    } else {
	default_timeout = YANGCLI_DEF_TIMEOUT;
    }

    /* get the user name */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_USER);
    if (parm && parm->res == NO_ERR) {
	res = add_clone_parm(parm, connect_valset);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the version parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_VERSION);
    if (parm && parm->res == NO_ERR) {
	versionmode = TRUE;
    }

    return NO_ERR;

} /* process_cli_input */


/********************************************************************
 * FUNCTION load_base_schema 
 * 
 * Load the following NCX modules:
 *   yangcli
 *
 * RETURNS:
 *     status
 *********************************************************************/
static status_t
    load_base_schema (void)
{
    status_t   res;

    log_debug2("\nYangcli: Loading NCX yangcli-cli Parmset");

    /* load in the agent boot parameter definition file */
    res = ncxmod_load_module(YANGCLI_MOD, NULL, &yangcli_mod);
    if (res != NO_ERR) {
	return res;
    }

    /* load in the NETCONF data types and RPC methods */
    res = ncxmod_load_module(NC_MODULE, NULL, &netconf_mod);
    if (res != NO_ERR) {
	return res;
    }

    /* load in the NCX extensions */
    res = ncxmod_load_module(NCXMOD_NCX, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* initialize the NETCONF operation attribute 
     * MUST be after the netconf.yang module is loaded
     */
    res = ncx_stage2_init();
    if (res != NO_ERR) {
	return res;
    }

    return NO_ERR;

}  /* load_base_schema */


/********************************************************************
 * FUNCTION load_core_schema 
 * 
 * Load the following NCX modules:
 *   xsd
 *   ncxtypes
 *   netconf
 *   inetAddress
 *
 * RETURNS:
 *     status
 *********************************************************************/
static status_t
    load_core_schema (void)
{
    status_t   res;

    log_debug2("\nNcxmgr: Loading NETCONF Module");

    /* load in the XSD data types */
    res = ncxmod_load_module(XSDMOD, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* load in the NCX data types */
    res = ncxmod_load_module(NCXDTMOD, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return NO_ERR;

}  /* load_core_schema */


/********************************************************************
* FUNCTION report_capabilities
* 
* Generate a start session report, listing the capabilities
* of the NETCONF agent
* 
* INPUTS:
*  agent_cb == agent control block to use
*  scb == session control block
*
*********************************************************************/
static void
    report_capabilities (agent_cb_t *agent_cb,
			 const ses_cb_t *scb)
{
    const mgr_scb_t    *mscb;
    const xmlChar      *agent;
    const val_value_t  *parm;


    mscb = (const mgr_scb_t *)scb->mgrcb;

    parm = val_find_child(agent_cb->connect_valset, 
			  YANGCLI_MOD, YANGCLI_AGENT);
    if (parm && parm->res == NO_ERR) {
	agent = VAL_STR(parm);
    } else {
	agent = (const xmlChar *)"--";
    }

    log_write("\n\nNETCONF session established for %s on %s",
	      scb->username, 
	      mscb->target ? mscb->target : agent);

    if (!LOGINFO) {
	/* skip the rest unless log level is INFO or higher */
	return;
    }

    log_write("\n\nManager Session Id: %u", scb->sid);
    log_write("\nAgent Session Id: %u", mscb->agtsid);

    log_write("\n\nAgent Protocol Capabilities");
    cap_dump_stdcaps(&mscb->caplist);

    log_write("\n\nAgent Module Capabilities");
    cap_dump_modcaps(&mscb->caplist);

    log_write("\n\nAgent Enterprise Capabilities");
    cap_dump_entcaps(&mscb->caplist);
    log_write("\n");

    log_write("\nDefault target set to: ");
    switch (mscb->targtyp) {
    case NCX_AGT_TARG_NONE:
	agent_cb->default_target = NULL;
	log_write("none");
	break;
    case NCX_AGT_TARG_CANDIDATE:
	agent_cb->default_target = NCX_EL_CANDIDATE;
	log_write("<candidate>");
	break;
    case NCX_AGT_TARG_RUNNING:
	agent_cb->default_target = NCX_EL_RUNNING;	
	log_write("<running>");
	break;
    case NCX_AGT_TARG_CAND_RUNNING:
	log_write("<candidate> (<running> also supported)");
	break;
    case NCX_AGT_TARG_LOCAL:
	agent_cb->default_target = NULL;
	log_write("none -- local file");	
	break;
    case NCX_AGT_TARG_REMOTE:
	agent_cb->default_target = NULL;
	log_write("none -- remote file");	
	break;
    default:
	agent_cb->default_target = NULL;
	SET_ERROR(ERR_INTERNAL_VAL);
	log_write("none -- unknown (%d)", mscb->targtyp);
	break;
    }

    log_write("\nSave operation mapped to: ");
    switch (mscb->targtyp) {
    case NCX_AGT_TARG_NONE:
	log_write("none");
	break;
    case NCX_AGT_TARG_CANDIDATE:
    case NCX_AGT_TARG_CAND_RUNNING:
	log_write("commit");
	if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
	    log_write(" + copy-config <running> <startup>");
	}
	break;
    case NCX_AGT_TARG_RUNNING:
	if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
	    log_write("copy-config <running> <startup>");
	} else {
	    log_write("none");
	}	    
	break;
    case NCX_AGT_TARG_LOCAL:
    case NCX_AGT_TARG_REMOTE:
	/* no way to assign these enums from the capabilities alone! */
	if (cap_std_set(&mscb->caplist, CAP_STDID_URL)) {
	    log_write("copy-config <running> <url>");
	} else {
	    log_write("none");
	}	    
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	log_write("none");
	break;
    }

    log_write("\nDefault with-defaults behavior: ");
    if (mscb->caplist.cap_defstyle) {
	log_write("%s", mscb->caplist.cap_defstyle);
    } else {
	log_write("unknown");
    }

    log_write("\nAdditional with-defaults behavior: ");
    if (mscb->caplist.cap_supported) {
	log_write("%s", mscb->caplist.cap_supported);
    } else {
	log_write("unknown");
    }

    log_write("\n");
    
} /* report_capabilities */


/********************************************************************
* FUNCTION check_module_capabilities
* 
* Check the modules reported by the agent
* If autoload is TRUE then load any missing modules
* otherwise just warn which modules are missing
* Also check for wrong module module and version errors
*
* INPUTS:
*  agent_cb == agent control block to use
*  scb == session control block
*
*********************************************************************/
static void
    check_module_capabilities (agent_cb_t *agent_cb,
			       const ses_cb_t *scb)
{
    const mgr_scb_t    *mscb;
    ncx_module_t       *mod;
    const cap_rec_t    *cap;
    const xmlChar      *module, *version;
    modptr_t           *modptr;
    xmlChar            *namebuff;
    uint32              modlen;
    status_t            res;


    mscb = (const mgr_scb_t *)scb->mgrcb;

    log_info("\n\nChecking Agent Modules...\n");

    /**** HARDWIRE ADD NETCONF V1 TO THE QUEUE
     **** NEED TO GET THE STD CAP INSTEAD
     ****/
    mod = ncx_find_module(NC_MODULE, NULL);
    if (mod) {
	modptr = new_modptr(mod);
	if (modptr == NULL) {
	    log_error("\nMalloc failure");
	    return;
	} else {
	    dlq_enque(modptr, &agent_cb->modptrQ);
	}
    }

    /* check all the YANG modules */
    cap = cap_first_modcap(&mscb->caplist);
    while (cap) {
	cap_split_modcap(cap,
			 &module,
			 &modlen,
			 &version);

	if (module==NULL || !modlen || !version) {
	    log_warn("\nWarning: skipping invalid module capability "
		     "for URI '%s'", cap->cap_uri);
	    cap = cap_next_modcap(cap);
	    continue;
	}

	namebuff = xml_strndup(module, modlen);
	if (namebuff == NULL) {
	    log_error("\nMalloc failure");
	    return;
	}

	mod = ncx_find_module(namebuff, version);
	if (mod == NULL) {
	    if (autoload) {
		res = ncxmod_load_module(namebuff, version, &mod);
		if (res != NO_ERR) {
		    log_error("\nyangcli error: Module %s not loaded (%s)!!",
			      namebuff, get_error_string(res));
		}
	    } else {
		log_info("\nyangcli warning: Module %s not loaded!!",
			 namebuff);
	    }
	}

	/* keep track of the exact modules the agent knows about */
	if (mod) {
	    modptr = new_modptr(mod);
	    if (modptr == NULL) {
		log_error("\nMalloc failure");
		return;
	    } else {
		dlq_enque(modptr, &agent_cb->modptrQ);
	    }

	    if (yang_compare_revision_dates(mod->version, version)) {
		log_warn("\nyangcli warning: Module %s "
			 "has different version on agent!! (%s)",
			 namebuff, mod->version);
	    }
	}

	m__free(namebuff);
	namebuff = NULL;

	cap = cap_next_modcap(cap);
    }
    
} /* check_module_capabilities */


/********************************************************************
* message_timed_out
*
* Check if the request in progress (!) has timed out
* TBD: check for a specific message if N requests per
* session are ever supported
*
* INPUTS:
*   scb == session control block to use
*
* RETURNS:
*   TRUE if any messages have timed out
*   FALSE if no messages have timed out
*********************************************************************/
static boolean
    message_timed_out (ses_cb_t *scb)
{
    mgr_scb_t    *mscb;
    uint32        deletecount;

    mscb = (mgr_scb_t *)scb->mgrcb;

    if (mscb) {
	deletecount = mgr_rpc_timeout_requestQ(&mscb->reqQ);
	if (deletecount) {
	    log_error("\nError: request to agent timed out");
	}
	return (deletecount) ? TRUE : FALSE;
    }

    /* else mgr_shutdown could have been issued via control-C
     * and the session control block has been deleted
     */
    return FALSE;

}  /* message_timed_out */


/********************************************************************
* yangcli_stdin_handler
*
* Temp: Calling readline which will block other IO while the user
*       is editing the line.  This is okay for this CLI application
*       but not multi-session applications;
* Need to prevent replies from popping up on the screen
* while new commands are being edited anyway
*
* RETURNS:
*   new program state
*********************************************************************/
static mgr_io_state_t
    yangcli_stdin_handler (void)
{
    agent_cb_t    *agent_cb;
    xmlChar       *line;
    ses_cb_t       *scb;
    boolean         getrpc, fileassign;
    status_t        res;
    uint32          len;

    /* assuming cur_agent_cb will be set dynamically
     * at some point; currently 1 session supported in yangcli
     */
    agent_cb = cur_agent_cb;
    
    if (agent_cb->cli_fn == NULL && !agent_cb->climore) {
	init_completion_state(&agent_cb->completion_state,
			      agent_cb, 
			      CMD_STATE_FULL);
    }

    if (mgr_shutdown_requested()) {
	agent_cb->state = MGR_IO_ST_SHUT;
    }

    switch (agent_cb->state) {
    case MGR_IO_ST_INIT:
	return agent_cb->state;
    case MGR_IO_ST_IDLE:
    case MGR_IO_ST_CONN_IDLE:
	break;
    case MGR_IO_ST_CONN_START:
	/* waiting until <hello> processing complete */
	scb = mgr_ses_get_scb(agent_cb->mysid);
	if (scb == NULL) {
	    /* session startup failed */
	    agent_cb->state = MGR_IO_ST_IDLE;
	} else if (scb->state == SES_ST_IDLE 
		   && dlq_empty(&scb->outQ)) {
	    /* incoming hello OK and outgoing hello is sent */
	    agent_cb->state = MGR_IO_ST_CONN_IDLE;
	    report_capabilities(agent_cb, scb);
	    check_module_capabilities(agent_cb, scb);
	} else {
	    /* check timeout */
	    if (message_timed_out(scb)) {
		agent_cb->state = MGR_IO_ST_IDLE;
		break;
	    } /* else still setting up session */
	    return agent_cb->state;
	}
	break;
    case MGR_IO_ST_CONN_RPYWAIT:
	/* check if session was dropped by remote peer */
	scb = mgr_ses_get_scb(agent_cb->mysid);
	if (scb==NULL || scb->state == SES_ST_SHUTDOWN_REQ) {
	    if (scb) {
		(void)mgr_ses_free_session(agent_cb->mysid);
	    }
	    clear_agent_cb_session(agent_cb);
	} else  {
	    /* check timeout */
	    if (message_timed_out(scb)) {
		agent_cb->state = MGR_IO_ST_CONN_IDLE;
		break;
	    }
	    /* keep waiting for reply */
	    return agent_cb->state;
	}
	break;
    case MGR_IO_ST_CONNECT:
    case MGR_IO_ST_SHUT:
    case MGR_IO_ST_CONN_CANCELWAIT:
    case MGR_IO_ST_CONN_SHUT:
    case MGR_IO_ST_CONN_CLOSEWAIT:
	/* check timeout */
	scb = mgr_ses_get_scb(agent_cb->mysid);
	if (scb==NULL || scb->state == SES_ST_SHUTDOWN_REQ) {
	    if (scb) {
		(void)mgr_ses_free_session(agent_cb->mysid);
	    }
	    clear_agent_cb_session(agent_cb);
	} else if (message_timed_out(scb)) {
	    clear_agent_cb_session(agent_cb);
	}

	/* do not accept chars in these states */
	return agent_cb->state;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return agent_cb->state;
    }

    /* check a batch-mode corner-case, nothing else to do */
    if (batchmode && !runscript) {
	mgr_request_shutdown();
	return agent_cb->state;
    }

    /* check the run-script parameters */
    if (runscript && !runscriptdone) {
	runscriptdone = TRUE;
	res = do_startup_script(agent_cb);
	if (res != NO_ERR) {
	    mgr_request_shutdown();
	    return agent_cb->state;
	}
    }

    /* get a line of user input */
    if (runstack_level()) {
	/* get one line of script text */
	line = runstack_get_cmd(&res);
	if (line==NULL || res != NO_ERR) {
	    if (batchmode) {
		mgr_request_shutdown();
	    }
	    return agent_cb->state;
	}
    } else {
	/* block until user enters some input */
	line = get_cmd_line(agent_cb, &res);
	if (line==NULL) {
	    return agent_cb->state;
	}
    }

    /* check if this is an assignment statement */
    res = check_assign_statement(agent_cb, 
				 line, 
				 &len, 
				 &getrpc,
				 &fileassign);
    if (res != NO_ERR) {
	log_error("\nyangcli: Variable assignment failed (%s) (%s)",
		  line, get_error_string(res));
    } else if (getrpc) {
	switch (agent_cb->state) {
	case MGR_IO_ST_IDLE:
	    /* waiting for top-level commands */
	    top_command(agent_cb, &line[len]);
	    break;
	case MGR_IO_ST_CONN_IDLE:
	    /* waiting for session commands */
	    conn_command(agent_cb, &line[len]);
	    break;
	case MGR_IO_ST_CONN_RPYWAIT:
	    /* waiting for RPC reply while more input typed */
	    break;
	case MGR_IO_ST_CONN_CANCELWAIT:
	    break;
	default:
	    break;
	}
    } else {
	log_info("\nOK\n");
    }

    return agent_cb->state;

} /* yangcli_stdin_handler */


/********************************************************************
 * FUNCTION yangcli_init
 * 
 * Init the NCX CLI application
 * 
 * INPUTS:
 *   argc == number of strings in argv array
 *   argv == array of command line strings
 * 
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    yangcli_init (int argc,
	      const char *argv[])
{
    const obj_template_t *obj;
    agent_cb_t           *agent_cb;
    ncx_lmem_t           *lmem;
    val_value_t          *parm;
    status_t              res;

#ifdef YANGCLI_DEBUG
    int   i;
#endif

    /* set the default debug output level */
#ifdef DEBUG
    log_level = LOG_DEBUG_DEBUG;
#else
    log_level = LOG_DEBUG_INFO;
#endif

    /* init the module static vars */
    dlq_createSQue(&agent_cbQ);
    dlq_createSQue(&mgrloadQ);

    /* state = MGR_IO_ST_INIT; */
    mgr_cli_valset = NULL;
    connect_valset = NULL;
    batchmode = FALSE;
    default_module = NULL;
    helpmode = FALSE;
    helpsubmode = HELP_MODE_NONE;
    versionmode = FALSE;
    modules = NULL;
    autoload = TRUE;
    fixorder = TRUE;
    optional = FALSE;
    autocomp = TRUE;
    logappend = FALSE;
    logfilename = NULL;
    runscript = NULL;
    runscriptdone = FALSE;
    malloc_cnt = 0;
    free_cnt = 0;

    /* set the character set LOCALE to the user default */
    setlocale(LC_CTYPE, "");

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     */
    res = ncx_init(NCX_SAVESTR, 
		   log_level, 
		   TRUE,
		   "\nStarting yangcli",
		   argc, 
		   argv);

    if (res != NO_ERR) {
	return res;
    }

#ifdef YANGCLI_DEBUG
    if (argc>1 && LOGDEBUG2) {
        log_debug2("\nCommand line parameters:");
        for (i=0; i<argc; i++) {
            log_debug2("\n   arg%d: %s", i, argv[i]);
        }
    }
#endif

    /* at this point, modules that need to read config
     * params can be initialized
     */

    /* Load the yangcli base module */
    res = load_base_schema();
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the Netconf Manager Library */
    res = mgr_init();
    if (res != NO_ERR) {
	return res;
    }

    /* init the connect parmset object template;
     * find the connect RPC method
     * !!! MUST BE AFTER load_base_schema !!!
     */
    obj = ncx_find_object(yangcli_mod, YANGCLI_CONNECT);
    if (obj==NULL) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* set the parmset object to the input node of the RPC */
    obj = obj_find_child(obj, NULL, YANG_K_INPUT);
    if (obj==NULL) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* treat the connect-to-agent parmset special
     * it is saved for auto-start plus restart parameters
     * Setup an empty parmset to hold the connect parameters
     */
    connect_valset = val_new_value();
    if (connect_valset==NULL) {
	return ERR_INTERNAL_MEM;
    } else {
	val_init_from_template(connect_valset, obj);
    }

    /* Get any command line and conf file parameters */
    res = process_cli_input(argc, argv);
    if (res != NO_ERR) {
	return res;
    }

    /* check print version */
    if (versionmode && !helpmode) {
	log_stdout("\nyangcli version %s\n", YANGCLI_PROGVER);
    }

    /* check print help and exit */
    if (helpmode) {
	log_stdout("\nyangcli version %s", YANGCLI_PROGVER);
	help_program_module(YANGCLI_MOD, 
			    YANGCLI_BOOT, 
			    helpsubmode);
    }

    /* check quick exit */
    if (helpmode || versionmode) {
	return NO_ERR;
    }

    /* create a default agent control block */
    agent_cb = new_agent_cb(YANGCLI_DEF_AGENT);
    if (agent_cb==NULL) {
	return ERR_INTERNAL_MEM;
    }
    dlq_enque(agent_cb, &agent_cbQ);

    cur_agent_cb = agent_cb;

    /* set the CLI handler */
    mgr_io_set_stdin_handler(yangcli_stdin_handler);

    /* Load the NETCONF, XSD, SMI and other core modules */
    if (autoload) {
	res = load_core_schema();
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* check if any explicitly listed modules should be loaded */
    if (modules) {
	lmem = ncx_first_lmem(&VAL_LIST(modules));
	while (lmem) {

	    log_info("\nyangcli: Loading requested module %s", 
		     NCX_LMEM_STRVAL(lmem));

	    res = ncxmod_load_module
		((const xmlChar *)NCX_LMEM_STRVAL(lmem),
		 NULL,   /*** need revision parameter ***/
		 NULL);
	    if (res != NO_ERR) {
		log_info("\n load failed (%s)", get_error_string(res));
	    } else {
		log_info("\n load OK");
	    }

	    lmem = (ncx_lmem_t *)dlq_nextEntry(lmem);
	}
    }

    /* load the system (read-only) variables */
    res = init_system_vars();
    if (res != NO_ERR) {
	return res;
    }

    /* load the system config variables */
    res = init_config_vars();
    if (res != NO_ERR) {
	return res;
    }

    /* make sure the startup screen is generated
     * before the auto-connect sequence starts
     */
    do_startup_screen();    

    /* check to see if a session should be auto-started
     * --> if the agent parameter is set a connect will
     * --> be attempted
     *
     * The yangcli_stdin_handler will call the finish_start_session
     * function when the user enters a line of keyboard text
     */
    agent_cb->state = MGR_IO_ST_IDLE;
    if (connect_valset) {
	parm = val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_AGENT);
	if (parm && parm->res == NO_ERR) {
	    do_connect(agent_cb, NULL, NULL, 0, TRUE);
	}
    }

    return NO_ERR;

}  /* yangcli_init */


/********************************************************************
 * FUNCTION yangcli_cleanup
 * 
 * 
 * 
 *********************************************************************/
static void
    yangcli_cleanup (void)
{
    agent_cb_t  *agent_cb;
    modptr_t    *modptr;

    log_debug2("\nShutting down yangcli\n");

    while (!dlq_empty(&mgrloadQ)) {
	modptr = (modptr_t *)dlq_deque(&mgrloadQ);
	free_modptr(modptr);
    }

    /* Cleanup the Netconf Agent Library */
    mgr_cleanup();

    /* cleanup the NCX engine and registries */
    ncx_cleanup();

    /* clean and reset all module static vars */
    if (cur_agent_cb) {
	cur_agent_cb->state = MGR_IO_ST_NONE;
	cur_agent_cb->mysid = 0;
    }

    while (!dlq_empty(&agent_cbQ)) {
	agent_cb = (agent_cb_t *)dlq_deque(&agent_cbQ);
	free_agent_cb(agent_cb);
    }

    if (mgr_cli_valset) {
	val_free_value(mgr_cli_valset);
	mgr_cli_valset = NULL;
    }

    if (connect_valset) {
	val_free_value(connect_valset);
	connect_valset = NULL;
    }

    if (default_module) {
	m__free(default_module);
	default_module = NULL;
    }

    if (modules) {
	val_free_value(modules);
	modules = NULL;
    }

    if (logfilename) {
	m__free(logfilename);
	logfilename = NULL;
    }

    if (confname) {
	m__free(confname);
	confname = NULL;
    }

    if (runscript) {
	m__free(runscript);
	runscript = NULL;
    }

    if (malloc_cnt != free_cnt) {
	log_error("\n*** Error: memory leak (m:%u f:%u)\n", 
		  malloc_cnt, free_cnt);
    }

    log_close();

}  /* yangcli_cleanup */





/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION get_autocomp
* 
*  Get the autocomp parameter value
* 
* RETURNS:
*    autocomp boolean value
*********************************************************************/
boolean
    get_autocomp (void)
{
    return autocomp;
}  /* get_autocomp */


/********************************************************************
* FUNCTION get_autoload
* 
*  Get the autoload parameter value
* 
* RETURNS:
*    autoload boolean value
*********************************************************************/
boolean
    get_autoload (void)
{
    return autoload;
}  /* get_autoload */


/********************************************************************
* FUNCTION get_batchmode
* 
*  Get the batchmode parameter value
* 
* RETURNS:
*    batchmode boolean value
*********************************************************************/
boolean
    get_batchmode (void)
{
    return batchmode;
}  /* get_batchmode */


/********************************************************************
* FUNCTION get_default_module
* 
*  Get the default module
* 
* RETURNS:
*    default module value
*********************************************************************/
const xmlChar *
    get_default_module (void)
{
    return default_module;
}  /* get_default_module */


/********************************************************************
* FUNCTION get_runscript
* 
*  Get the runscript variable
* 
* RETURNS:
*    runscript value
*********************************************************************/
const xmlChar *
    get_runscript (void)
{
    return runscript;

}  /* get_runscript */


/********************************************************************
* FUNCTION get_baddata
* 
*  Get the baddata parameter
* 
* RETURNS:
*    baddata enum value
*********************************************************************/
ncx_bad_data_t
    get_baddata (void)
{
    return baddata;
}  /* get_baddata */


/********************************************************************
* FUNCTION get_netconf_mod
* 
*  Get the netconf module
* 
* RETURNS:
*    netconf module
*********************************************************************/
ncx_module_t *
    get_netconf_mod (void)
{
    return netconf_mod;
}  /* get_netconf_mod */


/********************************************************************
* FUNCTION get_yangcli_mod
* 
*  Get the yangcli module
* 
* RETURNS:
*    yangcli module
*********************************************************************/
ncx_module_t *
    get_yangcli_mod (void)
{
    return yangcli_mod;
}  /* get_yangcli_mod */


/********************************************************************
* FUNCTION get_mgr_cli_valset
* 
*  Get the CLI value set
* 
* RETURNS:
*    mgr_cli_valset variable
*********************************************************************/
val_value_t *
    get_mgr_cli_valset (void)
{
    return mgr_cli_valset;
}  /* get_mgr_cli_valset */


/********************************************************************
* FUNCTION get_connect_valset
* 
*  Get the connect value set
* 
* RETURNS:
*    connect_valset variable
*********************************************************************/
val_value_t *
    get_connect_valset (void)
{
    return connect_valset;
}  /* get_connect_valset */


/********************************************************************
* FUNCTION get_mgrloadQ
* 
*  Get the mgrloadQ value pointer
* 
* RETURNS:
*    mgrloadQ variable
*********************************************************************/
dlq_hdr_t *
    get_mgrloadQ (void)
{
    return &mgrloadQ;
}  /* get_mgrloadQ */


/********************************************************************
 * FUNCTION yangcli_reply_handler
 * 
 * 
 * INPUTS:
 *   scb == session receiving RPC reply
 *   req == original request returned for freeing or reusing
 *   rpy == reply received from the agent (for checking then freeing)
 *
 * RETURNS:
 *   none
 *********************************************************************/
void
    yangcli_reply_handler (ses_cb_t *scb,
			   mgr_rpc_req_t *req,
			   mgr_rpc_rpy_t *rpy)
{
    agent_cb_t   *agent_cb;
    val_value_t  *val;
    mgr_scb_t    *mgrcb;
    status_t      res;
    boolean       anyout, anyerrors;
    uint32        usesid;

#ifdef DEBUG
    if (!scb || !req || !rpy) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    mgrcb = scb->mgrcb;
    if (mgrcb) {
	usesid = mgrcb->agtsid;
    } else {
	usesid = 0;
    }

    /***  TBD: multi-session support ***/
    agent_cb = cur_agent_cb;

    anyerrors = FALSE;

    /* check the contents of the reply */
    if (rpy && rpy->reply) {
	if (val_find_child(rpy->reply, NC_MODULE,
			   NCX_EL_RPC_ERROR)) {
	    log_error("\nRPC Error Reply %s for session %u:\n",
		      rpy->msg_id, usesid);
	    val_dump_value(rpy->reply, 0);
	    log_error("\n");
	    anyout = TRUE;
	    anyerrors = TRUE;
	} else if (val_find_child(rpy->reply, NC_MODULE, NCX_EL_OK)) {
	    log_info("\nRPC OK Reply %s for session %u:\n",
		     rpy->msg_id, usesid);
	    anyout = TRUE;
	} else if (LOGINFO) {
	    log_info("\nRPC Data Reply %s for session %u:\n",
		     rpy->msg_id, usesid);
	    if (LOGDEBUG) {
		val_dump_value(rpy->reply, 0);
		log_info("\n");
	    }
	    anyout = TRUE;
	} else {
	    anyout = FALSE;
	}

	/* output data even if there were errors
	 * TBD: use a CLI switch to control whether
	 * to save if <rpc-errors> received
	 */
	if (agent_cb->result_name || agent_cb->result_filename) {
	    /* save the data element if it exists */
	    val = val_first_child_name(rpy->reply, NCX_EL_DATA);
	    if (val) {
		val_remove_child(val);
	    } else {
		if (val_child_cnt(rpy->reply) == 1) {
		    val = val_get_first_child(rpy->reply);
		    val_remove_child(val);
		} else {
		    /* not 1 child node, so save the entire reply
		     * need a single top-level element to be a
		     * valid XML document
		     */
		    val = rpy->reply;
		    rpy->reply = NULL;
		}
	    }

	    /* hand off the malloced 'val' node here */
	    res = finish_result_assign(agent_cb, val, NULL);
	}  else if (!anyout && interactive_mode()) {
	    log_stdout("\nOK\n");
	}
    } else {
	log_error("\nError: yangcli: no reply parsed\n");
    }

    if (agent_cb->state == MGR_IO_ST_CONN_CLOSEWAIT) {
	agent_cb->mysid = 0;
	agent_cb->state = MGR_IO_ST_IDLE;
    } else if (agent_cb->state == MGR_IO_ST_CONN_RPYWAIT) {
	agent_cb->state = MGR_IO_ST_CONN_IDLE;
    } /* else leave state at its current value */

    /* free the request and reply */
    mgr_rpc_free_request(req);
    if (rpy) {
	mgr_rpc_free_reply(rpy);
    }

}  /* yangcli_reply_handler */


/********************************************************************
 * FUNCTION finish_result_assign
 * 
 * finish the assignment to result_name or result_filename
 * use 1 of these 2 parms:
 *    resultval == result to output to file
 *    resultstr == result to output as string
 *
 * INPUTS:
 *   agent_cb == agent control block to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t
    finish_result_assign (agent_cb_t *agent_cb,
			  val_value_t *resultvar,
			  const xmlChar *resultstr)
{
    val_value_t   *configvar;
    status_t       res;

#ifdef DEBUG
    if (!agent_cb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    if (agent_cb->result_filename) {
	res = output_file_result(agent_cb, resultvar, resultstr);
	if (resultvar) {
	    val_free_value(resultvar);
	}
    } else if (agent_cb->result_name) {
	if (agent_cb->result_vartype == VAR_TYP_CONFIG) {
	    configvar = var_get(agent_cb->result_name,
				VAR_TYP_CONFIG);
	    if (configvar==NULL) {
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    } else {
		res = handle_config_assign(agent_cb,
					   configvar,
					   resultvar,
					   resultstr);
	    }
	} else if (resultvar) {
	    /* save the filled in value
	     * hand off the malloced 'resultvar' here
	     */
	    res = var_set_move(agent_cb->result_name, 
			       xml_strlen(agent_cb->result_name),
			       agent_cb->result_vartype,
			       resultvar);
	    if (res != NO_ERR) {
		val_free_value(resultvar);
		log_error("\nError: set result for '%s' failed (%s)",
			  agent_cb->result_name, 
			  get_error_string(res));
	    } else {
		log_info("\nOK\n");
	    }
	} else {
	    /* this is just a string assignment */
	    res = var_set_from_string(agent_cb->result_name,
				      resultstr, 
				      agent_cb->result_vartype);
	    if (res != NO_ERR) {
		log_error("\nyangcli: Error setting variable %s (%s)",
			  agent_cb->result_name, 
			  get_error_string(res));
	    } else {
		log_info("\nOK\n");
	    }
	}
    }

    clear_result(agent_cb);

    return res;

}  /* finish_result_assign */



/********************************************************************
*                                                                   *
*			FUNCTION main				    *
*                                                                   *
*********************************************************************/
int 
    main (int argc, 
	  const char *argv[])
{
    status_t   res;

#ifdef MEMORY_DEBUG
    mtrace();
#endif

    res = yangcli_init(argc, argv);
    if (res != NO_ERR) {
	log_error("\nYangcli: init returned error (%s)\n", 
		  get_error_string(res));
    } else if (!(helpmode || versionmode)) {
	do_startup_screen();
	res = mgr_io_run();
	if (res != NO_ERR) {
	    log_error("\nmgr_io failed (%d)\n", res);
	}
    }

    print_errors();
    yangcli_cleanup();

    return 0;

} /* main */

/* END yangcli.c */
