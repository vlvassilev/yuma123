/*  FILE: agt_ps_parse.c

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

#ifndef _H_agt_ps_parse
#include "agt_ps_parse.h"
#endif

#ifndef _H_agt_rpcerr
#include "agt_rpcerr.h"
#endif

#ifndef _H_agt_ses
#include  "agt_ses.h"
#endif

#ifndef _H_agt_top
#include "agt_top.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val_parse
#include "agt_val_parse.h"
#endif

#ifndef _H_agt_xml
#include "agt_xml.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_log
#include "log.h"
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

#ifndef _H_rpc
#include "rpc.h"
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

/* #define AGT_PS_PARSE_DEBUG 1 */

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION check_block_err
* 
* Check for errors in the choice block
* Generate RPC errors as needed if errQ non-NULL
*
* INPUTS:
*   scb == session control block (may be NULL)
*   errQ == errQ to receive any errors
*          (NULL == NO RPC ERRORS RECORDED)
*   setval == TRUE if block supposed to be set
*       check for any missing parameters
*          == FALSE if block supposed to be not set
*       check for any set parameters
*   layer == NCX layer enum to use in rpc-error
*   ps == parmset to check
*   block == block to check

* RETURNS:
*   status
*********************************************************************/
static status_t
    check_block_err (ses_cb_t *scb,
		     dlq_hdr_t *errQ,
		     boolean setval,
		     ncx_layer_t  layer,
		     const ps_parmset_t *ps,
		     const psd_block_t  *block)
{
    const psd_parm_t *parm;
    status_t          res, retres;


    retres = NO_ERR;

    /* check all the parms in this block for missing or extra error */
    for (parm = (const psd_parm_t *)dlq_firstEntry(&block->blockQ);
	 parm != NULL;
	 parm = (const psd_parm_t *)dlq_nextEntry(parm)) {
	res = NO_ERR;
	if (setval) {
	    /* check for missing parm */
	    if (psd_parm_required(parm) && 
		!ps_parmnum_set(ps, parm->parm_id)) {
		/* parm is supposed to be set but is not */
		res = ERR_NCX_MISSING_PARM;
	    }
	} else {
	    /* check for extra parm */
	    if (ps_parmnum_set(ps, parm->parm_id)) {
		/* parm is supposed to be missing but it is set */
		res = ERR_NCX_EXTRA_CHOICE;
	    }
	}
	if (res != NO_ERR) {
	    if (errQ) {
		if (scb) {
		    agt_record_error(scb, errQ, layer, res, NULL, 
				     NCX_NT_PARM, parm, NCX_NT_PARM, parm);
		} else {
		    /****/;
		}
	    }
	    retres = res;
	}
    }

    return retres;
    
}  /* check_block_err */


