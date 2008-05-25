#ifndef _H_agt_top
#define _H_agt_top

/*  FILE: agt_top.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Top Element module

    Manage callback registry for received XML messages

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
30-dec-05    abb      Begun

*/

#ifndef _H_ses
#include "ses.h"
#endif


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* called by the transport manager (through the session manager)
 * when a new message is detected
 */
extern void
    agt_top_dispatch_msg (ses_cb_t  *scb);


#endif	    /* _H_agt_top */
