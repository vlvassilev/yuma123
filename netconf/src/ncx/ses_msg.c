/*  FILE: ses_msg.c

   NETCONF Session Message Manager: Common Support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
06jun06      abb      begun;

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>
#include  <unistd.h>
#include  <errno.h>
#include  <sys/uio.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_send_buff
#include  "send_buff.h"
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

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* max number of buffers a session is allowed to cache in its freeQ */
#define MAX_FREE_MSGS  32


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
static boolean   ses_msg_init_done = FALSE;
static uint32    freecnt;
static dlq_hdr_t freeQ;
static dlq_hdr_t inreadyQ;
static dlq_hdr_t outreadyQ;

/********************************************************************
* FUNCTION ses_msg_init
*
* Initialize the session message manager module data structures
*
* INPUTS:
*   none
* RETURNS:
*   none
*********************************************************************/
void 
    ses_msg_init (void)
{
    if (!ses_msg_init_done) {
	freecnt = 0;
	dlq_createSQue(&freeQ);
	dlq_createSQue(&inreadyQ);
	dlq_createSQue(&outreadyQ);
	ses_msg_init_done = TRUE;
    }

}  /* ses_msg_init */


/********************************************************************
* FUNCTION ses_msg_cleanup
*
* Cleanup the session message manager module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    ses_msg_cleanup (void)
{
    ses_msg_t *msg;

    if (ses_msg_init_done) {
	while (!dlq_empty(&freeQ)) {
	    msg = (ses_msg_t *)dlq_deque(&freeQ);
	    /* these do not belong to any session and do not have
	     * any buffers, so just toss the memory instead of
	     * using ses_msg_free_msg
	     */
	    m__free(msg);
	}

	/* nothing malloced in these Qs now */
	memset(&freeQ, 0x0, sizeof(dlq_hdr_t));
	memset(&inreadyQ, 0x0, sizeof(dlq_hdr_t));
	memset(&outreadyQ, 0x0, sizeof(dlq_hdr_t));
	freecnt = 0;
	ses_msg_init_done = FALSE;
    }

}  /* ses_msg_cleanup */


/********************************************************************
* FUNCTION ses_msg_new_msg
*
* Malloc a new session message control header
*
* INPUTS:
*   scb == session control block to malloc a new message for
*   msg == address of ses_msg_t pointer that will be set
*
* OUTPUTS:
*   *msg == malloced session message struct (if NO_ERR return)
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ses_msg_new_msg (ses_cb_t *scb, 
		     ses_msg_t **msg)
{
    ses_msg_t *newmsg;

#ifdef DEBUG
    if (!scb || !msg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* try the freeQ first */
    newmsg = (ses_msg_t *)dlq_deque(&freeQ);
    if (newmsg) {
	freecnt--;
    } else {
	/* freeQ is empty, malloc a msg */
	newmsg = m__getObj(ses_msg_t);
	if (!newmsg) {
	    return ERR_INTERNAL_MEM;
	}
    }

    /* set the fields and exit */
    memset(newmsg, 0x0, sizeof(ses_msg_t));
    dlq_createSQue(&newmsg->buffQ);
    *msg = newmsg;
    return NO_ERR;

} /* ses_msg_new_msg */


/********************************************************************
* FUNCTION ses_msg_free_msg
*
* Free the session message and all its buffer chunks
*
* INPUTS:
*   scb == session control block owning the message
*   msg == message to free (already removed from any Q)
*
*********************************************************************/
void
    ses_msg_free_msg (ses_cb_t *scb,
		      ses_msg_t *msg)
{
    ses_msg_buff_t *buff;

#ifdef DEBUG
    if (!scb || !msg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(&msg->buffQ)) {
	buff = dlq_deque(&msg->buffQ);
	ses_msg_free_buff(scb, buff);
    }

    if (freecnt < MAX_FREE_MSGS) {
	dlq_enque(msg, &freeQ);
	freecnt++;
    } else {
	m__free(msg);
    }
	
} /* ses_msg_free_msg */


