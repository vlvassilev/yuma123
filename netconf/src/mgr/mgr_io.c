/*  FILE: mgr_io.c

     1) call mgr_io_init

     2) call mgr_io_set_stdin_handler

     3a) When a session socket has been created,
        call mgr_io_activate_session.
     3b) When a session socket is being closed,
        call mgr_io_deactivate_session.

     4) call mgr_io_run to loop until program exits

     Step 2 or 3 can occur N times, and be changed while
     mgr_io_run is active (i.e., via stdin_handler)

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
18-feb-07    abb      begun; start from agt_ncxserver.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

     
#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include  "def_reg.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_mgr
#include  "mgr.h"
#endif

#ifndef _H_mgr_io
#include "mgr_io.h"
#endif

#ifndef _H_mgr_ses
#include  "mgr_ses.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_ses_msg
#include  "ses_msg.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif


/********************************************************************
 *                                                                  *
 *                       V A R I A B L E S                          *
 *                                                                  *
 ********************************************************************/

static mgr_io_stdin_fn_t  stdin_handler;


/********************************************************************
 * FUNCTION write_sessions
 * 
 * Go through any sessions ready to write  and send
 * the buffers ready to send
 *
 *********************************************************************/
static void
    write_sessions (void)
{
    ses_cb_t      *scb;
    mgr_scb_t     *mscb;
    status_t       res;

    scb = mgr_ses_get_first_outready();
    while (scb) {
        res = ses_msg_send_buffs(scb);
        if (res != NO_ERR) {
            if (LOGINFO) {
                mscb = mgr_ses_get_mscb(scb);
                log_info("\nmgr_io write failed; "
                         "closing session %u (a:%u)", 
                         scb->sid,
                         mscb->agtsid);
            }
            mgr_ses_free_session(scb->sid);
            scb = NULL;
        }
        /* check if any buffers left over for next loop */
        if (scb && !dlq_empty(&scb->outQ)) {
            ses_msg_make_outready(scb);
        }

        scb = mgr_ses_get_first_outready();
    }

}  /* write_sessions */


/********************************************************************
 * FUNCTION read_sessions
 * 
 * Go through any sessions and check any ready to read
 *
 *********************************************************************/
static void
    read_sessions (void)
{
    ses_cb_t      *scb, *nextscb;
    mgr_scb_t     *mscb;
    status_t       res;


    /* check read input from agent */
    scb = mgr_ses_get_first_session();
    while (scb) {
        nextscb = mgr_ses_get_next_session(scb);
        res = ses_accept_input(scb);
        if (res != NO_ERR) {
            if (res != ERR_NCX_SESSION_CLOSED) {
                if (LOGINFO) {
                    mscb = mgr_ses_get_mscb(scb);
                    log_info("\nmgr_io input failed"
                             " for session %u (a:%u) (%s)",
                             scb->sid, 
                             mscb->agtsid,
                             get_error_string(res));
                }
            }
            mgr_ses_free_session(scb->sid);
        }
        scb = nextscb;
    }

}  /* read_sessions */


/********************************************************************
 * FUNCTION mgr_io_init
 * 
 * Init the IO server loop vars for the ncx manager
 * Must call this function before any other function can
 * be used from this module
 *********************************************************************/
void
    mgr_io_init (void)
{
    stdin_handler = NULL;

} /* mgr_io_init */


/********************************************************************
 * FUNCTION mgr_io_set_stdin_handler
 * 
 * Set the callback function for STDIN processing
 * 
 * INPUTS:
 *   handler == address of the STDIN handler function
 *********************************************************************/
void
    mgr_io_set_stdin_handler (mgr_io_stdin_fn_t handler)
{
    stdin_handler = handler;

} /* mgr_io_set_stdin_handler */


/********************************************************************
 * FUNCTION mgr_io_run
 * 
 * IO server loop for the ncx manager
 * 
 * RETURNS:
 *   status
 *********************************************************************/
status_t
    mgr_io_run (void)
{
    int            ret;
    boolean        done, done2;
    mgr_io_state_t state;

    /* first loop, handle user IO and get some data to read or write */
    done = FALSE;
    while (!done) {

	/* check exit program */
	if (mgr_shutdown_requested()) {
	    done = TRUE;
	    continue;
	}

	state = MGR_IO_ST_INIT;
	done2 = FALSE;
	ret = 0;

	while (!done2) {
	    /* will block in idle states waiting for user KBD input
	     * while no command is active
	     */
	    if (stdin_handler) {
		state = (*stdin_handler)();
	    }

	    /* check exit program or session */
	    if (mgr_shutdown_requested()) {
		done = done2 = TRUE;
		continue;
	    } else if (state == MGR_IO_ST_CONN_SHUT) {
                done2 = TRUE;
                continue;
            } else if (state == MGR_IO_ST_SHUT) {
                done = done2 = TRUE;
                continue;
            }

            write_sessions();

            switch (state) {
            case MGR_IO_ST_INIT:
            case MGR_IO_ST_IDLE:
            case MGR_IO_ST_CONN_IDLE:
                /* user didn't do anything at the KBD
                 * skip the IO loop and try again
                 */
                continue;
            case MGR_IO_ST_CONNECT:
            case MGR_IO_ST_CONN_START:
            case MGR_IO_ST_CONN_RPYWAIT:
            case MGR_IO_ST_CONN_CANCELWAIT:
            case MGR_IO_ST_CONN_CLOSEWAIT:
                /* reply expected */
                break;
            case MGR_IO_ST_CONN_SHUT:
                /* session shutdown in progress */
                done2 = TRUE;
                continue;
            case MGR_IO_ST_SHUT:
                /* shutdown in progress */
                done = done2 = TRUE;
                continue;
            default:
                SET_ERROR(ERR_INTERNAL_VAL);
                done = done2 = TRUE;
            }

	    /* check error exit from this loop */
	    if (done2) {
		continue;
	    }

            read_sessions();

            /* drain the ready queue before accepting new input */
            if (!done) {
                done2 = FALSE;
                while (!done2) {
                    if (!mgr_ses_process_first_ready()) {
                        done2 = TRUE;
                    } else if (mgr_shutdown_requested()) {
                        done = done2 = TRUE;
                    }
                }
            }
        }  /* end inner loop */
    }  /* end outer loop */

    /* all open client sockets will be closed as the sessions are
     * torn down, but the original ncxserver socket needs to be closed now
     */
    return NO_ERR;

}  /* mgr_io_run */


/********************************************************************
 * FUNCTION mgr_io_process_timeout
 * 
 * mini server loop while waiting for KBD input
 * 
 * INPUTS:
 *    cursid == current session ID to check
 *
 * RETURNS:
 *   TRUE if session alive or not confirmed
 *   FALSE if cursid confirmed dropped
 *********************************************************************/
    boolean
    mgr_io_process_timeout (ses_id_t  cursid)
{
    ses_cb_t      *scb;
    boolean        done;

    read_sessions();

    done = FALSE;
    while (!done) {
	if (!mgr_ses_process_first_ready()) {
	    done = TRUE;
	} else if (mgr_shutdown_requested()) {
	    done = TRUE;
	}
    }

    scb = mgr_ses_get_scb(cursid);
    return (scb) ? TRUE : FALSE;

}  /* mgr_io_process_timeout */


/* END mgr_io.c */



