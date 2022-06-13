/*
 * Copyright (c) 2008 - 2012, Andy Bierman, All Rights Reserved.
 * Copyright (c) 2013 - 2017, Vladimir Vassilev, All Rights Reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*  FILE: agt_sys.c

   NETCONF System Data Model implementation: Server Side Support

identifiers:
container /system
leaf /system/sysName
leaf /system/sysCurrentDateTime
leaf /system/sysBootDateTime
leaf /system/sysLogLevel
leaf /system/sysNetconfServerId
notification /sysStartup
leaf /sysStartup/startupSource
list /sysStartup/bootError
leaf /sysStartup/bootError/error-type
leaf /sysStartup/bootError/error-tag
leaf /sysStartup/bootError/error-severity
leaf /sysStartup/bootError/error-app-tag
leaf /sysStartup/bootError/error-path
leaf /sysStartup/bootError/error-message
leaf /sysStartup/bootError/error-info
notification /sysConfigChange
leaf /sysConfigChange/userName
leaf /sysConfigChange/sessionId
list /sysConfigChange/edit
leaf /sysConfigChange/edit/target
leaf /sysConfigChange/edit/operation
notification /sysCapabilityChange
container /sysCapabilityChange/changed-by
choice /sysCapabilityChange/changed-by/server-or-user
case /sysCapabilityChange/changed-by/server-or-user/server
leaf /sysCapabilityChange/changed-by/server-or-user/server/server
case /sysCapabilityChange/changed-by/server-or-user/by-user
leaf /sysCapabilityChange/changed-by/server-or-user/by-user/userName
leaf /sysCapabilityChange/changed-by/server-or-user/by-user/sessionId
leaf /sysCapabilityChange/changed-by/server-or-user/by-user/remoteHost
leaf-list /sysCapabilityChange/added-capability
leaf-list /sysCapabilityChange/deleted-capability
notification /sysSessionStart
leaf /sysSessionStart/userName
leaf /sysSessionStart/sessionId
leaf /sysSessionStart/remoteHost
notification /sysSessionEnd
leaf /sysSessionEnd/userName
leaf /sysSessionEnd/sessionId
leaf /sysSessionEnd/remoteHost
leaf /sysSessionEnd/terminationReason
notification /sysConfirmedCommit
leaf /sysConfirmedCommit/userName
leaf /sysConfirmedCommit/sessionId
leaf /sysConfirmedCommit/remoteHost
leaf /sysConfirmedCommit/confirmEvent
rpc /set-log-level
leaf rpc/input/log-level
rpc /disable-cache

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
24feb09      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <assert.h>

#include "procdefs.h"
#include "agt.h"
#include "agt_cap.h"
#include "agt_cb.h"
#include "agt_cfg.h"
#include "agt_cli.h"
#include "agt_not.h"
#include "agt_rpc.h"
#include "agt_ses.h"
#include "agt_sys.h"
#include "agt_util.h"
#include "cfg.h"
#include "getcb.h"
#include "log.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "rpc.h"
#include "rpc_err.h"
#include "ses.h"
#include "ses_msg.h"
#include "status.h"
#include "tstamp.h"
#include "val.h"
#include "val_util.h"
#include "xmlns.h"
#include "xml_util.h"
#include "xml_wr.h"
#include "yangconst.h"


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define ietf_system_N_system_state (const xmlChar *)"system-state"
#define ietf_system_N_system (const xmlChar *)"system"
#define system_N_system (const xmlChar *)"yuma"
#define system_N_sysName (const xmlChar *)"sysName"
#define system_N_sysCurrentDateTime (const xmlChar *)"sysCurrentDateTime"
#define system_N_sysBootDateTime (const xmlChar *)"sysBootDateTime"
#define system_N_sysLogLevel (const xmlChar *)"sysLogLevel"
#define system_N_sysNetconfServerId (const xmlChar *)"sysNetconfServerId"
#define system_N_sysNetconfServerCLI (const xmlChar *)"sysNetconfServerCLI"

#define system_N_sysStartup (const xmlChar *)"sysStartup"

#define system_N_sysConfigChange (const xmlChar *)"sysConfigChange"
#define system_N_edit (const xmlChar *)"edit"

#define system_N_sysCapabilityChange (const xmlChar *)"sysCapabilityChange"
#define system_N_sysSessionStart (const xmlChar *)"sysSessionStart"
#define system_N_sysSessionEnd (const xmlChar *)"sysSessionEnd"
#define system_N_sysConfirmedCommit (const xmlChar *)"sysConfirmedCommit"



#define system_N_userName (const xmlChar *)"userName"
#define system_N_sessionId (const xmlChar *)"sessionId"
#define system_N_remoteHost (const xmlChar *)"remoteHost"
#define system_N_killedBy (const xmlChar *)"killedBy"
#define system_N_terminationReason (const xmlChar *)"terminationReason"

#define system_N_confirmEvent (const xmlChar *)"confirmEvent"

#define system_N_target (const xmlChar *)"target"
#define system_N_operation (const xmlChar *)"operation"

#define system_N_changed_by (const xmlChar *)"changed-by"
#define system_N_added_capability (const xmlChar *)"added-capability"
#define system_N_deleted_capability (const xmlChar *)"deleted-capability"

#define system_N_uname (const xmlChar *)"uname"
#define system_N_sysname (const xmlChar *)"sysname"
#define system_N_release (const xmlChar *)"release"
#define system_N_version (const xmlChar *)"version"
#define system_N_machine (const xmlChar *)"machine"
#define system_N_nodename (const xmlChar *)"nodename"

#define system_N_set_log_level (const xmlChar *)"set-log-level"
#define system_N_log_level (const xmlChar *)"log-level"

/** ietf-system system-state related CONST **/
#define ietf_system (const xmlChar *)"ietf-system"
#define ietf_system_state_clock (const xmlChar *)"clock"
#define ietf_system_state_platform (const xmlChar *)"platform"
#define ietf_system_state_os_name (const xmlChar *)"os-name"
#define ietf_system_state_os_release (const xmlChar *)"os-release"
#define ietf_system_state_os_version (const xmlChar *)"os-version"
#define ietf_system_state_machine (const xmlChar *)"machine"
#define ietf_system_state_current_datetime (const xmlChar *)"current-datetime"
#define ietf_system_state_boot_datetime (const xmlChar *)"boot-datetime"

