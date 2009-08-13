/*  FILE: agt_ncxserver.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
11-jan-07    abb      begun; gathered from glibc documentation

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
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

#ifndef _H_agt
#include  "agt.h"
#endif

#ifndef _H_agt_ncxserver
#include "agt_ncxserver.h"
#endif

#ifndef _H_agt_not
#include  "agt_not.h"
#endif

#ifndef _H_agt_ses
#include  "agt_ses.h"
#endif

#ifndef _H_def_reg
#include  "def_reg.h"
#endif

#ifndef _H_log
#include  "log.h"
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
#ifdef DEBUG
#define AGT_NCXSERVER_DEBUG   1
#endif

/* how often to check for agent shutown (in seconds) */
#define AGT_NCXSERVER_TIMEOUT  1

/* number of notifications to send out in 1 timeout interval */
#define MAX_NOTIFICATION_BURST  10


static fd_set active_fd_set;
static fd_set read_fd_set;
static fd_set write_fd_set;


/********************************************************************
 * FUNCTION make_named_socket
 * 
 * Create an AF_LOCAL socket for the ncxserver
 * 
 * INPUTS:
 *    filename == full filespec of the socket filename
 *    sock == ptr to return value
 *
 * OUTPUTS:
 *    *sock == the FD for the socket if return ok
 *
 * RETURNS:
 *    status   
 *********************************************************************/
static status_t
    make_named_socket (const char *filename, int *sock)
{
    int ret;
    size_t  size;
    struct sockaddr_un name;

    *sock = 0;

    /* Create the socket. */
    *sock = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (*sock < 0) {
        perror ("socket");
        return ERR_NCX_OPERATION_FAILED;
    }

    /* Give the socket a name. */
    name.sun_family = AF_LOCAL;
    strncpy(name.sun_path, filename, sizeof(name.sun_path));
    size = SUN_LEN(&name);
    ret = bind(*sock, (struct sockaddr *)&name, size);
    if (ret != 0) {
        perror ("bind");
        return ERR_NCX_OPERATION_FAILED;
    }

    return NO_ERR;
} /* make_named_socket */


/********************************************************************
 * FUNCTION send_some_notifications
 * 
 * Send some notifications as needed
 * 
 * INPUTS:
 *    filename == full filespec of the socket filename
 *    sock == ptr to return value
 *
 * OUTPUTS:
 *    *sock == the FD for the socket if return ok
 *
 * RETURNS:
 *    status   
 *********************************************************************/
static void
    send_some_notifications (void)
{
    const agt_profile_t  *agt_profile;
    uint32                sendcount, sendtotal, sendmax;
    boolean               done;

    sendcount = 0;
    sendtotal = 0;

    /* TBD: set to CLI param */
    agt_profile = agt_get_profile();
    sendmax = agt_profile->agt_maxburst;

    done = FALSE;
    while (!done) {
        sendcount = agt_not_send_notifications();
        if (sendcount) {
            sendtotal += sendcount;
            if (sendmax && (sendtotal >= sendmax)) {
                done = TRUE;
            }
        } else {
            done = TRUE;
        }
    }

    if (agt_profile->agt_eventlog_size == 0) {
        agt_not_clean_eventlog();
    }

} /* send_some_notifications */



/***********     E X P O R T E D   F U N C T I O N S   *************/


/********************************************************************
 * FUNCTION agt_ncxserver_run
 * 
 * IO server loop for the ncxserver socket
 * 
 * RETURNS:
 *   status
 *********************************************************************/
