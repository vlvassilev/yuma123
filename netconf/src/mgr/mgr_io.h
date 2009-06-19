#ifndef _H_mgr_io
#define _H_mgr_io

/*  FILE: mgr_io.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Manager IO Handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
18-feb-07    abb      Begun; start from agt_ncxserver.h

*/

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

typedef enum mgr_io_returncode_t_ {
    MGR_IO_RC_NONE,
    MGR_IO_RC_IDLE,
    MGR_IO_RC_DROPPED,
    MGR_IO_RC_WANTDATA,
    MGR_IO_RC_PROCESSED,
    MGR_IO_RC_DROPPED_NOW
} mgr_io_returncode_t;


/* Manager application IO states */
typedef enum mgr_io_state_t_ {
    MGR_IO_ST_NONE,
    MGR_IO_ST_INIT,                         /* session starting up */
    MGR_IO_ST_IDLE,                      /* top-level wait for CLI */
    MGR_IO_ST_CONNECT,                 /* conn attempt in progress */
    MGR_IO_ST_CONN_START,           /* session waiting for <hello> */
    MGR_IO_ST_CONN_IDLE,   	     /* session ready for commands */
    MGR_IO_ST_CONN_RPYWAIT,            /* ses wait for <rpc-reply> */
    MGR_IO_ST_CONN_CANCELWAIT,  /* ses wait for cancel to complete */
    MGR_IO_ST_CONN_CLOSEWAIT,     /* wait for own session to close */
    MGR_IO_ST_CONN_SHUT,                  /* session shutting down */
    MGR_IO_ST_SHUT                    /* application shutting down */
} mgr_io_state_t;

/* Callback function for STDIN input 
 *
 * Handle any input from the user keyboard
 *
 * INPUTS:
 *    none
 * RETURNS:
 *    new manager IO state (upon exit from the function
 */
typedef mgr_io_state_t (*mgr_io_stdin_fn_t) (void);

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    mgr_io_init (void);

extern void
    mgr_io_set_stdin_handler (mgr_io_stdin_fn_t handler);

extern void
    mgr_io_activate_session (int fd);

extern void
    mgr_io_deactivate_session (int fd);

extern status_t 
    mgr_io_run (void);

extern boolean
    mgr_io_process_timeout (ses_id_t  cursid,
                            boolean *wantdata);

#endif	    /* _H_mgr_io */
