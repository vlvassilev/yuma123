/*  FILE: op_wrreq.c

   NETCONF Protocol Operations: rpc_mgr_wrreq_fn_t Callbacks

   These callbacks are used to generate NETCONF protocol operation
   request PDUs that will be handed to the transport manager for
   transmission.
		
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

#ifndef _H_op_wrreq
#include  "op_wrreq.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_mgr
#include "rpc_mgr.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
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
* FUNCTION write_source
*
* Generate a <source> parameter
*
* INPUTS:
*    buffer == buffer to fill
*    source == source parameter to write
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
static status_t 
    write_source (buf_buffer_t * buffer, 
		  const op_source_t * source)
{
    status_t  res;
    xmlns_id_t  ns_id;

    ns_id = xmlns_nc_id();

    /* write the parameter name start tag */
    res = buf_write_tag(buffer, ns_id, NCX_EL_SOURCE, 
                        BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the source parameter contents depending on the type */
    switch (source->op_srctyp){
    case OP_SOURCE_CONFIG:
	res = buf_write_elem(buffer, ns_id, 
	      op_config_name(source->op_src.op_config), NULL, NULL);
	break;
    case OP_SOURCE_INLINE:
	res = buf_write_indent(buffer, source->op_src.op_inline);
	break;
    case OP_SOURCE_URL:
	res = buf_write_elem(buffer, ns_id, NCX_EL_URL,
			     source->op_src.op_url, NULL);
	break;
    case OP_SOURCE_NONE:
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    if (res != NO_ERR) {
	return res;
    }

    /* reset the indent level */
    buf_dec_indent_level(buffer);

    /* write the parameter name end tag */
    return buf_write_tag(buffer, ns_id, 
        NCX_EL_SOURCE, BUF_END_TAG, NULL);

} /* write_source */


/********************************************************************
* FUNCTION write_target
*
* Generate a <target> parameter
*
* INPUTS:
*    buffer == buffer to fill
*    target == target parameter to write
*
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
static status_t 
    write_target (buf_buffer_t * buffer, 
		  const op_target_t * target)
{
    status_t  res;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    /* write the parameter name start tag */
    res = buf_write_tag(buffer, ns_id, 
        NCX_EL_TARGET, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the target parameter contents */
    switch (target->op_targtyp) {
    case OP_TARGET_CONFIG:
	res= buf_write_elem(buffer, ns_id, 
	    op_config_name(target->op_targ.op_config), NULL, NULL);
	break;
    case OP_TARGET_URL:
	res=buf_write_elem(buffer, ns_id, 
             NCX_EL_URL, target->op_targ.op_url, NULL);
	break;
    case OP_SOURCE_NONE:
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    if (res != NO_ERR) {
	return res;
    }

    /* reset the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    return buf_write_tag(buffer, ns_id, 
         NCX_EL_TARGET, BUF_END_TAG, NULL);

} /* write_target */


/********************************************************************
* FUNCTION write_filter
*
* Generate a <filter> parameter
*
* INPUTS:
*    buffer == buffer to fill
*    filter == filter struct to fill
* 
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
static status_t 
    write_filter (buf_buffer_t * buffer, 
		  const op_filter_t * filter)
{
    status_t             res;
    xml_attrs_t          attrs;
    const xmlChar *str;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    /* create an attribute queue */
    xml_init_attrs(&attrs);

    /* get the filter type parameter string */
    switch (filter->op_filtyp){
    case OP_FILTER_SUBTREE:
	str =  NCX_EL_SUBTREE;
	break;
    case OP_FILTER_XPATH:
	str =  NCX_EL_XPATH;
	break;
    case OP_FILTER_NONE:
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* add the attribute to the attrs queue */
    res = xml_add_attr(&attrs, xmlns_nc_id(), 
         NCX_EL_FILTER, str);
    if (res != NO_ERR) {
	return NO_ERR;
    }

    /* write the parameter name start tag */
    res = buf_write_tag(buffer, ns_id,  NCX_EL_FILTER, 
			BUF_START_TAG, &attrs);

    /* free the attrs memory used */
    xml_clean_attrs(&attrs);

    if (res != NO_ERR) {
	return res;
    }

    /* indent for the filter contents */
    buf_inc_indent_level(buffer);

    /* print the filter contents */
    switch (filter->op_filtyp){
    case OP_FILTER_SUBTREE:
	res = buf_write_indent(buffer, filter->op_filter);
	break;
    case OP_FILTER_XPATH:
	res = buf_write_elem(buffer, ns_id,  NCX_EL_URL, 
			     filter->op_filter, NULL);
	break;
    case OP_FILTER_NONE:
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    if (res != NO_ERR) {
	return res;
    }

    /* reset the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    return buf_write_tag(buffer, ns_id, 
         NCX_EL_FILTER, BUF_END_TAG, NULL);

} /* write_filter */


/********************************************************************
* FUNCTION op_wrreq_get_config
*
* Generate a <get-config> request PDU buffer
*
* INPUTS:
*    source == get this config
*    filter == selection criteria 
*              - NULL == no filter 
*              - filter type == OP_FILTER_NONE == no filter
*    attrs  == optional attribute list for rpc header
*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_get_config (const op_source_t *source,
			 const op_filter_t *filter,
			 xml_attrs_t *attrs,
			 buf_buffer_t *buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!source || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name start tag */
    op = op_method_name(OP_GET_CONFIG);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <source> parameter */
    res = write_source(buffer, source);
    if (res != NO_ERR) {
	return res;
    }

    /* write the <filter> parameter if needed */
    if (filter) {
	switch (filter->op_filtyp) {
	case OP_FILTER_NONE:
	    break;
	case OP_FILTER_SUBTREE:
	case OP_FILTER_XPATH:
	    res = write_filter(buffer, filter);
	    if (res != NO_ERR) {
		return res;
	    }
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* reset the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_get_config */


/********************************************************************
* FUNCTION op_wrreq_edit_config
*
* Generate a <edit-config> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
op_wrreq_edit_config (const op_target_t * target,
		      op_defop_t    defop,
		      op_testop_t   testop,
		      op_errop_t    errop,
		      const op_source_t  *config,
		      xml_attrs_t *attrs,
		      buf_buffer_t       *buffer)
{
    status_t  res;
    const xmlChar * op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!target || !config || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    /* write the rpc header */
    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name start tag */
    op = op_method_name(OP_EDIT_CONFIG);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <target> parameter */
    res = write_target(buffer, target);
    if (res != NO_ERR) {
	return res;
    }

    /* write the <default-operation> parameter */
    res = buf_write_elem(buffer, ns_id,  NCX_EL_DEFOP,
		 op_defop_name(defop), NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* write the <test-option> parameter */
    res = buf_write_elem(buffer, ns_id,  NCX_EL_TESTOP,
		 op_testop_name(testop), NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* write the <error-option> parameter */
    res = buf_write_elem(buffer, ns_id,  NCX_EL_ERROP,
		 op_errop_name(errop), NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* write the <config> parameter */
    res = buf_write_tag(buffer, ns_id,  NCX_EL_CONFIG, 
			BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* increase the indent level */
    buf_inc_indent_level(buffer);

    /* write the config parameter, based on the source type */
    switch (config->op_srctyp) {
    case OP_SOURCE_INLINE:
	if (config->op_src.op_inline) {
	    res = buf_write_indent(buffer, config->op_src.op_inline);
	    if (res != NO_ERR) {
		return res;
	    }
	}
	break;
    case OP_SOURCE_URL:
	res = buf_write_elem(buffer, ns_id,  NCX_EL_URL,
	     config->op_src.op_url, NULL);
	if (res != NO_ERR) {
	    return res;
	}
	break;
    case OP_SOURCE_CONFIG:
	/* the config-name is not allowed in this operation */
    case OP_SOURCE_NONE:
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the config end tag */
    res = buf_write_tag(buffer, ns_id, 
           NCX_EL_CONFIG, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_edit_config */


/********************************************************************
* FUNCTION op_wrreq_copy_config
*
* Generate a <copy-config> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_copy_config (const op_source_t *source,
			  const op_target_t *target,
			  xml_attrs_t *attrs,
			  buf_buffer_t *buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!source || !target || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }
		    
    /* write the method name start tag */
    op = op_method_name(OP_COPY_CONFIG);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <source> parameter */
    res = write_source(buffer, source);
    if (res != NO_ERR) {
	return res;
    }

    /* write the <target> parameter */
    res = write_target(buffer, target);
    if (res != NO_ERR) {
	return res;
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_copy_config */


/********************************************************************
* FUNCTION op_wrreq_delete_config
*
* Generate a <delete-config> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_delete_config (const op_target_t *target,
			    xml_attrs_t *attrs,
			    buf_buffer_t *buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!target || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name start tag */
    op = op_method_name(OP_DELETE_CONFIG);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <target> parameter */
    res = write_target(buffer, target);
    if (res != NO_ERR) {
	return res;
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_delete_config */


/********************************************************************
* FUNCTION op_wrreq_lock
*
* Generate a <lock> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_lock (const op_target_t *target,
		   xml_attrs_t *attrs,
		   buf_buffer_t *buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!target || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name start tag */
    op = op_method_name(OP_LOCK);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <target> parameter */
    res = write_target(buffer, target);
    if (res != NO_ERR) {
	return res;
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_lock */


/********************************************************************
* FUNCTION op_wrreq_unlock
*
* Generate an <unlock> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_unlock (const op_target_t * target,
		     xml_attrs_t * attrs,
		     buf_buffer_t      * buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!target || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name start tag */
    op = op_method_name(OP_UNLOCK);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <target> parameter */
    res = write_target(buffer, target);
    if (res != NO_ERR) {
	return res;
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_unlock */


/********************************************************************
* FUNCTION op_wrreq_get
*
* Generate a <get> request PDU buffer
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_get (const op_filter_t * filter,
		  xml_attrs_t * attrs,
		  buf_buffer_t      * buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name start tag */
    op = op_method_name(OP_GET);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* write the <filter> parameter if needed */
    if (filter) {
	switch (filter->op_filtyp) {
	case OP_FILTER_NONE:
	    break;
	case OP_FILTER_SUBTREE:
	case OP_FILTER_XPATH:
	    buf_inc_indent_level(buffer);
	    res = write_filter(buffer, filter);
	    if (res != NO_ERR) {
		return res;
	    }
	    buf_dec_indent_level(buffer);
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_get */


/********************************************************************
* FUNCTION op_wrreq_close_session
*
* Generate a <close-session> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_close_session (xml_attrs_t * attrs,
			    buf_buffer_t      * buffer)
{
    status_t res;
    const xmlChar *op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name as an empty element */
    op = op_method_name(OP_CLOSE_SESSION);
    res = buf_write_elem(buffer, ns_id, op, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_close_session */


/********************************************************************
* FUNCTION op_wrreq_kill_session
*
* Generate a <kil-session> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_kill_session (uint32       session_id,
			   xml_attrs_t * attrs,
			   buf_buffer_t      * buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlChar num[OP_NUM_LEN];
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name start tag */
    op = op_method_name(OP_KILL_SESSION);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <session-id> parameter */
    xmlStrPrintf(num, OP_NUM_LEN,  (const xmlChar *)"%lu", session_id);
    res = buf_write_elem(buffer, ns_id, 
            NCX_EL_SESSION_ID, num, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_kill_session */


/********************************************************************
* FUNCTION op_wrreq_commit
*
* Generate a <commit> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_commit (boolean             confirmed,
		     uint32       timeout,
		     xml_attrs_t * attrs,
		     buf_buffer_t      * buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlChar num[OP_NUM_LEN];
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    op = op_method_name(OP_COMMIT);

    if (!confirmed) {
	/* write the method name as an empty element */
	res = buf_write_elem(buffer, ns_id, op, NULL, NULL);
	if (res != NO_ERR) {
	    return res;
	}
	return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);
    }

    /* ELSE confirmed-commit: write the method name start tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <confirmed> parameter */
    res = buf_write_elem(buffer, ns_id, 
             NCX_EL_CONFIRMED, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* write the <confirm-timeout> parameter */
    xmlStrPrintf(num, OP_NUM_LEN,  (const xmlChar *)"%lu", timeout);
    res = buf_write_elem(buffer, ns_id, 
          NCX_EL_CONFIRM_TIMEOUT, num, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }
    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_commit */


/********************************************************************
* FUNCTION op_wrreq_discard_changes
*
* Generate a <discard-changes> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_discard_changes (xml_attrs_t * attrs,
			      buf_buffer_t      * buffer)
{
    const xmlChar *op;
    status_t res;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name as an empty element */
    op = op_method_name(OP_DISCARD_CHANGES);
    res = buf_write_elem(buffer, ns_id, op, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }
    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_discard_changes */


/********************************************************************
* FUNCTION op_wrreq_validate
*
* Generate a <validate> request PDU
*
* INPUTS:

*    buffer == buffer to fill
* OUTPUTS:
*    buffer contains a completed PDU (if returns NO_ERR)
*    it may contain a partial PDU if return is some error
* RETURNS:
*    NO_ERR if generation is okay
*********************************************************************/
status_t 
    op_wrreq_validate (op_source_t       * source,
		       xml_attrs_t * attrs,
		       buf_buffer_t      * buffer)
{
    status_t  res;
    const xmlChar *op;
    xmlns_id_t ns_id;

    ns_id = xmlns_nc_id();

    if (!source || !buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = rpc_mgr_start_hdr(xmlns_nc_id(), attrs, buffer);
    if (res != NO_ERR) {
	return res;
    }

    /* write the method name start tag */
    op = op_method_name(OP_VALIDATE);
    res = buf_write_tag(buffer, ns_id, op, BUF_START_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* indent for the parameter list */
    buf_inc_indent_level(buffer);

    /* write the <source> parameter */
    res = write_source(buffer, source);
    if (res != NO_ERR) {
	return res;
    }

    /* decrease the indent level */
    buf_dec_indent_level(buffer);

    /* write the method name end tag */
    res = buf_write_tag(buffer, ns_id, op, BUF_END_TAG, NULL);
    if (res != NO_ERR) {
	return res;
    }
    return rpc_mgr_finish_hdr(xmlns_nc_id(), buffer);

} /* op_wrreq_validate */

/* END file op_wrreq.c */
