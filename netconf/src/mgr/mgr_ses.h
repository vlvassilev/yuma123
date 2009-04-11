#ifndef _H_mgr_ses
#define _H_mgr_ses
/*  FILE: mgr_ses.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   NETCONF Session Manager definitions module

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
08-feb-07    abb      Begun; started from agt_ses.h
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
    mgr_ses_init (void);

extern void 
    mgr_ses_cleanup (void);


extern status_t
    mgr_ses_new_session (const xmlChar *user,
			 const xmlChar *password,
			 const xmlChar *target,
			 uint16 port,
			 ses_id_t *retsid);

extern void
    mgr_ses_free_session (ses_id_t sid);

extern boolean
    mgr_ses_process_first_ready (void);

extern uint32
    mgr_ses_fill_writeset (fd_set *fdset,
			   int *maxfdnum);

extern ssize_t
    mgr_ses_readfn (void *scb,
		    char *buff,
		    size_t bufflen);

extern status_t
    mgr_ses_writefn (void *scb);

extern ses_cb_t *
    mgr_ses_get_scb (ses_id_t sid);

extern mgr_scb_t *
    mgr_ses_get_mscb (ses_cb_t *scb);

#endif            /* _H_mgr_ses */
