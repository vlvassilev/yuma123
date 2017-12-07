/*
    module test-ietf-routing-bis
 */
 
#include <xmlstring.h>
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
static ncx_module_t *ietf_routing_mod;
static obj_template_t* interfaces_obj;

/* Registered callback functions: get_system_cfg_interfaces */

static status_t
    get_system_cfg_interfaces(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res = NO_ERR;
    res = val_set_cplxval_obj(dst_val,dst_val->obj,"<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\
      <interface>\
        <name>eth0</name>\
        <description>Uplink to ISP.</description>\
        <type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">ianaift:ethernetCsmacd</type>\
        <phys-address>00:0C:42:E5:B1:E9</phys-address>\
        <oper-status>up</oper-status>\
	<statistics>\
          <discontinuity-time>2015-10-24T17:11:27+02:00</discontinuity-time>\
        </statistics>\
        <ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">\
          <address>\
            <ip>192.0.2.1</ip>\
            <prefix-length>24</prefix-length>\
          </address>\
        </ipv4>\
      </interface>\
    </interfaces>");

    assert(res == NO_ERR);
    return res;
}

/* The 3 mandatory callback functions: y_test_ietf_routing_bis_init, y_test_ietf_routing_bis_init2, y_test_ietf_routing_bis_cleanup */

status_t
    y_test_ietf_routing_bis_init (
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

status_t y_test_ietf_routing_bis_init2(void)
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

void y_test_ietf_routing_bis_cleanup (void)
{
}
