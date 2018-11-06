/*
 * Copyright (c) 2018 Vladimir Vassilev, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <assert.h>

#include "procdefs.h"
#include "agt.h"
#include "agt_cli.h"
#include "agt_nmda.h"
#include "agt_rpc.h"
#include "agt_util.h"
#include "cfg.h"
#include "cli.h"
#include "getcb.h"
#include "log.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "ncx_feature.h"
#include "ncx_list.h"
#include "rpc.h"
#include "rpc_err.h"
#include "ses.h"
#include "ses_msg.h"
#include "status.h"
#include "tstamp.h"
#include "val.h"
#include "val_set_cplxval_obj.h"
#include "val123.h"
#include "val_util.h"
#include "xmlns.h"
#include "xml_util.h"
#include "xml_val.h"
#include "xml_wr.h"
#include "yangconst.h"

obj_template_t* find_rpc_template(char* rpc_name)
{
    ncx_module_t * mod;
    obj_template_t* rpc;

    /* add all modules */
    for (mod = ncx_get_first_module();
         mod != NULL;
         mod = ncx_get_next_module(mod)) {
        if(!mod->implemented) {
            continue;
        }
        rpc = ncx_find_object(mod, rpc_name);
	if(rpc!=NULL && rpc->objtype==OBJ_TYP_RPC) {
            break;
        }
    }
    return rpc;
}

/********************************************************************
* FUNCTION yangcli_to_rpc
*
* INPUTS:
*    see agt/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t
    yangcli_to_rpc(ses_cb_t *scb,
                  rpc_msg_t *msg,
                  xml_node_t *methnode)
{
    val_value_t        *cmd_val;
    obj_template_t     *output_obj;
    obj_template_t     *output_rpc_obj;
    val_value_t        *output_rpc_val;
    val_value_t        *reqdata;
    val_value_t        *valset;
    val_value_t        *chval;
    obj_template_t     *rpc_obj;
    obj_template_t     *input_obj;
    val_value_t        *rpc_val;
    char*              first_space;
    char*              rpc_name_str;
    unsigned int       rpc_name_len;
    status_t            res;
    char* argv[2];

    cmd_val = val_find_child(msg->rpc_input,
                             "yuma123-yangcli-to-rpc",
                             "cmd");
    assert(cmd_val);
    printf("yangcli-to-rpc: %s\n",VAL_STRING(cmd_val));

    output_obj = obj_find_child(
        msg->rpc_input->obj->parent,
        "yuma123-yangcli-to-rpc",
        "output");
    assert(output_obj);

    output_rpc_obj = obj_find_child(
        output_obj,
        "yuma123-yangcli-to-rpc",
        "rpc");
    assert(output_rpc_obj);

    output_rpc_val = val_new_value();
    assert(output_rpc_val);

    val_init_from_template(output_rpc_val, output_rpc_obj);

#if 0
    res = val_set_cplxval_obj(output_rpc_val,
                              output_rpc_val->obj,
                              "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\"><get> <filter type=\"xpath\" select=\"/system\"/></get></rpc>");
#else

    first_space=strchr(VAL_STRING(cmd_val), ' ');
    if(first_space==NULL || first_space==VAL_STRING(cmd_val)) {
        res = ERR_NCX_INVALID_VALUE;
        return res;
    }

    rpc_name_len = first_space - (char*)VAL_STRING(cmd_val);
    rpc_name_str=malloc(rpc_name_len+1);
    memcpy(rpc_name_str, VAL_STRING(cmd_val), rpc_name_len);
    rpc_name_str[rpc_name_len]=0;
    argv[0]=rpc_name_str;
    argv[1]=first_space+1;

    rpc_obj=find_rpc_template(argv[0]);
    if(rpc_obj==NULL) {
        res = ERR_NCX_INVALID_VALUE;
        free(argv[0]);
        return res;
    }

    input_obj = obj_find_child(rpc_obj, NULL, YANG_K_INPUT);
    assert(input_obj);

    valset = cli_parse (NULL,
               2 /*argc*/,
               argv /*argv*/,
               input_obj,
               FULLTEST,
               TRUE/*script*/,
               TRUE,
               CLI_MODE_PROGRAM,
               &res);
    free(rpc_name_str);
    if(res!=NO_ERR) {
        return res;
    }

    val_dump_value(valset,1);

    rpc_val = xml_val_new_struct(obj_get_name(rpc_obj), obj_get_nsid(rpc_obj));
    if(rpc_val==NULL) {
        res = ERR_NCX_INVALID_VALUE;
        free(rpc_name_str);
        return res;
    }

    for(chval=val_get_first_child(valset);
        chval!=NULL;
        chval=val_get_next_child(chval)) {
        val_value_t        *newval;
        newval = val_clone(chval);
	val_add_child(newval, rpc_val);
    }
    val_free_value(valset);
    val_add_child(rpc_val, output_rpc_val);
#endif

    dlq_enque(output_rpc_val, &msg->rpc_dataQ);
    msg->rpc_data_type = RPC_DATA_YANG;

    return NO_ERR;

} /* yangcli_to_rpc */

/********************************************************************
* FUNCTION y_yangcli_to_rpc_init
*
* INIT 1:
*   Initialize the module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    y_yangcli_to_rpc_init (void)
{
    agt_profile_t  *agt_profile;
    status_t        res;
    ncx_module_t    *mod;
    obj_template_t  *root_obj;
    val_value_t*    clivalset;
    val_value_t*    val;

    /* load in the RPC methods */
    res = ncxmod_load_module( "yuma123-yangcli-to-rpc", NULL, NULL, NULL );
    assert(res == NO_ERR);

    /* yangcli-to-rpc */
    res = agt_rpc_register_method("yuma123-yangcli-to-rpc",
                                  "yangcli-to-rpc",
                                  AGT_RPC_PH_INVOKE,
                                  yangcli_to_rpc);
    assert(res == NO_ERR);

    return NO_ERR;

}  /* y_yangcli_to_rpc_init */


/********************************************************************
* FUNCTION y_yangcli_to_rpc_init2
*
* INIT 2:
*   Initialize the data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    y_yangcli_to_rpc_init2 (void)
{

    return NO_ERR;

}  /* y_yangcli_to_rpc_init2 */


/********************************************************************
* FUNCTION y_yangcli_to_rpc_cleanup
*
* Cleanup the module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    y_yangcli_to_rpc_cleanup (void)
{
    agt_rpc_unregister_method("yuma123-yangcli-to-rpc",
                              "yangcli-to-rpc");

}  /* y_yangcli_to_rpc_cleanup */
