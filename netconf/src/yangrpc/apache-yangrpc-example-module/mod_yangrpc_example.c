/* Include the required headers from httpd */
#define _GNU_SOURCE
#include "httpd.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_config.h"
#include "http_log.h"


#include <time.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

/* YANG database access headers */
#include "ncx.h"
#include "val.h"
#include "val_util.h"
#include "val_set_cplxval_obj.h"
#include "xml_rd.h"
#include "xml_wr.h"
#include "yangrpc.h"

static char* server_address;
static int server_port;
static char* username;
static char* password;
static char* private_key_path;
static char* public_key_path;


/* Define prototypes of our functions in this module */
static void register_hooks(apr_pool_t *pool);
static int example_handler(request_rec *r);

typedef struct {
    yangrpc_cb_ptr_t yangrpc_cb_ptr;
} my_svr_cfg ;

static void* my_create_svr_conf(apr_pool_t* pool, server_rec* svr)
{
    my_svr_cfg* svr_cfg = (my_svr_cfg*)apr_pcalloc(pool, sizeof(my_svr_cfg));
    /* Set up the default values for fields of svr */

    svr_cfg->yangrpc_cb_ptr=NULL;
    return svr_cfg;
}

const char* server_address_cmd_func(cmd_parms* cmd, void* cfg, const char* arg)
{
    server_address=strdup(arg);
    return NULL;
}

const char* server_port_cmd_func(cmd_parms* cmd, void* cfg, const char* arg)
{
    server_port=atoi(arg);
    return NULL;
}

const char* username_cmd_func(cmd_parms* cmd, void* cfg, const char* arg)
{
    username=strdup(arg);
    return NULL;
}

const char* password_cmd_func(cmd_parms* cmd, void* cfg, const char* arg)
{
    password=strdup(arg);
    return NULL;
}

const char* private_key_path_cmd_func(cmd_parms* cmd, void* cfg, const char* arg)
{
    private_key_path=strdup(arg);
    return NULL;
}

const char* public_key_path_cmd_func(cmd_parms* cmd, void* cfg, const char* arg)
{
    public_key_path=strdup(arg);
    return NULL;
}

static const command_rec my_cmds[] = {
    AP_INIT_TAKE1("ServerAddress", server_address_cmd_func, NULL/*my_ptr*/, OR_ALL, "Server address e.g. 127.0.0.1 or myserver.org"),
    AP_INIT_TAKE1("ServerPort", server_port_cmd_func, NULL/*my_ptr*/, OR_ALL, "Server port e.g. 830"),
    AP_INIT_TAKE1("Username", username_cmd_func, NULL/*my_ptr*/, OR_ALL, "Username e.g. root"),
    AP_INIT_TAKE1("Password", password_cmd_func, NULL/*my_ptr*/, OR_ALL, "Password e.g. mypass"),
    AP_INIT_TAKE1("PrivateKeyPath", private_key_path_cmd_func, NULL/*my_ptr*/, OR_ALL, "Private key path e.g. /root/.ssh/id_rsa"),
    AP_INIT_TAKE1("PublicKeyPath", public_key_path_cmd_func, NULL/*my_ptr*/, OR_ALL, "Public key path e.g. /root/.ssh/id_rsa.pub"),
    /* more directives as applicable */
    { NULL }
};

/* Define our module as an entity and assign a function for registering hooks  */

module AP_MODULE_DECLARE_DATA   yangrpc_example_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,            // Per-directory configuration handler
    NULL,            // Merge handler for per-directory configurations
    my_create_svr_conf, // Per-server configuration handler
    NULL,            // Merge handler for per-server configurations
    my_cmds,            // Any directives we may have for httpd
    register_hooks   // Our hook registering function
};

