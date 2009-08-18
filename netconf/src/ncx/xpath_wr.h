#ifndef _H_xpath_wr
#define _H_xpath_wr

/*  FILE: xpath_wr.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Write an XPath expression to a session output
    in normalized format

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
24-apr-09    abb      Begun

*/

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
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    xpath_wr_expr (ses_cb_t *scb,
		   val_value_t *xpathval);

#endif	    /* _H_xpath_wr */
