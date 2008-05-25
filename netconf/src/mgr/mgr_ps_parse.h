#ifndef _H_mgr_ps_parse
#define _H_mgr_ps_parse

/*  FILE: mgr_ps_parse.h
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
16-feb-07    abb      Begun; start from agt_ps_parse.h

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

extern status_t 
    mgr_ps_parse_val (ses_cb_t  *scb,
		      xml_msg_hdr_t *msg,
		      const psd_template_t *psd,
		      const xml_node_t *startnode,
		      ps_parmset_t  *ps);

#endif	    /* _H_mgr_ps_parse */
