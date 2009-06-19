/*  FILE: mgr_ses.c

   NETCONF Session Manager: Manager Side Support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
19may05      abb      begun
08feb07      abb      start rewrite, using new NCX IO functions

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/

/*
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
*/

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <libssh2.h>

#ifndef _H_procdefs
#include "procdefs.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_mgr
#include "mgr.h"
#endif

#ifndef _H_mgr_hello
#include "mgr_hello.h"
#endif

#ifndef _H_mgr_io
#include "mgr_io.h"
#endif

#ifndef _H_mgr_rpc
#include "mgr_rpc.h"
#endif

#ifndef _H_mgr_ses
#include "mgr_ses.h"
#endif

#ifndef _H_mgr_top
#include "mgr_top.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_ses_msg
#include "ses_msg.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define MGR_SES_DEBUG 1
#define MGR_SES_SSH2_TRACE 1
#endif

/* maximum number of concurrent outbound sessions 
 * Must be >= 2 since session 0 is used for the dummy scb
 */
#define MGR_SES_MAX_SESSIONS     16


/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/
    

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static boolean    mgr_ses_init_done = FALSE;

static uint32     next_sesid;

static ses_cb_t  *mgrses[MGR_SES_MAX_SESSIONS];


/********************************************************************
* FUNCTION connect_to_agent
*
* Try to connect to the NETCONF server indicated by the host entry
* Only trying one address at this time
*
* Only supports the SSH transport at this time
*
* !!! hack -- hardwired to try only the default netconf-ssh port
*
* INPUTS:
*   scb == session control block
*   hent == host entry struct
*   port == port number to try, or 0 for defaults
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    connect_to_agent (ses_cb_t *scb,
		      struct hostent *hent,
		      uint16_t port)
{
    struct sockaddr_in  targ;
    int                 ret;
    boolean             userport;

    /* get a file descriptor for the new socket */
    scb->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (scb->fd == -1) {
	return ERR_NCX_RESOURCE_DENIED;
    }

    /* set non-blocking IO */
    if (fcntl(scb->fd, F_SETFD, O_NONBLOCK)) {
	if (LOGINFO) {
	    log_info("\nmgr_ses: fnctl failed");
	}
    }

    /* activate the socket in the select loop */
    mgr_io_activate_session(scb->fd);

    /* set the NETCONF agent address */
    memset(&targ, 0x0, sizeof(targ));
    targ.sin_family = AF_INET;
    memcpy((char *)&targ.sin_addr.s_addr,
	   hent->h_addr_list[0],
	   (size_t)hent->h_length);

    /* try to connect to user-provided port or port 830 */
    if (port) {
	userport = TRUE;
    } else {
	port = NCX_NCSSH_PORT;
	userport = FALSE;
    }
    targ.sin_port = htons(port);
    ret = connect(scb->fd, 
		  (struct sockaddr *)&targ,
		  sizeof(struct sockaddr_in));
    if (!ret) {
	return NO_ERR;
    }

    /* try SSH (port 22) next */
    if (!userport) {
	port = NCX_SSH_PORT;
	targ.sin_port = htons(port);
	ret = connect(scb->fd, 
		      (struct sockaddr *)&targ,
		      sizeof(struct sockaddr_in));
	if (!ret) {
	    return NO_ERR;
	}
    }

    /* de-activate the socket in the select loop */
    mgr_io_deactivate_session(scb->fd);

    return ERR_NCX_CONNECT_FAILED;

}  /* connect_to_agent */


