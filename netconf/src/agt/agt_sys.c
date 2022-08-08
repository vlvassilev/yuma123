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
#include "../../../libintri/.libintrishare/libintrishare.h"

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

#define ietf_netconf (const xmlChar *)"ietf-netconf"
/** ietf-system system-state related CONST **/
#define ietf_system (const xmlChar *)"ietf-system"
#define ietf_system_clock (const xmlChar *)"clock"
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
#define ietf_system_clock_timezone_name (const xmlChar *)"timezone-name"
#define ietf_system_ntp (const xmlChar *)"ntp"
#define ietf_system_ntp_enabled (const xmlChar *)"enabled"
#define ietf_system_ntp_server (const xmlChar *)"server"
#define ietf_system_ntp_server_name (const xmlChar *)"name"
#define ietf_system_ntp_server_udp (const xmlChar *)"udp"
#define ietf_system_ntp_server_udp_address (const xmlChar *)"address"
#define ietf_system_ntp_server_prefer (const xmlChar *)"prefer"
#define ietf_system_authentication (const xmlChar *)"authentication"
#define ietf_system_authentication_user (const xmlChar *)"user"
#define ietf_system_radius (const xmlChar *)"radius"
#define ietf_system_radius_server (const xmlChar *)"server"
#define ietf_system_radius_server_name (const xmlChar *)"name"
#define ietf_system_radius_server_udp (const xmlChar *)"udp"
#define ietf_system_radius_server_udp_address (const xmlChar *)"address"
#define ietf_system_radius_server_udp_authentication_port (const xmlChar *)"authentication-port"
#define ietf_system_radius_server_udp_shared_secret (const xmlChar *)"shared-secret"

#define fixed_os_name (const xmlChar *)"Linux"
/*
    Nameing rules:
    private_api_${action}_${service_name}_${field}
*/
#define private_api_get_network_hostname (int)0
#define private_api_get_system_location (int)1
#define private_api_get_system_contact (int)2
#define private_api_get_time_timezone (int)3
#define private_api_get_system_last_boot_time (int)4
#define private_api_get_time_current_datetime (int)5
#define private_api_get_device_current_version (int)6
#define private_api_get_device_hw_version (int)7
#define private_api_get_system_os_name (int)9
#define private_api_get_device_model (int)8

#define private_api_set_network_hostname (int)100
#define private_api_set_system_location (int)101
#define private_api_set_system_contact (int)102
#define private_api_set_time_timezone (int)103

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

static boolean agt_sys_init_done = FALSE;

/* ietf-system.yang */
static ncx_module_t *ietf_sysmod;
/* system.yang */
static ncx_module_t *sysmod;
/* ietf-netconf-notifications.yang */
static ncx_module_t *ietf_netconf_notifications_mod;

/* cached pointer to the <system> element template */
static obj_template_t *ietf_system_state_obj;
static obj_template_t *ietf_system_obj;
static obj_template_t *yuma_system_obj;

/* cached pointers to the eventType nodes for this module */
static obj_template_t *sysStartupobj;
static obj_template_t *sysCapabilityChangeobj;
static obj_template_t *sysSessionEndobj;
static obj_template_t *sysConfirmedCommitobj;

static val_value_t *ietf_system_val = NULL;
static val_value_t *ietf_system_state_val = NULL;

/********************************************************************
 * FUNCTION init_static_vars
 *
 * Init the static vars
 *
 *********************************************************************/
static void
init_static_sys_vars(void)
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
    ietf_system_val = NULL;
    ietf_system_state_val = NULL;

} /* init_static_sys_vars */

/********************************************************************
 * FUNCTION get_api_string_router
 *
 *
 * This function is a router to return different field of different APi
 * following the diffent api_indicator
 *
 * INPUTS:
 *    api_indicator == indicator which field of which api to return
 *    ret == the pointer of the returned value, developer should free the ret when not used anymore
 *           or there might be a memory leak
 *
 * RETURNS:
 *    status
 *********************************************************************/
status_t get_api_string_router(int api_indicator, xmlChar *ret)
{
    xmlChar *buff;
    char strT[] = "T";
    char strZ[] = "Z";
    struct networkpb_Config *network_out;
    struct systempb_Config *sys_out;
    struct timepb_Config *time_out;
    struct systempb_Status *sys_status_out;
    struct timepb_Status *time_status_out;
    struct devicepb_Info *dev_info_out;
    struct emptypb_Empty *epty = malloc(sizeof(*(epty)));
    status_t res = NO_ERR;
    printf("\n@@@@@@@@@@@@@@@@@@@@@@ get_api_string_router %d\n", api_indicator);
    buff = malloc(sizeof(buff) * 512);
    memset(buff, 0, 512);

    switch (api_indicator)
    {
    case private_api_get_time_current_datetime:
        time_status_out = malloc(sizeof(*(time_status_out)));
        time_Time_GetStatus(epty, time_status_out);
        memcpy(buff, time_status_out->LocalDate, strlen(time_status_out->LocalDate));
        strcat(buff, strT);
        strcat(buff, time_status_out->LocalTime);
        strcat(buff, strZ);
        free(time_status_out);
        break;
    case private_api_get_system_last_boot_time:
        //     required pattern is
        //     `\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[\+\-]\d{2}:\d{2})`
        // **/
        // // "2022-07-07 08:57:44" => "2022-07-07T08:57:43Z"
        sys_status_out = malloc(sizeof(*(sys_status_out)));
        system_System_GetStatus(epty, sys_status_out);
        char *tmp = sys_status_out->LastBootTime;
        char *timebuf = strchr(tmp, ' ');
        if (timebuf != NULL)
        {
            timebuf += 1;
        }
        memcpy(buff, tmp, 10); // memcpy will force final char -> char*
        strcat(buff, strT);
        strcat(buff, timebuf);
        strcat(buff, strZ);
        free(sys_status_out);
        break;
    case private_api_get_network_hostname:
        network_out = malloc(sizeof(*(network_out)));
        network_Network_GetConfig(epty, network_out);
        memcpy(buff, network_out->Basic->HostName, strlen(network_out->Basic->HostName));
        free(network_out);
        break;
    case private_api_get_system_contact:
    case private_api_get_system_location:
        sys_out = malloc(sizeof(*(sys_out)));
        system_System_GetConfig(epty, sys_out);
        switch (api_indicator)
        {
        case private_api_get_system_contact:
            memcpy(buff, sys_out->SysContact, strlen(sys_out->SysContact));
            break;
        case private_api_get_system_location:
            memcpy(buff, sys_out->SysLocation, strlen(sys_out->SysLocation));
            break;
        }
        free(sys_out);
        break;
    case private_api_get_time_timezone:
        time_out = malloc(sizeof(*(time_out)));
        time_Time_GetConfig(epty, time_out);
        memcpy(buff, time_out->TimeZone, strlen(time_out->TimeZone));
        free(time_out);
        break;
    case private_api_get_system_os_name:
        memcpy(buff, fixed_os_name, sizeof(fixed_os_name));
        break;
    case private_api_get_device_current_version:
    case private_api_get_device_hw_version:
    case private_api_get_device_model:
        dev_info_out = malloc(sizeof(*(dev_info_out)));
        device_Device_GetDeviceInfo(epty, dev_info_out);
        switch (api_indicator)
        {
        case private_api_get_device_current_version:
            memcpy(buff, dev_info_out->CurrentSwVersion, strlen(dev_info_out->CurrentSwVersion));
            break;
        case private_api_get_device_hw_version:
            memcpy(buff, dev_info_out->HwVersion, strlen(dev_info_out->HwVersion));
            break;
        case private_api_get_device_model:
            memcpy(buff, dev_info_out->Model, strlen(dev_info_out->Model));
            break;
        }
        free(dev_info_out);
        break;
    }
    free(epty);
    memcpy(ret, buff, strlen(buff));
    return res;
} /* get_api_string_router */

