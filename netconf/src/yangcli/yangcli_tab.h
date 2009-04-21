#ifndef _H_yangcli_tab
#define _H_yangcli_tab

/*  FILE: yangcli_tab.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

   Tab word completion callback support for libtecla
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
18-apr-09    abb      Begun;

*/


#include "libtecla.h"


/********************************************************************
*								    *
*		      F U N C T I O N S 			    *
*								    *
*********************************************************************/

extern int
    yangcli_tab_callback (WordCompletion *cpl, 
			  void *data,
			  const char *line, 
			  int word_end);


#endif	    /* _H_yangcli_tab */
