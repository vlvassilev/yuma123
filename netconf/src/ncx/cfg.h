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
           
*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
15-apr-06    abb      Begun.
*/
#include <xmlstring.h>

#ifndef _H_ps
#include "ps.h"
#endif

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
    val_value_t   *root;              /* btyp == anyapp */
} cfg_template_t;


/* a struct holding all the parmsets for an application section */
typedef struct cfg_app_t_ {
    dlq_hdr_t            qhdr;
    const ncx_appnode_t *appdef;
    void                *parent;      /* val_value_t container */
    op_editop_t          editop;     /* used during operations */
    dlq_hdr_t            parmsetQ;
    struct cfg_app_t_   *last;      /* last value for rollback */
} cfg_app_t;


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

extern cfg_app_t *
    cfg_new_appnode (void);

extern void 
    cfg_init_appnode (cfg_app_t *app);

extern void 
    cfg_clean_appnode (cfg_app_t *app);

extern void 
    cfg_free_appnode (cfg_app_t *app);

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

extern status_t
    cfg_add_parmset (cfg_template_t *cfg,
		     ps_parmset_t *ps,
		     ses_id_t     loadby);

extern ps_parmset_t *
    cfg_get_parmset (cfg_template_t *cfg,
		     ps_parmset_t *ps);

extern status_t 
    cfg_del_parmset (cfg_template_t *cfg,
		     ps_parmset_t *ps,
		     boolean  delcheck);

extern status_t
    cfg_load_root (cfg_template_t *cfg);

extern cfg_app_t *
    cfg_get_appnode (cfg_template_t *cfg,
		     const xmlChar *module,
		     const xmlChar *appname);

extern void
    cfg_release_locks (ses_id_t sesid);

extern void
    cfg_get_lock_list (ses_id_t sesid,
		       val_value_t *retval);

extern ncx_data_class_t
    cfg_get_app_dataclass (const cfg_app_t *app);

#endif            /* _H_cfg */