/********************************************************************
* FUNCTION mark_errors
* 
* Set the proper flags field in the ps_parmset_t 
* Check the error queue for errors and/or warnings
*
* INPUTS:
*   ps == ps_parmset_t struct in progress
*   errQ == Q of rpc_err_rec_t to check
*
*********************************************************************/
static void
    mark_errors (ps_parmset_t *ps,
		 const dlq_hdr_t  *errQ)
{
    const rpc_err_rec_t *err;

    for (err = (const rpc_err_rec_t *)dlq_firstEntry(errQ);
	 err != NULL;
	 err = (const rpc_err_rec_t *)dlq_nextEntry(err)) {
	switch (err->error_severity) {
	case RPC_ERR_SEV_WARNING:
	    SET_PS_WARNING(ps);
	    break;
	case RPC_ERR_SEV_ERROR:
	    SET_PS_ERROR(ps);
	    break;
	default:
	    ;
	}
    }
}  /* mark_errors */


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
    boolean         done, errdone;
    xml_node_t      parmtop;
    psd_parm_t     *psd_parm;
    typ_template_t *psd_type;
    ps_parm_t      *ps_parm;
    xml_node_t      endnode;

    res = NO_ERR;
    retres = NO_ERR;

    /* initialize the parmset -- must set before calling
     * the agt_ps_parse_error_subtree function
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
	(void)agt_ps_parse_error_subtree(scb, msg, 
		     startnode, startnode, res, NCX_NT_NONE, 
		     NULL, NCX_NT_PARMSET, ps);

	/* mark this parmset as containing syntax errors */
	mark_errors(ps, &msg->errQ);
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
	res = agt_xml_consume_node(scb->reader, &endnode,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res == NO_ERR) {
	    res = xml_endnode_match(startnode, &endnode);
	} 
	if (res != NO_ERR) {
	    (void)agt_ps_parse_error_subtree(scb, msg,
	       &endnode, startnode, res, NCX_NT_NONE, NULL, 
					 NCX_NT_PARMSET, ps);
	    mark_errors(ps, &msg->errQ);
	    xml_clean_node(&endnode);
	    return res;
	}
	xml_clean_node(&endnode);
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
	(void)agt_ps_parse_error_subtree(scb, msg,
	     startnode, startnode, res, NCX_NT_NONE, NULL, 
	     NCX_NT_PARMSET, ps);
	mark_errors(ps, &msg->errQ);
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
	errdone = FALSE;
	ps_parm = NULL;

	/* Get the start of a parameter 
	 *   XML_NT_START for a complex parm or simple parm
	 *   XML_NT_EMPTY for an empty element
	 */
	res = agt_xml_consume_node_noeof(scb->reader, &parmtop,
	       NCX_LAYER_OPERATION, &msg->errQ);
	if (res == ERR_XML_READER_EOF) {
	    /* this hack is to support the internal load-config
	     * RPC call, which does not actually have an end tag
	     * to match its bogus method name start tag
	     */
	    done = TRUE;
	    continue;
	} else if (res != NO_ERR) {
	    errdone = TRUE;
	} else {

#ifdef AGT_PS_PARSE_DEBUG
	    log_debug3("\nagt_ps_parse: got parmtop node");
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

		    res = agt_val_parse(scb, msg, psd_type, 
					&parmtop, psd_parm->dataclass,
					ps_parm->val);
		    if (res == NO_ERR) {
			ps_mark_pflag_set(ps, psd_parm->parm_id);
			ps_parm->parent = ps;
			dlq_enque(ps_parm, &ps->parmQ);
		    } else {
			ps_mark_pflag_err(ps, psd_parm->parm_id);
			ps_free_parm(ps_parm);
			errdone = TRUE;
		    }
		}
	    }
	}  /* end if parmtop node was consumed ok */

	/* catch any errors that occurred for this parameter */
	if (res != NO_ERR) {
	    if (!errdone) {
		res = agt_ps_parse_error_subtree(scb, msg, 
		     &parmtop, &parmtop, res, NCX_NT_NONE, NULL, 
					     NCX_NT_PARMSET, ps);
		if (res != NO_ERR) {
		    done = TRUE;
		}
	    }
	    mark_errors(ps, &msg->errQ);
	    errdone = FALSE;
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
* FUNCTION agt_ps_parse_rpc
* 
* Generate 1 ps_parmset_t tree from a NETCONF XML subtree 
* which should conform to the specified psd_template_t definition.
*
* This entry point is for <rpc-reply> oriented procedures
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
*   msg == incoming RPC msg
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
status_t 
    agt_ps_parse_rpc (ses_cb_t  *scb,
		      rpc_msg_t *msg,
		      const psd_template_t *psd,
		      const xml_node_t *startnode,
		      ps_parmset_t  *ps)
{
    status_t        res;

#ifdef DEBUG
    if (!scb || !msg || !psd || !ps || !startnode || !ps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    ps->psd = psd;

    /* set the parmset type; this hack is done to
     * prevent making every caller do this same test
     */
    if (ps == &msg->rpc_input) {
	ps->psd_type = PSD_TYP_RPC;
	ps->name = xml_strdup(msg->rpc_meth_name);
    } else {
	ps->psd_type = psd->psd_type;
	ps->name = xml_strdup(startnode->elname);
    }
    if (!ps->name) {
	res = ERR_INTERNAL_MEM;
    } else {
	res = parse_ps(scb, &msg->mhdr, psd, startnode, ps);
    }
    return res;

}  /* agt_ps_parse_rpc */


/********************************************************************
* FUNCTION agt_ps_parse_val
* 
* Generate 1 ps_parmset_t tree from a NETCONF XML subtree 
* which should conform to the specified psd_template_t definition.
*
* This entry point is for the embedded parmsets, invoked
* from agt_val_parse.c, or any message handler such as agt_hello,
* that wants to process a non-rpc based parmset
*
*    Same as agt_ps_parse_rpc except the xml_msg_hdr_t
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
*   Any errors will be appended to the msg->errQ
*
*   If NO_ERR, then the end node for the corresponding 
*   psqname start node will be consumed (exit with this 
*   as the current node).
*********************************************************************/
status_t 
    agt_ps_parse_val (ses_cb_t  *scb,
		      xml_msg_hdr_t *msg,
		      const psd_template_t *psd,
		      const xml_node_t *startnode,
		      ps_parmset_t  *ps)
{
    status_t        res;

#ifdef DEBUG
    if (!scb || !msg || !psd || !startnode || !ps) {
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

}  /* agt_ps_parse_val */


/********************************************************************
* FUNCTION agt_ps_parse_instance_check
* 
* Check a ps_parmset_t struct against its expected PSD 
* for instance validation:
*
*    - instance qualifiers: 
*      FULL: validate expected instances for any nested parmsets
*      TOP:  validate expected instances for the top level only
*            This mode allows the RPC parmset to be completely
*            validated, but will treat nested parmsets as simple
*            parameters (within the current level)
*      PARTIAL: only check for too many instances. Do not check
*            for missing instances (NCX_IQUAL_ONE or NCX_IQUAL_1MORE)
*      NONE: do not perform any instance qualifier validation
*
* The input is checked against the specified PSD.
*
* Any generated <rpc-error> elements will be added to the errQ
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   ps == ps_parmset_t to check
*   layer == NCX layer calling this function
*
* OUTPUTS:
*    rpc->msg_errQ may have rpc_err_rec_t structs added to it 
*    which must be freed by the called with the 
*    rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
status_t 
    agt_ps_parse_instance_check (ses_cb_t *scb,
			     xml_msg_hdr_t *msg,
			     const ps_parmset_t *ps,
				 ncx_layer_t   layer)
{
    const psd_parm_t     *psd_parm;
    const typ_template_t *psd_type;
    psd_parmid_t          parmid;
    status_t              res, retres;
    ncx_iqual_t           iqual;

#ifdef DEBUG
    if (!ps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    retres = NO_ERR;

    /* Go through all the parameters in the PSD and check
     * the parmset to see if each parm (or choice block)
     * is present the correct number of instances
     * In NCX, only the RelaxNG instance qualifiers
     * are checked per parameter.  Specific number
     * of instances is supported through the table index
     *
     * The PSD node can be a choice or a simple parameter 
     */
    for (parmid = 1; parmid <= ps->psd->parmcnt; parmid++) {
	/* get the parameter descriptor */
	psd_parm = psd_find_parmnum(ps->psd, parmid);
	if (!psd_parm) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}

	/* check for missing parameter only for plain parameters
	 * and all test types except partial
	 */
	if (!psd_parm->choice_id) {
	    if (psd_parm_required(psd_parm)) {
		if (!ps_parmnum_seterr(ps, parmid)) {
		    /* param is not set */
		    retres = ERR_NCX_MISSING_PARM;
		    if (msg) {
			agt_record_error(scb, &msg->errQ, layer, retres, 
			    NULL, NCX_NT_PSDPARM, psd_parm, 
			    NCX_NT_PARMSET, ps);
		    }
		}
	    }
	}

	/* check too many instances for all test types */
	res = NO_ERR;
	psd_type = (const typ_template_t *)psd_parm->pdef;

	iqual = typ_get_iqualval((const typ_template_t *)
				     psd_parm->pdef);
	if ((iqual==NCX_IQUAL_ONE || iqual==NCX_IQUAL_OPT) &&
	    ps_parmnum_mset(ps, parmid)) {
	    /* max 1 allowed, but more than 1 set 
	     * generate an error 
	     */
	    res = ERR_NCX_EXTRA_PARMINST;
	}

	if (res != NO_ERR) {
	    retres = res;
	    if (msg) {
		agt_record_error(scb, &msg->errQ, layer, retres, 
		    NULL, NCX_NT_PSDPARM, psd_parm, NCX_NT_PARMSET, ps);
	    }
	}		
    }

    return retres;

}  /* agt_ps_parse_instance_check */


/********************************************************************
* FUNCTION agt_ps_parse_choice_check
* 
* Check a ps_parmset_t struct against its expected PSD 
* for instance validation:
*
*    - choice validation: 
*      only one member (or block) allowed if the data type is choice
*      Only issue errors based on the instance test qualifiers
*
*    - instance qualifiers: 
*      validate expected choices for the top level only
*
* The input is checked against the specified PSD.
*
* Any generated <rpc-error> elements will be added to the errQ
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   ps == ps_parmset_t to check
*   layer == NCX layer calling this function
*
* OUTPUTS:
*   *errQ may have rpc_err_rec_t structs added to it which must be
*    freed by the called with the rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
status_t 
    agt_ps_parse_choice_check (ses_cb_t  *scb,
			       xml_msg_hdr_t *msg,
			       const ps_parmset_t *ps,
			       ncx_layer_t     layer)
{
    const psd_hdronly_t  *psd_hdr, *choice_hdr;
    const psd_choice_t   *psd_choice;
    const psd_block_t    *psd_block;
    const psd_parm_t     *psd_parm, *set_parm;
    status_t              res, retres;
    boolean               choicereq;

#ifdef DEBUG
    if (!ps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check if there is any work to do at all */
    if (!ps->psd->choicecnt) {
	return NO_ERR;
    }

    retres = NO_ERR;

    /* Go through all the entries in the PSD parmQ and check
     * the choices against the parmset to see if each 
     * choice parm or choice block is present in the correct 
     * number of instances.
     *
     * The parmQ contains PSD_NT_PARM and PSD_NT_CHOICE entries
     */
    for (psd_hdr = (const psd_hdronly_t *)dlq_firstEntry(&ps->psd->parmQ);
	 psd_hdr != NULL;
	 psd_hdr = (const psd_hdronly_t *)dlq_nextEntry(psd_hdr)) {

	if (psd_hdr->ntyp != PSD_NT_CHOICE) {
	    /* skip PSD_NT_PARM entry */
	    continue;
	}

	psd_choice = (const psd_choice_t *)psd_hdr;
	set_parm = NULL;
	choicereq = TRUE;

	/* go through all the entries in this choice and validate them 
	 * The choiceQ contains PSD_NT_BLOCK or PSD_NT_PARM entries
	 */
	for (choice_hdr = (const psd_hdronly_t *)
		 dlq_firstEntry(&psd_choice->choiceQ);
	     choice_hdr != NULL;
	     choice_hdr = (const psd_hdronly_t *)
		 dlq_nextEntry(choice_hdr)) {
	    if (choice_hdr->ntyp==PSD_NT_BLOCK) {
		/* this entire block should be present or absent
		 * unless any of the parameters are optional
		 */
		psd_block = (const psd_block_t *)choice_hdr;

		if (!psd_block_required(psd_block)) {
		    choicereq = FALSE;
		}
		    
		if (ps_check_block_set(ps, psd_block)) {
		    /* at least 1 parm in the block is set */
		    if (set_parm) {
			/* this is an error for all the parms set */
			res = check_block_err(scb, &msg->errQ, 
					      FALSE, layer, 
					      ps, psd_block);
			if (res != NO_ERR) {
			    retres = res;
			}
		    } else {
			/* this becomes the selected choice member */
			set_parm = (const psd_parm_t *)
			    dlq_firstEntry(&psd_block->blockQ);

			/* check for any parms that should be set 
			 * but are not
			 */
			res = check_block_err(scb, &msg->errQ, 
					      TRUE, layer,
					      ps, psd_block);
			if (res != NO_ERR) {
			    retres = res;
			}
		    }
		} else {
		    /* nothing in this block was set */
		    continue;
		}
	    } else {
		/* this choice member is a simple parm */
		psd_parm = (const psd_parm_t *)choice_hdr;

		/* if any choice member is optional, then
		 * the entire choice is optional
		 */
		if (!psd_parm_required(psd_parm)) {
		    choicereq = FALSE;
		}

		/* check too many choices set error */
		if (ps_parmnum_set(ps, psd_parm->parm_id)) {
		    /* this parm is set */
		    if (set_parm) {
			/* choice already set elsewhere */
			retres = ERR_NCX_EXTRA_CHOICE;
			if (msg) {
			    agt_record_error(scb, &msg->errQ, 
					     layer, retres, NULL, 
					     NCX_NT_PSDPARM, psd_parm, 
					     NCX_NT_PARMSET, ps);
			}
		    } else {
			/* make this the selected choice */
			set_parm = psd_parm;
		    }
		} 
	    }
	}

	/* check for 'no choice made' error
	 * Do not generate error if any choice members are optional
	 */
	if (choicereq && !set_parm) {
	    retres = ERR_NCX_MISSING_CHOICE;
	    if (msg) {
		agt_record_error(scb, &msg->errQ, layer, retres, NULL, 
				 NCX_NT_PSDPARM, 
				 psd_first_choice_parm(ps->psd, psd_choice),
				 NCX_NT_PARMSET, ps);
	    }
	}

    }

    return retres;

}  /* agt_ps_parse_choice_check */


