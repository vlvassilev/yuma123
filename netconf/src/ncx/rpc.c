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

#ifndef _H_obj
#include  "obj.h"
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

    memset(msg, 0x0, sizeof(rpc_msg_t));
    xml_msg_init_hdr(&msg->mhdr);
    dlq_createSQue(&msg->rpc_dataQ);
    dlq_createSQue(&msg->rpc_undoQ);

    msg->rpc_input = val_new_value();
    if (!msg->rpc_input) {
	rpc_free_msg(msg);
	return NULL;
    }

    msg->rpc_top_editop = OP_EDITOP_MERGE;

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

    msg->rpc_in_attrs = NULL;

    return msg;

} /* rpc_new_out_msg */


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
    rpc_undo_rec_t *undo;
    val_value_t    *val;

#ifdef DEBUG
    if (!msg) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    xml_msg_clean_hdr(&msg->mhdr);

    msg->rpc_in_attrs = NULL;
    msg->rpc_method = NULL;
    msg->rpc_agt_state = 0;

    /* clean input parameter set */
    if (msg->rpc_input) {
	val_free_value(msg->rpc_input);
    }
    msg->rpc_user1 = NULL;
    msg->rpc_user2 = NULL;

    msg->rpc_filter.op_filtyp = OP_FILTER_NONE;
    msg->rpc_filter.op_filter = NULL;
    msg->rpc_datacb = NULL;

    /* clean data queue */
    while (!dlq_empty(&msg->rpc_dataQ)) {
	val = (val_value_t *)dlq_deque(&msg->rpc_dataQ);
	val_free_value(val);
    }

    /* clean undo queue */
    while (!dlq_empty(&msg->rpc_undoQ)) {
	undo = (rpc_undo_rec_t *)dlq_deque(&msg->rpc_undoQ);
	rpc_free_undorec(undo);
    }

    m__free(msg);

} /* rpc_free_msg */


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
    dlq_createSQue(&undo->extra_deleteQ);

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
    val_value_t *val;

#ifdef DEBUG
    if (!undo) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    undo->newnode = NULL;  /*********************/

    if (undo->curnode) {
	val_free_value(undo->curnode);
    }

    while (!dlq_empty(&undo->extra_deleteQ)) {
	val = (val_value_t *)
	    dlq_deque(&undo->extra_deleteQ);
	val_free_value(val);
    }

    rpc_init_undorec(undo);

} /* rpc_clean_undorec */


/* END file rpc.c */
