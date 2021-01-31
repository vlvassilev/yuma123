/*
 * Copyright (c) 2008 - 2012, Andy Bierman, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: yangcli_autoload.c

   NETCONF YANG-based CLI Tool

   autoload support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
13-aug-09    abb      begun; started from yangcli.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
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
#include <assert.h>
#include "libtecla.h"

#include "procdefs.h"
#include "log.h"
#include "mgr.h"
#include "mgr_ses.h"
#include "ncx.h"
#include "ncx_feature.h"
#include "ncx_list.h"
#include "ncxconst.h"
#include "ncxmod.h"
#include "obj.h"
#include "op.h"
#include "rpc.h"
#include "rpc_err.h"
#include "status.h"
#include "val_util.h"
#include "var.h"
#include "xmlns.h"
#include "xml_util.h"
#include "xml_val.h"
#include "yangconst.h"
#include "yangcli.h"
#include "yangcli_autoload.h"
#include "yangcli_cmd.h"
#include "yangcli_util.h"
#include "yangcli_yang_library.h"

#ifdef DEBUG
#define YANGCLI_AUTOLOAD_DEBUG 1
#endif

/********************************************************************
* FUNCTION make_get_schema_reqdata
* 
* Allocate and initialize reqdata value for <get-schema>
*
* format will be hard-wired to yang
*
* INPUTS:
*   server_cb == server control block to use
*   scb == session control block to use
*   module == module to get
*   revision == revision to get
*
* OUTPUTS:
*    out_rpc == obj_template_t** of the get-schema RPC
*    out_reqdata == val_value_t** of the get-schema data value
*
* RETURNS:
*    status
*********************************************************************/
status_t make_get_schema_reqdata(server_cb_t *server_cb,
                              ses_cb_t *scb,
                              const xmlChar *module,
                              const xmlChar *revision,
                              obj_template_t** out_rpc,
                              val_value_t** out_reqdata)
{
    ncx_module_t          *mod;
    obj_template_t        *rpc, *input, *parmobj;
    val_value_t           *reqdata, *parmval;
    status_t               res;
    xmlns_id_t             nsid;

    reqdata = NULL;
    res = NO_ERR;
    input = NULL;
    nsid = 0;
    mod = ncx_find_module(NCXMOD_IETF_NETCONF_STATE, NULL);
    if (mod == NULL) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* get the <get-schema> input template */
    rpc = ncx_find_rpc(mod,  NCX_EL_GET_SCHEMA);
    assert(rpc);
    nsid = obj_get_nsid(rpc);
    input = obj_find_child(rpc, NULL, YANG_K_INPUT);
    assert(input);

    /* construct a method + parameter tree */
    reqdata = val_new_value();
    assert(reqdata);
    val_init_from_template(reqdata, rpc);

    /* add /get-schema/input/identifier */
    parmobj = obj_find_child(input, NCXMOD_IETF_NETCONF_STATE, NCX_EL_IDENTIFIER);
    assert(parmobj);
    parmval = val_make_simval_obj(parmobj,
                                  module,
                                  &res);

    assert(parmval);
    val_add_child(parmval, reqdata);

    /* add /get-schema/input/version */
    parmobj = obj_find_child(input, NCXMOD_IETF_NETCONF_STATE, NCX_EL_VERSION);
    assert(parmobj);
    parmval = val_make_simval_obj(parmobj,
                                  (revision) ? revision : EMPTY_STRING,
                                  &res);

    assert(parmval);
    val_add_child(parmval, reqdata);

    /* add /get-schema/input/format */
    parmobj = obj_find_child(input, NCXMOD_IETF_NETCONF_STATE, NCX_EL_FORMAT);
    assert(parmobj);
    parmval = val_make_simval_obj(parmobj,
                                  "yang",
                                  &res);

    assert(parmval);
    val_add_child(parmval, reqdata);

    /* check any errors so far */
    if (res != NO_ERR) {
        val_free_value(reqdata);
        return res;
    }

    *out_rpc=rpc;
    *out_reqdata=reqdata;

    return res;

} /* make_get_schema_reqdata */

