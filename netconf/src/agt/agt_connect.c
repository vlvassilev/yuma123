/*  FILE: agt_connect.c

    Handle the <ncx-connect> (top-level) element.
    This message is used for thin clients to connect
    to the ncxserver. 

   Client --> SSH2 --> OpenSSH.subsystem(netconf) -->
 
      ncxserver_connect --> AF_LOCAL/ncxserver.sock -->

      ncxserver.listen --> top_dispatch -> ncx_connect_handler

		
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

#ifndef _H_agt_connect
#include "agt_connect.h"
#endif

#ifndef _H_agt_hello
#include "agt_hello.h"
#endif

#ifndef _H_agt_rpcerr
#include "agt_rpcerr.h"
#endif

#ifndef _H_agt_state
#include "agt_state.h"
#endif

#ifndef _H_agt_ses
#include "agt_ses.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
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


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_CONNECT_DEBUG 1
#endif


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean agt_connect_init_done = FALSE;

/********************************************************************
* FUNCTION agt_connect_init
*
* Initialize the agt_connect module
* Adds the agt_connect_dispatch function as the handler
* for the NCX <ncx-connect> top-level element.
*
* INPUTS:
*   none
* RETURNS:
*   NO_ERR if all okay, the minimum spare requests will be malloced
*********************************************************************/
status_t 
    agt_connect_init (void)
{
    status_t  res;

    if (!agt_connect_init_done) {
	res = top_register_node(NCX_MODULE, NCX_EL_NCXCONNECT, 
				agt_connect_dispatch);
	if (res != NO_ERR) {
	    return res;
	}
	agt_connect_init_done = TRUE;
    }
    return NO_ERR;

} /* agt_connect_init */


/********************************************************************
* FUNCTION agt_connect_cleanup
*
* Cleanup the agt_connect module.
* Unregister the top-level NCX <ncx-connect> element
*
*********************************************************************/
void 
    agt_connect_cleanup (void)
{
    if (agt_connect_init_done) {
	top_unregister_node(NCX_MODULE, NCX_EL_NCXCONNECT);
	agt_connect_init_done = FALSE;
    }

} /* agt_connect_cleanup */


/********************************************************************
* FUNCTION agt_connect_dispatch
*
* Handle an incoming <ncx-connect> request
*
* INPUTS:
*   scb == session control block
*   top == top element descriptor
*********************************************************************/
void 
    agt_connect_dispatch (ses_cb_t *scb,
			  xml_node_t *top)
{
    xml_attr_t      *attr;
    status_t         res;
    ncx_num_t        num;

#ifdef DEBUG
    if (!scb || !top) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

#ifdef AGT_CONNECT_DEBUG
    log_debug("\nagt_connect got node");
#endif

    res = NO_ERR;

    /* make sure 'top' is the right kind of node */
    if (top->nodetyp != XML_NT_EMPTY) {
	res = ERR_NCX_WRONG_NODETYP;
	/* TBD: stats update */
    }

    /* only process this message in session init state */
    if (res==NO_ERR && scb->state != SES_ST_INIT) {
	/* TBD: stats update */
	res = ERR_NCX_NO_ACCESS_STATE;
    } else {
	scb->state = SES_ST_IN_MSG;
    }

    /* check the ncxserver version */
    if (res == NO_ERR) {
	attr = xml_find_attr(top, 0, NCX_EL_VERSION);
	if (attr && attr->attr_val) {
	    res = ncx_convert_num(attr->attr_val, NCX_NF_DEC,
				  NCX_BT_UINT32, &num);
	    if (res == NO_ERR) {
		if (num.u != NCX_SERVER_VERSION) {
		    res = ERR_NCX_WRONG_VERSION;
		}
	    }
	} else {
	    res = ERR_NCX_MISSING_ATTR;
	}
    }

    /* check the ncxserver port number */
    if (res == NO_ERR) {
	attr = xml_find_attr(top, 0, NCX_EL_PORT);
	if (attr && attr->attr_val) {
	    res = ncx_convert_num(attr->attr_val, NCX_NF_DEC,
				  NCX_BT_UINT16, &num);
	    if (res == NO_ERR) {
		if (!agt_ses_ssh_port_allowed((uint16)num.u)) {
		    res = ERR_NCX_ACCESS_DENIED;
		}
	    }
	} else {
	    res = ERR_NCX_MISSING_ATTR;
	}
    }

    /* check the magic password string */
    if (res == NO_ERR) {
	attr = xml_find_attr(top, 0, NCX_EL_MAGIC);
	if (attr && attr->attr_val) {
	    if (xml_strcmp(attr->attr_val, 
			   (const xmlChar *)NCX_SERVER_MAGIC)) {
		res = ERR_NCX_ACCESS_DENIED;
	    }
	} else {
	    res = ERR_NCX_MISSING_ATTR;
	}
    }

    /* check the transport */
    if (res == NO_ERR) {
	attr = xml_find_attr(top, 0, NCX_EL_TRANSPORT);
	if (attr && attr->attr_val) {
	    if (xml_strcmp(attr->attr_val, 
			   (const xmlChar *)NCX_SERVER_TRANSPORT)) {
		res = ERR_NCX_ACCESS_DENIED;
	    }
	} else {
	    res = ERR_NCX_MISSING_ATTR;
	}
    }

    /* get the username */
    if (res == NO_ERR) {
	attr = xml_find_attr(top, 0, NCX_EL_USER);
	if (attr && attr->attr_val) {
	    scb->username = xml_strdup(attr->attr_val);
	    if (!scb->username) {
		res = ERR_INTERNAL_MEM;
	    }
	} else {
	    res = ERR_NCX_MISSING_ATTR;
	}
    }

    /* get the client address */
    if (res == NO_ERR) {
	attr = xml_find_attr(top, 0, NCX_EL_ADDRESS);
	if (attr && attr->attr_val) {
	    scb->peeraddr = xml_strdup(attr->attr_val);
	    if (!scb->peeraddr) {
		res = ERR_INTERNAL_MEM;
	    }
	} else {
	    res = ERR_NCX_MISSING_ATTR;
	}
    }

    if (res == NO_ERR) {
	/* add the session to the netconf-state DM */
	res = agt_state_add_session(scb);

	/* bump the session state and send the agent hello message */
	if (res == NO_ERR) {
	    res = agt_hello_send(scb);
	    if (res != NO_ERR) {
		agt_state_remove_session(scb->sid);
	    }
	}

	if (res == NO_ERR) {
	    scb->state = SES_ST_HELLO_WAIT;
	}
    }

    /* report first error and close session */
    if (res != NO_ERR) {
	agt_ses_request_close(scb->sid);
	log_error("\nagt_connect error (%s)\n  dropping session %d (%d)",
		  get_error_string(res), scb->sid, res);
    } else {
	log_debug("\nagt_connect msg ok");
    }

    
} /* agt_connect_dispatch */

/* END file agt_connect.c */