/********************************************************************
* FUNCTION ssh2_setup
*
* Setup the SSH2 session and channel
* Connection MUST already established

* INPUTS:
*   scb == session control block
*   hent == host entry struct
*   user == username to connect as
*   password == password to authenticate with
*
* OUTPUTS:
*   scb->mgrcb fileds set for libssh2 data 
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    ssh2_setup (ses_cb_t *scb,
		const char *user,
		const char *password)
{
    mgr_scb_t  *mscb;
    const char *fingerprint;
    char       *userauthlist;
    int         ret;
    boolean     authdone;

    authdone = FALSE;
    mscb = mgr_ses_get_mscb(scb);

    mscb->session = libssh2_session_init();
    if (!mscb->session) {
	if (LOGINFO) {
	    log_info("\nmgr_ses: SSH2 new session failed");
	}
	return ERR_NCX_SESSION_FAILED;
    }

    ret = libssh2_session_startup(mscb->session, scb->fd);
    if (ret) {
	if (LOGINFO) {
	    log_info("\nmgr_ses: SSH2 establishment failed");
	}
	return ERR_NCX_SESSION_FAILED;
    }

#ifdef MGR_SES_SSH2_TRACE
    if (LOGDEBUG2) {
        libssh2_trace(mscb->session,
                      LIBSSH2_TRACE_TRANS |
                      LIBSSH2_TRACE_KEX |
                      LIBSSH2_TRACE_AUTH |
                      LIBSSH2_TRACE_CONN |
                      LIBSSH2_TRACE_SCP |
                      LIBSSH2_TRACE_SFTP |
                      LIBSSH2_TRACE_ERROR |
                      LIBSSH2_TRACE_PUBLICKEY);
    }
#endif

    /* 
     * this does not work because the AUTH phase will not
     * handle the EAGAIN errors correctly
     *
     * libssh2_session_set_blocking(mscb->session, 0); 
     */

    /* get the host fingerprint */
    fingerprint = libssh2_hostkey_hash(mscb->session, 
				       LIBSSH2_HOSTKEY_HASH_MD5);

    /* TBD: check fingerprint against known hosts files !!! */


    /* get the userauth info by sending an SSH_USERAUTH_NONE
     * request to the agent, hoping it will fail, and the
     * list of supported auth methods will be returned
     */
    userauthlist = libssh2_userauth_list(mscb->session,
					 user, 
					 strlen(user));
    if (!userauthlist) {
	/* check if the agent accepted NONE as an auth method */
	if (libssh2_userauth_authenticated(mscb->session)) {
	    if (LOGINFO) {
		log_info("\nmgr_ses: agent accepted SSH_AUTH_NONE");
	    }
	    authdone = TRUE;
	} else {
	    if (LOGINFO) {
		log_info("\nmgr_ses: SSH2 get authlist failed");
	    }
	    return ERR_NCX_SESSION_FAILED;
	}
    }

    if (LOGDEBUG2) {
	log_debug2("\nmgr_ses: Got agent authentication methods: %s\n", 
		   userauthlist);
    }

    if (!authdone) {

	if (strstr(userauthlist, "publickey") != NULL) {
	    ; /***/
	}

	if (!authdone && strstr(userauthlist, "password") != NULL) {
	    ret = libssh2_userauth_password(mscb->session, user, password);
	    if (ret) {
		if (LOGINFO) {
		    log_info("\nmgr_ses: SSH2 password authentication failed");
		}
		return ERR_NCX_AUTH_FAILED;
	    } else {
		authdone = TRUE;
	    }
	}

    }

    if (!authdone) {
	if (LOGINFO) {
	    log_info("\nmgr_ses: No supported authentication methods found!\n");
	}
	return ERR_NCX_AUTH_FAILED;
    }

    /* Request a shell */
    mscb->channel = libssh2_channel_open_session(mscb->session);
    if (!mscb->channel) {
	if (LOGINFO) {
	    log_info("\nmgr_ses: SSH2 channel open failed");
	}
	return ERR_NCX_SESSION_FAILED;
    }

    /* Connect to the NETCONF subsystem */
    ret = libssh2_channel_subsystem(mscb->channel, "netconf");
    if (ret) {
	if (LOGINFO) {
	    log_info("\nmgr_ses: Unable to request netconf subsystem");
	}
	return ERR_NCX_SESSION_FAILED;
    }

    /* set IO mode to non-blocking;
     * the mgr_io module will use 'select' to control read/write
     * to the TCP connection, but the libssh2 IO functions will
     * still be used
     */
    libssh2_channel_set_blocking(mscb->channel, 0);

    libssh2_channel_handle_extended_data2
        (mscb->channel,
         LIBSSH2_CHANNEL_EXTENDED_DATA_MERGE);

    return NO_ERR;

}  /* ssh2_setup */


