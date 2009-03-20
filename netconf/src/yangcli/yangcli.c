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

#ifndef _H_obj_help
#include "obj_help.h"
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

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xml_val
#include "xml_val.h"
#endif

#ifndef _H_xml_wr
#include "xml_wr.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define YANGCLI_DEBUG   1

#endif

#define MAX_PROMPT_LEN 56

#define YANGCLI_MAX_NEST  16

#define YANGCLI_MAX_RUNPARMS 9

#define YANGCLI_LINELEN   4095

#define YANGCLI_BUFFLEN  32000

#define YANGCLI_HISTLEN  64000

#define YANGCLI_DEF_TIMEOUT   30

#define YANGCLI_MOD  (const xmlChar *)"yangcli"

#ifdef MAC
#define ENV_HOST        (const char *)"HOST"
#else
#define ENV_HOST        (const char *)"HOSTNAME"
#endif

#define ENV_SHELL       (const char *)"SHELL"
#define ENV_USER        (const char *)"USER"
#define ENV_LANG        (const char *)"LANG"

/* CLI parmset for the ncxcli application */
#define YANGCLI_BOOT YANGCLI_MOD

/* core modules auto-loaded at startup */
#define NCXDTMOD       (const xmlChar *)"ncxtypes"
#define XSDMOD         (const xmlChar *)"xsd"

#define DEF_PROMPT     (const xmlChar *)"yangcli> "
#define DEF_FN_PROMPT  (const xmlChar *)"yangcli:"
#define MORE_PROMPT    (const xmlChar *)"   more> "

#define YESNO_NODEF  0
#define YESNO_CANCEL 0
#define YESNO_YES    1
#define YESNO_NO     2

#define DEF_OPTIONS (const xmlChar *)"?, \?\?, \?s, ?c"

/* YANGCLI boot and operation parameter names 
 * matches parm clauses in yangcli container in yangcli.yang
 */

#define YANGCLI_AGENT       (const xmlChar *)"agent"
#define YANGCLI_AUTOCOMP    (const xmlChar *)"autocomp"
#define YANGCLI_AUTOLOAD    (const xmlChar *)"autoload"
#define YANGCLI_BADDATA     (const xmlChar *)"baddata"
#define YANGCLI_BATCHMODE   (const xmlChar *)"batch-mode"
#define YANGCLI_BRIEF       (const xmlChar *)"brief"
#define YANGCLI_COMMAND     (const xmlChar *)"command"
#define YANGCLI_COMMANDS    (const xmlChar *)"commands"
#define YANGCLI_CONF        (const xmlChar *)"conf"
#define YANGCLI_CURRENT_VALUE (const xmlChar *)"current-value"
#define YANGCLI_DEF_MODULE  (const xmlChar *)"default-module"
#define YANGCLI_DIR         (const xmlChar *)"dir"
#define YANGCLI_EDIT_TARGET (const xmlChar *)"edit-target"
#define YANGCLI_ERROR_OPTION (const xmlChar *)"error-option"
#define YANGCLI_FIXORDER    (const xmlChar *)"fixorder"
#define YANGCLI_FROM_CLI    (const xmlChar *)"from-cli"
#define YANGCLI_FULL        (const xmlChar *)"full"
#define YANGCLI_GLOBAL      (const xmlChar *)"global"
#define YANGCLI_GLOBALS     (const xmlChar *)"globals"
#define YANGCLI_OIDS        (const xmlChar *)"oids"
#define YANGCLI_LOCAL       (const xmlChar *)"local"
#define YANGCLI_LOCALS      (const xmlChar *)"locals"
#define YANGCLI_MODULE      (const xmlChar *)"module"
#define YANGCLI_MODULES     (const xmlChar *)"modules"
#define YANGCLI_NOFILL      (const xmlChar *)"nofill"
#define YANGCLI_OBJECTS     (const xmlChar *)"objects"
#define YANGCLI_OPERATION   (const xmlChar *)"operation"
#define YANGCLI_OPTIONAL    (const xmlChar *)"optional"
#define YANGCLI_ORDER       (const xmlChar *)"order"
#define YANGCLI_PASSWORD    (const xmlChar *)"password"
#define YANGCLI_PORT        (const xmlChar *)"port"
#define YANGCLI_RUN_SCRIPT  (const xmlChar *)"run-script"
#define YANGCLI_TEST_OPTION (const xmlChar *)"test-option"
#define YANGCLI_TIMEOUT     (const xmlChar *)"timeout"
#define YANGCLI_USER        (const xmlChar *)"user"
#define YANGCLI_VAR         (const xmlChar *)"var"
#define YANGCLI_VARREF      (const xmlChar *)"varref"
#define YANGCLI_VARS        (const xmlChar *)"vars"

#define BAD_DATA_DEFAULT NCX_BAD_DATA_CHECK

/* YANGCLI local RPC commands */
#define YANGCLI_CD      (const xmlChar *)"cd"
#define YANGCLI_CONNECT (const xmlChar *)"connect"
#define YANGCLI_CREATE  (const xmlChar *)"create"
#define YANGCLI_DELETE  (const xmlChar *)"delete"
#define YANGCLI_FILL    (const xmlChar *)"fill"
#define YANGCLI_HELP    (const xmlChar *)"help"
#define YANGCLI_INSERT  (const xmlChar *)"insert"
#define YANGCLI_LIST    (const xmlChar *)"list"
#define YANGCLI_MERGE   (const xmlChar *)"merge"
#define YANGCLI_MGRLOAD (const xmlChar *)"mgrload"
#define YANGCLI_PWD     (const xmlChar *)"pwd"
#define YANGCLI_QUIT    (const xmlChar *)"quit"
#define YANGCLI_REPLACE (const xmlChar *)"replace"
#define YANGCLI_RUN     (const xmlChar *)"run"
#define YANGCLI_SAVE    (const xmlChar *)"save"
#define YANGCLI_SET     (const xmlChar *)"set"
#define YANGCLI_SGET    (const xmlChar *)"sget"
#define YANGCLI_SGET_CONFIG   (const xmlChar *)"sget-config"
#define YANGCLI_SHOW    (const xmlChar *)"show"
#define YANGCLI_XGET    (const xmlChar *)"xget"
#define YANGCLI_XGET_CONFIG   (const xmlChar *)"xget-config"


/* specialized prompts for the fill command */
#define YANGCLI_PR_LLIST (const xmlChar *)"Add another leaf-list?"
#define YANGCLI_PR_LIST (const xmlChar *)"Add another list?"


#define YANGCLI_DEF_AGENT (const xmlChar *)"default"


/********************************************************************
*                                                                   *
*                          T Y P E S                                *
*                                                                   *
*********************************************************************/

/* cache the module pointers known by a particular agent,
 * as reported in the session <hello> message
 */
typedef struct modptr_t_ {
    dlq_hdr_t            qhdr;
    ncx_module_t         *mod;
} modptr_t;


/* save the requested result format type */
typedef enum result_format_t {
    RF_NONE,
    RF_TEXT,
    RF_XML
} result_format_t;


/* NETCONF agent control block */
typedef struct agent_cb_t_ {
    dlq_hdr_t            qhdr;
    xmlChar             *name;
    xmlChar             *address;
    xmlChar             *password;
    const xmlChar       *default_target;
    val_value_t         *connect_valset; 

    /* assignment statement support */
    xmlChar             *result_name;
    var_type_t          result_vartype;
    xmlChar             *result_filename;
    result_format_t      result_format;

    /* per-agent shadows of global config vars */
    boolean              get_optional;
    uint32               timeout;
    ncx_bad_data_t       baddata;
    log_debug_t          log_level;
    boolean              autoload;
    boolean              fixorder;
    op_testop_t          testoption;
    op_errop_t           erroption;

    /* session support */
    mgr_io_state_t       state;
    ses_id_t             mysid;

    /* TBD: session-specific user variables */
    dlq_hdr_t            varbindQ;   /* Q of ncx_var_t */

    /* contains only the modules that the agent is using
     * plus the 'netconf.yang' module
     */
    dlq_hdr_t            modptrQ;     /* Q of modptr_t */
} agent_cb_t;


/* logging function template to switch between
 * log_stdout and log_write
 */
typedef void (*logfn_t) (const char *fstr, ...);


/********************************************************************
*                                                                   *
*             F O R W A R D   D E C L A R A T I O N S               *
*                                                                   *
*********************************************************************/

/* forward decl needed by do_save function */
static void
    conn_command (agent_cb_t *agent_cb,
		  xmlChar *line);

/* forward decl needed by do_run function */
static void
    top_command (agent_cb_t *agent_cb,
		 xmlChar *line);

/* forward decl needed by send_copy_config_to_agent function */
static void
    yangcli_reply_handler (ses_cb_t *scb,
			   mgr_rpc_req_t *req,
			   mgr_rpc_rpy_t *rpy);


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

/* CLI input buffer */
static xmlChar         clibuff[YANGCLI_BUFFLEN];

/* CLI generic 'more' mode */
static boolean         climore;

/* CLI prompt function-specific extension mode */
static const xmlChar  *cli_fn;

/* libtecla data structure for 1 CLI context */
static GetLine        *cli_gl;

/* program version string */
static char progver[] = "0.9.1";


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


/********************************************************************
* FUNCTION is_top_command
* 
* Check if command name is a top command
* Must be full name
*
* INPUTS:
*   rpcname == command name to check
*
* RETURNS:
*   TRUE if this is a top command
*   FALSE if not
*********************************************************************/
static boolean
    is_top_command (const xmlChar *rpcname)
{
    if (!xml_strcmp(rpcname, YANGCLI_CD)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_CONNECT)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_FILL)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_HELP)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_LIST)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_MGRLOAD)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_PWD)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_QUIT)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_RUN)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_SHOW)) {
	;
    } else {
	return FALSE;
    }
    return TRUE;

}  /* is_top_command */


/********************************************************************
* FUNCTION new_modptr
* 
*  Malloc and init a new module pointer block
* 
* INPUTS:
*    mod == module to cache in this struct
*
* RETURNS:
*   malloced modptr_t struct or NULL of malloc failed
*********************************************************************/
static modptr_t *
    new_modptr (ncx_module_t *mod)
{
    modptr_t  *modptr;

    modptr = m__getObj(modptr_t);
    if (!modptr) {
	return NULL;
    }
    memset(modptr, 0x0, sizeof(modptr_t));
    modptr->mod = mod;
    return modptr;

}  /* new_modptr */


/********************************************************************
* FUNCTION free_modptr
* 
*  Clean and free a module pointer block
* 
* INPUTS:
*    modptr == mod pointer block to free
*              MUST BE REMOVED FROM ANY Q FIRST
*
*********************************************************************/
static void
    free_modptr (modptr_t *modptr)
{
    m__free(modptr);

}  /* free_modptr */


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

    agent_cb = m__getObj(agent_cb_t);
    if (!agent_cb) {
	return NULL;
    }
    memset(agent_cb, 0x0, sizeof(agent_cb_t));
    dlq_createSQue(&agent_cb->varbindQ);
    dlq_createSQue(&agent_cb->modptrQ);

    agent_cb->name = xml_strdup(name);
    if (!agent_cb->name) {
	m__free(agent_cb);
	return NULL;
    }

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

    return agent_cb;

}  /* new_agent_cb */


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

    var_clean_varQ(&agent_cb->varbindQ);

    while (!dlq_empty(&agent_cb->modptrQ)) {
	modptr = (modptr_t *)dlq_deque(&agent_cb->modptrQ);
	free_modptr(modptr);
    }

    m__free(agent_cb);

}  /* free_agent_cb */


/********************************************************************
* FUNCTION clear_agent_cb_session
* 
*  Clean the current session data from an agent control block
* 
* INPUTS:
*    agent_cb == control block to use for clearing
*                the session data
*********************************************************************/
static void
    clear_agent_cb_session (agent_cb_t *agent_cb)
{

    modptr_t  *modptr;

    while (!dlq_empty(&agent_cb->modptrQ)) {
	modptr = (modptr_t *)dlq_deque(&agent_cb->modptrQ);
	free_modptr(modptr);
    }
    agent_cb->mysid = 0;
    agent_cb->state = MGR_IO_ST_IDLE;

}  /* clear_agent_cb_session */