/********************************************************************
* FUNCTION agt_ps_parse_error_subtree
* 
* Generate an error during parmset processing for an element
* Add rpc_err_rec_t structs to the msg->errQ
*
* INPUTS:
*   scb == session control block (NULL means call is a no-op)
*   msg = xml_msg_hdr_t from msg in progress  (NULL means call is a no-op)
*   startnode == parent start node to match on exit
*         If this is NULL then the reader will not be advanced
*   errnode == error node being processed
*         If this is NULL then the current node will be
*         used if it can be retrieved with no errors,
*         and the node has naming properties.
*   errcode == error status_t of initial internal error
*         This will be used to pick the error-tag and
*         default error message
*   errnodetyp == internal VAL_PCH node type used for error_parm
*   error_parm == pointer to attribute name or namespace name, etc.
*         used in the specific error in agt_rpcerr.c
*   intnodetyp == internal VAL_PCH node type used for intnode
*   intnode == internal VAL_PCH node used for error_path
* RETURNS:
*   status of the operation; only fatal errors will be returned
*********************************************************************/
status_t 
    agt_ps_parse_error_subtree (ses_cb_t *scb,
			    xml_msg_hdr_t *msg,
			    const xml_node_t *startnode,
			    const xml_node_t *errnode,
			    status_t errcode,
			    ncx_node_t errnodetyp,			    
			    const void *error_parm,
			    ncx_node_t intnodetyp,
			    const void *intnode)
{
    status_t        res;

    res = NO_ERR;

    if (msg) {
	agt_record_error(scb, &msg->errQ, NCX_LAYER_OPERATION, errcode, 
		 errnode, errnodetyp, error_parm, intnodetyp, intnode);
    }

    if (scb && startnode) {
	res = agt_xml_skip_subtree(scb->reader, startnode);
    }

    return res;

}  /* agt_ps_parse_error_subtree */


