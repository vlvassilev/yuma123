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
22-aug-08    abb      changed reader parameter to the session
                      control block to record error stats;
                      also errQ to msghdr to record xmlns directives 
*/


#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_xml_consume_node (ses_cb_t *scb,
			  xml_node_t *node,
			  ncx_layer_t layer,
			  xml_msg_hdr_t *msghdr);


/* do not generate an EOF error if seen */
extern status_t 
    agt_xml_consume_node_noeof (ses_cb_t *scb,
				xml_node_t *node,
				ncx_layer_t layer,
				xml_msg_hdr_t *msghdr);


/* do not generate namespace errors if seen 
 * needed to process subtree filters properly
 */
extern status_t 
    agt_xml_consume_node_nons (ses_cb_t *scb,
			       xml_node_t *node,
			       ncx_layer_t layer,
			       xml_msg_hdr_t *msghdr);

/* do not advance the node pointer */
extern status_t 
    agt_xml_consume_node_noadv (ses_cb_t *scb,
				xml_node_t *node,
				ncx_layer_t layer,
				xml_msg_hdr_t *msghdr);

extern status_t 
    agt_xml_consume_node_nons_noadv (ses_cb_t *scb,
				     xml_node_t *node,
				     ncx_layer_t layer,
				     xml_msg_hdr_t *msghdr);

extern status_t 
    agt_xml_skip_subtree (ses_cb_t *scb,
			  const xml_node_t *startnode);


#endif	    /* _H_agt_xml */

