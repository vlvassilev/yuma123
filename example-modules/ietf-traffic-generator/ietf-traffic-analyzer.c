/*
    module ietf-traffic-analyzer
    implementation for Linux
    namespace urn:ietf:params:xml:ns:yang:ietf-traffic-analyzer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "procdefs.h"
#include "agt.h"
#include "agt_cb.h"
#include "agt_commit_complete.h"
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
#include "val_set_cplxval_obj.h"

#include "dict.h"

#define IF_MOD "ietf-interfaces"
#define TA_MOD "ietf-traffic-analyzer"

/* module static variables */
static ncx_module_t *ietf_interfaces_mod;
static ncx_module_t *iana_if_type_mod;
static ncx_module_t *ietf_traffic_analyzer_mod;

static dlq_hdr_t io_dict;
typedef struct io_t_ {
    FILE* in;
    FILE* out;
} io_t;

static status_t
    get_if_traffic_analyzer_state(ses_cb_t *scb,
                         getcb_mode_t cbmode,
                         val_value_t *vir_val,
                         val_value_t *dst_val)
{
    status_t res;
    obj_template_t* obj;
    val_value_t* val;

#if 1
    val_value_t* name_val;
    char* state_xml; // = "<state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-traffic-analyzer\"><pkts>34</pkts></state>";
    ssize_t n;
    io_t* io;

    name_val = val_find_child(dst_val->parent->parent,
                              "ietf-interfaces",
                              "name");
    assert(name_val);

    io = dict_get_data(&io_dict, VAL_STRING(name_val));
    assert(io);

    fputs("\n",io->out);
    fflush(io->out);

    n=0;
    state_xml=NULL;
    n = getline(&state_xml, &n, io->in);
    assert(n > 0 && state_xml != NULL);

    res = val_set_cplxval_obj(dst_val,dst_val->obj,state_xml);
    if(res != NO_ERR) {
        return res;
    }
    free(state_xml);

#else

    uint64_t pkts = 33;

    /* pkts */
    obj=obj_find_child(vir_val->obj, TA_MOD, "pkts");
    val=val_new_value();
    assert(val);
    val_init_from_template(val, obj);
    VAL_UINT64(val) = pkts;
    val_add_child(val, dst_val);

#endif

    return NO_ERR;
}


static void serialize_params(val_value_t* traffic_analyzer_val, char* cli_args_str)
{
    val_value_t* val;
    unsigned int i;

    val = val_find_child(traffic_analyzer_val->parent,"ietf-interfaces","name");
    sprintf(cli_args_str,"--interface-name=%s",VAL_STRING(val));

    //val = val_find_child(traffic_analyzer_val,"ietf-traffic-analyzer","frame-size");
    //sprintf(cli_args_str+strlen(cli_args_str)," --frame-size=%u",VAL_UINT32(val));

}

static void traffic_analyzer_delete(val_value_t* traffic_analyzer_val)
{
    char cmd_buf[4096];
    static char cmd_args_buf[4096];
    val_value_t* name_val;
    io_t* io;
    char* name_buf;

    printf("traffic_analyzer_io_dictdelete:\n");
    val_dump_value(traffic_analyzer_val,NCX_DEF_INDENT);
    name_val = val_find_child(traffic_analyzer_val->parent,"ietf-interfaces","name");
    assert(name_val);

    serialize_params(traffic_analyzer_val, cmd_args_buf);
    sprintf(cmd_buf, "pkill -f '/bin/sh -c traffic-analyzer %s'", cmd_args_buf);
    system(cmd_buf);

    io = dict_get_data(&io_dict, VAL_STRING(name_val));
    assert(io);

    fclose(io->in);
    fclose(io->out);

    name_buf = dict_get_name(&io_dict, io);
    dict_remove(&io_dict, VAL_STRING(name_val));
    free(name_buf);
    free(io);
}

static void traffic_analyzer_create(val_value_t* traffic_analyzer_val)
{
    char cmd_buf[4096];
    static char cmd_args_buf[4096];
    val_value_t* name_val;

    printf("traffic_analyzer_create:\n");
    val_dump_value(traffic_analyzer_val,NCX_DEF_INDENT);
    name_val = val_find_child(traffic_analyzer_val->parent,"ietf-interfaces","name");
    assert(name_val);

    serialize_params(traffic_analyzer_val, cmd_args_buf);
    sprintf(cmd_buf, "traffic-analyzer %s", cmd_args_buf);

    //system(cmd_buf);

    {
        int     fd_in[2];
        int     fd_out[2];
        pid_t   childpid;

        pipe(fd_in);
        pipe(fd_out);

        childpid = fork();

        if(childpid == -1) {
            perror("fork");
            assert(0);
        } else if(childpid == 0) {
            dup2(fd_out[0], 0);
            close(fd_out[0]);
            close(fd_out[1]);
            dup2(fd_in[1], 1);
            close(fd_in[0]);
            close(fd_in[1]);
            execl("/bin/sh", "sh", "-c", cmd_buf, (char *) 0);
        } else {
            int nbytes;
            char readbuffer[1024];
            char* lineptr;
            ssize_t n;
            ssize_t ret;
            io_t* io;

            io = malloc(sizeof(io_t));
            assert(io);

            close(fd_out[0]);
            close(fd_in[1]);

            io->out = fdopen(fd_out[1], "w");
            assert(io->out != NULL);
            io->in = fdopen(fd_in[0], "r");
            assert(io->in != NULL);

            dict_add(&io_dict, strdup(VAL_STRING(name_val)), (void *)io);

#if 1
            fputs("\n",io->out);
            fflush(io->out);

            n=0;
            lineptr=NULL;
            n = getline(&lineptr, &n, io->in);
            assert(n > 0 && lineptr != NULL);

            printf("Received string: %s", lineptr);
            free(lineptr);
#endif
        }
    }
}

