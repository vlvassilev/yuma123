#ifndef _H_agt_ps_parse
#define _H_agt_ps_parse

/*  FILE: agt_ps_parse.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    parameter set XML parser module

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
21-oct-05    abb      Begun
09-feb-06    abb      Change from xmlTextReader to rpc_agt callback
                      API format
*/

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
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

#ifndef _H_xml_msg
#include "xml_msg.h"
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


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* parse a parmset from a NETCONF PDU XML stream */
extern status_t 
    agt_ps_parse_rpc (ses_cb_t  *scb,
		      rpc_msg_t *msg,
		      const psd_template_t *psd,
		      const xml_node_t *startnode,
		      ps_parmset_t  *ps);

extern status_t 
    agt_ps_parse_val (ses_cb_t  *scb,
		      xml_msg_hdr_t *msg,
		      const psd_template_t *psd,
		      const xml_node_t *startnode,
		      ps_parmset_t  *ps);

extern status_t 
    agt_ps_parse_instance_check (ses_cb_t *scb,
				 xml_msg_hdr_t *msg,
				 const ps_parmset_t *ps,
				 ncx_layer_t     layer);

extern status_t 
    agt_ps_parse_choice_check (ses_cb_t *scb,
			       xml_msg_hdr_t *msg,
			       const ps_parmset_t *ps,
			       ncx_layer_t    layer);


extern status_t 
    agt_ps_parse_error_subtree (ses_cb_t *scb,
				xml_msg_hdr_t *msg,
				const xml_node_t *startnode,
				const xml_node_t *errnode,
				status_t errcode,
				ncx_node_t errnodetyp,
				const void *error_parm,
				ncx_node_t intnodetyp,
				const void *intnode);


extern void
    agt_ps_parse_error_attr (ses_cb_t *scb,
			     xml_msg_hdr_t *msg,
			     const xml_attr_t *errattr,
			     const xml_node_t *errnode,
			     status_t errcode,
			     ncx_node_t nodetyp,
			     const void *intnode);


#ifdef DEBUG
extern void
    agt_ps_parse_test (const char *testfile);
#endif

#endif	    /* _H_agt_ps_parse */
