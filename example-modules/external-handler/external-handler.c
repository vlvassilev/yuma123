/*
    module external-handler
 */

#define __USE_XOPEN 1
//#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>


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

/* The 3 mandatory callback functions: y_external_handler_init, y_external_handler_init2, y_external_handler_cleanup */

status_t
    y_external_handler_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    status_t res;

    agt_profile = agt_get_profile();

    //TODO: register agt_commit_complete callback it should handle regitration of config false under config true nodes.
    return res;
}

status_t y_external_handler_init2(void)
{
    //TODO: loop through get callback registrations and add virtual nodes in the config false only parent ancestors.
    return NO_ERR;
}

void y_external_handler_cleanup(void)
{
}
