/*
    module ietf-interfaces
    namespace urn:ietf:params:xml:ns:yang:ietf-interfaces
 */

#include <libxml/xmlstring.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

#define ietf_interfaces_interfaces_state_statistic_in_octets (xmlChar *)"in-octets"
#define ietf_interfaces_interfaces_state_statistic_in_unicast_pkts (xmlChar *)"in-unicast-pkts"
#define ietf_interfaces_interfaces_state_statistic_in_broadcast_pkts (xmlChar *)"in-broadcast-pkts"
#define ietf_interfaces_interfaces_state_statistic_in_multicast_pkts (xmlChar *)"in-multicast-pkts"
#define ietf_interfaces_interfaces_state_statistic_in_discards (xmlChar *)"in-discards"
#define ietf_interfaces_interfaces_state_statistic_in_errors (xmlChar *)"in-errors"
#define ietf_interfaces_interfaces_state_statistic_in_unknown_protos (xmlChar *)"in-unknown-protos"

#define ietf_interfaces_interfaces_state_statistic_out_octets (xmlChar *)"out-octets"
#define ietf_interfaces_interfaces_state_statistic_out_unicast_pkts (xmlChar *)"out-unicast-pkts"
#define ietf_interfaces_interfaces_state_statistic_out_broadcast_pkts (xmlChar *)"out-broadcast-pkts"
#define ietf_interfaces_interfaces_state_statistic_out_multicast_pkts (xmlChar *)"out-multicast-pkts"
#define ietf_interfaces_interfaces_state_statistic_out_discards (xmlChar *)"out-discards"
#define ietf_interfaces_interfaces_state_statistic_out_errors (xmlChar *)"out-errors"

#define ietf_ip (const xmlChar *)"ietf-ip"
/* common const */
#define ietf_ip_common_forwarding (const xmlChar *)"forwarding"
#define ietf_ip_common_mtu (const xmlChar *)"mtu"
#define ietf_ip_common_address_list_container (const xmlChar *)"address"
#define ietf_ip_common_ip (const xmlChar *)"ip"
#define ietf_ip_common_ipv4 (const xmlChar *)"ipv4"
#define ietf_ip_common_ipv6 (const xmlChar *)"ipv6"

/* [RW] */
#define ietf_ip_interfaces_ipv4_address_ip (const xmlChar *)"ip"
#define ietf_ip_interfaces_ipv4_address_netmask (const xmlChar *)"netmask"

#define ietf_ip_interfaces_ipv6_enabled (const xmlChar *)"enabled"
#define ietf_ip_interfaces_ipv6_prefix_length (const xmlChar *)"prefix-length"
#define ietf_ip_interfaces_ipv6_address_container (const xmlChar *)"address"

/* [RO] */
#define ietf_ip_interfaces_state_ipv4_address_netmask (const xmlChar *)"netmask"


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/
/* module static variables */
static val_value_t* with_nmda_param_val;