/** ietf-system system related CONST **/
#define ietf_system_hostname (const xmlChar *)"hostname"
#define ietf_system_contact (const xmlChar *)"contact"
#define ietf_system_location (const xmlChar *)"location"
#define ietf_system_ntp (const xmlChar *)"ntp"
#define ietf_system_radius (const xmlChar *)"radius"

/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/

static boolean              agt_sys_init_done = FALSE;

/* ietf-system.yang */
static ncx_module_t         *ietf_sysmod;
/* system.yang */
static ncx_module_t         *sysmod;
/* ietf-netconf-notifications.yang */
static ncx_module_t         *ietf_netconf_notifications_mod;

/* cached pointer to the <system> element template */
static obj_template_t *ietf_system_state_obj;
static obj_template_t *ietf_system_obj;
static obj_template_t *yuma_system_obj;

/* cached pointers to the eventType nodes for this module */
static obj_template_t *sysStartupobj;
static obj_template_t *sysCapabilityChangeobj;
static obj_template_t *sysSessionEndobj;
static obj_template_t *sysConfirmedCommitobj;

/* stored for being modifyed by fake function */
static xmlChar *fake_string;

/********************************************************************
* FUNCTION payload_error
*
* Generate an error for a payload leaf that failed
*
* INPUTS:
*    name == leaf name that failed
*    res == error status
*********************************************************************/
static void
    payload_error (const xmlChar *name,
                        status_t res)
{
    log_error( "\nError: cannot make payload leaf '%s' (%s)",
               name, get_error_string(res) );

}  /* payload_error */


/********************************************************************
* FUNCTION get_currentDateTime
*
* <get> operation handler for the sysCurrentDateTime leaf
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_currentDateTime (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    xmlChar      *buff;

    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
        buff = (xmlChar *)m__getMem(TSTAMP_MIN_SIZE);
        if (!buff) {
            return ERR_INTERNAL_MEM;
        }

        tstamp_datetime(buff);
        VAL_STR(dstval) = buff;
        return NO_ERR;
    } else {
        return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* get_currentDateTime */

/********************************************************************
* FUNCTION get_fake_string
*
* copied from get_currentDateTime
* <get> operation handler for the sysCurrentDateTime leaf
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_fake_string (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    xmlChar      *buff;

    (void)scb;
    (void)virval;

    if (*fake_string == NULL || *fake_string=="") {
        *fake_string ="init_value";
    }

    if (cbmode == GETCB_GET_VALUE) {
        buff = (xmlChar *)m__getMem(TSTAMP_MIN_SIZE);
        if (!buff) {
            return ERR_INTERNAL_MEM;
        }

        sprintf((char *)buff, *fake_string);
        VAL_STR(dstval) = buff;
        return NO_ERR;
    } else {
        return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* get_currentDateTime */


