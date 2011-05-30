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
/*  FILE: ses.c

   NETCONF Session Manager: Common Support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
06jun06      abb      begun; cloned from ses_mgr.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncx_num
#include  "ncx_num.h"
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

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define SES_DEBUG 1
/* #define SES_SSH_DEBUG 1 */
#endif

#define LTSTR     (const xmlChar *)"&lt;"
#define GTSTR     (const xmlChar *)"&gt;"
#define AMPSTR    (const xmlChar *)"&amp;"
#define QSTR      (const xmlChar *)"&quot;"

/* used by yangcli to read in between stdin polling */
#define MAX_READ_TRIES   100


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
static ses_total_stats_t totals;


/********************************************************************
* FUNCTION accept_buffer_ssh_v10
*
* Handle one input buffer within the ses_accept_input function
* transport is SSH; protocol is NETCONF:base:1.0
*
* Need to separate the input stream into separate XML instance
* documents and reset the xmlTextReader each time a new document
* is encountered.  For SSH, also need to detect the EOM flag
* and remove it + control input to the reader.
*
* This function breaks the byte stream into ses_msg_t structs
* that get queued on the session's msgQ
*
* INPUTS:
*   scb == session control block to accept input for
*   buff == buffer just read; it is consumed or freed!!!
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    accept_buffer_ssh_v10 (ses_cb_t *scb,
                              ses_msg_buff_t *buff)
{
    ses_msg_t      *msg, *msg2;
    ses_msg_buff_t *buff2, *lastbuff;
    const char     *endmatch;
    status_t        res;
    boolean         done;
    xmlChar         ch;
    uint32          left;

    endmatch = NC_SSH_END;
    done = FALSE;
    msg2 = NULL;
    buff->buffstart = 0;   /* default for netconf10 */

#ifdef SES_DEBUG
    if (LOGDEBUG3 && scb->state != SES_ST_INIT) {
        log_debug3("\nses: accept buffer (%u):\n%s\n", 
                   buff->bufflen, buff->buff);
    } else if (LOGDEBUG2) {
        log_debug2("\nses: accept buffer (%u)", buff->bufflen);
    }