/********************************************************************
* FUNCTION log_ssh2_error
*
* Get the ssh2 error message and print to error log
*
* INPUTS:
*   scb == session control block to use
*   mscb == manager control block to use
*   operation == string to use for operation (read, write, open)
*
* RETURNS:
*   number of bytes read; -1  for error; 0 for connection closed
*********************************************************************/
static void
    log_ssh2_error (ses_cb_t *scb,
                    mgr_scb_t  *mscb,
                    const char *operation)

{
    char      *errormsg;
    int        errcode;

    errcode = 0;
    errormsg = NULL;

    errcode = libssh2_session_last_error(mscb->session,
                                         &errormsg,
                                         NULL,
                                         0);

    if (errcode != LIBSSH2_ERROR_EAGAIN) {
        log_error("\nmgr_ses: channel %s failed on session %u (a:%u)",
                  operation,
                  scb->sid,
                  mscb->agtsid);
        if (LOGINFO) {
            log_info(" (%d:%s)",
                     errcode,
                     (errormsg) ? errormsg : "--");
        }
    }

}  /* log_ssh2_error */


/************ E X T E R N A L   F U N C T I O N S *****************/


/********************************************************************
* FUNCTION mgr_ses_init
*
* Initialize the session manager module data structures
*
* INPUTS:
*   none
* RETURNS:
*   none
*********************************************************************/
void 
    mgr_ses_init (void)
{
    uint32 i;

    if (!mgr_ses_init_done) {
	for (i=0; i<MGR_SES_MAX_SESSIONS; i++) {
	    mgrses[i] = NULL;
	}
	next_sesid = 1;
	mgr_ses_init_done = TRUE;
    }

}  /* mgr_ses_init */


/********************************************************************
* FUNCTION mgr_ses_cleanup
*
* Cleanup the session manager module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    mgr_ses_cleanup (void)
{
    uint32 i;

    if (mgr_ses_init_done) {
	for (i=0; i<MGR_SES_MAX_SESSIONS; i++) {
	    if (mgrses[i]) {
		mgr_ses_free_session(i);
	    }
	}
	next_sesid = 0;
	mgr_ses_init_done = FALSE;
    }

}  /* mgr_ses_cleanup */


