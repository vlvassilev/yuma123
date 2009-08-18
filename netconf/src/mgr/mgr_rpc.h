#ifndef _H_mgr_rpc
#define _H_mgr_rpc
/*  FILE: mgr_rpc.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol remote procedure call manager-side definitions

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
13-feb-07    abb      Begun; started from agt_rpc.h
*/
#include <time.h>

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

#define MGR_RPC_NUM_PHASES   6

/********************************************************************
*                                                                   *
*                         T Y P E S                                 *
*                                                                   *
*********************************************************************/


typedef struct mgr_rpc_req_t_ {
    dlq_hdr_t      qhdr;
    xml_msg_hdr_t  mhdr;
    uint32         group_id;
    obj_template_t *rpc;
    xmlChar       *msg_id;       /* malloced message ID */
    xml_attrs_t    attrs;          /* Extra <rpc> attrs */
    val_value_t   *data;      /* starts with the method */
    time_t         starttime;     /* tstamp for timeout */
    uint32         timeout;       /* timeout in seconds */
    void          *replycb;           /* mgr_rpc_cbfn_t */
} mgr_rpc_req_t;


typedef struct mgr_rpc_rpy_t_ {
    dlq_hdr_t      qhdr;
    /* xml_msg_hdr_t  mhdr; */
    uint32         group_id;
    xmlChar       *msg_id;
    status_t       res;
    val_value_t   *reply;
} mgr_rpc_rpy_t;


/* manager RPC reply callback function
 *
 *  INPUTS:
 *   scb == session control block for session that got the reply
 *   req == RPC request returned to the caller (for reuse or free)
 *   rpy == RPY reply header; this will be NULL if the timeout
 *          occurred so there is no reply
 */
typedef void (*mgr_rpc_cbfn_t) (ses_cb_t *scb,
				mgr_rpc_req_t *req,
				mgr_rpc_rpy_t *rpy);


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

/* should call once to init RPC mgr module */
extern status_t 
    mgr_rpc_init (void);


/* should call once to cleanup RPC mgr module */
extern void 
    mgr_rpc_cleanup (void);

extern mgr_rpc_req_t *
    mgr_rpc_new_request (ses_cb_t *scb);

extern void
    mgr_rpc_free_request (mgr_rpc_req_t *req);

extern void
    mgr_rpc_free_reply (mgr_rpc_rpy_t *rpy);

extern void
    mgr_rpc_clean_requestQ (dlq_hdr_t *reqQ);

/*** returning number of msgs timed out
 *** need a callback-based cleanup later on
 *** to support N concurrent requests per agent
 ***/
extern uint32
    mgr_rpc_timeout_requestQ (dlq_hdr_t *reqQ);

/* non-blocking send, reply function will be called when
 * one is received or a timeout occurs
 *
 */
extern status_t
    mgr_rpc_send_request (ses_cb_t *scb,
			  mgr_rpc_req_t *req,
			  mgr_rpc_cbfn_t rpyfn);


/* cancel a request in progress */
extern status_t
    mgr_rpc_cancel_request (ses_cb_t *scb,
			    const xmlChar *msg_id);


/* handle the <rpc-reply> element
 * called by mgr_top.c: 
 * This function is registered with top_register_node
 * for the module 'netconf', top-node 'rpc-reply'
 */
extern void
    mgr_rpc_dispatch (ses_cb_t  *scb,
		      xml_node_t *top);


#endif            /* _H_mgr_rpc */
