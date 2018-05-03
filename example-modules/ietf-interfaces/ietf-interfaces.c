/*
    module ietf-interfaces
    namespace urn:ietf:params:xml:ns:yang:ietf-interfaces
 */
 
#include <libxml/xmlstring.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "procdefs.h"
#include "agt.h"
#include "agt_cb.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "agt_not.h"
#include "agt_nmda.h"
#include "agt_rpc.h"
#include "dlq.h"
#include "ncx.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "status.h"
#include "rpc.h"
#include "val.h"
#include "val123.h"

/* module static variables */
static ncx_module_t *ietf_interfaces_mod;
static obj_template_t* interfaces_obj;

static val_value_t* root_prev_val=NULL;

static void my_send_link_state_notification(char* new_state, char* if_name)
{
    status_t res;
    obj_template_t* notification_obj;
    obj_template_t* link_up_obj;
    obj_template_t* link_down_obj;
    obj_template_t* if_name_obj;
    val_value_t* if_name_val;
    agt_not_msg_t *notif;
    ncx_module_t *mod;

    mod = ncx_find_module("interfaces-notifications", NULL);
    if(mod==NULL) {
        /* only send notification if the model is loaded */
        return;
    }

    link_down_obj = ncx_find_object(mod,"link-down");
    assert(link_down_obj);

    link_up_obj = ncx_find_object(mod,"link-up");
    assert(link_up_obj);

    if(0==strcmp(new_state, "down")) {
        notification_obj = link_down_obj;
    } else if(0==strcmp(new_state, "up")) {
        notification_obj = link_up_obj;
    } else {
        notification_obj = link_up_obj; /* work around */
    }

    notif = agt_not_new_notification(notification_obj);
    assert (notif != NULL);

    /* add params to payload */
    if_name_obj = obj_find_child(notification_obj,
                      "interfaces-notifications",
                      "if-name");
    assert(if_name_obj != NULL);
    if_name_val = val_new_value();
    assert(if_name_val != NULL);

    val_init_from_template(if_name_val, if_name_obj);
    res = val_set_simval_obj(if_name_val,
                         if_name_val->obj,
                         if_name);
    agt_not_add_to_payload(notif, if_name_val);

    agt_not_queue_notification(notif);
}


void oper_status_update(val_value_t* cur_val)
{
    status_t res;
    val_value_t* prev_val;
    val_value_t* val;
    val_value_t* last_change_prev_val;
    val_value_t* dummy_val;
    val_value_t* name_val;

    /* compare the oper-status with the corresponding value in the prev root */
    prev_val = val123_find_match(root_prev_val, cur_val);
    if(prev_val==NULL) {
        res=val123_clone_instance(root_prev_val, cur_val, &prev_val);
        assert(res==NO_ERR);
    }

    if(0!=strcmp(VAL_STRING(cur_val),VAL_STRING(prev_val))) {
        obj_template_t* last_change_obj;
        val_value_t* last_change_val;
        char tstamp_buf[32];
        tstamp_datetime(tstamp_buf);
        last_change_val = val_new_value();
        assert(last_change_val);
        last_change_obj = obj_find_child(cur_val->parent->obj,"ietf-interfaces","last-change");
        assert(last_change_obj);
        val_init_from_template(last_change_val, last_change_obj);
        val_set_simval_obj(last_change_val, last_change_obj, tstamp_buf);

        val = val123_find_match(root_prev_val, cur_val);
        val_remove_child(val);
        val_free_value(val);
        res=val123_clone_instance(root_prev_val, cur_val, &val);

        last_change_prev_val = val_find_child(val->parent, "ietf-interfaces", "last-change");
        if(last_change_prev_val) {
            val_remove_child(last_change_prev_val);
            val_free_value(last_change_prev_val);
        }
        val_add_child(last_change_val, val->parent);

        /* notify */
        name_val=val_find_child(cur_val->parent,"ietf-interfaces","name");
        assert(name_val);
        printf("Notification /interfaces/interface[name=%s]: oper-status changes from %s to %s at %s\n", VAL_STRING(name_val), VAL_STRING(prev_val),VAL_STRING(cur_val), VAL_STRING(last_change_val));
        my_send_link_state_notification(VAL_STRING(cur_val), VAL_STRING(name_val));

    }
}

static status_t
    get_last_change(ses_cb_t *scb,
                       getcb_mode_t cbmode,
                       val_value_t *vir_val,
                       val_value_t  *dst_val)
{
    status_t res;
    val_value_t* last_change_val;
    last_change_val = val123_find_match(root_prev_val, dst_val);
    if(last_change_val==NULL) {
        return ERR_NCX_SKIPPED;
    }
    val_set_simval_obj(dst_val, dst_val->obj, VAL_STRING(last_change_val));
    return NO_ERR;
}

