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


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define ietf_interfaces (const xmlChar *)"ietf-interfaces"
#define ietf_interfaces_interfaces_container (const xmlChar *)"interfaces"
/* /interfaces/interface or /interfaces-state/interface */
#define ietf_interfaces_interface (const xmlChar *)"interface"
#define ietf_interfaces_interfaces_name (const xmlChar *)"name"
#define ietf_interfaces_interfaces_description (const xmlChar *)"description"
#define ietf_interfaces_interfaces_type (const xmlChar *)"type"
#define ietf_interfaces_interfaces_enabled (const xmlChar *)"enabled"
#define ietf_interfaces_interfaces_link_trap (const xmlChar *)"link-up-down-trap-enable"

#define ietf_interfaces_interfaces_state_container (const xmlChar *)"interfaces-state"
#define ietf_interfaces_interfaces_state_name (const xmlChar *)"name"
#define ietf_interfaces_interfaces_state_type (const xmlChar *)"type"
#define ietf_interfaces_interfaces_state_oper_status (const xmlChar *)"oper-status"
#define ietf_interfaces_interfaces_state_admin_status (const xmlChar *)"admin-status"
#define ietf_interfaces_interfaces_state_last_change (const xmlChar *)"last-change"
#define ietf_interfaces_interfaces_state_if_index (const xmlChar *)"if-index"
#define ietf_interfaces_interfaces_state_phy_address (const xmlChar *)"phys-address"
#define ietf_interfaces_interfaces_state_speed (const xmlChar *)"speed"

#define ietf_interfaces_interfaces_state_statistic (const xmlChar *)"statistic"

#define ietf_interfaces_interfaces_state_statistic_in_octets (const xmlChar *)"in-octets"
#define ietf_interfaces_interfaces_state_statistic_in_unicast_pkts (const xmlChar *)"in-unicast-pkts"
#define ietf_interfaces_interfaces_state_statistic_in_broadcast_pkts (const xmlChar *)"in-broadcast-pkts"
#define ietf_interfaces_interfaces_state_statistic_in_multicast_pkts (const xmlChar *)"in-multicast-pkts"
#define ietf_interfaces_interfaces_state_statistic_in_discards (const xmlChar *)"in-discards"
#define ietf_interfaces_interfaces_state_statistic_in_errors (const xmlChar *)"in-errors"
#define ietf_interfaces_interfaces_state_statistic_in_unknown_protos (const xmlChar *)"in-unknown-protos"

#define ietf_interfaces_interfaces_state_statistic_out_octets (const xmlChar *)"out-octets"
#define ietf_interfaces_interfaces_state_statistic_out_unicast_pkts (const xmlChar *)"out-unicast-pkts"
#define ietf_interfaces_interfaces_state_statistic_out_broadcast_pkts (const xmlChar *)"out-broadcast-pkts"
#define ietf_interfaces_interfaces_state_statistic_out_multicast_pkts (const xmlChar *)"out-multicast-pkts"
#define ietf_interfaces_interfaces_state_statistic_out_discards (const xmlChar *)"out-discards"
#define ietf_interfaces_interfaces_state_statistic_out_errors (const xmlChar *)"out-errors"

/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/
/* module static variables */
static val_value_t* root_prev_val;
static val_value_t* with_nmda_param_val;


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
    val_value_t *interface_val;
    val_value_t *name_val;
    interface_val = vir_val->parent;
    assert(interface_val);

    name_val = val_find_child(interface_val,
                              "ietf-interfaces",
                              "name");
    assert(name_val);

    struct emptypb_Empty *in;
    struct portpb_Config *out;
    // struct portpb_Status *out_status;

    // port_Port_GetConfig(in, out);
    // port_Port_GetStatus(in2, out_status);
    // for (int i = 0; i< out->List_Len; i++) {
    //     char *name_as_index[6];
    //     sprintf(name_as_index, "Port%d", out->List[i]->IdentifyNo->PortNo);
    //     if (name_as_index==VAL_STRING(name_val) && out_status->List[i]->IdentifyNo->PortNo==out->List[i]->IdentifyNo->PortNo) {
    //         res = val_set_simval_obj(dst_val,
    //                                 dst_val->obj,
    //                                 (const char *)out_status->List[i]->LastLinkChange);
    //         if (res!=NO_ERR) {
    //             return SET_ERROR(res);
    //         }
    //     }

    // }
    return NO_ERR;
}

