/*
 * Copyright (c) 2017 Vladimir Vassilev, All Rights Reserved.
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
#include <openssl/sha.h>

#include "procdefs.h"
#include "agt.h"
#include "agt_cli.h"
#include "agt_nmda.h"
#include "agt_rpc.h"
#include "agt_util.h"
#include "cfg.h"
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
#include "val123.h"
#include "val_util.h"
#include "xmlns.h"
#include "xml_util.h"
#include "xml_wr.h"
#include "yangconst.h"

/* merge running(intended), system and learned origins */

static val_value_t* root_operational_val=NULL;
static val_value_t* root_system_val=NULL;
static val_value_t* root_learned_val=NULL;
ncx_module_t* ietf_origin_mod;

#if 0
/* Work in progress: more efficient solution then the proof-of-concept hack below */

static status_t
    operational_get_callback(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val);

static void operational_merge_cplx(val_value_t* dst, val_value_t* src, char* origin_identity)
{
    val_value_t* chval;
    val_value_t* match_val;
    for (chval = val_get_first_child(src);
         chval != NULL;
         chval = val_get_next_child(chval)) {

#if 0
        if(obj_is_key(chval->obj)) {
            continue;
        }
#endif
        match_val = val123_find_match(dst, chval);
        if(match_val==NULL) {
            val_value_t* new_val;
            assert(!obj_is_key(chval->obj));
            new_val=val_new_value();
            assert(new_val);
            val_init_virtual(new_val,
                     operational_get_callback,
                     chval->obj);
            /*add keys*/
            TODO -add keys
            val_add_child(new_val,dst);
        } else {
            continue;
        }
    }
    return NO_ERR;
}

static status_t
    operational_get_callback(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res = NO_ERR;
    ncx_module_t* ietf_origin_mod;
    val_value_t* interfaces_val;
    val_value_t* interface_val;
    val_value_t* origin_val;


    /* Add all children from running,system and learned with keys as real values non-keys as virtual */
    running_val = val123_find_match(root_val, vir_val);
    system_val = val123_find_match(root_system_val, vir_val);
    learned_val = val123_find_match(root_learned_val, vir_val);


    cur_origin=agt_nmda_operational_check_origin(dst_val);
    if(running_val) {
        val_value_t* running_cloned_real_val;
        running_cloned_real_val = val123_clone_real(running_val);
        assert(running_cloned_real_val);
        operational_merge_cplx(dst_val, running_cloned_real_val, "or:intended");
    }
    if(system_val) {
        operational_merge_cplx(dst_val, system_val, "or:system");
    }
    if(learned_val) {
        operational_merge_cplx(dst_val, learned_val, "or:learned");
    }

    assert(res == NO_ERR);
    return res;
}

#endif

static void add_origin(val_value_t* val, char* origin_id)
{
    val_value_t* origin_val;
    origin_val = val_make_string(ietf_origin_mod->nsid, "origin", origin_id /*e.g. "or:intended"*/);
    val_add_meta(origin_val, val);
}

/* the origin meta for configuration store value b is added to the nodes existing only in b to operational */
static void operational_resolve_origin_meta(val_value_t* operational_val, val_value_t* a_val, val_value_t* b_val, char* origin_b_str)
{
    val_value_t* chval;
    val_value_t* child_a_val;
    val_value_t* child_b_val;

    assert(obj_is_config(operational_val->obj));

    for (chval = val_get_first_child(operational_val);
         chval != NULL;
         chval = val_get_next_child(chval)) {

        if(!obj_get_config_flag(chval->obj)) {
            /* config false data has no config origin */
            continue;
        }
#if 0
        if(obj_is_key(chval->obj)) {
            continue;
        }
#endif
        child_b_val = val123_find_match(b_val, chval);
        if(child_b_val==NULL) {
            continue;
        }
        child_a_val = val123_find_match(a_val, chval);


        if(child_a_val==NULL && !obj_is_np_container(chval->obj)) {
                add_origin(chval,origin_b_str);
                continue;
        }
        if(!typ_is_simple(chval->btyp)) {
            operational_resolve_origin_meta(chval, child_a_val, child_b_val, origin_b_str);
        }
    }
    return;
}