static status_t
    get_oper_status(ses_cb_t *scb,
                       getcb_mode_t cbmode,
                       val_value_t *vir_val,
                       val_value_t  *dst_val)
{
    status_t res;
    int oper_status = 0;
    val_value_t *interface_val;
    val_value_t *name_val;
    ncx_btype_t btyp;
    typ_def_t *typdef, *basetypdef;
    typ_enum_t  *typenum;
    FILE* f;
    char cmd_buf[NCX_MAX_LINELEN];
    char status_buf[NCX_MAX_LINELEN];
    char* fgets_ret;

    interface_val = vir_val->parent;
    assert(interface_val);

    name_val = val_find_child(interface_val,
                              "ietf-interfaces",
                              "name");
    assert(name_val);

    /* open the /proc/net/dev file for reading */
    sprintf(cmd_buf, "cat /sys/class/net/%s/operstate", VAL_STRING(name_val));
    f = popen(cmd_buf, "r");
    if (f == NULL) {
        return errno_to_status();
    }
    fgets_ret = fgets((char *)status_buf, NCX_MAX_LINELEN, f);
    assert(fgets_ret!=NULL);
    fclose(f);
    strtok(status_buf,"\n");
    /* check if we have corresponding entry in the oper-status enum */
    btyp = obj_get_basetype(dst_val->obj);
    typdef = obj_get_typdef(dst_val->obj);
    basetypdef = typ_get_base_typdef(typdef);
    assert(btyp==NCX_BT_ENUM);
    for (typenum = typ_first_enumdef(basetypdef);
         typenum != NULL;
         typenum = typ_next_enumdef(typenum)) {
        if(0==strcmp((const char *)typenum->name, status_buf)) {
            break;
        }
    }

    if(typenum==NULL) {
        printf("Warning: unknown oper-status %s, reporting \"unknown\" instead.\n", status_buf);

        strcpy(status_buf, "unknown");
    }

    res = val_set_simval_obj(dst_val,
                             dst_val->obj,
                             (const char *)status_buf);

    oper_status_update(dst_val);

    return res;
}

/*

Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo: 4307500   49739    0    0    0     0          0         0  4307500   49739    0    0    0     0       0          0
 wlan0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
  eth0: 604474858 23680030    0    0    0     0          0    357073 701580994 6935958    0    0    0     0       1          0
 */
 
static status_t
    add_system_interface_entry(char* buf, val_value_t* interfaces_val)
{
    /*objs*/
    obj_template_t* interface_obj;
    obj_template_t* name_obj;
    obj_template_t* oper_status_obj;
    obj_template_t* last_change_obj;
    obj_template_t* statistics_obj;
    obj_template_t* obj;

    /*vals*/
    val_value_t* interface_val;
    val_value_t* name_val;
    val_value_t* oper_status_val;
    val_value_t* last_change_val;
    val_value_t* statistics_val;
    val_value_t* val;

    status_t res=NO_ERR;
    boolean done;
    char* name;
    char* str;
    char* endptr;
    unsigned int i;
    uint64_t counter;
    int ret;

    char* counter_names_array[] = {
        "in-octets",
        "in-unicast-pkts",
        "in-errors",
        "in-discards",
        NULL/*"in-fifo"*/,
        NULL/*"in-frames"*/,
        NULL/*in-compressed*/,
        "in-multicast-pkts",
        "out-octets",
        "out-unicast-pkts",
        "out-errors",
        "out-discards",
        NULL/*"out-fifo"*/,
        NULL/*out-collisions*/,
        NULL/*out-carrier*/,
        NULL/*out-compressed*/
    };

    /* get the start of the interface name */
    str = buf;
    while (*str && isspace(*str)) {
        str++;
    }
    if (*str == '\0') {
        /* not expecting a line with just whitespace on it */
        return ERR_NCX_SKIPPED;
    } else {
        name = str++;
    }

    /* get the end of the interface name */
    while (*str && *str != ':') {
        str++;
    }

    if (*str != ':') {
        /* expected e.g. eth0: ...*/
        return ERR_NCX_SKIPPED;
    } else {
    	*str=0;
    	str++;
    }

    /* /interfaces/interface */
    interface_obj = obj_find_child(interfaces_val->obj,
                                   "ietf-interfaces",
                                   "interface");
    assert(interface_obj != NULL);

    interface_val = val_new_value();
    if (interface_val == NULL) {
        return ERR_INTERNAL_MEM;
    }

    val_init_from_template(interface_val, interface_obj);

    val_add_child(interface_val, interfaces_val);

    /* /interfaces/interface/name */
    name_obj = obj_find_child(interface_obj,
                              "ietf-interfaces",
                              "name");
    assert(name_obj != NULL);


    name_val = val_new_value();
    if (name_val == NULL) {
                return ERR_INTERNAL_MEM;
    }       
    
    val_init_from_template(name_val, name_obj);

    res = val_set_simval_obj(name_val, name_obj, name);

    val_add_child(name_val, interface_val);

    res = val_gen_index_chain(interface_obj, interface_val);
    assert(res == NO_ERR);

    /* /interfaces/interface/oper-state */
    oper_status_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "oper-status");

    oper_status_val = val_new_value();
    assert(oper_status_val);

    val_init_virtual(oper_status_val,
                     get_oper_status,
                     oper_status_obj);

    val_add_child(oper_status_val, interface_val);

    /* /interfaces/interface/oper-state */
    last_change_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "last-change");

    last_change_val = val_new_value();
    assert(last_change_val);

    val_init_virtual(last_change_val,
                     get_last_change,
                     last_change_obj);

    val_add_child(last_change_val, interface_val);

    /* /interfaces/interface/statistics */
    statistics_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "statistics");
    assert(statistics_obj != NULL);
    statistics_val = val_new_value();
    if (statistics_val == NULL) {
        return ERR_INTERNAL_MEM;
    }

    val_init_from_template(statistics_val, statistics_obj);

    val_add_child(statistics_val, interface_val);

    done = FALSE;
    for(i=0;i<(sizeof(counter_names_array)/sizeof(char*));i++) {
        endptr = NULL;
        counter = strtoull((const char *)str, &endptr, 10);
        if (counter == 0 && str == endptr) {
            /* number conversion failed */
            log_error("Error: /proc/net/dev number conversion failed.");
            return ERR_NCX_OPERATION_FAILED;
        }

        if(counter_names_array[i]!=NULL) {
            obj = obj_find_child(statistics_obj,
                                 "ietf-interfaces",
                                 counter_names_array[i]);
    	    assert(obj != NULL);

            val = val_new_value();
            if (val == NULL) {
                return ERR_INTERNAL_MEM;
            }
            val_init_from_template(val, obj);
            VAL_UINT64(val) = counter;
            val_add_child(val, statistics_val);
        }

        str = (xmlChar *)endptr;
        if (*str == '\0' || *str == '\n') {
            break;
        }
    }
    return res;
}