static char* get_username(int uid)
{
    struct passwd pwd;
    struct passwd *result;
    char* username;
    char *buf;
    size_t bufsize;
    int s;

    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)          /* Value was indeterminate */
        bufsize = 16384;        /* Should be more than enough */

    buf = malloc(bufsize);
    if (buf == NULL) {
        return NULL;
    }
    s = getpwuid_r(uid, &pwd, buf, bufsize, &result);
    if (result == NULL) {
        free(buf);
        return NULL;
    }
    username = malloc(strlen(pwd.pw_gecos)+1);
    strcpy(username, pwd.pw_name);
    free(buf);
    return username;
}

static int example_pre_config(apr_pool_t *pconf, apr_pool_t *plog, apr_pool_t *ptemp)
{

    server_address = strdup("127.0.0.1");
    server_port = 830;
    username = strdup("root");
    password = strdup("mysecretpass");
    private_key_path = strdup("/var/www/.ssh/id_rsa");
    public_key_path = strdup("/var/www/.ssh/id_rsa.pub");
    return OK;
}
/* register_hooks: Adds a hook to the httpd process */
static void register_hooks(apr_pool_t *pool) 
{
    ap_hook_pre_config(example_pre_config, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_handler(example_handler, NULL, NULL, APR_HOOK_LAST);
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

uint64_t get_counter(val_value_t* counter_abs_val, val_value_t* root_base_val)
{
    val_value_t* name_val;
    val_value_t* base_val;
    char xpath_str[1024];
    status_t res;
    uint64_t counter;
    counter=VAL_UINT64(counter_abs_val);
    if(root_base_val==NULL) {
        return counter;
    }
    name_val = val_find_child(counter_abs_val->parent->parent,"ietf-interfaces","name");
    sprintf(xpath_str,"/interfaces-state/interface[name='%s']/statistics/%s", VAL_STRING(name_val), obj_get_name(counter_abs_val->obj));
    res = xpath_find_val_target(root_base_val, NULL/*mod*/, xpath_str , &base_val);
    assert(res==NO_ERR);
    if(base_val) {
        counter -= VAL_UINT64(base_val);
    }
    return counter;
}

void serialize_ietf_interfaces_state_val(request_rec *r, val_value_t* root_val)
{
    int i;
    char* counter_name[]= {"in-octets","in-unicast-pkts","out-octets","out-unicast-pkts","in-errors", "in-discards"};

    char timestamp_string[]="2017-03-05T14:59:00+1234";
    struct tm* time_info;
    static val_value_t* root_epoch_val=NULL;
    static val_value_t* root_prev_val=NULL;
    static time_t ts_epoch=0;
    static time_t ts_cur;
    static time_t ts_prev;

    val_value_t* interfaces_state_val;
    val_value_t* interface_val;

    ts_cur=time(NULL);

    if(r->args && 0==strcmp(r->args,"clear=1")) {
        if(root_epoch_val!=NULL) {
            val_free_value(root_epoch_val);
        }
        ts_epoch=ts_cur;
        root_epoch_val = val_clone(root_val);

        if(root_prev_val!=NULL) {
            val_free_value(root_prev_val);
        }
        root_prev_val = NULL;
    }



    ap_rprintf(r, "<html><head>\
<meta http-equiv=\"content-type\" content=\"text/html; charset=windows-1252\">\
</head><body><table cellspacing=\"0\" width=\"620\">\
 <tbody><tr><td><h1>Statistics</h1></td><td width=\"60\"><form action=\"ietf-interfaces-state.html\"><input value=\"Refresh\" type=\"submit\"></form>\
   </td><td align=\"right\" width=\"100\"><form action=\"ietf-interfaces-state.html\"><input value=\"Clear Counters\" type=\"submit\"><input name=\"clear\" value=\"1\" type=\"hidden\"></form>\
   </td></tr></tbody></table>", timestamp_string);

    ap_rprintf(r, "<html><head>\
<meta http-equiv=\"content-type\" content=\"text/html; charset=windows-1252\">\
</head><body><table cellspacing=\"0\" width=\"620\">\
 <tbody>");

    time_info = localtime(&ts_epoch);
    strftime(timestamp_string, sizeof(timestamp_string), "%Y-%m-%dT%H:%M:%S%z", time_info);
    ap_rprintf(r, "<tr><td>Epoch:</td><td width=\"60\" align=\"right\">%s</td></tr>", timestamp_string);

    if(root_prev_val!=NULL) {
    time_info = localtime(&ts_prev);
    strftime(timestamp_string, sizeof(timestamp_string), "%Y-%m-%dT%H:%M:%S%z", time_info);
    ap_rprintf(r, "<tr><td>Tic:</td><td width=\"60\" align=\"right\">%s</td></tr>", timestamp_string);

    time_info = localtime(&ts_cur);
    strftime(timestamp_string, sizeof(timestamp_string), "%Y-%m-%dT%H:%M:%S%z", time_info);
    ap_rprintf(r, "<tr><td>Toc:</td><td width=\"60\" align=\"right\">%s</td></tr>", timestamp_string);

    ap_rprintf(r, "<tr><td>Interval (sec):</td><td width=\"60\" align=\"right\">%u</td></tr>", ts_cur-ts_prev);
    }

    ap_rprintf(r, "</tbody></table>");

    ap_rprintf(r, "<table border=\"1\" cellspacing=\"0\" width=\"620\">\
   <tbody>\
   <tr align=\"center\">\
    <th width=\"30\"><b>name</b></th>");

    for(i=0;i<6;i++) {
        if(0==strcmp(counter_name[i],"in-octets") || 0==strcmp(counter_name[i],"out-octets")) {
            ap_rprintf(r, "<th width=\"70\" colspan=\"3\"><b>%s</b></th>", counter_name[i]);
        } else {
            ap_rprintf(r, "<th width=\"70\" colspan=\"2\"><b>%s</b></th>", counter_name[i]);
        }
    }
    ap_rprintf(r, "</tr>");

   ap_rprintf(r, "<tr align=\"center\">\
    <td width=\"30\"><b>type</b></td>");

    for(i=0;i<6;i++) {
        ap_rprintf(r, "<td width=\"70\"><b>abs</b></th>");
        ap_rprintf(r, "<td width=\"70\"><b>rate</b></td>");
        if(0==strcmp(counter_name[i],"in-octets") || 0==strcmp(counter_name[i],"out-octets")) {
            ap_rprintf(r, "<td width=\"70\"><b>%</b></td>");
        }
    }
    ap_rprintf(r, "</tr>");



    interfaces_state_val = val_find_child(root_val,
                                    "ietf-interfaces",
                                    "interfaces-state");


    for (interface_val = val_get_first_child(interfaces_state_val);
         interface_val != NULL;
         interface_val = val_get_next_child(interface_val)) {
 
        val_value_t* name_val;
        val_value_t* statistics_val;
        val_value_t* speed_val;
        name_val = val_find_child(interface_val,"ietf-interfaces","name");

        ap_rprintf(r, "<tr><td align=\"left\"><a href=\"state.xml?xpath=/interfaces-state/interface[name='%s']\"><b>%s</b></a></td>", VAL_STRING(name_val), VAL_STRING(name_val));

        statistics_val = val_find_child(interface_val, "ietf-interfaces", "statistics");
        speed_val = val_find_child(interface_val, "ietf-interfaces", "speed");

        for(i=0;i<6;i++) {
            val_value_t* val; 
            uint64_t rate;
            uint64_t speed_in_bytes;
            if(speed_val) {
                speed_in_bytes=VAL_UINT64(speed_val)/8;
            } else {
                speed_in_bytes=100000000/8; /* workaround use 100Mb */
            }
            val = val_find_child(statistics_val,
                                    "ietf-interfaces",
                                    counter_name[i]);

            ap_rprintf(r, "<td align=\"right\">");
            if(val!=NULL) {
                uint64_t counter;
                char buf[20+1];
                //counter = VAL_UINT64(val);
                counter=get_counter(val,root_epoch_val);
                sprintf(buf,"%lld",counter);
                ap_rprintf(r, buf);
            }
            ap_rprintf(r, "</td>");

            /* per sec. */
            ap_rprintf(r, "<td align=\"right\">");
            if(val!=NULL && root_prev_val!=NULL && (ts_cur-ts_prev)!=0) {
                uint64_t counter;
                char buf[20+1];
                //counter = VAL_UINT64(val);
                counter=get_counter(val,root_prev_val);
                rate=counter/(ts_cur-ts_prev);
                sprintf(buf,"%lld",rate);
                if(root_prev_val!=NULL) {
                    ap_rprintf(r, buf);
                }
            }
            ap_rprintf(r, "</td>");

            if(0!=strcmp(counter_name[i],"in-octets") && 0!=strcmp(counter_name[i],"out-octets")) {
                continue;
            }
            /* % */
            /* per sec. */
            ap_rprintf(r, "<td align=\"right\">");
            if(val!=NULL && root_prev_val!=NULL && (ts_cur-ts_prev)!=0) {
                uint64_t counter;
                char buf[20+1];
                //counter = VAL_UINT64(val);
                counter=get_counter(val,root_prev_val);
                rate=counter/(ts_cur-ts_prev);
                sprintf(buf,"%lld %",100*rate/speed_in_bytes);
                if(root_prev_val!=NULL) {
                    ap_rprintf(r, buf);
                }
            }
            ap_rprintf(r, "</td>");
        }
    }
    ap_rprintf(r, "%s","</tbody></table></body></html>");

    if(root_prev_val!=NULL) {
        val_free_value(root_prev_val);
    }
    root_prev_val = val_clone(root_val);
    ts_prev=ts_cur;
}

void serialize_val(request_rec *r, val_value_t* root_val)
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


    if(0==strcmp(r->uri, "/config.xml")) {
        testfn=nodetest_fn;
    } else {
        testfn=NULL;
    }

    res = xml_wr_check_open_file(fp,
                        root_val,
                        &attrs,
                        TRUE/*docmode*/,
                        FALSE/*xmlhdr*/,
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
        {
            char *s;
            for(s = val; *s; ++s) {
                if ('+' == *s) {
                    *s = ' ';
                }
            }
        }

        ap_unescape_url((char*)val);

        apr_table_merge(*tab, key, val);
    }

    return OK;
}

static int edit_config_form(request_rec *r)
{
    ap_rprintf(r, "%s","<html><body><form name=\"input\" action=\"/edit-config.xml\" method=\"post\"><textarea name=\"config\" rows=25 cols=80 ></textarea><input type=\"submit\" value=\"Submit\"></form></body></html>");
    return OK;
}

static int edit_config(request_rec *r)
{
    status_t res;
    ncx_module_t * netconf_mod;
    apr_table_t *tab=NULL;
    const char* config_xml_str;
    obj_template_t* root_obj;
    val_value_t* root_val;
    int rc;
    obj_template_t* edit_config_rpc_obj;
    obj_template_t* input_obj;
    obj_template_t* commit_rpc_obj;
    val_value_t* edit_config_rpc_val;
    val_value_t* edit_config_rpc_reply_val;
    val_value_t* commit_rpc_val;
    val_value_t* commit_rpc_reply_val;
    char* rpc_format_str;
    char* rpc_str;
    my_svr_cfg* svr_cfg;

    FILE *fp = NULL;
    rc = read_post(r, &tab);
    if(rc!=OK) return rc;

    config_xml_str = apr_table_get(tab, "config") ;
    if(config_xml_str==NULL) {
        return DECLINED;
    }
    //ap_rprintf(r,"%s",config_xml_str);

    svr_cfg = ap_get_module_config(r->server->module_config, &yangrpc_example_module);

#if 0
    rc = val_set_cplxval_obj(dst_val,dst_val->obj,config_xml_str);
    if(rc != NO_ERR) {
        return DECLINED;
    }
#else
    fp = fmemopen((void*)config_xml_str, strlen(config_xml_str), "r");
    res = ncxmod_load_module (NCXMOD_NETCONF, NULL, NULL, &netconf_mod);
    root_obj = ncx_find_object(netconf_mod, "config");

    rc = xml_rd_open_file (fp, root_obj, &root_val);


    edit_config_rpc_obj = ncx_find_object(netconf_mod, "edit-config");
    assert(obj_is_rpc(edit_config_rpc_obj));
    input_obj = obj_find_child(edit_config_rpc_obj, NULL, "input");
    assert(input_obj!=NULL);


    rpc_format_str = "<input><target><candidate/></target><default-operation>replace</default-operation>%s</input>";
    rpc_str = malloc(strlen(rpc_format_str)+strlen(config_xml_str)+1);
    sprintf(rpc_str, rpc_format_str, config_xml_str);

    edit_config_rpc_val = val_new_value();
    val_init_from_template(edit_config_rpc_val, edit_config_rpc_obj);
    res = val_set_cplxval_obj(edit_config_rpc_val, input_obj, rpc_str);
    free(rpc_str);
    if(res != NO_ERR) {
        val_free_value(edit_config_rpc_val);
        edit_config_rpc_val = NULL;
        edit_config_rpc_reply_val=NULL;
    } else {
        res = yangrpc_exec(svr_cfg->yangrpc_cb_ptr, edit_config_rpc_val, &edit_config_rpc_reply_val);
        assert(res==NO_ERR);
    }

    if(edit_config_rpc_reply_val!=NULL && NULL!=val_find_child(edit_config_rpc_reply_val,NULL,"ok")) {
        commit_rpc_obj = ncx_find_object(netconf_mod, "commit");
        assert(obj_is_rpc(commit_rpc_obj));
        commit_rpc_val = val_new_value();
        val_init_from_template(commit_rpc_val, commit_rpc_obj);
        res = yangrpc_exec(svr_cfg->yangrpc_cb_ptr, commit_rpc_val, &commit_rpc_reply_val);
        assert(res==NO_ERR);
    } else {
        commit_rpc_val=NULL;
        commit_rpc_reply_val=NULL;
    }


#endif
    ap_rprintf(r,"<netconf-chat>");
    if(edit_config_rpc_val) {
        serialize_val(r, edit_config_rpc_val);
        val_free_value(edit_config_rpc_val);
    }
    if(edit_config_rpc_reply_val) {
        serialize_val(r, edit_config_rpc_reply_val);
        val_free_value(edit_config_rpc_reply_val);
    }
    if(commit_rpc_val) {
        serialize_val(r, commit_rpc_val);
        val_free_value(commit_rpc_val);
    }
    if(commit_rpc_reply_val) {
        serialize_val(r, commit_rpc_reply_val);
        val_free_value(commit_rpc_reply_val);
    }
    ap_rprintf(r,"</netconf-chat>");
    apr_table_clear(tab);
    return OK;
}


static int ietf_interfaces_state_report(request_rec *r)
{
    status_t res;
    ncx_module_t * netconf_mod;
    apr_table_t *tab=NULL;
    const char* config_xml_str;
    obj_template_t* root_obj;
    obj_template_t* rpc_obj;
    obj_template_t* filter_obj;
    val_value_t* root_val;
    val_value_t* request_val;
    val_value_t* filter_val;
    val_value_t* select_meta_val;
    val_value_t* type_meta_val;
    val_value_t* reply_val;
    int rc;
    obj_template_t* input_obj;
    char* rpc_format_str;
    char* rpc_str;
    my_svr_cfg* svr_cfg;

    FILE *fp = NULL;
    rc = read_post(r, &tab);
    if(rc!=OK) return rc;

    svr_cfg = ap_get_module_config(r->server->module_config, &yangrpc_example_module);


    res = ncxmod_load_module (NCXMOD_NETCONF, NULL, NULL, &netconf_mod);
    assert(res==NO_ERR);

    rpc_obj = ncx_find_object(netconf_mod, "get");
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
    select_meta_val = val_make_string(0, "select", "/interfaces-state");

    val_add_meta(select_meta_val, filter_val);
    val_add_meta(type_meta_val, filter_val);
    val_add_child(filter_val, request_val);


    res = yangrpc_exec(svr_cfg->yangrpc_cb_ptr, request_val, &reply_val);
    assert(res==NO_ERR);

    {
        obj_template_t* config_obj;
        val_value_t* config_val;
        val_value_t* data_val;
        val_value_t* interfaces_val;
        val_value_t* interface_val;
        char* interface_row_str[512];

        data_val = val_find_child(reply_val,NULL,"data");
        config_obj = ncx_find_object(netconf_mod, "config");
        config_val = val_new_value();
        val_init_from_template(config_val, config_obj);
        val_move_children(data_val,config_val);
        //serialize_val(r, config_val);
	serialize_ietf_interfaces_state_val(r,config_val);
        val_free_value(config_val);

    }
    val_free_value(request_val);
    val_free_value(reply_val);

    //ap_rprintf(r,"</netconf-chat>");
    return OK;
}

/* The handler function for our module.
 * This is where all the fun happens!
 */
static int example_handler(request_rec *r)
{
    status_t res;
    ncx_module_t * netconf_mod;
    obj_template_t* rpc_obj;
    obj_template_t* input_obj;
    obj_template_t* filter_obj;

    val_value_t* request_val;
    val_value_t* reply_val;
    val_value_t* filter_val;
    val_value_t* type_meta_val;
    val_value_t* select_meta_val;
    my_svr_cfg* svr_cfg;

    /* First off, we need to check if this is a call for the "example" handler.
     * If it is, we accept it and do our things, it not, we simply return DECLINED,
     * and Apache will try somewhere else.
     */
    if (!r->handler || (0!=strcmp(r->uri, "/config.xml") &&
                        0!=strcmp(r->uri, "/state.xml") &&
                        0!=strcmp(r->uri, "/edit-config.html") &&
                        0!=strcmp(r->uri, "/edit-config.xml") &&
                        0!=strcmp(r->uri, "/ietf-interfaces-state.html")
       )) return (DECLINED);

    svr_cfg = ap_get_module_config(r->server->module_config, &yangrpc_example_module);

    if(svr_cfg->yangrpc_cb_ptr == NULL) {
        int res;
        char* arg = "--keep-session-model-copies-after-compilation=false";
        res = yangrpc_init(arg);
        assert(res==NO_ERR);

        res = yangrpc_connect(server_address, server_port, username, password, public_key_path, private_key_path, NULL /*extra_args*/, &svr_cfg->yangrpc_cb_ptr);
        if(res!=NO_ERR) {
            assert(0);
            return OK;
        }
    }

    res = ncxmod_load_module (NCXMOD_NETCONF, NULL, NULL, &netconf_mod);
    assert(res==NO_ERR);
    if(0==strcmp(r->uri, "/edit-config.html")) {
        return edit_config_form(r);
    } else if(0==strcmp(r->uri, "/ietf-interfaces-state.html")) {
        return ietf_interfaces_state_report(r);
    } else if(0==strcmp(r->uri, "/edit-config.xml")) {
        return edit_config(r);
    }

    rpc_obj = ncx_find_object(netconf_mod, "get");
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


    res = yangrpc_exec(svr_cfg->yangrpc_cb_ptr, request_val, &reply_val);
    assert(res==NO_ERR);

    {
        obj_template_t* config_obj;
        val_value_t* config_val;
    	val_value_t* data_val;
    	val_value_t* interfaces_val;
    	val_value_t* interface_val;
    	char* interface_row_str[512];

        data_val = val_find_child(reply_val,NULL,"data");
        config_obj = ncx_find_object(netconf_mod, "config");
        config_val = val_new_value();
        val_init_from_template(config_val, config_obj);
        val_move_children(data_val,config_val);
        serialize_val(r, config_val);
        val_free_value(config_val);

    }
    val_free_value(request_val);
    val_free_value(reply_val);
    return OK;
}
