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
#include "agt_cli.h"
#include "agt_nmda.h"
#include "agt_rpc.h"
#include "agt_util.h"
#include "cfg.h"
#include "getcb.h"
#include "log.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "ncx_feature.h"
#include "ncx_list.h"
#include "rpc.h"
#include "rpc_err.h"
#include "ses.h"
#include "ses_msg.h"
#include "status.h"
#include "tstamp.h"
#include "val.h"
#include "val123.h"
#include "val_util.h"
#include "xmlns.h"
#include "xml_util.h"
#include "xml_wr.h"
#include "yangconst.h"


/********************************************************************
* FUNCTION ping_pong
*
* INPUTS:
*    see agt/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t
    ping_pong(ses_cb_t *scb,
                  rpc_msg_t *msg,
                  xml_node_t *methnode)
{
    val_value_t        *ping_val;
    obj_template_t     *output_obj;
    obj_template_t     *pong_obj;
    val_value_t        *pong_val;
    val_value_t        *chval;
    status_t            res;

    ping_val = val_find_child(msg->rpc_input,
                             "test-anyxml",
                             "ping");
    assert(ping_val);
    printf("ping:\n");
    val_dump_value_max_w_file(ping_val,0/*startident*/,NCX_DEF_INDENT/*indent_amount*/,NCX_DISPLAY_MODE_XML,TRUE/*with_meta*/,FALSE/*configonly*/,stdout);

    output_obj = obj_find_child(
        msg->rpc_input->obj->parent,
        "test-anyxml",
        "output");
    assert(output_obj);

    pong_obj = obj_find_child(
        output_obj,
        "test-anyxml",
        "pong");
    assert(pong_obj);

    pong_val = val_new_value();
    assert(pong_val);

    val_init_from_template(pong_val, pong_obj);

    for(chval=val_get_first_child(ping_val);
        chval!=NULL;
        chval=val_get_next_child(chval)) {
        val_value_t        *newval;
        newval = val_clone(chval);
	val_add_child(newval, pong_val);
    }
#if 0
    res = val_set_simval_obj(
        output_rpc_val,
        output_rpc_val->obj,
        "<get xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\"><filter type=\"xpath\" select=\"/system\"/></get>");
#endif

    dlq_enque(pong_val, &msg->rpc_dataQ);
    msg->rpc_data_type = RPC_DATA_YANG;
    
    return NO_ERR;

} /* anyxml */

/********************************************************************
* FUNCTION y_test_anyxml_init
*
* INIT 1:
*   Initialize the module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    y_test_anyxml_init (void)
{
    agt_profile_t  *agt_profile;
    status_t        res;
    ncx_module_t    *mod;
    obj_template_t  *root_obj;
    val_value_t*    clivalset;
    val_value_t*    val;

    /* load in the RPC methods */
    res = ncxmod_load_module( "test-anyxml", NULL, NULL, NULL );
    assert(res == NO_ERR);

    /* yangcli-to-rpc */
    res = agt_rpc_register_method("test-anyxml",
                                  "ping-pong",
                                  AGT_RPC_PH_INVOKE,
                                  ping_pong);
    assert(res == NO_ERR);

    return NO_ERR;

}  /* y_test_anyxml_init */


/********************************************************************
* FUNCTION y_test_anyxml_init2
*
* INIT 2:
*   Initialize the data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    y_test_anyxml_init2 (void)
{

    return NO_ERR;

}  /* y_test_anyxml_init2 */


/********************************************************************
* FUNCTION y_test_anyxml_cleanup
*
* Cleanup the module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    y_test_anyxml_cleanup (void)
{
    agt_rpc_unregister_method("test-anyxml",
                              "ping-pong");

}  /* y_test_anyxml_cleanup */
