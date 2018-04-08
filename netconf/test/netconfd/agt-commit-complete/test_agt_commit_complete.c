/*
    module test-agt-commit-complete(based on test-agt-commit-complete.yang)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>


#include <libxml/xmlstring.h>
#include "procdefs.h"
#include "agt.h"
#include "agt_cb.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "agt_not.h"
#include "agt_rpc.h"
#include "agt_commit_complete.h"
#include "dlq.h"
#include "ncx.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "status.h"
#include "rpc.h"
#include "xpath.h"

/* module static variables */
static ncx_module_t *ietf_system_mod;


void my_transaction_handler(unsigned int transaction_id, val_value_t* prev_root_config_val, val_value_t* root_val)
{
    status_t res;
    val_value_t* location_before_val=NULL;
    val_value_t* location_after_val=NULL;
    val_value_t* hostname_before_val=NULL;
    val_value_t* hostname_after_val=NULL;

    if(prev_root_config_val) {
        res = xpath_find_val_target(prev_root_config_val, ietf_system_mod/*mod*/,"/sys:system/sys:location", &location_before_val);
        res = xpath_find_val_target(prev_root_config_val, ietf_system_mod/*mod*/,"/sys:system/sys:hostname", &hostname_before_val);
    }

    res = xpath_find_val_target(root_val, ietf_system_mod/*mod*/,"/sys:system/sys:location", &location_after_val);
    res = xpath_find_val_target(root_val, ietf_system_mod/*mod*/,"/sys:system/sys:hostname", &hostname_after_val);
    printf("\n#%u: location=%s -> location=%s, hostname=%s -> hostname=%s\n", transaction_id,
        (location_before_val)?VAL_STRING(location_before_val):"None",
        (location_after_val)?VAL_STRING(location_after_val):"None",
        (hostname_before_val)?VAL_STRING(hostname_before_val):"None",
        (hostname_after_val)?VAL_STRING(hostname_after_val):"None");
}

static val_value_t* prev_root_config_val=NULL;
static unsigned int transaction_id=0;

status_t my_commit_complete_cb(void)
{
    cfg_template_t* runningcfg;
    status_t res;
    val_value_t*    cur_root_config_val;

    printf("in my_commit_complete_cb\n");
    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg!=NULL && runningcfg->root!=NULL);

    cur_root_config_val = val_clone_config_data(runningcfg->root, &res);
    assert(res==NO_ERR);

    val_dump_value(cur_root_config_val, NCX_DEF_INDENT);

    printf("\nTransaction id=%u", transaction_id);
    printf("\nBefore:");
    if(prev_root_config_val==NULL) {
        printf("\nNone.");
    } else {
        val_dump_value(prev_root_config_val, NCX_DEF_INDENT);
    }
    printf("\nAfter:");
    val_dump_value(cur_root_config_val, NCX_DEF_INDENT);

    my_transaction_handler(transaction_id, prev_root_config_val, runningcfg->root);

    if(prev_root_config_val!=NULL) {
        val_free_value(prev_root_config_val);
    }
    prev_root_config_val = cur_root_config_val;
    transaction_id++;
    return NO_ERR;
}

/* The 3 mandatory callback functions: y_ietf_system_init, y_ietf_system_init2, y_ietf_system_cleanup */

status_t
    y_test_agt_commit_complete_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    status_t res;

    agt_profile = agt_get_profile();

    res=agt_commit_complete_register("test-agt-commit-complete",
                                      my_commit_complete_cb);
    assert(res == NO_ERR);

    res = ncxmod_load_module(
        "ietf-system",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_system_mod);
    if (res != NO_ERR) {
        return res;
    }

    return res;
}

status_t y_test_agt_commit_complete_init2(void)
{
    return NO_ERR;
}

void y_test_agt_commit_complete_cleanup (void)
{
    agt_commit_complete_unregister("test-agt-commit-complete");
    if(prev_root_config_val!=NULL) {
        val_free_value(prev_root_config_val);
    }
}
