/* Include the required headers from httpd */
#define _GNU_SOURCE
#include "httpd.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_config.h"


#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

/* YANG database access headers */
#include "ncx.h"
#include "val.h"
#include "val_util.h"
#include "yangrpc.h"

yangrpc_cb_t* yangrpc_cb;

/* Define prototypes of our functions in this module */
static void register_hooks(apr_pool_t *pool);
static int example_handler(request_rec *r);

/* Define our module as an entity and assign a function for registering hooks  */

module AP_MODULE_DECLARE_DATA   yanghttpd_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,            // Per-directory configuration handler
    NULL,            // Merge handler for per-directory configurations
    NULL,            // Per-server configuration handler
    NULL,            // Merge handler for per-server configurations
    NULL,            // Any directives we may have for httpd
    register_hooks   // Our hook registering function
};


/* register_hooks: Adds a hook to the httpd process */
static void register_hooks(apr_pool_t *pool) 
{
    /* Hook the request handler */
    ap_hook_handler(example_handler, NULL, NULL, APR_HOOK_LAST);
    {
    	status_t res;
    	char* argv[] = {"blah"};
    	int argc = 1;
        res = yangrpc_init(argc, argv);
        assert(res==NO_ERR);
        yangrpc_cb = yangrpc_connect("127.0.0.1"/*server*/,"root"/*user*/,"hadm1_123"/*password*/, "/home/vladimir/.ssh_/id_rsa.pub"/*public-key*/, "/home/vladimir/.ssh_/id_rsa"/*private-key*/);
    }
}

static ssize_t writer_fn(void *cookie, const void *buffer, size_t size)
{
    size_t bytes_sent=0;
    while(bytes_sent<size) {
    	size_t res;
        res = ap_rwrite((char*)buffer+bytes_sent, size-bytes_sent, (request_rec *)cookie);
        if(res<=0) {
            return res;
        }
        bytes_sent+=res;
    }
}
boolean nodetest_fn (ncx_withdefaults_t withdef,
                          boolean realtest,
			  val_value_t *node)
{
    return val_is_config_data(node);
}

void serialize_root(request_rec *r, val_value_t* root_val)
{
    status_t res;
    FILE* fp;
    cookie_io_functions_t io_functions;
    xml_attrs_t        attrs;
    val_nodetest_fn_t  testfn;
    xml_init_attrs(&attrs);
    io_functions.read=NULL;
    io_functions.write=writer_fn;
    io_functions.seek=NULL;
    io_functions.close=NULL;
    

    fp=fopencookie (r, "w", io_functions);
    ap_set_content_type(r, "application/xml;charset=utf-8");


    if(0==strcmp(r->uri, "/yanghttpd/config.xml")) {
        testfn=nodetest_fn;
    } else {
        testfn=NULL;
    }

    res = xml_wr_check_open_file(fp,
                        root_val,
                        &attrs,
                        TRUE/*docmode*/,
                        TRUE/*xmlhdr*/,
                        TRUE/*withns*/,
                        0/*startindent*/,
                        4/*indent*/,
                        testfn);
    fclose(fp);

}

static int util_read(request_rec *r, const char **rbuf)
{
    int rc;

    if ((rc = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)) != OK) {
        return rc;
    }

    if (ap_should_client_block(r)) {
        char buff[AP_IOBUFSIZE];
        int rsize, len_read, rpos=0;
        long length = r->remaining;
        *rbuf = apr_pcalloc(r->pool, length + 1);

        while ((len_read = ap_get_client_block(r, buff, sizeof(buff)))>0) { 
            if ((rpos + len_read) > length) {
                rsize = length - rpos;
            }
            else {
                rsize = len_read;
            }
            memcpy((char*)*rbuf + rpos, buff, rsize);
            rpos += rsize;
        }
    }
    return rc;
}

#define DEFAULT_ENCTYPE "application/x-www-form-urlencoded"

static int read_post(request_rec *r, apr_table_t **tab)
{
    const char *data;
    const char *key, *val, *type;
    int rc = OK;

    if(r->method_number != M_POST) {
        return rc;
    }

    type = apr_table_get(r->headers_in, "Content-Type");
    if(strcasecmp(type, DEFAULT_ENCTYPE) != 0) {
        return DECLINED;
    }

    if((rc = util_read(r, &data)) != OK) {
        return rc;
    }

    *tab = apr_table_make(r->pool, 8);

    while(*data && (val = ap_getword(r->pool, &data, '&'))) { 
        key = ap_getword(r->pool, &val, '=');

        ap_unescape_url((char*)key);
        ap_unescape_url((char*)val);

        apr_table_merge(*tab, key, val);
    }

    return OK;
}


static int edit_config(request_rec *r)
{
    apr_table_t *tab=NULL;
    const char* config_str;
    int rc;
    rc = read_post(r, &tab);
    if(rc!=OK) return rc;

    config_str = apr_table_get(tab, "config") ;
    if(config_str==NULL) {
        return DECLINED;
    }
    ap_rprintf(r,"%s",config_str);
    apr_table_clear(tab);
    return OK;
}

/* The handler function for our module.
 * This is where all the fun happens!
 */
static int example_handler(request_rec *r)
{
    status_t res;
    ncx_module_t * ietf_netconf_mod;
    obj_template_t* rpc_obj;
    obj_template_t* input_obj;
    obj_template_t* filter_obj;

    val_value_t* request_val;
    val_value_t* reply_val;
    val_value_t* filter_val;
    val_value_t* type_meta_val;
    val_value_t* select_meta_val;

    /* First off, we need to check if this is a call for the "example" handler.
     * If it is, we accept it and do our things, it not, we simply return DECLINED,
     * and Apache will try somewhere else.
     */
    if (!r->handler || (0!=strcmp(r->uri, "/yanghttpd/config.xml") &&
                        0!=strcmp(r->uri, "/yanghttpd/state.xml") &&
                        0!=strcmp(r->uri, "/yanghttpd/edit-config.html")
       )) return (DECLINED);

    
    res = ncxmod_load_module ("ietf-netconf", NULL, NULL, &ietf_netconf_mod);
    assert(res==NO_ERR);
    if(0==strcmp(r->uri, "/yanghttpd/edit-config.html")) {
        return edit_config(r);
    }

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
    assert(res==NO_ERR);

    {
    	val_value_t* data_val;
    	val_value_t* interfaces_val;
    	val_value_t* interface_val;
    	char* interface_row_str[512];

        data_val = val_find_child(reply_val,NULL,"data");
        serialize_root(r, data_val);

    }
    val_free_value(reply_val);
    return OK;
}
