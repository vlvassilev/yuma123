/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
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

#include <time.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_status
#include "status.h"
#endif


/* timer callback function
 *
 * Process the timer expired event
 *
 * INPUTS:
 *    timer_id == timer identifier 
 *    cookie == context pointer, such as a session control block,
 *            passed to agt_timer_set function (may be NULL)
 *
 * RETURNS:
 *     0 == normal exit
 *    -1 == error exit, delete timer upon return
 */
typedef int (*agt_timer_fn_t) (uint32  timer_id,
			       void *cookie);


typedef struct agt_timer_cb_t_ {
    dlq_hdr_t       qhdr;
    boolean         timer_periodic;
    uint32          timer_id;
    agt_timer_fn_t  timer_cbfn;
    time_t          timer_start_time;
    uint32          timer_duration;   /* seconds */
    void           *timer_cookie;
} agt_timer_cb_t;


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
    agt_timer_create (uint32   seconds,
                      boolean is_periodic,
                      agt_timer_fn_t  timer_fn,
                      void *cookie,
                      uint32 *ret_timer_id);

/* reset a timer -- give it a new timeout value */
extern status_t
    agt_timer_restart (uint32 timer_id,
                       uint32 seconds);

/* periodic timers need to be deleted to be stopped
 * 1-shot timers will be deleted automatically after
 * they expire and the callback is invoked
 */
extern void
    agt_timer_delete (uint32  timer_id);

#endif	    /* _H_agt_timer */