/********************************************************************
* FUNCTION send_get_schema_to_server
* 
* Send an <get-schema> operation to the specified server
* in MGR_IO_ST_AUTOLOAD state
*
* format will be hard-wired to yang
*
* INPUT:
*   server_cb == server control block to use
*   scb == session control block to use
*   module == module to get
*   revision == revision to get
*
* OUTPUTS:
*    server_cb->state may be changed or other action taken
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_get_schema_to_server (server_cb_t *server_cb,
                              ses_cb_t *scb,
                              const xmlChar *module,
                              const xmlChar *revision)
{
    status_t              res;
    obj_template_t*       rpc;
    val_value_t*          reqdata;
    mgr_rpc_req_t         *req;

    req = NULL;

    res = make_get_schema_reqdata(server_cb, scb, module, revision, &rpc, &reqdata);
    if(res!=NO_ERR) {
        return NO_ERR;
    }

    /* allocate an RPC request and send it */
    req = mgr_rpc_new_request(scb);
    if (!req) {
        res = ERR_INTERNAL_MEM;
        log_error("\nError allocating a new RPC request");
    } else {
        req->data = reqdata;
        req->rpc = rpc;
        req->timeout = server_cb->timeout;
    }
        
    if (res == NO_ERR) {
        if (LOGDEBUG) {
            log_debug("\nSending autoload request for '%s', r'%s'",
                      module,
                      (revision) ? revision : EMPTY_STRING);
        } 
        if (LOGDEBUG2) {
            log_debug2("\nabout to send RPC request with reqdata:");
            val_dump_value_max(reqdata, 
                               0,
                               server_cb->defindent,
                               DUMP_VAL_LOG,
                               server_cb->display_mode,
                               FALSE,
                               FALSE);
        }

        /* the request will be stored if this returns NO_ERR */
        res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
    }

    if (res != NO_ERR) {
        if (req) {
            mgr_rpc_free_request(req);
        } else if (reqdata) {
            val_free_value(reqdata);
        }
    } else {
        server_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_get_schema_to_server */

/********************************************************************
* FUNCTION save_schema_file
* 
* Save the <data> node in the <rpc-reply> as the
* specified YANG file in the session work directory
*
* INPUTS:
*    server_cb == server control block to use
*    module == module name
*    revision == revision date
*    targetfile == filespec of the output file
*    resultval == result to output to file
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    save_schema_file (server_cb_t *server_cb,
                      const xmlChar *module,
                      const xmlChar *revision,
                      const xmlChar *targetfile,
                      val_value_t *resultval)
{
    status_t     res;
    struct stat  statbuf;
    int          statresult;

    res = NO_ERR;

    if (LOGDEBUG) {
        log_debug("\nGot autoload reply for '%s' r'%s'",
                  module,
                  (revision) ? revision : EMPTY_STRING);
    }
    if (LOGDEBUG2) {
        log_debug2("\n*** output <get-schema> result "
                   "\n   module '%s'"
                   "\n   revision '%'s"
                   "\n   target '%s'",
                   module,
                   (revision) ? revision : EMPTY_STRING,
                   targetfile);
    }

    /* see if file already exists */
    statresult = stat((const char *)targetfile, &statbuf);
    if (statresult == 0) {
        log_error("\nError: temporary file '%s' already exists",
                  targetfile);
        return ERR_NCX_DATA_EXISTS;
    }
    
    /* output in text format to the specified file */
    res = log_alt_open((const char *)targetfile);
    if (res != NO_ERR) {
        log_error("\nError: temporary file '%s' could "
                  "not be opened (%s)",
                  targetfile,
                  get_error_string(res));
    } else {
        /* do not use session display mode and other parameters
         * when saving a schema file; save as-is
         */
        val_dump_alt_value(resultval, 0);
        log_alt_close();

        /* copy the target filename into the search result */
        server_cb->cursearchresult->source = xml_strdup(targetfile);
        if (server_cb->cursearchresult->source == NULL) {
            log_error("\nError: malloc failed for temporary file '%s'",
                      targetfile);
            return ERR_INTERNAL_MEM;
        }
    }

    return res;

}  /* save_schema_file */


/********************************************************************
* FUNCTION reset_feature
* 
* Go through the feature list and see if the specified
* feature should be enabled or not
*
* INPUTS:
*    mod == module containing this feature
*    feature == feature found
*    cookie == cookie passed in (feature_list)
*
* RETURNS:
*    TRUE if processing should continue, FALSE if done
*********************************************************************/
static boolean
    reset_feature (const ncx_module_t *mod,
                   ncx_feature_t *feature,
                   void *cookie)
{
    const ncx_list_t *feature_list;

    (void)mod;
    feature_list = (const ncx_list_t *)cookie;

    feature->enabled = 
        (ncx_string_in_list(feature->name, feature_list)) ?
        TRUE : FALSE;

    return TRUE;

}  /* reset_feature */

