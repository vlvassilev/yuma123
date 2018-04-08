/*
    module test-rollback-on-error(based on ietf-system.yang)
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


#include <libxml/xmlstring.h>
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

/* module static variables */
static ncx_module_t *ietf_system_mod;

static status_t
    edit_cb (
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
    char* editop_str=NULL;
    char* cbtyp_str=NULL;
    char* newval_str=NULL;
    char* curval_str=NULL;

    res = NO_ERR;
    errorval = NULL;
    errorstr = NULL;

    switch (cbtyp) {
    case AGT_CB_VALIDATE:
        /* description-stmt validation here */
        cbtyp_str = "AGT_CB_VALIDATE";
        break;
    case AGT_CB_APPLY:
        /* database manipulation done here */
        cbtyp_str = "AGT_CB_APPLY";
        break;
    case AGT_CB_COMMIT:
        /* device instrumentation done here */
        cbtyp_str = "AGT_CB_COMMIT";
        break;
    case AGT_CB_ROLLBACK:
        /* undo device instrumentation here */
        cbtyp_str = "AGT_CB_ROLLBACK";
        break;
    default:
        cbtyp_str = "AGT_CB_UNKNOWN!";
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    switch (editop) {
    case OP_EDITOP_DELETE:
        editop_str = "OP_EDITOP_DELETE";
        break;
    case OP_EDITOP_LOAD:
        editop_str = "OP_EDITOP_LOAD";
        break;
    case OP_EDITOP_MERGE:
        editop_str = "OP_EDITOP_MERGE";
        break;
    case OP_EDITOP_REPLACE:
        editop_str = "OP_EDITOP_REPLACE";
        break;
    case OP_EDITOP_CREATE:
        editop_str = "OP_EDITOP_CREATE";
        break;
    }
    if(cbtyp==AGT_CB_COMMIT) {
        if(newval!=NULL && obj_has_text_content(newval->obj)) {
            int ret;
            ret = !memcmp("error",VAL_STRING(newval),(strlen("error")>strlen(VAL_STRING(newval)))?strlen(VAL_STRING(newval)):strlen("error"));
            if(ret != 0) {
                errorval=newval;
                errorstr="For the sake of testing all text values that start with \"error\" cause commit error!";
                res = SET_ERROR(ERR_INTERNAL_VAL);
            }
       }
    }

    if(newval!=NULL) {
        val_make_serialized_string (newval, NCX_DISPLAY_MODE_XML, &newval_str);
    }
    if(curval!=NULL) {
        val_make_serialized_string (curval, NCX_DISPLAY_MODE_XML, &curval_str);
    }

    printf("<rollback-on-error><edit_cb><cbtyp>%s</cbtyp><editop>%s</editop><newval>%s</newval><curval>%s</curval></edit_cb></rollback-on-error>\n", cbtyp_str, editop_str, newval?newval_str:"NULL", curval?curval_str:"NULL");
    if(newval_str!=NULL) {
        free(newval_str);
    }
    if(curval_str!=NULL) {
        free(curval_str);
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


/* The 3 mandatory callback functions: y_ietf_system_init, y_ietf_system_init2, y_ietf_system_cleanup */

status_t
    y_test_rollback_on_error_init (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    status_t res;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "ietf-system",
        NULL,
        &agt_profile->agt_savedevQ,
        &ietf_system_mod);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        edit_cb);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/hostname",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        edit_cb);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_cb_register_callback(
        "ietf-system",
        (const xmlChar *)"/system/location",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        edit_cb);
    if (res != NO_ERR) {
        return res;
    }

    return res;
}

status_t y_test_rollback_on_error_init2(void)
{
    return NO_ERR;
}

void y_test_rollback_on_error_cleanup (void)
{
}
