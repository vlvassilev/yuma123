#ifndef _H_mgr
#define _H_mgr

/*  FILE: mgr.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Manager message handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
03-feb-06    abb      Begun

*/

/* used by the manager for the SSH2 interface */
#include <libssh2.h>

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

#define MGR_MAX_REQUEST_ID 0xfffffffe

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/* extension to the ses_cb_t for a manager session */
typedef struct mgr_scb_t_ {

    /* agent info */
    ncx_agttarg_t   targtyp;
    ncx_agtstart_t  starttyp;
    cap_list_t      caplist;
    uint32          agtsid;   /* agent assigned session ID */
    boolean         closed;

    /* temp directory for downloaded modules */
    ncxmod_temp_progcb_t *temp_progcb;
    ncxmod_temp_sescb_t  *temp_sescb;

    /* running config cached info */
    val_value_t    *root;
    xmlChar        *chtime;
    val_value_t    *lastroot;
    xmlChar        *lastchtime;

    /* transport info */
    xmlChar         *target;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;
    int              returncode;

    /* RPC request info */
    uint32           next_id;
    dlq_hdr_t        reqQ;
} mgr_scb_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    mgr_init (void);

extern void 
    mgr_cleanup (void);

extern mgr_scb_t *
    mgr_new_scb (void);

extern void
    mgr_init_scb (mgr_scb_t *mscb);

extern void
    mgr_free_scb (mgr_scb_t *mscb);

extern void
    mgr_clean_scb (mgr_scb_t *mscb);

extern void
    mgr_request_shutdown (void);

extern boolean
    mgr_shutdown_requested (void);

#endif	    /* _H_mgr */