/********************************************************************
* FUNCTION is_top
* 
* Check the state and determine if the top or conn
* mode is active
* 
* INPUTS:
*   agent state to use
*
* RETURNS:
*  TRUE if this is TOP mode
*  FALSE if this is CONN mode (or associated states)
*********************************************************************/
static boolean
    is_top (mgr_io_state_t state)
{
    switch (state) {
    case MGR_IO_ST_INIT:
    case MGR_IO_ST_IDLE:
	return TRUE;
    case MGR_IO_ST_CONNECT:
    case MGR_IO_ST_CONN_START:
    case MGR_IO_ST_SHUT:
    case MGR_IO_ST_CONN_IDLE:
    case MGR_IO_ST_CONN_RPYWAIT:
    case MGR_IO_ST_CONN_CANCELWAIT:
    case MGR_IO_ST_CONN_CLOSEWAIT:
    case MGR_IO_ST_CONN_SHUT:
	return FALSE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

}  /* is_top */


/********************************************************************
* FUNCTION use_agentcb
* 
* Check if the agent_cb should be used for modules right now
*
* INPUTS:
*   agent_cb == agent control block to check
*
* RETURNS:
*   TRUE to use agent_cb
*   FALSE if not
*********************************************************************/
static boolean
    use_agentcb (agent_cb_t *agent_cb)
{
    if (!agent_cb || is_top(agent_cb->state)) {
	return FALSE;
    } else if (dlq_empty(&agent_cb->modptrQ)) {
	return FALSE;
    }
    return TRUE;
}  /* use_agentcb */


/********************************************************************
* FUNCTION find_module
* 
*  Check the agent_cb for the specified module; if not found
*  then try ncx_find_module
* 
* INPUTS:
*    agent_cb == control block to free
*    modname == module name
*
* RETURNS:
*   pointer to the requested module
*      using the registered 'current' version
*   NULL if not found
*********************************************************************/
static ncx_module_t *
    find_module (agent_cb_t *agent_cb,
		 const xmlChar *modname)
{

    modptr_t      *modptr;
    ncx_module_t  *mod;

    if (use_agentcb(agent_cb)) {
	for (modptr = (modptr_t *)dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (modptr_t *)dlq_nextEntry(modptr)) {

	    if (!xml_strcmp(modptr->mod->name, modname)) {
		return modptr->mod;
	    }
	}
    }

    mod = ncx_find_module(modname, NULL);

    return mod;

}  /* find_module */


/********************************************************************
* FUNCTION interactive_mode
* 
*  Check if the program is in interactive mode
* 
* RETURNS:
*   TRUE if insteractive mode, FALSE if script or batch mode
*********************************************************************/
static boolean
    interactive_mode (void)
{
    return (batchmode || runstack_level()) ? FALSE : TRUE;

}  /* interactive_mode */


/********************************************************************
* FUNCTION findparm
* 
* Get the specified string parm from the parmset and then
* make a strdup of the value
*
* INPUTS:
*   valset == value set to search
*   modname == optional module name defining the parameter to find
*   parmname  == name of parm to get, or partial name to get
*
* RETURNS:
*   pointer to val_value_t if found
*********************************************************************/
static val_value_t *
    findparm (val_value_t *valset,
	      const xmlChar *modname,
	      const xmlChar *parmname)
{
    val_value_t *parm;

    if (!valset) {
	return NULL;
    }

    parm = val_find_child(valset, modname, parmname);
    if (!parm && autocomp) {
	parm = val_match_child(valset, modname, parmname);
    }
    return parm;

}  /* findparm */


/********************************************************************
* FUNCTION get_strparm
* 
* Get the specified string parm from the parmset and then
* make a strdup of the value
*
* INPUTS:
*   valset == value set to check if not NULL
*   modname == module defining parmname
*   parmname  == name of parm to get
*
* RETURNS:
*   pointer to string !!! THIS IS A MALLOCED COPY !!!
*********************************************************************/
static xmlChar *
    get_strparm (val_value_t *valset,
		 const xmlChar *modname,
		 const xmlChar *parmname)
{
    val_value_t    *parm;
    xmlChar        *str;
    
    str = NULL;
    parm = findparm(valset, modname, parmname);
    if (parm) {
	str = xml_strdup(VAL_STR(parm));
	if (!str) {
	    log_error("\nyangcli: Out of Memory error");
	}
    }
    return str;

}  /* get_strparm */


/********************************************************************
* FUNCTION add_clone_parm
* 
*  Create a parm 
* 
* INPUTS:
*   val == value to clone and add
*   valset == value set to add parm into
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    add_clone_parm (const val_value_t *val,
		    val_value_t *valset)
{
    val_value_t    *parm;

    parm = val_clone(val);
    if (!parm) {
	log_error("\nyangcli: val_clone failed");
	return ERR_INTERNAL_MEM;
    } else {
	val_add_child(parm, valset);
    }
    return NO_ERR;

}  /* add_clone_parm */


/********************************************************************
* FUNCTION is_yangcli_ns
* 
*  Check the namespace and make sure this is an YANGCLI command
* 
* INPUTS:
*   ns == namespace ID to check
*
* RETURNS:
*  TRUE if this is the YANGCLI namespace ID
*********************************************************************/
static boolean
    is_yangcli_ns (xmlns_id_t ns)
{
    const xmlChar *modname;

    modname = xmlns_get_module(ns);
    if (modname && !xml_strcmp(modname, YANGCLI_MOD)) {
	return TRUE;
    } else {
	return FALSE;
    }

}  /* is_yangcli_ns */



/********************************************************************
 * FUNCTION clear_result
 * 
 * clear out the pending result info
 *
 * INPUTS:
 *   agent_cb == agent control block to use
 *
 *********************************************************************/
static void
    clear_result (agent_cb_t *agent_cb)

{
    if (agent_cb->result_name) {
	m__free(agent_cb->result_name);
	agent_cb->result_name = NULL;
    }
    if (agent_cb->result_filename) {
	m__free(agent_cb->result_filename);
	agent_cb->result_filename = NULL;
    }

}  /* clear_result */



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
	if (!VAL_STR(newval)) {
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
	    if (!dupval) {
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
		log_error("\n       (none, test, test-then-set, "
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
	/* output to the specified file */
	xml_init_attrs(&attrs);
	res = xml_wr_check_file(agent_cb->result_filename,
				resultval,
				&attrs, 
				XMLMODE, 
				WITHHDR, 
				NCX_DEF_INDENT,
				NULL);
	xml_clean_attrs(&attrs);
    } else if (resultstr) {
	fil = fopen((const char *)agent_cb->result_filename, "w");
	if (!fil) {
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
    } else {
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res == NO_ERR) {
	log_info("\nOK\n");
    }

    clear_result(agent_cb);

    return res;

}  /* output_file_result */


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
static status_t
    finish_result_assign (agent_cb_t *agent_cb,
			  val_value_t *resultvar,
			  const xmlChar *resultstr)
{
    val_value_t   *configvar;
    status_t       res;

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
	    if (!configvar) {
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
* FUNCTION check_filespec
* 
* Check the filespec string for a file assignment statement
* Save it if it si good
*
* INPUTS:
*    agent_cb == agent control block to use
*    filespec == string to check
*    varname == variable name to use in log_error
*              if this is complex form
*
* OUTPUTS:
*    agent_cb->result_filename will get set if NO_ERR
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_filespec (agent_cb_t *agent_cb,
		    const xmlChar *filespec,
		    const xmlChar *varname)
{
    const xmlChar *teststr;

    if (!filespec || !*filespec) {
	if (varname) {
	    log_error("\nError: file assignment variable '%s' "
		      "is empty string", varname);
	} else {
	    log_error("\nError: file assignment filespec "
		      "is empty string");
	}
	return ERR_NCX_INVALID_VALUE;
    }

    /* variable must be a string with only
     * valid filespec chars in it; no spaces
     * are allowed; too many security holes
     * if arbitrary strings are allowed here
     */
    if (val_need_quotes(filespec)) {
	if (varname) {
	    log_error("\nError: file assignment variable '%s' "
		      "contains whitespace (%s)", 
		      varname, filespec);
	} else {
	    log_error("\nError: file assignment filespec '%s' "
		      "contains whitespace", filespec);
	}
	return ERR_NCX_INVALID_VALUE;
    }

    /* check for acceptable chars */
    teststr = filespec;
    while (*teststr) {
	if (*teststr == NCXMOD_PSCHAR ||
	    *teststr == '.' ||
#ifdef WINDOWS
	    *teststr == ':' ||
#endif
	    ncx_valid_name_ch(*teststr)) {
	    teststr++;
	} else {
	    if (varname) {
		log_error("\nError: file assignment variable '%s' "
			  "contains invalid filespec (%s)", 
			  varname, filespec);
	    } else {
		log_error("\nError: file assignment filespec '%s' "
			  "contains invalid filespec", filespec);
	    }
	    return ERR_NCX_INVALID_VALUE;
	}
    }

    /* toss out the old value, if any */
    if (agent_cb->result_filename) {
	m__free(agent_cb->result_filename);
    }

    /* save the filename, may still be an invalid fspec  */
    agent_cb->result_filename = xml_strdup(filespec);
    if (!agent_cb->result_filename) {
	return ERR_INTERNAL_MEM;
    }
    return NO_ERR;

}  /* check_filespec */


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
	    if (!curval) {
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
	if (!tempstr) {
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
	if (!obj || !xml_strcmp(val->name, NCX_EL_STRING)) {
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
		if (!curval) {
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
	    if (!agent_cb->result_name) {
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
* FUNCTION parse_rpc_cli
* 
*  Call the cli_parse for an RPC input value set
* 
* INPUTS:
*   rpc == RPC to parse CLI for
*   line == input line to parse, starting with the parms to parse
*   res == pointer to status output
*
* OUTPUTS: 
*   *res == status
*
* RETURNS:
*    pointer to malloced value set or NULL if none created,
*    may have errors, check *res
*********************************************************************/
static val_value_t *
    parse_rpc_cli (const obj_template_t *rpc,
		   const xmlChar *args,
		   status_t  *res)
{
    const obj_template_t   *obj;
    const xmlChar          *myargv[2];

    /* construct an argv array, 
     * convert the CLI into a parmset 
     */
    obj = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (obj && obj_get_child_count(obj)) {
	myargv[0] = obj_get_name(rpc);
	myargv[1] = args;
	return cli_parse(2, (const char **)myargv, 
			 obj, VALONLY, SCRIPTMODE,
			 autocomp, res);
    } else {
	*res = ERR_NCX_SKIPPED;
	return NULL;
    }
    /*NOTREACHED*/

}  /* parse_rpc_cli */


/********************************************************************
* FUNCTION get_prompt
* 
* Construct the CLI prompt for the current state
* 
* INPUTS:
*   buff == bufffer to hold prompt (zero-terminated string)
*   bufflen == length of buffer (max bufflen-1 chars can be printed
*
*********************************************************************/
static void
    get_prompt (agent_cb_t *agent_cb,
		xmlChar *buff,
		uint32 bufflen)
{
    xmlChar        *p;
    val_value_t    *parm;
    uint32          len;

    if (climore) {
	xml_strncpy(buff, MORE_PROMPT, bufflen);
	return;
    }

    switch (agent_cb->state) {
    case MGR_IO_ST_INIT:
    case MGR_IO_ST_IDLE:
    case MGR_IO_ST_CONNECT:
    case MGR_IO_ST_CONN_START:
    case MGR_IO_ST_SHUT:
	if (cli_fn) {
	    if ((xml_strlen(DEF_FN_PROMPT) 
		 + xml_strlen(cli_fn) + 2) < bufflen) {
		p = buff;
		p += xml_strcpy(p, DEF_FN_PROMPT);
		p += xml_strcpy(p, cli_fn);
		xml_strcpy(p, (const xmlChar *)"> ");
	    } else {
		xml_strncpy(buff, DEF_PROMPT, bufflen);
	    }
	} else {
	    xml_strncpy(buff, DEF_PROMPT, bufflen);
	}
	break;
    case MGR_IO_ST_CONN_IDLE:
    case MGR_IO_ST_CONN_RPYWAIT:
    case MGR_IO_ST_CONN_CANCELWAIT:
    case MGR_IO_ST_CONN_CLOSEWAIT:
    case MGR_IO_ST_CONN_SHUT:
	p = buff;
	len = xml_strncpy(p, (const xmlChar *)"yangcli ", bufflen);
	p += len;
	bufflen -= len;

	if (bufflen == 0) {
	    return;
	}

	parm = NULL;
	if (agent_cb->connect_valset) {
	    parm = val_find_child(agent_cb->connect_valset, 
				  YANGCLI_MOD, YANGCLI_USER);
	}

	/*
	if (!parm && connect_valset) {
	    parm = val_find_child(connect_valset, 
				  YANGCLI_MOD, YANGCLI_USER);
	}
	*/

	if (parm) {
	    len = xml_strncpy(p, VAL_STR(parm), bufflen);
	    p += len;
	    bufflen -= len;
	    if (bufflen == 0) {
		return;
	    }
	    *p++ = NCX_AT_CH;
	    --bufflen;
	    if (bufflen == 0) {
		return;
	    }
	}

	parm = NULL;
	if (agent_cb->connect_valset) {
	    parm = val_find_child(agent_cb->connect_valset, 
				  YANGCLI_MOD, YANGCLI_AGENT);
	}

	/*
	if (!parm && connect_valset) {
	    parm= val_find_child(connect_valset, 
				 YANGCLI_MOD, YANGCLI_AGENT);
	}
	*/

	if (parm) {
	    len = xml_strncpy(p, VAL_STR(parm), bufflen);
	    p += len;
	    bufflen -= len;
	    if (bufflen == 0) {
		return;
	    }
	}

	if (cli_fn && bufflen > 3) {
	    *p++ = ':';
	    len = xml_strncpy(p, cli_fn, --bufflen);
	    p += len;
	    bufflen -= len;
	}

	if (bufflen > 2) {
	    *p++ = '>';
	    *p++ = ' ';
	    *p = 0;
	    bufflen -= 2;
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	xml_strncpy(buff, DEF_PROMPT, bufflen);	
	break;
    }

}  /* get_prompt */


/********************************************************************
* get_line
*
* Generate a prompt based on the program state and
* use the tecla library read function to handle
* user keyboard input
*
*
* Do not free this line after use, just discard it
* It will be freed by the tecla lib
*
* INPUTS:
*   agent_cb == agent control block to use
*
* RETURNS:
*   static line from tecla read line, else NULL if some error
*********************************************************************/
static xmlChar *
    get_line (agent_cb_t *agent_cb)
{
    xmlChar *line;
    xmlChar prompt[MAX_PROMPT_LEN];

    line = NULL;
    get_prompt(agent_cb, prompt, MAX_PROMPT_LEN-1);

    if (!climore) {
	log_stdout("\n");
    }
    line = (xmlChar *)gl_get_line(cli_gl,
				  (const char *)prompt,
				  NULL, -1);
    if (!line) {
	log_stdout("\nyangcli: Error: gl_get_line failed");
    }

    return line;

} /* get_line */


/********************************************************************
* FUNCTION get_cmd_line
* 
*  Read the current runstack context and construct
*  a command string for processing by do_run.
*    - Extended lines will be concatenated in the
*      buffer.  If a buffer overflow occurs due to this
*      concatenation, an error will be returned
* 
* INPUTS:
*   agent_cb == agent control block to use
*   res == address of status result
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to the command line to process (should treat as CONST !!!)
*   NULL if some error
*********************************************************************/
static xmlChar *
    get_cmd_line (agent_cb_t *agent_cb,
		  status_t *res)
{
    xmlChar        *start, *str;
    boolean         done;
    int             len, total, maxlen;

    /* init locals */
    total = 0;
    str = NULL;
    maxlen = sizeof(clibuff);
    done = FALSE;
    start = clibuff;

    /* get a command line, handling comment and continuation lines */
    while (!done) {

	/* read the next line from the user */
	str = get_line(agent_cb);
	if (!str) {
	    *res = ERR_NCX_READ_FAILED;
	    done = TRUE;
	    continue;
	}

	/* find end of string */
	len = xml_strlen(str);
	
	/* get rid of EOLN if present */
	if (len && str[len-1]=='\n') {
	    str[--len] = 0;
	}

	/* check line continuation */
	if (len && str[len-1]=='\\') {
	    /* get rid of the final backslash */
	    str[--len] = 0;
	    climore = TRUE;
	} else {
	    /* done getting lines */
	    *res = NO_ERR;
	    done = TRUE;
	}

	/* copy the string to the clibuff */
	if (total + len < maxlen) {
	    xml_strcpy(start, str);
	    start += len;
	    total += len;
	} else {
	    *res = ERR_BUFF_OVFL;
	    done = TRUE;
	}
	    
	str = NULL;
    }

    climore = FALSE;
    if (*res == NO_ERR) {
	return clibuff;
    } else {
	return NULL;
    }

}  /* get_cmd_line */


/********************************************************************
* FUNCTION try_parse_def
* 
* Parse the possibly module-qualified definition (module:def)
* and find the template for the requested definition
*
* INPUTS:
*   agent_cb == agent control block to use
*   modname == module name to try
*   defname == definition name to try
*   dtyp == definition type 
*
* OUTPUTS:
*    *dtyp is set if it started as NONE
*
* RETURNS:
*   pointer to the found definition template or NULL if not found
*********************************************************************/
static void *
    try_parse_def (agent_cb_t *agent_cb,
		   const xmlChar *modname,
		   const xmlChar *defname,
		   ncx_node_t *dtyp)

{
    void          *def;
    ncx_module_t  *mod;

    mod = find_module(agent_cb, modname);
    if (!mod) {
	return NULL;
    }

    def = NULL;
    switch (*dtyp) {
    case NCX_NT_NONE:
	def = ncx_find_object(mod, defname);
	if (def) {
	    *dtyp = NCX_NT_OBJ;
	    break;
	}
	def = ncx_find_grouping(mod, defname);
	if (def) {
	    *dtyp = NCX_NT_GRP;
	    break;
	}
	def = ncx_find_type(mod, defname);
	if (def) {
	    *dtyp = NCX_NT_TYP;
	    break;
	}
	break;
    case NCX_NT_OBJ:
	def = ncx_find_object(mod, defname);
	break;
    case NCX_NT_GRP:
	def = ncx_find_grouping(mod, defname);
	break;
    case NCX_NT_TYP:
	def = ncx_find_type(mod, defname);
	break;
    default:
	def = NULL;
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    return def;
    
} /* try_parse_def */


/********************************************************************
* FUNCTION parse_def
* 
* Definitions have two forms:
*   def       (default module used)
*   module:def (explicit module name used)
*   prefix:def (if prefix-to-module found, explicit module name used)
*
* Parse the possibly module-qualified definition (module:def)
* and find the template for the requested definition
*
* INPUTS:
*   agent_cb == agent control block to use
*   dtyp == definition type 
*       (NCX_NT_OBJ or  NCX_NT_TYP)
*   line == input command line from user
*   len  == pointer to output var for number of bytes parsed
*
* OUTPUTS:
*    *dtyp is set if it started as NONE
*    *len == number of bytes parsed
*
* RETURNS:
*   pointer to the found definition template or NULL if not found
*********************************************************************/
static void *
    parse_def (agent_cb_t *agent_cb,
	       ncx_node_t *dtyp,
	       xmlChar *line,
	       uint32 *len)
{
    void           *def;
    xmlChar        *start, *p, *q, oldp, oldq;
    const xmlChar  *prefix, *defname, *modname;
    ncx_module_t   *mod;
    obj_template_t *obj;
    modptr_t       *modptr;
    uint32          prelen;
    xmlns_id_t      nsid;
    
    def = NULL;
    q = NULL;
    prelen = 0;
    *len = 0;
    start = line;

    /* skip any leading whitespace */
    while (*start && xml_isspace(*start)) {
	start++;
    }

    p = start;

    /* look for a colon or EOS or whitespace to end method name */
    while (*p && (*p != ':') && !xml_isspace(*p)) {
	p++;
    }

    /* make sure got something */
    if (p==start) {
	return NULL;
    }

    /* search for a module prefix if a separator was found */
    if (*p == ':') {

	/* use an explicit module prefix in YANG */
	prelen = p - start;
	q = p+1;
	while (*q && !xml_isspace(*q)) {
	    q++;
	}
	*len = q - line;

	oldq = *q;
	*q = 0;
	oldp = *p;
	*p = 0;

	prefix = start;
	defname = p+1;
    } else {
	/* no module prefix, use default module, if any */
	*len = p - line;

	oldp = *p;
	*p = 0;

	/* try the default module, which will be NULL
	 * unless set by the default-module CLI param
	 */
	prefix = NULL;
	defname = start;
    }

    /* look in the registry for the definition name 
     * first check if only the user supplied a module name
     */
    if (prefix) {
	modname = NULL;
	nsid = xmlns_find_ns_by_prefix(prefix);
	if (nsid) {
	    modname = xmlns_get_module(nsid);
	}
	if (modname) {
	    def = try_parse_def(agent_cb,
				modname, defname, dtyp);
	} else {
	    log_error("\nError: no module found for prefix '%s'", 
		      prefix);
	}
    } else {
	def = try_parse_def(agent_cb,
			    YANGCLI_MOD, defname, dtyp);

	if (!def && default_module) {
	    def = try_parse_def(agent_cb,
				default_module, defname, dtyp);
	}
	if (!def && (!default_module ||
		     xml_strcmp(default_module, NC_MODULE))) {

	    def = try_parse_def(agent_cb,
				NC_MODULE, defname, dtyp);
	}

	/* if not found, try any module */
	if (!def) {
	    /* try any of the agent modules first */
	    if (use_agentcb(agent_cb)) {
		for (modptr = (modptr_t *)
			 dlq_firstEntry(&agent_cb->modptrQ);
		     modptr != NULL && !def;
		     modptr = (modptr_t *)dlq_nextEntry(modptr)) {

		    def = try_parse_def(agent_cb, 
					modptr->mod->name, 
					defname, 
					dtyp);
		}
	    }

	    /* try any of the manager-loaded modules */
	    for (mod = ncx_get_first_module();
		 mod != NULL && !def;
		 mod = ncx_get_next_module(mod)) {

		def = try_parse_def(agent_cb, 
				    mod->name, defname, dtyp);
	    }
	}

	/* if not found, try a partial RPC command name */
	if (!def && autoload) {
	    switch (*dtyp) {
	    case NCX_NT_NONE:
	    case NCX_NT_OBJ:
		if (use_agentcb(agent_cb)) {
		    for (modptr = (modptr_t *)
			     dlq_firstEntry(&agent_cb->modptrQ);
			 modptr != NULL && !def;
			 modptr = (modptr_t *)dlq_nextEntry(modptr)) {

			def = ncx_match_any_rpc(modptr->mod->name, 
						defname);
			if (def) {
			    *dtyp = NCX_NT_OBJ;
			}
		    }
		}
		if (!def) {
		    def = ncx_match_any_rpc(NULL, defname);
		    if (def) {
			obj = (obj_template_t *)def;
			if (obj_get_nsid(obj) == xmlns_nc_id()) {
			    /* matched a NETCONF RPC and not connected;
			     * would have matched in the use_agentcb()
			     * code above if this is a partial NC op
			     */
			    def = NULL;
			} else {
			    *dtyp = NCX_NT_OBJ;
			}
		    }
		}
		break;
	    default:
		;
	    }
	}
    }

    /* restore string as needed */
    *p = oldp;
    if (q) {
	*q = oldq;
    }

    return def;
    
} /* parse_def */


/********************************************************************
* FUNCTION get_yesno
* 
* Get the user-selected choice, yes or no
*
* INPUTS:
*   agent_cb == agent control block to use
*   prompt == prompt message
*   defcode == default answer code
*      0 == no def answer
*      1 == def answer is yes
*      2 == def answer is no
*   retcode == address of return code
*
* OUTPUTS:
*    *retcode is set if return NO_ERR
*       0 == cancel
*       1 == yes
*       2 == no
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_yesno (agent_cb_t *agent_cb,
	       const xmlChar *prompt,
	       uint32 defcode,
	       uint32 *retcode)
{
    xmlChar                 *myline, *str;
    status_t                 res;
    boolean                  done;


    res = NO_ERR;

    if (prompt) {
	log_stdout("\n%s", prompt);
    }
    log_stdout("\nEnter Y for yes, N for no, or C to cancel:");
    switch (defcode) {
    case YESNO_NODEF:
	break;
    case YESNO_YES:
	log_stdout(" [default: Y]");
	break;
    case YESNO_NO:
	log_stdout(" [default: N]");
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    done = FALSE;
    while (!done) {

	/* get input from the user STDIN */
	myline = get_cmd_line(agent_cb, &res);
	if (!myline) {
	    return res;
	}

	/* strip leading whitespace */
	str = myline;
	while (*str && xml_isspace(*str)) {
	    str++;
	}

	/* convert to a number, check [ENTER] for default */
	if (*str) {
	    if (*str == 'Y' || *str == 'y') {
		*retcode = YESNO_YES;
		done = TRUE;
	    } else if (*str == 'N' || *str == 'n') {
		*retcode = YESNO_NO;
		done = TRUE;
	    } else if (*str == 'C' || *str == 'c') {
		*retcode = YESNO_CANCEL;
		done = TRUE;
	    }
	} else {
	    /* default accepted */
	    switch (defcode) {
	    case YESNO_NODEF:
		break;
	    case YESNO_YES:
		*retcode = YESNO_YES;
		done = TRUE;
		break;
	    case YESNO_NO:
		*retcode = YESNO_NO;
		done = TRUE;
		break;
	    default:
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
	if (!done) {
	    log_stdout("\nError: invalid value '%s'\n", str);
	}
    }
    return res;

}  /* get_yesno */


/********************************************************************
* FUNCTION get_complex_parm
* 
* Fill the specified parm, which is a complex type
* This function will block on readline to get input from the user
*
* INPUTS:
*   agent_cb == agent control block to use
*   parm == parm to get from the CLI
*   valset == value set being filled
*
* OUTPUTS:
*    new val_value_t node will be added to valset if NO_ERR
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_complex_parm (agent_cb_t *agent_cb,
		      const obj_template_t *parm,
		      val_value_t *valset)
{
    xmlChar          *line;
    val_value_t      *new_parm;
    status_t          res;

    res = NO_ERR;

    log_stdout("\nEnter parameter value %s (%s)", 
	       obj_get_name(parm), 
	       tk_get_btype_sym(obj_get_basetype(parm)));

    /* get a line of input from the user */
    line = get_cmd_line(agent_cb, &res);
    if (!line) {
	return res;
    }

    new_parm = val_new_value();
    if (!new_parm) {
	res = ERR_INTERNAL_MEM;
    } else {
	val_init_from_template(new_parm, parm);
	(void)var_get_script_val(parm,
				 new_parm,
				 line, ISPARM, &res);
	if (res == NO_ERR) {
	    /* add the parm to the parmset */
	    val_add_child(new_parm, valset);
	}
    }

    if (res != NO_ERR) {
	log_stdout("\nyangcli: Error in %s (%s)",
		   obj_get_name(parm), get_error_string(res));
    }

    return res;
    
} /* get_complex_parm */


/********************************************************************
* FUNCTION get_parm
* 
* Fill the specified parm
* Use the old value for the default if any
* This function will block on readline to get input from the user
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method that is being called
*   parm == parm to get from the CLI
*   valset == value set being filled
*   oldvalset == last set of values (NULL if none)
*
* OUTPUTS:
*    new val_value_t node will be added to valset if NO_ERR
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_parm (agent_cb_t *agent_cb,
	      const obj_template_t *rpc,
	      const obj_template_t *parm,
	      val_value_t *valset,
	      val_value_t *oldvalset)
{
    const xmlChar    *def, *parmname, *str;
    const typ_def_t  *typdef;
    val_value_t      *oldparm, *newparm;
    xmlChar          *line, *start, *objbuff, *buff;
    xmlChar          *line2, *start2, *saveline;
    status_t          res;
    ncx_btype_t       btyp;
    boolean           done;
    uint32            len;

    if (!obj_is_mandatory(parm) && !agent_cb->get_optional) {
	return NO_ERR;
    }

    res = NO_ERR;

    if (obj_is_data_db(parm)) {
	objbuff = NULL;
	res = obj_gen_object_id(parm, &objbuff);
	if (res != NO_ERR) {
	    log_error("\nError: generate object ID failed (%s)",
		      get_error_string(res));
	    return res;
	}

	/* let the user know about the new nest level */
	if (obj_is_key(parm)) {
	    str = YANG_K_KEY;
	} else if (obj_is_mandatory(parm)) {
	    str = YANG_K_MANDATORY;
	} else {
	    str = (const xmlChar *)"optional";
	}

	log_stdout("\nFilling %s %s %s:", str,
		   obj_get_typestr(parm), objbuff);
	    
	m__free(objbuff);
    }

    switch (btyp) {
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
	return get_complex_parm(agent_cb, parm, valset);
    default:
	;
    }

    parmname = obj_get_name(parm);
    typdef = obj_get_ctypdef(parm);
    btyp = obj_get_basetype(parm);
    res = NO_ERR;
    oldparm = NULL;
    def = NULL;
  
    done = FALSE;
    while (!done) {
	if (btyp==NCX_BT_EMPTY) {
	    log_stdout("\nShould flag %s be set? (Y, N, %s)", 
		       parmname, DEF_OPTIONS);
	} else {
	    if (typdef && typdef->typename) {
		def = typdef->typename;
	    } else {
		def = (const xmlChar *)tk_get_btype_sym(btyp);
	    }
	    log_stdout("\nEnter value for %s %s (%s, %s)", 
		       obj_get_typestr(parm),
		       parmname, def, DEF_OPTIONS);
	    def = NULL;
	}
	if (oldvalset) {
	    oldparm = val_find_child(oldvalset, 
				     obj_get_mod_name(parm),
				     parmname);
	}

	/* pick a default value, either old value or default clause */
	if (!oldparm) {
	    /* try to get the defined default value */
	    if (btyp != NCX_BT_EMPTY) {
		def = obj_get_default(parm);
		if (!def && (obj_get_nsid(rpc) == xmlns_nc_id() &&
			     (!xml_strcmp(parmname, NCX_EL_TARGET) ||
			      !xml_strcmp(parmname, NCX_EL_SOURCE)))) {
		    /* offer the default target for the NETCONF
		     * <source> and <target> parameters
		     */
		    def = agent_cb->default_target;
		}
	    }
	    if (def) {
		log_stdout(" [%s]\n", def);
	    } else if (btyp==NCX_BT_EMPTY) {
		log_stdout(" [N]\n");
	    }
	} else {
	    /* use the old value for the default */
	    log_stdout(" [");
	    if (btyp==NCX_BT_EMPTY) {
		log_stdout("Y");
	    } else {
		res = val_sprintf_simval_nc(NULL, oldparm, &len);
		if (res != NO_ERR) {
		    return SET_ERROR(res);
		}
		buff = m__getMem(len+1);
		if (!buff) {
		    return ERR_INTERNAL_MEM;
		}
		res = val_sprintf_simval_nc(buff, oldparm, &len);
		if (res == NO_ERR) {
		    log_stdout("%s", buff);
		}
		m__free(buff);
	    }
	    log_stdout("]\n");
	}

	/* get a line of input from the user */
	line = get_cmd_line(agent_cb, &res);
	if (!line) {
	    return res;
	}

	/* skip whitespace */
	start = line;
	while (*start && xml_isspace(*start)) {
	    start++;
	}

	/* check for question-mark char sequences */
	if (*start == '?') {
	    if (start[1] == '?') {
		/* ?? == full help */
		obj_dump_template(parm, HELP_MODE_FULL, 0,
				  NCX_DEF_INDENT);
	    } else if (start[1] == 'C' || start[1] == 'c') {
		/* ?c or ?C == cancel the operation */
		log_stdout("\n%s command canceled",
			   obj_get_name(rpc));
		return ERR_NCX_CANCELED;
	    } else if (start[1] == 'S' || start[1] == 's') {
		/* ?s or ?S == skip this parameter */
		log_stdout("\n%s parameter skipped",
			   obj_get_name(parm));
		return ERR_NCX_SKIPPED;
	    } else {
		/* ? == normal help mode */
		obj_dump_template(parm, HELP_MODE_NORMAL, 4,
				  NCX_DEF_INDENT);
	    }
	    log_stdout("\n");
	    continue;
	} else {
	    /* top loop to get_parm is only executed once */
	    done = TRUE;
	}
    }

    /* check if any non-whitespace chars entered */
    if (!*start) {
	/* no input, use default or old value or EMPTY_STRING */
	if (def) {
	    /* use default */
	    res = cli_parse_parm_ex(valset, parm, 
				    def, SCRIPTMODE, baddata);
	} else if (oldparm) {
	    /* no default, try old value */
	    if (btyp==NCX_BT_EMPTY) {
		res = cli_parse_parm_ex(valset, parm, NULL,
					SCRIPTMODE, baddata);
	    } else {
		/* use a copy of the last value */
		newparm = val_clone(oldparm);
		if (!newparm) {
		    res = ERR_INTERNAL_MEM;
		} else {
		    val_add_child(newparm, valset);
		}
	    }
	} else if (btyp == NCX_BT_EMPTY) {
	    res = cli_parse_parm_ex(valset, parm, NULL,
				    SCRIPTMODE, baddata);
	} else if (val_simval_ok(obj_get_ctypdef(parm), 
				 EMPTY_STRING) == NO_ERR) {
	    res = cli_parse_parm_ex(valset, parm, EMPTY_STRING,
				    SCRIPTMODE, baddata);
	} else {
	    /* data type requires some form of input */
	    res = ERR_NCX_DATA_MISSING;
	}  /* else flag should not be set */
    } else if (btyp==NCX_BT_EMPTY) {
	/* empty data type handled special Y: set, N: leave out */
	if (*start=='Y' || *start=='y') {
	    res = cli_parse_parm_ex(valset, parm, NULL, 
				    SCRIPTMODE, baddata);
	} else if (*start=='N' || *start=='n') {
	    ; /* skip; do not add the flag */
	} else if (oldparm) {
	    /* previous value was set, so add this flag */
	    res = cli_parse_parm_ex(valset, parm, NULL,
				    SCRIPTMODE, baddata);
	} else {
	    /* some value was entered, other than Y or N */
	    res = ERR_NCX_WRONG_VAL;
	}
    } else {
	/* normal case: input for regular data type */
	res = cli_parse_parm_ex(valset, parm, start,
				SCRIPTMODE, baddata);
    }

    if (res != NO_ERR) {
	switch (baddata) {
	case NCX_BAD_DATA_IGNORE:
	case NCX_BAD_DATA_WARN:
	    /* if these modes had error return status then the
	     * problem was not invalid value; maybe malloc error
	     */
	    break;
	case NCX_BAD_DATA_CHECK:
	    if (NEED_EXIT(res)) {
		break;
	    }

	    saveline = (start) ? xml_strdup(start) :
		xml_strdup(EMPTY_STRING);
	    if (!saveline) {
		res = ERR_INTERNAL_MEM;
		break;
	    }

	    done = FALSE;
	    while (!done) {
		log_stdout("\nError: parameter '%s' value '%s' is invalid"
			   "\nShould this value be used anyway? (Y, N, %s)"
			   " [N]", 
			   obj_get_name(parm),
			   (start) ? start : EMPTY_STRING,
			   DEF_OPTIONS);

		/* save the previous value because it is about
		 * to get trashed by getting a new line
		 */

		/* get a line of input from the user */
		line2 = get_cmd_line(agent_cb, &res);
		if (!line2) {
		    m__free(saveline);
		    return res;
		}

		/* skip whitespace */
		start2 = line2;
		while (*start2 && xml_isspace(*start2)) {
		    start2++;
		}

		/* check for question-mark char sequences */
		if (!*start2) {
		    /* default N: try again for a different input */
		    m__free(saveline);
		    res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
		    done = TRUE;
		} else if (*start2 == '?') {
		    if (start2[1] == '?') {
			/* ?? == full help */
			obj_dump_template(parm, HELP_MODE_FULL, 0,
					  NCX_DEF_INDENT);
		    } else if (start2[1] == 'C' || start2[1] == 'c') {
			/* ?c or ?C == cancel the operation */
			log_stdout("\n%s command canceled",
				   obj_get_name(rpc));
			m__free(saveline);
			res = ERR_NCX_CANCELED;
			done = TRUE;
		    } else if (start2[1] == 'S' || start2[1] == 's') {
			/* ?s or ?S == skip this parameter */
			log_stdout("\n%s parameter skipped",
				   obj_get_name(parm));
			m__free(saveline);
			res = ERR_NCX_SKIPPED;
			done = TRUE;
		    } else {
			/* ? == normal help mode */
			obj_dump_template(parm, HELP_MODE_NORMAL, 4,
					  NCX_DEF_INDENT);
		    }
		    log_stdout("\n");
		    continue;
		} else if (*start2 == 'Y' || *start2 == 'y') {
		    /* use the invalid value */
		    res = cli_parse_parm_ex(valset, parm, 
					    saveline, SCRIPTMODE, 
					    NCX_BAD_DATA_IGNORE);
		    m__free(saveline);
		    done = TRUE;
		} else if (*start2 == 'N' || *start2 == 'n') {
		    /* recurse: try again for a different input */
		    m__free(saveline);
		    res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
		    done = TRUE;
		} else {
		    log_stdout("\nInvalid input.");
		}
	    }
	    break;
	case NCX_BAD_DATA_ERROR:
	    log_stdout("\nError: set parameter '%s' failed (%s)\n",
		       obj_get_name(parm),
		       get_error_string(res));
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    return res;
    
} /* get_parm */


/********************************************************************
* FUNCTION get_case
* 
* Get the user-selected case, which is not fully set
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC template in progress
*   cas == case object template header
*   valset == value set to fill
*   oldvalset == last set of values (or NULL if none)
*
* OUTPUTS:
*    new val_value_t nodes may be added to valset
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_case (agent_cb_t *agent_cb,
	      const obj_template_t *rpc,
	      const obj_template_t *cas,
	      val_value_t *valset,
	      val_value_t *oldvalset)
{
    const obj_template_t    *parm;
    val_value_t             *pval;
    status_t                 res;
    boolean                  saveopt;

    /* make sure a case was selected or found */
    if (!obj_is_config(cas) || obj_is_abstract(cas)) {
	log_stdout("\nError: No writable objects to fill for this case");
	return ERR_NCX_SKIPPED;
    }

    saveopt = agent_cb->get_optional;
    agent_cb->get_optional = TRUE;
    res = NO_ERR;

    /* corner-case: user selected a case, and that case has
     * one empty leaf in it; 
     * e.g., <source> and <target> parms
     */
    if (obj_get_child_count(cas) == 1) {
	parm = obj_first_child(cas);
	if (parm && obj_get_basetype(parm)==NCX_BT_EMPTY) {
	    return cli_parse_parm(valset, parm, NULL, FALSE);
	}
    }

    /* finish the selected case */
    for (parm = obj_first_child(cas);
	 parm != NULL && res == NO_ERR;
	 parm = obj_next_child(parm)) {

	if (!obj_is_config(parm) || obj_is_abstract(parm)) {
	    continue;
	}

	pval = val_find_child(valset, 
			      obj_get_mod_name(parm),
			      obj_get_name(parm));
	if (pval) {
	    continue;
	}

	/* node is config and not already set */
	res = get_parm(agent_cb, rpc, parm, valset, oldvalset);

	if (res == ERR_NCX_SKIPPED) {
	    res = NO_ERR;
	}
    }

    agent_cb->get_optional = saveopt;
    return res;

} /* get_case */


/********************************************************************
* FUNCTION get_choice
* 
* Get the user-selected choice, which is not fully set
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC template in progress
*   choic == choice object template header
*   valset == value set to fill
*   oldvalset == last set of values (or NULL if none)
*
* OUTPUTS:
*    new val_value_t nodes may be added to valset
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_choice (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const obj_template_t *choic,
		val_value_t *valset,
		val_value_t *oldvalset)
{
    const obj_template_t    *parm, *cas, *usecase;
    val_value_t             *pval;
    xmlChar                 *myline, *str, *objbuff;
    status_t                 res;
    int                      casenum, num;
    boolean                  first, done, usedef, redo, saveopt;

    if (!obj_is_config(choic) || obj_is_abstract(choic)) {
	log_stdout("\nError: choice '%s' has no configurable parameters",
		   obj_get_name(choic));
	return ERR_NCX_ACCESS_DENIED;
    }

    res = NO_ERR;

    if (obj_is_data_db(choic)) {
	objbuff = NULL;
	res = obj_gen_object_id(choic, &objbuff);
	if (res != NO_ERR) {
	    log_error("\nError: generate object ID failed (%s)",
		      get_error_string(res));
	    return res;
	}

	/* let the user know about the new nest level */
	log_stdout("\nFilling choice %s:", objbuff);
	m__free(objbuff);
    }

    saveopt = agent_cb->get_optional;
    
    if (obj_is_mandatory(choic)) {
	agent_cb->get_optional = TRUE;
    }

    /* first check the partial block corner case */
    pval = val_get_choice_first_set(valset, choic);
    if (pval) {
	/* found something set from this choice, finish the case */
	log_stdout("\nEnter more parameters to complete the choice:");

	cas = pval->casobj;
	if (!cas) {
	    agent_cb->get_optional = saveopt;
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	for (parm = obj_first_child(cas); 
	     parm != NULL;
	     parm = obj_next_child(parm)) {

	    if (!obj_is_config(parm) || obj_is_abstract(parm)) {
		continue;
	    }

	    pval = val_find_child(valset,
				  obj_get_mod_name(parm),
				  obj_get_name(parm));
	    if (pval) {
		continue;   /* node within case already set */
	    }

	    res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
	    switch (res) {
	    case NO_ERR:
		break;
	    case ERR_NCX_SKIPPED:
		res = NO_ERR;
		break;
	    case ERR_NCX_CANCELED:
		agent_cb->get_optional = saveopt;
		return res;
	    default:
		agent_cb->get_optional = saveopt;
		return res;
	    }
	}
	agent_cb->get_optional = saveopt;
	return NO_ERR;
    }

    /* check corner-case -- choice with no cases defined */
    cas = obj_first_child(choic);
    if (!cas) {
	log_stdout("\nNo case nodes defined for choice %s\n",
		   obj_get_name(choic));
	agent_cb->get_optional = saveopt;
	return NO_ERR;
    }


    /* else not a partial block corner case but a normal
     * situation where no case has been selected at all
     */
    log_stdout("\nEnter a number of the selected case statement:\n");

    num = 1;
    usedef = FALSE;

    for (; cas != NULL;
         cas = obj_next_child(cas)) {

	first = TRUE;
	for (parm = obj_first_child(cas);
	     parm != NULL;
	     parm = obj_next_child(parm)) {

	    if (!obj_is_config(parm) || obj_is_abstract(parm)) {
		continue;
	    }

	    if (first) {
		log_stdout("\n  %d: case %s:", 
			   num++, obj_get_name(cas));
		first = FALSE;
	    }

	    log_stdout("\n       %s %s",
		       obj_get_typestr(parm),
		       obj_get_name(parm));
	}
    }

    done = FALSE;
    log_stdout("\n");
    while (!done) {

	redo = FALSE;

	/* Pick a prompt, depending on the choice default case */
	if (obj_get_default(choic)) {
	    log_stdout("\nEnter choice number (%d - %d, %s), "
		       "[ENTER] for default (%s): ",
		       1, num-1, DEF_OPTIONS,
		       obj_get_default(choic));
	} else {
	    log_stdout("\nEnter choice number (%d - %d, %s): ",
		       1, num-1, DEF_OPTIONS);
	}

	/* get input from the user STDIN */
	myline = get_cmd_line(agent_cb, &res);
	if (!myline) {
	    agent_cb->get_optional = saveopt;
	    return res;
	}

	/* strip leading whitespace */
	str = myline;
	while (*str && xml_isspace(*str)) {
	    str++;
	}

	/* convert to a number, check [ENTER] for default */
	if (!*str) {
	    usedef = TRUE;
	} else if (*str == '?') {
	    redo = TRUE;
	    if (str[1] == '?') {
		obj_dump_template(choic, HELP_MODE_FULL, 0,
				  NCX_DEF_INDENT);
	    } else if (str[1] == 'C' || str[1] == 'c') {
		log_stdout("\n%s command canceled\n",
			   obj_get_name(rpc));
		agent_cb->get_optional = saveopt;
		return ERR_NCX_CANCELED;
	    } else if (str[1] == 'S' || str[1] == 's') {
		log_stdout("\n%s choice skipped\n",
			   obj_get_name(choic));
		agent_cb->get_optional = saveopt;
		return ERR_NCX_SKIPPED;
	    } else {
		obj_dump_template(choic, HELP_MODE_NORMAL, 4,
				  NCX_DEF_INDENT);
	    }
	    log_stdout("\n");
	} else {
	    casenum = atoi((const char *)str);
	    usedef = FALSE;
	}

	if (redo) {
	    continue;
	}

	/* check if default requested */
	if (usedef) {
	    if (obj_get_default(choic)) {
		done = TRUE;
	    } else {
		log_stdout("\nError: Choice does not have a default case\n");
		usedef = FALSE;
	    }
	} else if (casenum < 0 || casenum >= num) {
	    log_stdout("\nError: invalid value '%s'\n", str);
	} else {
	    done = TRUE;
	}
    }

    /* got a valid choice number or use the default case
     * now get the object template for the correct case 
     */
    if (usedef) {
	cas = obj_find_child(choic,
			     obj_get_mod_name(choic),
			     obj_get_default(choic));
    } else {

	num = 1;
	done = FALSE;
	usecase = NULL;

	for (cas = obj_first_child(choic); 
	     cas != NULL && !done;
	     cas = obj_next_child(cas)) {

	    if (!obj_is_config(cas) || obj_is_abstract(cas)) {
		continue;
	    }

	    if (casenum == num) {
		done = TRUE;
		usecase = cas;
	    } else {
		num++;
	    }
	}

	cas = usecase;
    }

    /* make sure a case was selected or found */
    if (!cas) {
	log_stdout("\nError: No case to fill for this choice");
	agent_cb->get_optional = saveopt;
	return ERR_NCX_SKIPPED;
    }

    res = get_case(agent_cb, rpc, cas, valset, oldvalset);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_NCX_SKIPPED:
	res = NO_ERR;
	break;
    case ERR_NCX_CANCELED:
	break;
    default:
	;
    }

    agent_cb->get_optional = saveopt;

    return res;

} /* get_choice */


/********************************************************************
* FUNCTION fill_value
* 
* Malloc and fill the specified value
* Use the last value for the default (may be NULL)
* This function will block on readline for user input
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method that is being called
*   parm == object for value to fill
*   oldval == last value (or NULL if none)
*   res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced value, filled in or NULL if some error
*********************************************************************/
static val_value_t *
    fill_value (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const obj_template_t *parm,
		val_value_t *oldval,
		status_t  *res)
{
    const obj_template_t  *parentobj;
    val_value_t           *dummy, *newval;
    boolean                saveopt;

    switch (parm->objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	break;
    default:
	*res = SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    if (obj_is_abstract(parm)) {
	*res = ERR_NCX_NO_ACCESS_MAX;
	log_error("\nError: no access to abstract objects");
	return NULL;
    }

    /* since the CLI and original NCX language were never designed
     * to support top-level leafs, a dummy container must be
     * created for the new and old leaf or leaf-list entries
     */
    if (parm->parent && !obj_is_root(parm->parent)) {
	parentobj = parm->parent;
    } else {
	parentobj = ncx_get_gen_container();
    }

    dummy = val_new_value();
    if (!dummy) {
	*res = ERR_INTERNAL_MEM;
	log_error("\nError: malloc failed");
	return NULL;
    }
    val_init_from_template(dummy, parentobj);

    cli_fn = obj_get_name(rpc);

    if (oldval) {
	oldval = oldval->parent;
    }

    newval = NULL;
    saveopt = agent_cb->get_optional;
    agent_cb->get_optional = TRUE;
    *res = get_parm(agent_cb, rpc, parm, dummy, oldval);
    agent_cb->get_optional = saveopt;

    switch (*res) {
    case NO_ERR:
	break;
    case ERR_NCX_SKIPPED:
	*res = NO_ERR;
	break;
    case ERR_NCX_CANCELED:
    default:
	break;
    }

    if (*res == NO_ERR) {
	newval = val_get_first_child(dummy);
	if (newval) {
	    val_remove_child(newval);
	}
    }
    cli_fn = NULL;
    val_free_value(dummy);
    return newval;

} /* fill_value */


/********************************************************************
* FUNCTION fill_valset
* 
* Fill the specified value set with any missing parameters.
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
*   agent_cb == current agent control block (NULL if none)
*   rpc == RPC method that is being called
*   valset == value set to fill
*   oldvalset == last set of values (or NULL if none)
*
* OUTPUTS:
*    new val_value_t nodes may be added to valset
*
* RETURNS:
*    status,, valset may be partially filled if not NO_ERR
*********************************************************************/
static status_t
    fill_valset (agent_cb_t *agent_cb,
		 const obj_template_t *rpc,
		 val_value_t *valset,
		 val_value_t *oldvalset)
{
    const obj_template_t  *parm;
    val_value_t           *val, *oldval;
    xmlChar               *objbuff;
    status_t               res;
    boolean                done;
    uint32                 yesnocode;

    res = NO_ERR;
    cli_fn = obj_get_name(rpc);

    if (obj_is_data_db(valset->obj)) {
	objbuff = NULL;
	res = obj_gen_object_id(valset->obj, &objbuff);
	if (res != NO_ERR) {
	    log_error("\nError: generate object ID failed (%s)",
		      get_error_string(res));
	    return res;
	}

	/* let the user know about the new nest level */
	log_stdout("\nFilling %s %s:",
		   obj_get_typestr(valset->obj), objbuff);
	m__free(objbuff);
    }

    for (parm = obj_first_child(valset->obj);
         parm != NULL && res==NO_ERR;
         parm = obj_next_child(parm)) {

	if (!obj_is_config(parm) || obj_is_abstract(parm)) {
	    continue;
	}

	if (!agent_cb->get_optional && !obj_is_mandatory(parm)) {
	    continue;
	}

        switch (parm->objtype) {
        case OBJ_TYP_CHOICE:
	    if (!val_choice_is_set(valset, parm)) {
		res = get_choice(agent_cb, rpc, parm, 
				 valset, oldvalset);
		switch (res) {
		case NO_ERR:
		    break;
		case ERR_NCX_SKIPPED:
		    res = NO_ERR;
		    break;
		case ERR_NCX_CANCELED:
		    break;
		default:
		    ;
		}
	    }
            break;
        case OBJ_TYP_LEAF:
	    val = val_find_child(valset, 
				 obj_get_mod_name(parm),
				 obj_get_name(parm));
	    if (!val) {
		res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
		switch (res) {
		case NO_ERR:
		    break;
		case ERR_NCX_SKIPPED:
		    res = NO_ERR;
		    break;
		case ERR_NCX_CANCELED:
		    break;
		default:
		    ;
		}
	    }
	    break;
	case OBJ_TYP_LEAF_LIST:
	    done = FALSE;
	    while (!done && res == NO_ERR) {
		res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
		switch (res) {
		case NO_ERR:
		    /* prompt for more leaf-list objects */
		    res = get_yesno(agent_cb, YANGCLI_PR_LLIST,
				    YESNO_NO, &yesnocode);
		    if (res == NO_ERR) {
			switch (yesnocode) {
			case YESNO_CANCEL:
			    res = ERR_NCX_CANCELED;
			    break;
			case YESNO_YES:
			    break;
			case YESNO_NO:
			    done = TRUE;
			    break;
			default:
			    res = SET_ERROR(ERR_INTERNAL_VAL);
			}
		    }
		    break;
		case ERR_NCX_SKIPPED:
		    done = TRUE;
		    res = NO_ERR;
		    break;
		case ERR_NCX_CANCELED:
		    break;
		default:
		    ;
		}
	    }
	    break;
	case OBJ_TYP_CONTAINER:
	case OBJ_TYP_NOTIF:
	case OBJ_TYP_RPCIO:
	    /* if the parm is not already set and is not read-only
	     * then try to get a value from the user at the CLI
	     */
	    val = val_find_child(valset, 
				 obj_get_mod_name(parm),
				 obj_get_name(parm));
	    if (val) {
		break;
	    }
			
	    if (oldvalset) {
		oldval = val_find_child(oldvalset, 
					obj_get_mod_name(parm),
					obj_get_name(parm));
	    } else {
		oldval = NULL;
	    }

	    val = val_new_value();
	    if (!val) {
		res = ERR_INTERNAL_MEM;
		log_error("\nError: malloc of new value failed");
		break;
	    } else {
		val_init_from_template(val, parm);
		val_add_child(val, valset);
	    }

	    /* recurse with the child nodes */
	    res = fill_valset(agent_cb, rpc, val, oldval);

	    switch (res) {
	    case NO_ERR:
		break;
	    case ERR_NCX_SKIPPED:
		res = NO_ERR;
		break;
	    case ERR_NCX_CANCELED:
		break;
	    default:
		;
	    }
	    break;
	case OBJ_TYP_LIST:
	    done = FALSE;
	    while (!done && res == NO_ERR) {
		val = val_new_value();
		if (!val) {
		    res = ERR_INTERNAL_MEM;
		    log_error("\nError: malloc of new value failed");
		    continue;
		} else {
		    val_init_from_template(val, parm);
		    val_add_child(val, valset);
		}

		/* recurse with the child node -- NO OLD VALUE
		 * TBD: get keys, then look up old matching entry
		 */
		res = fill_valset(agent_cb, rpc, val, NULL);

		switch (res) {
		case NO_ERR:
		    /* prompt for more list entries */
		    res = get_yesno(agent_cb, YANGCLI_PR_LIST,
				    YESNO_NO, &yesnocode);
		    if (res == NO_ERR) {
			switch (yesnocode) {
			case YESNO_CANCEL:
			    res = ERR_NCX_CANCELED;
			    break;
			case YESNO_YES:
			    break;
			case YESNO_NO:
			    done = TRUE;
			    break;
			default:
			    res = SET_ERROR(ERR_INTERNAL_VAL);
			}
		    }
		    break;
		case ERR_NCX_SKIPPED:
		    res = NO_ERR;
		    break;
		case ERR_NCX_CANCELED:
		    break;
		default:
		    ;
		}
	    }
            break;
        default: 
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    cli_fn = NULL;
    return res;

} /* fill_valset */


/********************************************************************
 * FUNCTION get_valset
 * 
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the command being processed
 *    line == CLI input in progress
 *    res == address of status result
 *
 * OUTPUTS:
 *    *res is set to the status
 *
 * RETURNS:
 *   malloced valset filled in with the parameters for 
 *   the specified RPC
 *
 *********************************************************************/
static val_value_t *
    get_valset (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		status_t  *res)
{
    const obj_template_t  *obj;
    val_value_t           *valset;
    uint32                 len;

    *res = NO_ERR;
    valset = NULL;
    len = 0;

    /* skip leading whitespace */
    while (line[len] && xml_isspace(line[len])) {
	len++;
    }

    /* check any non-whitespace entered after RPC method name */
    if (line[len]) {
	valset = parse_rpc_cli(rpc, &line[len], res);
	if (*res == ERR_NCX_SKIPPED) {
	    log_stdout("\nError: no parameters defined for RPC %s",
		       obj_get_name(rpc));
	} else if (*res != NO_ERR) {
	    log_stdout("\nError in the parameters for RPC %s (%s)",
		       obj_get_name(rpc), get_error_string(*res));
	}
    }

    obj = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!obj || !obj_get_child_count(obj)) {
	*res = ERR_NCX_SKIPPED;
	if (valset) {
	    val_free_value(valset);
	}
	return NULL;
    }

    /* check no input from user, so start a parmset */
    if (*res == NO_ERR && !valset) {
	valset = val_new_value();
	if (!valset) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    val_init_from_template(valset, obj);
	}
    }

    /* fill in any missing parameters from the CLI */
    if (*res==NO_ERR && interactive_mode()) {
	*res = fill_valset(agent_cb, rpc, valset, NULL);
    }

    return valset;

}  /* get_valset */


/********************************************************************
* FUNCTION create_session
* 
* Start a NETCONF session and change the program state
* Since this is called in sequence with readline, the STDIN IO
* handler may get called if the user enters keyboard text 
*
* The STDIN handler will not do anything with incoming chars
* while state == MGR_IO_ST_CONNECT
* 
* INPUTS:
*   agent_cb == agent control block to use
*
* OUTPUTS:
*   'agent_cb->mysid' is set to the output session ID, if NO_ERR
*   'agent_cb->state' is changed based on the success of the session setup
*
*********************************************************************/
static void
    create_session (agent_cb_t *agent_cb)
{
    const xmlChar *agent, *username, *password;
    val_value_t   *val;
    status_t       res;
    uint16         port;

    if (agent_cb->mysid) {
	if (mgr_ses_get_scb(agent_cb->mysid)) {
	    SET_ERROR(ERR_INTERNAL_INIT_SEQ);
	    return;
	} else {
	    /* session was reset */
	    agent_cb->mysid = 0;
	}
    }

    /* retrieving the parameters should not fail */
    val =  val_find_child(agent_cb->connect_valset, 
			  YANGCLI_MOD, YANGCLI_USER);
    if (val && val->res == NO_ERR) {
	username = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    val = val_find_child(agent_cb->connect_valset,
			 YANGCLI_MOD, YANGCLI_AGENT);
    if (val && val->res == NO_ERR) {
	agent = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    val = val_find_child(agent_cb->connect_valset,
			 YANGCLI_MOD, YANGCLI_PASSWORD);
    if (val && val->res == NO_ERR) {
	password = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    port = 0;
    val = val_find_child(agent_cb->connect_valset,
			 YANGCLI_MOD, YANGCLI_PORT);
    if (val && val->res == NO_ERR) {
	port = VAL_UINT16(val);
    }

    log_info("\nyangcli: Starting NETCONF session for %s on %s",
	     username, agent);

    agent_cb->state = MGR_IO_ST_CONNECT;

    /* this function call will cause us to block while the
     * protocol layer connect messages are processed
     */
    res = mgr_ses_new_session(username, password, 
			      agent, port, 
			      &agent_cb->mysid);
    if (res == NO_ERR) {
	agent_cb->state = MGR_IO_ST_CONN_START;
	log_debug("\nyangcli: Start session %d OK for agent '%s'", 
		  agent_cb->mysid, agent_cb->name);
    } else {
	log_info("\nyangcli: Start session failed for user %s on "
		 "%s (%s)\n", username, agent, get_error_string(res));
	agent_cb->state = MGR_IO_ST_IDLE;
    }
    
} /* create_session */


/********************************************************************
 * FUNCTION do_connect
 * 
 * INPUTS:
 *   agent_cb == agent control block to use
 *   rpc == rpc header for 'connect' command
 *   line == input text from readline call, not modified or freed here
 *   start == byte offset from 'line' where the parse RPC method
 *            left off.  This is eiother empty or contains some 
 *            parameters from the user
 *   cli == TRUE if this is a connect request from the CLI
 *       == FALSE if this is from top-level command input
 *
 * OUTPUTS:
 *   connect_valset parms may be set 
 *   create_session may be called
 *
 *********************************************************************/
static void
    do_connect (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		uint32 start,
		boolean  cli)
{
    const obj_template_t  *obj;
    val_value_t           *valset;
    status_t               res;
    boolean                s1, s2, s3;

    /* retrieve the 'connect' RPC template, if not done already */
    if (!rpc) {
	rpc = ncx_find_object(yangcli_mod, YANGCLI_CONNECT);
	if (!rpc) {
	    log_write("\nError finding the 'connect' RPC method");
	    return;
	}
    }	    

    obj = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_VAL);
	log_write("\nError finding the connect RPC 'input' node");	
	return;
    }

    /* process any parameters entered on the command line */
    valset = NULL;
    if (line) {
	while (line[start] && xml_isspace(line[start])) {
	    start++;
	}
	if (line[start]) {
	    valset = parse_rpc_cli(rpc, &line[start], &res);
	    if (!valset || res != NO_ERR) {
		log_write("\nError in the parameters for RPC %s (%s)",
			  obj_get_name(rpc), get_error_string(res));
		return;
	    }
	}
    }

    /* get an empty parmset and use the old set for defaults 
     * unless this is a cli from the program startup and all
     * of the parameters are entered
     */
    if (!valset) {
	s1 = s2 = s3 = FALSE;
	if (cli) {
	    s1 = val_find_child(connect_valset, YANGCLI_MOD, 
				YANGCLI_AGENT) ? TRUE : FALSE;
	    s2 = val_find_child(connect_valset, YANGCLI_MOD,
				YANGCLI_USER) ? TRUE : FALSE;
	    s3 = val_find_child(connect_valset, YANGCLI_MOD,
				YANGCLI_PASSWORD) ? TRUE : FALSE;
	}
	if (!(s1 && s2 && s3)) {
	    valset = val_new_value();
	    if (!valset) {
		log_write("\nError: malloc failure");
		return;
	    } else {
		val_init_from_template(valset, obj);
	    }
	} /* else use all of connect_valset */
    }

    /* complete the connect valset if needed
     * and transfer it to the agent_cb version
     */
    res = NO_ERR;
    if (valset) {
	/* try to get any missing params in valset */
	if (interactive_mode()) {
	    res = fill_valset(agent_cb, rpc, valset, connect_valset);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
	    }
	}

	/* save the malloced valset */
	if (res == NO_ERR) {
	    if (agent_cb->connect_valset) {
		val_free_value(agent_cb->connect_valset);
	    }
	    agent_cb->connect_valset = valset;
	}
    } else if (!cli) {
	if (interactive_mode()) {
	    if (!agent_cb->connect_valset) {
		agent_cb->connect_valset = val_new_value();
		if (!agent_cb->connect_valset) {
		    log_write("\nError: malloc failure");
		    return;
		} else {
		    val_init_from_template(agent_cb->connect_valset,
					   obj);
		}
	    }
	    (void)fill_valset(agent_cb, rpc,
			      agent_cb->connect_valset, 
			      connect_valset);
	}
    } else {
	if (agent_cb->connect_valset) {
	    val_free_value(agent_cb->connect_valset);
	}
	agent_cb->connect_valset = val_clone(connect_valset);
	if (!agent_cb->connect_valset) {
	    res = ERR_INTERNAL_MEM;
	}
    }

    /* check result so far */
    if (res != NO_ERR) {
	log_write("\nError: Connect failed (%s)", 
		  get_error_string(res));
	agent_cb->state = MGR_IO_ST_IDLE;
    } else {
	/* make sure the 3 required parms are set */
	s1 = val_find_child(agent_cb->connect_valset, YANGCLI_MOD, 
			    YANGCLI_AGENT) ? TRUE : FALSE;
	s2 = val_find_child(agent_cb->connect_valset, YANGCLI_MOD,
			    YANGCLI_USER) ? TRUE : FALSE;
	s3 = val_find_child(agent_cb->connect_valset, YANGCLI_MOD,
			    YANGCLI_PASSWORD) ? TRUE : FALSE;

	/* check if all params present yet */
	if (s1 && s2 && s3) {
	    create_session(agent_cb);
	} else {
	    log_write("\nError: Connect failed due to missing parameters");
	    agent_cb->state = MGR_IO_ST_IDLE;
	}
    }

}  /* do_connect */


/********************************************************************
* FUNCTION send_copy_config_to_agent
* 
* Send a <copy-config> operation to the agent to support
* the save operation
*
* INPUTS:
*    agent_cb == agent control block to use
*
* OUTPUTS:
*    state may be changed or other action taken
*    config_content is consumed -- freed or transfered
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_copy_config_to_agent (agent_cb_t *agent_cb)
{
    const obj_template_t  *rpc, *input, *child;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *parm, *target, *source;
    ses_cb_t              *scb;
    status_t               res;

    req = NULL;
    reqdata = NULL;
    res = NO_ERR;
    rpc = NULL;

    /* get the <copy-config> template */
    rpc = ncx_find_object(netconf_mod, NCX_EL_COPY_CONFIG);
    if (!rpc) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* get the 'input' section container */
    input = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!input) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* construct a method + parameter tree */
    reqdata = xml_val_new_struct(obj_get_name(rpc), 
				 obj_get_nsid(rpc));
    if (!reqdata) {
	log_error("\nError allocating a new RPC request");
	return ERR_INTERNAL_MEM;
    }

    /* set the edit-config/input/target node to the default_target */
    child = obj_find_child(input, NC_MODULE, NCX_EL_TARGET);
    parm = val_new_value();
    if (!parm) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);

    target = xml_val_new_flag((const xmlChar *)"startup",
			      obj_get_nsid(child));
    if (!target) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_add_child(target, parm);

    /* set the edit-config/input/default-operation node to 'none' */
    child = obj_find_child(input, NC_MODULE, NCX_EL_SOURCE);
    parm = val_new_value();
    if (!parm) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);

    source = xml_val_new_flag((const xmlChar *)"running",
			      obj_get_nsid(child));
    if (!source) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_add_child(source, parm);

    /* allocate an RPC request and send it */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    } else {
	req = mgr_rpc_new_request(scb);
	if (!req) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nError allocating a new RPC request");
	} else {
	    req->data = reqdata;
	    req->rpc = rpc;
	    req->timeout = agent_cb->timeout;
	}
    }
	
    if (res == NO_ERR) {
	if (LOGDEBUG2) {
	    log_debug2("\nabout to send RPC request with reqdata:");
	    val_dump_value(reqdata, NCX_DEF_INDENT);
	}

	/* the request will be stored if this returns NO_ERR */
	res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
    }

    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_copy_config_to_agent */


/********************************************************************
 * FUNCTION do_save
 * 
 * INPUTS:
 *    agent_cb == agent control block to use
 *
 * OUTPUTS:
 *   copy-config and/or commit operation will be sent to agent
 *
 *********************************************************************/
static void
    do_save (agent_cb_t *agent_cb)
{
    const ses_cb_t   *scb;
    const mgr_scb_t  *mscb;
    xmlChar          *line;
    status_t          res;

    /* get the session info */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    mscb = (const mgr_scb_t *)scb->mgrcb;

    log_info("\nSaving configuration to non-volative storage");

    /* determine which commands to send */
    switch (mscb->targtyp) {
    case NCX_AGT_TARG_NONE:
	log_stdout("\nWarning: No writable targets supported on this agent");
	break;
    case NCX_AGT_TARG_CANDIDATE:
    case NCX_AGT_TARG_CAND_RUNNING:
	line = xml_strdup(NCX_EL_COMMIT);
	if (line) {
	    conn_command(agent_cb, line);
#ifdef NOT_YET
	    if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
		log_stdout(" + copy-config <running> <startup>");
	    }
#endif
	    m__free(line);
	} else {
	    log_stdout("\nError: Malloc failed");
	}
	break;
    case NCX_AGT_TARG_RUNNING:
	if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
	    res = send_copy_config_to_agent(agent_cb);
	    if (res != NO_ERR) {
		log_stdout("\nError: send copy-config failed (%s)",
			   get_error_string(res));
	    }
	} else {
	    log_stdout("\nWarning: No distinct save operation needed "
		       "for this agent");
	}
	break;
    case NCX_AGT_TARG_LOCAL:
	log_stdout("Error: Local URL target not supported");
	break;
    case NCX_AGT_TARG_REMOTE:
	log_stdout("Error: Local URL target not supported");
	break;
    default:
	log_stdout("Error: Internal target not set");
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* do_save */


/********************************************************************
 * FUNCTION do_mgrload (local RPC)
 * 
 * mgrload module=mod-name
 *
 * Get the module parameter and load the specified NCX module
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   specified module is loaded into the definition registry, if NO_ERR
 *
 *********************************************************************/
static void
    do_mgrload (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		uint32  len)
{
    val_value_t  *valset, *val;
    status_t      res;

    val = NULL;
    res = NO_ERR;

    valset = get_valset(agent_cb, rpc, &line[len], &res);

    /* get the module name */
    if (res == NO_ERR) {
	val = val_find_child(valset, YANGCLI_MOD, NCX_EL_MODULE);
	if (!val) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else if (val->res != NO_ERR) {
	    res = val->res;
	}
    }

    /* load the module */
    if (res == NO_ERR) {
	res = ncxmod_load_module(VAL_STR(val), NULL, NULL);
    }

    /* print the result to stdout */
    if (res == NO_ERR) {
	log_stdout("\nLoad module %s OK", VAL_STR(val));
    } else {
	log_stdout("\nError: Load module failed (%s)",
		   get_error_string(res));
    }

    if (valset) {
	val_free_value(valset);
    }

}  /* do_mgrload */


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

    return NO_ERR;

} /* init_config_vars */



/********************************************************************
 * FUNCTION do_show_vars (sub-mode of local RPC)
 * 
 * show brief info for all user variables
 *
 * INPUTS:
 *  mode == help mode requested
 *  shortmode == TRUE if printing just global or local variables
 *               FALSE to print everything
 *  isglobal == TRUE if print just globals
 *              FALSE to print just locals
 *              Ignored unless shortmode==TRUE
 * isany == TRUE to choose global or local
 *          FALSE to use 'isglobal' valuse only
 *********************************************************************/
static void
    do_show_vars (help_mode_t mode,
		  boolean shortmode,
		  boolean isglobal,
		  boolean isany)

{
    ncx_var_t  *var;
    dlq_hdr_t  *que;
    logfn_t     logfn;
    boolean     first, imode;

    imode = interactive_mode();
    if (imode) {
	logfn = log_stdout;
    } else {
	logfn = log_write;
    }

    if (mode > HELP_MODE_BRIEF && !shortmode) {
	/* CLI Parameters */
	if (mgr_cli_valset && val_child_cnt(mgr_cli_valset)) {
	    (*logfn)("\nCLI Variables\n");
	    if (imode) {
		val_stdout_value(mgr_cli_valset, NCX_DEF_INDENT);
	    } else {
		val_dump_value(mgr_cli_valset, NCX_DEF_INDENT);
	    }
	    (*logfn)("\n");
	} else {
	    (*logfn)("\nNo CLI variables\n");
	}
    }

    /* System Script Variables */
    if (!shortmode) {
	que = runstack_get_que(ISGLOBAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {

	    if (var->vartype != VAR_TYP_SYSTEM) {
		continue;
	    }

	    if (first) {
		(*logfn)("\nRead-only system variables");
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		(*logfn)("\n   %s (%s)", 
			 var->name,
			 tk_get_btype_sym(var->val->btyp));
		if (mode == HELP_MODE_FULL) {
		    if (imode) {
			val_stdout_value(var->val, NCX_DEF_INDENT);
		    } else {
			val_dump_value(var->val, NCX_DEF_INDENT);
		    }
		}
	    }
	}
	if (first) {
	    (*logfn)("\nNo read-only system variables");
	}
	(*logfn)("\n");
    }

    /* System Config Variables */
    if (!shortmode || isglobal) {
	que = runstack_get_que(ISGLOBAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {

	    if (var->vartype != VAR_TYP_CONFIG) {
		continue;
	    }

	    if (first) {
		(*logfn)("\nRead-write system variables");
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		(*logfn)("\n   %s (%s)", 
			 var->name,
			 tk_get_btype_sym(var->val->btyp));
		if (mode == HELP_MODE_FULL) {
		    if (imode) {
			val_stdout_value(var->val, NCX_DEF_INDENT);
		    } else {
			val_dump_value(var->val, NCX_DEF_INDENT);
		    }
		}
	    }
	}
	if (first) {
	    (*logfn)("\nNo system config variables");
	}
	(*logfn)("\n");
    }

    /* Global Script Variables */
    if (!shortmode || isglobal) {
	que = runstack_get_que(ISGLOBAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {

	    if (var->vartype != VAR_TYP_GLOBAL) {
		continue;
	    }

	    if (first) {
		(*logfn)("\nGlobal variables");
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		(*logfn)("\n   %s (%s)", 
			 var->name,
			 tk_get_btype_sym(var->val->btyp));
		if (mode == HELP_MODE_FULL) {
		    if (imode) {
			val_stdout_value(var->val, NCX_DEF_INDENT);
		    } else {
			val_dump_value(var->val, NCX_DEF_INDENT);
		    }
		}
	    }
	}
	if (first) {
	    (*logfn)("\nNo global variables");
	}
	(*logfn)("\n");
    }

    /* Local Script Variables */
    if (!shortmode || !isglobal || isany) {
	que = runstack_get_que(ISLOCAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {
	    if (first) {
		(*logfn)("\nLocal variables");
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		/* just print the data type name for complex types */
		(*logfn)("\n   %s (%s)", 
			 var->name,
			 tk_get_btype_sym(var->val->btyp));
		if (mode == HELP_MODE_FULL) {
		    if (imode) {
			val_stdout_value(var->val, NCX_DEF_INDENT);
		    } else {
			val_dump_value(var->val, NCX_DEF_INDENT);
		    }
		}
	    }
	}
	if (first) {
	    (*logfn)("\nNo local variables");
	}
	(*logfn)("\n");
    }

} /* do_show_vars */


/********************************************************************
 * FUNCTION do_show_var (sub-mode of local RPC)
 * 
 * show full info for one user var
 *
 * INPUTS:
 *   name == variable name to find 
 *   isglobal == TRUE if global var, FALSE if local var
 *   isany == TRUE if don't care (global or local)
 *         == FALSE to force local or global with 'isglobal'
 *   mode == help mode requested
 *********************************************************************/
static void
    do_show_var (const xmlChar *name,
		 boolean isglobal,
		 boolean isany,
		 help_mode_t mode)
{
    const val_value_t *val;
    logfn_t            logfn;
    boolean            imode;

    imode = interactive_mode();
    if (imode) {
	logfn = log_stdout;
    } else {
	logfn = log_write;
    }

    if (isany || isglobal) {
	val = var_get(name, VAR_TYP_GLOBAL);
	if (!val) {
	    val = var_get(name, VAR_TYP_CONFIG);
	}
    } else {
	val = var_get_local(name);
    }

    if (val) {
	if (mode == HELP_MODE_BRIEF) {
	    if (typ_is_simple(val->btyp)) {
		if (imode) {
		    val_stdout_value(val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(val, NCX_DEF_INDENT);
		}
	    } else {
		(*logfn)("\n  %s (complex type)");
	    }
	} else {
	    if (imode) {
		val_stdout_value(val, NCX_DEF_INDENT);
	    } else {
		val_dump_value(val, NCX_DEF_INDENT);
	    }
	}
    } else {
	(*logfn)("\nVariable '%s' not found", name);
    }

} /* do_show_var */


/********************************************************************
 * FUNCTION do_show_module (sub-mode of local RPC)
 * 
 * show module=mod-name
 *
 * INPUTS:
 *    mod == module to show
 *    mode == requested help mode
 * 
 *********************************************************************/
static void
    do_show_module (const ncx_module_t *mod,
		    help_mode_t mode)
{
    help_data_module(mod, mode);

} /* do_show_module */




/********************************************************************
 * FUNCTION do_show_one_module (sub-mode of show modules RPC)
 * 
 * for 1 of N: show modules
 *
 * INPUTS:
 *    mod == module to show
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_show_one_module (ncx_module_t *mod,
			help_mode_t mode)
{
    boolean        anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    if (mode == HELP_MODE_BRIEF) {
	if (imode) {
	    log_stdout("\n  %s", mod->name); 
	} else {
	    log_write("\n  %s", mod->name); 
	}
    } else if (mode == HELP_MODE_NORMAL) {
	if (imode) {
	    if (mod->version) {
		log_stdout("\n  %s:%s/%s", mod->prefix, 
			   mod->name, mod->version);
	    } else {
		log_stdout("\n  %s:%s", mod->prefix, 
			   mod->name);
	    }
	} else {
	    if (mod->version) {
		log_write("\n  %s/%s", mod->name, mod->version);
	    } else {
		log_write("\n  %s", mod->name);
	    }
	}
    } else {
	help_data_module(mod, HELP_MODE_BRIEF);
    }
}  /* do_show_one_module */


/********************************************************************
 * FUNCTION do_show_modules (sub-mode of local RPC)
 * 
 * show modules
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_show_modules (agent_cb_t *agent_cb,
		     help_mode_t mode)
{
    ncx_module_t  *mod;
    modptr_t      *modptr;
    boolean        anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    if (use_agentcb(agent_cb)) {
	for (modptr = (modptr_t *)dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (modptr_t *)dlq_nextEntry(modptr)) {

	    do_show_one_module(modptr->mod, mode);
	    anyout = TRUE;
	}
    } else {
	mod = ncx_get_first_module();
	while (mod) {
	    do_show_one_module(mod, mode);
	    anyout = TRUE;
	    mod = ncx_get_next_module(mod);
	}
    }

    if (anyout) {
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    } else {
	if (imode) {
	    log_stdout("\nyangcli: no modules loaded\n");
	} else {
	    log_error("\nyangcli: no modules loaded\n");
	}
    }

} /* do_show_modules */


/********************************************************************
 * FUNCTION do_show_one_object (sub-mode of show objects local RPC)
 * 
 * show objects: 1 of N
 *
 * INPUTS:
 *    obj == object to show
 *    mode == requested help mode
 *    anyout == address of return anyout status
 *
 * OUTPUTS:
 *    *anyout set to TRUE only if any suitable objects found
 *********************************************************************/
static void
    do_show_one_object (const obj_template_t *obj,
			help_mode_t mode,
			boolean *anyout)
{
    boolean               imode;

    imode = interactive_mode();

    if (obj_is_data_db(obj) && 
	obj_has_name(obj) &&
	!obj_is_hidden(obj) && !obj_is_abstract(obj)) {

	if (mode == HELP_MODE_BRIEF) {
	    if (imode) {
		log_stdout("\n%s:%s",
			   obj_get_mod_name(obj),
			   obj_get_name(obj));
	    } else {
		log_write("\n%s:%s",
			  obj_get_mod_name(obj),
			  obj_get_name(obj));
	    }
	} else {
	    obj_dump_template(obj, mode-1, 0, 0); 
	}
	*anyout = TRUE;
    }

} /* do_show_one_object */


/********************************************************************
 * FUNCTION do_show_objects (sub-mode of local RPC)
 * 
 * show objects
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_show_objects (agent_cb_t *agent_cb,
		     help_mode_t mode)
{
    const ncx_module_t   *mod;
    const obj_template_t *obj;
    const modptr_t       *modptr;
    boolean               anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    if (use_agentcb(agent_cb)) {
	for (modptr = (const modptr_t *)
		 dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    for (obj = ncx_get_first_object(modptr->mod);
		 obj != NULL;
		 obj = ncx_get_next_object(modptr->mod, obj)) {

		do_show_one_object(obj, mode, &anyout);
	    }
	}
    } else {
	mod = ncx_get_first_module();
	while (mod) {
	    for (obj = ncx_get_first_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_object(mod, obj)) {

		do_show_one_object(obj, mode, &anyout);
	    }
	    mod = (const ncx_module_t *)ncx_get_next_module(mod);
	}
    }
    if (anyout) {
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    }

} /* do_show_objects */


/********************************************************************
 * FUNCTION do_show (local RPC)
 * 
 * show module=mod-name
 *      modules
 *      def=def-nmae
 *
 * Get the specified parameter and show the internal info,
 * based on the parameter
 *
 * INPUTS:
 * agent_cb == agent control block to use
 *    rpc == RPC method for the show command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 *********************************************************************/
static void
    do_show (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t        *valset, *parm;
    const ncx_module_t *mod;
    status_t            res;
    boolean             imode, done;
    help_mode_t         mode;

    imode = interactive_mode();
    valset = get_valset(agent_cb, rpc, &line[len], &res);

    if (valset && res == NO_ERR) {
	mode = HELP_MODE_NORMAL;

	/* check if the 'brief' flag is set first */
	parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_BRIEF);
	if (parm && parm->res == NO_ERR) {
	    mode = HELP_MODE_BRIEF;
	} else {
	    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_FULL);
	    if (parm && parm->res == NO_ERR) {
		mode = HELP_MODE_FULL;
	    }
	}
	    
	/* get the 1 of N 'showtype' choice */
	done = FALSE;
	parm = val_find_child(valset, YANGCLI_MOD,
			      YANGCLI_LOCAL);
	if (parm) {
	    do_show_var(VAL_STR(parm), ISLOCAL, FALSE, mode);
	    done = TRUE;
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  YANGCLI_LOCALS);
	    if (parm) {
		do_show_vars(mode, TRUE, FALSE, FALSE);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  YANGCLI_OBJECTS);
	    if (parm) {
		do_show_objects(agent_cb, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  YANGCLI_GLOBAL);
	    if (parm) {
		do_show_var(VAL_STR(parm), ISGLOBAL, FALSE, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  YANGCLI_GLOBALS);
	    if (parm) {
		do_show_vars(mode, TRUE, TRUE, FALSE);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  YANGCLI_VAR);
	    if (parm) {
		do_show_var(VAL_STR(parm), ISLOCAL, TRUE, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  YANGCLI_VARS);
	    if (parm) {
		do_show_vars(mode, FALSE, FALSE, TRUE);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  YANGCLI_MODULE);
	    if (parm) {
		mod = find_module(agent_cb, VAL_STR(parm));
		if (mod) {
		    do_show_module(mod, mode);
		} else {
		    if (imode) {
			log_stdout("\nyangcli: module (%s) not loaded",
				   VAL_STR(parm));
		    } else {
			log_error("\nyangcli: module (%s) not loaded",
				  VAL_STR(parm));
		    }
		}
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  YANGCLI_MODULES);
	    if (parm) {
		do_show_modules(agent_cb, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD,
				  NCX_EL_VERSION);
	    if (parm) {
		if (imode) {
		    log_stdout("\nyangcli version %s\n", progver);
		} else {
		    log_write("\nyangcli version %s\n", progver);
		}
		done = TRUE;
	    }
	}
    }

    if (valset) {
	val_free_value(valset);
    }

}  /* do_show */


/********************************************************************
 * FUNCTION do_list_one_oid (sub-mode of list oids RPC)
 * 
 * list oids: 1 of N
 *
 * INPUTS:
 *    obj == the object to use
 *********************************************************************/
static void
    do_list_one_oid (const obj_template_t *obj)
{
    xmlChar      *buffer;
    boolean       imode;
    status_t      res;

    if (obj_is_data_db(obj) && 
	obj_has_name(obj) &&
	!obj_is_hidden(obj) && 
	!obj_is_abstract(obj)) {

	buffer = NULL;
	res = obj_gen_object_id(obj, &buffer);
	if (res != NO_ERR) {
	    log_error("\nError: list OID failed (%s)",
		      get_error_string(res));
	} else {
	    if (imode) {
		log_stdout("\n   %s", buffer);
	    } else {
		log_write("\n   %s", buffer);
	    }
	}
	if (buffer) {
	    m__free(buffer);
	}
    }

} /* do_list_one_oid */


/********************************************************************
 * FUNCTION do_list_oid (sub-mode of local RPC)
 * 
 * list oids
 *
 * INPUTS:
 *    obj == object to use
 *    nestlevel to stop at
 *********************************************************************/
static void
    do_list_oid (const obj_template_t *obj,
		 uint32 level)
{
    const obj_template_t  *chobj;

    if (obj_get_level(obj) <= level) {
	do_list_one_oid(obj);
	for (chobj = obj_first_child(obj);
	     chobj != NULL;
	     chobj = obj_next_child(chobj)) {
	    do_list_oid(chobj, level);
	}
    }

} /* do_list_oid */


/********************************************************************
 * FUNCTION do_list_oids (sub-mode of local RPC)
 * 
 * list oids
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mod == the 1 module to use
 *           NULL to use all available modules instead
 *    mode == requested help mode
 *********************************************************************/
static void
    do_list_oids (agent_cb_t *agent_cb,
		  const ncx_module_t *mod,
		  help_mode_t mode)
{
    const modptr_t        *modptr;
    const obj_template_t  *obj;
    boolean                imode;
    uint32                 level;


    switch (mode) {
    case HELP_MODE_NONE:
	return;
    case HELP_MODE_BRIEF:
	level = 1;
	break;
    case HELP_MODE_NORMAL:
	level = 5;
	break;
    case HELP_MODE_FULL:
	level = 999;
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    imode = interactive_mode();

    if (mod) {
	obj = ncx_get_first_object(mod);
	while (obj) {
	    do_list_oid(obj, level);
	    obj = ncx_get_next_object(mod, obj);
	}
    } else if (use_agentcb(agent_cb)) {
	for (modptr = (const modptr_t *)
		 dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    obj = ncx_get_first_object(modptr->mod);
	    while (obj) {
		do_list_oid(obj, level);
		obj = ncx_get_next_object(modptr->mod, obj);
	    }
	}
    } else {
	return;
    }
    if (imode) {
	log_stdout("\n");
    } else {
	log_write("\n");
    }


} /* do_list_oids */


/********************************************************************
 * FUNCTION do_list_one_command (sub-mode of list command RPC)
 * 
 * list commands: 1 of N
 *
 * INPUTS:
 *    obj == object template for the RPC command to use
 *    mode == requested help mode
 *********************************************************************/
static void
    do_list_one_command (const obj_template_t *obj,
			 help_mode_t mode)
{
    if (interactive_mode()) {
	if (mode == HELP_MODE_BRIEF) {
	    log_stdout("\n   %s", obj_get_name(obj));
	} else if (mode == HELP_MODE_NORMAL) {
	    log_stdout("\n   %s:%s",
		       obj_get_mod_prefix(obj),
		       obj_get_name(obj));
	} else {
	    log_stdout("\n   %s:%s",
		       obj_get_mod_name(obj),
		       obj_get_name(obj));
	}
    } else {
	if (mode == HELP_MODE_BRIEF) {
	    log_write("\n   %s", obj_get_name(obj));
	} else if (mode == HELP_MODE_NORMAL) {
	    log_write("\n   %s:%s",
		      obj_get_mod_prefix(obj),
		      obj_get_name(obj));
	} else {
	    log_write("\n   %s:%s",
		      obj_get_mod_name(obj),
		      obj_get_name(obj));
	}
    }

} /* do_list_one_command */


/********************************************************************
 * FUNCTION do_list_objects (sub-mode of local RPC)
 * 
 * list objects
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mod == the 1 module to use
 *           NULL to use all available modules instead
 *    mode == requested help mode
 *********************************************************************/
static void
    do_list_objects (agent_cb_t *agent_cb,
		     const ncx_module_t *mod,
		     help_mode_t mode)
{
    const modptr_t        *modptr;
    const obj_template_t  *obj;
    boolean                imode;

    imode = interactive_mode();

    if (mod) {
	obj = ncx_get_first_object(mod);
	while (obj) {
	    if (obj_is_data_db(obj) && 
		obj_has_name(obj) &&
		!obj_is_hidden(obj) && 
		!obj_is_abstract(obj)) {
		do_list_one_command(obj, mode);
	    }
	    obj = ncx_get_next_object(mod, obj);
	}
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    } else if (use_agentcb(agent_cb)) {
	for (modptr = (const modptr_t *)
		 dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    obj = ncx_get_first_object(modptr->mod);
	    while (obj) {
		if (obj_is_data_db(obj) && 
		    obj_has_name(obj) &&
		    !obj_is_hidden(obj) && 
		    !obj_is_abstract(obj)) {

		    do_list_one_command(obj, mode);
		}
		obj = ncx_get_next_object(modptr->mod, obj);
	    }
	}

	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    }

} /* do_list_objects */


/********************************************************************
 * FUNCTION do_list_commands (sub-mode of local RPC)
 * 
 * list commands
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mod == the 1 module to use
 *           NULL to use all available modules instead
 *    mode == requested help mode
 *********************************************************************/
static void
    do_list_commands (agent_cb_t *agent_cb,
		      const ncx_module_t *mod,
		      help_mode_t mode)
{
    const modptr_t        *modptr;
    const obj_template_t  *obj;
    boolean                imode;

    imode = interactive_mode();

    if (mod) {
	obj = ncx_get_first_object(mod);
	while (obj) {
	    if (obj_is_rpc(obj)) {
		do_list_one_command(obj, mode);
	    }
	    obj = ncx_get_next_object(mod, obj);
	}
    } else if (use_agentcb(agent_cb)) {
	if (imode) {
	    log_stdout("\nAgent Commands:");
	} else {
	    log_write("\nAgent Commands:");
	}
	
	for (modptr = (const modptr_t *)
		 dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    obj = ncx_get_first_object(modptr->mod);
	    while (obj) {
		if (obj_is_rpc(obj)) {
		    do_list_one_command(obj, mode);
		}
		obj = ncx_get_next_object(modptr->mod, obj);
	    }
	}
    }

    if (imode) {
	log_stdout("\n\nLocal Commands:");
    } else {
	log_write("\n\nLocal Commands:");
    }

    obj = ncx_get_first_object(yangcli_mod);
    while (obj) {
	if (obj_is_rpc(obj)) {
	    if (use_agentcb(agent_cb)) {
		/* list all local commands */
		do_list_one_command(obj, mode);
	    } else {
		/* session not active so filter out
		 * all the commands except top command
		 */
		if (is_top_command(obj_get_name(obj))) {
		    do_list_one_command(obj, mode);
		}
	    }
	}
	obj = ncx_get_next_object(yangcli_mod, obj);
    }

    if (imode) {
	log_stdout("\n");
    } else {
	log_write("\n");
    }

} /* do_list_commands */


/********************************************************************
 * FUNCTION do_list (local RPC)
 * 
 * list objects   [module=mod-name]
 *      ids
 *      commands
 *
 * List the specified information based on the parameters
 *
 * INPUTS:
 * agent_cb == agent control block to use
 *    rpc == RPC method for the show command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 *********************************************************************/
static void
    do_list (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t        *valset, *parm;
    const ncx_module_t *mod;
    status_t            res;
    boolean             imode, done;
    help_mode_t         mode;

    mod = NULL;
    done = FALSE;

    imode = interactive_mode();

    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (valset && res == NO_ERR) {
	mode = HELP_MODE_NORMAL;

	/* check if the 'brief' flag is set first */
	parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_BRIEF);
	if (parm && parm->res == NO_ERR) {
	    mode = HELP_MODE_BRIEF;
	} else {
	    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_FULL);
	    if (parm && parm->res == NO_ERR) {
		mode = HELP_MODE_FULL;
	    }
	}

	parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_MODULE);
	if (parm && parm->res == NO_ERR) {
	    mod = find_module(agent_cb, VAL_STR(parm));
	    if (!mod) {
		if (imode) {
		    log_stdout("\nError: no module found named '%s'",
			       VAL_STR(parm)); 
		} else {
		    log_write("\nError: no module found named '%s'",
			      VAL_STR(parm)); 
		}
		done = TRUE;
	    }
	}

	/* find the 1 of N choice */
	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD, 
				  YANGCLI_COMMANDS);
	    if (parm) {
		/* do list commands */
		do_list_commands(agent_cb, mod, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD, 
				  YANGCLI_OBJECTS);
	    if (parm) {
		/* do list objects */
		do_list_objects(agent_cb, mod, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, YANGCLI_MOD, 
				  YANGCLI_OIDS);
	    if (parm) {
		/* do list oids */
		do_list_oids(agent_cb, mod, mode);
		done = TRUE;
	    }
	}
    }

    if (valset) {
	val_free_value(valset);
    }

}  /* do_list */


/********************************************************************
 * FUNCTION do_help_commands (sub-mode of local RPC)
 * 
 * help commands
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_help_commands (agent_cb_t *agent_cb,
		      help_mode_t mode)
{
    const modptr_t        *modptr;
    const obj_template_t  *obj;
    boolean                anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    if (use_agentcb(agent_cb)) {
	if (imode) {
	    log_stdout("\nAgent Commands:\n");
	} else {
	    log_write("\nAgent Commands:\n");
	}

	for (modptr = (const modptr_t *)
		 dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    obj = ncx_get_first_object(modptr->mod);
	    while (obj) {
		if (obj_is_rpc(obj)) {
		    if (mode == HELP_MODE_BRIEF) {
			obj_dump_template(obj, mode, 1, 0);
		    } else {
			obj_dump_template(obj, mode, 0, 0);
		    }
		    anyout = TRUE;
		}
		obj = ncx_get_next_object(modptr->mod, obj);
	    }
	}
    }

    if (imode) {
	log_stdout("\nLocal Commands:\n");
    } else {
	log_write("\nLocal Commands:\n");
    }

    obj = ncx_get_first_object(yangcli_mod);
    while (obj) {
	if (obj_is_rpc(obj)) {
	    if (mode == HELP_MODE_BRIEF) {
		obj_dump_template(obj, mode, 1, 0);
	    } else {
		obj_dump_template(obj, mode, 0, 0);
	    }
	    anyout = TRUE;
	}
	obj = ncx_get_next_object(yangcli_mod, obj);
    }

    if (anyout) {
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    }

} /* do_help_commands */


/********************************************************************
 * FUNCTION do_help
 * 
 * Print the general yangcli help text to STDOUT
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    do_help (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    const typ_template_t *typ;
    const obj_template_t *obj;
    val_value_t          *valset, *parm;
    status_t              res;
    help_mode_t           mode;
    boolean               imode, done;
    ncx_node_t            dtyp;
    uint32                dlen;

    imode = interactive_mode();
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    done = FALSE;
    mode = HELP_MODE_NORMAL;

    /* look for the 'brief' parameter */
    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_BRIEF);
    if (parm && parm->res == NO_ERR) {
	mode = HELP_MODE_BRIEF;
    } else {
	/* look for the 'full' parameter */
	parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_FULL);
	if (parm && parm->res == NO_ERR) {
	    mode = HELP_MODE_FULL;
	}
    }

    parm = val_find_child(valset, YANGCLI_MOD, 
			  YANGCLI_COMMAND);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(agent_cb, &dtyp, VAL_STR(parm), &dlen);
	if (obj && obj->objtype == OBJ_TYP_RPC) {
	    help_object(obj, mode);
	} else {
	    if (imode) {
		log_stdout("\nyangcli: command (%s) not found",
			   VAL_STR(parm));
	    } else {
		log_error("\nyangcli: command (%s) not found",
			  VAL_STR(parm));
	    }
	}
	val_free_value(valset);
	return;
    }

    /* look for the specific definition parameters */
    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_COMMANDS);
    if (parm && parm->res==NO_ERR) {
	do_help_commands(agent_cb, mode);
	val_free_value(valset);
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_TYPE);
    if (parm && parm->res==NO_ERR) {
	dtyp = NCX_NT_TYP;
	typ = parse_def(agent_cb, &dtyp, VAL_STR(parm), &dlen);
	if (typ) {
	    help_type(typ, mode);
	} else {
	    if (imode) {
		log_stdout("\nyangcli: type definition (%s) not found",
			   VAL_STR(parm));
	    } else {
		log_error("\nyangcli: type definition (%s) not found",
			  VAL_STR(parm));
	    }
	}
	val_free_value(valset);
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_OBJECT);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(agent_cb, &dtyp, VAL_STR(parm), &dlen);
	if (obj && obj_is_data(obj)) {
	    help_object(obj, mode);
	} else {
	    if (imode) {
		log_stdout("\nyangcli: object definition (%s) not found",
			   VAL_STR(parm));
	    } else {
		log_error("\nyangcli: object definition (%s) not found",
			  VAL_STR(parm));
	    }
	}
	val_free_value(valset);
	return;
    }


    parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_NOTIF);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(agent_cb, &dtyp, VAL_STR(parm), &dlen);
	if (obj && obj->objtype == OBJ_TYP_NOTIF) {
	    help_object(obj, mode);
	} else {
	    if (imode) {
		log_stdout("\nyangcli: notification definition (%s) not found",
			   VAL_STR(parm));
	    } else {
		log_error("\nyangcli: notification definition (%s) not found",
			  VAL_STR(parm));
	    }
	}
	val_free_value(valset);
	return;
    }


    /* no parameters entered except maybe brief or full */
    switch (mode) {
    case HELP_MODE_BRIEF:
    case HELP_MODE_NORMAL:
	log_stdout("\n\nyangcli summary:");
	log_stdout("\n\n  Commands are defined with YANG rpc statements.");
	log_stdout("\n  Use 'help commands' to see current list of commands.");
	log_stdout("\n\n  Global variables are created with 2 dollar signs"
		   "\n  in assignment statements ($$foo = 7).");
	log_stdout("\n  Use 'show globals' to see current list "
		   "of global variables.");
	log_stdout("\n\n  Local variables (within a stack frame) are created"
		   "\n  with 1 dollar sign in assignment"
		   " statements ($foo = $bar).");
	log_stdout("\n  Use 'show locals' to see current list "
		   "of local variables.");
	log_stdout("\n\n  Use 'show vars' to see all program variables.\n");

	if (mode==HELP_MODE_BRIEF) {
	    break;
	}

	obj = ncx_find_object(yangcli_mod, YANGCLI_HELP);
	if (obj && obj->objtype == OBJ_TYP_RPC) {
	    help_object(obj, HELP_MODE_FULL);
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
	break;
    case HELP_MODE_FULL:
	help_program_module(YANGCLI_MOD, YANGCLI_BOOT, 
			    HELP_MODE_FULL);
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    val_free_value(valset);

}  /* do_help */


/********************************************************************
 * FUNCTION do_start_script (sub-mode of run local RPC)
 * 
 * run script=script-filespec
 *
 * run the specified script
 *
 * INPUTS:
 *   agent_cb == agent control block to use
 *   source == file source
 *   valset == value set for the run script parameters
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    do_start_script (agent_cb_t *agent_cb,
		     const xmlChar *source,
		     val_value_t *valset)
{
    xmlChar       *str, *fspec;
    FILE          *fp;
    val_value_t   *parm;
    status_t       res;
    int            num;
    xmlChar        buff[4];

    /* saerch for the script */
    fspec = ncxmod_find_script_file(source, &res);
    if (!fspec) {
	return res;
    }

    /* open a new runstack frame if the file exists */
    fp = fopen((const char *)fspec, "r");
    if (!fp) {
	m__free(fspec);
	return ERR_NCX_MISSING_FILE;
    } else {
	res = runstack_push(fspec, fp);
	m__free(fspec);
	if (res != NO_ERR) {
	    fclose(fp);
	    return res;
	}
    }
    
    /* setup the numeric script parameters */
    memset(buff, 0x0, 4);
    buff[0] = 'P';

    /* add the P1 through P9 parameters that are present */
    for (num=1; num<=YANGCLI_MAX_RUNPARMS; num++) {
	buff[1] = '0' + num;
	parm = (valset) ? val_find_child(valset, YANGCLI_MOD, buff) : NULL;
	if (parm) {
	    /* store P7 named as ASCII 7 */
	    res = var_set_str(buff+1, 1, parm, VAR_TYP_LOCAL);
	    if (res != NO_ERR) {
		runstack_pop();
		return res;
	    }
	}
    }

    /* execute the first command in the script */
    str = runstack_get_cmd(&res);
    if (str && res == NO_ERR) {
	/* execute the line as an RPC command */
	if (is_top(agent_cb->state)) {
	    top_command(agent_cb, str);
	} else {
	    conn_command(agent_cb, str);
	}
    }

    return res;

}  /* do_start_script */