static status_t
get_clock_current_datetime(xmlChar *ret)
{
    return get_api_string_router(private_api_get_time_current_datetime, ret);
}
static status_t
get_clock_boot_datetime(xmlChar *ret)
{
    return get_api_string_router(private_api_get_system_last_boot_time, ret);
}

static status_t
get_system_hostname(xmlChar *ret)
{
    return get_api_string_router(private_api_get_network_hostname, ret);
    ;
}

static status_t
get_system_contact(xmlChar *ret)
{
    return get_api_string_router(private_api_get_system_contact, ret);
}

static status_t
get_system_location(xmlChar *ret)
{
    return get_api_string_router(private_api_get_system_location, ret);
}

static status_t
get_clock_timezone(xmlChar *ret)
{
    return get_api_string_router(private_api_get_time_timezone, ret);
}

static status_t
get_platform_os_name(xmlChar *ret)
{
    return get_api_string_router(private_api_get_system_os_name, ret);
}

static status_t
get_platform_os_release(xmlChar *ret)
{
    return get_api_string_router(private_api_get_device_current_version, ret);
}

static status_t
get_platform_os_version(xmlChar *ret)
{
    return get_api_string_router(private_api_get_device_current_version, ret);
}

static status_t
get_platform_machine(xmlChar *ret)
{
    return get_api_string_router(private_api_get_device_hw_version, ret);
}

status_t set_api_string_router(int api_indicator, xmlChar *arg)
{
    status_t res = NO_ERR;
    struct emptypb_Empty *epty;
    struct systempb_Config *sys_config;
    struct networkpb_Config *network_config;
    struct timepb_Config *time_config;
    if (LOGDEBUG)
    {
        log_debug("\n in set_api_string_router, args is %s", arg);
    }
    switch (api_indicator)
    {
    case private_api_set_network_hostname:
        epty = malloc(sizeof(*epty));
        network_config = malloc(sizeof(*network_config));
        network_Network_GetConfig(epty, network_config);
        network_config->Basic->HostName = arg;
        network_Network_SetBasicConfig(network_config->Basic, epty);
        break;
    case private_api_set_system_contact:
    case private_api_set_system_location:
        epty = malloc(sizeof(*epty));
        sys_config = malloc(sizeof(*sys_config));
        system_System_GetConfig(epty, sys_config);
        if (api_indicator == private_api_set_system_contact)
        {
            sys_config->SysContact = arg;
        }
        else if (api_indicator == private_api_set_system_location)
        {
            sys_config->SysLocation = arg;
        }
        system_System_SetConfig(sys_config, epty);
        break;
    case private_api_set_time_timezone:
        epty = malloc(sizeof(*epty));
        time_config = malloc(sizeof(*time_config));
        printf("\n whay1 ");
        time_Time_GetConfig(epty, time_config);
        printf("\n whay2 ");
        time_config->TimeZone = arg;
        printf("\n whay3 ");
        time_Time_SetConfig(time_config, epty);
        printf("\n whay4 ");
        break;
    }
    return res;
}

/************* E X T E R N A L    F U N C T I O N S ***************/
static status_t
ietf_system_state_clock_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{
    status_t res = NO_ERR;
    val_value_t *childval;

    /* Add /system-state/clock/current-datetime */
    xmlChar *current_datetime;
    current_datetime = malloc(sizeof(current_datetime) * 20);
    memset(current_datetime, 0, sizeof(current_datetime) * 20);
    res = get_clock_current_datetime(current_datetime);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_state_current_datetime,
        current_datetime,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system-state/clock/boot-datetime */
    xmlChar *boot_datetime;
    boot_datetime = malloc(sizeof(boot_datetime) * 20);
    memset(boot_datetime, 0, sizeof(boot_datetime) * 20);
    res = get_clock_boot_datetime(boot_datetime);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_state_boot_datetime,
        boot_datetime,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    return res;
}

status_t ietf_system_state_clock_mro(val_value_t *parentval)
{
    status_t res = NO_ERR;
    val_init_virtual(
        parentval,
        ietf_system_state_clock_get,
        parentval->obj);
    return res;
}

status_t ieft_system_state_platform_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{
    status_t res = NO_ERR;
    val_value_t *childval = NULL;

    /* Add /system-state/platform/os-name */
    xmlChar *os_name;
    os_name = malloc(sizeof(os_name) * 512);
    memset(os_name, 0, sizeof(os_name) * 512);
    res = get_platform_os_name(os_name);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_state_os_name,
        os_name,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system-state/platform/os-release */
    xmlChar *os_release;
    os_release = malloc(sizeof(os_release) * 512);
    memset(os_release, 0, sizeof(os_release) * 512);
    res = get_platform_os_release(os_release);
    if (res != NO_ERR)
    {
        return res;
    }
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_state_os_release,
        os_release,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system-state/platform/os-version */
    xmlChar *os_version;
    os_version = malloc(sizeof(os_version) * 512);
    memset(os_version, 0, sizeof(os_version) * 512);
    res = get_platform_os_version(os_version);
    if (res != NO_ERR)
    {
        return res;
    }
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_state_os_version,
        os_version,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system-state/platform/machine */
    xmlChar *machine;
    machine = malloc(sizeof(machine) * 512);
    memset(machine, 0, sizeof(machine) * 512);
    res = get_platform_machine(machine);
    if (res != NO_ERR)
    {
        return res;
    }
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_state_machine,
        machine,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    return res;
}

status_t ietf_system_state_platform_mro(val_value_t *parentval)
{
    status_t res = NO_ERR;
    val_init_virtual(
        parentval,
        ieft_system_state_platform_get,
        parentval->obj);
    return res;
}

status_t ietf_system_state_mro(val_value_t *parentval)
{
    status_t res = NO_ERR;
    val_value_t *childval = NULL;
    /* Add /system-state/clock */
    res = agt_add_container(
        ietf_system,
        ietf_system_clock,
        parentval,
        &childval);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    res = ietf_system_state_clock_mro(childval);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system-state/platform */
    res = agt_add_container(
        ietf_system,
        ietf_system_state_platform,
        parentval,
        &childval);

    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    res = ietf_system_state_platform_mro(childval);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    return res;
}

