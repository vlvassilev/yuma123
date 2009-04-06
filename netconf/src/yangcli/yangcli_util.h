#ifndef _H_yangcli_util
#define _H_yangcli_util

/*  FILE: yangcli_util.h
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
27-mar-09    abb      Begun; moved from yangcli.c

*/


#include <xmlstring.h>

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif


/********************************************************************
*								    *
*		      F U N C T I O N S 			    *
*								    *
*********************************************************************/

extern boolean
    is_top_command (const xmlChar *rpcname);

extern modptr_t *
    new_modptr (ncx_module_t *mod);

extern void
    free_modptr (modptr_t *modptr);

extern void
    clear_agent_cb_session (agent_cb_t *agent_cb);

extern boolean
    is_top (mgr_io_state_t state);

extern boolean
    use_agentcb (agent_cb_t *agent_cb);

extern ncx_module_t *
    find_module (agent_cb_t *agent_cb,
		 const xmlChar *modname);

extern xmlChar *
    get_strparm (val_value_t *valset,
		 const xmlChar *modname,
		 const xmlChar *parmname);

extern val_value_t *
    findparm (val_value_t *valset,
	      const xmlChar *modname,
	      const xmlChar *parmname);

extern status_t
    add_clone_parm (const val_value_t *val,
		    val_value_t *valset);

extern boolean
    is_yangcli_ns (xmlns_id_t ns);

extern void
    clear_result (agent_cb_t *agent_cb);

extern status_t
    check_filespec (agent_cb_t *agent_cb,
		    const xmlChar *filespec,
		    const xmlChar *varname);

extern val_value_t *
    get_instanceid_parm (agent_cb_t *agent_cb,
			 const xmlChar *target,
			 const obj_template_t **targobj,
			 val_value_t **targval,
			 status_t *retres);

extern boolean
    file_is_text (const xmlChar *filespec);

#endif	    /* _H_yangcli_util */