status_t
    agt_ncxserver_run (void)
{
    ses_cb_t *scb;
    int ncxsock, maxwrnum, maxrdnum;
    int i, new, ret;
    struct sockaddr_un clientname;
    struct timeval timeout;
    socklen_t size;
    status_t res;
    boolean done, done2;

    /* Create the socket and set it up to accept connections. */
    res = make_named_socket(NCXSERVER_SOCKNAME, &ncxsock);
    if (res != NO_ERR) {
        log_error("\n*** Cannot connect to ncxserver socket"
                  "\n*** If no other instances of netconfd are running,"
                  "\n*** try deleting /tmp/ncxserver.sock\n");
        return res;
    }

    if (listen(ncxsock, 1) < 0) {
        log_error("\nError: listen failed");
        return ERR_NCX_OPERATION_FAILED;
    }
     
    /* Initialize the set of active sockets. */
    FD_ZERO(&read_fd_set);
    FD_ZERO(&write_fd_set);
    FD_ZERO(&active_fd_set);
    FD_SET(ncxsock, &active_fd_set);
    maxwrnum = maxrdnum = ncxsock;

    done = FALSE;
    while (!done) {

        /* check exit program */
        if (agt_shutdown_requested()) {
            done = TRUE;
            continue;
        }

        ret = 0;
        done2 = FALSE;
        while (!done2) {
            read_fd_set = active_fd_set;
            agt_ses_fill_writeset(&write_fd_set, &maxwrnum);
            timeout.tv_sec = AGT_NCXSERVER_TIMEOUT;
            timeout.tv_usec = 0;

            /* Block until input arrives on one or more active sockets. 
             * or the timer expires
             */
            ret = select(max(maxrdnum+1, maxwrnum+1), 
                         &read_fd_set, 
                         &write_fd_set, 
                         NULL, 
                         &timeout);
            if (ret > 0) {
                done2 = TRUE;
            } else if (ret < 0) {
                if (!(errno == EINTR || errno==EAGAIN)) {
                    done2 = TRUE;
                }
            } else if (ret == 0) {
                /* should only happen if a timeout occurred */
                if (agt_shutdown_requested()) {
                    done2 = TRUE; 
                } else {
                    agt_ses_check_timeouts();
                    send_some_notifications();
                }
            } else {
                /* normal return with some bytes */
                done2 = TRUE;
            }
        }

        /* check exit program */
        if (agt_shutdown_requested()) {
            done = TRUE;
            continue;
        }

        /* check select return status for non-recoverable error */
        if (ret < 0) {
            res = ERR_NCX_OPERATION_FAILED;
            log_error("\nncxserver select failed (%s)", 
                      strerror(errno));
            agt_request_shutdown(NCX_SHUT_EXIT);
            done = TRUE;
            continue;
        }
     
        /* Service all the sockets with input and/or output pending */
        done2 = FALSE;
        for (i = 0; i < max(maxrdnum+1, maxwrnum+1) && !done2; i++) {

            /* check write output to client sessions */
            if (FD_ISSET(i, &write_fd_set)) {
                /* try to send 1 packet worth of buffers for a session */
                scb = def_reg_find_scb(i);
                if (scb) {
                    /* check if anything to write */
                    if (!dlq_empty(&scb->outQ)) {
                        res = ses_msg_send_buffs(scb);
                        if (res != NO_ERR) {
                            if (LOGINFO) {
                                log_info("\nagt_ncxserver write failed; "
                                         "closing session %d ", 
                                         scb->sid);
                            }
                            agt_ses_kill_session(scb->sid, 
                                                 scb->sid,
                                                 SES_TR_OTHER);
                            scb = NULL;
                        } else if (scb->state == SES_ST_SHUTDOWN_REQ) {
                            /* close-session reply sent, now kill ses */
                            agt_ses_kill_session(scb->sid, 
                                                 scb->killedbysid,
                                                 scb->termreason);
                            scb = NULL;
                        }
                    }

                    /* check if any buffers left over for next loop */
                    if (scb && !dlq_empty(&scb->outQ)) {
                        ses_msg_make_outready(scb);
                    }
                }
            }

            /* check read input from client sessions */
            if (FD_ISSET(i, &read_fd_set)) {
                if (i == ncxsock) {
                    /* Connection request on original socket. */
                    size = (socklen_t)sizeof(clientname);
                    new = accept(ncxsock,
                                 (struct sockaddr *)&clientname,
                                 &size);
                    if (new < 0) {
                        if (LOGINFO) {
                            log_info("\nagt_ncxserver accept "
                                     "connection failed (%d)",
                                     new);
                        }
                        continue;
                    }

                    /* get a new session control block */
                    if (!agt_ses_new_session(SES_TRANSPORT_SSH, new)) {
                        close(new);
                        if (LOGINFO) {
                            log_info("\nagt_ncxserver new "
                                     "session failed (%d)", 
                                     new);
                        }
                    } else {
                        /* set non-blocking IO */
                        if (fcntl(new, F_SETFD, O_NONBLOCK)) {
                            if (LOGINFO) {
                                log_info("\nfnctl failed");
                            }
                        }
                        FD_SET(new, &active_fd_set);
                        if (new > maxrdnum) {
                            maxrdnum = new;
                        }
                    }
                } else {
                    /* Data arriving on an already-connected socket.
                     * Need to have the xmlreader for this session
                     */
                    scb = def_reg_find_scb(i);
                    if (scb) {
                        res = ses_accept_input(scb);
                        if (res != NO_ERR) {
                            if (i >= maxrdnum) {
                                maxrdnum = i-1;
                            }
                            if (res != ERR_NCX_SESSION_CLOSED) {
                                if (LOGINFO) {
                                    log_info("\nagt_ncxserver: input failed"
                                             " for session %d (%s)",
                                             scb->sid, 
                                             get_error_string(res));
                                }
                            }
                            agt_ses_kill_session(scb->sid,
                                                 scb->sid,
                                                 SES_TR_DROPPED);

                        } 
                    }
                }
            }
        }

        /* drain the ready queue before accepting new input */
        if (!done) {
            done2 = FALSE;
            while (!done2) {
                if (!agt_ses_process_first_ready()) {
                    done2 = TRUE;
                } else if (agt_shutdown_requested()) {
                    done = done2 = TRUE;
                } else {
                    send_some_notifications();
                }
            }
        }
    }  /* end select loop */

    /* all open client sockets will be closed as the sessions are
     * torn down, but the original ncxserver socket needs to be closed now
     */
    close(ncxsock);
    unlink(NCXSERVER_SOCKNAME);
    return NO_ERR;

}  /* agt_ncxserver_run */


/********************************************************************
 * FUNCTION agt_ncxserver_clear_fd
 * 
 * Clear a dead session from the select loop
 * 
 * INPUTS:
 *   fd == file descriptor number for the socket to clear
 *********************************************************************/
void
    agt_ncxserver_clear_fd (int fd)
{
    FD_CLR(fd, &active_fd_set);

} /* agt_ncxserver_clear_fd */


/* END agt_ncxserver.c */