static status_t
    ietf_interfaces_interface_name_edit(
        ses_cb_t *scb,
        rpc_msg_t *msg,
        agt_cbtyp_t cbtyp,
        op_editop_t editop,
        val_value_t *newval,
        val_value_t *curval)
{
    status_t res;
    val_value_t *errorval = (curval) ? curval : newval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    if (LOGDEBUG) {
        log_debug("\n >>>>> walter: func = ietf_interfaces_interface_name_edit ");
        log_debug("\n cb = %s , op = %s", agt_cbtype_name(cbtyp), op_editop_name(editop));
        log_debug("\n newval is NULL: %s, curval is NULL %s", newval == NULL ? "yes":"no" , curval == NULL?"yes":"no");
        if (newval!=NULL) {
            log_debug("\n newval11 ");
            log_debug("\n newval.name is %s", newval->name);
            log_debug("\n newval22 ");
            log_debug("\n newval = %s",VAL_STRING(newval));
            log_debug("\n newval33 ");
        }
        if (curval!=NULL) {
            log_debug("\n curval11 ");
            log_debug("\n curval = %s",curval->name);
            log_debug("\n curval22 ");
            log_debug("\n curval = %s",VAL_STRING(curval));
            log_debug("\n curval44 ");

        }
        val_dump_value(newval, 0);
        val_dump_value(curval, 0);
    }

    switch (cbtyp) {
    case AGT_CB_VALIDATE:
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        switch (editop) {
        case OP_EDITOP_LOAD:
            break;
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
            if(newval!=NULL) {
                log_debug("\nSetting newval %s\n", VAL_STRING(newval));
                // res = set_api_string_router(private_api_set_time_timezone, VAL_STRING(newval));
                // struct timepb_Config *time_config = malloc(sizeof(*time_config));
                // struct emptypb_Empty *epty = malloc(sizeof(*epty));
                // log_debug("\n whay1 ");
                // time_Time_GetConfig(epty, time_config);
                // log_debug("\n whay2 ");
                // time_config->TimeZone = VAL_STRING(newval);
                // log_debug("\n whay3 ");
                // time_Time_SetConfig(time_config, epty);
            }
            break;
        case OP_EDITOP_CREATE:
            break;
        case OP_EDITOP_DELETE:
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case AGT_CB_ROLLBACK:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    log_debug("\n res is %s", get_error_string(res));
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
    log_debug("\n walter: b4 ietf_interfaces_interface_name_edit return");
    return res;
}




static status_t
    add_interface_entry(val_value_t *parentval,
        struct portpb_ConfigEntry *entry,
        struct networkpb_Config *network_cfg,
        struct networkpb_IPv4Status *ipv4_status,
        struct networkpb_IPv6Status *ipv6_status)
{
    /*objs*/
    obj_template_t* obj;

    /*vals*/
    val_value_t* tmp_val=NULL;
    val_value_t* v4_address_val=NULL;
    val_value_t* v6_address_val=NULL;

    status_t res=NO_ERR;
    val_value_t *childval = NULL;
    boolean done;
    char name[6];
    char* str;
    char* endptr;
    unsigned int i;
    uint64_t counter;
    int ret;


    assert(entry != NULL);

    /* interface/name */
    xmlChar name_as_index[10];
    sprintf(name_as_index, "Port%d", entry->IdentifyNo->PortNo);
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_name,
        name_as_index,
        &res);

    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = val_gen_index_chain(parentval->obj, parentval);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* interface/description */
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_description,
        entry->Alias,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* interface/type */
    xmlChar * if_type= "ethernetCsmacd"; // ethernetCsmacd(6)
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_type,
        if_type,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* interface/enabled */
    const xmlChar * enabled = "false";
    if (entry->PortOperation) {
        enabled = "true";
    }
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_enabled,
        enabled,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* [ietf-ip] interface/ipv4 */
    res = agt_add_container(
        ietf_ip,
        ietf_ip_common_ipv4,
        parentval,
        &childval);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    if (LOGDEBUG) {
        log_debug("\n current childval name: %s", childval->name);
    }

    /* [ietf-ip] interface/ipv4/forwarding */
    xmlChar *is_v4_forarding = "true";
    tmp_val = agt_make_leaf(
            childval->obj,
            ietf_ip_common_forwarding,
            is_v4_forarding,
            &res);
    if (tmp_val != NULL) {
        val_add_child_sorted(tmp_val, childval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* [ietf-ip] interface/ipv4/mtu */
    log_debug("\n mtu is %d", network_cfg->Basic->LocalMTU);
    tmp_val = agt_make_int_leaf(
            childval->obj,
            ietf_ip_common_mtu,
            network_cfg->Basic->LocalMTU,
            &res);
    if (tmp_val != NULL) {
        val_add_child_sorted(tmp_val, childval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    /* [ietf-ip] interface/ipv4/address */
    v4_address_val = agt_make_list(
        childval->obj,
        ietf_ip_common_address_list_container,
        &res);
    if (v4_address_val != NULL) {
        val_add_child(v4_address_val, childval);
    } else if (res!=NO_ERR) {
        return SET_ERROR(res);
    }

    tmp_val = agt_make_leaf(
        v4_address_val->obj,
        ietf_ip_common_ip,
        ipv4_status->OutgoingDeviceIP,
        &res);

    if (tmp_val != NULL) {
        val_add_child_sorted(tmp_val, v4_address_val);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    tmp_val = agt_make_leaf(
        v4_address_val->obj,
        ietf_ip_interfaces_ipv4_address_netmask,
        ipv4_status->DynamicSubnetMask,
        &res);
    if (tmp_val != NULL) {
        val_add_child_sorted(tmp_val, v4_address_val);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    /* [ietf-ip] interface/ipv6 */
    if (network_cfg->IP->V6->Enabled) {
        res = agt_add_container(
            ietf_ip,
            ietf_ip_common_ipv6,
            parentval,
            &childval);
        if (res != NO_ERR) {
            return SET_ERROR(res);
        }

        if (LOGDEBUG) {
            log_debug("\n current childval name: %s", childval->name);
        }

        /* [ietf-ip] interface/ipv6/forwarding */
        xmlChar *is_v6_forarding = "true";
        tmp_val = agt_make_leaf(
                childval->obj,
                ietf_ip_common_forwarding,
                is_v6_forarding,
                &res);
        if (tmp_val != NULL) {
            val_add_child_sorted(tmp_val, childval);
        } else if (res != NO_ERR) {
            return SET_ERROR(res);
        }

        if (ipv6_status->List_Len > 0) {
            /*[ietf-ip] interface/ipv6/address */
            v6_address_val = agt_make_list(
                childval->obj,
                ietf_ip_common_address_list_container,
                &res);
            if (v4_address_val != NULL) {
                val_add_child(v6_address_val, childval);
            } else if (res!=NO_ERR) {
                return SET_ERROR(res);
            }
        }

        for (int i = 0; i<ipv6_status->List_Len;i++) {
            /* split fe80::eade:d6ff:fe00:205/64 =>  fe80::eade:d6ff:fe00:205 and 64*/
            char *addr=malloc(128);
            char *prefix_len=malloc(128);
            char *origin_addr=malloc(128);
            strcpy(origin_addr,ipv6_status->List[i]->IPAddress);
            strcpy(addr, strtok(origin_addr, "/"));
            strcpy(prefix_len, strtok(NULL, "/"));

            /* interface/ipv6/address/ip */
            tmp_val = agt_make_leaf(
                v6_address_val->obj,
                ietf_ip_common_ip,
                addr,
                &res);
            if (tmp_val != NULL) {
                val_add_child_sorted(tmp_val, v6_address_val);
            } else if (res != NO_ERR) {
                return SET_ERROR(res);
            }

            /* interface/ipv6/address/prefix-length */
            tmp_val = agt_make_int_leaf(
                v6_address_val->obj,
                ietf_ip_interfaces_ipv6_prefix_length,
                atoi(prefix_len),
                &res);
            if (tmp_val != NULL) {
                val_add_child_sorted(tmp_val, v6_address_val);
            } else if (res != NO_ERR) {
                return SET_ERROR(res);
            }
            free(addr);
            free(prefix_len);
            free(origin_addr);
        }
    } // end of ipv6_status->Enabled

    return res;
}
static status_t
    add_interface_state_entry(val_value_t *parentval,
        struct portpb_ConfigEntry *entry,
        struct portpb_StatusEntry *status_entry,
        struct devicepb_Info *device_entry,
        struct rmonpb_IngressEntry *ingress_entry,
        struct rmonpb_EgressEntry *egress_entry,
        struct networkpb_Config *network_cfg,
        struct networkpb_IPv4Status *ipv4_status,
        struct networkpb_IPv6Status *ipv6_status)
{
    /*vals*/
    status_t res=NO_ERR;
    val_value_t *tmp_val=NULL;
    val_value_t *childval=NULL;
    val_value_t *v4_address_val=NULL;
    val_value_t *v6_address_val=NULL;
    boolean done;
    log_debug("\n in add_interface_state_entry 1");
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
    assert(entry !=NULL);
    assert(status_entry !=NULL);
    assert(device_entry !=NULL);
    assert(ingress_entry !=NULL);
    assert(egress_entry !=NULL);
    assert(network_cfg !=NULL);
    assert(ipv4_status !=NULL);
    assert(ipv6_status !=NULL);

    log_debug("\n in add_interface_state_entry 2");

    /* /interfaces-state/interface/name */
    char name_as_index[20];
    sprintf(name_as_index, "Port%d", entry->IdentifyNo->PortNo);
    log_debug("\n in add_interface_state_entry 3, name_as_index is %s ", name_as_index);
    const xmlChar *key = name_as_index;
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_name,
        key,
        &res);
    log_debug("\n in add_interface_state_entry 4");

    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }
    log_debug("\n in add_interface_state_entry 5");

    res = val_gen_index_chain(parentval->obj, parentval);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }
    log_debug("\n in add_interface_state_entry 6");

    // [DEPRECATED][FIXME] open this will get Error 258 invalid value, check it later
    // /* /interfaces-state/interface/type */
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

    /* /interfaces-state/interface/oper-state */
    const xmlChar *link_up = "down";
    if (status_entry->LinkUp) {
        link_up = "up";
    }
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_oper_status,
        link_up,
        &res);
    log_debug("\n in add_interface_state_entry 7");
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }
    log_debug("\n in add_interface_state_entry 9");
    /* /interfaces-state/interface/admin-state */
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



    // /** /interfaces-state/interface/last-change
    //     required pattern is
    //     `\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[\+\-]\d{2}:\d{2})`
    // **/
    // // "2022-07-07 08:57:44" => "2022-07-07T08:57:43Z"
    assert(status_entry->LastLinkChange!=NULL);
    char strT[]="T";
    char strZ[]="Z";
    char final[20];
    char *tmp = status_entry->LastLinkChange;
    char *timebuf = strchr(tmp, ' ');
    if (timebuf != NULL) {
        timebuf+=1;
    }
    memset(final, '\0', sizeof(final));
    memcpy(final, tmp, 10); // memcpy will force final char -> char*
    strcat(final, strT);
    strcat(final, timebuf);
    strcat(final, strZ);

    xmlChar *what = final;
    childval = agt_make_leaf(
        parentval->obj,
        ietf_interfaces_interfaces_state_last_change,
        what,
        &res);
    if (childval != NULL) {
        val_add_child_sorted(childval, parentval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    log_debug("\n in add_interface_state_entry after 10");
    log_debug("\n in add_interface_state_entry mac addr is %s", device_entry->MACAddr);
    /* /interfaces-state/interface/phys-address */
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

    /* /interfaces-state/interface/if-index */
    printf("\n portno is %d", status_entry->IdentifyNo->PortNo);
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
    printf("\n speed duplicexed is %d", status_entry->SpeedDuplexUsed);
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


    /* /interfaces-state/interface/speed */
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
    log_debug("\n current childval name after speed is : %s", childval->name);

    /* [ietf-ip] /interfaces-state/interface/ipv4 */
    res = agt_add_container(
        ietf_ip,
        ietf_ip_common_ipv4,
        parentval,
        &childval);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    if (LOGDEBUG) {
        log_debug("\n current childval name: %s", childval->name);
    }

    /* [ietf-ip] /interfaces-state/interface/ipv4/forwarding */
    xmlChar *is_v4_forarding = "true";
    tmp_val = agt_make_leaf(
            childval->obj,
            ietf_ip_common_forwarding,
            is_v4_forarding,
            &res);
    if (tmp_val != NULL) {
        val_add_child_sorted(tmp_val, childval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* [ietf-ip] /interfaces-state/interface/ipv4/mtu */
    log_debug("\n mtu is %d", network_cfg->Basic->LocalMTU);
    tmp_val = agt_make_int_leaf(
            childval->obj,
            ietf_ip_common_mtu,
            network_cfg->Basic->LocalMTU,
            &res);
    if (tmp_val != NULL) {
        val_add_child_sorted(tmp_val, childval);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    /* [ietf-ip] /interfaces-state/interface/ipv4/address */
    v4_address_val = agt_make_list(
        childval->obj,
        ietf_ip_common_address_list_container,
        &res);
    if (v4_address_val != NULL) {
        val_add_child(v4_address_val, childval);
    } else if (res!=NO_ERR) {
        return SET_ERROR(res);
    }

    log_debug("\n outgoingDeviceIP is %s", ipv4_status->OutgoingDeviceIP);
    tmp_val = agt_make_leaf(
        v4_address_val->obj,
        ietf_ip_common_ip,
        ipv4_status->OutgoingDeviceIP,
        &res);

    if (tmp_val != NULL) {
        val_add_child_sorted(tmp_val, v4_address_val);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    log_debug("\n dynmiac subnetmask is %s", ipv4_status->DynamicSubnetMask);
    tmp_val = agt_make_leaf(
        v4_address_val->obj,
        ietf_ip_interfaces_ipv4_address_netmask,
        ipv4_status->DynamicSubnetMask,
        &res);
    log_debug("\n b4 add child %s", ipv4_status->DynamicSubnetMask);
    if (tmp_val != NULL) {
        log_debug("\n b4 add child2 %s", ipv4_status->DynamicSubnetMask);
        val_add_child_sorted(tmp_val, v4_address_val);
        log_debug("\n after add child2 %s", ipv4_status->DynamicSubnetMask);
    } else if (res != NO_ERR) {
        log_debug("\n res != NO_ERR %s", ipv4_status->DynamicSubnetMask);
        return SET_ERROR(res);
    }

    /* [ietf-ip] /interfaces-state/interface/ipv6 */
    if (network_cfg->IP->V6->Enabled) {
        log_debug("\n ipv6 enabled");
        log_debug("\n start add ipv6");
        res = agt_add_container(
            ietf_ip,
            ietf_ip_common_ipv6,
            parentval,
            &childval);
        if (res != NO_ERR) {
            return SET_ERROR(res);
        }

        if (LOGDEBUG) {
            log_debug("\n current childval name: %s", childval->name);
        }

        /* [ietf-ip] /interfaces-state/interface/ipv6/forwarding */
        xmlChar *is_v6_forarding = "true";
        tmp_val = agt_make_leaf(
                childval->obj,
                ietf_ip_common_forwarding,
                is_v6_forarding,
                &res);
        log_debug("\n ipv6 is forwarding %s", is_v6_forarding);
        if (tmp_val != NULL) {
            val_add_child_sorted(tmp_val, childval);
        } else if (res != NO_ERR) {
            return SET_ERROR(res);
        }

        if (ipv6_status->List_Len > 0) {
            /*[ietf-ip] /interfaces-state/interface/ipv6/address */
            v6_address_val = agt_make_list(
                childval->obj,
                ietf_ip_common_address_list_container,
                &res);
            if (v4_address_val != NULL) {
                val_add_child(v6_address_val, childval);
            } else if (res!=NO_ERR) {
                return SET_ERROR(res);
            }
        }

        for (int i = 0; i<ipv6_status->List_Len;i++) {
            /* split fe80::eade:d6ff:fe00:205/64 =>  fe80::eade:d6ff:fe00:205 and 64*/
            log_debug("\n adding ipv6/address/ip");
            char *addr=malloc(128);
            char *prefix_len=malloc(128);
            char *origin_addr=malloc(128);
            strcpy(origin_addr,ipv6_status->List[i]->IPAddress);
            strcpy(addr, strtok(origin_addr, "/"));
            strcpy(prefix_len, strtok(NULL, "/"));

            /* /interfaces-state/interface/ipv6/address/ip */
            tmp_val = agt_make_leaf(
                v6_address_val->obj,
                ietf_ip_common_ip,
                addr,
                &res);
            if (tmp_val != NULL) {
                val_add_child_sorted(tmp_val, v6_address_val);
            } else if (res != NO_ERR) {
                return SET_ERROR(res);
            }

            /* /interfaces-state/interface/ipv6/address/prefix-length */
            tmp_val = agt_make_int_leaf(
                v6_address_val->obj,
                ietf_ip_interfaces_ipv6_prefix_length,
                atoi(prefix_len),
                &res);
            if (tmp_val != NULL) {
                val_add_child_sorted(tmp_val, v6_address_val);
            } else if (res != NO_ERR) {
                return SET_ERROR(res);
            }
            free(addr);
            free(prefix_len);
            free(origin_addr);
        }
    } // end of ipv6_status->Enabled

    /* /interfaces-state/interface/statistics */
    res = agt_add_container(
        ietf_interfaces,
        ietf_interfaces_interfaces_state_statistic,
        parentval,
        &childval);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    log_debug("\n start statistic loop");
    log_debug("\n start statistic loo %u", sizeof(counter_names_array));
    log_debug("\n start statistic loo %u",sizeof(counter_names_array)/sizeof(char*));
    for(int i=0;i<(sizeof(counter_names_array)/sizeof(char*));i++) {
        val_value_t *stats_val = NULL;
        uint64_t target_val;
        xmlChar *counter = counter_names_array[i];
        log_debug("\n start statistic loop , counter name is %s ", counter_names_array[i]);
        log_debug("\n start statistic loop , counter name is %s ", counter);
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

        stats_val = agt_make_uint64_leaf(
            childval->obj,
            counter_names_array[i],
            target_val,
            &res);
        if (stats_val != NULL) {
            val_add_child_sorted(stats_val, childval);
        } else if (res != NO_ERR) {
            if (LOGDEBUG) {
                log_debug("\n[debug] parent name: %s", stats_val->name);
                log_debug("\n[debug] err idx is %d\n", i);
                log_debug("\n[debug] errr counter name is  %s\n", counter_names_array[i]);
            }
            return SET_ERROR(res);
        }
    }
    log_debug("\n res is %s", get_error_string(res));
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
ietf_interfaces_list_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval) {
    status_t res = NO_ERR;

    if (LOGDEBUG) {
        log_debug("\nEnter intri_device_intri_device_port_list_get");
    }

    struct emptypb_Empty *in = malloc(sizeof(*(in)));
    struct portpb_Config *out= malloc(sizeof(*(out)));
    port_Port_GetConfig(in, out);

    struct emptypb_Empty *in6 = malloc(sizeof(*(in6)));
    struct networkpb_IPv4Status *network_v4_status = malloc(sizeof(*(network_v4_status)));
    network_Network_GetV4Status(in6, network_v4_status);

    struct emptypb_Empty *in7 = malloc(sizeof(*(in7)));
    struct networkpb_IPv6Status *network_v6_status = malloc(sizeof(*(network_v6_status)));
    network_Network_GetV6Status(in7, network_v6_status);

    struct emptypb_Empty *in8 = malloc(sizeof(*(in8)));
    struct networkpb_Config *network_cfg = malloc(sizeof(*(network_cfg)));
    network_Network_GetConfig(in8, network_cfg);

    for (int i =0; i < out->List_Len; i++) {
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
        res = add_interface_entry(
            entry_val,
            out->List[i],
            network_cfg,
            network_v4_status,
            network_v6_status);

        if (res != NO_ERR) {
            return SET_ERROR(res);
        }
    }
    free(in);
    free(in6);
    free(in7);
    free(in8);
    free(out);
    free(network_v4_status);
    free(network_v6_status);
    free(network_cfg);
    return res;
}

static status_t
ietf_interfaces_state_list_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval) {
    status_t res = NO_ERR;

    if (LOGDEBUG) {
        log_debug("\nEnter ietf_interfaces_state_list_get");
    }

    struct emptypb_Empty *in = malloc(sizeof(*(in)));
    struct portpb_Config *out= malloc(sizeof(*(out)));
    port_Port_GetConfig(in, out);

    struct emptypb_Empty *in2 = malloc(sizeof(*(in2)));
    struct portpb_Status *status_out = malloc(sizeof(*(status_out)));
    port_Port_GetStatus(in2, status_out);

    struct emptypb_Empty *in3 = malloc(sizeof(*(in3)));
    struct devicepb_Info *device_out = malloc(sizeof(*(device_out)));
    device_Device_GetDeviceInfo(in3, device_out);

    struct devicepb_PortList *in4 = malloc(sizeof(*(in4)));
    struct rmonpb_Ingress *ingress_out = malloc(sizeof(*(ingress_out)));
    struct devicepb_PortList *in5 = malloc(sizeof(*(in5)));
    struct rmonpb_Egress *egress_out = malloc(sizeof(*(egress_out)));

    struct emptypb_Empty *in6 = malloc(sizeof(*(in6)));
    struct networkpb_IPv4Status *network_v4_status = malloc(sizeof(*(network_v4_status)));
    network_Network_GetV4Status(in6, network_v4_status);

    struct emptypb_Empty *in7 = malloc(sizeof(*(in7)));
    struct networkpb_IPv6Status *network_v6_status = malloc(sizeof(*(network_v6_status)));
    network_Network_GetV6Status(in7, network_v6_status);

    struct emptypb_Empty *in8 = malloc(sizeof(*(in8)));
    struct networkpb_Config *network_cfg = malloc(sizeof(*(network_cfg)));
    network_Network_GetConfig(in8, network_cfg);

    in4->List_Len = 30;
    in5->List_Len = 30;
    in4->List = malloc(in4->List_Len * sizeof(*(in4->List)));
    in5->List = malloc(in5->List_Len * sizeof(*(in5->List)));

    log_debug("\nEnter ietf_interfaces_state_list_get4\n");

    for (int i = 0; i<=29 ;i++) {
        in4->List[i] = malloc(sizeof(*(in4->List[i])));
        in4->List[i]->PortNo = i+1;
        in4->List[i]->Type = devicepb_InterfaceTypeOptions_INTERFACE_TYPE_PORT;
        in4->List[i]->DeviceID = 0;

        in5->List[i] = malloc(sizeof(*(in5->List[i])));;
        in5->List[i]->PortNo = i+1;
        in5->List[i]->Type = devicepb_InterfaceTypeOptions_INTERFACE_TYPE_PORT;
        in5->List[i]->DeviceID = 0;
    }
    log_debug("\nEnter ietf_interfaces_state_list_get5\n");

    rmon_RMON_GetIngress(in4, ingress_out);
    rmon_RMON_GetEgress(in5, egress_out);
    log_debug("\nEnter ietf_interfaces_state_list_get6\n");

    for (int i =0; i < out->List_Len; i++) {
        val_value_t *entry_val = NULL;
        log_debug("\nEnter ietf_interfaces_state_list_get6.1, index is %d", i);
        entry_val = agt_make_list(
            dstval->obj,
            ietf_interfaces_interface,
            &res);
        if (entry_val != NULL) {
            val_add_child(entry_val, dstval);
        } else if (res!=NO_ERR) {
            return SET_ERROR(res);
        }
        log_debug("\nEnter ietf_interfaces_state_list_get6.5, entry_val is %s", entry_val->name);
        log_debug("\nEnter ietf_interfaces_state_list_get6.5, out->List[i] is %d", out->List[i]->IdentifyNo->PortNo);
        res = add_interface_state_entry(
            entry_val,
            out->List[i],
            status_out->List[i],
            device_out,
            ingress_out->List[i],
            egress_out->List[i],
            network_cfg,
            network_v4_status,
            network_v6_status);
        fprintf(stderr, "\n walter: done add_interface_state_entry");
        if (res != NO_ERR) {
            return SET_ERROR(res);
        }
    }
    log_debug("\nEnter ietf_interfaces_state_list_get7\n");

    free(out);
    free(status_out);
    free(device_out);
    free(ingress_out);
    free(egress_out);
    free(network_v4_status);
    free(network_v6_status);
    free(network_cfg);

    free(in);
    free(in2);
    free(in3);
    free(in4);
    free(in5);
    free(in6);
    free(in7);
    free(in8);
    return res;
}

static status_t
ietf_interfaces_interfaces_mro(val_value_t *parentval) {
    status_t res = NO_ERR;

    val_init_virtual(
        parentval,
        ietf_interfaces_list_get,
        parentval->obj);
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

    res = ncxmod_load_module(
        "ietf-ip",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);

    res = agt_cb_register_callback(
        "ietf-interfaces",
        (const xmlChar *)"/interfaces/interface/name",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_interfaces_interface_name_edit);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

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

    interfaces_obj = ncx_find_object(
        mod,
        ietf_interfaces_interfaces_container);
    interfaces_val = val_find_child(root_val,
                                    ietf_interfaces,
                                    ietf_interfaces_interfaces_container);

    /* not designed to coexist with other implementations */
    assert(interfaces_state_val==NULL);
    assert(interfaces_val==NULL);
    res = agt_add_top_container(interfaces_state_obj, &interfaces_state_val);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    res = ietf_interfaces_interface_state_mro(interfaces_state_val);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_add_top_container(interfaces_obj, &interfaces_val);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = ietf_interfaces_interfaces_mro(interfaces_val);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }


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

