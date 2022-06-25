/*
    module ietf-interfaces
    namespace urn:ietf:params:xml:ns:yang:ietf-interfaces
 */

#include <libxml/xmlstring.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "procdefs.h"
#include "agt.h"
#include "agt_cb.h"
#include "agt_cli.h"
#include "agt_commit_complete.h"
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
#include "../../libintri/.libintrishare/libintrishare.h"

/* module static variables */
static val_value_t* root_prev_val;
static val_value_t* with_nmda_param_val;
static     uint32 timer_id;

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

        last_change_prev_val = val_find_child(prev_val->parent, "ietf-interfaces", "last-change");
        if(last_change_prev_val) {
            val_remove_child(last_change_prev_val);
            val_free_value(last_change_prev_val);
        }
        val_add_child(last_change_val, prev_val->parent);

        /* notify */
        name_val=val_find_child(cur_val->parent,"ietf-interfaces","name");
        assert(name_val);
        printf("Notification /interfaces/interface[name=%s]: oper-status changes from %s to %s at %s\n", VAL_STRING(name_val), VAL_STRING(prev_val),VAL_STRING(cur_val), VAL_STRING(last_change_val));
        my_send_link_state_notification(VAL_STRING(cur_val), VAL_STRING(name_val));
        val_set_simval_obj(prev_val, prev_val->obj, VAL_STRING(cur_val));

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

static status_t
    get_speed(ses_cb_t *scb,
                       getcb_mode_t cbmode,
                       val_value_t *vir_val,
                       val_value_t  *dst_val)
{
    status_t res;
    val_value_t *interface_val;
    val_value_t *name_val;
    FILE* f;
    char filename[NCX_MAX_LINELEN];
    char status_buf[NCX_MAX_LINELEN];
    char* fgets_ret;

    interface_val = vir_val->parent;
    assert(interface_val);

    name_val = val_find_child(interface_val,
                              "ietf-interfaces",
                              "name");
    assert(name_val);

    /* open the /proc/net/dev file for reading */
    sprintf(filename, "/sys/class/net/%s/speed", VAL_STRING(name_val));
    f = fopen(filename, "r");
    if (f == NULL) {
        return ERR_NCX_SKIPPED;
    }
    fgets_ret = fgets((char *)status_buf, NCX_MAX_LINELEN, f);
    fclose(f);
    if(fgets_ret==NULL) {
        return ERR_NCX_SKIPPED;
    }
    strtok(status_buf,"\n");

    VAL_UINT64(dst_val) = (uint64_t)1000000*atoi(status_buf);

    return NO_ERR;
}


static status_t
    add_interface_state_entry(char* buf, val_value_t* interfaces_val)
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

    if(NULL!=getenv("INTERFACE_NAME_PREFIX")) {
        char* prefix=getenv("INTERFACE_NAME_PREFIX");
        if(strlen(name)<strlen(prefix) || 0!=memcmp(prefix,name,strlen(prefix))) {
            return NO_ERR; /*skip*/
        }
    }

    /* interface */
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

    /* interface/name */
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

    /* interface/oper-state */
    oper_status_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "oper-status");

    oper_status_val = val_new_value();
    assert(oper_status_val);

    val_init_virtual(oper_status_val,
                     get_oper_status,
                     oper_status_obj);

    val_add_child(oper_status_val, interface_val);

    /* interface/last-change */
    last_change_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "last-change");

    last_change_val = val_new_value();
    assert(last_change_val);

    val_init_virtual(last_change_val,
                     get_last_change,
                     last_change_obj);

    val_add_child(last_change_val, interface_val);

    /* interface/speed */
    obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "speed");

    val = val_new_value();
    assert(val);

    val_init_virtual(val,
                     get_speed,
                     obj);

    val_add_child(val, interface_val);

    /* interface/statistics */
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
static status_t
    add_interface_entry(char* buf, val_value_t* interfaces_val)
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

    if(NULL!=getenv("INTERFACE_NAME_PREFIX")) {
        char* prefix=getenv("INTERFACE_NAME_PREFIX");
        if(strlen(name)<strlen(prefix) || 0!=memcmp(prefix,name,strlen(prefix))) {
            return NO_ERR; /*skip*/
        }
    }

    /* interface */
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

    /* interface/name */
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

    /* interface/oper-state */
    oper_status_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "oper-status");

    oper_status_val = val_new_value();
    assert(oper_status_val);

    val_init_virtual(oper_status_val,
                     get_oper_status,
                     oper_status_obj);

    val_add_child(oper_status_val, interface_val);

    /* interface/last-change */
    last_change_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "last-change");

    last_change_val = val_new_value();
    assert(last_change_val);

    val_init_virtual(last_change_val,
                     get_last_change,
                     last_change_obj);

    val_add_child(last_change_val, interface_val);

    /* interface/speed */
    obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "speed");

    val = val_new_value();
    assert(val);

    val_init_virtual(val,
                     get_speed,
                     obj);

    val_add_child(val, interface_val);

    /* interface/statistics */
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

