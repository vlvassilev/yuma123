/*  FILE: mgr.c

     NETCONF Manager Access Library

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
03feb06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <libssh2.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_mgr
#include "mgr.h"
#endif

#ifndef _H_mgr_cap
#include "mgr_cap.h"
#endif

#ifndef _H_mgr_hello
#include "mgr_hello.h"
#endif

#ifndef _H_mgr_io
#include "mgr_io.h"
#endif

#ifndef _H_mgr_rpc
#include "mgr_rpc.h"
#endif

#ifndef _H_mgr_ses
#include "mgr_ses.h"
#endif

#ifndef _H_mgr_signal
#include "mgr_signal.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define MGR_DEBUG 1
#endif

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean mgr_init_done = FALSE;

static boolean mgr_shutdown;


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION mgr_init
* 
* Initialize the Manager Library
* 
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    mgr_init (void)
{
    status_t  res;

    if (mgr_init_done) {
	log_info("\nManager Init Redo skipped...");
	return NO_ERR;
    }

#ifdef MGR_DEBUG
    log_debug3("\nManager Init Starting...");
#endif

    mgr_shutdown = FALSE;

    res = mgr_cap_set_caps();
    if (res != NO_ERR) {
	return res;
    }

    res = mgr_rpc_init();
    if (res != NO_ERR) {
	return res;
    }

    res = mgr_hello_init();
    if (res != NO_ERR) {
	return res;
    }

    mgr_ses_init();
    mgr_io_init();
    mgr_signal_init();

    mgr_init_done = TRUE;
    return NO_ERR;

}  /* mgr_init */


/********************************************************************
* FUNCTION mgr_cleanup
*
* Cleanup the Manager Library
* 
* TBD -- put platform-specific manager cleanup here
*
*********************************************************************/
void
    mgr_cleanup (void)
{
    if (mgr_init_done) {
#ifdef MGR_DEBUG
        log_debug3("\nManager Cleanup Starting...\n");
#endif

	mgr_cap_cleanup();
	mgr_rpc_cleanup();
	mgr_ses_cleanup();
	mgr_hello_cleanup();
	mgr_signal_cleanup();
	mgr_shutdown = FALSE;
	mgr_init_done = FALSE;
    }
}   /* mgr_cleanup */


/********************************************************************
* FUNCTION mgr_new_scb
* 
* Malloc and Initialize the Manager Session Control Block
* 
* RETURNS:
*   manager session control block struct or NULL if malloc error
*********************************************************************/
mgr_scb_t *
    mgr_new_scb (void)
{
    mgr_scb_t *mscb;

    mscb = m__getObj(mgr_scb_t);
    if (mscb) {
	mgr_init_scb(mscb);
    }
    return mscb;

}  /* mgr_new_scb */


/********************************************************************
* FUNCTION mgr_init_scb
* 
* Initialize the Manager Session Control Block
* 
* INPUTS:
*   mscb == manager session control block struct to initialize
*
*********************************************************************/
void
    mgr_init_scb (mgr_scb_t *mscb)
{
#ifdef DEBUG
    if (!mscb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(mscb, 0x0, sizeof(mgr_scb_t));
    cap_init_caplist(&mscb->caplist);
    dlq_createSQue(&mscb->reqQ);
    mscb->next_id = 1;

}  /* mgr_init_scb */


/********************************************************************
* FUNCTION mgr_free_scb
* 
* Clean and Free a Manager Session Control Block
* 
* INPUTS:
*   mscb == manager session control block struct to free
*********************************************************************/
void
    mgr_free_scb (mgr_scb_t *mscb)
{
#ifdef DEBUG
    if (!mscb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    mgr_clean_scb(mscb);
    m__free(mscb);

}  /* mgr_free_scb */


/********************************************************************
* FUNCTION mgr_clean_scb
* 
* Clean a Manager Session Control Block
* 
* INPUTS:
*   mscb == manager session control block struct to clean
*********************************************************************/
void
    mgr_clean_scb (mgr_scb_t *mscb)
{
#ifdef DEBUG
    if (!mscb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    cap_clean_caplist(&mscb->caplist);

    if (mscb->root) {
	val_free_value(mscb->root);
	mscb->root = NULL;
    }

    if (mscb->lastroot) {
	val_free_value(mscb->lastroot);
	mscb->lastroot = NULL;
    }

    if (mscb->chtime) {
	m__free(mscb->chtime);
	mscb->chtime = NULL;
    }

    if (mscb->lastchtime) {
	m__free(mscb->lastchtime);
	mscb->lastchtime = NULL;
    }

    if (mscb->target) {
	m__free(mscb->target);
	mscb->target = NULL;
    }

    if (mscb->channel) {
	libssh2_channel_free(mscb->channel);
	mscb->channel = NULL;
    }

    if (mscb->session) {
	libssh2_session_disconnect(mscb->session, "SSH2 session closed");
	libssh2_session_free(mscb->session);
	mscb->session = NULL;
    }

    mgr_rpc_clean_requestQ(&mscb->reqQ);

}  /* mgr_clean_scb */


/********************************************************************
* FUNCTION mgr_request_shutdown
* 
* Request a manager shutdown
* 
*********************************************************************/
void
    mgr_request_shutdown (void)
{
    mgr_shutdown = TRUE;

}  /* mgr_request_shutdown */


/********************************************************************
* FUNCTION mgr_shutdown_requested
* 
* Check if a manager shutdown is in progress
* 
* RETURNS:
*    TRUE if shutdown mode has been started
*
*********************************************************************/
boolean
    mgr_shutdown_requested (void)
{
    return mgr_shutdown;

}  /* mgr_shutdown_requested */


/* END file mgr.c */
