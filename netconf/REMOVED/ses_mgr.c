/*  FILE: ses_mgr.c

   NETCONF Session Manager: Manager Side Support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
19may05      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_hello
#include  "hello.h"
#endif

#ifndef _H_rpc
#include  "rpc.h"
#endif

#ifndef _H_rpc_mgr
#include  "rpc_mgr.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_ses_mgr
#include  "ses_mgr.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define SES_MGR_RING_SIZE   64

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
static dlq_hdr_t     sessions[SES_MGR_RING_SIZE];
static boolean       shutting_down;


#ifdef NOT_YET
/********************************************************************
* FUNCTION find_session
*
* Find a session in the active sessions array
* This function does not remove the session from its queue
*
* INPUTS:
*   ses_id == the session-id of the session to find
* RETURNS:
*   pointer to session control block or NULL if not found
*********************************************************************/
static ses_cb_t * 
    find_session (uint32 ses_id)
{
    ses_cb_t  *scb;

    for (scb = (ses_cb_t *)
	     dlq_firstEntry(&sessions[ses_id % SES_MGR_RING_SIZE]);
	 scb != NULL;
	 scb = (ses_cb_t *)dlq_nextEntry(scb)) {
	if (scb->sid == ses_id) {
	    return scb;
	}
    }
    return NULL;

} /* find_session */
#endif


/********************************************************************
* FUNCTION close_all_sessions
*
* Close all open sessions
*
* INPUTS:
*   none
* RETURNS:
*   none
*********************************************************************/
static 
    void close_all_sessions (void)
{

} /* close_all_sessions */


/********************************************************************
* FUNCTION ses_mgr_init
*
* Initialize the session manager module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status == NO_ERR if all okay
*********************************************************************/
status_t 
    ses_mgr_init (void)
{
    uint32 i;

    /* initialize the sessions-in-progress array of queues */
    for (i=0; i<SES_MGR_RING_SIZE; i++) {
	dlq_createSQue(&sessions[i]);
    }
    shutting_down = FALSE;
    return NO_ERR;

}  /* ses_mgr_init */


/********************************************************************
* FUNCTION ses_mgr_cleanup
*
* Cleanup the session manager module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    ses_mgr_cleanup (void)
{
#ifdef NOT_YET
    uint32 i;
    ses_cb_t    *scb;
#endif

    shutting_down = TRUE;

#ifdef NOT_YET
    for (i=0; i<SES_MGR_RING_SIZE; i++) {
	while ((scb = (ses_cb_t *)dlq_deque(&reqs[i])) != NULL) {
	    free_scb(scb);
	}
    }
#endif

    close_all_sessions();

}  /* ses_mgr_cleanup */


/********************************************************************
* FUNCTION ses_mgr_open
*
* Open a NETCONF session in the manager role
*
* INPUTS:
*   peer == NETCONF agent to connect with
*   user == user to use for security, AAA purposes
*   group_id == 0 if not used, non-zero == transaction group ID
*               to use in RPC requests
* OUTPUTS:
*   peer_caps == filled in with the agents capabilities if
*                return is NO_ERR
*   scb == session control block filled in and active if 
*                return is NO_ERR
* RETURNS:
*   NO_ERR if all okay, Some error otherwise:
*   parameter, connection, session, capabilities-mismatch, security, etc
*********************************************************************/
status_t 
    ses_mgr_open (net_device_t  *peer,
		  net_user_t    *user,
		  uint32        group_id,
		  cap_list_t    *peer_caps,
		  ses_cb_t      *scb)
{
    return ERR_NCX_OPERATION_NOT_SUPPORTED;
}


/********************************************************************
* FUNCTION 
*
* 
*
* INPUTS:
*   
* RETURNS:
*   
*********************************************************************/
void 
    ses_mgr_close (ses_cb_t *scb)
{

}


/********************************************************************
* FUNCTION 
*
* 
*
* INPUTS:
*   
* RETURNS:
*   
*********************************************************************/
void 
    ses_mgr_kill (ses_id_t session_id)
{
}


#ifdef NOT_YET
/********************************************************************
* FUNCTION 
*
* 
*
* INPUTS:
*   
* RETURNS:
*   
*********************************************************************/
status_t 
    ses_mgr_send (ses_cb_t  *scb,
		  buf_buffer_t *buf)
{
    return ERR_NCX_OPERATION_NOT_SUPPORTED;
}
#endif


/* END file ses_mgr.c */