/* Registered callback functions: get_interfaces */

static status_t
    get_interfaces(ses_cb_t *scb,
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
            printf("first 2 is %s\n", buf);
            continue;
        }

        printf("buf is %s\n", buf);

        res = add_interface_entry(buf, dst_val);
        if (res != NO_ERR) {
             done = TRUE;
        }
    }

    fclose(f);
    free(buf);
    printf("###### res %d\n", res);
    return res;
}

static status_t
    get_interfaces_state(ses_cb_t *scb,
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
            printf("first 2 is %s\n", buf);
            continue;
        }

        printf("buf is %s\n", buf);

        res = add_interfac_state_entry(buf, dst_val);
        if (res != NO_ERR) {
             done = TRUE;
        }
    }

    fclose(f);
    free(buf);
    printf("###### res %d\n", res);
    return res;
}

int my_timer_fn(uint32 timer_id, void *cookie)
{
    /*
     * Brute force method for polling for connection state changes
     * without this link-up and link-down notifications will be
     * generated only when someone reads oper-state
     */
    val_value_t* root_system_val;
    val_value_t* interfaces_val = cookie;
    xmlChar* dummy_serialized_data_str;
    status_t res;

    res = NO_ERR;

    /* by serializing the value all virtual node callbacks are periodically executed */
    res = val_make_serialized_string(interfaces_val, NCX_DISPLAY_MODE_JSON, &dummy_serialized_data_str);
    free(dummy_serialized_data_str);

    return 0;

}

static status_t init2_w_nmda(void)
{
}

static void interface_delete(val_value_t* interface_val)
{
    int ret;
    int n;
    char* cmd_buf;
    val_value_t* name_val;

    name_val = val_find_child(interface_val,"ietf-interfaces","name");
    assert(name_val);

    n = snprintf(NULL, 0, "ifconfig %s down", VAL_STRING(name_val));
    assert(n>0);
    cmd_buf=malloc(n+1);
    snprintf(cmd_buf, n+1, "ifconfig %s down", VAL_STRING(name_val));
    log_info("Interface down: %s\n", cmd_buf);
    ret=system(cmd_buf);
    //assert(ret==0);
    if(ret!=0) {
        perror(cmd_buf);
    }
    free(cmd_buf);
}

static void interface_create(val_value_t* interface_val)
{
    int ret;
    int n;
    char* cmd_buf;
    val_value_t* name_val;

    name_val = val_find_child(interface_val,"ietf-interfaces","name");
    assert(name_val);

    n = snprintf(NULL, 0, "ifconfig %s up", VAL_STRING(name_val));
    assert(n>0);
    cmd_buf=malloc(n+1);
    snprintf(cmd_buf, n+1, "ifconfig %s up", VAL_STRING(name_val));
    log_info("Interface up: %s\n", cmd_buf);
    ret=system(cmd_buf);
    //assert(ret==0);
    if(ret!=0) {
        perror(cmd_buf);
    }

    free(cmd_buf);
}

static int update_config(val_value_t* config_cur_val, val_value_t* config_new_val)
{

    status_t res;

    val_value_t *interfaces_cur_val, *interface_cur_val;
    val_value_t *interfaces_new_val, *interface_new_val;


    if(config_new_val == NULL) {
        interfaces_new_val = NULL;
    } else {
        interfaces_new_val = val_find_child(config_new_val,
                               "ietf-interfaces",
                               "interfaces");
    }

    if(config_cur_val == NULL) {
        interfaces_cur_val = NULL;
    } else {
        interfaces_cur_val = val_find_child(config_cur_val,
                                       "ietf-interfaces",
                                       "interfaces");
    }

    /* 2 step (delete/add) interface configuration */

    /* 1. deactivation loop - deletes all deleted interface -s */
    if(interfaces_cur_val!=NULL) {
        for (interface_cur_val = val_get_first_child(interfaces_cur_val);
             interface_cur_val != NULL;
             interface_cur_val = val_get_next_child(interface_cur_val)) {
            interface_new_val = val123_find_match(config_new_val, interface_cur_val);
            if(interface_new_val==NULL) {
                interface_delete(interface_cur_val);
            }
        }
    }

    /* 2. activation loop - creates all new interface -s */
    if(interfaces_new_val!=NULL) {
        for (interface_new_val = val_get_first_child(interfaces_new_val);
             interface_new_val != NULL;
             interface_new_val = val_get_next_child(interface_new_val)) {
            interface_cur_val = val123_find_match(config_cur_val, interface_new_val);
            if(interface_cur_val==NULL) {
                interface_create(interface_new_val);
            }
        }
    }
    return NO_ERR;
}

