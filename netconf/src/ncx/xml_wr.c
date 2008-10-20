/*  FILE: xml_wr.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
24may06      abb      begun; split out from agt_ncx.c
12feb07      abb      split out non-agent specific write fns back to ncx

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>
#include  <string.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_obj
#include  "obj.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_msg
#include  "xml_msg.h"
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
#define XML_WR_DEBUG  1
#endif


#define XML_WR_MAX_LINESTR   34

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION fit_on_line
*
* Check if the specified value will fit on the current line
* or if a newline is needed first
*
* INPUTS:
*   scb == session control block
*   val == value to check
*
* RETURNS:
*   TRUE if value will fit on current line, FALSE if not
*********************************************************************/
static boolean
    fit_on_line (ses_cb_t *scb,
		 val_value_t *val)
{
    uint32     len;
    status_t   res;

    if (!val_fit_oneline(val)) {
	return FALSE;
    }

    if (!typ_is_simple(val->btyp)) {
	return TRUE;
    }

    if (ses_get_mode(scb) != SES_MODE_XMLDOC) {
	return TRUE;
    }

    res = val_sprintf_simval_nc(NULL, val, &len);
    if (res != NO_ERR) {
	return TRUE;
    }

    return (len <= ses_line_left(scb)) ? TRUE : FALSE;

}  /* fit_on_line */


/********************************************************************
* FUNCTION write_extern
*
* Write an external file to the session
*
* INPUTS:
*   scb == session control block
*   val == value to write (NCX_BT_EXTERN)
*
* RETURNS:
*   none
*********************************************************************/
static void
    write_extern (ses_cb_t *scb,
		  val_value_t *val)
{
    FILE               *fil;
    boolean             done;
    int                 ch;

    if (val->v.fname) {
	fil = fopen((const char *)val->v.fname, "r");
	if (fil) {
	    done = FALSE;
	    while (!done) {
		ch = fgetc(fil);
		if (ch == EOF) {
		    fclose(fil);
		    done = TRUE;
		} else {
		    ses_putchar(scb, (uint32)ch);
		}
	    }
	}
    }
    
}  /* write_extern */


/********************************************************************
* FUNCTION write_intern
*
* Write an internal buffer to the session
*
* INPUTS:
*   scb == session control block
*   val == value to write (NCX_BT_INTERN)
*
* RETURNS:
*   none
*********************************************************************/
static void
    write_intern (ses_cb_t *scb,
		  val_value_t *val)
{
    if (val->v.intbuff) {
	ses_putstr(scb, val->v.intbuff);
    }
    
}  /* write_intern */


/********************************************************************
* FUNCTION write_xmlns_decl
*
* Write an xmlns declaration
*
* INPUTS:
*   scb == session control block
*   pfix == prefix to use
*   nsid == namespace ID to use for xmlns value
*   indent == actual indent amount; xml_wr_indent will not be checked
*
* RETURNS:
*   none
*********************************************************************/
static void
    write_xmlns_decl (ses_cb_t *scb,
		      const xmlChar *pfix,
		      xmlns_id_t  nsid,
		      int32 indent)
{
    const xmlChar  *val;

    val = xmlns_get_ns_name(nsid);
    if (!val) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    if (indent <= 0) {
	ses_putchar(scb, ' ');
    } else {
	ses_indent(scb, indent);
    }

    /* write the xmlns attribute name */
    ses_putstr(scb, XMLNS);

    /* generate a prefix if this attribute has a namespace ID */
    if (pfix) {
	ses_putchar(scb, ':');
	ses_putstr(scb, pfix);
    }
    ses_putchar(scb, '=');
    ses_putchar(scb, '\"');
    ses_putstr(scb, val);      /* write the namespace URI value */
    ses_putchar(scb, '\"');
    
}  /* write_xmlns_decl */


