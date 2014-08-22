#include <assert.h>
#include <stdio.h>

#include "ncx.h"
#include "val.h"
#include "yangrpc.h"

int main(int argc, char* argv[])
{
    status_t res;
    yangrpc_cb_t *yangrpc_cb;
    ncx_module_t * ietf_netconf_mod;
    obj_template_t* rpc_obj;
    obj_template_t* input_obj;
    obj_template_t* filter_obj;
    
    val_value_t* request_val;
    val_value_t* reply_val;
    val_value_t* filter_val;
    val_value_t* type_meta_val;
    val_value_t* select_meta_val;

    res = yangrpc_init(argc, argv);
    assert(res==NO_ERR);
    yangrpc_cb = yangrpc_connect("127.0.0.1"/*server*/,"vladimir"/*user*/,""/*password*/,"/home/vladimir/.ssh/id_rsa.pub"/*public_key*/, "/home/vladimir/.ssh/id_rsa"/*private_key*/);

    res = ncxmod_load_module ("ietf-netconf", NULL, NULL, &ietf_netconf_mod);
    assert(res==NO_ERR);

    rpc_obj = ncx_find_object(ietf_netconf_mod, "get");
    assert(obj_is_rpc(rpc_obj));
    input_obj = obj_find_child(rpc_obj, NULL, "input");
    assert(input_obj!=NULL);
    filter_obj = obj_find_child(input_obj, NULL, "filter");
    assert(filter_obj!=NULL);

    request_val = val_new_value();
    val_init_from_template(request_val, rpc_obj);
    filter_val = val_new_value();
    val_init_from_template(filter_val, filter_obj);
    
    type_meta_val = val_make_string(0, "type","xpath");
    select_meta_val = val_make_string(0, "select", "/");

    val_add_meta(select_meta_val, filter_val);
    val_add_meta(type_meta_val, filter_val);
    val_add_child(filter_val, request_val);

    res = yangrpc_exec(yangrpc_cb, request_val, &reply_val);
    assert(res==0);

    val_dump_value(reply_val,0);
    {
    	val_value_t* data_val;
    	val_value_t* interfaces_state_val;
        data_val = val_find_child(reply_val,NULL,"data");
        interfaces_state_val = val_find_child(data_val,"ietf-interfaces","interfaces-state");
        val_dump_value(interfaces_state_val,0);
    }

    val_free_value(reply_val);

    yangrpc_close(yangrpc_cb);
    return 0;
}
