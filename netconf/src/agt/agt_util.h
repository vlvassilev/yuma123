#ifndef _H_agt_util
#define _H_agt_util

/*  FILE: agt_util.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Utility Functions for NCX Agent method routines

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
24-may-06    abb      Begun

*/

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
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
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_get_cfg_from_parm (const xmlChar *parmname,
			   rpc_msg_t *msg,
			   xml_node_t *methnode,
			   cfg_template_t  **retcfg);

extern const val_value_t *
    agt_get_parmval (const xmlChar *parmname,
		     rpc_msg_t *msg);

extern void
    agt_record_error (ses_cb_t *scb,
		      dlq_hdr_t *errQ,
		      ncx_layer_t layer,
		      status_t  res,
		      const xml_node_t *xmlnode,
		      ncx_node_t parmtyp,
		      const void *error_parm,
		      ncx_node_t nodetyp,
		      const void *errnode);

extern void
    agt_record_error_errinfo (ses_cb_t *scb,
			      dlq_hdr_t *errQ,
			      ncx_layer_t layer,
			      status_t  res,
			      const xml_node_t *xmlnode,
			      ncx_node_t parmtyp,
			      const void *error_parm,
			      ncx_node_t nodetyp,
			      const void *errnode,
			      const ncx_errinfo_t *errinfo);

extern void
    agt_record_attr_error (ses_cb_t *scb,
			   dlq_hdr_t *errQ,
			   ncx_layer_t layer,
			   status_t  res,
			   const xml_attr_t *xmlattr,
			   const xml_node_t *xmlnode,
			   const xmlChar *badns,
			   ncx_node_t nodetyp,
			   const void *errnode);


extern status_t 
    agt_validate_filter (ses_cb_t *scb,
			 rpc_msg_t *msg);

/* filter: returns TRUE for NCX_DC_CONFIG or NCX_DC_TCONFIG 
 *  and with-defaults test passes as well
 */
extern boolean
    agt_check_config (boolean withdef,
		      ncx_node_t nodetyp,
		      const void *node);

/* filter: returns TRUE for NCX_DC_CONFIG 
 * and with-defaults test passes as well
 */
extern boolean
    agt_check_save (boolean withdef,
		    ncx_node_t nodetyp,
		    const void *node);

extern status_t
    agt_output_filter (ses_cb_t *scb,
		       rpc_msg_t *msg,
		       int32 indent);

extern status_t
    agt_check_max_access (op_editop_t  op,
			  ncx_access_t acc,
			  boolean cur_exists);

extern status_t
    agt_check_editop (op_editop_t pop,
		      op_editop_t *cop,
		      const val_value_t *newnode,
		      const val_value_t *curnode,
		      ncx_iqual_t  iqual);


#endif	    /* _H_agt_util */
