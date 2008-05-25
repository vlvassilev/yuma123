#ifndef _H_ses_mgr
#define _H_ses_mgr
/*  FILE: ses_mgr.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   NETCONF Session Manager: Manager Side Support

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
19-may-05    abb      Begun.
*/

#ifndef _H_buf
#include "buf.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
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

typedef xmlChar * net_user_t;
typedef xmlChar * net_device_t;

/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern status_t ses_mgr_init (void);

extern void ses_mgr_cleanup (void);

extern status_t ses_mgr_open (net_device_t  *peer,
			      net_user_t    *user,
			      uint32    group_id,
			      cap_list_t    *peer_caps,
			      ses_cb_t      *scb);

extern void ses_mgr_close (ses_cb_t *scb);

extern void ses_mgr_kill (ses_id_t session_id);

#ifdef NOT_YET
extern status_t 
    ses_mgr_send (ses_cb_t  *scb,
		  buf_buffer_t *buf);
#endif


#endif            /* _H_ses_mgr */
