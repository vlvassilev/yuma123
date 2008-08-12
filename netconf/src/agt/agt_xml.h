#ifndef _H_agt_xml
#define _H_agt_xml
/*  FILE: agt_xml.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Agent XML Reader interface

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
14-oct-05    abb      begun
2-jan-06     abb      rewrite xml_consume_* API to use simpler 
                      xml_node_t
11-feb-07    abb      moved consume_node fns to agt_xml.h
*/


/* From /usr/include/libxml2/libxml/ */
#include <xmlreader.h>
#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
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
    agt_xml_consume_node (xmlTextReaderPtr reader,
			  xml_node_t       *node,
			  ncx_layer_t       layer,
			  dlq_hdr_t         *errQ);


/* do not generate an EOF error if seen */
extern status_t 
    agt_xml_consume_node_noeof (xmlTextReaderPtr reader,
				xml_node_t       *node,
				ncx_layer_t       layer,
				dlq_hdr_t        *errQ);


/* do not generate namespace errors if seen 
 * needed to process subtree filters properly
 */
extern status_t 
    agt_xml_consume_node_nons (xmlTextReaderPtr reader,
			       xml_node_t      *node,
			       ncx_layer_t      layer,
			       dlq_hdr_t       *errQ);

/* do not advance the node pointer */
extern status_t 
    agt_xml_consume_node_noadv (xmlTextReaderPtr reader,
				xml_node_t       *node,
				ncx_layer_t       layer,
				dlq_hdr_t        *errQ);


extern status_t 
    agt_xml_skip_subtree (xmlTextReaderPtr reader,
			  const xml_node_t *startnode);

extern status_t 
    agt_xml_consume_node_nons_noadv (xmlTextReaderPtr reader,
				     xml_node_t      *node,
				     ncx_layer_t      layer,
				     dlq_hdr_t        *errQ);


#endif	    /* _H_agt_xml */

