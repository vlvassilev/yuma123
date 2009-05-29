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
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>
#include  <unistd.h>
#include  <errno.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_log
#include  "log.h"
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
#endif

#define LTSTR     ((const xmlChar *)"&lt;")
#define GTSTR     ((const xmlChar *)"&gt;")
#define AMPSTR    ((const xmlChar *)"&amp;")

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
static ses_total_stats_t totals;

/********************************************************************
* FUNCTION accept_buffer
*
* Handle one input buffer within the ses_accept_input function
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
    accept_buffer (ses_cb_t *scb,
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

#ifdef SES_DEBUG
    if (LOGDEBUG3) {
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
     * TBD: ses how xmlReader handles non-well-frmed XML 
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

	/* handle the char just copied into the buffer
	 * based on the input state 
	 */
	switch (scb->instate) {
	case SES_INST_IDLE:
	    /* check for EOM if SSH or just copy the char */
	    if (scb->transport==SES_TRANSPORT_SSH && ch==*endmatch) {
		scb->instate = SES_INST_INEND;
		scb->inendpos = 1;
	    } else {
		scb->instate = SES_INST_INMSG;
	    }
	    break;
	case SES_INST_INMSG:
	    /* check for EOM if SSH or just copy the char */
	    if (scb->transport==SES_TRANSPORT_SSH && ch==*endmatch) {
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
		     */
		    if (buff->buffpos == buff->bufflen-1 &&
			buff->buff[buff->buffpos] == '\0') {
			/* don't barf if the client sends a 
			 * zero-terminated string instead of
			 * just the contents of the string
			 */
			buff->bufflen--;
		    }

		    /* handle any bytes left at the end of 'buff' */
		    if (buff->buffpos < buff->bufflen) {

			/* get a new buffer to hold the overflow */
			res = ses_msg_new_buff(scb, &buff2);
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
			    scb->stats.in_drop_msgs++;
			    totals.stats.in_drop_msgs++;

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
			}
		    }

		    /* reset reader state */
		    scb->instate = SES_INST_IDLE;
		    scb->inendpos = 0;

		    /* check if more work to do */
		    if (buff2) {
			buff = buff2;
			msg = msg2;
		    } else {
			done = TRUE;
		    }
		}
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

}  /* accept_buffer */


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
    scb->withmeta = NCX_DEF_WITHMETA;
    scb->indent = NCX_DEF_INDENT;
    scb->xmladvance = TRUE;
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
* THIS FUNCTION DOES NOT CHECK ANY PARAMTERS TO SAVE TIME
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
	if (!scb->outbuff) {
	    res = ses_msg_new_buff(scb, &scb->outbuff);
	}
	if (scb->outbuff) {
	    buff = scb->outbuff;
	    res = ses_msg_write_buff(buff, ch);
	    if (res == ERR_BUFF_OVFL) {
		res = ses_msg_new_output_buff(scb);
		if (res == NO_ERR) {
		    buff = scb->outbuff;
		    res = ses_msg_write_buff(buff, ch);
		}
	    }
	} else {
	    res = ERR_NCX_OPERATION_FAILED;
	}

	if (res != NO_ERR) {
	    scb->stats.out_drop_bytes++;
	    totals.stats.out_drop_bytes++;
	} else {
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
* Write a zero-terminated content string to the session
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
    scb->mode = mode;  /***/
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

    /* Generate Start of XML Message Directive
     * hack to keep the xmlTextReader happy for now
     * It requires the first char to be a '\n' if
     * reading from the ses_read_cb function
     * by if it uses its own reader from a file,
     * the first char must be the '<' to start the XML directive
     */
    if (scb->fp || scb->mode==SES_MODE_XMLDOC) {
	ses_putstr(scb, XML_START_FILMSG);
    } else {
	ses_putstr(scb, XML_START_MSG);
    }

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
	ses_putstr(scb, (const xmlChar *)NC_SSH_END);
    }

    /* add a final newline when writing to a file or STDOUT */
    if (scb->fp || !scb->fd) {
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
* is encountered.  For SSH, also need to detect the EOM flag
* and remove it + control input to the reader.
*
* Uses a complex state machine which does not assume that the
* input from the network is going to arrive in well-formed chunks.
* It has to be treated as a byte stream (SOCK_STREAM).
*
* Does not remove char entities or any XML, just the SSH EOM directive
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
    ses_cb_t    *scb;
    ses_msg_t   *msg;
    ses_msg_buff_t  *buff, *buff2;
    int          retlen;
    boolean      done;

    scb = (ses_cb_t *)context;
    if (scb->state >= SES_ST_SHUTDOWN_REQ) {
	return -1;
    }
    msg = (ses_msg_t *)dlq_firstEntry(&scb->msgQ);
    if (!msg) {
	return 0;
    }
    buff = msg->curbuff;

    /* check if this is the first read */
    if (!buff) {
	buff = (ses_msg_buff_t *)dlq_firstEntry(&msg->buffQ);
	if (!buff) {
	    return 0;
	} else {
	    buff->buffpos = 0;
	    msg->curbuff = buff;
	}
    }

    /* check current buffer end has been reached */
    if (buff->buffpos == buff->bufflen) {
	buff = (ses_msg_buff_t *)dlq_nextEntry(buff);
	if (!buff) {
	    return 0;
	} else {
	    buff->buffpos = 0;
	    msg->curbuff = buff;
	}
    }

    /* start transferring bytes to the return buffer */
    retlen = 0;
    done = FALSE;
    while (!done) {

	buffer[retlen++] = (char)buff->buff[buff->buffpos++];

	/* check xmlreader buffer full */
	if (retlen==len) {
	    done = TRUE;
	    continue;
	}

	/* check current buffer end has been reached */
	if (buff->buffpos == buff->bufflen) {
	    buff2 = (ses_msg_buff_t *)dlq_nextEntry(buff);
	    if (!buff2) {
		done = TRUE;
		continue;
	    } else {
		buff = buff2;
		buff->buffpos = 0;
		msg->curbuff = buff;
	    }
	}
    }

    /* this hack is needed to reset the read buffer
     * after the newReaderForIO function is called
     * which reads 4 bytes and corruptsthe input stream
     * for the normal parse mode read functions.
     * Resetting the buffer back to the start seems to
     * fix the problem
     */
    if (len==4 && buff->buffpos==4) {
	buff->buffpos = 0;
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
    boolean         done;

#ifdef DEBUG
    if (!scb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef SES_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nses accept input for session %d", 
		   scb->sid);
    }
#endif

    res = NO_ERR;
    done = FALSE;

    while (!done) {
	if (scb->state >= SES_ST_SHUTDOWN_REQ) {
	    return ERR_NCX_SESSION_CLOSED;
	}

	/* get a new buffer */
	res = ses_msg_new_buff(scb, &buff);
	if (res != NO_ERR) {
	    return res;
	}

	/* read data into the new buffer */
	if (scb->rdfn) {
	    ret = (*scb->rdfn)(scb, 
			       (char *)buff->buff, 
			       SES_MSG_BUFFSIZE);
	    if (ret < 0) {
		/* !!! treat any read error as nothing to read !!! */
		/*** NEED TO FIX : WHEN AGENT DROPS SESSION
		 *** THEN THIS CODE NEEDS TO MAKE SURE THE
		 *** SSH SESSION IS STILL UP (-1 returned every error)
		 ***/
		ses_msg_free_buff(scb, buff);
		scb->retries++;
		if (scb->retries < SES_MAX_RETRIES) {
		    return NO_ERR;
		} else {
		    return ERR_NCX_READ_FAILED;
		}
	    }
	} else {
	    scb->retries = 0;
	    ret = read(scb->fd, buff->buff, SES_MSG_BUFFSIZE);
	    if (ret < 0 && errno == EAGAIN) {
		ses_msg_free_buff(scb, buff);
		return NO_ERR;
	    }
	}
	if (ret < 0) {
	    /* this should cause the reader to call the IO close fn
	     * this socket should not be selected unless there is 
	     * something to read or the connection is closed
	     */

#ifdef SES_DEBUG
	    if (LOGDEBUG2) {
		log_debug2("\nses read failed on session %d (%s)", 
			   scb->sid, strerror(errno));
	    }
#endif

	    ses_msg_free_buff(scb, buff);
	    return ERR_NCX_READ_FAILED;
	} else if (ret == 0) {
	    /* session closed by remote peer */
	    if (LOGINFO) {
		log_info("\nses: session %d shut by remote peer", 
			 scb->sid);
	    }
	    ses_msg_free_buff(scb, buff);
	    return ERR_NCX_SESSION_CLOSED;
	} else {

#ifdef SES_DEBUG
	    if (LOGDEBUG2) {
		log_debug2("\nses read OK (%d) on session %d", 
			   ret, 
			   scb->sid);
	    }
#endif
	    scb->retries = 0;
	    buff->bufflen = (size_t)ret;
	    scb->stats.in_bytes += (uint32)ret;
	    totals.stats.in_bytes += (uint32)ret;
	}

	res = accept_buffer(scb, buff);
	if (res != NO_ERR || ret < SES_MSG_BUFFSIZE || !scb->rdfn) {
	    done = TRUE;
	} /* else the SSH2 channel probably has more bytes to read */
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
    return scb->withdef;

} /* ses_withdef */


/********************************************************************
* FUNCTION ses_withmeta
*
* Get the with-metadata value for this session
*
* INPUTS:
*   scb == session control block to check
*
* RETURNS:
*   with-metadata value for the session
*********************************************************************/
boolean
    ses_withmeta (const ses_cb_t *scb)
{
    return scb->withmeta;

} /* ses_withmeta */


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
     scb == session to write
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
	return (const xmlChar *)"SSH";
    case SES_TRANSPORT_BEEP:
	return (const xmlChar *)"SSL";
    case SES_TRANSPORT_SOAP:
	return (const xmlChar *)"HTTPS";
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return (const xmlChar *)"none";
    }

} /* ses_get_transport_name */


/* END file ses.c */
