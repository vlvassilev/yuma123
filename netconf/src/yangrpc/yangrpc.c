/*
 * Copyright (c) 2013 - 2016, Vladimir Vassilev, All Rights Reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

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
#include <time.h>
#include <sys/time.h>
#include <assert.h>

#include "mgr.h"

#include "procdefs.h"
#include "cli.h"
#include "conf.h"
#include "help.h"
#include "json_wr.h"
#include "log.h"
#include "ncxmod.h"
#include "mgr.h"
#include "mgr_hello.h"
#include "mgr_io.h"
#include "mgr_not.h"
#include "mgr_rpc.h"
#include "mgr_ses.h"
#include "ncx.h"
#include "ncx_list.h"
#include "ncx_num.h"
#include "ncx_str.h"
#include "ncxconst.h"
#include "ncxmod.h"
#include "obj.h"
#include "op.h"
#include "rpc.h"
#include "runstack.h"
#include "ses_msg.h"
#include "status.h"
#include "val.h"
#include "val_util.h"
#include "var.h"
#include "xml_util.h"
#include "xml_val.h"
#include "xml_wr.h"
#include "yangconst.h"
#include "yangcli.h"
#include "yangcli_cmd.h"
#include "yangcli_alias.h"
#include "yangcli_autoload.h"
#include "yangcli_autolock.h"
#include "yangcli_save.h"
#include "yangcli_tab.h"
#include "yangcli_uservars.h"
#include "yangcli_util.h"
#include "yangcli_globals.h"
#include "yangcli_wordexp.h"

#include "yangrpc.h"

/*****************  I N T E R N A L   V A R S  ****************/

/* TBD: use multiple server control blocks, stored in this Q */
static dlq_hdr_t      server_cbQ;


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

/* name of script passed at invocation to auto-run */
static xmlChar        *runscript;

/* TRUE if runscript has been completed */
static boolean         runscriptdone;

/* command string passed at invocation to auto-run */
static xmlChar        *runcommand;

/* TRUE if runscript has been completed */
static boolean         runcommanddone;

/* controls automaic command line history buffer load/save */
static boolean autohistory;

/* Q of modtrs that have been loaded with 'mgrload' */
static dlq_hdr_t       mgrloadQ;

/* temporary file control block for the program instance */
static ncxmod_temp_progcb_t  *temp_progcb;

/* Q of ncxmod_search_result_t structs representing all modules
 * and submodules found in the module library path at boot-time
 */
static dlq_hdr_t      modlibQ;

/* Q of alias_cb_t structs representing all command aliases */
static dlq_hdr_t      aliasQ;

/* flag to indicate init never completed OK; used during cleanup */
static boolean       init_done;

/*****************  C O N F I G   V A R S  ****************/

/* TRUE if OK to load aliases automatically
 * FALSE if --autoaliases=false set by user
 * when yangcli starts, this var controls
 * whether the ~/.yuma/.yangcli_aliases file will be loaded
 * into this application automatically
 */
static boolean         autoaliases;

/* set if the --aliases-file parameter is present */
static const xmlChar *aliases_file;

/* TRUE if OK to load modules automatically
 * FALSE if --autoload=false set by user
 * when server connection is made, and module discovery is done
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

/* TRUE if OK to load user vars automatically
 * FALSE if --autouservars=false set by user
 * when yangcli starts, this var controls
 * whether the ~/.yuma/.yangcli_uservars file will be loaded
 * into this application automatically
 */
static boolean         autouservars;

/* set if the --uservars=filespec parameter is set */
static const xmlChar  *uservars_file;

/* NCX_BAD_DATA_IGNORE to silently accept invalid input values
 * NCX_BAD_DATA_WARN to warn and accept invalid input values
 * NCX_BAD_DATA_CHECK to prompt user to keep or re-enter value
 * NCX_BAD_DATA_ERROR to prompt user to re-enter value
 */
static ncx_bad_data_t  baddata;

/* name of external CLI config file used on invocation */
static xmlChar        *confname;

/* the module to check first when no prefix is given and there
 * is no parent node to check;
 * usually set to module 'netconf'
 */
static xmlChar        *default_module;  

/* 0 for no timeout; N for N seconds message timeout */
static uint32          default_timeout;

/* default value for val_dump_value display mode */
static ncx_display_mode_t   display_mode;

/* FALSE to send PDUs in manager-specified order
 * TRUE to always send in correct canonical order
 */
static boolean         fixorder;

/* FALSE to skip optional nodes in do_fill
 * TRUE to check optional nodes in do_fill
 */
static boolean         optional;

/* default NETCONF test-option value */
static op_testop_t     testoption;

/* default NETCONF error-option value */
static op_errop_t      erroption;

/* default NETCONF default-operation value */
static op_defop_t      defop;

/* default NETCONF with-defaults value */
static ncx_withdefaults_t  withdefaults;

/* default indent amount */
static int32            defindent;

/* default echo-replies */
static boolean echo_replies;

/* default time-rpcs */
static boolean time_rpcs;

/* default match-names */
static ncx_name_match_t match_names;

/* default alt-names */
static boolean alt_names;

/* default force-target */
static const xmlChar *force_target;

/* default use-xmlheader */
static boolean use_xmlheader;


/********************************************************************
 * FUNCTION create_system_var
 * 
 * create a read-only system variable
 *
 * INPUTS:
 *   server_cb == server control block to use
 *   varname == variable name
 *   varval  == variable string value (may be NULL)
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    create_system_var (server_cb_t *server_cb,
                       const char *varname,
                       const char *varval)
{
    status_t  res;

    res = var_set_from_string(server_cb->runstack_context,
                              (const xmlChar *)varname,
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
 *   server_cb == server control block
 *   varname == variable name
 *   varval  == variable string value (may be NULL)
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    create_config_var (server_cb_t *server_cb,
                       const xmlChar *varname,
                       const xmlChar *varval)
{
    status_t  res;

    res = var_set_from_string(server_cb->runstack_context,
                              varname, 
                              varval,
                              VAR_TYP_CONFIG);
    return res;

} /* create_config_var */

/********************************************************************
* FUNCTION free_server_cb
* 
*  Clean and free an server control block
* 
* INPUTS:
*    server_cb == control block to free
*                MUST BE REMOVED FROM ANY Q FIRST
*
*********************************************************************/
static void
    free_server_cb (server_cb_t *server_cb)
{

    modptr_t                *modptr;
    mgr_not_msg_t           *notif;
    int                      retval;

    /* save the history buffer if needed */
    if (server_cb->cli_gl != NULL && server_cb->history_auto) {
        retval = 
            gl_save_history(server_cb->cli_gl,
                            (const char *)server_cb->history_filename,
                            "#",   /* comment prefix */
                            -1);    /* save all entries */
        if (retval) {
            log_error("\nError: could not save command line "
                      "history file '%s'",
                      server_cb->history_filename);
        }
    }

    if (server_cb->name) {
        m__free(server_cb->name);
    }
    if (server_cb->address) {
        m__free(server_cb->address);
    }
    if (server_cb->password) {
        m__free(server_cb->password);
    }
    if (server_cb->local_result) {
        val_free_value(server_cb->local_result);
    }
    if (server_cb->result_name) {
        m__free(server_cb->result_name);
    }
    if (server_cb->result_filename) {
        m__free(server_cb->result_filename);
    }
    if (server_cb->history_filename) {
        m__free(server_cb->history_filename);
    }
    if (server_cb->history_line) {
        m__free(server_cb->history_line);
    }

    if (server_cb->connect_valset) {
        val_free_value(server_cb->connect_valset);
    }


    /* cleanup the user edit buffer */
    if (server_cb->cli_gl) {
        (void)del_GetLine(server_cb->cli_gl);
    }

    var_clean_varQ(&server_cb->varbindQ);

    ncxmod_clean_search_result_queue(&server_cb->searchresultQ);

    while (!dlq_empty(&server_cb->modptrQ)) {
        modptr = (modptr_t *)dlq_deque(&server_cb->modptrQ);
        free_modptr(modptr);
    }

    while (!dlq_empty(&server_cb->notificationQ)) {
        notif = (mgr_not_msg_t *)dlq_deque(&server_cb->notificationQ);
        mgr_not_free_msg(notif);
    }

    if (server_cb->runstack_context) {
        runstack_free_context(server_cb->runstack_context);
    }

    m__free(server_cb);

}  /* free_server_cb */


