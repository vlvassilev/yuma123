#ifndef _H_yangcli_cmd
#define _H_yangcli_cmd

/*  FILE: yangcli_cmd.h
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
11-apr-09    abb      Begun; moved from yangcli.c

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

extern void
    top_command (agent_cb_t *agent_cb,
		 xmlChar *line);

extern void
    conn_command (agent_cb_t *agent_cb,
		  xmlChar *line);

extern status_t
    do_startup_script (agent_cb_t *agent_cb);

extern xmlChar *
    get_cmd_line (agent_cb_t *agent_cb,
		  status_t *res);

extern void
    do_connect (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		uint32 start,
		boolean  cli);

#endif	    /* _H_yangcli_cmd */
