#ifndef _H_ses_msg
#define _H_ses_msg
/*  FILE: ses_msg.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   NETCONF Session Message Common definitions module

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
20-jan-07    abb      Begun.
*/

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/



/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/
extern void 
    ses_msg_init (void);

extern void 
    ses_msg_cleanup (void);

extern status_t
    ses_msg_new_msg (ses_cb_t *scb, 
		     ses_msg_t **msg);

extern void
    ses_msg_free_msg (ses_cb_t *scb,
		      ses_msg_t *msg);

extern status_t
    ses_msg_new_buff (ses_cb_t *scb, 
		      ses_msg_buff_t **buff);

extern void
    ses_msg_free_buff (ses_cb_t *scb,
		       ses_msg_buff_t *buff);

extern status_t
    ses_msg_write_buff (ses_msg_buff_t *buff,
			uint32 ch);
extern status_t
    ses_msg_send_buff (int fd,
		       ses_msg_buff_t *buff);

extern status_t
    ses_msg_send_buffs (ses_cb_t *scb);

extern status_t
    ses_msg_new_output_buff (ses_cb_t *scb);

extern void
    ses_msg_make_inready (ses_cb_t *scb);

extern void
    ses_msg_make_outready (ses_cb_t *scb);

extern void
    ses_msg_finish_outmsg (ses_cb_t *scb);

extern ses_ready_t *
    ses_msg_get_first_inready (void);

extern ses_ready_t *
    ses_msg_get_first_outready (void);

extern void
    ses_msg_dump (const ses_msg_t *msg,
		  const xmlChar *text);

#endif            /* _H_ses_msg */
