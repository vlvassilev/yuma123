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
#ifndef _H_agt_rpc
#define _H_agt_rpc
/*  FILE: agt_rpc.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol remote procedure call agent-side definitions

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
30-apr-05    abb      Begun.
*/

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
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

/* this constant is for the number of callback slots
 * allocated in a 'cbset', and only includes the
 * RPC phases that allow callback functions
 */
#define AGT_RPC_NUM_PHASES   3

/********************************************************************
*                                                                   *
*                         T Y P E S                                 *
*                                                                   *
*********************************************************************/

/* There are 6 different callbacks possible in the
 * agent processing chain. 
 *
 * Only AGT_RPC_PH_INVOKE is mandatory
 * Validate is needed if parameter checking beyond the
 * NCX syntax capabilities, such as checking if a needed
 * lock is available
 *
 * The engine will check for optional callbacks during 
 * RPC processing.
 *
 */
typedef enum agt_rpc_phase_t_ {
    AGT_RPC_PH_VALIDATE,         /* (2) cb after the input is parsed */
    AGT_RPC_PH_INVOKE,      /* (3) cb to invoke the requested method */
    AGT_RPC_PH_POST_REPLY,    /* (5) cb after the reply is generated */ 
    AGT_RPC_PH_PARSE,                    /* (1) NO CB FOR THIS STATE */ 
    AGT_RPC_PH_REPLY                     /* (4) NO CB FOR THIS STATE */ 
} agt_rpc_phase_t;


/* Template for RPC agent callbacks
 * The same template is used for all RPC callback phases
 */
typedef status_t 
    (*agt_rpc_method_t) (ses_cb_t *scb,
			 rpc_msg_t *msg,
			 xml_node_t *methnode);


typedef struct agt_rpc_cbset_t_ {
    agt_rpc_method_t  acb[AGT_RPC_NUM_PHASES];
} agt_rpc_cbset_t;



/* Callback template for RPCs that use an inline callback
 * function instead of generating a malloced val_value_t tree
 *
 * INPUTS:
 *   scb == session control block
 *   msg == RPC request in progress
 *   indent == start indent amount; ignored if the agent
 *             is configured not to use PDU indentation
 * RETURNS:
 *   status of the output operation
 */
typedef status_t 
    (*agt_rpc_data_cb_t) (ses_cb_t *scb, 
			  rpc_msg_t *msg,
			  uint32 indent);

				      
				      
/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

/* should call once to init RPC agent module */
extern status_t 
    agt_rpc_init (void);


/* should call once to cleanup RPC agent module */
extern void 
    agt_rpc_cleanup (void);


/* add callback for 1 phase of RPC processing */
extern status_t 
    agt_rpc_register_method (const xmlChar *module,
			     const xmlChar *method_name,
			     agt_rpc_phase_t  phase,
			     agt_rpc_method_t method);


/* remove callback node for all phases of RPC processing */
extern void
    agt_rpc_unregister_method (const xmlChar *module,
			       const xmlChar *method_name);

/* mark an RPC in the definition file as a supported operation */
extern void 
    agt_rpc_support_method (const xmlChar *module,
                            const xmlChar *method_name);

/* mark an RPC in the definition file as an unsupported operation */
extern void
    agt_rpc_unsupport_method (const xmlChar *module,
			      const xmlChar *method_name);


/* called by top.c: 
 * This function is registered with top_register_node
 * for the owner 'ietf', top-node 'rpc'
 */
extern void
    agt_rpc_dispatch (ses_cb_t  *scb,
		      xml_node_t *top);


/* used for OP_EDITOP_LOAD to load the running from startup
 * and OP_EDITOP_REPLACE to restore running from backup
 */
extern status_t
    agt_rpc_load_config_file (const xmlChar *filespec,
			      cfg_template_t  *cfg,
                              boolean isload,
                              ses_id_t  use_sid);


/* used to make the sysStartup notification */
extern status_t
    agt_rpc_fill_rpc_error (const rpc_err_rec_t *err,
			    val_value_t *rpcerror);

#endif            /* _H_agt_rpc */
