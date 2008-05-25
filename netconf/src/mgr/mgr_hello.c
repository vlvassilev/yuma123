/*  FILE: mgr_hello.c

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

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
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

#ifndef _H_mgr_ses
#include "mgr_ses.h"
#endif

#ifndef _H_mgr_val_parse
#include "mgr_val_parse.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
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

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_xml_wr
#include  "xml_wr.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define MGR_HELLO_DEBUG 1
#endif

#define MGR_HELLO_TYP ((const xmlChar *)"NcManagerHello")

#define MGR_AGENT_HELLO_TYP ((const xmlChar *)"NcAgentHello")


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean mgr_hello_init_done = FALSE;


/********************************************************************
* FUNCTION process_agent_hello
*
* Process the agent hello contents
*
*  1) Protocol capabilities
*  2) Module capabilities
*  3) Unrecognized capabilities
*
* INPUTS:
*    scb == session control block to set
*    hello == value struct for the hello message to check
*
* OUTPUTS:
*    agent caps in the scb->mgrcb is set
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    process_agent_hello (ses_cb_t *scb,
			 val_value_t *hello)
{

    val_value_t  *caps, *cap, *sidval;
    mgr_scb_t    *mscb;
    boolean       c1, c2;
    status_t      res;

    mscb = (mgr_scb_t *)scb->mgrcb;

    /* make sure the capabilities element is present
     * This should not fail, since already parsed this far 
     */
    caps = val_first_child_name(hello, NCX_EL_CAPABILITIES);
    if (!caps || !typ_has_children(caps->btyp)) {
	return ERR_NCX_MISSING_VAL_INST;
    }	

    /* make sure the session-id element is present
     * This should not fail, since already parsed this far 
     */
    sidval = val_first_child_name(hello, NCX_EL_SESSION_ID);
    if (!sidval || sidval->btyp != NCX_BT_UINT32) {
	return ERR_NCX_MISSING_VAL_INST;
    } else {
	mscb->agtsid = VAL_UINT(sidval);
    }


    /* go through the caps child nodes and construct a caplist */
    for (cap = (val_value_t *)dlq_firstEntry(&caps->v.childQ);
	 cap != NULL;
	 cap = (val_value_t *)dlq_nextEntry(cap)) {
	res = cap_add_std_string(&mscb->caplist, 
				 (const xmlChar *)VAL_STR(cap));
	if (res == ERR_NCX_SKIPPED) {
	    res = cap_add_module_string(&mscb->caplist,
					(const xmlChar *)VAL_STR(cap));
	    if (res == ERR_NCX_SKIPPED) {
		res = cap_add_ent(&mscb->caplist, VAL_STR(cap));
		if (res != NO_ERR) {
		    return res;
		}
	    }
	}
    }

    /* check if the mandatory base protocol capability was set */
    if (!cap_std_set(&mscb->caplist, CAP_STDID_V1)) {
	return ERR_NCX_MISSING_VAL_INST;
    }	

    /* set target type var in the manager session control block */
    c1 = cap_std_set(&mscb->caplist, CAP_STDID_WRITE_RUNNING);
    c2 = cap_std_set(&mscb->caplist, CAP_STDID_CANDIDATE);

    if (c1 && c2) {
	mscb->targtyp = NCX_AGT_TARG_CAND_RUNNING;
    } else if (c1) {
	mscb->targtyp = NCX_AGT_TARG_RUNNING;
    } else if (c2) {
	mscb->targtyp = NCX_AGT_TARG_CANDIDATE;
    } else {
	mscb->targtyp = NCX_AGT_TARG_NONE;
	log_info("\nmgr_hello: no writable target found for"
		 " session %d", scb->sid);
    }

    /* set the startup type in the mscb */
    if (cap_std_set(&mscb->caplist, CAP_STDID_STARTUP)) {
	mscb->starttyp = NCX_AGT_START_DISTINCT;
    } else {
	mscb->starttyp = NCX_AGT_START_MIRROR;
    }

    return NO_ERR;

} /* process_agent_hello */


/********************************************************************
* FUNCTION mgr_hello_init
*
* Initialize the mgr_hello module
* Adds the mgr_hello_dispatch function as the handler
* for the NETCONF <hello> top-level element.
*
* INPUTS:
*   none
* RETURNS:
*   NO_ERR if all okay, the minimum spare requests will be malloced
*********************************************************************/
status_t 
    mgr_hello_init (void)
{
    status_t  res;

    if (!mgr_hello_init_done) {
	res = top_register_node(NC_MODULE, NCX_EL_HELLO, 
				mgr_hello_dispatch);
	if (res != NO_ERR) {
	    return res;
	}
	mgr_hello_init_done = TRUE;
    }
    return NO_ERR;

} /* mgr_hello_init */