static ncxmod_temp_filcb_t *
    get_new_temp_filcb (mgr_scb_t *mscb,
                        const xmlChar *module,
                        const xmlChar *revision,
                        boolean isyang,
                        status_t *res)
{
    xmlChar             *filebuffer, *p;
    ncxmod_temp_filcb_t *temp_filcb;
    uint32               len_needed;

    *res = NO_ERR;

    /* figure out how big the filename will be */
    len_needed = xml_strlen(module);
    if (revision) {
        len_needed += (xml_strlen(revision) + 1);
    }
    if (isyang) {
        len_needed += 5;   /* .yang */
    } else {
        len_needed += 4;   /* .yin */
    }

    filebuffer = m__getMem(len_needed+1);
    if (filebuffer == NULL) {
        *res = ERR_INTERNAL_MEM;
        return NULL;
    }

    /* construct a file name for the target file */
    p = filebuffer;
    p += xml_strcpy(p, module);
    if (revision && *revision!=0) {
        *p++ = '@';
        p += xml_strcpy(p, revision);
    }
    if (isyang) {
        p += xml_strcpy(p, (const xmlChar *)".yang");
    } else {
        p += xml_strcpy(p, (const xmlChar *)".yin");
    }

    /* get a temp file control block
     * it will be stored in the session control block
     * so it is not a live malloced pointer
     */
    temp_filcb = ncxmod_new_session_tempfile(mscb->temp_sescb,
                                             filebuffer,
                                             res);
    m__free(filebuffer);
    return temp_filcb;

}   /* get_new_temp_filcb */

status_t get_schema_reply_to_temp_filcb(server_cb_t * server_cb, mgr_scb_t *mscb, const xmlChar* module, const xmlChar* revision, val_value_t* reply)
{
    ncxmod_search_result_t  *searchresult;
    ncxmod_temp_filcb_t     *temp_filcb;
    val_value_t             *dataval;
    status_t                 res;

    /* get the data node out of the reply;
     * it contains the requested YANG module
     * in raw text form
     */
    dataval = val_find_child(reply, NULL, NCX_EL_DATA);
    if (dataval == NULL) {
        res = SET_ERROR(ERR_NCX_DATA_MISSING);
    } else {
        /* get a file handle in the temp session
         * directory
         */
        temp_filcb = get_new_temp_filcb(mscb,
                                        module,
                                        revision,
                                        TRUE,   /* isyang */
                                        &res);
        if (temp_filcb != NULL) {
           /* copy the value node to the work directory
            * as a YANG file
            */
            res = save_schema_file(server_cb,
                                   module,
                                   revision,
                                   temp_filcb->source,
                                   dataval);
        }
    }

    return res;
}

/********************************************************************
 * FUNCTION copy_module_to_tempdir
 * 
 * Copy the YANG source file to the session work directory
 *
 * INPUTS:
 *   mscb == manager session control block to use
 *   module == module name to copy
 *   revision == revision date to copy
 *   source == complete pathspec of source YANG file
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    copy_module_to_tempdir (mgr_scb_t *mscb,
                            const xmlChar *module,
                            const xmlChar *revision,
                            const xmlChar *source)
{
    xmlChar             *linebuffer;
    FILE                *srcfile, *destfile;
    ncxmod_temp_filcb_t *temp_filcb;
    boolean              done, isyang;
    status_t             res;

    res = NO_ERR;
    linebuffer = NULL;
    srcfile = NULL;
    destfile = NULL;

    if (yang_fileext_is_yang(source)) {
        isyang = TRUE;
    } else if (yang_fileext_is_yin(source)) {
        isyang = FALSE;
    } else {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    temp_filcb = get_new_temp_filcb(mscb, module, revision, isyang, &res);
    if (temp_filcb == NULL) {
        return res;
    }

    /* get a buffer for transferring lines */
    linebuffer = m__getMem(NCX_MAX_LINELEN+1);;
    if (linebuffer == NULL) {
        ncxmod_free_session_tempfile(temp_filcb);
        return ERR_INTERNAL_MEM;
    }

#ifdef YANGCLI_AUTOLOAD_DEBUG
    if (LOGDEBUG2) {
        log_debug2("\nyangcli_autoload: Copying '%s' to '%s'",
                   source,
                   temp_filcb->source);
    }