status_t build_ntp_server(val_value_t *parentval, char *name, char *address)
{
    status_t res = NO_ERR;
    val_value_t *childval = NULL;
    val_value_t *udp_val = NULL;

    childval = agt_make_leaf(
        parentval->obj,
        ietf_system_ntp_server_name,
        name,
        &res);
    if (childval != NULL)
    {
        val_add_child_sorted(childval, parentval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    res = agt_add_container(
        ietf_system,
        ietf_system_ntp_server_udp,
        parentval,
        &udp_val);

    childval = agt_make_leaf(
        udp_val->obj,
        ietf_system_ntp_server_udp_address,
        address,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, udp_val);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    printf("\n======================");
    return res;
}

/********************************************************************
 * FUNCTION ietf_system_ntp_server_get
 *
 * Get database object callback
 * Path: /system/ntp
 * Fill in 'dstval' contents
 *
 * INPUTS:
 *     see ncx/getcb.h for details
 *
 * RETURNS:
 *     error status
 ********************************************************************/

static status_t
ietf_system_ntp_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{

    val_value_t *childval = NULL;
    val_value_t *primary_list_val = NULL;
    val_value_t *secondary_list_val = NULL;
    val_value_t *ntp_val = NULL;

    status_t res = NO_ERR;
    if (LOGDEBUG)
    {
        log_debug("\nEnter ietf_system_ntp_get");
    }
    /* calls the private api */
    struct emptypb_Empty *in = malloc(sizeof(*in));
    struct timepb_Config *out = malloc(sizeof(*out));
    time_Time_GetConfig(in, out);

    if (LOGDEBUG)
    {
        log_debug("\nadding ntp/enabled");
    }
    /* Add /system/ntp/enabled */
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_ntp_enabled,
        "true",
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system/ntp/server */
    primary_list_val = agt_make_list(
        dstval->obj,
        ietf_system_ntp_server,
        &res);
    if (primary_list_val != NULL)
    {
        val_add_child(primary_list_val, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    res = build_ntp_server(primary_list_val, "primary", out->MainNTPServer);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    if (strlen(out->BackupNTPServer) != 0)
    {
        /* Add /system/ntp/server */
        secondary_list_val = agt_make_list(
            dstval->obj,
            ietf_system_ntp_server,
            &res);
        if (secondary_list_val != NULL)
        {
            val_add_child(secondary_list_val, dstval);
        }
        else if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }

        res = build_ntp_server(secondary_list_val, "secondary", out->BackupNTPServer);
        if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }
    }

    return res;
}

status_t ietf_system_ntp_mro(val_value_t *parentval)
{
    status_t res = NO_ERR;
    val_init_virtual(
        parentval,
        ietf_system_ntp_get,
        parentval->obj);
    return res;
}

status_t build_radius_server(
    val_value_t *parentval,
    struct accesspb_AuthenticationServerEntry *entry)
{
    status_t res = NO_ERR;
    val_value_t *childval = NULL;
    val_value_t *udp_val = NULL;

    childval = agt_make_object(
        parentval->obj,
        ietf_system_radius_server_name,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, parentval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    res = val_set_simval_obj(
        childval,
        childval->obj,
        entry->Name);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    res = agt_add_container(
        ietf_system,
        ietf_system_radius_server_udp,
        parentval,
        &udp_val);

    childval = agt_make_leaf(
        udp_val->obj,
        ietf_system_radius_server_udp_address,
        entry->HostAddress,
        &res);

    if (childval != NULL)
    {
        val_add_child(childval, udp_val);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    childval = agt_make_int_leaf(
        udp_val->obj,
        ietf_system_radius_server_udp_authentication_port,
        entry->PortNumber,
        &res);

    if (childval != NULL)
    {
        val_add_child(childval, udp_val);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    childval = agt_make_leaf(
        udp_val->obj,
        ietf_system_radius_server_udp_shared_secret,
        entry->SharedSecret,
        &res);

    if (childval != NULL)
    {
        val_add_child(childval, udp_val);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    return res;
}

static status_t
ietf_system_radius_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{

    status_t res = NO_ERR;
    if (LOGDEBUG)
    {
        log_debug("\nEnter ietf_system_radius_get");
    }
    /* calls the private api */
    struct emptypb_Empty *in = malloc(sizeof(*in));
    struct accesspb_AuthenticationServersConfig *out = malloc(sizeof(*out));
    access_Access_GetAuthenticatorServerConfig(in, out);

    for (int i = 0; i < (out->List_Len); i++)
    {
        /* Only show the tacacs server*/
        if (out->List[i]->ServerType == accesspb_AuthenticationServerTypeOptions_AUTHENTICATION_SERVER_TYPE_TACACS)
        {
            continue;
        }
        val_value_t *child_val = NULL;

        child_val = agt_make_list(
            dstval->obj,
            ietf_system_radius_server,
            &res);
        if (child_val != NULL)
        {
            val_add_child(child_val, dstval);
        }
        else if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }

        res = build_radius_server(child_val, out->List[i]);
        if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }
    }

    return res;
}

status_t build_authentication_user(
    val_value_t *parentval,
    struct accesspb_UserEntry *entry)
{
    status_t res = NO_ERR;
    val_value_t *childval = NULL;
    val_value_t *udp_val = NULL;

    childval = agt_make_object(
        parentval->obj,
        ietf_system_ntp_server_name,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, parentval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    res = val_set_simval_obj(
        childval,
        childval->obj,
        entry->Name);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    return res;
}

static status_t
ietf_system_authentication_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{

    status_t res = NO_ERR;
    if (LOGDEBUG)
    {
        log_debug("\nEnter ietf_system_authentication_get");
    }
    /* calls the private api */
    struct emptypb_Empty *in = malloc(sizeof(*in));
    struct accesspb_UsersConfig *out = malloc(sizeof(*out));
    access_Access_GetUsers(in, out);

    for (int i = 0; i < (out->List_Len); i++)
    {
        val_value_t *child_val = NULL;

        child_val = agt_make_list(
            dstval->obj,
            ietf_system_authentication_user,
            &res);

        if (child_val != NULL)
        {
            val_add_child(child_val, dstval);
        }
        else if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }

        res = build_authentication_user(child_val, out->List[i]);
        if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }
    }
    free(in);
    free(out);

    return res;
}

status_t ietf_system_radius_mro(val_value_t *parentval)
{
    status_t res = NO_ERR;
    printf("\n@@@@ obj name %s\n", obj_get_name(parentval->obj));
    val_init_virtual(
        parentval,
        ietf_system_radius_get,
        parentval->obj);
    return res;
}

status_t ietf_system_authentication_mro(val_value_t *parentval)
{
    status_t res = NO_ERR;
    printf("\n@@@@ obj name %s\n", obj_get_name(parentval->obj));
    val_init_virtual(
        parentval,
        ietf_system_authentication_get,
        parentval->obj);
    return res;
}

static status_t
ietf_system_clock_timezone_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{

    status_t res = NO_ERR;
    if (LOGDEBUG)
    {
        log_debug("\nEnter ietf_system_clock_timezone_get");
        log_debug("\nwalter: virval name is %s", virval->name);
        log_debug("\nwalter: dstval name is %s", dstval->name);
    }

    val_value_t *childval;
    xmlChar *timezone;
    timezone = malloc(sizeof(timezone) * 512);
    memset(timezone, 0, sizeof(timezone) * 512);
    res = get_clock_timezone(timezone);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_clock_timezone_name,
        timezone,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    return res;
}

status_t ietf_system_clock_timezone_mro(val_value_t *parentval)
{
    status_t res = NO_ERR;
    val_init_virtual(
        parentval,
        ietf_system_clock_timezone_get,
        parentval->obj);
    return res;
}

status_t ietf_system_get(
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval)
{
    status_t res = NO_ERR;
    val_value_t *childval = NULL;
    val_value_t *clockval = NULL;
    val_value_t *radiusval = NULL;
    val_value_t *authenticationval = NULL;
    val_value_t *ntpval = NULL;

    xmlChar *hostname;
    hostname = malloc(sizeof(hostname) * 512);
    memset(hostname, 0, sizeof(hostname) * 512);
    res = get_system_hostname(hostname);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_hostname,
        hostname,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system/location */
    xmlChar *location;
    location = malloc(sizeof(location) * 512);
    memset(location, 0, sizeof(location) * 512);
    res = get_system_location(location);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_location,
        location,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system/contact */
    xmlChar *contact;
    contact = malloc(sizeof(contact) * 512);
    memset(contact, 0, sizeof(contact) * 512);
    res = get_system_contact(contact);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    childval = agt_make_leaf(
        dstval->obj,
        ietf_system_contact,
        contact,
        &res);
    if (childval != NULL)
    {
        val_add_child(childval, dstval);
    }
    else if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system/clock */
    log_debug("\n@@@@ add /system/clock\n");
    res = agt_add_container(
        ietf_system,
        ietf_system_clock,
        dstval,
        &clockval);
    res = ietf_system_clock_timezone_mro(clockval);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* calls the private api */
    struct emptypb_Empty *epty1 = malloc(sizeof(*epty1));
    struct accesspb_AuthenticationServersConfig *radius_out = malloc(sizeof(*radius_out));
    access_Access_GetAuthenticatorServerConfig(epty1, radius_out);
    if (radius_out->List_Len != 0)
    {
        res = agt_add_container(
            ietf_system,
            ietf_system_radius,
            dstval,
            &radiusval);
        res = ietf_system_radius_mro(radiusval);
        if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }
    }

    /* Add /system/authentication */
    res = agt_add_container(
        ietf_system,
        ietf_system_authentication,
        dstval,
        &authenticationval);

    /* Add /system/authentication/user/name */
    res = ietf_system_authentication_mro(authenticationval);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    struct emptypb_Empty *ety = malloc(sizeof(*ety));
    struct timepb_Config *time_confg = malloc(sizeof(*time_confg));
    time_Time_GetConfig(ety, time_confg);

    if (time_confg->Mode == timepb_ModeTypeOptions_MODE_TYPE_AUTO)
    {
        res = agt_add_container(
            ietf_system,
            ietf_system_ntp,
            dstval,
            &ntpval);
        res = ietf_system_ntp_mro(ntpval);
        if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }
    }

    return res;
}

status_t ietf_system_mro(val_value_t *parentval)
{
    status_t res = NO_ERR;
    val_init_virtual(
        parentval,
        ietf_system_get,
        parentval->obj);
    return res;
}

static status_t
ietf_system_callback_router(
    int api_indicator,
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    status_t res;
    val_value_t *errorval = (curval) ? curval : newval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    if (LOGDEBUG)
    {
        log_debug("\n >>>>> walter: func = api_indicator %d", api_indicator);
        log_debug("\n cb = %s , op = %s", agt_cbtype_name(cbtyp), op_editop_name(editop));
        if (newval != NULL)
        {
            log_debug("\n newval = %s", VAL_STRING(newval));
            log_debug("\n newval.name is %s", newval->name);
            val_dump_value(newval, 0);
        }
        if (curval != NULL)
        {
            log_debug("\n curval = %s", VAL_STRING(curval));
            log_debug("\n curval.name is %s", curval->name);
            val_dump_value(curval, 0);
        }
    }

    switch (cbtyp)
    {
    case AGT_CB_VALIDATE:
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        switch (editop)
        {
        case OP_EDITOP_LOAD:
            break;
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
            if (newval != NULL)
            {
                printf("\nSetting newval %s\n", VAL_STRING(newval));
                res = set_api_string_router(api_indicator, VAL_STRING(newval));
            }
            break;
        case OP_EDITOP_CREATE:
            break;
        case OP_EDITOP_DELETE:
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
    case AGT_CB_ROLLBACK:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    log_debug("\n res is %s", get_error_string(res));
    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR)
    {
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
    log_debug("\n walter: b4 ietf_system_callback_router return");
    return res;
}

static status_t
ietf_system_ntp_edit_handler(
    char *handler_caller,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    status_t res = NO_ERR;
    val_value_t *server_name_val, *udp_val, *address_val;
    if (LOGDEBUG)
    {
        log_debug("\n===============================================");
        log_debug("\nwalter: func = ietf_system_ntp_edit_handler , called by %s", handler_caller);
        log_debug("\nwalter: cb = %s , op = %s", agt_cbtype_name(cbtyp), op_editop_name(editop));
        log_debug("\nwalter: newval is NULL: %s, curval is NULL %s", newval == NULL ? "yes" : "no", curval == NULL ? "yes" : "no");
        if (newval != NULL)
        {
            log_debug("\nwalter: newval.name is %s", newval->name);
            val_dump_value(newval, 0);
        }
        if (curval != NULL)
        {
            log_debug("\nwalter: curval.name is %s", curval->name);
            val_dump_value(curval, 0);
        }
        log_debug("\n===============================================");
    }

    switch (cbtyp)
    {
    case AGT_CB_VALIDATE:
        break;
    case AGT_CB_APPLY:
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        /* database manipulation done here */
        switch (editop)
        {
        case OP_EDITOP_LOAD:
            break;
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
            log_debug("\nwalter: op = %s, newval.name is %s", op_editop_name(editop), newval->name);
            struct timepb_Config *time_config = malloc(sizeof(*time_config));
            struct emptypb_Empty *epty = malloc(sizeof(*epty));
            time_Time_GetConfig(epty, time_config);
            if (curval != NULL)
            {
                /*
                curval existing means this val has been created and
                */
                log_debug("\nwalter: op = %s, curval.name is %s", op_editop_name(editop), curval->name);
                log_debug("\nwalter: op = %s, curval.parent.name is %s", op_editop_name(editop), curval->parent->name);
                udp_val = curval->parent;
                log_debug("\nwalter: udp_val,parent is %s", udp_val->parent->name);
                if (udp_val != NULL)
                {
                    server_name_val = val_find_child(udp_val->parent, ietf_system, ietf_system_ntp_server_name);
                    if (server_name_val != NULL)
                    {
                        log_debug("\nwalter: server_name_val's val is %s", VAL_STRING(server_name_val));
                        if (xml_strcmp(VAL_STRING(server_name_val), "primary") == 0)
                        {
                            time_config->MainNTPServer = VAL_STRING(newval);
                        }
                        else if (xml_strcmp(VAL_STRING(server_name_val), "secondary") == 0)
                        {
                            time_config->BackupNTPServer = VAL_STRING(newval);
                        }
                    }
                }
            }
            else if (newval != NULL)
            {
                /*
                    Other than the upper case, the newval should always exist,
                */
                server_name_val = val_find_child(newval, ietf_system, ietf_system_ntp_server_name);
                if (server_name_val != NULL)
                {
                    log_debug("\nwalter: server's name is %s", VAL_STRING(server_name_val));
                    udp_val = val_find_child(newval, ietf_system, ietf_system_ntp_server_udp);
                    if (udp_val != NULL)
                    {
                        address_val = val_find_child(udp_val, ietf_system, ietf_system_ntp_server_udp_address);
                        log_debug("\nwalter: address is %s", VAL_STRING(address_val));
                        if (xml_strcmp(VAL_STRING(server_name_val), "primary") == 0)
                        {
                            time_config->MainNTPServer = VAL_STRING(address_val);
                        }
                        else if (xml_strcmp(VAL_STRING(server_name_val), "secondary") == 0)
                        {
                            time_config->BackupNTPServer = VAL_STRING(address_val);
                        }
                    }
                }
            }
            time_Time_SetConfig(time_config, epty);
            log_debug("\n==================================");
            log_debug("\nwalter: API applyied!!!!!!!!");
            log_debug("\n==================================");
            // should check the situation that the curval exists
            break;
        case OP_EDITOP_CREATE:
            break;
        case OP_EDITOP_DELETE:
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;

    case AGT_CB_ROLLBACK:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    return res;
}

static status_t
ietf_system_ntp_server_edit(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    status_t res;
    val_value_t *errorval = (curval) ? curval : newval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    res = ietf_system_ntp_edit_handler(
        "ietf_system_ntp_server_edit",
        cbtyp,
        editop,
        newval,
        curval);
    log_debug("\nwalter: res is %s", get_error_string(res));
    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR)
    {
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
    log_debug("\nwalter: b4 ietf_system_ntp_server_edit return");
    return res;
}

static status_t
ietf_system_ntp_edit(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    status_t res;
    val_value_t *errorval = (curval) ? curval : newval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    res = ietf_system_ntp_edit_handler(
        "ietf_system_ntp_edit",
        cbtyp,
        editop,
        newval,
        curval);
    log_debug("\nwalter: res is %s", get_error_string(res));
    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR)
    {
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
    log_debug("\nwalter: b4 ietf_system_ntp_edit return");
    return res;
}

static status_t
ietf_system_ntp_enabled_edit(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    status_t res;
    val_value_t *errorval = (curval) ? curval : newval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    if (LOGDEBUG)
    {
        log_debug("\n======================================================");
        log_debug("\nwalter: func = ietf_system_ntp_enabled_edit ");
        log_debug("\nwalter: cb = %s , op = %s", agt_cbtype_name(cbtyp), op_editop_name(editop));
        log_debug("\nwalter: newval is NULL: %s, curval is NULL %s", newval == NULL ? "yes" : "no", curval == NULL ? "yes" : "no");
        if (newval != NULL)
        {
            log_debug("\nwalter: newval.name is %s", newval->name);
            val_dump_value(newval, 0);
        }
        if (curval != NULL)
        {
            log_debug("\nwalter: curval.name is %s", curval->name);
            val_dump_value(curval, 0);
        }
        log_debug("\n======================================================");
    }

    switch (cbtyp)
    {
    case AGT_CB_VALIDATE:
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        switch (editop)
        {
        case OP_EDITOP_LOAD:
            break;
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
            if (newval != NULL)
            {
                log_debug("\nwalter: Setting bool %s\n", VAL_BOOL(newval) ? "true" : "false");
                struct timepb_Config *time_config = malloc(sizeof(*time_config));
                struct emptypb_Empty *epty = malloc(sizeof(*epty));
                // log_debug("\nwalter: whay1 ");
                time_Time_GetConfig(epty, time_config);
                // log_debug("\nwalter: whay2 ");
                if (VAL_BOOL(newval))
                {
                    time_config->Mode = timepb_ModeTypeOptions_MODE_TYPE_AUTO;
                }
                else
                {
                    time_config->Mode = timepb_ModeTypeOptions_MODE_TYPE_MANUAL;
                }
                time_Time_SetConfig(time_config, epty);
            }
            break;
        case OP_EDITOP_CREATE:
            break;
        case OP_EDITOP_DELETE:
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        break;
    case AGT_CB_ROLLBACK:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    log_debug("\nwalter: res is %s", get_error_string(res));
    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR)
    {
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
    log_debug("\nwalter: b4 ietf_system_ntp_enabled_edit return");
    return res;
}

static status_t
ietf_system_clock_edit(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    status_t res;
    val_value_t *errorval = (curval) ? curval : newval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    if (LOGDEBUG)
    {
        log_debug("\nwalter: func = ietf_system_clock_edit ");
        log_debug("\nwalter: cb = %s , op = %s", agt_cbtype_name(cbtyp), op_editop_name(editop));
        log_debug("\nwalter: newval is NULL: %s, curval is NULL %s", newval == NULL ? "yes" : "no", curval == NULL ? "yes" : "no");
        if (newval != NULL)
        {
            log_debug("\nwalter: newval11 ");
            log_debug("\nwalter: newval.name is %s", newval->name);
            log_debug("\nwalter: newval22 ");
            val_dump_value(newval, 0);
        }
        if (curval != NULL)
        {
            log_debug("\nwalter: curval11 ");
            log_debug("\nwalter: curval.name is %s", curval->name);
            log_debug("\nwalter: curval22 ");
            val_dump_value(curval, 0);
        }
    }

    switch (cbtyp)
    {
    case AGT_CB_VALIDATE:
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        switch (editop)
        {
        case OP_EDITOP_LOAD:
            break;
        case OP_EDITOP_MERGE:
            break;
        case OP_EDITOP_REPLACE:
            break;
        case OP_EDITOP_CREATE:
            break;
        case OP_EDITOP_DELETE:
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        break;
    case AGT_CB_ROLLBACK:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    log_debug("\nwalter: res is %s", get_error_string(res));
    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR)
    {
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
    log_debug("\nwalter: b4 ietf_system_clock_edit return");
    return res;
}

static status_t
ietf_system_clock_timzone_name_edit(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    status_t res;
    val_value_t *errorval = (curval) ? curval : newval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    if (LOGDEBUG)
    {
        log_debug("\nwalter: func = ietf_system_clock_timezone_name_edit ");
        log_debug("\nwalter: cb = %s , op = %s", agt_cbtype_name(cbtyp), op_editop_name(editop));
        log_debug("\nwalter: newval is NULL: %s, curval is NULL %s", newval == NULL ? "yes" : "no", curval == NULL ? "yes" : "no");
        if (newval != NULL)
        {
            log_debug("\nwalter: newval11 ");
            log_debug("\nwalter: newval.name is %s", newval->name);
            log_debug("\nwalter: newval22 ");
            log_debug("\nwalter: newval = %s", VAL_STRING(newval));
            log_debug("\nwalter: newval33 ");
            val_dump_value(newval, 0);
        }
        if (curval != NULL)
        {
            log_debug("\nwalter: curval11 ");
            log_debug("\nwalter: curval.name is %s", curval->name);
            log_debug("\nwalter: curval22 ");
            log_debug("\nwalter: curval = %s", VAL_STRING(curval));
            log_debug("\nwalter: curval33 ");
            val_dump_value(curval, 0);
        }
    }

    switch (cbtyp)
    {
    case AGT_CB_VALIDATE:
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        switch (editop)
        {
        case OP_EDITOP_LOAD:
            break;
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
            if (newval != NULL)
            {
                log_debug("\nwalter: Setting newval %s\n", VAL_STRING(newval));
                struct timepb_Config *time_config = malloc(sizeof(*time_config));
                struct emptypb_Empty *epty = malloc(sizeof(*epty));
                log_debug("\nwalter: whay1 ");
                time_Time_GetConfig(epty, time_config);
                log_debug("\nwalter: whay2 ");
                time_config->TimeZone = VAL_STRING(newval);
                log_debug("\nwalter: whay3 ");
                time_Time_SetConfig(time_config, epty);
            }
            break;
        case OP_EDITOP_CREATE:
            break;
        case OP_EDITOP_DELETE:
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        break;
    case AGT_CB_ROLLBACK:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    log_debug("\nwalter: res is %s", get_error_string(res));
    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR)
    {
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
    log_debug("\nwalter: b4 ietf_system_clock_timzone_name_edit return");
    return res;
}

static status_t
ietf_system_hostname_edit(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    return ietf_system_callback_router(
        private_api_set_network_hostname,
        scb,
        msg,
        cbtyp,
        editop,
        newval,
        curval);
} /* ietf_system_hostname_edit */

static status_t
ietf_system_contact_edit(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    return ietf_system_callback_router(
        private_api_set_system_contact,
        scb,
        msg,
        cbtyp,
        editop,
        newval,
        curval);
} /* ietf_system_contact_edit */

static status_t
ietf_system_location_edit(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    agt_cbtyp_t cbtyp,
    op_editop_t editop,
    val_value_t *newval,
    val_value_t *curval)
{
    return ietf_system_callback_router(
        private_api_set_system_location,
        scb,
        msg,
        cbtyp,
        editop,
        newval,
        curval);
} /* ietf_system_location_edit */

static status_t
ietf_system_system_restart(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode)
{
    struct emptypb_Empty *in = malloc(sizeof(*(in)));
    struct commonpb_Confirm *confirm = malloc(sizeof(*(confirm)));
    confirm->Confirm = "CONFIRM";
    maintenance_Maintenance_RunRebootDevice(confirm, in);
    return NO_ERR;
}

char *replace_string(
    char *original,
    char *pattern,
    char *replacement)
{
    size_t const replen = strlen(replacement);
    size_t const patlen = strlen(pattern);
    size_t const orilen = strlen(original);

    size_t patcnt = 0;
    const char *oriptr;
    const char *patloc;

    // find how many times the pattern occurs in the original string
    for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
    {
        patcnt++;
    }

    {
        // allocate memory for the new string
        size_t const retlen = orilen + patcnt * (replen - patlen);
        char *const returned = (char *)malloc(sizeof(char) * (retlen + 1));

        if (returned != NULL)
        {
            // copy the original string,
            // replacing all the instances of the pattern
            char *retptr = returned;
            for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
            {
                size_t const skplen = patloc - oriptr;
                // copy the section until the occurence of the pattern
                strncpy(retptr, oriptr, skplen);
                retptr += skplen;
                // copy the replacement
                strncpy(retptr, replacement, replen);
                retptr += replen;
            }
            // copy the rest of the string.
            strcpy(retptr, oriptr);
        }
        return returned;
    }
}

static status_t
ietf_system_set_current_datetime(
    ses_cb_t *scb,
    rpc_msg_t *msg,
    xml_node_t *methnode)
{
    val_value_t *current_datetime_val;

    current_datetime_val = val_find_child(
        msg->rpc_input,
        ietf_system,
        ietf_system_state_current_datetime);
    assert(current_datetime_val != NULL);

    struct emptypb_Empty *in = malloc(sizeof(*(in)));
    struct emptypb_Empty *in2 = malloc(sizeof(*(in2)));
    struct timepb_Config *time_cfg = malloc(sizeof(*(time_cfg)));

    time_Time_GetConfig(in, time_cfg);
    if (time_cfg->Mode != timepb_ModeTypeOptions_MODE_TYPE_MANUAL)
    {
        return ERR_NCX_OPERATION_FAILED;
    }
    // time_Time_SetConfig(struct timepb_Config * param0, struct emptypb_Empty * param1);

    log_debug("\nwalter val is %s", VAL_STRING(current_datetime_val));
    char *tmp = replace_string(VAL_STRING(current_datetime_val), "Z", "");
    tmp = replace_string(tmp, "T", " ");
    time_cfg->Manual = tmp;
    time_Time_SetConfig(time_cfg, in2);
    return NO_ERR;
}

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
payload_error(const xmlChar *name,
              status_t res)
{
    log_error("\nError: cannot make payload leaf '%s' (%s)",
              name, get_error_string(res));

} /* payload_error */

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
set_log_level_invoke(ses_cb_t *scb,
                     rpc_msg_t *msg,
                     xml_node_t *methnode)
{
    val_value_t *levelval;
    log_debug_t levelenum;
    status_t res;

    (void)scb;
    (void)methnode;

    res = NO_ERR;

    /* get the level parameter */
    levelval = val_find_child(msg->rpc_input,
                              AGT_SYS_MODULE,
                              NCX_EL_LOGLEVEL);
    if (levelval)
    {
        if (levelval->res == NO_ERR)
        {
            levelenum =
                log_get_debug_level_enum((const char *)
                                             VAL_ENUM_NAME(levelval));
            if (levelenum != LOG_DEBUG_NONE)
            {
                log_set_debug_level(levelenum);
            }
            else
            {
                res = ERR_NCX_OPERATION_FAILED;
            }
        }
        else
        {
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
send_sysStartup(void)
{
    agt_not_msg_t * not ;
    cfg_template_t *cfg;
    val_value_t *leafval, *bootErrorval;
    rpc_err_rec_t *rpcerror;
    obj_template_t *bootErrorobj;
    status_t res;

    log_debug("\nagt_sys: generating <sysStartup> notification");

    not = agt_not_new_notification(sysStartupobj);
    if (!not )
    {
        log_error("\nError: malloc failed; cannot send <sysStartup>");
        return;
    }

    /* add sysStartup/startupSource */
    cfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (cfg)
    {
        if (cfg->src_url)
        {
            leafval = agt_make_leaf(sysStartupobj,
                                    (const xmlChar *)"startupSource", cfg->src_url, &res);
            if (leafval)
            {
                agt_not_add_to_payload(not, leafval);
            }
            else
            {
                payload_error((const xmlChar *)"startupSource", res);
            }
        }

        /* add sysStartup/bootError for each error recorded */
        if (!dlq_empty(&cfg->load_errQ))
        {
            /* get the bootError object */
            bootErrorobj = obj_find_child(sysStartupobj, AGT_SYS_MODULE,
                                          (const xmlChar *)"bootError");
            if (bootErrorobj)
            {
                rpcerror = (rpc_err_rec_t *)dlq_firstEntry(&cfg->load_errQ);
                for (; rpcerror;
                     rpcerror = (rpc_err_rec_t *)dlq_nextEntry(rpcerror))
                {
                    /* make the bootError value struct */
                    bootErrorval = val_new_value();
                    if (!bootErrorval)
                    {
                        payload_error((const xmlChar *)"bootError",
                                      ERR_INTERNAL_MEM);
                    }
                    else
                    {
                        val_init_from_template(bootErrorval, bootErrorobj);
                        res = agt_rpc_fill_rpc_error(rpcerror, bootErrorval);
                        if (res != NO_ERR)
                        {
                            log_error("\nError: problems making <bootError> "
                                      "(%s)",
                                      get_error_string(res));
                        }
                        /* add even if there are some missing leafs */
                        agt_not_add_to_payload(not, bootErrorval);
                    }
                }
            }
            else
            {
                SET_ERROR(ERR_INTERNAL_VAL);
            }
        }
    }
    else
    {
        SET_ERROR(ERR_INTERNAL_VAL);
    }

    agt_not_queue_notification(not );
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
netconf_notifications_add_common_session_parms(const ses_cb_t *scb,
                                               agt_not_msg_t * not,
                                               val_value_t *parentval)
{
    obj_template_t *parent_obj;
    val_value_t *leafval;
    status_t res;
    ses_id_t use_sid;

    if (not != NULL)
    {
        assert(parentval == NULL);
        parent_obj = not ->notobj;
    }
    else if (parentval != NULL)
    {
        assert(not == NULL);
        parent_obj = parentval->obj;
    }
    else
    {
        assert(0);
    }

    /* add userName */
    if (scb->username)
    {
        leafval = agt_make_leaf(parent_obj,
                                "username",
                                scb->username,
                                &res);
        assert(leafval);
        if (not )
        {
            agt_not_add_to_payload(not, leafval);
        }
        else
        {
            val_add_child(leafval, parentval);
        }
    }

    /* add sessionId */
    if (scb->sid)
    {
        use_sid = scb->sid;
    }
    else if (scb->rollback_sid)
    {
        use_sid = scb->rollback_sid;
    }
    else
    {
        res = ERR_NCX_NOT_IN_RANGE;
        use_sid = 0;
    }

    if (use_sid)
    {
        leafval = agt_make_uint_leaf(parent_obj,
                                     "session-id",
                                     use_sid,
                                     &res);
        assert(leafval);
        if (not )
        {
            agt_not_add_to_payload(not, leafval);
        }
        else
        {
            val_add_child(leafval, parentval);
        }
    }

    /* add remoteHost */
    if (scb->peeraddr)
    {
        leafval = agt_make_leaf(parent_obj,
                                "source-host",
                                scb->peeraddr,
                                &res);
        assert(leafval);
        if (not )
        {
            agt_not_add_to_payload(not, leafval);
        }
        else
        {
            val_add_child(leafval, parentval);
        }
    }

} /* netconf_notifications_add_common_session_parms */

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
agt_sys_init(void)
{
    agt_profile_t *agt_profile;
    status_t res;
    printf("\nwalter: @@@@@ agt_sys_init @@@\n");
    if (agt_sys_init_done)
    {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    if (LOGDEBUG2)
    {
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
    if (res != NO_ERR)
    {
        return res;
    }

    /* load the ietf-system module */
    res = ncxmod_load_module(AGT_IETF_SYS_MODULE,
                             NULL,
                             &agt_profile->agt_savedevQ,
                             &ietf_sysmod);
    if (res != NO_ERR)
    {
        return res;
    }

    /* load the ietf-netconf-notifications module */
    res = ncxmod_load_module("ietf-netconf-notifications",
                             NULL,
                             &agt_profile->agt_savedevQ,
                             &ietf_netconf_notifications_mod);
    if (res != NO_ERR)
    {
        return res;
    }

    /* find the object definition for the system element */
    ietf_system_state_obj = ncx_find_object(ietf_sysmod,
                                            ietf_system_N_system_state);

    if (!ietf_system_state_obj)
    {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* find the object definition for the system element */
    ietf_system_obj = ncx_find_object(ietf_sysmod,
                                      ietf_system_N_system);

    if (!ietf_system_obj)
    {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* disable not supported features */
    agt_disable_feature(ietf_system, "dns-resolver");
    agt_disable_feature(ietf_system, "dns-udp-tcp-port");
    agt_disable_feature(ietf_system, "ntp-udp-port");

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/hostname",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_system_hostname_edit);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/contact",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_system_contact_edit);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/location",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_system_location_edit);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /*
        The callback for the container is still required, even the
        registered callback would do nothing, otherwise it would
        take two times of command to apply the changes to the device
    */
    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/clock",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_system_clock_edit);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/clock/timezone-name",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_system_clock_timzone_name_edit);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    /*
        Require register callback at /system/ntp/ to avoid
        applying won't take effect until the second apply
    */
    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/ntp",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_system_ntp_edit);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/ntp/server",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_system_ntp_server_edit);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }
    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/ntp/enabled",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        ietf_system_ntp_enabled_edit);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    yuma_system_obj =
        obj_find_child(ietf_system_state_obj,
                       AGT_SYS_MODULE,
                       system_N_system);
    if (!yuma_system_obj)
    {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    sysStartupobj =
        ncx_find_object(sysmod,
                        system_N_sysStartup);
    if (!sysStartupobj)
    {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* set up set-log-level RPC operation */
    res = agt_rpc_register_method(AGT_SYS_MODULE,
                                  system_N_set_log_level,
                                  AGT_RPC_PH_INVOKE,
                                  set_log_level_invoke);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(
        ietf_system,
        "set-current-datetime",
        AGT_RPC_PH_INVOKE,
        ietf_system_set_current_datetime);
    if (res != NO_ERR)
    {
        return res;
    }

    res = agt_rpc_register_method(
        ietf_system,
        "system-restart",
        AGT_RPC_PH_INVOKE,
        ietf_system_system_restart);
    if (res != NO_ERR)
    {
        return res;
    }

    return NO_ERR;

} /* agt_sys_init */

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
agt_sys_init2(void)
{

    cfg_template_t *runningcfg;
    status_t res;

    printf("\n@@@@@ agt_sys_init2\n");
    if (!agt_sys_init_done)
    {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }
    ietf_system_val = agt_init_cache(
        ietf_system,
        ietf_system_N_system,
        &res);

    if (res != NO_ERR)
    {
        return res;
    }

    /* Add /system */
    if (ietf_system_val == NULL)
    {
        res = agt_add_top_container(ietf_system_obj, &ietf_system_val);
        if (res != NO_ERR)
        {
            return res;
        }
    }
    res = ietf_system_mro(ietf_system_val);
    if (res != NO_ERR)
    {
        return SET_ERROR(res);
    }

    /* Add /system-state */
    ietf_system_state_val = agt_init_cache(
        ietf_system,
        ietf_system_N_system_state,
        &res);

    if (res != NO_ERR)
    {
        return res;
    }

    if (ietf_system_state_val == NULL)
    {
        res = agt_add_top_container(ietf_system_state_obj, &ietf_system_state_val);
        if (res != NO_ERR)
        {
            return SET_ERROR(res);
        }
    }
    res = ietf_system_state_mro(ietf_system_state_val);
    printf("\n @@@@@@ done adding system-state\n");

    /* add sysStartup to notificationQ */
    send_sysStartup();

    return NO_ERR;

} /* agt_sys_init2 */

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
void agt_sys_cleanup(void)
{
    if (agt_sys_init_done)
    {
        init_static_sys_vars();
        agt_sys_init_done = FALSE;
        agt_rpc_unregister_method(AGT_SYS_MODULE,
                                  system_N_set_log_level);
    }

    agt_cb_unregister_callbacks(
        ietf_system,
        (const xmlChar *)"/system/hostname");

    agt_cb_unregister_callbacks(
        ietf_system,
        (const xmlChar *)"/system/contact");

    agt_cb_unregister_callbacks(
        ietf_system,
        (const xmlChar *)"/system/location");

    agt_cb_unregister_callbacks(
        ietf_system,
        (const xmlChar *)"/system/clock");

    agt_cb_unregister_callbacks(
        ietf_system,
        (const xmlChar *)"/system/clock/timezone-name");

    agt_cb_unregister_callbacks(
        ietf_system,
        (const xmlChar *)"/system/ntp");

    agt_cb_unregister_callbacks(
        ietf_system,
        (const xmlChar *)"/system/ntp/server");

    agt_cb_unregister_callbacks(
        ietf_system,
        (const xmlChar *)"/system/ntp/enabled");

} /* agt_sys_cleanup */

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
void agt_sys_send_netconf_session_start(const ses_cb_t *scb)
{
    agt_not_msg_t * not ;
    obj_template_t *netconf_session_start_obj;

    if (LOGDEBUG)
    {
        log_debug("\nagt_sys: generating <netconf-session-start> "
                  "notification");
    }

    netconf_session_start_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-session-start");
    assert(netconf_session_start_obj);

    not = agt_not_new_notification(netconf_session_start_obj);
    if (!not )
    {
        log_error("\nError: malloc failed; cannot "
                  "send <netconf-session-start>");
        return;
    }

    netconf_notifications_add_common_session_parms(scb, not, NULL /*parentval*/);

    agt_not_queue_notification(not );

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
static const xmlChar *
get_termination_reason_str(ses_term_reason_t termreason)
{
    const xmlChar *termreasonstr;

    switch (termreason)
    {
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
void agt_sys_send_netconf_session_end(const ses_cb_t *scb,
                                      ses_term_reason_t termination_reason,
                                      ses_id_t killed_by)
{
    agt_not_msg_t * not ;
    val_value_t *leafval;
    const xmlChar *termination_reason_str;
    status_t res;

    obj_template_t *netconf_session_end_obj;

    assert(scb && "agt_sys_send_netconf_session_end() - param scb is NULL");

    log_debug("\nagt_sys: generating <netconf-session-end> notification");

    netconf_session_end_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-session-end");
    assert(netconf_session_end_obj);

    not = agt_not_new_notification(netconf_session_end_obj);
    assert(not );

    /* session started;  not just being killed
     * in the <ncxconnect> message handler */
    if (termination_reason != SES_TR_BAD_START)
    {
        netconf_notifications_add_common_session_parms(scb, not, NULL /*parentval*/);
    }

    /* add sysSessionEnd/killedBy */
    if (termination_reason == SES_TR_KILLED)
    {
        leafval = agt_make_uint_leaf(netconf_session_end_obj, "killed-by",
                                     killed_by, &res);
        assert(leafval);
        agt_not_add_to_payload(not, leafval);
    }

    /* add sysSessionEnd/terminationReason */
    termination_reason_str = get_termination_reason_str(termination_reason);
    leafval = agt_make_leaf(netconf_session_end_obj, "termination-reason",
                            termination_reason_str, &res);
    assert(leafval);

    agt_not_add_to_payload(not, leafval);

    agt_not_queue_notification(not );
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
void agt_sys_send_netconf_config_change(const ses_cb_t *scb,
                                        dlq_hdr_t *auditrecQ)
{
    agt_not_msg_t * not ;
    agt_cfg_audit_rec_t *auditrec;
    val_value_t *leafval, *listval;
    obj_template_t *netconf_config_change_obj;
    obj_template_t *listobj;
    status_t res;

#ifdef DEBUG
    if (!scb || !auditrecQ)
    {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (LOGDEBUG)
    {
        log_debug("\nagt_sys: generating <netconf-config-change> "
                  "notification");
    }
    netconf_config_change_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-config-change");
    assert(netconf_config_change_obj);

    not = agt_not_new_notification(netconf_config_change_obj);
    assert(not );
    {
        obj_template_t *changed_by_obj;
        val_value_t *changed_by_val;

        changed_by_obj =
            obj_find_child(not ->notobj,
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
    if (listobj == NULL)
    {
        SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }
    else
    {
        for (auditrec = (agt_cfg_audit_rec_t *)dlq_firstEntry(auditrecQ);
             auditrec != NULL;
             auditrec = (agt_cfg_audit_rec_t *)dlq_nextEntry(auditrec))
        {

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

    agt_not_queue_notification(not );

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
void agt_sys_send_netconf_capability_change(ses_cb_t *changed_by,
                                            boolean is_add,
                                            const xmlChar *capstr)
{
    agt_not_msg_t * not ;
    obj_template_t *netconf_capability_change_obj;
    val_value_t *leafval;
    status_t res;

#ifdef DEBUG
    if (!capstr)
    {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (LOGDEBUG)
    {
        log_debug("\nagt_sys: generating <netconf-capability-change> "
                  "notification");
    }

    netconf_capability_change_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-capability-change");
    assert(netconf_capability_change_obj);

    not = agt_not_new_notification(netconf_capability_change_obj);
    assert(not );

    {
        obj_template_t *changed_by_obj;
        val_value_t *changed_by_val;

        changed_by_obj =
            obj_find_child(not ->notobj,
                           "ietf-netconf-notifications",
                           "changed-by");
        assert(changed_by_obj);
        changed_by_val = val_new_value();
        val_init_from_template(changed_by_val, changed_by_obj);

        if (changed_by)
        {
            netconf_notifications_add_common_session_parms(changed_by, NULL /*not*/, changed_by_val);
        }
        else
        {
            leafval = agt_make_leaf(changed_by_obj,
                                    "server",
                                    NULL,
                                    &res);
            assert(leafval);
            val_add_child(leafval, changed_by_val);
        }
        agt_not_add_to_payload(not, changed_by_val);
    }

    if (is_add)
    {
        /* add netconf-capability-change/added-capability */
        leafval = agt_make_leaf(netconf_capability_change_obj,
                                "added-capability",
                                capstr,
                                &res);
    }
    else
    {
        /* add netconf-capability-change/deleted-capability */
        leafval = agt_make_leaf(netconf_capability_change_obj,
                                "deleted-capability",
                                capstr,
                                &res);
    }
    assert(leafval);
    agt_not_add_to_payload(not, leafval);

    agt_not_queue_notification(not );

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
void agt_sys_send_netconf_confirmed_commit(const ses_cb_t *scb,
                                           ncx_confirm_event_t event)
{
    agt_not_msg_t * not ;
    obj_template_t *netconf_confirmed_commit_obj;
    val_value_t *leafval;
    const xmlChar *eventstr;
    status_t res;

    res = NO_ERR;

    eventstr = ncx_get_confirm_event_str(event);

    if (!eventstr)
    {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }

    if (LOGDEBUG)
    {
        log_debug("\nagt_sys: generating <netconf-confirmed-commit> "
                  "notification (%s)",
                  eventstr);
    }

    netconf_confirmed_commit_obj =
        ncx_find_object(ietf_netconf_notifications_mod,
                        "netconf-confirmed-commit");
    assert(netconf_confirmed_commit_obj);

    not = agt_not_new_notification(netconf_confirmed_commit_obj);
    assert(not );

    if (event != NCX_CC_EVENT_TIMEOUT)
    {
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

    agt_not_queue_notification(not );

} /* agt_sys_send_netconf_confirmed_commit */

/* END file agt_sys.c */
