#ifndef _H_cfg
#define _H_cfg
/*  FILE: cfg.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NCX configuration database manager

    Configuration segments are stored in sequential order.

    OLD NCX MODEL
    =============

    Configuration database (running, candidate, startup, etc.)
      +
      |
      +-- (root: /)
           +
           |
           +--- application X  (netconf, security, routing, etc.)
           |         |
           |         +---- parmset A , B, C
           |           
           +--- application Y
           |         |
           |         +---- parmset D , E
           V


   Parmset A is defined (hard-wired) to belong to application X


   NEW YANG OBJECT MODEL
   =====================

    Configuration database (running, candidate, startup, etc.)
      +
      |
      +-- (root: /)
           +
           |
           +--- object X  (netconf, security, routing, etc.)
           |         |
           |         +---- object A , B, C
           |           
           +--- object Y
           |         |
           |         +---- object D , E
           V


   MODULE USAGE
   ============

   1) call cfg_init()

   2) call cfg_init_static_db for the various hard-wired databases
      that need to be created by the agent

   3) call cfg_load_root() with the startup database contents
      to load into the running config

   4) call cfg_set_target() [NCX_CFGID_CANDIDATE or NCX_CFGID_RUNNING]
   
   5) call cfg_set_state() to setup config db access, when ready
      for NETCONF operations

   6) Use cfg_ok_to_* functions to test config access

   7) use cfg_lock and cfg_unlock as desired to set global write locks

   8) use cfg_release_locks when a session terminates

   9) call cfg_cleanup() when program is terminating

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
15-apr-06    abb      Begun.
29-may-08    abb      Converted NCX database model to simplified
                      YANG object model
*/
#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
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
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

/* bit definitions for the cfg_template->flags field */
#define CFG_FL_TARGET       bit0


/********************************************************************
*                                                                   *
*                        T Y P E S                                  *
*                                                                   *
*********************************************************************/


/* current configuration state */
typedef enum cfg_state_t_ {
    CFG_ST_NONE,                  /* not set */
    CFG_ST_INIT,                  /* init in progress */
    CFG_ST_READY,                 /* ready and no locks */
    CFG_ST_PLOCK,                 /* partial lock active */
    CFG_ST_FLOCK,                 /* full lock active */
    CFG_ST_CLEANUP                /* cleanup in progress */
} cfg_state_t;


/* classify the config source */
typedef enum cfg_source_t_ {
    CFG_SRC_NONE,
    CFG_SRC_INTERNAL,
    CFG_SRC_NETCONF,
    CFG_SRC_CLI,
    CFG_SRC_SNMP,
    CFG_SRC_HTTP,
    CFG_SRC_OTHER
} cfg_source_t;


/* classify the config location */
typedef enum cfg_location_t_ {
    CFG_LOC_NONE,
    CFG_LOC_INTERNAL,
    CFG_LOC_FILE,
    CFG_LOC_NAMED,
    CFG_LOC_LOCAL_URL,
    CFG_LOC_REMOTE_URL
} cfg_location_t;


/* struct representing 1 configuration database */
typedef struct cfg_template_t_ {
    dlq_hdr_t      qhdr;
    ncx_cfg_t      cfg_id;
    cfg_location_t cfg_loc;
    cfg_state_t    cfg_state;
    xmlChar       *name;
    xmlChar       *src_url;
    xmlChar       *load_time;
    xmlChar       *last_ch_time;
    uint32         flags;
    ses_id_t       locked_by;
    cfg_source_t   lock_src;
    dlq_hdr_t      load_errQ;    /* Q of rpc_err_rec_t */
    val_value_t   *root;          /* btyp == container */
} cfg_template_t;


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern void
    cfg_init (void);

extern void
    cfg_cleanup (void);

extern status_t
    cfg_init_static_db (ncx_cfg_t cfg_id);

extern cfg_template_t *
    cfg_new_template (const xmlChar *name,
		      ncx_cfg_t cfg_id);

extern void
    cfg_free_template (cfg_template_t *cfg);

extern cfg_state_t
    cfg_get_state (ncx_cfg_t cfg_id);

extern void
    cfg_set_state (ncx_cfg_t cfg_id,
		   cfg_state_t new_state);

extern cfg_template_t *
    cfg_get_config (const xmlChar *cfgname);

extern cfg_template_t *
    cfg_get_config_id (ncx_cfg_t cfgid);

extern void
    cfg_set_target (ncx_cfg_t cfg_id);

extern status_t
    cfg_ok_to_lock (const cfg_template_t *cfg);

extern status_t
    cfg_ok_to_unlock (const cfg_template_t *cfg,
		      ses_id_t sesid);

extern status_t
    cfg_ok_to_read (const cfg_template_t *cfg);

extern status_t
    cfg_ok_to_write (const cfg_template_t *cfg,
		     ses_id_t sesid);

extern status_t
    cfg_lock (cfg_template_t *cfg,
	      ses_id_t locked_by,
	      cfg_source_t  lock_src);

extern status_t
    cfg_unlock (cfg_template_t *cfg,
		ses_id_t locked_by);

extern void
    cfg_release_locks (ses_id_t sesid);

extern void
    cfg_get_lock_list (ses_id_t sesid,
		       val_value_t *retval);


extern val_value_t *
    cfg_find_datanode (const xmlChar *target,
		       ncx_cfg_t  cfgid);

extern val_value_t *
    cfg_find_modrel_datanode (ncx_module_t *mod,
			      const xmlChar *target,
			      ncx_cfg_t  cfgid);

#endif            /* _H_cfg */
