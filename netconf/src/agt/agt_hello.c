/*  FILE: agt_hello.c

    Handle the NETCONF <hello> (top-level) element.

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
15jan07      abb      begun

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

#ifndef _H_agt_hello
#include "agt_hello.h"
#endif

#ifndef _H_agt_ses
#include "agt_ses.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val_parse
#include "agt_val_parse.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_top
#include "top.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_xml_wr
#include "xml_wr.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_HELLO_DEBUG 1
#endif

#define MGR_HELLO_CON ((const xmlChar *)"mgr-hello")

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean agt_hello_init_done = FALSE;


/********************************************************************
* FUNCTION check_manager_hello
*
* Verify that the same NETCONF protocol verion is supported
* by the manager and this agent
* 
* INPUTS:
*    val == value struct for the hello message to check
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_manager_hello (val_value_t *val)
{
    val_value_t  *caps, *cap;

    /* look for the NETCONF base capability string */
    caps = val_find_child(val, NC_MODULE, NCX_EL_CAPABILITIES);
    if (caps && caps->res == NO_ERR) {

	for (cap = val_find_child(caps, NC_MODULE, NCX_EL_CAPABILITY);
	     cap != NULL;
	     cap = val_find_next_child(caps, NC_MODULE, 
				       NCX_EL_CAPABILITY, cap)) {

	    if (cap->res == NO_ERR) {
		if (!xml_strcmp(VAL_STR(cap), CAP_BASE_URN)) {
		    return NO_ERR;
		}
	    }
	}
    }

    return ERR_NCX_MISSING_VAL_INST;

} /* check_manager_hello */


/********************************************************************
* FUNCTION agt_hello_init
*
* Initialize the agt_hello module
* Adds the agt_hello_dispatch function as the handler
* for the NETCONF <hello> top-level element.
*
* INPUTS:
*   none
* RETURNS:
*   NO_ERR if all okay, the minimum spare requests will be malloced
*********************************************************************/
status_t 
    agt_hello_init (void)
{
    status_t  res;

    if (!agt_hello_init_done) {
	res = top_register_node(NC_MODULE, NCX_EL_HELLO, 
				agt_hello_dispatch);
	if (res != NO_ERR) {
	    return res;
	}
	agt_hello_init_done = TRUE;
    }
    return NO_ERR;

} /* agt_hello_init */


/********************************************************************
* FUNCTION agt_hello_cleanup
*
* Cleanup the agt_hello module.
* Unregister the top-level NETCONF <hello> element
*
*********************************************************************/
void 
    agt_hello_cleanup (void)
{
    if (agt_hello_init_done) {
	top_unregister_node(NC_MODULE, NCX_EL_HELLO);
	agt_hello_init_done = FALSE;
    }

} /* agt_hello_cleanup */


