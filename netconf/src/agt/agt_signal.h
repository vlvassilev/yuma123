#ifndef _H_agt_signal
#define _H_agt_signal

/*  FILE: agt_signal.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Handle interrupt signals for the agent


*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
22-jan-07    abb      Begun

*/

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    agt_signal_init (void);

extern void 
    agt_signal_cleanup (void);

extern void
    agt_signal_handler (int intr);


#endif	    /* _H_agt_signal */