static int update_config(val_value_t* config_cur_val, val_value_t* config_new_val)
{

    status_t res;

    val_value_t *interfaces_cur_val, *interface_cur_val , *traffic_analyzer_cur_val;
    val_value_t *interfaces_new_val, *interface_new_val , *traffic_analyzer_new_val;


    if(config_new_val == NULL) {
        interfaces_new_val = NULL;
    } else {
        interfaces_new_val = val_find_child(config_new_val,
                               IF_MOD,
                               "interfaces");
    }

    if(config_cur_val == NULL) {
        interfaces_cur_val = NULL;
    } else {
        interfaces_cur_val = val_find_child(config_cur_val,
                                       IF_MOD,
                                       "interfaces");
    }

    /* 2 step (delete/add) interface configuration */

    /* 1. deactivation loop - deletes all deleted or modified interface/traffic-analyzer -s */
    if(interfaces_cur_val!=NULL) {
        for (interface_cur_val = val_get_first_child(interfaces_cur_val);
             interface_cur_val != NULL;
             interface_cur_val = val_get_next_child(interface_cur_val)) {
            traffic_analyzer_cur_val = val_find_child(interface_cur_val, TA_MOD, "traffic-analyzer");
            if(traffic_analyzer_cur_val==NULL) {
                continue;
            }
            traffic_analyzer_new_val = val123_find_match(config_new_val, traffic_analyzer_cur_val);
            if(traffic_analyzer_new_val==NULL || 0!=val_compare_ex(traffic_analyzer_cur_val,traffic_analyzer_new_val,TRUE)) {
                traffic_analyzer_delete(traffic_analyzer_cur_val);
            }
        }
    }

    /* 2. activation loop - adds all new or modified interface/traffic-analyzer -s */
    if(interfaces_new_val!=NULL) {
        for (interface_new_val = val_get_first_child(interfaces_new_val);
             interface_new_val != NULL;
             interface_new_val = val_get_next_child(interface_new_val)) {
            traffic_analyzer_new_val = val_find_child(interface_new_val, TA_MOD, "traffic-analyzer");
            if(traffic_analyzer_new_val==NULL) {
                continue;
            }
            traffic_analyzer_cur_val = val123_find_match(config_cur_val, traffic_analyzer_new_val);
            if(traffic_analyzer_cur_val==NULL || 0!=val_compare_ex(traffic_analyzer_new_val,traffic_analyzer_cur_val,TRUE)) {
                traffic_analyzer_create(traffic_analyzer_new_val);
            }
            if(traffic_analyzer_new_val!=NULL) {
                val_value_t * state_val;
                state_val = val_find_child(traffic_analyzer_new_val,TA_MOD,"state");
                if(state_val==NULL) {
                    /* add state container */
                    obj_template_t* state_obj;
                    state_obj = obj_find_child(traffic_analyzer_new_val->obj,TA_MOD,"state");
                    assert(state_obj);
                    state_val = val_new_value();
                    assert(state_val!=NULL);

                    val_init_virtual(state_val,
                                     get_if_traffic_analyzer_state,
                                     state_obj);
                    val_add_child(state_val, traffic_analyzer_new_val);
                }
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

/* The 3 mandatory callback functions: y_ietf_traffic_analyzer_init, y_ietf_traffic_analyzer_init2, y_ietf_traffic_analyzer_cleanup */

status_t
    y_ietf_traffic_analyzer_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t* agt_profile;
    obj_template_t* flows_obj;
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
    res = ncxmod_load_module(
        "iana-if-type",
        NULL,
        &agt_profile->agt_savedevQ,
        &iana_if_type_mod);
    if (res != NO_ERR) {
        return res;
    }
    res = ncxmod_load_module(
        "ietf-traffic-analyzer",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_traffic_analyzer_mod);
    if (res != NO_ERR) {
        return res;
    }

    agt_disable_feature ("ietf-traffic-analyzer", "multi-stream");

    res=agt_commit_complete_register("ietf-traffic-analyzer" /*SIL id string*/,
                                     y_commit_complete);
    assert(res == NO_ERR);

    dict_init(&io_dict);

    return res;
}

status_t y_ietf_traffic_analyzer_init2(void)
{
    status_t res=NO_ERR;
    cfg_template_t* runningcfg;
    ncx_module_t* mod;
    obj_template_t* interfaces_obj;
    val_value_t* root_val;
    val_value_t* interfaces_val;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    assert(runningcfg && runningcfg->root);
    root_val = runningcfg->root;

    y_commit_complete();

    return res;
}

void y_ietf_traffic_analyzer_cleanup (void)
{
}

