/*
 * Copyright (c) 2009, Andy Bierman
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
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
29apr11      abb      add support for NETCONF:base:1.1 
                      message framing

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

/* clear buffers before use */
/* #define SES_MSG_CLEAR_INIT_BUFFERS 1 */

/* max number of buffers a session is allowed to cache in its freeQ */
#define MAX_FREE_MSGS  32


/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/
    

/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/
static boolean   ses_msg_init_done = FALSE;
static uint32    freecnt;
static dlq_hdr_t freeQ;
static dlq_hdr_t inreadyQ;
static dlq_hdr_t outreadyQ;


/********************************************************************
* FUNCTION do_send_buff
*
* Send the specified buffer.
* Add framing chars if needed for base:1.1 over SSH
*
* INPUTS:
*   scb == session control block to use
*   buff == buffer to send
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    do_send_buff (ses_cb_t *scb,
                  ses_msg_buff_t *buff)
{
    status_t   res = NO_ERR;

    if (scb->framing11) {
        ses_msg_add_framing(scb, buff);
        /* bufflen has been adjusted for buffstart */
        if (buff->bufflen > 0) {
            res = send_buff(scb->fd, 
                            (const char *)&buff->buff[buff->buffstart], 
                            buff->bufflen);
        }
    } else if (buff->bufflen > buff->buffstart) {
        /* send with base:1.0 framing; EOM markers in the buffer */
        res = send_buff(scb->fd, 
                        (const char *)buff->buff, 
                        buff->bufflen);
    } else if (LOGDEBUG2) {
        log_debug2("\nses: skip sending empty buffer on sesion '%d'",
                  scb->sid);
    }

    return res;

}  /* do_send_buff */


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
*   none
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
*   outbuff == TRUE if this is for outgoing message
*              FALSE if this is for incoming message
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
                      boolean outbuff,
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
            /* use buffer from freeQ */
            ses_msg_init_buff(scb, outbuff, newbuff);

#ifdef SES_MSG_CLEAR_INIT_BUFFERS
            memset(newbuff->buff, 0x0, SES_MSG_BUFFSIZE);
#endif

            *buff = newbuff;
            scb->freecnt--;

            if (LOGDEBUG4) {
                log_debug4("\nses_msg: reused %s buff %p for s %u", 
                           (outbuff) ? "out" : "in",
                           newbuff,
                           scb->sid);
            }

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
    if (newbuff == NULL) {
        return ERR_INTERNAL_MEM;
    }

    /* set the fields and exit */
    ses_msg_init_buff(scb, outbuff, newbuff);

#ifdef DEBUG
    memset(newbuff->buff, 0x0, SES_MSG_BUFFSIZE);
#endif

    *buff = newbuff;
    scb->buffcnt++;

    if (LOGDEBUG4) {
        log_debug4("\nses_msg: new %s buff %p for s %u", 
                   (outbuff) ? "out" : "in",
                   newbuff,
                   scb->sid);
    }

    return NO_ERR;

} /* ses_msg_new_buff */


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

        if (LOGDEBUG4) {
            log_debug4("\nses_msg: cache buff %p for s %u", 
                       buff,
                       scb->sid);
        }
    } else {
        if (LOGDEBUG4) {
            log_debug4("\nses_msg: free buff %p for s %u", 
                       buff,
                       scb->sid);
        }
        m__free(buff);
        scb->buffcnt--;
    }

} /* ses_msg_free_buff */