/********************************************************************
* FUNCTION write_attrs
*
* Write all the required attributes for this element
*
* INPUTS:
*   scb == session control block
*   msg == header for the rpc_msg_t in progress
*   attrQ == Q of xml_attr_t or val_value_t to write
*   isattrq == TRUE for Q of xml_attr_t
*           == FALSE for Q of val_value_t
*   indent == actual indent amount; xml_wr_indent will not be checked
*   elem_nsid == namespace ID of the parent element
*
*********************************************************************/
static void
    write_attrs (ses_cb_t *scb,
		 xml_msg_hdr_t *msg,
		 const dlq_hdr_t *attrQ,
		 boolean isattrq,
		 int32 indent,
		 xmlns_id_t elem_nsid)
{
    const xml_attr_t  *attr;
    val_value_t *val;
    dlq_hdr_t    *hdr;
    const xmlChar     *pfix, *attr_name, *attr_qname;
    boolean            xneeded;
    uint32             len;
    xmlns_id_t         ns_id, attr_nsid;

    ns_id = xmlns_ns_id();

    for (hdr = dlq_firstEntry(attrQ); 
	 hdr != NULL;
	 hdr = dlq_nextEntry(hdr)) {

	/* set up the data fields; len is not precise, ignores prefix */
	if (isattrq) {
	    attr = (const xml_attr_t *)hdr;
	    attr_nsid = attr->attr_ns;
	    attr_name = attr->attr_name;
	    attr_qname = attr->attr_qname;
	    len = xml_strlen(attr->attr_val) + xml_strlen(attr->attr_name);
	} else {
	    val = (val_value_t *)hdr;
	    attr_nsid = val->nsid;
	    attr_name = val->name;
	    attr_qname = NULL;
	    len = xml_strlen(val->v.str) + xml_strlen(val->name);
	}

	/* deal with initial indent */
	if (indent < 0) {
	    ses_putchar(scb, ' ');
	} else if (len + 4 + SES_LINELEN(scb) >= SES_LINESIZE(scb)) {
	    ses_indent(scb, indent);
	} else {
	    ses_putchar(scb, ' ');
	}

	/* generate one attribute name value pair
	 *
	 * generate a prefix if this attribute has a namespace ID 
	 * make sure to skip the XMLNS namespace; this is 
	 * handled different than all other attributes 
	 *
	 */
	/* check if this is an XMLNS directive */
	if (XMLNS_EQ(attr_nsid, ns_id)) {
	    /* xmlns:prefix format */
	    if (attr_name != attr_qname) {
		/* this is a namespace decl with a prefix */
		ses_putstr(scb, XMLNS);
		ses_putchar(scb, ':');	    
	    }
	} else if (attr_nsid) {
	    /* prefix:attribute-name format */
	    pfix = xml_msg_get_prefix(msg, elem_nsid, 
				      attr_nsid, &xneeded);
	    if (xneeded) {
		write_xmlns_decl(scb, pfix, attr_nsid, indent);
	    }

	    /* deal with indent again */
	    if (indent < 0) {
		ses_putchar(scb, ' ');
	    } else if (len + 4 + SES_LINELEN(scb) >= SES_LINESIZE(scb)) {
		ses_indent(scb, indent);
	    } else {
		ses_putchar(scb, ' ');
	    }

	    if (pfix) {
		ses_putstr(scb, pfix);
		ses_putchar(scb, ':');
	    }
	}

	ses_putstr(scb, attr_name);
	ses_putchar(scb, '=');
	ses_putchar(scb, '\"');
	if (isattrq) {
	    ses_putstr(scb, attr->attr_val);
	} else {
	    /* write the simple value meta var */
	    xml_wr_val(scb, msg, val, -1);
	}	    
	ses_putchar(scb, '\"');
    }

}  /* write_attrs */


/************  E X T E R N A L    F U N C T I O N S    **************/


/********************************************************************
* FUNCTION xml_wr_write
*
* Write some xmlChars to the specified session
*
* INPUTS:
*   scb == session control block to start msg 
*   buff == buffer to write
*   bufflen == number of bytes to write, not including any
*              EOS char at the end of the buffer
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_buff (ses_cb_t *scb,
		 const xmlChar *buff,
		 uint32 bufflen)
{

    uint32  i;

#ifdef DEBUG
    if (!scb || !buff) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    for (i=0; i<bufflen; i++) {
	ses_putchar(scb, *buff++);
    }

}  /* xml_wr_buff */


