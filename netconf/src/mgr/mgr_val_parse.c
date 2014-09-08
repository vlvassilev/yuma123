/*
 * Copyright (c) 2008 - 2012, Andy Bierman, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: mgr_val_parse.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
11feb06      abb      begun; hack, clone agent code and remove
                      all the rpc-error handling code; later a proper
                      libxml2 docPtr interface will be used instead

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#include <xmlstring.h>
#include <xmlreader.h>


#include "procdefs.h"
#include "b64.h"
#include "cfg.h"
#include "def_reg.h"
#include "dlq.h"
#include "log.h"
#include "mgr.h"
#include "mgr_val_parse.h"
#include "val_parse.h"
#include "mgr_xml.h"
#include "ncx.h"
#include "ncx_num.h"
#include "ncx_str.h"
#include "ncx_list.h"
#include "ncxconst.h"
#include "obj.h"
#include "status.h"
#include "tk.h"
#include "typ.h"
#include "val.h"
#include "val_util.h"
#include "xmlns.h"
#include "xml_util.h"
#include "xpath.h"
#include "xpath_yang.h"
#include "xpath1.h"
#include "yangconst.h"


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define MGR_VAL_PARSE_DEBUG 1
#endif


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION mgr_val_parse
* 
* parse a value for a YANG type from a NETCONF PDU XML stream 
*
* Parse NETCONF PDU sub-contents into value fields
* This module does not enforce complex type completeness.
* Different subsets of configuration data are permitted
* in several standard (and any proprietary) RPC methods
*
* A seperate parsing phase is used to validate the input
* contained in the returned val_value_t struct.
*
* This parsing phase checks that simple types are complete
* and child members of complex types are valid (but maybe 
* missing or incomplete child nodes.
*
* INPUTS:
*     scb == session control block
*     obj == obj_template_t for the object type to parse
*     startnode == top node of the parameter to be parsed
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    mgr_val_parse (ses_cb_t  *scb,
               obj_template_t *obj,
               const xml_node_t *startnode,
               val_value_t  *retval)
{
    return val_parse(scb, obj, startnode, retval);
}  /* mgr_val_parse */


/********************************************************************
* FUNCTION mgr_val_parse_reply
*
* parse an <rpc-reply> element 
* parse a value for a YANG type from a NETCONF PDU XML stream 
* Use the RPC object output type to parse any data
*
* Parse NETCONF PDU sub-contents into value fields
* This module does not enforce complex type completeness.
* Different subsets of configuration data are permitted
* in several standard (and any proprietary) RPC methods
*
* A seperate parsing phase is used to validate the input
* contained in the returned val_value_t struct.
*
* This parsing phase checks that simple types are complete
* and child members of complex types are valid (but maybe 
* missing or incomplete child nodes.
*
* INPUTS:
*     scb == session control block
*     obj == obj_template_t for the top-level reply to parse
*     rpc == RPC template to use for any data in the output
*     startnode == top node of the parameter to be parsed
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    mgr_val_parse_reply (ses_cb_t  *scb,
                         obj_template_t *obj,
                         obj_template_t *rpc,
                         const xml_node_t *startnode,
                         val_value_t  *retval)
{
    obj_template_t  *output;
    status_t  res;

#ifdef DEBUG
    if (!scb || !obj || !startnode || !retval) {
        /* non-recoverable error */
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef MGR_VAL_PARSE_DEBUG
    if (LOGDEBUG3) {
        log_debug3("\nmgr_val_parse_reply: %s:%s btyp:%s", 
                   obj_get_mod_prefix(obj),
                   obj_get_name(obj), 
                   tk_get_btype_sym(obj_get_basetype(obj)));
    }
#endif

    output = (rpc) ? obj_find_child(rpc, NULL, NCX_EL_OUTPUT) : NULL;

    /* get the element values */
    res = val_parse_split(scb, 
                            obj, 
                            output, 
                            startnode, 
                            retval);
    
    return res;

}  /* mgr_val_parse_reply */


/********************************************************************
* FUNCTION mgr_val_parse_notification
* 
* parse a <notification> element
* parse a value for a YANG type from a NETCONF PDU XML stream 
* Use the notification object output type to parse any data
*
* This parsing phase checks that simple types are complete
* and child members of complex types are valid (but maybe 
* missing or incomplete child nodes.
*
* INPUTS:
*     scb == session control block
*     notobj == obj_template_t for the top-level notification
*     startnode == top node of the parameter to be parsed
*     retval ==  val_value_t that should get the results of the parsing
*     
* OUTPUTS:
*    *retval will be filled in
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    mgr_val_parse_notification (ses_cb_t  *scb,
                                obj_template_t *notobj,
                                const xml_node_t *startnode,
                                val_value_t  *retval)
{
    status_t  res;

#ifdef DEBUG
    if (!scb || !notobj || !startnode || !retval) {
        /* non-recoverable error */
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef MGR_VAL_PARSE_DEBUG
    if (LOGDEBUG3) {
        log_debug3("\nmgr_val_parse_notification: start");
    }
#endif

    /* get the element values */
    res = val_parse_split(scb, 
                            notobj, 
                            NULL, 
                            startnode, 
                            retval);

    return res;

}  /* mgr_val_parse_notification */


/* END file mgr_val_parse.c */
