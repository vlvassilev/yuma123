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
#include <time.h>
#include <sys/time.h>

/* #define MEMORY_DEBUG 1 */

#ifdef MEMORY_DEBUG
#include <mcheck.h>
#endif

#include "libtecla.h"

#define _C_main 1

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
#include "status.h"
#include "val.h"
#include "val_util.h"
#include "var.h"
#include "xml_util.h"
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

/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/

/***************** GLOBAL VARS ****************/

/* yangcli.yang module */
ncx_module_t  *yangcli_mod;

/* yangcli-ex.yang module */
ncx_module_t  *yangcli_ex_mod;

/* netconf.yang module */
ncx_module_t  *netconf_mod;

#if 0
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
#endif

/* global connect param set, copied to server connect parmsets */
val_value_t   *connect_valset;

#if 0
/* name of external CLI config file used on invocation */
static xmlChar        *confname;

/* the module to check first when no prefix is given and there
 * is no parent node to check;
 * usually set to module 'netconf'
 */
static xmlChar        *default_module;  

/* 0 for no timeout; N for N seconds message timeout */
static uint32          default_timeout;

/* TRUE if OK to keep model copies stored in $USER/.yuma/tmp/<>/<sid> until yangcli is terminated.
 * FALSE if --keep-session-model-copies-after-compilation=false
 */
static boolean         keep_session_model_copies_after_compilation;

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
#endif
/* END yangcli_globals.c */
