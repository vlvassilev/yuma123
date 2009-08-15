#ifndef _H_yangcli_show
#define _H_yangcli_show

/*  FILE: yangcli_show.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
13-augr-09    abb      Begun; move from yangcli_cmd.c

*/

#include <xmlstring.h>

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_yangcli
#include "yangcli.h"
#endif


/********************************************************************
*								    *
*		      F U N C T I O N S 			    *
*								    *
*********************************************************************/

extern status_t
    do_show (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len);


#endif	    /* _H_yangcli_show */