/********************************************************************
* FUNCTION xml_wr_begin_elem_ex
*
* Write a start or empty XML tag to the specified session
*
* INPUTS:
*   scb == session control block
*   msg == top header from message in progress
*   parent_nsid == namespace ID of the parent element, if known
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   attrQ == Q of xml_attr_t or val_value_t records to write in
*            the element; NULL == none
*   isattrq == TRUE if the qQ contains xml_attr_t nodes
*              FALSE if the Q contains val_value_t nodes (metadata)
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*   empty == TRUE for empty node
*         == FALSE for start node
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_begin_elem_ex (ses_cb_t *scb,
			  xml_msg_hdr_t *msg,
			  xmlns_id_t  parent_nsid,
			  xmlns_id_t  nsid,
			  const xmlChar *elname,
			  const dlq_hdr_t *attrQ,
			  boolean isattrq,
			  int32 indent,
			  boolean empty)
{
    const xmlChar       *pfix;
    boolean              xneeded;

#ifdef DEBUG
    if (!scb || !msg || !elname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ses_indent(scb, indent);

    /* start the element and write the prefix, if any */
    ses_putchar(scb, '<');
    pfix = xml_msg_get_prefix(msg, parent_nsid, nsid, &xneeded);
    if (pfix) {
	ses_putstr(scb, pfix);
	ses_putchar(scb, ':');
    }

    /* write the element name */
    ses_putstr(scb, elname);

    if (xneeded || (attrQ && !dlq_empty(attrQ))) {
	if (indent >= 0) {
	    indent += ses_indent_count(scb);
	}
	if (attrQ) {
	    write_attrs(scb, msg, attrQ, isattrq, indent, nsid);
	}
	if (xneeded) {
	    if (!attrQ || dlq_empty(attrQ)) {
		indent = -1;
	    }
	    write_xmlns_decl(scb, pfix, nsid, indent);
	}
    }

    /* finish up the element */
    if (empty) {
	ses_putchar(scb, '/');
    }
    ses_putchar(scb, '>');

    /* hack in XMLDOC mode to get more readable XSD output */
    if (empty && scb->mode==SES_MODE_XMLDOC && indent < 
	(3*ses_indent_count(scb))) {
	ses_putchar(scb, '\n');
    }

}  /* xml_wr_begin_elem_ex */


/********************************************************************
* FUNCTION xml_wr_begin_elem
*
* Write a start XML tag to the specified session without attributes
*
* INPUTS:
*   scb == session control block
*   msg == top header from message in progress
*   parent_nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_begin_elem (ses_cb_t *scb,
		       xml_msg_hdr_t *msg,
		       xmlns_id_t  parent_nsid,
		       xmlns_id_t  nsid,
		       const xmlChar *elname,
		       int32 indent)
{
    xml_wr_begin_elem_ex(scb, msg, parent_nsid, nsid, elname,
			 NULL, FALSE, indent, FALSE);

} /* xml_wr_begin_elem */


/********************************************************************
* FUNCTION xml_wr_empty_elem
*
* Write an empty XML tag to the specified session without attributes
*
* INPUTS:
*   scb == session control block
*   msg == top header from message in progress
*   parent_nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_empty_elem (ses_cb_t *scb,
		       xml_msg_hdr_t *msg,
		       xmlns_id_t  parent_nsid,
		       xmlns_id_t  nsid,
		       const xmlChar *elname,
		       int32 indent)
{
    xml_wr_begin_elem_ex(scb, msg, parent_nsid, nsid, elname,
			 NULL, FALSE, indent, TRUE);

} /* xml_wr_empty_elem */


/********************************************************************
* FUNCTION xml_wr_end_elem
*
* Write an end tag to the specified session
*
* INPUTS:
*   scb == session control block to start msg 
*   msg == header from message in progress
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   indent == number of chars to indent after a newline
*             will be ignored if indent is turned off
*             in the agent profile
*           == -1 means no newline or indent
*           == 0 means just newline
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_end_elem (ses_cb_t *scb,
		  xml_msg_hdr_t *msg,
		  xmlns_id_t  nsid,
		  const xmlChar *elname,
		  int32 indent)
{
    const xmlChar       *pfix;
    boolean              xneeded;

#ifdef DEBUG
    if (!scb || !msg || !elname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ses_indent(scb, indent);

    /* start the element and write the prefix, if any */
    ses_putchar(scb, '<');
    ses_putchar(scb, '/');
    pfix = xml_msg_get_prefix(msg, 0, nsid, &xneeded);
    if (pfix) {
	ses_putstr(scb, pfix);
	ses_putchar(scb, ':');
    }

    /* write the element name */
    ses_putstr(scb, elname);
	
    /* finish up the element */
    ses_putchar(scb, '>');

    /* hack in XMLDOC mode to get more readable XSD output */
    if (scb->mode==SES_MODE_XMLDOC && 
	indent==ses_indent_count(scb)) {
	ses_putchar(scb, '\n');
    }

}  /* xml_wr_end_elem */


