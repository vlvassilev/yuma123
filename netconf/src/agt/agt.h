/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_agt
#define _H_agt

/*  FILE: agt.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent message handler

     - NETCONF PDUs

        - PDU instance document parsing

        - Object instance syntax validation

        - Basic object-level referential integrity

        - Value instance document parsing 

        - Value instance document syntax validation

        - Value instance count validation

        - Value instance default value insertion


    The netconfd agent <edit-config> handler registers callbacks
    with the agt_val module that can be called IN ADDITION
    to the automated callback, for referential integrity checking,
    resource reservation, etc.

    NCX Write Model
    ---------------

    netconfd uses a 3 pass callback model to process 
    <edit-config> PDUs.
    
  Pass 1:  Validation : AGT_CB_VALIDATE

     The target database is not touched during this phase.

       - XML parsing
       - data type validation
       - pattern and regular expression matching
       - dynamic instance validation
       - instance count validation
       - choice and choice group validation
       - insertion by default validation
       - nested 'operation' attribute validation

    If any errors occur within a value node (within the 
    /config/ container) then that entire value node
    is considered unusable and it is not applied.

    If continue-on-error is requested, then all sibling nodes
    will be processed if the validation tests all pass.

  Pass 2: Apply : AGT_CB_APPLY

    On a per-node granularity, an operation is processed only
    if no errors occurred during the validation phase.

    The target database is modified.  User callbacks should
    not emit external PDUs (e.g., BGP changes) until the COMMIT
    phase.  The automated PDU processing will make non-destructive
    edits during this phase, even if the 'rollback-on-error'
    error-option is not requested (in order to simplify the code).

    If an error-option other than continue-on-error is requested,
    then any error returned during this phase will cause further
    'apply' phase processing to be terminated.

    If error-option is 'stop-on-error' then phase 3 is not executed
    if this phase terminates with an error.

  Pass 3-OK: Commit (Positive Outcome) :  AGT_CB_COMMIT
       
    The database edits are automatically completed during this phase.
    The user callbacks must not edit the database -- only modify
    their own data structures, send PDUs, etc.

  Pass 3-ERR: Rollback (Negative Outcome) : AGT_CB_ROLLBACK

    If rollback-on-error is requested, then this phase will be
    executed for only the application nodes (and sibling parmsets 
    within the errored application) that have already executed
    the 'apply' phase.

    

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
04-nov-05    abb      Begun
02-aug-08    abb      Convert from NCX typdef to YANG object design
03-oct-08    abb      Convert to YANG only, remove app, parmset, 
                      and parm layers in the NETCONF database
*/

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

#define AGT_NUM_CB   (AGT_CB_ROLLBACK+1)

/* ncxserver magic cookie hack */
#define NCX_SERVER_MAGIC "x56o8ab17eg92z343i55a0a964a864aOal1"

#define AGT_MAX_PORTS  4

#define AGT_DEF_CONF_FILE (const xmlChar *)"/etc/yuma/netconfd.conf"


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* matches access-control enumeration in netconfd.yang */
typedef enum agt_acmode_t_ {
    AGT_ACMOD_NONE,
    AGT_ACMOD_ENFORCING,
    AGT_ACMOD_PERMISSIVE,
    AGT_ACMOD_DISABLED,
    AGT_ACMOD_OFF
} agt_acmode_t;


/* enumeration of the different agent callback types 
 * These are used are array indices so there is no dummy zero enum
 */
typedef enum agt_cbtyp_t_ {
    AGT_CB_VALIDATE,               /* P1: write operation validate */
    AGT_CB_APPLY,                     /* P2: write operation apply */
    AGT_CB_COMMIT,               /* P3-pos: write operation commit */
    AGT_CB_ROLLBACK,           /* P3-neg: write operation rollback */
    AGT_CB_TEST_APPLY          /* P1a-int: config data only, no CB */
} agt_cbtyp_t;


/* hardwire some of the agent profile parameters
 * because they are needed before the NCX engine is running
 * They cannot be changed after boot-time.
 */
typedef struct agt_profile_t_ {
    ncx_agttarg_t       agt_targ;
    ncx_agtstart_t      agt_start;
    log_debug_t         agt_loglevel;
    boolean             agt_has_startup;
    boolean             agt_usestartup;   /* --no-startup flag */
    boolean             agt_startup_error;  /* T: stop, F: continue */
    boolean             agt_logappend;
    boolean             agt_xmlorder;
    boolean             agt_deleteall_ok;   /* TBD: not implemented */
    const xmlChar      *agt_accesscontrol;
    const xmlChar      *agt_conffile;
    const xmlChar      *agt_logfile;
    const xmlChar      *agt_startup;
    const xmlChar      *agt_defaultStyle;
    const xmlChar      *agt_superuser;
    uint32              agt_eventlog_size;
    uint32              agt_maxburst;
    uint32              agt_hello_timeout;
    uint32              agt_idle_timeout;
    uint32              agt_linesize;
    int32               agt_indent;
    boolean             agt_usevalidate;
    ncx_withdefaults_t  agt_defaultStyleEnum;
    agt_acmode_t        agt_accesscontrol_enum;
    uint16              agt_ports[AGT_MAX_PORTS];

    /* Q of malloced ncx_save_deviations_t */
    dlq_hdr_t           agt_savedevQ;  
} agt_profile_t;


/* SIL init function */
typedef status_t (*agt_sil_init_fn_t)(const xmlChar *modname,
                                      const xmlChar *revision);

/* SIL init2 function */
typedef status_t (*agt_sil_init2_fn_t)(void);

/* SIL cleanup function */
typedef void (*agt_sil_cleanup_fn_t)(void);

/* struct to keep track of the dynamic libraries
 * opened by the 'load' command
 */
typedef struct agt_dynlib_cb_t_ {
    dlq_hdr_t             qhdr;
    void                 *handle;
    xmlChar              *modname;
    xmlChar              *revision;
    ncx_module_t         *mod;
    agt_sil_init_fn_t     initfn;
    agt_sil_init2_fn_t    init2fn;
    agt_sil_cleanup_fn_t  cleanupfn;
    status_t              init_status;
    status_t              init2_status;
    boolean               init2_done;
    boolean               cleanup_done;
} agt_dynlib_cb_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_init1 (int argc,
	       const char *argv[],
	       boolean *showver,
	       help_mode_t *showhelpmode);

extern status_t
    agt_init2 (void);

extern void 
    agt_cleanup (void);

extern agt_profile_t *
    agt_get_profile (void);

extern void
    agt_request_shutdown (ncx_shutdowntyp_t mode);

extern boolean
    agt_shutdown_requested (void);

extern ncx_shutdowntyp_t
    agt_shutdown_mode_requested (void);

extern const xmlChar *
    agt_cbtype_name (agt_cbtyp_t cbtyp);

extern status_t
    agt_load_sil_code (const xmlChar *modname,
                       const xmlChar *revision,
                       boolean cfgloaded);

extern boolean
    agt_advertise_module_needed (const xmlChar *modname);

#endif	    /* _H_agt */