/********************************************************************
* FUNCTION mgr_ses_new_session
*
* Create a new session control block and
* start a NETCONF session with to the specified agent
*
* After this functions returns OK, the session state will
* be in HELLO_WAIT state. An agent <hello> must be received
* before any <rpc> requests can be sent
*
* INPUTS:
*   user == user name
*   password == user password
*   target == ASCII IP address or DNS hostname of target
*   port == NETCONF port number to use, or 0 to use defaults
*   retsid == address of session ID output
*
* OUTPUTS:
*   *retsid == session ID, if no error
*
* RETURNS:
*   status
*********************************************************************/
status_t
    mgr_ses_new_session (const xmlChar *user,
			 const xmlChar *password,
			 const xmlChar *target,
			 uint16 port,
			 ses_id_t *retsid)
{
    ses_cb_t  *scb;
    mgr_scb_t *mscb;
    status_t   res;
    struct hostent *hent;
    uint       i, slot;

#ifdef DEBUG
    if (!user || !password || !target || !retsid) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    if (!mgr_ses_init_done) {
	mgr_ses_init();
    }

    *retsid = 0;

    /* check if any sessions are available */
    if (!next_sesid) {
	/* end has already been reached, so now in brute force
	 * session reclaim mode
	 */
	slot = 0;
	for (i=1; i<MGR_SES_MAX_SESSIONS && !slot; i++) {
	    if (!mgrses[i]) {
		slot = i;
	    }
	}
    } else {
	slot = next_sesid;
    }
    if (!slot) {
	/* max concurrent sessions reached */
	return ERR_NCX_RESOURCE_DENIED;
    }

    /* make sure there is memory for a session control block */
    scb = ses_new_scb();
    if (!scb) {
	return ERR_INTERNAL_MEM;
    }

    /* get the memory for the manager SCB */
    scb->mgrcb = mscb = mgr_new_scb();
    if (!mscb) {
	ses_free_scb(scb);
	return ERR_INTERNAL_MEM;
    }

    /* make sure at least 1 outbuff buffer is available */
    res = ses_msg_new_buff(scb, &scb->outbuff);
    if (res != NO_ERR) {
	mgr_free_scb(scb->mgrcb);
	scb->mgrcb = NULL;
	ses_free_scb(scb);
	return res;
    }

    /* initialize the static vars */
    scb->type = SES_TYP_NETCONF;
    scb->transport = SES_TRANSPORT_SSH;
    scb->state = SES_ST_INIT;
    scb->mode = SES_MODE_XML;
    scb->state = SES_ST_INIT;
    scb->instate = SES_INST_IDLE;
    scb->rdfn = mgr_ses_readfn;
    scb->wrfn = mgr_ses_writefn;

    /* not important if the user name is missing on the manager */
    scb->username = xml_strdup((const xmlChar *)user);
    mscb->target = xml_strdup(target);

    /* get the hostname for the specified target */
    hent = gethostbyname((const char *)target);
    if (hent) {
	/* entry OK, try to get a TCP connection to agent */
	res = connect_to_agent(scb, hent, port);
    } else {
	res = ERR_NCX_UNKNOWN_HOST;
    } 

    /* if TCP OK, get an SSH connection and channel */
    if (res == NO_ERR) {
	res = ssh2_setup(scb, 
                         (const char *)user,
			 (const char *)password);
    }

    if (res != NO_ERR) {
	mgr_free_scb(scb->mgrcb);
	scb->mgrcb = NULL;
	ses_free_scb(scb);
	return res;
    }

    /* do not have a session-ID from the agent yet 
     * so use the address of the scb in the ready blocks
     */
    scb->sid = slot;
    scb->inready.sid = slot;
    scb->outready.sid = slot;

    /* add the FD to SCB mapping in the definition registry */
    if (res == NO_ERR) {
	res = def_reg_add_scb(scb->fd, scb);
    }

    /* send the manager hello to the agent */
    if (res == NO_ERR) {
	res = mgr_hello_send(scb);
    }

    if (res != NO_ERR) {
	mgr_free_scb(scb->mgrcb);
	scb->mgrcb = NULL;
	ses_free_scb(scb);
	scb = NULL;
    } else {
	scb->state = SES_ST_HELLO_WAIT;
	mgrses[slot] = scb;
	if (++next_sesid==MGR_SES_MAX_SESSIONS) {
	    /* reached the end, start hunting for open slots instead  */
	    next_sesid = 0;
	}
    }

    *retsid = slot;
    return res;

}  /* mgr_ses_new_session */


/********************************************************************
* FUNCTION mgr_ses_free_session
*
* Free a real session control block
*
* INPUTS:
*   scb == session control block to free
*
*********************************************************************/
void
    mgr_ses_free_session (ses_id_t sid)
{

    ses_cb_t  *scb;

#ifdef DEBUG
    if (!mgr_ses_init_done) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    /* check if the session exists */
    scb = mgrses[sid];
    if (!scb) {
	if (LOGINFO) {
	    log_info("\nmgr_ses: delete invalid session (%d)", sid);
	}
	return;
    }

    /* close the session before deactivating the IO */
    if (scb->mgrcb) {
	mgr_free_scb(scb->mgrcb);
	scb->mgrcb = NULL;
    }

    /* deactivate the session IO */
    if (scb->fd) {
	mgr_io_deactivate_session(scb->fd);
	def_reg_del_scb(scb->fd);
    }

    ses_free_scb(scb);
    mgrses[sid] = NULL;

}  /* mgr_ses_free_session */


