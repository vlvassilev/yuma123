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
    top_command (server_cb_t *server_cb,
		 xmlChar *line);

extern status_t
    conn_command (server_cb_t *server_cb,
		  xmlChar *line);

extern status_t
    do_startup_script (server_cb_t *server_cb,
                       const xmlChar *runscript);

extern status_t
    do_startup_command (server_cb_t *server_cb,
                        const xmlChar *runcommand);

extern xmlChar *
    get_cmd_line (server_cb_t *server_cb,
		  status_t *res);

extern status_t
    do_connect (server_cb_t *server_cb,
		obj_template_t *rpc,
		const xmlChar *line,
		uint32 start,
                boolean climode);

extern void *
    parse_def (server_cb_t *server_cb,
	       ncx_node_t *dtyp,
	       xmlChar *line,
	       uint32 *len);

extern status_t
    send_keepalive_get (server_cb_t *server_cb);


extern val_value_t *
    get_valset (server_cb_t *server_cb,
		obj_template_t *rpc,
		const xmlChar *line,
		status_t  *res);

#endif	    /* _H_yangcli_cmd */