/********************************************************************
* FUNCTION get_currentLogLevel
*
* <get> operation handler for the sysCurrentDateTime leaf
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_currentLogLevel (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    const xmlChar  *loglevelstr;
    log_debug_t     loglevel;
    status_t        res;

    (void)scb;
    (void)virval;
    res = NO_ERR;

    if (cbmode == GETCB_GET_VALUE) {
        loglevel = log_get_debug_level();
        loglevelstr = log_get_debug_level_string(loglevel);
        if (loglevelstr == NULL) {
            res = ERR_NCX_OPERATION_FAILED;
        } else {
            res = ncx_set_enum(loglevelstr, VAL_ENU(dstval));
        }
    } else {
        res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

    return res;

} /* get_currentLogLevel */


/********************************************************************
* FUNCTION set_log_level_invoke
*
* set-log-level : invoke params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t
    set_log_level_invoke (ses_cb_t *scb,
                          rpc_msg_t *msg,
                          xml_node_t *methnode)
{
    val_value_t     *levelval;
    log_debug_t      levelenum;
    status_t         res;

    (void)scb;
    (void)methnode;

    res = NO_ERR;

    /* get the level parameter */
    levelval = val_find_child(msg->rpc_input,
                              AGT_SYS_MODULE,
                              NCX_EL_LOGLEVEL);
    if (levelval) {
        if (levelval->res == NO_ERR) {
            levelenum =
                log_get_debug_level_enum((const char *)
                                         VAL_ENUM_NAME(levelval));
            if (levelenum != LOG_DEBUG_NONE) {
                log_set_debug_level(levelenum);
            } else {
                res = ERR_NCX_OPERATION_FAILED;
            }
        } else {
            res = levelval->res;
        }
    }

    return res;

} /* set_log_level_invoke */

/********************************************************************
* FUNCTION send_sysStartup
*
* Queue the <sysStartup> notification
*
*********************************************************************/
static void
    send_sysStartup (void)
{
    agt_not_msg_t         *not;
    cfg_template_t        *cfg;
    val_value_t           *leafval, *bootErrorval;
    rpc_err_rec_t         *rpcerror;
    obj_template_t        *bootErrorobj;
    status_t               res;

    log_debug("\nagt_sys: generating <sysStartup> notification");

    not = agt_not_new_notification(sysStartupobj);
    if (!not) {
        log_error("\nError: malloc failed; cannot send <sysStartup>");
        return;
    }

    /* add sysStartup/startupSource */
    cfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (cfg) {
        if (cfg->src_url) {
            leafval = agt_make_leaf(sysStartupobj,
                    (const xmlChar *)"startupSource", cfg->src_url, &res);
            if (leafval) {
                agt_not_add_to_payload(not, leafval);
            } else {
                payload_error((const xmlChar *)"startupSource", res);
            }
        }

        /* add sysStartup/bootError for each error recorded */
        if (!dlq_empty(&cfg->load_errQ)) {
            /* get the bootError object */
            bootErrorobj = obj_find_child( sysStartupobj, AGT_SYS_MODULE,
                                           (const xmlChar *)"bootError");
            if (bootErrorobj) {
                rpcerror = (rpc_err_rec_t *)dlq_firstEntry(&cfg->load_errQ);
                for ( ; rpcerror;
                     rpcerror = (rpc_err_rec_t *)dlq_nextEntry(rpcerror)) {
                    /* make the bootError value struct */
                    bootErrorval = val_new_value();
                    if (!bootErrorval) {
                        payload_error((const xmlChar *)"bootError",
                                      ERR_INTERNAL_MEM);
                    } else {
                        val_init_from_template(bootErrorval, bootErrorobj);
                        res = agt_rpc_fill_rpc_error(rpcerror, bootErrorval);
                        if (res != NO_ERR) {
                            log_error("\nError: problems making <bootError> "
                                      "(%s)", get_error_string(res));
                        }
                        /* add even if there are some missing leafs */
                        agt_not_add_to_payload(not, bootErrorval);
                    }
                }
            } else {
                SET_ERROR(ERR_INTERNAL_VAL);
            }
        }
    } else {
        SET_ERROR(ERR_INTERNAL_VAL);
    }

    agt_not_queue_notification(not);
} /* send_sysStartup */


