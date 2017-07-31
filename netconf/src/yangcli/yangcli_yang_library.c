/*
 * Copyright (c) 2017, VLadimir Vassilev, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: yangcli_yang_library.c

   NETCONF YANG-based CLI Tool

   yang library support

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libssh2.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "libtecla.h"

#include "procdefs.h"
#include "log.h"
#include "mgr.h"
#include "mgr_ses.h"
#include "ncx.h"
#include "ncx_feature.h"
#include "ncx_list.h"
#include "ncxconst.h"
#include "ncxmod.h"
#include "obj.h"
#include "op.h"
#include "rpc.h"
#include "rpc_err.h"
#include "status.h"
#include "val_util.h"
#include "var.h"
#include "xmlns.h"
#include "xml_util.h"
#include "xml_val.h"
#include "yangconst.h"
#include "yangcli.h"
#include "yangcli_autoload.h"
#include "yangcli_cmd.h"
#include "yangcli_util.h"


/********************************************************************
* FUNCTION make_get_yang_library_modules_state_reqdata
* 
* Allocate and initialize reqdata value for <get> /modules-state
*
* INPUTS:
*   server_cb == server control block to use
*   scb == session control block to use
*
* OUTPUTS:
*    out_rpc == obj_template_t** of the get-schema RPC
*    out_reqdata == val_value_t** of the get-schema data value
*
* RETURNS:
*    status
*********************************************************************/
status_t make_get_yang_library_modules_state_reqdata(server_cb_t *server_cb,
                              ses_cb_t *scb,
                              obj_template_t** out_rpc,
                              val_value_t** out_reqdata)
{
    ncx_module_t          *ietf_netconf_mod;
    ncx_module_t          *ietf_yang_library_mod;
    obj_template_t        *rpc_obj, *input_obj, *filter_obj, *modules_state_obj;
    val_value_t           *request_val, *filter_val, *modules_state_val;
    val_value_t           *type_meta_val;
    status_t               res;
    xmlns_id_t             nsid;

    res = NO_ERR;

    res = ncxmod_load_module ("ietf-netconf", NULL, NULL, &ietf_netconf_mod);
    assert(res==NO_ERR);

    rpc_obj = ncx_find_rpc(ietf_netconf_mod, "get");
    assert(obj_is_rpc(rpc_obj));
    input_obj = obj_find_child(rpc_obj, NULL, "input");
    assert(input_obj!=NULL);
    filter_obj = obj_find_child(input_obj, NULL, "filter");
    assert(filter_obj!=NULL);


    res = ncxmod_load_module ("ietf-yang-library", NULL, NULL, &ietf_yang_library_mod);
    assert(res==NO_ERR);

    modules_state_obj = ncx_find_object(ietf_yang_library_mod, "modules-state");
    assert(modules_state_obj);

    request_val = val_new_value();
    val_init_from_template(request_val, rpc_obj);
    filter_val = val_new_value();
    val_init_from_template(filter_val, filter_obj);

    modules_state_val = val_new_value();
    val_init_from_template(modules_state_val, modules_state_obj);

    type_meta_val = val_make_string(0, "type","subtree");

    val_add_meta(type_meta_val, filter_val);
    val_add_child(filter_val, request_val);
    val_add_child(modules_state_val, filter_val);

    input_obj = obj_find_child(rpc_obj, NULL, YANG_K_INPUT);
    assert(input_obj);


    *out_rpc=rpc_obj;
    *out_reqdata=request_val;

    return res;

} /* make_get_yang_library_modules_state_reqdata */

