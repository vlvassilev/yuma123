/*  FILE: agt_xml.c

    Agent XML Reader interface

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
11feb07      abb      begun; split from xml_util.c


*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include "procdefs.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_xml
#include "agt_xml.h"
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

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                          C O N S T A N T S                        *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_XML_DEBUG  1
#endif

#define XML_READER_OPTIONS    XML_PARSE_RECOVER+XML_PARSE_NOERROR+\
	XML_PARSE_NOWARNING+XML_PARSE_NOBLANKS+XML_PARSE_NONET

#define XML_SES_URL "netconf://pdu"

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION get_errnode
* 
* Parse the current node and return its namespace, type and name
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   node    == pointer to an initialized xml_node_t struct
*              to be filled in
*   
* OUTPUTS:
*   *node == xml_node_t struct filled in 
*   
*********************************************************************/
static void
    get_errnode (xmlTextReaderPtr reader,
		 xml_node_t      *node)
{
    int             nodetyp;
    const xmlChar  *str, *errstr;
    xmlChar        *valstr;
    uint32          len;

    /* get the node depth to match the end node correctly */
    node->depth = xmlTextReaderDepth(reader);
    if (node->depth == -1) {
	node->depth = 0;
    }

    /* get the internal nodetype, check it and convert it */
    nodetyp = xmlTextReaderNodeType(reader);
    switch (nodetyp) {
    case XML_ELEMENT_NODE:
	if (xmlTextReaderIsEmptyElement(reader)) {
	    node->nodetyp = XML_NT_EMPTY;
	} else {
	    node->nodetyp = XML_NT_START;
	}
	break;
    case XML_ELEMENT_DECL:
	node->nodetyp = XML_NT_END;
	break;
    case XML_DTD_NODE:
    case XML_TEXT_NODE:
	node->nodetyp = XML_NT_STRING;
	break;
    default:
	node->nodetyp = XML_NT_NONE;
    }

    /* finish the node, depending on its type */
    switch (node->nodetyp) {
    case XML_NT_START:
    case XML_NT_END:
    case XML_NT_EMPTY:
	/* get the element QName */
	str = xmlTextReaderConstName(reader);
	if (!str) {
	    str = (const xmlChar *)"--";
	}
	node->qname = str;

	/* check for namespace prefix in the name 
	 * ignore unknown namespace error if any
	 */
	len = 0;
	(void)xml_check_ns(reader, str, &node->nsid, &len, &errstr);

	/* set the element name to the char after the prefix, if any */
	node->elname = (const xmlChar *)(str+len);
	node->module = NULL;
	break;
    case XML_NT_STRING:
	/* get the text value -- this is a malloced string */
	node->simval = NULL;
	valstr = xmlTextReaderValue(reader);
	if (valstr) {
	    /* normal case: assume clean the string is desired! */
	    node->simfree = xml_copy_clean_string(valstr);
	    if (node->simfree) {
		node->simlen = xml_strlen(node->simfree);
		node->simval = (const xmlChar *)node->simfree;
	    }
	    xmlFree(valstr);
	}
	if (!node->simval) {
	    /* prevent a NULL ptr reference */
	    node->simval = (const xmlChar *)"";
	    node->simlen = 0;     
	    node->simfree = NULL;
	}
	break;
    default:
	break;
    }

}  /* get_errnode */




