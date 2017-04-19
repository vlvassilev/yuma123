/*
    module interfaces-alarms
    namespace http://yuma123.org/ns/interfaces-alarms
 */
 
#include <xmlstring.h>
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
#include "alarmctrl.h"

#include <string.h>
#include <assert.h>

static status_t
    y_interfaces_alarms_alarms_edit (
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
/* callback intercepting all notifications. The registeration is done in y_interfaces_alarms_init */
status_t notification_cb(agt_not_msg_t *notif)
{
    val_value_t *payload_val;
    char description_str[1024];
    status_t res;
    int ret;

    if((0==strcmp(obj_get_name(notif->notobj),"link-up")) || (0==strcmp(obj_get_name(notif->notobj),"link-down"))) {
        int down;
    	down = (0==strcmp(obj_get_name(notif->notobj),"link-down"));
        for (payload_val = (val_value_t *)dlq_firstEntry(&notif->payloadQ);
             payload_val != NULL;
             payload_val = (val_value_t *)dlq_nextEntry(payload_val)) {
            if(0==strcmp("if-name",obj_get_name(payload_val->obj))) {
                char* resource_str;
                sprintf(description_str,"Link down - %s",VAL_STRING(payload_val));
                //alarm_event_w_type(description_str, "minor", "communications", down?1:0);
                resource_str=malloc(strlen("/interfaces/interface[name=\'%s\']")+strlen(VAL_STRING(payload_val))+1);
                sprintf(resource_str,"/interfaces/interface[name=\'%s\']",VAL_STRING(payload_val));
                ret=alarmctrl_event(resource_str, "link-alarm"/*alarm_type_id_str*/, ""/*alarm_type_qualifier_str*/, "major", "Probably someone disconnected something?!", down?1:0);
                //assert(ret==0);
                free(resource_str);
                break;
            }
        }
    }
    return NO_ERR;
}


/* The 3 mandatory callback functions: y_interfaces_alarms_init, y_interfaces_alarms_init2, y_interfaces_alarms_cleanup */

status_t
    y_interfaces_alarms_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    status_t res;
    ncx_module_t *mod;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "interfaces-alarms",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_cb_register_callback(
        "ietf-alarms",
        (const xmlChar *)"/alarms",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        y_interfaces_alarms_alarms_edit);
    if (res != NO_ERR) {
        return res;
    }

    agt_not_queue_notification_cb_register("intrfaces-alarms", notification_cb);

    return res;
}

status_t y_interfaces_alarms_init2(void)
{
    status_t res;
    cfg_template_t* runningcfg;
    val_value_t* alarms_val;

    res = NO_ERR;

    return res;
}

void y_interfaces_alarms_cleanup (void)
{
}