/********************************************************************
* FUNCTION mgr_hello_cleanup
*
* Cleanup the mgr_hello module.
* Unregister the top-level NETCONF <hello> element
*
*********************************************************************/
void 
    mgr_hello_cleanup (void)
{
    if (mgr_hello_init_done) {
	top_unregister_node(NC_MODULE, NCX_EL_HELLO);
	mgr_hello_init_done = FALSE;
    }

} /* mgr_hello_cleanup */


/********************************************************************
* FUNCTION mgr_hello_dispatch
*
* Handle an incoming <hello> message from the client
*
* INPUTS:
*   scb == session control block
*   top == top element descriptor
*********************************************************************/
void 
    mgr_hello_dispatch (ses_cb_t *scb,
			xml_node_t *top)
{
    status_t         res;
    xml_msg_hdr_t    msg;
    val_value_t     *val;
    typ_template_t  *typ;
    ncx_node_t       dtyp;

#ifdef DEBUG
    if (!scb || !top) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

#ifdef MGR_HELLO_DEBUG
    log_debug("\nmgr_hello got node");
    if (LOGDEBUG2) {
	xml_dump_node(top);
    }
#endif

    /* only process this message in hello wait state */
    if (scb->state != SES_ST_HELLO_WAIT) {
	/* TBD: stats update */
	log_info("\nmgr_hello dropped, wrong state for session %d",
		 scb->sid);
	return;
    }

    /* init local vars */
    res = NO_ERR;
    val = NULL;
    typ = NULL;
    dtyp = NCX_NT_TYP;
    xml_msg_init_hdr(&msg);

    /* get a value struct to hold the agent hello msg */
    val = val_new_value();
    if (!val) {
	res = ERR_INTERNAL_MEM;
    }

    /* get the type definition from the registry */
    if (res == NO_ERR) {
	typ = (typ_template_t *)
	    def_reg_find_moddef(NC_MODULE, MGR_AGENT_HELLO_TYP, &dtyp);
	if (!typ) {
	    /* netconf module should have loaded this definition */
	    res = SET_ERROR(ERR_INTERNAL_PTR);
	}
    }

    /* parse an agent hello message */
    if (res == NO_ERR) {
	res = mgr_val_parse(scb, &msg, typ, top, val);
    }
    
    /* examine the agent capability list
     * and it matches the agent protocol version
     */
    if (res == NO_ERR) {
	res = process_agent_hello(scb, val);
    }

    /* report first error and close session */
    if (res != NO_ERR) {
	log_info("\nmgr_connect error (%s)\n  dropping session %d (%d)",
		 get_error_string(res), scb->sid, res);
    } else {
	scb->state = SES_ST_IDLE;
	log_debug("\nmgr_hello manager hello ok");
    }
    if (val) {
	val_free_value(val);
    }

} /* mgr_hello_dispatch */


/********************************************************************
* FUNCTION mgr_hello_send
*
* Send the manager <hello> message to the agent on the 
* specified session
*
* INPUTS:
*   scb == session control block
*
* RETURNS:
*   status
*********************************************************************/
status_t
    mgr_hello_send (ses_cb_t *scb)
{
    val_value_t  *mycaps;
    xml_msg_hdr_t msg;
    status_t      res;
    xml_attrs_t   attrs;
    boolean       anyout;
    xmlns_id_t    nc_id;

#ifdef DEBUG
    if (!scb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef MGR_HELLO_DEBUG
    log_debug2("\nmgr sending hello on session %d", scb->sid);
#endif

    res = NO_ERR;
    anyout = FALSE;
    xml_msg_init_hdr(&msg);
    xml_init_attrs(&attrs);
    nc_id = xmlns_nc_id();

    /* get the agent caps */
    mycaps = mgr_cap_get_capsval();
    if (!mycaps) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* setup the prefix map with the NETCONF namespace */
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
			     &attrs, ATTRQ, 0, START);
    }
    
    /* send the capabilities list */
    if (res == NO_ERR) {
	xml_wr_full_val(scb, &msg, mycaps, NCX_DEF_INDENT);
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

} /* mgr_hello_send */


/* END file mgr_hello.c */


