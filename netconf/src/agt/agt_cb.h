#ifndef _H_agt_cb
#define _H_agt_cb

/*  FILE: agt_cb.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Data Model callback handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
16-apr-07    abb      Begun; split out from agt_ps.h

*/

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_ps
#include "ps.h"
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

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/
/* symbolic tags for agt_cb_register_callback 'forall' boolean */
#define FORALL  TRUE
#define FORONE  FALSE


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* Callback function for agent parmset handler 
 * Used to provide a PS callback sub-mode for
 * a specific named parmset
 * 
 * INPUTS:
 *   scb == session control block
 *   msg == incoming rpc_msg_t in progress
 *   cbtyp == reason for the callback
 *   editop == the parent edit-config operation type, which
 *             is also used for all other callbacks
 *             that operate on parmsets
 *   newps == parmset holding the proposed changes to
 *           apply to the current config, depending on
 *           the editop value. Will not be NULL.
 *   curps == current parmset values from the <running> 
 *           or <candidate> configuration, if any. Could be NULL
 *           for create and other operations.
 *
 * RETURNS:
 *    status:
 */
typedef status_t 
    (*agt_cb_pscb_t) (ses_cb_t  *scb,
		      rpc_msg_t *msg,
		      agt_cbtyp_t cbtyp,
		      op_editop_t  editop,
		      ps_parmset_t  *newps,
		      ps_parmset_t  *curps);



/* set of agent parmset callback functions */
typedef struct agt_cb_pscbset_t_ {
    agt_cb_pscb_t    pscb[AGT_NUM_CB];
} agt_cb_pscbset_t;


/* Callback function for agent parm handler 
 * Used to provide a parm callback sub-mode for
 * a specific named parm
 * 
 * INPUTS:
 *   scb == session control block
 *   msg == incoming rpc_msg_t in progress
 *   cbtyp == reason for the callback
 *   editop == the parent edit-config operation type, which
 *             is also used for all other callbacks
 *             that operate on parmsets
 *   newp == parm holding the proposed changes to
 *           apply to the current config, depending on
 *           the editop value.  Will not be NULL.
 *   curp == current parm value from the <running> 
 *           or <candidate> configuration, if any. Could be NULL
 *           for create and other operations.
 *
 * RETURNS:
 *    status:
 */
typedef status_t 
    (*agt_cb_pcb_t) (ses_cb_t  *scb,
		     rpc_msg_t *msg,
		     agt_cbtyp_t cbtyp,
		     op_editop_t  editop,
		     ps_parm_t *newp,
		     ps_parm_t *curp);


/* set of agent parm callback functions */
typedef struct agt_cb_pcbset_t_ {
    agt_cb_pcb_t    pcb[AGT_NUM_CB];
} agt_cb_pcbset_t;


/* Callback function for agent typedef handler 
 * Used to provide a PS callback sub-mode for
 * a specific named type
 *
 * INPUTS:
 *   scb == session control block
 *   msg == incoming rpc_msg_t in progress
 *   cbtyp == reason for the callback
 *   editop == the edit-config operation type, which
 *             is also used for all other callbacks
 *             that operate on data types
 *   newval == value struct holding the proposed changes to
 *           apply to the current config, depending on
 *           the editop value.  Will not be NULL.
 *   curval == current value from the <running> 
 *           or <candidate> configuration, if any. Could be NULL
 *           for create and other operations.
 *
 * RETURNS:
 *    status:
 */
typedef status_t 
    (*agt_cb_tcb_t) (ses_cb_t  *scb,
		     rpc_msg_t *msg,
		     agt_cbtyp_t cbtyp,
		     op_editop_t  editop,
		     val_value_t  *newval,
		     val_value_t  *curval);

/* set of agent typedef callback functions */
typedef struct agt_cb_tcbset_t_ {
    agt_cb_tcb_t    tcb[AGT_NUM_CB];
} agt_cb_tcbset_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_cb_register_ps_callback (const xmlChar *module,
				 const xmlChar *defname,
				 boolean forall,
				 agt_cbtyp_t cbtyp,
				 agt_cb_pscb_t    cbfn);


extern status_t 
    agt_cb_register_parm_callback (const xmlChar *module,
				   const xmlChar *psdname,
				   const xmlChar *defname,
				   boolean forall,
				   agt_cbtyp_t cbtyp,
				   agt_cb_pcb_t    cbfn);


extern status_t 
    agt_cb_register_typ_callback (const xmlChar *module,
				  const xmlChar *defname,
				  boolean forall,
				  agt_cbtyp_t cbtyp,
				  agt_cb_tcb_t    cbfn);


extern void
    agt_cb_unregister_ps_callback (const xmlChar *module,
				const xmlChar *defname);

extern void
    agt_cb_unregister_parm_callback (const xmlChar *module,
				     const xmlChar *psdname,
				     const xmlChar *defname);

extern void
    agt_cb_unregister_typ_callback (const xmlChar *module,
				    const xmlChar *defname);


#endif	    /* _H_agt_cb */