/********************************************************************
* FUNCTION new_server_cb
* 
*  Malloc and init a new server control block
* 
* INPUTS:
*    name == name of server record
*
* RETURNS:
*   malloced server_cb struct or NULL of malloc failed
*********************************************************************/
static server_cb_t *
    new_server_cb (const xmlChar *name)
{
    server_cb_t  *server_cb;
    int          retval;

    server_cb = m__getObj(server_cb_t);
    if (server_cb == NULL) {
        return NULL;
    }
    memset(server_cb, 0x0, sizeof(server_cb_t));

    server_cb->runstack_context = runstack_new_context();
    if (server_cb->runstack_context == NULL) {
        m__free(server_cb);
        return NULL;
    }

    dlq_createSQue(&server_cb->varbindQ);
    dlq_createSQue(&server_cb->searchresultQ);
    dlq_createSQue(&server_cb->modptrQ);
    dlq_createSQue(&server_cb->notificationQ);
    dlq_createSQue(&server_cb->autoload_modcbQ);
    dlq_createSQue(&server_cb->autoload_devcbQ);

    /* set the default CLI history file (may not get used) */
    server_cb->history_filename = xml_strdup(YANGCLI_DEF_HISTORY_FILE);
    if (server_cb->history_filename == NULL) {
        free_server_cb(server_cb);
        return NULL;
    }
    server_cb->history_auto = autohistory;

    /* store per-session temp files */
    server_cb->temp_progcb = temp_progcb;

    /* the name is not used yet; needed when multiple
     * server profiles are needed at once instead
     * of 1 session at a time
     */
    server_cb->name = xml_strdup(name);
    if (server_cb->name == NULL) {
        free_server_cb(server_cb);
        return NULL;
    }
#if 0
    /* get a tecla CLI control block */
    server_cb->cli_gl = new_GetLine(YANGCLI_LINELEN, YANGCLI_HISTLEN);
    if (server_cb->cli_gl == NULL) {
        log_error("\nError: cannot allocate a new GL");
        free_server_cb(server_cb);
        return NULL;
    }

    /* setup CLI tab line completion */
    retval = gl_customize_completion(server_cb->cli_gl,
                                     &server_cb->completion_state,
                                     yangcli_tab_callback);
    if (retval != 0) {
        log_error("\nError: cannot set GL tab completion");
        free_server_cb(server_cb);
        return NULL;
    }

    /* setup the inactivity timeout callback function */
    retval = gl_inactivity_timeout(server_cb->cli_gl,
                                   get_line_timeout,
                                   server_cb,
                                   1,
                                   0);
    if (retval != 0) {
        log_error("\nError: cannot set GL inactivity timeout");
        free_server_cb(server_cb);
        return NULL;
    }

    /* setup the history buffer if needed */
    if (server_cb->history_auto) {
        retval = gl_load_history(server_cb->cli_gl,
                                 (const char *)server_cb->history_filename,
                                 "#");   /* comment prefix */
        if (retval) {
            log_error("\nError: cannot load command line history buffer");
            free_server_cb(server_cb);
            return NULL;
        }
    }
#endif
    /* set up lock control blocks for get-locks */
    server_cb->locks_active = FALSE;
    server_cb->locks_waiting = FALSE;
    server_cb->locks_cur_cfg = NCX_CFGID_RUNNING;
    server_cb->locks_timeout = 120;
    server_cb->locks_retry_interval = 1;
    server_cb->locks_cleanup = FALSE;
    server_cb->locks_start_time = (time_t)0;
    server_cb->lock_cb[NCX_CFGID_RUNNING].config_id = 
        NCX_CFGID_RUNNING;
    server_cb->lock_cb[NCX_CFGID_RUNNING].config_name = 
        NCX_CFG_RUNNING;
    server_cb->lock_cb[NCX_CFGID_RUNNING].lock_state = 
        LOCK_STATE_IDLE;

    server_cb->lock_cb[NCX_CFGID_CANDIDATE].config_id = 
        NCX_CFGID_CANDIDATE;
    server_cb->lock_cb[NCX_CFGID_CANDIDATE].config_name = 
        NCX_CFG_CANDIDATE;
    server_cb->lock_cb[NCX_CFGID_CANDIDATE].lock_state = 
        LOCK_STATE_IDLE;

    server_cb->lock_cb[NCX_CFGID_STARTUP].config_id = 
        NCX_CFGID_STARTUP;
    server_cb->lock_cb[NCX_CFGID_STARTUP].config_name = 
        NCX_CFG_STARTUP;
    server_cb->lock_cb[NCX_CFGID_STARTUP].lock_state = 
        LOCK_STATE_IDLE;

    /* set default server flags to current settings */
    server_cb->state = MGR_IO_ST_INIT;
    server_cb->baddata = baddata;
    server_cb->log_level = log_get_debug_level();
    server_cb->autoload = autoload;
    server_cb->fixorder = fixorder;
    server_cb->get_optional = optional;
    server_cb->testoption = testoption;
    server_cb->erroption = erroption;
    server_cb->defop = defop;
    server_cb->timeout = default_timeout;
    server_cb->display_mode = display_mode;
    server_cb->withdefaults = withdefaults;
    server_cb->history_size = YANGCLI_HISTLEN;
    server_cb->command_mode = CMD_MODE_NORMAL;
    server_cb->defindent = defindent;
    server_cb->echo_replies = echo_replies;
    server_cb->time_rpcs = time_rpcs;
    server_cb->match_names = match_names;
    server_cb->alt_names = alt_names;

    /* TBD: add user config for this knob */
    server_cb->overwrite_filevars = TRUE;

    server_cb->use_xmlheader = use_xmlheader;

    return server_cb;

}  /* new_server_cb */


/********************************************************************
* FUNCTION update_server_cb_vars
* 
*  Update the 
* 
* INPUTS:
*    server_cb == server_cb to update
*
* OUTPUTS: 
*     server_cb->foo is updated if it is a shadow of a global var
*
*********************************************************************/
static void
    update_server_cb_vars (server_cb_t *server_cb)
{
    server_cb->baddata = baddata;
    server_cb->log_level = log_get_debug_level();
    server_cb->autoload = autoload;
    server_cb->fixorder = fixorder;
    server_cb->get_optional = optional;
    server_cb->testoption = testoption;
    server_cb->erroption = erroption;
    server_cb->defop = defop;
    server_cb->timeout = default_timeout;
    server_cb->display_mode = display_mode;
    server_cb->withdefaults = withdefaults;
    server_cb->defindent = defindent;
    server_cb->echo_replies = echo_replies;
    server_cb->time_rpcs = time_rpcs;
    server_cb->match_names = match_names;
    server_cb->alt_names = alt_names;
    server_cb->use_xmlheader = use_xmlheader;

}  /* update_server_cb_vars */

/********************************************************************
 * FUNCTION load_base_schema 
 * 
 * Load the following YANG modules:
 *   yangcli
 *   yuma-netconf
 *
 * RETURNS:
 *     status
 *********************************************************************/
static status_t
    load_base_schema (void)
{
    status_t   res;

    log_debug2("\nyangcli: Loading NCX yangcli-cli Parmset");

    /* load in the server boot parameter definition file */
    res = ncxmod_load_module(YANGCLI_MOD, 
                             NULL, 
                             NULL,
                             &yangcli_mod);
    if (res != NO_ERR) {
        return res;
    }

    /* load in the NETCONF data types and RPC methods */
    res = ncxmod_load_module(NCXMOD_NETCONF, 
                             NULL, 
                             NULL,
                             &netconf_mod);
    if (res != NO_ERR) {
        return res;
    }

    /* load the netconf-state module to use
     * the <get-schema> operation 
     */
    res = ncxmod_load_module(NCXMOD_IETF_NETCONF_STATE, 
                             NULL,
                             NULL,
                             NULL);
    if (res != NO_ERR) {
        return res;
    }


    return NO_ERR;

}  /* load_base_schema */

/********************************************************************
 * FUNCTION load_core_schema 
 * 
 * Load the following YANG modules:
 *   yuma-xsd
 *   yuma-types
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
    res = ncxmod_load_module(XSDMOD, 
                             NULL, 
                             NULL,
                             NULL);
    if (res != NO_ERR) {
        return res;
    }

    /* load in the NCX data types */
    res = ncxmod_load_module(NCXDTMOD, 
                             NULL, 
                             NULL,
                             NULL);
    if (res != NO_ERR) {
        return res;
    }

    return NO_ERR;

}  /* load_core_schema */