/********************************************************************
* FUNCTION send_get_yang_library_modules_state_to_server
* 
* Send an <get> /modules-state operation to the specified server
* in MGR_IO_ST_AUTOLOAD state
*
* INPUTS:
*   server_cb == server control block to use
*   scb == session control block to use
*
* OUTPUTS:
*    server_cb->state may be changed or other action taken
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_get_yang_library_modules_state_to_server (server_cb_t *server_cb,
                              ses_cb_t *scb)
{
    status_t              res;
    obj_template_t*       rpc;
    val_value_t*          reqdata;
    mgr_rpc_req_t         *req;

    req = NULL;

    res = make_get_yang_library_modules_state_reqdata(server_cb, scb, &rpc, &reqdata);
    if(res!=NO_ERR) {
        return NO_ERR;
    }

    /* allocate an RPC request and send it */
    req = mgr_rpc_new_request(scb);
    if (!req) {
        res = ERR_INTERNAL_MEM;
        log_error("\nError allocating a new RPC request");
    } else {
        req->data = reqdata;
        req->rpc = rpc;
        req->timeout = server_cb->timeout;
    }
        
    if (res == NO_ERR) {
        if (LOGDEBUG) {
            log_debug("\nSending yang-library /modules-state <get> autoload request.");
        } 
        if (LOGDEBUG2) {
            log_debug2("\nabout to send RPC request with reqdata:");
            val_dump_value_max(reqdata, 
                               0,
                               server_cb->defindent,
                               DUMP_VAL_LOG,
                               server_cb->display_mode,
                               FALSE,
                               FALSE);
        }

        /* the request will be stored if this returns NO_ERR */
        res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
    }

    if (res != NO_ERR) {
        if (req) {
            mgr_rpc_free_request(req);
        } else if (reqdata) {
            val_free_value(reqdata);
        }
    } else {
        server_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_get_yang_library_modules_state_to_server */

status_t get_yang_library_modules_state_reply_to_searchresult_entries(server_cb_t * server_cb, ses_cb_t *scb, val_value_t* reply)
{
    ncxmod_search_result_t  *searchresult;
    val_value_t             *data_val;
    val_value_t             *modules_state_val;
    val_value_t             *module_val;
    mgr_scb_t               *mscb;
    status_t                res = NO_ERR;

    mscb = (mgr_scb_t *)scb->mgrcb;

    data_val = val_find_child(reply, NULL, NCX_EL_DATA);
    if (data_val == NULL) {
        res = SET_ERROR(ERR_NCX_DATA_MISSING);
    }
    modules_state_val = val_find_child(data_val, "ietf-yang-library", "modules-state");
    if (modules_state_val == NULL) {
        res = SET_ERROR(ERR_NCX_DATA_MISSING);
    }

#if 0
    for(module_val = val_find_child(modules_state_val, "ietf-yang-library", "module");
        module_val != NULL;
        module_val = val_find_next_child(modules_state_val, "ietf-yang-library", "module", module_val)) {
        val_dump_value(module_val,1);
    }
#endif

    mscb->modules_state_val = val_clone(modules_state_val);

    return res;
}

/**************    E X T E R N A L   F U N C T I O N S **********/

/********************************************************************
* FUNCTION yang_library_start_get_module_set
* 
* Start the MGR_SES_IO_CONN_YANG_LIBRARY state
*
* Seng <get> for /modules-state yang library container.
*
* INPUTS:
*   server_cb == server session control block to use
*   scb == session control block to use
*
* OUTPUTS:
*   
* RETURNS:
*    status
*********************************************************************/
status_t
    yang_library_start_get_module_set (server_cb_t *server_cb,
                                ses_cb_t *scb)
{
    ncxmod_search_result_t  *searchresult;
    status_t                 res;
    boolean                  done;

#ifdef DEBUG
    if (!server_cb || !scb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    done = FALSE;
    res = NO_ERR;

    res = send_get_yang_library_modules_state_to_server(server_cb,
                                       scb);
    if (res == NO_ERR) {
        server_cb->command_mode = CMD_MODE_YANG_LIBRARY;
    }

    return res;

}  /* yang_library_start_get_module_set */


/********************************************************************
* FUNCTION yang_library_handle_rpc_reply
* 
* Handle the current <get-schema> response
*
* INPUTS:
*   server_cb == server session control block to use
*   scb == session control block to use
*   reply == data node from the <rpc-reply> PDU
*   anyerrors == TRUE if <rpc-error> detected instead
*                of <data>
*             == FALSE if no <rpc-error> elements detected
*
* OUTPUTS:
*    yang library module set retrieval process is completed
*   
* RETURNS:
*    status
*********************************************************************/
status_t
    yang_library_handle_rpc_reply (server_cb_t *server_cb,
                               ses_cb_t *scb,
                               val_value_t *reply,
                               boolean anyerrors)
{
    mgr_scb_t               *mscb;
    ncxmod_search_result_t  *searchresult;
    const xmlChar           *module, *revision;
    status_t                 res;
    boolean                  done;

    res = get_yang_library_modules_state_reply_to_searchresult_entries(server_cb, scb, reply);
    return res;    

}  /* yang_library_handle_rpc_reply */

/* END yangcli_yang_library.c */