#if 0
/********************************************************************
* FUNCTION xml_wr_begin_app_elem
*
* Write a start or empty XML tag to the specified session
*
* INPUTS:
*   scb == session control block
*   msg == message in progress
*   app == cfg_app_t to print
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*   empty == TRUE for empty node
*         == FALSE for start node
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_begin_app_elem (ses_cb_t *scb,
			   xml_msg_hdr_t *msg,
			   const cfg_app_t *app,
			   int32 indent,
			   boolean empty)
{
    xmlns_id_t        nsid, lastdef;
    status_t          res;


#ifdef DEBUG
    if (!scb || !msg || !app) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    nsid = app->appdef->nsid;

    lastdef = msg->last_defns;

    msg->last_defns = msg->defns;
    msg->defns = nsid;

    res = xml_msg_gen_new_prefix(msg, msg->last_defns, 
				 &msg->last_defpfix,
				 XML_MSG_PREFIX_SIZE+1);
    if (res != NO_ERR) {
	msg->defns = msg->last_defns;
	msg->last_defns = lastdef;
	SET_ERROR(res);
	return;
    }

    ses_indent(scb, indent);

    /* start the element and write the prefix, if any */
    ses_putchar(scb, '<');

    /* write the application name */
    ses_putstr(scb, app->appdef->appname);

    write_xmlns_decl(scb, NULL, nsid, -1);

    if (indent >=0) {
	indent += ses_indent_count(scb);
    }

    if (msg->last_defns) {
	write_xmlns_decl(scb, msg->last_defpfix, 
			 msg->last_defns, indent);    
    }

    /* finish up the element */
    if (empty) {
	ses_putchar(scb, '/');
    }
    ses_putchar(scb, '>');

}  /* xml_wr_begin_app_elem */


/********************************************************************
* FUNCTION xml_wr_end_app_elem
*
* Write an end tag to the specified session
*
* INPUTS:
*   scb == session control block to start msg 
*   msg == header from message in progress
*   app == cfg_app_t to end
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_end_app_elem (ses_cb_t *scb,
		      xml_msg_hdr_t *msg,
		      const cfg_app_t  *app,
		      int32 indent)
{

#ifdef DEBUG
    if (!scb || !msg || !app) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ses_indent(scb, indent);

    /* start the element and write the prefix, if any */
    ses_putchar(scb, '<');
    ses_putchar(scb, '/');

    /* write the element name */
    ses_putstr(scb, app->appdef->appname);
	
    /* finish up the element */
    ses_putchar(scb, '>');

    msg->defns = msg->last_defns;
    msg->last_defns = 0;
    msg->last_defpfix[0] = 0;

}  /* xml_wr_end_app_elem */
#endif  /*** 0 ***/


/********************************************************************
* FUNCTION xml_wr_value_elem
*
* Write a start tag, simple value content, and an end tag
* to the specified session.  A flag element and
* ename will vary from this format.
*
* INPUTS:
*   scb == session control block
*   msg == header from message in progress
*   val == simple value to write as element content
*   nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   attrQ == Q of xml_attr_t or val_value_t records to write in
*            the element; NULL == none
*   isattrq == TRUE for Q of xml_attr_t, FALSE for val_value_t
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_value_elem (ses_cb_t *scb,
		       xml_msg_hdr_t *msg,
		       val_value_t *val,
		       xmlns_id_t  parent_nsid,
		       xmlns_id_t  nsid,
		       const xmlChar *elname,
		       const dlq_hdr_t *attrQ,
		       boolean isattrq,
		       int32 indent)
{
    boolean   newln;
    int32     valdent;

#ifdef DEBUG
    if (!scb || !msg || !val || !elname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (val->parent)
	xml_wr_begin_elem_ex(scb, msg, parent_nsid, 
			     nsid, elname, attrQ, isattrq, 
			     indent, FALSE);

    newln = !fit_on_line(scb, val);
    if (newln) {
	valdent =  (indent < 0) ? indent : indent + ses_indent_count(scb);
    } else {
	valdent = -1;
    }
    xml_wr_val(scb, msg, val, valdent);
    xml_wr_end_elem(scb, msg, nsid, elname, (newln) ? indent : -1);

}  /* xml_wr_value_elem */


