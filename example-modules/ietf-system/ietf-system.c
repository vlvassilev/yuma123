/*
    module ietf-system
    namespace urn:ietf:params:xml:ns:yang:ietf-system
 */

#define _BSD_SOURCE
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
    struct timespec ts;
    struct timeval tv;
    struct tm tm;
    char* ptr;
    int ret;

    current_datetime_val = val_find_child(
        msg->rpc_input,
        "ietf-system",
        "current-datetime");
    assert(current_datetime_val!=NULL);

    memset(&tm, 0, sizeof(struct tm));
    memset(&ts, 0, sizeof(struct timespec));
    memset(&tv, 0, sizeof(struct timeval));
    ptr=strptime(VAL_STRING(current_datetime_val), "%Y-%m-%dT%H:%M:%S", &tm);
    assert(ptr!=NULL);
    ts.tv_sec=mktime(&tm);
    ts.tv_nsec=0;

    //TIMESPEC_TO_TIMEVAL(ts, tv);
    tv.tv_sec = ts.tv_sec;
    tv.tv_usec = ts.tv_nsec / 1000;

    ret=clock_settime(CLOCK_REALTIME, &ts);
    assert(ret==0);
    ret=settimeofday (&tv, NULL);
    assert(ret==0);
    ret=system("hwclock --systohc");
    assert(ret==0);

    return NO_ERR;
}


static status_t
     y_ietf_system_system_restart (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        xml_node_t *methnode)
{
    agt_request_shutdown(NCX_SHUT_EXIT);

    /* wait for process termination before rebooting */
    {
        int status;
        pid_t child_pid;

        log_debug("Waiting for netconfd to complete shutdown.\n");
        if((child_pid = fork()) != 0) {
            while (child_pid != wait(&status)) {
                if(child_pid==-1) {
                    exit(-1);
                }
            }
            fprintf(stderr, "system-restart: Rebooting system ...");
            status = system("reboot");
            if(status!=0) {
                return ERR_NCX_OPERATION_FAILED;
            }
        }
    }
    return NO_ERR;
}

static status_t
    y_ietf_system_system_hostname_edit (
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
            if(newval!=NULL) {
                char* buf;
                int ret;
                buf=malloc(strlen("hostname ") + strlen(VAL_STRING(newval))+1);
                sprintf(buf,"hostname %s", VAL_STRING(newval));
                printf("Setting /system/hostname to %s - cmd=%s\n", VAL_STRING(newval), buf);
                ret=system(buf);
                if(ret != 0) {
                    errorval=newval;
                    errorstr="Can't set hostname. Are you sure your server is running as root?"; /* strdup(strerror(errno)); */
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                }
            }
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

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/hostname",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        y_ietf_system_system_hostname_edit);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_rpc_register_method(
        "ietf-system",
        "set-current-datetime",
        AGT_RPC_PH_INVOKE,
        y_ietf_system_set_current_datetime);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_rpc_register_method(
        "ietf-system",
        "system-restart",
        AGT_RPC_PH_INVOKE,
        y_ietf_system_system_restart);
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

    val_init_virtual(current_datetime_val,
                     get_system_state_clock_current_datetime,
                     obj);

    val_add_child(current_datetime_val, clock_val);

    return res;
}

void y_ietf_system_cleanup (void)
{
}
