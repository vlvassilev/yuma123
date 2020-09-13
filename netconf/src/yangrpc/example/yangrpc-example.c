/* Usage:
   ./yangrpc-example --server=myserver.com --port=830 --user=vladimir --password='mypass' \
                     --private-key=/home/vladimir/.ssh/id_rsa \
                     --public-key=/home/vladimir/.ssh/id_rsa.pub
*/

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "ncx.h"
#include "ncxmod.h"
#include "val.h"
#include "yangrpc.h"

static struct option const long_options[] =
{
    {"server", required_argument, NULL, 's'},
    {"port", required_argument, NULL, 'p'},
    {"user", required_argument, NULL, 'u'},
    {"password", required_argument, NULL, 'P'},
    {"private-key", required_argument, NULL, 'k'},
    {"public-key", required_argument, NULL, 'K'},
    {NULL, 0, NULL, 0}
};

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

    int optc;

    char* server="127.0.0.1";
    unsigned int port=830;
    char* user=getlogin();
    char* password=NULL;
    char private_key[1024];
    char public_key[1024];

    sprintf(private_key,"/home/%s/.ssh/id_rsa",user);
    sprintf(public_key,"/home/%s/.ssh/id_rsa.pub",user);

    while ((optc = getopt_long (argc, argv, "s:p:u:P:k:K", long_options, NULL)) != -1) {
        switch (optc) {
            case 's':
                server=optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'u':
                user = optarg;
                break;
            case 'P':
                password = optarg;
                break;
            case 'k':
                strcpy(private_key,optarg);
                break;
            case 'K':
                strcpy(public_key,optarg);
                break;
            default:
                exit (-1);
        }
    }


    res = yangrpc_init(NULL);
    assert(res==NO_ERR);
    res = yangrpc_connect(server /*127.0.0.1*/, port /*830*/, user /*vladimir*/, password /* "" */, public_key /* "/home/vladimir/.ssh/id_rsa.pub" */, private_key /* "/home/vladimir/.ssh/id_rsa" */, NULL, &yangrpc_cb_ptr);
    assert(res==NO_ERR);

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
