/*
    module test-ietf-interfaces-bis
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
#include "agt_rpc.h"
#include "dlq.h"
#include "ncx.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "status.h"
#include "rpc.h"
#include "val.h"
#include "val123.h"
#include "agt_nmda.h"

/* module static variables */
static ncx_module_t *ietf_interfaces_mod;
static ncx_module_t *ietf_ip_mod;
static ncx_module_t *ietf_interfaces_mod;
static obj_template_t* interfaces_obj;

/* Registered callback functions: get_system_cfg_interfaces */

static status_t
    get_system_cfg_interfaces(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res = NO_ERR;
    res = val_set_cplxval_obj(dst_val,dst_val->obj,"<interfaces\
           xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\"\
           xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\"\
           xmlns:vlan=\"http://example.com/vlan\"\
           xmlns:or=\"urn:ietf:params:xml:ns:yang:ietf-origin\">\
         <interface>\
           <name>eth0</name>\
           <type>ianaift:ethernetCsmacd</type>\
           <enabled>false</enabled>\
           <admin-status>down</admin-status>\
           <oper-status>down</oper-status>\
           <if-index>2</if-index>\
           <phys-address>00:01:02:03:04:05</phys-address>\
           <statistics>\
             <discontinuity-time>\
               2013-04-01T03:00:00+00:00\
             </discontinuity-time>\
             <!-- counters now shown here -->\
           </statistics>\
         </interface>\
         <interface>\
           <name>eth1</name>\
           <type>ianaift:ethernetCsmacd</type>\
           <enabled>true</enabled>\
           <admin-status>up</admin-status>\
           <oper-status>up</oper-status>\
           <if-index>7</if-index>\
           <phys-address>00:01:02:03:04:06</phys-address>\
           <higher-layer-if>eth1.10</higher-layer-if>\
           <statistics>\
             <discontinuity-time>\
               2013-04-01T03:00:00+00:00\
             </discontinuity-time>\
             <!-- counters now shown here -->\
           </statistics>\
           <vlan:vlan-tagging>true</vlan:vlan-tagging>\
         </interface>\
         <interface>\
           <name>eth1.10</name>\
           <type>ianaift:l2vlan</type>\
           <enabled>true</enabled>\
           <admin-status>up</admin-status>\
           <oper-status>up</oper-status>\
           <if-index>9</if-index>\
           <lower-layer-if>eth1</lower-layer-if>\
           <statistics>\
             <discontinuity-time>\
               2013-04-01T03:00:00+00:00\
             </discontinuity-time>\
             <!-- counters now shown here -->\
           </statistics>\
           <vlan:base-interface>eth1</vlan:base-interface>\
           <vlan:vlan-id>10</vlan:vlan-id>\
         </interface>\
         <!-- This interface is not configured -->\
         <interface>\
           <name>eth2</name>\
           <type>ianaift:ethernetCsmacd</type>\
           <admin-status>down</admin-status>\
           <oper-status>down</oper-status>\
           <if-index>8</if-index>\
           <phys-address>00:01:02:03:04:07</phys-address>\
           <statistics>\
             <discontinuity-time>\
               2013-04-01T03:00:00+00:00\
             </discontinuity-time>\
             <!-- counters now shown here -->\
           </statistics>\
         </interface>\
         <interface>\
           <name>lo1</name>\
           <type>ianaift:softwareLoopback</type>\
           <enabled>true</enabled>\
           <admin-status>up</admin-status>\
           <oper-status>up</oper-status>\
           <if-index>1</if-index>\
           <statistics>\
             <discontinuity-time>\
               2013-04-01T03:00:00+00:00\
             </discontinuity-time>\
             <!-- counters now shown here -->\
           </statistics>\
         </interface>\
       </interfaces>");

    assert(res == NO_ERR);
    return res;
}

/* The 3 mandatory callback functions: y_test_ietf_interfaces_bis_init, y_test_ietf_interfaces_bis_init2, y_test_ietf_interfaces_bis_cleanup */

status_t
    y_test_ietf_interfaces_bis_init (
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

    interfaces_obj = ncx_find_object(
        ietf_interfaces_mod,
        "interfaces");
    assert(interfaces_obj != NULL);
    return res;
}

status_t y_test_ietf_interfaces_bis_init2(void)
{
    status_t res;
    val_value_t* root_system_val;
    val_value_t* interfaces_val;

    res = NO_ERR;

    root_system_val = agt_nmda_get_root_system();
    assert(root_system_val);

    interfaces_val = val_find_child(root_system_val,
                                          "ietf-interfaces",
                                          "interfaces");
    assert(interfaces_val==NULL);

    interfaces_val = val_new_value();
    assert(interfaces_val);

    val_init_virtual(interfaces_val,
                     get_system_cfg_interfaces,
                     interfaces_obj);

    val_add_child(interfaces_val, root_system_val);

    return NO_ERR;
}

void y_test_ietf_interfaces_bis_cleanup (void)
{
}
