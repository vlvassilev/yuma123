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
#define ietf_system_radius_server_name (const xmlChar *)"name"
#define ietf_system_radius_server_udp (const xmlChar *)"udp"
#define ietf_system_radius_server_udp_address (const xmlChar *)"address"
#define ietf_system_radius_server_udp_authentication_port (const xmlChar *)"authentication-port"
#define ietf_system_radius_server_udp_shared_secret (const xmlChar *)"shared-secret"

#define private_api_get_network_hostname (int) 0
#define private_api_get_system_location (int) 1
#define private_api_get_system_contact (int) 2
#define private_api_get_ntp_timezone (int) 3
#define private_api_get_system_boot_datetime (int) 4
#define private_api_get_system_current_datetime (int) 5
#define private_api_get_system_software_version (int) 6
#define private_api_get_system_hardware_version (int) 7
#define private_api_get_system_os_name (int) 9
#define private_api_get_system_model_name (int) 8
#define fixed_os_name (const xmlChar *)"Linux"

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

static xmlChar *fake_string;


/********************************************************************
* FUNCTION get_api_string
*
*
* This function is a router to return different field of different APi
* following the diffent api_indicator
*
* INPUTS:
*    api_indicator == indicator which field of which api to return
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_api_string (int api_indicator, ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    xmlChar      *buff;
    (void)scb;
    (void)virval;
    struct networkpb_Config *network_out;
    struct systempb_Config *sys_out;
    struct timepb_Config *time_out;
    struct systempb_Status *sys_status_out;
    struct timepb_Status *time_status_out;
    struct devicepb_Info *dev_info_out;

    printf("\n@@@@@@@@@@@@@@@@@@@@@@ get_api_string %d\n", api_indicator);
    if (cbmode == GETCB_GET_VALUE) {
        buff = (xmlChar *)m__getMem(TSTAMP_MIN_SIZE);
        if (!buff) {
            return ERR_INTERNAL_MEM;
        }
        struct emptypb_Empty *epty = malloc(sizeof(*(epty)));

        switch (api_indicator) {
        case private_api_get_network_hostname:
            network_out = malloc(sizeof(*(network_out)));
            network_Network_GetConfig(epty, network_out);
            sprintf((char *)buff, network_out->Basic->HostName);
            free(network_out);
            break;
        case private_api_get_system_contact:
        case private_api_get_system_location:
            sys_out = malloc(sizeof(*(sys_out)));
            system_System_GetConfig(epty, sys_out);
            switch (api_indicator) {
                case private_api_get_system_contact:
                    sprintf((char *)buff, sys_out->SysContact);
                    break;
                case private_api_get_system_location:
                    sprintf((char *)buff, sys_out->SysLocation);
                    break;
            }
            free(sys_out);
            break;
        case private_api_get_ntp_timezone:
            time_out = malloc(sizeof(*(time_out)));
            time_Time_GetConfig(epty, time_out);
            sprintf((char *)buff, time_out->TimeZone);
            free(time_out);
            break;
        case private_api_get_system_boot_datetime:
            sys_status_out = malloc(sizeof(*(sys_status_out)));
            system_System_GetStatus(epty, sys_status_out);
            sprintf((char *)buff, sys_status_out->LastBootTime);
            free(sys_status_out);
            break;
        case private_api_get_system_current_datetime:
            time_status_out = malloc(sizeof(*(time_status_out)));
            time_Time_GetStatus(epty, time_status_out);
            sprintf((char *)buff, time_status_out->LocalDate);
            sprintf((char *)buff, " ");
            sprintf((char *)buff, time_status_out->LocalTime);
            free(time_status_out);
            break;
        case private_api_get_system_os_name:
            sprintf((char *)buff, fixed_os_name);
            break;
        case private_api_get_system_software_version:
        case private_api_get_system_hardware_version:
        case private_api_get_system_model_name:
            dev_info_out = malloc(sizeof(*(dev_info_out)));
            device_Device_GetDeviceInfo(epty, dev_info_out);
            switch (api_indicator) {
                case private_api_get_system_software_version:
                    sprintf((char *)buff, dev_info_out->CurrentSwVersion);
                    break;
                case private_api_get_system_hardware_version:
                    sprintf((char *)buff, dev_info_out->HwVersion);
                    break;
                case private_api_get_system_model_name:
                    sprintf((char *)buff, dev_info_out->Model);
                    break;
            }
            free(dev_info_out);
            break;
        }
        free(epty);
        VAL_STRING(dstval) = buff;
        return NO_ERR;

    } else {
        return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* get_api_string */