/********************************************************************
* FUNCTION agt_ps_parse_error_attr
* 
* Generate an error during parmset processing for an attribute
* Add rpc_err_rec_t structs to the msg->rpc_errQ
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t from msg in progress (NULL == NO RPC-ERRORS RECORDED)
*   errattr == attribute with the error
*   errnode == error node being processed
*         If this is NULL then the current node will be
*         used if it can be retrieved with no errors,
*         and the node has naming properties.
*   errcode == error status_t of initial internal error
*         This will be used to pick the error-tag and
*         default error message
*   nodetyp == internal VAL_PCH node type used for error_path
*   intnode == internal VAL_PCH node used for error_path
*
*********************************************************************/
void
    agt_ps_parse_error_attr (ses_cb_t *scb,
			 xml_msg_hdr_t  *msg,
			 const xml_attr_t *errattr,
			 const xml_node_t *errnode,
			 status_t errcode,
			 ncx_node_t nodetyp,
			 const void *intnode)
{
    /* this function would only be called after namespace errors 
     * have been checked, so the 'badns' param is hard-wired to NULL
     */
    if (msg) {
	agt_record_attr_error(scb, &msg->errQ, 
			      NCX_LAYER_OPERATION, errcode,  
			      errattr, errnode, NULL, nodetyp, intnode);
    }

}  /* agt_ps_parse_error_attr */


#ifdef DEBUG
/********************************************************************
* FUNCTION agt_ps_parse_test
* 
* scaffold code to get agt_ps_parse tested 
*
* INPUTS:
*   testfile == NETCONF PDU in a test file
*
*********************************************************************/
void
    agt_ps_parse_test (const char *testfile)
{
    ses_cb_t  *scb;
    status_t   res;

    /* create a dummy session control block */
    scb = agt_ses_new_dummy_session();
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_MEM);
	return;
    }

    /* open the XML reader */
    res = xml_get_reader_from_filespec(testfile, &scb->reader);
    if (res != NO_ERR) {
	SET_ERROR(res);
	agt_ses_free_dummy_session(scb);
	return;
    }

    /* dispatch the test PDU */
    agt_top_dispatch_msg(scb);

    /* clean up and exit */
    agt_ses_free_dummy_session(scb);

}  /* agt_ps_parse_test */
#endif


/* END file agt_ps_parse.c */