/********************************************************************
* FUNCTION ses_msg_write_buff
*
* Add some text to the message buffer
*
* Upper layer code should never write framing chars to the
* output buff -- that is always done in this module.
* Use ses_finish_msg to cause framing chars to be written/
*
* INPUTS:
*   scb == session control block to use
*   buff == buffer to write to
*   ch  == xmlChar to write
*
* RETURNS:
*   status_t
*
*********************************************************************/
status_t
    ses_msg_write_buff (ses_cb_t *scb,
                        ses_msg_buff_t *buff,
                        uint32 ch)
{
    status_t   res;

#ifdef DEBUG
    if (buff == NULL) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    if (scb->framing11) {
        if (buff->bufflen < (SES_MSG_BUFFSIZE - SES_ENDCHUNK_PAD)) {
            buff->buff[buff->bufflen++] = (xmlChar)ch;
        } else {
            res = ERR_BUFF_OVFL;
        }
    } else {
        if (buff->bufflen < SES_MSG_BUFFSIZE) {
            buff->buff[buff->bufflen++] = (xmlChar)ch;
        } else {
            res = ERR_BUFF_OVFL;
        }
    }
    return res;
    
} /* ses_msg_write_buff */


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
    uint32           buffleft, total;
    ssize_t          retcnt;
    int              i, cnt;
    boolean          done, dologmsg;
    status_t         res;
    struct iovec     iovs[SES_MAX_BUFFSEND];

#ifdef DEBUG
    if (!scb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    dologmsg = LOGDEBUG2;

    if (LOGDEBUG) {
        log_debug("\nses got send request on session %d", 
                  scb->sid);
    }

    if (dologmsg) {
        buff = (ses_msg_buff_t *)dlq_firstEntry(&scb->outQ);
        if (buff) {
            if (LOGDEBUG3) {
                log_debug3("\nses_msg_send full msg:\n%s",
                           &buff->buff[buff->buffpos]);
                buff = (ses_msg_buff_t *)dlq_nextEntry(buff);
                while (buff != NULL) {
                    log_debug3("%s",
                               &buff->buff[buff->buffpos]);
                    buff = (ses_msg_buff_t *)dlq_nextEntry(buff);
                }
            } else {
                log_debug2("\nses_msg_send first buffer:\n%s",
                           &buff->buff[buff->buffpos]);

            }
        }
    }

    /* check if an external write function is used */
    if (scb->wrfn) {
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
    if (iovs[0].iov_base == NULL) {
        return SET_ERROR(ERR_NCX_OPERATION_FAILED);
    }

    if (scb->framing11) {
        /* send the 'cnt' number of buffs identified above
         * do not use the iovs array because this function
         * may not write the entire amount requested
         * and that would not match the huge chunksize;
         * so just send each buffer as a chunk
         */
        for (i=0; i < cnt; i++) {
            buff = (ses_msg_buff_t *)dlq_deque(&scb->outQ);
            if (buff == NULL) {
                return SET_ERROR(ERR_INTERNAL_VAL);
            }
            res = do_send_buff(scb, buff);
            ses_msg_free_buff(scb, buff);            
            if (res != NO_ERR) {
                return res;
            }
        }
        return NO_ERR;
    }

    /* else base:1.0 framing
     * write a packet to the session socket 
     */
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
                       retcnt, 
                       total, 
                       scb->sid);
        }
    }

    /* clean up the buffers that were written */
    buff = (ses_msg_buff_t *)dlq_firstEntry(&scb->outQ);

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
*   !!! buffer will be sent if stream output mode, then buffer reused
*   
* RETURNS:
*   status, could return malloc or buffers exceeded error
*********************************************************************/
status_t
    ses_msg_new_output_buff (ses_cb_t *scb)
{
    ses_msg_buff_t *buff;
    status_t        res;

#ifdef DEBUG
    if (!scb || !scb->outbuff) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    buff = scb->outbuff;
    buff->buffpos = 0;

    if (scb->stream_output) {
        /* send this buffer right now 
         * this works because the agt_ncxserver loop and mgr_io
         * loop are single threaded and a notification cannot
         * be in the middle of being sent right now
         * If that code is changed, then make sure a notification
         * is not being streamed right now
         */
        if (buff->bufflen) {
            res = do_send_buff(scb, buff);
            ses_msg_init_buff(scb, TRUE, buff);
        } else {
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }

        /* reuse the same outbuff again */
    } else {
        /* save the buffer in the message loop do be sent when
         * the main loop checks if any output pending
         */
        dlq_enque(scb->outbuff, &scb->outQ);
        ses_msg_make_outready(scb);
        scb->outbuff = NULL;
        res = ses_msg_new_buff(scb, TRUE, &scb->outbuff);
    }
    return res;

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
    status_t   res;

#ifdef DEBUG
    if (scb == NULL || scb->outbuff == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (scb->stream_output) {
        res = do_send_buff(scb, scb->outbuff);
        ses_msg_init_buff(scb, TRUE, scb->outbuff);
        if (res != NO_ERR) {
            log_error("\nError: IO failed on session '%d' (%s)", 
                      scb->sid,
                      get_error_string(res));
        }
    } else {
        scb->outbuff->buffpos = scb->outbuff->buffstart;
        dlq_enque(scb->outbuff, &scb->outQ);
        scb->outbuff = NULL;
        (void)ses_msg_new_buff(scb, TRUE, &scb->outbuff);

        ses_msg_make_outready(scb);
    }

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
        for (i = buff->buffstart; i < buff->bufflen; i++) {
            log_write("%c", buff->buff[i]);
        }
        anytext = TRUE;
    }

    if (anytext) {
        log_write("\n");
    }

} /* ses_msg_dump */


