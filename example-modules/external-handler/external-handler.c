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
#include "agt_commit_complete.h"
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


static val_value_t* prev_root_config_val=NULL;
static unsigned int transaction_id=0;
static char* commit_prog;
static char* get_prog;
static char* get_cb_schema_map;

void string_to_file(char* str, char* filename)
{
    FILE* fp;
    int res;
    fp = fopen(filename, "w");
    assert(fp!=NULL);
    res=fprintf(fp,str);
    assert(res==strlen(str));
    fclose(fp);
}


#define EMPTY_CONFIG "<config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\"/>\n"
void my_transaction_handler(unsigned int transaction_id, val_value_t* prev_root_config_val, val_value_t* cur_root_config_val)
{
    status_t res;
    xmlChar* before;
    xmlChar* after;
    char* cmd_buf;

    size_t needed;
    if(prev_root_config_val!=NULL) {
        res=val_make_serialized_string(prev_root_config_val, NCX_DISPLAY_MODE_XML, (xmlChar **)&before);
        assert(res=NO_ERR);
    } else {
        before=malloc(strlen(EMPTY_CONFIG)+1);
        strcpy(before,EMPTY_CONFIG);
    }
    res=val_make_serialized_string(cur_root_config_val, NCX_DISPLAY_MODE_XML, (xmlChar **)&after);
    assert(res==NO_ERR);
    string_to_file(before,"/tmp/before.xml");
    string_to_file(after,"/tmp/after.xml");
    needed=snprintf(NULL,0,"%s --before=/tmp/before.xml --after=/tmp/after.xml", commit_prog);
    cmd_buf = malloc(needed+1);
    snprintf(cmd_buf, needed+1, "%s --before=/tmp/before.xml --after=/tmp/after.xml", commit_prog);
    res=system(cmd_buf);
    assert(res==0);
    free(before);
    free(after);
}

status_t external_handler_commit_complete_cb(void)
{
    cfg_template_t* runningcfg;
    status_t res;
    val_value_t*    cur_root_config_val;

    printf("in external_handler_commit_complete_cb\n");
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

    my_transaction_handler(transaction_id, prev_root_config_val, cur_root_config_val);

    if(prev_root_config_val!=NULL) {
        val_free_value(prev_root_config_val);
    }
    prev_root_config_val = cur_root_config_val;
    transaction_id++;
    return NO_ERR;
}

/* The 3 mandatory callback functions: y_external_handler_init, y_external_handler_init2, y_external_handler_cleanup */

status_t
    y_external_handler_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    status_t res;
    res=agt_commit_complete_register("external-handler",
                                      external_handler_commit_complete_cb);
    assert(res == NO_ERR);


    commit_prog=getenv("COMMIT_PROG");
    assert(commit_prog);
    get_prog=getenv("GET_PROG");
    assert(get_prog);
    get_cb_schema_map=getenv("GET_CB_SCHEMA_MAP");
    assert(get_cb_schema_map);

    return NO_ERR;
}

status_t y_external_handler_init2(void)
{
    //TODO: loop through get callback registrations and add virtual nodes in the config false only parent ancestors.
    return NO_ERR;
}

void y_external_handler_cleanup(void)
{
}