/********************************************************************
* FUNCTION netconf_notifications_add_common_session_parms
*
* Add the leafs from the SysCommonSessionParms grouping
*
* INPUTS:
*   scb == session control block to use for payload values
*   not == notification msg to use to add parms into
*
* OUTPUTS:
*   'not' payloadQ has malloced entries added to it
*********************************************************************/
static void
    netconf_notifications_add_common_session_parms (const ses_cb_t *scb,
                              agt_not_msg_t *not,
                              val_value_t* parent_val)
{
    obj_template_t  *parent_obj;
    val_value_t     *leafval;
    status_t         res;
    ses_id_t         use_sid;

    if(not!=NULL) {
        assert(parent_val==NULL);
        parent_obj = not->notobj;
    } else if(parent_val!=NULL) {
        assert(not==NULL);
        parent_obj=parent_val->obj;
    } else {
        assert(0);
    }

    /* add userName */
    if (scb->username) {
        leafval = agt_make_leaf(parent_obj,
                                "username",
                                scb->username,
                                &res);
        assert(leafval);
        if(not) {
            agt_not_add_to_payload(not, leafval);
        } else {
            val_add_child(leafval, parent_val);
        }
    }

    /* add sessionId */
    if (scb->sid) {
        use_sid = scb->sid;
    } else if (scb->rollback_sid) {
        use_sid = scb->rollback_sid;
    } else {
        res = ERR_NCX_NOT_IN_RANGE;
        use_sid = 0;
    }

    if (use_sid) {
        leafval = agt_make_uint_leaf(parent_obj,
                                "session-id",
                                use_sid,
                                &res);
        assert(leafval);
        if(not) {
            agt_not_add_to_payload(not, leafval);
        } else {
            val_add_child(leafval, parent_val);
        }
    }

    /* add remoteHost */
    if (scb->peeraddr) {
        leafval = agt_make_leaf(parent_obj,
                                "source-host",
                                scb->peeraddr,
                                &res);
        assert(leafval);
        if(not) {
            agt_not_add_to_payload(not, leafval);
        } else {
            val_add_child(leafval, parent_val);
        }
    }

} /* netconf_notifications_add_common_session_parms */


/********************************************************************
* FUNCTION init_static_vars
*
* Init the static vars
*
*********************************************************************/
static void
    init_static_sys_vars (void)
{
    ietf_sysmod = NULL;
    sysmod = NULL;
    ietf_system_state_obj = NULL;
    ietf_system_obj = NULL;
    yuma_system_obj = NULL;
    sysStartupobj = NULL;
    sysCapabilityChangeobj = NULL;
    sysSessionEndobj = NULL;
    sysConfirmedCommitobj = NULL;

} /* init_static_sys_vars */


/************* E X T E R N A L    F U N C T I O N S ***************/


