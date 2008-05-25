#ifndef _H_mgr_xml
#define _H_mgr_xml
/*  FILE: mgr_xml.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Manager XML Reader interface

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
12-feb-07    abb      begun; start from agt_xml.c
*/

/* From /usr/include/libxml2/libxml/ */
#include <xmlreader.h>
#include <xmlstring.h>

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/**************** XMLTextReader APIs ******************/

extern status_t 
    mgr_xml_consume_node (xmlTextReaderPtr reader,
			  xml_node_t       *node);

/* do not generate namespace errors if seen 
 * needed to process subtree filters properly
 */
extern status_t 
    mgr_xml_consume_node_nons (xmlTextReaderPtr reader,
			       xml_node_t      *node);

/* re-get  the current node */
status_t 
    mgr_xml_consume_node_noadv (xmlTextReaderPtr reader,
				xml_node_t      *node);

extern status_t 
    mgr_xml_skip_subtree (xmlTextReaderPtr reader,
			  const xml_node_t *startnode);

#endif	    /* _H_mgr_xml */
