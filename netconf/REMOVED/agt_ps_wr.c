/*  FILE: agt_ps_wr.c

   Manage Agent callbacks for parmset manipulation

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
28apr06      abb      begun
28aug06      abb      split from agt_ps.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include <memory.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_acl
#include "agt_acl.h"
#endif

#ifndef _H_agt_ps
#include "agt_ps.h"
#endif

#ifndef _H_agt_ps_wr
#include "agt_ps_wr.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val_wr
#include "agt_val_wr.h"
#endif

#ifndef _H_agt_xml_wr
#include "agt_xml_wr.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xml_msg
#include  "xml_msg.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION agt_ps_write_nc
* 
* Write a NETCONF PDU parmset as data value output
*
* This function is only used for data parmsets, not RPC parmsets
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   ps == parmset to write
*   indent == start indent amount if indent enabled
*   useacl == TRUE if nested access control should be enforced
*   
* RETURNS:
*   none
*********************************************************************/
void
    agt_ps_check_write_nc (ses_cb_t *scb,
			   xml_msg_hdr_t *msg,
			   const ps_parmset_t *ps,
			   int32  indent,
			   boolean useacl,
			   ncx_nodetest_fn_t testfn)
{
    const cfg_app_t  *app;
    const ps_parm_t  *parm;
    const xmlChar    *name;  
    boolean           empty;
    xmlns_id_t        nsid;
    dlq_hdr_t         prefixQ;

#ifdef DEBUG
    if (!scb || !msg || !ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* check access control to this application, parmset */
    if (useacl && !agt_acl_ps_read_allowed(scb->username, ps)) {
	return;   /* skip this entry */
    }

    /* check user callback filter test */
    if (testfn) {
	if (!(*testfn)(NCX_NT_PARMSET, ps)) {
	    return;    /* skip this entry */
	}
    }

    dlq_createSQue(&prefixQ);

    empty = dlq_empty(&ps->parmQ);
    name = (ps->name) ? ps->name : ps->psd->name;

    /* write the <parmset-name> node */
    app = (const cfg_app_t *)ps->parent;
    nsid = app->appdef->nsid;

    agt_begin_elem(scb, msg, nsid, name, NULL, 
		   FALSE, indent, empty, &prefixQ);

    /* check corner-case; empty parmset placeholder */
    if (empty) {
	xml_msg_clean_prefixq(&prefixQ);
	return;
    } 

    /* indent all the parameters */
    if (indent >= 0) {
	indent += NCX_DEF_INDENT;
    }

    /* write each parameter */
    for (parm = (const ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (const ps_parm_t *)dlq_nextEntry(parm)) {
	if (useacl && !agt_acl_parm_read_allowed(scb->username, parm)) {
	    continue;
	}
	agt_begin_elem(scb, msg, nsid, parm->parm->name, 
	       &parm->val->metaQ, FALSE, indent, empty, &prefixQ);

	if (indent >= 0) {
	    indent += NCX_DEF_INDENT;
	}
	agt_val_check_write_nc(scb, msg, parm->val, indent, useacl, testfn);

	if (indent >= 0) {
	    indent -= NCX_DEF_INDENT;
	}

	agt_end_elem(scb, msg, nsid, parm->parm->name, indent);
    }

    /* reset the indent and write the parmset end node */
    if (indent >= 0) {
	indent -= NCX_DEF_INDENT;
    }
    agt_end_elem(scb, msg, nsid, name, indent);

    xml_msg_clean_prefixq(&prefixQ);

}  /* agt_ps_check_write_nc */


/********************************************************************
* FUNCTION agt_ps_write_nc
* 
* Write a NETCONF PDU parmset as data value output
*
* This function is only used for data parmsets, not RPC parmsets
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   ps == parmset to write
*   indent == start indent amount if indent enabled
*   useacl == TRUE if nested access control should be enforced
*   
* RETURNS:
*   none
*********************************************************************/
void
    agt_ps_write_nc (ses_cb_t *scb,
		     xml_msg_hdr_t *msg,
		     const ps_parmset_t *ps,
		     int32  indent,
		     boolean useacl)
{
    agt_ps_check_write_nc(scb, msg, ps, indent, useacl, NULL);

}  /* agt_ps_write_nc */


/* END file agt_ps_wr.c */
