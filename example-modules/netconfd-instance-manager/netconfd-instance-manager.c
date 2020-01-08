/*
    module netconfd-instance-manager
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
#include <sys/types.h>
#include <sys/wait.h>


#include <libxml/xmlstring.h>
#include "procdefs.h"
#include "agt.h"
#include "agt_commit_complete.h"
#include "agt_cb.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "agt_not.h"
#include "agt_rpc.h"
#include "dlq.h"
#include "ncx.h"
#include "val.h"
#include "val123.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "status.h"

#define SERVICES_MOD "yuma123-services"
#define SERVICES_NETCONFD_MOD "yuma123-services-netconfd"

void sshd_for_netconfd_start(unsigned int port)
{
    FILE* f;
    int ret;
    char filename[]="/var/run/yuma/sshd-netconf-config-65536";
    char cmd_buf[]="/usr/sbin/sshd -f /var/run/yuma/sshd-netconf-config-65536 &";

    sprintf(filename, "/var/run/yuma/sshd-netconf-config-%u",port);
    f=fopen(filename,"w");
    assert(f!=NULL);
    fprintf(f,"PidFile /var/run/yuma/sshd-netconf-%u.pid\n"
              "ChallengeResponseAuthentication no\n"
              "UsePAM yes\n"
              "AcceptEnv LANG LC_*\n"
              "PermitRootLogin yes\n"
              "Port %u\n"
              "Subsystem netconf \"/usr/sbin/netconf-subsystem --ncxserver-sockname=%u@/var/run/yuma/ncxserver-%u.sock\"\n"
              "#ForceCommand /usr/run/yuma/netconf-subsystem-%u\n"
              "#LogLevel DEBUG3\n"
              "#SyslogFacility\n"
              ,port,port,port,port,port);
    fclose(f);
    sprintf(cmd_buf, "/usr/sbin/sshd -f %s &",filename);
    ret=system(cmd_buf);
    assert(ret==0);
}

/*
   The returned value is the strlen of string.
   If cli_args_str is NULL the function only calculates the length.
 */
static unsigned int container_to_cli_args_str(val_value_t* container_val, char* cli_args_str, unsigned int* str_len)
{
    val_value_t* val;
    char* val_str;
    unsigned int len=0;
    for (val = val_get_first_child(container_val);
         val != NULL;
         val = val_get_next_child(val)) {

        val_str = val_make_sprintf_string(val);
        len += snprintf(cli_args_str?cli_args_str+len:NULL, cli_args_str?(*str_len-len):0, "--%s=%s ", obj_get_name(val->obj), val_str);
        free(val_str);
    }
    *str_len=len;
    return len;
}

static void setup_env_vars(val_value_t* service_val)
{
    val_value_t* environment_variables_val;
    val_value_t* val;
    int ret;
    environment_variables_val = val_find_child(service_val,SERVICES_MOD,"environment-variables");
    if(environment_variables_val==NULL) {
        return;
    }
    for (val = val_get_first_child(environment_variables_val);
         val != NULL;
         val = val_get_next_child(val)) {
        val_value_t* name_val;
        val_value_t* value_val;

        name_val = val_find_child(val,SERVICES_MOD,"name");
        assert(name_val!=NULL);
        value_val = val_find_child(val,SERVICES_MOD,"value");
        assert(value_val!=NULL);
        ret=setenv(VAL_STRING(name_val), VAL_STRING(value_val), TRUE /*overwrite*/);
        assert(ret==0);
    }
}

/* free the malloc-ed buffer when done using it */
char* generate_netconfd_cmd(val_value_t* service_val)
{
    char* buf;
    unsigned int header_len;
    unsigned int len;
    val_value_t* parameters_val;
    val_value_t* port_val;

    parameters_val = val_find_child(service_val,SERVICES_NETCONFD_MOD,"parameters");
    port_val = val_find_child(parameters_val,SERVICES_NETCONFD_MOD,"port");

    header_len = snprintf(NULL, 0, "/usr/sbin/netconfd --startup=/var/lib/yuma/startup-cfg-%u.xml --ncxserver-sockname=/var/run/yuma/ncxserver-%u.sock ", (unsigned int)VAL_UINT16(port_val), (unsigned int)VAL_UINT16(port_val));

    container_to_cli_args_str(parameters_val, NULL, &len);
    buf=malloc(header_len+len+1);
    header_len = snprintf(buf, header_len+1, "/usr/sbin/netconfd --startup=/var/lib/yuma/startup-cfg-%u.xml --ncxserver-sockname=/var/run/yuma/ncxserver-%u.sock ", (unsigned int)VAL_UINT16(port_val), (unsigned int)VAL_UINT16(port_val));
    container_to_cli_args_str(parameters_val, buf+header_len, &len);
    return buf;
}

void service_add(val_value_t* service_new_val)
{
    char cmd_buf[1024];
    char* buf;
    char* background_cmd_buf;
    val_value_t* parameters_val;
    val_value_t* port_val;
    int res;
    unsigned int len;

    parameters_val = val_find_child(service_new_val,SERVICES_NETCONFD_MOD,"parameters");
    port_val = val_find_child(parameters_val,SERVICES_NETCONFD_MOD,"port");

    sprintf(cmd_buf, "rm /var/run/yuma/ncxserver-%u.pid",(unsigned int)VAL_UINT16(port_val));
    system(cmd_buf);
    sprintf(cmd_buf, "rm /var/run/yuma/ncxserver-%u.sock",(unsigned int)VAL_UINT16(port_val));
    system(cmd_buf);
//    sprintf(cmd_buf, "rm /var/lib/yuma/startup-cfg-%u.xml", (unsigned int)VAL_UINT16(port_val));
//    system(cmd_buf);

    val_dump_value(service_new_val,NCX_DEF_INDENT);

    buf = generate_netconfd_cmd(service_new_val);

    len = snprintf(NULL, 0, "%s &", buf);
    background_cmd_buf=malloc(len+1);
    snprintf(background_cmd_buf, len+1, "%s &", buf);

    {
        int status;
        pid_t child_pid;

        if((child_pid = fork()) == 0) {

            /* child */
            setup_env_vars(service_new_val);
            res=system(background_cmd_buf);
            exit(res);
        } else {
            /* parent */
            while (child_pid != wait(&status)) {
                if(child_pid==-1) {
                    assert(0);
                }
            }
            assert(status==0);
        }
    }

    free(background_cmd_buf);
    free(buf);

    sshd_for_netconfd_start(VAL_UINT16(port_val));
}

