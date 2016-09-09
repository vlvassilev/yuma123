/*
    module ietf-interfaces
    namespace urn:ietf:params:xml:ns:yang:ietf-interfaces
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

#include <string.h>
#include <assert.h>

/* module static variables */
static ncx_module_t *ietf_interfaces_mod;
static obj_template_t* interfaces_state_obj;

static val_value_t* root_prev_val=NULL;

static val_value_t* val123_find_match(val_value_t* haystack_root_val, val_value_t* needle_val)
{
    val_value_t* val;
    char* pathbuff;
    status_t res = val_gen_instance_id(NULL, needle_val, NCX_IFMT_XPATH1, (xmlChar **) &pathbuff);
    assert(res==NO_ERR);
    res = xpath_find_val_target(haystack_root_val, NULL/*mod*/, pathbuff, &val);
    return val;
}

static status_t val123_clone_instance_ex(val_value_t* clone_root_val, val_value_t* original_val, val_value_t** return_clone_val, boolean without_non_index_children)
{
    status_t res;
    val_value_t* clone_parent_val;
    val_value_t* clone_val;
    val_index_t* index;
    val_value_t* val;

    if(obj_is_root(original_val->obj) || (original_val->parent==NULL)) {
        return ERR_NCX_INVALID_VALUE;
    }
    if(obj_is_root(original_val->parent->obj)) {
        clone_parent_val = clone_root_val;
    } else {
        res = val123_clone_instance_ex(clone_root_val, original_val->parent, &clone_parent_val, TRUE);
        if(res != NO_ERR) {
            return res;
        }
    }

    clone_val = val_clone(original_val);
    assert(clone_val);
    if(without_non_index_children) {
        clone_val = val_new_value();
        val_init_from_template(clone_val, original_val->obj);
        for(index = val_get_first_index(original_val);
            index != NULL;
            index = val_get_next_index(index)) {
            val = val_clone(index->val);
            assert(val!=NULL);
            val_add_child(val, clone_val);
        }
    } else {
        clone_val = val_clone(original_val);
    }
    val_add_child(clone_val,clone_parent_val);
    *return_clone_val = clone_val;
    return NO_ERR;
}

status_t val123_clone_instance(val_value_t* root_val, val_value_t* original_val, val_value_t** clone_val)
{
    return val123_clone_instance_ex(root_val, original_val, clone_val, FALSE);
}

void oper_status_update(val_value_t* cur_val)
{
    status_t res;
    val_value_t* prev_val;
    val_value_t* val;
    val_value_t* last_change_prev_val;
    val_value_t* dummy_val;
    /* compare the oper-status with the corresponding value in the prev root */
    prev_val = val123_find_match(root_prev_val, cur_val);
    if(prev_val==NULL) {
        res=val123_clone_instance(root_prev_val, cur_val, &prev_val);
        assert(res==NO_ERR);
    }

    prev_val = val_clone(prev_val);

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
        printf("Notification: oper-status changes from %s to %s at %s\n",VAL_STRING(prev_val),VAL_STRING(cur_val), VAL_STRING(last_change_val));
        val_free_value(prev_val);
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
    fgets((char *)status_buf, NCX_MAX_LINELEN, f);
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
    add_interfaces_state_entry(char* buf, val_value_t* interfaces_state_val)
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

    /* /interfaces-state/interface */
    interface_obj = obj_find_child(interfaces_state_val->obj,
                                   "ietf-interfaces",
                                   "interface");
    assert(interface_obj != NULL);

    interface_val = val_new_value();
    if (interface_val == NULL) {
        return ERR_INTERNAL_MEM;
    }

    val_init_from_template(interface_val, interface_obj);

    val_add_child(interface_val, interfaces_state_val);

    /* /interfaces-state/interface/name */
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

    /* /interfaces-state/interface/oper-state */
    oper_status_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "oper-status");

    oper_status_val = val_new_value();
    assert(oper_status_val);

    val_init_virtual(oper_status_val,
                     get_oper_status,
                     oper_status_obj);

    val_add_child(oper_status_val, interface_val);

    /* /interfaces-state/interface/oper-state */
    last_change_obj = obj_find_child(interface_obj,
                         "ietf-interfaces",
                         "last-change");

    last_change_val = val_new_value();
    assert(last_change_val);

    val_init_virtual(last_change_val,
                     get_last_change,
                     last_change_obj);

    val_add_child(last_change_val, interface_val);

    /* /interfaces-state/interface/statistics */
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

/* Registered callback functions: get_interfaces_state */

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
            continue;
        }

        res = add_interfaces_state_entry(buf, dst_val);
        if (res != NO_ERR) {
             done = TRUE;
        }
    }

    fclose(f);
    free(buf);

    return res;
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

    interfaces_state_obj = ncx_find_object(
        ietf_interfaces_mod,
        "interfaces-state");
    if (interfaces_state_obj == NULL) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }
    
    return res;
}

status_t y_ietf_interfaces_init2(void)
{
    status_t res;
    cfg_template_t* runningcfg;
    val_value_t* interfaces_state_val;

    res = NO_ERR;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!runningcfg || !runningcfg->root) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    interfaces_state_val = val_find_child(runningcfg->root,
                                          "ietf-interfaces",
                                          "interfaces-state");
    /* Can not coexist with other implementation
     * of ietf-interfaces.
     */
    if(interfaces_state_val!=NULL) {
        log_error("\nError: /interfaces-state already present!");
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    interfaces_state_val = val_new_value();
    if (interfaces_state_val == NULL) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    val_init_virtual(interfaces_state_val,
                     get_interfaces_state,
                     interfaces_state_obj);

    val_add_child(interfaces_state_val, runningcfg->root);

    /* init a root value to store copies of prev state data values */
    root_prev_val = val_new_value();
    val_init_from_template(root_prev_val, runningcfg->root->obj);

    return res;
}

void y_ietf_interfaces_cleanup (void)
{
}