/********************************************************************
* FUNCTION xml_wr_string_elem
*
* Write a start tag, simple string content, and an end tag
* to the specified session.  A flag element and
* ename will vary from this format.
*
* Simple content nodes are completed on a single line to
* prevent introduction of extra whitespace
*
* INPUTS:
*   scb == session control block
*   msg == header from message in progress
*   str == simple string to write as element content
*   parent_nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   attrQ == Q of xml_attr_t records to write in
*            the element; NULL == none
*   isattrq == TRUE for Q of xml_attr_t, FALSE for val_value_t
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*  
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_string_elem (ses_cb_t *scb,
			xml_msg_hdr_t *msg,
			const xmlChar *str,
			xmlns_id_t  parent_nsid,
			xmlns_id_t  nsid,
			const xmlChar *elname,
			const dlq_hdr_t *attrQ,
			boolean isattrq,
			int32 indent)
{

#ifdef DEBUG
    if (!scb || !msg || !str || !elname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    xml_wr_begin_elem_ex(scb, msg, parent_nsid, 
			 nsid, elname, attrQ, isattrq, 
			 indent, FALSE);
    ses_putstr(scb, str);
    xml_wr_end_elem(scb, msg, nsid, elname, -1);

}  /* xml_wr_string_elem */


/********************************************************************
* FUNCTION xml_wr_qname_elem
*
* Write a start tag, QName string content, and an end tag
* to the specified session.  A flag element and
* ename will vary from this format.
*
* The ses_start_msg must be called before this
* function, in order for it to allow any writes
*
* INPUTS:
*   scb == session control block
*   msg == header from message in progres
*   val_nsid == namespace ID of the QName prefix
*   str == simple string to write as element content
*   parent_nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   attrQ == Q of xml_attr_t records to write in
*            the element; NULL == none
*   isattrq == TRUE for Q of xml_attr_t, FALSE for val_value_t
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_qname_elem (ses_cb_t *scb,
		    xml_msg_hdr_t *msg,
		    xmlns_id_t val_nsid,
		    const xmlChar *str,
		    xmlns_id_t  parent_nsid,
		    xmlns_id_t  nsid,
		    const xmlChar *elname,
		    const dlq_hdr_t *attrQ,
		    boolean isattrq,
		    int32 indent)
{
    boolean         xneeded;
    const xmlChar  *pfix;

#ifdef DEBUG
    if (!scb || !msg || !str || !elname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    xml_wr_begin_elem_ex(scb, msg, parent_nsid, 
			 nsid, elname, attrQ, isattrq, 
			 indent, FALSE);

    /* counting on xmlns decl to be in <rpc-error> parent node */
    pfix = xml_msg_get_prefix(msg, parent_nsid, val_nsid, &xneeded);
    if (pfix) {
	ses_putstr(scb, pfix);
	ses_putchar(scb, XMLNS_SEPCH);
    }
    ses_putstr(scb, str);

    xml_wr_end_elem(scb, msg, nsid, elname, -1);

}  /* xml_wr_qname_elem */