/********************************************************************
 * FUNCTION do_run (local RPC)
 * 
 * run script=script-filespec
 *
 *
 * Get the specified parameter and run the specified script
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the show command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    do_run (agent_cb_t *agent_cb,
	    const obj_template_t *rpc,
	    const xmlChar *line,
	    uint32  len)
{
    val_value_t  *valset, *parm;
    status_t      res;

    res = NO_ERR;

    /* get the 'script' parameter */
    valset = get_valset(agent_cb, rpc, &line[len], &res);

    if (valset && res == NO_ERR) {
	/* there is 1 parm */
	parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_SCRIPT);
	if (!parm) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else if (parm->res != NO_ERR) {
	    res = parm->res;
	} else {
	    /* the parm val is the script filespec */
	    res = do_start_script(agent_cb, VAL_STR(parm), valset);
	    if (res != NO_ERR) {
		log_write("\nError: start script %s failed (%s)",
			  obj_get_name(rpc),
			  get_error_string(res));
	    }
	}
    }

    if (valset) {
	val_free_value(valset);
    }

    return res;

}  /* do_run */


/********************************************************************
 * FUNCTION do_startup_script
 * 
 * Process run-script CLI parameter
 *
 * INPUTS:
 *   agent_cb == agent control block to use
 *
 * SIDE EFFECTS:
 *   runstack start with the runscript script if no errors
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    do_startup_script (agent_cb_t *agent_cb)
{
    const obj_template_t *rpc;
    xmlChar              *line, *p;
    status_t              res;
    uint32                linelen;

    /* make sure there is a runscript string */
    if (!runscript || !*runscript) {
	return ERR_NCX_INVALID_VALUE;
    }

    /* get the 'run' RPC method template */
    rpc = ncx_find_object(yangcli_mod, YANGCLI_RUN);
    if (!rpc) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* create a dummy command line 'script <runscipt-text>' */
    linelen = xml_strlen(runscript) + xml_strlen(NCX_EL_SCRIPT) + 1;
    line = m__getMem(linelen+1);
    if (!line) {
	return ERR_INTERNAL_MEM;
    }
    p = line;
    p += xml_strcpy(p, NCX_EL_SCRIPT);
    *p++ = ' ';
    xml_strcpy(p, runscript);
    
    /* fill in the value set for the input parameters */
    res = do_run(agent_cb, rpc, line, 0);
    return res;

}  /* do_startup_script */


