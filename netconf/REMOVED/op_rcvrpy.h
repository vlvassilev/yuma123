#ifndef _H_op_rcvrpy
#define _H_op_rcvrpy
/*  FILE: op_rcvrpy.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol operations: receive rpc-reply PDU callbacks

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
05-may-05    abb      Begun.
*/

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/


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

#ifdef NOT_YET
extern status_t op_rcvrpy_get_config (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_edit_config (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_copy_config (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_delete_config (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_lock (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_unlock (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_get (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_close_session (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_kill_session (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_commit (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_discard_changes (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

extern status_t op_rcvrpy_validate (
      xml_ns_id_t              ns_id,
      const unsigned char     *method_nm,
      const rpc_rpy_t         *rpy);

#endif /* NOT_YET */

#endif            /* _H_op_rcvrpy */