#endif

    /* make sure there is a current message */
    msg = (ses_msg_t *)dlq_lastEntry(&scb->msgQ);
    if (!msg || msg->ready) {
        /* need a new message */
        res = ses_msg_new_msg(scb, &msg);
        if (res != NO_ERR) {
            ses_msg_free_buff(scb, buff);
            return res;
        }
        dlq_enque(msg, &scb->msgQ);
    }

    /* check the chars in the buffer for the 
     * the NETCONF EOM if this is an SSH session
     *
     * TBD: ses how xmlReader handles non-well-formed XML 
     * and junk text.  May need some sort of error state
     * to dump chars until the EOM if SSH, or force the
     * session closed, or force the transport to reset somehow
     */
    while (!done) {

        /* check if the internal buffer end has been reached */
        if (buff->buffpos == buff->bufflen) {
            /* done checking the current buffer,
             * add this buffer to the current message and exit 
             */
            buff->buffpos = 0;
            dlq_enque(buff, &msg->buffQ);
            done = TRUE;
            continue;
        }

        /* get the next char in the input buffer and advance the pointer */
        ch = buff->buff[buff->buffpos++];

        /* handle the char in the buffer based on the input state */
        switch (scb->instate) {
        case SES_INST_IDLE:
            /* check for EOM if SSH or just copy the char */
            if (ch==*endmatch) {
                scb->instate = SES_INST_INEND;
                scb->inendpos = 1;
            } else {
                scb->instate = SES_INST_INMSG;
            }
            break;
        case SES_INST_INMSG:
            /* check for EOM if SSH or just copy the char */
            if (ch==*endmatch) {
                scb->instate = SES_INST_INEND;
                scb->inendpos = 1;
            }
            break;
        case SES_INST_INEND:
            /* already matched at least 1 EOM char
             * try to match the rest of the SSH EOM string 
             */
            if (ch == endmatch[scb->inendpos]) {

                /* init local vars */
                buff2 = NULL;
                lastbuff = (ses_msg_buff_t *)dlq_lastEntry(&msg->buffQ);

                /* check message complete */
                if (++scb->inendpos == NC_SSH_END_LEN) {

                    /* completely matched the SSH EOM marker 
                     * finish the current message and put it in the inreadyQ
                     *
                     * buff->buffpos points to the first char after
                     * the EOM string, check any left over bytes
                     * to start a new message
                     *
                     * handle any bytes left at the end of 'buff'
                     * if this is a base:1.1 session then there
                     * is a possibility the new framing will start
                     * right away, because the peer sent a <hello>
                     * and an <rpc> back-to-back
                     */
                    if (buff->buffpos < buff->bufflen) {

                        /* get a new buffer to hold the overflow */
                        res = ses_msg_new_buff(scb, FALSE, &buff2);
                        if (res == NO_ERR) {
                            /* get a new message header for buff2 
                             * but do not add buff2 to msg2 yet 
                             */
                            res = ses_msg_new_msg(scb, &msg2);
                            if (res == NO_ERR) {
                                /* put msg2 in the msg Q and
                                 * copy the rest of buff into buff2 
                                 */
                                dlq_enque(msg2, &scb->msgQ);
                                buff2->bufflen = buff->bufflen - buff->buffpos;
                                memcpy(buff2->buff, 
                                       &buff->buff[buff->buffpos],
                                       buff2->bufflen);
                                buff->bufflen = buff->buffpos;
                            }
                        }
                        if (res != NO_ERR) {
                            if (buff2) {
                                ses_msg_free_buff(scb, buff2);
                                buff2 = NULL;
                            }
                            
                            /* truncate the input and continue */
                            buff->bufflen = buff->buffpos;
                            if (LOGINFO) {
                                log_info("\nses: dropping input "
                                         "for session %d (%s)",
                                         scb->sid,
                                         get_error_string(res));
                            }

                            /* do not barf on input overflow error */
                            res = NO_ERR;   
                        }
                    }

                    /* save the buffer and make the message ready to parse 
                     * don't let the xmlreader see the EOM string
                     */
                    if (buff->bufflen > NC_SSH_END_LEN) {
                        /* finish message and add to the ready Q */
                        buff->bufflen -= NC_SSH_END_LEN;
                        buff->buffpos = 0;
                        dlq_enque(buff, &msg->buffQ);
                        msg->curbuff = NULL;
                        msg->ready = TRUE;
                        ses_msg_make_inready(scb);
                    }  else {
                        /* only thing in the buffer was an EOM string 
                         * or part of the string, corner-case, need to
                         * back out the EOM chars in 1 or more previous buffs
                         */
                        left = NC_SSH_END_LEN - buff->bufflen;
                        ses_msg_free_buff(scb, buff);

                        while (left && lastbuff) {
                            if (lastbuff->bufflen <= left) {
                                left -= lastbuff->bufflen;
                                /* zap the last buffer, borrow 'buff' var */
                                buff = (ses_msg_buff_t *)
                                    dlq_prevEntry(lastbuff);
                                dlq_remove(lastbuff);
                                ses_msg_free_buff(scb, lastbuff);
                                lastbuff = buff;
                            } else {
                                /* just truncate the last buffer */
                                lastbuff->bufflen -= left;
                                left = 0;
                            }
                        }

                        /* check anything left in the msg */
                        if (dlq_empty(&msg->buffQ)) {
                            dlq_remove(msg);
                            ses_msg_free_msg(scb, msg);
                            msg = NULL;
                        } else {
                            msg->ready = TRUE;
                            ses_msg_make_inready(scb);
                        }
                    }

                    /* reset reader state */
                    scb->instate = SES_INST_IDLE;
                    scb->inendpos = 0;

                    /* check if more work to do */
                    if (buff2) {
                        /* check if this is the special corner-case where
                         * the protocol version has not been set yet
                         * only allow the old EOM while protocol not set
                         * for 2 messages <ncx-connect> and <hello>
                         * Defer any <rpc> messages that follow these
                         * two initial messages
                         */
                        if (scb->protocol == NCX_PROTO_NONE &&
                            ncx_protocol_enabled(NCX_PROTO_NETCONF11) &&
                            dlq_count(&scb->msgQ) >= 3) {
                            /* do not continue processing buff2 and msg2 */
                            if (LOGDEBUG2) {
                                log_debug2("\nses: defer msg for s:%d until "
                                           "protocol framing set",
                                           scb->sid);
                            }
                            dlq_enque(buff2, &msg2->buffQ);
                            msg2->deferred = TRUE;
                            done = TRUE;
                        } else {
                            buff = buff2;
                            msg = msg2;
                        }
                    } else {
                        done = TRUE;
                    }
                }  /* else still more chars in EOM string to match */
            } else {
                /* char did not match the expected position in the 
                 * EOM string, go back to MSG state
                 */
                scb->instate = SES_INST_INMSG;
                scb->inendpos = 0;
            }
            break;
        default:
            /* should not happen */
            if (buff) {
                ses_msg_free_buff(scb, buff);
            }
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    return NO_ERR;

}  /* accept_buffer_ssh_v10 */


/********************************************************************
* FUNCTION accept_buffer_ssh_v11
*
* Handle one input buffer within the ses_accept_input function
* Transport is SSH.  Protocol is NETCONF:base:1.1
* Use SSH base:1.1 framing, not old EOM sequence
*
* Need to separate the input stream into separate XML instance
* documents and reset the xmlTextReader each time a new document
* is encountered.  Handle NETCONF over SSH base:1.1 protocol framing
* 
* This function breaks the byte stream into ses_msg_t structs
* that get queued on the session's msgQ.  
*
* The chunks encoded into each incoming buffer will be
* be mapped and stored in 1 or more ses_buffer_t structs.
* There will be wasted buffer space if the chunks
* are very small and more than SES_MAX_BUFF_CHUNKS
* per buffer are received
*
* RFC 4742bis, 4.2.  Chunked Framing Mechanism

   This mechanism encodes all NETCONF messages with a chunked framing.
   Specifically, the message follows the ABNF [RFC5234] rule Chunked-
   Message:


        Chunked-Message = 1*chunk
                          end-of-chunks

        chunk           = LF HASH chunk-size LF
                          chunk-data
        chunk-size      = 1*DIGIT1 0*DIGIT
        chunk-data      = 1*OCTET

        end-of-chunks   = LF HASH HASH LF

        DIGIT1          = %x31-39
        DIGIT           = %x30-39
        HASH            = %x23
        LF              = %x0A
        OCTET           = %x00-FF


   The chunk-size field is a string of decimal digits indicating the
   number of octets in chunk-data.  Leading zeros are prohibited, and
   the maximum allowed chunk-size value is 4294967295.

* INPUTS:
*   scb == session control block to accept input for
*   buff == buffer just read; it is consumed or freed!!!
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    accept_buffer_ssh_v11 (ses_cb_t *scb,
                           ses_msg_buff_t *buff)
{
    ses_msg_t      *msg, *msg2;
    ses_msg_buff_t *buff2;
    const char     *chunkmatch;
    status_t        res;
    uint32          chunkidx;
    boolean         done, qbuffdone, inframing;
    xmlChar         ch;
    size_t          chunkleft, buffleft, copylen;
    ncx_num_t       num;

    chunkmatch = NC_SSH_START_CHUNK;  /* just first 2 chars \n# */
    done = FALSE;
    msg2 = NULL;
    res = NO_ERR;
    buff->buffstart = 0;
    ch = 0;
    chunkidx = 0;

#ifdef SES_DEBUG
    if (LOGDEBUG3 && scb->state != SES_ST_INIT) {
        log_debug3("\nses: accept base:1.1 buffer (%u):\n%s\n", 
                   buff->bufflen, 
                   buff->buff);
    } else if (LOGDEBUG2) {
        log_debug2("\nses: accept base:1.1 buffer (%u)", buff->bufflen);
    }
#endif

    if (!scb->eomdone) {
        /* first time through ... check for any deferred messaged */
        for (msg = (ses_msg_t *)dlq_firstEntry(&scb->msgQ);
             msg != NULL;
             msg = (ses_msg_t *)dlq_nextEntry(msg)) {

            if (msg->deferred) {
                /* need to process all the buffs in all the
                 * deferred messages; if there is more than one,
                 * it is because the old EOM marker was hit
                 * start at this first deferred message;
                 * there may be first 2 messages ahead of this one
                 * that are supposed to have old framing
                 */
                /***/SET_ERROR(ERR_INTERNAL_VAL);
            }
        }
        scb->eomdone = TRUE;
    }

    /* make sure there is a current message */
    msg = (ses_msg_t *)dlq_lastEntry(&scb->msgQ);
    if (msg == NULL || msg->ready) {
        /* need a new message */
        res = ses_msg_new_msg(scb, &msg);
        if (res != NO_ERR) {
            ses_msg_free_buff(scb, buff);
            return res;
        }
        dlq_enque(msg, &scb->msgQ);
    }

    /* save buffer early for garbage collection or use */
    dlq_enque(buff, &msg->buffQ); 
    qbuffdone = TRUE;

    /* check starting state corner cases */
    switch (scb->instate) {
    case SES_INST_INSTART:
    case SES_INST_INBETWEEN:
    case SES_INST_INEND:
        inframing = TRUE;
        break;
    default:
        inframing = FALSE;
    }

    /* check the chars in the buffer for the 
     * frame markers, based on session instate value
     * the framing breaks will be mixed in with the XML
     */
    while (!done) {

        /* check if the internal buffer end has been reached */
        if (scb->instate != SES_INST_FINMSG) {
            if (buff->buffpos == buff->bufflen) {
                /* done checking the current buffer,
                 * add this buffer to the current message and exit 
                 */
                done = TRUE;
                continue;
            }
        }

        switch (scb->instate) {
        case SES_INST_NONE:
            break;  /* error handled below */
        case SES_INST_INMSG:
        case SES_INST_FINMSG:
            break;  /* not processing char at a time */
        case SES_INST_INBETWEEN:
        case SES_INST_INEND:
            /* get the next char in the input buffer; no advance  */
            ch = buff->buff[buff->buffpos + scb->inendpos];
            break;
        default:
            /* get the next char in the input buffer 
             * and advance the pointer 
             */
            ch = buff->buff[buff->buffpos++];
        }

        /* handle the char in the buffer based on the input state */
        switch (scb->instate) {
        case SES_INST_IDLE:
            /* expecting start of the first chunk of a message */
            if (ch == *chunkmatch) {
                /* matched the \n to start a chunk */
                scb->instate = SES_INST_INSTART;
                scb->inendpos = 1;
            } else {
                /* return framing error */
                done = TRUE;
                res = ERR_NCX_INVALID_FRAMING;
            }
            break;
        case SES_INST_INSTART:
            inframing = TRUE;
            if (scb->inendpos == 1) {
                if (ch == chunkmatch[1]) {
                    /* matched first hash mark # */
                    scb->inendpos++;
                } else {
                    /* return framing error; 
                     * save buff for garbage collection 
                     */
                    done = TRUE;
                    res = ERR_NCX_INVALID_FRAMING;
                }
            } else if (scb->inendpos == 2) {
                /* expecting at least 1 starting digit */
                if (ch >= '1' && ch <= '9') {
                    scb->startchunk[0] = ch;   /* save first num char */
                    scb->inendpos++;  /* == 3 now */
                } else {
                    /* return framing error */
                    done = TRUE;
                    res = ERR_NCX_INVALID_FRAMING;
                }
            } else {
                /* looking for end of a number
                 * expecting an ending digit or \n
                 */
                if (scb->inendpos == SES_MAX_STARTCHUNK_SIZE) {
                    /* invalid number -- too long error */
                    done = TRUE;
                    res = ERR_NCX_INVALID_FRAMING;
                } else if (ch == '\n') {
                    /* have the complete number now
                     * done with chunk start tag
                     * get a binary number from this number
                     */
                    ncx_init_num(&num);
                    scb->startchunk[scb->inendpos - 2] = 0;
                    scb->inendpos = 0;
                    res = ncx_convert_num(scb->startchunk,
                                          NCX_NF_DEC,
                                          NCX_BT_UINT32,
                                          &num);
                    if (res == NO_ERR) {
                        msg->expchunksize = num.u;
                        msg->curchunksize = 0;
                        buff->inchunks[chunkidx].chunkstart 
                            = buff->buffpos;
                        if (chunkidx == 0) {
                            buff->buffstart = buff->buffpos;
                        }
                        scb->instate = SES_INST_INMSG;
                    } else {
                        done = TRUE;
                        res = ERR_NCX_INVALID_FRAMING;
                    }
                    ncx_clean_num(NCX_BT_UINT32, &num);
                } else if (ch >= '0' && ch <= '9') {
                    /* continue collecting digits */
                    scb->startchunk[scb->inendpos - 2] = ch;
                    scb->inendpos++;                    
                } else {
                    /* return framing error; invalid char */
                    done = TRUE;
                    res = ERR_NCX_INVALID_FRAMING;
                }
            }
            break;
        case SES_INST_INMSG:
            inframing = FALSE;

            /* expecting first part or Nth part of a chunk */
            chunkleft = msg->expchunksize - msg->curchunksize;
            buffleft = buff->bufflen - buff->buffpos;

            if (buffleft < chunkleft) {
                /* finish off the buffer; chunk not done
                 * wait for the next buffer to finish the chunk
                 */
                msg->curchunksize += buffleft;
                buff->buffpos = buff->bufflen;
                buff->inchunks[chunkidx++].chunklen = buffleft;
            } else {
                /* buffer >= remainder of chunk
                 * buffer may not be done; 
                 * move to next inchunk map or split into new buffer
                 */
                chunkmatch = NC_SSH_END_CHUNKS;
                scb->inendpos = 0;
                buff->buffpos += chunkleft;
                buff->inchunks[chunkidx++].chunklen = chunkleft;

                copylen = buff->bufflen;

                /* check common case; message and EoChunks
                 * fits into the same buffer
                 * If bufflen big enough, check EoCh now
                 */
                if ((buffleft >= (chunkleft + NC_SSH_END_CHUNKS_LEN)) &&
                    !strncmp((const char *)&buff->buff[buff->buffpos],
                             chunkmatch,
                             NC_SSH_END_CHUNKS_LEN)) {
                    /* process EoChunks here; make msg ready */
                    scb->instate = SES_INST_FINMSG;

                    if (buffleft > (chunkleft + NC_SSH_END_CHUNKS_LEN)) {
                        if (chunkidx < SES_MAX_BUFF_CHUNKS) {
                            /* skip over EoChunks and loop to next chunk */
                            buff->buffpos += NC_SSH_END_CHUNKS_LEN;
                        } else {
                            /* shorten the buffer length to the chunk size */
                            buff->bufflen = buff->buffpos;

                            /* get a new buffer to hold the overflow */
                            res = ses_msg_new_buff(scb, FALSE, &buff2);
                            if (res == NO_ERR) {
                                buff2->bufflen = copylen 
                                    - (buff->buffpos + NC_SSH_END_CHUNKS_LEN);
                                memcpy(buff2->buff, 
                                       &buff->buff[buff->buffpos + 
                                                   NC_SSH_END_CHUNKS_LEN],
                                       buff2->bufflen);
                                if (!qbuffdone) {
                                    dlq_enque(buff, &msg->buffQ);
                                }
                                qbuffdone = FALSE;
                                buff = buff2;
                                buff2 = NULL;
                                chunkidx = 0;
                            } else {
                                done = TRUE;
                            }
                        }
                    } else {
                        /* buffer exactly chunk-part+EoChunks */
                        if (!qbuffdone) {
                            dlq_enque(buff, &msg->buffQ);
                            qbuffdone = TRUE;
                        }
                        buff = NULL;
                    }
                } else {
                    scb->instate = SES_INST_INBETWEEN;

                    if (buffleft > chunkleft) {
                        if (chunkidx < SES_MAX_BUFF_CHUNKS) {
                            ;
                        } else {
                            /* get a new buffer to hold the overflow */
                            res = ses_msg_new_buff(scb, FALSE, &buff2);
                            if (res == NO_ERR) {
                                buff2->bufflen = copylen - buff->buffpos;
                                memcpy(buff2->buff, 
                                       &buff->buff[buff->buffpos],
                                       buff2->bufflen);
                                if (!qbuffdone) {
                                    dlq_enque(buff, &msg->buffQ);
                                }
                                qbuffdone = FALSE;
                                buff = buff2;
                                buff2 = NULL;
                                chunkidx = 0;
                            } else {
                                done = TRUE;
                            }
                        }
                    } else {
                        done = TRUE;
                    }
                }
            }
            break;
        case SES_INST_INBETWEEN:
            inframing = TRUE;
            if (scb->inendpos == 0) {
                if (ch == '\n') {
                    scb->inendpos++;
                } else {
                    done = TRUE;
                    res = ERR_NCX_INVALID_FRAMING;
                }
            } else if (scb->inendpos == 1) {
                if (ch == '#') {
                    scb->inendpos++;
                } else {
                    done = TRUE;
                    res = ERR_NCX_INVALID_FRAMING;
                }
            } else {
                if (ch == '#') {
                    scb->inendpos++;
                    scb->instate = SES_INST_INEND;
                } else if (ch >= '1' && ch <= '9') {
                    /* back up and process this char in start state
                     * account for first 2 chars \n#
                     */
                    buff->buffpos += 2;  
                    scb->instate = SES_INST_INSTART;
                } else {
                    done = TRUE;
                    res = ERR_NCX_INVALID_FRAMING;
                }
            }
            break;
        case SES_INST_INEND:
            inframing = TRUE;

            /* expect to match 4 char \n##\n sequence */
            if (ch != chunkmatch[scb->inendpos]) {
                done = TRUE;
                res = ERR_NCX_INVALID_FRAMING;
            } else if (++scb->inendpos == NC_SSH_END_CHUNKS_LEN) {
                scb->instate = SES_INST_FINMSG;
            }  /* else still more chars in EOM string to match */
            break;
        case SES_INST_FINMSG:
            /* completely matched the SSH End of Chunks marker
             * finish the current message and put it in the inreadyQ
             */
            msg->curbuff = NULL;
            msg->ready = TRUE;
            ses_msg_make_inready(scb);

            /* reset reader state */
            scb->instate = SES_INST_IDLE;
            scb->inendpos = 0;
            inframing = FALSE;

            if (buff != NULL && buff->buffpos < buff->bufflen) {
                /*
                 * buff->buffpos points to the first char after
                 * the EOCh string, check any left over bytes
                 * to start a new message
                 */
                res = ses_msg_new_msg(scb, &msg2);
                if (res == NO_ERR) {
                    /* put msg2 in the msg Q */
                    dlq_enque(msg2, &scb->msgQ);
                    msg = msg2;
                } else {
                    if (!qbuffdone) {
                        ses_msg_free_buff(scb, buff);
                        buff = NULL;
                    }
                    done = TRUE;
                }
            } else {
                /* end of buffer; create message next time */
                done = TRUE;
            }
            break;
        default:
            /* should not happen */
            if (buff && !qbuffdone) {
                ses_msg_free_buff(scb, buff);
            }
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

    }

    if (buff != NULL && !qbuffdone) {
        if (inframing || msg == NULL) {
            /* do not save if just framing chars */
            if (LOGDEBUG2) {
                log_debug2("\nses: dropping buff w/ framing only");
            }
            ses_msg_free_buff(scb, buff);
        } else {
            /* was buff2 now saved in msg2 */
            dlq_enque(buff, &msg->buffQ);
        }
    }

    return res;

}  /* accept_buffer_ssh_v11 */


/********************************************************************
* FUNCTION put_char_entity
*
* Write a character entity for the specified character
*
* INPUTS:
*   scb == session control block to start msg 
*   ch == character to write as a character entity
*
*********************************************************************/
static void
    put_char_entity (ses_cb_t *scb,
                     xmlChar ch)
{
    xmlChar     numbuff[NCX_MAX_NUMLEN];

    sprintf((char *)numbuff, "%u", (uint32)ch);

    ses_putchar(scb, '&');
    ses_putchar(scb, '#');
    ses_putstr(scb, numbuff);
    ses_putchar(scb, ';');

}  /* put_char_entity */


/********************************************************************
* FUNCTION handle_prolog_state
*
* Deal with the first few characters of an incoming message
* hack: xmlTextReaderRead wants to start off
* with a newline for some reason, so always
* start the first buffer with a newline, even if
* none was sent by the NETCONF peer.
* Only the first 0xa char seems to matter
* Trailing newlines do not seem to affect the problem
*
* Also, the first line needs to be the <?xml ... ?>
* prolog directive, or the libxml2 parser refuses
* to use the incoming message.  
* It quits and returns EOF instead.
*
* Only the current buffer is processed within the message
* INPUTS:
*   msg == current message to process
*   buffer == buffer to fill in
*   bufflen == max buffer size
*   buff == current buffer about to be read
*   endpos == max end pos of buff to use
*   retlen == address of running return length
*
* OUTPUTS:
*   buffer is filled in with the prolog if needed
*   msg->prolog_state is updated as needed
*   *retlen may be increased if prolog or newline added
*********************************************************************/
static void
    handle_prolog_state (ses_msg_t *msg,
                         char *buffer,
                         int bufflen,
                         ses_msg_buff_t *buff,
                         size_t endpos,
                         int *retlen)
{
    boolean needprolog = FALSE;
    boolean needfirstnl = FALSE;
    char    tempbuff[4];
    int     i, j, k;

    memset(tempbuff, 0x0, 4);

    switch (msg->prolog_state) {
    case SES_PRST_NONE:
        if ((endpos - buff->buffpos) < 3) {
            msg->prolog_state = SES_PRST_WAITING;
            return;
        } else if (!strncmp((const char *)&buff->buff[buff->buffpos], 
                            "\n<?", 
                            3)) {
            /* expected string is present */
            msg->prolog_state = SES_PRST_DONE;
            return;
        } else if (!strncmp((const char *)&buff->buff[buff->buffpos], 
                            "<?", 
                            2)) {
            /* expected string except a newline needs
             * to be inserted first to make libxml2 happy
             */
            needfirstnl = TRUE;
            msg->prolog_state = SES_PRST_DONE;
        } else {
            needprolog = TRUE;
            msg->prolog_state = SES_PRST_DONE;
        }
        break;
    case SES_PRST_WAITING:
        if ((*retlen + (endpos - buff->buffpos)) < 3) {
            /* keep waiting */
            return;
        } else {
            msg->prolog_state = SES_PRST_DONE;

            /* save the first 3 chars in the temp buffer */
            strncpy(tempbuff, buffer, *retlen);
            for (i = *retlen, j=buff->buffpos; i <= 3; i++, j++) {
                tempbuff[i] = (char)buff->buff[j];
            }

            if (!strncmp(tempbuff, "\n<?", 3)) {
                /* expected string is present */
                ;
            } else if (!strncmp(tempbuff, "<?", 2)) {
                /* expected string except a newline needs
                 * to be inserted first to make libxml2 happy
                 */
                needfirstnl = TRUE;
            } else {
                needprolog = TRUE;
            }
        }
        break;
    case SES_PRST_DONE:
        return;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }

    if (needfirstnl || needprolog) {
        buffer[0] = '\n';
        i = 1;
    } else {
        i = 0;
    }

    if (needprolog) {
        /* expected string sequence is not present */
        if (bufflen < XML_START_MSG_SIZE+5) {
            SET_ERROR(ERR_INTERNAL_VAL);
        } else {
            /* copy the prolog string inline
             * to make libxml2 happy
             */
            strcpy(&buffer[i], (const char *)XML_START_MSG);

            if (tempbuff[0]) {
                for (j = XML_START_MSG_SIZE+i, k = 0; 
                     k <= *retlen; 
                     j++, k++) {
                    if (k == *retlen) {
                        buffer[j] = 0;
                    } else {
                        buffer[j] = tempbuff[k];
                    }
                }
            }
            *retlen += XML_START_MSG_SIZE;
        }
    }

    *retlen += i;

}  /* handle_prolog_state */


/************   E X T E R N A L   F U N C T I O N S     ***********/


/********************************************************************
* FUNCTION ses_new_scb
*
* Create a new session control block
*
* INPUTS:
*   none
* RETURNS:
*   pointer to initialized SCB, or NULL if malloc error
*********************************************************************/
ses_cb_t *
    ses_new_scb (void)
{
    ses_cb_t  *scb;
    xmlChar   *now;
    
    now = m__getMem(TSTAMP_MIN_SIZE);
    if (!now) {
        return NULL;
    }
    tstamp_datetime(now);

    scb = m__getObj(ses_cb_t);
    if (!scb) {
        m__free(now);
        return NULL;
    }

    memset(scb, 0x0, sizeof(ses_cb_t));
    scb->start_time = now;
    dlq_createSQue(&scb->msgQ);
    dlq_createSQue(&scb->freeQ);
    dlq_createSQue(&scb->outQ);
    scb->linesize = SES_DEF_LINESIZE;
    scb->withdef = NCX_DEF_WITHDEF;
    scb->indent = NCX_DEF_INDENT;
    scb->cache_timeout = NCX_DEF_VTIMEOUT;
    return scb;

}  /* ses_new_scb */


/********************************************************************
* FUNCTION ses_new_dummy_scb
*
* Create a new dummy session control block
*
* INPUTS:
*   none
* RETURNS:
*   pointer to initialized SCB, or NULL if malloc error
*********************************************************************/
ses_cb_t *
    ses_new_dummy_scb (void)
{
    ses_cb_t  *scb;

    scb = ses_new_scb();
    if (!scb) {
        return NULL;
    }

    scb->type = SES_TYP_DUMMY;
    scb->mode = SES_MODE_XML;
    scb->state = SES_ST_IDLE;
    scb->sid = 0;
    scb->username = xml_strdup(NCX_DEF_SUPERUSER);

    return scb;

}  /* ses_new_dummy_scb */


/********************************************************************
* FUNCTION ses_free_scb
*
* Free a session control block
*
* INPUTS:
*   scb == session control block to free
* RETURNS:
*   none
*********************************************************************/
void 
    ses_free_scb (ses_cb_t *scb)
{
    ses_msg_t *msg;
    ses_msg_buff_t *buff;

#ifdef DEBUG
    if (!scb) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (scb->start_time) {
        m__free(scb->start_time);
    }

    if (scb->username) {
        m__free(scb->username);
    }

    if (scb->peeraddr) {
        m__free(scb->peeraddr);
    }

    if (scb->reader) {
        xml_free_reader(scb->reader);
    }

    if (scb->fd) {
        close(scb->fd);
    }

    if (scb->fp) {
        fclose(scb->fp);
    }

    while (!dlq_empty(&scb->msgQ)) {
        msg = (ses_msg_t *)dlq_deque(&scb->msgQ);
        ses_msg_free_msg(scb, msg);
    }

    if (scb->outbuff) {
        ses_msg_free_buff(scb, scb->outbuff);
    }

    while (!dlq_empty(&scb->outQ)) {
        buff = (ses_msg_buff_t *)dlq_deque(&scb->outQ);
        ses_msg_free_buff(scb, buff);
    }

    /* the freeQ must be cleared after the outQ because
     * the normal free buffer action is to move it to
     * the scb->freeQ
     */
    while (!dlq_empty(&scb->freeQ)) {
        buff = (ses_msg_buff_t *)dlq_deque(&scb->freeQ);
        ses_msg_free_buff(scb, buff);
    }

    if (scb->buffcnt) {
        log_error("\nsession %d terminated with %d buffers",
                  scb->sid, scb->buffcnt);
    }

    /* the mgrcb must be cleaned before this function is called */

    m__free(scb);

}  /* ses_free_scb */


/********************************************************************
* FUNCTION ses_putchar
*
* Write one char to the session, without any translation
*
* THIS FUNCTION DOES NOT CHECK ANY PARAMETERS TO SAVE TIME
*
* NO CHARS ARE ACTUALLY WRITTEN TO A REAL SESSION!!!
* The 'output ready' indicator will be set and the session
* queued in the outreadyQ.  Non-blocking IO functions
* will send the data when the connection allows.
*
* INPUTS:
*   scb == session control block to start msg 
*   ch = xmlChar to write, cast as uint32 to avoid compiler warnings
*
*********************************************************************/
void
    ses_putchar (ses_cb_t *scb,
                 uint32    ch)
{
    ses_msg_buff_t *buff;
    status_t res;

    if (scb->fd) {
        /* Normal NETCONF session mode: */
        res = NO_ERR;
        if (scb->outbuff == NULL) {
            res = ses_msg_new_buff(scb, TRUE, &scb->outbuff);
        }
        if (scb->outbuff != NULL) {
            buff = scb->outbuff;
            res = ses_msg_write_buff(scb, buff, ch);
            if (res == ERR_BUFF_OVFL) {
                res = ses_msg_new_output_buff(scb);
                if (res == NO_ERR) {
                    buff = scb->outbuff;
                    res = ses_msg_write_buff(scb, buff, ch);
                }
            }
        } else {
            res = ERR_NCX_OPERATION_FAILED;
        }

        if (res == NO_ERR) {
            scb->stats.out_bytes++;
            totals.stats.out_bytes++;
        }
    } else if (scb->fp) {
        /* debug session, sending output to a file */
        fputc((int)ch, scb->fp);
    } else {
        /* debug session, sending output to the screen */
        putchar((int)ch);
    }

    if (ch=='\n') {
        scb->stats.out_line = 0;
    } else {
        scb->stats.out_line++;
    }

}  /* ses_putchar */


/********************************************************************
* FUNCTION ses_putstr
*
* Write a zero-terminated string to the session
*
* THIS FUNCTION DOES NOT CHECK ANY PARAMTERS TO SAVE TIME
*
* INPUTS:
*   scb == session control block to start msg 
*   str == string to write
*
*********************************************************************/
void
    ses_putstr (ses_cb_t *scb,
                const xmlChar *str)
{
    while (*str) {
        ses_putchar(scb, *str++);
    }

}  /* ses_putstr */


/********************************************************************
* FUNCTION ses_putstr_indent
*
* Write a zero-terminated content string to the session
* with indentation
*
* THIS FUNCTION DOES NOT CHECK ANY PARAMTERS TO SAVE TIME
* EXCEPT THAT ILLEGAL XML CHARS ARE CONVERTED TO CHAR ENTITIES
*
* INPUTS:
*   scb == session control block to start msg 
*   str == string to write
*   indent == current indent amount
*
*********************************************************************/
void
    ses_putstr_indent (ses_cb_t *scb,
                       const xmlChar *str,
                       int32 indent)
{
    ses_indent(scb, indent);
    while (*str) {
        if (*str == '\n') {
            if (indent < 0) {
                ses_putchar(scb, *str++);
            } else {
                ses_indent(scb, indent);
                str++;
            }
        } else {
            ses_putchar(scb, *str++);
        }
    }
}  /* ses_putstr_indent */


/********************************************************************
* FUNCTION ses_putcstr
*
* write XML element safe content string
* Write a zero-terminated element content string to the session
*
* THIS FUNCTION DOES NOT CHECK ANY PARAMTERS TO SAVE TIME
* EXCEPT THAT ILLEGAL XML CHARS ARE CONVERTED TO CHAR ENTITIES
*
* INPUTS:
*   scb == session control block to start msg 
*   str == string to write
*   indent == current indent amount
*
*********************************************************************/
void
    ses_putcstr (ses_cb_t *scb,
                 const xmlChar *str,
                 int32 indent)
{
    while (*str) {
        if (*str == '<') {
            ses_putstr(scb, LTSTR);
            str++;
        } else if (*str == '>') {
            ses_putstr(scb, GTSTR);
            str++;
        } else if (*str == '&') {
            ses_putstr(scb, AMPSTR);
            str++;
        } else if ((scb->mode == SES_MODE_XMLDOC
                    || scb->mode == SES_MODE_TEXT) && *str == '\n') {
            if (indent < 0) {
                ses_putchar(scb, *str++);
            } else {
                ses_indent(scb, indent);
                str++;
            }
        } else {
            ses_putchar(scb, *str++);
        }
    }
}  /* ses_putcstr */


/********************************************************************
* FUNCTION ses_putcchar
*
* Write one content char to the session, with translation as needed
*
* THIS FUNCTION DOES NOT CHECK ANY PARAMETERS TO SAVE TIME
*
* NO CHARS ARE ACTUALLY WRITTEN TO A REAL SESSION!!!
* The 'output ready' indicator will be set and the session
* queued in the outreadyQ.  Non-blocking IO functions
* will send the data when the connection allows.
*
* INPUTS:
*   scb == session control block to write 
*   ch = xmlChar to write, cast as uint32 to avoid compiler warnings
*
*********************************************************************/
void
    ses_putcchar (ses_cb_t *scb,
                  uint32    ch)
{
    int32  indent;

    indent = ses_indent_count(scb);

    if (ch) {
        if (ch == '<') {
            ses_putstr(scb, LTSTR);
        } else if (ch == '>') {
            ses_putstr(scb, GTSTR);
        } else if (ch == '&') {
            ses_putstr(scb, AMPSTR);
        } else if ((scb->mode == SES_MODE_XMLDOC
                    || scb->mode == SES_MODE_TEXT) && 
                   ch == '\n') {
            if (indent < 0) {
                ses_putchar(scb, ch);
            } else {
                ses_indent(scb, indent);
            }
        } else {
            ses_putchar(scb, ch);
        }
    }
}  /* ses_putcchar */


/********************************************************************
* FUNCTION ses_putastr
*
* write XML attribute safe content string
* Write a zero-terminated attribute content string to the session
*
* THIS FUNCTION DOES NOT CHECK ANY PARAMTERS TO SAVE TIME
* EXCEPT THAT ILLEGAL XML CHARS ARE CONVERTED TO CHAR ENTITIES
*
* INPUTS:
*   scb == session control block to start msg 
*   str == string to write
*   indent == current indent amount
*
*********************************************************************/
void
    ses_putastr (ses_cb_t *scb,
                 const xmlChar *str,
                 int32 indent)
{
    while (*str) {
        if (*str == '<') {
            ses_putstr(scb, LTSTR);
            str++;
        } else if (*str == '>') {
            ses_putstr(scb, GTSTR);
            str++;
        } else if (*str == '&') {
            ses_putstr(scb, AMPSTR);
            str++;
        } else if (*str == '"') {
            ses_putstr(scb, QSTR);
            str++;
        } else if (*str == '\n') {
            if (scb->mode == SES_MODE_XMLDOC || 
                scb->mode == SES_MODE_TEXT) {
                if (indent < 0) {
                    ses_putchar(scb, *str++);
                } else {
                    ses_indent(scb, indent);
                    str++;
                }
            } else {
                put_char_entity(scb, *str++);
            }
        } else if (isspace(*str)) {
            put_char_entity(scb, *str++);
        } else {
            ses_putchar(scb, *str++);
        }
    }
}  /* ses_putastr */


/********************************************************************
* FUNCTION ses_indent
*
* Write the proper newline + indentation to the specified session
*
* THIS FUNCTION DOES NOT CHECK ANY PARAMETERS TO SAVE TIME
*
* INPUTS:
*   scb == session control block to start msg 
*   indent == number of chars to indent after a newline
*             will be ignored if indent is turned off
*             in the agent profile
*          == -1 means no newline or indent
*          == 0 means just newline
*
*********************************************************************/
void
    ses_indent (ses_cb_t *scb,
                int32 indent)
{
    int32 i;

    /* set limit on indentation in case of bug */
    indent = min(indent, 255);

    if (indent >= 0) {
        ses_putchar(scb, '\n');
    }
    for (i=0; i<indent; i++) {
        ses_putchar(scb, ' ');
    }

}  /* ses_indent */

/********************************************************************
* FUNCTION ses_indent_count
*
* Get the indent count for this session
*
* THIS FUNCTION DOES NOT CHECK ANY PARAMETERS TO SAVE TIME
*
* INPUTS:
*   scb == session control block to check
*
* RETURNS:
*   indent value for the session
*********************************************************************/
int32
    ses_indent_count (const ses_cb_t *scb)
{
    return scb->indent;

} /* ses_indent_count */


/********************************************************************
* FUNCTION ses_set_indent
*
* Set the indent count for this session
*
* INPUTS:
*   scb == session control block to check
*   indent == value to use (may get adjusted)
*
*********************************************************************/
void
    ses_set_indent (ses_cb_t *scb,
                    int32 indent)
{
#ifdef DEBUG
    if (scb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (indent < 0) {
        indent = 0;
    } else if (indent > 9) {
        indent = 9;
    }
    scb->indent = indent;

} /* ses_set_indent */


/********************************************************************
* FUNCTION ses_set_mode
*
* Set the output mode for the specified session
*
* INPUTS:
*   scb == session control block to set
*   mode == new mode value
* RETURNS:
*   none
*********************************************************************/
void
    ses_set_mode (ses_cb_t *scb,
                  ses_mode_t mode)
{
#ifdef DEBUG
    if (scb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    scb->mode = mode;
} /* ses_set_mode */


/********************************************************************
* FUNCTION ses_get_mode
*
* Get the output mode for the specified session
*
* INPUTS:
*   scb == session control block to get
*
* RETURNS:
*   session mode value
*********************************************************************/
ses_mode_t
    ses_get_mode (ses_cb_t *scb)
{
#ifdef DEBUG
    if (scb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return SES_MODE_NONE;
    }
#endif

    return scb->mode;
} /* ses_get_mode */


/********************************************************************
* FUNCTION ses_start_msg
*
* Start a new outbound message on the specified session
*
* INPUTS:
*   scb == session control block to start msg 
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ses_start_msg (ses_cb_t *scb)
{

#ifdef DEBUG
    if (!scb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check if this session will allow a msg to start now */
    if (scb->state >= SES_ST_SHUTDOWN) {
        return ERR_NCX_OPERATION_FAILED;
    }

    /* Generate Start of XML Message Directive */
    ses_putstr(scb, XML_START_MSG);

    return NO_ERR;

}  /* ses_start_msg */


/********************************************************************
* FUNCTION ses_finish_msg
*
* Finish an outbound message on the specified session
*
* INPUTS:
*   scb == session control block to finish msg 
* RETURNS:
*   none
*********************************************************************/
void
    ses_finish_msg (ses_cb_t *scb)
{

#ifdef DEBUG
    if (!scb) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    /* add the NETCONF EOM marker */
    if (scb->transport==SES_TRANSPORT_SSH) {
        if (scb->framing11) {
            scb->outbuff->islast = TRUE;
        } else {
            ses_putstr(scb, (const xmlChar *)NC_SSH_END);
        }
    }

    /* add a final newline when writing to a file
     * but never if writing to a socket or STDOUT
     */
    if (scb->fd == 0 && scb->fp != stdout) {
        ses_putchar(scb, '\n');
    }

    /* queue for output if not done so already */
    if (scb->type != SES_TYP_DUMMY) {
        ses_msg_finish_outmsg(scb);
    }

}  /* ses_finish_msg */


/********************************************************************
* FUNCTION ses_read_cb
*
* The IO input front-end for the xmlTextReader parser read fn
*
* Need to separate the input stream into separate XML instance
* documents and reset the xmlTextReader each time a new document
* is encountered.
*
* The underlying transport (accept_buffer*) has already stripped
* all the framing characters from the incoming stream
*
* Uses a complex state machine which does not assume that the
* input from the network is going to arrive in well-formed chunks.
* It has to be treated as a byte stream (SOCK_STREAM).
*
* Does not remove char entities or any XML, just the SSH framing chars
*
* INPUTS:
*   context == scb pointer for the session to read
*   buffer == char buffer to fill
*   len == length of the buffer
*
* RETURNS:
*   number of bytes read into the buffer
*   -1     indicates error and EOF
*********************************************************************/
int
    ses_read_cb (void *context,
                 char *buffer,
                 int len)
{
    ses_cb_t         *scb;
    ses_msg_t        *msg;
    ses_msg_buff_t   *buff;
    size_t            endpos;
    int               retlen;
    boolean           done, usechunks;

    if (len == 0) {
        return 0;
    }

    scb = (ses_cb_t *)context;
    if (scb->state >= SES_ST_SHUTDOWN_REQ) {
        return -1;
    }

    msg = (ses_msg_t *)dlq_firstEntry(&scb->msgQ);
    if (msg == NULL) {
        return 0;
    }

    /* hack: the xmlTextReader parser will request 4 bytes
     * to test the message and a buffer size of 4096
     * if the request is for real.  The 4096 constant
     * is not hard-wired into the code, though.
     */
    if (len == 4) {
        strcpy(buffer, "\n<?x");
        return 4;
    }

    retlen = 0;

    usechunks = (ses_get_protocol(scb) == NCX_PROTO_NETCONF11) 
        ? TRUE : FALSE;

    /* check if this is the first read for this message */
    buff = msg->curbuff;
    if (buff == NULL) {
        scb->inchunkidx = 0;
        buff = (ses_msg_buff_t *)dlq_firstEntry(&msg->buffQ);
        if (buff == NULL) {
            return 0;
        } else {
            buff->buffpos = buff->buffstart;
            msg->curbuff = buff;
        }
    }

    if (usechunks) {
        endpos = buff->inchunks[scb->inchunkidx].chunkstart +
            buff->inchunks[scb->inchunkidx].chunklen;
    } else {
        endpos = buff->bufflen;
    }

    /* check current buffer end has been reached */
    if (buff->buffpos == endpos) {
        if (usechunks &&
            (scb->inchunkidx < (SES_MAX_BUFF_CHUNKS-1) &&
             buff->inchunks[scb->inchunkidx+1].chunkstart != 0)) {

            scb->inchunkidx++;
            buff->buffpos = buff->inchunks[scb->inchunkidx].chunkstart;
            endpos = buff->buffpos +
                buff->inchunks[scb->inchunkidx].chunklen;
        } else {
            scb->inchunkidx = 0;
            buff = (ses_msg_buff_t *)dlq_nextEntry(buff);
            if (buff == NULL) {
                return 0;
            } else {
                buff->buffpos = buff->buffstart;
                msg->curbuff = buff;
                if (usechunks) {
                    endpos = buff->buffstart +
                        buff->inchunks[0].chunklen;
                } else {
                    endpos = buff->bufflen;
                }
            }
        }
    }

    handle_prolog_state(msg, buffer, len, buff, endpos, &retlen);

    /* start transferring bytes to the return buffer */
    done = FALSE;
    while (!done) {

        buffer[retlen++] = (char)buff->buff[buff->buffpos++];

        /* check xmlreader buffer full */
        if (retlen == len) {
            done = TRUE;
            continue;
        }

        /* check current buffer end has been reached */
        if (buff->buffpos == endpos) {
            if (usechunks &&
                (scb->inchunkidx < (SES_MAX_BUFF_CHUNKS-1) &&
                 buff->inchunks[scb->inchunkidx+1].chunkstart != 0)) {

                scb->inchunkidx++;
                buff->buffpos = buff->inchunks[scb->inchunkidx].chunkstart;
                endpos = buff->buffpos +
                    buff->inchunks[scb->inchunkidx].chunklen;
            } else {
                scb->inchunkidx = 0;
                buff = (ses_msg_buff_t *)dlq_nextEntry(buff);
                if (buff == NULL) {
                    done = TRUE;
                    continue;
                } else {
                    buff->buffpos = buff->buffstart;
                    msg->curbuff = buff;
                    if (usechunks) {
                        endpos = buff->buffstart +
                            buff->inchunks[0].chunklen;
                    } else {
                        endpos = buff->bufflen;
                    }

                    handle_prolog_state(msg, 
                                        buffer, 
                                        len, 
                                        buff, 
                                        endpos, 
                                        &retlen);
                }
            }
        }
    }

    return retlen;

}  /* ses_read_cb */


/********************************************************************
* FUNCTION ses_accept_input
*
* The IO input handler for the ncxserver loop
*
* Need to separate the input stream into separate XML instance
* documents and reset the xmlTextReader each time a new document
* is encountered.  For SSH, also need to detect the EOM flag
* and remove it + control input to the reader.
*
* This function breaks the byte stream into ses_msg_t structs
* that get queued on the session's msgQ
*
* INPUTS:
*   scb == session control block to accept input for
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ses_accept_input (ses_cb_t *scb)
{
    ses_msg_buff_t *buff;
    status_t        res;
    ssize_t         ret;
    boolean         done, readdone, erragain;

#ifdef DEBUG
    if (!scb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef SES_DEBUG
    if (LOGDEBUG3) {
        log_debug3("\nses_accept_input on session %d", 
                   scb->sid);
    }
#endif

    res = NO_ERR;
    done = FALSE;
    readdone = FALSE;
    ret = 0;

 
    while (!done) {
        if (scb->state >= SES_ST_SHUTDOWN_REQ) {
            return ERR_NCX_SESSION_CLOSED;
        }

        /* get a new buffer */
        res = ses_msg_new_buff(scb, FALSE, &buff);
        if (res != NO_ERR) {
            return res;
        }

        /* loop until 1 buffer is read OK or retry count hit */
        readdone = FALSE;
        while (!readdone && res == NO_ERR) {
            /* read data into the new buffer */
            if (scb->rdfn) {
                erragain = FALSE;
                ret = (*scb->rdfn)(scb, 
                                   (char *)buff->buff, 
                                   SES_MSG_BUFFSIZE,
                                   &erragain);
            } else {
                ret = read(scb->fd, 
                           buff->buff, 
                           SES_MSG_BUFFSIZE);
            }

            if (ret < 0) {
                /* some error or EAGAIN */
                res = NO_ERR;

                if (scb->rdfn) {
                    if (!erragain) {
                        res = ERR_NCX_READ_FAILED;
                    }
                } else {
                    if (errno != EAGAIN) {
                        res = ERR_NCX_READ_FAILED;
                    }
                }

                if (res == ERR_NCX_READ_FAILED) {
                    log_error("\nError: ses read failed on session %d (%s)", 
                              scb->sid, 
                              strerror(errno));
                }
            } else {
                /* channel closed or returned byte count */
                res = NO_ERR;
                readdone = TRUE;
            }
        }

        if (res != NO_ERR) {
            ses_msg_free_buff(scb, buff);
            return res;
        }

        /* read was done if we reach here */
        if (ret == 0) {
            /* session closed by remote peer */
            if (LOGINFO) {
                log_info("\nses: session %d shut by remote peer", 
                         scb->sid);
            }
            ses_msg_free_buff(scb, buff);
            return ERR_NCX_SESSION_CLOSED;
        } else {
            if (LOGDEBUG2) {
                log_debug2("\nses read OK (%d) on session %d", 
                           ret, 
                           scb->sid);
            }

            buff->bufflen = (size_t)ret;
            scb->stats.in_bytes += (uint32)ret;
            totals.stats.in_bytes += (uint32)ret;

            if (buff->buff[buff->buffpos] == '\0') {
                /* don't barf if the client sends a 
                 * zero-terminated string instead of
                 * just the contents of the string
                 */
                if (LOGDEBUG3) {
                    log_debug3("\nses: dropping zero byte at EObuff");
                }
                buff->bufflen--;
            }

            /* hand off the malloced buffer in 1 of these functions
             * to handle the buffer framing
             */
            if (ses_get_protocol(scb) == NCX_PROTO_NETCONF11) {
                res = accept_buffer_ssh_v11(scb, buff);
            } else {
                res = accept_buffer_ssh_v10(scb, buff);
            }

            if (res != NO_ERR || ret < SES_MSG_BUFFSIZE || !scb->rdfn) {
                done = TRUE;
            } /* else the SSH2 channel probably has more bytes to read */
        }
    }
    return res;

}  /* ses_accept_input */


