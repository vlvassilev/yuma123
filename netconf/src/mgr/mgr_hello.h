#ifndef _H_mgr_hello
#define _H_mgr_hello
/*  FILE: mgr_hello.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol hello message

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
17-may-05    abb      Begun.
*/

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/
#define NC_HELLO_STR    "hello"
#define NC_SESSION_ID   "session-id"

/********************************************************************
*                                                                   *
*                                    T Y P E S                      *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern status_t 
    mgr_hello_init (void);

extern void 
    mgr_hello_cleanup (void);

extern void 
    mgr_hello_dispatch (ses_cb_t *scb,
			xml_node_t *top);

extern status_t
    mgr_hello_send (ses_cb_t *scb);


#endif            /* _H_mgr_hello */

