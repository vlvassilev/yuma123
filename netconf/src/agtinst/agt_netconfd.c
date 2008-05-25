/*  FILE: agt_netconfd.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
29apr06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_cap
#include "agt_cap.h"
#endif

#ifndef _H_agt_cb
#include "agt_cb.h"
#endif

#ifndef _H_agt_cli
#include "agt_cli.h"
#endif

#ifndef _H_agt_ncx
#include "agt_ncx.h"
#endif

#ifndef _H_agt_netconfd
#include "agt_netconfd.h"
#endif

#ifndef _H_agt_ps
#include "agt_ps.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
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

#define AGT_NETCONFD_DEBUG 1


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


#ifdef NOT_YET
/********************************************************************
* FUNCTION filelist_callback
*
* FileList typdef callback function
*
* INPUTS:
*    see agt/agt_cb.h  (agt_cb_tcb_t)
* RETURNS:
*    status
*********************************************************************/
static status_t 
    filelist_callback (ses_cb_t  *scb,
		       rpc_msg_t  *msg,
		       agt_cbtyp_t cbtyp,
		       op_editop_t  editop,
		       val_value_t  *newval,
		       val_value_t  *curval)
{
    const xmlChar *str;
    uint32         strcnt, i;

#ifdef AGT_NETCONFD_DEBUG
    log_debug("\nagt_netconfd: filelist_cb n:%s s:%d", newval->name, cbtyp);
#endif

    switch (cbtyp) {
    case AGT_CB_LOAD:
	/* this data type has static values and does not
	 * need any setup or cleanup callbacks
	 */
	break;
    case AGT_CB_VALIDATE:
    case AGT_CB_APPLY:
	/* validate the cfg-path parm if present */
	strcnt = val_liststr_count(newval);
	for (i = 0; i < strcnt; i++) {
	    str = val_get_liststr(newval, i);
	    if (!str) {
		return SET_ERROR(ERR_INTERNAL_PTR);
	    }
	    if (cbtyp==AGT_CB_VALIDATE) {
		; /* check if the directory exists */

	    } else {
		; /* apply stage */
	    }
	}
	break;
    case AGT_CB_ROLLBACK:
	/* the automatic rollback is sufficient for this parmset */
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    return NO_ERR;

} /* filelist_callback */
#endif


/********************************************************************
* FUNCTION ncboot_callback
*
* ncboot parmset load callback function
*
* INPUTS:
*    see agt/agt_cb.h  (agt_cb_pscb_t)
* RETURNS:
*    status
*********************************************************************/
static status_t 
    ncboot_callback (ses_cb_t  *scb,
		     rpc_msg_t  *msg,
		     agt_cbtyp_t cbtyp,
		     op_editop_t  editop,
		     ps_parmset_t  *newps,
		     ps_parmset_t  *curps)
{

#ifdef AGT_NETCONFD_DEBUG
    log_debug("\nagt_netconfd: ncboot_cb n:%s s:%d", 
	   newps->psd->name, cbtyp);
#endif

    switch (cbtyp) {
    case AGT_CB_LOAD:
	/* this parmset has static values and does not
	 * need any setup or cleanup callbacks
	 */
	break;
    case AGT_CB_VALIDATE:
    case AGT_CB_APPLY:
	break;
    case AGT_CB_ROLLBACK:
	/* the automatic rollback is sufficient for this parmset */
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    
    return NO_ERR;

} /* ncboot_callback */


/********************************************************************
* FUNCTION register_netconfd_callbacks
*
* Register the agent callback functions for this module
*
* RETURNS:
*    status, NO_ERR if all registered okay
*********************************************************************/
static status_t 
    register_netconfd_callbacks (void)
{
    status_t  res;

    /* netconfd parmset : 1 callback for all cb types */
    res = agt_cb_register_ps_callback(AGT_CLI_MODULE, NCX_PS_NC_BOOT,
				      FORALL, 0, ncboot_callback);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

#ifdef NOT_YET
    /* register the FileList validate callback function */
    res = agt_cb_register_typ_callback(AGT_CLI_MODULE,
				       NCX_TYP_FILELIST, 
				       FORONE, AGT_CB_VALIDATE,
				       filelist_callback);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }
#endif

    return NO_ERR;

} /* register_netconfd_callbacks */


/********************************************************************
* FUNCTION unregister_netconfd_callbacks
*
* Unregister the agent callback functions for this module
*
*********************************************************************/
static void
    unregister_netconfd_callbacks (void)
{

    /* netconfd parmset : 1 callback for all cb types */
    agt_cb_unregister_ps_callback(NCX_APP_NCAGENT, NCX_PS_NC_BOOT);

#ifdef NOT_YET
    agt_cb_unregister_typ_callback(NCX_APP_NCAGENT, NCX_TYP_FILELIST);
#endif

} /* unregister_netconfd_callbacks */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_netconfd_init
* 
* Initialize the NCX Agent Boot parmset method routines
* 
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    agt_netconfd_init (void)
{
    status_t  res;

    res = register_netconfd_callbacks();
    if (res != NO_ERR) {
	unregister_netconfd_callbacks();
    }
    return res;

}  /* agt_netconfd_init */


/********************************************************************
* FUNCTION agt_netconfd_cleanup
* 
* Cleanup the NCX Agent Parmset method routines
* 
*
*********************************************************************/
void
    agt_netconfd_cleanup (void)
{
    unregister_netconfd_callbacks();

}  /* agt_netconfd_cleanup */


/* END file agt_netconfd.c */
