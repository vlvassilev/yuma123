/*
    module ietf-network-bridge
    namespace urn:ietf:params:xml:ns:yang:ietf-network-bridge
    implementation of common functions e.g. deriving out-discards form all scheduler discard counters
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


#include <libxml/xmlstring.h>
#include "procdefs.h"
#include "agt.h"
#include "agt_cb.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "agt_not.h"
#include "agt_rpc.h"
#include "dlq.h"
#include "ncx.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "status.h"
#include "rpc.h"
#include "val123.h"

#define SCHED_STATE_MOD "ietf-network-bridge-scheduler-state"

/* module static variables */
static ncx_module_t *ietf_interfaces_mod;
static ncx_module_t *iana_if_type_mod;
static ncx_module_t *ietf_network_bridge_mod;
static ncx_module_t *ietf_network_bridge_scheduler_mod;
static ncx_module_t *ietf_network_bridge_flows_mod;
static ncx_module_t *example_bridge_mod;
static obj_template_t* bridge_obj;


/*
 * This implementation independent function sums all discard counter instances with schema id
 * /interfaces-state/interface/scheduler/gate-controllers/gate-controller/inputs/input/discards
 * and adds the sum as /interfaces-state/interface/statistics/out-discards value
 */
status_t
    get_if_out_discards_cbfn(ses_cb_t *scb,
                     getcb_mode_t cbmode,
                     val_value_t *vir_val,
                     val_value_t  *dst_val)

{
    status_t res;
    uint64_t discards_sum=0;
    val_value_t* scheduler_val;
    val_value_t* gate_controllers_val;
    val_value_t* gate_controller_val;
    obj_template_t* out_discards_obj;
    val_value_t* out_discards_val;

    out_discards_obj = obj_find_child(vir_val->obj,
                      "ietf-interfaces",
                      "out-discards");
    assert(out_discards_obj);

    scheduler_val = val_find_child(vir_val->parent,
                                   SCHED_STATE_MOD,
                                   "scheduler");
    if(scheduler_val==NULL) {
        return ERR_NCX_SKIPPED;
    }

    if (val_is_virtual(scheduler_val)) {
        scheduler_val = val_get_virtual_value(NULL, scheduler_val, &res);
        if(res==ERR_NCX_SKIPPED) {
            return res;
        }
        assert(res==NO_ERR);
        assert(scheduler_val);
    }

    /* gate-controllers */
    gate_controllers_val = val_find_child(scheduler_val,
                                   SCHED_STATE_MOD,
                                   "gate-controllers");
    assert(gate_controllers_val);

    /* gate-controller */
    for(gate_controller_val = val_get_first_child(gate_controllers_val);
        gate_controller_val != NULL;
        gate_controller_val = val_get_next_child(gate_controller_val)) {

        val_value_t* inputs_val;
        val_value_t* input_val;
        val_value_t* input_classes_val;
        val_value_t* input_class_val;

        /* inputs */
       inputs_val = val_find_child(gate_controller_val,
                                   SCHED_STATE_MOD,
                                   "inputs");
        assert(inputs_val);

        /* input */
        for(input_val = val_get_first_child(inputs_val);
            input_val != NULL;
            input_val = val_get_next_child(input_val)) {
            val_value_t* discards_val;

            /* discards */
           discards_val = val_find_child(input_val,
                                   SCHED_STATE_MOD,
                                   "discards");
           if(discards_val==NULL) {
               /* some gate controllers do not have individual input discards counter */
               continue;
           }

           discards_sum+=VAL_UINT64(discards_val);
        }

        /* input-classes */
        input_classes_val = val_find_child(gate_controller_val,
                                   SCHED_STATE_MOD,
                                   "input-classes");
        if(input_classes_val) {
            /* input-class */
            for (input_class_val = val_get_first_child(input_classes_val);
                 input_class_val != NULL;
                 input_class_val = val_get_next_child(input_class_val)) {


                val_value_t* discards_val;

                /* discards */
                discards_val = val_find_child(input_class_val,
                                        SCHED_STATE_MOD,
                                        "discards");
                if(discards_val==NULL) {
                   continue;
                }

                discards_sum+=VAL_UINT64(discards_val);
            }
        }
    }

    out_discards_val = val_new_value();
    assert(out_discards_val);
    val_init_from_template(out_discards_val, out_discards_obj);
    VAL_UINT32(out_discards_val)=(uint32_t)(discards_sum&0xFFFFFFFF);
    val_add_child(out_discards_val, dst_val);

    return NO_ERR;
}

/* The 3 mandatory callback functions: y_ietf_network_bridge_init, y_ietf_network_bridge_init2, y_ietf_network_bridge_cleanup */

status_t
    y_ietf_network_bridge_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t* agt_profile;
    obj_template_t* flows_obj;
    status_t res;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "ietf-interfaces",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_interfaces_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "iana-if-type",
        NULL,
        &agt_profile->agt_savedevQ,
        &iana_if_type_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-network-bridge",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_network_bridge_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-network-bridge",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_network_bridge_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-network-bridge-flows",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_network_bridge_flows_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-network-bridge-scheduler",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_network_bridge_scheduler_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "example-bridge",
        NULL,
        &agt_profile->agt_savedevQ,
        &example_bridge_mod);
    if (res != NO_ERR) {
        return res;
    }

    flows_obj = ncx_find_object(
        ietf_network_bridge_flows_mod,
        "flows");
    assert(flows_obj != NULL);


    return res;
}

status_t y_ietf_network_bridge_init2(void)
{
    status_t res;
    cfg_template_t* runningcfg;
    val_value_t* interfaces_state_val;
    val_value_t* interface_val;
    obj_template_t* interface_obj;
    obj_template_t* statistics_obj;

    res = NO_ERR;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg && runningcfg->root);

    interfaces_state_val = val_find_child(runningcfg->root,
                               "ietf-interfaces",
                               "interfaces-state");
    assert(interfaces_state_val);

    interface_obj = obj_find_child(interfaces_state_val->obj,
                      "ietf-interfaces",
                      "interface");
    assert(interface_obj);

    statistics_obj = obj_find_child(interface_obj,
                      "ietf-interfaces",
                      "statistics");
    assert(statistics_obj);

    /* register /interfaces-state/interface/statistics/out-discards virtual values callbacks */
    for(interface_val = val_get_first_child(interfaces_state_val);
        interface_val != NULL;
        interface_val = val_get_next_child(interface_val)) {
        val_value_t* statistics_val;

        statistics_val = val_find_child(interface_val,
                               "ietf-interfaces",
                               "statistics");
        if(statistics_val==NULL) {
            statistics_val = val_new_value();
            assert(statistics_val);

            val_init_virtual(statistics_val,
                             get_if_out_discards_cbfn,
                             statistics_obj);
            val_add_child(statistics_val, interface_val);
        } else {
            val123_add_virtual_cb(statistics_val,
                             get_if_out_discards_cbfn);
        }
    }
    return res;
}

void y_ietf_network_bridge_cleanup (void)
{
}