/********************************************************************
 * FUNCTION pwd
 * 
 * Print the current working directory
 *
 * INPUTS:
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    pwd (void)
{
    char             *buff;
    boolean           imode;

    imode = interactive_mode();

    buff = m__getMem(YANGCLI_BUFFLEN);
    if (!buff) {
	if (imode) {
	    log_stdout("\nMalloc failure\n");
	} else {
	    log_write("\nMalloc failure\n");
	}
	return;
    }

    if (!getcwd(buff, YANGCLI_BUFFLEN)) {
	if (imode) {
	    log_stdout("\nGet CWD failure\n");
	} else {
	    log_write("\nGet CWD failure\n");
	}
	m__free(buff);
	return;
    }

    if (imode) {
	log_stdout("\nCurrent working directory is %s\n", buff);
    } else {
	log_write("\nCurrent working directory is %s\n", buff);
    }

    m__free(buff);

}  /* pwd */


/********************************************************************
 * FUNCTION do_pwd
 * 
 * Print the current working directory
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    do_pwd (agent_cb_t *agent_cb,
	    const obj_template_t *rpc,
	    const xmlChar *line,
	    uint32  len)
{
    val_value_t      *valset;
    status_t          res;
    boolean           imode;

    imode = interactive_mode();
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (res == NO_ERR || res == ERR_NCX_SKIPPED) {
	pwd();
    }
    if (valset) {
	val_free_value(valset);
    }

}  /* do_pwd */