/********************************************************************
* FUNCTION agt_hello_dispatch
*
* Handle an incoming <hello> message from the client
*
* INPUTS:
*   scb == session control block
*   top == top element descriptor
*********************************************************************/
void 
    agt_hello_dispatch (ses_cb_t *scb,
			xml_node_t *top)
{
    status_t         res;
    xml_msg_hdr_t    msg;
    val_value_t     *val;
    const obj_template_t  *obj;
    ncx_node_t       dtyp;

#ifdef DEBUG
    if (!scb || !top) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

#ifdef AGT_HELLO_DEBUG
    log_debug("\nagt_hello got node");
    if (LOGDEBUG) {
	xml_dump_node(top);
    }
#endif

    /* only process this message in hello wait state */
    if (scb->state != SES_ST_HELLO_WAIT) {
	/* TBD: stats update */
	log_info("\nagt_hello dropped, wrong state (%d) for session %d",
		 scb->state, scb->sid);
	return;
    }

    /* init local vars */
    res = NO_ERR;
    obj = NULL;
    dtyp = NCX_NT_OBJ;
    xml_msg_init_hdr(&msg);

    /* get a value struct to hold the client hello msg */
    val = val_new_value();
    if (!val) {
	res = ERR_INTERNAL_MEM;
    }

    /* get the type definition from the registry */
    if (res == NO_ERR) {
	obj = (const obj_template_t *)
	    def_reg_find_moddef(NC_MODULE, MGR_HELLO_CON, &dtyp);
	if (!obj) {
	    /* netconf module should have loaded this definition */
	    res = SET_ERROR(ERR_INTERNAL_PTR);
	}
    }

    /* parse a manager hello message */
    if (res == NO_ERR) {
	res = agt_val_parse_nc(scb, &msg, obj, top, NCX_DC_STATE, val);
    }
    
    /* check that the NETCONF base capability is included
     * and it matches the agent protocol version
     */
    if (res == NO_ERR) {
	res = check_manager_hello(val);
    }

    /* report first error and close session */
    if (res != NO_ERR) {
	log_info("\nagt_connect error (%s), dropping session %d",
		 get_error_string(res), scb->sid);
    } else {
	scb->state = SES_ST_IDLE;
	scb->active = TRUE;
	log_info("\nSession %d for %s@%s now active", 
		 scb->sid, scb->username, scb->peeraddr);
    }
    if (val) {
	val_free_value(val);
    }

} /* agt_hello_dispatch */


/********************************************************************
* FUNCTION agt_hello_send
*
* Send the agent <hello> message to the manager on the 
* specified session
*
* INPUTS:
*   scb == session control block
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_hello_send (ses_cb_t *scb)
{
    val_value_t  *mycaps;
    xml_msg_hdr_t msg;
    status_t      res;
    xml_attrs_t   attrs;
    boolean       anyout;
    xmlns_id_t    nc_id, ncx_id;
    xmlChar       numbuff[NCX_MAX_NLEN];

#ifdef DEBUG
    if (!scb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    anyout = FALSE;
    xml_msg_init_hdr(&msg);
    xml_init_attrs(&attrs);
    nc_id = xmlns_nc_id();
    ncx_id = xmlns_ncx_id();

    /* get the agent caps */
    mycaps = agt_cap_get_capsval();
    if (!mycaps) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* setup the prefix map with the NETCONF and NCX namepsaces */
    if (res == NO_ERR) {
	res = xml_msg_build_prefix_map(&msg, &attrs, TRUE, FALSE);
    }

    /* send the <?xml?> directive */
    if (res == NO_ERR) {
	res = ses_start_msg(scb);
    }

    /* start the hello element */
    if (res == NO_ERR) {
	anyout = TRUE;
	xml_wr_begin_elem_ex(scb, &msg, 0, nc_id, NCX_EL_HELLO, 
			     &attrs, TRUE, 0, FALSE);
    }
    
    /* send the capabilities list */
    if (res == NO_ERR) {
	xml_wr_full_val(scb, &msg, mycaps, NCX_DEF_INDENT);
    }

    /* send the session ID */
    if (res == NO_ERR) {
	xml_wr_begin_elem(scb, &msg, nc_id, nc_id,
			  NCX_EL_SESSION_ID, NCX_DEF_INDENT);
    }
    if (res == NO_ERR) {
	sprintf((char *)numbuff, "%d", scb->sid);
	ses_putstr(scb, numbuff);
    }
    if (res == NO_ERR) {
	xml_wr_end_elem(scb, &msg, nc_id, NCX_EL_SESSION_ID, -1);
    }

    /* finish the hello element */
    if (res == NO_ERR) {
	xml_wr_end_elem(scb, &msg, nc_id, NCX_EL_HELLO, 0);
    }

    /* finish the message */
    if (anyout) {
	ses_finish_msg(scb);
    }

    xml_clean_attrs(&attrs);
    xml_msg_clean_hdr(&msg);
    return res;

} /* agt_hello_send */


/* END file agt_hello.c */