/********************************************************************
* FUNCTION xml_wr_check_val
* 
* Write an NCX value in XML encoding
* while checking nodes for suppression of output with
* the supplied test fn
*
* !!! NOTE !!!
* 
* This function generates the contents of the val_value_t
* but not the top node itself.  This function is called
* recursively and this is the intended behavior.
*
* To generate XML for an entire val_value_t, including
* the top-level node, use the xml_wr_full_val fn.
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   testcb == callback function to use, NULL if not used
*   
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_check_val (ses_cb_t *scb,
		      xml_msg_hdr_t *msg,
		      val_value_t *val,
		      int32  indent,
		      ncx_nodetest_fn_t testfn)
{
    const ncx_lmem_t   *listmem;
    val_value_t        *v_val, *v_chval, *chval, *useval, *usechval;
    uint32              len;
    status_t            res;
    boolean             empty, first, wspace;
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
	if (!(*testfn)(msg->withdef, NCX_NT_VAL, val)) {
	    return;   /* skip this entry */
	}
    }

    /* check if this is an external file to send */
    if (val->btyp == NCX_BT_EXTERN) {
	write_extern(scb, val);
	return;
    }

    /* check if this is an internal buffer to send */
    if (val->btyp == NCX_BT_INTERN) {
	write_intern(scb, val);
	return;
    }

    v_val = NULL;

    if (val_is_virtual(val)) {
	v_val = val_get_virtual_value(scb, val, &res);
	if (!v_val) {
	    SET_ERROR(res);
	    return;
	}
    }

    useval = (v_val) ? v_val : val;

    if (useval->btyp == NCX_BT_UNION) {
	btyp = useval->unbtyp;
    } else {
	btyp = useval->btyp;
    }

    switch (btyp) {
    case NCX_BT_ENUM:
	if (useval->v.enu.name) {
	    ses_putstr(scb, useval->v.enu.name);
	}
	break;
    case NCX_BT_EMPTY:
	if (useval->v.bool) {
	    xml_wr_empty_elem(scb, msg, val_get_parent_nsid(useval),
			      useval->nsid, useval->name, -1);
	}
	break;
    case NCX_BT_BOOLEAN:
	if (useval->v.bool) {
	    ses_putcstr(scb, NCX_EL_TRUE, indent);
	} else {
	    ses_putcstr(scb, NCX_EL_FALSE, indent);
	}
	break;
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
    case NCX_BT_INT64:
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
    case NCX_BT_UINT64:
    case NCX_BT_FLOAT32:
    case NCX_BT_FLOAT64:
	res = ncx_sprintf_num(buff, &useval->v.num, btyp, &len);
	if (res == NO_ERR) {
	    ses_putstr(scb, buff); 
	} else {
	    SET_ERROR(res);
	}
	break;
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /******/
	if (VAL_STR(useval)) {
	    if (!fit_on_line(scb, useval) && (indent>0)) {
		ses_indent(scb, indent);
	    }
	    ses_putcstr(scb, VAL_STR(useval), indent);
	}
	break;
    case NCX_BT_BINARY:
	if (VAL_USTR(useval)) {
	    ses_putcstr(scb, VAL_USTR(useval), indent);
	}
	break;
    case NCX_BT_BITS:
    case NCX_BT_SLIST:
	listbtyp = useval->v.list.btyp;
	first = TRUE;
	for (listmem = (const ncx_lmem_t *)
		 dlq_firstEntry(&useval->v.list.memQ);
	     listmem != NULL;
	     listmem = (const ncx_lmem_t *)dlq_nextEntry(listmem)) {

	    wspace = FALSE;

	    /* handle indent+double quote or space for non-strings */
	    if (listbtyp==NCX_BT_STRING || listbtyp==NCX_BT_BINARY) {

		/* get len and whitespace flag */
		len = xml_strlen_sp(listmem->val.str, &wspace);

		/* check special case -- empty string
		 * handle it here instead of going through the loop
		 */
		if (!len) {
		    if (!first) {
			ses_putstr(scb, (const xmlChar *)" \"\"");
		    } else {
			ses_putstr(scb, (const xmlChar *)"\"\"");
			first = FALSE;
		    }
		    continue;
		}
	    }

	    /* handle newline+indent or space between list elements */
	    if (first) {
		first = FALSE;
	    } else if (SES_LINELEN(scb) > SES_LINESIZE(scb)) {
		ses_indent(scb, indent);
	    } else {
		ses_putchar(scb, ' ');
	    }		

	    /* check if double quotes needed */
	    if (wspace) {
		ses_putchar(scb, '\"');
	    }

	    /* print the list member content as a string */
	    switch (listbtyp) {
	    case NCX_BT_STRING:
	    case NCX_BT_BINARY:
	    case NCX_BT_INSTANCE_ID:
		ses_putcstr(scb, listmem->val.str, indent);
		break;
	    case NCX_BT_ENUM:
		ses_putstr(scb, listmem->val.enu.name); 
		break;
	    default:
		(void)ncx_sprintf_num(buff, &listmem->val.num, 
				      listbtyp, &len);
		ses_putcstr(scb, buff, indent);
	    }

	    /* check finish quoted string */
	    if (wspace) {
		ses_putchar(scb, '\"');
	    }
	}
	break;
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
    case NCX_BT_LIST:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
	for (chval = val_get_first_child(useval);
	     chval != NULL;
	     chval = val_get_next_child(chval)) {

	    /* check special manager-only external mode;
	     * send the contents of a file instead of the val node
	     */
	    if (chval->btyp == NCX_BT_EXTERN) {
		write_extern(scb, chval);
		continue;
	    }
	    if (chval->btyp == NCX_BT_INTERN) {
		write_intern(scb, chval);
		continue;
	    }

	    v_chval = NULL;

	    if (val_is_virtual(chval)) {
		v_chval = val_get_virtual_value(scb, chval, &res);
		if (!v_chval) {
		    SET_ERROR(res);
		    continue;
		}
	    }

	    usechval = (v_chval) ? v_chval : chval;

	    /* check empty node or some element content */
	    empty = !val_has_content(usechval);


	    /* write the <ns:childname> node 
	     * NCX_BT_EMPTY type is a special case
	     * always an empty element, and only visible
	     * if set to TRUE; otherwise skip the value node
	     */
	    if (usechval->btyp == NCX_BT_EMPTY) {
		if (!usechval->v.bool) {
		    /* skip this false flag */
		    continue;
		} else {
		    empty = TRUE;
		}
	    }
	    xml_wr_begin_elem_ex(scb, msg, 
				 useval->nsid,
				 usechval->nsid, 
				 usechval->name, &usechval->metaQ, 
				 FALSE, indent, empty);

	    /* check corner-case; empty application placeholder */
	    if (!empty) {
		/* indent all the child nodes if any */
		if (indent >= 0) {
		    indent += ses_indent_count(scb);
		}

		/* write the child node value */
		xml_wr_check_val(scb, msg, usechval, indent, testfn);

		/* reset the indent and write the value end node */
		if (indent >= 0) {
		    indent -= ses_indent_count(scb);
		}
		xml_wr_end_elem(scb, msg, usechval->nsid, usechval->name, 
				fit_on_line(scb, usechval) ? -1 : indent);
	    }

	    if (v_chval) {
		val_free_value(v_chval);
	    }
	} 
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (v_val) {
	val_free_value(v_val);
    }

}  /* xml_wr_check_val */


