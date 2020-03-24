/*
 * Copyright (c) 2013 - 2016, Vladimir Vassilev, All Rights Reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <libssh2.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

#include "mgr.h"

#include "procdefs.h"
#include "cli.h"
#include "conf.h"
#include "help.h"
#include "json_wr.h"
#include "log.h"
#include "ncxmod.h"
#include "mgr.h"
#include "mgr_hello.h"
#include "mgr_io.h"
#include "mgr_not.h"
#include "mgr_rpc.h"
#include "mgr_ses.h"
#include "ncx.h"
#include "ncx_list.h"
#include "ncx_num.h"
#include "ncx_str.h"
#include "ncxconst.h"
#include "ncxmod.h"
#include "obj.h"
#include "op.h"
#include "rpc.h"
#include "runstack.h"
#include "ses_msg.h"
#include "status.h"
#include "val.h"
#include "val_util.h"
#include "var.h"
#include "xml_util.h"
#include "xml_val.h"
#include "xml_wr.h"
#include "yangconst.h"
#include "yangcli.h"
#include "yangcli_cmd.h"
#include "yangcli_alias.h"
#include "yangcli_autoload.h"
#include "yangcli_autolock.h"
#include "yangcli_yang_library.h"
#include "yangcli_save.h"
#include "yangcli_tab.h"
#include "yangcli_uservars.h"
#include "yangcli_util.h"
#include "yangcli_globals.h"
#include "yangcli_wordexp.h"

#include "yangrpc.h"

extern void
    create_session (server_cb_t *server_cb);

static void
    yangrpc_notification_handler (ses_cb_t *scb,
                                  mgr_not_msg_t *msg,
                                  boolean *consumed)
{
    assert(0);
}

/********************************************************************
 * FUNCTION do_connect_
 * 
 * INPUTS:
 *   server_cb == server control block to use
 *   rpc == rpc header for 'connect' command
 *   line == input text from readline call, not modified or freed here
 *   start == byte offset from 'line' where the parse RPC method
 *            left off.  This is eiother empty or contains some 
 *            parameters from the user
 *   startupmode == TRUE if starting from init and should try
 *              to connect right away if the mandatory parameters
 *              are present.
 *               == FALSE to check --optional and add parameters
 *                  if set or any missing mandatory parms
 *
 * OUTPUTS:
 *   connect_valset parms may be set 
 *   create_session may be called
 *
 * RETURNS:
 *   status
 *********************************************************************/