/********************************************************************
* FUNCTION ses_state_name
*
* Get the name of a session state from the enum value
*
* INPUTS:
*   state == session state enum value
*
* RETURNS:
*   staing corresponding to the state name
*********************************************************************/
const xmlChar *
    ses_state_name (ses_state_t state)
{
    switch (state) {
    case SES_ST_NONE:
        return (const xmlChar *)"none";
    case SES_ST_INIT:
        return (const xmlChar *)"init";
    case SES_ST_HELLO_WAIT:
        return (const xmlChar *)"hello-wait";
    case SES_ST_IDLE:
        return (const xmlChar *)"idle";
    case SES_ST_IN_MSG:
        return (const xmlChar *)"in-msg";
    case SES_ST_SHUTDOWN_REQ:
        return (const xmlChar *)"shutdown-requested";
    case SES_ST_SHUTDOWN:
        return (const xmlChar *)"shutdown";
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return (const xmlChar *)"--";   
    }
    /*NOTREACHED*/

} /* ses_state_name */


/********************************************************************
* FUNCTION ses_withdef
*
* Get the with-defaults value for this session
*
* INPUTS:
*   scb == session control block to check
*
* RETURNS:
*   with-defaults value for the session
*********************************************************************/
ncx_withdefaults_t
    ses_withdef (const ses_cb_t *scb)
{
#ifdef DEBUG
    if (!scb) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_WITHDEF_NONE;
    }
