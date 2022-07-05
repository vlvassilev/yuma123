/*
    module ietf-ip
    namespace urn:ietf:params:xml:ns:yang:ietf-ip
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
#define ietf_ip (const xmlChar *)"ietf-ip"
#define ietf_interfaces (const xmlChar *)"ietf-interfaces"
#define ietf_interfaces_interfaces_container (const xmlChar *)"interfaces"
#define ietf_interfaces_interfaces_state_container (const xmlChar *)"interfaces-state"
#define ietf_interfaces_interface (const xmlChar *)"interface"
/* [RW] */
/* /if:interfaces/if:interface/ipv4 */
#define ietf_ip_interfaces_ipv4_container (const xmlChar *)"ipv4"
/* /if:interfaces/if:interface/ipv4/address */
#define ietf_ip_interfaces_ipv4_address (const xmlChar *)"address"
#define ietf_ip_interfaces_ipv4_address_ip (const xmlChar *)"ip"
#define ietf_ip_interfaces_ipv4_address_netmask (const xmlChar *)"netmask"

#define ietf_ip_interfaces_ipv6_container (const xmlChar *)"ipv6"
/* /if:interfaces/if:interface/ipv6/enabled */
#define ietf_ip_interfaces_ipv6_enabled (const xmlChar *)"enabled"
#define ietf_ip_interfaces_ipv6_address (const xmlChar *)"address"
/* /if:interfaces/if:interface/ipv6/address/ip */
#define ietf_ip_interfaces_ipv6_address_ip (const xmlChar *)"ip"

/* [RO] */
#define ietf_ip_interfaces_state_ipv4_container (const xmlChar *)"ipv4"
#define ietf_ip_interfaces_state_ipv4_forwarding (const xmlChar *)"forwarding"
#define ietf_ip_interfaces_state_ipv4_mtu (const xmlChar *)"mtu"
#define ietf_ip_interfaces_state_ipv4_address_ip (const xmlChar *)"ip"
#define ietf_ip_interfaces_state_ipv4_address_netmask (const xmlChar *)"netmask"

#define ietf_ip_interfaces_state_ipv6_container (const xmlChar *)"ipv6"
#define ietf_ip_interfaces_state_ipv6_forwarding (const xmlChar *)"forwarding"
#define ietf_ip_interfaces_state_ipv6_mtu (const xmlChar *)"mtu"
#define ietf_ip_interfaces_state_ipv6_address (const xmlChar *)"address"
#define ietf_ip_interfaces_state_ipv6_ip (const xmlChar *)"ip"
#define ietf_ip_interfaces_state_ipv6_origin (const xmlChar *)"origin"
/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/
/* module static variables */
static val_value_t* with_nmda_param_val;

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
ietf_ip_interfaces_state_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval) {
    status_t res = NO_ERR;

    if (LOGDEBUG) {
        log_debug("\nEnter ietf_ip_interfaces_state_get");
    }

    struct emptypb_Empty *in = malloc(sizeof(*(in)));
    struct networkpb_IPv4Status *v4_status= malloc(sizeof(*(v4_status)));
    network_Network_GetV4Status(in, v4_status);

    struct emptypb_Empty *in2 = malloc(sizeof(*(in2)));
    struct networkpb_BasicConfig *basic_cfg= malloc(sizeof(*(basic_cfg)));
    network_Network_GetBasicConfig(in, basic_cfg);

    struct emptypb_Empty *in3 = malloc(sizeof(*(in3)));
    struct networkpb_IPv6Status *v6_status = malloc(sizeof(*(v6_status)));
    network_Network_GetV6Status(in3, v6_status);

    log_debug("\n parentval name: %s", dstval->name);



    // for (int i =0; i < out->List_Len; i++) {
    //     val_value_t *entry_val = NULL;
    //     entry_val = agt_make_list(
    //         dstval->obj,
    //         ietf_interfaces_interface,
    //         &res);
    //     if (entry_val != NULL) {
    //         val_add_child(entry_val, dstval);
    //     } else if (res!=NO_ERR) {
    //         return SET_ERROR(res);
    //     }
    //     res = add_interface_state_entry(entry_val, out->List[i], status_out->List[i], device_out, ingress_out->List[i], egress_out->List[i]);
    //     if (res != NO_ERR) {
    //         return SET_ERROR(res);
    //     }
    // }

    free(v4_status);
    free(v6_status);
    free(basic_cfg);

    free(in);
    free(in2);
    free(in3);
    return res;
}

static status_t
ietf_ip_interface_state_mro(val_value_t *parentval) {
    status_t res = NO_ERR;

    val_init_virtual(
        parentval,
        ietf_ip_interfaces_state_get,
        parentval->obj);
    return res;
}


/* The 3 mandatory callback functions: y_ietf_ip_init, y_ietf_ip_init2, y_ietf_ip_cleanup */

status_t
    y_ietf_ip_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    ncx_module_t *mod;
    ncx_module_t *interface_mod;
    ncx_module_t *if_type_mod;
    ncx_module_t *if_notification_mod;
    status_t res = NO_ERR;

    val_value_t *clivalset;
    if (LOGDEBUG) {
        log_debug("@@@@ y_ietf_ip_init\n");
    }
    /* check for --with-nmda=true (param defined in netconfd-ex.yang) */
    clivalset = agt_cli_get_valset();
    with_nmda_param_val = val_find_child(clivalset, "netconfd-ex", "with-nmda");

    agt_profile = agt_get_profile();

    interface_mod = ncx_find_module(ietf_interfaces, NULL);
    if (interface_mod==NULL) {
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
    }

    if_type_mod = ncx_find_module("iana-if-type", NULL);
    if (if_type_mod==NULL) {
        res = ncxmod_load_module(
            "iana-if-type",
            NULL,
            &agt_profile->agt_savedevQ,
            &mod);
        assert(res == NO_ERR);
    }

    if_notification_mod = ncx_find_module("interfaces-notifications", NULL);
    if (if_notification_mod==NULL) {
        res = ncxmod_load_module(
            "interfaces-notifications",
            NULL,
            &agt_profile->agt_savedevQ,
            &mod);
        assert(res == NO_ERR);
    }

    res = ncxmod_load_module(
        ietf_ip,
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    assert(res == NO_ERR);
    return res;
}

status_t y_ietf_ip_init2(void)
{
    status_t res=NO_ERR;
    ncx_module_t* mod;
    obj_template_t* interfaces_obj;
    obj_template_t* interfaces_state_obj;
    val_value_t* interfaces_val;
    val_value_t* interfaces_state_val;
    val_value_t* root_val;

    if (LOGDEBUG) {
        log_debug("@@@@ y_ietf_ip_init2 \n");
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


    /* designed with ietf-interfaces existed */
    assert(interfaces_state_val!=NULL);
    // res = agt_add_top_container(interfaces_state_obj, &interfaces_state_val);
    // if (res != NO_ERR) {
    //     return SET_ERROR(res);
    // }
    log_debug("@@@@ y_ietf_ip_init2 1\n");


    res = ietf_ip_interface_state_mro(interfaces_state_val);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }
    log_debug("@@@@ y_ietf_ip_init2 2\n");

    return res;
}


void y_ietf_ip_cleanup (void)
{
#if 0
    agt_cb_unregister_callbacks( "ietf-interfaces",
                               (const xmlChar *)"/interfaces/interface");
    agt_cb_unregister_callbacks( "ietf-interfaces",
                               (const xmlChar *)"/interfaces/interface/enable");
#endif
}

