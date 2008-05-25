/*  FILE: rpc.c

   NETCONF RPC Operations

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
09nov05      abb      begun

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

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_op
#include  "op.h"
#endif

#ifndef _H_rpc
#include  "rpc.h"
#endif

#ifndef _H_rpc_err
#include  "rpc_err.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_msg
#include  "xml_msg.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


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


/********************************************************************
* FUNCTION rpc_new_template
*
* Malloc and initialize a new rpc_template_t struct
*
* INPUTS:
*   none
* RETURNS:
*   pointer to struct or NULL or memory error
*********************************************************************/
rpc_template_t *
    rpc_new_template (void)
{
    rpc_template_t *rpc;

    rpc = m__getObj(rpc_template_t);
    if (!rpc) {
        return NULL;
    }
    memset(rpc, 0x0, sizeof(rpc_template_t));
    dlq_createSQue(&rpc->appinfoQ);
    rpc->supported = TRUE;
    return rpc;

} /* rpc_new_template */


/********************************************************************
* FUNCTION rpc_free_template
*
* Free all the memory used by the specified rpc_template_t
*
* INPUTS:
*   rpc == rpc_template_t to clean and delete
* RETURNS:
*   none
*********************************************************************/
void 
    rpc_free_template (rpc_template_t *rpc)
{
    ncx_appinfo_t  *appinfo;

#ifdef DEBUG
    if (!rpc) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (rpc->name) {
        m__free(rpc->name);
    }
    if (rpc->in_modstr) {
        m__free(rpc->in_modstr);
    }
    if (rpc->in_psd_name) {
        m__free(rpc->in_psd_name);
    } else if (rpc->in_psd) {
	/* NULL name and non-NULL PSD --> internal parmset */
	psd_free_template(rpc->in_psd);
    }
    if (rpc->out_modstr) {
        m__free(rpc->out_modstr);
    }
    if (rpc->out_data_name) {
        m__free(rpc->out_data_name);
    }
    if (rpc->descr) {
        m__free(rpc->descr);
    }
    if (rpc->condition) {
        m__free(rpc->condition);
    }
    while (!dlq_empty(&rpc->appinfoQ)) {
	appinfo = (ncx_appinfo_t *)dlq_deque(&rpc->appinfoQ);
	ncx_free_appinfo(appinfo);
    }
	
    m__free(rpc);

} /* rpc_free_template */


/********************************************************************
* FUNCTION rpc_new_msg
*
* Malloc and initialize a new rpc_msg_t struct
*
* INPUTS:
*   none
* RETURNS:
*   pointer to struct or NULL or memory error
*********************************************************************/
rpc_msg_t *
    rpc_new_msg (void)
{
    rpc_msg_t *msg;

    msg = m__getObj(rpc_msg_t);
    if (!msg) {
        return NULL;
    }
    rpc_init_msg(msg);
    return msg;

} /* rpc_new_msg */


/********************************************************************
* FUNCTION rpc_new_out_msg
*
* Malloc and initialize a new rpc_msg_t struct for output
* or for dummy use
*
* INPUTS:
*   none
* RETURNS:
*   pointer to struct or NULL or memory error
*********************************************************************/
rpc_msg_t *
    rpc_new_out_msg (void)
{
    rpc_msg_t *msg;

    msg = rpc_new_msg();
    if (!msg) {
        return NULL;
    }

    msg->rpc_incoming = FALSE;
    msg->rpc_nsid = 0;    /* no default NS !! */
    msg->rpc_module = NCX_DEF_MODULE;
    msg->rpc_in_attrs = NULL;
    msg->rpc_group.u = 0;
    msg->rpc_msg_id = (const xmlChar *)"1";

    return msg;

} /* rpc_new_out_msg */