static status_t
    get_oper_status(ses_cb_t *scb,
                       getcb_mode_t cbmode,
                       val_value_t *vir_val,
                       val_value_t  *dst_val)
{
    status_t res = NO_ERR;
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

    printf("\n[debug] after calling API in get_oper_status\n");
    struct portpb_StatusEntry out_status;
    struct devicepb_InterfaceIdentify dev_infy;
    printf("[debug] b4 port no\n");
    const char *a = strtok(VAL_STRING(name_val), "Port");
    printf("[debug] portNO is %d\n", atoi(a));
    dev_infy.PortNo = atoi(a);
    dev_infy.Type = devicepb_InterfaceTypeOptions_INTERFACE_TYPE_PORT;
    port_Port_GetPortStatus(&dev_infy, &out_status);

    printf("\n[debug] got port status in get_oper_status %s\n", out_status.LinkUp ? "true" : "false");
    printf("\n[debug] got port enabled in get_oper_status %s\n", out_status.Enabled ? "true" : "false");

    /* check if we have corresponding entry in the oper-status enum */
    strcpy(status_buf,"down");
    sleep(1);
    printf("stats is %s", status_buf);
    // btyp = obj_get_basetype(dst_val->obj);
    // typdef = obj_get_typdef(dst_val->obj);
    // basetypdef = typ_get_base_typdef(typdef);
    // assert(btyp==NCX_BT_ENUM);
    // printf("\n0.1");
    // for (typenum = typ_first_enumdef(basetypdef);
    //      typenum != NULL;
    //      typenum = typ_next_enumdef(typenum)) {
    //         printf("\n0.2");
    //         printf("\ntypeenum %s", typenum->descr);
    //     if(0==strcmp((const char *)typenum->name, status_buf)) {
    //         break;
    //     }
    // }
    printf("\n0.3");
    if(typenum==NULL) {
        printf("Warning: unknown oper-status %s, reporting \"unknown\" instead.\n", status_buf);
        strcpy(status_buf, "unknown");
    }

    printf("\n1");
    res = val_set_simval_obj(dst_val,
                            dst_val->obj,
                            status_buf);
    printf("\n2");
    if (res != NO_ERR) {
        printf("\n3");
        return SET_ERROR(res);
    }
    // oper_status_update(dst_val);

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

/*

Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo: 4307500   49739    0    0    0     0          0         0  4307500   49739    0    0    0     0       0          0
 wlan0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
  eth0: 604474858 23680030    0    0    0     0          0    357073 701580994 6935958    0    0    0     0       1          0
 */

static status_t
    add_interface_state_entry(val_value_t *parentval,
        struct portpb_ConfigEntry *entry,
        struct portpb_StatusEntry *status_entry,
        struct devicepb_Info *device_entry)
{
    /*objs*/
    obj_template_t* interface_state_obj;
    obj_template_t* name_obj;
    obj_template_t* oper_status_obj;
    obj_template_t* last_change_obj;
    obj_template_t* statistics_obj;
    obj_template_t* obj;

    /*vals*/
    val_value_t* interface_state_val;
    val_value_t* name_val;
    val_value_t* oper_status_val;
    val_value_t* last_change_val;
    val_value_t* statistics_val;
    val_value_t* val;

    status_t res=NO_ERR;
    val_value_t *childval = NULL;
    boolean done;
    char name[6];
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

    printf("\n[debug] in add_interface_state_entry\n");
    assert(entry != NULL);
    assert(status_entry != NULL);
    printf("\n[debug] in add_interface_state_entry, portNo of status is %d\n", status_entry->IdentifyNo->PortNo);


    /* interface/name */
    char *name_as_index[6];
    sprintf(name_as_index, "Port%d", entry->IdentifyNo->PortNo);

    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_name,
        name_as_index,
        &res);

    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    // [FIXME] open this will get Error 258 invalid value, check it later
    // /* interface/type */
    // int32_t *if_type = 6; // ethernetCsmacd(6)
    // childval = agt_make_int_leaf(
    //     parentval->obj,
    //     ietf_interfaces_interfaces_state_type,
    //     if_type,
    //     &res);
    // if (childval != NULL) {
    //     val_add_child_sorted(childval, parentval);
    // } else if (res != NO_ERR) {
    //     return SET_ERROR(res);
    // }

    /* interface/oper-state */
    const xmlChar *link_up = "down";
    if (status_entry->LinkUp) {
        link_up = "up";
    }
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_oper_status,
        link_up,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* interface/admin-state */
    const xmlChar * enabled = "down";
    if (status_entry->Enabled) {
        enabled = "up";
    }
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_admin_status,
        link_up,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }



    /** interface/last-change
        required pattern is
        `\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[\+\-]\d{2}:\d{2})`
    **/
    xmlChar *time[20];
    memset(time, 0, 20);
    xmlChar *tmp = strtok(status_entry->LastLinkChange, " ");
    strcat(time, tmp);
    strcat(time, "T");
    tmp = strtok(NULL, " ");
    strcat(time, tmp);
    strcat(time, "Z");
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_last_change,
        time,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* interface/phys-address */
    printf("[debug] mac address is %s\n", device_entry->MACAddr);
    printf("[debug] mac address is %d\n", sizeof(device_entry->MACAddr));
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_phy_address,
        device_entry->MACAddr,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* interface/if-index */
    childval = agt_make_int_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_if_index,
        status_entry->IdentifyNo->PortNo,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    int32 speed_int = 0;
    switch (status_entry->SpeedDuplexUsed) {
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_NA:
            speed_int = 0;
            break;
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_10M_FULL:
            speed_int = 10000000;
            break;
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_100M_FULL:
            speed_int = 100000000;
            break;
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_1000M_FULL:
            speed_int = 1000000000;
            break;
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_2500M_FULL:
            speed_int = 2500000000;
            break;
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_5G_FULL:
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_10G_FULL:
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_25G_FULL:
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_40G_FULL:
        case eventpb_PortSpeedDuplexTypeOptions_PORT_SPEED_DUPLEX_TYPE_100G_FULL:
            speed_int = 4294967295;
            break;
    }


    // /* interface/speed */
     childval = agt_make_int_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_speed,
        speed_int,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    // /* interface/statistics */
    // statistics_obj = obj_find_child(interface_obj,
    //                      "ietf-interfaces",
    //                      "statistics");
    // assert(statistics_obj != NULL);
    // statistics_val = val_new_value();
    // if (statistics_val == NULL) {
    //     return ERR_INTERNAL_MEM;
    // }

    // val_init_from_template(statistics_val, statistics_obj);

    // val_add_child(statistics_val, interface_val);

    // done = FALSE;
    // for(i=0;i<(sizeof(counter_names_array)/sizeof(char*));i++) {
    //     endptr = NULL;
    //     counter = strtoull((const char *)str, &endptr, 10);
    //     if (counter == 0 && str == endptr) {
    //         /* number conversion failed */
    //         log_error("Error: /proc/net/dev number conversion failed.");
    //         return ERR_NCX_OPERATION_FAILED;
    //     }

    //     if(counter_names_array[i]!=NULL) {
    //         obj = obj_find_child(statistics_obj,
    //                              "ietf-interfaces",
    //                              counter_names_array[i]);
    // 	    assert(obj != NULL);

    //         val = val_new_value();
    //         if (val == NULL) {
    //             return ERR_INTERNAL_MEM;
    //         }
    //         val_init_from_template(val, obj);
    //         VAL_UINT64(val) = counter;
    //         val_add_child(val, statistics_val);
    //     }

    //     str = (xmlChar *)endptr;
    //     if (*str == '\0' || *str == '\n') {
    //         break;
    //     }
    // }
    return res;
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
    printf("\n[debug] in update_config\n");
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

