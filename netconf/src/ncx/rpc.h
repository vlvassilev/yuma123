#ifndef _H_rpc
#define _H_rpc
/*  FILE: rpc.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol remote procedure call common definitions
    This module is only used by the agent at this time

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
01-may-05    abb      Begun.
*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/
#define RPC_STR_REQ     "rpc"
#define RPC_STR_RPY     "rpc-reply"
#define RPC_STR_ERR     "rpc-error"
#define RPC_STR_MSG_ID  "message-id"
#define RPC_STR_GRP_ID  "group-id"


/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/


/* From the NCX module rpc-type parameter */
typedef enum rpc_type_t_ {
    RPC_TYP_NONE,
    RPC_TYP_OTHER,
    RPC_TYP_CONFIG,
    RPC_TYP_EXEC,
    RPC_TYP_MONITOR,
    RPC_TYP_DEBUG
} rpc_type_t;


/* Type of the NCX module rpc.out-data parameter */
typedef enum rpc_outtyp_t_ {
    RPC_OT_NONE,
    RPC_OT_TYPE,
    RPC_OT_PSD,
    RPC_OT_MON
} rpc_outtyp_t;


/* struct of params to undo an edit-config opration
 * The actual nodes used depend on the edit operation value
 */
typedef struct rpc_undo_rec_t_ {
    dlq_hdr_t       qhdr;
    boolean         ismeta;
    op_editop_t     editop;
    val_value_t    *newnode;      
    val_value_t    *curnode;      
    val_value_t    *parentnode; 
    dlq_hdr_t       extra_deleteQ;     
    status_t        res;
} rpc_undo_rec_t;


/* NCX Agent RPC Request/Reply Message Header */
typedef struct rpc_msg_t_ {
    dlq_hdr_t        qhdr;

    /* generic XML message header */
    xml_msg_hdr_t    mhdr; 

    /* incoming: top-level rpc element data */
    xml_attrs_t     *rpc_in_attrs;     /* borrowed from <rpc> elem */

    /* incoming: 
     * 2nd-level method name element data, used in agt_output_filter
     * to check get or get-config
     */
    const struct obj_rpc_t_ *rpc_method; 

    /* incoming: AGENT RPC processing state */
    int              rpc_agt_state;        /* rpc_agt_phase_t */
    op_errop_t       rpc_err_option;       
    val_value_t      rpc_input;
    status_t         rpc_status;     /* final internal status */

    /* incoming:
     * hooks for method routines to save context or whatever 
     */
    void           *rpc_user1;
    void           *rpc_user2;

    /* incoming: get method reply handling builtin 
     * If the rpc_datacb is non-NULL then it will be used as a
     * callback to generate the rpc-reply inline, instead of
     * buffering the output.  
     * The rpc_data and rpc_filter parameters are optionally used
     * by the rpc_datacb function to generate a reply.
     */
    void           *rpc_datacb;              /* rpc_agt_data_cb_t */
    val_value_t    *rpc_data;                       /* data reply */
    op_filter_t     rpc_filter;        /* filter for get* methods */

    /* incoming: rollback or commit phase support builtin
     * As an edit-config (or other RPC) is processed, a
     * queue of 'undo records' is collected.
     * If the apply phase succeeds then it is discarded,
     * otherwise if rollback-on-error is requested,
     * it is used to undo each change that did succeed (if any)
     * before an error occured in the apply phase.
     */
    boolean          rpc_need_undo;   /* set by edit_config_validate */
    dlq_hdr_t        rpc_undoQ;       /* Q of rpc_undo_rec_t */

} rpc_msg_t;


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/


/********************** RPC MESSAGES **************************/

extern rpc_msg_t * 
    rpc_new_msg (void);

extern rpc_msg_t * 
    rpc_new_out_msg (void);

extern void 
    rpc_init_msg (rpc_msg_t *msg);

extern void 
    rpc_free_msg (rpc_msg_t *msg);

extern void 
    rpc_clean_msg (rpc_msg_t *msg);

extern const xmlChar *
    rpc_get_rpctype_str (rpc_type_t rpctyp);

extern rpc_undo_rec_t *
    rpc_new_undorec (void);

extern void
    rpc_init_undorec (rpc_undo_rec_t *undo);

extern void
    rpc_free_undorec (rpc_undo_rec_t *undorec);

extern void 
    rpc_clean_undorec (rpc_undo_rec_t *undo);

#endif            /* _H_rpc */
