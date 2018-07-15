/*
    module ietf-network-bridge
    namespace urn:ietf:params:xml:ns:yang:ietf-network-bridge
 */

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>


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

/* module static variables */
static ncx_module_t *ietf_interfaces_mod;
static ncx_module_t *iana_if_type_mod;
static ncx_module_t *ietf_network_bridge_mod;
static ncx_module_t *ietf_network_bridge_scheduler_mod;
static ncx_module_t *ietf_network_bridge_flows_mod;
static ncx_module_t *example_bridge_mod;
static obj_template_t* bridge_obj;



static status_t
     y_ietf_network_bridge_flows_transmit_packet (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        xml_node_t *methnode)
{
    return NO_ERR;
}

static status_t
    y_ietf_network_bridge_flows_edit (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        agt_cbtyp_t cbtyp,
        op_editop_t editop,
        val_value_t *newval,
        val_value_t *curval)
{
    status_t res;
    val_value_t *errorval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    switch (cbtyp) {
    case AGT_CB_VALIDATE:
        /* description-stmt validation here */
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        switch (editop) {
        case OP_EDITOP_LOAD:
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
        case OP_EDITOP_CREATE:
        case OP_EDITOP_DELETE:
            /*TODO*/
            break;
        default:
            assert(0);
        }

        break;
    case AGT_CB_ROLLBACK:
        /* undo device instrumentation here */
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR) {
        agt_record_error(
            scb,
            &msg->mhdr,
            NCX_LAYER_CONTENT,
            res,
            NULL,
            NCX_NT_STRING,
            errorstr,
            NCX_NT_VAL,
            errorval);
    }

    return res;
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

    res = agt_cb_register_callback(
        "ietf-network-bridge-flows",
        (const xmlChar *)"/flows",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        y_ietf_network_bridge_flows_edit);
    assert(res == NO_ERR);

    res = agt_rpc_register_method(
        "ietf-network-bridge-flows",
        "transmit-packet",
        AGT_RPC_PH_INVOKE,
        y_ietf_network_bridge_flows_transmit_packet);
    assert(res == NO_ERR);

    return res;
}

status_t y_ietf_network_bridge_init2(void)
{
    status_t res;
    cfg_template_t* runningcfg;
    val_value_t* system_state_val;
    obj_template_t* obj;
    val_value_t* clock_val;
    val_value_t* current_datetime_val;

    res = NO_ERR;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!runningcfg || !runningcfg->root) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }


    return res;
}

void y_ietf_network_bridge_cleanup (void)
{
}
