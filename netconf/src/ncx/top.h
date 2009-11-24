/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_top
#define _H_top

/*  FILE: top.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Common Top Element module

    Manage callback registry for received XML messages

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
30-dec-05    abb      Begun
11-feb-07    abb      Move some common fns back to ncx/top.h
*/

#include <xmlstring.h>

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/* callback function template for a top handler 
 *
 * INPUTS:
 *   scb == session control block; source of input
 *          The session manager will only stream 
 *          one message to this function and wait
 *          for it to return.
 *   top == very first node of the incoming message
 *          This allows a single handler to support
 *          multiple top nodes, and also contains
 *          the attributes present in the node
 * RETURNS:
 *   none
 */
typedef void (*top_handler_t) (ses_cb_t   *scb,
			       xml_node_t *top);


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    top_init (void);

extern void 
    top_cleanup (void);

extern status_t 
    top_register_node (const xmlChar *owner,
		       const xmlChar *elname,
		       top_handler_t handler);

extern void
    top_unregister_node (const xmlChar *owner,
			 const xmlChar *elname);

extern top_handler_t
    top_find_handler (const xmlChar *owner,
		      const xmlChar *elname);


#endif	    /* _H_top */
