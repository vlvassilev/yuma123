/*
    module ietf-alarms
    namespace urn:ietf:params:xml:ns:yang:ietf-alarms
 */
 
#include <xmlstring.h>
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

#include <string.h>
#include <assert.h>

/* The 3 mandatory callback functions: y_ietf_alarms_init, y_ietf_alarms_init2, y_ietf_alarms_cleanup */

status_t
    y_ietf_alarms_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    status_t res;
    ncx_module_t *mod;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "ietf-alarms",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/alarms",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        y_ietf_alarms_alarms_edit);
    if (res != NO_ERR) {
        return res;
    }

    return res;
}

status_t y_ietf_alarms_init2(void)
{
    status_t res;
    cfg_template_t* runningcfg;
    val_value_t* alarms_val;

    res = NO_ERR;

    return res;
}

void y_ietf_alarms_cleanup (void)
{
}
