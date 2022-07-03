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

#define ietf_interfaces_interfaces_state_statistic (const xmlChar *)"statistics"

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
static val_value_t* with_nmda_param_val;





static status_t
    add_interface_state_entry(val_value_t *parentval,
        struct portpb_ConfigEntry *entry,
        struct portpb_StatusEntry *status_entry,
        struct devicepb_Info *device_entry,
        struct rmonpb_IngressEntry *ingress_entry,
        struct rmonpb_EgressEntry *egress_entry)
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


// #define ietf_interfaces_interfaces_state_statistic_in_octets (const xmlChar *)"in-octets"
// #define ietf_interfaces_interfaces_state_statistic_in_unicast_pkts (const xmlChar *)"in-unicast-pkts"
// #define ietf_interfaces_interfaces_state_statistic_in_broadcast_pkts (const xmlChar *)"in-broadcast-pkts"
// #define ietf_interfaces_interfaces_state_statistic_in_multicast_pkts (const xmlChar *)"in-multicast-pkts"
// #define ietf_interfaces_interfaces_state_statistic_in_discards (const xmlChar *)"in-discards"
// #define ietf_interfaces_interfaces_state_statistic_in_errors (const xmlChar *)"in-errors"
// #define ietf_interfaces_interfaces_state_statistic_in_unknown_protos (const xmlChar *)"in-unknown-protos"