/********************************************************************
* FUNCTION get_all_attrs
* 
*  Copy all the attributes from the current node to
*  the xml_attrs_t queue
*
* INPUTS:
*    scb == session control block
*    msghdr  == msg hdr w/ Q to get any rpc-errors as found, 
*               NULL if not used
*    nserr == TRUE if unknown namespace should cause the
*             function to fail and the attr not to be saved 
*             This is the normal mode.
*          == FALSE and the namespace will be marked INVALID
*             but an error will not be returned
*
* OUTPUTS:
*   attrs Q contains 0 or more entries
*   *msghdr.errQ may have rpc-errors added to it
*
* RETURNS:
*   status of the operation
*   returns NO_ERR if all copied okay or even zero copied
*********************************************************************/
static status_t
    get_all_attrs (ses_cb_t *scb,
		   xml_attrs_t *attrs,
		   ncx_layer_t layer,
		   xml_msg_hdr_t *msghdr,
		   boolean nserr)
{
    int            i, cnt, ret;
    xmlChar       *value;
    const xmlChar *badns, *name;
    xmlns_id_t     nsid;
    status_t       res;
    boolean        done;
    xml_attr_t     errattr;
    xml_node_t     errnode;
    uint32         plen;

    /* check the attribute count first */
    cnt = xmlTextReaderAttributeCount(scb->reader);
    if (cnt==0) {
	return NO_ERR;
    }

    /* move through the list of attributes */
    for (i=0, done=FALSE; i<cnt && !done; i++) {
	res = NO_ERR;
	name = NULL;
	value = NULL;
	badns = NULL;
	plen = 0;
	nsid = 0;

	/* get the next attribute */
	if (i==0) {
	    ret = xmlTextReaderMoveToFirstAttribute(scb->reader);
	} else {
	    ret = xmlTextReaderMoveToNextAttribute(scb->reader);
	}
	if (ret != 1) {
	    res = ERR_XML_READER_INTERNAL;
	    done = TRUE;
	} else {
	    /* get the attribute name */
	    name = xmlTextReaderConstName(scb->reader);
	    if (!name) {
		res = ERR_XML_READER_NULLNAME;
	    } else {
		value = NULL;
		res = xml_check_ns(scb->reader, name, 
				   &nsid, &plen, &badns);
		if (!nserr && res != NO_ERR) {
		    nsid = xmlns_inv_id();
		    plen = 0;
		    res = NO_ERR;
		}

		/* get the attribute value even if a NS error */
		value = xmlTextReaderValue(scb->reader);
		if (value) {
		    /* save the values as received, may be QName 
		     * only error that can occur is a malloc fail
		     */
		    res = xml_add_qattr(attrs, nsid, name, plen, value);
		    xmlFree(value);
		} else {
		    res = ERR_XML_READER_NULLVAL;
		}
	    }
	}

	/* check the result */
	if (res != NO_ERR && msghdr) {
	    xml_init_node(&errnode);
	    get_errnode(scb->reader, &errnode);

	    /* save the error info */
	    if (res==ERR_XML_READER_NULLVAL ||
		res==ERR_NCX_UNKNOWN_NAMESPACE) {

		memset(&errattr, 0x0, sizeof(xml_attr_t));
		errattr.attr_ns = 0;
		errattr.attr_qname = name;
		errattr.attr_name = name;
		errattr.attr_val = value;

		/* generate an attribute error */
		agt_record_attr_error(scb, msghdr, 
				      layer, res, &errattr, 
				      &errnode, badns,
				      NCX_NT_NONE, NULL);
	    } else {
		/* generate an operation-failed error */
		agt_record_error(scb, msghdr, layer, res, 
				 &errnode, NCX_NT_STRING, 
				 name, NCX_NT_NONE, NULL);
	    }
	    xml_clean_node(&errnode);
	}
    }

    /* reset the current node to where we started */
    ret = xmlTextReaderMoveToElement(scb->reader);
    if (ret != 1) {
	if (msghdr) {
	    res = ERR_XML_READER_INTERNAL;
	    xml_init_node(&errnode);
	    get_errnode(scb->reader, &errnode);

	    agt_record_error(scb, msghdr, layer, res, 
			     &errnode, NCX_NT_STRING, 
			     name, NCX_NT_NONE, NULL);
	    xml_clean_node(&errnode);
	}
    }	

    return res;

}  /* get_all_attrs */