#endif

    /* open the destination file for writing */
    destfile = fopen((const char *)temp_filcb->source, "w+");
    if (destfile == NULL) {
        res = errno_to_status();
        ncxmod_free_session_tempfile(temp_filcb);
        m__free(linebuffer);
        return res;
    }

    /* open the YANG or YIN source file for reading */
    srcfile = fopen((const char *)source, "r");
    if (srcfile == NULL) {
        res = errno_to_status();
        fclose(destfile);
        ncxmod_free_session_tempfile(temp_filcb);
        m__free(linebuffer);
        return res;
    }

    done = FALSE;
    while (!done) {
        if (!fgets((char *)linebuffer, NCX_MAX_LINELEN, srcfile)) {
            /* read line failed, not an error */
            done = TRUE;
            continue;
        }

        if (fputs((const char *)linebuffer, destfile) == EOF) {
            log_error("\nError: copy to temp file failed");
            /*** keeping partial file around!!! ***/
            done = TRUE;
            res = ERR_FIL_WRITE;
        }
    }

    fclose(srcfile);
    fclose(destfile);
    m__free(linebuffer);

    return res;

}   /* copy_module_to_tempdir */


/********************************************************************
* FUNCTION set_temp_ync_features
* 
* Set the features list for the new session
* based on the capabilities reported by the server
*
* INPUTS:
*    mscb == manager session control block to use
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    set_temp_ync_features (mgr_scb_t *mscb)
{
    status_t    res;

    if (cap_std_set(&mscb->caplist, CAP_STDID_WRITE_RUNNING)) {
        res = ncx_set_list(NCX_BT_STRING,
                           NCX_EL_WRITABLE_RUNNING,
                           &mscb->temp_ync_features);
        if (res != NO_ERR) {
            return res;
        }
    }

    if (cap_std_set(&mscb->caplist, CAP_STDID_CANDIDATE)) {
        res = ncx_set_list(NCX_BT_STRING,
                           NCX_EL_CANDIDATE,
                           &mscb->temp_ync_features);
        if (res != NO_ERR) {
            return res;
        }
    }

    if (cap_std_set(&mscb->caplist, CAP_STDID_CONF_COMMIT)) {
        res = ncx_set_list(NCX_BT_STRING,
                           NCX_EL_CONFIRMED_COMMIT,
                           &mscb->temp_ync_features);
        if (res != NO_ERR) {
            return res;
        }
    }

    if (cap_std_set(&mscb->caplist, CAP_STDID_ROLLBACK_ERR)) {
        res = ncx_set_list(NCX_BT_STRING,
                           NCX_EL_ROLLBACK_ON_ERROR,
                           &mscb->temp_ync_features);
        if (res != NO_ERR) {
            return res;
        }
    }

    if (cap_std_set(&mscb->caplist, CAP_STDID_VALIDATE)) {
        res = ncx_set_list(NCX_BT_STRING,
                           NCX_EL_VALIDATE,
                           &mscb->temp_ync_features);
        if (res != NO_ERR) {
            return res;
        }
    }

    if (cap_std_set(&mscb->caplist, CAP_STDID_STARTUP)) {
        res = ncx_set_list(NCX_BT_STRING,
                           NCX_EL_STARTUP,
                           &mscb->temp_ync_features);
        if (res != NO_ERR) {
            return res;
        }
    }

    if (cap_std_set(&mscb->caplist, CAP_STDID_URL)) {
        res = ncx_set_list(NCX_BT_STRING,
                           NCX_EL_URL,
                           &mscb->temp_ync_features);
        if (res != NO_ERR) {
            return res;
        }
    }

    if (cap_std_set(&mscb->caplist, CAP_STDID_XPATH)) {
        res = ncx_set_list(NCX_BT_STRING,
                           NCX_EL_XPATH,
                           &mscb->temp_ync_features);
        if (res != NO_ERR) {
            return res;
        }
    }

    return NO_ERR;
    
}  /* set_temp_ync_features */


