#include <assert.h>
#include <stdio.h>

#include "ncx.h"
#include "val.h"
#include "yangrpc.h"

int main(int argc, char* argv[])
{
    status_t res;
    yangrpc_cb_ptr_t yangrpc_cb_ptr;
    ncx_module_t * ietf_netconf_mod;
    obj_template_t* rpc_obj;
    obj_template_t* input_obj;
    obj_template_t* filter_obj;
    
    val_value_t* request_val;
    val_value_t* reply_val;
    val_value_t* filter_val;
    val_value_t* type_meta_val;
    val_value_t* select_meta_val;

    res = yangrpc_init(NULL);
    assert(res==NO_ERR);
    res = yangrpc_connect("localhost"/*server*/, 830/*port*/, "vladimir"/*user*/,"mysecretpass"/*password*/,"/home/vladimir/.ssh/id_rsa.pub"/*public_key*/, "/home/vladimir/.ssh/id_rsa"/*private_key*/, NULL, &yangrpc_cb_ptr);
    assert(res==NO_ERR);

    res = yangrpc_parse_cli(yangrpc_cb_ptr, "xget /interfaces-state", &request_val);
    assert(res==0);

    res = yangrpc_exec(yangrpc_cb_ptr, request_val, &reply_val);
    assert(res==0);

    val_dump_value(reply_val,0);
    {
    	val_value_t* data_val;
    	val_value_t* interfaces_state_val;
        data_val = val_find_child(reply_val,NULL,"data");
        interfaces_state_val = val_find_child(data_val,"ietf-interfaces","interfaces-state");
        val_dump_value(interfaces_state_val,0);
    }

    val_free_value(request_val);
    val_free_value(reply_val);

    yangrpc_close(yangrpc_cb_ptr);

    return 0;
}

