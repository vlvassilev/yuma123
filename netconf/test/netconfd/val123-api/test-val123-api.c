/*
    module test-val123-api
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "ncxmod.h"
#include "val123.h"
#include "agt.h"
#include "ses.h"
#include "cfg.h"
#include "getcb.h"

/* module static variables */
static ncx_module_t *ietf_system_mod;

static status_t
    get_os_name(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    obj_template_t* obj;
    val_value_t* val;
    status_t res;
    res = NO_ERR;

    val = val_new_value();
    assert(val);

    obj = obj_find_child(dst_val->obj,
                         "ietf-system",
                         "os-name");
    assert(obj);

    val_init_from_template(val,obj);

    /* /system-state/platform/os-name */
    res = val_set_simval_obj(val, val->obj, "foo");

    val_add_child(val, dst_val);

    return res;
}

static status_t
    get_os_release(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    obj_template_t* obj;
    val_value_t* val;
    status_t res;
    res = NO_ERR;

    obj = obj_find_child(dst_val->obj,
                         "ietf-system",
                         "os-release");
    assert(obj);

    val = val_new_value();
    assert(val);

    val_init_from_template(val,obj);

    /* /system-state/platform/os-release */
    res = val_set_simval_obj(val, val->obj, "bar");

    val_add_child(val, dst_val);

    return res;
}


/* The 3 mandatory callback functions: y_test_val123_api_init, y_test_val123_api_init2, y_test_val123_api_cleanup */

status_t
    y_test_val123_api_init (
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

    return res;
}

status_t y_test_val123_api_init2(void)
{
    status_t res;
    cfg_template_t* runningcfg;
    val_value_t* system_state_val;
    obj_template_t* system_state_obj;
    obj_template_t* platform_obj;
    val_value_t* platform_val;

    res = NO_ERR;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg && runningcfg->root);

    system_state_obj = ncx_find_object(
        ietf_system_mod,
        "system-state");
    assert(system_state_obj);

    system_state_val = val_new_value();
    assert(system_state_val);

    val_init_from_template(system_state_val,
                           system_state_obj);
    val_add_child(system_state_val, runningcfg->root);

    platform_obj = obj_find_child(system_state_val->obj,
                         "ietf-system",
                         "platform");
    assert(platform_obj != NULL);

    platform_val = val_new_value();
    assert(platform_val != NULL);

    val_init_virtual(platform_val,
                     get_os_name,
                     platform_obj);

    val_add_child(platform_val, system_state_val);

    val123_add_virtual_cb(platform_val,
                     get_os_release);

    return NO_ERR;
}

void y_test_val123_api_cleanup (void)
{
}