static status_t
    get_system_hostname (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_network_hostname, scb, cbmode, virval, dstval);
}

static status_t
    get_system_contact (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_system_contact, scb, cbmode, virval, dstval);
}

static status_t
    get_system_location (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_system_location, scb, cbmode, virval, dstval);
}

static status_t
    get_clock_timezone (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_ntp_timezone, scb, cbmode, virval, dstval);
}

static status_t
    get_clock_boot_datetime (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_system_boot_datetime, scb, cbmode, virval, dstval);
}

static status_t
    get_clock_current_datetime (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_system_current_datetime, scb, cbmode, virval, dstval);
}

static status_t
    get_platform_os_name (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_system_os_name, scb, cbmode, virval, dstval);
}


static status_t
    get_platform_os_release (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_system_software_version, scb, cbmode, virval, dstval);
}

static status_t
    get_platform_os_version (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_system_software_version, scb, cbmode, virval, dstval);
}


static status_t
    get_platform_machine (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    return get_api_string(private_api_get_system_hardware_version, scb, cbmode, virval, dstval);
}



/********************************************************************
* FUNCTION get_fake_bool
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
    get_ntp_enabled (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    boolean      *buff;

    (void)scb;
    (void)virval;

    if (cbmode == GETCB_GET_VALUE) {
        struct timepb_Config *time_out;
        struct emptypb_Empty *epty = malloc(sizeof(*(epty)));
        time_out = malloc(sizeof(*(time_out)));

        time_Time_GetConfig(epty, time_out);
        buff = time_out->Mode==timepb_ModeTypeOptions_MODE_TYPE_AUTO;
        free(time_out);
        free(epty);
        VAL_BOOL(dstval) = buff;
        return NO_ERR;
    } else {
        return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* get_ntp_enabled */


static status_t
    add_ntp_entry(char* name, char* addr, val_value_t* ntp_val)
{
    /*objs*/
    obj_template_t* server_obj;
    obj_template_t* name_obj;
    obj_template_t* addr_obj;
    obj_template_t* udp_obj;
    obj_template_t* prefer_obj;

    /*vals*/
    val_value_t* server_val;
    val_value_t* name_val;
    val_value_t* addr_val;
    val_value_t* udp_val;
    val_value_t* prefer_val;

    status_t res=NO_ERR;
    int ret;

    /* ntp */
    server_obj = obj_find_child(ntp_val->obj,
                                   "ietf-system",
                                   "server");
    assert(server_obj != NULL);

    server_val = val_new_value();
    if (server_val == NULL) {
        return ERR_INTERNAL_MEM;
    }

    val_init_from_template(server_val, server_obj);

    val_add_child(server_val, ntp_val);

    /* ntp/name */
    name_obj = obj_find_child(server_obj,
                              ietf_system,
                              "name");
    assert(name_obj != NULL);


    name_val = val_new_value();
    if (name_val == NULL) {
                return ERR_INTERNAL_MEM;
    }

    val_init_from_template(name_val, name_obj);
    res = val_set_simval_obj(name_val, name_obj, name);

    val_add_child(name_val, server_val);

    res = val_gen_index_chain(server_obj, server_val);
    assert(res == NO_ERR);


    udp_obj = obj_find_child(server_obj,
                            ietf_system,
                            "udp");
    assert(udp_obj!=NULL);
    /* ntp/udp/address */
    udp_val = val_find_child(server_val,
                            ietf_system,
                            "udp");
    if (udp_val == NULL) {
        udp_val= val_new_value();
        assert(udp_val != NULL);

        val_init_from_template(udp_val,
                               udp_obj);

        val_add_child(udp_val, server_val);
    }

    addr_obj = obj_find_child(udp_obj,
                         ietf_system,
                         "address");
    assert(addr_obj!= NULL);

    addr_val = val_new_value();
    assert(addr_val);
    val_set_simval_obj(addr_val, addr_obj, addr);

    val_add_child(addr_val, udp_val);

    return res;
}