/********************************************************************
* FUNCTION ses_msg_new_buff
*
* Malloc a new session buffer chuck
*
* Note that the buffer memory is not cleared after each use
* since this is not needed for byte stream IO
*
* INPUTS:
*   scb == session control block to malloc a new message for
*   buff == address of ses_msg_buff_t pointer that will be set
*
* OUTPUTS:
*   *buff == malloced session buffer chunk (if NO_ERR return)
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ses_msg_new_buff (ses_cb_t *scb, 
		      ses_msg_buff_t **buff)
{
    ses_msg_buff_t *newbuff;

#ifdef DEBUG
    if (!scb || !buff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* handle the session freeQ separately */
    if (scb->freecnt) {
	newbuff = (ses_msg_buff_t *)dlq_deque(&scb->freeQ);
	if (newbuff) {
	    newbuff->bufflen = 0;
	    newbuff->buffpos = 0;
#ifdef DEBUG
	    memset(newbuff->buff, 0x0, SES_MSG_BUFFSIZE);
#endif
	    *buff = newbuff;
	    scb->freecnt--;
	    return NO_ERR;
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    scb->freecnt = 0;
	}
    }

    /* check buffers exceeded error */
    if (scb->buffcnt+1 >= SES_MAX_BUFFERS) {
	return ERR_NCX_RESOURCE_DENIED;
    }

    /* malloc the buffer */
    newbuff = m__getObj(ses_msg_buff_t);
    if (!newbuff) {
	return ERR_INTERNAL_MEM;
    }

    /* set the fields and exit */
    newbuff->bufflen = 0;
    newbuff->buffpos = 0;

#ifdef DEBUG
    memset(newbuff->buff, 0x0, SES_MSG_BUFFSIZE);
#endif

    *buff = newbuff;
    scb->buffcnt++;
    return NO_ERR;

} /* ses_new_buff */


/********************************************************************
* FUNCTION ses_msg_free_buff
*
* Free the session buffer chunk
*
* INPUTS:
*   scb == session control block owning the message
*   buff == buffer to free (already removed from any Q)
*
* RETURNS:
*   none
*********************************************************************/
void
    ses_msg_free_buff (ses_cb_t *scb,
		       ses_msg_buff_t *buff)
{
    if (scb->state < SES_ST_SHUTDOWN_REQ &&
	scb->freecnt < SES_MAX_FREE_BUFFERS) {
	dlq_enque(buff, &scb->freeQ);
	scb->freecnt++;
    } else {
	m__free(buff);
	scb->buffcnt--;
    }

} /* ses_msg_free_buff */