/********************************************************************
* FUNCTION rpc_init_msg
*
* Initialize a new rpc_msg_t struct
*
* INPUTS:
*   msg == rpc_msg_t memory to initialize
* RETURNS:
*   none
*********************************************************************/
void
    rpc_init_msg (rpc_msg_t *msg)
{
#ifdef DEBUG
    if (!msg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(msg, 0x0, sizeof(rpc_msg_t));
    xml_msg_init_hdr(&msg->mhdr);
    xml_init_attrs(&msg->rpc_attrs);
    ps_init_parmset(&msg->rpc_input);
    dlq_createSQue(&msg->rpc_undoQ);

} /* rpc_init_msg */


/********************************************************************
* FUNCTION rpc_free_msg
*
* Free all the memory used by the specified rpc_msg_t
*
* INPUTS:
*   msg == rpc_msg_t to clean and delete
* RETURNS:
*   none
*********************************************************************/
void 
    rpc_free_msg (rpc_msg_t *msg)
{
#ifdef DEBUG
    if (!msg) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    rpc_clean_msg(msg);
    m__free(msg);

} /* rpc_free_msg */


/********************************************************************
* FUNCTION rpc_clean_msg
*
* Clean all the memory used by the specified rpc_msg_t
* but do not free the struct itself
*
* INPUTS:
*   msg == rpc_msg_t to clean
* RETURNS:
*   none
*********************************************************************/
void 
    rpc_clean_msg (rpc_msg_t *msg)
{
    rpc_undo_rec_t *undo;

#ifdef DEBUG
    if (!msg) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    xml_msg_clean_hdr(&msg->mhdr);

    msg->rpc_group.u = 0;
    msg->rpc_msg_id = NULL;
    msg->rpc_nsid = 0;
    msg->rpc_module = NULL;
    msg->rpc_in_attrs = NULL;

    /* clean rpc_attrs -- used by manager only */
    xml_clean_attrs(&msg->rpc_attrs);

    msg->rpc_meth_nsid = 0;
    msg->rpc_meth_name = NULL;
    msg->rpc_method = NULL;
    msg->rpc_agt_state = 0;

    /* clean input parameter set */
    ps_clean_parmset(&msg->rpc_input);
    msg->rpc_status = NO_ERR;
    msg->rpc_user1 = NULL;
    msg->rpc_user2 = NULL;

    
    if (msg->rpc_data) {
	val_free_value((val_value_t *)msg->rpc_data);
	msg->rpc_data = NULL;
    }
    
    msg->rpc_filter.op_filtyp = OP_FILTER_NONE;
    msg->rpc_filter.op_filter = NULL;
    msg->rpc_datacb = NULL;

    /* clean undo queue */
    while (!dlq_empty(&msg->rpc_undoQ)) {
	undo = (rpc_undo_rec_t *)dlq_deque(&msg->rpc_undoQ);
	rpc_free_undorec(undo);
    }

    msg->rpc_incoming = FALSE;

} /* rpc_clean_msg */


/********************************************************************
* FUNCTION ncx_get_access_enum
* 
* Get the enum for the string name of a ncx_access_t enum
* 
* INPUTS:
*   str == string name of the enum value 
*
* RETURNS:
*   enum value
*********************************************************************/
const xmlChar *
    rpc_get_rpctype_str (rpc_type_t rpctyp)
{
    switch (rpctyp) {
    case RPC_TYP_OTHER:
	return NCX_EL_OTHER;
    case RPC_TYP_CONFIG:
	return NCX_EL_CONFIG;
    case RPC_TYP_EXEC:
	return NCX_EL_EXEC;
    case RPC_TYP_MONITOR:
	return NCX_EL_MONITOR;	
    case RPC_TYP_DEBUG:
	return NCX_EL_DEBUG;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NCX_EL_NONE;
    }
    /*NOTREACHED*/

}  /* rpc_get_rpctype_str */


/********************************************************************
* FUNCTION rpc_new_undorec
*
* Malloc and initialize a new rpc_undo_rec_t struct
*
* INPUTS:
*   none
* RETURNS:
*   pointer to struct or NULL or memory error
*********************************************************************/
rpc_undo_rec_t *
    rpc_new_undorec (void)
{
    rpc_undo_rec_t *undo;

    undo = m__getObj(rpc_undo_rec_t);
    if (!undo) {
        return NULL;
    }
    rpc_init_undorec(undo);
    return undo;

} /* rpc_new_undorec */


/********************************************************************
* FUNCTION rpc_init_undorec
*
* Initialize a new rpc_undo_rec_t struct
*
* INPUTS:
*   undo == rpc_undo_rec_t memory to initialize
* RETURNS:
*   none
*********************************************************************/
void
    rpc_init_undorec (rpc_undo_rec_t *undo)
{
#ifdef DEBUG
    if (!undo) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(undo, 0x0, sizeof(rpc_undo_rec_t));

} /* rpc_init_undorec */


/********************************************************************
* FUNCTION rpc_free_undorec
*
* Free all the memory used by the specified rpc_undo_rec_t
*
* INPUTS:
*   undo == rpc_undo_rec_t to clean and delete
* RETURNS:
*   none
*********************************************************************/
void 
    rpc_free_undorec (rpc_undo_rec_t *undo)
{
#ifdef DEBUG
    if (!undo) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    rpc_clean_undorec(undo);
    m__free(undo);

} /* rpc_free_undorec */


/********************************************************************
* FUNCTION rpc_clean_undorec
*
* Clean all the memory used by the specified rpc_undo_rec_t
* but do not free the struct itself
*
*  !!! The caller must free internal pointers that were malloced
*  !!! instead of copied.  This function does not check them!!!
*  
* INPUTS:
*   undo == rpc_undo_rec_t to clean
* RETURNS:
*   none
*********************************************************************/
void 
    rpc_clean_undorec (rpc_undo_rec_t *undo)
{
#ifdef DEBUG
    if (!undo) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (undo->curnode) {
	switch (undo->curnodetyp) {
	case NCX_NT_VAL:
	    val_free_value((val_value_t *)undo->curnode);
	    break;
	case NCX_NT_PARM:
	    ps_free_parm((ps_parm_t *)undo->curnode);
	    break;
	default:
	    break;
	}
    }
    memset(undo, 0x0, sizeof(rpc_undo_rec_t));

} /* rpc_clean_undorec */


/* END file rpc.c */