#endif

    return scb->withdef;

} /* ses_withdef */


/********************************************************************
* FUNCTION ses_line_left
*
* Get the number of bytes that can be added to the current line
* before the session linesize limit is reached
*
* INPUTS:
*   scb == session control block to check
*
* RETURNS:
*   number of bytes left, or zero if limit already reached
*********************************************************************/
uint32
    ses_line_left (const ses_cb_t *scb)
{
    if (scb->stats.out_line >= scb->linesize) {
        return 0;
    } else {
        return scb->linesize - scb->stats.out_line;
    }

} /* ses_line_left */


/********************************************************************
* FUNCTION ses_put_extern
* 
*  write the contents of a file to the session
*
* INPUTS:
*    scb == session to write
*    fspec == filespec to write
*
*********************************************************************/
void
    ses_put_extern (ses_cb_t *scb,
                    const xmlChar *fname)
{
    FILE               *fil;
    boolean             done;
    int                 ch;

    fil = fopen((const char *)fname, "r");
    if (!fil) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    } 

    done = FALSE;
    while (!done) {
        ch = fgetc(fil);
        if (ch == EOF) {
            fclose(fil);
            done = TRUE;
        } else {
            ses_putchar(scb, (uint32)ch);
        }
    }

} /* ses_put_extern */


