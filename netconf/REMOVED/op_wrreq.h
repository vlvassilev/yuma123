#ifndef _H_op_wrreq
#define _H_op_wrreq
/*  FILE: op_wrreq.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol operations: write request PDU callbacks

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
02-may-05    abb      Begun.
*/

#ifndef _H_buf
#include "buf.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

/* non-floating point number buffer size used in op_wrreq.c */
#define OP_NUM_LEN 16


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
    op_wrreq_get_config (const op_source_t * source,
			 const op_filter_t * filter,
			 xml_attrs_t * attrs,
			 buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_edit_config (const op_target_t * target,
			  op_defop_t    defop,
			  op_testop_t   testop,
			  op_errop_t    errop,
			  const op_source_t  * config,
			  xml_attrs_t * attrs,
			  buf_buffer_t       * buffer);

extern status_t 
    op_wrreq_copy_config (const op_source_t * source,
			  const op_target_t * target,
			  xml_attrs_t * attrs,
			  buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_delete_config (const op_target_t * target,
			    xml_attrs_t * attrs,
			    buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_lock (const op_target_t * target,
		   xml_attrs_t * attrs,
		   buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_unlock (const op_target_t * target,
		     xml_attrs_t * attrs,
		     buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_get (const op_filter_t * filter,
		  xml_attrs_t * attrs,
		  buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_close_session (xml_attrs_t * attrs,
			    buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_kill_session (uint32          session_id,
			   xml_attrs_t * attrs,
			   buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_commit (boolean             confirmed,
		     uint32            timeout,
		     xml_attrs_t * attrs,
		     buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_discard_changes (xml_attrs_t * attrs,
			      buf_buffer_t      * buffer);

extern status_t 
    op_wrreq_validate (op_source_t       * source,
		       xml_attrs_t * attrs,
		       buf_buffer_t      * buffer);

#endif            /* _H_op_wrreq */