static status_t
ietf_interfaces_state_list_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval) {
    status_t res = NO_ERR;

    if (LOGDEBUG) {
        log_debug("\nEnter intri_device_intri_device_port_list_get");
    }

    struct emptypb_Empty in;
    struct portpb_Config out;
    port_Port_GetConfig(&in, &out);

    struct emptypb_Empty in2;
    struct portpb_Status status_out;
    port_Port_GetStatus(&in2, &status_out);

    struct emptypb_Empty in3;
    struct devicepb_Info device_out;
    device_Device_GetDeviceInfo(&in2, &device_out);

    for (int i =0; i < out.List_Len; i++) {
        val_value_t *entry_val = NULL;
        entry_val = agt_make_list(
            dstval->obj,
            ietf_interfaces_interface,
            &res);
        if (entry_val != NULL) {
            val_add_child(entry_val, dstval);
        } else if (res!=NO_ERR) {
            return SET_ERROR(res);
        }

        res = add_interface_state_entry(entry_val, out.List[i], status_out.List[i], &device_out);
        if (res != NO_ERR) {
            return SET_ERROR(res);
        }
    }
    return res;
}

static status_t
ietf_interfaces_interface_state_mro(val_value_t *parentval) {
    status_t res = NO_ERR;

    val_init_virtual(
        parentval,
        ietf_interfaces_state_list_get,
        parentval->obj);
    return res;
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

    printf("@@@@ y_ietf_interfaces_init\n");
    /* check for --with-nmda=true (param defined in netconfd-ex.yang) */
    clivalset = agt_cli_get_valset();
    with_nmda_param_val = val_find_child(clivalset, "netconfd-ex", "with-nmda");

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        ietf_interfaces,
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    assert(res == NO_ERR);

    if(with_nmda_param_val && VAL_BOOL(with_nmda_param_val)) {
        assert(0==strcmp(mod->version,"2018-02-20"));
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
    printf("@@@@ y_ietf_interfaces_init2 \n");

    mod = ncx_find_module(ietf_interfaces, NULL);
    assert(mod);

    if(with_nmda_param_val && VAL_BOOL(with_nmda_param_val)) {
        printf("@@@@ not support nmda\n");
        return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }
    cfg_template_t* runningcfg;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg && runningcfg->root);
    root_val = runningcfg->root;

    interfaces_state_obj = ncx_find_object(
        mod,
        ietf_interfaces_interfaces_state_container);
    assert(interfaces_state_obj);
    interfaces_state_val = val_find_child(root_val,
                                    ietf_interfaces,
                                    ietf_interfaces_interfaces_state_container);


    /* not designed to coexist with other implementations */
    assert(interfaces_state_val==NULL);
    res = agt_add_top_container(interfaces_state_obj, &interfaces_state_val);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    res = ietf_interfaces_interface_state_mro(interfaces_state_val);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

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