/* Initial proof-of-concept implementation with complete merge of the intended and system trees instead of recursive tree resolution */
static status_t
    operational_get_callback(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res = NO_ERR;
    val_value_t* running_real_clone_val;
    val_value_t* system_real_clone_val;
    val_value_t* learned_real_clone_val;
    val_value_t* oper_w_running_clone_val;
    val_value_t* oper_w_system_clone_val;
    cfg_template_t        *runningcfg;


    /* get the running config */
    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert (runningcfg && runningcfg->root);

    running_real_clone_val = val123_clone_real(runningcfg->root);
    assert(running_real_clone_val);
    val123_merge_cplx(dst_val, running_real_clone_val);

    oper_w_running_clone_val = val_clone(dst_val);
    assert(oper_w_running_clone_val);

    system_real_clone_val = val123_clone_real(root_system_val);
    assert(system_real_clone_val);
    val123_merge_cplx(dst_val, system_real_clone_val);

    oper_w_system_clone_val = val_clone(dst_val);
    assert(oper_w_system_clone_val);

    learned_real_clone_val = val123_clone_real(root_learned_val);
    assert(learned_real_clone_val);
    val123_merge_cplx(dst_val, learned_real_clone_val);

    operational_resolve_origin_meta(dst_val, NULL, running_real_clone_val, "or:intended");
    operational_resolve_origin_meta(dst_val, oper_w_running_clone_val, system_real_clone_val, "or:system");
    operational_resolve_origin_meta(dst_val, oper_w_system_clone_val, learned_real_clone_val, "or:learned");

    val_free_value(oper_w_running_clone_val);
    val_free_value(oper_w_system_clone_val);

    assert(res == NO_ERR);
    return res;
}

/************* E X T E R N A L    F U N C T I O N S ***************/
val_value_t* agt_nmda_get_root_operational(void)
{
    return root_operational_val;
}
val_value_t* agt_nmda_get_root_system(void)
{
    return root_system_val;
}
val_value_t* agt_nmda_get_root_learned(void)
{
    return root_learned_val;
}

/********************************************************************
* FUNCTION get_data_validate
*
* get_data : validate params callback
*
* INPUTS:
*    see agt/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_data_validate(ses_cb_t *scb,
                  rpc_msg_t *msg,
                  xml_node_t *methnode)
{
    cfg_template_t     *source;
    val_value_t        *testval;
    status_t            res, res2;
    boolean             empty_callback = FALSE;
    static cfg_template_t *operational_cfg=NULL;


    if(operational_cfg==NULL) {
        operational_cfg = cfg_new_template("operational", 1000 /*dummy cfg_id*/);
        assert(operational_cfg);
        operational_cfg->root = agt_nmda_get_root_operational();

    }
    /* TODO set to the configuration datastore selected with datastore if not operational*/
    source = operational_cfg;

#if 0
    /* check if the <running> config is ready to read */
    source = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!source) {
        res = ERR_NCX_OPERATION_FAILED;
    } else {
        res = cfg_ok_to_read(source);
    }
    if (res != NO_ERR) {
        agt_record_error(scb,
                         &msg->mhdr,
                         NCX_LAYER_OPERATION,
                         res,
                         methnode,
                         NCX_NT_NONE,
                         NULL,
                         NCX_NT_NONE,
                         NULL);
        return res;
    }
#endif

    /* check if the optional filter parameter is ok */
    res = agt_validate_filter(scb, msg);

    /* check the with-defaults parameter */
    res2 = agt_set_with_defaults(scb, msg, methnode);

    if (res != NO_ERR) {
        return res;   /* error already recorded */
    }

    if (res2 != NO_ERR) {
        return res2;   /* error already recorded */
    }

