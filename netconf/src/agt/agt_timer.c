/*  FILE: agt_timer.c

    Handle timer services
		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
23jan07      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_timer
#include "agt_timer.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_TIMER_DEBUG 1
#endif

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static boolean agt_timer_init_done = FALSE;


/********************************************************************
* FUNCTION agt_timer_init
*
* Initialize the agt_timer module
*
* INPUTS:
*   none
* RETURNS:
*   NO_ERR if all okay, the minimum spare requests will be malloced
*********************************************************************/
void
    agt_timer_init (void)
{
    if (!agt_timer_init_done) {
	/* setup agt_timer_handler */

	agt_timer_init_done = TRUE;
    }

} /* agt_timer_init */


/********************************************************************
* FUNCTION agt_timer_cleanup
*
* Cleanup the agt_timer module.
*
*********************************************************************/
void 
    agt_timer_cleanup (void)
{
    if (agt_timer_init_done) {
	/* restore previous handlers */

	agt_timer_init_done = FALSE;
    }

} /* agt_timer_cleanup */


/********************************************************************
* FUNCTION agt_timer_handler
*
* Handle an incoming interrupt timer
*
* INPUTS:
*   intr == interrupt numer
*
*********************************************************************/
void 
    agt_timer_handler (void)
{

} /* agt_timer_handler */

/* END file agt_timer.c */