/********************************************************************
 * FUNCTION do_cd
 * 
 * Change the current working directory
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    do_cd (agent_cb_t *agent_cb,
	   const obj_template_t *rpc,
	   const xmlChar *line,
	   uint32  len)
{
    val_value_t      *valset, *parm;
    status_t          res;
    int               ret;
    boolean           imode;

    imode = interactive_mode();
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_DIR);
    if (!parm || parm->res != NO_ERR) {
	val_free_value(valset);
	return;
    }

    ret = chdir((const char *)VAL_STR(parm));
    if (ret) {
	if (imode) {
	    log_stdout("\nChange CWD failure (%s)\n",
		       get_error_string(errno_to_status()));
	} else {
	    log_write("\nChange CWD failure (%s)\n",
		      get_error_string(errno_to_status()));
	}
    } else {
	pwd();
    }

    val_free_value(valset);

}  /* do_cd */


/********************************************************************
 * FUNCTION do_fill
 * 
 * Fill an object for use in a PDU
 *
 * INPUTS:
 *    agent_cb == agent control block to use (NULL if none)
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   is usually part of an assignment statement
 *
 *********************************************************************/
static void
    do_fill (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t           *valset, *parm, *newparm, *curparm;
    obj_template_t        *targobj;
    const xmlChar         *target;
    status_t               res;
    boolean                imode, save_getopt;

    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, 
			  NCX_EL_TARGET);
    if (!parm || parm->res != NO_ERR) {
	val_free_value(valset);
	return;
    } else {
	target = VAL_STR(parm);
    }

    save_getopt = agent_cb->get_optional;
    imode = interactive_mode();
    newparm = NULL;
    curparm = NULL;

    res = xpath_find_schema_target_int(target, &targobj);
    if (res != NO_ERR) {
	log_error("\nError: Object '%s' not found", target);
	val_free_value(valset);
	return;
    }	

    parm = val_find_child(valset, YANGCLI_MOD, 
			  YANGCLI_CURRENT_VALUE);
    if (parm && parm->res == NO_ERR) {
	curparm = var_get_script_val(targobj, NULL, 
				     VAL_STR(parm),
				     ISPARM, &res);
	if (!curparm || res != NO_ERR) {
	    log_error("\nError: Script value '%s' invalid (%s)", 
		      VAL_STR(parm), get_error_string(res)); 
	    val_free_value(valset);
	    return;
	}
    }

    parm = val_find_child(valset, YANGCLI_MOD, 
			  YANGCLI_OPTIONAL);
    if (parm && parm->res == NO_ERR) {
	agent_cb->get_optional = TRUE;
    }

    switch (targobj->objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	newparm = fill_value(agent_cb, rpc, targobj, curparm, &res);
	break;
    case OBJ_TYP_CHOICE:
	newparm = val_new_value();
	if (!newparm) {
	    log_error("\nError: malloc failure");
	    res = ERR_INTERNAL_MEM;
	} else {
	    val_init_from_template(newparm, targobj);
	    
	    res = get_choice(agent_cb, rpc, targobj, newparm, curparm);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
	    }
	}
	break;
    case OBJ_TYP_CASE:
	newparm = val_new_value();
	if (!newparm) {
	    log_error("\nError: malloc failure");
	    res = ERR_INTERNAL_MEM;
	} else {
	    val_init_from_template(newparm, targobj);

	    res = get_case(agent_cb, rpc, targobj, newparm, curparm);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
	    }
	}
	break;
    default:
	newparm = val_new_value();
	if (!newparm) {
	    log_error("\nError: malloc failure");
	    res = ERR_INTERNAL_MEM;
	} else {
	    val_init_from_template(newparm, targobj);
	    res = fill_valset(agent_cb, rpc, newparm, curparm);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
	    }
	}
    }

    if (res == NO_ERR) {
	if (agent_cb->result_name || agent_cb->result_filename) {
	    /* save the filled in value */
	    res = finish_result_assign(agent_cb, newparm, NULL);
	    newparm = NULL;
	}
    } else {
	clear_result(agent_cb);
    }

    /* cleanup */
    val_free_value(valset);
    if (newparm) {
	val_free_value(newparm);
    }
    if (curparm) {
	val_free_value(curparm);
    }
    agent_cb->get_optional = save_getopt;

}  /* do_fill */