/* Registered callback functions: get_system_interfaces */

static status_t
    get_system_interfaces(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    FILE* f;
    status_t res;
    res = NO_ERR;
    boolean done;
    char* buf;
    unsigned int line;

    /* open /proc/net/dev for reading */
    f = fopen("/proc/net/dev", "r");
    if (f == NULL) {
        return ERR_INTERNAL_VAL;
    }

    /* get a file read line buffer */
    buf = (char*)malloc(NCX_MAX_LINELEN);
    if (buf == NULL) {
        fclose(f);
        return ERR_INTERNAL_MEM;
    }

    done = FALSE;
    line = 0;

    while (!done) {

        if (NULL == fgets((char *)buf, NCX_MAX_LINELEN, f)) {
            done = TRUE;
            continue;
        } else {
            line++;
        }

        if (line < 3) {
            /* skip the first 2 lines */
            continue;
        }

        res = add_system_interface_entry(buf, dst_val);
        if (res != NO_ERR) {
             done = TRUE;
        }
    }

    fclose(f);
    free(buf);

    return res;
}

int
    my_timer_fn (uint32 timer_id,
                      void *cookie)
{
    /*
     * Brute force method for polling for connection state changes
     * without this link-up and link-down notifications will be
     * generated only when someone reads oper-state
     */
    val_value_t* root_system_val;
    val_value_t* interfaces_val;
    xmlChar* dummy_serialized_data_str;
    status_t res;

    res = NO_ERR;

    root_system_val = agt_nmda_get_root_system();
    assert(root_system_val);

    interfaces_val = val_find_child(root_system_val,
                                    "ietf-interfaces",
                                    "interfaces");
    assert(interfaces_val);
    res = val_make_serialized_string (interfaces_val, NCX_DISPLAY_MODE_JSON, &dummy_serialized_data_str);
    free(dummy_serialized_data_str);

    return 0;

}

/* The 3 mandatory callback functions: y_ietf_interfaces_init, y_ietf_interfaces_init2, y_ietf_interfaces_cleanup */

status_t
    y_ietf_interfaces_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
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

    assert(0!=strcmp(ietf_interfaces_mod->version,"2014-05-08"));

    interfaces_obj = ncx_find_object(
        ietf_interfaces_mod,
        "interfaces");
    if(interfaces_obj == NULL) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }
    
    return res;
}

status_t y_ietf_interfaces_init2(void)
{
    status_t res;
    val_value_t* interfaces_val;
    uint32 timer_id;
    val_value_t* root_system_val;
    xmlChar* dummy_serialized_data_str;

    res = NO_ERR;

    root_system_val = agt_nmda_get_root_system();
    assert(root_system_val);

    interfaces_val = val_find_child(root_system_val,
                                    "ietf-interfaces",
                                    "interfaces");
    /* Can not coexist with other implementation
     * of ietf-interfaces.
     */
    if(interfaces_val!=NULL) {
        log_error("\nError: /interfaces already present!");
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    interfaces_val = val_new_value();
    if (interfaces_val == NULL) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    val_init_virtual(interfaces_val,
                     get_system_interfaces,
                     interfaces_obj);

    val_add_child(interfaces_val, root_system_val);

    /* init a root value to store copies of prev state data values */
    root_prev_val = val_new_value();
    val_init_from_template(root_prev_val, root_system_val->obj);

    res = agt_timer_create(1/* 1 sec period */,
                           TRUE/*periodic*/,
                           my_timer_fn,
                           NULL/*cookie*/,
                           &timer_id);
    return res;
}

void y_ietf_interfaces_cleanup (void)
{
}