static val_value_t* prev_root_val = NULL;
static int update_config_wrapper()
{
    cfg_template_t        *runningcfg;
    status_t res;
    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg!=NULL && runningcfg->root!=NULL);
    if(prev_root_val!=NULL) {
        val_value_t* cur_root_val;
        cur_root_val = val_clone_config_data(runningcfg->root, &res);
        if(0==val_compare(cur_root_val,prev_root_val)) {
            /*no change*/
            val_free_value(cur_root_val);
            return 0;
        }
        val_free_value(cur_root_val);
    }
    update_config(prev_root_val, runningcfg->root);

    if(prev_root_val!=NULL) {
        val_free_value(prev_root_val);
    }
    prev_root_val = val_clone_config_data(runningcfg->root, &res);
    return 0;
}

static status_t y_commit_complete(void)
{
    update_config_wrapper();
    return NO_ERR;
}

/* The 3 mandatory callback functions: y_ietf_interfaces_init, y_ietf_interfaces_init2, y_ietf_interfaces_cleanup */

status_t
    y_ietf_interfaces_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    ncx_module_t *mod;
    status_t res;

    val_value_t *clivalset;

    /* check for --with-nmda=true (param defined in netconfd-ex.yang) */
    clivalset = agt_cli_get_valset();
    with_nmda_param_val = val_find_child(clivalset, "netconfd-ex", "with-nmda");

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "ietf-interfaces",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    assert(res == NO_ERR);

    if(with_nmda_param_val && VAL_BOOL(with_nmda_param_val)) {
        printf("\n Not support nmda \n");
        assert(0==strcmp(mod->version,"2018-02-20"));
        return ERR_NCX_OPERATION_NOT_SUPPORTED;
    } else {
        assert(0==strcmp(mod->version,"2014-05-08"));
    }

    res = ncxmod_load_module(
        "iana-if-type",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    assert(res == NO_ERR);

    res = ncxmod_load_module(
        "interfaces-notifications",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    assert(res == NO_ERR);

    res=agt_commit_complete_register("ietf-interfaces" /*SIL id string*/,
                                     y_commit_complete);
    assert(res == NO_ERR);

    return res;
}

status_t y_ietf_interfaces_init2(void)
{
    status_t res;
    ncx_module_t* mod;
    obj_template_t* interfaces_obj;
    obj_template_t* interfaces_state_obj;
    val_value_t* interfaces_val;
    val_value_t* interfaces_state_val;
    val_value_t* root_val;

    res = NO_ERR;

    mod = ncx_find_module("ietf-interfaces", NULL);
    assert(mod);

    cfg_template_t* runningcfg;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg && runningcfg->root);
    root_val = runningcfg->root;
    /* Add /interfaces-state */
    interfaces_state_obj = ncx_find_object(
        mod,
        "interfaces-state");
    assert(interfaces_state_obj);
    interfaces_state_val = val_find_child(root_val,
                                    "ietf-interfaces",
                                    "interfaces-state");


    /* not designed to coexist with other implementations */
    assert(interfaces_state_val==NULL);

    interfaces_state_val = val_new_value();
    assert(interfaces_state_val);

    val_init_virtual(interfaces_state_val,
                     get_interfaces_state,
                     interfaces_state_obj);


    val_add_child(interfaces_state_val, root_val);

    /* Add /interfaces */
    interfaces_obj = ncx_find_object(
        mod,
        "interfaces");
    assert(interfaces_obj);
    interfaces_val = val_find_child(root_val,
                                    "ietf-interfaces",
                                    "interfaces");


    /* not designed to coexist with other implementations */
    assert(interfaces_val==NULL);

    interfaces_val = val_new_value();
    assert(interfaces_val);

    val_init_virtual(interfaces_val,
                     get_interfaces,
                     interfaces_obj);


    val_add_child(interfaces_state_val, root_val);



    /* init a root value to store copies of prev state data values */
    root_prev_val = val_new_value();
    val_init_from_template(root_prev_val, root_val->obj);

    y_commit_complete();

    return res;
}


void y_ietf_interfaces_cleanup (void)
{
#if 0
    agt_cb_unregister_callbacks( "ietf-interfaces",
                               (const xmlChar *)"/interfaces/interface");
    agt_cb_unregister_callbacks( "ietf-interfaces",
                               (const xmlChar *)"/interfaces/interface/enable");
#endif
}