/********************************************************************
* FUNCTION consume_node
* 
* Internal consume XML node function
* see agt_xml_consume_node for details.
*
* EXTRA INPUTS:
*   eoferr == TRUE if an End of File error should be generated
*          == FALSE if not
*    nserr == TRUE if bad namespace should be checked
*          == FALSE if not
*    clean == TRUE is a string should be cleaned before returned 
*          == FALSE if a string node should be returned as-is
*
* RETURNS:
*   status of the operation
*   Try to fail on fatal errors only
*********************************************************************/
static status_t 
    consume_node (ses_cb_t *scb,
		  boolean advance,
		  xml_node_t *node,
		  ncx_layer_t layer,
		  xml_msg_hdr_t *msghdr,
		  boolean eoferr,
		  boolean nserr,
		  boolean clean)
{
    int             ret, nodetyp;
    const xmlChar  *str, *badns;
    xmlChar        *valstr;
    uint32          len;
    status_t        res, res2;
    boolean         done;

    /* init local vars */
    done = FALSE;
    res = NO_ERR;
    res2 = NO_ERR;
    badns = NULL;

    /* loop past any unused xmlTextReader node types */
    while (!done) {
	/* check if a new node should be read */
	if (advance) {
	    /* advance the node pointer */
	    ret = xmlTextReaderRead(scb->reader);
	    if (ret != 1) {
		/* do not treat this as an internal error */
		res = ERR_XML_READER_EOF;
		if (msghdr && eoferr) {
		    /* generate an operation-failed error */
		    agt_record_error(scb, msghdr, layer, res, 
				     NULL, NCX_NT_NONE, NULL, 
				     NCX_NT_NONE, NULL);
		}
		return res;
	    }
	}

	/* get the node depth to match the end node correctly */
	node->depth = xmlTextReaderDepth(scb->reader);
	if (node->depth == -1) {
	    /* this never actaully happens */
	    SET_ERROR(ERR_XML_READER_INTERNAL);
	    node->depth = 0;
	}

	/* get the internal nodetype, check it and convert it */
	nodetyp = xmlTextReaderNodeType(scb->reader);
	switch (nodetyp) {
	case XML_ELEMENT_NODE:
	    /* classify element as empty or start */
	    if (xmlTextReaderIsEmptyElement(scb->reader)) {
		node->nodetyp = XML_NT_EMPTY;
	    } else {
		node->nodetyp = XML_NT_START;
	    }
	    done = TRUE;
	    break;
	case XML_ELEMENT_DECL:
	    node->nodetyp = XML_NT_END;
	    done = TRUE;
	    break;
	case XML_TEXT_NODE:
     /* case XML_DTD_NODE: */
	    node->nodetyp = XML_NT_STRING;
	    done = TRUE;
	    break;
	default:
	    /* unused node type -- keep trying */
#ifdef XML_UTIL_DEBUG
	    log_debug3("\nxml_consume_node: skip unused node (%s)",
		   xml_get_node_name(nodetyp));
#endif
	    advance = TRUE;
	}
    }

    /* finish the node, depending on its type */
    switch (node->nodetyp) {
    case XML_NT_START:
    case XML_NT_END:
    case XML_NT_EMPTY:
	/* get the element QName */
	str = xmlTextReaderConstName(scb->reader);
	if (!str) {
	    /* this never really happens */
	    SET_ERROR(ERR_XML_READER_NULLNAME);
	    str = (const xmlChar *)"null";
	}
	node->qname = (const xmlChar *)str;

	/* check for namespace prefix in the name 
	 * only error returned is unknown-namespace 
	 */
	len = 0;
	res = xml_check_ns(scb->reader, str, &node->nsid, &len, &badns);
	if (!nserr && res != NO_ERR) {
	    node->nsid = xmlns_inv_id();
	    len = 0;
	    res = NO_ERR;
	}
	    
	/* set the element name to the char after the prefix, if any */
	node->elname = (const xmlChar *)(str+len);
	
	/* get all the attributes -- should be none for XML_NT_END */
	if (res == NO_ERR) {
	    res2 = get_all_attrs(scb, &node->attrs, 
				 layer, msghdr, nserr);
	}

	/* Set the node module */
	if (res == NO_ERR) {
	    if (node->nsid) {
		node->module = xmlns_get_module(node->nsid);
	    } else {
		/* no entry, use the default module (ncx) */
		node->module = NCX_DEF_MODULE;
	    }
	}
	break;
    case XML_NT_STRING:
	/* get the text value -- this is a malloced string */
	node->simval = NULL;
	valstr = xmlTextReaderValue(scb->reader);
	if (valstr) {
	    if (clean) {
		node->simfree = xml_copy_clean_string(valstr);
	    } else {
		node->simfree = xml_strdup(valstr);
	    }
	    if (node->simfree) {
		node->simlen = xml_strlen(node->simfree);
		node->simval = (const xmlChar *)node->simfree;
	    }

	    /* see if this is a QName string; if so save the NSID */
	    xml_check_qname_content(scb->reader, node);

	    xmlFree(valstr);
	}
	if (!node->simval) {
	    /* prevent a NULL ptr reference */
	    node->simval = (const xmlChar *)"";
	    node->simlen = 0;
	    node->simfree = NULL;
	}
	break;
    default:
	break;
    }

    if ((res != NO_ERR) && msghdr) {
	if (badns) {
	    /* generate an operation-failed error */
	    agt_record_error(scb, msghdr, layer, res, 
			     node, NCX_NT_STRING, badns, 
			     NCX_NT_NONE, NULL);
	} else {
	    agt_record_error(scb, msghdr, layer, res, 
			     node, NCX_NT_NONE, NULL, 
			     NCX_NT_NONE, NULL);
	}
    }

#ifdef XML_UTIL_DEBUG
    log_debug3("\nxml_consume_node: return (%d)", 
	   (res==NO_ERR) ? res2 : res);
    if (LOGDEBUG3) {
	xml_dump_node(node);
    }
#endif

    /* return general error first, then attribute error 
     * It doesn't really matter since the caller will 
     * assume all error reports have been queued upon return
     */
    return (res==NO_ERR) ? res2 : res;

}  /* consume_node */