/********************************************************************
 * FUNCTION init_system_vars
 * 
 * create the read-only system variables
 *
 * INPUTS:
 *   server_cb == server control block to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    init_system_vars (server_cb_t *server_cb)
{
    const char *envstr;
    status_t    res;

    envstr = getenv(NCXMOD_PWD);
    res = create_system_var(server_cb, NCXMOD_PWD, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = (const char *)ncxmod_get_home();
    res = create_system_var(server_cb, USER_HOME, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = getenv(ENV_HOST);
    res = create_system_var(server_cb, ENV_HOST, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = getenv(ENV_SHELL);
    res = create_system_var(server_cb, ENV_SHELL, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = getenv(ENV_USER);
    res = create_system_var(server_cb, ENV_USER, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = getenv(ENV_LANG);
    res = create_system_var(server_cb, ENV_LANG, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = getenv(NCXMOD_HOME);
    res = create_system_var(server_cb, NCXMOD_HOME, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = getenv(NCXMOD_MODPATH);
    res = create_system_var(server_cb, NCXMOD_MODPATH, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = getenv(NCXMOD_DATAPATH);
    res = create_system_var(server_cb, NCXMOD_DATAPATH, envstr);
    if (res != NO_ERR) {
        return res;
    }

    envstr = getenv(NCXMOD_RUNPATH);
    res = create_system_var(server_cb, NCXMOD_RUNPATH, envstr);
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
 * INPUTS:
 *   server_cb == server control block to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    init_config_vars (server_cb_t *server_cb)
{
    val_value_t    *parm;
    const xmlChar  *strval;
    status_t        res;
    xmlChar         numbuff[NCX_MAX_NUMLEN];

    /* $$server = ip-address */
    strval = NULL;
    parm = val_find_child(mgr_cli_valset, NULL, YANGCLI_SERVER);
    if (parm) {
        strval = VAL_STR(parm);
    }
    res = create_config_var(server_cb, YANGCLI_SERVER, strval);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ autoaliases = boolean */
    res = create_config_var(server_cb, YANGCLI_AUTOALIASES, 
                            (autoaliases) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ aliases-file = filespec */
    res = create_config_var(server_cb, YANGCLI_ALIASES_FILE, aliases_file);
    if (res != NO_ERR) {
        return res;
    }

    /* $$autocomp = boolean */
    res = create_config_var(server_cb, YANGCLI_AUTOCOMP, 
                            (autocomp) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ autohistory = boolean */
    res = create_config_var(server_cb, YANGCLI_AUTOHISTORY, 
                            (autohistory) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ autoload = boolean */
    res = create_config_var(server_cb, YANGCLI_AUTOLOAD, 
                            (autoload) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ autouservars = boolean */
    res = create_config_var(server_cb, YANGCLI_AUTOUSERVARS, 
                            (autouservars) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ uservars-file = filespec */
    res = create_config_var(server_cb, YANGCLI_USERVARS_FILE, uservars_file);
    if (res != NO_ERR) {
        return res;
    }

    /* $$baddata = enum */
    res = create_config_var(server_cb, YANGCLI_BADDATA, 
                            ncx_get_baddata_string(baddata));
    if (res != NO_ERR) {
        return res;
    }

    /* $$default-module = string */
    res = create_config_var(server_cb, YANGCLI_DEF_MODULE, default_module);
    if (res != NO_ERR) {
        return res;
    }

    /* $$display-mode = enum */
    res = create_config_var(server_cb, YANGCLI_DISPLAY_MODE, 
                            ncx_get_display_mode_str(display_mode));
    if (res != NO_ERR) {
        return res;
    }

    /* $$echo-replies = boolean */
    res = create_config_var(server_cb, YANGCLI_ECHO_REPLIES, 
                            (echo_replies) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ time-rpcs = boolean */
    res = create_config_var(server_cb, YANGCLI_TIME_RPCS, 
                            (time_rpcs) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ match-names = enum */
    res = create_config_var(server_cb, YANGCLI_MATCH_NAMES, 
                            ncx_get_name_match_string(match_names));
    if (res != NO_ERR) {
        return res;
    }

    /* $$ alt-names = boolean */
    res = create_config_var(server_cb, YANGCLI_ALT_NAMES, 
                            (alt_names) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$ use-xmlheader = boolean */
    res = create_config_var(server_cb, YANGCLI_USE_XMLHEADER, 
                            (use_xmlheader) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$user = string */
    strval = NULL;
    parm = val_find_child(mgr_cli_valset, NULL, YANGCLI_USER);
    if (parm) {
        strval = VAL_STR(parm);
    } else {
        strval = (const xmlChar *)getenv(ENV_USER);
    }
    res = create_config_var(server_cb, YANGCLI_USER, strval);
    if (res != NO_ERR) {
        return res;
    }

    /* $$test-option = enum */
    res = create_config_var(server_cb, YANGCLI_TEST_OPTION,
                            op_testop_name(testoption));
    if (res != NO_ERR) {
        return res;
    }

    /* $$error-optiona = enum */
    res = create_config_var(server_cb, YANGCLI_ERROR_OPTION,
                            op_errop_name(erroption)); 
    if (res != NO_ERR) {
        return res;
    }

    /* $$default-timeout = uint32 */
    sprintf((char *)numbuff, "%u", default_timeout);
    res = create_config_var(server_cb, YANGCLI_TIMEOUT, numbuff);
    if (res != NO_ERR) {
        return res;
    }

    /* $$indent = int32 */
    sprintf((char *)numbuff, "%d", defindent);
    res = create_config_var(server_cb, NCX_EL_INDENT, numbuff);
    if (res != NO_ERR) {
        return res;
    }

    /* $$optional = boolean */
    res = create_config_var(server_cb, YANGCLI_OPTIONAL, 
                            (server_cb->get_optional) 
                            ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$log-level = enum
     * could have changed during CLI processing; do not cache
     */
    res = create_config_var(server_cb, NCX_EL_LOGLEVEL, 
                            log_get_debug_level_string
                            (log_get_debug_level()));
    if (res != NO_ERR) {
        return res;
    }

    /* $$fixorder = boolean */
    res = create_config_var(server_cb, YANGCLI_FIXORDER, 
                            (fixorder) ? NCX_EL_TRUE : NCX_EL_FALSE);
    if (res != NO_ERR) {
        return res;
    }

    /* $$with-defaults = enum */
    res = create_config_var(server_cb, YANGCLI_WITH_DEFAULTS,
                            ncx_get_withdefaults_string(withdefaults)); 
    if (res != NO_ERR) {
        return res;
    }

    /* $$default-operation = enum */
    res = create_config_var(server_cb, NCX_EL_DEFAULT_OPERATION,
                            op_defop_name(defop)); 
    if (res != NO_ERR) {
        return res;
    }

    return NO_ERR;

} /* init_config_vars */

/********************************************************************
 * FUNCTION yangcli_notification_handler
 * 
 * matches callback template mgr_not_cbfn_t
 *
 * INPUTS:
 *   scb == session receiving RPC reply
 *   msg == notification msg that was parsed
 *   consumed == address of return consumed message flag
 *
 *  OUTPUTS:
 *     *consumed == TRUE if msg has been consumed so
 *                  it will not be freed by mgr_not_dispatch
 *               == FALSE if msg has been not consumed so
 *                  it will be freed by mgr_not_dispatch
 *********************************************************************/
static void
    yangcli_notification_handler (ses_cb_t *scb,
                                  mgr_not_msg_t *msg,
                                  boolean *consumed)
{
    server_cb_t   *server_cb;
    mgr_scb_t    *mgrcb;

#ifdef DEBUG
    if (!scb || !msg || !consumed) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    *consumed = FALSE;
    mgrcb = scb->mgrcb;
    if (mgrcb) {
        ncx_set_temp_modQ(&mgrcb->temp_modQ);
    }

    /* check the contents of the notification */
    if (msg && msg->notification) {
        if (LOGWARN) {
            gl_normal_io(server_cb->cli_gl);
            if (LOGINFO) {
                log_info("\n\nIncoming notification:");
                val_dump_value_max(msg->notification, 
                                   0,
                                   server_cb->defindent,
                                   DUMP_VAL_LOG,
                                   server_cb->display_mode,
                                   FALSE,
                                   FALSE);
                log_info("\n\n");
            } else if (msg->eventType) {
                log_warn("\n\nIncoming <%s> "
                         "notification\n\n", 
                         msg->eventType->name);
            }
        }

        /* Log the notification by removing it from the message
         * and storing it in the server notification log
         */
        dlq_enque(msg, &server_cb->notificationQ);
        *consumed = TRUE;
    }
    
}  /* yangcli_notification_handler */


extern void
    create_session (server_cb_t *server_cb);

/********************************************************************
 * FUNCTION do_connect_
 * 
 * INPUTS:
 *   server_cb == server control block to use
 *   rpc == rpc header for 'connect' command
 *   line == input text from readline call, not modified or freed here
 *   start == byte offset from 'line' where the parse RPC method
 *            left off.  This is eiother empty or contains some 
 *            parameters from the user
 *   startupmode == TRUE if starting from init and should try
 *              to connect right away if the mandatory parameters
 *              are present.
 *               == FALSE to check --optional and add parameters
 *                  if set or any missing mandatory parms
 *
 * OUTPUTS:
 *   connect_valset parms may be set 
 *   create_session may be called
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t
    do_connect_ (server_cb_t *server_cb,
                obj_template_t *rpc,
                const xmlChar *line,
                uint32 start,
                boolean startupmode)
{
    obj_template_t        *obj;
    val_value_t           *connect_valset;
    val_value_t           *valset, *testval;
    status_t               res;
    boolean                s1, s2, s3, s4, s5, tcp;

#ifdef DEBUG
    if (server_cb == NULL) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* retrieve the 'connect' RPC template, if not done already */
    if (rpc == NULL) {
        rpc = ncx_find_object(get_yangcli_mod(), YANGCLI_CONNECT);
        if (rpc == NULL) {
            server_cb->state = MGR_IO_ST_IDLE;
            log_write("\nError finding the 'connect' RPC method");
            return ERR_NCX_DEF_NOT_FOUND;
        }
    }            

    obj = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (obj == NULL) {
        server_cb->state = MGR_IO_ST_IDLE;
        log_write("\nError finding the connect RPC 'input' node");        
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    res = NO_ERR;
    tcp = FALSE;

    connect_valset = get_connect_valset();

    /* process any parameters entered on the command line */
    valset = NULL;
#if 0
    if (line != NULL) {
        while (line[start] && xml_isspace(line[start])) {
            start++;
        }
        if (line[start]) {
            valset = parse_rpc_cli(server_cb, rpc, &line[start], &res);
            if (valset == NULL || res != NO_ERR) {
                if (valset != NULL) {
                    val_free_value(valset);
                }
                log_write("\nError in the parameters for '%s' command (%s)",
                          obj_get_name(rpc), 
                          get_error_string(res));
                server_cb->state = MGR_IO_ST_IDLE;
                return res;
            }
        }
    }
#endif
    if (valset == NULL) {
        if (startupmode) {
            /* just clone the connect valset to start with */
            valset = val_clone(connect_valset);
            if (valset == NULL) {
                server_cb->state = MGR_IO_ST_IDLE;
                log_write("\nError: malloc failed");
                return ERR_INTERNAL_MEM;
            }
        } else {
            valset = val_new_value();
            if (valset == NULL) {
                log_write("\nError: malloc failed");
                server_cb->state = MGR_IO_ST_IDLE;
                return ERR_INTERNAL_MEM;
            } else {
                val_init_from_template(valset, obj);
            }
        }
    }

    /* make sure the 3 required parms are set */
    s1 = val_find_child(valset, YANGCLI_MOD, 
                        YANGCLI_SERVER) ? TRUE : FALSE;
    s2 = val_find_child(valset, YANGCLI_MOD,
                        YANGCLI_USER) ? TRUE : FALSE;
    s3 = val_find_child(valset, YANGCLI_MOD,
                        YANGCLI_PASSWORD) ? TRUE : FALSE;
    s4 = val_find_child(valset, YANGCLI_MOD,
                        YANGCLI_PUBLIC_KEY) ? TRUE : FALSE;
    s5 = val_find_child(valset, YANGCLI_MOD,
                        YANGCLI_PRIVATE_KEY) ? TRUE : FALSE;

    /* check the transport parameter */
    testval = val_find_child(valset, 
                             YANGCLI_MOD,
                             YANGCLI_TRANSPORT);
    if (testval != NULL && 
        testval->res == NO_ERR && 
        !xml_strcmp(VAL_ENUM_NAME(testval),
                    (const xmlChar *)"tcp")) {
        tcp = TRUE;
    }
#if 0    
    /* complete the connect valset if needed
     * and transfer it to the server_cb version
     *
     * try to get any missing params in valset 
     */
    if (interactive_mode()) {
        if (startupmode && s1 && s2 && ((s3 || (s4 && s5)) || tcp)) {
            if (LOGDEBUG3) {
                log_debug3("\nyangcli: CLI direct connect mode");
            }
        } else {
            res = fill_valset(server_cb, rpc, valset, connect_valset, 
                              TRUE, FALSE);
            if (res == ERR_NCX_SKIPPED) {
                res = NO_ERR;
            }
        }
    }
#endif
    /* check error or operation canceled */
    if (res != NO_ERR) {
        if (res != ERR_NCX_CANCELED) {
            log_write("\nError: Connect failed (%s)", 
                      get_error_string(res));
        }
        server_cb->state = MGR_IO_ST_IDLE;
        val_free_value(valset);
        return res;
    }

    /* passing off valset memory here */
    s1 = s2 = s3 = s4 = s5 = FALSE;
    if (valset != NULL) {
        /* save the malloced valset */
        if (server_cb->connect_valset != NULL) {
            val_free_value(server_cb->connect_valset);
        }
        server_cb->connect_valset = valset;

        /* make sure the 3 required parms are set */
        s1 = val_find_child(server_cb->connect_valset,
                            YANGCLI_MOD, 
                            YANGCLI_SERVER) ? TRUE : FALSE;
        s2 = val_find_child(server_cb->connect_valset, 
                            YANGCLI_MOD,
                            YANGCLI_USER) ? TRUE : FALSE;
        s3 = val_find_child(server_cb->connect_valset, 
                            YANGCLI_MOD,
                            YANGCLI_PASSWORD) ? TRUE : FALSE;
        s4 = val_find_child(server_cb->connect_valset,
                            YANGCLI_MOD,
                            YANGCLI_PUBLIC_KEY) ? TRUE : FALSE;
        s5 = val_find_child(server_cb->connect_valset,
                            YANGCLI_MOD,
                            YANGCLI_PRIVATE_KEY) ? TRUE : FALSE;
    }

    /* check if all params present yet */
    if (s1 && s2 && ((s3 || (s4 && s5)) || tcp)) {

        res = replace_connect_valset(server_cb->connect_valset);
        if (res != NO_ERR) {
            log_warn("\nWarning: connection parameters could not be saved");
            res = NO_ERR;
        }
        create_session(server_cb);
    } else {
        res = ERR_NCX_MISSING_PARM;
        log_write("\nError: Connect failed due to missing parameter(s)");
        server_cb->state = MGR_IO_ST_IDLE;
    }

    return res;

}  /* do_connect_ */

/********************************************************************
* FUNCTION process_cli_input
*
* Process the param line parameters against the hardwired
* parmset for the ncxmgr program
*
* INPUTS:
*    server_cb == server control block to use
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
    process_cli_input (server_cb_t *server_cb,
                       int argc,
                       char *argv[])
{
    obj_template_t        *obj;
    val_value_t           *parm;
    status_t               res;
    ncx_display_mode_t     dmode;
    boolean                defs_done;

    res = NO_ERR;
    mgr_cli_valset = NULL;
    defs_done = FALSE;

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
                val_init_from_template(mgr_cli_valset, obj);
            }
        } else {
            /* parse the command line against the object template */    
            mgr_cli_valset = cli_parse(server_cb->runstack_context,
                                       argc, 
                                       argv, 
                                       obj,
                                       FULLTEST, 
                                       PLAINMODE,
                                       autocomp,
                                       CLI_MODE_PROGRAM,
                                       &res);
	    defs_done = TRUE;
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
                           YANGCLI_MOD, 
                           YANGCLI_CONFIG);
    if (confname != NULL) {
        res = conf_parse_val_from_filespec(confname, 
                                           mgr_cli_valset,
                                           TRUE, 
                                           TRUE);
    } else {
        res = conf_parse_val_from_filespec(YANGCLI_DEF_CONF_FILE,
                                           mgr_cli_valset,
                                           TRUE, 
                                           FALSE);
    }

    if (res == NO_ERR && defs_done == FALSE) {
        res = val_add_defaults(mgr_cli_valset, NULL, NULL, FALSE);
    }

    if (res != NO_ERR) {
        return res;
    }

    /****************************************************
     * go through the yangcli params in order,
     * after setting up the logging parameters
     ****************************************************/

    /* set the logging control parameters */
    val_set_logging_parms(mgr_cli_valset);

    /* set the file search path parms */
    val_set_path_parms(mgr_cli_valset);

    /* set the warning control parameters */
    val_set_warning_parms(mgr_cli_valset);

    /* set the subdirs parm */
    val_set_subdirs_parm(mgr_cli_valset);

    /* set the protocols parm */
    res = val_set_protocols_parm(mgr_cli_valset);
    if (res != NO_ERR) {
        return res;
    }

    /* set the feature code generation parameters */
    val_set_feature_parms(mgr_cli_valset);

    /* get the server parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_SERVER);
    if (parm && parm->res == NO_ERR) {
        /* save to the connect_valset parmset */
        res = add_clone_parm(parm, connect_valset);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* get the autoaliases parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AUTOALIASES);
    if (parm && parm->res == NO_ERR) {
        autoaliases = VAL_BOOL(parm);
    } else {
        autoaliases = TRUE;
    }

    /* get the aliases-file parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_ALIASES_FILE);
    if (parm && parm->res == NO_ERR) {
        aliases_file = VAL_STR(parm);
    } else {
        aliases_file = YANGCLI_DEF_ALIASES_FILE;
    }

    /* get the autocomp parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AUTOCOMP);
    if (parm && parm->res == NO_ERR) {
        autocomp = VAL_BOOL(parm);
    } else {
        autocomp = TRUE;
    }

    /* get the autohistory parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AUTOHISTORY);
    if (parm && parm->res == NO_ERR) {
        autohistory = VAL_BOOL(parm);
    } else {
        autohistory = TRUE;
    }

    /* get the autoload parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AUTOLOAD);
    if (parm && parm->res == NO_ERR) {
        autoload = VAL_BOOL(parm);
    } else {
        autoload = TRUE;
    }

    /* get the autouservars parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AUTOUSERVARS);
    if (parm && parm->res == NO_ERR) {
        autouservars = VAL_BOOL(parm);
    } else {
        autouservars = TRUE;
    }

    /* get the baddata parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_BADDATA);
    if (parm && parm->res == NO_ERR) {
        baddata = ncx_get_baddata_enum(VAL_ENUM_NAME(parm));
        if (baddata == NCX_BAD_DATA_NONE) {
            SET_ERROR(ERR_INTERNAL_VAL);
            baddata = YANGCLI_DEF_BAD_DATA;
        }
    } else {
        baddata = YANGCLI_DEF_BAD_DATA;
    }

    /* get the batch-mode parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_BATCHMODE);
    if (parm && parm->res == NO_ERR) {
        batchmode = TRUE;
    }

    /* get the default module for unqualified module addesses */
    default_module = get_strparm(mgr_cli_valset, YANGCLI_MOD, 
                                 YANGCLI_DEF_MODULE);

    /* get the display-mode parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_DISPLAY_MODE);
    if (parm && parm->res == NO_ERR) {
        dmode = ncx_get_display_mode_enum(VAL_ENUM_NAME(parm));
        if (dmode != NCX_DISPLAY_MODE_NONE) {
            display_mode = dmode;
        } else {
            display_mode = YANGCLI_DEF_DISPLAY_MODE;
        }
    } else {
        display_mode = YANGCLI_DEF_DISPLAY_MODE;
    }

    /* get the echo-replies parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_ECHO_REPLIES);
    if (parm && parm->res == NO_ERR) {
        echo_replies = VAL_BOOL(parm);
    } else {
        echo_replies = TRUE;
    }

    /* get the time-rpcs parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_TIME_RPCS);
    if (parm && parm->res == NO_ERR) {
        time_rpcs = VAL_BOOL(parm);
    } else {
        time_rpcs = FALSE;
    }

    /* get the fixorder parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_FIXORDER);
    if (parm && parm->res == NO_ERR) {
        fixorder = VAL_BOOL(parm);
    } else {
        fixorder = YANGCLI_DEF_FIXORDER;
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

    /* get indent param */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_INDENT);
    if (parm && parm->res == NO_ERR) {
        defindent = (int32)VAL_UINT(parm);
    } else {
        defindent = NCX_DEF_INDENT;
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

    /* get the --ncport parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_NCPORT);
    if (parm && parm->res == NO_ERR) {
        /* save to the connect_valset parmset */
        res = add_clone_parm(parm, connect_valset);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* get the --private-key parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_PRIVATE_KEY);
    if (parm && parm->res == NO_ERR) {
        /* save to the connect_valset parmset */
        res = add_clone_parm(parm, connect_valset);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* get the --public-key parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_PUBLIC_KEY);
    if (parm && parm->res == NO_ERR) {
        /* save to the connect_valset parmset */
        res = add_clone_parm(parm, connect_valset);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* get the --transport parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_TRANSPORT);
    if (parm && parm->res == NO_ERR) {
        /* save to the connect_valset parmset */
        res = add_clone_parm(parm, connect_valset);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* get the --tcp-direct-enable parameter */
    parm = val_find_child(mgr_cli_valset, 
                          YANGCLI_MOD, 
                          YANGCLI_TCP_DIRECT_ENABLE);
    if (parm && parm->res == NO_ERR) {
        /* save to the connect_valset parmset */
        res = add_clone_parm(parm, connect_valset);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* get the run-script parameter */
    runscript = get_strparm(mgr_cli_valset, YANGCLI_MOD, YANGCLI_RUN_SCRIPT);

    /* get the run-command parameter */
    runcommand = get_strparm(mgr_cli_valset, YANGCLI_MOD, YANGCLI_RUN_COMMAND);

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

    /* get the match-names parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_MATCH_NAMES);
    if (parm && parm->res == NO_ERR) {
        match_names = ncx_get_name_match_enum(VAL_ENUM_NAME(parm));
        if (match_names == NCX_MATCH_NONE) {
            return ERR_NCX_INVALID_VALUE;
        }
    } else {
        match_names = NCX_MATCH_EXACT;
    }

    /* get the alt-names parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_ALT_NAMES);
    if (parm && parm->res == NO_ERR) {
        alt_names = VAL_BOOL(parm);
    } else {
        alt_names = FALSE;
    }

    /* get the force-target parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_FORCE_TARGET);
    if (parm && parm->res == NO_ERR) {
        force_target = VAL_ENUM_NAME(parm);
    } else {
        force_target = NULL;
    }

    /* get the use-xmlheader parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_USE_XMLHEADER);
    if (parm && parm->res == NO_ERR) {
        use_xmlheader = VAL_BOOL(parm);
    } else {
        use_xmlheader = TRUE;
    }

    /* get the uservars-file parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_USERVARS_FILE);
    if (parm && parm->res == NO_ERR) {
        uservars_file = VAL_STR(parm);
    } else {
        uservars_file = YANGCLI_DEF_USERVARS_FILE;
    }

    return NO_ERR;

} /* process_cli_input */

status_t yangrpc_init(char* args)
{
    int ret;
    obj_template_t       *obj;
    status_t              res;
    log_debug_t           log_level;
    yangcli_wordexp_t p;
    char* prog_w_args;
#ifdef YANGCLI_DEBUG
    int   i;
#endif
    prog_w_args = malloc(strlen("prog-placeholder ") + ((args==NULL)?0:strlen(args)) + 1);
    sprintf(prog_w_args, "prog-placeholder %s",(args==NULL)?"":args);
    ret = yangcli_wordexp(prog_w_args, &p, 0);
    free(prog_w_args);
    if(ret!=0) {
        perror(args);
        return ERR_CMDLINE_OPT_UNKNOWN;
    }

    /* set the default debug output level */
    log_level = LOG_DEBUG_INFO;

    /* init the module static vars */
    yangcli_mod = NULL;
    netconf_mod = NULL;
    mgr_cli_valset = NULL;
    batchmode = FALSE;
    helpmode = FALSE;
    helpsubmode = HELP_MODE_NONE;
    versionmode = FALSE;
    runscript = NULL;
    runscriptdone = FALSE;
    runcommand = NULL;
    runcommanddone = FALSE;
    dlq_createSQue(&mgrloadQ);
    aliases_file = YANGCLI_DEF_ALIASES_FILE;
    autoaliases = TRUE;
    autocomp = TRUE;
    autohistory = TRUE;
    autoload = TRUE;
    autouservars = TRUE;
    baddata = YANGCLI_DEF_BAD_DATA;
    connect_valset = NULL;
    confname = NULL;
    default_module = NULL;
    default_timeout = 30;
    display_mode = NCX_DISPLAY_MODE_PLAIN;
    fixorder = TRUE;
    optional = FALSE;
    testoption = YANGCLI_DEF_TEST_OPTION;
    erroption = YANGCLI_DEF_ERROR_OPTION;
    defop = YANGCLI_DEF_DEFAULT_OPERATION;
    withdefaults = YANGCLI_DEF_WITH_DEFAULTS;
    echo_replies = TRUE;
    time_rpcs = FALSE;
    uservars_file = YANGCLI_DEF_USERVARS_FILE;
    temp_progcb = NULL;
    dlq_createSQue(&modlibQ);
    dlq_createSQue(&aliasQ);

    /* set the character set LOCALE to the user default */
    setlocale(LC_CTYPE, "");

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     */
    res = ncx_init(NCX_SAVESTR, log_level, TRUE, NULL, p.we_wordc, p.we_wordv);
    if (res != NO_ERR) {
        return res;
    }

#ifdef YANGCLI_DEBUG
    if (p.we_wordc>1 && LOGDEBUG2) {
        log_debug2("\nCommand line parameters:");
        for (i=0; i<argc; i++) {
            log_debug2("\n   arg%d: %s", i, argv[i]);
        }
    }
#endif

    /* make sure the Yuma directory
     * exists for saving per session data
     */
    res = ncxmod_setup_yumadir();
    if (res != NO_ERR) {
        log_error("\nError: could not setup yuma dir '%s'",
                  ncxmod_get_yumadir());
        return res;
    }

    /* make sure the Yuma temp directory
     * exists for saving per session data
     */
    res = ncxmod_setup_tempdir();
    if (res != NO_ERR) {
        log_error("\nError: could not setup temp dir '%s/tmp'",
                  ncxmod_get_yumadir());
        return res;
    }

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

    /* set up handler for incoming notifications */
    mgr_not_set_callback_fn(yangcli_notification_handler);

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

    /* treat the connect-to-server parmset special
     * it is saved for auto-start plus restart parameters
     * Setup an empty parmset to hold the connect parameters
     */
    connect_valset = val_new_value();
    if (connect_valset==NULL) {
        return ERR_INTERNAL_MEM;
    } else {
        val_init_from_template(connect_valset, obj);
    }

    /* create the program instance temporary directory */
    temp_progcb = ncxmod_new_program_tempdir(&res);
    if (temp_progcb == NULL || res != NO_ERR) {
        return res;
    }

#if 0
    /* set the CLI handler */
    mgr_io_set_stdin_handler(yangcli_stdin_handler);
#endif


    return NO_ERR;
}

/********************************************************************
* FUNCTION namespace_mismatch_warning
*
* Issue a namespace mismatch warning
*
* INPUTS:
*  module == module name
*  revision == revision date (may be NULL)
*  namespacestr == server expected URI
*  clientns == client provided URI
*********************************************************************/
static void
    namespace_mismatch_warning (const xmlChar *module,
                                const xmlChar *revision,
                                const xmlChar *namespacestr,
                                const xmlChar *clientns)
{
    /* !!! FIXME: need a warning number for suppression */
    log_warn("\nWarning: module namespace URI mismatch:"
             "\n   module:    '%s'"
             "\n   revision:  '%s'"
             "\n   server ns: '%s'"
             "\n   client ns: '%s'",
             module,
             (revision) ? revision : EMPTY_STRING,
             namespacestr,
             clientns);

}  /* namespace_mismatch_warning */


/********************************************************************
* FUNCTION check_module_capabilities
*
* Check the modules reported by the server
* If autoload is TRUE then load any missing modules
* otherwise just warn which modules are missing
* Also check for wrong module module and version errors
*
* The server_cb->searchresultQ is filled with
* records for each module or deviation specified in
* the module capability URIs.
*
* After determining all the files that the server has,
* the <get-schema> operation is used (if :schema-retrieval
* advertised by the device and --autoload=true)
*
* All the files are copied into the session work directory
* to make sure the correct versions are used when compiling
* these files and applying features and deviations
*
* All files are compiled against the versions of the imports
* advertised in the capabilities, to make sure that imports
* without revision-date statements will still select the
* same revision as the server (INSTEAD OF ALWAYS SELECTING
* THE LATEST VERSION).
*
* If the device advertises an incomplete set of modules,
* then searches for any missing imports will be done
* using the normal search path, including YUMA_MODPATH.
*
* INPUTS:
*  server_cb == server control block to use
*  scb == session control block
*
*********************************************************************/
static void
    check_module_capabilities (server_cb_t *server_cb,
                               ses_cb_t *scb)
{
    mgr_scb_t              *mscb;
    ncx_module_t           *mod;
    cap_rec_t              *cap;
    const xmlChar          *module, *revision, *namespacestr;
    ncxmod_search_result_t *searchresult, *libresult;
    status_t                res;
    boolean                 retrieval_supported;

    mscb = (mgr_scb_t *)scb->mgrcb;

    log_info("\n\nChecking Server Modules...\n");

    if (!cap_std_set(&mscb->caplist, CAP_STDID_V1)) {
        log_warn("\nWarning: NETCONF v1 capability not found");
    }

    retrieval_supported = cap_set(&mscb->caplist,
                                  CAP_SCHEMA_RETRIEVAL);

    /* check all the YANG modules;
     * build a list of modules that
     * the server needs to get somehow
     * or proceed without them
     * save the results in the server_cb->searchresultQ
     */
    cap = cap_first_modcap(&mscb->caplist);
    while (cap) {
        mod = NULL;
        module = NULL;
        revision = NULL;
        namespacestr = NULL;
        libresult = NULL;

        cap_split_modcap(cap,
                         &module,
                         &revision,
                         &namespacestr);

        if (namespacestr == NULL) {
            /* try the entire base part of the URI if there was
             * no module capability parsed
             */
            namespacestr = cap->cap_uri;
        }

        if (namespacestr == NULL) {
            if (ncx_warning_enabled(ERR_NCX_RCV_INVALID_MODCAP)) {
                log_warn("\nWarning: skipping enterprise capability "
                         "for URI '%s'",
                         cap->cap_uri);
            }
            cap = cap_next_modcap(cap);
            continue;
        }

        if (module == NULL) {
            /* check if there is a module in the modlibQ that
             * has the same namespace URI as 'namespacestr' base
             */
            libresult = ncxmod_find_search_result(&modlibQ,
                                                  NULL,
                                                  NULL,
                                                  namespacestr);
            if (libresult != NULL) {
                module = libresult->module;
                revision = libresult->revision;
            } else {
                /* nothing found and modname is NULL, so continue */
                if (ncx_warning_enabled(ERR_NCX_RCV_INVALID_MODCAP)) {
                    log_warn("\nWarning: skipping enterprise capability "
                             "for URI '%s'",
                             cap->cap_uri);
                }
                cap = cap_next_modcap(cap);
                continue;
            }
        }

        mod = ncx_find_module(module, revision);
        if (mod != NULL) {
            /* make sure that the namespace URIs match */
            if (ncx_compare_base_uris(mod->ns, namespacestr)) {
                namespace_mismatch_warning(module,
                                           revision,
                                           namespacestr,
                                           mod->ns);
                /* force a new search or auto-load */
                mod = NULL;
            }
        }

        if (mod == NULL && module != NULL) {
            /* check if there is a module in the modlibQ that
             * has the same namespace URI as 'namespacestr' base
             */
            libresult = ncxmod_find_search_result(&modlibQ,
                                                  NULL,
                                                  NULL,
                                                  namespacestr);
            if (libresult != NULL) {
                module = libresult->module;
                revision = libresult->revision;
            }
        }

        if (mod == NULL) {
            /* module was not found in the module search path
             * of this instance of yangcli
             * try to auto-load the module if enabled
             */
            if (server_cb->autoload) {
                if (libresult) {
                    searchresult = ncxmod_clone_search_result(libresult);
                    if (searchresult == NULL) {
                        log_error("\nError: cannot load file, "
                                  "malloc failed");
                        return;
                    }
                } else {
                    searchresult = ncxmod_find_module(module, revision);
                }
                if (searchresult) {
                    searchresult->cap = cap;
                    if (searchresult->res != NO_ERR) {
                        if (LOGDEBUG2) {
                            log_debug2("\nLocal module search failed (%s)",
                                       get_error_string(searchresult->res));
                        }
                    } else if (searchresult->source) {
                        /* module with matching name, revision
                         * was found on the local system;
                         * check if the namespace also matches
                         */
                        if (searchresult->namespacestr) {
                            if (ncx_compare_base_uris
                                (searchresult->namespacestr, namespacestr)) {
                                /* cannot use this local file because
                                 * it has a different namespace
                                 */
                                namespace_mismatch_warning
                                    (module,
                                     revision,
                                     namespacestr,
                                     searchresult->namespacestr);
                            } else {
                                /* can use the local system file found */
                                searchresult->capmatch = TRUE;
                            }
                        } else {
                            /* this module found is invalid;
                             * has no namespace statement
                             */
                            log_error("\nError: found module '%s' "
                                      "revision '%s' "
                                      "has no namespace-stmt",
                                      module,
                                      (revision) ? revision : EMPTY_STRING);
                        }
                    } else {
                        /* the search result is valid, but no source;
                         * specified module is not available
                         * on the manager platform; see if the
                         * get-schema operation is available
                         */
                        if (!retrieval_supported) {
                            /* no <get-schema> so SOL, do without this module
                             * !!! FIXME: need warning number
                             */
                            if (revision != NULL) {
                                log_warn("\nWarning: module '%s' "
                                         "revision '%s' not available",
                                         module,
                                         revision);
                            } else {
                                log_warn("\nWarning: module '%s' "
                                         "(no revision) not available",
                                         module);
                            }
                        } else if (LOGDEBUG) {
                            log_debug("\nautoload: Module '%s' not available,"
                                      " will try <get-schema>",
                                      module);
                        }
                    }

                    /* save the search result no matter what */
                    dlq_enque(searchresult, &server_cb->searchresultQ);
                } else {
                    /* libresult was NULL, so there was no searchresult
                     * ncxmod did not find any YANG module with this namespace
                     * the module is not available
                     */
                    if (!retrieval_supported) {
                        /* no <get-schema> so SOL, do without this module 
                         * !!! need warning number 
                         */
                        if (revision != NULL) {
                            log_warn("\nWarning: module '%s' "
                                     "revision '%s' not available",
                                     module,
                                     revision);
                        } else {
                            log_warn("\nWarning: module '%s' "
                                     "(no revision) not available",
                                     module);
                        }
                    } else {
                        /* setup a blank searchresult so auto-load
                         * will attempt to retrieve it
                         */
                        searchresult =
                            ncxmod_new_search_result_str(module,
                                                         revision);
                        if (searchresult) {
                            searchresult->cap = cap;
                            dlq_enque(searchresult, &server_cb->searchresultQ);
                            if (LOGDEBUG) {
                                log_debug("\nyangcli_autoload: Module '%s' "
                                          "not available, will try "
                                          "<get-schema>",
                                          module);
                            }
                        } else {
                            log_error("\nError: cannot load file, "
                                      "malloc failed");
                            return;
                        }
                    }
                }
            } else {
                /* --autoload=false */
                if (LOGINFO) {
                    log_info("\nModule '%s' "
                             "revision '%s' not "
                             "loaded, autoload disabled",
                             module,
                             (revision) ? revision : EMPTY_STRING);
                }
            }
        } else {
            /* since the module was already loaded, it is
             * OK to use, even if --autoload=false
             * just copy the info into a search result record
             * so it will be copied and recompiled with the
             * correct features and deviations applied
             */
            searchresult = ncxmod_new_search_result_ex(mod);
            if (searchresult == NULL) {
                log_error("\nError: cannot load file, malloc failed");
                return;
            } else {
                searchresult->cap = cap;
                searchresult->capmatch = TRUE;
                dlq_enque(searchresult, &server_cb->searchresultQ);
            }
        }

        /* move on to the next module */
        cap = cap_next_modcap(cap);
    }

    /* get all the advertised YANG data model modules into the
     * session temp work directory that are local to the system
     */
    res = autoload_setup_tempdir(server_cb, scb);
    if (res != NO_ERR) {
        log_error("\nError: autoload setup temp files failed (%s)",
                  get_error_string(res));
    }

    /* go through all the search results (if any)
     * and see if <get-schema> is needed to pre-load
     * the session work directory YANG files
     */
    if (res == NO_ERR &&
        retrieval_supported &&
        server_cb->autoload) {

        /* compile phase will be delayed until autoload
         * get-schema operations are done
         */
        res = autoload_start_get_modules(server_cb, scb);
        if (res != NO_ERR) {
            log_error("\nError: autoload get modules failed (%s)",
                      get_error_string(res));
        }
    }

    /* check autoload state did not start or was not requested */
    if (res == NO_ERR && server_cb->command_mode != CMD_MODE_AUTOLOAD) {
        /* parse and hold the modules with the correct deviations,
         * features and revisions.  The returned modules
         * from yang_parse.c will not be stored in the local module
         * directory -- just used for this one session then deleted
         */
        res = autoload_compile_modules(server_cb, scb);
        if (res != NO_ERR) {
            log_error("\nError: autoload compile modules failed (%s)",
                      get_error_string(res));
        }
    }

} /* check_module_capabilities */

status_t yangrpc_connect(char* server, uint16_t port, char* user, char* password, char* public_key, char* private_key, char* extra_args, yangrpc_cb_ptr_t* yangrpc_cb_ptr)
{
    char* server_arg;
    char* port_arg;
    char* user_arg;
    char* password_arg;
    char* public_key_arg;
    char* private_key_arg;
    char* mandatory_argv[]={"exec-name-dummy", "--server=?", "--port=?", "--user=?", "--password=?", "--private-key=?", "--public-key=?"};
    int mandatory_argc=sizeof(mandatory_argv)/sizeof(char*);
    char** argv;
    int argc;
    server_cb_t          *server_cb;
    ses_cb_t             *ses_cb;
    status_t res;
    xmlChar versionbuffer[NCX_VERSION_BUFFSIZE];
    val_value_t          *parm, *modval;
    dlq_hdr_t             savedevQ;
    xmlChar              *savestr, *revision;
    uint32                modlen;
    int                   ret;
    yangcli_wordexp_t     p;

    if(extra_args!=NULL) {
        ret = yangcli_wordexp(extra_args, &p, 0);
        if(ret!=0) {
            perror(extra_args);
            return ERR_CMDLINE_OPT_UNKNOWN;
        }
        argc = mandatory_argc+p.we_wordc;
        argv = malloc(argc*sizeof(char*));
        memcpy(argv,mandatory_argv,mandatory_argc*sizeof(char*));
        memcpy(argv+mandatory_argc,p.we_wordv,p.we_wordc*sizeof(char*));
        yangcli_wordfree(&p);
    } else {
        argc = mandatory_argc;
        argv = malloc(argc*sizeof(char*));
        memcpy(argv,mandatory_argv,argc*sizeof(char*));
    }

    dlq_createSQue(&savedevQ);

    /* create a default server control block */
    server_cb = new_server_cb(YANGCLI_DEF_SERVER);
    if (server_cb==NULL) {
        return ERR_INTERNAL_PTR;
    }

    argc=1;
    server_arg = malloc(strlen("--server=")+strlen(server)+1);
    assert(server_arg!=NULL);
    sprintf(server_arg,"--server=%s",server);
    argv[argc++]=server_arg;

    port_arg = malloc(strlen("--ncport=")+strlen("65535")+1);
    assert(port_arg!=NULL);
    sprintf(port_arg,"--ncport=%u", (unsigned int)port);
    argv[argc++]=port_arg;

    user_arg = malloc(strlen("--user=")+strlen(user)+1);
    assert(user_arg!=NULL);
    sprintf(user_arg,"--user=%s",user);
    argv[argc++]=user_arg;

    /* at least password or public_key and private_key pair has to be specified */
    assert(password!=NULL || (public_key!=NULL && private_key!=NULL));

    if(password!=NULL) {
        password_arg = malloc(strlen("--password=")+strlen(password)+1);
        assert(password_arg!=NULL);
        sprintf(password_arg,"--password=%s",password);
        argv[argc++]=password_arg;
    }

    if(public_key!=NULL && private_key!=NULL) {
        public_key_arg = malloc(strlen("--public-key=")+strlen(public_key)+1);
        assert(public_key_arg!=NULL);
        sprintf(public_key_arg,"--public-key=%s",public_key);
        argv[argc++]=public_key_arg;

        private_key_arg = malloc(strlen("--private-key=")+strlen(private_key)+1);
        assert(private_key_arg!=NULL);
        sprintf(private_key_arg,"--private-key=%s",private_key);
        argv[argc++]=private_key_arg;
    }

    /* Get any command line and conf file parameters */
    res = process_cli_input(server_cb, argc, argv);
    if (res != NO_ERR) {
        return res;
    }

    /* check print version */
    if (versionmode || helpmode) {
        res = ncx_get_version(versionbuffer, NCX_VERSION_BUFFSIZE);
        if (res == NO_ERR) {
            log_stdout("\nyangcli version %s\n", versionbuffer);
        } else {
            return res;
        }
    }

    /* check print help and exit */
    if (helpmode) {
        help_program_module(YANGCLI_MOD, YANGCLI_BOOT, helpsubmode);
    }

    /* check quick exit */
    if (helpmode || versionmode) {
        return ERR_NCX_SKIPPED;
    }

    /* set any server control block defaults which were supplied
     * in the CLI or conf file
     */
    update_server_cb_vars(server_cb);

    /* Load the NETCONF, XSD, SMI and other core modules */
    if (autoload) {
        res = load_core_schema();
        if (res != NO_ERR) {
            return res;
        }
    }

    /* check if there are any deviation parameters to load first */
    for (modval = val_find_child(mgr_cli_valset, YANGCLI_MOD,
                                 NCX_EL_DEVIATION);
         modval != NULL && res == NO_ERR;
         modval = val_find_next_child(mgr_cli_valset, YANGCLI_MOD,
                                      NCX_EL_DEVIATION,
                                      modval)) {

        res = ncxmod_load_deviation(VAL_STR(modval), &savedevQ);
        if (res != NO_ERR) {
            log_error("\n load deviation failed (%s)", 
                      get_error_string(res));
        } else {
            log_info("\n load OK");
        }
    }

    if (res == NO_ERR) {
        /* check if any explicitly listed modules should be loaded */
        modval = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_MODULE);
        while (modval != NULL && res == NO_ERR) {
            log_info("\nyangcli: Loading requested module %s", 
                     VAL_STR(modval));

            revision = NULL;
            savestr = NULL;
            modlen = 0;

            if (yang_split_filename(VAL_STR(modval), &modlen)) {
                savestr = &(VAL_STR(modval)[modlen]);
                *savestr = '\0';
                revision = savestr + 1;
            }

            res = ncxmod_load_module(VAL_STR(modval), revision,
                                     &savedevQ, NULL);
            if (res != NO_ERR) {
                log_error("\n load module failed (%s)", 
                          get_error_string(res));
            } else {
                log_info("\n load OK");
            }

            modval = val_find_next_child(mgr_cli_valset, YANGCLI_MOD,
                                         NCX_EL_MODULE, modval);
        }
    }

    /* discard any deviations loaded from the CLI or conf file */
    ncx_clean_save_deviationsQ(&savedevQ);
    if (res != NO_ERR) {
        return res;
    }

    /* load the system (read-only) variables */
    res = init_system_vars(server_cb);
    if (res != NO_ERR) {
        return res;
    }

    /* load the system config variables */
    res = init_config_vars(server_cb);
    if (res != NO_ERR) {
        return res;
    }

    /* initialize the module library search result queue */
    {
        log_debug_t dbglevel = log_get_debug_level();
        if (LOGDEBUG3) {
            ; 
        } else {
            log_set_debug_level(LOG_DEBUG_NONE);
        }
        res = ncxmod_find_all_modules(&modlibQ);
        log_set_debug_level(dbglevel);
        if (res != NO_ERR) {
            return res;
        }
    }
#if 0
    /* load the user aliases */
    if (autoaliases) {
        res = load_aliases(aliases_file);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* load the user variables */
    if (autouservars) {
        res = load_uservars(server_cb, uservars_file);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* make sure the startup screen is generated
     * before the auto-connect sequence starts
     */
    do_startup_screen();    
#endif

    /* check to see if a session should be auto-started
     * --> if the server parameter is set a connect will
     * --> be attempted
     *
     * The yangcli_stdin_handler will call the finish_start_session
     * function when the user enters a line of keyboard text
     */
    server_cb->state = MGR_IO_ST_IDLE;

    if (connect_valset) {
        parm = val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_SERVER);
        if (parm && parm->res == NO_ERR) {
            res = do_connect_(server_cb, NULL, NULL, 0, TRUE);
            if (res != NO_ERR) {
                if (!batchmode) {
                    res = NO_ERR;
                }
            }
        }
    }

    /* the request will be stored if this returns NO_ERR */
    //res = mgr_rpc_send_request(scb, req, yangcli_reply_handler_);
    //assert(res==NO_ERR);
    //mgr_io_run();

    {
        ses_cb_t* scb;
        mgr_scb_t* mscb;

        scb = mgr_ses_get_scb(server_cb->mysid);
        assert(scb!=NULL);

        res = ses_msg_send_buffs(scb);
        assert(res==NO_ERR);
        while(1) {
            res = ses_accept_input(scb);
            if(res!=NO_ERR) {
                return res;
            }
            if(mgr_ses_process_first_ready()) {
                break;
            }
        }
        /* incoming hello OK and outgoing hello is sent */
        server_cb->state = MGR_IO_ST_CONN_IDLE;
        report_capabilities(server_cb, scb, TRUE, HELP_MODE_NONE);
        check_module_capabilities(server_cb, scb);
        mscb = (mgr_scb_t *)scb->mgrcb;
        ncx_set_temp_modQ(&mscb->temp_modQ);

    }

    *yangrpc_cb_ptr = (yangrpc_cb_ptr_t)server_cb;
    return NO_ERR;
}

val_value_t* global_reply_val;

/********************************************************************
 * FUNCTION yangcli_reply_handler_
 * 
 *  handle incoming <rpc-reply> messages
 * 
 * INPUTS:
 *   scb == session receiving RPC reply
 *   req == original request returned for freeing or reusing
 *   rpy == reply received from the server (for checking then freeing)
 *
 * RETURNS:
 *   none
 *********************************************************************/
void
    yangcli_reply_handler_ (ses_cb_t *scb,
                           mgr_rpc_req_t *req,
                           mgr_rpc_rpy_t *rpy)
{
    server_cb_t   *server_cb;
    val_value_t  *val;
    mgr_scb_t    *mgrcb;
    lock_cb_t    *lockcb;
    rpc_err_t     rpcerrtyp;
    status_t      res;
    boolean       anyout, anyerrors, done;
    uint32        usesid;

#ifdef DEBUG
    if (!scb || !req || !rpy) {
        assert(0);
    }
#endif


    /* check the contents of the reply */
    if (rpy && rpy->reply) {
#if 0
        if (val_find_child(rpy->reply, 
                           NC_MODULE,
                           NCX_EL_RPC_ERROR)) {
            if (server_cb->command_mode == CMD_MODE_NORMAL || LOGDEBUG) {
                log_error("\nRPC Error Reply %s for session %u:\n",
                          rpy->msg_id, 
                          usesid);
                val_dump_value_max(rpy->reply, 
                                   0,
                                   server_cb->defindent,
                                   DUMP_VAL_LOG,
                                   server_cb->display_mode,
                                   FALSE,
                                   FALSE);
                log_error("\n");
                anyout = TRUE;
            }
        } else if (val_find_child(rpy->reply, NC_MODULE, NCX_EL_OK)) {
            global_reply_val = val_clone(rpy->reply);
            if (server_cb->command_mode == CMD_MODE_NORMAL || LOGDEBUG2) {
                if (server_cb->echo_replies) {
                    log_info("\nRPC OK Reply %s for session %u:\n",
                             rpy->msg_id, 
                             usesid);
                    anyout = TRUE;
                }
            }
        }
#else
        //val_dump_value(rpy->reply,0);
        global_reply_val = val_clone(rpy->reply);
        assert(global_reply_val!=NULL);
#endif
    }

    /* free the request and reply */
    mgr_rpc_free_request(req);
    if (rpy) {
        mgr_rpc_free_reply(rpy);
    }

}  /* yangcli_reply_handler_ */

#include "yangcli_cmd.h"
status_t yangrpc_parse_cli(yangrpc_cb_ptr_t yangrpc_cb_ptr, char* original_line, val_value_t** request_val)
{


    obj_template_t        *rpc, *input;
    val_value_t           *reqdata = NULL, *valset = NULL, *parm;
    xmlChar               *newline = NULL, *useline = NULL;
    uint32                 len, linelen;
    status_t               res = NO_ERR;
    boolean                shut = FALSE;
    ncx_node_t             dtyp;
    char* line;

    server_cb_t* server_cb;
    server_cb = (server_cb_t*)yangrpc_cb_ptr;

    line = strdup(original_line);
#ifdef DEBUG
    if (!server_cb || !line) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* make sure there is something to parse */
    linelen = xml_strlen(line);
    if (!linelen) {
        return res;
    }

#if 0
    /* first check the command keyword to see if it is an alias */
    newline = expand_alias(line, &res);
    if (res == ERR_NCX_SKIPPED) {
        res = NO_ERR;
        useline = line;
    } else if (res == NO_ERR) {
        if (newline == NULL) {
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        useline = newline;
        linelen = xml_strlen(newline);
    } else {
        log_error("\nError: %s\n", get_error_string(res));
        if (newline) {
            m__free(newline);
        }
        return res;
    }
#else
    useline = line;
#endif
    /* get the RPC method template */
    dtyp = NCX_NT_OBJ;
    rpc = (obj_template_t *)parse_def(server_cb, &dtyp, useline, &len, &res);
    if (rpc == NULL || !obj_is_rpc(rpc)) {
        if (server_cb->result_name || server_cb->result_filename) {
            res = finish_result_assign(server_cb, NULL, useline);
        } else {
            if (res == ERR_NCX_DEF_NOT_FOUND) {
                /* this is an unknown command */
                log_error("\nError: Unrecognized command");
            } else if (res == ERR_NCX_AMBIGUOUS_CMD) {
                log_error("\n");
            } else {
                log_error("\nError: %s", get_error_string(res));
            }
        }
        if (newline) {
            m__free(newline);
        }
        return res;
    }

    /* check local commands */
    if (is_yangcli_ns(obj_get_nsid(rpc))) {
        if (!xml_strcmp(obj_get_name(rpc), YANGCLI_CONNECT)) {
            res = ERR_NCX_OPERATION_FAILED;
            log_stdout("\nError: Already connected");
        } else {
            uint32 timeval;
            res = do_local_conn_command_reqdata(server_cb, rpc, useline, len, &reqdata, &timeval);
            if (res == ERR_NCX_SKIPPED) {
                assert(0);
		/*res = do_local_command(server_cb, rpc, useline, len);*/
            }
        }
        if (newline) {
            m__free(newline);
        }
    } else {

        /* else treat this as an RPC request going to the server
         * make sure this is a TRUE conditional command
         */

        /* construct a method + parameter tree */
        reqdata = xml_val_new_struct(obj_get_name(rpc), obj_get_nsid(rpc));
        if (!reqdata) {
            log_error("\nError allocating a new RPC request");
            res = ERR_INTERNAL_MEM;
            input = NULL;
        } else {
            /* should find an input node */
            input = obj_find_child(rpc, NULL, YANG_K_INPUT);
        }

        /* check if any params are expected */
        if (res == NO_ERR && input) {
            while (useline[len] && xml_isspace(useline[len])) {
                len++;
            }

            if (len < linelen) {
                valset = parse_rpc_cli(server_cb, rpc, &useline[len], &res);
                if (res != NO_ERR) {
                    log_error("\nError in the parameters for '%s' command (%s)",
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
        }

#if 0
        /* fill in any missing parameters from the CLI */
        if (res == NO_ERR) {
            if (interactive_mode()) {
                res = fill_valset(server_cb, rpc, valset, NULL, TRUE, FALSE);
                if (res == ERR_NCX_SKIPPED) {
                    res = NO_ERR;
                }
            }
        }
#endif

        /* make sure the values are in canonical order
         * so compliant some servers will not complain
         */
        val_set_canonical_order(valset);

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
    *request_val = reqdata;
    return res;
}

status_t yangrpc_exec(yangrpc_cb_ptr_t yangrpc_cb_ptr, val_value_t* request_val, val_value_t** reply_val)
{
    status_t res;
    ses_cb_t* scb;
    mgr_rpc_req_t         *req;
    server_cb_t* server_cb;
    server_cb = (server_cb_t*)yangrpc_cb_ptr;

    scb = mgr_ses_get_scb(server_cb->mysid);
    if (!scb) {
        res = SET_ERROR(ERR_INTERNAL_PTR);
        return res;
    }
    req = mgr_rpc_new_request(scb);
    if (!req) {
        res = ERR_INTERNAL_MEM;
        log_error("\nError allocating a new RPC request");
        return res;
    }
    req->data = val_clone(request_val);/*reqdata*/
    req->rpc = request_val->obj;
    req->timeout = 1000/*timeoutval*/;

    /* the request will be stored if this returns NO_ERR */
    global_reply_val=NULL;
    res = mgr_rpc_send_request(scb, req, yangcli_reply_handler_);

    //mgr_io_run();
    res = ses_msg_send_buffs(scb);
    assert(res==NO_ERR);
    while(1) {
    	
        res = ses_accept_input(scb);
        if(res!=NO_ERR) {
            printf("error: ses_accept_input res=%d\n",res);
            assert(0);
        }
        if(mgr_ses_process_first_ready() && global_reply_val!=NULL) {
            break;
        }
    }
    *reply_val = global_reply_val;

    return NO_ERR;
}

void yangrpc_close(yangrpc_cb_ptr_t yangrpc_cb_ptr)
{
}
