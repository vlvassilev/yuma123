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
30-sep-08    abb      Implement AGT_CB_TEST_APPLY and 
                      agt_val_split_root_test for YANG support of 
                      dummy running config commit-time validation
*/

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h.h"
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

#ifndef _H_xml_msg
#include "xml_msg.h"
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

/* General val_value_t processing
 *
 * step 1: call agt_val_parse_nc
 * step 2: call val_add_defaults
 * step 3: call agt_val_instance_check
 *
 * Additional steps to write to a config database
 *
 * step 4: call agt_val_validate_write
 * step 5: call agt_vall_apply_write
 */
extern status_t 
    agt_val_instance_check (ses_cb_t *scb,
			    xml_msg_hdr_t *msg,
			    val_value_t *valset,
			    ncx_layer_t   layer);


extern status_t 
    agt_val_root_check (ses_cb_t *scb,
			xml_msg_hdr_t *msg,
			val_value_t *root);

extern status_t 
    agt_val_split_root_check (ses_cb_t *scb,
			      rpc_msg_t *msg,
			      val_value_t *newroot,
			      val_value_t *root,
			      op_editop_t defop);

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
			 op_editop_t  editop);


extern status_t
    agt_val_apply_commit (ses_cb_t  *scb,
			  rpc_msg_t  *msg,
			  cfg_template_t *source,
			  cfg_template_t *target);

#endif	    /* _H_agt_val */