static status_t
    get_ntp(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res;
    res = NO_ERR;
    char* name_buf;
    char* address_buf;

    struct timepb_Config *time_out;
    struct emptypb_Empty *epty = malloc(sizeof(*(epty)));
    time_out = malloc(sizeof(*(time_out)));

    time_Time_GetConfig(epty, time_out);
    if(time_out->Mode!=timepb_ModeTypeOptions_MODE_TYPE_AUTO) {
        free(epty);
        free(time_out);
        return res;
    }
    name_buf = (char*)malloc(512);
    if (name_buf == NULL) {
        return ERR_INTERNAL_MEM;
    }
    address_buf = (char*)malloc(512);
    if (address_buf == NULL) {
        return ERR_INTERNAL_MEM;
    }

    sprintf((char *)name_buf, "primary");
    sprintf((char *)address_buf, time_out->MainNTPServer);
    printf("\n@@@ adding primary ntp %s\n", time_out->MainNTPServer);
    res = add_ntp_entry(name_buf, address_buf, dst_val);

    if(time_out->BackupNTPServer==NULL || strlen(time_out->BackupNTPServer)==0) {
        free(name_buf);
        free(address_buf);
        free(time_out);
        free(epty);
        printf("###### res %d\n", res);
        return res;
    }
    sprintf((char *)name_buf, "secondary");
    // printf("@@@@@@@ %s\n", time_out->BackupNTPServer);
    // printf("@@@@@@@ %s\n", time_out->BackupNTPServer==NULL ? "true" : "false");
    // printf("@@@@@@@ size %d\n", strlen(time_out->BackupNTPServer));
    sprintf((char *)address_buf, time_out->BackupNTPServer);

    printf("\n@@@ adding secondary ntp %s\n", time_out->BackupNTPServer);
    res = add_ntp_entry(name_buf, address_buf, dst_val);

    free(name_buf);
    free(address_buf);
    free(time_out);
    free(epty);
    printf("###### res %d\n", res);
    return res;
}


static status_t
    add_user_entry(char* name, val_value_t* user_val)
{
    /*objs*/
    obj_template_t* auth_obj;
    obj_template_t* name_obj;

    /*vals*/
    val_value_t* auth_val;
    val_value_t* name_val;

    status_t res=NO_ERR;
    int ret;

    /* authentication */
    auth_obj = obj_find_child(user_val->obj,
                                   ietf_system,
                                   ietf_system_authentication);
    assert(auth_obj != NULL);

    auth_val = val_new_value();
    if (auth_val == NULL) {
        return ERR_INTERNAL_MEM;
    }

    val_init_from_template(auth_val, auth_obj);

    val_add_child(auth_val, user_val);

    /* authentication/user */
    name_obj = obj_find_child(auth_obj,
                              ietf_system,
                              "user");
    assert(name_obj != NULL);


    name_val = val_new_value();
    if (name_val == NULL) {
                return ERR_INTERNAL_MEM;
    }

    val_init_from_template(name_val, name_obj);
    res = val_set_simval_obj(name_val, name_obj, name);

    val_add_child(name_val, auth_val);

    return res;
}