/********************************************************************
* FUNCTION ses_msg_add_framing
*
* Add the base:1.1 framing chars to the buffer and adjust
* the buffer size pointers
*
* INPUTS:
*   scb == session control block
*   buff == buffer control block
*
* OUTPUTS:
*   framing chars added to buff->buff
*
*********************************************************************/
void
    ses_msg_add_framing (ses_cb_t *scb,
                         ses_msg_buff_t *buff)
{
    size_t     buffsize;
    int32      numlen;
    char       *p, numbuff[SES_MAX_CHUNKNUM_SIZE];

#ifdef DEBUG
    if (scb == NULL || buff == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (!scb->framing11) {
        return;
    }

    /* get the chunk size */
    buffsize = buff->bufflen - SES_STARTCHUNK_PAD;
    numlen = sprintf(numbuff, "%zu", buffsize);

    /* figure out where to put the start chunks within
     * the beginning pad area; total size is numlen+3
     *     \n#numlen\n
     */
    buff->buffstart = SES_STARTCHUNK_PAD - (numlen + 3);

    p = (char *)&buff->buff[buff->buffstart];

    *p++ = '\n';
    *p++ = '#';
    memcpy(p, numbuff, numlen);
    p += numlen;
    *p = '\n';

    if (buff->islast) {
        memcpy(&buff->buff[buff->bufflen], 
               NC_SSH_END_CHUNKS,
               NC_SSH_END_CHUNKS_LEN);
        buff->bufflen += NC_SSH_END_CHUNKS_LEN;
    }

    buff->bufflen -= buff->buffstart;

}  /* ses_msg_add_framing */


/********************************************************************
* FUNCTION ses_msg_init_buff
*
* Init the buffer fields
*
* INPUTS:
*   scb == session control block
*   outbuff == TRUE if oupput buffer; FALSE if input buffer
*   buff == buffer to send
*********************************************************************/
void
    ses_msg_init_buff (ses_cb_t *scb,
                       boolean outbuff,
                       ses_msg_buff_t *buff)
{
    buff->bufflen = 0;
    buff->buffpos = 0;
    buff->buffstart = 0;
    buff->islast = FALSE;
    if (outbuff && scb->framing11) {
        buff->buffstart = SES_STARTCHUNK_PAD;
        buff->bufflen = SES_STARTCHUNK_PAD;
    }

    /* do not clear mem in buffer buff->buff */

}  /* ses_msg_init_buff */


/* END file ses_msg.c */