/********************************************************************
* FUNCTION mgr_ses_process_first_ready
*
* Check the readyQ and process the first message, if any
*
* RETURNS:
*     TRUE if a message was processed
*     FALSE if the readyQ was empty
*********************************************************************/
boolean
    mgr_ses_process_first_ready (void)
{
    ses_cb_t     *scb;
    ses_ready_t  *rdy;
    ses_msg_t    *msg;
    status_t      res;
#ifdef MGR_SES_DEBUG
    xmlChar       buff[32];
    uint32        cnt;
#endif

    rdy = ses_msg_get_first_inready();
    if (!rdy) {
	return FALSE;
    }

    /* get the session control block that rdy is embedded into */
    scb = mgrses[rdy->sid];
    if (!scb) {
	return FALSE;
    }

#ifdef MGR_SES_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nmgr_ses: msg ready for session");
    }
#endif

    /* check the session control block state */
    if (scb->state >= SES_ST_SHUTDOWN_REQ) {
	/* don't process the message or even it mark it
	 * It will be cleaned up when the session is freed
	 */
	return TRUE;
    }

#ifdef MGR_SES_DEBUG
    /* make sure a message is really there */
    msg = (ses_msg_t *)dlq_firstEntry(&scb->msgQ);
    if (!msg || !msg->ready) {
	SET_ERROR(ERR_INTERNAL_PTR);
	log_error("\nmgr_ses: ready Q message not correct");
	return FALSE;
    } else if (LOGDEBUG2) {
	cnt = xml_strcpy(buff, (const xmlChar *)"Incoming msg for session ");
	sprintf((char *)(&buff[cnt]), "%u", scb->sid);
	ses_msg_dump(msg, buff);
    }
#endif

    /* setup the XML parser */
    if (scb->reader) {
	/* reset the xmlreader */
	res = xml_reset_reader_for_session(ses_read_cb,
					   NULL, scb, scb->reader);
    } else {
	res = xml_get_reader_for_session(ses_read_cb,
					 NULL, scb, &scb->reader);
    }

    /* process the message */
    if (res == NO_ERR) {
	/* process the message */
	mgr_top_dispatch_msg(scb);
    } else {
	log_error("\nReset xmlreader failed for session");
	scb->state = SES_ST_SHUTDOWN_REQ;
    }

    /* free the message that was just processed */
    msg = (ses_msg_t *)dlq_deque(&scb->msgQ);
    if (msg) {
	ses_msg_free_msg(scb, msg);
    }

    /* check if any messages left for this session */
    if (!dlq_empty(&scb->msgQ)) {
	ses_msg_make_inready(scb);
    }

    return TRUE;
    
}  /* mgr_ses_process_first_ready */


/********************************************************************
* FUNCTION mgr_ses_fill_writeset
*
* Drain the ses_msg outreadyQ and set the specified fdset
* Used by mgr_ncxserver write_fd_set
*
* INPUTS:
*    fdset == pointer to fd_set to fill
*    maxfdnum == pointer to max fd int to fill in
*
* OUTPUTS:
*    *fdset is updated in
*    *maxfdnum may be updated
*
* RETURNS:
*    number of ready-for-output sessions found
*********************************************************************/
uint32
    mgr_ses_fill_writeset (fd_set *fdset,
			   int *maxfdnum)
{
    ses_ready_t *rdy;
    ses_cb_t    *scb;
    boolean      done;
    uint32       cnt;

    cnt = 0;
    FD_ZERO(fdset);
    done = FALSE;
    while (!done) {
	rdy = ses_msg_get_first_outready();
	if (!rdy) {
	    done = TRUE;
	} else {
	    scb = mgrses[rdy->sid];
	    if (scb && scb->state < SES_ST_SHUTDOWN_REQ) {
		cnt++;
		FD_SET(scb->fd, fdset);
		if (scb->fd > *maxfdnum) {
		    *maxfdnum = scb->fd;
		}
	    }
	}
    }
    return cnt;

}  /* mgr_ses_fill_writeset */