/********************************************************************
* FUNCTION ses_get_total_stats
* 
*  Get a r/w pointer to the the session totals stats
*
* RETURNS:
*  pointer to the global session stats struct 
*********************************************************************/
ses_total_stats_t *
    ses_get_total_stats (void)
{
    return &totals;
} /* ses_get_total_stats */


/********************************************************************
* FUNCTION ses_get_transport_name
* 
*  Get the name of the transport for a given enum value
*
* INPUTS:
*   transport == ses_transport_t enum value
*
* RETURNS:
*   pointer to the string value for the specified enum
*********************************************************************/
const xmlChar *
    ses_get_transport_name (ses_transport_t transport)
{
    /* needs to match netconf-state DM values */
    switch (transport) {
    case SES_TRANSPORT_NONE:
        return (const xmlChar *)"none";
    case SES_TRANSPORT_SSH:
        return (const xmlChar *)"netconf-ssh";
    case SES_TRANSPORT_BEEP:
        return (const xmlChar *)"netconf-beep";
    case SES_TRANSPORT_SOAP:
        return (const xmlChar *)"netconf-soap-over-https";
    case SES_TRANSPORT_SOAPBEEP:
        return (const xmlChar *)"netconf-soap-over-beep";
    case SES_TRANSPORT_TLS:
        return (const xmlChar *)"netconf-tls";
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return (const xmlChar *)"none";
    }

} /* ses_get_transport_name */