/********************************************************************
* FUNCTION add_content
* 
* Add the config nodes to the parent
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method in progress
*   config_content == the node associated with the target
*             to be used as content nested within the 
*             <config> element
*   curobj == the current object node for config_content, going
*                 up the chain to the root object.
*                 First call should pass config_content->obj
*   dofill == TRUE if interactive mode and mandatory parms
*             need to be specified (they can still be skipped)
*             FALSE if no checking for mandatory parms
*             Just fill out the minimum path to root from
*             the 'config_content' node
*   curtop == address of steady-storage node to add the
*             next new level
*
* OUTPUTS:
*    config node is filled in with child nodes
*    curtop is set to the latest top value
*       It is not needed after the last call and should be ignored
*
* RETURNS:
*    status; config_content is NOT freed if returning an error
*********************************************************************/
static status_t
    add_content (agent_cb_t *agent_cb,
		 const obj_template_t *rpc,
		 val_value_t *config_content,
		 const obj_template_t *curobj,
		 boolean dofill,
		 val_value_t **curtop)
{

    const obj_key_t       *curkey;
    val_value_t           *newnode, *keyval, *lastkey;
    status_t               res;
    boolean                done, content_used;

    res = NO_ERR;

    /* add content based on the current node type */
    switch (curobj->objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	if (curobj != config_content->obj) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	if (!obj_is_key(config_content->obj)) {
	    val_add_child(config_content, *curtop);
	    *curtop = config_content;
	}
	break;
    case OBJ_TYP_LIST:
	/* get all the key nodes for the current object,
	 * if they do not already exist
	 */
	if (curobj == config_content->obj) {
	    val_add_child(config_content, *curtop);
	    *curtop = config_content;
	} else {
	    newnode = val_new_value();
	    if (!newnode) {
		return ERR_INTERNAL_MEM;
	    }
	    val_init_from_template(newnode, curobj);
	    val_add_child(newnode, *curtop);
	    *curtop = newnode;
	}

	content_used = FALSE;
	lastkey = NULL;
	for (curkey = obj_first_ckey(curobj);
	     curkey != NULL;
	     curkey = obj_next_ckey(curkey)) {

	    keyval = val_find_child(*curtop,
				    obj_get_mod_name(curkey->keyobj),
				    obj_get_name(curkey->keyobj));
	    if (!keyval) {
		if (curkey->keyobj == config_content->obj) {
		    keyval = config_content;
		    val_insert_child(keyval, lastkey, *curtop);
		    content_used = TRUE;
		    lastkey = keyval;
		    res = NO_ERR;
		} else if (dofill) {
		    res = get_parm(agent_cb, rpc, 
				   curkey->keyobj, *curtop, NULL);
		    if (res != NO_ERR && res != ERR_NCX_SKIPPED) {
			return res;
		    }
		    if (res == NO_ERR) {
			keyval = val_find_child(*curtop,
						obj_get_mod_name
						(curkey->keyobj),
						obj_get_name
						(curkey->keyobj));
			if (!keyval) {
			    return SET_ERROR(ERR_INTERNAL_VAL);
			}
			val_remove_child(keyval);
			val_insert_child(keyval, lastkey, *curtop);
			lastkey = keyval;
		    } /* else skip this key (for debugging agent) */
		}  /* else --nofill; skip this node */
	    }
	}

	/* wait until all the key leafs are accounted for before
	 * changing the *curtop pointer
	 */
	if (content_used) {
	    *curtop = config_content;
	}
	break;
    case OBJ_TYP_CONTAINER:
	if (curobj == config_content->obj) {
	    val_add_child(config_content, *curtop);
	    *curtop = config_content;
	} else {
	    newnode = val_new_value();
	    if (!newnode) {
		return ERR_INTERNAL_MEM;
	    }
	    val_init_from_template(newnode, curobj);
	    val_add_child(newnode, *curtop);
	    *curtop = newnode;
	}
	break;
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
	if (curobj != config_content->obj) {
	    /* nothing to do for the choice level if the target is a case */
	    res = NO_ERR;
	    break;
	}
	done = FALSE;
	while (!done) {
	    newnode = val_get_first_child(config_content);
	    if (newnode) {
		val_remove_child(newnode);
		res = add_content(agent_cb, 
				  rpc, 
				  newnode, 
				  newnode->obj, 
				  dofill,
				  curtop);
		if (res != NO_ERR) {
		    val_free_value(newnode);
		    done = TRUE;
		}
	    } else {
		done = TRUE;
	    }
	}
	if (res == NO_ERR) {
	    val_free_value(config_content);
	}
	*curtop = newnode;
	break;
    default:
	/* any other object type is an error */
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* add_content */


/********************************************************************
* FUNCTION add_config_from_content_node
* 
* Add the config node content for the edit-config operation
* Build the <config> nodfe top-down, by recursing bottom-up
* from the node to be edited.
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method in progress
*   config_content == the node associated with the target
*             to be used as content nested within the 
*             <config> element
*   curobj == the current object node for config_content, going
*                 up the chain to the root object.
*                 First call should pass config_content->obj
*   config == the starting <config> node to add the data into
*   curtop == address of stable storage for current add-to node
*            This pointer MUST be set to NULL upon first fn call
* OUTPUTS:
*    config node is filled in with child nodes
*
* RETURNS:
*    status; config_content is NOT freed if returning an error
*********************************************************************/
static status_t
    add_config_from_content_node (agent_cb_t *agent_cb,
				  const obj_template_t *rpc,
				  val_value_t *config_content,
				  const obj_template_t *curobj,
				  val_value_t *config,
				  val_value_t **curtop)
{
    const obj_template_t  *parent;
    status_t               res;

    /* get to the root of the object chain */
    parent = obj_get_cparent(curobj);
    if (parent && !obj_is_root(parent)) {
	res = add_config_from_content_node(agent_cb,
					   rpc,
					   config_content,
					   parent,
					   config,
					   curtop);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* set the current target, working down the stack
     * on the way back from the initial dive
     */
    if (!*curtop) {
	/* first time through to this point */
	*curtop = config;
    }

    res = add_content(agent_cb, 
		      rpc, 
		      config_content, 
		      curobj, 
		      TRUE,
		      curtop);

    return res;

}  /* add_config_from_content_node */


/********************************************************************
* FUNCTION add_filter_from_content_node
* 
* Add the filter node content for the get or get-config operation
* Build the <filter> node top-down, by recursing bottom-up
* from the node to be edited.
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method in progress
*   get_content == the node associated with the target
*             to be used as content nested within the 
*             <filter> element
*   curobj == the current object node for get_content, going
*                 up the chain to the root object.
*                 First call should pass get_content->obj
*   filter == the starting <filter> node to add the data into
*   dofill == TRUE for fill mode
*             FALSE to skip filling any nodes
*   curtop == address of stable storage for current add-to node
*            This pointer MUST be set to NULL upon first fn call
* OUTPUTS:
*    filter node is filled in with child nodes
*
* RETURNS:
*    status; get_content is NOT freed if returning an error
*********************************************************************/
static status_t
    add_filter_from_content_node (agent_cb_t *agent_cb,
				  const obj_template_t *rpc,
				  val_value_t *get_content,
				  const obj_template_t *curobj,
				  val_value_t *filter,
				  boolean dofill,
				  val_value_t **curtop)
{
    const obj_template_t  *parent;
    status_t               res;

    /* get to the root of the object chain */
    parent = obj_get_cparent(curobj);
    if (parent && !obj_is_root(parent)) {
	res = add_filter_from_content_node(agent_cb,
					   rpc,
					   get_content,
					   parent,
					   filter,
					   dofill,
					   curtop);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* set the current target, working down the stack
     * on the way back from the initial dive
     */
    if (!*curtop) {
	/* first time through to this point */
	*curtop = filter;
    }

    res = add_content(agent_cb, 
		      rpc, 
		      get_content, 
		      curobj, 
		      dofill,
		      curtop);
    return res;

}  /* add_filter_from_content_node */


/********************************************************************
* FUNCTION send_edit_config_to_agent
* 
* Send an <edit-config> operation to the agent
*
* Fills out the <config> node based on the config_target node
* Any missing key nodes will be collected (via CLI prompt)
* along the way.
*
* INPUTS:
*   agent_cb == agent control block to use
*   config_content == the node associated with the target
*             to be used as content nested within the 
*             <config> element
*   timeoutval == timeout value to use
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*
*    !!! config_content is consumed -- freed or transfered to a PDU
*    !!! that will be freed later
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_edit_config_to_agent (agent_cb_t *agent_cb,
			       val_value_t *config_content,
			       uint32 timeoutval)
{
    const obj_template_t  *rpc, *input, *child;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *parm, *target, *dummy_parm;
    ses_cb_t              *scb;
    status_t               res;

    req = NULL;
    reqdata = NULL;
    res = NO_ERR;

    if (!agent_cb->default_target) {
	log_error("\nError: no <edit-config> target available on agent");
	val_free_value(config_content);
	return ERR_NCX_OPERATION_FAILED;
    }

    /* get the <edit-config> template */
    rpc = ncx_find_object(netconf_mod, NCX_EL_EDIT_CONFIG);
    if (!rpc) {
	val_free_value(config_content);
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* get the 'input' section container */
    input = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!input) {
	val_free_value(config_content);
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* construct a method + parameter tree */
    reqdata = xml_val_new_struct(obj_get_name(rpc), 
				 obj_get_nsid(rpc));
    if (!reqdata) {
	val_free_value(config_content);
	log_error("\nError allocating a new RPC request");
	return ERR_INTERNAL_MEM;
    }

    /* set the edit-config/input/target node to the default_target */
    child = obj_find_child(input, NC_MODULE,
			   NCX_EL_TARGET);
    parm = val_new_value();
    if (!parm) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);

    target = xml_val_new_flag(agent_cb->default_target,
			      obj_get_nsid(child));
    if (!target) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }

    val_add_child(target, parm);

    /* set the edit-config/input/default-operation node to 'none' */
    child = obj_find_child(input, NC_MODULE,
			   NCX_EL_DEFAULT_OPERATION);
    parm = val_new_value();
    if (!parm) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);
    res = val_set_simval(parm,
			 obj_get_ctypdef(child),
			 obj_get_nsid(child),
			 obj_get_name(child),
			 NCX_EL_NONE);

    if (res != NO_ERR) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return res;
    }

    if (agent_cb->testoption != OP_TESTOP_NONE) {
	/* set the edit-config/input/test-option node to
	 * the user-specified value
	 */
	child = obj_find_child(input, NC_MODULE,
			       NCX_EL_TEST_OPTION);
	parm = val_new_value();
	if (!parm) {
	    val_free_value(config_content);
	    val_free_value(reqdata);
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(parm, child);
	val_add_child(parm, reqdata);
	res = val_set_simval(parm,
			     obj_get_ctypdef(child),
			     obj_get_nsid(child),
			     obj_get_name(child),
			     op_testop_name(agent_cb->testoption));
	if (res != NO_ERR) {
	    val_free_value(config_content);
	    val_free_value(reqdata);
	    return res;
	}
    }

    if (agent_cb->erroption != OP_ERROP_NONE) {
	/* set the edit-config/input/error-option node to
	 * the user-specified value
	 */
	child = obj_find_child(input, NC_MODULE,
			       NCX_EL_ERROR_OPTION);
	parm = val_new_value();
	if (!parm) {
	    val_free_value(config_content);
	    val_free_value(reqdata);
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(parm, child);
	val_add_child(parm, reqdata);
	res = val_set_simval(parm,
			     obj_get_ctypdef(child),
			     obj_get_nsid(child),
			     obj_get_name(child),
			     op_errop_name(agent_cb->erroption));
	if (res != NO_ERR) {
	    val_free_value(config_content);
	    val_free_value(reqdata);
	    return res;
	}
    }

    /* set the edit-config/input/config node to the
     * config_content, but after filling in any
     * missing nodes from the root to the target
     */
    child = obj_find_child(input, NC_MODULE,
			   NCX_EL_CONFIG);
    parm = val_new_value();
    if (!parm) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);

    dummy_parm = NULL;
    res = add_config_from_content_node(agent_cb,
				       rpc, config_content,
				       config_content->obj,
				       parm, &dummy_parm);
    if (res != NO_ERR) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return res;
    }

    if (agent_cb->fixorder) {
	/* must set the order of a root container seperately */
	val_set_canonical_order(parm);
    }

    /* !!! config_content consumed at this point !!!
     * allocate an RPC request and send it 
     */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    } else {
	req = mgr_rpc_new_request(scb);
	if (!req) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nError allocating a new RPC request");
	} else {
	    req->data = reqdata;
	    req->rpc = rpc;
	    req->timeout = timeoutval;
	}
    }
	
    if (res == NO_ERR) {
	if (LOGDEBUG2) {
	    log_debug2("\nabout to send RPC request with reqdata:");
	    val_dump_value(reqdata, NCX_DEF_INDENT);
	}

	/* the request will be stored if this returns NO_ERR */
	res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
    }

    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_edit_config_to_agent */


/********************************************************************
 * FUNCTION add_filter_attrs
 * 
 * Add the type and possibly the select 
 * attribute to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    selectstr == select value string to use, (type=xpath)
 *               == NULL for type = subtree
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_filter_attrs (val_value_t *val,
		      const xmlChar *selectstr)
{
    val_value_t          *metaval;

    /* create a value node for the meta-value */
    metaval = val_make_string(0, NCX_EL_TYPE,
			      (selectstr) 
			      ?  NCX_EL_XPATH : NCX_EL_SUBTREE);
    if (!metaval) {
	return ERR_INTERNAL_MEM;
    }
    dlq_enque(metaval, &val->metaQ);

    if (selectstr) {
	metaval = val_make_string(0, NCX_EL_SELECT,  selectstr);
	if (!metaval) {
	    return ERR_INTERNAL_MEM;
	}
	dlq_enque(metaval, &val->metaQ);
    }

    return NO_ERR;

} /* add_filter_attrs */