// #define ietf_interfaces_interfaces_state_statistic_out_octets (const xmlChar *)"out-octets"
// #define ietf_interfaces_interfaces_state_statistic_out_unicast_pkts (const xmlChar *)"out-unicast-pkts"
// #define ietf_interfaces_interfaces_state_statistic_out_broadcast_pkts (const xmlChar *)"out-broadcast-pkts"
// #define ietf_interfaces_interfaces_state_statistic_out_multicast_pkts (const xmlChar *)"out-multicast-pkts"
// #define ietf_interfaces_interfaces_state_statistic_out_discards (const xmlChar *)"out-discards"
// #define ietf_interfaces_interfaces_state_statistic_out_errors (const xmlChar *)"out-errors"

    char* counter_names_array[12] = {
        ietf_interfaces_interfaces_state_statistic_in_octets,
        ietf_interfaces_interfaces_state_statistic_in_unicast_pkts,
        ietf_interfaces_interfaces_state_statistic_in_broadcast_pkts,
        ietf_interfaces_interfaces_state_statistic_in_multicast_pkts,
        ietf_interfaces_interfaces_state_statistic_in_discards,
        ietf_interfaces_interfaces_state_statistic_in_errors,
        ietf_interfaces_interfaces_state_statistic_out_octets,
        ietf_interfaces_interfaces_state_statistic_out_unicast_pkts,
        ietf_interfaces_interfaces_state_statistic_out_broadcast_pkts,
        ietf_interfaces_interfaces_state_statistic_out_multicast_pkts,
        ietf_interfaces_interfaces_state_statistic_out_discards,
        ietf_interfaces_interfaces_state_statistic_out_errors,
    };
    assert(entry != NULL);
    assert(status_entry != NULL);


    /* interface/name */
    xmlChar *name_as_index[10];
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
    res = agt_add_container(
        ietf_interfaces,
        ietf_interfaces_interfaces_state_statistic,
        parentval,
        &childval);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    // done = FALSE;
    log_debug("\nchildval name: %s", childval->name);
    for(int i=0;i<(sizeof(counter_names_array)/sizeof(char*));i++) {
        val_value_t *stats_val = NULL;
        uint64_t target_val;
        xmlChar *counter = counter_names_array[i];

        log_debug("\nidx: %d", i);
        log_debug("\n port number is %d", &ingress_entry->IdentifyNo->PortNo);
        log_debug("\n port number is %d", ingress_entry->IdentifyNo->PortNo);
        log_debug("\nchildval name: %s", childval->name);
        log_debug("\ncounter name: %s", counter_names_array[i]);
        log_debug("\ncounter equal %s\n", counter == ietf_interfaces_interfaces_state_statistic_in_octets ? "true": "false");
        log_debug("\ncounter goodOctets %d\n", ingress_entry->InGoodOctets);
        log_debug("\ncounter badOctets %d\n", ingress_entry->InBadOctets);
        log_debug("\ncounter badOctets %d\n", ingress_entry->InBroadcasts);
        if (counter == ietf_interfaces_interfaces_state_statistic_in_octets) {
            target_val = ingress_entry->InGoodOctets + ingress_entry->InBadOctets;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_in_broadcast_pkts) {
            target_val = ingress_entry->InBroadcasts;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_in_discards) {
            target_val = ingress_entry->InDiscarded;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_in_errors) {
            target_val = ingress_entry->InTotalReceiveErrors;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_in_multicast_pkts) {
            target_val = ingress_entry->InMulticasts;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_in_unicast_pkts) {
            target_val = ingress_entry->InUnicasts;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_out_broadcast_pkts) {
            target_val = egress_entry->OutBroadcasts;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_out_errors) {
            target_val = egress_entry->OutDeferred+egress_entry->OutTotalCollisions;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_out_multicast_pkts) {
            target_val = egress_entry->OutMulticasts;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_out_octets) {
            target_val = egress_entry->OutGoodOctets;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_out_unicast_pkts) {
            target_val = egress_entry->OutUnicasts;
        } else if (counter == ietf_interfaces_interfaces_state_statistic_out_discards) {
            target_val = egress_entry->OutDroppedPackets;
        }
        log_debug("\ncounter equal4 %s\n", counter == ietf_interfaces_interfaces_state_statistic_in_octets ? "true": "false");

        stats_val = agt_make_uint64_leaf(
            childval->obj,
            counter_names_array[i],
            target_val,
            &res);
        log_debug("\nstats_val name2: %s", stats_val->name);
        if (stats_val != NULL) {
            val_add_child(stats_val, childval);
        } else if (res != NO_ERR) {
            if (LOGDEBUG) {
                log_debug("\n[debug] parent name: %s", stats_val->name);
                log_debug("\n[debug] err idx is %d\n", i);
                log_debug("\n[debug] errr counter name is  %s\n", counter_names_array[i]);
            }
            return SET_ERROR(res);
        }
    }
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

    struct devicepb_PortList in4;
    struct rmonpb_Ingress ingress_out;
    struct devicepb_PortList in5;
    struct rmonpb_Egress egress_out;
    in4.List_Len = 30;
    in5.List_Len = 30;
    in4.List = malloc(in4.List_Len * sizeof(*(in4.List)));
    in5.List = malloc(in5.List_Len * sizeof(*(in5.List)));

    for (int i = 0; i< in4.List_Len;i++) {
        struct devicepb_InterfaceIdentify *tmp = malloc(sizeof(*(tmp)));
        struct devicepb_InterfaceIdentify *tmp2 = malloc(sizeof(*(tmp2)));
        tmp->PortNo = i+1;
        tmp->Type = devicepb_InterfaceTypeOptions_INTERFACE_TYPE_PORT;
        tmp->DeviceID = 0;
        tmp->LAGNo = 0;

        tmp2->PortNo = i+1;
        tmp2->Type = devicepb_InterfaceTypeOptions_INTERFACE_TYPE_PORT;
        tmp2->DeviceID = 0;
        tmp2->LAGNo = 0;
        log_debug("\nprinting list1 %d\n", i);
        in4.List[i] = tmp;
        log_debug("\nprinting list2 %d\n", i);
        in5.List[i] = tmp2;
        free(tmp);
        free(tmp2);
    }

    rmon_RMON_GetIngress(&in4, &ingress_out);
    rmon_RMON_GetEgress(&in5, &egress_out);

    log_debug("\nlen of ingeress_out %d\n", ingress_out.List_Len);
    log_debug("\nlen of out_list_len %d\n", out.List_Len);
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
        res = add_interface_state_entry(entry_val, out.List[i], status_out.List[i], &device_out, ingress_out.List[i], egress_out.List[i]);
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
    if (LOGDEBUG) {
        log_debug("@@@@ y_ietf_interfaces_init\n");
    }
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
    if (LOGDEBUG) {
        log_debug("@@@@ y_ietf_interfaces_init2 \n");
    }

    mod = ncx_find_module(ietf_interfaces, NULL);
    assert(mod);

    if(with_nmda_param_val && VAL_BOOL(with_nmda_param_val)) {
        if (LOGDEBUG) {
            log_debug("@@@@ not support nmda\n");
        }
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