/********************************************************************
* FUNCTION ses_set_xml_nons
* 
*  force xmlns attributes to be skipped in XML mode
*
* INPUTS:
*    scb == session to set
*
*********************************************************************/
void
    ses_set_xml_nons (ses_cb_t *scb)
{
    scb->noxmlns = TRUE;

} /* ses_set_xml_nons */


/********************************************************************
* FUNCTION ses_get_xml_nons
* 
*  force xmlns attributes to be skipped in XML mode
*
* INPUTS:
*    scb == session to get
*
* RETURNS:
*   TRUE if no xmlns attributes set
*   FALSE if OK to use xmlns attributes
*********************************************************************/
boolean
    ses_get_xml_nons (const ses_cb_t *scb)
{
    return scb->noxmlns;

} /* ses_get_xml_nons */


/********************************************************************
* FUNCTION ses_set_protocol
* 
*  set the NETCONF protocol version in use
*
* INPUTS:
*    scb == session to set
*    proto == protocol to set
* RETURNS:
*    status
*********************************************************************/
status_t
    ses_set_protocol (ses_cb_t *scb,
                      ncx_protocol_t proto)
{
#ifdef DEBUG
    if (scb == NULL) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif
    if (proto == NCX_PROTO_NONE) {
        return ERR_NCX_INVALID_VALUE;
    }
    if (scb->protocol != NCX_PROTO_NONE) {
        return ERR_NCX_DUP_ENTRY;
    }
    scb->protocol = proto;
    if (scb->transport == SES_TRANSPORT_SSH &&
        proto == NCX_PROTO_NETCONF11) {
        scb->framing11 = TRUE;
    }

    if (scb->outbuff != NULL && scb->framing11) {
        if (scb->outbuff->bufflen != 0) {
            SET_ERROR(ERR_INTERNAL_VAL);
        }
        ses_msg_init_buff(scb, TRUE, scb->outbuff);
    }

    return NO_ERR;

}  /* ses_set_protocol */