/********************************************************************
* FUNCTION ses_msg_write_buff
*
* Add some text to the message buffer
*
* INPUTS:
*   buff == buffer to write to
*   ch  == xmlChar to write
*
* RETURNS:
*   status_t
*
*********************************************************************/
status_t
    ses_msg_write_buff (ses_msg_buff_t *buff,
			uint32 ch)
{

#ifdef DEBUG
    if (!buff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (buff->bufflen < SES_MSG_BUFFSIZE) {
	buff->buff[buff->bufflen++] = (xmlChar)ch;
	return NO_ERR;
    } else {
	return ERR_BUFF_OVFL;
    }
    
} /* ses_msg_write_buff */


/********************************************************************
* FUNCTION ses_msg_send_buff
*
* Send a buffer to the session client socket
*
* INPUTS:
*   fd == session file descriptor number
*   buff == buffer to write to
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ses_msg_send_buff (int fd,
		       ses_msg_buff_t *buff)
{
    status_t  res;

#ifdef DEBUG
    if (!buff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!fd) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    if (!buff->bufflen) {
	return NO_ERR;
    }

    res = send_buff(fd, (const char *)buff->buff, buff->bufflen);
    buff->bufflen = 0;
    buff->buffpos = 0;
    return res;

} /* ses_msg_send_buff */


/********************************************************************
* FUNCTION ses_msg_send_buffs
*
* Send multiple buffers to the session client socket
* Tries to send one packet at maximum MTU
*
* INPUTS:
*   scb == session control block
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ses_msg_send_buffs (ses_cb_t *scb)
{
    ses_msg_buff_t  *buff;
    uint32    buffleft, total;
    ssize_t   retcnt;
    int       i, cnt;
    boolean   done, dologmsg;
    struct iovec iovs[SES_MAX_BUFFSEND];

#ifdef DEBUG
    if (!scb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    dologmsg = LOGDEBUG2;

    if (dologmsg) {
	log_debug2("\nses got send request on session %d", 
		   scb->sid);
    }

    /* check if an external write function is used */
    if (scb->wrfn) {
	buff = (ses_msg_buff_t *)dlq_firstEntry(&scb->outQ);
	if (buff) {
	    if (dologmsg) {
		log_debug2("\nses_msg_send_wrfn: first buff %s",
			   &buff->buff[buff->buffpos]);
	    }
	}
	return (*scb->wrfn)(scb);
    }

    memset(iovs, 0x0, sizeof(iovs));
    total = 0;
    cnt = 0;
    done = FALSE;
    buff = (ses_msg_buff_t *)dlq_firstEntry(&scb->outQ);

    /* setup the writev call */
    for (i=0; i<SES_MAX_BUFFSEND && !done && buff; i++) {
	buffleft = buff->bufflen - buff->buffpos;
	if ((total+buffleft) > SES_MAX_BYTESEND) {
	    done = TRUE;
	} else {
	    total += buffleft;
	    iovs[i].iov_base = &buff->buff[buff->buffpos];
	    iovs[i].iov_len = buffleft;
	    buff = (ses_msg_buff_t *)dlq_nextEntry(buff);

	    if (LOGDEBUG3) {
		log_debug3("\nses_msg: setup send buff %d\n%s\n", 
			   i,
			   iovs[i].iov_base);
	    }
	    cnt++;
	}
    }

    /* make sure there is at least one buffer set */
    if (!iovs[0].iov_base) {
	return SET_ERROR(ERR_NCX_OPERATION_FAILED);
    }

    /* write a packet to the session socket */
    retcnt = writev(scb->fd, iovs, cnt);
    if (retcnt < 0) {
	/* should not need retries because the select loop
	 * indicated this session was ready for output
	 */
	log_info("\nses msg write failed for session %d", scb->sid);
	return errno_to_status();
    } else {
	if (dologmsg) {
	    log_debug2("\nses wrote %d of %d bytes on session %d\n", 
		       retcnt, total, scb->sid);
	}
    }

    /* clean up the buffers that were written */
    buff = (ses_msg_buff_t *)dlq_firstEntry(&scb->outQ);

    if (buff) {
	if (LOGDEBUG3) {
	    ; /* buffers already printed */
	} else if (dologmsg) {
	    log_debug2("\nses_msg_send: first buff %s",
		       &buff->buff[buff->buffpos]);
	}
    }

    while (retcnt && buff) {
	/* get the number of bytes written from this buffer */
	buffleft = buff->bufflen - buff->buffpos;

	/* free the buffer if all of it was written or just
	 * bump the buffer pointer if not
	 */
	if ((uint32)retcnt >= buffleft) {
	    dlq_remove(buff);
	    ses_msg_free_buff(scb, buff);
	    retcnt -= (ssize_t)buffleft;
	    buff = (ses_msg_buff_t *)dlq_firstEntry(&scb->outQ);	    
	} else {
	    buff->buffpos += (uint32)retcnt;
	    retcnt = 0;
	}
    }

    return NO_ERR;

} /* ses_msg_send_buffs */


/********************************************************************
* FUNCTION ses_msg_new_output_buff
*
* Put the current outbuff on the outQ
* Put the session on the outreadyQ if it is not already there
* Try to allocate a new buffer for the session
*
* INPUTS:
*   scb == session control block
*
* OUTPUTS:
*   scb->outbuff, scb->outready, and scb->outQ will be changed
*   
* RETURNS:
*   status, could return malloc or buffers exceeded error
*********************************************************************/
status_t
    ses_msg_new_output_buff (ses_cb_t *scb)
{

#ifdef DEBUG
    if (!scb || !scb->outbuff) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    scb->outbuff->buffpos = 0;
    dlq_enque(scb->outbuff, &scb->outQ);
    ses_msg_make_outready(scb);
    scb->outbuff = NULL;
    return ses_msg_new_buff(scb, &scb->outbuff);

} /* ses_msg_new_output_buff */