#if 0
    testval = val_find_child(msg->rpc_input,
                             y_yuma_time_filter_M_yuma_time_filter,
                             IF_MODIFIED_SINCE);
    if (testval != NULL && testval->res == NO_ERR) {
        boolean isneg = FALSE;
        xmlChar *utcstr;
        int ret;
        utcstr = tstamp_convert_to_utctime(VAL_STR(testval), &isneg, &res);
        if (res != NO_ERR || isneg) {
            if (utcstr) {
                m__free(utcstr);
            }
            if (isneg) {
                res = ERR_NCX_INVALID_VALUE;
            }

            agt_record_error(scb,
                             &msg->mhdr,
                             NCX_LAYER_OPERATION,
                             res,
                             methnode,
                             NCX_NT_VAL,
                             testval,
                             NCX_NT_VAL,
                             testval);
            return res;
        }
        ret = xml_strcmp(source->last_ch_time, utcstr);
        if (ret <= 0) {
            empty_callback = TRUE;
        }
        m__free(utcstr);
    }
#endif
    /* cache the 2 parameters and the data output callback function
     * There is no invoke function -- it is handled automatically
     * by the agt_rpc module
     */
    msg->rpc_user1 = source;
    msg->rpc_data_type = RPC_DATA_STD;
    if (empty_callback) {
        msg->rpc_datacb = agt_output_empty;
    } else {
        msg->rpc_datacb = agt_output_filter;
    }

    return NO_ERR;

} /* get_data_validate */

/********************************************************************
* FUNCTION agt_nmda_init
*
* INIT 1:
*   Initialize the NMDA module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_nmda_init (void)
{
    agt_profile_t  *agt_profile;
    status_t        res;
    ncx_module_t    *mod;
    obj_template_t  *root_obj;
    val_value_t*    clivalset;
    val_value_t*    val;

    clivalset = agt_cli_get_valset();
    val = val_find_child(clivalset, NCXMOD_NETCONFD_EX, NCX_EL_WITH_NMDA);
    if(!VAL_BOOL(val)) {
      return NO_ERR;
    }

    agt_profile = agt_get_profile();

    /* load in the NMDA NETCONF data types and RPC methods */
    res = ncxmod_load_module( "ietf-netconf-datastores", NULL, NULL, NULL );
    assert(res == NO_ERR);

    /* get-data */
    res = agt_rpc_register_method("ietf-netconf-datastores"/*NC_MODULE*/,
                                  "get-data"/*op_method_name(OP_GET)*/,
                                  AGT_RPC_PH_VALIDATE,
                                  get_data_validate);
    assert(res == NO_ERR);

    /* load the module */
    res = ncxmod_load_module("ietf-origin", 
                             NULL, 
                             &agt_profile->agt_savedevQ,
                             &ietf_origin_mod);
    assert(res == NO_ERR);

    mod = ncx_find_module(NCXMOD_NETCONF, NULL);
    assert(mod);
    root_obj = ncx_find_object(mod, NCX_EL_CONFIG);
    assert(root_obj);

    /*init root_operational_val */
    root_operational_val = val_new_value();
    assert(root_operational_val);
    val_init_virtual(root_operational_val,
                     operational_get_callback,
                     root_obj);

    /*init root_system_val */
    root_system_val = val_new_value();
    assert(root_system_val);
    val_init_from_template(root_system_val,root_obj);

    /*init root_learned_val */
    root_learned_val = val_new_value();
    assert(root_learned_val);
    val_init_from_template(root_learned_val,root_obj);

    return NO_ERR;

}  /* agt_nmda_init */


/********************************************************************
* FUNCTION agt_nmda_init2
*
* INIT 2:
*   Initialize the yang library data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_nmda_init2 (void)
{
    val_value_t*    clivalset;
    val_value_t*    val;

    clivalset = agt_cli_get_valset();
    val = val_find_child(clivalset, NCXMOD_NETCONFD_EX, NCX_EL_WITH_NMDA);
    if(!VAL_BOOL(val)) {
      return NO_ERR;
    }

    return NO_ERR;

}  /* agt_nmda_init2 */


/********************************************************************
* FUNCTION agt_nmda_cleanup
*
* Cleanup the module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    agt_nmda_cleanup (void)
{
    val_value_t*    clivalset;
    val_value_t*    val;

    clivalset = agt_cli_get_valset();
    val = val_find_child(clivalset, NCXMOD_NETCONFD_EX, NCX_EL_WITH_NMDA);
    if(!VAL_BOOL(val)) {
      return NO_ERR;
    }

}  /* agt_nmda_cleanup */