static status_t
    get_user(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res;
    res = NO_ERR;
    char* name_buf;
    char* address_buf;
    unsigned int i;
    boolean done;

    done = FALSE;

    struct accesspb_UsersConfig *access_user_out;
    struct emptypb_Empty *epty = malloc(sizeof(*(epty)));
    access_user_out = malloc(sizeof(*(access_user_out)));

    access_Access_GetUsers(epty, access_user_out);
    /* get a file read line buffer */
    name_buf = (char*)malloc(512);
    if (name_buf == NULL) {
        return ERR_INTERNAL_MEM;
    }
    for(i=0;i<sizeof(access_user_out->List)-1;i++) {
        if (done) {
            break;
        }
        printf("\n@@@ adding primary ntp\n");
        sprintf((char *)name_buf, access_user_out->List[i]->Name);
        res = add_user_entry(name_buf, dst_val);
        if (res!=NO_ERR) {
            done = TRUE;
        }
    }
    free(epty);
    free(access_user_out);
    free(name_buf);
    printf("###### res %d\n", res);
    return res;
}





/********************************************************************
* FUNCTION get_fake_list
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
    get_fake_list (ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         const val_value_t *virval,
                         val_value_t  *dstval)
{
    boolean      *buff;

    (void)scb;
    (void)virval;


    if (cbmode == GETCB_GET_VALUE) {
        buff = FALSE;
        VAL_BOOL(dstval) = buff;
        return NO_ERR;
    } else {
        return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

} /* get_fake_list */



