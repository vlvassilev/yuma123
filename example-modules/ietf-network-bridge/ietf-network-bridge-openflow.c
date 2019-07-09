/*
    module ietf-network-bridge-flows
    implementation for OpenFlow device management
    namespace urn:ietf:params:xml:ns:yang:ietf-network-bridge-flows
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


#include <libxml/xmlstring.h>
#include "procdefs.h"
#include "agt.h"
#include "agt_cb.h"
#include "agt_commit_complete.h"
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
#include "val123.h"

/* openvswitch */
#include <openvswitch/vconn.h>
#include <openvswitch/ofp-actions.h>
#include <openvswitch/ofp-parse.h>
#include <openvswitch/ofp-print.h>
#include <openvswitch/ofpbuf.h>
#include <openflow/openflow.h>
#include <lib/ofp-version-opt.h>
#include <lib/flow.h>
#include <lib/socket-util.h>
#include <lib/flow.h>
#include <lib/dp-packet.h>

#define SCHED_STATE_MOD "ietf-network-bridge-scheduler-state"
#define FLOWS_MOD "ietf-network-bridge-flows"

/* module static variables */
static ncx_module_t *ietf_interfaces_mod;
static ncx_module_t *iana_if_type_mod;
static ncx_module_t *ietf_network_bridge_mod;
static ncx_module_t *ietf_network_bridge_scheduler_mod;
static ncx_module_t *ietf_network_bridge_flows_mod;
static ncx_module_t *example_bridge_mod;
static obj_template_t* bridge_obj;

static obj_template_t* packet_received_notification_obj;
static obj_template_t* packet_received_notification_ingress_obj;
static obj_template_t* packet_received_notification_payload_obj;

static struct vconn *vconn;
static int ofp_version;

void transmit_packet(char* if_name, uint8_t* packet_data, unsigned int len);
/* Helper functions */

#include "ovs-ofctl-utils.h"

typedef struct dict_node_t_ {
    dlq_hdr_t      qhdr;
    unsigned int portnum;
    char name[256];
} dict_node_t;


static void dict_init(dlq_hdr_t *que) {
    dlq_createSQue(que);
}
static void dict_clear(dlq_hdr_t *que)
{
    dict_node_t *dn;
    while (!dlq_empty(que)) {
        dn = (dict_node_t *)dlq_deque(que);
        free(dn);
    }
    dlq_createSQue(que);
}

static char* dict_get(dlq_hdr_t *que, unsigned int portnum)
{
    dict_node_t *dn;
    for (dn = (dict_node_t *)dlq_firstEntry(que);
         dn != NULL;
         dn = (dict_node_t *)dlq_nextEntry(dn)) {
        if(dn->portnum==portnum) {
            return dn->name;
        }
    }
    return NULL;
}

static int dict_get_num(dlq_hdr_t *que, const char* name)
{
    dict_node_t *dn;
    for (dn = (dict_node_t *)dlq_firstEntry(que);
         dn != NULL;
         dn = (dict_node_t *)dlq_nextEntry(dn)) {
        if(0==strcmp(dn->name,name)) {
            return dn->portnum;
        }
    }
    return -1;
}

static void dict_add(dlq_hdr_t *que, unsigned int portnum, const char* name)
{
    dict_node_t *dn = malloc(sizeof(dict_node_t));
    assert(dn);
    dn->portnum=portnum;
    strcpy(dn->name,name);
    dlq_enque (dn, que);
}

/* Registered callback functions: get_interfaces */
static dlq_hdr_t port_to_name_dict;

static status_t
     y_transmit_packet_invoke (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        xml_node_t *methnode)
{
    status_t res;
    int ret;
    val_value_t *egress_val;
    val_value_t *payload_val;

    egress_val = val_find_child(
        msg->rpc_input,
        FLOWS_MOD,
        "egress");
    payload_val = val_find_child(
        msg->rpc_input,
        FLOWS_MOD,
        "payload");

    printf("transmit-packet egress=%s\n",VAL_STRING(egress_val));
    transmit_packet(VAL_STRING(egress_val), payload_val->v.binary.ustr, payload_val->v.binary.ustrlen);

    return NO_ERR;

} /* y_transmit_packet_invoke */


