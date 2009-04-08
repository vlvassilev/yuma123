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
 *                                                                   *
 *                       C O N S T A N T S                           *
 *                                                                   *
 *********************************************************************/

/* how often to check for agent shutown (in seconds) */
#define MGR_IO_TIMEOUT  1


static mgr_io_stdin_fn_t  stdin_handler;

static fd_set active_fd_set, read_fd_set, write_fd_set;

static int maxwrnum;
static int maxrdnum;



/********************************************************************
 * FUNCTION any_fd_set
 * 
 * Check if any bits are set in the fd_set
 * INPUTS:
 *   fs == fs_set to check
 *   maxfd == max FD number possibly in use
 *
 * RETURNS:
 *   TRUE if any bits set
 *   FALSE if no bits set
 *********************************************************************/
static boolean
    any_fd_set (fd_set *fd,
		int maxfd)
{
    int  i;

    for (i=0; i<=maxfd; i++) {
	if (FD_ISSET(i, fd)) {
	    return TRUE;
	}
    }
    return FALSE;

} /* any_fd_set */

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
    FD_ZERO(&write_fd_set);
    FD_ZERO(&active_fd_set);
    maxwrnum = 0;
    maxrdnum = 0;

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
 * FUNCTION mgr_io_activate_session
 * 
 * Tell the IO manager to start listening on the specified socket
 * 
 * INPUTS:
 *   fd == file descriptor number of the socket
 *********************************************************************/
void
    mgr_io_activate_session (int fd)
{
    FD_SET(fd, &active_fd_set);
    if (fd+1 > maxwrnum) {
	maxwrnum = fd+1;
    }
    if (fd+1 > maxrdnum) {
	maxrdnum = fd+1;
    }

} /* mgr_io_activate_session */


/********************************************************************
 * FUNCTION mgr_io_deactivate_session
 * 
 * Tell the IO manager to stop listening on the specified socket
 * 
 * INPUTS:
 *   fd == file descriptor number of the socket
 *********************************************************************/
void
    mgr_io_deactivate_session (int fd)
{
    FD_CLR(fd, &active_fd_set);
    if (fd+1 == maxwrnum) {
	maxwrnum--;
    }
    if (fd+1 == maxrdnum) {
	maxrdnum--;
    }

} /* mgr_io_deactivate_session */


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
    ses_cb_t      *scb;
    fd_set         write_copy;
    struct timeval timeout;
    int            i, ret;
    status_t       res;
    boolean        done, done2, first;
    uint32         cnt;
    mgr_io_state_t state;


    /* first loop, handle user IO and get some data to read or write */
    done = FALSE;
    while (!done) {

	/* check exit program */
	if (mgr_shutdown_requested()) {
	    done = TRUE;
	    continue;
	}


	first = TRUE;
	done2 = FALSE;
	ret = 0;
	while (!done2) {
	    /* will block in idle states waiting for user KBD input
	     * while no command is active
	     */
	    if (stdin_handler) {
		state = (*stdin_handler)();
	    }

	    /* get the write fd_set once, since this call will empty
	     * the outreadyQ in ses_msg.c
	     */
	    cnt = 0;
	    if (first) {
		cnt = mgr_ses_fill_writeset(&write_copy, &maxwrnum);
		if (cnt) {
		    first = FALSE;
		}
	    }

	    /* check exit program */
	    if (mgr_shutdown_requested()) {
		done2 = done = TRUE;
		continue;
	    }

	    /* check no output and not expecting any reply 
	     * Note that this will not work when notifications
	     * are supported!!!  Need to run the IO-receive
	     * every N times when there is a session active
	     */
	    if (!cnt) {
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
		}
	    }

	    /* check error exit from this loop */
	    if (done2) {
		continue;
	    }

	    /* setup select parameters */
	    read_fd_set = active_fd_set;
	    write_fd_set = write_copy;
	    timeout.tv_sec = MGR_IO_TIMEOUT;
	    timeout.tv_usec = 0;

	    /* check if there are no sessions active to wait for,
	     * so just go back to the STDIN handler
	     */
	    if (!(any_fd_set(&read_fd_set, maxrdnum) ||
		  any_fd_set(&write_fd_set, maxwrnum))) {
		continue;
	    }

	    /* Block until input arrives on one or more active sockets. 
	     * or the timer expires
	     */
	    ret = select(max(maxrdnum+1, maxwrnum+1), &read_fd_set, 
			 &write_fd_set, NULL, &timeout);
	    if (ret > 0) {
		done2 = TRUE;
	    } else if (ret < 0) {
		if (!(errno == EINTR || errno==EAGAIN)) {
		    done2 = TRUE;
		}
	    } else if (ret == 0) {
		/* should only happen if a timeout occurred */
		if (mgr_shutdown_requested()) {
		    done2 = TRUE; 
		}
	    } else {
		/* normal return with some bytes */
		done2 = TRUE;
	    }
	}

	/* check exit program */
	if (mgr_shutdown_requested()) {
	    done = TRUE;
	    continue;
	}

	/* check select return status for non-recoverable error */
	if (ret < 0) {
	    res = ERR_NCX_OPERATION_FAILED;
	    log_error("\nmgr_io select failed (%s)", 
		      strerror(errno));
	    mgr_request_shutdown();
	    done = TRUE;
	    continue;
	}
     
	/* 2nd loop: go through the file desciptor numbers and
	 * service all the sockets with input and/or output pending 
	 */
	done2 = FALSE;     /* used to quit-end-early-exit */
	for (i = 0; i <= max(maxrdnum, maxwrnum) && !done2; i++) {

	    /* check write output to agent */
	    if (FD_ISSET(i, &write_fd_set)) {
		/* try to send 1 packet worth of buffers for a session */
		scb = def_reg_find_scb(i);
		if (scb) {
		    /* check if anything to write */
		    if (!dlq_empty(&scb->outQ)) {
			res = ses_msg_send_buffs(scb);
			if (res != NO_ERR) {
			    log_info("\nmgr_io write failed; "
				     "closing session %d ", scb->sid);
			    mgr_ses_free_session(scb->sid);
			    scb = NULL;
			    FD_CLR(i, &active_fd_set);
			}
		    }

		    /* check if any buffers left over for next loop */
		    if (scb && !dlq_empty(&scb->outQ)) {
			ses_msg_make_outready(scb);
		    }
		}
	    }

	    /* check read input from agent */
	    if (FD_ISSET(i, &read_fd_set)) {
		/* Data arriving on an already-connected socket.
		 * Need to have the xmlreader for this session
		 * unless it is input from STDIO
		 */
		scb = def_reg_find_scb(i);
		if (scb) {
		    res = ses_accept_input(scb);
		    if (res != NO_ERR) {
			mgr_ses_free_session(scb->sid);
			FD_CLR(i, &active_fd_set);
			if (i >= maxrdnum) {
			    maxrdnum = i-1;
			}
			if (res != ERR_NCX_SESSION_CLOSED) {
			    log_info("\nmgr_io input failed"
				     " for session %d (%s)",
				     scb->sid, get_error_string(res));
			}
		    }
		}
	    }
	}

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
    }  /* end 2nd loop */

    /* all open client sockets will be closed as the sessions are
     * torn down, but the original ncxserver socket needs to be closed now
     */
    return NO_ERR;

}  /* mgr_io_run */



/* END mgr_io.c */



