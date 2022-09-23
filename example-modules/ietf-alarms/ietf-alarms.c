/*
    module ietf-alarms
    namespace urn:ietf:params:xml:ns:yang:ietf-alarms
 */
 
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

#include <string.h>
#include <assert.h>

static status_t
    y_ietf_alarms_alarms_edit (
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
            /* device instrumentation here */
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

/* The 3 mandatory callback functions: y_ietf_alarms_init, y_ietf_alarms_init2, y_ietf_alarms_cleanup */

static obj_template_t* alarms_obj;

status_t
    y_ietf_alarms_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    status_t res;
    ncx_module_t *mod;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "ietf-alarms",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    if (res != NO_ERR) {
        return res;
    }

    alarms_obj = ncx_find_object(
        mod,
        "alarms");
    assert(alarms_obj);

    agt_disable_feature ("ietf-alarms", "operator-actions");
    agt_disable_feature ("ietf-alarms", "alarm-shelving");
    agt_disable_feature ("ietf-alarms", "alarm-history");

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/alarms",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        y_ietf_alarms_alarms_edit);
    if (res != NO_ERR) {
        return res;
    }

    return res;
}

status_t y_ietf_alarms_init2(void)
{
    status_t res;
    cfg_template_t* runningcfg;
    obj_template_t* alarm_list_obj;
    obj_template_t* number_of_alarms_obj;
    val_value_t* alarms_val;
    val_value_t* alarm_list_val;
    val_value_t* number_of_alarms_val;
    res = NO_ERR;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg!=NULL && runningcfg->root!=NULL);

    alarms_val = val_find_child(runningcfg->root,
                                "ietf-alarms",
                                "alarms");
    if(alarms_val==NULL) {
        alarms_val=val_new_value();
        assert(alarms_val != NULL);
        val_init_from_template(alarms_val, alarms_obj);
        val_add_child(alarms_val, runningcfg->root);
    }

    alarm_list_obj = obj_find_child(alarms_obj,
                         "ietf-alarms",
                         "alarm-list");
    assert(alarm_list_obj);
    alarm_list_val=val_new_value();
    assert(alarm_list_val != NULL);
    val_init_from_template(alarm_list_val, alarm_list_obj);
    val_add_child(alarm_list_val, alarms_val);

    number_of_alarms_obj = obj_find_child(alarm_list_obj,
                         "ietf-alarms",
                         "number-of-alarms");
    assert(number_of_alarms_obj);
    number_of_alarms_val=val_new_value();
    assert(number_of_alarms_val != NULL);
    val_init_from_template(number_of_alarms_val, number_of_alarms_obj);
    VAL_UINT32(number_of_alarms_val)=0;
    val_add_child(number_of_alarms_val, alarm_list_val);

    return res;
}

void y_ietf_alarms_cleanup (void)
{
}