/********************************************************************
* FUNCTION autoload_module
* 
* auto-load the specified module
*
* INPUTS:
*   modname == module name
*   revision == module revision
*   devlist  == deviation list
*   retmod == address of return module
*
* OUTPUTS:
*   *retmod == loaded module (if NO_ERR)
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    autoload_module (const xmlChar *modname,
                     const xmlChar *revision,
                     ncx_list_t *devlist,
                     dlq_hdr_t *savedevQ_ptr,
                     ncx_module_t **retmod)
{
    ncx_lmem_t             *listmember;
    status_t                res;
    log_debug_t             loglevel;

    if (LOGDEBUG2) {
        log_debug2("\nStarting autoload for module '%s', "
                   "revision '%s'",
                   modname,
                   (revision) ? revision : EMPTY_STRING);
    }

    res = NO_ERR;

    /* first load any deviations */
    if (devlist != NULL) {
        for (listmember = ncx_first_lmem(devlist);
             listmember != NULL && res == NO_ERR;
             listmember = (ncx_lmem_t *)dlq_nextEntry(listmember)) {

            res = ncxmod_load_deviation(listmember->val.str,
                                        savedevQ_ptr);
            if (res != NO_ERR) {
                log_error("\nError: Deviation module %s not loaded (%s)!!",
                          listmember->val.str, 
                          get_error_string(res));
            }
        }
    }

    /* parse the requested module now 
     * retmod is a live pointer
     */
    if (res == NO_ERR) {
        if (LOGDEBUG) {
            res = ncxmod_parse_module(modname, revision, savedevQ_ptr, retmod);
        } else {
            /* ignore parse warnings during autoload unless debug mode */
            loglevel = log_get_debug_level();
            log_set_debug_level(LOG_DEBUG_ERROR);
            res = ncxmod_parse_module(modname, revision, savedevQ_ptr, retmod);
            log_set_debug_level(loglevel);
        }
        if (res != NO_ERR) {
            log_error("\nError: Auto-load for module '%s' failed (%s)",
                      modname, 
                      get_error_string(res));
        } else if (retmod != NULL && *retmod != NULL) {
            (*retmod)->defaultrev = TRUE;
        }
    }

    return res;

}  /* autoload_module */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION autoload_setup_tempdir
* 
* copy all the advertised YANG modules into the
* session temp files directory
*
* DOES NOT COPY FILES UNLESS THE 'source' FIELD IS SET
* IN THE SEARCH RESULT (ONLY LOCAL FILES COPIED)
*
* INPUTS:
*   server_cb == server session control block to use
*   scb == current session in progress
*
* OUTPUTS:
*   $HOME/.yuma/tmp/<progdir>/<sesdir>/ filled with
*   the the specified YANG files that are already available
*   on this system.
*   
*   These search records will be removed from the 
*   server_cb->searchresultQ and modptr records 
*   added to the server_cb->modptrQ
*
* RETURNS:
*    status
*********************************************************************/
status_t
    autoload_setup_tempdir (server_cb_t *server_cb,
                            ses_cb_t *scb)
{
    mgr_scb_t               *mscb;
    ncxmod_search_result_t  *searchresult;
    ncx_module_t            *testmod;
    status_t                 res, retres;
    boolean                  need_yt, need_ncx, need_nacm, need_ync;
    dlq_hdr_t                savedevQ;

#ifdef DEBUG
    if (!server_cb || !scb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;
    mscb = (mgr_scb_t *)scb->mgrcb;

    need_yt = TRUE;
    need_ncx = TRUE;
    need_nacm = TRUE;
    need_ync = TRUE;

    /* try to copy as many files as possible, even if some errors */
    for (searchresult = (ncxmod_search_result_t *)
             dlq_firstEntry(&server_cb->searchresultQ);
         searchresult != NULL;
         searchresult = (ncxmod_search_result_t *)
             dlq_nextEntry(searchresult)) {

        /* skip bad entries and not-found entries */
        if (searchresult->module == NULL ||
            searchresult->source == NULL ||
            searchresult->res != NO_ERR) {
            continue;
        }

        /* check if the URI was not the same, even though
         * the module was found, so the source was set
         */
        if (!searchresult->capmatch) {
            m__free(searchresult->source);
            searchresult->source = NULL;
            continue;
        }

        /* check yuma-netconf hack; remove this code
         * when ietf-netconf is supported
         */
        if (!xml_strcmp(searchresult->module, 
                        NCXMOD_NCX)) {
            need_ncx = FALSE;
        } else if (!xml_strcmp(searchresult->module, 
                               NCXMOD_IETF_YANG_TYPES)) {
            need_yt = FALSE;
        } else if (!xml_strcmp(searchresult->module, 
                               NCXMOD_IETF_NETCONF_ACM)) {
            need_nacm = FALSE;
        } else if (!xml_strcmp(searchresult->module, 
                               NCXMOD_YUMA_NETCONF)) {
            need_ync = FALSE;
        }

        /* copy this local module to the work dir so
         * it will be found in an import, even if the
         * revision-date is missing from the import
         */
        res = copy_module_to_tempdir(mscb,
                                     searchresult->module,
                                     searchresult->revision,
                                     searchresult->source);
        if (res != NO_ERR) {
            searchresult->res = res;
            retres = res;
        }
    }

    if (retres == NO_ERR) {
        dlq_createSQue(&savedevQ);

        if (need_yt) {
            res = ncxmod_load_module(NCXMOD_IETF_YANG_TYPES, NULL, &savedevQ,
                                     &testmod);
            if (res == NO_ERR) {
                res = copy_module_to_tempdir(mscb,
                                             testmod->name,
                                             testmod->version,
                                             testmod->source);
            } else {
                SET_ERROR(res);
            }
        }
        if (need_ncx) {
            res = ncxmod_load_module(NCXMOD_NCX, NULL, &savedevQ, &testmod);
            if (res == NO_ERR) {
                res = copy_module_to_tempdir(mscb,
                                             testmod->name,
                                             testmod->version,
                                             testmod->source);
            } else {
                SET_ERROR(res);
            }
        }
        if (need_nacm) {
            res = ncxmod_load_module(NCXMOD_IETF_NETCONF_ACM, NULL, &savedevQ,
                                     &testmod);
            if (res == NO_ERR) {
                res = copy_module_to_tempdir(mscb,
                                             testmod->name,
                                             testmod->version,
                                             testmod->source);
            } else {
                SET_ERROR(res);
            }
        }
        if (need_ync) {
            res = ncxmod_load_module(NCXMOD_YUMA_NETCONF, NULL, &savedevQ, &testmod);
            if (res == NO_ERR) {
                res = copy_module_to_tempdir(mscb,
                                             testmod->name,
                                             testmod->version,
                                             testmod->source);
            } else {
                SET_ERROR(res);
            }
        }

        ncx_clean_save_deviationsQ(&savedevQ);
    }
    return retres;

}  /* autoload_setup_tempdir */


/********************************************************************
* FUNCTION autoload_start_get_modules
* 
* Start the MGR_SES_IO_CONN_AUTOLOAD state
*
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
    autoload_start_get_modules (server_cb_t *server_cb,
                                ses_cb_t *scb)
{
    ncxmod_search_result_t  *searchresult;
    status_t                 res;
    boolean                  done;

#ifdef DEBUG
    if (!server_cb || !scb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    done = FALSE;
    res = NO_ERR;

    /* find first file that needs to be retrieved with get-schema */
    for (searchresult = (ncxmod_search_result_t *)
             dlq_firstEntry(&server_cb->searchresultQ);
         searchresult != NULL && !done;
         searchresult = (ncxmod_search_result_t *)
             dlq_nextEntry(searchresult)) {

        /* skip found entries */
        if (searchresult->source != NULL) {
            continue;
        }

        /* skip found modules with errors */          
        if (searchresult->res != NO_ERR &&
	    !(searchresult->res == ERR_NCX_WRONG_VERSION ||
              searchresult->res == ERR_NCX_MOD_NOT_FOUND)) {
            continue;
        }

        /* found an entry that needs to be retrieved
         * either module not found or wrong version found
         */
        done = TRUE;

        res = send_get_schema_to_server(server_cb,
                                       scb,
                                       searchresult->module,
                                       searchresult->revision);
        if (res == NO_ERR) {
            server_cb->command_mode = CMD_MODE_AUTOLOAD;
            server_cb->cursearchresult = searchresult;
        }
        /* exit loop if we get here */
    }

    return res;

}  /* autoload_start_get_modules */