status_t
    do_connect_ (server_cb_t *server_cb,
                obj_template_t *rpc,
                const xmlChar *line,
                uint32 start,
                boolean startupmode)
{
    obj_template_t        *obj;
    val_value_t           *connect_valset;
    val_value_t           *valset, *testval;
    status_t               res;
    boolean                s1, s2, s3, s4, s5, tcp;

#ifdef DEBUG
    if (server_cb == NULL) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* retrieve the 'connect' RPC template, if not done already */
    if (rpc == NULL) {
        rpc = ncx_find_object(get_yangcli_mod(), YANGCLI_CONNECT);
        if (rpc == NULL) {
            server_cb->state = MGR_IO_ST_IDLE;
            log_write("\nError finding the 'connect' RPC method");
            return ERR_NCX_DEF_NOT_FOUND;
        }
    }            

    obj = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (obj == NULL) {
        server_cb->state = MGR_IO_ST_IDLE;
        log_write("\nError finding the connect RPC 'input' node");        
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    res = NO_ERR;
    tcp = FALSE;

    connect_valset = get_connect_valset();

    /* process any parameters entered on the command line */
    valset = NULL;
#if 0
    if (line != NULL) {
        while (line[start] && xml_isspace(line[start])) {
            start++;
        }
        if (line[start]) {
            valset = parse_rpc_cli(server_cb, rpc, &line[start], &res);
            if (valset == NULL || res != NO_ERR) {
                if (valset != NULL) {
                    val_free_value(valset);
                }
                log_write("\nError in the parameters for '%s' command (%s)",
                          obj_get_name(rpc), 
                          get_error_string(res));
                server_cb->state = MGR_IO_ST_IDLE;
                return res;
            }
        }
    }
#endif
    if (valset == NULL) {
        if (startupmode) {
            /* just clone the connect valset to start with */
            valset = val_clone(connect_valset);
            if (valset == NULL) {
                server_cb->state = MGR_IO_ST_IDLE;
                log_write("\nError: malloc failed");
                return ERR_INTERNAL_MEM;
            }
        } else {
            valset = val_new_value();
            if (valset == NULL) {
                log_write("\nError: malloc failed");
                server_cb->state = MGR_IO_ST_IDLE;
                return ERR_INTERNAL_MEM;
            } else {
                val_init_from_template(valset, obj);
            }
        }
    }

    /* make sure the 3 required parms are set */
    s1 = val_find_child(valset, YANGCLI_MOD, 
                        YANGCLI_SERVER) ? TRUE : FALSE;
    s2 = val_find_child(valset, YANGCLI_MOD,
                        YANGCLI_USER) ? TRUE : FALSE;
    s3 = val_find_child(valset, YANGCLI_MOD,
                        YANGCLI_PASSWORD) ? TRUE : FALSE;
    s4 = val_find_child(valset, YANGCLI_MOD,
                        YANGCLI_PUBLIC_KEY) ? TRUE : FALSE;
    s5 = val_find_child(valset, YANGCLI_MOD,
                        YANGCLI_PRIVATE_KEY) ? TRUE : FALSE;

    /* check the transport parameter */
    testval = val_find_child(valset, 
                             YANGCLI_MOD,
                             YANGCLI_TRANSPORT);
    if (testval != NULL && 
        testval->res == NO_ERR && 
        !xml_strcmp(VAL_ENUM_NAME(testval),
                    (const xmlChar *)"tcp")) {
        tcp = TRUE;
    }
#if 0    
    /* complete the connect valset if needed
     * and transfer it to the server_cb version
     *
     * try to get any missing params in valset 
     */
    if (interactive_mode()) {
        if (startupmode && s1 && s2 && ((s3 || (s4 && s5)) || tcp)) {
            if (LOGDEBUG3) {
                log_debug3("\nyangcli: CLI direct connect mode");
            }
        } else {
            res = fill_valset(server_cb, rpc, valset, connect_valset, 
                              TRUE, FALSE);
            if (res == ERR_NCX_SKIPPED) {
                res = NO_ERR;
            }
        }
    }
#endif
    /* check error or operation canceled */
    if (res != NO_ERR) {
        if (res != ERR_NCX_CANCELED) {
            log_write("\nError: Connect failed (%s)", 
                      get_error_string(res));
        }
        server_cb->state = MGR_IO_ST_IDLE;
        val_free_value(valset);
        return res;
    }

    /* passing off valset memory here */
    s1 = s2 = s3 = s4 = s5 = FALSE;
    if (valset != NULL) {
        /* save the malloced valset */
        if (server_cb->connect_valset != NULL) {
            val_free_value(server_cb->connect_valset);
        }
        server_cb->connect_valset = valset;

        /* make sure the 3 required parms are set */
        s1 = val_find_child(server_cb->connect_valset,
                            YANGCLI_MOD, 
                            YANGCLI_SERVER) ? TRUE : FALSE;
        s2 = val_find_child(server_cb->connect_valset, 
                            YANGCLI_MOD,
                            YANGCLI_USER) ? TRUE : FALSE;
        s3 = val_find_child(server_cb->connect_valset, 
                            YANGCLI_MOD,
                            YANGCLI_PASSWORD) ? TRUE : FALSE;
        s4 = val_find_child(server_cb->connect_valset,
                            YANGCLI_MOD,
                            YANGCLI_PUBLIC_KEY) ? TRUE : FALSE;
        s5 = val_find_child(server_cb->connect_valset,
                            YANGCLI_MOD,
                            YANGCLI_PRIVATE_KEY) ? TRUE : FALSE;
    }

    /* check if all params present yet */
    if (s1 && s2 && ((s3 || (s4 && s5)) || tcp)) {

        res = replace_connect_valset(server_cb->connect_valset);
        if (res != NO_ERR) {
            log_warn("\nWarning: connection parameters could not be saved");
            res = NO_ERR;
        }
        create_session(server_cb);
    } else {
        res = ERR_NCX_MISSING_PARM;
        log_write("\nError: Connect failed due to missing parameter(s)");
        server_cb->state = MGR_IO_ST_IDLE;
    }

    return res;

}  /* do_connect_ */

status_t yangrpc_init(char* args)
{
    int ret;
    obj_template_t       *obj;
    status_t              res;
    log_debug_t           log_level;
    yangcli_wordexp_t p;
    char* prog_w_args;
#ifdef YANGCLI_DEBUG
    int   i;
#endif
    prog_w_args = malloc(strlen("prog-placeholder ") + ((args==NULL)?0:strlen(args)) + 1);
    sprintf(prog_w_args, "prog-placeholder %s",(args==NULL)?"":args);
    ret = yangcli_wordexp(prog_w_args, &p, 0);
    free(prog_w_args);
    if(ret!=0) {
        perror(args);
        return ERR_CMDLINE_OPT_UNKNOWN;
    }

    /* set the default debug output level */
    log_level = LOG_DEBUG_INFO;

    yangcli_init_module_static_vars();

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     */
    res = ncx_init(NCX_SAVESTR, log_level, TRUE, NULL, p.we_wordc, p.we_wordv);
    if (res != NO_ERR) {
        return res;
    }

#ifdef YANGCLI_DEBUG
    if (p.we_wordc>1 && LOGDEBUG2) {
        log_debug2("\nCommand line parameters:");
        for (i=0; i<argc; i++) {
            log_debug2("\n   arg%d: %s", i, argv[i]);
        }
    }
#endif

    /* make sure the Yuma directory
     * exists for saving per session data
     */
    res = ncxmod_setup_yumadir();
    if (res != NO_ERR) {
        log_error("\nError: could not setup yuma dir '%s'",
                  ncxmod_get_yumadir());
        return res;
    }

    /* make sure the Yuma temp directory
     * exists for saving per session data
     */
    res = ncxmod_setup_tempdir();
    if (res != NO_ERR) {
        log_error("\nError: could not setup temp dir '%s/tmp'",
                  ncxmod_get_yumadir());
        return res;
    }

    /* at this point, modules that need to read config
     * params can be initialized
     */

    /* Load the yangcli base module */
    res = load_base_schema();
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the Netconf Manager Library */
    res = mgr_init();
    if (res != NO_ERR) {
        return res;
    }

    /* set up handler for incoming notifications */
    mgr_not_set_callback_fn(yangrpc_notification_handler);

    /* init the connect parmset object template;
     * find the connect RPC method
     * !!! MUST BE AFTER load_base_schema !!!
     */
    obj = ncx_find_object(yangcli_mod, YANGCLI_CONNECT);
    if (obj==NULL) {
        return ERR_NCX_DEF_NOT_FOUND;
    }

    /* set the parmset object to the input node of the RPC */
    obj = obj_find_child(obj, NULL, YANG_K_INPUT);
    if (obj==NULL) {
        return ERR_NCX_DEF_NOT_FOUND;
    }

#if 0
    /* set the CLI handler */
    mgr_io_set_stdin_handler(yangcli_stdin_handler);
#endif


    return NO_ERR;
}

/********************************************************************
* FUNCTION autoload_blocking_get_modules
* *
* Go through all the search result records and 
* make sure all files are present.  Try to use the
* <get-schema> operation to fill in any missing modules
*
* INPUTS:
*   server_cb == server session control block to use
*   scb == session control block to use
*
* OUTPUTS:
*   $HOME/.yuma/tmp/<progdir>/<sesdir>/ filled with
*   the the specified YANG files that are ertrieved from
*   the device with <get-schema>
*   
* RETURNS:
*    status
*********************************************************************/
status_t
    autoload_blocking_get_modules (server_cb_t *server_cb,
                                ses_cb_t *scb)
{
    ncxmod_search_result_t  *searchresult;
    status_t                 res;
    obj_template_t          *rpc;
    val_value_t             *reqdata;
    val_value_t             *reply_val;

#ifdef DEBUG
    if (!server_cb || !scb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    /* find first file that needs to be retrieved with get-schema */
    for (searchresult = (ncxmod_search_result_t *)
             dlq_firstEntry(&server_cb->searchresultQ);
         searchresult != NULL;
         searchresult = (ncxmod_search_result_t *)
             dlq_nextEntry(searchresult)) {

        /* skip found entries */
        if (searchresult->source != NULL) {
            continue;
        }

        /* skip found modules with errors */          
        if (!(searchresult->res == ERR_NCX_WRONG_VERSION ||
              searchresult->res == ERR_NCX_MOD_NOT_FOUND)) {
            continue;
        }
        server_cb->cursearchresult = searchresult;
#if 0
        res = send_get_schema_to_server(server_cb,
                                       scb,
                                       searchresult->module,
                                       searchresult->revision);
        if (res == NO_ERR) {
            server_cb->command_mode = CMD_MODE_AUTOLOAD;
            server_cb->cursearchresult = searchresult;
        }
        /* exit loop if we get here */
#else
	log_info("\n<get-schema>:%s",searchresult->module);
        res = make_get_schema_reqdata(server_cb,
                                      scb,
                                      searchresult->module,
                                      searchresult->revision,
                                      &rpc,
                                      &reqdata);
        assert(res == NO_ERR);
        res = yangrpc_exec((yangrpc_cb_ptr_t) server_cb, reqdata, &reply_val);
        res = get_schema_reply_to_temp_filcb(server_cb, (mgr_scb_t *)scb->mgrcb /*mscb*/, searchresult->module, searchresult->revision, reply_val);
        if (res != NO_ERR) {
            log_error("\nError: save <get-schema> content "
                      " for module '%s' revision '%s' failed (%s)",
                      searchresult->module,
                      (searchresult->revision) ? searchresult->revision : EMPTY_STRING,
                      get_error_string(res));
            searchresult->res = res;
        }
	continue;
#endif
    }

    return res;

}  /* autoload_blocking_get_modules */

/********************************************************************
* FUNCTION yang_library_blocking_get_modules
*
* Send <get> for /modules-state
*
* INPUTS:
*   server_cb == server session control block to use
*   scb == session control block to use
*
* OUTPUTS:
*
*
* RETURNS:
*    status
*********************************************************************/
status_t
    yang_library_blocking_get_module_set (server_cb_t *server_cb,
                                ses_cb_t *scb)
{
    status_t                 res;
    obj_template_t          *rpc;
    val_value_t             *reqdata;
    val_value_t             *reply_val;

    res = make_get_yang_library_modules_state_reqdata(server_cb,
                                      scb,
                                      &rpc,
                                      &reqdata);
    assert(res == NO_ERR);
    res = yangrpc_exec((yangrpc_cb_ptr_t) server_cb, reqdata, &reply_val);
    assert(res == NO_ERR);
    res = get_yang_library_modules_state_reply_to_searchresult_entries(server_cb, scb, reply_val);
    return res;

}  /* yang_library_blocking_get_modules */

status_t yangrpc_connect(const char * const server, uint16_t port,
                         const char * const user,
                         const char * const password,
                         const char * const public_key,
                         const char * const private_key,
                         const char * const extra_args,
                         yangrpc_cb_ptr_t* yangrpc_cb_ptr)
{
    char* server_arg;
    char* port_arg;
    char* user_arg;
    char* password_arg;
    char* public_key_arg;
    char* private_key_arg;
    char* mandatory_argv[]={"exec-name-dummy", "--server=?", "--port=?", "--user=?", "--password=?", "--private-key=?", "--public-key=?"};
    int mandatory_argc=sizeof(mandatory_argv)/sizeof(char*);
    char** argv;
    int argc;
    int extra_argc;
    server_cb_t          *server_cb;
    ses_cb_t             *ses_cb;
    status_t res;
    xmlChar versionbuffer[NCX_VERSION_BUFFSIZE];
    val_value_t          *parm, *modval;
    dlq_hdr_t             savedevQ;
    xmlChar              *savestr, *revision;
    uint32                modlen;
    int                   ret;
    yangcli_wordexp_t     p;
    obj_template_t       *obj;

    ncx_clear_temp_modQ();
    ncx_clear_session_modQ();

    /* new connect_valset */
    if(connect_valset!=NULL) {
        val_free_value(connect_valset);
        connect_valset=NULL;
    }
    connect_valset = val_new_value();
    assert(connect_valset);
    obj = ncx_find_object(yangcli_mod, YANGCLI_CONNECT);
    assert(obj!=NULL);
    val_init_from_template(connect_valset, obj);

    dlq_createSQue(&savedevQ);

    /* create a default server control block */
    server_cb = new_server_cb(YANGCLI_DEF_SERVER);
    if (server_cb==NULL) {
        log_error("\n new_server_cb failed (%s)", get_error_string(res));
        return ERR_INTERNAL_PTR;
    }
    argv = mandatory_argv;

    argc=0;
    argv[argc++]="yangrpc-conn-instance";

    server_arg = malloc(strlen("--server=")+strlen(server)+1);
    assert(server_arg!=NULL);
    sprintf(server_arg,"--server=%s",server);
    argv[argc++]=server_arg;

    port_arg = malloc(strlen("--ncport=")+strlen("65535")+1);
    assert(port_arg!=NULL);
    sprintf(port_arg,"--ncport=%u", (unsigned int)port);
    argv[argc++]=port_arg;

    user_arg = malloc(strlen("--user=")+strlen(user)+1);
    assert(user_arg!=NULL);
    sprintf(user_arg,"--user=%s",user);
    argv[argc++]=user_arg;

    /* at least password or public_key and private_key pair has to be specified */
    assert(password!=NULL || (public_key!=NULL && private_key!=NULL));

    if(password!=NULL) {
        password_arg = malloc(strlen("--password=")+strlen(password)+1);
        assert(password_arg!=NULL);
        sprintf(password_arg,"--password=%s",password);
        argv[argc++]=password_arg;
    }

    if(public_key!=NULL && private_key!=NULL) {
        public_key_arg = malloc(strlen("--public-key=")+strlen(public_key)+1);
        assert(public_key_arg!=NULL);
        sprintf(public_key_arg,"--public-key=%s",public_key);
        argv[argc++]=public_key_arg;

        private_key_arg = malloc(strlen("--private-key=")+strlen(private_key)+1);
        assert(private_key_arg!=NULL);
        sprintf(private_key_arg,"--private-key=%s",private_key);
        argv[argc++]=private_key_arg;
    }

    /* process extra args */
    if(extra_args!=NULL) {
        ret = yangcli_wordexp(extra_args, &p, 0);
        if(ret!=0) {
            perror(extra_args);
            log_error("\n yangcli_wordexp failed (%s)", get_error_string(ERR_CMDLINE_OPT_UNKNOWN));
            return ERR_CMDLINE_OPT_UNKNOWN;
        }
        extra_argc = p.we_wordc;
        argv = malloc((argc+extra_argc)*sizeof(char*));
        memcpy(argv,mandatory_argv,argc*sizeof(char*));
        memcpy(argv+argc,p.we_wordv,p.we_wordc*sizeof(char*));
        argc+=extra_argc;
        yangcli_wordfree(&p);
    } else {
        argv = malloc((argc)*sizeof(char*));
        memcpy(argv,mandatory_argv,argc*sizeof(char*));
    }

    /* Get any command line and conf file parameters */
    res = process_cli_input(server_cb, argc, argv);
    if (res != NO_ERR) {
        log_error("\n process_cli_input failed (%s)", get_error_string(res));
        return res;
    }

    /* set any server control block defaults which were supplied
     * in the CLI or conf file
     */
    update_server_cb_vars(server_cb);

    /* Load the NETCONF, XSD, SMI and other core modules */
    if (TRUE/*autoload*/) {
        res = load_core_schema();
        if (res != NO_ERR) {
            log_error("\n load_core_schema failed (%s)", get_error_string(res));
            return res;
        }
    }

    /* check if there are any deviation parameters to load first */
    for (modval = val_find_child(mgr_cli_valset, YANGCLI_MOD,
                                 NCX_EL_DEVIATION);
         modval != NULL && res == NO_ERR;
         modval = val_find_next_child(mgr_cli_valset, YANGCLI_MOD,
                                      NCX_EL_DEVIATION,
                                      modval)) {

        res = ncxmod_load_deviation(VAL_STR(modval), &savedevQ);
        if (res != NO_ERR) {
            log_error("\n load deviation failed (%s)", 
                      get_error_string(res));
        } else {
            log_info("\n load OK");
        }
    }

    if (res == NO_ERR) {
        /* check if any explicitly listed modules should be loaded */
        modval = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_MODULE);
        while (modval != NULL && res == NO_ERR) {
            log_info("\nyangcli: Loading requested module %s", 
                     VAL_STR(modval));

            revision = NULL;
            savestr = NULL;
            modlen = 0;

            if (yang_split_filename(VAL_STR(modval), &modlen)) {
                savestr = &(VAL_STR(modval)[modlen]);
                *savestr = '\0';
                revision = savestr + 1;
            }

            res = ncxmod_load_module(VAL_STR(modval), revision,
                                     &savedevQ, NULL);
            if (res != NO_ERR) {
                log_error("\n load module failed (%s)", 
                          get_error_string(res));
            } else {
                log_info("\n load OK");
            }

            modval = val_find_next_child(mgr_cli_valset, YANGCLI_MOD,
                                         NCX_EL_MODULE, modval);
        }
    }

    /* discard any deviations loaded from the CLI or conf file */
    ncx_clean_save_deviationsQ(&savedevQ);
    if (res != NO_ERR) {
        log_error("\n ncx_clean_save_deviationsQ failed (%s)", get_error_string(res));
        return res;
    }

    /* load the system (read-only) variables */
    res = init_system_vars(server_cb);
    if (res != NO_ERR) {
        log_error("\n init_system_vars failed (%s)", get_error_string(res));
        return res;
    }

    /* load the system config variables */
    res = init_config_vars(server_cb);
    if (res != NO_ERR) {
        log_error("\n init_config_vars failed (%s)", get_error_string(res));
        return res;
    }

    /* initialize the module library search result queue */
    {
        log_debug_t dbglevel = log_get_debug_level();
        if (LOGDEBUG3) {
            ; 
        } else {
            log_set_debug_level(LOG_DEBUG_NONE);
        }
        res = ncxmod_find_all_modules(&modlibQ);
        log_set_debug_level(dbglevel);
        if (res != NO_ERR) {
            log_error("\n ncxmod_find_all_modules failed (%s)", get_error_string(res));
            return res;
        }
    }
#if 0
    /* load the user aliases */
    if (autoaliases) {
        res = load_aliases(aliases_file);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* load the user variables */
    if (autouservars) {
        res = load_uservars(server_cb, uservars_file);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* make sure the startup screen is generated
     * before the auto-connect sequence starts
     */
    do_startup_screen();    
#endif

    /* check to see if a session should be auto-started
     * --> if the server parameter is set a connect will
     * --> be attempted
     *
     * The yangcli_stdin_handler will call the finish_start_session
     * function when the user enters a line of keyboard text
     */
    server_cb->state = MGR_IO_ST_IDLE;

    if (connect_valset) {
        parm = val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_SERVER);
        if (parm && parm->res == NO_ERR) {
            res = do_connect_(server_cb, NULL, NULL, 0, TRUE);
            if (res != NO_ERR) {
                if (FALSE /*!batchmode*/) {
                    res = NO_ERR;
                }
            }
        }
    }

    /* the request will be stored if this returns NO_ERR */
    //res = mgr_rpc_send_request(scb, req, yangcli_reply_handler_);
    //assert(res==NO_ERR);
    //mgr_io_run();

    {
        ses_cb_t* scb;
        mgr_scb_t* mscb;

        scb = mgr_ses_get_scb(server_cb->mysid);
        assert(scb!=NULL);

        res = ses_msg_send_buffs(scb);
        assert(res==NO_ERR);
        while(1) {
            res = ses_accept_input(scb);
            if(res!=NO_ERR) {
                log_error("\n ses_accept failed (%s)", get_error_string(res));
                return res;
            }
            if(mgr_ses_process_first_ready()) {
                break;
            }
        }
        /* incoming hello OK and outgoing hello is sent */
        server_cb->state = MGR_IO_ST_CONN_IDLE;
        mscb = (mgr_scb_t *)scb->mgrcb;
        ncx_set_temp_modQ(&mscb->temp_modQ);
        report_capabilities(server_cb, scb, TRUE, HELP_MODE_NONE);
        check_module_capabilities(server_cb, scb, autoload_blocking_get_modules, yang_library_blocking_get_module_set);

    }

    *yangrpc_cb_ptr = (yangrpc_cb_ptr_t)server_cb;
    return NO_ERR;
}

val_value_t* global_reply_val;

/********************************************************************
 * FUNCTION yangcli_reply_handler_
 * 
 *  handle incoming <rpc-reply> messages
 * 
 * INPUTS:
 *   scb == session receiving RPC reply
 *   req == original request returned for freeing or reusing
 *   rpy == reply received from the server (for checking then freeing)
 *
 * RETURNS:
 *   none
 *********************************************************************/
void
    yangcli_reply_handler_ (ses_cb_t *scb,
                           mgr_rpc_req_t *req,
                           mgr_rpc_rpy_t *rpy)
{
    server_cb_t   *server_cb;
    val_value_t  *val;
    mgr_scb_t    *mgrcb;
    lock_cb_t    *lockcb;
    rpc_err_t     rpcerrtyp;
    status_t      res;
    boolean       anyout, anyerrors, done;
    uint32        usesid;

#ifdef DEBUG
    if (!scb || !req || !rpy) {
        assert(0);
    }
#endif


    /* check the contents of the reply */
    if (rpy && rpy->reply) {
#if 0
        if (val_find_child(rpy->reply, 
                           NC_MODULE,
                           NCX_EL_RPC_ERROR)) {
            if (server_cb->command_mode == CMD_MODE_NORMAL || LOGDEBUG) {
                log_error("\nRPC Error Reply %s for session %u:\n",
                          rpy->msg_id, 
                          usesid);
                val_dump_value_max(rpy->reply, 
                                   0,
                                   server_cb->defindent,
                                   DUMP_VAL_LOG,
                                   server_cb->display_mode,
                                   FALSE,
                                   FALSE);
                log_error("\n");
                anyout = TRUE;
            }
        } else if (val_find_child(rpy->reply, NC_MODULE, NCX_EL_OK)) {
            global_reply_val = val_clone(rpy->reply);
            if (server_cb->command_mode == CMD_MODE_NORMAL || LOGDEBUG2) {
                if (server_cb->echo_replies) {
                    log_info("\nRPC OK Reply %s for session %u:\n",
                             rpy->msg_id, 
                             usesid);
                    anyout = TRUE;
                }
            }
        }
#else
        //val_dump_value(rpy->reply,0);
        global_reply_val = val_clone(rpy->reply);
        assert(global_reply_val!=NULL);
#endif
    }

    /* free the request and reply */
    mgr_rpc_free_request(req);
    if (rpy) {
        mgr_rpc_free_reply(rpy);
    }

}  /* yangcli_reply_handler_ */

#include "yangcli_cmd.h"
status_t yangrpc_parse_cli(yangrpc_cb_ptr_t yangrpc_cb_ptr,
                           const char * const original_line,
                           val_value_t** request_val)
{


    obj_template_t        *rpc, *input;
    val_value_t           *reqdata = NULL, *valset = NULL, *parm;
    xmlChar               *newline = NULL, *useline = NULL;
    uint32                 len, linelen;
    status_t               res = NO_ERR;
    boolean                shut = FALSE;
    ncx_node_t             dtyp;
    char* line;
    ses_cb_t* scb;
    mgr_scb_t* mscb;

    server_cb_t* server_cb;
    server_cb = (server_cb_t*)yangrpc_cb_ptr;

    scb = mgr_ses_get_scb(server_cb->mysid);
    if (!scb) {
        res = SET_ERROR(ERR_INTERNAL_PTR);
        return res;
    }
    mscb = (mgr_scb_t *)scb->mgrcb;
    ncx_set_temp_modQ(&mscb->temp_modQ);
    ncx_set_session_modQ(&mscb->temp_modQ);


    line = strdup(original_line);
#ifdef DEBUG
    if (!server_cb || !line) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* make sure there is something to parse */
    linelen = xml_strlen(line);
    if (!linelen) {
        return res;
    }

#if 0
    /* first check the command keyword to see if it is an alias */
    newline = expand_alias(line, &res);
    if (res == ERR_NCX_SKIPPED) {
        res = NO_ERR;
        useline = line;
    } else if (res == NO_ERR) {
        if (newline == NULL) {
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        useline = newline;
        linelen = xml_strlen(newline);
    } else {
        log_error("\nError: %s\n", get_error_string(res));
        if (newline) {
            m__free(newline);
        }
        return res;
    }
#else
    useline = line;
#endif
    /* get the RPC method template */
    dtyp = NCX_NT_OBJ;
    rpc = (obj_template_t *)parse_def(server_cb, &dtyp, useline, &len, &res);
    if (rpc == NULL || !obj_is_rpc(rpc)) {
        if (server_cb->result_name || server_cb->result_filename) {
            res = finish_result_assign(server_cb, NULL, useline);
        } else {
            if (res == ERR_NCX_DEF_NOT_FOUND) {
                /* this is an unknown command */
                log_error("\nError: Unrecognized command");
            } else if (res == ERR_NCX_AMBIGUOUS_CMD) {
                log_error("\n");
            } else {
                log_error("\nError: %s", get_error_string(res));
            }
        }
        if (newline) {
            m__free(newline);
        }
        return res;
    }

    /* check local commands */
    if (is_yangcli_ns(obj_get_nsid(rpc))) {
        if (!xml_strcmp(obj_get_name(rpc), YANGCLI_CONNECT)) {
            res = ERR_NCX_OPERATION_FAILED;
            log_stdout("\nError: Already connected");
        } else {
            uint32 timeval;
            res = do_local_conn_command_reqdata(server_cb, rpc, useline, len, &reqdata, &timeval);
            if (res == ERR_NCX_SKIPPED) {
                assert(0);
		/*res = do_local_command(server_cb, rpc, useline, len);*/
            }
        }
        if (newline) {
            m__free(newline);
        }
    } else {

        /* else treat this as an RPC request going to the server
         * make sure this is a TRUE conditional command
         */

        /* construct a method + parameter tree */
        reqdata = xml_val_new_struct(obj_get_name(rpc), obj_get_nsid(rpc));
        if (!reqdata) {
            log_error("\nError allocating a new RPC request");
            res = ERR_INTERNAL_MEM;
            input = NULL;
        } else {
            /* should find an input node */
            input = obj_find_child(rpc, NULL, YANG_K_INPUT);
        }

        /* check if any params are expected */
        if (res == NO_ERR && input) {
            while (useline[len] && xml_isspace(useline[len])) {
                len++;
            }

            if (len < linelen) {
                valset = parse_rpc_cli(server_cb, rpc, &useline[len], &res);
                if (res != NO_ERR) {
                    log_error("\nError in the parameters for '%s' command (%s)",
                            obj_get_name(rpc), get_error_string(res));
                }
            }

            /* check no input from user, so start a parmset */
            if (res == NO_ERR && !valset) {
                valset = val_new_value();
                if (!valset) {
                    res = ERR_INTERNAL_MEM;
                } else {
                    val_init_from_template(valset, input);
                }
            }
        }

#if 0
        /* fill in any missing parameters from the CLI */
        if (res == NO_ERR) {
            if (interactive_mode()) {
                res = fill_valset(server_cb, rpc, valset, NULL, TRUE, FALSE);
                if (res == ERR_NCX_SKIPPED) {
                    res = NO_ERR;
                }
            }
        }
#endif

        /* make sure the values are in canonical order
         * so compliant some servers will not complain
         */
        val_set_canonical_order(valset);

        /* go through the parm list and move the values 
         * to the reqdata struct. 
         */
        if (res == NO_ERR) {
            parm = val_get_first_child(valset);
            while (parm) {
                val_remove_child(parm);
                val_add_child(parm, reqdata);
                parm = val_get_first_child(valset);
            }
        }
    }
    *request_val = reqdata;
    free(line);
    return res;
}

status_t yangrpc_exec(yangrpc_cb_ptr_t yangrpc_cb_ptr, val_value_t* request_val, val_value_t** reply_val)
{
    status_t res;
    ses_cb_t* scb;
    mgr_scb_t* mscb;
    mgr_rpc_req_t         *req;
    server_cb_t* server_cb;
    server_cb = (server_cb_t*)yangrpc_cb_ptr;

    scb = mgr_ses_get_scb(server_cb->mysid);
    if (!scb) {
        res = SET_ERROR(ERR_INTERNAL_PTR);
        return res;
    }
    mscb = (mgr_scb_t *)scb->mgrcb;
    ncx_set_temp_modQ(&mscb->temp_modQ);
    ncx_set_session_modQ(&mscb->temp_modQ);

    req = mgr_rpc_new_request(scb);
    if (!req) {
        res = ERR_INTERNAL_MEM;
        log_error("\nError allocating a new RPC request");
        return res;
    }
    req->data = val_clone(request_val);/*reqdata*/
    req->rpc = request_val->obj;
    req->timeout = 1000/*timeoutval*/;

    /* the request will be stored if this returns NO_ERR */
    global_reply_val=NULL;
    res = mgr_rpc_send_request(scb, req, yangcli_reply_handler_);

    //mgr_io_run();
    res = ses_msg_send_buffs(scb);
    assert(res==NO_ERR);
    while(1) {
    	
        res = ses_accept_input(scb);
        if(res!=NO_ERR) {
            log_error("\nerror: ses_accept_input res=%d",res);
            assert(0);
        }
        if(mgr_ses_process_first_ready() && global_reply_val!=NULL) {
            break;
        }
    }
    *reply_val = global_reply_val;

    return NO_ERR;
}

void yangrpc_close(yangrpc_cb_ptr_t yangrpc_cb_ptr)
{
    log_info("Closing session\n");
}