/********************************************************************
* FUNCTION send_get_to_agent
* 
* Send an <get> operation to the specified agent
*
* Fills out the <filter> node based on the config_target node
* Any missing key nodes will be collected (via CLI prompt)
* along the way.
*
* INPUTS:
*   agent_cb == agent control block to use
*   get_content == the node associated with the target
*             to be used as content nested within the 
*             <filter> element
*             == NULL to use selectstr instead
*   selectstr == XPath select string
*             == NULL to use get_content instead
*   source == optional database source 
*             <candidate>, <running>
*   timeoutval == timeout value to use
*   dofill == TRUE if interactive mode and mandatory parms
*             need to be specified (they can still be skipped)
*             FALSE if no checking for mandatory parms
*             Just fill out the minimum path to root from
*             the 'get_content' node
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*
*    !!! get_content is consumed -- freed or transfered to a PDU
*    !!! that will be freed later
*
*    !!! source is consumed -- freed or transfered to a PDU
*    !!! that will be freed later
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_get_to_agent (agent_cb_t *agent_cb,
		       val_value_t *get_content,
		       const xmlChar *selectstr,
		       val_value_t *source,
		       uint32 timeoutval,
		       boolean dofill)
{
    const obj_template_t  *rpc, *input;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *filter, *dummy_parm;
    ses_cb_t              *scb;
    status_t               res;

    req = NULL;
    reqdata = NULL;
    res = NO_ERR;
    input = NULL;

    /* get the <get> or <get-config> input template */
    rpc = ncx_find_object(netconf_mod,
			  (source) ? 
			  NCX_EL_GET_CONFIG : NCX_EL_GET);
    if (rpc) {
	input = obj_find_child(rpc, NULL, YANG_K_INPUT);
    }

    if (!input) {
	if (source) {
	    val_free_value(source);
	}
	if (get_content) {
	    val_free_value(get_content);
	}
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* construct a method + parameter tree */
    reqdata = xml_val_new_struct(obj_get_name(rpc), 
				 obj_get_nsid(rpc));
    if (!reqdata) {
	if (source) {
	    val_free_value(source);
	}
	if (get_content) {
	    val_free_value(get_content);
	}
	log_error("\nError allocating a new RPC request");
	return ERR_INTERNAL_MEM;
    }

    if (source) {
	val_add_child(source, reqdata);
    }

    if (get_content) {
	/* set the get/input/filter node to the
	 * get_content, but after filling in any
	 * missing nodes from the root to the target
	 */
	filter = xml_val_new_struct(NCX_EL_FILTER, xmlns_nc_id());
    } else {
	filter = xml_val_new_flag(NCX_EL_FILTER, xmlns_nc_id());
    }
    if (!filter) {
	if (get_content) {
	    val_free_value(get_content);
	}
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_add_child(filter, reqdata);

    res = add_filter_attrs(filter, selectstr);
    if (res != NO_ERR) {
	if (get_content) {
	    val_free_value(get_content);
	}
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }

    if (get_content) {
	dummy_parm = NULL;
	res = add_filter_from_content_node(agent_cb,
					   rpc, 
					   get_content,
					   get_content->obj,
					   filter, 
					   dofill,
					   &dummy_parm);
	if (res != NO_ERR && res != ERR_NCX_SKIPPED) {
	    /*  val_free_value(get_content); already freed! */
	    val_free_value(reqdata);
	    return res;
	}
	res = NO_ERR;
    }

    /* !!! get_content consumed at this point !!!
     * allocate an RPC request and send it 
     */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    } else {
	req = mgr_rpc_new_request(scb);
	if (!req) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nError allocating a new RPC request");
	} else {
	    req->data = reqdata;
	    req->rpc = rpc;
	    req->timeout = timeoutval;
	}
    }
	
    if (res == NO_ERR) {
	if (LOGDEBUG2) {
	    log_debug2("\nabout to send RPC request with reqdata:");
	    val_dump_value(reqdata, NCX_DEF_INDENT);
	}

	/* the request will be stored if this returns NO_ERR */
	res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
    }

    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_get_to_agent */


/********************************************************************
 * FUNCTION get_content_from_choice
 * 
 * Get the content input for the EditOps from choice
 *
 * If the choice is 'from-cli' and this is interactive mode
 * then the fill_valset will be called to get input
 * based on the 'target' parameter also in the valset
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    valset == parsed CLI valset
 *
 * RETURNS:
 *   malloced result of the choice; this is the content
 *   that will be affected by the edit-config operation
 *   via create, merge, or replace
 *********************************************************************/
static val_value_t *
    get_content_from_choice (agent_cb_t *agent_cb,
			     const obj_template_t *rpc,
			     val_value_t *valset)
{
    val_value_t           *parm, *curparm, *newparm;
    const val_value_t     *userval;
    obj_template_t        *targobj;
    const xmlChar         *fromstr;
    var_type_t             vartype;
    boolean                iscli, isselect, saveopt;
    status_t               res;

    /* init locals */
    iscli = FALSE;
    isselect = FALSE;
    fromstr = NULL;
    res = NO_ERR;

    /* look for the 'from' parameter variant */
    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_VARREF);
    if (parm) {
	fromstr = VAL_STR(parm);
    } else {
	parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_SELECT);
	if (parm) {
	    isselect = TRUE;
	    fromstr = VAL_STR(parm);
	} else {
	    iscli = TRUE;
	}
    }

    if (iscli) {
	saveopt = agent_cb->get_optional;
	parm = val_find_child(valset, YANGCLI_MOD, 
			      YANGCLI_OPTIONAL);
	if (parm && parm->res == NO_ERR) {
	    agent_cb->get_optional = TRUE;
	}

	/* from CLI -- look for the 'target' parameter */
	parm = val_find_child(valset, YANGCLI_MOD, 
			      NCX_EL_TARGET);
	if (!parm) {
	    log_error("\nError: target parameter is missing");
	    agent_cb->get_optional = saveopt;
	    return NULL;
	}

	res = xpath_find_schema_target_int(VAL_STR(parm), &targobj);
	if (res != NO_ERR) {
	    log_error("\nError: Object '%s' not found", VAL_STR(parm));
	    agent_cb->get_optional = saveopt;
	    return NULL;
	}	

	curparm = NULL;
	parm = val_find_child(valset, YANGCLI_MOD, 
			      YANGCLI_CURRENT_VALUE);
	if (parm && parm->res == NO_ERR) {
	    curparm = var_get_script_val(targobj, NULL, 
					 VAL_STR(parm),
					 ISPARM, &res);
	    if (!curparm || res != NO_ERR) {
		log_error("\nError: Script value '%s' invalid (%s)", 
			  VAL_STR(parm), get_error_string(res)); 
		agent_cb->get_optional = saveopt;
		return NULL;
	    }
	    if (curparm->obj != targobj) {
		log_error("\nError: current value '%s' "
			  "object type is incorrect.",
			  VAL_STR(parm));
		agent_cb->get_optional = saveopt;
		return NULL;
	    }
	}

	switch (targobj->objtype) {
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    newparm = fill_value(agent_cb, rpc, targobj, 
				 curparm, &res);
	    break;
	case OBJ_TYP_CHOICE:
	    newparm = val_new_value();
	    if (!newparm) {
		log_error("\nError: malloc failure");
		res = ERR_INTERNAL_MEM;
	    } else {
		val_init_from_template(newparm, targobj);
	    
		res = get_choice(agent_cb, rpc, targobj,
				 newparm, curparm);
		if (res == ERR_NCX_SKIPPED) {
		    res = NO_ERR;
		}
	    }
	    break;
	case OBJ_TYP_CASE:
	    newparm = val_new_value();
	    if (!newparm) {
		log_error("\nError: malloc failure");
		res = ERR_INTERNAL_MEM;
	    } else {
		val_init_from_template(newparm, targobj);

		res = get_case(agent_cb, rpc, targobj,
			       newparm, curparm);
		if (res == ERR_NCX_SKIPPED) {
		    res = NO_ERR;
		}
	    }
	    break;
	default:
	    newparm = val_new_value();
	    if (!newparm) {
		log_error("\nError: malloc failure");
		res = ERR_INTERNAL_MEM;
	    } else {
		val_init_from_template(newparm, targobj);

		res = fill_valset(agent_cb, rpc,
				  newparm, curparm);
		if (res == ERR_NCX_SKIPPED) {
		    res = NO_ERR;
		}
	    }
	}

	agent_cb->get_optional = saveopt;
	if (res != NO_ERR) {
	    if (newparm) {
		val_free_value(newparm);
	    }
	    return NULL;
	} else {
	    return newparm;
	}
    } else if (isselect) {
	newparm = val_make_string(xmlns_nc_id(),
				  NCX_EL_SELECT, 
				  fromstr);
	if (!newparm) {
	    log_error("\nError: make string failed");
	}
	return newparm;
    } else {
	/* from global or local variable */
	if (!fromstr) {
	    ;  /* should not be NULL */
	} else if (*fromstr == '$' && fromstr[1] == '$') {
	    /* $$foo */
	    vartype = VAR_TYP_GLOBAL;
	    fromstr += 2;
	} else if (*fromstr == '$') {
	    /* $foo */
	    vartype = VAR_TYP_LOCAL;
	    fromstr++;
	} else {
	    /* 'foo' : just assume local, not error */
	    vartype = VAR_TYP_LOCAL;
	}
	if (fromstr) {
	    userval = var_get(fromstr, vartype);
	    if (!userval) {
		log_error("\nError: variable '%s' not found", 
			  fromstr);
		return NULL;
	    } else {
		newparm = val_clone(userval);
		if (!newparm) {
		    log_error("\nError: valloc failed");
		}
		return newparm;
	    }
	}
    }
    return NULL;

}  /* get_content_from_choice */