/********************************************************************
* FUNCTION ses_get_protocol
* 
*  Get the NETCONF protocol set (or unset) for this session
*
* INPUTS:
*    scb == session to get
*
* RETURNS:
*   protocol enumeration in use
*********************************************************************/
ncx_protocol_t
    ses_get_protocol (const ses_cb_t *scb)
{
#ifdef DEBUG
    if (scb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_PROTO_NONE;
    }
#endif
    return scb->protocol;

}  /* ses_get_protocol */


/********************************************************************
* FUNCTION ses_set_protocols_requested
* 
*  set the NETCONF protocol versions requested
*
* INPUTS:
*    scb == session to set
*    proto == protocol to set
* RETURNS:
*    status
*********************************************************************/
void
    ses_set_protocols_requested (ses_cb_t *scb,
                                 ncx_protocol_t proto)
{
#ifdef DEBUG
    if (scb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif
    switch (proto) {
    case NCX_PROTO_NETCONF10:
        scb->protocols_requested |= NCX_FL_PROTO_NETCONF10;
        break;
    case NCX_PROTO_NETCONF11:
        scb->protocols_requested |= NCX_FL_PROTO_NETCONF11;
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* ses_set_protocols_requested */


/********************************************************************
* FUNCTION ses_protocol_requested
* 
*  check if the NETCONF protocol version was requested
*
* INPUTS:
*    scb == session to check
*    proto == protocol to check
* RETURNS:
*    TRUE is requested; FALSE otherwise
*********************************************************************/
boolean
    ses_protocol_requested (ses_cb_t *scb,
                            ncx_protocol_t proto)
{
    boolean ret = FALSE;

#ifdef DEBUG
    if (scb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif
    switch (proto) {
    case NCX_PROTO_NETCONF10:
        if (scb->protocols_requested & NCX_FL_PROTO_NETCONF10) {
            ret = TRUE;
        }
        break;
    case NCX_PROTO_NETCONF11:
        if (scb->protocols_requested & NCX_FL_PROTO_NETCONF11) {
            ret = TRUE;
        }
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }
    return ret;

}  /* ses_protocol_requested */




/* END file ses.c */

