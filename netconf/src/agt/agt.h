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

        - Parmameter Set instance document parsing

        - Parmameter Set syntax validation

        - Parmameter Set basic referential integrity

        - Value instance document parsing 

        - Value instance document syntax validation

        - Value instance count validation

        - Value instance default value insertion


    The NCX agent <edit-config> handler registers callbacks
    with the agt_ps module that can be called IN ADDITION
    to the automated callback, for referential integrity checking,
    resource reservation, etc.

    NCX Write Model
    ---------------

    NCX uses a 3 pass callback model to process <edit-config> PDUs.
    
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

    If any errors occur within a parameter set (within the 
    /config/appname/ container) then that entire parameter set
    is considered unusable and it is not applied.

    If continue-on-error is requested, then all sibling parameter
    sets within an application node, and all sibling
    application nodes will be processed if the validation tests
    all pass.

    Managers should be careful wrt/ the parmsets and applications
    that are edited simultaneously within a single <edit-config> operation.

  Pass 2: Apply : AGT_CB_APPLY

    On a per-parmset granularity, an operation is processed only
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

  Pass 3-pos: Commit (Positive Outcome) :  AGT_CB_COMMIT
       
    The database edits are automatically completed during this phase.
    The user callbacks must not edit the database -- only modify
    their own data structures, send PDUs, etc.

  Pass 3-neg: Rollback (Negative Outcome) : AGT_CB_ROLLBACK

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

*/

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


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/* enumeration of the different agent callback types 
 * These are used are array indices so there is no dummy zero enum
 */
typedef enum agt_cbtyp_t_ {
    AGT_CB_LOAD,           /* internal PS handler startup/shutdown */
    AGT_CB_VALIDATE,               /* P1: write operation validate */
    AGT_CB_APPLY,                     /* P2: write operation apply */
    AGT_CB_COMMIT,               /* P3-pos: write operation commit */
    AGT_CB_ROLLBACK            /* P3-neg: write operation rollback */
} agt_cbtyp_t;


/* hardwire some of the agent profile parameters
 * because they are needed before the NCX engine is running
 * They cannot be changed after boot-time.
 */
typedef struct agt_profile_t_ {
    ncx_agttarg_t    agt_targ;
    ncx_agtstart_t   agt_start;
    boolean          agt_del_startup;
    log_debug_t      agt_loglevel;
    boolean          agt_usestartup;
    boolean          agt_logappend;
    const xmlChar   *agt_logfile;
    const xmlChar   *agt_startup;
    const xmlChar   *agt_modpath;
    const xmlChar   *agt_datapath;
    const xmlChar   *agt_runpath;
} agt_profile_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_init (int argc,
	      const char *argv[],
	      boolean *showver,
	      boolean *showhelp);

extern void 
    agt_cleanup (void);

extern const agt_profile_t *
    agt_get_profile (void);

extern void
    agt_request_shutdown (ncx_shutdowntyp_t mode);

extern boolean
    agt_shutdown_requested (void);

#endif	    /* _H_agt */
