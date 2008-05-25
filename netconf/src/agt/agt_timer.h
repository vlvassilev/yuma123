#ifndef _H_agt_timer
#define _H_agt_timer

/*  FILE: agt_timer.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Handle timer services for the agent


*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
23-jan-07    abb      Begun

*/

#ifndef _H_status
#include "status.h"
#endif

/* handle for timer ID */
typedef unsigned int agt_timer_t;

/* type of event sent to a timer callback */
typedef enum agt_timer_event_t_ {
    AGT_TIMER_NONE,
    AGT_TIME_EXPIRE,
    AGT_TIMER_RESET,
    AGT_TIMER_CANCEL,
    AGT_TIMER_DESTROY
} agt_timer_event_t;


/* timer callback function
 *
 * Process the peridic event
 *
 * INPUTS:
 *    event_type == reason for callback
 *    timer_id == timer identifier 
 *    context == context pointer, such as a session control block,
 *            passed to agt_timer_set function (may be NULL)
 *
 * RETURNS:
 *     0 == OK exit, keep the timer going
 *    -1 == error exit, call agt_timer_destroy upon return
 */

typedef int (*agt_timer_fn_t) (agt_timer_event_t event_type,
			       agt_timer_t  timer_id,
			       void *context);


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* module init */
extern void
    agt_timer_init (void);

/* module cleanup */
extern void 
    agt_timer_cleanup (void);

/* main routine called by agt_signal_handler */
extern void
    agt_timer_handler (void);

/* create a timer and start it going */
extern status_t
    agt_timer_set (uint32   seconds,
		   agt_timer_fn_t  timer_fn,
		   void *context,
		   agt_timer_t *ret_timer);

/* reset a timer -- give it a new timeout value */
extern status_t
    agt_timer_reset (uint32 seconds,
		     agt_timer_t  timer_id);

/* make a timer inactive, but reusable */		   
extern status_t
    agt_timer_clear (agt_timer_t timer_id);

/* destroy a timer; the ID may get reused if the agt_timer_set
 * function is called again with new parameters
 */		   
extern status_t
    agt_timer_destroy (agt_timer_t  timer_id);
		   


#endif	    /* _H_agt_timer */
