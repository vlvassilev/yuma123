#ifndef _H_yangcli
#define _H_yangcli

/*  FILE: yangcli.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
27-mar-09    abb      Begun; moved from yangcli.c

*/


#include <xmlstring.h>

#include "libtecla.h"

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_mgr_io
#include "mgr_io.h"
#endif

#ifndef _H_mgr_rpc
#include "mgr_rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/
#define YANGCLI_PROGVER    (const xmlChar *)"0.9.5"

#define PROGNAME            "yangcli"

#define MAX_PROMPT_LEN 56

#define YANGCLI_MAX_NEST  16

#define YANGCLI_MAX_RUNPARMS 9

#define YANGCLI_LINELEN   4095

/* 8K CLI buffer per agent session */
#define YANGCLI_BUFFLEN  8192

#define YANGCLI_HISTLEN  4095

#define YANGCLI_DEF_HISTORY_FILE  (const xmlChar *)"~/.yangcli_history"

#define YANGCLI_DEF_TIMEOUT   30

#define YANGCLI_DEF_AGENT (const xmlChar *)"default"

#define YANGCLI_DEF_DISPLAY_MODE   NCX_DISPLAY_MODE_PLAIN

#define YANGCLI_DEF_FIXORDER   TRUE

#define YANGCLI_MOD  (const xmlChar *)"yangcli"

#ifdef MACOSX
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

/* YANGCLI boot and operation parameter names 
 * matches parm clauses in yangcli container in yangcli.yang
 */

#define YANGCLI_AGENT       (const xmlChar *)"agent"
#define YANGCLI_AUTOCOMP    (const xmlChar *)"autocomp"
#define YANGCLI_AUTOLOAD    (const xmlChar *)"autoload"
#define YANGCLI_BADDATA     (const xmlChar *)"baddata"
#define YANGCLI_BATCHMODE   (const xmlChar *)"batch-mode"
#define YANGCLI_BRIEF       (const xmlChar *)"brief"
#define YANGCLI_CLEAR       (const xmlChar *)"clear"
#define YANGCLI_COMMAND     (const xmlChar *)"command"
#define YANGCLI_COMMANDS    (const xmlChar *)"commands"
#define YANGCLI_CONFIG      (const xmlChar *)"config"
#define YANGCLI_DEF_MODULE  (const xmlChar *)"default-module"
#define YANGCLI_DIR         (const xmlChar *)"dir"
#define YANGCLI_DISPLAY_MODE  (const xmlChar *)"display-mode"
#define YANGCLI_EDIT_TARGET (const xmlChar *)"edit-target"
#define YANGCLI_ERROR_OPTION (const xmlChar *)"error-option"
#define YANGCLI_FIXORDER    (const xmlChar *)"fixorder"
#define YANGCLI_FROM_CLI    (const xmlChar *)"from-cli"
#define YANGCLI_FULL        (const xmlChar *)"full"
#define YANGCLI_GLOBAL      (const xmlChar *)"global"
#define YANGCLI_GLOBALS     (const xmlChar *)"globals"
#define YANGCLI_OIDS        (const xmlChar *)"oids"
#define YANGCLI_LOAD        (const xmlChar *)"load"
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
#define YANGCLI_RECALL      (const xmlChar *)"recall"
#define YANGCLI_RUN_SCRIPT  (const xmlChar *)"run-script"
#define YANGCLI_TEST_OPTION (const xmlChar *)"test-option"
#define YANGCLI_TIMEOUT     (const xmlChar *)"timeout"
#define YANGCLI_USER        (const xmlChar *)"user"
#define YANGCLI_VALUE       (const xmlChar *)"value"
#define YANGCLI_VAR         (const xmlChar *)"var"
#define YANGCLI_VARREF      (const xmlChar *)"varref"
#define YANGCLI_VARS        (const xmlChar *)"vars"
#define YANGCLI_WITH_DEFAULTS  (const xmlChar *)"with-defaults"

#define BAD_DATA_DEFAULT NCX_BAD_DATA_CHECK