/********************************************************************
* FUNCTION autoload_handle_rpc_reply
* 
* Handle the current <get-schema> response
*
* INPUTS:
*   server_cb == server session control block to use
*   scb == session control block to use
*   reply == data node from the <rpc-reply> PDU
*   anyerrors == TRUE if <rpc-error> detected instead
*                of <data>
*             == FALSE if no <rpc-error> elements detected
*
* OUTPUTS:
*   $HOME/.yuma/tmp/<progdir>/<sesdir>/ filled with
*   the the specified YANG files that was retrieved from
*   the device with <get-schema>
*
*   Next request is started, or autoload process is completed
*   and the command_mode is changed back to CMD_MODE_NORMAL
*
* RETURNS:
*    status
*********************************************************************/
status_t
    autoload_handle_rpc_reply (server_cb_t *server_cb,
                               ses_cb_t *scb,
                               val_value_t *reply,
                               boolean anyerrors)
{
    mgr_scb_t               *mscb;
    ncxmod_search_result_t  *searchresult;
    const xmlChar           *module, *revision;
    status_t                 res;
    boolean                  done;

#ifdef DEBUG
    if (!server_cb || !scb || !reply) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    done = FALSE;
    res = NO_ERR;
    mscb = (mgr_scb_t *)scb->mgrcb;
    searchresult = server_cb->cursearchresult;

    if(searchresult==NULL) {
        /* check if this is yang-library <get> for /modules-state and add new searchresult entries */
        res = get_yang_library_modules_state_reply_to_searchresult_entries(server_cb, scb, reply);
        if (res == NO_ERR) {
            searchresult = server_cb->cursearchresult;
        } else {
            assert(0);
        }
        if(searchresult==NULL) {
            /* no search results left to get */
            return autoload_compile_modules(server_cb, scb);
        }
    }

    module = searchresult->module;
    revision = searchresult->revision;

    if (anyerrors) {
        /* going to skip this module any try to
         * compile without it
         */
        log_error("\nError: <get-schema> for module '%s', "
                  "revision '%s' failed",
                  module,
                  (revision) ? revision : EMPTY_STRING);
        if (!LOGDEBUG2) {
            /* the error was never printed */
            if (LOGINFO) {
                val_dump_value_max(reply, 
                                   0,
                                   server_cb->defindent,
                                   DUMP_VAL_LOG,
                                   server_cb->display_mode,
                                   FALSE,
                                   FALSE);
            }
        }
    } else {
        res = get_schema_reply_to_temp_filcb(server_cb, mscb, module, revision, reply);
        if (res != NO_ERR) {
            log_error("\nError: save <get-schema> content "
                      " for module '%s' revision '%s' failed (%s)",
                      module,
                      (revision) ? revision : EMPTY_STRING,
                      get_error_string(res));
            server_cb->cursearchresult->res = res;
        }

    }

    /* find next file that needs to be retrieved with get-schema */
    for (searchresult = (ncxmod_search_result_t *)
             dlq_nextEntry(server_cb->cursearchresult);
         searchresult != NULL && !done;
         searchresult = (ncxmod_search_result_t *)
             dlq_nextEntry(searchresult)) {

        /* skip found entries */
        if (searchresult->source != NULL) {
            continue;
        }

        /* skip found modules with errors */          
        if (searchresult->res != NO_ERR &&
            !(searchresult->res == ERR_NCX_WRONG_VERSION ||
              searchresult->res == ERR_NCX_MOD_NOT_FOUND)) {
            continue;
        }

        /* found an entry that needs to be retrieved */
        server_cb->command_mode = CMD_MODE_AUTOLOAD;
        server_cb->cursearchresult = searchresult;
        done = TRUE;

        res = send_get_schema_to_server(server_cb,
                                       scb,
                                       searchresult->module,
                                       searchresult->revision);
    }

    if (!done) {
        /* no search results left to get */
        return autoload_compile_modules(server_cb, scb);
    }

    return res;

}  /* autoload_handle_rpc_reply */


