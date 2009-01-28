#ifndef _H_agt_xpath
#define _H_agt_xpath

/*  FILE: agt_xpath.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Agent XPath filter processing for select attribute in
    <filter> element in <get> and <get-config> operations

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
27-jan-09    abb      Begun

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

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    agt_xpath_output_filter (ses_cb_t *scb,
			     rpc_msg_t *msg,
			     const cfg_template_t *cfg,
			     boolean getop,
			     int32 indent);

#endif	    /* _H_agt_xpath */
