#ifndef _H_agt_tree
#define _H_agt_tree

/*  FILE: agt_tree.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Agent subtree filter processing for <filter> element in
    <get> and <get-config> operations

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
16-jun-06    abb      Begun

*/

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* get and get-config step 1 */
extern ncx_filptr_t *
    agt_tree_prune_filter (ses_cb_t *scb,
			   rpc_msg_t *msg,
			   const cfg_template_t *cfg,
			   boolean getop);

/* get and get-config step 2 */
extern void
    agt_tree_output_filter (ses_cb_t *scb,
			    rpc_msg_t *msg,
			    ncx_filptr_t *top,
			    int32 indent,
			    boolean getop);

/* notification */
extern boolean
    agt_tree_test_filter (xml_msg_hdr_t *msghdr,
                          ses_cb_t *scb,
                          val_value_t *filter,
                          val_value_t *topval);


#endif	    /* _H_agt_tree */
