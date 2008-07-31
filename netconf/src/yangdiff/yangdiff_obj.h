#ifndef _H_yangdiff_obj
#define _H_yangdiff_obj

/*  FILE: yangdiff_obj.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Report differences related to YANG objects
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
10-jul-08    abb      Split out from yangdiff.c

*/

#ifndef _H_yangdiff
#include "yangdiff.h"
#endif


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern uint32
    datadefQ_changed (yangdiff_diffparms_t *cp,
		      dlq_hdr_t *oldQ,
		      dlq_hdr_t *newQ);

extern void
    output_datadefQ_diff (yangdiff_diffparms_t *cp,
			  dlq_hdr_t *oldQ,
			  dlq_hdr_t *newQ);

#endif	    /* _H_yangdiff_obj */
