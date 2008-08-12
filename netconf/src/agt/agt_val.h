#ifndef _H_agt_val
#define _H_agt_val

/*  FILE: agt_val.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Value/Typedef callback handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
20-may-06    abb      Begun

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
    agt_val_validate_write (ses_cb_t  *scb,
			    rpc_msg_t  *msg,
			    cfg_template_t *target,
			    val_value_t *valroot,
			    op_editop_t  editop);


extern status_t
    agt_val_apply_write (ses_cb_t  *scb,
			 rpc_msg_t  *msg,
			 cfg_template_t *target,
			 val_value_t    *pducfg,
			 op_editop_t  editop,
			 op_errop_t  errop);


#endif	    /* _H_agt_val */
