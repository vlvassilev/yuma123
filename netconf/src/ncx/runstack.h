#ifndef _H_runstack
#define _H_runstack

/*  FILE: runstack.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************


 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
22-aug-07    abb      Begun; split from ncxcli.c

*/

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

#define RUNSTACK_MAX_PARMS  9


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

extern uint32
    runstack_level (void);

extern status_t
    runstack_push (const xmlChar *source,
		   FILE *fp);

extern void
    runstack_pop (void);


extern xmlChar *
    runstack_get_cmd (status_t *res);

extern void
    runstack_cancel (void);

extern dlq_hdr_t *
    runstack_get_que (boolean isglobal);

extern dlq_hdr_t *
    runstack_get_parm_que (void);

extern void
    runstack_init (void);

extern void
    runstack_cleanup (void);

#endif	    /* _H_runstack */
