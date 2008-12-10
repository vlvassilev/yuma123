/*  FILE: mgr_xml.c

    Manager XML Reader interface

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

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_mgr_xml
#include "mgr_xml.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
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
* FUNCTION dump_skipnode
* 
*  Dump some debug info during XML node skipping
*
* INPUTS:
*    node == xml node to print debug info for
*
*********************************************************************/
static void
    dump_skipnode (const xml_node_t *node)
{
    if (LOGDEBUG2) {
	switch (node->nodetyp) {
	case XML_NT_NONE:
	    log_debug2("\nmgr_xml: skip NULL node");
	    break;
	case XML_NT_EMPTY:
	    log_debug2("\nmgr_xml: skip empty node %s",
		       node->elname);
	    break;
	case XML_NT_START:
	    log_debug2("\nmgr_xml: skip start node %s",
		       node->elname);
	    break;
	case XML_NT_END:
	    log_debug2("\nmgr_xml: skip end node %s",
		       node->elname);
	    break;
	case XML_NT_STRING:
	    log_debug2("\nmgr_xml: skip string node %20s",
		       (node->simval) ? 
		       (const char *)node->simval : "--");
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
} /* dump_skipnode */


/********************************************************************
* FUNCTION get_attrs
* 
*  Copy all the attributes from the current node to
*  the xml_attrs_t queue
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*    attrs == pointer to output var
*    nserr == TRUE if unknown namespace should cause the
*             function to fail and the attr not to be saved 
*             This is the normal mode.
*          == FALSE and the namespace will be marked INVALID
*             but an error will not be returned
*
* OUTPUTS:
*   attrs Q contains 0 or more entries
*   *errQ may have rpc-errors added to it
*
* RETURNS:
*   status of the operation
*   returns NO_ERR if all copied okay or even zero copied
*********************************************************************/
static status_t
    get_attrs (xmlTextReaderPtr reader,
		   xml_attrs_t  *attrs,
		   boolean nserr)
{
    int            i, cnt, ret;
    xmlChar       *value;
    const xmlChar *badns, *name;
    xmlns_id_t     nsid;
    status_t       res;
    boolean        done;
    uint32         plen;

    /* check the attribute count first */
    cnt = xmlTextReaderAttributeCount(reader);
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
	    ret = xmlTextReaderMoveToFirstAttribute(reader);
	} else {
	    ret = xmlTextReaderMoveToNextAttribute(reader);
	}
	if (ret != 1) {
	    res = ERR_XML_READER_INTERNAL;
	    done = TRUE;
	} else {
	    /* get the attribute name */
	    name = xmlTextReaderConstName(reader);
	    if (!name) {
		res = ERR_XML_READER_NULLNAME;
	    } else {
		value = NULL;
		res = xml_check_ns(reader, name, &nsid, &plen, &badns);
		if (!nserr && res != NO_ERR) {
		    nsid = xmlns_inv_id();
		    plen = 0;
		    res = NO_ERR;
		}
		
		/* get the attribute value even if a NS error */
		value = xmlTextReaderValue(reader);
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
    }

    /* reset the current node to where we started */
    ret = xmlTextReaderMoveToElement(reader);
    if (ret != 1 && res==NO_ERR) {
	res = ERR_XML_READER_INTERNAL;
    }	

    return res;

}  /* get_attrs */


/********************************************************************
* FUNCTION mconsume_node
* 
* INPUTS:
*    reader == xmlTextReader to use
*    node == allocated and initialized xml_node_t to fill in
*    nserr == TRUE if bad namespace should be checked
*          == FALSE if not
*    adv == TRUE if advance reader
*        == FALSE if no advance (reget current node)
*
* RETURNS:
*   status of the operation
*   Try to fail on fatal errors only
*********************************************************************/
static status_t 
    mconsume_node (xmlTextReaderPtr reader,
		   xml_node_t  *node,
		   boolean nserr,
		   boolean adv)     
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
    node->nodetyp = XML_NT_NONE;

    /* loop past any unused xmlTextReader node types */
    while (!done) {
	if (adv) {
	    /* advance the node pointer */
	    ret = xmlTextReaderRead(reader);
	    if (ret != 1) {
		/* do not treat this as an internal error */
		return ERR_XML_READER_EOF;
	    }
	} else {
	    /* make sure no 2nd pass through the loop */
	    done = TRUE;
	}

	/* get the node depth to match the end node correctly */
	node->depth = xmlTextReaderDepth(reader);
	if (node->depth == -1) {
	    /* this never actaully happens */
	    SET_ERROR(ERR_XML_READER_INTERNAL);
	    node->depth = 0;
	}

	/* get the internal nodetype, check it and convert it */
	nodetyp = xmlTextReaderNodeType(reader);
	switch (nodetyp) {
	case XML_ELEMENT_NODE:
	    /* classify element as empty or start */
	    if (xmlTextReaderIsEmptyElement(reader)) {
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
	    if (done) {
		/* re-get of current node should not fail */
		res = ERR_XML_READER_INTERNAL;
	    }
	}
    }

    /* finish the node, depending on its type */
    switch (node->nodetyp) {
    case XML_NT_START:
    case XML_NT_END:
    case XML_NT_EMPTY:
	/* get the element QName */
	str = xmlTextReaderConstName(reader);
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
	res = xml_check_ns(reader, str, &node->nsid, &len, &badns);
	if (!nserr && res != NO_ERR) {
	    node->nsid = xmlns_inv_id();
	    len = 0;
	    res = NO_ERR;
	}
	    
	/* set the element name to the char after the prefix, if any */
	node->elname = (const xmlChar *)(str+len);
	
	/* get all the attributes -- should be none for XML_NT_END */
	res2 = get_attrs(reader, &node->attrs, nserr);

	/* Set the node owner */
	if (node->nsid) {
	    node->module = xmlns_get_module(node->nsid);
	} else {
	    /* no entry, use the default owner (netconf) */
	    node->module = NCX_DEF_MODULE;
	}
	break;
    case XML_NT_STRING:
	/* get the text value */
	node->simval = NULL;
	valstr = xmlTextReaderValue(reader);
	if (valstr) {
	    node->simfree = xml_copy_clean_string(valstr);
	    if (node->simfree) {
		node->simlen = xml_strlen(node->simfree);
		node->simval = (const xmlChar *)node->simfree;
	    }

	    /* see if this is a QName string; if so save the NSID */
	    xml_check_qname_content(reader, node);

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

#ifdef XML_UTIL_DEBUG
    log_debug3("\nmgr_xml_consume_node: return (%d)", 
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

}  /* mconsume_node */


/************** E X T E R N A L   F U N C T I O N S   **************/


/********************************************************************
* FUNCTION mgr_xml_consume_node
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
* The node pointer for the reader will be advanced before the
* node is read.
* 
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   node    == pointer to an initialized xml_node_t struct
*              to be filled in
*   
* OUTPUTS:
*   *node == xml_node_t struct filled in 
*   reader will be advanced
*
* RETURNS:
*   status of the operation
*   Try to fail on fatal errors only
*********************************************************************/
status_t 
    mgr_xml_consume_node (xmlTextReaderPtr reader,
			  xml_node_t      *node)
{
    return mconsume_node(reader, node, TRUE, TRUE);

}  /* mgr_xml_consume_node */



status_t 
    mgr_xml_consume_node_nons (xmlTextReaderPtr reader,
			       xml_node_t      *node)
{
    return mconsume_node(reader, node, FALSE, TRUE);

}  /* mgr_xml_consume_node_nons */

status_t 
    mgr_xml_consume_node_noadv (xmlTextReaderPtr reader,
				xml_node_t      *node)
{
    return mconsume_node(reader, node, TRUE, FALSE);

}  /* mgr_xml_consume_node_noadv */


/********************************************************************
* FUNCTION mgr_xml_skip_subtree
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
    mgr_xml_skip_subtree (xmlTextReaderPtr reader,
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
    if (!reader || !startnode) {
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
    res = mgr_xml_consume_node_noadv(reader, &node);
    if (res == NO_ERR) {
	dump_skipnode(&node);
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
	ret = xmlTextReaderRead(reader);
	if (ret != 1) {
	    /* fatal error */
	    return ERR_XML_READER_EOF;
	}

	dump_skipnode(&node);

	/* get the node depth to match the end node correctly */
	depth = xmlTextReaderDepth(reader);
	if (depth == -1) {
	    /* not sure if this can happen, treat as fatal error */
	    return ERR_XML_READER_INTERNAL;
	} else if (depth <= startnode->depth) {
	    /* this depth override will cause errors to be ignored
             *   - wrong namespace in matching end node
             *   - unknown namespace in matching end node
	     *   - wrong name in 'matching' end node
	     */
	    done = TRUE;
	}

	/* get the internal nodetype, check it and convert it */
	nodetyp = xmlTextReaderNodeType(reader);

	/* get the element QName */
	qname = xmlTextReaderConstName(reader);
	if (qname) {
	    /* check for namespace prefix in the name 
	     * only error is 'unregistered namespace ID'
	     * which doesn't matter in this case
	     */
	    nsid = 0;
	    (void)xml_check_ns(reader, qname, &nsid, &len, &badns);

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

}  /* mgr_xml_skip_subtree */


/* END mgr_xml.c */