/********************************************************************
* FUNCTION mgr_ses_get_first_outready
*
* Get the first ses_msg outreadyQ entry
* Will remove the entry from the Q
*
* RETURNS:
*    pointer to the session control block of the
*    first session ready to write output to a socket
*    or the screen or a file
*********************************************************************/
ses_cb_t *
    mgr_ses_get_first_outready (void)
{
    ses_ready_t *rdy;
    ses_cb_t    *scb;

    rdy = ses_msg_get_first_outready();
    if (rdy) {
        scb = mgrses[rdy->sid];
        if (scb && scb->state < SES_ST_SHUTDOWN_REQ) {
            return scb;
        }
    }
    return NULL;

}  /* mgr_ses_get_first_outready */


/********************************************************************
* FUNCTION mgr_ses_get_first_session
*
* Get the first active session
*
* RETURNS:
*    pointer to the session control block of the
*    first active session found
*********************************************************************/
ses_cb_t *
    mgr_ses_get_first_session (void)
{
    ses_cb_t    *scb;
    uint32       i;

    /* skip zero (dummy) session */
    for (i = 1; i < MGR_SES_MAX_SESSIONS; i++) {
        scb = mgrses[i];
        if (scb && scb->state < SES_ST_SHUTDOWN_REQ) {
            return scb;
        }
    }
    return NULL;

}  /* mgr_ses_get_first_session */


/********************************************************************
* FUNCTION mgr_ses_get_next_session
*
* Get the next active session
*
* INPUTS:
*   curscb == current session control block to use
*
* RETURNS:
*    pointer to the session control block of the
*    next active session found
*
*********************************************************************/
ses_cb_t *
    mgr_ses_get_next_session (ses_cb_t *curscb)
{
    ses_cb_t    *scb;
    uint32       i;

    for (i = curscb->sid+1; i < MGR_SES_MAX_SESSIONS; i++) {
        scb = mgrses[i];
        if (scb && scb->state < SES_ST_SHUTDOWN_REQ) {
            return scb;
        }
    }
    return NULL;

}  /* mgr_ses_get_next_session */


/********************************************************************
* FUNCTION mgr_ses_readfn
*
* Read callback function for a manager SSH2 session
*
* INPUTS:
*   s == session control block
*   buff == buffer to fill
*   bufflen == length of buff
*   erragain == address of return EAGAIN flag
*
* OUTPUTS:
*   *erragain == TRUE if ret value < zero means 
*                read would have blocked (not really an error)
*                FALSE if the ret value is not < zero, or
*                if it is, it is a real error
*
* RETURNS:
*   number of bytes read; -1 for error; 0 for connection closed
*********************************************************************/
ssize_t 
    mgr_ses_readfn (void *s,
		    char *buff,
		    size_t bufflen,
                    boolean *erragain)
{
    ses_cb_t  *scb;
    mgr_scb_t *mscb;
    int        ret, errcode;
    char      *errormsg;

#ifdef DEBUG
    if (!s || !buff || !erragain) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return (ssize_t) -1;
    }
