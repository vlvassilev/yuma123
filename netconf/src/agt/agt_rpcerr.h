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
#ifndef _H_agt_rpcerr
#define _H_agt_rpcerr
/*  FILE: agt_rpcerr.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol <rpc-error> agent-side handler

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
07-mar-06    abb      Begun.
13-jan-07    abb      moved from rpc dir to agt; rename rpc_agterr
                      to agt_rpcerr
*/

#include <xmlstring.h>

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

/* generate an element (or non-attribute) related error
 * for any layer. Does not use the application layer fields
 */
extern rpc_err_rec_t *
    agt_rpcerr_gen_error (ncx_layer_t layer,
			  status_t   interr,
			  const xml_node_t *errnode,
			  ncx_node_t parmtyp,
			  const void *error_parm,
			  xmlChar *error_path);


extern rpc_err_rec_t *
    agt_rpcerr_gen_error_errinfo (ncx_layer_t layer,
				  status_t   interr,
				  const xml_node_t *errnode,
				  ncx_node_t  parmtyp,
				  const void *error_parm,
				  xmlChar *error_path,
				  const ncx_errinfo_t *errinfo);


extern rpc_err_rec_t *
    agt_rpcerr_gen_error_ex (ncx_layer_t layer,
                             status_t   interr,
                             const xml_node_t *errnode,
                             ncx_node_t  parmtyp,
                             const void *error_parm,
                             xmlChar *error_path,
                             const ncx_errinfo_t *errinfo,
                             ncx_node_t  nodetyp,
                             const void *error_path_raw);

/* generate an attribute related error for any layer. 
 *  Does not use the application layer fields
 */
extern rpc_err_rec_t *
    agt_rpcerr_gen_attr_error (ncx_layer_t layer,
			       status_t   interr,
			       const xml_attr_t *attr,
			       const xml_node_t *errnode,
			       const xmlChar *badns,
			       xmlChar *error_path);

/* generate a YANG missing-instance error */
extern rpc_err_rec_t *
    agt_rpcerr_gen_insert_error (ncx_layer_t layer,
				 status_t   interr,
				 const val_value_t *errval,
				 xmlChar *error_path);


extern rpc_err_rec_t *
    agt_rpcerr_gen_unique_error (xml_msg_hdr_t *msghdr,
				 ncx_layer_t layer,
				 status_t   interr,
				 const dlq_hdr_t *valuniqueQ,
				 xmlChar *error_path);


#endif            /* _H_agt_rpcerr */
