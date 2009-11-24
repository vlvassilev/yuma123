/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
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

#ifndef _H_yangcli
#include "yangcli.h"
#endif


/********************************************************************
*								    *
*		      F U N C T I O N S 			    *
*								    *
*********************************************************************/

extern boolean
    is_top_command (const xmlChar *rpcname);

extern modptr_t *
    new_modptr (ncx_module_t *mod,
		ncx_list_t *feature_list,
		ncx_list_t *deviation_list);

extern modptr_t *
    find_modptr (dlq_hdr_t *modptrQ,
                 const xmlChar *modname,
                 const xmlChar *revision);

extern void
    free_modptr (modptr_t *modptr);

extern void
    clear_server_cb_session (server_cb_t *server_cb);

extern boolean
    is_top (mgr_io_state_t state);

extern boolean
    use_servercb (server_cb_t *server_cb);

extern ncx_module_t *
    find_module (server_cb_t *server_cb,
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
    clear_result (server_cb_t *server_cb);

extern status_t
    check_filespec (server_cb_t *server_cb,
		    const xmlChar *filespec,
		    const xmlChar *varname);

extern val_value_t *
    get_instanceid_parm (server_cb_t *server_cb,
			 const xmlChar *target,
			 boolean schemainst,
			 obj_template_t **targobj,
			 val_value_t **targval,
			 status_t *retres);

extern boolean
    file_is_text (const xmlChar *filespec);

extern boolean
    interactive_mode (void);

extern void
    init_completion_state (completion_state_t *completion_state,
			   server_cb_t *server_cb,
			   command_state_t  cmdstate);


/* server_cb field must already be set */
extern void
    set_completion_state (completion_state_t *completion_state,
			  obj_template_t *rpc,
			  obj_template_t *parm,
			  command_state_t  cmdstate);

extern void
    set_completion_state_curparm (completion_state_t *completion_state,
				  obj_template_t *parm);

#endif	    /* _H_yangcli_util */
