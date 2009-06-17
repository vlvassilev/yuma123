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

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif

#ifndef _H_val
#include "val.h"
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

extern boolean
    agt_xpath_test_filter (xml_msg_hdr_t *msghdr,
                           ses_cb_t *scb,
                           const val_value_t *selectval,
                           val_value_t *val);

#endif	    /* _H_agt_xpath */
