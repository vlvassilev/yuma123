#ifndef _H_mgr_val_parse
#define _H_mgr_val_parse

/*  FILE: mgr_val_parse.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Parameter Value Parser Module

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
18-feb-07    abb      Begun; start from agt_val_parse.c

*/

#ifndef _H_obj
#include "obj.h"
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

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* parse a value for a YANG object from a NETCONF PDU XML stream */
extern status_t 
    mgr_val_parse (ses_cb_t  *scb,
		   const obj_template_t *obj,
		   const xml_node_t *startnode,
		   val_value_t  *retval);


extern status_t 
    mgr_val_parse_reply (ses_cb_t  *scb,
			 const obj_template_t *obj,
			 const obj_template_t *rpc,
			 const xml_node_t *startnode,
			 val_value_t  *retval);

#endif	    /* _H_mgr_val_parse */