/********************************************************************
* FUNCTION xml_wr_val
* 
* Write an NCX value node in XML encoding
* See xml_wr_check_write for full details of this fn.
* It is the same, except a NULL testfn is supplied.
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_val (ses_cb_t *scb,
		xml_msg_hdr_t *msg,
		val_value_t *val,
		int32  indent)
{
    xml_wr_check_val(scb, msg, val, indent, NULL);

}  /* xml_wr_val */



/********************************************************************
* FUNCTION xml_wr_full_check_val
* 
* Write an entire val_value_t out as XML, including the top level
* Using an optional testfn to filter output
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   testcb == callback function to use, NULL if not used
*   
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_full_check_val (ses_cb_t *scb,
			   xml_msg_hdr_t *msg,
			   val_value_t *val,
			   int32  indent,
			   ncx_nodetest_fn_t testfn)
{
    val_value_t  *vir;
    val_value_t *out;
    status_t  res;

#ifdef DEBUG
    if (!scb || !msg || !val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (val_is_virtual(val)) {
	vir = val_get_virtual_value(scb, val, &res);
	if (!vir) {
	    SET_ERROR(res);
	    return;
	}
	out = vir;
    } else {
	vir = NULL;
	out = val;
    }

    /* check if this is a false (not present) flag */
    if (out->btyp==NCX_BT_EMPTY && !VAL_BOOL(out)) {
	return;
    }

    /* write the value node contents or an empty node if none */
    if (val_has_content(out)) {

	/* write the top-level start node */
	xml_wr_begin_elem_ex(scb, msg,
			     val_get_parent_nsid(out),
			     out->nsid, out->name, 
			     &out->metaQ, METAQ, 
			     indent, START);

	/* write the value node contents */
	xml_wr_check_val(scb, msg, out, 
			 indent+ses_indent_count(scb), testfn);

	/* write the top-level end node */
	xml_wr_end_elem(scb, msg, out->nsid, out->name, 
			fit_on_line(scb, out) ? -1 : indent);
    } else {
	/* write the top-level empty node */
	xml_wr_begin_elem_ex(scb, msg,
			     val_get_parent_nsid(out),
			     out->nsid, out->name, 
			     &out->metaQ, METAQ, 
			     indent, EMPTY);
    }

    if (vir) {
	val_free_value(vir);
    }

}  /* xml_wr_full_check_val */


/********************************************************************
* FUNCTION xml_wr_full_val
* 
* Write an entire val_value_t out as XML, including the top level
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_full_val (ses_cb_t *scb,
		     xml_msg_hdr_t *msg,
		     val_value_t *val,
		     int32  indent)
{
    xml_wr_full_check_val(scb, msg, val, indent, NULL);
				
} /* xml_wr_full_val */