// set_no_delete_func is used disable the delete on the assigned path
// so this function will return res when editop is delete
static status_t
    set_no_delete_func (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        agt_cbtyp_t cbtyp,
        op_editop_t editop,
        val_value_t *newval,
        val_value_t *curval)
{
    status_t res;
    val_value_t *errorval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    switch (cbtyp) {
    case AGT_CB_VALIDATE:
        /* description-stmt validation here */
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        switch (editop) {
        case OP_EDITOP_LOAD:
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
        case OP_EDITOP_CREATE:
        case OP_EDITOP_DELETE:
            return res;
        default:
            assert(0);
        }

        break;
    case AGT_CB_ROLLBACK:
        /* undo device instrumentation here */
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR) {
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

    return res;
}

static status_t
    set_fake_string (
        ses_cb_t *scb,
        rpc_msg_t *msg,
        agt_cbtyp_t cbtyp,
        op_editop_t editop,
        val_value_t *newval,
        val_value_t *curval)
{
    status_t res;
    val_value_t *errorval;
    const xmlChar *errorstr;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    printf("Setting /system/hostname to %s - cbtype=%d\n", VAL_STRING(newval), cbtyp);
    printf("Setting /system/hostname to %s - editop=%d\n", VAL_STRING(newval), editop);
    switch (cbtyp) {
    case AGT_CB_VALIDATE:
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        switch (editop) {
        case OP_EDITOP_LOAD:
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
        case OP_EDITOP_CREATE:
            if(newval!=NULL) {
                printf("Setting /system/hostname, newval %s\n", VAL_STRING(newval));
                xmlChar* buf;
                buf=malloc(strlen(VAL_STRING(newval)));
                sprintf(buf, VAL_STRING(newval));
                fake_string = buf;
            }
            break;
        case OP_EDITOP_DELETE:
            return res;
        default:
            assert(0);
        }

    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    /* if error: set the res, errorstr, and errorval parms */
    if (res != NO_ERR) {
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

    return res;
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
        VAL_STRING(dstval) = buff;
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
* FUNCTION add_child_val_under_parent
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
status_t
    add_child_val_under_parent(
        val_value_t *parent_val,
        const xmlChar *leafname,
        getcb_fn_t cbfn)
{
    status_t res = NO_ERR;
    val_value_t *child_val = NULL;
    child_val = agt_make_virtual_leaf(
        parent_val->obj,
        leafname,
        cbfn,
        &res);
    if (child_val != NULL) {
        val_add_child_sorted(child_val, parent_val);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }
    return res;
}


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
    printf("\n@@@@@ agt_sys_init @@@\n");
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
    printf("start register callback\n");
    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/hostname",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        set_fake_string);
    printf("register callback end\n");

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


status_t ietf_system_state_mro(val_value_t *parent_val) {
    status_t res = NO_ERR;
    val_value_t *clock_val = NULL;
    val_value_t *platform_val = NULL;

    /* Add /system-state/clock */
    res = agt_add_container(
        ietf_system,
        ietf_system_clock,
        parent_val,
        &clock_val);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system-state/clock/current-datetime */
    res = add_child_val_under_parent(clock_val, ietf_system_state_current_datetime, get_clock_current_datetime);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system-state/clock/boot-datetime */
    res = add_child_val_under_parent(clock_val, ietf_system_state_boot_datetime, get_clock_boot_datetime);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system-state/platform */
    res = agt_add_container(
        ietf_system,
        ietf_system_state_platform,
        parent_val,
        &platform_val);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system-state/platform/os-name */
    res = add_child_val_under_parent(platform_val, ietf_system_state_os_name, get_platform_os_name);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system-state/platform/os-release */
    res = add_child_val_under_parent(platform_val, ietf_system_state_os_release, get_platform_os_release);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system-state/platform/os-version */
    res = add_child_val_under_parent(platform_val, ietf_system_state_os_version, get_platform_os_version);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system-state/platform/machine */
    res = add_child_val_under_parent(platform_val, ietf_system_state_machine, get_platform_machine);

    return res;
}

status_t build_ntp_server (val_value_t *parent_val, char *name, char *address) {
    status_t res = NO_ERR;
    val_value_t *child_val = NULL;
    val_value_t *udp_val = NULL;

    child_val = agt_make_leaf(
        parent_val->obj,
        ietf_system_ntp_server_name,
        name,
        &res);
    if (child_val != NULL) {
        val_add_child(child_val, parent_val);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_add_container(
        ietf_system,
        ietf_system_ntp_server_udp,
        parent_val,
        &udp_val);

    child_val = agt_make_leaf(
        udp_val->obj,
        ietf_system_ntp_server_udp_address,
        address,
        &res);

    if (child_val != NULL) {
        val_add_child(child_val, udp_val);
    } else if (res != NO_ERR) {
        return SET_ERROR(res);
    }
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
ietf_system_ntp_server_get (
    ses_cb_t *scb,
    getcb_mode_t cbmode,
    const val_value_t *virval,
    val_value_t *dstval) {

    status_t res = NO_ERR;
    if (LOGDEBUG) {
        log_debug("\nEnter ietf_system_ntp_server_get");
    }
    /* calls the private api */
    struct emptypb_Empty in;
    struct timepb_Config out;
    time_Time_GetConfig(&in, &out);

    if(out.Mode==timepb_ModeTypeOptions_MODE_TYPE_AUTO) {
        val_value_t *primary_val = NULL;
        val_value_t *seconday_val = NULL;
        val_value_t *ntp_val = NULL;
         /* Add /system/ntp */
        res = agt_add_container(
            ietf_system,
            ietf_system_ntp,
            dstval,
            &ntp_val);

        /* Add /system/ntp/enabled */
        res = add_child_val_under_parent(ntp_val, ietf_system_ntp_enabled, get_ntp_enabled);
        if (res != NO_ERR) {
            return res;
        }

        /* Add /system/ntp/server */
        primary_val = agt_make_list(
            ntp_val->obj,
            ietf_system_ntp_server,
            &res);
        if (primary_val != NULL) {
            val_add_child(primary_val, dstval);
        } else if (res != NO_ERR) {
            return SET_ERROR(res);
        }
        res = build_ntp_server(primary_val, "primary", out.MainNTPServer);
        if (res != NO_ERR) {
            return SET_ERROR(res);
        }

        // support secondary later
        // seconday_val = agt_make_list(
        //     ntp_val->obj,
        //     ietf_system_ntp_server,
        //     &res);
        // if (seconday_val != NULL) {
        //     val_add_child(seconday_val, dstval);
        // } else if (res != NO_ERR) {
        //     return SET_ERROR(res);
        // }

        // res = build_ntp_server(seconday_val, "secondary", out.BackupNTPServer);
        // if (res != NO_ERR) {
        //     return SET_ERROR(res);
        // }
        // return res;
    }
    return res;


}

status_t ietf_system_ntp_server_mro(val_value_t *parent_val) {
    status_t res = NO_ERR;

    /* init /system/ntp/server */
    val_init_virtual(
        parent_val,
        ietf_system_ntp_server_get,
        parent_val->obj);
    return res;
}

status_t ietf_system_mro(val_value_t *parent_val) {
    status_t res = NO_ERR;
    val_value_t *authentication_val = NULL;
    val_value_t *clock_val = NULL;
    val_value_t *ntp_val = NULL;
    val_value_t *server_val = NULL;

    /* Add /system/hostname */
    res = add_child_val_under_parent(parent_val, ietf_system_hostname, get_system_hostname);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system/location */
    res = add_child_val_under_parent(parent_val, ietf_system_location, get_system_location);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system/contact */
    res = add_child_val_under_parent(parent_val, ietf_system_contact, get_system_contact);
    if (res != NO_ERR) {
        return res;
    }

    /* Add /system/clock */
    res = agt_add_container(
        ietf_system,
        ietf_system_clock,
        parent_val,
        &clock_val);

    /* Add /system/clock/timezone-name */
    res = add_child_val_under_parent(clock_val, ietf_system_clock_timezone_name, get_clock_timezone);
    if (res != NO_ERR) {
        return res;
    }

    // If ntp is enabled, add the ntp container, else rm it
    res = ietf_system_ntp_server_mro(parent_val);
    if (res != NO_ERR) {
        return res;
    }



    // /* Add /system/radius */
    res = agt_add_container(
        ietf_system,
        ietf_system_authentication,
        parent_val,
        &authentication_val);

    /* Add /system/radius/server/name */
    // res = add_child_val_under_parent(authentication_val, ietf_system_ntp_enabled, get_ntp_enabled);
    // if (res != NO_ERR) {
    //     return res;
    // }
    // /* Add /system/authentication */
    // res = agt_add_container(
    //     ietf_system,
    //     ietf_system_authentication,
    //     parent_val,
    //     &authentication_val);

    /* Add /system/authentication/user/name */
    // res = add_child_val_under_parent(authentication_val, ietf_system_ntp_enabled, get_ntp_enabled);
    // if (res != NO_ERR) {
    //     return res;
    // }

    return res;
}


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
    val_value_t           *ietf_system_val, *ietf_system_state_val;
    cfg_template_t        *runningcfg;
    status_t               res;

    printf("\n@@@@@ agt_sys_init2\n");
    if (!agt_sys_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    ietf_system_val = agt_init_cache(
        ietf_system,
        ietf_system_N_system,
        &res);

    if (res != NO_ERR) {
        return res;
    }

    /* Add /system */
    res = agt_add_top_container(ietf_system_obj, &ietf_system_val);
    if (res != NO_ERR) {
        return res;
    }
    res = ietf_system_mro(ietf_system_val);


    printf("\n @@@@@@ what b4 add system-state\n");
    /* Add /system-state */
    res = agt_add_top_container(ietf_system_state_obj, &ietf_system_state_val);
    if (res != NO_ERR) {
        return res;
    }
    res = ietf_system_state_mro(ietf_system_state_val);
    printf("\n @@@@@@ done adding system-state\n");

    /* add sysStartup to notificationQ */
    printf("\n@@@ send sysStartup @@@\n");
    send_sysStartup();

    return NO_ERR;

}  /* agt_sys_init2 */



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
