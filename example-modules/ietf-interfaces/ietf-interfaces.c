/*
    module ietf-interfaces
    namespace urn:ietf:params:xml:ns:yang:ietf-interfaces
 */
 
#include "val.h"
#include "agt.h"
#include "agt_cli.h"

static val_value_t* with_nmda_param_val=NULL;

extern status_t
    y_ietf_interfaces_w_nmda_init (
        const xmlChar *modname,
        const xmlChar *revision);
extern status_t y_ietf_interfaces_w_nmda_init2(void);
extern void y_ietf_interfaces_w_nmda_cleanup (void);

/* The 3 mandatory callback functions: y_ietf_interfaces_init, y_ietf_interfaces_init2, y_ietf_interfaces_cleanup */

status_t
    y_ietf_interfaces_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    val_value_t *clivalset;

    /* check if netconfd is started with --with-nmda=true (param in netconfd-ex.yang)
     * and validate correct module version is loaded
     */
    clivalset = agt_cli_get_valset();
    with_nmda_param_val = val_find_child(clivalset, "netconfd-ex", "with-nmda");
    if(with_nmda_param_val && VAL_BOOL(with_nmda_param_val)) {
        return y_ietf_interfaces_w_nmda_init (modname,revision);
    } else {
        //return y_ietf_interfaces_wo_nmda_init (modname,revision);
    }
}

status_t y_ietf_interfaces_init2(void)
{
    if(with_nmda_param_val && VAL_BOOL(with_nmda_param_val)) {
        return y_ietf_interfaces_w_nmda_init2();
    } else {
        //return y_ietf_interfaces_wo_nmda_init2();
    }
}

void y_ietf_interfaces_cleanup (void)
{
    if(with_nmda_param_val && VAL_BOOL(with_nmda_param_val)) {
        y_ietf_interfaces_w_nmda_cleanup();
    } else {
        //y_ietf_interfaces_wo_nmda_cleanup();
    }
}