void service_delete(val_value_t* service_cur_val)
{
    char* buf;
    char* pkill_cmd_buf;
    unsigned int len;
    int res;

    buf = generate_netconfd_cmd(service_cur_val);

    len = snprintf(NULL, 0, "pkill -x -f '%s'", buf);
    pkill_cmd_buf=malloc(len+1);
    snprintf(pkill_cmd_buf, len+1, "pkill -x -f '%s'", buf);

    res=system(pkill_cmd_buf);

    free(pkill_cmd_buf);
    free(buf);
}

static status_t
    get_service_state(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    int ret;
    status_t res;
    res = NO_ERR;
    int retval;

    obj_template_t* is_running_obj;
    val_value_t* is_running_val;
    unsigned int len;
    char* pgrep_cmd_buf;
    char* buf;

    res = NO_ERR;

    is_running_obj = obj_find_child(dst_val->obj,
                         "yuma123-services",
                         "is-running");
    assert(is_running_obj);

    is_running_val=val_new_value();
    assert(is_running_val);
    val_init_from_template(is_running_val, is_running_obj);

    buf = generate_netconfd_cmd(dst_val->parent);

    len = snprintf(NULL, 0, "pgrep -x -f '%s'", buf);
    pgrep_cmd_buf=malloc(len+1);
    snprintf(pgrep_cmd_buf, len+1, "pgrep -x -f '%s'", buf);

    retval=system(pgrep_cmd_buf);

    free(pgrep_cmd_buf);
    free(buf);

    VAL_BOOL(is_running_val)=(retval==0)?TRUE:FALSE;
    val_add_child(is_running_val, dst_val);

    return res;
}

static val_value_t* prev_root_config_val=NULL;

static int update_config(val_value_t* config_cur_val, val_value_t* config_new_val)
{
    status_t res;

    val_value_t *services_cur_val, *service_cur_val;
    val_value_t *services_new_val, *service_new_val;


    if(config_new_val == NULL) {
        services_new_val = NULL;
    } else {
        services_new_val = val_find_child(config_new_val,
                               SERVICES_MOD,
                               "services");
    }

    if(config_cur_val == NULL) {
        services_cur_val = NULL;
    } else {
        services_cur_val = val_find_child(config_cur_val,
                                       SERVICES_MOD,
                                       "services");
    }

    /* 2 step (delete/add) service configuration */

    /* 1. deactivation loop - deletes all deleted or modified services */
    if(services_cur_val!=NULL) {
        for (service_cur_val = val_get_first_child(services_cur_val);
             service_cur_val != NULL;
             service_cur_val = val_get_next_child(service_cur_val)) {
            service_new_val = val123_find_match(config_new_val, service_cur_val);
            if(service_new_val==NULL || 0!=val_compare_ex(service_cur_val,service_new_val,TRUE)) {
                service_delete(service_cur_val);
            }
        }
    }

    /* 2. activation loop - adds all new or modified services */
    if(services_new_val!=NULL) {
        for (service_new_val = val_get_first_child(services_new_val);
             service_new_val != NULL;
             service_new_val = val_get_next_child(service_new_val)) {

            service_cur_val = val123_find_match(config_cur_val, service_new_val);
            if(service_cur_val==NULL || 0!=val_compare_ex(service_new_val,service_cur_val,TRUE)) {
                service_add(service_new_val);

                /* register state */
                obj_template_t* service_state_obj;
                val_value_t* service_state_val;
                service_state_obj = obj_find_child(service_new_val->obj,
                                   "yuma123-services",
                                   "state");
                assert(service_state_obj);

                service_state_val = val_new_value();
                assert(service_state_val);

                val_init_virtual(service_state_val,
                     get_service_state,
                     service_state_obj);
                val_add_child(service_state_val, service_new_val);
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

status_t netconfd_instance_manager_commit_complete_cb(void)
{
    update_config_wrapper();
    return NO_ERR;
}

/* The 3 mandatory callback functions: y_netconfd_instance_manager_init, y_neconfd_instance_manager_init2, y_neconfd_instance_manager_cleanup */

status_t
    y_netconfd_instance_manager_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    status_t res;
    agt_profile_t* agt_profile;
    ncx_module_t * mod;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        SERVICES_MOD,
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    assert(res == NO_ERR);

    res = ncxmod_load_module(
        SERVICES_NETCONFD_MOD,
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    assert(res == NO_ERR);

    res=agt_commit_complete_register("external-handler",
                                     netconfd_instance_manager_commit_complete_cb);
    assert(res == NO_ERR);
    return NO_ERR;
}

status_t y_netconfd_instance_manager_init2(void)
{
    netconfd_instance_manager_commit_complete_cb();
    return NO_ERR;
}

void y_netconfd_instance_manager_cleanup(void)
{
}
