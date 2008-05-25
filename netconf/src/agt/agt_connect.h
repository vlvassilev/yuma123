#ifndef _H_agt_connect
#define _H_agt_connect

/*  FILE: agt_connect.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Handle the <ncx-connect> (top-level) element.
    This message is used for thin clients to connect
    to the ncxserver. 

   Client --> SSH2 --> OpenSSH.subsystem(netconf) -->
 
      ncxserver_connect --> AF_LOCAL/ncxserver.sock -->

      ncxserver.listen --> top_dispatch -> ncx_connect_handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
15-jan-07    abb      Begun

*/

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_connect_init (void);

extern void 
    agt_connect_cleanup (void);

extern void
    agt_connect_dispatch (ses_cb_t *scb,
			  xml_node_t *top);


#endif	    /* _H_agt_connect */
