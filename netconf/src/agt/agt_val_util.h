#ifndef _H_agt_val_util
#define _H_agt_val_util

/*  FILE: agt_val_util.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Agent Value Struct Utilities

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
11-feb-06    abb      Begun
06-aug-08    abb      Rewrite for OBJ/VAL only model in YANG
                      Remove app, parmset, and parm from database model
08aug08      abb      Clone val_util functions for agent error reporting

*/

#ifndef _H_ncxtypes
#include "ncxtypes.h.h"
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

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_val_instance_check (ses_cb_t *scb,
			    xml_msg_hdr_t *msg,
			    val_value_t *valset,
			    ncx_layer_t   layer);


#endif	    /* _H_agt_val_util */