static char* make_flow_spec_str(val_value_t* flow_val)
{
    status_t res;
    val_value_t* id_val;
    val_value_t* match_val;
    val_value_t* in_port_val;
    val_value_t* traffic_class_val;
    val_value_t* match_ethernet_source_address_val;
    val_value_t* match_ethernet_destination_address_val;
    val_value_t* match_ethernet_type_val;
    val_value_t* match_vlan_id_val;
    val_value_t* actions_val;
    val_value_t* action_val;
    boolean      add_comma;

    char flow_spec_str[512]=""; /* flow spec str compatible with the ovs-ofctl FLOW format e.g. "in_port=2,actions=output:1" */

    id_val=val_find_child(flow_val,FLOWS_MOD,"id");
    assert(id_val);

    res = xpath_find_val_target(flow_val, NULL/*mod*/, "./match/in-port", &in_port_val);
    assert(res==NO_ERR);
    if(in_port_val) {
        sprintf(flow_spec_str+strlen(flow_spec_str),"in_port=%d",dict_get_num(&port_to_name_dict, VAL_STRING(in_port_val)));
    }

    res = xpath_find_val_target(flow_val, NULL/*mod*/, "./match/ethernet-match/ethernet-destination/address", &match_ethernet_destination_address_val);
    assert(res==NO_ERR);
    if(match_ethernet_destination_address_val) {
        val_value_t* mask_val;
        mask_val = val_find_child(match_ethernet_destination_address_val->parent,"ietf-network-bridge-flows","mask");
        sprintf(flow_spec_str+strlen(flow_spec_str),",dl_dst=%s",VAL_STRING(match_ethernet_destination_address_val));
        if(mask_val) {
        sprintf(flow_spec_str+strlen(flow_spec_str),",/%s",VAL_STRING(mask_val));
        }
    }

    res = xpath_find_val_target(flow_val, NULL/*mod*/, "./match/ethernet-match/ethernet-source/address", &match_ethernet_source_address_val);
    assert(res==NO_ERR);
    if(match_ethernet_source_address_val) {
        val_value_t* mask_val;
        mask_val = val_find_child(match_ethernet_source_address_val->parent,"ietf-network-bridge-flows","mask");
        sprintf(flow_spec_str+strlen(flow_spec_str),",dl_src=%s",VAL_STRING(match_ethernet_source_address_val));
        if(mask_val) {
        sprintf(flow_spec_str+strlen(flow_spec_str),",/%s",VAL_STRING(mask_val));
        }
    }

    res = xpath_find_val_target(flow_val, NULL/*mod*/, "./match/ethernet-match/ethernet-type/type", &match_ethernet_type_val);
    assert(res==NO_ERR);
    if(match_ethernet_type_val) {
        sprintf(flow_spec_str+strlen(flow_spec_str),",dl_type=%u",VAL_UINT32(match_ethernet_type_val));
    }

    res = xpath_find_val_target(flow_val, NULL/*mod*/, "./match/vlan-match/vlan-id/vlan-id", &match_vlan_id_val);
    if(match_vlan_id_val) {
        sprintf(flow_spec_str+strlen(flow_spec_str),",dl_vlan=%u",VAL_UINT32(match_vlan_id_val));
    }


    sprintf(flow_spec_str+strlen(flow_spec_str),",actions=");


    actions_val = val_find_child(flow_val, FLOWS_MOD, "actions");
    add_comma=FALSE;
    for(action_val = val_find_child(actions_val, FLOWS_MOD, "action");
        action_val != NULL;
        action_val = val_find_next_child(actions_val, FLOWS_MOD, "action", action_val)) {
        val_value_t* order_val;
        val_value_t* output_action_val;
        val_value_t* push_vlan_action_val;
        val_value_t* pop_vlan_action_val;

        if(add_comma) {
            sprintf(flow_spec_str+strlen(flow_spec_str),",");
        } else {
            add_comma=TRUE;
        }

        order_val = val_find_child(action_val, FLOWS_MOD, "order");
        assert(order_val);
        output_action_val = val_find_child(action_val, FLOWS_MOD, "output-action");
        push_vlan_action_val = val_find_child(action_val, FLOWS_MOD, "push-vlan-action");
        pop_vlan_action_val = val_find_child(action_val, FLOWS_MOD, "pop-vlan-action");
        if(output_action_val!=NULL) {
            val_value_t* out_port_val;
            out_port_val=val_find_child(output_action_val, FLOWS_MOD, "out-port");
            sprintf(flow_spec_str+strlen(flow_spec_str),"output:%d",dict_get_num(&port_to_name_dict, VAL_STRING(out_port_val)));
        } else if(push_vlan_action_val!=NULL) {
            val_value_t* val;
            val=val_find_child(push_vlan_action_val,FLOWS_MOD,"vlan-id");
            if(val) {
                sprintf(flow_spec_str+strlen(flow_spec_str),"mod_vlan_vid:%u", VAL_UINT16(val));
            }
            val=val_find_child(push_vlan_action_val,FLOWS_MOD,"ethernet-type");
            if(val) {
                sprintf(flow_spec_str+strlen(flow_spec_str),"push_vlan:%u", VAL_UINT32(val));
            }
            val=val_find_child(push_vlan_action_val,FLOWS_MOD,"pcp");
            if(val) {
                sprintf(flow_spec_str+strlen(flow_spec_str),"mod_vlan_pcp:%u", VAL_UINT32(val));
            }
            val=val_find_child(push_vlan_action_val,FLOWS_MOD,"vlan-cfi");
            if(val) {
                /*TODO VAL_UINT32(val)*/
                assert(0);
            }
        } else if(pop_vlan_action_val!=NULL) {
            sprintf(flow_spec_str+strlen(flow_spec_str),"strip_vlan");
        } else {
            assert(0);
        }
    }
    return strdup(flow_spec_str);
}