/* YANGCLI local RPC commands */
#define YANGCLI_CD      (const xmlChar *)"cd"
#define YANGCLI_CONNECT (const xmlChar *)"connect"
#define YANGCLI_CREATE  (const xmlChar *)"create"
#define YANGCLI_DELETE  (const xmlChar *)"delete"
#define YANGCLI_FILL    (const xmlChar *)"fill"
#define YANGCLI_HELP    (const xmlChar *)"help"
#define YANGCLI_HISTORY (const xmlChar *)"history"
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


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* cache the module pointers known by a particular agent,
 * as reported in the session <hello> message
 */
typedef struct modptr_t_ {
    dlq_hdr_t            qhdr;
    ncx_module_t         *mod;               /* back-ptr, not live */
    ncx_list_t           *feature_list;      /* back-ptr, not live */
    ncx_list_t           *deviation_list;    /* back-ptr, not live */
} modptr_t;


/* save the requested result format type */
typedef enum result_format_t {
    RF_NONE,
    RF_TEXT,
    RF_XML
} result_format_t;


/* command state enumerations for each situation
 * where the tecla get_line function is called
 */
typedef enum command_state_t {
    CMD_STATE_NONE,
    CMD_STATE_FULL,
    CMD_STATE_GETVAL,
    CMD_STATE_YESNO,
    CMD_STATE_MORE
} command_state_t;


/* saved state for libtecla command line completion */
typedef struct completion_state_t_ {
    const obj_template_t  *cmdobj;
    const obj_template_t  *cmdinput;
    const obj_template_t  *cmdcurparm;
    struct agent_cb_t_    *agent_cb;
    ncx_module_t          *cmdmodule;
    command_state_t        cmdstate;
    boolean                assignstmt;
} completion_state_t;


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
    var_type_t           result_vartype;
    xmlChar             *result_filename;
    result_format_t      result_format;

    /* per-agent shadows of global config vars */
    boolean              get_optional;
    ncx_display_mode_t   display_mode;
    uint32               timeout;
    ncx_bad_data_t       baddata;
    log_debug_t          log_level;
    boolean              autoload;
    boolean              fixorder;
    op_testop_t          testoption;
    op_errop_t           erroption;
    ncx_withdefaults_t   withdefaults;

    /* session support */
    mgr_io_state_t       state;
    ses_id_t             mysid;
    mgr_io_returncode_t  returncode;
    int32                errnocode;

    /* TBD: session-specific user variables */
    dlq_hdr_t            varbindQ;   /* Q of ncx_var_t */

    /* contains only the modules that the agent is using
     * plus the 'netconf.yang' module
     */
    dlq_hdr_t            modptrQ;     /* Q of modptr_t */

    /* per-session CLI support */
    const xmlChar       *cli_fn;
    GetLine             *cli_gl;
    xmlChar             *history_filename;
    xmlChar             *history_line;
    boolean              history_line_active;
    uint32               history_size;
    completion_state_t   completion_state;
    boolean              climore;
    xmlChar              clibuff[YANGCLI_BUFFLEN];
} agent_cb_t;


/* logging function template to switch between
 * log_stdout and log_write
 */
typedef void (*logfn_t) (const char *fstr, ...);


extern boolean
    get_autocomp (void);

extern boolean
    get_autoload (void);

extern boolean
    get_batchmode (void);

extern const xmlChar *
    get_default_module (void);

extern const xmlChar *
    get_runscript (void);

extern ncx_bad_data_t
    get_baddata (void);

extern ncx_module_t *
    get_netconf_mod (void);

extern ncx_module_t *
    get_yangcli_mod (void);

extern val_value_t *
    get_mgr_cli_valset (void);

extern val_value_t *
    get_connect_valset (void);

extern dlq_hdr_t *
    get_mgrloadQ (void);

/* forward decl needed by send_copy_config_to_agent function */
extern void
    yangcli_reply_handler (ses_cb_t *scb,
			   mgr_rpc_req_t *req,
			   mgr_rpc_rpy_t *rpy);

extern status_t
    finish_result_assign (agent_cb_t *agent_cb,
			  val_value_t *resultvar,
			  const xmlChar *resultstr);

#endif	    /* _H_yangcli */
