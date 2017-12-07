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
#include "agt_nmda.h"
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

/* 'or:intended' has always higher priority then 'or:system' */
static void operational_resolve_origin_meta(val_value_t* operational_val, val_value_t* running_val, val_value_t* system_val, boolean is_parent_origin_intended)
{
    val_value_t* chval;
    val_value_t* running_child_val;
    val_value_t* system_child_val;

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
        running_child_val = val123_find_match(running_val, chval);
        system_child_val = val123_find_match(system_val, chval);

        assert(running_child_val || system_child_val);

        if(typ_is_simple(chval->btyp)) {
            if(is_parent_origin_intended) {
                if(running_child_val) {
                    continue;
                } else {
                    add_origin(chval,"or:system");
                }
            } else {
                if(running_child_val) {
                    add_origin(chval,"or:intended");
                } else {
                    add_origin(chval,"or:system");
                }
            }
        } else {
            if(obj_is_np_container(chval->obj)) {
                operational_resolve_origin_meta(chval, running_child_val, system_child_val, is_parent_origin_intended);

            } else {
                if(running_child_val) {
                    add_origin(chval,"or:intended");
                    operational_resolve_origin_meta(chval, running_child_val, system_child_val, TRUE);
                } else {
                    add_origin(chval,"or:system");
                }
            }
        }
    }
    return NO_ERR;
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
    cfg_template_t        *runningcfg;

    /* get the running config */
    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert (runningcfg && runningcfg->root);

    running_real_clone_val = val123_clone_real(runningcfg->root);
    assert(running_real_clone_val);
    val123_merge_cplx(dst_val, running_real_clone_val);
    system_real_clone_val = val123_clone_real(root_system_val);
    assert(system_real_clone_val);
    val123_merge_cplx(dst_val, system_real_clone_val);

    operational_resolve_origin_meta(dst_val, running_real_clone_val, system_real_clone_val, FALSE);

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

    agt_profile = agt_get_profile();

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
}  /* agt_nmda_cleanup */