/********************************************************************
* FUNCTION agt_sys_init
*
* INIT 1:
*   Initialize the server notification module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_sys_init (void)
{
    agt_profile_t  *agt_profile;
    status_t        res;

    if (agt_sys_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    if (LOGDEBUG2) {
        log_debug2("\nagt_sys: Loading notifications module");
    }

    agt_profile = agt_get_profile();
    init_static_sys_vars();
    agt_sys_init_done = TRUE;

    /* load the yuma module */
    res = ncxmod_load_module(AGT_SYS_MODULE,
                             NULL,
                             &agt_profile->agt_savedevQ,
                             &sysmod);
    if (res != NO_ERR) {
        return res;
    }

    /* load the ietf-system module */
    res = ncxmod_load_module(AGT_IETF_SYS_MODULE,
                             NULL,
                             &agt_profile->agt_savedevQ,
                             &ietf_sysmod);
    if (res != NO_ERR) {
        return res;
    }

    /* load the ietf-netconf-notifications module */
    res = ncxmod_load_module("ietf-netconf-notifications",
                             NULL,
                             &agt_profile->agt_savedevQ,
                             &ietf_netconf_notifications_mod);
    if (res != NO_ERR) {
        return res;
    }

    /* find the object definition for the system element */
    ietf_system_state_obj = ncx_find_object(ietf_sysmod,
                                ietf_system_N_system_state);

    if (!ietf_system_state_obj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* find the object definition for the system element */
    ietf_system_obj = ncx_find_object(ietf_sysmod,
                                ietf_system_N_system);

    if (!ietf_system_obj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    yuma_system_obj =
        obj_find_child(ietf_system_state_obj,
                        AGT_SYS_MODULE,
                        system_N_system);
    if (!yuma_system_obj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    sysStartupobj =
        ncx_find_object(sysmod,
                        system_N_sysStartup);
    if (!sysStartupobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* set up set-log-level RPC operation */
    res = agt_rpc_register_method(AGT_SYS_MODULE,
                                  system_N_set_log_level,
                                  AGT_RPC_PH_INVOKE,
                                  set_log_level_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    return NO_ERR;

}  /* agt_sys_init */


/********************************************************************
* FUNCTION agt_sys_init2
*
* INIT 2:
*   Initialize the monitoring data structures
*   This must be done after the <running> config is loaded
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_sys_init2 (void)
{
    val_value_t           *ietf_system_val, *ietf_system_state_val, *yuma_system_val, *unameval, *childval, *tempval;
    cfg_template_t        *runningcfg;
    const xmlChar         *myhostname;
    obj_template_t        *unameobj;
    status_t               res;
    xmlChar               *buffer, *p, tstampbuff[TSTAMP_MIN_SIZE];
    struct utsname         utsbuff;
    int                    retval;

    // For adding /system-state/ and /system/ and link them to the private API
    obj_template_t*        obj;
    val_value_t*           tmp_sub_dir_val;
    val_value_t*           tmp_val;

    if (!agt_sys_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    /* get the running config to add some static data into */
    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!runningcfg || !runningcfg->root) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* Add /system */
    ietf_system_val = val_find_child(runningcfg->root,
                                          ietf_system,
                                          "system");
    if(ietf_system_val==NULL) {
        ietf_system_val = val_new_value();
        if (!ietf_system_val) {
            return ERR_INTERNAL_MEM;
        }
        val_init_from_template(ietf_system_val, ietf_system_obj);
        val_add_child(ietf_system_val, runningcfg->root);
    }

    add_sub_val_under_dir(ietf_system_val, ietf_system, ietf_system_hostname , get_fake_string);

    /* Add /system-state */
    ietf_system_state_val = val_new_value();
    if (!ietf_system_state_val) {
        return ERR_INTERNAL_MEM;
    }
    val_init_from_template(ietf_system_state_val, ietf_system_state_obj);


    /* Start adding ietf-system and using private API*/
    /* Add /system-state/clock */
    tmp_sub_dir_val = val_find_child(ietf_system_state_val,
                                          ietf_system,
                                          ietf_system_state_clock);
    if(tmp_sub_dir_val==NULL) {
        printf("got null clock_val");
        obj = obj_find_child(ietf_system_state_val->obj,
                                       ietf_system,
                                       ietf_system_state_clock);
        assert(obj != NULL);

        tmp_sub_dir_val = val_new_value();
        assert(tmp_sub_dir_val != NULL);

        val_init_from_template(tmp_sub_dir_val,
                               obj);

        val_add_child(tmp_sub_dir_val, ietf_system_state_val);

    }
    /* Add /system-state/clock/current-datetime */
    add_sub_val_under_dir(tmp_sub_dir_val, ietf_system, ietf_system_state_current_datetime, get_currentDateTime);

    /* Add /system-state/clock/boot-datetime */
    add_sub_val_under_dir(tmp_sub_dir_val, ietf_system, ietf_system_state_boot_datetime, get_fake_string);

    /* Add /system-state/platform */
    tmp_sub_dir_val = val_find_child(ietf_system_state_val,
                                          ietf_system,
                                          ietf_system_state_platform);
    if(tmp_sub_dir_val==NULL) {
        printf("got null platfoorm_val");
        obj = obj_find_child(ietf_system_state_val->obj,
                                       ietf_system,
                                       ietf_system_state_platform);
        assert(obj != NULL);

        tmp_sub_dir_val = val_new_value();
        assert(tmp_sub_dir_val != NULL);

        val_init_from_template(tmp_sub_dir_val,
                               obj);

        val_add_child(tmp_sub_dir_val, ietf_system_state_val);
    }

    /* Add /system-state/platform/os-name */
    add_sub_val_under_dir(tmp_sub_dir_val, ietf_system, ietf_system_state_os_name, get_fake_string);

    /* Add /system-state/platform/os-release */
    add_sub_val_under_dir(tmp_sub_dir_val, ietf_system, ietf_system_state_os_release, get_fake_string);

    /* Add /system-state/platform/os-version */
    add_sub_val_under_dir(tmp_sub_dir_val, ietf_system, ietf_system_state_os_version, get_fake_string);

    /* Add /system-state/platform/machine */
    add_sub_val_under_dir(tmp_sub_dir_val, ietf_system, ietf_system_state_machine, get_fake_string);

    /* [DONE] Start adding ietf-system and using private API*/
    /* handing off the malloced memory here */
    val_add_child_sorted(ietf_system_state_val, runningcfg->root);

    /* add sysStartup to notificationQ */
    send_sysStartup();

    return NO_ERR;

}  /* agt_sys_init2 */




/********************************************************************
* FUNCTION add_sub_val_under_dir
*
* Add the node under the selected dir
*
* INPUTS:
*   dir == the dir that will be added node
*   modname == module name
*   nodename == the node name that will be added
*   cb == the callback that in charge of the real value of this node
* RETURNS:
*   none
*********************************************************************/
void
    add_sub_val_under_dir(
        val_value_t *dir,
        const xmlChar *modname,
        const xmlChar *nodename,
        void *cbfn
        )
{
    obj_template_t* obj;
    val_value_t* tmp_val;

    obj = obj_find_child(dir->obj,
                         modname,
                         nodename);
    assert(obj != NULL);

    tmp_val = val_new_value();
    assert(tmp_val != NULL);

    val_init_virtual(tmp_val,
                     cbfn,
                     obj);

    val_add_child(tmp_val, dir);
}

/********************************************************************
* FUNCTION agt_sys_cleanup
*
* Cleanup the module data structures
*
* INPUTS:
*
* RETURNS:
*   none
*********************************************************************/
void
    agt_sys_cleanup (void)
{
    if (agt_sys_init_done) {
        init_static_sys_vars();
        agt_sys_init_done = FALSE;
        agt_rpc_unregister_method(AGT_SYS_MODULE,
                                  system_N_set_log_level);
    }
}  /* agt_sys_cleanup */


/********************************************************************
* FUNCTION agt_sys_send_netconf_session_start
*
* Queue the <netconf-session-start> notification
*
* INPUTS:
*   scb == session control block to use for payload values
*
* OUTPUTS:
*   notification generated and added to notificationQ
*
*********************************************************************/
void
    agt_sys_send_netconf_session_start (const ses_cb_t *scb)
{
    agt_not_msg_t         *not;
    obj_template_t        *netconf_session_start_obj;

    if (LOGDEBUG) {
        log_debug("\nagt_sys: generating <netconf-session-start> "
                  "notification");
    }

    netconf_session_start_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-session-start");
    assert(netconf_session_start_obj);

    not = agt_not_new_notification(netconf_session_start_obj);
    if (!not) {
        log_error("\nError: malloc failed; cannot "
                  "send <netconf-session-start>");
        return;
    }

    netconf_notifications_add_common_session_parms(scb, not, NULL /*parent_val*/);

    agt_not_queue_notification(not);

} /* agt_sys_send_netconf_session_start */


/********************************************************************
* FUNCTION get_termination_reason_str
*
* Convert the termination reason enum to a string
*
* INPUTS:
*   termreason == enum for the terminationReason leaf
*
* OUTPUTS:
*   the termination reason string
*
*********************************************************************/
static const xmlChar*
    get_termination_reason_str ( ses_term_reason_t termreason)
{
    const xmlChar         *termreasonstr;

    switch (termreason) {
    case SES_TR_NONE:
        SET_ERROR(ERR_INTERNAL_VAL);
        termreasonstr = (const xmlChar *)"other";
        break;
    case SES_TR_CLOSED:
        termreasonstr = (const xmlChar *)"closed";
        break;
    case SES_TR_KILLED:
        termreasonstr = (const xmlChar *)"killed";
        break;
    case SES_TR_DROPPED:
        termreasonstr = (const xmlChar *)"dropped";
        break;
    case SES_TR_TIMEOUT:
        termreasonstr = (const xmlChar *)"timeout";
        break;
    case SES_TR_OTHER:
        termreasonstr = (const xmlChar *)"other";
        break;
    case SES_TR_BAD_HELLO:
        termreasonstr = (const xmlChar *)"bad-hello";
        break;
    case SES_TR_BAD_START:
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        termreasonstr = (const xmlChar *)"other";
    }

    return termreasonstr;
}


/********************************************************************
* FUNCTION agt_sys_send_netconf_session_end
*
* Queue the <netconf-session-end> notification
*
* INPUTS:
*   scb == session control block to use for payload values
*   termreason == enum for the termination-reason leaf
*   killedby == session-id for killed-by leaf if termination_reason == "killed"
*               ignored otherwise
*
* OUTPUTS:
*   notification generated and added to notificationQ
*
*********************************************************************/
void
    agt_sys_send_netconf_session_end(const ses_cb_t *scb,
                                ses_term_reason_t termination_reason,
                                ses_id_t killed_by)
{
    agt_not_msg_t         *not;
    val_value_t           *leafval;
    const xmlChar         *termination_reason_str;
    status_t               res;

    obj_template_t        *netconf_session_end_obj;


    assert(scb && "agt_sys_send_netconf_session_end() - param scb is NULL");

    log_debug("\nagt_sys: generating <netconf-session-end> notification");

    netconf_session_end_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-session-end");
    assert(netconf_session_end_obj);

    not = agt_not_new_notification(netconf_session_end_obj);
    assert(not);

    /* session started;  not just being killed
     * in the <ncxconnect> message handler */
    if (termination_reason != SES_TR_BAD_START) {
        netconf_notifications_add_common_session_parms(scb, not, NULL /*parent_val*/);
    }

    /* add sysSessionEnd/killedBy */
    if (termination_reason == SES_TR_KILLED) {
        leafval = agt_make_uint_leaf( netconf_session_end_obj, "killed-by",
                                      killed_by, &res );
        assert(leafval);
        agt_not_add_to_payload(not, leafval);
    }

    /* add sysSessionEnd/terminationReason */
    termination_reason_str = get_termination_reason_str(termination_reason);
    leafval = agt_make_leaf( netconf_session_end_obj, "termination-reason",
                             termination_reason_str, &res );
    assert(leafval);

    agt_not_add_to_payload(not, leafval);

    agt_not_queue_notification(not);
} /* agt_sys_send_netconf_session_end */



/********************************************************************
* FUNCTION agt_sys_send_netconf_config_change
*
* Queue the <netconf-config-change> notification
*
* INPUTS:
*   scb == session control block to use for payload values
*   auditrecQ == Q of agt_cfg_audit_rec_t structs to use
*                for the notification payload contents
*
* OUTPUTS:
*   notification generated and added to notificationQ
*
*********************************************************************/
void
    agt_sys_send_netconf_config_change (const ses_cb_t *scb,
                                  dlq_hdr_t *auditrecQ)
{
    agt_not_msg_t         *not;
    agt_cfg_audit_rec_t   *auditrec;
    val_value_t           *leafval, *listval;
    obj_template_t        *netconf_config_change_obj;
    obj_template_t        *listobj;
    status_t               res;

#ifdef DEBUG
    if (!scb || !auditrecQ) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (LOGDEBUG) {
        log_debug("\nagt_sys: generating <netconf-config-change> "
                  "notification");
    }
    netconf_config_change_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-config-change");
    assert(netconf_config_change_obj);

    not = agt_not_new_notification(netconf_config_change_obj);
    assert(not);
    {
        obj_template_t  *changed_by_obj;
        val_value_t     *changed_by_val;

        changed_by_obj =
            obj_find_child(not->notobj,
                            "ietf-netconf-notifications",
                            "changed-by");
        assert(changed_by_obj);
        changed_by_val = val_new_value();
        val_init_from_template(changed_by_val, changed_by_obj);


        netconf_notifications_add_common_session_parms(scb, NULL /*not*/, changed_by_val);
        agt_not_add_to_payload(not, changed_by_val);

    }

    listobj = obj_find_child(netconf_config_change_obj,
                             "ietf-netconf-notifications",
                             "edit");
    if (listobj == NULL) {
        SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    } else {
        for (auditrec = (agt_cfg_audit_rec_t *)dlq_firstEntry(auditrecQ);
             auditrec != NULL;
             auditrec = (agt_cfg_audit_rec_t *)dlq_nextEntry(auditrec)) {

            /* add netconf-config-change/edit */
            listval = val_new_value();
            assert(listval != NULL);
            {
                val_init_from_template(listval, listobj);

                /* pass off listval malloc here */
                agt_not_add_to_payload(not, listval);

                /* add netconf-config-change/edit/target */
                leafval = agt_make_leaf(listobj,
                                        "target",
                                        auditrec->target,
                                        &res);
                assert(leafval);
                val_add_child(leafval, listval);

                /* add netconf-config-change/edit/operation */
                leafval = agt_make_leaf(listobj,
                                        "operation",
                                        op_editop_name(auditrec->editop),
                                        &res);
                assert(leafval);
                val_add_child(leafval, listval);
            }
        }
    }

    agt_not_queue_notification(not);

} /* agt_sys_send_netconf_config_change */


/********************************************************************
* FUNCTION agt_sys_send_netconf_capablity_change
*
* Send a <netconf-capability-change> event for a module
* being added
*
* Queue the <netconf-capability-change> notification
*
* INPUTS:
*   changed_by == session control block that made the
*                 change to add this module
*             == NULL if the server made the change
*   is_add    == TRUE if the capability is being added
*                FALSE if the capability is being deleted
*   capstr == capability string that was added or deleted
*
* OUTPUTS:
*   notification generated and added to notificationQ
*
*********************************************************************/
void
    agt_sys_send_netconf_capability_change (ses_cb_t *changed_by,
                                      boolean is_add,
                                      const xmlChar *capstr)
{
    agt_not_msg_t         *not;
    obj_template_t        *netconf_capability_change_obj;
    val_value_t           *leafval;
    status_t               res;

#ifdef DEBUG
    if (!capstr) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (LOGDEBUG) {
        log_debug("\nagt_sys: generating <netconf-capability-change> "
                  "notification");
    }

    netconf_capability_change_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-capability-change");
    assert(netconf_capability_change_obj);

    not = agt_not_new_notification(netconf_capability_change_obj);
    assert(not);

    {
        obj_template_t  *changed_by_obj;
        val_value_t     *changed_by_val;

        changed_by_obj =
            obj_find_child(not->notobj,
                            "ietf-netconf-notifications",
                            "changed-by");
        assert(changed_by_obj);
        changed_by_val = val_new_value();
        val_init_from_template(changed_by_val, changed_by_obj);

        if(changed_by) {
            netconf_notifications_add_common_session_parms(changed_by, NULL /*not*/, changed_by_val);
        } else {
            leafval = agt_make_leaf(changed_by_obj,
                                    "server",
                                    NULL,
                                    &res);
            assert(leafval);
            val_add_child(leafval, changed_by_val);
        }
        agt_not_add_to_payload(not, changed_by_val);

    }

    if (is_add) {
        /* add netconf-capability-change/added-capability */
        leafval = agt_make_leaf(netconf_capability_change_obj,
                                "added-capability",
                                capstr,
                                &res);
    } else {
        /* add netconf-capability-change/deleted-capability */
        leafval = agt_make_leaf(netconf_capability_change_obj,
                                "deleted-capability",
                                capstr,
                                &res);
    }
    assert(leafval);
    agt_not_add_to_payload(not, leafval);

    agt_not_queue_notification(not);

} /* agt_sys_send_netconf_capability_change */


/********************************************************************
* FUNCTION agt_sys_send_netconf_confirmed_commit
*
* Queue the <netconf-confirmed-commit> notification
*
* INPUTS:
*   scb == session control block to use for payload values
*   event == enum for the confirmEvent leaf
*
* OUTPUTS:
*   notification generated and added to notificationQ
*
*********************************************************************/
void
    agt_sys_send_netconf_confirmed_commit (const ses_cb_t *scb,
                                     ncx_confirm_event_t event)
{
    agt_not_msg_t         *not;
    obj_template_t        *netconf_confirmed_commit_obj;
    val_value_t           *leafval;
    const xmlChar         *eventstr;
    status_t               res;

    res = NO_ERR;

    eventstr = ncx_get_confirm_event_str(event);

    if (!eventstr) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }

    if (LOGDEBUG) {
        log_debug("\nagt_sys: generating <netconf-confirmed-commit> "
                  "notification (%s)",
                  eventstr);
    }

    netconf_confirmed_commit_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-confirmed-commit");
    assert(netconf_confirmed_commit_obj);

    not = agt_not_new_notification(netconf_confirmed_commit_obj);
    assert(not);

    if (event!=NCX_CC_EVENT_TIMEOUT) {
        assert(scb);
        netconf_notifications_add_common_session_parms(scb, not, NULL /*changed_by_val*/);
    }

    /* add sysConfirmedCommit/confirmEvent */
    leafval = agt_make_leaf(netconf_confirmed_commit_obj,
                            "confirm-event",
                            eventstr,
                            &res);
    assert(leafval);
    agt_not_add_to_payload(not, leafval);

    agt_not_queue_notification(not);

} /* agt_sys_send_netconf_confirmed_commit */



/* END file agt_sys.c */
