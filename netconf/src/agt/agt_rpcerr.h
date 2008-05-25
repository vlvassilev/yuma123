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

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

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


#ifdef NOT_YET
/* generate an NCX_LAYER_CONTENT error */
extern rpc_err_rec_t *
    agt_rpcerr_gen_app_error (
			  status_t   interr,
			  const xml_node_t *errnode,
			  const xmlChar *apptag,
			  xmlChar *error_msg,
			  const xmlChar *error_lang,
			  void *error_parm);
#endif


#endif            /* _H_agt_rpcerr */
