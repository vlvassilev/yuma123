/*
    module test-multiple-edit-callbacks(based on ietf-interfaces.yang)
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

#ifdef INSTANCE_A
#define INSTANCE_NAME(func) y_test_multiple_edit_callbacks_a_ ## func 
#define INSTANCE_STR "a"
#elif INSTANCE_B
#define INSTANCE_NAME(func) y_test_multiple_edit_callbacks_b_ ## func 
#define INSTANCE_STR "b"
#else
#error "Unexpected instance"
#endif


static status_t
    myget(ses_cb_t *scb,
          getcb_mode_t cbmode,
          val_value_t *vir_val,
          val_value_t  *dst_val)
{
    status_t res;
    res = val_set_simval_obj(dst_val,
                         dst_val->obj,
                         "Hello from " INSTANCE_STR "!");
    return res;
}

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
        if(newval!=NULL) {
            val_value_t* val;
            obj_template_t* obj;

            printf("register %s\n", "blah");

            obj = obj_find_child(newval->obj,
                             "test-multiple-edit-callbacks",
                             INSTANCE_STR);

            val = val_new_value();
            assert(val!=NULL);

            val_init_virtual(val, myget, obj);
            val_add_child(val, newval);
       }
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


/* The 3 mandatory callback functions: y_test_multiple_edit_callbacks_init, y_test_multiple_edit_callbacks_init2, y_test_multiple_edit_callbacks_cleanup */

status_t
    INSTANCE_NAME(init) (
        const xmlChar *modname,
        const xmlChar *revision)
{
    agt_profile_t *agt_profile;
    ncx_module_t *mod;
    status_t res;

    agt_profile = agt_get_profile();

    res = ncxmod_load_module(
        "ietf-interfaces",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    if (res != NO_ERR) {
        return res;
    }

    res = ncxmod_load_module(
        "test-multiple-edit-callbacks",
        NULL,
        &agt_profile->agt_savedevQ,
        &mod);
    if (res != NO_ERR) {
        return res;
    }

    res = agt_cb_register_callback(
        "ietf-interfaces",
        (const xmlChar *)"/interfaces/interface",
        (const xmlChar *)NULL /*"YYYY-MM-DD"*/,
        edit_cb);
    if (res != NO_ERR) {
        return res;
    }
    return res;
}

status_t INSTANCE_NAME(init2)(void)
{
    return NO_ERR;
}

void INSTANCE_NAME(cleanup) (void)
{
}
