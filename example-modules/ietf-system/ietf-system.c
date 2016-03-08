/*
    module ietf-system
    namespace urn:ietf:params:xml:ns:yang:ietf-system
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

#define __USE_XOPEN 1
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <assert.h>

/* module static variables */
static ncx_module_t *ietf_system_mod;
static obj_template_t* system_state_obj;

/* Registered callback functions: get_system_state_clock */

static status_t
    get_system_state_clock_current_datetime(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res;
    res = NO_ERR;
    char tstamp_buf[32];

    tstamp_datetime(tstamp_buf);

    /* /system-state/clock/current-datetime */
    res = val_set_simval_obj(dst_val, dst_val->obj, tstamp_buf);

    return res;
}

static status_t
     y_ietf_system_set_current_datetime (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        xml_node_t *methnode)
{
    val_value_t* current_datetime_val;
    struct timespec tp;
    struct tm tm;
    char* ptr;
    int ret;

    current_datetime_val = val_find_child(
        msg->rpc_input,
        "ietf-system",
        "current-datetime");
    assert(current_datetime_val!=NULL);

    memset(&tm, 0, sizeof(struct tm));
    memset(&tp, 0, sizeof(struct timespec));
    ptr=strptime(VAL_STRING(current_datetime_val), "%Y-%m-%dT%H:%M:%S", &tm);
    assert(ptr!=NULL);
    tp.tv_sec=mktime(&tm);
    tp.tv_nsec=0;

    ret=clock_settime(CLOCK_REALTIME, &tp);
    ret=settimeofday (&tp, NULL);
    assert(ret==0);
    ret=system("hwclock --systohc");
    assert(ret==0);

    return NO_ERR;
}


/* The 3 mandatory callback functions: y_ietf_system_init, y_ietf_system_init2, y_ietf_system_cleanup */

status_t
    y_ietf_system_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    status_t res;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "ietf-system",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_system_mod);
    if (res != NO_ERR) {
        return res;
    }

    system_state_obj = ncx_find_object(
        ietf_system_mod,
        "system-state");
    if (system_state_obj == NULL) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    res = agt_rpc_register_method(
        "ietf-system",
        "set-current-datetime",
        AGT_RPC_PH_INVOKE,
        y_ietf_system_set_current_datetime);
    if (res != NO_ERR) {
        return res;
    }

    return res;
}

status_t y_ietf_system_init2(void)
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

    system_state_val = val_find_child(runningcfg->root,
                                          "ietf-system",
                                          "system-state");
    /* Can not coexist with other implementation
     * of ietf-system.
     */
    if(system_state_val==NULL) {
        system_state_val = val_new_value();
        assert(system_state_val != NULL);

        val_init_from_template(system_state_val,
                               system_state_obj);
        val_add_child(system_state_val, runningcfg->root);
    }

    /* /system-state/clock */
    clock_val = val_find_child(system_state_val,
                                          "ietf-system",
                                          "clock");

    if(clock_val==NULL) {
        obj = obj_find_child(system_state_val->obj,
                                       "ietf-system",
                                       "clock");
        assert(obj != NULL);

        clock_val = val_new_value();
        assert(clock_val != NULL);

        val_init_from_template(clock_val,
                               obj);

        val_add_child(clock_val, system_state_val);

    }

    obj = obj_find_child(clock_val->obj,
                         "ietf-system",
                         "current-datetime");
    assert(obj != NULL);

    current_datetime_val = val_new_value();
    assert(current_datetime_val != NULL);

    val_init_from_template(current_datetime_val,
                           obj);

    val_init_virtual(current_datetime_val,
                     get_system_state_clock_current_datetime,
                     current_datetime_val->obj);

    val_add_child(current_datetime_val, clock_val);

    return res;
}

void y_ietf_system_cleanup (void)
{
}
