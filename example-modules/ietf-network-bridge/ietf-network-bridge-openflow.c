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
static uint32 timer_id;

static status_t
     y_ietf_network_bridge_flows_transmit_packet (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        xml_node_t *methnode)
{
    return NO_ERR;
}

static status_t
    y_ietf_network_bridge_flows_edit (
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
            /*TODO*/
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

    request = ofputil_encode_port_desc_stats_request(vconn_get_version(vconn), /*port*/ OFPP_ANY);
    retval=vconn_transact(vconn, request, &reply);
    assert(retval==0);
    ofp_print(stdout, reply->data, reply->size, 10 + 1);
    ofpbuf_delete(reply);

    struct port_iterator pi;
    struct ofputil_phy_port p;

    dict_clear(&port_to_name_dict);

    for (port_iterator_init(&pi, vconn); port_iterator_next(&pi, &p); ) {
        struct ofputil_port_stats ps;

        printf("[%d] %s\n", p.port_no, p.name);
        dict_add(&port_to_name_dict, p.port_no, p.name);

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
    send_packet_received_notification(dict_get(&port_to_name_dict, 6), pin.packet, pin.packet_len);
}

int network_bridge_timer(uint32 timer_id, void *cookie)
{
    int retval;
    struct ofpbuf *msg;
    struct ofpbuf *request;
    struct ofpbuf *reply;
    enum ofptype type;

    printf("In network_bridge_timer.\n");
    do {
        retval = vconn_recv(vconn, &msg);
        if(retval == EAGAIN) {
            break;
        } else if(retval!=0) {
            assert(0);
        }
        ofp_print(stdout, msg->data, msg->size, 10 + 1);

        if(ofptype_pull(&type, msg)) {
            assert(0);
        }

        if (type == OFPTYPE_ECHO_REQUEST) {
            process_echo_request(msg->header);
        } else if (type == OFPTYPE_PACKET_IN) {
            process_packet_in(msg->header);
        } else if (type == OFPTYPE_FLOW_REMOVED) {
            /* Nothing to do. */
            assert(0);
        }

        ofpbuf_delete(msg);

    } while(retval==0);

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

    res = agt_cb_register_callback(
        "ietf-network-bridge-flows",
        (const xmlChar *)"/flows",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        y_ietf_network_bridge_flows_edit);
    assert(res == NO_ERR);

    res = agt_rpc_register_method(
        "ietf-network-bridge-flows",
        "transmit-packet",
        AGT_RPC_PH_INVOKE,
        y_ietf_network_bridge_flows_transmit_packet);
    assert(res == NO_ERR);

    /* Wait for connection from OpenFlow node */
    {
        int retval;
        struct pvconn *pvconn;

        printf("Listen on %s ...\n", "ptcp:16635");
        retval = pvconn_open("ptcp:16635", get_allowed_ofp_versions(), DSCP_DEFAULT,
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

        int ofp_version = vconn_get_version(vconn);
    
        assert(ofp_version > 0 && ofp_version < 0xff);
         
        /* Send OFPT_FEATURES_REQUEST. */
        request = ofpraw_alloc(OFPRAW_OFPT_FEATURES_REQUEST, ofp_version, 0);
        retval=vconn_transact(vconn, request, &reply);
        assert(retval==0);

        ofp_print(stdout, reply->data, reply->size, 10 + 1);
        ofpbuf_delete(reply);

        dict_init(&port_to_name_dict);
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

