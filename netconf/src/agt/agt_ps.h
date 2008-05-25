#ifndef _H_agt_ps
#define _H_agt_ps

/*  FILE: agt_ps.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Parmset callback handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
28-apr-06    abb      Begun

*/

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_getcb
#include "getcb.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
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

#ifndef _H_val
#include "val.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/



/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    agt_ps_validate_write (ses_cb_t  *scb,
			   rpc_msg_t  *msg,
			   cfg_template_t *target,
			   val_value_t *valroot,
			   op_editop_t  editop);


status_t
    agt_ps_apply_write (ses_cb_t  *scb,
			rpc_msg_t  *msg,
			cfg_template_t *target,
			val_value_t    *pducfg,
			op_editop_t  editop,
			op_errop_t  errop);


#if 0
extern status_t
    agt_ps_invoke_cb (agt_cbtyp_t cbtyp,
		      op_editop_t editop,
		      ses_cb_t  *scb,
		      rpc_msg_t  *msg,
		      cfg_template_t *target,
		      ps_parmset_t  *ps,
		      ps_parmset_t  *curps,
		      boolean        conterr);
#endif

extern ps_parmset_t *
    agt_ps_new_vparmset (const xmlChar *module,
			 const xmlChar *psdname,
			 getcb_fn_t cbfn,
			 boolean  full,
			 status_t *res);

extern ps_parm_t *
    agt_ps_new_vparm (ps_parmset_t *ps,
		      const psd_parm_t *parmdef,
		      getcb_fn_t cbfn,
		      status_t *res);

extern void
    agt_ps_set_lastchange (ps_parmset_t *ps);

#endif	    /* _H_agt_ps */