static void flow_common(val_value_t* flow_val, int delete)
{
    printf("flow_common: %s\n", delete?"delete":"add");
    val_dump_value(flow_val,1);

    struct ofputil_flow_mod fm;
    struct ofpbuf *reply;
    char *error;
    enum ofputil_protocol usable_protocols;
    enum ofputil_protocol protocol;
    char* flow_spec_str;

    protocol = ofputil_protocol_from_ofp_version(ofp_version);

    flow_spec_str = make_flow_spec_str(flow_val);
    error = parse_ofp_flow_mod_str(&fm, flow_spec_str /*for example "in_port=2,actions=output:1" */, delete?OFPFC_DELETE:OFPFC_ADD,
                                       &usable_protocols);
    assert(error==NULL);

    vconn_transact_noreply(vconn, ofputil_encode_flow_mod(&fm, protocol), &reply);
    free(CONST_CAST(struct ofpact *, fm.ofpacts));
    free(flow_spec_str);
}

static void flow_add(val_value_t* flow_val)
{
    flow_common(flow_val, 0 /*delete*/);
}

static void flow_delete(val_value_t* flow_val)
{
    flow_common(flow_val, 1 /*delete*/);
}

static unsigned int get_port_out_action_count(char* if_name, val_value_t* root_val)
{
    status_t res;
    unsigned int match_count=0;
    obj_template_t* out_port_obj;
    val_value_t* out_port_val;

    res = xpath_find_schema_target_int("/flow:flows/flow/actions/action/output-action/out-port",&out_port_obj);
    assert(res==NO_ERR && out_port_obj!=NULL);

    for(out_port_val=val123_get_first_obj_instance(root_val, out_port_obj);
        out_port_val!=NULL;
        out_port_val=val123_get_next_obj_instance(root_val, out_port_val)) {
        if(0==strcmp(VAL_STRING(out_port_val),if_name)) {
            match_count++;
        }
    }
    return match_count;
}

static status_t
    get_flow_statistics(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val);

