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

/*
#include <xmlstring.h>

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif
*/

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/
#define YANGCLI_PROGVER    (const xmlChar *)"0.9.4"

#define PROGNAME            "yangcli"

#define MAX_PROMPT_LEN 56

#define YANGCLI_MAX_NEST  16

#define YANGCLI_MAX_RUNPARMS 9

#define YANGCLI_LINELEN   4095

#define YANGCLI_BUFFLEN  32000

#define YANGCLI_HISTLEN  64000

#define YANGCLI_DEF_TIMEOUT   30

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

#define DEF_OPTIONS (const xmlChar *)" ? (help), \?\? \
(full help), \?s (skip) , ?c (cancel)"

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
*								    *
*			     T Y P E S				    *
*								    *
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


extern boolean
    get_autocomp (void);

#endif	    /* _H_yangcli */
