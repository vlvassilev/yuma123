#ifndef _H_rpc_mgr
#define _H_rpc_mgr
/*  FILE: rpc_mgr.h
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
30-apr-05    abb      Begun.
*/


#ifndef _H_buf
#include "buf.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/

typedef enum rpc_mgr_state_t_ {
    RPC_ST_NONE,
    RPC_ST_INIT,
    RPC_ST_FREE,
    RPC_ST_ALLOC,
    RPC_ST_SENT,
    RPC_ST_RPY
} rpc_mgr_state_t;

/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern status_t 
rpc_mgr_init (void);

extern void 
rpc_mgr_cleanup (void);

extern rpc_msg_t * 
rpc_mgr_alloc_req (uint32 reqsize,
		   uint32 group_id);

extern void 
rpc_mgr_free_req (rpc_msg_t *req);

extern status_t 
rpc_mgr_start_hdr (xmlns_id_t      ns_id,
		   xml_attrs_t    *attrs,
		   buf_buffer_t   *buffer);

extern status_t 
rpc_mgr_finish_hdr (xmlns_id_t        ns_id,
		    buf_buffer_t     *buffer);

#endif            /* _H_rpc_mgr */