static int update_config(val_value_t* config_cur_val, val_value_t* config_new_val)
{
    status_t res;

    val_value_t *flows_cur_val, *flow_cur_val;
    val_value_t *flows_new_val, *flow_new_val;


    if(config_new_val == NULL) {
        flows_new_val = NULL;
    } else {
        flows_new_val = val_find_child(config_new_val,
                               FLOWS_MOD,
                               "flows");
    }

    if(config_cur_val == NULL) {
        flows_cur_val = NULL;
    } else {
        flows_cur_val = val_find_child(config_cur_val,
                                       FLOWS_MOD,
                                       "flows");
    }

    /* 2 step (delete/add) flow configuration */

    /* 1. deactivation loop - deletes all deleted or modified flows */
    if(flows_cur_val!=NULL) {
        for (flow_cur_val = val_get_first_child(flows_cur_val);
             flow_cur_val != NULL;
             flow_cur_val = val_get_next_child(flow_cur_val)) {
            flow_new_val = val123_find_match(config_new_val, flow_cur_val);
            if(flow_new_val==NULL || 0!=val_compare_ex(flow_cur_val,flow_new_val,TRUE)) {
                flow_delete(flow_cur_val);
            }
        }
    }

    /* 2. activation loop - adds all new or modified flows */
    if(flows_new_val!=NULL) {
        for (flow_new_val = val_get_first_child(flows_new_val);
             flow_new_val != NULL;
             flow_new_val = val_get_next_child(flow_new_val)) {

            flow_cur_val = val123_find_match(config_cur_val, flow_new_val);
            if(flow_cur_val==NULL || 0!=val_compare_ex(flow_new_val,flow_cur_val,TRUE)) {
                flow_add(flow_new_val);

                /* register flow-statistics */
                obj_template_t* flow_statistics_obj;
                val_value_t* flow_statistics_val;
                flow_statistics_obj = obj_find_child(flow_new_val->obj,
                                   "ietf-network-bridge-flows",
                                   "flow-statistics");
                assert(flow_statistics_obj);

                flow_statistics_val = val_new_value();
                assert(flow_statistics_val);

                val_init_virtual(flow_statistics_val,
                     get_flow_statistics,
                     flow_statistics_obj);
                val_add_child(flow_statistics_val, flow_new_val);
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
    get_interfaces(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res;
    res = NO_ERR;
    int retval;
    struct ofpbuf *request;
    struct ofpbuf *reply;
    int port;

    obj_template_t* interface_obj;
    obj_template_t* name_obj;
    obj_template_t* oper_status_obj;
    obj_template_t* speed_obj;
    obj_template_t* statistics_obj;
    obj_template_t* out_octets_obj;
    obj_template_t* out_discards_obj;
    obj_template_t* in_octets_obj;
    obj_template_t* in_discards_obj;

    val_value_t* interfaces_val;
    val_value_t* interface_val;
    val_value_t* name_val;
    val_value_t* oper_status_val;
    val_value_t* speed_val;
    val_value_t* statistics_val;

    interfaces_val = dst_val;

    res = NO_ERR;
    printf("Called get_interfaces.\n");

    interface_obj = obj_find_child(interfaces_val->obj,
                                   "ietf-interfaces",
                                   "interface");
    assert(interface_obj);

    name_obj = obj_find_child(interface_obj,
                              "ietf-interfaces",
                              "name");
    assert(name_obj);

    oper_status_obj = obj_find_child(interface_obj,
                              "ietf-interfaces",
                              "oper-status");
    assert(oper_status_obj);

    speed_obj = obj_find_child(interface_obj,
                              "ietf-interfaces",
                              "speed");
    assert(speed_obj);

    statistics_obj = obj_find_child(interface_obj,
                                   "ietf-interfaces",
                                   "statistics");
    assert(statistics_obj);

    out_octets_obj = obj_find_child(statistics_obj,
                                   "ietf-interfaces",
                                   "out-octets");
    assert(out_octets_obj);

    in_octets_obj = obj_find_child(statistics_obj,
                                   "ietf-interfaces",
                                   "in-octets");
    assert(in_octets_obj);

    out_discards_obj = obj_find_child(statistics_obj,
                                   "ietf-interfaces",
                                   "out-discards");
    assert(out_discards_obj);

    in_discards_obj = obj_find_child(statistics_obj,
                                   "ietf-interfaces",
                                   "in-discards");
    assert(in_discards_obj);


    struct port_iterator pi;
    struct ofputil_phy_port p;

    for (port_iterator_init(&pi, vconn); port_iterator_next(&pi, &p); ) {
        struct ofputil_port_stats ps;

        printf("[%d] %s\n", p.port_no, p.name);
        //dict_add(&port_to_name_dict, p.port_no, p.name);
        assert(p.port_no==dict_get_num(&port_to_name_dict, p.name));
        interface_val=val_new_value();
        assert(interface_val);
        val_init_from_template(interface_val, interface_obj);
        val_add_child(interface_val, interfaces_val);

        /* name */
        name_val=val_new_value();
        assert(name_val);
        res = val_set_simval_obj(name_val, name_obj, p.name);
        assert(res==NO_ERR);

        val_add_child(name_val, interface_val);

        res = val_gen_index_chain(interface_obj, interface_val);
        assert(res == NO_ERR);

        /* oper-status */
        oper_status_val = val_new_value();
        assert(oper_status_val);
        res = val_set_simval_obj(oper_status_val, oper_status_obj,  p.state==OFPUTIL_PS_LINK_DOWN?"down":"up");
        assert(res==NO_ERR);
        val_add_child(oper_status_val, interface_val);

        /* speed */
        if(p.curr_speed>0 || (p.max_speed>0 && p.state==OFPUTIL_PS_LINK_DOWN)) {
            speed_val=val_new_value();
            assert(speed_val);
            val_init_from_template(speed_val, speed_obj);
            VAL_UINT32(speed_val)=(p.state==OFPUTIL_PS_LINK_DOWN?p.max_speed:p.curr_speed)*1000;
            val_add_child(speed_val, interface_val);
        }
    }
    port_iterator_destroy(&pi);

    request = ofputil_encode_dump_ports_request(vconn_get_version(vconn), /*port*/ OFPP_ANY);
    retval=vconn_transact(vconn, request, &reply);
    assert(retval==0);
    ofp_print(stdout, reply->data, reply->size, 10 + 1);

    for (;;) {
        char* port_name;
        struct ofputil_port_stats ps;

        retval = ofputil_decode_port_stats(&ps, reply);
        if(retval==EOF) {
            break;
        }
        assert(retval==0);

        printf("[%u]", ofp_to_u16(ps.port_no));
        printf("\n");

        printf("rx:\n");
        printf("pkts=%llu\n", ps.stats.rx_packets);
        printf("bytes=%llu\n", ps.stats.rx_bytes);
        printf("drop=%llu\n", ps.stats.rx_dropped);
        printf("errs=%llu\n", ps.stats.rx_errors);
        printf("frame=%llu\n", ps.stats.rx_frame_errors);
        printf("over=%llu\n", ps.stats.rx_over_errors);
        printf("crc=%llu\n", ps.stats.rx_crc_errors);
        printf("\n");

        printf("tx:\n");
        printf("pkts=%llu\n", ps.stats.tx_packets);
        printf("bytes=%llu\n", ps.stats.tx_bytes);
        printf("drop=%llu\n", ps.stats.tx_dropped);
        printf("errs=%llu\n", ps.stats.tx_errors);
        printf("coll=%llu\n", ps.stats.collisions);
        printf("\n");

        if (ps.duration_sec != UINT32_MAX) {
            printf("%10u.%09u\n", ps.duration_sec, ps.duration_nsec);
        }

        port_name=dict_get(&port_to_name_dict, ps.port_no);
        assert(port_name);

        for (interface_val = val_get_first_child(interfaces_val);
             interface_val != NULL;
             interface_val = val_get_next_child(interface_val)) {
            val_value_t* val;

            name_val = val_find_child(interface_val,
                                     "ietf-interfaces",
                                     "name");
            if(0!=strcmp(VAL_STRING(name_val),port_name)) {
                continue;
            }

            /* statistics */
            statistics_val=val_new_value();
            assert(statistics_val);
            val_init_from_template(statistics_val, statistics_obj);
            val_add_child(statistics_val, interface_val);

            /* out-octets */
            val=val_new_value();
            assert(val);
            val_init_from_template(val, out_octets_obj);
            VAL_UINT64(val)=ps.stats.tx_bytes;
            val_add_child(val, statistics_val);

            /* out-discards */
            val=val_new_value();
            assert(val);
            val_init_from_template(val, out_discards_obj);
            VAL_UINT64(val)=ps.stats.tx_dropped;
            val_add_child(val, statistics_val);

            /* in-octets */
            val=val_new_value();
            assert(val);
            val_init_from_template(val, in_octets_obj);
            VAL_UINT64(val)=ps.stats.rx_bytes;
            val_add_child(val, statistics_val);

            /* in-discards */
            val=val_new_value();
            assert(val);
            val_init_from_template(val, in_discards_obj);
            VAL_UINT64(val)=ps.stats.rx_dropped;
            val_add_child(val, statistics_val);
        }
    }
    ofpbuf_delete(reply);


    return res;
}

static status_t
    get_flow_statistics(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    int ret;
    status_t res;
    res = NO_ERR;
    int retval;
    struct ofpbuf *request;
    struct ofpbuf *reply;
    int port;
    enum ofputil_protocol protocol;

    obj_template_t* obj;
    val_value_t* val;

    val_value_t* flow_statistics_val;

    flow_statistics_val = dst_val;

    res = NO_ERR;
    printf("Called get_flow_statistics.\n");


    struct ofputil_flow_stats fs;
    struct ofpbuf ofpacts;
    ovs_be32 send_xid;
    ovs_be32 recv_xid;
    struct ofputil_flow_stats_request fsr;
    char* flow_spec_str;
    enum ofputil_protocol usable_protocols;
    char* error;

    /* fill the protocol independent flow stats request struct */
    flow_spec_str = make_flow_spec_str(flow_statistics_val->parent);

    struct ofputil_flow_mod fm;
    error = parse_ofp_flow_mod_str(&fm, flow_spec_str /*for example "in_port=2,actions=output:1" */, OFPFC_ADD /* not important */,
                                       &usable_protocols);
    assert(error==NULL);
    free(flow_spec_str);

    fsr.aggregate = false;
    //match_init_catchall(&fsr.match);
    fsr.match=fm.match;
    fsr.out_port = OFPP_ANY;
    fsr.out_group = OFPG_ANY;
    fsr.table_id = 0xff;
    fsr.cookie = fsr.cookie_mask = htonll(0);

    /* generate protocol specific request */
    protocol = ofputil_protocol_from_ofp_version(ofp_version);
    request = ofputil_encode_flow_stats_request(&fsr, protocol);

    send_xid = ((struct ofp_header *) request->data)->xid;
    vconn_send_block(vconn, request);
    ret = vconn_recv_xid(vconn, send_xid, &reply);
    assert(ret==0);

    /* Pull an individual flow stats reply out of the message. */
    ofpbuf_init(&ofpacts, 0);
    retval = ofputil_decode_flow_stats_reply(&fs, reply, false, &ofpacts);
    assert(retval==0);

    ofpbuf_uninit(&ofpacts);
    ofpbuf_delete(reply);

    /* packet-count */
    obj = obj_find_child(flow_statistics_val->obj,
                         "ietf-network-bridge-flows",
                         "packet-count");
    assert(obj);

    val=val_new_value();
    assert(val);
    val_init_from_template(val, obj);
    VAL_UINT64(val)=fs.packet_count;
    val_add_child(val, flow_statistics_val);

    /* byte-count */
    obj = obj_find_child(flow_statistics_val->obj,
                         "ietf-network-bridge-flows",
                         "byte-count");
    assert(obj);

    val=val_new_value();
    assert(val);
    val_init_from_template(val, obj);
    VAL_UINT64(val)=fs.byte_count;
    val_add_child(val, flow_statistics_val);


    return res;
}

static void
process_echo_request(const struct ofp_header *rq)
{
    struct ofpbuf *reply;
    reply=make_echo_reply(rq);
    printf("process_echo_request send reply len=%d\n", reply->size);
    ofp_print(stdout, reply->data, reply->size, 10 + 1);
    vconn_send(vconn, reply);
}

static void send_packet_received_notification(char* ingress, uint8_t* payload_buf, uint32_t payload_len)
{
    status_t res;
    agt_not_msg_t *notif;
    val_value_t* ingress_val;
    val_value_t* payload_val;

    notif = agt_not_new_notification(packet_received_notification_obj);
    assert (notif != NULL);

    /* add params to payload */

    /* ingress */
    ingress_val = val_new_value();
    assert(ingress_val);

    res = val_set_simval_obj(ingress_val,
                         packet_received_notification_ingress_obj,
                         ingress);
    agt_not_add_to_payload(notif, ingress_val);

    /* payload */
    payload_val = val_new_value();
    assert(payload_val);

    val_init_from_template(payload_val, packet_received_notification_payload_obj);

    payload_val->v.binary.ustr = malloc(payload_len);
    assert(payload_val->v.binary.ustr);
    memcpy(payload_val->v.binary.ustr,payload_buf,payload_len);
    payload_val->v.binary.ustrlen = payload_len;

    agt_not_add_to_payload(notif, payload_val);

    agt_not_queue_notification(notif);
}

static void
process_packet_in(const struct ofp_header *msg)
{
    struct ofputil_packet_in pin;
    struct ofpbuf continuation;
    enum ofperr error = ofputil_decode_packet_in(msg, true, &pin,
                                                 NULL, NULL, &continuation);

    assert(error==0);

    //assert(pin.reason==OFPR_ACTION);

    struct ofpbuf userdata = ofpbuf_const_initializer(pin.userdata,
                                                      pin.userdata_len);
    //const struct action_header *ah = ofpbuf_pull(&userdata, sizeof(*ah));
    //assert(ah);

    struct dp_packet packet;
    dp_packet_use_const(&packet, pin.packet, pin.packet_len);
    struct flow headers;
    flow_extract(&packet, &headers);


    char* ingress;
    uint8_t* payload_buf;
    uint32_t payload_len;
    send_packet_received_notification(dict_get(&port_to_name_dict, pin.flow_metadata.flow.in_port.ofp_port), pin.packet, pin.packet_len);
}

void transmit_packet(char* if_name, uint8_t* packet_data, unsigned int len)
{
    struct ofputil_packet_out po;
    struct ofpbuf ofpacts;
    struct dp_packet *packet;
    struct ofpbuf *opo;
    const char *error_msg;
    struct ofpbuf *reply;
    enum ofputil_protocol usable_protocols;
    enum ofputil_protocol protocol;
    char action_str[]="output:2147483648";

    protocol = ofputil_protocol_from_ofp_version(ofp_version);

    ofpbuf_init(&ofpacts, 64);
    sprintf(action_str,"output:%u",dict_get_num(&port_to_name_dict, if_name));
    error_msg = ofpacts_parse_actions(action_str, &ofpacts, &usable_protocols);
    assert(error_msg==NULL);

    assert(usable_protocols&protocol);

    po.buffer_id = UINT32_MAX;
    po.in_port = OFPP_NONE;//str_to_port_no(ctx->argv[1], ctx->argv[2]);
    po.ofpacts = ofpacts.data;
    po.ofpacts_len = ofpacts.size;

#if 0
    error_msg = eth_from_hex("6CA96F0000026CA96F00000108004500002ED4A500000A115816C0000201C0000202C0200007001A00000102030405060708090A0B0C0D0E0F101112", &packet);
    assert(error_msg==0);
#else
    packet=dp_packet_new(len);
    dp_packet_put(packet, (const void *) packet_data, len);
#endif

    po.packet = dp_packet_data(packet);
    po.packet_len = dp_packet_size(packet);
    ofp_version = vconn_get_version(vconn);
    opo = ofputil_encode_packet_out(&po, protocol);
    //vconn_transact_noreply(vconn, opo, &reply);
    vconn_send_block(vconn, opo);
    dp_packet_delete(packet);
    ofpbuf_uninit(&ofpacts);
}

int network_bridge_timer(uint32 timer_id, void *cookie)
{
    unsigned int i;
    int retval=0;
    struct ofpbuf *msg;
    struct ofpbuf *request;
    struct ofpbuf *reply;
    enum ofptype type;

    printf("In network_bridge_timer.\n");
    for(i=0;retval==0;i++) {
        retval = vconn_recv(vconn, &msg);
        if(retval == EAGAIN) {
            break;
        } else if(retval!=0) {
            assert(0);
        }
        if(i<10) {
            ofp_print(stdout, msg->data, msg->size, 10 + 1);
        }

        if(ofptype_pull(&type, msg)) {
            assert(0);
        }

        if (type == OFPTYPE_ECHO_REQUEST) {
            process_echo_request(msg->header);
        } else if (type == OFPTYPE_PACKET_IN) {
            if(i<10) {
                process_packet_in(msg->header);
            }
        } else if (type == OFPTYPE_FLOW_REMOVED) {
            /* Nothing to do. */
            assert(0);
        }

        ofpbuf_delete(msg);

    }
    if(i>1) {
        printf("OpenFlow notifications processed: %u\n", i);
    }
    printf("Out network_bridge_timer.\n");
    return 0;
}

/* The 3 mandatory callback functions: y_ietf_network_bridge_openflow_init, y_ietf_network_bridge_openflow_init2, y_ietf_network_bridge_openflow_cleanup */

status_t
    y_ietf_network_bridge_openflow_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t* agt_profile;
    obj_template_t* flows_obj;
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
    res = ncxmod_load_module(
        "iana-if-type",
        NULL,
        &agt_profile->agt_savedevQ,
        &iana_if_type_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-network-bridge",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_network_bridge_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-network-bridge",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_network_bridge_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-network-bridge-flows",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_network_bridge_flows_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-network-bridge-scheduler",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_network_bridge_scheduler_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "example-bridge",
        NULL,
        &agt_profile->agt_savedevQ,
        &example_bridge_mod);
    if (res != NO_ERR) {
        return res;
    }

    /* packet-received notification */
    packet_received_notification_obj = ncx_find_object(
        ietf_network_bridge_flows_mod,
        "packet-received");
    assert(packet_received_notification_obj);  

    packet_received_notification_ingress_obj = obj_find_child(packet_received_notification_obj,
                      FLOWS_MOD,  
                      "ingress");
    assert(packet_received_notification_ingress_obj);
    
    packet_received_notification_payload_obj = obj_find_child(packet_received_notification_obj,
                      FLOWS_MOD,
                      "payload");
    assert(packet_received_notification_payload_obj);


    flows_obj = ncx_find_object(
        ietf_network_bridge_flows_mod,
        "flows");
    assert(flows_obj != NULL);

    res=agt_commit_complete_register("ietf-network-bridge-openflow" /*SIL id string*/,
                                     y_commit_complete);
    assert(res == NO_ERR);

    res = agt_rpc_register_method(
        "ietf-network-bridge-flows",
        "transmit-packet",
        AGT_RPC_PH_INVOKE,
        y_transmit_packet_invoke);
    assert(res == NO_ERR);

    /* Wait for connection from OpenFlow node */
    {
        int retval;
        struct pvconn *pvconn;
        char* vconn_arg;

        vconn_arg=getenv("VCONN_ARG");
        if(vconn_arg==NULL) {
            vconn_arg="ptcp:16635";
        }

        printf("Listen on %s ...\n", vconn_arg);
        retval = pvconn_open(vconn_arg, /*get_allowed_ofp_versions()*/(1u << OFP10_VERSION), DSCP_DEFAULT,
                            &pvconn);

        printf("Waiting for connection");

        while(1) {
            retval = pvconn_accept(pvconn, &vconn);
            if(retval == EAGAIN) {
                printf(".");
                fflush(stdout);
                sleep(1);
                continue;
            } else if (retval==0) {
                printf("Connected retval=%d.\n", retval);
                //new_switch(&switches[n_switches++], vconn);
                break;
            } else {
                assert(0);
            }
        }

        pvconn_close(pvconn);

        retval = vconn_connect_block(vconn);
        assert(retval==0);

        struct ofpbuf *request;
        struct ofpbuf *reply;

        ofp_version = vconn_get_version(vconn);
    
        assert(ofp_version > 0 && ofp_version < 0xff);
         
        /* Send OFPT_FEATURES_REQUEST. */
        request = ofpraw_alloc(OFPRAW_OFPT_FEATURES_REQUEST, ofp_version, 0);
        retval=vconn_transact(vconn, request, &reply);
        assert(retval==0);

        ofp_print(stdout, reply->data, reply->size, 10 + 1);
        ofpbuf_delete(reply);

        dict_init(&port_to_name_dict);
        struct port_iterator pi;
        struct ofputil_phy_port p;

        for (port_iterator_init(&pi, vconn); port_iterator_next(&pi, &p); ) {
            struct ofputil_port_stats ps;

            printf("[%d] %s\n", p.port_no, p.name);
            dict_add(&port_to_name_dict, p.port_no, p.name);
        }
        port_iterator_destroy(&pi);
    }

    return res;
}

status_t y_ietf_network_bridge_openflow_init2(void)
{
    status_t res;
    cfg_template_t* runningcfg;
    ncx_module_t* mod;
    obj_template_t* interfaces_obj;
    val_value_t* root_val;
    val_value_t* interfaces_val;
    uint32 timer_id;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg && runningcfg->root);
    root_val = runningcfg->root;

    res = NO_ERR;

    mod = ncx_find_module("ietf-interfaces", NULL);
    assert(mod);

    interfaces_obj = ncx_find_object(
        mod,
        "interfaces-state");
    assert(interfaces_obj);
    interfaces_val = val_find_child(root_val,
                                    "ietf-interfaces",
                                    "interfaces-state");

    /* not designed to coexist with other implementations */
    assert(interfaces_val==NULL);

    interfaces_val = val_new_value();
    assert(interfaces_val);

    val_init_virtual(interfaces_val,
                     get_interfaces,
                     interfaces_obj);

    val_add_child(interfaces_val, root_val);

    res = agt_timer_create(1/* 1 sec period */,
                           TRUE/*periodic*/,
                           network_bridge_timer,
                           interfaces_val/*cookie*/,
                           &timer_id);

    return res;
}

void y_ietf_network_bridge_openflow_cleanup (void)
{
}