/********************************************************************
 * FUNCTION add_one_operation_attr
 * 
 * Add the nc:operation attribute to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    op == edit operation to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_one_operation_attr (val_value_t *val,
			    op_editop_t op)
{
    const obj_template_t *operobj;
    const xmlChar        *editopstr;
    val_value_t          *metaval;
    status_t              res;

    /* get the internal nc:operation object */
    operobj = ncx_find_object(netconf_mod, NC_OPERATION_ATTR_NAME);
    if (!operobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* create a value node for the meta-value */
    metaval = val_new_value();
    if (!metaval) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(metaval, operobj);

    /* get the string value for the edit operation */
    editopstr = op_editop_name(op);
    if (!editopstr) {
	val_free_value(metaval);
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* set the meta variable value and other fields */
    res = val_set_simval(metaval,
			 obj_get_ctypdef(operobj),
			 obj_get_nsid(operobj),
			 obj_get_name(operobj),
			 editopstr);

    if (res != NO_ERR) {
	val_free_value(metaval);
	return res;
    }

    dlq_enque(metaval, &val->metaQ);
    return NO_ERR;

} /* add_one_operation_attr */


/********************************************************************
 * FUNCTION add_operation_attr
 * 
 * Add the nc:operation attribute to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    op == edit operation to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_operation_attr (val_value_t *val,
			op_editop_t op)
{
    val_value_t          *childval;
    status_t              res;

    res = NO_ERR;

    switch (val->obj->objtype) {
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
	for (childval = val_get_first_child(val);
	     childval != NULL;
	     childval = val_get_next_child(childval)) {

	    res = add_one_operation_attr(childval, op);
	    if (res != NO_ERR) {
		return res;
	    }
	}
	break;
    default:
	res = add_one_operation_attr(val, op);
    }

    return res;

} /* add_operation_attr */


/********************************************************************
 * FUNCTION add_one_insert_attrs
 * 
 * Add the yang:insert attribute(s) to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    insop == insert operation to use
 *    edit_target == edit target to use for key or value attr
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_one_insert_attrs (val_value_t *val,
			  op_insertop_t insop,
			  const xmlChar *edit_target)
{
    const obj_template_t *operobj;
    const xmlChar        *insopstr;
    val_value_t          *metaval;
    ncx_node_t            dtyp;
    status_t              res;
    xmlns_id_t            yangid;

    yangid = xmlns_yang_id();

    /* get the internal nc:operation object */
    dtyp = NCX_NT_OBJ;
    operobj = ncx_get_gen_string();
    if (!operobj) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    insopstr = op_insertop_name(insop);

    /* create a value node for the meta-value */
    metaval = val_new_value();
    if (!metaval) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(metaval, operobj);
    val_set_qname(metaval, yangid,
		  YANG_K_INSERT,
		  xml_strlen(YANG_K_INSERT));

    /* set the meta variable value and other fields */
    res = val_set_simval(metaval,
			 metaval->typdef,
			 yangid,
			 YANG_K_INSERT,
			 insopstr);
    if (res != NO_ERR) {
	val_free_value(metaval);
	return res;
    } else {
	dlq_enque(metaval, &val->metaQ);
    }

    if (insop == OP_INSOP_BEFORE || insop == OP_INSOP_AFTER) {
	/* create a value node for the meta-value */
	metaval = val_new_value();
	if (!metaval) {
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(metaval, operobj);

	/* set the attribute name */
	if (val->obj->objtype==OBJ_TYP_LEAF_LIST) {
	    val_set_qname(metaval, yangid,
			  YANG_K_VALUE,
			  xml_strlen(YANG_K_VALUE));
	} else {
	    val_set_qname(metaval, yangid,
			  YANG_K_KEY,
			  xml_strlen(YANG_K_KEY));
	}

	/* set the meta variable value and other fields */
	res = val_set_simval(metaval,
			     metaval->typdef,
			     yangid, NULL,
			     edit_target);
	if (res != NO_ERR) {
	    val_free_value(metaval);
	    return res;
	} else {
	    dlq_enque(metaval, &val->metaQ);
	}
    }	  

    return NO_ERR;

} /* add_one_insert_attrs */


/********************************************************************
 * FUNCTION add_insert_attrs
 * 
 * Add the yang:insert attribute(s) to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    insop == insert operation to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_insert_attrs (val_value_t *val,
		      op_insertop_t insop,
		      const xmlChar *edit_target)
{
    val_value_t          *childval;
    status_t              res;

    res = NO_ERR;

    switch (val->obj->objtype) {
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
	for (childval = val_get_first_child(val);
	     childval != NULL;
	     childval = val_get_next_child(childval)) {

	    res = add_one_insert_attrs(childval, insop,
				       edit_target);
	    if (res != NO_ERR) {
		return res;
	    }
	}
	break;
    default:
	res = add_one_insert_attrs(val, insop, edit_target);
    }

    return res;

} /* add_insert_attrs */


/********************************************************************
 * FUNCTION do_create
 * 
 * Create some database object on the agent
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   an edit-config operation is sent to the agent
 *
 *********************************************************************/
static void
    do_create (agent_cb_t *agent_cb,
	       const obj_template_t *rpc,
	       const xmlChar *line,
	       uint32  len)
{
    val_value_t           *valset, *content, *parm;
    status_t               res;
    uint32                 timeoutval;

    /* init locals */
    res = NO_ERR;
    content = NULL;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, rpc, valset);
    if (!content) {
	val_free_value(valset);
	return;
    }


    /* add nc:operation attribute to the value node */
    res = add_operation_attr(content, OP_EDITOP_CREATE);
    if (res != NO_ERR) {
	log_error("\nError: Creation of nc:operation"
		  " attribute failed");
	val_free_value(valset);
	val_free_value(content);
	return;
    }

    /* construct an edit-config PDU with default parameters */
    res = send_edit_config_to_agent(agent_cb, content, timeoutval);
    if (res != NO_ERR) {
	log_error("\nError: send create operation failed (%s)",
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_create */


/********************************************************************
 * FUNCTION do_merge
 * 
 * Merge some database object on the agent
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the merge command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   an edit-config operation is sent to the agent
 *
 *********************************************************************/
static void
    do_merge (agent_cb_t *agent_cb,
	      const obj_template_t *rpc,
	      const xmlChar *line,
	      uint32  len)
{
    val_value_t           *valset, *content, *parm;
    status_t               res;
    uint32                 timeoutval;

    /* init locals */
    res = NO_ERR;
    content = NULL;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, rpc, valset);
    if (!content) {
	val_free_value(valset);
	return;
    }

    /* add nc:operation attribute to the value node */
    res = add_operation_attr(content, OP_EDITOP_MERGE);
    if (res != NO_ERR) {
	log_error("\nError: Creation of nc:operation"
		  " attribute failed");
	val_free_value(valset);
	val_free_value(content);
	return;
    }

    /* construct an edit-config PDU with default parameters */
    res = send_edit_config_to_agent(agent_cb, content, timeoutval);
    if (res != NO_ERR) {
	log_error("\nError: send merge operation failed (%s)",
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_merge */


/********************************************************************
 * FUNCTION do_replace
 * 
 * Replace some database object on the agent
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the replace command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   an edit-config operation is sent to the agent
 *
 *********************************************************************/
static void
    do_replace (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		uint32  len)
{
    val_value_t           *valset, *content, *parm;
    status_t               res;
    uint32                 timeoutval;

    /* init locals */
    res = NO_ERR;
    content = NULL;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, rpc, valset);
    if (!content) {
	val_free_value(valset);
	return;
    }

    /* add nc:operation attribute to the value node */
    res = add_operation_attr(content, OP_EDITOP_REPLACE);
    if (res != NO_ERR) {
	log_error("\nError: Creation of nc:operation"
		  " attribute failed");
	val_free_value(valset);
	val_free_value(content);
	return;
    }

    /* construct an edit-config PDU with default parameters */
    res = send_edit_config_to_agent(agent_cb, content, timeoutval);
    if (res != NO_ERR) {
	log_error("\nError: send replace operation failed (%s)",
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_replace */


/********************************************************************
 * FUNCTION do_delete
 * 
 * Delete some database object on the agent
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the delete command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   an edit-config operation is sent to the agent
 *
 *********************************************************************/
static void
    do_delete (agent_cb_t *agent_cb,
	       const obj_template_t *rpc,
	       const xmlChar *line,
	       uint32  len)
{
    obj_template_t        *targobj;
    val_value_t           *valset, *content, *target, *parm;
    status_t               res;
    uint32                 timeoutval;

    /* init locals */
    res = NO_ERR;
    content = NULL;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    target = val_find_child(valset, YANGCLI_MOD, 
			    NCX_EL_TARGET);
    if (!target) {
	log_error("\nError: target parameter is missing");
	val_free_value(valset);
	return;
    }

    res = xpath_find_schema_target_int(VAL_STR(target), 
				       &targobj);
    if (res != NO_ERR) {
	log_error("\nError: Object '%s' not found", 
		  VAL_STR(target));
	val_free_value(valset);
	return;
    }

    /* add content only if this is a leaf-list */
    if (targobj->objtype == OBJ_TYP_LEAF_LIST) {
	log_stdout("\nSpecify the leaf-list value to delete:");
	content = fill_value(agent_cb, rpc, targobj, NULL, &res);
    } else {
	/* create an empty content node to delete */
	content = val_new_value();
	if (content) {
	    val_init_from_template(content, targobj);
	}
    }

    if (!content) {
	val_free_value(valset);
	return;
    }

    /* add nc:operation attribute to the value node */
    res = add_operation_attr(content, OP_EDITOP_DELETE);
    if (res != NO_ERR) {
	log_error("\nError: Creation of nc:operation"
		  " attribute failed");
	val_free_value(valset);
	val_free_value(content);
	return;
    }

    /* construct an edit-config PDU with default parameters */
    res = send_edit_config_to_agent(agent_cb, content, timeoutval);
    if (res != NO_ERR) {
	log_error("\nError: send delete operation failed (%s)",
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_delete */


/********************************************************************
 * FUNCTION do_insert
 * 
 * Insert a database object on the agent
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   an edit-config operation is sent to the agent
 *
 *********************************************************************/
static void
    do_insert (agent_cb_t *agent_cb,
	       const obj_template_t *rpc,
	       const xmlChar *line,
	       uint32  len)
{
    val_value_t      *valset, *content, *tempval, *parm;
    const xmlChar    *edit_target;
    op_editop_t       editop;
    op_insertop_t     insertop;
    status_t          res;
    uint32            timeoutval;

    /* init locals */
    res = NO_ERR;
    content = NULL;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, rpc, valset);
    if (!content) {
	val_free_value(valset);
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    /* get the insert order */
    tempval = val_find_child(valset, YANGCLI_MOD,
			     YANGCLI_ORDER);
    if (tempval && tempval->res == NO_ERR) {
	insertop = op_insertop_id(VAL_ENUM_NAME(tempval));
    } else {
	insertop = OP_INSOP_LAST;
    }

    /* get the edit-config operation */
    tempval = val_find_child(valset, YANGCLI_MOD,
			     YANGCLI_OPERATION);
    if (tempval && tempval->res == NO_ERR) {
	editop = op_editop_id(VAL_ENUM_NAME(tempval));
    } else {
	editop = OP_EDITOP_MERGE;
    }

    /* get the edit-target parameter only if the
     * order is 'before' or 'after'; ignore otherwise
     */
    tempval = val_find_child(valset, YANGCLI_MOD,
			     YANGCLI_EDIT_TARGET);
    if (tempval && tempval->res == NO_ERR) {
	edit_target = VAL_STR(tempval);
    } else {
	edit_target = NULL;
    }

    /* check if the edit-target is needed */
    switch (insertop) {
    case OP_INSOP_BEFORE:
    case OP_INSOP_AFTER:
	if (!edit_target) {
	    log_error("\nError: edit-target parameter missing");
	    val_free_value(content);
	    val_free_value(valset);
	    return;
	}
	break;
    default:
	;
    }

    /* add nc:operation attribute to the value node */
    res = add_operation_attr(content, editop);
    if (res != NO_ERR) {
	log_error("\nError: Creation of nc:operation"
		  " attribute failed");
    }

    /* add yang:insert attribute and possibly a key or value
     * attribute as well
     */
    if (res == NO_ERR) {
	res = add_insert_attrs(content, insertop,
			       edit_target);
	if (res != NO_ERR) {
	    log_error("\nError: Creation of yang:insert"
		  " attribute(s) failed");
	}
    }

    /* send the PDU, hand off the content node */
    if (res == NO_ERR) {
	/* construct an edit-config PDU with default parameters */
	res = send_edit_config_to_agent(agent_cb, content, timeoutval);
	if (res != NO_ERR) {
	    log_error("\nError: send create operation failed (%s)",
		      get_error_string(res));
	}
	content = NULL;
    }

    /* cleanup and exit */
    if (content) {
	val_free_value(content);
    }

    val_free_value(valset);

}  /* do_insert */


/********************************************************************
 * FUNCTION do_sget
 * 
 * Get some running config and/or state data with the <get> operation,
 * using an optional subtree
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   a <get> operation is sent to the agent
 *
 *********************************************************************/
static void
    do_sget (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t           *valset, *content, *parm;
    status_t               res;
    uint32                 timeoutval;
    boolean                dofill;

    /* init locals */
    res = NO_ERR;
    content = NULL;
    dofill = TRUE;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_NOFILL);
    if (parm && parm->res == NO_ERR) {
	dofill = FALSE;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, rpc, valset);
    if (!content) {
	val_free_value(valset);
	return;
    }

    /* construct a get PDU with the content as the filter */
    res = send_get_to_agent(agent_cb, 
			    content, 
			    NULL, 
			    NULL, 
			    timeoutval, 
			    dofill);
    if (res != NO_ERR) {
	log_error("\nError: send get operation failed (%s)",
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_sget */


/********************************************************************
 * FUNCTION do_sget_config
 * 
 * Get some config data with the <get-config> operation,
 * using an optional subtree
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   a <get-config> operation is sent to the agent
 *
 *********************************************************************/
static void
    do_sget_config (agent_cb_t *agent_cb,
		    const obj_template_t *rpc,
		    const xmlChar *line,
		    uint32  len)
{
    val_value_t     *valset, *content, *source, *parm;
    status_t         res;
    uint32           timeoutval;
    boolean          dofill;

    dofill = TRUE;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_NOFILL);
    if (parm && parm->res == NO_ERR) {
	dofill = FALSE;
    }

    /* get the source parameter */
    source = val_find_child(valset, NULL, NCX_EL_SOURCE);
    if (!source) {
	log_error("\nError: mandatory source parameter missing");
	val_free_value(valset);
	return;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, rpc, valset);
    if (!content) {
	val_free_value(valset);
	return;
    }

    /* hand off this malloced node to send_get_to_agent */
    val_remove_child(source);
    val_change_nsid(source, xmlns_nc_id());

    /* construct a get PDU with the content as the filter */
    res = send_get_to_agent(agent_cb, 
			    content, 
			    NULL, 
			    source, 
			    timeoutval, 
			    dofill);
    if (res != NO_ERR) {
	log_error("\nError: send get-config operation failed (%s)",
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_sget_config */


/********************************************************************
 * FUNCTION do_xget
 * 
 * Get some running config and/or state data with the <get> operation,
 * using an optional XPath filter
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   a <get> operation is sent to the agent
 *
 *********************************************************************/
static void
    do_xget (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    const ses_cb_t      *scb;
    const mgr_scb_t     *mscb;
    val_value_t         *valset, *content, *parm;
    const xmlChar       *str;
    status_t             res;
    uint32               retcode, timeoutval;

    /* get the session info */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    mscb = (const mgr_scb_t *)scb->mgrcb;

    /* init locals */
    res = NO_ERR;
    content = NULL;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    /* check if the agent supports :xpath */
    if (!cap_std_set(&mscb->caplist, CAP_STDID_XPATH)) {
	switch (agent_cb->baddata) {
	case NCX_BAD_DATA_IGNORE:
	    break;
	case NCX_BAD_DATA_WARN:
	    log_warn("\nWarning: agent does not have :xpath support");
	    break;
	case NCX_BAD_DATA_CHECK:
	    retcode = 0;
	    log_warn("\nWarning: agent does not have :xpath support");
	    res = get_yesno(agent_cb,
			    (const xmlChar *)"Send request anyway?",
			    YESNO_NO, &retcode);
	    if (res == NO_ERR) {
		switch (retcode) {
		case YESNO_CANCEL:
		case YESNO_NO:
		    res = ERR_NCX_CANCELED;
		    break;
		case YESNO_YES:
		    break;
		default:
		    res = SET_ERROR(ERR_INTERNAL_VAL);
		}
	    }
	    break;
	case NCX_BAD_DATA_ERROR:
	    log_error("\nError: agent does not have :xpath support");
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    break;
	case NCX_BAD_DATA_NONE:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* check any error so far */
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, rpc, valset);
    if (content) {
	if (content->btyp == NCX_BT_STRING && VAL_STR(content)) {
	    str = VAL_STR(content);
	    while (*str && *str != '"') {
		str++;
	    }
	    if (*str) {
		log_error("\nError: select string cannot "
			  "contain a double quote");
	    } else {
		/* construct a get PDU with the content
		 * as the filter 
		 */
		res = send_get_to_agent(agent_cb, 
					NULL, 
					VAL_STR(content), 
					NULL, 
					timeoutval,
					FALSE);
		if (res != NO_ERR) {
		    log_error("\nError: send get operation"
			      " failed (%s)",
			      get_error_string(res));
		}
	    }
	} else {
	    log_error("\nError: xget content wrong type");
	}
	val_free_value(content);
    }

    val_free_value(valset);

}  /* do_xget */


/********************************************************************
 * FUNCTION do_xget_config
 * 
 * Get some config data with the <get-config> operation,
 * using an optional XPath filter
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   a <get-config> operation is sent to the agent
 *
 *********************************************************************/
static void
    do_xget_config (agent_cb_t *agent_cb,
		    const obj_template_t *rpc,
		    const xmlChar *line,
		    uint32  len)
{
    const ses_cb_t      *scb;
    const mgr_scb_t     *mscb;
    val_value_t         *valset, *content, *source, *parm;
    const xmlChar       *str;
    status_t             res;
    uint32               retcode, timeoutval;

    /* get the session info */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    mscb = (const mgr_scb_t *)scb->mgrcb;

    /* init locals */
    res = NO_ERR;
    content = NULL;
      
    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    /* check if the agent supports :xpath */
    if (!cap_std_set(&mscb->caplist, CAP_STDID_XPATH)) {
	switch (agent_cb->baddata) {
	case NCX_BAD_DATA_IGNORE:
	    break;
	case NCX_BAD_DATA_WARN:
	    log_warn("\nWarning: agent does not have :xpath support");
	    break;
	case NCX_BAD_DATA_CHECK:
	    retcode = 0;
	    log_warn("\nWarning: agent does not have :xpath support");
	    res = get_yesno(agent_cb,
			    (const xmlChar *)"Send request anyway?",
			    YESNO_NO, &retcode);
	    if (res == NO_ERR) {
		switch (retcode) {
		case YESNO_CANCEL:
		case YESNO_NO:
		    res = ERR_NCX_CANCELED;
		    break;
		case YESNO_YES:
		    break;
		default:
		    res = SET_ERROR(ERR_INTERNAL_VAL);
		}
	    }
	    break;
	case NCX_BAD_DATA_ERROR:
	    log_error("\nError: agent does not have :xpath support");
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    break;
	case NCX_BAD_DATA_NONE:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* check any error so far */
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    /* get the source parameter */
    source = val_find_child(valset, NULL, NCX_EL_SOURCE);
    if (!source) {
	log_error("\nError: mandatory source parameter missing");
	val_free_value(valset);
	return;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, rpc, valset);
    if (content) {
	if (content->btyp == NCX_BT_STRING && VAL_STR(content)) {
	    str = VAL_STR(content);
	    while (*str && *str != '"') {
		str++;
	    }
	    if (*str) {
		log_error("\nError: select string cannot "
			  "contain a double quote");
	    } else {
		/* hand off this malloced node to send_get_to_agent */
		val_remove_child(source);
		val_change_nsid(source, xmlns_nc_id());

		/* construct a get PDU with the content as the filter */
		res = send_get_to_agent(agent_cb, 
					NULL, 
					VAL_STR(content), 
					source,
					timeoutval,
					FALSE);
		if (res != NO_ERR) {
		    log_error("\nError: send get-config "
			      "operation failed (%s)",
			      get_error_string(res));
		}
	    }
	} else {
	    log_error("\nError: xget content wrong type");
	}
	val_free_value(content);
    }

    val_free_value(valset);

}  /* do_xget_config */


/********************************************************************
* FUNCTION do_local_conn_command
* 
* Handle local connection mode RPC operations from yangcli.yang
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == template for the local RPC
*   line == input command line from user
*   len == line length
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
* RETURNS:
*    NO_ERR if a RPC was executed
*    ERR_NCX_SKIPPED if no command was invoked
*********************************************************************/
static status_t
    do_local_conn_command (agent_cb_t *agent_cb,
			   const obj_template_t *rpc,
			   xmlChar *line,
			   uint32  len)
{
    const xmlChar *rpcname;

    rpcname = obj_get_name(rpc);

    if (!xml_strcmp(rpcname, YANGCLI_CREATE)) {
	do_create(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_DELETE)) {
	do_delete(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_INSERT)) {
	do_insert(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_MERGE)) {
	do_merge(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_REPLACE)) {
	do_replace(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_SAVE)) {
	if (len < xml_strlen(line)) {
	    log_error("\nWarning: Extra characters ignored (%s)",
		      &line[len]);
	}
	do_save(agent_cb);
    } else if (!xml_strcmp(rpcname, YANGCLI_SGET)) {
	do_sget(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_SGET_CONFIG)) {
	do_sget_config(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_XGET)) {
	do_xget(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_XGET_CONFIG)) {
	do_xget_config(agent_cb, rpc, line, len);
    } else {
	return ERR_NCX_SKIPPED;
    }
    return NO_ERR;

} /* do_local_conn_command */


/********************************************************************
* FUNCTION do_local_command
* 
* Handle local RPC operations from yangcli.yang
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == template for the local RPC
*   line == input command line from user
*   len == length of line in bytes
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
static void
    do_local_command (agent_cb_t *agent_cb,
		      const obj_template_t *rpc,
		      xmlChar *line,
		      uint32  len)
{
    const xmlChar *rpcname;

    rpcname = obj_get_name(rpc);

    if (!xml_strcmp(rpcname, YANGCLI_CD)) {
	do_cd(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_CONNECT)) {
	do_connect(agent_cb, rpc, line, len, FALSE);
    } else if (!xml_strcmp(rpcname, YANGCLI_FILL)) {
	do_fill(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_HELP)) {
	do_help(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_LIST)) {
	do_list(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_MGRLOAD)) {
	do_mgrload(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_PWD)) {
	do_pwd(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_QUIT)) {
	agent_cb->state = MGR_IO_ST_SHUT;
	mgr_request_shutdown();
    } else if (!xml_strcmp(rpcname, YANGCLI_RUN)) {
	(void)do_run(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_SHOW)) {
	do_show(agent_cb, rpc, line, len);
    } else {
	log_error("\nError: The %s command is not allowed in this mode",
		   rpcname);
    }
} /* do_local_command */



/********************************************************************
* FUNCTION top_command
* 
* Top-level command handler
*
* INPUTS:
*   agent_cb == agent control block to use
*   line == input command line from user
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
static void
    top_command (agent_cb_t *agent_cb,
		 xmlChar *line)
{
    const obj_template_t  *rpc;
    uint32                 len;
    ncx_node_t             dtyp;
    status_t               res;

    if (!xml_strlen(line)) {
	return;
    }

    dtyp = NCX_NT_OBJ;
    rpc = (const obj_template_t *)parse_def(agent_cb,
					    &dtyp, line, &len);
    if (!rpc) {
	if (agent_cb->result_name || agent_cb->result_filename) {
	    res = finish_result_assign(agent_cb, NULL, line);
	} else {
	    /* this is an unknown command */
	    log_error("\nError: Unrecognized command");
	}
	return;
    }

    /* check  handful of yangcli commands */
    if (is_yangcli_ns(obj_get_nsid(rpc))) {
	do_local_command(agent_cb, rpc, line, len);
    } else {
	log_error("\nError: Not connected to agent."
		  "\nLocal commands only in this mode.");
    }

} /* top_command */


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
static void
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
* FUNCTION conn_command
* 
* Connection level command handler
*
* INPUTS:
*   agent_cb == agent control block to use
*   line == input command line from user
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
static void
    conn_command (agent_cb_t *agent_cb,
		  xmlChar *line)
{
    const obj_template_t  *rpc, *input;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *valset, *parm;
    ses_cb_t              *scb;
    uint32                 len, linelen;
    status_t               res;
    boolean                shut, load;
    ncx_node_t             dtyp;

    req = NULL;
    reqdata = NULL;
    valset = NULL;
    res = NO_ERR;
    shut = FALSE;
    load = FALSE;

    /* make sure there is something to parse */
    linelen = xml_strlen(line);
    if (!linelen) {
	return;
    }

    /* get the RPC method template */
    dtyp = NCX_NT_OBJ;
    rpc = (const obj_template_t *)parse_def(agent_cb,
					    &dtyp, line, &len);
    if (!rpc) {
	if (agent_cb->result_name || agent_cb->result_filename) {
	    res = finish_result_assign(agent_cb, NULL, line);
	} else {
	    /* this is an unknown command */
	    log_stdout("\nUnrecognized command");
	}
	return;
    }

    /* check local commands */
    if (is_yangcli_ns(obj_get_nsid(rpc))) {
	if (!xml_strcmp(obj_get_name(rpc), YANGCLI_CONNECT)) {
	    log_stdout("\nError: Already connected");
	} else {
	    res = do_local_conn_command(agent_cb, rpc, line, len);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
		do_local_command(agent_cb, rpc, line, len);
	    }
	}
	return;
    }

    /* else treat this as an RPC request going to the agent
     * first construct a method + parameter tree 
     */
    reqdata = xml_val_new_struct(obj_get_name(rpc), 
				 obj_get_nsid(rpc));
    if (!reqdata) {
	log_error("\nError allocating a new RPC request");
	res = ERR_INTERNAL_MEM;
    }

    /* should find an input node */
    input = obj_find_child(rpc, NULL, YANG_K_INPUT);

    /* check if any params are expected */
    if (res == NO_ERR && input && obj_get_child_count(input)) {
	while (line[len] && xml_isspace(line[len])) {
	    len++;
	}

	if (len < linelen) {
	    valset = parse_rpc_cli(rpc, &line[len], &res);
	    if (res != NO_ERR) {
		log_error("\nError in the parameters for RPC %s (%s)",
			  obj_get_name(rpc), get_error_string(res));
	    }
	}

	/* check no input from user, so start a parmset */
	if (res == NO_ERR && !valset) {
	    valset = val_new_value();
	    if (!valset) {
		res = ERR_INTERNAL_MEM;
	    } else {
		val_init_from_template(valset, input);
	    }
	}

	/* fill in any missing parameters from the CLI */
	if (res == NO_ERR) {
	    if (interactive_mode()) {
		res = fill_valset(agent_cb, rpc, valset, NULL);
		if (res == ERR_NCX_SKIPPED) {
		    res = NO_ERR;
		}
	    }
	}

	/* go through the parm list and move the values 
	 * to the reqdata struct. 
	 */
	if (res == NO_ERR) {
	    parm = val_get_first_child(valset);
	    while (parm) {
		val_remove_child(parm);
		val_add_child(parm, reqdata);
		parm = val_get_first_child(valset);
	    }
	}
    }

    /* check the close-session corner cases */
    if (res == NO_ERR && !xml_strcmp(obj_get_name(rpc), 
				     NCX_EL_CLOSE_SESSION)) {
	shut = TRUE;
    }
	    
    /* allocate an RPC request and send it */
    if (res == NO_ERR) {
	scb = mgr_ses_get_scb(agent_cb->mysid);
	if (!scb) {
	    res = SET_ERROR(ERR_INTERNAL_PTR);
	} else {
	    req = mgr_rpc_new_request(scb);
	    if (!req) {
		res = ERR_INTERNAL_MEM;
		log_error("\nError allocating a new RPC request");
	    } else {
		req->data = reqdata;
		req->rpc = rpc;
		req->timeout = agent_cb->timeout;
	    }
	}
	
	if (res == NO_ERR) {
	    if (LOGDEBUG2) {
		log_debug2("\nabout to send RPC request with reqdata:");
		val_dump_value(reqdata, NCX_DEF_INDENT);
	    }

	    /* the request will be stored if this returns NO_ERR */
	    res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
	}
    }

    if (valset) {
	val_free_value(valset);
    }

    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else if (shut) {
	agent_cb->state = MGR_IO_ST_CONN_CLOSEWAIT;
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

} /* conn_command */


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
    if (!obj) {
	res = ERR_NCX_NOT_FOUND;
    }

    if (res == NO_ERR) {
	/* check no command line parms */
	if (argc <= 1) {
	    mgr_cli_valset = val_new_value();
	    if (!mgr_cli_valset) {
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
    confname = get_strparm(mgr_cli_valset, YANGCLI_MOD, YANGCLI_CONF);
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
	if (!modules) {
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

    log_stdout("\n\nNETCONF session established for %s on %s",
	   scb->username, mscb->target ? mscb->target : agent);

    log_stdout("\n\nAgent Protocol Capabilities");
    cap_dump_stdcaps(&mscb->caplist);

    log_stdout("\n\nAgent Module Capabilities");
    cap_dump_modcaps(&mscb->caplist);

    log_stdout("\n\nAgent Enterprise Capabilities");
    cap_dump_entcaps(&mscb->caplist);
    log_stdout("\n");

    log_stdout("\nDefault target set to: ");
    switch (mscb->targtyp) {
    case NCX_AGT_TARG_NONE:
	agent_cb->default_target = NULL;
	log_stdout("none");
	break;
    case NCX_AGT_TARG_CANDIDATE:
	agent_cb->default_target = NCX_EL_CANDIDATE;
	log_stdout("<candidate>");
	break;
    case NCX_AGT_TARG_RUNNING:
	agent_cb->default_target = NCX_EL_RUNNING;	
	log_stdout("<running>");
	break;
    case NCX_AGT_TARG_CAND_RUNNING:
	log_stdout("<candidate> (<running> also supported)");
	break;
    case NCX_AGT_TARG_LOCAL:
	agent_cb->default_target = NULL;
	log_stdout("none -- local file");	
	break;
    case NCX_AGT_TARG_REMOTE:
	agent_cb->default_target = NULL;
	log_stdout("none -- remote file");	
	break;
    default:
	agent_cb->default_target = NULL;
	SET_ERROR(ERR_INTERNAL_VAL);
	log_stdout("none -- unknown (%d)", mscb->targtyp);
	break;
    }

    log_stdout("\nSave operation mapped to: ");
    switch (mscb->targtyp) {
    case NCX_AGT_TARG_NONE:
	log_stdout("none");
	break;
    case NCX_AGT_TARG_CANDIDATE:
    case NCX_AGT_TARG_CAND_RUNNING:
	log_stdout("commit");
	if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
	    log_stdout(" + copy-config <running> <startup>");
	}
	break;
    case NCX_AGT_TARG_RUNNING:
	if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
	    log_stdout("copy-config <running> <startup>");
	} else {
	    log_stdout("none");
	}	    
	break;
    case NCX_AGT_TARG_LOCAL:
    case NCX_AGT_TARG_REMOTE:
	/* no way to assign these enums from the capabilities alone! */
	if (cap_std_set(&mscb->caplist, CAP_STDID_URL)) {
	    log_stdout("copy-config <running> <url>");
	} else {
	    log_stdout("none");
	}	    
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	log_stdout("none");
	break;
    }

    log_stdout("\nDefault with-defaults behavior: ");
    if (mscb->caplist.cap_defstyle) {
	log_stdout("%s", mscb->caplist.cap_defstyle);
    } else {
	log_stdout("unknown");
    }

    log_stdout("\n");
    
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
    uint32              modlen;
    status_t            res;
    xmlChar             namebuff[NCX_MAX_NLEN+1];

    mscb = (const mgr_scb_t *)scb->mgrcb;

    log_info("\n\nChecking Agent Modules...");

    /**** HARDWIRE ADD NETCONF V1 TO THE QUEUE
     **** NEED TO GET THE STD CAP INSTEAD
     ****/
    mod = ncx_find_module(NC_MODULE, NULL);
    if (mod) {
	modptr = new_modptr(mod);
	if (!modptr) {
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

	if (!module || !modlen || !version) {
	    log_warn("\nWarning: skipping invalid module capability "
		     "for URI '%s'", cap->cap_uri);
	    cap = cap_next_modcap(cap);
	    continue;
	}

	xml_strncpy(namebuff, module, modlen);
	mod = ncx_find_module(namebuff, version);
	if (!mod) {
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
	    if (!modptr) {
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

    /****/
    agent_cb = cur_agent_cb;

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
	if (!scb) {
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
	if (!scb || scb->state == SES_ST_SHUTDOWN_REQ) {
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
	if (!scb || scb->state == SES_ST_SHUTDOWN_REQ) {
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
	if (!line || res != NO_ERR) {
	    if (batchmode) {
		mgr_request_shutdown();
	    }
	    return agent_cb->state;
	}
    } else {
	/* block until user enters some input */
	line = get_cmd_line(agent_cb, &res);
	if (!line) {
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
    memset(clibuff, 0x0, sizeof(clibuff));
    climore = FALSE;
    malloc_cnt = 0;
    free_cnt = 0;

    /* get a read line context with a history buffer 
    * change later to not get allocated if batch mode active
    */
    cli_gl = new_GetLine(YANGCLI_LINELEN, YANGCLI_HISTLEN);
    if (!cli_gl) {
	return ERR_INTERNAL_MEM;
    }

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     */
    res = ncx_init(NCX_SAVESTR, 
		   log_level, 
		   TRUE,
		   "\nStarting yangcli",
		   argc, argv);

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
    if (!obj) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* set the parmset object to the input node of the RPC */
    obj = obj_find_child(obj, NULL, YANG_K_INPUT);
    if (!obj) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* treat the connect-to-agent parmset special
     * it is saved for auto-start plus restart parameters
     * Setup an empty parmset to hold the connect parameters
     */
    connect_valset = val_new_value();
    if (!connect_valset) {
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
	log_stdout("\nyangcli version %s\n", progver);
    }

    /* check print help and exit */
    if (helpmode) {
	log_stdout("\nyangcli version %s", progver);
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
    if (!agent_cb) {
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

    log_debug2("\nShutting down yangcli\n");

    /* cleanup the user edit buffer */
    if (cli_gl) {
	(void)del_GetLine(cli_gl);
	cli_gl = NULL;
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
	log_stdout("\nyangcli version %s\n", progver);
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
