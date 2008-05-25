/*  FILE: op_rcvrpy.c

   NETCONF Protocol Operations: rpc_mgr_rcvrpy_fn_t Callbacks

   These callbacks are used to receive rpc-reply messages for
   the associated request messages.

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
02may05      abb      begun

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

#ifndef _H_cap
#include  "cap.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_op
#include  "op.h"
#endif

#ifndef _H_op_rcvrpy
#include  "op_rcvrpy.h"
#endif

#ifndef _H_status
#include  "status.h"
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

#ifdef NOT_YET
/********************************************************************
* FUNCTION op_rcvrpy_get_config
*
* Handle a <get-config> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'get-config')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_get_config (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;

    if (!method_nm || !rpy) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 
    op = op_method_name(OP_GET_CONFIG);
    if (ns_id != xml_nc_id() || strcmp(method_nm, op)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }


} /* op_rcvrpy_get_config */


/********************************************************************
* FUNCTION op_rcvrpy_edit_config
*
* Handle a <edit-config> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'edit-config')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_edit_config (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;

    if (!method_nm || !param || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    op = op_method_name(OP_EDIT_CONFIG);
    if (ns_id != xml_nc_id() || strcmp(method_nm, op)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }


} /* op_rcvrpy_edit_config */


/********************************************************************
* FUNCTION op_rcvrpy_copy_config
*
* Handle a <copy-config> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'copy-config')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_copy_config (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;

} /* op_rcvrpy_copy_config */


/********************************************************************
* FUNCTION op_rcvrpy_delete_config
*
* Handle a <delete-config> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'delete-config')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_delete_config (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;


} /* op_rcvrpy_delete_config */


/********************************************************************
* FUNCTION op_rcvrpy_lock
*
* Handle a <lock> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'lock')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_lock (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;

    if (!method_nm || !rpy) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    op = op_method_name(OP_LOCK);
    if (ns_id != xml_nc_id() || strcmp(method_nm, op)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* op_rcvrpy_lock */


/********************************************************************
* FUNCTION op_rcvrpy_unlock
*
* Handle a <unlock> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'unlock')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_unlock (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;

} /* op_rcvrpy_unlock */


/********************************************************************
* FUNCTION op_rcvrpy_get
*
* Handle a <get> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'get')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_get (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;


} /* op_rcvrpy_get */


/********************************************************************
* FUNCTION op_rcvrpy_close_session
*
* Handle a <close-session> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'close-session')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_close_session (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    const unsigned char *op;

    if (!method_nm || !rpy) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    op = op_method_name(OP_CLOSE_SESSION);
    if (ns_id != xml_nc_id() || strcmp(method_nm, op)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* op_rcvrpy_close_session */


/********************************************************************
* FUNCTION op_rcvrpy_kill_session
*
* Handle a <kill-session> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'kill-session')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_kill_session (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;

} /* op_rcvrpy_kill_session */


/********************************************************************
* FUNCTION op_rcvrpy_commit
*
* Handle a <commit> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'commit')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_commit (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;

} /* op_rcvrpy_commit */


/********************************************************************
* FUNCTION op_rcvrpy_discard_changes
*
* Handle a <discard-changes> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'discard-changes')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_discard_changes (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      rpc_rpy_t               *rpy)
{
    status_t  res;
    const unsigned char *op;

    if (!method_nm || !rpy) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

} /* op_rcvrpy_discard_changes */


/********************************************************************
* FUNCTION op_rcvrpy_validate
*
* Handle a <validate> reply
*
* INPUTS:
*    ns_id == namespace ID (should be NETCONF)
*    method_nm == method name (should be 'validate')
*    rpy == reply from the other side
* RETURNS:
*    NO_ERR if no errors at all
*********************************************************************/
status_t op_rcvrpy_validate (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy)
{
    status_t  res;
    const unsigned char *op;

    if (!method_nm || !rpy) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 
    op = op_method_name(OP_VALIDATE);
    if (ns_id != xml_nc_id() || strcmp(method_nm, op)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }



} /* op_rcvrpy_validate */


#endif  /* NOT_YET */

/* END file op_rcvrpy.c */
