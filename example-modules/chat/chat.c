/*
    module chat (based on chat.yang)
 */

#define __USE_XOPEN 1
//#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
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
static ncx_module_t *chat_mod;


/********************************************************************
* FUNCTION y_chat_send_message_invoke
* 
* RPC invocation phase
* All constraints have passed at this point.
* Call device instrumentation code in this function.
* 
* INPUTS:
*     see agt/agt_rpc.h for details
* 
* RETURNS:
*     error status
********************************************************************/
static status_t
    y_chat_send_message_invoke (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        xml_node_t *methnode)
{
    val_value_t* input_text_val=NULL;
    val_value_t* text_val=NULL;
    status_t res;
    obj_template_t* notification_obj;
    obj_template_t* text_obj;
    agt_not_msg_t *notif;
    res = NO_ERR;

    /* remove the next line if scb is used */
    (void)scb;

    /* remove the next line if methnode is used */
    (void)methnode;

    /* invoke your device instrumentation code here */
    input_text_val = val_find_child(
        msg->rpc_input,
        "chat",
        "text");
    assert(input_text_val);
    printf("message is: %s\n", VAL_STRING(input_text_val));

    notification_obj = ncx_find_object(
        chat_mod,
        "message");
    assert(notification_obj);

    notif = agt_not_new_notification(notification_obj);
    assert (notif != NULL);

    /* add params to payload */
    text_obj = obj_find_child(notification_obj,
                      "chat",
                      "text");
    assert(text_obj);
    text_val = val_new_value();
    assert(text_val);

    val_init_from_template(text_val, text_obj);
    res = val_set_simval_obj(text_val,
                         text_val->obj,
                         VAL_STRING(input_text_val));
    agt_not_add_to_payload(notif, text_val);

    agt_not_queue_notification(notif);

    return res;

}


/* The 3 mandatory callback functions: y_chat_init, y_chat_init2, y_chat_cleanup */

status_t
    y_chat_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    status_t res;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "chat",
        NULL,
        &agt_profile->agt_savedevQ,
        &chat_mod);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_rpc_register_method(
        "chat",
        "send-message",
        AGT_RPC_PH_INVOKE,
        y_chat_send_message_invoke);
    if (res != NO_ERR) {
        return res;
    }

    return res;
}

status_t y_chat_init2(void)
{
    return NO_ERR;
}

void y_chat_cleanup(void)
{
}