/************** E X T E R N A L   F U N C T I O N S   **************/


/********************************************************************
* FUNCTION agt_xml_consume_node
* 
* Parse the next node and return its namespace, type and name
* The xml_init_node or xml_clean_node API must be called before
* this function for the node parameter
*
* There are 2 types of XML element start nodes 
*   - empty node                          (XML_NT_EMPTY)
*   - start of a simple or complex type   (XML_NT_START)
*
* There is one string content node for simpleType content
*   - string node                         (XML_NT_STRING)
*
* There is one end node to end both simple and complex types
*   - end node                            (XML_NT_END)
*
* If nodetyp==XML_NT_EMPTY, then no further nodes will occur
* for this element. This node may contain attributes. The
* naming parameters will all be set.
*
* If nodetyp==XML_NT_START, then the caller should examine
* the schema for that start node.  
* For complex types, the next node is probably another XML_NT_START.
* For simple types, the next node will be XML_NT_STRING,
* followed by an XML_NT_END node. This node may contain attributes. 
* The naming parameters will all be set.
*
* If the nodetype==XML_NT_STRING, then the simval and simlen
* fields will be set.  There are no attributes or naming parameters
* for this node type.
*
* IF the nodetype==XML_NT_END, then no further nodes for this element
* will occur.  This node should not contain attributes.
* All of the naming parameters will be set. The xml_endnode_match
* function should be used to confirm that the XML_NT_START and
* XML_NT_END nodes are paired correctly.
*
* The node pointer for the scb->reader will be advanced before the
* node is read.
* 
* INPUTS:
*   scb == session control block containing XmlTextReader
*
*   node    == pointer to an initialized xml_node_t struct
*              to be filled in
*   layer == protocol layer of caller (only used if errQ != NULL)
*    msghdr  == msg hdr w/ Q to get any rpc-errors as found, 
*               NULL if not used
*   
* OUTPUTS:
*   *node == xml_node_t struct filled in 
*   *errQ may have errors appended to it
*   
* RETURNS:
*   status of the operation
*   Try to fail on fatal errors only
*********************************************************************/
status_t 
    agt_xml_consume_node (ses_cb_t *scb,
			  xml_node_t *node,
			  ncx_layer_t layer,
			  xml_msg_hdr_t *msghdr)
{
    return consume_node(scb, TRUE, node, layer, msghdr, 
			TRUE, TRUE, TRUE);

}  /* agt_xml_consume_node */


status_t 
    agt_xml_consume_node_noeof (ses_cb_t *scb,
				xml_node_t *node,
				ncx_layer_t layer,
				xml_msg_hdr_t *msghdr)
{
    return consume_node(scb, TRUE, node, layer, msghdr, 
			FALSE, TRUE, TRUE);

}  /* agt_xml_consume_node_noeof */


status_t 
    agt_xml_consume_node_nons (ses_cb_t *scb,
			       xml_node_t *node,
			       ncx_layer_t layer,
			       xml_msg_hdr_t *msghdr)
{
    return consume_node(scb, TRUE, node, layer, msghdr, 
			FALSE, FALSE, TRUE);

}  /* agt_xml_consume_node_nons */


