/*  FILE: agt_val_wr.c

   Agent output functions to support <rpc-reply> and <hello> messages

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
20may06      abb      begun
27aug06      abb      split out from agt_val.c

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

#ifndef _H_agt_ps_wr
#include "agt_ps_wr.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val
#include "agt_val.h"
#endif

#ifndef _H_agt_val_wr
#include "agt_val_wr.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION agt_val_check_write_app_nc
* 
* Write a NETCONF PDU data value
* Check if it should be written first by calling a user-supplied
* test function.
*
* Writes the entire contents of the app node,
* including the top node (application name)
*
* INPUTS:
*   scb == session control block
*   msg == header of the msg in progress
*   app == cfg_app_t to write
*   indent == start indent amount if indent enabled
*   useacl == TRUE if nested access control should be enforced
*   testfn == callback function to use, NULL if not used
*   
* RETURNS:
*   none
*********************************************************************/
void
    agt_val_check_write_app_nc (ses_cb_t *scb,
				xml_msg_hdr_t *msg,
				const cfg_app_t *app,
				int32  indent,
				boolean useacl,
				ncx_nodetest_fn_t testfn)
{
    const ps_parmset_t *ps;
    boolean             empty;

#ifdef DEBUG
    if (!scb || !msg || !app) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* check access control to this application */
    if (useacl && !agt_acl_app_allowed(scb->username,
	       app->appdef->owner, app->appdef->appname, OP_READ)) {
	return;    /* skip this entry */
    }

    if (testfn) {
	if (!(*testfn)(NCX_NT_APP, app)) {
	    return;  /* returned FALSE -- skip this entry */
	}
    }
    /* write the <ns:appname> node */
    empty = dlq_empty(&app->parmsetQ);
    xml_wr_begin_app_elem(scb, msg, app, indent, empty);

    /* check corner-case; empty application placeholder */
    if (empty) {
	return;
    } 

    /* indent all the parmsets */
    if (indent >= 0) {
	indent += NCX_DEF_INDENT;
    }

    /* write each parmset */
    for (ps = (const ps_parmset_t *)dlq_firstEntry(&app->parmsetQ);
	 ps != NULL;
	 ps = (const ps_parmset_t *)dlq_nextEntry(ps)) {
	agt_ps_write_nc(scb, msg, ps, indent, useacl);
    }

    /* reset the indent and write the appname end node */
    if (indent >= 0) {
	indent -= NCX_DEF_INDENT;
    }
    xml_wr_end_app_elem(scb, msg, app, indent);

}  /* agt_val_check_write_app_nc */


/********************************************************************
* FUNCTION agt_val_write_app_nc
* 
* Write a NETCONF PDU cfg_app_t node value
*
* Writes the entire contents of the app node,
* including the top node (application name)
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   app == cfg_app_t to write
*   indent == start indent amount if indent enabled
*   useacl == TRUE if nested access control should be enforced
*   
* RETURNS:
*   none
*********************************************************************/
void
    agt_val_write_app_nc (ses_cb_t *scb,
			  xml_msg_hdr_t *msg,
			  const cfg_app_t *app,
			  int32  indent,
			  boolean useacl)
{
    agt_val_check_write_app_nc(scb, msg, app, indent, useacl, NULL);

}  /* agt_val_write_app_nc */