/********************************************************************
* FUNCTION autoload_compile_modules
* 
* Go through all the search result records and parse
* the modules that the device advertised.
* DOES NOT LOAD THESE MODULES INTO THE MODULE DIRECTORY
* THE RETURNED ncx_module_t STRUCT IS JUST FOR ONE SESSION USE
*
* Apply the deviations and features specified in
* the search result cap back-ptr, to the module 
*
* INPUTS:
*   server_cb == server session control block to use
*   scb == session control block to use
*
* OUTPUTS:
*   $HOME/.yuma/tmp/<progdir>/<sesdir>/ filled with
*   the the specified YANG files that are already available
*   on this system.
*   
*   These search records will be removed from the 
*   server_cb->searchresultQ and modptr records 
*   added to the server_cb->modptrQ
*
* RETURNS:
*    status
*********************************************************************/
status_t
    autoload_compile_modules (server_cb_t *server_cb,
                              ses_cb_t *scb)
{
    mgr_scb_t              *mscb;
    ncxmod_search_result_t *searchresult;
    modptr_t               *modptr;
    ncx_module_t           *mod, *ncmod;
    status_t                res;

#ifdef DEBUG
    if (!server_cb || !scb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* should not happen, but it is possible that the
     * server did not send any YANG module capabilities
     */
    if (dlq_empty(&server_cb->searchresultQ)) {
        return NO_ERR;
    }

    mod = NULL;
    ncmod = NULL;
    res = NO_ERR;
    mscb = (mgr_scb_t *)scb->mgrcb;

    /* set the alternate path to point at the
     * session work directory; this will cause
     * the server revision date of each module to be
     * used instead of a random version that the
     * yangcli registry contains
     */
    ncxmod_set_altpath(mscb->temp_sescb->source);

    /* set the altername module Q so the imports
     * do not get re-compiled over and over
     */
    ncx_set_cur_modQ(&mscb->temp_modQ);

    /* !!! temp until the ietf-netconf.yang module
     * is fully supported.  The yuma-netconf.yang
     * module is pre-loaded as the first module
     */
    res = autoload_module(NCXMOD_YUMA_NETCONF,
                          NULL,
                          NULL, 
                          &server_cb->autoload_savedevQ,
                          &ncmod);
    if (res == NO_ERR && ncmod != NULL) {
        /* Set the features in yuma-netconf.yang according
         * to the standard capabilities that were announced
         * by the server
         */
        set_temp_ync_features(mscb);

        modptr = new_modptr(ncmod, 
                            &mscb->temp_ync_features,
                            NULL);
        if (modptr == NULL) {
            res = ERR_INTERNAL_MEM;
            log_error("\nMalloc failure");
        } else {
            dlq_enque(modptr, &server_cb->modptrQ);
        }
    }

    /* go through all the search results and load
     * the modules in order; all imports should already
     * be preloaded into the session work directory
     */
    while (/*res == NO_ERR && */
           !dlq_empty(&server_cb->searchresultQ)) {

        searchresult = (ncxmod_search_result_t *)
            dlq_deque(&server_cb->searchresultQ);

	if (searchresult->res == ERR_NCX_MOD_NOT_FOUND){
            searchresult->res = NO_ERR;
	}

        if (searchresult->res != NO_ERR ||
            searchresult->source == NULL) {
            ncxmod_free_search_result(searchresult);
            continue;
        }

        mod = NULL;
        mod = ncx_find_module(searchresult->module,
                              searchresult->revision);
        if(mod==NULL) {
            ncx_list_t *devlist = NULL;

            /* prefer the list of deviations from modules-state */
            if (ncx_list_cnt(&searchresult->devlist) > 0) {
                devlist = &searchresult->devlist;
            } else if (searchresult->cap) {
                devlist = &searchresult->cap->cap_deviation_list;
            }

            res = autoload_module(searchresult->module,
                                  searchresult->revision,
                                  devlist,
                                  &server_cb->autoload_savedevQ,
                                  &mod);

            searchresult->res = res;
        }

        if (res == NO_ERR) {
            if (mod == NULL) {
                /* ??? not sure if this could happen ?? */
                mod = ncx_find_module(searchresult->module,
                                      searchresult->revision);
                if (mod == NULL) {
                    log_warn("\nWarning: no module parsed "
                             "for module %s, rev %s",
                             searchresult->module,
                             (searchresult->revision) ?
                             searchresult->revision : NCX_EL_NONE);
                }
            }

            /* make sure this module is not stored more than once */
            modptr = find_modptr(&server_cb->modptrQ,
                                 searchresult->module,
                                 searchresult->revision);

            if (mod != NULL && modptr == NULL) {
                /* mod can be NULL if the module was already 
                 * in the temp_modQ 
                 */
                modptr = new_modptr(mod, 
                                    searchresult->cap?&searchresult->cap->cap_feature_list:NULL,
                                    searchresult->cap?&searchresult->cap->cap_deviation_list:NULL);
                if (modptr == NULL) {
                    log_error("\nMalloc failure");
                } else {
                    dlq_enque(modptr, &server_cb->modptrQ);
                }
            }
        }

        ncxmod_free_search_result(searchresult);
    }

    /* undo the temporary MODPATH setting */
    ncxmod_clear_altpath();

    /* undo the temporary module Q */
    ncx_reset_modQ();

    /* set the session module Q so the val.c
     * functions will find the server modules
     * instead of the pre-loaded versions
     */
    ncx_set_session_modQ(&mscb->temp_modQ);

    if (!server_cb->keep_session_model_copies_after_compilation && mscb->temp_progcb && mscb->temp_sescb) {
        /* we won't be keeping the compiled yang sources */
        ncxmod_free_session_tempdir(mscb->temp_progcb,
                                    mscb->temp_sescb->sidnum);
        mscb->temp_sescb = NULL;
    }


    /* need to wait until all the modules are loaded to
     * go through the modptr list and enable/disable the features
     * to match what the server has reported
     */
    for (modptr = (modptr_t *)
             dlq_firstEntry(&server_cb->modptrQ);
         modptr != NULL;
         modptr = (modptr_t *)dlq_nextEntry(modptr)) {

        if (modptr->feature_list) {
            ncx_for_all_features(modptr->mod,
                                 reset_feature,
                                 modptr->feature_list,
                                 FALSE);
        }
    }

    server_cb->command_mode = CMD_MODE_NORMAL;
    server_cb->cursearchresult = NULL;

    return res;

}  /* autoload_compile_modules */


/* END yangcli_autoload.c */
