#ifndef _H_mgr_top
#define _H_mgr_top

/*  FILE: mgr_top.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Manager Top Element module

    Manage callback registry for received XML messages

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
11-feb-07    abb      Begun; start from agt_top.c

*/

#ifndef _H_ses
#include "ses.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    mgr_top_dispatch_msg (ses_cb_t  *scb);


#endif	    /* _H_mgr_top */