/********************************************************************
* FUNCTION agt_val_check_write_nc
* 
* Write a NETCONF PDU data value in NETCONF/XML encoding
* while checking nodes for suppression of output with
* the supplied test fn
*
* !!! NOTE !!!
* 
* This function generates the contents of the val_value_t
* but not the top node itself.  This function is called
* recusrively and this is the intended behavior.
*
* To generate XML for an entire val_value_t, including
* the top-level node, use the agt_val_full_write_nc fn.
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   useacl == TRUE if nested access control should be enforced
*   testcb == callback function to use, NULL if not used
*   
* RETURNS:
*   none
*********************************************************************/
void
    agt_val_check_write_nc (ses_cb_t *scb,
			    xml_msg_hdr_t *msg,
			    const val_value_t *val,
			    int32  indent,
			    boolean useacl,
			    ncx_nodetest_fn_t testfn)
{
    const val_value_t  *chval;
    const ncx_lstr_t   *liststr;
    const ncx_lmem_t   *listmem;
    const cfg_app_t    *app;
    uint32              len;
    status_t            res;
    boolean             empty;
    ncx_btype_t         btyp, listbtyp;
    xmlChar             buff[NCX_MAX_NUMLEN];

#ifdef DEBUG
    if (!scb || !msg || !val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* check the user filter callback function */
    if (testfn) {
	if (!(*testfn)(NCX_NT_VAL, val)) {
	    return;   /* skip this entry */
	}
    }

    if (val->btyp == NCX_BT_UNION) {
	btyp = val->unbtyp;
    } else {
	btyp = val->btyp;
    }
    switch (btyp) {
    case NCX_BT_ANYAPP:
	for (app = (const cfg_app_t *)dlq_firstEntry(&val->v.appQ);
	     app != NULL;
	     app = (const cfg_app_t *)dlq_nextEntry(app)) {
	    agt_val_check_write_app_nc(scb, msg, app, indent, useacl, testfn);
	} 
	break;
    case NCX_BT_ENAME:
	xml_wr_empty_elem(scb, msg, val->nsid, VAL_USTR(val), -1);
	break;
    case NCX_BT_ENUM:
	res = ncx_sprintf_num(buff, &val->v.num, NCX_BT_INT, &len);
	if (res == NO_ERR) {
	    ses_putstr(scb, val->v.enu.name); 
	    ses_putchar(scb, '(');
	    ses_putstr(scb, buff);
	    ses_putchar(scb, ')');
	} else {
	    SET_ERROR(res);
	}
	break;
    case NCX_BT_FLAG:
	if (val->v.bool) {
	    xml_wr_empty_elem(scb, msg, val->nsid, val->name, -1);
	}
	break;
    case NCX_BT_INT:
    case NCX_BT_LONG:
    case NCX_BT_UINT:
    case NCX_BT_ULONG:
    case NCX_BT_FLOAT:
    case NCX_BT_DOUBLE:
	res = ncx_sprintf_num(buff, &val->v.num, btyp, &len);
	if (res == NO_ERR) {
	    ses_putstr(scb, buff); 
	} else {
	    SET_ERROR(res);
	}
	break;
    case NCX_BT_STRING:
	if (VAL_STR(val)) {
	    if (xml_strlen((const xmlChar *)VAL_STR(val)) 
		>  AGT_MAX_LINESTR) {
		ses_indent(scb, indent);
	    }
	    ses_putcstr(scb, (const xmlChar *)VAL_STR(val));
	}
	break;
    case NCX_BT_USTRING:
	if (VAL_USTR(val)) {
	    if (xml_strlen(VAL_USTR(val)) >  AGT_MAX_LINESTR) {
		ses_indent(scb, indent);
	    }
	    ses_putcstr(scb, VAL_USTR(val));
	}
	break;
    case NCX_BT_LIST:
	listbtyp = val->v.list.btyp;
	for (listmem = (const ncx_lmem_t *)dlq_firstEntry(&val->v.list.memQ);
	     listmem != NULL;
	     listmem = (const ncx_lmem_t *)dlq_nextEntry(listmem)) {
	    ses_indent(scb, indent);
	    ses_putchar(scb, '\"');
	    switch (listbtyp) {
	    case NCX_BT_STRING:
		ses_putcstr(scb, (const xmlChar *)listmem->val.str.s);
		break;
	    case NCX_BT_USTRING:
		/***/
		ses_putcstr(scb, listmem->val.str.os);
		break;
	    default:
		(void)ncx_sprintf_num(buff, &listmem->val.num, 
				      listbtyp, &len);
		ses_putcstr(scb, buff);
	    }
	    ses_putchar(scb, '\"');
	}
	break;
    case NCX_BT_XLIST:
	for (liststr = (const ncx_lstr_t *)dlq_firstEntry(&val->v.xlist.strQ);
	     liststr != NULL;
	     liststr = (const ncx_lstr_t *)dlq_nextEntry(liststr)) {
	    ses_indent(scb, indent);
	    ses_putchar(scb, '\"');
	    ses_putcstr(scb, (const xmlChar *)liststr->str.s);
	    ses_putchar(scb, '\"');
	}
	break;
    case NCX_BT_ANY:
    case NCX_BT_STRUCT:
    case NCX_BT_CHOICE:
    case NCX_BT_TABLE:
    case NCX_BT_CONTAINER:
	for (chval = (const val_value_t *)dlq_firstEntry(&val->v.childQ);
	     chval != NULL;
	     chval = (const val_value_t *)dlq_nextEntry(chval)) {

	    empty = FALSE;

	    /* write the <ns:childname> node 
	     * NCX_BT_FLAG type is a special case
	     */
	    if (chval->btyp == NCX_BT_FLAG) {
		if (!chval->v.bool) {
		    /* skip this false flag */
		    continue;
		} else {
		    empty = TRUE;
		}
	    }
	    xml_wr_begin_elem_ex(scb, msg, chval->nsid, 
				 chval->name, &chval->metaQ, 
				 FALSE, indent, empty, NULL);

	    /* check corner-case; empty application placeholder */
	    if (empty) {
		continue;
	    } 

	    /* indent all the child nodes if any */
	    if (indent >= 0) {
		indent += NCX_DEF_INDENT;
	    }

	    /* write the child node value */
	    agt_val_check_write_nc(scb, msg, chval, indent, useacl, testfn);

	    /* reset the indent and write the value end node */
	    if (indent >= 0) {
		indent -= NCX_DEF_INDENT;
	    }
	    xml_wr_end_elem(scb, msg, chval->nsid, chval->name, 
			    agt_val_oneline_nc(chval) ? -1 : indent);
	} 
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* agt_val_check_write_nc */


/********************************************************************
* FUNCTION agt_val_write_nc
* 
* Write a NETCONF PDU data value in NETCONF/XML encoding
* See agt_val_check_write_nc for full details of this fn.
* It is the same, except a NULL testfn is supplied.
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   useacl == TRUE if nested access control should be enforced
*   
* RETURNS:
*   none
*********************************************************************/
void
    agt_val_write_nc (ses_cb_t *scb,
		      xml_msg_hdr_t *msg,
		      const val_value_t *val,
		      int32  indent,
		      boolean useacl)
{
    agt_val_check_write_nc(scb, msg, val, indent, useacl, NULL);

}  /* agt_val_write_nc */


/********************************************************************
* FUNCTION agt_val_oneline_nc
* 
* Check if the NETCONF (XML) encoding for the specified val_value_t
* should take one line or more than one line
*
* INPUTS:
*   val == value to write
*   
* RETURNS:
*   TRUE if the val is a simple type that should fit on one line
*   FALSE otherwise
*********************************************************************/
boolean
    agt_val_oneline_nc (const val_value_t *val)
{
    const ncx_lstr_t *liststr;
    const ncx_lmem_t *listmem;
    ncx_btype_t       btyp;
    uint32            cnt;


#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return TRUE;
    }
#endif

    if (val->btyp == NCX_BT_UNION) {
	btyp = val->unbtyp;
    } else {
	btyp = val->btyp;
    }

    switch (btyp) {
    case NCX_BT_ANYAPP:
	return FALSE;
    case NCX_BT_ENAME:
    case NCX_BT_ENUM:
    case NCX_BT_FLAG:
    case NCX_BT_INT:
    case NCX_BT_LONG:
    case NCX_BT_UINT:
    case NCX_BT_ULONG:
    case NCX_BT_FLOAT:
    case NCX_BT_DOUBLE:
	return TRUE;
    case NCX_BT_STRING:
	if (VAL_STR(val)) {
	    if (xml_strlen((const xmlChar *)VAL_STR(val)) 
		>  AGT_MAX_LINESTR) {
		return FALSE;
	    }
	}
	return TRUE;
    case NCX_BT_USTRING:
	if (VAL_USTR(val)) {
	    if (xml_strlen(VAL_USTR(val)) > AGT_MAX_LINESTR) {
		return FALSE;
	    }
	}
	return TRUE;
    case NCX_BT_LIST:
	cnt = 0;
	for (listmem = (const ncx_lmem_t *)dlq_firstEntry(&val->v.list.memQ);
	     listmem != NULL;
	     listmem = (const ncx_lmem_t *)dlq_nextEntry(listmem)) {
	    if (++cnt > 1) {
		return FALSE;
	    }
	}
	return TRUE;
    case NCX_BT_XLIST:
	cnt = 0;
	for (liststr = (const ncx_lstr_t *)dlq_firstEntry(&val->v.xlist.strQ);
	     liststr != NULL;
	     liststr = (const ncx_lstr_t *)dlq_nextEntry(liststr)) {
	    if (++cnt > 1) {
		return FALSE;
	    }
	}
	return TRUE;
    case NCX_BT_ANY:
    case NCX_BT_STRUCT:
    case NCX_BT_CHOICE:
    case NCX_BT_TABLE:
    case NCX_BT_CONTAINER:
	return dlq_empty(&val->v.childQ);
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return TRUE;
    }

}  /* agt_val_oneline_nc */


/********************************************************************
* FUNCTION agt_val_full_check_write_nc
* 
* Write an entire val_value_t out as XML, including the top level
* Using an optional testfn to filter output
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   useacl == TRUE if nested access control should be enforced
*   testcb == callback function to use, NULL if not used
*   
* RETURNS:
*   none
*********************************************************************/
void
    agt_val_full_check_write_nc (ses_cb_t *scb,
				 xml_msg_hdr_t *msg,
				 const val_value_t *val,
				 int32  indent,
				 boolean useacl,
				 ncx_nodetest_fn_t testfn)
{
#ifdef DEBUG
    if (!scb || !msg || !val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* write the top-level start node */
    xml_wr_begin_elem_ex(scb, msg, val->nsid, 
			 val->name, &val->metaQ, FALSE, 
			 indent, FALSE, &msg->prefixQ);

    /* write the value node contents */
    agt_val_check_write_nc(scb, msg, val, 
			   indent+NCX_DEF_INDENT, 
			   useacl, testfn);

    /* write the top-level end node */
    xml_wr_end_elem(scb, msg, val->nsid, val->name, 
		    agt_val_oneline_nc(val) ? -1 : indent);

}  /* agt_val_full_check_write_nc */


/********************************************************************
* FUNCTION agt_val_full_write_nc
* 
* Write an entire val_value_t out as XML, including the top level
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   useacl == TRUE if nested access control should be enforced
*   
* RETURNS:
*   none
*********************************************************************/
void
    agt_val_full_write_nc (ses_cb_t *scb,
			   xml_msg_hdr_t *msg,
			   const val_value_t *val,
			   int32  indent,
			   boolean useacl)
{
    agt_val_full_check_write_nc(scb, msg, val, indent, useacl, NULL);
				
} /* agt_val_full_write_nc */

/* END file agt_val_wr.c */