/********************************************************************
* FUNCTION xml_wr_check_file
* 
* Write the specified value to a FILE in XML format
*
* INPUTS:
*    filespec == exact path of filename to open
*    val == value for output
*    attrs == top-level attributes to generate
*    docmode == TRUE if XML_DOC output mode should be used
*            == FALSE if XML output mode should be used
*    xmlhdr == TRUE if <?xml?> directive should be output
*            == FALSE if not
*    indent == indent amount (0..9 spaces)
*    testfn == callback test function to use
*
* RETURNS:
*    status
*********************************************************************/
status_t
    xml_wr_check_file (const xmlChar *filespec, 
		       val_value_t *val,
		       xml_attrs_t *attrs,
		       boolean docmode,
		       boolean xmlhdr,
		       int32  indent,
		       ncx_nodetest_fn_t testfn)
{
    FILE       *fp;
    ses_cb_t   *scb;
    rpc_msg_t  *msg;
    status_t    res;
    ses_mode_t  sesmode;
    boolean     anyout;

    res = NO_ERR;
    msg = NULL;
    anyout = FALSE;
    fp = NULL;
    indent = min(indent, 9);

    if (filespec) {
	fp = fopen((const char *)filespec, "w");
	if (!fp) {
	    return ERR_FIL_OPEN;
	}
    }

    /* get a dummy session control block */
    scb = ses_new_dummy_scb();
    if (!scb) {
	res = ERR_INTERNAL_MEM;
    } else {
	scb->fp = fp;
	scb->indent = indent;
    }

    /* get a dummy output message */
    if (res == NO_ERR) {
	msg = rpc_new_out_msg();
	if (!msg) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    /* hack -- need a queue because there is no top
	     * element which this usually shadows
	     */
	    msg->rpc_in_attrs = attrs;
	}
    }

    /* XML output mode will add more whitespace if this is ncxdump calling */
    if (res == NO_ERR && docmode) {
	sesmode = ses_get_mode(scb);
	ses_set_mode(scb, SES_MODE_XMLDOC);
    }
    
    /* send the XML declaration */
    if (res == NO_ERR && xmlhdr) {
	res = ses_start_msg(scb);
	if (res == NO_ERR) {
	    anyout = TRUE;
	}
    }

    /* setup an empty prefix map */
    if (res == NO_ERR) {
	res = xml_msg_build_prefix_map(&msg->mhdr,
				       msg->rpc_in_attrs, FALSE, FALSE);
    }

    /* generate the <foo> start tag */
    if (res == NO_ERR) {
	xml_wr_begin_elem_ex(scb, &msg->mhdr,
			     0, val->nsid, val->name, 
			     attrs, TRUE, 0, FALSE);
	anyout = TRUE;
    }

    /* output the value */
    if (res == NO_ERR) {
	xml_wr_check_val(scb, &msg->mhdr, val, indent, testfn);
    }

    /* generate the <foo> end tag */
    if (res == NO_ERR) {
	xml_wr_end_elem(scb, &msg->mhdr, val->nsid, val->name, 0);
    }

    /* finish the message, should be NO-OP  */
    if (anyout) {
	ses_finish_msg(scb);
    }

    if (res == NO_ERR && docmode) {
	ses_set_mode(scb, sesmode);
    }

    /* clean up and exit */
    if (msg) {
	rpc_free_msg(msg);
    }
    if (scb) {
	ses_free_scb(scb);
    }

    return res;

} /* xml_wr_check_file */


/********************************************************************
* FUNCTION xml_wr_file
* 
* Write the specified value to a FILE in XML format
*
* INPUTS:
*    filespec == exact path of filename to open
*    val == value for output
*    attrs == top-level attributes to generate
*    docmode == TRUE if XML_DOC output mode should be used
*            == FALSE if XML output mode should be used
*    xmlhdr == TRUE if <?xml?> directive should be output
*            == FALSE if not
*    indent == indent amount (0..9 spaces)
*
* RETURNS:
*    status
*********************************************************************/
status_t
    xml_wr_file (const xmlChar *filespec,
		 val_value_t *val,
		 xml_attrs_t *attrs,
		 boolean docmode,
		 boolean xmlhdr,
		 int32 indent)
{
    return xml_wr_check_file(filespec, val, attrs, 
			     docmode, xmlhdr, indent, NULL);

} /* xml_wr_file */



/* END file xml_wr.c */
