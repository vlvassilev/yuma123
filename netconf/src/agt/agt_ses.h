#ifndef _H_agt_ses
#define _H_agt_ses
/*  FILE: agt_ses.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   NETCONF Session Agent definitions module

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
06-jun-06    abb      Begun.
*/

#include <xmlstring.h>

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif


/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

#define AGT_SES_MODULE      (const xmlChar *)"sessions"

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

extern status_t
    agt_ses_init (void);

extern status_t
    agt_ses_init2 (void);

extern void 
    agt_ses_cleanup (void);

extern ses_cb_t *
    agt_ses_new_dummy_session (void);

extern void
    agt_ses_free_dummy_session (ses_cb_t *scb);

extern ses_cb_t *
    agt_ses_new_session (ses_transport_t transport,
			 int fd);

extern void
    agt_ses_free_session (ses_cb_t *scb);

extern boolean
    agt_ses_session_id_valid (ses_id_t  sid);

extern void
    agt_ses_request_close (ses_id_t sid);

extern void
    agt_ses_kill_session (ses_id_t sid);

extern int
    agt_ses_read_cb (void *context,
		     char *buffer,
		     int len);

extern status_t
    agt_ses_accept_input (ses_cb_t *scb);

extern boolean
    agt_ses_process_first_ready (void);

extern boolean
    agt_ses_ssh_port_allowed (uint16 port);

extern void
    agt_ses_fill_writeset (fd_set *fdset,
			   int *maxfdnum);

#endif            /* _H_agt_ses */