status_t 
    agt_xml_consume_node_noadv (ses_cb_t *scb,
				xml_node_t *node,
				ncx_layer_t layer,
				xml_msg_hdr_t *msghdr)
{
    return consume_node(scb, FALSE, node, layer, msghdr, 
			TRUE, TRUE, TRUE);

}  /* agt_xml_consume_node_noadv */


status_t 
    agt_xml_consume_node_nons_noadv (ses_cb_t *scb,
				     xml_node_t *node,
				     ncx_layer_t layer,
				     xml_msg_hdr_t *msghdr)
{
    return consume_node(scb, FALSE, node, layer, msghdr, 
			TRUE, FALSE, TRUE);

}  /* agt_xml_consume_node_nons_noadv */


/********************************************************************
* FUNCTION agt_xml_skip_subtree
* 
* Already encountered an error, so advance nodes until the
* matching start-node is reached or a terminating error occurs
*   - end of input
*   - start depth level reached
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   startnode  == xml_node_t of the start node of the sub-tree to skip
* RETURNS:
*   status of the operation
* SIDE EFFECTS:
*   the xmlreader state is advanced until the current node is the
*   end node of the specified start node or a fatal error occurs
*********************************************************************/
status_t 
    agt_xml_skip_subtree (ses_cb_t *scb,
			  const xml_node_t *startnode)
{
    xml_node_t       node;
    const xmlChar   *qname, *elname, *badns;
    uint32           len;
    int              ret, depth, nodetyp;
    xmlns_id_t       nsid;
    boolean          done, justone;
    status_t         res;

#ifdef DEBUG
    if (!scb || !startnode) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    justone = FALSE;

    switch (startnode->nodetyp) {
    case XML_NT_START:
	break;
    case XML_NT_EMPTY:
	return NO_ERR;
    case XML_NT_STRING:
	justone = TRUE;
	break;
    case XML_NT_END:
	return NO_ERR;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    xml_init_node(&node);
    res = agt_xml_consume_node_noadv(scb, &node, 
				     NCX_LAYER_NONE, NULL);
    if (res == NO_ERR) {
	res = xml_endnode_match(startnode, &node);
	xml_clean_node(&node);
	if (res == NO_ERR) {
	    return NO_ERR;
	}
    }

    if (justone) {
	return NO_ERR;
    }

    done = FALSE;
    while (!done) {

	/* advance the node pointer */
	ret = xmlTextReaderRead(scb->reader);
	if (ret != 1) {
	    /* fatal error */
	    return SET_ERROR(ERR_XML_READER_EOF);
	}

	/* get the node depth to match the end node correctly */
	depth = xmlTextReaderDepth(scb->reader);
	if (depth == -1) {
	    /* not sure if this can happen, treat as fatal error */
	    return SET_ERROR(ERR_XML_READER_INTERNAL);
	} else if (depth <= startnode->depth) {
	    /* this depth override will cause errors to be ignored
             *   - wrong namespace in matching end node
             *   - unknown namespace in matching end node
	     *   - wrong name in 'matching' end node
	     */
	    done = TRUE;
	}

	/* get the internal nodetype, check it and convert it */
	nodetyp = xmlTextReaderNodeType(scb->reader);

	/* get the element QName */
	qname = xmlTextReaderConstName(scb->reader);
	if (qname) {
	    /* check for namespace prefix in the name 
	     * only error is 'unregistered namespace ID'
	     * which doesn't matter in this case
	     */
	    nsid = 0;
	    (void)xml_check_ns(scb->reader, qname, &nsid, 
			       &len, &badns);

	    /* set the element name to the char after the prefix */
	    elname = qname+len;
	} else {
	    qname = (const xmlChar *)"";
	}

	/* check the normal case to see if the search is done */
	if (depth == startnode->depth &&
	    !xml_strcmp(qname, startnode->qname) &&
	    nodetyp == XML_ELEMENT_DECL) {
	    done = TRUE;
	}

#ifdef XML_UTIL_DEBUG
	log_debug3("\nxml_skip: %s L:%d T:%s",
	       qname, depth, xml_get_node_name(nodetyp));
#endif
    }

    return NO_ERR;

}  /* agt_xml_skip_subtree */

/* END agt_xml.c */
