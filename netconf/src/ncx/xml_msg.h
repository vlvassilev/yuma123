#ifndef _H_xml_msg
#define _H_xml_msg
/*  FILE: xml_msg.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   XML Message send and receive

   Deals with generic namespace and xmlns optimization
   and tries to keep changing the default namespace so
   most nested elements do not have prefixes

   Deals with the NETCONF requirement that the attributes
   in <rpc> are returned in <rpc-reply> unchanged.  Although
   XML allows the xnmlns prefixes to change, the same prefixes
   are used in the <rpc-reply> that the NMS provided in the <rpc>.

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
14-jan-07    abb      Begun; split from agt_rpc.h
*/

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
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                         T Y P E S                                 *
*                                                                   *
*********************************************************************/
				      
/* Common XML Message Header */
typedef struct xml_msg_hdr_t_ {
    /* incoming: 
     * See ??? for details on rpc-reply NS prefix minimization
     * Special hack: any namespace decls that were in the <rpc>
     * request are used in the <rpc-reply>, so the same prefixes
     * will be used, and the XML on the wire will be easier to debug
     */
    xmlns_id_t      defns;       /* req. default namespace ID */
    xmlns_id_t      cur_defns;        /* minimize xmlns decls */
    xmlns_id_t      last_defns;       /* hack for NS handling */
    boolean         withdef;           /* with-defaults value */
    boolean         withmeta;          /* with-metadata value */
    dlq_hdr_t       prefixQ;             /* Q of xmlns_pmap_t */
    dlq_hdr_t       prefix2Q;            /* Q of xmlns_pmap_t */
    dlq_hdr_t       errQ;               /* Q of rpc_err_rec_t */
    xmlChar         last_defpfix[XMLNS_MAX_PREFIX_SIZE+1];
} xml_msg_hdr_t;

				      
/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern void
    xml_msg_init_hdr (xml_msg_hdr_t *msg);

extern void
    xml_msg_clean_hdr (xml_msg_hdr_t *msg);

extern const xmlChar *
    xml_msg_get_prefix (xml_msg_hdr_t *msg,
			xmlns_id_t parent_nsid,
			xmlns_id_t nsid,
			boolean  *xneeded);

extern status_t 
    xml_msg_gen_new_prefix (xml_msg_hdr_t *msg,
			    xmlns_id_t  nsid,
			    xmlChar *retbuff);

extern status_t
    xml_msg_build_prefix_map (xml_msg_hdr_t *msg,
			      xml_attrs_t *attrs,
			      boolean addncid,
			      boolean addncxid);


extern status_t
    xml_msg_check_xmlns_attr (xml_msg_hdr_t *msg, 
			      xmlns_id_t nsid,
			      const xmlChar *badns,
			      xml_attrs_t  *attrs);


extern void
    xml_msg_clean_prefixq (dlq_hdr_t *prefixQ);

#endif            /* _H_xml_msg */