#endif

    errcode = 0;
    errormsg = NULL;
    *erragain = FALSE;

    scb = (ses_cb_t *)s;
    mscb = mgr_ses_get_mscb(scb);

    ret = libssh2_channel_read(mscb->channel, buff, bufflen);
    mscb->returncode = ret;
    if (ret < 0) {
        if (ret == LIBSSH2_ERROR_EAGAIN) {
            *erragain = TRUE;
#if 0
            if (LOGDEBUG3) {
                log_debug3("\nmgr_ses: channel read EAGAIN "
                           "on session %u (a:%u)",
                           scb->sid,
                           mscb->agtsid);
            }
#endif
        } else {
            log_ssh2_error(scb, mscb, "read");
        }
    } else if (ret > 0) {
	if (LOGDEBUG2) {
	    log_debug2("\nmgr_ses: channel read %d bytes OK"
		       "on session %u (a:%u)",
		       ret, 
		       scb->sid,
		       mscb->agtsid);
	}
    } else {
	if (LOGDEBUG2) {
	    log_debug2("\nmgr_ses: channel closed on session %u (a:%u)", 
		       scb->sid,
		       mscb->agtsid);
	}
    }

    return (ssize_t)ret;

}  /* mgr_ses_readfn */


/********************************************************************
* FUNCTION mgr_ses_writefn
*
* Write callback function for a manager SSH2 session
*
* INPUTS:
*   s == session control block
*
* RETURNS:
*   status of the write operation
*********************************************************************/
status_t
    mgr_ses_writefn (void *s)
{
    ses_cb_t          *scb;
    mgr_scb_t         *mscb;
    ses_msg_buff_t    *buff;
    int                ret;
    uint32             i;
    boolean            done;
    status_t           res;

    scb = (ses_cb_t *)s;
    mscb = mgr_ses_get_mscb(scb);
    ret = 0;
    res = NO_ERR;

    /* go through buffer outQ */
    buff = (ses_msg_buff_t *)dlq_deque(&scb->outQ);

    if (!buff) {
	if (LOGINFO) {
	    log_info("\nmgr_ses: channel write no out buffer");
	}
    }

    while (buff) {
        done = FALSE;
        while (!done) {
            ret = libssh2_channel_write(mscb->channel, 
                                        (char *)buff->buff, 
                                        buff->bufflen);
            if (ret < 0 || ret != (int)buff->bufflen) {
                if (ret == LIBSSH2_ERROR_EAGAIN) {
                    /* not done; sleep and try again */
                    usleep(1000);   /* 1000 micro-seconds */
                    continue;
                }
                log_ssh2_error(scb, mscb, "write");
            }
            done = TRUE;
        }

        if (ret == 0) {
            res = ERR_NCX_SESSION_CLOSED;
        } else if (ret > 0) {
	    if (LOGDEBUG2) {
		log_debug2("\nmgr_ses: channel write %u bytes OK",
			   buff->bufflen);
	    }
	    if (LOGDEBUG3) {
		for (i=0; i < buff->bufflen; i++) {
		    log_debug3("%c", buff->buff[i]);
		}
	    }
	} else {
            res = ERR_NCX_OPERATION_FAILED;
        }

	ses_msg_free_buff(scb, buff);

        if (res == NO_ERR) {
            buff = (ses_msg_buff_t *)dlq_deque(&scb->outQ);
        } else {
            buff = NULL;
        }
    }

    return res;

}  /* mgr_ses_writefn */


/********************************************************************
* FUNCTION mgr_ses_get_scb
*
* Retrieve the session control block
*
* INPUTS:
*   sid == manager session ID (not agent assigned ID)
*
* RETURNS:
*   pointer to session control block or NULL if invalid
*********************************************************************/
ses_cb_t *
    mgr_ses_get_scb (ses_id_t sid)
{
    if (sid==0 || sid>=MGR_SES_MAX_SESSIONS) {
	return NULL;
    }
    return mgrses[sid];

}  /* mgr_ses_get_scb */


/********************************************************************
* FUNCTION mgr_ses_get_mscb
*
* Retrieve the manager session control block
*
* INPUTS:
*   scb == session control block to use
*
* RETURNS:
*   pointer to manager session control block or NULL if invalid
*********************************************************************/
mgr_scb_t *
    mgr_ses_get_mscb (ses_cb_t *scb)
{
#ifdef DEBUG
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (mgr_scb_t *)scb->mgrcb;

}  /* mgr_ses_get_mscb */


/* END file mgr_ses.c */