/********************************************************************
* FUNCTION ses_msg_make_inready
*
* Put the session on the inreadyQ if it is not already there
*
* INPUTS:
*   scb == session control block
*
* OUTPUTS:
*   scb->inready will be queued on the inreadyQ
*********************************************************************/
void
    ses_msg_make_inready (ses_cb_t *scb)
{

#ifdef DEBUG
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (!scb->inready.inq) {
	dlq_enque(&scb->inready, &inreadyQ);
	scb->inready.inq = TRUE;
    }

} /* ses_msg_make_inready */


/********************************************************************
* FUNCTION ses_msg_make_outready
*
* Put the session on the outreadyQ if it is not already there
*
* INPUTS:
*   scb == session control block
*
* OUTPUTS:
*   scb->outready will be queued on the outreadyQ
*********************************************************************/
void
    ses_msg_make_outready (ses_cb_t *scb)
{

#ifdef DEBUG
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (!scb->outready.inq) {
	dlq_enque(&scb->outready, &outreadyQ);
	scb->outready.inq = TRUE;
    }

} /* ses_msg_make_outready */


/********************************************************************
* FUNCTION ses_msg_finish_outmsg
*
* Put the outbuff in the outQ if non-empty
* Put the session on the outreadyQ if it is not already there
*
* INPUTS:
*   scb == session control block
*
* OUTPUTS:
*   scb->outready will be queued on the outreadyQ
*********************************************************************/
void
    ses_msg_finish_outmsg (ses_cb_t *scb)
{

#ifdef DEBUG
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (scb->outbuff && scb->outbuff->bufflen) {
	scb->outbuff->buffpos = 0;
	dlq_enque(scb->outbuff, &scb->outQ);
	scb->outbuff = NULL;
	(void)ses_msg_new_buff(scb, &scb->outbuff);
    }
    ses_msg_make_outready(scb);

} /* ses_msg_finish_outmsg */


/********************************************************************
* FUNCTION ses_msg_get_first_inready
*
* Dequeue the first entry in the inreadyQ, if any
*
* RETURNS:
*    first entry in the inreadyQ or NULL if none
*********************************************************************/
ses_ready_t *
    ses_msg_get_first_inready (void)
{
    ses_ready_t *rdy;

    rdy = (ses_ready_t *)dlq_deque(&inreadyQ);
    if (rdy) {
	rdy->inq = FALSE;
    }
    return rdy;

} /* ses_msg_get_first_inready */


/********************************************************************
* FUNCTION ses_msg_get_first_outready
*
* Dequeue the first entry in the outreadyQ, if any
*
* RETURNS:
*    first entry in the outreadyQ or NULL if none
*********************************************************************/
ses_ready_t *
    ses_msg_get_first_outready (void)
{
    ses_ready_t *rdy;

    rdy = (ses_ready_t *)dlq_deque(&outreadyQ);
    if (rdy) {
	rdy->inq = FALSE;
    }
    return rdy;

} /* ses_msg_get_first_outready */


/********************************************************************
* FUNCTION ses_msg_dump
*
* Dump the message contents
*
* INPUTS:
*   msg == message to dump
*   text == start text before message dump (may be NULL)
*
*********************************************************************/
void
    ses_msg_dump (const ses_msg_t *msg,
		  const xmlChar *text)
{
    const ses_msg_buff_t *buff;
    boolean anytext;
    uint32  i;

#ifdef DEBUG
    if (!msg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (text) {
	log_write("\n%s\n", text);
	anytext = TRUE;
    } else {
	anytext = FALSE;
    }

    for (buff = (const ses_msg_buff_t *)dlq_firstEntry(&msg->buffQ);
	 buff != NULL;
	 buff = (const ses_msg_buff_t *)dlq_nextEntry(buff)) {
	for (i=0; i<buff->bufflen; i++) {
	    log_write("%c", buff->buff[i]);
	}
	anytext = TRUE;
    }

    if (anytext) {
	log_write("\n");
    }

} /* ses_msg_dump */


/* END file ses_msg.c */
