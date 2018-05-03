/*
    module test-ietf-ip-bis
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
static obj_template_t* interfaces_obj;

/* Registered callback functions: get_system_cfg_interfaces */

static status_t
    get_learned_cfg_interfaces(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res = NO_ERR;

//             <forwarding>false</forwarding>\
//             <mtu>1500</mtu>\
//             <mtu>1280</mtu>\
//             <forwarding>false</forwarding>\

    res = val_set_cplxval_obj(dst_val,dst_val->obj,"<interfaces\
           xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\"\
           xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\"\
           xmlns:or=\"urn:ietf:params:xml:ns:yang:ietf-origin\">\
         <interface>\
           <name>eth0</name>\
           <type>ianaift:ethernetCsmacd</type>\
           <!-- other parameters from ietf-interfaces omitted -->\
           <ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\
             <address>\
               <ip>192.0.2.1</ip>\
               <prefix-length>24</prefix-length>\
               <origin>static</origin>\
             </address>\
             <neighbor>\
               <ip>192.0.2.2</ip>\
               <link-layer-address>\
                 00:01:02:03:04:05\
               </link-layer-address>\
             </neighbor>\
           </ipv4>\
           <ipv6 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\
             <address>\
               <ip>2001:db8::10</ip>\
               <prefix-length>32</prefix-length>\
               <origin>static</origin>\
               <status>preferred</status>\
             </address>\
             <address>\
               <ip>2001:db8::1:100</ip>\
               <prefix-length>32</prefix-length>\
               <origin>dhcp</origin>\
               <status>preferred</status>\
             </address>\
             <dup-addr-detect-transmits>0</dup-addr-detect-transmits>\
             <neighbor>\
               <ip>2001:db8::1</ip>\
               <link-layer-address>\
                 00:01:02:03:04:05\
               </link-layer-address>\
               <origin>dynamic</origin>\
               <is-router/>\
               <state>reachable</state>\
             </neighbor>\
             <neighbor>\
               <ip>2001:db8::4</ip>\
               <origin>dynamic</origin>\
               <state>incomplete</state>\
             </neighbor>\
           </ipv6>\
         </interface>\
       </interfaces>");

    assert(res == NO_ERR);
    return res;
}

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
           xmlns:or=\"urn:ietf:params:xml:ns:yang:ietf-origin\">\
         <interface>\
           <name>eth0</name>\
           <type>ianaift:ethernetCsmacd</type>\
           <!-- other parameters from ietf-interfaces omitted -->\
         </interface>\
       </interfaces>");

    assert(res == NO_ERR);
    return res;
}

/* The 3 mandatory callback functions: y_test_ietf_ip_bis_init, y_test_ietf_ip_bis_init2, y_test_ietf_ip_bis_cleanup */

status_t
    y_test_ietf_ip_bis_init (
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
    assert(res == NO_ERR);

    res = ncxmod_load_module(
        "ietf-ip",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_ip_mod);
    assert(res == NO_ERR);

    interfaces_obj = ncx_find_object(
        ietf_interfaces_mod,
        "interfaces");
    assert(interfaces_obj != NULL);
    return res;
}

status_t y_test_ietf_ip_bis_init2(void)
{
    status_t res;
    val_value_t* root_system_val;
    val_value_t* root_learned_val;
    val_value_t* interfaces_val;

    res = NO_ERR;

    root_system_val = agt_nmda_get_root_system();
    assert(root_system_val);

    root_learned_val = agt_nmda_get_root_learned();
    assert(root_learned_val);

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

    interfaces_val = val_new_value();
    assert(interfaces_val);
    val_init_virtual(interfaces_val,
                     get_learned_cfg_interfaces,
                     interfaces_obj);

    val_add_child(interfaces_val, root_learned_val);

    return NO_ERR;
}

void y_test_ietf_ip_bis_cleanup (void)
{
}
