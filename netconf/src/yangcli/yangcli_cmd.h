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

#ifndef _H_rpc_err
#include "rpc_err.h"
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
    top_command (agent_cb_t *agent_cb,
		 xmlChar *line);

extern status_t
    conn_command (agent_cb_t *agent_cb,
		  xmlChar *line);

extern status_t
    do_startup_script (agent_cb_t *agent_cb,
                       const xmlChar *runscript);

extern status_t
    do_startup_command (agent_cb_t *agent_cb,
                        const xmlChar *runcommand);

extern xmlChar *
    get_cmd_line (agent_cb_t *agent_cb,
		  status_t *res);

extern status_t
    do_connect (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		uint32 start,
                boolean climode);

extern void *
    parse_def (agent_cb_t *agent_cb,
	       ncx_node_t *dtyp,
	       xmlChar *line,
	       uint32 *len);

extern status_t
    send_keepalive_get (agent_cb_t *agent_cb);

extern status_t
    handle_get_locks_request_to_agent (agent_cb_t *agent_cb,
                                       boolean first,
                                       boolean *done);

extern status_t
    handle_release_locks_request_to_agent (agent_cb_t *agent_cb,
                                           boolean first,
                                           boolean *done);

extern void
    handle_locks_cleanup (agent_cb_t *agent_cb);


extern boolean
    check_locks_timeout (agent_cb_t *agent_cb);

extern status_t
    send_discard_changes_pdu_to_agent (agent_cb_t *agent_cb);

#endif	    /* _H_yangcli_cmd */
