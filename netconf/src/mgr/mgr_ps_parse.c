/*  FILE: mgr_ps_parse.c

  Parse an XML representation of a parameter set instance,
  and create a ps_parmset_t struct that contains the data found.

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
21oct05      abb      begun
19dec05      abb      restart after refining NCX
10feb06      abb      change raw xmlTextReader interface to use
                      ses_cbt_t instead
*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_mgr_ps_parse
#include "mgr_ps_parse.h"
#endif

#ifndef _H_mgr_ses
#include  "mgr_ses.h"
#endif

#ifndef _H_mgr_top
#include "mgr_top.h"
#endif

#ifndef _H_mgr_val_parse
#include "mgr_val_parse.h"
#endif

#ifndef _H_mgr_xml
#include "mgr_xml.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_ps_parse
#include "ps_parse.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define MGR_PS_PARSE_DEBUG 1 */

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION parse_ps
* 
* Shared access parmset handler
*
* INPUTS:
*   scb == session control block
*   msg == incoming XML msg hdr
*   psd == psd_template_t that should be used to validate the input
*   startnode == xml_node_t for the parent level start node
*         This will be the 'rpc method' element for RPC parmsets
*   ps == initialized ps_parmset_t to fill in
*
* OUTPUTS:
*   *ps == ps_parmset_t is filled in, if NO_ERR
*
* RETURNS:
*   status of the operation
*
* SIDE EFFECTS:
*   The scb->xmlreader will be advanced N nodes.
*
*   Any errors will be appended to the msg->rpc_errsQ
*
*   If NO_ERR, then the end node for the corresponding 
*   psqname start node will be consumed (exit with this 
*   as the current node).
*********************************************************************/
static status_t 
    parse_ps (ses_cb_t  *scb,
	      xml_msg_hdr_t *msg,
	      const psd_template_t *psd,
	      const xml_node_t *startnode,
	      ps_parmset_t  *ps)
{
    status_t        res, retres;
    boolean         done;
    xml_node_t      parmtop;
    psd_parm_t     *psd_parm;
    typ_template_t *psd_type;
    ps_parm_t      *ps_parm;
    xml_node_t      endnode;

    res = NO_ERR;
    retres = NO_ERR;

    /* initialize the parmset -- must set before calling
     * the mgr_ps_parse_error_subtree function
     */
    ps->psd = psd;

    if (!(startnode->nodetyp==XML_NT_EMPTY ||
	  startnode->nodetyp==XML_NT_START)) {
	/* check the parmset startnode type */
	res = ERR_NCX_WRONG_NODETYP;
    } else {
	/* check the startnode namespace */
	res = xml_node_match(startnode, msg->cur_appns, 
			     NULL, XML_NT_NONE);
    }

    /* check any errors in the start node */
    if (res != NO_ERR) {
	mgr_xml_skip_subtree(scb->reader, startnode);
	return res;
    }

    /* check a corner case -- should not usually happen 
     * because PSDs must define at least one parm clause
     * but support a no-op mode anyway
     */
    res = NO_ERR;
    if (!psd->parmcnt) {
	if (startnode->nodetyp==XML_NT_EMPTY) {
	    return NO_ERR;
	}

	/* else get the closing node for this parmset */
	xml_init_node(&endnode);
	res = mgr_xml_consume_node(scb->reader, &endnode);
	if (res == NO_ERR) {
	    res = xml_endnode_match(startnode, &endnode);
	} 
	xml_clean_node(&endnode);
	if (res != NO_ERR) {
	    mgr_xml_skip_subtree(scb->reader, startnode);
	    return res;
	}
    }

    if (res == NO_ERR) {
	/* The parmset startnode type is correct at this point.
	 * First set up the parm flags array.
	 * This array is used to record the edit status of each
	 * parameter. It will be used in the validatation stage 
	 */
	res = ps_setup_parmset(ps, psd, psd->psd_type);
    }

    /* check any errors in the zero parmcnt corner case 
     * or mallocing the parm flags array
     */
    if (res != NO_ERR) {
	mgr_xml_skip_subtree(scb->reader, startnode);
	return res;
    }

    /* check for empty method node, which means use
     * all defaults for mandatory params, and leave 
     * all other params empty
     */
    if (startnode->nodetyp==XML_NT_EMPTY) {
	return NO_ERR;
    }

    /* loop until all the input is accounted for */
    xml_init_node(&parmtop);
    done = FALSE;
    while (!done) {
	ps_parm = NULL;

	/* Get the start of a parameter 
	 *   XML_NT_START for a complex parm or simple parm
	 *   XML_NT_EMPTY for an empty element
	 */
	res = mgr_xml_consume_node(scb->reader, &parmtop);
	if (res == ERR_XML_READER_EOF) {
	    /* this hack is to support the internal load-config
	     * RPC call, which does not actually have an end tag
	     * to match its bogus method name start tag
	     */
	    done = TRUE;
	    continue;
	} else if (res == NO_ERR) {

#ifdef MGR_PS_PARSE_DEBUG
	    log_debug3("\nmgr_ps_parse: got parmtop node");
	    if (LOGDEBUG3) {
		xml_dump_node(&parmtop);
	    }
#endif
	    /* got a new node -- check the node type */
	    switch (parmtop.nodetyp) {
	    case XML_NT_START:
	    case XML_NT_EMPTY:
		/* check the parm node namespace */
		res = xml_node_match(&parmtop, msg->cur_appns, 
			     NULL, XML_NT_NONE);
		break;
	    case XML_NT_END:
		/* see if this is the end node for the parmset parent */
		res = xml_endnode_match(startnode, &parmtop);
		if (res==NO_ERR) {
		    done = TRUE;
		    xml_clean_node(&parmtop);
		    continue;
		} 
		break;
	    case XML_NT_STRING:
		res = ERR_NCX_WRONG_NODETYP;
		break;
	    default:
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    /* figure out which parm is present */
	    if (res == NO_ERR) {
		/* get the parm template in the PSD
		 * check if the parm order is loose or strict 
		 */
		psd_parm = psd_find_parm(psd, parmtop.elname);
		if (!psd_parm) {
		    res = ERR_NCX_UNKNOWN_PARM;
		} else {
		    /* check parameter order */
		    res = NO_ERR;
		    switch (psd->order) {
		    case PSD_OT_LOOSE:
			break;
		    case PSD_OT_STRICT:
			if (psd_parm->parm_id < ps->curparm) {
			    /* parm order went backwards */
			    res = ERR_NCX_WRONG_ORDER;
			} else {
			    ps->curparm = psd_parm->parm_id;
			}
			break;
		    default:
			/* internal error */
			res = SET_ERROR(ERR_INTERNAL_VAL);
		    }
		}
	    }

	    /* parse the child parm */
	    if (res == NO_ERR) {
		/* the parmtop element name and NS are valid 
		 * start a ps_parm_t or ps_parmset_t to add to the parmQ
		 */
		ps_parm = ps_new_parm();
		if (!ps_parm) {
		    res = ERR_INTERNAL_MEM;
		} else {
		    ps_setup_parm(ps_parm, ps, psd_parm);
		    psd_type = (typ_template_t *)psd_parm->pdef;

		    res = mgr_val_parse(scb, msg, psd_type, 
					 &parmtop, ps_parm->val);
		    if (res == NO_ERR) {
			ps_mark_pflag_set(ps, psd_parm->parm_id);
			ps_parm->parent = ps;
			dlq_enque(ps_parm, &ps->parmQ);
		    } else {
			ps_mark_pflag_err(ps, psd_parm->parm_id);
			ps_free_parm(ps_parm);
		    }
		}
	    }
	}  /* end if parmtop node was consumed ok */

	/* catch any errors that occurred for this parameter */
	if (res != NO_ERR) {
	    mgr_xml_skip_subtree(scb->reader, startnode);
	    retres = res;
	}

	xml_clean_node(&parmtop);
	
	/* the entire parmtop element subtree is valid syntax
	 * and the parm or parmset has been saved in the ps->parmQ
	 * go back to the top of the loop
	 */
    }  /* end loop per parameter */

    /* go through each parm node and set its name-specific 
     * sequence number
     * this is done afterwards to support loose paramter order
     */
    ps_set_instseq(ps);

    return retres;

}  /* parse_ps */

/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION mgr_ps_parse_val
* 
* Generate 1 ps_parmset_t tree from an XML subtree 
* which should conform to the specified psd_template_t definition.
*
* This entry point is for the embedded parmsets, invoked
* from mgr_val_parse.c, or any message handler such as mgr_hello,
* that wants to process a non-rpc based parmset
*
*    Same as mgr_ps_parse_rpc_nc except the xml_msg_hdr_t
*    is used instead of the rpc_msg_t, and the ps->psd_type
*    field is assumed not to be RPC
*
* The input is parsed against the specified PSD.
* A ps_parmset_t tree is built as the input is read.
* Each parameter is syntax checked as is is parsed.
* This function will attempt to parse the input as fully as
* possible to generate <rpc-error> elements as much as possible.
* If possible, the parser will skip to next parm or the next parmset,
* in order to support 'continue-on-error' type of operations.
*
* Unless any xmlTextReader functions fail, the entire XML stream
* up to the end node to match 'startnode' will be consumed.
*
* If a parameter is encountered that itself is a parmset, then
* this function will be called recursively.
*
* Note that referential integrity, such as all parms present,
* or parm choice logic is obeyed is done later because this
* depends on the actual RPC method being performed.
*
* INPUTS:
*   scb == session control block
*   msg == xml_hdr_t from incoming msg
*   psd == psd_template_t that should be used to validate the input
*   startnode == xml_node_t for the parent level start node
*         This will be the <hello> element for a hello msg
*   ps == initialized ps_parmset_t to fill in
*
* OUTPUTS:
*   *ps == ps_parmset_t is filled in, if NO_ERR
*
* RETURNS:
*   status of the operation
*
* SIDE EFFECTS:
*   The scb->xmlreader will be advanced N nodes.
*
*   If NO_ERR, then the end node for the corresponding 
*   start node will be consumed (exit with this 
*   as the current node).
*********************************************************************/
status_t 
    mgr_ps_parse_val (ses_cb_t  *scb,
		      xml_msg_hdr_t *msg,
		      const psd_template_t *psd,
		      const xml_node_t *startnode,
		      ps_parmset_t  *ps)
{
    status_t        res;

#ifdef DEBUG
    if (!scb || !msg || !psd || !ps || !startnode) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* set the parmset type; this hack is done to
     * prevent making the caller do this same test
     */
    ps->psd_type = psd->psd_type;
    ps->name = xml_strdup(startnode->elname);
    if (!ps->name) {
	res = ERR_INTERNAL_MEM;
    } else {
	res = parse_ps(scb, msg, psd, startnode, ps);
    }
    return res;

}  /* mgr_ps_parse_val */


/* END file mgr_ps_parse.c */
