/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: ncx.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
30oct05      abb      begun
30oct07      abb      change identifier separator from '.' to '/'
                      and change valid identifier chars to match YANG

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#ifdef HAS_FLOAT
#include <math.h>
#include <tgmath.h>
#endif

#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#include "curversion.h"

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_cli
#include "cli.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ext
#include "ext.h"
#endif

#ifndef _H_grp
#include "grp.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_runstack
#include "runstack.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_ses_msg
#include "ses_msg.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_top
#include "top.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_version
#include "version.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define INV_PREFIX  ((const xmlChar *)"inv")


/********************************************************************
*                                                                   *
*                            T Y P E S                              *
*                                                                   *
*********************************************************************/

/* warning suppression entry */
typedef struct warnoff_t_ {
    dlq_hdr_t    qhdr;
    status_t     res;
} warnoff_t;


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/

/* Q of ncx_module_t
 * list of active modules
 */
static dlq_hdr_t         ncx_modQ;

/* pointer to the current Q of active modules */
static dlq_hdr_t        *ncx_curQ;

/* pointer to the current session Q of active modules
 * this is for yangcli to serialize access to the
 * database of modules
 */
static dlq_hdr_t        *ncx_sesmodQ;

/* Q of ncx_filptr_t
 * used as a cache of subtree filtering headers 
 */
static dlq_hdr_t         ncx_filptrQ;

/* maximum number of filter cache entries */
static uint32            ncx_max_filptrs;

/* current number of filter cache entries */
static uint32            ncx_cur_filptrs;

/* generic anyxml object template */
static obj_template_t   *gen_anyxml;

/* generic container object template */
static obj_template_t   *gen_container;

/* generic string object template */
static obj_template_t   *gen_string;

/* generic empty object template */
static obj_template_t   *gen_empty;

/* generic root container object template */
static obj_template_t   *gen_root;

/* generic binary leaf object template */
static obj_template_t   *gen_binary;

/* module load callback function
 * TBD: support multiple callbacks 
 *  used when a ncxmod loads a module
 */
static ncx_load_cbfn_t  mod_load_callback;

/* 1st stage init */
static boolean       ncx_init_done = FALSE;

/* 2nd stage init */
static boolean       stage2_init_done = FALSE;

/* save descriptive clauses like description, reference */
static boolean       save_descr = FALSE;

/* system warning length for identifiers */
static uint32        warn_idlen;

/* system warning length for YANG file line length */
static uint32        warn_linelen;

/* Q of warnoff_t
 * used to suppress warnings
 * from being counted in the total
 * only filters out the ncx_print_errormsg calls
 * not the log_warn text messages
 */
static dlq_hdr_t     warnoffQ;

/* pointer to the current Q of modules to use
 * for yangcli sessions; will be 1 per thread
 * later but now this works because access is
 * serialized and the temp_modQ is set when each
 * session needs to look for objects (CLI or XPath)
 */
static dlq_hdr_t   *temp_modQ;

/* default diplay mode in all programs except yangcli,
 * which uses a more complex schem for diplay-mode
 */
static ncx_display_mode_t  display_mode;


/********************************************************************
* FUNCTION check_moddef
* 
* Check if a specified module is loaded
* If not, load it.
*
* Search the module for the data struct for the 
* specified definition name.
*
* INPUTS:
*   pcb == parser control block to use
*   imp == import struct to use
*   defname == name of the app-specific definition to find
*   dtyp == address of return definition type (for verification)
*   *dtyp == NCX_NT_NONE for any match, or a specific type to find
*   dptr == addres of return definition pointer
*
* OUTPUTS:
*   pcb == parser control block to use
*   imp->mod may be set if not already
*   *dtyp == node type found NCX_NT_OBJ or NCX_NT_TYPE, etc. 
*   *dptr == pointer to data struct or NULL if not found
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_moddef (yang_pcb_t *pcb,
                  ncx_import_t *imp,
                  const xmlChar *defname,
                  ncx_node_t *dtyp,
                  void **dptr)
{
    status_t       res, retres;

    retres = NO_ERR;

    /* First find or load the module */
    if (!imp->mod) {
        imp->mod = ncx_find_module(imp->module, imp->revision);
    }

    if (!imp->mod) {
        res = ncxmod_load_module(imp->module, 
                                 imp->revision, 
                                 pcb->savedevQ,
                                 &imp->mod);
        CHK_EXIT(res, retres);
        if (!imp->mod) {
            return ERR_NCX_MOD_NOT_FOUND;
        }
    }

    /* have a module loaded that might contain this def 
     * look for the defname
     * the module may be loaded with non-fatal errors
     */
    switch (*dtyp) {
    case NCX_NT_TYP:
        *dptr = ncx_find_type(imp->mod, defname);
        break;
    case NCX_NT_GRP:
        *dptr = ncx_find_grouping(imp->mod, defname);
        break;
        break;
    case NCX_NT_OBJ:
        *dptr = obj_find_template(&imp->mod->datadefQ, 
                                  imp->module, 
                                  defname);
        break;
    case NCX_NT_NONE:
        *dptr = ncx_find_type(imp->mod, defname);
        if (*dptr) {
            *dtyp = NCX_NT_TYP;
        }
        if (!*dptr) {
            *dptr = ncx_find_grouping(imp->mod, defname);
            if (*dptr) {
                *dtyp = NCX_NT_GRP;
            }
        }
        if (!*dptr) {
            *dptr = obj_find_template(&imp->mod->datadefQ, 
                                      imp->module, 
                                      defname);
            if (*dptr) {
                *dtyp = NCX_NT_OBJ;
            }
        }
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        *dptr = NULL;
    }

    return (*dptr) ? NO_ERR : ERR_NCX_DEF_NOT_FOUND;

}  /* check_moddef */


/********************************************************************
* FUNCTION consume_appinfo_entry
* 
* Check if an appinfo sub-clause is present
*
*   foovar "fred";
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Consume the appinfo clause tokens, and save it in
* appinfoQ, if that var is non-NULL
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress
*   appinfoQ == address of Queue to hold this entry (may be NULL)
*   bkup == TRUE if not looking for a right brace
*           FALSE if right brace should be checked
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_appinfo_entry (tk_chain_t *tkc,
                           ncx_module_t  *mod,
                           dlq_hdr_t     *appinfoQ,
                           boolean bkup)
{
    ncx_appinfo_t   *appinfo;
    status_t         res, retres;

    /* right brace means appinfo is done */
    if (tkc->source == TK_SOURCE_YANG && !bkup) {
        if (tk_next_typ(tkc)==TK_TT_RBRACE) {
            return ERR_NCX_SKIPPED;
        }
    }

    res = NO_ERR;
    retres = NO_ERR;
    appinfo = NULL;

    appinfo = ncx_new_appinfo(FALSE);
    if (!appinfo) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    /* get the appinfo prefix value and variable name
     *
     * Get the first token; should be an unquoted string
     * if OK then malloc a new appinfo struct and make
     * a copy of the token value.
     */
    res = yang_consume_pid_string(tkc, 
                                  mod,
                                  &appinfo->prefix,
                                  &appinfo->name);
    if (res != NO_ERR) {
        retres = res;
        if (NEED_EXIT(res)) {
            ncx_free_appinfo(appinfo);
            return retres;
        }
    }

    ncx_set_error(&appinfo->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    if (!bkup && appinfo->prefix == NULL) {
        /* skip the entire YANG statement */
        if (LOGDEBUG) {
            log_debug("\nSkipping YANG statement inside "
                      "an unknown-stmt");
        }
        yang_skip_statement(tkc, mod);
        ncx_free_appinfo(appinfo);
        return NO_ERR;
    }

    /* at this point, if appinfoQ non-NULL:
     *    appinfo is malloced initialized
     *    appinfo prefix and name are set
     *
     * Now get the optional appinfo value string
     *
     * move to the 2nd token, either a string or a semicolon
     * if the value is missing
     */
    switch (tk_next_typ(tkc)) {
    case TK_TT_SEMICOL:
    case TK_TT_LBRACE:
        break;
    default:
        res = yang_consume_string(tkc, mod, &appinfo->value);
        if (res != NO_ERR) {
            retres = res;
            if (NEED_EXIT(res)) {
                ncx_free_appinfo(appinfo);
                return retres;
            }
        }
    }

    /* go around and get nested extension statements or semi-colon */
    res = yang_consume_semiapp(tkc, mod, appinfo->appinfoQ);
    if (res != NO_ERR) {
        retres = res;
        if (NEED_EXIT(res)) {
            ncx_free_appinfo(appinfo);
            return retres;
        }
    }

    if (retres != NO_ERR || !appinfoQ) {
        ncx_free_appinfo(appinfo);
    } else {
        dlq_enque(appinfo, appinfoQ);
    }

    return retres;

}  /* consume_appinfo_entry */


/********************************************************************
* FUNCTION consume_appinfo
* 
* Check if an appinfo clause is present
*
* Save in appinfoQ if non-NULL
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress (NULL if none)
*   appinfoQ  == queue to use for any found entries (may be NULL)
*   bkup == TRUE if token should be backed up first
*           FALSE if not
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_appinfo (tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     dlq_hdr_t *appinfoQ,
                     boolean bkup)
{
    status_t       res;
    boolean        done;

    if (tkc->source == TK_SOURCE_YANG && bkup) {
        /* hack: all the YANG fns that call this function
         * already parsed the MSTRING since extensions
         * can be spread out throughout the file
         */
        TK_BKUP(tkc);
    }

    res = NO_ERR;
    done = FALSE;
    while (!done) {
        res = consume_appinfo_entry(tkc, 
                                    mod, 
                                    appinfoQ, 
                                    bkup);
        if (res != NO_ERR || tkc->source == TK_SOURCE_YANG) {
            done = TRUE;
        }
    }

    return res;

}  /* consume_appinfo */


/********************************************************************
* FUNCTION set_toplevel_defs
*
* INPUTS:
*   mod == module to check
*   nsid == namespace ID of the main module being added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    set_toplevel_defs (ncx_module_t *mod,
                       xmlns_id_t    nsid)
{
    typ_template_t *typ;
    grp_template_t *grp;
    obj_template_t *obj;
    ext_template_t *ext;

    for (typ = (typ_template_t *)dlq_firstEntry(&mod->typeQ);
         typ != NULL;
         typ = (typ_template_t *)dlq_nextEntry(typ)) {

        typ->nsid = nsid;
    }

    for (grp = (grp_template_t *)dlq_firstEntry(&mod->groupingQ);
         grp != NULL;
         grp = (grp_template_t *)dlq_nextEntry(grp)) {

        grp->nsid = nsid;
    }

    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (obj_is_data_db(obj) || obj_is_rpc(obj) 
            || obj_is_notif(obj)) {
            obj->parent = gen_root;
        }
    }

    for (ext = (ext_template_t *)dlq_firstEntry(&mod->extensionQ);
         ext != NULL;
         ext = (ext_template_t *)dlq_nextEntry(ext)) {
        ext->nsid = nsid;
    }

    return NO_ERR;

}  /* set_toplevel_defs */


/********************************************************************
* FUNCTION free_module
* 
* Scrub the memory in a ncx_module_t by freeing all
* the sub-fields and then freeing the entire struct itself 
*
* MUST remove this struct from the ncx_modQ before calling
* Do not need to remove module definitions from the registry
*
* Use the ncx_remove_module function if the module was 
* already successfully added to the modQ and definition registry
*
* INPUTS:
*    mod == ncx_module_t data structure to free
*********************************************************************/
static void 
    free_module (ncx_module_t *mod)
{
    ncx_revhist_t  *revhist;
    ncx_import_t   *import;
    ncx_include_t  *incl;
    ncx_feature_t  *feature;
    ncx_identity_t *identity;
    yang_stmt_t    *stmt;

    /* clear the revision Q */
    while (!dlq_empty(&mod->revhistQ)) {
        revhist = (ncx_revhist_t *)dlq_deque(&mod->revhistQ);
        ncx_free_revhist(revhist);
    }

    /* clear the import Que */
    while (!dlq_empty(&mod->importQ)) {
        import = (ncx_import_t *)dlq_deque(&mod->importQ);
        ncx_free_import(import);
    }

    /* clear the include Que */
    while (!dlq_empty(&mod->includeQ)) {
        incl = (ncx_include_t *)dlq_deque(&mod->includeQ);
        ncx_free_include(incl);
    }

    /* clear the type Que */
    typ_clean_typeQ(&mod->typeQ);

    /* clear the grouping Que */
    grp_clean_groupingQ(&mod->groupingQ);

    /* clear the datadefQ */
    obj_clean_datadefQ(&mod->datadefQ);

    /* clear the extension Que */
    ext_clean_extensionQ(&mod->extensionQ);

    obj_clean_deviationQ(&mod->deviationQ);

    ncx_clean_appinfoQ(&mod->appinfoQ);

    ncx_clean_typnameQ(&mod->typnameQ);

    yang_clean_import_ptrQ(&mod->saveimpQ);

    yang_clean_nodeQ(&mod->saveincQ);

    /* clear the YANG stmtQ, used for docmode only */
    while (!dlq_empty(&mod->stmtQ)) {
        stmt = (yang_stmt_t *)dlq_deque(&mod->stmtQ);
        yang_free_stmt(stmt);
    }

    /* clear the YANG featureQ */
    while (!dlq_empty(&mod->featureQ)) {
        feature = (ncx_feature_t *)dlq_deque(&mod->featureQ);
        ncx_free_feature(feature);
    }

    /* clear the YANG identityQ */
    while (!dlq_empty(&mod->identityQ)) {
        identity = (ncx_identity_t *)dlq_deque(&mod->identityQ);
        ncx_free_identity(identity);
    }

    /* clear the name and other fields last for easier debugging */
    if (mod->name) {
        m__free(mod->name);
    }
    if (mod->version) {
        m__free(mod->version);
    }
    if (mod->organization) {
        m__free(mod->organization);
    }
    if (mod->contact_info) {
        m__free(mod->contact_info);
    }
    if (mod->descr) {
        m__free(mod->descr);
    }
    if (mod->ref) {
        m__free(mod->ref);
    }
    if (mod->ismod && mod->ns) {
        m__free(mod->ns);
    } 
    if (mod->prefix) {
        m__free(mod->prefix);
    }
    if (mod->xmlprefix) {
        m__free(mod->xmlprefix);
    }
    if (mod->source) {
        m__free(mod->source);
    }
    if (mod->belongs) {
        m__free(mod->belongs);
    }

    ncx_clean_list(&mod->devmodlist);

    m__free(mod);

}  /* free_module */


/********************************************************************
* FUNCTION remove_trailing_zero_count
* 
* Get the number of trailing zeros and maybe the decimal point too
*
* INPUTS:
*   buff == number buffer to check
*
* RETURNS:
*   number of chars to remove from the end of string
*********************************************************************/
static int32
    remove_trailing_zero_count (const xmlChar *buff)
{
    int32  len, newlen;
    const xmlChar  *str;

    len = strlen((const char *)buff);
    if (!len) {
        return 0;
    }

    str = &buff[len-1];

    while (str >= buff && *str == '0') {
        str--;
    }

    if (*str == '.') {
        str--;
    }

    newlen = (int32)(str - buff) + 1;

    return len - newlen;

}  /* remove_trailing_zero_count */


/********************************************************************
* FUNCTION bootstrap_cli
* 
* Handle the CLI parms that need to be set even before
* any other initialization has been done, or any YANG modules
* have been loaded.
*
* Hardwired list of bootstrap parameters
* DO NOT RESET THEM FROM THE OUTPUT OF THE cli_parse function
* THESE CLI PARAMS WILL STILL BE IN THE argv array
*
* Common CLI parameters handled as bootstrap parameters
*
*   log-level
*   log-append
*   log
*   modpath
*
* INPUTS:
*    argc == CLI argument count
*    argv == array of CLI parms
*    dlevel == default debug level
*    logtstamps == flag for log_open, if 'log' parameter is present
*
* RETURNS:
*   status of the bootstrap CLI procedure
*********************************************************************/
static status_t 
    bootstrap_cli (int argc,
                   const char *argv[],
                   log_debug_t dlevel,
                   boolean logtstamps)
{
    dlq_hdr_t        parmQ;
    cli_rawparm_t   *parm;
    char            *logfilename;
    log_debug_t      loglevel;
    boolean          logappend;
    status_t         res;

    dlq_createSQue(&parmQ);

    res = NO_ERR;
    logfilename = NULL;
    logappend = FALSE;
    loglevel = LOG_DEBUG_NONE;

    /* create bootstrap parm: log-level */
    parm = cli_new_rawparm(NCX_EL_LOGLEVEL);
    if (parm) {
        dlq_enque(parm, &parmQ);
    } else {
        log_error("\nError: malloc failed");
        res = ERR_INTERNAL_MEM;
    }

    /* create bootstrap parm: log */
    if (res == NO_ERR) {
        parm = cli_new_rawparm(NCX_EL_LOG);
        if (parm) {
            dlq_enque(parm, &parmQ);
        } else {
            log_error("\nError: malloc failed");
            res = ERR_INTERNAL_MEM;
        }
    }

    /* create bootstrap parm: log-append */
    if (res == NO_ERR) {
        parm = cli_new_empty_rawparm(NCX_EL_LOGAPPEND);
        if (parm) {
            dlq_enque(parm, &parmQ);
        } else {
            log_error("\nError: malloc failed");
            res = ERR_INTERNAL_MEM;
        }
    }

    /* create bootstrap parm: modpath */
    if (res == NO_ERR) {
        parm = cli_new_rawparm(NCX_EL_MODPATH);
        if (parm) {
            dlq_enque(parm, &parmQ);
        } else {
            log_error("\nError: malloc failed");
            res = ERR_INTERNAL_MEM;
        }
    }

    /* create bootstrap parm: yuma-home */
    if (res == NO_ERR) {
        parm = cli_new_rawparm(NCX_EL_YUMA_HOME);
        if (parm) {
            dlq_enque(parm, &parmQ);
        } else {
            log_error("\nError: malloc failed");
            res = ERR_INTERNAL_MEM;
        }
    }

    /* check if any of these bootstrap parms are present
     * and process them right away; this is different than
     * normal CLI where all the parameters are gathered,
     * validated, and then invoked
     */
    if (res == NO_ERR) {
        res = cli_parse_raw(argc, argv, &parmQ);
        if (res != NO_ERR) {
            log_error("\nError: bootstrap CLI failed (%s)",
                      get_error_string(res));
        }
    }

    if (res != NO_ERR) {
        cli_clean_rawparmQ(&parmQ);
        return res;
    }

    /* --log-level=<debug_level> */
    parm = cli_find_rawparm(NCX_EL_LOGLEVEL, &parmQ);
    if (parm && parm->count) {
        if (parm->count > 1) {
            log_error("\nError: Only one log-level parameter allowed");
            res = ERR_NCX_DUP_ENTRY;
        } else if (parm->value) {
            loglevel = log_get_debug_level_enum(parm->value);
            if (loglevel == LOG_DEBUG_NONE) {
                log_error("\nError: '%s' not valid log-level",
                          parm->value);
                res = ERR_NCX_INVALID_VALUE;
            } else {
                log_set_debug_level(loglevel);
            }
        } else {
            log_error("\nError: no value entered for "
                      "'log-level' parameter");
            res = ERR_NCX_INVALID_VALUE;
        }
    } else {
        log_set_debug_level(dlevel);
    }

    /* --log-append */
    if (res == NO_ERR) {
        parm = cli_find_rawparm(NCX_EL_LOGAPPEND, &parmQ);
        logappend = (parm && parm->count) ? TRUE : FALSE;
        if (parm->value) {
            log_error("\nError: log-append is empty parameter");
            res = ERR_NCX_INVALID_VALUE;
        }
    }

    /* --log=<logfilespec> */
    if (res == NO_ERR) {
        parm = cli_find_rawparm(NCX_EL_LOG, &parmQ);
        if (parm && parm->count) {
            if (parm->count > 1) {
                log_error("\nError: Only one 'log' filename allowed");
                res = ERR_NCX_DUP_ENTRY;
            } else if (parm->value) {
                res = NO_ERR;
                logfilename = (char *)
                    ncx_get_source((const xmlChar *)parm->value, &res);
                if (logfilename) {
                    res = log_open(logfilename, logappend, logtstamps);
                    if (res != NO_ERR) {
                        log_error("\nError: open logfile '%s' failed (%s)",
                                  logfilename,
                                  get_error_string(res));
                    }
                    m__free(logfilename);
                }
            } else {
                log_error("\nError: no value entered for "
                          "'log' parameter");
                res = ERR_NCX_INVALID_VALUE;
            }
        } /* else use default log (stdout) */
    }

    /* --modpath=<pathspeclist> */
    if (res == NO_ERR) {
        parm = cli_find_rawparm(NCX_EL_MODPATH, &parmQ);
        if (parm && parm->count) {
            if (parm->count > 1) {
                log_error("\nError: Only one 'modpath' parameter allowed");
                res = ERR_NCX_DUP_ENTRY;
            } else if (parm->value) {
                /*** VALIDATE MODPATH FIRST ***/
                ncxmod_set_modpath((const xmlChar *)parm->value);
            } else {
                log_error("\nError: no value entered for "
                          "'modpath' parameter");
                res = ERR_NCX_INVALID_VALUE;
            }
        } /* else use default modpath */
    }

    /* --yuma-home=<$YUMA_HOME> */
    if (res == NO_ERR) {
        parm = cli_find_rawparm(NCX_EL_YUMA_HOME, &parmQ);
        if (parm && parm->count) {
            if (parm->count > 1) {
                log_error("\nError: Only one 'yang-home' parameter allowed");
                res = ERR_NCX_DUP_ENTRY;
            } else {
                /*** VALIDATE YUMA_HOME ***/
                ncxmod_set_yuma_home((const xmlChar *)parm->value);
            }
        } /* else use default modpath */
    }

    cli_clean_rawparmQ(&parmQ);
    return res;

} /* bootstrap_cli */


/********************************************************************
* FUNCTION add_to_modQ
*
* INPUTS:
*   mod == module to add to modQ
*   modQ == Q of ncx_module_t to use
*
*********************************************************************/
static void
    add_to_modQ (ncx_module_t *mod,
                 dlq_hdr_t *modQ)
{
    ncx_module_t   *testmod;
    boolean         done;
    int32           retval;

    done = FALSE;

    for (testmod = (ncx_module_t *)dlq_firstEntry(modQ);
         testmod != NULL && !done;
         testmod = (ncx_module_t *)dlq_nextEntry(testmod)) {

        retval = xml_strcmp(mod->name, testmod->name);
        if (retval == 0) {
            retval = yang_compare_revision_dates(mod->version,
                                                 testmod->version);
            if (retval == 0) {
                if ((!mod->version && !testmod->version) ||
                    (mod->version && testmod->version)) {
                    /* !!! adding duplicate version !!! */
                    log_info("\nInfo: Adding duplicate revision '%s' of "
                             "%s module (%s)",
                             (mod->version) ? mod->version : EMPTY_STRING,
                             mod->name,
                             mod->source);
                }
                testmod->defaultrev = FALSE;
                mod->defaultrev = TRUE;
                dlq_insertAhead(mod, testmod);
                done = TRUE;
            } else if (retval > 0) {
                testmod->defaultrev = FALSE;
                mod->defaultrev = TRUE;
                dlq_insertAhead(mod, testmod);
                done = TRUE;
            } else {
                mod->defaultrev = FALSE;
                dlq_insertAfter(mod, testmod);
                done = TRUE;
            }
        } else if (retval < 0) {
            mod->defaultrev = TRUE;
            dlq_insertAhead(mod, testmod);
            done = TRUE;
        } /* else keep going */
    }

    if (!done) {
        mod->defaultrev = TRUE;
        dlq_enque(mod, modQ);
    }
            
}  /* add_to_modQ */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION ncx_init
* 
* Initialize the NCX module
*
* INPUTS:
*    savestr == TRUE if parsed description strings that are
*               not needed by the agent at runtime should
*               be saved anyway.  Converters should use this value.
*                 
*            == FALSE if uneeded strings should not be saved.
*               Embedded agents should use this value
*
*    dlevel == desired debug output level
*    logtstamps == TRUE if log should use timestamps
*                  FALSE if not; not used unless 'log' is present
*    startmsg == log_debug2 message to print before starting;
*                NULL if not used;
*    argc == CLI argument count for bootstrap CLI
*    argv == array of CLI parms for bootstrap CLI
*
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    ncx_init (boolean savestr,
              log_debug_t dlevel,
              boolean logtstamps,
              const char *startmsg,
              int argc,
              const char *argv[])
{
    status_t     res;
    xmlns_id_t   nsid;
    
    if (ncx_init_done) {
        return NO_ERR;
    }

    status_init();

    save_descr = savestr;
    warn_idlen = NCX_DEF_WARN_IDLEN;
    warn_linelen = NCX_DEF_WARN_LINELEN;

    mod_load_callback = NULL;
    log_set_debug_level(dlevel);

    /* create the module and appnode queues */
    dlq_createSQue(&ncx_modQ);
    ncx_curQ = &ncx_modQ;
    ncx_sesmodQ = NULL;
    dlq_createSQue(&ncx_filptrQ);
    dlq_createSQue(&warnoffQ);
    temp_modQ = NULL;

    display_mode = NCX_DISPLAY_MODE_PREFIX;

    ncx_max_filptrs = NCX_DEF_FILPTR_CACHESIZE;
    ncx_cur_filptrs = 0;

    /* check that the correct version of libxml2 is installed */
    LIBXML_TEST_VERSION;

    /* init module library handler */
    ncxmod_init();

    /* deal with bootstrap CLI parms */
    res = bootstrap_cli(argc, argv, dlevel, logtstamps);
    if (res != NO_ERR) {
        return res;
    }

    if (startmsg) {
        log_write(startmsg);
    }

    /* init runstack script support */
    runstack_init();

    /* init top level msg dispatcher */
    top_init();

    /* initialize the definition resistry */
    def_reg_init();

    /* initialize the namespace registry */
    xmlns_init();

    ncx_init_done = TRUE;

    /* Initialize the INVALID namespace to help filter handling */
    res = xmlns_register_ns(INVALID_URN, 
                            INV_PREFIX, 
                            NCX_MODULE, 
                            NULL, 
                            &nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the XML namespace for NETCONF */
    res = xmlns_register_ns(NC_URN, 
                            NC_PREFIX, 
                            NC_MODULE, 
                            NULL, 
                            &nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the XML namespace for YANG */
    res = xmlns_register_ns(YANG_URN, 
                            YANG_PREFIX, 
                            YANG_MODULE, 
                            NULL, 
                            &nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the XML namespace for YIN */
    res = xmlns_register_ns(YIN_URN, 
                            YIN_PREFIX, 
                            YIN_MODULE, 
                            NULL, 
                            &nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the XMLNS namespace for xmlns attributes */
    res = xmlns_register_ns(NS_URN, 
                            NS_PREFIX, 
                            NCX_MODULE, 
                            NULL, 
                            &nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the XSD namespace for ncxdump program */
    res = xmlns_register_ns(XSD_URN, 
                            XSD_PREFIX, 
                            NCX_MODULE, 
                            NULL, 
                            &nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the XSI namespace for ncxdump program */
    res = xmlns_register_ns(XSI_URN, 
                            XSI_PREFIX, 
                            NCX_MODULE, 
                            NULL, 
                            &nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* Initialize the XML namespace for xml:lang attribute support */
    res = xmlns_register_ns(XML_URN, 
                            XML_PREFIX, 
                            NCX_MODULE, 
                            NULL, 
                            &nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* load the basetypes into the definition registry */
    res = typ_load_basetypes();
    if (res != NO_ERR) {
        return res;
    }

    /* initialize the configuration manager */
    cfg_init();

    /* initialize the session message manager */
    ses_msg_init();

    return NO_ERR;

}  /* ncx_init */


/********************************************************************
* FUNCTION ncx_stage2_init
* 
* Initialize the NCX module during stage 2 startup,
* after the object database has been loaded, but before the 
* agent has started accepting PDUs
*
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    ncx_stage2_init (void)
{
    ncx_module_t     *mod;

    if (stage2_init_done) {
        return NO_ERR;
    }

    mod = ncx_find_module(NCX_MODULE, NULL);
    if (!mod) {
        return ERR_NCX_MOD_NOT_FOUND;
    }

    gen_anyxml = ncx_find_object(mod, NCX_EL_ANY);
    if (!gen_anyxml) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    gen_container = ncx_find_object(mod, NCX_EL_STRUCT);
    if (!gen_container) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    gen_string = ncx_find_object(mod, NCX_EL_STRING);
    if (!gen_string) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    gen_empty = ncx_find_object(mod, NCX_EL_EMPTY);
    if (!gen_empty) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    gen_root = ncx_find_object(mod, NCX_EL_ROOT);
    if (!gen_root) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    gen_binary = ncx_find_object(mod, NCX_EL_BINARY);
    if (!gen_binary) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    stage2_init_done = TRUE;
    return NO_ERR;

}  /* ncx_stage2_init */


/********************************************************************
* FUNCTION ncx_cleanup
*
*  cleanup NCX module
*********************************************************************/
void
    ncx_cleanup (void)
{
    ncx_module_t   *mod;
    ncx_filptr_t   *filptr;
    warnoff_t      *warnoff;

    if (!ncx_init_done) {
        return;
    }

    while (!dlq_empty(&ncx_modQ)) {
        mod = (ncx_module_t *)dlq_deque(&ncx_modQ);
        free_module(mod);
    }

    while (!dlq_empty(&ncx_filptrQ)) {
        filptr = (ncx_filptr_t *)dlq_deque(&ncx_filptrQ);
        m__free(filptr);
    }

    while (!dlq_empty(&warnoffQ)) {
        warnoff = (warnoff_t *)dlq_deque(&warnoffQ);
        m__free(warnoff);
    }

    if (stage2_init_done) {
        gen_anyxml = NULL;
        gen_container = NULL;
        gen_string = NULL;
        gen_empty = NULL;
        gen_root = NULL;
        gen_binary = NULL;
    }

    typ_unload_basetypes();
    xmlns_cleanup();
    def_reg_cleanup();
    cfg_cleanup();
    ses_msg_cleanup();
    top_cleanup();
    runstack_cleanup();
    ncxmod_cleanup();
    xmlCleanupParser();
    ncx_init_done = FALSE;
    stage2_init_done = FALSE;

}   /* ncx_cleanup */


/********************************************************************
* FUNCTION ncx_new_module
* 
* Malloc and initialize the fields in a ncx_module_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
ncx_module_t *
    ncx_new_module (void)
{
    ncx_module_t  *mod;

    mod = m__getObj(ncx_module_t);
    if (!mod) {
        return NULL;
    }

    (void)memset(mod, 0x0, sizeof(ncx_module_t));
    mod->langver = 1;
    dlq_createSQue(&mod->revhistQ);
    dlq_createSQue(&mod->importQ);
    dlq_createSQue(&mod->includeQ);
    dlq_createSQue(&mod->typeQ);
    dlq_createSQue(&mod->groupingQ);
    dlq_createSQue(&mod->datadefQ);
    dlq_createSQue(&mod->extensionQ);
    dlq_createSQue(&mod->deviationQ);
    dlq_createSQue(&mod->appinfoQ);
    dlq_createSQue(&mod->typnameQ);
    dlq_createSQue(&mod->saveimpQ);
    dlq_createSQue(&mod->saveincQ);
    dlq_createSQue(&mod->stmtQ);
    dlq_createSQue(&mod->featureQ);
    dlq_createSQue(&mod->identityQ);
    ncx_init_list(&mod->devmodlist, NCX_BT_STRING);
    return mod;

}  /* ncx_new_module */


/********************************************************************
* FUNCTION ncx_find_module
*
* Find a ncx_module_t in the ncx_modQ
* These are the modules that are already loaded
*
* INPUTS:
*   modname == module name
*   revision == module revision date
*
* RETURNS:
*  module pointer if found or NULL if not
*********************************************************************/
ncx_module_t *
    ncx_find_module (const xmlChar *modname,
                     const xmlChar *revision)
{
    ncx_module_t *mod;

#ifdef DEBUG
    if (!modname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;    /* error */
    }
#endif

    /* check the yangcli session module Q
     * and then the current module Q
     */
    mod = NULL;
    if (ncx_sesmodQ) {
        mod = ncx_find_module_que(ncx_sesmodQ, modname, revision);
    }
    if (mod == NULL) {
        mod = ncx_find_module_que(ncx_curQ, modname, revision);
    }
    return mod;

}   /* ncx_find_module */


/********************************************************************
* FUNCTION ncx_find_module_que
*
* Find a ncx_module_t in the specified Q
* Check the namespace ID
*
* INPUTS:
*   modQ == module Q to search
*   modname == module name
*   revision == module revision date
*
* RETURNS:
*  module pointer if found or NULL if not
*********************************************************************/
ncx_module_t *
    ncx_find_module_que (dlq_hdr_t *modQ,
                         const xmlChar *modname,
                         const xmlChar *revision)
{
    ncx_module_t  *mod;
    int32          retval;

#ifdef DEBUG
    if (!modQ || !modname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;    /* error */
    }
#endif

    for (mod = (ncx_module_t *)dlq_firstEntry(modQ);
         mod != NULL;
         mod = (ncx_module_t *)dlq_nextEntry(mod)) {

        retval = xml_strcmp(modname, mod->name);
        if (retval == 0) {
            if (!revision || !mod->version) {
                if (mod->defaultrev) {
                    return mod;
                }
            } else {
                retval = yang_compare_revision_dates(revision,
                                                     mod->version);
                if (retval == 0) {
                    return mod;
                } else if (retval > 0) {
                    return NULL;
                }
            }
        } else if (retval < 0) {
            return NULL;
        }
    }
    return NULL;

}   /* ncx_find_module_que */


/********************************************************************
* FUNCTION ncx_find_module_que_nsid
*
* Find a ncx_module_t in the specified Q
* Check the namespace ID
*
* INPUTS:
*   modQ == module Q to search
*   modname == module name
*   revision == module revision date
*
* RETURNS:
*  module pointer if found or NULL if not
*********************************************************************/
ncx_module_t *
    ncx_find_module_que_nsid (dlq_hdr_t *modQ,
                              xmlns_id_t nsid)
{
    ncx_module_t  *mod;

#ifdef DEBUG
    if (modQ == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;    /* error */
    }
    if (nsid == 0) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }
#endif

    for (mod = (ncx_module_t *)dlq_firstEntry(modQ);
         mod != NULL;
         mod = (ncx_module_t *)dlq_nextEntry(mod)) {

        if (mod->nsid == nsid) {
            return mod;
        }
    }
    return NULL;

}   /* ncx_find_module_que_nsid */


/********************************************************************
* FUNCTION ncx_free_module
* 
* Scrub the memory in a ncx_module_t by freeing all
* the sub-fields and then freeing the entire struct itself 
*
* MUST remove this struct from the ncx_modQ before calling
* Does not remove module definitions from the registry
*
* Use the ncx_remove_module function if the module was 
* already successfully added to the modQ and definition registry
*
* INPUTS:
*    mod == ncx_module_t data structure to free
*********************************************************************/
void 
    ncx_free_module (ncx_module_t *mod)
{
#ifdef DEBUG
    if (mod == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    free_module(mod);

}  /* ncx_free_module */


/********************************************************************
* FUNCTION ncx_any_mod_errors
* 
* Check if any of the loaded modules are loaded with non-fatal errors
*
* RETURNS:
*    TRUE if any modules are loaded with non-fatal errors
*    FALSE if all modules present have a status of NO_ERR
*********************************************************************/
boolean
    ncx_any_mod_errors (void)
{
    ncx_module_t  *mod;

    for (mod = (ncx_module_t *)dlq_firstEntry(ncx_curQ);
         mod != NULL;
         mod = (ncx_module_t *)dlq_nextEntry(mod)) {
        if (mod->status != NO_ERR) {
            return TRUE;
        }
    }

    return FALSE;

}  /* ncx_any_mod_errors */


/********************************************************************
* FUNCTION ncx_any_dependency_errors
* 
* Check if any of the imports that this module relies on
* were loadeds are loaded with non-fatal errors
*
* RETURNS:
*    TRUE if any modules are loaded with non-fatal errors
*    FALSE if all modules present have a status of NO_ERR
*********************************************************************/
boolean
    ncx_any_dependency_errors (const ncx_module_t *mod)
{
    const ncx_module_t       *testmod;
    const yang_import_ptr_t  *impptr;
    const dlq_hdr_t          *impQ;

#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    impQ = (mod->allimpQ) ? mod->allimpQ : &mod->saveimpQ;

    for (impptr = (yang_import_ptr_t *)dlq_firstEntry(impQ);
         impptr != NULL;
         impptr = (yang_import_ptr_t *)dlq_nextEntry(impptr)) {

        testmod = ncx_find_module(impptr->modname,
                                  impptr->revision);
        if (!testmod) {
            /* missing import */
            return TRUE;
        }
            
        if (testmod->status != NO_ERR) {
            if (get_errtyp(testmod->status) < ERR_TYP_WARN) {
                return TRUE;
            }
        }
    }

    return FALSE;

}  /* ncx_any_dependency_errors */


/********************************************************************
* FUNCTION ncx_find_type
*
* Check if a typ_template_t in the mod->typeQ
*
* INPUTS:
*   mod == ncx_module to check
*   typname == type name
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
typ_template_t *
    ncx_find_type (ncx_module_t *mod,
                   const xmlChar *typname)
{
    typ_template_t *typ;
    yang_node_t    *node;
    ncx_include_t  *inc;
    dlq_hdr_t      *que;

#ifdef DEBUG
    if (!mod || !typname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    typ = ncx_find_type_que(&mod->typeQ, typname);
    if (typ) {
        return typ;
    }

    que = (mod->allincQ) ? mod->allincQ : &mod->saveincQ;

    /* check all the submodules, but only the ones visible
     * to this module or submodule, YANG only
     */
    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
         inc != NULL;
         inc = (ncx_include_t *)dlq_nextEntry(inc)) {

        /* get the real submodule struct */
        if (!inc->submod) {
            node = yang_find_node(que, 
                                  inc->submodule,
                                  inc->revision);
            if (node) {
                inc->submod = node->submod;
            }
            if (!inc->submod) {
                /* include not found, should not be in Q !!! */
                SET_ERROR(ERR_INTERNAL_VAL);
                continue;
            }
        }

        /* check the type Q in this submodule */
        typ = ncx_find_type_que(&inc->submod->typeQ, typname);
        if (typ) {
            return typ;
        }
    }

    return NULL;

}   /* ncx_find_type */


/********************************************************************
* FUNCTION ncx_find_type_que
*
* Check if a typ_template_t in the mod->typeQ
*
* INPUTS:
*   que == type Q to check
*   typname == type name
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
typ_template_t *
    ncx_find_type_que (const dlq_hdr_t *typeQ,
                       const xmlChar *typname)
{
    typ_template_t *typ;

#ifdef DEBUG
    if (!typeQ || !typname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (typ = (typ_template_t *)dlq_firstEntry(typeQ);
         typ != NULL;
         typ = (typ_template_t *)dlq_nextEntry(typ)) {
        if (typ->name && !xml_strcmp(typ->name, typname)) {
            return typ;
        }
    }
    return NULL;

}   /* ncx_find_type_que */


/********************************************************************
* FUNCTION ncx_find_grouping
*
* Check if a grp_template_t in the mod->groupingQ
*
* INPUTS:
*   mod == ncx_module to check
*   grpname == group name
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
grp_template_t *
    ncx_find_grouping (ncx_module_t *mod,
                       const xmlChar *grpname)
{
    grp_template_t *grp;
    yang_node_t    *node;
    ncx_include_t  *inc;
    dlq_hdr_t      *que;

#ifdef DEBUG
    if (!mod || !grpname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* check the main module */
    grp = ncx_find_grouping_que(&mod->groupingQ, grpname);
    if (grp) {
        return grp;
    }

    que = (mod->allincQ) ? mod->allincQ : &mod->saveincQ;

    /* check all the submodules, but only the ones visible
     * to this module or submodule, YANG only
     */
    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
         inc != NULL;
         inc = (ncx_include_t *)dlq_nextEntry(inc)) {

        /* get the real submodule struct */
        if (!inc->submod) {
            node = yang_find_node(que, 
                                  inc->submodule,
                                  inc->revision);
            if (node) {
                inc->submod = node->submod;
            }
            if (!inc->submod) {
                /* include not found, should not be in Q !!! */
                SET_ERROR(ERR_INTERNAL_VAL);
                continue;
            }
        }

        /* check the type Q in this submodule */
        grp = ncx_find_grouping_que(&inc->submod->groupingQ, grpname);
        if (grp) {
            return grp;
        }
    }

    return NULL;

}   /* ncx_find_grouping */


/********************************************************************
* FUNCTION ncx_find_grouping_que
*
* Check if a grp_template_t in the specified Q
*
* INPUTS:
*   groupingQ == Queue of grp_template_t to check
*   grpname == group name
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
grp_template_t *
    ncx_find_grouping_que (const dlq_hdr_t *groupingQ,
                           const xmlChar *grpname)
{
    grp_template_t *grp;

#ifdef DEBUG
    if (!groupingQ || !grpname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (grp = (grp_template_t *)dlq_firstEntry(groupingQ);
         grp != NULL;
         grp = (grp_template_t *)dlq_nextEntry(grp)) {
        if (grp->name && !xml_strcmp(grp->name, grpname)) {
            return grp;
        }
    }
    return NULL;

}   /* ncx_find_grouping_que */


/********************************************************************
* FUNCTION ncx_find_rpc
*
* Check if a rpc_template_t in the mod->rpcQ
*
* INPUTS:
*   mod == ncx_module to check
*   rpcname == RPC name
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
obj_template_t *
    ncx_find_rpc (const ncx_module_t *mod,
                  const xmlChar *rpcname)
{
    obj_template_t *rpc;

#ifdef DEBUG
    if (!mod || !rpcname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (rpc = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         rpc != NULL;
         rpc = (obj_template_t *)dlq_nextEntry(rpc)) {
        if (rpc->objtype == OBJ_TYP_RPC) {
            if (!xml_strcmp(obj_get_name(rpc), rpcname)) {
                return rpc;
            }
        }
    }
    return NULL;

}   /* ncx_find_rpc */


/********************************************************************
* FUNCTION ncx_match_rpc
*
* Check if a rpc_template_t in the mod->rpcQ
*
* INPUTS:
*   mod == ncx_module to check
*   rpcname == RPC name to match
*   retcount == address of return match count
*
* OUTPUTS:
*   *retcount == number of matches found
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
obj_template_t *
    ncx_match_rpc (const ncx_module_t *mod,
                   const xmlChar *rpcname,
                   uint32 *retcount)
{
    obj_template_t *rpc, *firstfound;
    uint32          len, cnt;

#ifdef DEBUG
    if (!mod || !rpcname || !retcount) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    *retcount = 0;
    cnt = 0;
    firstfound = NULL;
    len = xml_strlen(rpcname);

    for (rpc = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         rpc != NULL;
         rpc = (obj_template_t *)dlq_nextEntry(rpc)) {
        if (rpc->objtype == OBJ_TYP_RPC) {
            if (!xml_strncmp(obj_get_name(rpc), rpcname, len)) {
                if (firstfound == NULL) {
                    firstfound = rpc;
                }
                cnt++;
            }
        }
    }

    *retcount = cnt;
    return firstfound;

}   /* ncx_match_rpc */


/********************************************************************
* FUNCTION ncx_match_any_rpc
*
* Check if a rpc_template_t in in any module that
* matches the rpc name string and maybe the owner
*
* INPUTS:
*   module == module name to check (NULL == check all)
*   rpcname == RPC name to match
*   retcount == address of return count of matches
*
* OUTPUTS:
*   *retcount == number of matches found
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
obj_template_t *
    ncx_match_any_rpc (const xmlChar *module,
                       const xmlChar *rpcname,
                       uint32 *retcount)
{
    obj_template_t *rpc, *firstfound;
    ncx_module_t   *mod;
    uint32          cnt, tempcnt;

#ifdef DEBUG
    if (!rpcname || !retcount) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    firstfound = NULL;
    *retcount = 0;

    if (module) {
        mod = ncx_find_module(module, NULL);
        if (mod) {
            firstfound = ncx_match_rpc(mod, rpcname, retcount);
        }
    } else {
        cnt = 0;
        for (mod = ncx_get_first_module();
             mod != NULL;
             mod =  ncx_get_next_module(mod)) {

            tempcnt = 0;
            rpc = ncx_match_rpc(mod, rpcname, &tempcnt);
            if (rpc) {
                if (firstfound == NULL) {
                    firstfound = rpc;
                }
                cnt += tempcnt;
            }
        }
        *retcount = cnt;
    }

    return firstfound;

}   /* ncx_match_any_rpc */


/********************************************************************
* FUNCTION ncx_find_any_object
*
* Check if an obj_template_t in in any module that
* matches the object name string
*
* INPUTS:
*   objname == object name to match
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
obj_template_t *
    ncx_find_any_object (const xmlChar *objname)
{
    obj_template_t *obj;
    ncx_module_t   *mod;
    boolean         useses;

#ifdef DEBUG
    if (!objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    obj = NULL;
    mod = NULL;
    useses = FALSE;

    if (ncx_sesmodQ != NULL) {
        mod = (ncx_module_t *)dlq_firstEntry(ncx_sesmodQ);
        if (mod != NULL) {
            useses = TRUE;
        }
    }
    if (mod == NULL) {
        mod = ncx_get_first_module();
    }

    for (;
         mod != NULL;
         mod =  ncx_get_next_module(mod)) {

        obj = obj_find_template_top(mod, 
                                    ncx_get_modname(mod), 
                                    objname);
        if (obj) {
            return obj;
        }
    }

    if (useses) {
        /* make 1 more loop trying the main moduleQ */
        for (mod = ncx_get_first_module();
             mod != NULL;
             mod = ncx_get_next_module(mod)) {

            obj = obj_find_template_top(mod, 
                                        ncx_get_modname(mod), 
                                        objname);
            if (obj) {
                return obj;
            }
        }
    }

    return NULL;

}   /* ncx_find_any_object */


/********************************************************************
* FUNCTION ncx_find_any_object_que
*
* Check if an obj_template_t in in any module that
* matches the object name string
*
* INPUTS:
*   modQ == Q of modules to check
*   objname == object name to match
*
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
obj_template_t *
    ncx_find_any_object_que (dlq_hdr_t *modQ,
                             const xmlChar *objname)
{
    obj_template_t *obj;
    ncx_module_t   *mod;

#ifdef DEBUG
    if (!modQ || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    obj = NULL;
    for (mod = (ncx_module_t *)dlq_firstEntry(modQ);
         mod != NULL;
         mod = (ncx_module_t *)dlq_nextEntry(mod)) {

        obj = obj_find_template_top(mod, 
                                    ncx_get_modname(mod), 
                                    objname);
        if (obj) {
            return obj;
        }
    }
    return NULL;

}   /* ncx_find_any_object_que */


/********************************************************************
* FUNCTION ncx_find_object
*
* Find a top level module object
*
* INPUTS:
*   mod == ncx_module to check
*   typname == type name
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
obj_template_t *
    ncx_find_object (ncx_module_t *mod,
                     const xmlChar *objname)
{
#ifdef DEBUG
    if (!mod || !objname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    return obj_find_template_top(mod, mod->name, objname);

}  /* ncx_find_object */


/********************************************************************
* FUNCTION ncx_add_namespace_to_registry
*
* Add the namespace and prefix to the registry
* or retrieve it if already set
* 
* INPUTS:
*   mod == module to add to registry
*   tempmod == TRUE if this is a temporary add mode
*              FALSE if this is a real registry add
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_add_namespace_to_registry (ncx_module_t *mod,
                                   boolean tempmod)
{
    xmlns_t        *ns;
    xmlChar        *buffer, *p;
    const xmlChar  *modname;
    status_t        res;
    xmlns_id_t      nsid;
    uint32          prefixlen, i;
    boolean         isnetconf;

#ifdef DEBUG
    if (!mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (!mod->ismod) {
        return NO_ERR;
    }

    if (mod->ns == NULL) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    res = NO_ERR;
    isnetconf = FALSE;

    /* if this is the XSD module, then use the NS ID already registered */
    if (!xml_strcmp(mod->name, NCX_EL_XSD)) {
        mod->nsid = xmlns_xs_id();
    } else if (!xml_strcmp(mod->name,
                           (const xmlChar *)"ietf-netconf")) {
        mod->nsid = xmlns_nc_id();
        isnetconf = TRUE;
    } else {
        mod->nsid = xmlns_find_ns_by_module(mod->name);
    }

    if (!tempmod) {
        /* check module prefix collision */
        nsid = xmlns_find_ns_by_prefix(mod->prefix);
        if (nsid) {
            modname = xmlns_get_module(nsid);
            if (xml_strcmp(mod->name, modname) && !isnetconf) {
                if (ncx_warning_enabled(ERR_NCX_DUP_PREFIX)) {
                    log_warn("\nWarning: prefix '%s' already in use "
                             "by module '%s'",
                             mod->prefix, 
                             modname);
                    ncx_print_errormsg(NULL, mod, ERR_NCX_DUP_PREFIX);
                }
                
                /* redo the module xmlprefix */
                prefixlen = xml_strlen(mod->prefix);
                buffer = m__getMem(prefixlen + 6);
                if (!buffer) {
                    return ERR_INTERNAL_MEM;
                }
                p = buffer;
                p += xml_strcpy(p, mod->prefix);

                /* keep adding numbers to end of prefix until
                 * 1 is unused or run out of numbers
                 */
                for (i=1; i<10000 && nsid; i++) {
                    sprintf((char *)p, "%u", i);
                    nsid = xmlns_find_ns_by_prefix(buffer);
                }
                if (nsid) {
                    log_error("\nError: could not assign module prefix");
                    res = ERR_NCX_OPERATION_FAILED;
                    ncx_print_errormsg(NULL, mod, res);
                    m__free(buffer);
                    return res;
                }

                /* else the current buffer contains an unused prefix */
                mod->xmlprefix = buffer;
            }
        }
    }

    ns = def_reg_find_ns(mod->ns);
    if (ns) {
        if (tempmod) {
            mod->nsid = ns->ns_id;
        } else if (isnetconf) {
            ;
        } else if (xml_strcmp(mod->name, ns->ns_module) &&
                   xml_strcmp(ns->ns_module, NCX_MODULE)) {
            /* this NS string already registered to another module */
            log_error("\nncx reg: Module '%s' registering "
                      "duplicate namespace '%s'\n    "
                      "registered by module '%s'",
                      mod->name, 
                      mod->ns, 
                      ns->ns_module);
            return ERR_DUP_NS;
        } else {
            /* same owner so okay */
            mod->nsid = ns->ns_id;
        }
    } else if (!isnetconf) {
        res = xmlns_register_ns(mod->ns, 
                                (mod->xmlprefix) 
                                ? mod->xmlprefix : mod->prefix, 
                                mod->name, 
                                (tempmod) ? NULL : mod, 
                                &mod->nsid);
        if (res != NO_ERR) {
            /* this NS registration failed */
            log_error("\nncx reg: Module '%s' registering "
                      "namespace '%s' failed (%s)",
                      mod->name, 
                      mod->ns,
                      get_error_string(res));
            return res;
        }
    }

    return res;

}  /* ncx_add_namespace_to_registry */


/********************************************************************
* FUNCTION ncx_add_to_registry
*
* Add all the definitions stored in an ncx_module_t to the registry
* This step is deferred to keep the registry stable as possible
* and only add modules in an all-or-none fashion.
* 
* INPUTS:
*   mod == module to add to registry
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_add_to_registry (ncx_module_t *mod)
{
    yang_node_t    *node;
    status_t        res;

#ifdef DEBUG
    if (!mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    /* check module parse code */
    if (mod->status != NO_ERR) {
        res = mod->status;
        if (NEED_EXIT(res)) {
            /* should not happen */
            log_error("\nError: cannot add module '%s' to registry"
                      " with fatal errors", 
                      mod->name);
            ncx_print_errormsg(NULL, mod, res);
            return SET_ERROR(ERR_INTERNAL_VAL);
        } else {
            log_debug2("\nAdding module '%s' to registry"
                       " with errors", 
                       mod->name);
            res = NO_ERR;
        }
    }

    res = set_toplevel_defs(mod, mod->nsid);
    if (res != NO_ERR) {
        return res;
    }

    /* add all the submodules included in this module */
    for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
         node != NULL;
         node = (yang_node_t *)dlq_nextEntry(node)) {

        node->submod->nsid = mod->nsid;
        res = set_toplevel_defs(node->submod, mod->nsid);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* add the module itself for fast lookup in imports
     * of other modules
     */
    if (mod->ismod) {
        /* save the module in the module Q */
        add_to_modQ(mod, &ncx_modQ);
        mod->added = TRUE;

        /* !!! hack to cleanup after xmlns init cycle !!!
         * check for netconf.yang or ncx.yang and back-fill
         * all the xmlns entries for those modules with the
         * real module pointer
         */
        if (!xml_strcmp(mod->name, NC_MODULE)) {
            xmlns_set_modptrs(NC_MODULE, mod);
        } else if (!xml_strcmp(mod->name, NCX_MODULE)) {
            xmlns_set_modptrs(NCX_MODULE, mod);
        }

        if (mod_load_callback) {
            (*mod_load_callback)(mod);
        }

    }
    
    return res;

}  /* ncx_add_to_registry */


/********************************************************************
* FUNCTION ncx_add_to_modQ
*
* Add module to the current module Q
* Used by yangdiff to bypass add_to_registry to support
* N different module trees
*
* INPUTS:
*   mod == module to add to current module Q
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_add_to_modQ (ncx_module_t *mod)
{
#ifdef DEBUG
    if (!mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    add_to_modQ(mod, ncx_curQ);
    mod->added = TRUE;
    return NO_ERR;

} /* ncx_add_to_modQ */


/********************************************************************
* FUNCTION ncx_is_duplicate  (NCX only)
* 
* Search the specific module for the specified definition name.
* This function is for modules in progress which have not been
* added to the registry yet.
*
* INPUTS:
*     mod == ncx_module_t to check
*     defname == name of definition to find
* RETURNS:
*    TRUE if found, FALSE otherwise
*********************************************************************/
boolean
    ncx_is_duplicate (ncx_module_t *mod,
                      const xmlChar *defname)
{
#ifdef DEBUG
    if (!mod || !defname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (ncx_find_type(mod, defname)) {
        return TRUE;
    }
    if (ncx_find_rpc(mod, defname)) {
        return TRUE;
    }
    return FALSE;

}  /* ncx_is_duplicate */


/********************************************************************
* FUNCTION ncx_get_first_module
* 
* Get the first module in the ncx_modQ
* 
* RETURNS:
*   pointer to the first entry or NULL if empty Q
*********************************************************************/
ncx_module_t *
    ncx_get_first_module (void)
{
    ncx_module_t *mod;

    mod = (ncx_module_t *)dlq_firstEntry(ncx_curQ);
    while (mod) {
        if (mod->defaultrev) {
            return mod;
        }
        mod = (ncx_module_t *)dlq_nextEntry(mod);
    }
    return mod;

}  /* ncx_get_first_module */


/********************************************************************
* FUNCTION ncx_get_next_module
* 
* Get the next module in the ncx_modQ
* 
* RETURNS:
*   pointer to the first entry or NULL if empty Q
*********************************************************************/
ncx_module_t *
    ncx_get_next_module (const ncx_module_t *mod)
{
    ncx_module_t *nextmod;

#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    nextmod = (ncx_module_t *)dlq_nextEntry(mod);
    while (nextmod) {
        if (nextmod->defaultrev) {
            return nextmod;
        }
        nextmod = (ncx_module_t *)dlq_nextEntry(nextmod);
    }
    return nextmod;

}  /* ncx_get_next_module */


/********************************************************************
* FUNCTION ncx_get_first_session_module
* 
* Get the first module in the ncx_sesmodQ
* 
* RETURNS:
*   pointer to the first entry or NULL if empty Q
*********************************************************************/
ncx_module_t *
    ncx_get_first_session_module (void)
{
    ncx_module_t *mod;

    if (ncx_sesmodQ == NULL) {
        return NULL;
    }

    mod = (ncx_module_t *)dlq_firstEntry(ncx_sesmodQ);
    return mod;

}  /* ncx_get_first_session_module */


/********************************************************************
* FUNCTION ncx_get_next_session_module
* 
* Get the next module in the ncx_sesmodQ
* 
* RETURNS:
*   pointer to the first entry or NULL if empty Q
*********************************************************************/
ncx_module_t *
    ncx_get_next_session_module (const ncx_module_t *mod)
{
    ncx_module_t *nextmod;

#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    nextmod = (ncx_module_t *)dlq_nextEntry(mod);
    return nextmod;

}  /* ncx_get_next_session_module */


/********************************************************************
* FUNCTION ncx_get_modname
* 
* Get the main module name
* 
* RETURNS:
*   main module name or NULL if error
*********************************************************************/
const xmlChar *
    ncx_get_modname (const ncx_module_t *mod)
{
#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif
    return (mod->ismod) ? mod->name : mod->belongs;

}  /* ncx_get_modname */


/********************************************************************
* FUNCTION ncx_get_modversion
* 
* Get the [sub]module version
* 
* RETURNS:
*   module version or NULL if error
*********************************************************************/
const xmlChar *
    ncx_get_modversion (const ncx_module_t *mod)
{
#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif
    return mod->version;

}  /* ncx_get_modversion */


/********************************************************************
* FUNCTION ncx_get_modnamespace
* 
* Get the module namespace
* 
* RETURNS:
*   module namespace or NULL if error
*********************************************************************/
const xmlChar *
    ncx_get_modnamespace (const ncx_module_t *mod)
{
#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif
    return mod->ns;

}  /* ncx_get_modnamespace */


/********************************************************************
* FUNCTION ncx_get_mainmod
* 
* Get the main module
* 
* RETURNS:
*   main module NULL if error
*********************************************************************/
ncx_module_t *
    ncx_get_mainmod (ncx_module_t *mod)
{

#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif
    if (mod->ismod) {
        return mod;
    }

    /**** DO NOT KNOW THE REAL MAIN MODULE REVISION ****/
    return ncx_find_module(mod->belongs, NULL);

}  /* ncx_get_mainmod */


/********************************************************************
* FUNCTION ncx_get_first_object
* 
* Get the first object in the datadefQs for the specified module
* Get any object with a name
* 
* INPUTS:
*   mod == module to search for the first object
*
* RETURNS:
*   pointer to the first object or NULL if empty Q
*********************************************************************/
obj_template_t *
    ncx_get_first_object (ncx_module_t *mod)
{
    obj_template_t *obj;
    yang_node_t    *node;

#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {
        if (!obj_has_name(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) || 
            obj_is_abstract(obj)) {
            continue;
        }
        return obj;
    }

    for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
         node != NULL;
         node = (yang_node_t *)dlq_nextEntry(node)) {

        if (!node->submod) {
            SET_ERROR(ERR_INTERNAL_PTR);
            continue;
        }

        for (obj = (obj_template_t *)
                 dlq_firstEntry(&node->submod->datadefQ);
             obj != NULL;
             obj = (obj_template_t *)dlq_nextEntry(obj)) {

            if (!obj_has_name(obj)  || 
                !obj_is_enabled(obj) ||
                obj_is_cli(obj) ||
                obj_is_abstract(obj)) {
                continue;
            }

            return obj;
        }
    }

    return NULL;

}  /* ncx_get_first_object */


/********************************************************************
* FUNCTION ncx_get_next_object
* 
* Get the next object in the specified module
* Get any object with a name
*
* RETURNS:
*   pointer to the next object or NULL if none
*********************************************************************/
obj_template_t *
    ncx_get_next_object (ncx_module_t *mod,
                         obj_template_t *curobj)
{
    obj_template_t *obj;
    yang_node_t    *node;
    boolean         start;

#ifdef DEBUG
    if (!mod || !curobj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (obj = (obj_template_t *)dlq_nextEntry(curobj);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) || 
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        return obj;
    }

    start = (curobj->tkerr.mod == mod) ? TRUE : FALSE;

    for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
         node != NULL;
         node = (yang_node_t *)dlq_nextEntry(node)) {

        if (!node->submod) {
            SET_ERROR(ERR_INTERNAL_PTR);
            continue;
        }

        if (!start) {
            if (node->submod == curobj->tkerr.mod) {
                start = TRUE;
            }
            continue;
        }

        for (obj = (obj_template_t *)
                 dlq_firstEntry(&node->submod->datadefQ);
             obj != NULL;
             obj = (obj_template_t *)dlq_nextEntry(obj)) {

            if (!obj_has_name(obj) || 
                !obj_is_enabled(obj) ||
                obj_is_cli(obj) ||
                obj_is_abstract(obj)) {
                continue;
            }

            return obj;
        }
    }

    return NULL;

}  /* ncx_get_next_object */


/********************************************************************
* FUNCTION ncx_get_first_data_object
* 
* Get the first database object in the datadefQs 
* for the specified module
* 
* INPUTS:
*   mod == module to search for the first object
*
* RETURNS:
*   pointer to the first object or NULL if empty Q
*********************************************************************/
obj_template_t *
    ncx_get_first_data_object (ncx_module_t *mod)
{
    obj_template_t *obj;
    yang_node_t    *node;

#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {
        if (!obj_has_name(obj) || 
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }
        if (obj_is_data_db(obj)) {
            return obj;
        }
    }

    for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
         node != NULL;
         node = (yang_node_t *)dlq_nextEntry(node)) {

        if (!node->submod) {
            SET_ERROR(ERR_INTERNAL_PTR);
            continue;
        }

        for (obj = (obj_template_t *)
                 dlq_firstEntry(&node->submod->datadefQ);
             obj != NULL;
             obj = (obj_template_t *)dlq_nextEntry(obj)) {

            if (!obj_has_name(obj) || 
                !obj_is_enabled(obj) ||
                obj_is_cli(obj) ||
                obj_is_abstract(obj)) {
                continue;
            }

            if (obj_is_data_db(obj)) {
                return obj;
            }
        }
    }

    return NULL;

}  /* ncx_get_first_data_object */


/********************************************************************
* FUNCTION ncx_get_next_data_object
* 
* Get the next database object in the specified module
* 
* RETURNS:
*   pointer to the next object or NULL if none
*********************************************************************/
obj_template_t *
    ncx_get_next_data_object (ncx_module_t *mod,
                              obj_template_t *curobj)
{
    obj_template_t *obj;
    yang_node_t    *node;
    boolean         start;

#ifdef DEBUG
    if (!mod || !curobj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (obj = (obj_template_t *)dlq_nextEntry(curobj);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) || 
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        if (obj_is_data_db(obj)) {
            return obj;
        }
    }

    start = (curobj->tkerr.mod == mod) ? TRUE : FALSE;

    for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
         node != NULL;
         node = (yang_node_t *)dlq_nextEntry(node)) {

        if (!node->submod) {
            SET_ERROR(ERR_INTERNAL_PTR);
            continue;
        }

        if (!start) {
            if (node->submod == curobj->tkerr.mod) {
                start = TRUE;
            }
            continue;
        }

        for (obj = (obj_template_t *)
                 dlq_firstEntry(&node->submod->datadefQ);
             obj != NULL;
             obj = (obj_template_t *)dlq_nextEntry(obj)) {

            if (!obj_has_name(obj) || 
                !obj_is_enabled(obj) ||
                obj_is_cli(obj) ||
                obj_is_abstract(obj)) {
                continue;
            }

            if (obj_is_data_db(obj)) {
                return obj;
            }
        }
    }

    return NULL;

}  /* ncx_get_next_data_object */


/********************************************************************
* FUNCTION ncx_new_import
* 
* Malloc and initialize the fields in a ncx_import_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
ncx_import_t * 
    ncx_new_import (void)
{
    ncx_import_t  *import;

    import = m__getObj(ncx_import_t);
    if (!import) {
        return NULL;
    }
    (void)memset(import, 0x0, sizeof(ncx_import_t));
    dlq_createSQue(&import->appinfoQ);
    return import;

}  /* ncx_new_import */


/********************************************************************
* FUNCTION ncx_free_import
* 
* Scrub the memory in a ncx_import_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    import == ncx_import_t data structure to free
*********************************************************************/
void 
    ncx_free_import (ncx_import_t *import)
{
#ifdef DEBUG
    if (!import) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (import->module) {
        m__free(import->module);
    }

    if (import->prefix) {
        m__free(import->prefix);
    }

    if (import->revision) {
        m__free(import->revision);
    }

    /* YANG only */
    ncx_clean_appinfoQ(&import->appinfoQ);

    m__free(import);

}  /* ncx_free_import */


/********************************************************************
* FUNCTION ncx_find_import
* 
* Search the importQ for a specified module name
* 
* INPUTS:
*   mod == module to search (mod->importQ)
*   module == module name to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_import_t * 
    ncx_find_import (const ncx_module_t *mod,
                     const xmlChar *module)
{
#ifdef DEBUG
    if (!mod || !module) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    return ncx_find_import_que(&mod->importQ, module);

} /* ncx_find_import */


/********************************************************************
* FUNCTION ncx_find_import_que
* 
* Search the specified importQ for a specified module name
* 
* INPUTS:
*   importQ == Q of ncx_import_t to search
*   module == module name to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_import_t * 
    ncx_find_import_que (const dlq_hdr_t *importQ,
                         const xmlChar *module)
{
    ncx_import_t  *import;

#ifdef DEBUG
    if (!importQ || !module) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (import = (ncx_import_t *)dlq_firstEntry(importQ);
         import != NULL;
         import = (ncx_import_t *)dlq_nextEntry(import)) {
        if (!xml_strcmp(import->module, module)) {
            import->used = TRUE;
            return import;
        }
    }
    return NULL;

} /* ncx_find_import_que */


/********************************************************************
* FUNCTION ncx_find_import_test
* 
* Search the importQ for a specified module name
* Do not set used flag
*
* INPUTS:
*   mod == module to search (mod->importQ)
*   module == module name to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_import_t * 
    ncx_find_import_test (const ncx_module_t *mod,
                          const xmlChar *module)
{
    ncx_import_t  *import;

#ifdef DEBUG
    if (!mod || !module) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (import = (ncx_import_t *)dlq_firstEntry(&mod->importQ);
         import != NULL;
         import = (ncx_import_t *)dlq_nextEntry(import)) {
        if (!xml_strcmp(import->module, module)) {
            return import;
        }
    }
    return NULL;

} /* ncx_find_import_test */


/********************************************************************
* FUNCTION ncx_find_pre_import
* 
* Search the importQ for a specified prefix value
* 
* INPUTS:
*   mod == module to search (mod->importQ)
*   prefix == prefix string to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_import_t * 
    ncx_find_pre_import (const ncx_module_t *mod,
                         const xmlChar *prefix)
{
#ifdef DEBUG
    if (!mod || !prefix) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    return ncx_find_pre_import_que(&mod->importQ, prefix);

} /* ncx_find_pre_import */


/********************************************************************
* FUNCTION ncx_find_pre_import_que
* 
* Search the specified importQ for a specified prefix value
* 
* INPUTS:
*   importQ == Q of ncx_import_t to search
*   prefix == prefix string to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_import_t * 
    ncx_find_pre_import_que (const dlq_hdr_t *importQ,
                             const xmlChar *prefix)
{
    ncx_import_t  *import;

#ifdef DEBUG
    if (!importQ || !prefix) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (import = (ncx_import_t *)dlq_firstEntry(importQ);
         import != NULL;
         import = (ncx_import_t *)dlq_nextEntry(import)) {
        if (import->prefix && !xml_strcmp(import->prefix, prefix)) {
            import->used = TRUE;
            return import;
        }
    }
    return NULL;

} /* ncx_find_pre_import_que */


/********************************************************************
* FUNCTION ncx_find_pre_import_test
* 
* Search the importQ for a specified prefix value
* Test only, do not set used flag
*
* INPUTS:
*   mod == module to search (mod->importQ)
*   prefix == prefix string to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_import_t * 
    ncx_find_pre_import_test (const ncx_module_t *mod,
                              const xmlChar *prefix)
{
    ncx_import_t  *import;

#ifdef DEBUG
    if (!mod || !prefix) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (import = (ncx_import_t *)dlq_firstEntry(&mod->importQ);
         import != NULL;
         import = (ncx_import_t *)dlq_nextEntry(import)) {
        if (import->prefix && !xml_strcmp(import->prefix, prefix)) {
            return import;
        }
    }
    return NULL;

}  /* ncx_find_pre_import_test */


/********************************************************************
* FUNCTION ncx_locate_modqual_import
* 
* Search the specific module for the specified definition name.
*
* Okay for YANG or NCX
*
*  - typ_template_t (NCX_NT_TYP)
*  - grp_template_t (NCX_NT_GRP)
*  - obj_template_t (NCX_NT_OBJ)
*  - rpc_template_t  (NCX_NT_RPC)
*  - not_template_t (NCX_NT_NOTIF)
*
* INPUTS:
*     pcb == parser control block to use
*     imp == NCX import struct to use
*     defname == name of definition to find
*     *deftyp == specified type or NCX_NT_NONE if any will do
*
* OUTPUTS:
*    imp->mod may get set if not already
*    *deftyp == type retrieved if NO_ERR
*
* RETURNS:
*    pointer to the located definition or NULL if not found
*********************************************************************/
void *
    ncx_locate_modqual_import (yang_pcb_t *pcb,
                               ncx_import_t *imp,
                               const xmlChar *defname,
                               ncx_node_t *deftyp)
{
    void *dptr;
    status_t  res;

#ifdef DEBUG
    if (!imp || !defname || !deftyp) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    res = check_moddef(pcb, imp, defname, deftyp, &dptr);
    return (res==NO_ERR) ? dptr : NULL;
    /*** error res is lost !!! ***/

}  /* ncx_locate_modqual_import */


/********************************************************************
* FUNCTION ncx_new_include
* 
* Malloc and initialize the fields in a ncx_include_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
ncx_include_t * 
    ncx_new_include (void)
{
    ncx_include_t  *inc;

    inc = m__getObj(ncx_include_t);
    if (!inc) {
        return NULL;
    }
    (void)memset(inc, 0x0, sizeof(ncx_include_t));
    dlq_createSQue(&inc->appinfoQ);
    return inc;

}  /* ncx_new_include */


/********************************************************************
* FUNCTION ncx_free_include
* 
* Scrub the memory in a ncx_include_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    inc == ncx_include_t data structure to free
*********************************************************************/
void 
    ncx_free_include (ncx_include_t *inc)
{
#ifdef DEBUG
    if (!inc) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (inc->submodule) {
        m__free(inc->submodule);
    }

    if (inc->revision) {
        m__free(inc->revision);
    }

    ncx_clean_appinfoQ(&inc->appinfoQ);
    m__free(inc);

}  /* ncx_free_include */


/********************************************************************
* FUNCTION ncx_find_include
* 
* Search the includeQ for a specified submodule name
* 
* INPUTS:
*   mod == module to search (mod->includeQ)
*   submodule == submodule name to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_include_t * 
    ncx_find_include (const ncx_module_t *mod,
                      const xmlChar *submodule)
{
    ncx_include_t  *inc;

#ifdef DEBUG
    if (!mod || !submodule) {
        return NULL;
    }
#endif

    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
         inc != NULL;
         inc = (ncx_include_t *)dlq_nextEntry(inc)) {
        if (!xml_strcmp(inc->submodule, submodule)) {
            return inc;
        }
    }
    return NULL;

} /* ncx_find_include */


/********************************************************************
* FUNCTION ncx_init_num
* 
* Init a ncx_num_t struct
*
* INPUTS:
*     num == number to initialize
*********************************************************************/
void
    ncx_init_num (ncx_num_t *num)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    memset(num, 0x0, sizeof(ncx_num_t));

}  /* ncx_init_num */


/********************************************************************
* FUNCTION ncx_clean_num
* 
* Scrub the memory in a ncx_num_t by freeing all
* the sub-fields. DOES NOT free the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    btyp == base type of number
*    num == ncx_num_t data structure to clean
*********************************************************************/
void 
    ncx_clean_num (ncx_btype_t btyp,
                   ncx_num_t *num)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;  
    }
#endif

    /* clean the num->union, depending on base type */
    switch (btyp) {
    case NCX_BT_NONE:
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
    case NCX_BT_INT64:
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
    case NCX_BT_UINT64:
        memset(num, 0x0, sizeof(ncx_num_t));
        break;
    case NCX_BT_DECIMAL64:
        num->dec.val = 0;
        num->dec.digits = 0;
        break;
    case NCX_BT_FLOAT64:
        num->d = 0;
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* ncx_clean_num */


/********************************************************************
* FUNCTION ncx_compare_nums
* 
* Compare 2 ncx_num_t union contents
*
* INPUTS:
*     num1 == first number
*     num2 == second number
*     btyp == expected data type (NCX_BT_INT, UINT, REAL)
* RETURNS:
*     -1 if num1 is < num2
*      0 if num1 == num2
*      1 if num1 is > num2
*********************************************************************/
int32
    ncx_compare_nums (const ncx_num_t *num1,
                      const ncx_num_t *num2,
                      ncx_btype_t  btyp)
{
    int64    temp1, temp2;
    uint8    diffdigits;

#ifdef DEBUG
    if (!num1 || !num2) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        if (num1->i < num2->i) {
            return -1;
        } else if (num1->i == num2->i) {
            return 0;
        } else {
            return 1;
        }
    case NCX_BT_INT64:
        if (num1->l < num2->l) {
            return -1;
        } else if (num1->l == num2->l) {
            return 0;
        } else {
            return 1;
        }
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        if (num1->u < num2->u) {
            return -1;
        } else if (num1->u == num2->u) {
            return 0;
        } else {
            return 1;
        }
    case NCX_BT_UINT64:
        if (num1->ul < num2->ul) {
            return -1;
        } else if (num1->ul == num2->ul) {
            return 0;
        } else {
            return 1;
        }
    case NCX_BT_DECIMAL64:
        /* check the base parts first */
        temp1 = ncx_get_dec64_base(num1);
        temp2 = ncx_get_dec64_base(num2);

        if (temp1 < temp2) {
            return -1;
        } else if (temp1 == temp2) {
            /* check fraction parts next */
            temp1 = ncx_get_dec64_fraction(num1);
            temp2 = ncx_get_dec64_fraction(num2);

            /* normalize these numbers to compare them */
            if (num1->dec.digits > num2->dec.digits) {
                diffdigits = num1->dec.digits - num2->dec.digits;
                temp2 *= (10 * diffdigits);
            } else if (num1->dec.digits < num2->dec.digits) {
                diffdigits = num2->dec.digits - num1->dec.digits;
                temp1 *= (10 * diffdigits);
            }

            if (temp1 < temp2) {
                return -1;
            } else if (temp1 == temp2) {
                return 0;
            } else {
                return 1;
            }
        } else {
            return 1;
        }
    case NCX_BT_FLOAT64:
        if (num1->d < num2->d) {
            return -1;
        } else if (num1->d == num2->d) {
            return 0;
        } else {
            return 1;
        }
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return 0;
    }
    /*NOTREACHED*/
}  /* ncx_compare_nums */


/********************************************************************
* FUNCTION ncx_set_num_min
* 
* Set a number to the minimum value for its type
*
* INPUTS:
*     num == number to set
*     btyp == expected data type
*
*********************************************************************/
void
    ncx_set_num_min (ncx_num_t *num,
                     ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
        num->i = NCX_MIN_INT8;
        break;
    case NCX_BT_INT16:
        num->i = NCX_MIN_INT16;
        break;
    case NCX_BT_INT32:
        num->i = NCX_MIN_INT;
        break;
    case NCX_BT_INT64:
        num->l = NCX_MIN_LONG;
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        num->u = NCX_MIN_UINT;
        break;
    case NCX_BT_UINT64:
        num->ul = NCX_MIN_ULONG;
        break;
    case NCX_BT_DECIMAL64:
        num->dec.val = NCX_MIN_LONG;
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        num->d = -INFINITY;
#else
        num->d = NCX_MIN_LONG;
#endif
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* ncx_set_num_min */


/********************************************************************
* FUNCTION ncx_set_num_max
* 
* Set a number to the maximum value for its type
*
* INPUTS:
*     num == number to set
*     btyp == expected data type
*
*********************************************************************/
void
    ncx_set_num_max (ncx_num_t *num,
                     ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
        num->i = NCX_MAX_INT8;
        break;
    case NCX_BT_INT16:
        num->i = NCX_MAX_INT16;
        break;
    case NCX_BT_INT32:
        num->i = NCX_MAX_INT;
        break;
    case NCX_BT_INT64:
        num->l = NCX_MAX_LONG;
        break;
    case NCX_BT_UINT8:
        num->u = NCX_MAX_UINT8;
        break;
    case NCX_BT_UINT16:
        num->u = NCX_MAX_UINT16;
        break;
    case NCX_BT_UINT32:
        num->u = NCX_MAX_UINT;
        break;
    case NCX_BT_UINT64:
        num->ul = NCX_MAX_ULONG;
        break;
    case NCX_BT_DECIMAL64:
        num->dec.val = NCX_MAX_LONG;
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        num->d = INFINITY;
#else
        num->d = NCX_MAX_LONG-1;
#endif
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* ncx_set_num_max */


/********************************************************************
* FUNCTION ncx_set_num_one
* 
* Set a number to one
*
* INPUTS:
*     num == number to set
*     btyp == expected data type
*
*********************************************************************/
void
    ncx_set_num_one (ncx_num_t *num,
                     ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        num->i = 1;
        break;
    case NCX_BT_INT64:
        num->l = 1;
        break;
    case NCX_BT_UINT8:
        num->u = 1;
        break;
    case NCX_BT_UINT64:
        num->ul = 1;
        break;
    case NCX_BT_DECIMAL64:
        num->dec.val = 10 * num->dec.digits;
        break;
    case NCX_BT_FLOAT64:
        num->d = 1;
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* ncx_set_num_one */


/********************************************************************
* FUNCTION ncx_set_num_zero
* 
* Set a number to zero
*
* INPUTS:
*     num == number to set
*     btyp == expected data type
*
*********************************************************************/
void
    ncx_set_num_zero (ncx_num_t *num,
                      ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        num->i = 0;
        break;
    case NCX_BT_INT64:
        num->l = 0;
        break;
    case NCX_BT_UINT8:
        num->u = 0;
        break;
    case NCX_BT_UINT64:
        num->ul = 0;
        break;
    case NCX_BT_DECIMAL64:
        num->dec.val = 0;
        break;
    case NCX_BT_FLOAT64:
        num->d = 0;
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* ncx_set_num_zero */


/********************************************************************
* FUNCTION ncx_set_num_nan
* 
* Set a FP number to the Not a Number value
*
* INPUTS:
*     num == number to set
*     btyp == expected data type
*
*********************************************************************/
void
    ncx_set_num_nan (ncx_num_t *num,
                     ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (btyp == NCX_BT_FLOAT64) {
#ifdef HAS_FLOAT
        num->d = NAN;
#else
        num->d = NCX_MAX_LONG;
#endif
    }

} /* ncx_set_num_nan */


/********************************************************************
* FUNCTION ncx_num_is_nan
* 
* Check if a FP number is set to the Not a Number value
*
* INPUTS:
*     num == number to check
*     btyp == expected data type
*
*********************************************************************/
boolean
    ncx_num_is_nan (ncx_num_t *num,
                    ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return TRUE;
    }
#endif

    if (btyp == NCX_BT_FLOAT64) {
#ifdef HAS_FLOAT
        return (num->d == NAN) ? TRUE : FALSE;
#else
        return (num->d == NCX_MAX_LONG) ? TRUE : FALSE;
#endif
    }
    return FALSE;

} /* ncx_num_is_nan */


/********************************************************************
* FUNCTION ncx_num_zero
* 
* Compare a ncx_num_t to zero
*
* INPUTS:
*     num == number to check
*     btyp == expected data type (e.g., NCX_BT_INT32, NCX_BT_UINT64)
*
* RETURNS:
*     TRUE if value is equal to zero
*     FALSE if value is not equal to zero
*********************************************************************/
boolean
    ncx_num_zero (const ncx_num_t *num,
                  ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        return (num->i) ? FALSE : TRUE;
    case NCX_BT_INT64:
        return (num->l) ? FALSE : TRUE;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        return (num->u) ? FALSE : TRUE;
    case NCX_BT_UINT64:
        return (num->ul) ? FALSE : TRUE;
    case NCX_BT_DECIMAL64:
        return (num->dec.val == 0) ? TRUE : FALSE;
    case NCX_BT_FLOAT64:
        return (num->d == 0) ? TRUE : FALSE;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return FALSE;
    }
    /*NOTREACHED*/
}  /* ncx_num_zero */


/********************************************************************
* FUNCTION ncx_convert_num
* 
* Convert a number string to a numeric type
*
* INPUTS:
*     numstr == number string
*     numfmt == NCX_NF_OCTAL, NCX_NF_DEC, NCX_NF_HEX, or NCX_NF_REAL
*     btyp == desired number type 
*             (e.g., NCX_BT_INT32, NCX_BT_UINT32, NCX_BT_FLOAT64)
*     val == pointer to ncx_num_t to hold result
*
* OUTPUTS:
*     *val == converted number value
*
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_convert_num (const xmlChar *numstr,
                     ncx_numfmt_t   numfmt,
                     ncx_btype_t  btyp,
                     ncx_num_t    *val)
{
    char  *err;
    long  l;
    long long  ll;
    unsigned long ul;
    unsigned long long ull;

#ifdef HAS_FLOAT
    double d;
#else
    int64 d;
#endif

#ifdef DEBUG
    if (!numstr || !val) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (*numstr == '\0') {
        return ERR_NCX_INVALID_VALUE;
    }

    err = NULL;
    l = 0;
    ll = 0;
    ul = 0;
    ull = 0;

    /* check the number format set to don't know */
    if (numfmt==NCX_NF_NONE) {
        numfmt = ncx_get_numfmt(numstr);
    }


    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        switch (numfmt) {
        case NCX_NF_OCTAL:
            l = strtol((const char *)numstr, &err, 8);
            break;
        case NCX_NF_DEC:
            l = strtol((const char *)numstr, &err, 10);
            break;
        case NCX_NF_HEX:
            l = strtol((const char *)numstr, &err, 16);
            break;
        case NCX_NF_REAL:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (err && *err) {
            return ERR_NCX_INVALID_NUM;
        }

        switch (btyp) {
        case NCX_BT_INT8:
            if (l < NCX_MIN_INT8 || l > NCX_MAX_INT8) {
                return ERR_NCX_NOT_IN_RANGE;
            }
            break;
        case NCX_BT_INT16:
            if (l < NCX_MIN_INT16 || l > NCX_MAX_INT16) {
                return ERR_NCX_NOT_IN_RANGE;
            }
            break;
        default:
            ;
        }
        val->i = (int32)l;
        break;
    case NCX_BT_INT64:
        switch (numfmt) {
        case NCX_NF_OCTAL:
            ll = strtoll((const char *)numstr, &err, 8);
            break;
        case NCX_NF_DEC:
            ll = strtoll((const char *)numstr, &err, 10);
            break;
        case NCX_NF_HEX:
            ll = strtoll((const char *)numstr, &err, 16);
            break;
        case NCX_NF_REAL:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (err && *err) {
            return ERR_NCX_INVALID_NUM;
        }
        val->l = (int64)ll;
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        switch (numfmt) {
        case NCX_NF_OCTAL:
            ul = strtoul((const char *)numstr, &err, 8);
            break;
        case NCX_NF_DEC:
            ul = strtoul((const char *)numstr, &err, 10);
            break;
        case NCX_NF_HEX:
            ul = strtoul((const char *)numstr, &err, 16);
            break;
        case NCX_NF_REAL:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (err && *err) {
            return ERR_NCX_INVALID_NUM;
        }

        switch (btyp) {
        case NCX_BT_UINT8:
            if (ul > NCX_MAX_UINT8) {
                return ERR_NCX_NOT_IN_RANGE;
            }
            break;
        case NCX_BT_UINT16:
            if (ul > NCX_MAX_UINT16) {
                return ERR_NCX_NOT_IN_RANGE;
            }
            break;
        default:
            ;
        }

        if (*numstr == '-') {
            return ERR_NCX_NOT_IN_RANGE;
        }

        val->u = (uint32)ul;
        break;
    case NCX_BT_UINT64:
        switch (numfmt) {
        case NCX_NF_OCTAL:
            ull = strtoull((const char *)numstr, &err, 8);
            break;
        case NCX_NF_DEC:
            ull = strtoull((const char *)numstr, &err, 10);
            break;
        case NCX_NF_HEX:
            ull = strtoull((const char *)numstr, &err, 16);
            break;
        case NCX_NF_REAL:
            return ERR_NCX_WRONG_TKTYPE;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (err && *err) {
            return ERR_NCX_INVALID_NUM;
        }

        if (*numstr == '-') {
            return ERR_NCX_NOT_IN_RANGE;
        }

        val->ul = (uint64)ull;
        break;
    case NCX_BT_DECIMAL64:
        return SET_ERROR(ERR_INTERNAL_VAL);
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        switch (numfmt) {
        case NCX_NF_OCTAL:
        case NCX_NF_DEC:
        case NCX_NF_REAL:
            errno = 0;
            d = strtod((const char *)numstr, &err);
            if (errno) {
                return ERR_NCX_INVALID_NUM;
            }
            val->d = d;
            break;
        case NCX_NF_HEX:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
#else
        switch (numfmt) {
        case NCX_NF_OCTAL:
            ll = strtoll((const char *)numstr, &err, 8);
            if (err && *err) {
                return ERR_NCX_INVALID_NUM;
            }
            val->d = (int64)ll;
            break;
        case NCX_NF_DEC:
            ll = strtoll((const char *)numstr, &err, 10);
            if (err && *err) {
                return ERR_NCX_INVALID_NUM;
            }
            val->d = (int64)ll;
            break;
        case NCX_NF_HEX:
            ll = strtoll((const char *)numstr, &err, 16);
            if (err && *err) {
                return ERR_NCX_INVALID_HEXNUM;
            }
            val->d = (int64)ll;
            break;
        case NCX_NF_REAL:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
#endif
        break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    return NO_ERR;

}  /* ncx_convert_num */


/********************************************************************
* FUNCTION ncx_convert_dec64
* 
* Convert a number string to a decimal64 number
*
* INPUTS:
*     numstr == number string
*     numfmt == number format used
*     digits == number of fixed-point digits expected
*     val == pointer to ncx_num_t to hold result
*
* OUTPUTS:
*     *val == converted number value
*
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_convert_dec64 (const xmlChar *numstr,
                       ncx_numfmt_t numfmt,
                       uint8 digits,
                       ncx_num_t *val)
{
    const xmlChar  *point, *str;
    char           *err;
    int64           basenum, fracnum, testnum;
    uint32          numdigits;
    boolean         isneg;
    uint8           i;
    xmlChar         numbuff[NCX_MAX_NUMLEN];

#ifdef DEBUG
    if (!numstr || !val) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (*numstr == '\0') {
        val->dec.val = 0;
        val->dec.digits = digits;
        return NO_ERR;
    }

    err = NULL;
    point = NULL;
    basenum = 0;
    fracnum = 0;
    isneg = FALSE;

    /* check the number format set to don't know */
    if (numfmt==NCX_NF_NONE) {
        numfmt = ncx_get_numfmt(numstr);
    }

    /* check the number string for plus or minus sign */
    str = numstr;
    if (*str == '+') {
        str++;
    } else if (*str == '-') {
        str++;
        isneg = TRUE;
    }
    while (isdigit((char)*str)) {
        str++;
    }

    /* check if stopped on a decimal point */
    if (*str == '.') {
        /* get just the base part now */
        point = str;
        xml_strncpy(numbuff, 
                    numstr, 
                    (uint32)(point - numstr));
        basenum = strtoll((const char *)numbuff, &err, 10);
    } else {
        /* assume the entire string is just a base part
         * the token parser broke up the string
         * already so a string concat '123foo' should
         * not happen here
         */
        switch (numfmt) {
        case NCX_NF_OCTAL:
            basenum = strtoll((const char *)numstr, &err, 8);
            break;
        case NCX_NF_DEC:
        case NCX_NF_REAL:
            basenum = strtoll((const char *)numstr, &err, 10);
            break;
        case NCX_NF_HEX:
            basenum = strtoll((const char *)numstr, &err, 16);
            break;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

        /* check if strtoll accepted the number string */
        if (err && *err) {
            return ERR_NCX_INVALID_NUM;
        }
    }

    /* check that the number is actually in range */
    if (isneg) {
        testnum = NCX_MIN_LONG;
    } else {
        testnum = NCX_MAX_LONG;
    }

    /* adjust the test number to the maximum for
     * the specified number of fraction digits
     */
    for (i = 0; i < digits; i++) {
        testnum /= 10;
    }

    /* check if the base number is OK wrt/ testnum */
    if (isneg) {
        if (basenum < testnum) {
            return ERR_NCX_DEC64_BASEOVFL;
        }
    } else {
        if (basenum > testnum) {
            return ERR_NCX_DEC64_BASEOVFL;
        }
    }

    /* check if there is a fraction part entered */
    if (point) {
        fracnum = 0;
        str = point + 1;
        while (isdigit((char)*str)) {
            str++;
        }
        numdigits = (uint32)(str - point - 1);

        /* check if fraction part too big */
        if (numdigits > (uint32)digits) {
            return ERR_NCX_DEC64_FRACOVFL;
        }

        if (numdigits) {
            err = NULL;
            xml_strncpy(numbuff, point+1, numdigits);
            fracnum = strtoll((const char *)numbuff, &err, 10);

            /* check if strtoll accepted the number string */
            if (err && *err) {
                return ERR_NCX_INVALID_NUM;
            }

            /* adjust the fraction part will trailing zeros
             * if the user omitted them
             */
            for (i = numdigits; i < digits; i++) {
                fracnum *= 10;
            }

            if (isneg) {
                fracnum *= -1;
            }
        }
    }

    /* encode the base part shifted left 10 * fraction-digits */
    if (basenum) {
        for (i= 0; i < digits; i++) {
            basenum *= 10;
        }
    }

    /* save the number with the fraction-digits value added in */
    val->dec.val = basenum + fracnum;
    val->dec.digits = digits;

    return NO_ERR;

}  /* ncx_convert_dec64 */


/********************************************************************
* FUNCTION ncx_decode_num
* 
* Handle some sort of number string
*
* INPUTS:
*   numstr == number string
*    btyp == desired number type
*   retnum == pointer to initialized ncx_num_t to hold result
*
* OUTPUTS:
*   *retnum == converted number
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_decode_num (const xmlChar *numstr,
                    ncx_btype_t  btyp,
                    ncx_num_t  *retnum)
{
    const xmlChar *str;

#ifdef DEBUG
    if (!numstr || !retnum) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check if this is a hex number */
    if (*numstr == '0' && NCX_IS_HEX_CH(*(numstr+1))) {
        return ncx_convert_num(numstr+2, 
                               NCX_NF_HEX, 
                               btyp, 
                               retnum);
    }

    /* check if this is a real number */
    str = numstr;
    while (*str && (*str != '.')) {
        str++;
    }
    if (*str) {
        return ncx_convert_num(numstr, NCX_NF_REAL, btyp, retnum);
    }

    /* check octal number */
    if (*numstr == '0' && numstr[1] != '.') {
        return ncx_convert_num(numstr, 
                               NCX_NF_OCTAL, 
                               btyp, 
                               retnum);
    }

    /* else assume this is a decimal number */
    return ncx_convert_num(numstr, 
                           NCX_NF_DEC, 
                           btyp, 
                           retnum);

}  /* ncx_decode_num */


/********************************************************************
* FUNCTION ncx_decode_dec64
* 
* Handle some sort of decimal64 number string (NCX_BT_DECIMAL64)
*
* INPUTS:
*   numstr == number string
*    digits == number of expected digits for this decimal64
*   retnum == pointer to initialized ncx_num_t to hold result
*
* OUTPUTS:
*   *retnum == converted number
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_decode_dec64 (const xmlChar *numstr,
                      uint8  digits,
                      ncx_num_t  *retnum)
{
    const xmlChar *str;

#ifdef DEBUG
    if (!numstr || !retnum) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check if this is a hex number */
    if (*numstr == '0' && NCX_IS_HEX_CH(*(numstr+1))) {
        return ncx_convert_dec64(numstr+2, 
                                 NCX_NF_HEX, 
                                 digits, 
                                 retnum);
    }

    /* check if this is a real number */
    str = numstr;
    while (*str && (*str != '.')) {
        str++;
    }
    if (*str) {
        return ncx_convert_dec64(numstr, 
                                 NCX_NF_REAL, 
                                 digits, 
                                 retnum);
    }

    /* check octal number */
    if (*numstr == '0') {
        return ncx_convert_dec64(numstr, 
                                 NCX_NF_OCTAL, 
                                 digits, 
                                 retnum);
    }

    /* else assume this is a decimal number */
    return ncx_convert_dec64(numstr, 
                             NCX_NF_DEC, 
                             digits, 
                             retnum);

}  /* ncx_decode_dec64 */


/********************************************************************
* FUNCTION ncx_copy_num
* 
* Copy the contents of num1 to num2
*
* Supports all NCX numeric types:
*    NCX_BT_INT*
*    NCX_BT_UINT*
*    NCX_BT_DECIMAL64
*    NCX_BT_FLOAT64
*
* INPUTS:
*     num1 == first number
*     num2 == second number
*     btyp == expected data type (NCX_BT_INT, UINT, REAL)
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_copy_num (const ncx_num_t *num1,
                  ncx_num_t *num2,
                  ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num1 || !num2) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        num2->i = num1->i;
        break;
    case NCX_BT_INT64:
        num2->l = num1->l;
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        num2->u = num1->u;
        break;
    case NCX_BT_UINT64:
        num2->ul = num1->ul;
        break;
    case NCX_BT_DECIMAL64:
        num2->dec.val = num1->dec.val;
        num2->dec.digits = num1->dec.digits;
        break;
    case NCX_BT_FLOAT64:
        num2->d = num1->d;
        break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    return NO_ERR;
}  /* ncx_copy_num */


/********************************************************************
* FUNCTION ncx_cast_num
* 
* Cast a number as another number type
*
* Supports all NCX numeric types:
*    NCX_BT_INT*
*    NCX_BT_UINT*
*    NCX_BT_FLOAT64
*
* INPUTS:
*     num1 == source number
*     btyp1 == expected data type of num1
*     num2 == target number
*     btyp2 == desired data type of num2
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ncx_cast_num (const ncx_num_t *num1,
                  ncx_btype_t  btyp1,
                  ncx_num_t *num2,
                  ncx_btype_t  btyp2)
{
    int64      testbase, testfrac;
    status_t   res;

#ifdef DEBUG
    if (!num1 || !num2) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    switch (btyp1) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        switch (btyp2) {
        case NCX_BT_INT8:
        case NCX_BT_INT16:
        case NCX_BT_INT32:
            num2->i = num1->i;
            break;
        case NCX_BT_INT64:
            num2->l = (int64)num1->i;
            break;
        case NCX_BT_UINT8:
        case NCX_BT_UINT16:
        case NCX_BT_UINT32:
            num2->u = (uint32)num1->i;
            break;
        case NCX_BT_UINT64:
            num2->ul = (uint64)num1->i;
            break;
        case NCX_BT_DECIMAL64:
            if (num2->dec.digits == 0) {
                /* hack: set a default if none set already */
                num2->dec.digits = NCX_DEF_FRACTION_DIGITS;
            }
            /* this may cause an overflow, but too bad !!! */
            num2->dec.val = 
                (int64)(num1->i * (10 * num2->dec.digits));
            break;
        case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
            num2->d = (double)num1->i;
#else
            num2->d = (int64)num1->i;
#endif
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case NCX_BT_INT64:
        switch (btyp2) {
        case NCX_BT_INT8:
        case NCX_BT_INT16:
        case NCX_BT_INT32:
        case NCX_BT_UINT8:
        case NCX_BT_UINT16:
        case NCX_BT_UINT32:
            res = ERR_NCX_INVALID_VALUE;
            break;
        case NCX_BT_INT64:
            num2->l = num1->l;
            break;
        case NCX_BT_UINT64:
            num2->ul = (uint64)num1->l;
            break;
        case NCX_BT_DECIMAL64:
            if (num2->dec.digits == 0) {
                /* hack: set a default if none set already */
                num2->dec.digits = NCX_DEF_FRACTION_DIGITS;
            }
            /* this may cause an overflow, but too bad !!! */
            num2->dec.val = 
                (int64)(num1->l * (10 * num2->dec.digits));
            break;
        case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
            num2->d = (double)num1->l;
#else
            num2->d = (int64)num1->l;
#endif
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        switch (btyp2) {
        case NCX_BT_INT8:
        case NCX_BT_INT16:
        case NCX_BT_INT32:
            num2->i = (int32)num1->u;
            break;
        case NCX_BT_INT64:
            num2->l = (int64)num1->u;
            break;
        case NCX_BT_UINT8:
        case NCX_BT_UINT16:
        case NCX_BT_UINT32:
            num2->u = num1->u;
            break;
        case NCX_BT_UINT64:
            num2->ul = (uint64)num1->u;
            break;
        case NCX_BT_DECIMAL64:
            if (num2->dec.digits == 0) {
                /* hack: set a default if none set already */
                num2->dec.digits = NCX_DEF_FRACTION_DIGITS;
            }
            /* this may cause an overflow, but too bad !!! */
            num2->dec.val = 
                (int64)(num1->u * (10 * num2->dec.digits));
            break;
        case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
            num2->d = (double)num1->u;
#else
            num2->d = (int64)num1->u;
#endif
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case NCX_BT_UINT64:
        switch (btyp2) {
        case NCX_BT_INT8:
        case NCX_BT_INT16:
        case NCX_BT_INT32:
        case NCX_BT_UINT8:
        case NCX_BT_UINT16:
        case NCX_BT_UINT32:
            res = ERR_NCX_INVALID_VALUE;
            break;
        case NCX_BT_INT64:
            num2->l = (int64)num1->ul;
            break;
        case NCX_BT_UINT64:
            num2->ul = num1->ul;
            break;
        case NCX_BT_DECIMAL64:
            if (num2->dec.digits == 0) {
                /* hack: set a default if none set already */
                num2->dec.digits = NCX_DEF_FRACTION_DIGITS;
            }
            /* this may cause an overflow, but too bad !!! */
            num2->dec.val = 
                (int64)(num1->ul * (10 * num2->dec.digits));
            break;
        case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
            num2->d = (double)num1->ul;
#else
            num2->d = (int64)num1->ul;
#endif
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case NCX_BT_DECIMAL64:
        if (num1->dec.digits == 0) {
            res = ERR_NCX_INVALID_VALUE;
        } else {
            /* just use testbase for now;
             * not sure if this will ever be used
             */
            testbase = num1->dec.val / (10 * num1->dec.digits);
            testfrac = num1->dec.val % (10 * num1->dec.digits);

            switch (btyp2) {
            case NCX_BT_INT8:
            case NCX_BT_INT16:
            case NCX_BT_INT32:
            case NCX_BT_UINT8:
            case NCX_BT_UINT16:
            case NCX_BT_UINT32:
                return ERR_NCX_INVALID_VALUE;
            case NCX_BT_INT64:
                /* just do a floor() function for now */
                num2->l = testbase;
                break;
            case NCX_BT_UINT64:
                num2->ul = (uint64)testbase;
                break;
            case NCX_BT_DECIMAL64:
                num2->dec.val = num1->dec.val;
                num2->dec.digits = num1->dec.digits;
                break;
            case NCX_BT_FLOAT64:
                num2->d = (double)testbase;
                break;
            default:
                res = SET_ERROR(ERR_INTERNAL_VAL);
            }
        }
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        switch (btyp2) {
        case NCX_BT_INT8:
        case NCX_BT_INT16:
        case NCX_BT_INT32:
        case NCX_BT_UINT8:
        case NCX_BT_UINT16:
        case NCX_BT_UINT32:
        case NCX_BT_DECIMAL64:
            return ERR_NCX_INVALID_VALUE;
        case NCX_BT_INT64:
            num2->l = (int64)lrint(num1->d);
            break;
        case NCX_BT_UINT64:
            num2->ul = (uint64)lrint(num1->d);
            break;
        case NCX_BT_FLOAT64:
            num2->d = num1->d;
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
#else
        switch (btyp2) {
        case NCX_BT_INT8:
        case NCX_BT_INT16:
        case NCX_BT_INT32:
        case NCX_BT_UINT8:
        case NCX_BT_UINT16:
        case NCX_BT_UINT32:
        case NCX_BT_DECIMAL64:
            return ERR_NCX_INVALID_VALUE;
        case NCX_BT_INT64:
            num2->l = num1->d;
            break;
        case NCX_BT_UINT64:
            num2->ul = (uint64)num1->d;
            break;
        case NCX_BT_FLOAT64:
            num2->d = num1->d;
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
#endif
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* ncx_cast_num */


/********************************************************************
* FUNCTION ncx_num_floor
* 
* Get the floor value of a number
*
* Supports all NCX numeric types:
*    NCX_BT_INT*
*    NCX_BT_UINT*
*    NCX_BT_DECIMAL64
*    NCX_BT_FLOAT64
*
* INPUTS:
*     num1 == source number
*     num2 == target number
*     btyp == expected data type of numbers
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ncx_num_floor (const ncx_num_t *num1,
                   ncx_num_t *num2,
                   ncx_btype_t  btyp)
{
    status_t   res;

#ifdef DEBUG
    if (!num1 || !num2) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        num2->i = num1->i;
        break;
    case NCX_BT_INT64:
        num2->l = num1->l;
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        num2->u = num1->u;
        break;
    case NCX_BT_UINT64:
        num2->ul = num1->ul;
        break;
    case NCX_BT_DECIMAL64:
        num2->dec.digits = num1->dec.digits;
        num2->dec.val = num1->dec.val % (10 * num1->dec.digits);
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        num2->d = floor(num1->d);
#else
        num2->d = num1->d;
#endif
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;
    
}  /* ncx_num_floor */


/********************************************************************
* FUNCTION ncx_num_ceiling
* 
* Get the ceiling value of a number
*
* Supports all NCX numeric types:
*    NCX_BT_INT*
*    NCX_BT_UINT*
*    NCX_BT_DECIMAL64
*    NCX_BT_FLOAT64
*
* INPUTS:
*     num1 == source number
*     num2 == target number
*     btyp == expected data type of numbers
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ncx_num_ceiling (const ncx_num_t *num1,
                     ncx_num_t *num2,
                     ncx_btype_t  btyp)
{
    status_t   res;

#ifdef DEBUG
    if (!num1 || !num2) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        num2->i = num1->i;
        break;
    case NCX_BT_INT64:
        num2->l = num1->l;
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        num2->u = num1->u;
        break;
    case NCX_BT_UINT64:
        num2->ul = num1->ul;
        break;
    case NCX_BT_DECIMAL64:
        num2->dec.digits = num1->dec.digits;
        /*** this is not right !!!! ***/
        num2->dec.val = num1->dec.val % (10 * num1->dec.digits);
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        num2->d = ceil(num1->d);
#else
        num2->d = num1->d;
#endif
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;
    
}  /* ncx_num_ceiling */


/********************************************************************
* FUNCTION ncx_round_num
* 
* Get the rounded value of a number
*
* Supports all NCX numeric types:
*    NCX_BT_INT*
*    NCX_BT_UINT*
*    NCX_BT_DECIMAL64
*    NCX_BT_FLOAT64
*
* INPUTS:
*     num1 == source number
*     num2 == target number
*     btyp == expected data type of numbers
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ncx_round_num (const ncx_num_t *num1,
                   ncx_num_t *num2,
                   ncx_btype_t  btyp)
{
    status_t   res;

#ifdef DEBUG
    if (!num1 || !num2) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        num2->i = num1->i;
        break;
    case NCX_BT_INT64:
        num2->l = num1->l;
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        num2->u = num1->u;
        break;
    case NCX_BT_UINT64:
        num2->ul = num1->ul;
        break;

    case NCX_BT_DECIMAL64:
        num2->dec.digits = num1->dec.digits;
        /*** this is not right !!!! ***/
        num2->dec.val = num1->dec.val % (10 * num1->dec.digits);
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        num2->d = round(num1->d);
#else
        num2->d = num1->d;
#endif
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;
    
}  /* ncx_round_num */


/********************************************************************
* FUNCTION ncx_num_is_integral
* 
* Check if the number is integral or if it has a fractional part
* Supports all NCX numeric types:
*    NCX_BT_INT*
*    NCX_BT_UINT*
*    NCX_BT_DECIMAL64
*    NCX_BT_FLOAT64
*
* INPUTS:
*     num == number to check
*     btyp == expected data type
*
* RETURNS:
*     TRUE if integral, FALSE if not
*********************************************************************/
boolean
    ncx_num_is_integral (const ncx_num_t *num,
                         ncx_btype_t  btyp)
{
#ifdef HAS_FLOAT
    double d;
#endif

#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
    case NCX_BT_INT64:
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
    case NCX_BT_UINT64:
        return TRUE;
    case NCX_BT_DECIMAL64:
        if (num->dec.digits == 0) {
            SET_ERROR(ERR_INTERNAL_VAL);
            return FALSE;
        }
        if (num->dec.val / (10 * num->dec.digits)) {
            return TRUE;
        } else {
            return FALSE;
        }
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT        
        d = round(num->d);
        return (d == num->d) ? TRUE : FALSE;
#else
        return TRUE;
#endif
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return FALSE;
    }
    /*NOTREACHED*/

}  /* ncx_num_is_integral */


/********************************************************************
* FUNCTION ncx_cvt_to_int64
* 
* Convert a number to an integer64;
* Use rounding for float64
*
* INPUTS:
*     num == number to convert
*     btyp == data type of num
*
* RETURNS:
*     int64 representation
*********************************************************************/
int64
    ncx_cvt_to_int64 (const ncx_num_t *num,
                      ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        return (int64)num->i;
    case NCX_BT_INT64:
        return num->l;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        return (int64)num->u;
    case NCX_BT_UINT64:
        return (int64)num->ul;
    case NCX_BT_DECIMAL64:
        if (num->dec.digits == 0) {
            SET_ERROR(ERR_INTERNAL_VAL);
            return 0;
        }
        return (int64)(num->dec.val / (10 * num->dec.digits));
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT        
        return lrint(num->d);
#else
        return num->d;
#endif
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return 0;
    }
    /*NOTREACHED*/

}  /* ncx_cvt_to_int64 */


/********************************************************************
* FUNCTION ncx_get_numfmt
* 
* Get the number format of the specified string
* Does not check for valid format
* Just figures out which type it must be if it were valid
*
* INPUTS:
*     numstr == number string
*
* RETURNS:
*    NCX_NF_NONE, NCX_NF_DEC, NCX_NF_HEX, or NCX_NF_REAL
*********************************************************************/
ncx_numfmt_t
    ncx_get_numfmt (const xmlChar *numstr)
{
#ifdef DEBUG
    if (!numstr) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_NF_NONE;
    }
#endif

    if (*numstr == '\0') {
        return NCX_NF_NONE;
    }

    /* check for a HEX string first */
    if (*numstr=='0' && (numstr[1]=='x' || numstr[1]=='X')) {
        return NCX_NF_HEX;
    }

    /* check real number next */
    while (*numstr && (*numstr != '.')) {
        numstr++;
    }
    if (*numstr) {
        return NCX_NF_REAL;
    }

    /* leading zero means octal, otherwise decimal */
    return (*numstr == '0') ? NCX_NF_OCTAL : NCX_NF_DEC;

}  /* ncx_get_numfmt */


/********************************************************************
* FUNCTION ncx_printf_num
* 
* Printf a ncx_num_t contents
*
* INPUTS:
*    num == number to printf
*    btyp == number base type
*
*********************************************************************/
void
    ncx_printf_num (const ncx_num_t *num,
                    ncx_btype_t  btyp)
{
    xmlChar   numbuff[VAL_MAX_NUMLEN];
    uint32    len;
    status_t  res;

#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    res = ncx_sprintf_num(numbuff, num, btyp, &len);
    if (res != NO_ERR) {
        log_write("invalid num '%s'", get_error_string(res));
    } else {
        log_write("%s", numbuff);
    }

} /* ncx_printf_num */


/********************************************************************
* FUNCTION ncx_alt_printf_num
* 
* Printf a ncx_num_t contents to the alternate log file
*
* INPUTS:
*    num == number to printf
*    btyp == number base type
*
*********************************************************************/
void
    ncx_alt_printf_num (const ncx_num_t *num,
                        ncx_btype_t  btyp)
{
    xmlChar   numbuff[VAL_MAX_NUMLEN];
    uint32    len;
    status_t  res;

#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    res = ncx_sprintf_num(numbuff, num, btyp, &len);
    if (res != NO_ERR) {
        log_alt_write("invalid num '%s'", get_error_string(res));
    } else {
        log_alt_write("%s", numbuff);
    }

} /* ncx_alt_printf_num */


/********************************************************************
* FUNCTION ncx_sprintf_num
* 
* Sprintf a ncx_num_t contents
*
* INPUTS:
*    buff == buffer to write; NULL means just get length
*    num == number to printf
*    btyp == number base type
*    len == address of return length
*
* OUTPUTS::
*    *len == number of bytes written (or would have been) to buff
*
* RETURNS:
*    status
*********************************************************************/
status_t
    ncx_sprintf_num (xmlChar *buff,
                     const ncx_num_t *num,
                     ncx_btype_t  btyp,
                     uint32   *len)
{
    xmlChar  *point;
    int32     ilen, pos, tzcount;
    uint32    ulen;
    xmlChar   dumbuff[VAL_MAX_NUMLEN];
    xmlChar   decbuff[VAL_MAX_NUMLEN];

#ifdef DEBUG
    if (!num || !len) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (!buff) {
        buff = dumbuff;    
    }

    ilen = 0;
    *len = 0;
    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        ilen = sprintf((char *)buff, "%d", num->i);
        break;
    case NCX_BT_INT64:
        ilen = sprintf((char *)buff, "%lld", num->l);
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        ilen = sprintf((char *)buff, "%u", num->u);
        break;
    case NCX_BT_UINT64:
        ilen = sprintf((char *)buff, "%llu", num->ul);
        break;
    case NCX_BT_DECIMAL64:
        if (num->dec.val == 0) {
            ilen = xml_strcpy(buff, (const xmlChar *)"0.0");
        } else {
            if (num->dec.digits == 0) {
                return SET_ERROR(ERR_INTERNAL_VAL);
            } else {
                /* get the encoded number in the temp buffer */
                pos = sprintf((char *)decbuff, 
                              "%lld", 
                              num->dec.val);

                if (pos <= num->dec.digits) {
                    return SET_ERROR(ERR_INTERNAL_VAL);
                } else {
                    /* find where the decimal point should go */
                    point = &decbuff[pos - num->dec.digits];

                    /* copy the base part to the real buffer */
                    ulen = xml_strncpy(buff, 
                                       decbuff, 
                                       (uint32)(point - decbuff));

                    buff[ulen] = '.';

                    xml_strcpy(&buff[ulen+1], point);

                    /* current length is pos+1
                     * need to check for trailing zeros
                     * and remove them
                     * (!!! need flag to override!!!)
                     * !!! TBD: WAITING FOR WG TO DECIDE
                     * !! RETURN WITH TRAILING ZEROS FOR NOW
                     */
                    ilen = pos + 1;
                }
            }
        }
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        ilen = sprintf((char *)buff, "%.14f", num->d);
        tzcount = remove_trailing_zero_count(buff);
        if (tzcount) {
            ilen -= tzcount;
            if (buff != dumbuff) {
                buff[ilen] =  0;
            }
        }
#else
        ilen = sprintf((char *)buff, "%lld", num->d);
#endif
        break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check the sprintf return value */
    if (ilen < 0) {
        return ERR_NCX_INVALID_NUM;
    } else {
        *len = (uint32)ilen;
    }
    return NO_ERR;

} /* ncx_sprintf_num */


/********************************************************************
* FUNCTION ncx_is_min
* 
* Return TRUE if the specified number is the min value
   for its type
*
* INPUTS:
*     num == number to check
*     btyp == data type of num
* RETURNS:
*     TRUE if this is the minimum value
*     FALSE otherwise
*********************************************************************/
boolean
    ncx_is_min (const ncx_num_t *num,
                ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
        return (num->i == NCX_MIN_INT8) ? TRUE : FALSE;
    case NCX_BT_INT16:
        return (num->i == NCX_MIN_INT16) ? TRUE : FALSE;
    case NCX_BT_INT32:
        return (num->i == NCX_MIN_INT) ? TRUE : FALSE;
    case NCX_BT_INT64:
        return (num->l == NCX_MIN_LONG) ? TRUE : FALSE;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        return (num->u == NCX_MIN_UINT) ? TRUE : FALSE;
    case NCX_BT_UINT64:
        return (num->ul == NCX_MIN_ULONG) ? TRUE : FALSE;

    case NCX_BT_DECIMAL64:
        return (num->dec.val == NCX_MIN_LONG) ? TRUE : FALSE;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        return (num->d == -INFINITY) ? TRUE : FALSE;
#else
        return (num->d == NCX_MIN_LONG) ? TRUE : FALSE;
#endif
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return FALSE;
    }
    /*NOTREACHED*/

}  /* ncx_is_min */


/********************************************************************
* FUNCTION ncx_is_max
* 
* Return TRUE if the specified number is the max value
   for its type
*
* INPUTS:
*     num == number to check
*     btyp == data type of num
* RETURNS:
*     TRUE if this is the maximum value
*     FALSE otherwise
*********************************************************************/
boolean
    ncx_is_max (const ncx_num_t *num,
                ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    switch (btyp) {
    case NCX_BT_INT8:
        return (num->i == NCX_MAX_INT8) ? TRUE : FALSE;
    case NCX_BT_INT16:
        return (num->i == NCX_MAX_INT16) ? TRUE : FALSE;
    case NCX_BT_INT32:
        return (num->i == NCX_MAX_INT) ? TRUE : FALSE;
    case NCX_BT_INT64:
        return (num->l == NCX_MAX_LONG) ? TRUE : FALSE;
    case NCX_BT_UINT8:
        return (num->u == NCX_MAX_UINT8) ? TRUE : FALSE;
    case NCX_BT_UINT16:
        return (num->u == NCX_MAX_UINT16) ? TRUE : FALSE;
    case NCX_BT_UINT32:
        return (num->u == NCX_MAX_UINT) ? TRUE : FALSE;
    case NCX_BT_UINT64:
        return (num->ul == NCX_MAX_ULONG) ? TRUE : FALSE;
    case NCX_BT_DECIMAL64:
        return (num->dec.val == NCX_MAX_LONG) ? TRUE : FALSE;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        return (num->d == INFINITY) ? TRUE : FALSE;
#else
        return (num->d == NCX_MAX_LONG-1) ? TRUE : FALSE;
#endif
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return FALSE;
    }
    /*NOTREACHED*/

}  /* ncx_is_max */


/********************************************************************
* FUNCTION ncx_convert_tkcnum
* 
* Convert the current token in a token chain to
* a ncx_num_t struct
*
* INPUTS:
*     tkc == token chain; current token will be converted
*            tkc->typ == TK_TT_DNUM, TK_TT_HNUM, TK_TT_RNUM
*     btyp == desired number type 
*             (e.g., NCX_BT_INT32, NCX_BT_UINT64, NCX_BT_FLOAT64)
* OUTPUTS:
*     *val == converted number value (0..2G-1), if NO_ERR
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_convert_tkcnum (tk_chain_t *tkc,
                        ncx_btype_t btyp,
                        ncx_num_t *val)
{
    const xmlChar *numstr;

    if (btyp == NCX_BT_DECIMAL64) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_DNUM:
        numstr = TK_CUR_VAL(tkc);
        if (numstr && *numstr=='0') {
            return ncx_convert_num(TK_CUR_VAL(tkc), 
                                   NCX_NF_OCTAL, 
                                   btyp, 
                                   val);
        } else {
            return ncx_convert_num(TK_CUR_VAL(tkc), 
                                   NCX_NF_DEC, 
                                   btyp, 
                                   val);
        }
    case TK_TT_HNUM:
        return ncx_convert_num(TK_CUR_VAL(tkc), 
                               NCX_NF_HEX, 
                               btyp, 
                               val);
    case TK_TT_RNUM:
        return ncx_convert_num(TK_CUR_VAL(tkc), 
                               NCX_NF_REAL, 
                               btyp, 
                               val);
    default:
        /* if this is a string, then this might work */
        return ncx_decode_num(TK_CUR_VAL(tkc), btyp, val);
    }
}  /* ncx_convert_tkcnum */


/********************************************************************
* FUNCTION ncx_convert_tkc_dec64
* 
* Convert the current token in a token chain to
* a ncx_num_t struct, expecting NCX_BT_DECIMAL64
*
* INPUTS:
*     tkc == token chain; current token will be converted
*            tkc->typ == TK_TT_DNUM, TK_TT_HNUM, TK_TT_RNUM
*     digits == number of expected digits
*
* OUTPUTS:
*     *val == converted number value, if NO_ERR
*
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_convert_tkc_dec64 (tk_chain_t *tkc,
                           uint8 digits,
                           ncx_num_t *val)
{
    const xmlChar *numstr;

#ifdef DEBUG
    if (!tkc || !val) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_DNUM:
        numstr = TK_CUR_VAL(tkc);
        if (numstr && *numstr=='0' && numstr[1] != '.') {
            return ncx_convert_dec64(TK_CUR_VAL(tkc), 
                                     NCX_NF_OCTAL, 
                                     digits, 
                                     val);
        } else {
            return ncx_convert_dec64(TK_CUR_VAL(tkc), 
                                     NCX_NF_DEC, 
                                     digits, 
                                     val);
        }
    case TK_TT_HNUM:
        return ncx_convert_dec64(TK_CUR_VAL(tkc), 
                                 NCX_NF_HEX, 
                                 digits, 
                                 val);
    case TK_TT_RNUM:
        return ncx_convert_dec64(TK_CUR_VAL(tkc), 
                                 NCX_NF_REAL, 
                                 digits, 
                                 val);
    default:
        /* if this is a string, then this might work */
        return ncx_decode_dec64(TK_CUR_VAL(tkc), 
                                digits, 
                                val);
    }
}  /* ncx_convert_tkc_dec64 */


/********************************************************************
* FUNCTION ncx_compare_strs
* 
* Compare 2 ncx_str_t union contents
*
* INPUTS:
*     str1 == first string
*     str2 == second string
*     btyp == expected data type 
*             (NCX_BT_STRING, NCX_BT_INSTANCE_ID)
* RETURNS:
*     -1 if str1 is < str2
*      0 if str1 == str2   (also for error, after SET_ERROR called)
*      1 if str1 is > str2
*********************************************************************/
int32
    ncx_compare_strs (const ncx_str_t *str1,
                      const ncx_str_t *str2,
                      ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!str1 || !str2) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
    if (!typ_is_string(btyp)) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return 0;
    }   
#endif

    return xml_strcmp(*str1, *str2);

    /*NOTREACHED*/
}  /* ncx_compare_strs */


/********************************************************************
* FUNCTION ncx_copy_str
* 
* Copy the contents of str1 to str2
* Supports base types:
*     NCX_BT_STRING
*     NCX_BT_INSTANCE_ID
*     NCX_BT_LEAFREF
*
* INPUTS:
*     str1 == first string
*     str2 == second string
*     btyp == expected data type
*
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_copy_str (const ncx_str_t *str1,
                  ncx_str_t *str2,
                  ncx_btype_t  btyp)
{
#ifdef DEBUG
    if (!str1 || !str2) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!(typ_is_string(btyp) || btyp==NCX_BT_BITS)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }   
#endif

    if (*str1) {
        *str2 = xml_strdup(*str1);
        if (!*str2) {
            return ERR_INTERNAL_MEM;
        }
    } else {
        *str2 = NULL;
    }
    return NO_ERR;

}  /* ncx_copy_str */


/********************************************************************
* FUNCTION ncx_clean_str
* 
* Scrub the memory in a ncx_str_t by freeing all
* the sub-fields. DOES NOT free the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    str == ncx_str_t data structure to clean
*********************************************************************/
void 
    ncx_clean_str (ncx_str_t *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;  
    }
#endif

    /* clean the num->union, depending on base type */
    if (*str) {
        m__free(*str);
        *str = NULL;
    }

}  /* ncx_clean_str */


/********************************************************************
* FUNCTION ncx_new_list
* 
* Malloc Initialize an allocated ncx_list_t
*
* INPUTS:
*    btyp == type of list desired
*
* RETURNS:
*   pointer to new entry, or NULL if memory error
*********************************************************************/
ncx_list_t *
    ncx_new_list (ncx_btype_t btyp)
{
    ncx_list_t *list;

    list = m__getObj(ncx_list_t);
    if (list) {
        ncx_init_list(list, btyp);
    }
    return list;

} /* ncx_new_list */


/********************************************************************
* FUNCTION ncx_init_list
* 
* Initialize an allocated ncx_list_t
*
* INPUTS:
*    list == pointer to ncx_list_t memory
*    btyp == base type for the list
*********************************************************************/
void
    ncx_init_list (ncx_list_t *list,
                   ncx_btype_t btyp)
{
#ifdef DEBUG
    if (!list) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    list->btyp = btyp;
    dlq_createSQue(&list->memQ);

} /* ncx_init_list */


/********************************************************************
* FUNCTION ncx_clean_list
* 
* Scrub the memory of a ncx_list_t but do not delete it
*
* INPUTS:
*    list == ncx_list_t struct to clean
*********************************************************************/
void
    ncx_clean_list (ncx_list_t *list)
{
    ncx_lmem_t  *lmem;

#ifdef DEBUG
    if (!list) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    /* clean the string Q */
    while (!dlq_empty(&list->memQ)) {
        lmem = (ncx_lmem_t *)dlq_deque(&list->memQ);
        ncx_clean_lmem(lmem, list->btyp);
        m__free(lmem);
    }

    list->btyp = NCX_BT_NONE;
    /* leave the list->memQ ready to use again */

} /* ncx_clean_list */


/********************************************************************
* FUNCTION ncx_free_list
* 
* Clean and free an allocated ncx_list_t
*
* INPUTS:
*    list == pointer to ncx_list_t memory
*********************************************************************/
void
    ncx_free_list (ncx_list_t *list)
{
#ifdef DEBUG
    if (!list) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    ncx_clean_list(list);
    m__free(list);

} /* ncx_free_list */


/********************************************************************
* FUNCTION ncx_list_cnt
* 
* Get the number of entries in the list
*
* INPUTS:
*    list == pointer to ncx_list_t memory
* RETURNS:
*    number of entries counted
*********************************************************************/
uint32
    ncx_list_cnt (const ncx_list_t *list)
{
    const ncx_lmem_t *lmem;
    uint32      cnt;

#ifdef DEBUG
    if (!list) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    cnt = 0;
    for (lmem = (const ncx_lmem_t *)dlq_firstEntry(&list->memQ);
         lmem != NULL;
         lmem = (const ncx_lmem_t *)dlq_nextEntry(lmem)) {
        cnt++;
    }
    return cnt;

} /* ncx_list_cnt */


/********************************************************************
* FUNCTION ncx_list_empty
* 
* Check if the list is empty or not
*
* INPUTS:
*    list == pointer to ncx_list_t memory
* RETURNS:
*    TRUE if list is empty
*    FALSE otherwise
*********************************************************************/
boolean
    ncx_list_empty (const ncx_list_t *list)
{
#ifdef DEBUG
    if (!list) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return TRUE;
    }
#endif
    
    return dlq_empty(&list->memQ);

} /* ncx_list_empty */


/********************************************************************
* FUNCTION ncx_string_in_list
* 
* Check if the string value is in the list
* List type must be string based, or an enum
*
* INPUTS:
*     str == string to find in the list
*     list == slist to check
*
* RETURNS:
*     status
*********************************************************************/
boolean
    ncx_string_in_list (const xmlChar *str,
                        const ncx_list_t *list)
{
    const ncx_lmem_t *lmem;

#ifdef DEBUG
    if (!str || !list) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    /* screen the list base type */
    switch (list->btyp) {
    case NCX_BT_STRING:
    case NCX_BT_ENUM:
    case NCX_BT_BITS:
        break;
    default:
        SET_ERROR(ERR_NCX_WRONG_TYPE);
        return FALSE;
    }

    /* search the list for a match */
    for (lmem = (const ncx_lmem_t *)dlq_firstEntry(&list->memQ);
         lmem != NULL;
         lmem = (const ncx_lmem_t *)dlq_nextEntry(lmem)) {

        switch (list->btyp) {
        case NCX_BT_ENUM:
            if (!xml_strcmp(str, lmem->val.enu.name)) {
                return TRUE;
            }
            break;
        case NCX_BT_BITS:
            if (!xml_strcmp(str, lmem->val.bit.name)) {
                return TRUE;
            }
            break;
        default:
            if (!xml_strcmp(str, lmem->val.str)) {
                return TRUE;
            }
        }
    }

    return FALSE;

}  /* ncx_string_in_list */


/********************************************************************
* FUNCTION ncx_compare_lists
* 
* Compare 2 ncx_list_t struct contents
*
* Expected data type (NCX_BT_SLIST)
*
* INPUTS:
*     list1 == first number
*     list2 == second number
* RETURNS:
*     -1 if list1 is < list2
*      0 if list1 == list2   (also for error, after SET_ERROR called)
*      1 if list1 is > list2
*********************************************************************/
int32
    ncx_compare_lists (const ncx_list_t *list1,
                       const ncx_list_t *list2)
{
    const ncx_lmem_t  *s1, *s2;
    int                retval;

#ifdef DEBUG
    if (!list1 || !list2) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return -1;
    }
    if (list1->btyp != list2->btyp) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return -1;
    }   
#endif

    /* get start strings */
    s1 = (const ncx_lmem_t *)dlq_firstEntry(&list1->memQ);
    s2 = (const ncx_lmem_t *)dlq_firstEntry(&list2->memQ);
        
    /* have 2 start structs to compare */
    for (;;) {
        if (!s1 && !s2) {
            return 0;
        } else if (!s1) {
            return -1;
        } else if (!s2) {
            return 1;
        }

        if (typ_is_string(list1->btyp)) {
            retval = ncx_compare_strs(&s1->val.str, 
                                      &s2->val.str, 
                                      NCX_BT_STRING);
        } else if (typ_is_number(list1->btyp)) {
            retval = ncx_compare_nums(&s1->val.num, 
                                      &s2->val.num,
                                      list1->btyp);
        } else {
            switch (list1->btyp) {
            case NCX_BT_BITS:
                retval = ncx_compare_bits(&s1->val.bit, 
                                          &s2->val.bit);
                break;
            case NCX_BT_ENUM:
                retval = ncx_compare_enums(&s1->val.enu, 
                                           &s2->val.enu);
                break;
            default:
                SET_ERROR(ERR_INTERNAL_VAL);
                return 0;
            }
        }

        switch (retval) {
        case -1:
            return -1;
        case 0:
            break;
        case 1:
            return 1;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
            return 0;
        }

        s1 = (const ncx_lmem_t *)dlq_nextEntry(s1);
        s2 = (const ncx_lmem_t *)dlq_nextEntry(s2);
    }
    /*NOTREACHED*/

}  /* ncx_compare_lists */


/********************************************************************
* FUNCTION ncx_copy_list
* 
* Copy the contents of list1 to list2
* Supports base type NCX_BT_SLIST
*
* A partial copy may occur, and list2 should be properly cleaned
* and freed, even if an error is returned
*
* INPUTS:
*     list1 == first list
*     list2 == second list
*
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_copy_list (const ncx_list_t *list1,
                   ncx_list_t *list2)
{
    const ncx_lmem_t *lmem;
    ncx_lmem_t       *lcopy;
    status_t                 res;

#ifdef DEBUG
    if (!list1 || !list2) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    list2->btyp = list1->btyp;
    dlq_createSQue(&list2->memQ);

    /* go through all the list members and copy each one */
    for (lmem = (const ncx_lmem_t *)dlq_firstEntry(&list1->memQ);
         lmem != NULL;
         lmem = (const ncx_lmem_t *)dlq_nextEntry(lmem)) {
        lcopy = ncx_new_lmem();
        if (!lcopy) {
            return ERR_INTERNAL_MEM;
        }

        /* copy the string or number from lmem to lcopy */
        switch (list1->btyp) {
        case NCX_BT_STRING:
            res = ncx_copy_str(&lmem->val.str, &lcopy->val.str, list1->btyp);
            break;
        case NCX_BT_BITS:
            lcopy->val.bit.pos = lmem->val.bit.pos;
            lcopy->val.bit.dname = xml_strdup(lmem->val.bit.name);
            if (!lcopy->val.bit.dname) {
                res = ERR_INTERNAL_MEM;
            } else {
                lcopy->val.bit.name = lcopy->val.bit.dname;
            }
            break;
        case NCX_BT_ENUM:
            lcopy->val.enu.val = lmem->val.enu.val;
            lcopy->val.enu.dname = xml_strdup(lmem->val.enu.name);
            if (!lcopy->val.enu.dname) {
                res = ERR_INTERNAL_MEM;
            } else {
                lcopy->val.enu.name = lcopy->val.enu.dname;
            }
            break;
        case NCX_BT_BOOLEAN:
            lcopy->val.boo = lmem->val.boo;
            break;
        default:
            if (typ_is_number(list1->btyp)) {
                res = ncx_copy_num(&lmem->val.num, 
                                   &lcopy->val.num, list1->btyp);
            } else {
                res = SET_ERROR(ERR_INTERNAL_VAL);
            }
        }

        if (res != NO_ERR) {
            ncx_free_lmem(lcopy, list1->btyp);
            return res;
        }

        /* save lcopy in list2 */
        dlq_enque(lcopy, &list2->memQ);
    }
    return NO_ERR;

}  /* ncx_copy_list */


/********************************************************************
* FUNCTION ncx_merge_list
* 
* The merge function is handled specially for lists.
* The contents are not completely replaced like a string.
* Instead, only new entries from src are added to the dest list.
*
* NCX merge algorithm for lists:
*
* If list types not the same, then error exit;
*
* If allow_dups == FALSE:
*    check if entry exists; if so, exit;
*
*   Merge src list member into dest, based on mergetyp enum
* }
*
* INPUTS:
*    src == ncx_list_t struct to merge from
*    dest == ncx_list_t struct to merge into
*    mergetyp == type of merge used for this list
*    allow_dups == TRUE if this list allows duplicate values
*    
* OUTPUTS:
*    ncx_lmem_t structs will be moved from the src to dest as needed
*
* RETURNS:
*   none
*********************************************************************/
void
    ncx_merge_list (ncx_list_t *src,
                    ncx_list_t *dest,
                    ncx_merge_t mergetyp,
                    boolean allow_dups)
{
    ncx_lmem_t      *lmem, *dest_lmem;

#ifdef DEBUG
    if (!src || !dest) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
    if (src->btyp != dest->btyp) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }
#endif

    /* get rid of dups in the src list if duplicates not allowed */
    if (!allow_dups) {
        for (dest_lmem = (ncx_lmem_t *)dlq_firstEntry(&dest->memQ);
             dest_lmem != NULL;
             dest_lmem = (ncx_lmem_t *)dlq_nextEntry(dest_lmem)) {

            lmem = ncx_find_lmem(src, dest_lmem);
            if (lmem) {
                dlq_remove(lmem);
                ncx_free_lmem(lmem, dest->btyp);
            }
        }
    }

    /* transfer the source members to the dest list */
    while (!dlq_empty(&src->memQ)) {

        /* pick an entry to merge, reverse of the merge type
         * to preserve the source order in the dest list
         */
        switch (mergetyp) {
        case NCX_MERGE_FIRST:
            lmem = (ncx_lmem_t *)dlq_lastEntry(&src->memQ);
            break;
        case NCX_MERGE_LAST:
        case NCX_MERGE_SORT:
            lmem = (ncx_lmem_t *)dlq_firstEntry(&src->memQ);
            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
            return;
        }
        dlq_remove(lmem);

        /* merge lmem into the dest list */
        ncx_insert_lmem(dest, lmem, mergetyp);
    }

}  /* ncx_merge_list */


/********************************************************************
* FUNCTION ncx_set_strlist
* 
* Convert a text line into an ncx_list_t using NCX_BT_STRING
* as the list type. Must call ncx_init_list first !!!
*
* INPUTS:
*     liststr == list value in string form
*     list == ncx_list_t that should be initialized and
*             filled with the values from the string
*
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_set_strlist (const xmlChar *liststr,
                  ncx_list_t *list)
{
#ifdef DEBUG
    if (!liststr || !list) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    ncx_init_list(list, NCX_BT_STRING);
    return ncx_set_list(NCX_BT_STRING, liststr, list);

}  /* ncx_set_strlist */


/********************************************************************
* FUNCTION ncx_set_list
* 
* Parse the XML input as an NCX_BT_SLIST
* Do not check the individual strings against any restrictions
* Just check that the strings parse as the expected type.
* Mark list members with errors as needed
*
* Must call ncx_init_list first!!!
*
* INPUTS:
*     btyp == expected basetype for the list member type
*     strval == cleaned XML string to parse into ncx_str_t or 
*             ncx_num_t values
*     list == ncx_list_t in progress that will get the ncx_lmem_t
*             structs added to it, as they are parsed
*
* OUTPUTS:
*     list->memQ has 1 or more ncx_lmem_t structs appended to it
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    ncx_set_list (ncx_btype_t btyp,
                  const xmlChar *strval,
                  ncx_list_t  *list)
{
    const xmlChar     *str1, *str2;
    ncx_lmem_t        *lmem;
    uint32             len;
    status_t           res;
    boolean            done;

#ifdef DEBUG
    if (!strval || !list) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* probably already set but make sure */
    list->btyp = btyp;

    if (!*strval) {
        return NO_ERR;
    }

    str1 = strval;
    str2 = NULL;
    done = FALSE;

    while (!done) {
        /* skip any leading whitespace */
        while (xml_isspace(*str1)) {
            str1++;
        }
        if (!*str1) {
            done = TRUE;
            continue;
        }

        /* set up a new list string struct */
        lmem = ncx_new_lmem();
        if (!lmem) {
            return ERR_INTERNAL_MEM;
        }

        /* parse the string either as whitespace-allowed
         * or whitespace-not-allowed string
         */
        if (*str1==NCX_STR_START) {
            /* The XML string starts with a double quote
             * so interpret the string as whitespace-allowed
             * do not save the double quote char 
             */
            str2 = ++str1;
            while (*str2 && (*str2 != NCX_STR_END)) {
                str2++;
            }
            len = (uint32)(str2-str1);
            if (*str2) {
                str2++;
            } else {
                log_info("\nncx_set_list: missing EOS marker\n  (%s)",
                          str1);
            }
        } else {
            /* consume string until a WS, str-start, or EOS seen */
            str2 = str1+1;
            while (*str2 && !xml_isspace(*str2) && 
                   (*str2 != NCX_STR_START)) {
                str2++;
            }
            len = (uint32)(str2-str1);
        }

        /* copy the string just parsed 
         * for now just separate into strings and do not
         * validate or parse into enums or numbers
         */
        res = NO_ERR;
        lmem->val.str = xml_strndup(str1, len);
        if (!lmem->val.str) {
            res = ERR_INTERNAL_MEM;
        }

        if (res != NO_ERR) {
            ncx_free_lmem(lmem, NCX_BT_STRING);
            return res;
        }

        /* save the list member in the Q */
        dlq_enque(lmem, &list->memQ);

        /* reset the string pointer and loop */
        str1 = str2;
    }

    return NO_ERR;

}  /* ncx_set_list */


/********************************************************************
* FUNCTION ncx_finish_list
* 
* Finish converting the list members to the proper format
*
* INPUTS:
*    typdef == typ_def_t for the designated list member type
*    list == list struct with ncx_lmem_t structs to check
*
* OUTPUTS:
*   If return other than NO_ERR:
*     each list->lmem.flags field may contain bits set
*     for errors:
*        NCX_FL_RANGE_ERR: size out of range
*        NCX_FL_VALUE_ERR  value not permitted by value set, 
*                          or pattern
* RETURNS:
*    status
*********************************************************************/
status_t
    ncx_finish_list (typ_def_t *typdef,
                     ncx_list_t *list)
{
    ncx_lmem_t      *lmem;
    xmlChar         *str;
    ncx_btype_t      btyp;
    status_t         res, retres;
    dlq_hdr_t        tempQ;

#ifdef DEBUG
    if (!typdef || !list) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    btyp = typ_get_basetype(typdef);
    res = NO_ERR;
    retres = NO_ERR;

    /* check if any work to do */
    switch (btyp) {
    case NCX_BT_STRING:
    case NCX_BT_BOOLEAN:
        return NO_ERR;
    default:
        ;
    }

    /* go through all the list members and check them */
    for (lmem = (ncx_lmem_t *)dlq_firstEntry(&list->memQ);
         lmem != NULL;
         lmem = (ncx_lmem_t *)dlq_nextEntry(lmem)) {

        str = lmem->val.str;
        if (btyp == NCX_BT_ENUM) {
            res = val_enum_ok(typdef, 
                              str,
                              &lmem->val.enu.val,
                              &lmem->val.enu.name);
        } else if (btyp == NCX_BT_BITS) {
            /* transfer the malloced string from 
             * val.str to val.bit.dname
             */
            lmem->val.bit.dname = str;
            lmem->val.bit.name = lmem->val.bit.dname;
            res = val_bit_ok(typdef, str, 
                             &lmem->val.bit.pos);
        } else if (typ_is_number(btyp)){
            res = ncx_decode_num(str, btyp, &lmem->val.num);
        } else {
            SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (btyp != NCX_BT_BITS) {
            m__free(str);
        }

        if (res != NO_ERR) {
            /* the string did not match this pattern */
            CHK_EXIT(res, retres);
            lmem->flags |= NCX_FL_VALUE_ERR;
        } 
    }

    if (retres == NO_ERR && btyp == NCX_BT_BITS) {
        /* put bits in their canonical order */
        dlq_createSQue(&tempQ);
        dlq_block_enque(&list->memQ, &tempQ);

        while (!dlq_empty(&tempQ)) {
            lmem = (ncx_lmem_t *)dlq_deque(&tempQ);
            ncx_insert_lmem(list, lmem, NCX_MERGE_SORT);
        }
    }
        
    return retres;

} /* ncx_finish_list */


/********************************************************************
* FUNCTION ncx_new_lmem
*
* Malloc and fill in a new ncx_lmem_t struct
*
* INPUTS:
*   none
* RETURNS:
*   pointer to malloced and initialized ncx_lmem_t struct
*   NULL if malloc error
*********************************************************************/
ncx_lmem_t *
    ncx_new_lmem (void)
{
    ncx_lmem_t  *lmem;

    lmem = m__getObj(ncx_lmem_t);
    if (!lmem) {
        return NULL;
    }
    memset(lmem, 0x0, sizeof(ncx_lmem_t));
    return lmem;

}  /* ncx_new_lmem */


/********************************************************************
* FUNCTION ncx_clean_lmem
* 
* Scrub the memory of a ncx_lmem_t but do not delete it
*
* INPUTS:
*    lmem == ncx_lmem_t struct to clean
*    btyp == base type of list member (lmem)
*********************************************************************/
void
    ncx_clean_lmem (ncx_lmem_t *lmem,
                    ncx_btype_t btyp)
{

#ifdef DEBUG
    if (!lmem) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (typ_is_string(btyp)) {
        ncx_clean_str(&lmem->val.str);
    } else if (typ_is_number(btyp)) {
        ncx_clean_num(btyp, &lmem->val.num);
    } else {
        switch (btyp) {
        case NCX_BT_ENUM:
            ncx_clean_enum(&lmem->val.enu);
            break;
        case NCX_BT_BITS:
            ncx_clean_bit(&lmem->val.bit);
            break;
        case NCX_BT_BOOLEAN:
            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

} /* ncx_clean_lmem */


/********************************************************************
* FUNCTION ncx_free_lmem
*
* Free all the memory in a  ncx_lmem_t struct
*
* INPUTS:
*   lmem == struct to clean and free
*   btyp == base type of the list member
*
*********************************************************************/
void
    ncx_free_lmem (ncx_lmem_t *lmem,
                   ncx_btype_t btyp)
{
#ifdef DEBUG
    if (!lmem) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif
    ncx_clean_lmem(lmem, btyp);
    m__free(lmem);

}  /* ncx_free_lmem */


/********************************************************************
* FUNCTION ncx_find_lmem
*
* Find a the first matching list member with the specified value
*
* INPUTS:
*   list == list to check
*   memval == value to find, based on list->btyp
*
* RETURNS:
*  pointer to the first instance of this value, or NULL if none
*********************************************************************/
ncx_lmem_t *
    ncx_find_lmem (ncx_list_t *list,
                   const ncx_lmem_t *memval)
{
    ncx_lmem_t        *lmem;
    const ncx_num_t   *num;
    const ncx_str_t   *str;
    const ncx_enum_t  *enu;
    const ncx_bit_t   *bit;
    int32              cmpval;
    boolean            boo;

#ifdef DEBUG
    if (!list || !memval) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    num = NULL;
    str = NULL;
    enu = NULL;
    bit = NULL;
    boo = FALSE;

    if (typ_is_number(list->btyp)) {
        num = &memval->val.num;
    } else if (typ_is_string(list->btyp)) {
        str = &memval->val.str;
    } else if (list->btyp == NCX_BT_ENUM) {
        enu = &memval->val.enu;
    } else if (list->btyp == NCX_BT_BITS) {
        bit = &memval->val.bit;
    } else if (list->btyp == NCX_BT_BOOLEAN) {
        boo = memval->val.boo;
    } else {
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }

    for (lmem = (ncx_lmem_t *)dlq_firstEntry(&list->memQ);
         lmem != NULL;
         lmem = (ncx_lmem_t *)dlq_nextEntry(lmem)) {
        if (num) {
            cmpval = ncx_compare_nums(&lmem->val.num, num, list->btyp);
        } else if (str) {
            cmpval = ncx_compare_strs(&lmem->val.str, str, list->btyp);
        } else if (enu) {
            cmpval = ncx_compare_enums(&lmem->val.enu, enu);
        } else if (bit) {
                cmpval = ncx_compare_bits(&lmem->val.bit, bit);
        } else {
            cmpval = (lmem->val.boo && boo) ? 0 : 1;
        }

        if (!cmpval) {
            return lmem;
        }
    }
    return NULL;

}  /* ncx_find_lmem */


/********************************************************************
* FUNCTION ncx_insert_lmem
*
* Insert a list entry into the specified list
*
* INPUTS:
*   list == list to insert into
*   memval == value to insert, based on list->btyp
*   mergetyp == requested merge type for the insertion
*
* RETURNS:
*   none
*********************************************************************/
void
    ncx_insert_lmem (ncx_list_t *list,
                     ncx_lmem_t *memval,
                     ncx_merge_t mergetyp)
{
    ncx_lmem_t        *lmem;
    const ncx_num_t   *num;
    const ncx_str_t   *str;
    const ncx_enum_t  *enu;
    const ncx_bit_t   *bit;
    int32              cmpval;
    boolean            boo;

#ifdef DEBUG
    if (!list || !memval) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    switch (mergetyp) {
    case NCX_MERGE_FIRST:
        lmem = (ncx_lmem_t *)dlq_firstEntry(&list->memQ);
        if (lmem) {
            dlq_insertAhead(memval, lmem);
        } else {
            dlq_enque(memval, &list->memQ);
        }
        break;
    case NCX_MERGE_LAST:
        dlq_enque(memval, &list->memQ);
        break;
    case NCX_MERGE_SORT:
        num = NULL;
        str = NULL;
        enu = NULL;
        bit = NULL;
        boo = FALSE;

        if (typ_is_number(list->btyp)) {
            num = &memval->val.num;
        } else if (typ_is_string(list->btyp)) {
            str = &memval->val.str;
        } else if (list->btyp == NCX_BT_ENUM) {
            enu = &memval->val.enu;
        } else if (list->btyp == NCX_BT_BITS) {
            bit = &memval->val.bit;
        } else if (list->btyp == NCX_BT_BOOLEAN) {
            boo = memval->val.boo;
        } else {
            SET_ERROR(ERR_INTERNAL_VAL);
            return;
        }

        for (lmem = (ncx_lmem_t *)dlq_firstEntry(&list->memQ);
             lmem != NULL;
             lmem = (ncx_lmem_t *)dlq_nextEntry(lmem)) {
            if (num) {
                cmpval = ncx_compare_nums(&lmem->val.num, num, list->btyp);
            } else if (str) {
                cmpval = ncx_compare_strs(&lmem->val.str, str, list->btyp);
            } else if (enu) {
                cmpval = ncx_compare_enums(&lmem->val.enu, enu);
            } else if (bit) {
                cmpval = ncx_compare_bits(&lmem->val.bit, bit);
            } else {
                if (lmem->val.boo) {
                    cmpval = (boo) ? 0 : 1;
                } else {
                    cmpval = (boo) ? -1 : 0;
                }
            }

            if (cmpval >= 0) {
                dlq_insertAhead(memval, lmem);
                return;
            }
        }

        /* make new last entry */
        dlq_enque(memval, &list->memQ);
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }   

}  /* ncx_insert_lmem */


/********************************************************************
* FUNCTION ncx_first_lmem
*
* Return the first list member
*
* INPUTS:
*   list == list to check
*
* RETURNS:
*  pointer to the first list member or NULL if none
*********************************************************************/
ncx_lmem_t *
    ncx_first_lmem (ncx_list_t *list)
{
#ifdef DEBUG
    if (!list) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif
    return (ncx_lmem_t *)dlq_firstEntry(&list->memQ);

}  /* ncx_first_lmem */


/********************************************************************
* FUNCTION ncx_new_binary
*
* Malloc and fill in a new ncx_binary_t struct
*
* INPUTS:
*   none
* RETURNS:
*   pointer to malloced and initialized ncx_binary_t struct
*   NULL if malloc error
*********************************************************************/
ncx_binary_t *
    ncx_new_binary (void)
{
    ncx_binary_t  *binary;
    
    binary = m__getObj(ncx_binary_t);
    if (!binary) {
        return NULL;
    }

    ncx_init_binary(binary);
    return binary;

}  /* ncx_new_binary */


/********************************************************************
* FUNCTION ncx_init_binary
* 
* Init the memory of a ncx_binary_t struct
*
* INPUTS:
*    binary == ncx_binary_t struct to init
*********************************************************************/
void
    ncx_init_binary (ncx_binary_t *binary)
{

#ifdef DEBUG
    if (!binary) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    memset(binary, 0x0, sizeof(ncx_binary_t));

} /* ncx_init_binary */


/********************************************************************
* FUNCTION ncx_clean_binary
* 
* Scrub the memory of a ncx_binary_t but do not delete it
*
* INPUTS:
*    binary == ncx_binary_t struct to clean
*********************************************************************/
void
    ncx_clean_binary (ncx_binary_t *binary)
{

#ifdef DEBUG
    if (!binary) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif
    if (binary->ustr) {
        m__free(binary->ustr);
    }
    memset(binary, 0x0, sizeof(ncx_binary_t));

} /* ncx_clean_binary */


/********************************************************************
* FUNCTION ncx_free_binary
*
* Free all the memory in a  ncx_binary_t struct
*
* INPUTS:
*   binary == struct to clean and free
*
*********************************************************************/
void
    ncx_free_binary (ncx_binary_t *binary)
{
#ifdef DEBUG
    if (!binary) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif
    ncx_clean_binary(binary);
    m__free(binary);

}  /* ncx_free_binary */


/********************************************************************
* FUNCTION ncx_new_appinfo
* 
* Create an appinfo entry
*
* INPOUTS:
*   isclone == TRUE if this is for a cloned object
*
* RETURNS:
*    malloced appinfo entry or NULL if malloc error
*********************************************************************/
ncx_appinfo_t *
    ncx_new_appinfo (boolean isclone)
{
    ncx_appinfo_t *appinfo;

    appinfo = m__getObj(ncx_appinfo_t);
    if (!appinfo) {
        return NULL;
    }
    memset(appinfo, 0x0, sizeof(ncx_appinfo_t));
    appinfo->isclone = isclone;

    if (!isclone) {
        appinfo->appinfoQ = dlq_createQue();
        if (!appinfo->appinfoQ) {
            m__free(appinfo);
            appinfo = NULL;
        }
    }

    return appinfo;

}  /* ncx_new_appinfo */


/********************************************************************
* FUNCTION ncx_free_appinfo
* 
* Free an appinfo entry
*
* INPUTS:
*    appinfo == ncx_appinfo_t data structure to free
*********************************************************************/
void 
    ncx_free_appinfo (ncx_appinfo_t *appinfo)
{
#ifdef DEBUG
    if (!appinfo) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (!appinfo->isclone) {
        if (appinfo->prefix) {
            m__free(appinfo->prefix);
        }
        if (appinfo->name) {
            m__free(appinfo->name);
        }
        if (appinfo->value) {
            m__free(appinfo->value);
        }
        if (appinfo->appinfoQ) {
            ncx_clean_appinfoQ(appinfo->appinfoQ);
            dlq_destroyQue(appinfo->appinfoQ);
        }
    }
    m__free(appinfo);

}  /* ncx_free_appinfo */


/********************************************************************
* FUNCTION ncx_find_appinfo
* 
* Find an appinfo entry by name (First match is returned)
* The entry returned is not removed from the Q
*
* INPUTS:
*    appinfoQ == pointer to Q of ncx_appinfo_t data structure to check
*    prefix == module prefix that defines the extension 
*            == NULL to pick the first match (not expecting
*               appinfo name collisions)
*    varname == name string of the appinfo variable to find
*
* RETURNS:
*    pointer to the ncx_appinfo_t struct for the entry if found
*    NULL if the entry is not found
*********************************************************************/
ncx_appinfo_t *
    ncx_find_appinfo (dlq_hdr_t *appinfoQ,
                      const xmlChar *prefix,
                      const xmlChar *varname)
{
    ncx_appinfo_t *appinfo;

#ifdef DEBUG
    if (!appinfoQ || !varname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (appinfo = (ncx_appinfo_t *)dlq_firstEntry(appinfoQ);
         appinfo != NULL;
         appinfo = (ncx_appinfo_t *)dlq_nextEntry(appinfo)) {

        if (prefix && appinfo->prefix &&
            xml_strcmp(prefix, appinfo->prefix)) {
            continue;
        }

        if (!xml_strcmp(varname, appinfo->name)) {
            return appinfo;
        }
    }
    return NULL;

}  /* ncx_find_appinfo */


/********************************************************************
* FUNCTION ncx_find_const_appinfo
* 
* Find an appinfo entry by name (First match is returned)
* The entry returned is not removed from the Q
*
* INPUTS:
*    appinfoQ == pointer to Q of ncx_appinfo_t data structure to check
*    prefix == module prefix that defines the extension 
*            == NULL to pick the first match (not expecting
*               appinfo name collisions)
*    varname == name string of the appinfo variable to find
*
* RETURNS:
*    pointer to the ncx_appinfo_t struct for the entry if found
*    NULL if the entry is not found
*********************************************************************/
const ncx_appinfo_t *
    ncx_find_const_appinfo (const dlq_hdr_t *appinfoQ,
                            const xmlChar *prefix,
                            const xmlChar *varname)
{
    const ncx_appinfo_t *appinfo;

#ifdef DEBUG
    if (!appinfoQ || !varname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (appinfo = (const ncx_appinfo_t *)dlq_firstEntry(appinfoQ);
         appinfo != NULL;
         appinfo = (const ncx_appinfo_t *)dlq_nextEntry(appinfo)) {

        if (prefix && appinfo->prefix &&
            xml_strcmp(prefix, appinfo->prefix)) {
            continue;
        }

        if (!xml_strcmp(varname, appinfo->name)) {
            return appinfo;
        }
    }
    return NULL;

}  /* ncx_find_const_appinfo */


/********************************************************************
* FUNCTION ncx_find_next_appinfo
* 
* Find the next instance of an appinfo entry by name
* (First match is returned)
* The entry returned is not removed from the Q
*
* INPUTS:
*    current == pointer to current ncx_appinfo_t data structure to check
*    prefix == module prefix that defines the extension 
*            == NULL to pick the first match (not expecting
*               appinfo name collisions)
*    varname == name string of the appinfo variable to find
*
* RETURNS:
*    pointer to the ncx_appinfo_t struct for the entry if found
*    NULL if the entry is not found
*********************************************************************/
const ncx_appinfo_t *
    ncx_find_next_appinfo (const ncx_appinfo_t *current,
                           const xmlChar *prefix,
                           const xmlChar *varname)
{
    ncx_appinfo_t *appinfo;

#ifdef DEBUG
    if (!current || !varname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (appinfo = (ncx_appinfo_t *)dlq_nextEntry(current);
         appinfo != NULL;
         appinfo = (ncx_appinfo_t *)dlq_nextEntry(appinfo)) {

        if (prefix && appinfo->prefix &&
            xml_strcmp(prefix, appinfo->prefix)) {
            continue;
        }

        if (!xml_strcmp(varname, appinfo->name)) {
            return appinfo;
        }
    }
    return NULL;

}  /* ncx_find_next_appinfo */


/********************************************************************
* FUNCTION ncx_find_next_appinfo2
* 
* Find the next instance of an appinfo entry by name
* (First match is returned)
* The entry returned is not removed from the Q
*
* INPUTS:
*    current == pointer to current ncx_appinfo_t data structure to check
*    prefix == module prefix that defines the extension 
*            == NULL to pick the first match (not expecting
*               appinfo name collisions)
*    varname == name string of the appinfo variable to find
*
* RETURNS:
*    pointer to the ncx_appinfo_t struct for the entry if found
*    NULL if the entry is not found
*********************************************************************/
ncx_appinfo_t *
    ncx_find_next_appinfo2 (ncx_appinfo_t *current,
                           const xmlChar *prefix,
                           const xmlChar *varname)
{
    ncx_appinfo_t *appinfo;

#ifdef DEBUG
    if (!current || !varname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (appinfo = (ncx_appinfo_t *)dlq_nextEntry(current);
         appinfo != NULL;
         appinfo = (ncx_appinfo_t *)dlq_nextEntry(appinfo)) {

        if (prefix && appinfo->prefix &&
            xml_strcmp(prefix, appinfo->prefix)) {
            continue;
        }

        if (!xml_strcmp(varname, appinfo->name)) {
            return appinfo;
        }
    }
    return NULL;

}  /* ncx_find_next_appinfo2 */


/********************************************************************
* FUNCTION ncx_clone_appinfo
* 
* Clone an appinfo value
*
* INPUTS:
*    appinfo ==  ncx_appinfo_t data structure to clone
*
* RETURNS:
*    pointer to the malloced ncx_appinfo_t struct clone of appinfo
*    NULL if a malloc error
*********************************************************************/
ncx_appinfo_t *
    ncx_clone_appinfo (ncx_appinfo_t *appinfo)
{
    ncx_appinfo_t *newapp;

#ifdef DEBUG
    if (!appinfo) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    newapp = ncx_new_appinfo(TRUE);
    if (!newapp) {
        return NULL;
    }
    newapp->prefix = appinfo->prefix;
    newapp->name = appinfo->name;
    newapp->value = appinfo->value;
    newapp->appinfoQ = appinfo->appinfoQ;

    return newapp;

}  /* ncx_clone_appinfo */


/********************************************************************
* FUNCTION ncx_clean_appinfoQ
* 
* Check an initialized appinfoQ for any entries
* Remove them from the queue and delete them
*
* INPUTS:
*    appinfoQ == Q of ncx_appinfo_t data structures to free
*********************************************************************/
void 
    ncx_clean_appinfoQ (dlq_hdr_t *appinfoQ)
{
    ncx_appinfo_t *appinfo;

#ifdef DEBUG
    if (!appinfoQ) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    while (!dlq_empty(appinfoQ)) {
        appinfo = (ncx_appinfo_t *)dlq_deque(appinfoQ);
        ncx_free_appinfo(appinfo);
    }
} /* ncx_clean_appinfoQ */


/********************************************************************
* FUNCTION ncx_consume_appinfo
* 
* Check if an appinfo clause is present
*
* Save in appinfoQ if non-NULL
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress (NULL if none)
*   appinfoQ  == queue to use for any found entries (may be NULL)
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_consume_appinfo (tk_chain_t *tkc,
                         ncx_module_t  *mod,
                         dlq_hdr_t *appinfoQ)
{

#ifdef DEBUG
    if (!tkc || !appinfoQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return consume_appinfo(tkc, mod, appinfoQ, TRUE);

}  /* ncx_consume_appinfo */


/********************************************************************
* FUNCTION ncx_consume_appinfo2
* 
* Check if an appinfo clause is present
* Do not backup the current token
* The TK_TT_MSTRING token has not been seen yet
* Called from yang_consume_semiapp
*
* Save in appinfoQ if non-NULL
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod    == ncx_module_t in progress (NULL if none)
*   appinfoQ  == queue to use for any found entries (may be NULL)
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_consume_appinfo2 (tk_chain_t *tkc,
                          ncx_module_t  *mod,
                          dlq_hdr_t *appinfoQ)
{
#ifdef DEBUG
    if (!tkc || !appinfoQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return consume_appinfo(tkc, mod, appinfoQ, FALSE);

}  /* ncx_consume_appinfo2 */


/********************************************************************
* FUNCTION ncx_resolve_appinfoQ
* 
* Validate all the appinfo clauses present in the specified Q
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == ncx_module_t in progress
*   appinfoQ == queue to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_resolve_appinfoQ (yang_pcb_t *pcb,
                          tk_chain_t *tkc,
                          ncx_module_t  *mod,
                          dlq_hdr_t *appinfoQ)
{
     ncx_appinfo_t  *appinfo;
    ext_template_t  *ext;
    status_t         res, retres;

#ifdef DEBUG
    if (!tkc || !mod || !appinfoQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    retres = NO_ERR;

    for (appinfo = (ncx_appinfo_t *)dlq_firstEntry(appinfoQ);
         appinfo != NULL;
         appinfo = (ncx_appinfo_t *)dlq_nextEntry(appinfo)) {

        if (appinfo->isclone) {
            continue;
        }

        if (appinfo->ext) {
            /* this is a redo validation */
            continue;
        }


        if (appinfo->prefix &&
            xml_strcmp(appinfo->prefix, mod->prefix)) {

            res = yang_find_imp_extension(pcb,
                                          tkc, 
                                          mod, 
                                          appinfo->prefix,
                                          appinfo->name, 
                                          &appinfo->tkerr,
                                          &ext);
            CHK_EXIT(res, retres);
        } else {

            ext = ext_find_extension(&mod->extensionQ, appinfo->name);
            if (!ext) {
                log_error("\nError: Local module extension '%s' not found",
                          appinfo->name);
                res = retres = ERR_NCX_DEF_NOT_FOUND;
                tkc->curerr = &appinfo->tkerr;
                ncx_print_errormsg(tkc, mod, retres);
            } else {
                res = NO_ERR;
            }
        }

        if (res == NO_ERR) {
            appinfo->ext = ext;
            if (ext->arg && !appinfo->value) {
                retres = ERR_NCX_MISSING_PARM;
                log_error("\nError: argument missing for extension '%s:%s' ",
                          appinfo->prefix, ext->name);
                tkc->curerr = &appinfo->tkerr;
                ncx_print_errormsg(tkc, mod, retres);
            } else if (!ext->arg && appinfo->value) {
                retres = ERR_NCX_EXTRA_PARM;
                log_error("\nError: argument '%s' provided for"
                          " extension '%s:%s' is not allowed",
                          appinfo->value, 
                          appinfo->prefix, 
                          ext->name);
                tkc->curerr = &appinfo->tkerr;
                ncx_print_errormsg(tkc, mod, retres);
            }
        }

        /* recurse through any nested appinfo statements */
        res = ncx_resolve_appinfoQ(pcb, tkc, mod, appinfo->appinfoQ);
        CHK_EXIT(res, retres);
    }

    return retres;

}  /* ncx_resolve_appinfoQ */


/********************************************************************
* FUNCTION ncx_new_iffeature
* 
* Get a new ncx_iffeature_t struct
*
* INPUTS:
*    none
* RETURNS:
*    pointer to a malloced ncx_iffeature_t struct,
*    or NULL if malloc error
*********************************************************************/
ncx_iffeature_t *
    ncx_new_iffeature (void)
{
    ncx_iffeature_t *iff;

    iff = m__getObj(ncx_iffeature_t);
    if (!iff) {
        return NULL;
    }
    memset(iff, 0x0, sizeof(ncx_iffeature_t));

    return iff;

} /* ncx_new_iffeature */


/********************************************************************
* FUNCTION ncx_free_iffeature
* 
* Free a malloced ncx_iffeature_t struct
*
* INPUTS:
*    iff == struct to free
*
*********************************************************************/
void 
    ncx_free_iffeature (ncx_iffeature_t *iff)
{

#ifdef DEBUG
    if (!iff) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (iff->prefix) {
        m__free(iff->prefix);
    }
    if (iff->name) {
        m__free(iff->name);
    }

    m__free(iff);
    
} /* ncx_free_iffeature */


/********************************************************************
* FUNCTION ncx_clean_iffeatureQ
* 
* Clean a Q of malloced ncx_iffeature_t struct
*
* INPUTS:
*    iffeatureQ == address of Q to clean
*
*********************************************************************/
void 
    ncx_clean_iffeatureQ (dlq_hdr_t *iffeatureQ)
{

    ncx_iffeature_t  *iff;

#ifdef DEBUG
    if (!iffeatureQ) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    while (!dlq_empty(iffeatureQ)) {
        iff = (ncx_iffeature_t *)dlq_deque(iffeatureQ);
        ncx_free_iffeature(iff);
    }
    
} /* ncx_clean_iffeatureQ */


/********************************************************************
* FUNCTION ncx_find_iffeature
* 
* Search a Q of ncx_iffeature_t structs for a match
*
* INPUTS:
*    iffeatureQ == address of Q to search
*    prefix == prefix to check for
*              a NULL value indicates the current module
*    name == feature name string to find
*********************************************************************/
ncx_iffeature_t *
    ncx_find_iffeature (dlq_hdr_t *iffeatureQ,
                        const xmlChar *prefix,
                        const xmlChar *name,
                        const xmlChar *modprefix)
{
    ncx_iffeature_t  *iff;

#ifdef DEBUG
    if (!iffeatureQ || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (iff = (ncx_iffeature_t *)
             dlq_firstEntry(iffeatureQ);
         iff != NULL;
         iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

        /* check if name fields the same */
        if (iff->name && !xml_strcmp(iff->name, name)) {

            /* check if prefix fields reference
             * different modules, if set or implied
             */
            if (!ncx_prefix_different(prefix,
                                      iff->prefix,
                                      modprefix)) {
                return iff;
            }
        }
    }
    return NULL;
    
} /* ncx_find_iffeature */


/********************************************************************
* FUNCTION ncx_new_feature
* 
* Get a new ncx_feature_t struct
*
* INPUTS:
*    none
* RETURNS:
*    pointer to a malloced ncx_feature_t struct,
*    or NULL if malloc error
*********************************************************************/
ncx_feature_t *
    ncx_new_feature (void)
{
    ncx_feature_t *feature;

    feature = m__getObj(ncx_feature_t);
    if (!feature) {
        return NULL;
    }
    memset(feature, 0x0, sizeof(ncx_feature_t));

    dlq_createSQue(&feature->iffeatureQ);
    dlq_createSQue(&feature->appinfoQ);

    /*** setting feature enabled as the default
     *** the agent code needs to adjust this
     *** with agt_disable_feature() ifneeded
     ***/
    feature->enabled = TRUE;

    return feature;

} /* ncx_new_feature */


/********************************************************************
* FUNCTION ncx_free_feature
* 
* Free a malloced ncx_feature_t struct
*
* INPUTS:
*    feature == struct to free
*
*********************************************************************/
void 
    ncx_free_feature (ncx_feature_t *feature)
{

#ifdef DEBUG
    if (!feature) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (feature->name) {
        m__free(feature->name);
    }

    if (feature->descr) {
        m__free(feature->descr);
    }

    if (feature->ref) {
        m__free(feature->ref);
    }

    ncx_clean_iffeatureQ(&feature->iffeatureQ);

    ncx_clean_appinfoQ(&feature->appinfoQ);

    m__free(feature);
    
} /* ncx_free_feature */


/********************************************************************
* FUNCTION ncx_find_feature
* 
* Find a ncx_feature_t struct in the module and perhaps
* any of its submodules
*
* INPUTS:
*    mod == module to search
*    name == feature name to find
*
* RETURNS:
*    pointer to found feature or NULL if not found
*********************************************************************/
ncx_feature_t *
    ncx_find_feature (ncx_module_t *mod,
                      const xmlChar *name)
{
    ncx_feature_t  *feature;
    dlq_hdr_t      *que;
    yang_node_t    *node;
    ncx_include_t  *inc;

#ifdef DEBUG
    if (!mod || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    feature = ncx_find_feature_que(&mod->featureQ, name);
    if (feature) {
        return feature;
    }

    que = (mod->allincQ) ? mod->allincQ : &mod->saveincQ;

    /* check all the submodules, but only the ones visible
     * to this module or submodule, YANG only
     */
    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
         inc != NULL;
         inc = (ncx_include_t *)dlq_nextEntry(inc)) {

        /* get the real submodule struct */
        if (!inc->submod) {
            node = yang_find_node(que, 
                                  inc->submodule,
                                  inc->revision);
            if (node) {
                inc->submod = node->submod;
            }
            if (!inc->submod) {
                /* include not found, should not be in Q !!! */
                SET_ERROR(ERR_INTERNAL_VAL);
                continue;
            }
        }

        /* check the type Q in this submodule */
        feature = ncx_find_feature_que(&inc->submod->featureQ, name);
        if (feature) {
            return feature;
        }
    }

    return NULL;

} /* ncx_find_feature */


/********************************************************************
* FUNCTION ncx_find_feature_que
* 
* Find a ncx_feature_t struct in the specified Q
*
* INPUTS:
*    featureQ == Q of ncx_feature_t to search
*    name == feature name to find
*
* RETURNS:
*    pointer to found feature or NULL if not found
*********************************************************************/
ncx_feature_t *
    ncx_find_feature_que (dlq_hdr_t *featureQ,
                          const xmlChar *name)
{
    ncx_feature_t *feature;

#ifdef DEBUG
    if (!featureQ || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (feature = (ncx_feature_t *)dlq_firstEntry(featureQ);
         feature != NULL;
         feature = (ncx_feature_t *)dlq_nextEntry(feature)) {

        if (!xml_strcmp(feature->name, name)) {
            return feature;
        }
    }
    return NULL;
         
} /* ncx_find_feature_que */


/********************************************************************
* FUNCTION ncx_for_all_features
* 
* Execute a callback function for all features in this module
* and any submodules
*
* INPUTS:
*    mod == module to search for features
*    cbfn == feature callback function
*    cookie == cookie value to pass to each iteration of the callback
*    enabledonly == TRUE if only callbacks for enabled features
*                   FALSE if all features should invoke callbacks
*********************************************************************/
void
    ncx_for_all_features (const ncx_module_t *mod,
                          ncx_feature_cbfn_t  cbfn,
                          void *cookie,
                          boolean enabledonly)
{
    ncx_feature_t        *feature;
    const dlq_hdr_t      *que;
    yang_node_t          *node;
    ncx_include_t        *inc;
    boolean               keepgoing;

#ifdef DEBUG
    if (!mod || !cbfn) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    keepgoing = TRUE;

    for (feature = (ncx_feature_t *)dlq_firstEntry(&mod->featureQ);
         feature != NULL && keepgoing;
         feature = (ncx_feature_t *)dlq_nextEntry(feature)) {

        if (enabledonly && !ncx_feature_enabled(feature)) {
            continue;
        }

        keepgoing = (*cbfn)(mod, feature, cookie);
    }   
        
    que = (mod->allincQ) ? mod->allincQ : &mod->saveincQ;

    /* check all the submodules, but only the ones visible
     * to this module or submodule, YANG only
     */
    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
         inc != NULL && keepgoing;
         inc = (ncx_include_t *)dlq_nextEntry(inc)) {

        /* get the real submodule struct */
        if (!inc->submod) {
            node = yang_find_node(que, 
                                  inc->submodule,
                                  inc->revision);
            if (node) {
                inc->submod = node->submod;
            }
            if (!inc->submod) {
                /* include not found, should not be in Q !!! */
                SET_ERROR(ERR_INTERNAL_VAL);
                continue;
            }
        }

        for (feature = (ncx_feature_t *)
                 dlq_firstEntry(&inc->submod->featureQ);
             feature != NULL && keepgoing;
             feature = (ncx_feature_t *)dlq_nextEntry(feature)) {

            if (enabledonly && !ncx_feature_enabled(feature)) {
                continue;
            }

            keepgoing = (*cbfn)(mod, feature, cookie);
        }
    }

} /* ncx_for_all_features */


/********************************************************************
* FUNCTION ncx_feature_count
* 
* Get the total feature count for this module
* and any submodules
*
* INPUTS:
*    mod == module to search for features
*    enabledonly == TRUE to only count enabled features
*                   FALSE to count all features
*********************************************************************/
uint32
    ncx_feature_count (const ncx_module_t *mod,
                       boolean enabledonly)
{
    const ncx_feature_t  *feature;
    const yang_node_t    *node;
    const dlq_hdr_t      *que;
    ncx_include_t        *inc;
    uint32                count;

#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    count = 0;

    for (feature = (const ncx_feature_t *)dlq_firstEntry(&mod->featureQ);
         feature != NULL;
         feature = (const ncx_feature_t *)dlq_nextEntry(feature)) {

        if (enabledonly && !ncx_feature_enabled(feature)) {
            continue;
        }

        count++;
    }   
        
    que = (mod->allincQ) ? mod->allincQ : &mod->saveincQ;

    /* check all the submodules, but only the ones visible
     * to this module or submodule, YANG only
     */
    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
         inc != NULL;
         inc = (ncx_include_t *)dlq_nextEntry(inc)) {

        /* get the real submodule struct */
        if (!inc->submod) {
            node = yang_find_node(que, 
                                  inc->submodule,
                                  inc->revision);
            if (node) {
                inc->submod = node->submod;
            }
            if (!inc->submod) {
                /* include not found, should not be in Q !!! */
                SET_ERROR(ERR_INTERNAL_VAL);
                continue;
            }
        }

        for (feature = (const ncx_feature_t *)
                 dlq_firstEntry(&inc->submod->featureQ);
             feature != NULL;
             feature = (const ncx_feature_t *)dlq_nextEntry(feature)) {

            if (enabledonly && !ncx_feature_enabled(feature)) {
                continue;
            }
            count++;
        }
    }
    return count;

} /* ncx_feature_count */


/********************************************************************
* FUNCTION ncx_feature_enabled
* 
* Check if the specified feature and any referenced
* if-features are enabled
*
* INPUTS:
*    feature == feature to check
*
* RETURNS:
*   TRUE if feature is completely enabled
*   FALSE if feature is not enabled, or partially enabled
*********************************************************************/
boolean
    ncx_feature_enabled (const ncx_feature_t *feature)
{
    const ncx_iffeature_t  *iffeature;

#ifdef DEBUG
    if (!feature) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (!feature->enabled) {
        return FALSE;
    }

    /* make sure all nested if-features are also enabled */
    for (iffeature = (const ncx_iffeature_t *)
             dlq_firstEntry(&feature->iffeatureQ);
         iffeature != NULL;
         iffeature = (const ncx_iffeature_t *)
             dlq_nextEntry(iffeature)) {

        if (!iffeature->feature) {
            /* feature was not found, so call it disabled */
            return FALSE;
        }

        if (!ncx_feature_enabled(iffeature->feature)) {
            return FALSE;
        }
    }

    return TRUE;

} /* ncx_feature_enabled */


/********************************************************************
* FUNCTION ncx_new_identity
* 
* Get a new ncx_identity_t struct
*
* INPUTS:
*    none
* RETURNS:
*    pointer to a malloced ncx_identity_t struct,
*    or NULL if malloc error
*********************************************************************/
ncx_identity_t *
    ncx_new_identity (void)
{
    ncx_identity_t *identity;

    identity = m__getObj(ncx_identity_t);
    if (!identity) {
        return NULL;
    }
    memset(identity, 0x0, sizeof(ncx_identity_t));

    dlq_createSQue(&identity->childQ);
    dlq_createSQue(&identity->appinfoQ);
    identity->idlink.identity = identity;
    return identity;

} /* ncx_new_identity */


/********************************************************************
* FUNCTION ncx_free_identity
* 
* Free a malloced ncx_identity_t struct
*
* INPUTS:
*    identity == struct to free
*
*********************************************************************/
void 
    ncx_free_identity (ncx_identity_t *identity)
{

#ifdef DEBUG
    if (!identity) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    /*** !!! ignoring the back-ptr Q threading the
     *** !!! idlink headers; do not delete from system
     *** !!! until some way to clear all or issue an error
     *** !!! is done.  Assume this free is part of the 
     *** !!! system cleanup for now
     ***
     *** !!! Clearing out the back-ptrs in case the heap 'free'
     *** !!! function does not set these fields to garbage
     ***/
    identity->idlink.inq = FALSE;
    identity->idlink.identity = NULL;

    if (identity->name) {
        m__free(identity->name);
    }

    if (identity->baseprefix) {
        m__free(identity->baseprefix);
    }

    if (identity->basename) {
        m__free(identity->basename);
    }

    if (identity->descr) {
        m__free(identity->descr);
    }

    if (identity->ref) {
        m__free(identity->ref);
    }

    ncx_clean_appinfoQ(&identity->appinfoQ);

    m__free(identity);
    
} /* ncx_free_identity */


/********************************************************************
* FUNCTION ncx_find_identity
* 
* Find a ncx_identity_t struct in the module and perhaps
* any of its submodules
*
* INPUTS:
*    mod == module to search
*    name == identity name to find
*
* RETURNS:
*    pointer to found feature or NULL if not found
*********************************************************************/
ncx_identity_t *
    ncx_find_identity (ncx_module_t *mod,
                       const xmlChar *name)
{
    ncx_identity_t  *identity;
    dlq_hdr_t       *que;
    yang_node_t     *node;
    ncx_include_t   *inc;

#ifdef DEBUG
    if (!mod || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    identity = ncx_find_identity_que(&mod->identityQ, name);
    if (identity) {
        return identity;
    }

    que = (mod->allincQ) ? mod->allincQ : &mod->saveincQ;

    /* check all the submodules, but only the ones visible
     * to this module or submodule, YANG only
     */
    for (inc = (ncx_include_t *)dlq_firstEntry(&mod->includeQ);
         inc != NULL;
         inc = (ncx_include_t *)dlq_nextEntry(inc)) {

        /* get the real submodule struct */
        if (!inc->submod) {
            node = yang_find_node(que, 
                                  inc->submodule,
                                  inc->revision);
            if (node) {
                inc->submod = node->submod;
            }
            if (!inc->submod) {
                /* include not found, should not be in Q !!! */
                SET_ERROR(ERR_INTERNAL_VAL);
                continue;
            }
        }

        /* check the type Q in this submodule */
        identity = ncx_find_identity_que(&inc->submod->identityQ, name);
        if (identity) {
            return identity;
        }
    }

    return NULL;

} /* ncx_find_identity */


/********************************************************************
* FUNCTION ncx_find_identity_que
* 
* Find a ncx_identity_t struct in the specified Q
*
* INPUTS:
*    identityQ == Q of ncx_identity_t to search
*    name == identity name to find
*
* RETURNS:
*    pointer to found identity or NULL if not found
*********************************************************************/
ncx_identity_t *
    ncx_find_identity_que (dlq_hdr_t *identityQ,
                           const xmlChar *name)
{
    ncx_identity_t *identity;

#ifdef DEBUG
    if (!identityQ || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (identity = (ncx_identity_t *)dlq_firstEntry(identityQ);
         identity != NULL;
         identity = (ncx_identity_t *)dlq_nextEntry(identity)) {

        if (!xml_strcmp(identity->name, name)) {
            return identity;
        }
    }
    return NULL;
         
} /* ncx_find_identity_que */


/********************************************************************
* FUNCTION ncx_new_filptr
* 
* Get a new ncx_filptr_t struct
*
* INPUTS:
*    none
* RETURNS:
*    pointer to a malloced or cached ncx_filptr_t struct,
*    or NULL if none available
*********************************************************************/
ncx_filptr_t *
    ncx_new_filptr (void)
{
    ncx_filptr_t *filptr;

    /* check the cache first */
    if (ncx_cur_filptrs) {
        filptr = (ncx_filptr_t *)dlq_deque(&ncx_filptrQ);
        ncx_cur_filptrs--;
        return filptr;
    }
    
    /* create a new one */
    filptr = m__getObj(ncx_filptr_t);
    if (!filptr) {
        return NULL;
    }
    memset (filptr, 0x0, sizeof(ncx_filptr_t));
    dlq_createSQue(&filptr->childQ);
    return filptr;

} /* ncx_new_filptr */


/********************************************************************
* FUNCTION ncx_free_filptr
* 
* Free a new ncx_filptr_t struct or add to the cache if room
*
* INPUTS:
*    filptr == struct to free
* RETURNS:
*    none
*********************************************************************/
void 
    ncx_free_filptr (ncx_filptr_t *filptr)
{

    ncx_filptr_t *fp;

#ifdef DEBUG
    if (!filptr) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    /* recursively clean out the child Queues */
    while (!dlq_empty(&filptr->childQ)) {
        fp = (ncx_filptr_t *)dlq_deque(&filptr->childQ);
        ncx_free_filptr(fp);
    }

    /* check if any malloced memory inside */
    if (filptr->virtualnode) {
        val_free_value(filptr->virtualnode);
    }

    /* check if this entry should be put in the cache */
    if (ncx_cur_filptrs < ncx_max_filptrs) {
        memset(filptr, 0x0, sizeof(ncx_filptr_t));
        dlq_createSQue(&filptr->childQ);
        dlq_enque(filptr, &ncx_filptrQ);
        ncx_cur_filptrs++;
    } else {
        /* cache full, so just delete this entry */
        m__free(filptr);
    }
    
} /* ncx_free_filptr */


/********************************************************************
* FUNCTION ncx_new_revhist
* 
* Create a revision history entry
*
* RETURNS:
*    malloced revision history entry or NULL if malloc error
*********************************************************************/
ncx_revhist_t *
    ncx_new_revhist (void)
{
    ncx_revhist_t *revhist;

    revhist = m__getObj(ncx_revhist_t);
    if (!revhist) {
        return NULL;
    }
    memset(revhist, 0x0, sizeof(ncx_revhist_t));
    return revhist;

}  /* ncx_new_revhist */


/********************************************************************
* FUNCTION ncx_free_revhist
* 
* Free a revision history entry
*
* INPUTS:
*    revhist == ncx_revhist_t data structure to free
*********************************************************************/
void 
    ncx_free_revhist (ncx_revhist_t *revhist)
{
#ifdef DEBUG
    if (!revhist) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (revhist->version) {
        m__free(revhist->version);
    }
    if (revhist->descr) {
        m__free(revhist->descr);
    }
    if (revhist->ref) {
        m__free(revhist->ref);
    }
    m__free(revhist);

}  /* ncx_free_revhist */


/********************************************************************
* FUNCTION ncx_find_revhist
* 
* Search the revhistQ for a specified revision
* 
* INPUTS:
*   mod == module to search (mod->importQ)
*   ver == version string to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_revhist_t * 
    ncx_find_revhist (const ncx_module_t *mod,
                      const xmlChar *ver)
{
    ncx_revhist_t  *revhist;

#ifdef DEBUG
    if (!mod || !ver) {
        return NULL;
    }
#endif

    for (revhist = (ncx_revhist_t *)dlq_firstEntry(&mod->revhistQ);
         revhist != NULL;
         revhist = (ncx_revhist_t *)dlq_nextEntry(revhist)) {
        if (!xml_strcmp(revhist->version, ver)) {
            return revhist;
        }
    }
    return NULL;

} /* ncx_find_revhist */


/********************** ncx_enum_t *********************/


/********************************************************************
* FUNCTION ncx_init_enum
* 
* Init the memory of a ncx_enum_t
*
* INPUTS:
*    enu == ncx_enum_t struct to init
*********************************************************************/
void
    ncx_init_enum (ncx_enum_t *enu)
{
#ifdef DEBUG
    if (!enu) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    enu->name = NULL;
    enu->dname = NULL;
    enu->val = 0;

} /* ncx_init_enum */


/********************************************************************
* FUNCTION ncx_clean_enum
* 
* Scrub the memory of a ncx_enum_t but do not delete it
*
* INPUTS:
*    enu == ncx_enum_t struct to clean
*********************************************************************/
void
    ncx_clean_enum (ncx_enum_t *enu)
{
#ifdef DEBUG
    if (!enu) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    enu->name = NULL;
    if (enu->dname) {
        m__free(enu->dname);
        enu->dname = NULL;
    }
    enu->val = 0;

} /* ncx_clean_enum */


/********************************************************************
* FUNCTION ncx_compare_enums
* 
* Compare 2 enum values
*
* INPUTS:
*    enu1 == first  ncx_enum_t check
*    enu2 == second ncx_enum_t check
*   
* RETURNS:
*     -1 if enu1 is < enu2
*      0 if enu1 == enu2
*      1 if enu1 is > enu2

*********************************************************************/
int32
    ncx_compare_enums (const ncx_enum_t *enu1,
                       const ncx_enum_t *enu2)
{
#ifdef DEBUG
    if (!enu1 || !enu2) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    /*** !!! just check strings, not int value !!! */
    return xml_strcmp(enu1->name, enu2->name);

} /* ncx_compare_enums */


/********************** ncx_bit_t *********************/


/********************************************************************
* FUNCTION ncx_init_bit
* 
* Init the memory of a ncx_bit_t
*
* INPUTS:
*    bit == ncx_bit_t struct to init
*********************************************************************/
void
    ncx_init_bit (ncx_bit_t *bit)
{
#ifdef DEBUG
    if (!bit) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    bit->name = NULL;
    bit->dname = NULL;
    bit->pos = 0;

} /* ncx_init_bit */


/********************************************************************
* FUNCTION ncx_clean_bit
* 
* Scrub the memory of a ncx_bit_t but do not delete it
*
* INPUTS:
*    bit == ncx_bit_t struct to clean
*********************************************************************/
void
    ncx_clean_bit (ncx_bit_t *bit)
{
#ifdef DEBUG
    if (!bit) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (bit->dname) {
        m__free(bit->dname);
        bit->dname = NULL;
    }
    bit->pos = 0;
    bit->name = NULL;

} /* ncx_clean_bit */


/********************************************************************
* FUNCTION ncx_compare_bits
* 
* Compare 2 bit values by their schema order position
*
* INPUTS:
*    bitone == first ncx_bit_t check
*    bitone == second ncx_bit_t check
*   
* RETURNS:
*     -1 if bitone is < bittwo
*      0 if bitone == bittwo
*      1 if bitone is > bittwo
*
*********************************************************************/
int32
    ncx_compare_bits (const ncx_bit_t *bitone,
                      const ncx_bit_t *bittwo)
{
#ifdef DEBUG
    if (!bitone || !bittwo) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    if (bitone->pos < bittwo->pos) {
        return -1;
    } else if (bitone->pos > bittwo->pos) {
        return 1;
    } else {
        return 0;
    }
    /*NOTREACHED*/

} /* ncx_compare_bits */


/********************************************************************
* FUNCTION ncx_decode_enum
* 
* Parse an enumerated integer string into its 2 parts
*
* Form 1: name only : foo
* Form 2: number only : 16
* Form 3: name and number : foo(16)
*
* INPUTS:
*    enumval == enum string value to parse
*    retval == pointer to return integer variable
*    retlen == pointer to return string name length variable
* OUTPUTS:
*    *retval == integer value of enum
*    *retset == TRUE if *retval is set
*    *retlen == length of enumval that is the name portion
* RETURNS:
*    status
*********************************************************************/
status_t
    ncx_decode_enum (const xmlChar *enumval,
                     int32 *retval,
                     boolean *retset,
                     uint32 *retlen)
{
    status_t       res, res2;
    const xmlChar  *str1, *str2;
    xmlChar        numstr[NCX_MAX_NUMLEN];
    uint32         i;
    ncx_num_t      num;

#ifdef DEBUG
    if (!enumval ||!retval ||!retset || !retlen) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    /* split the buffer into name and value parts as needed */
    if (isdigit(*((const char *)enumval))) {
        /* Can only be the 2nd form -- number only */
        res = ncx_decode_num(enumval, NCX_BT_INT32, &num);
        if (res == NO_ERR) {
            *retval = num.i;
            *retset = TRUE;
            *retlen = 0;
            return NO_ERR;
        }
    } else {
        /* look for the 3rd form -- name and number */
        str1 = enumval;
        while (*str1 && (*str1 != NCX_ENU_START)) {
            str1++;
        }
        if (!*str1) {
            /* did not find any left paren
             * can only be the 1st form 'foo'
             */
            *retval = 0;
            *retset = FALSE;
            *retlen = (uint32)(str1-enumval);
            return NO_ERR;
        } else {
            /* found a left paren -- get a number and rparen */
            numstr[0] = 0;
            str2 = str1+1;
            for (i=0; i<NCX_MAX_NUMLEN && *str2!=NCX_ENU_END; i++) {
                numstr[i] = *str2++;
            }
            if (i==NCX_MAX_NUMLEN) {
                /* ran out of buffer before right paren 
                 * the number couldn't be valid if this happens
                 */
                return ERR_NCX_NUMLEN_TOOBIG;
            } else {
                /* setup the string return now */
                *retlen = (uint32)(str1-enumval);

                /* terminate the number buffer */
                numstr[i] = 0;

                /* make sure the enum is terminated properly */
                if (*(str2+1)) {
                    /* should be zero -- treat this as a warning */
                    res = ERR_NCX_EXTRA_ENUMCH;
                } else {
                    res = NO_ERR;
                }
            }
            res2 = ncx_decode_num(numstr, NCX_BT_INT32, &num);
            if (res2 == NO_ERR) {
                /* return the name and number that was decoded */
                *retval = num.i;
                *retset = TRUE;
            } else {
                res = res2;   /* drop the res warning if set */
            }
        }
    }

    return res;

} /* ncx_decode_enum */


/********************************************************************
* FUNCTION ncx_set_enum
* 
* Parse an enumerated integer string into an ncx_enum_t
* without matching it against any typdef
*
* Mallocs a copy of the enum name, using the enu->dname field
*
* INPUTS:
*    enumval == enum string value to parse
*    retenu == pointer to return enuym variable to fill in
*    
* OUTPUTS:
*    *retenu == enum filled in
*
* RETURNS:
*    status
*********************************************************************/
status_t
    ncx_set_enum (const xmlChar *enumval,
                  ncx_enum_t *retenu)
{
    xmlChar       *str;
    int32          ev;
    boolean        evset;
    uint32         namlen;
    status_t       res;

#ifdef DEBUG
    if (!enumval ||!retenu) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = ncx_decode_enum(enumval, &ev, &evset, &namlen);
    if (res != NO_ERR) {
        return res;
    }
    
    str = m__getMem(namlen+1);
    if (!str) {
        return ERR_INTERNAL_MEM;
    }
    xml_strncpy(str, enumval, namlen);

    retenu->dname = str;
    retenu->name = str;
    retenu->val = ev;

    return NO_ERR;

} /* ncx_set_enum */


/********************** ncx_typname_t *********************/


/********************************************************************
* FUNCTION ncx_new_typname
* 
*   Malloc and init a typname struct
*
* RETURNS:
*   malloced struct or NULL if memory error
*********************************************************************/
ncx_typname_t *
    ncx_new_typname (void)
{
    ncx_typname_t  *tn;

    tn = m__getObj(ncx_typname_t);
    if (!tn) {
        return NULL;
    }
    memset(tn, 0x0, sizeof(ncx_typname_t));
    return tn;

} /* ncx_new_typname */


/********************************************************************
* FUNCTION ncx_free_typname
* 
*   Free a typname struct
*
* INPUTS:
*    typnam == ncx_typname_t struct to free
*
*********************************************************************/
void
    ncx_free_typname (ncx_typname_t *typnam)
{
#ifdef DEBUG
    if (!typnam) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif
    if (typnam->typname_malloc) {
        m__free(typnam->typname_malloc);
    }
    m__free(typnam);

} /* ncx_free_typname */


/********************************************************************
* FUNCTION ncx_find_typname
* 
*   Find a typname struct in the specified Q for a typ pointer
*
* INPUTS:
*    que == Q of ncx_typname_t struct to check
*    typ == matching type template to find
*
* RETURNS:
*   name assigned to this type template
*********************************************************************/
const xmlChar *
    ncx_find_typname (const typ_template_t *typ,
                      const dlq_hdr_t *que)
{
    const ncx_typname_t  *tn;

#ifdef DEBUG
    if (!typ || !que) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (tn = (const ncx_typname_t *)dlq_firstEntry(que);
         tn != NULL;
         tn = (const ncx_typname_t *)dlq_nextEntry(tn)) {
        if (tn->typ == typ) {
            return tn->typname;
        }
    }
    return NULL;

} /* ncx_find_typname */


/********************************************************************
* FUNCTION ncx_find_typname_type
* 
*   Find a typ_template_t pointer in a typename mapping, 
*   in the specified Q
*
* INPUTS:
*    que == Q of ncx_typname_t struct to check
*    typname == matching type name to find
*
* RETURNS:
*   pointer to the stored typstatus
*********************************************************************/
const typ_template_t *
    ncx_find_typname_type (const dlq_hdr_t *que,
                           const xmlChar *typname)
{
    const ncx_typname_t  *tn;

#ifdef DEBUG
    if (!que || !typname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (tn = (const ncx_typname_t *)dlq_firstEntry(que);
         tn != NULL;
         tn = (const ncx_typname_t *)dlq_nextEntry(tn)) {
        if (!xml_strcmp(tn->typname, typname)) {
            return tn->typ;
        }
    }
    return NULL;

}  /* ncx_find_typname_type */


/********************************************************************
* FUNCTION ncx_clean_typnameQ
* 
*   Delete all the Q entries, of typname mapping structs
*
* INPUTS:
*    que == Q of ncx_typname_t struct to delete
*
*********************************************************************/
void
    ncx_clean_typnameQ (dlq_hdr_t *que)
{
    ncx_typname_t  *tn;

#ifdef DEBUG
    if (!que) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    while (!dlq_empty(que)) {
        tn = (ncx_typname_t *)dlq_deque(que);
        ncx_free_typname(tn);
    }

}  /* ncx_clean_typnameQ */


/********************************************************************
* FUNCTION ncx_get_gen_anyxml
* 
* Get the object template for the NCX generic anyxml container
*
*********************************************************************/
obj_template_t *
    ncx_get_gen_anyxml (void)
{
    if (!stage2_init_done) {
        return NULL;
    }
    return gen_anyxml;

} /* ncx_get_gen_anyxml */


/********************************************************************
* FUNCTION ncx_get_gen_container
* 
* Get the object template for the NCX generic container
*
*********************************************************************/
obj_template_t *
    ncx_get_gen_container (void)
{
    if (!stage2_init_done) {
        return NULL;
    }
    return gen_container;

} /* ncx_get_gen_container */


/********************************************************************
* FUNCTION ncx_get_gen_string
* 
* Get the object template for the NCX generic string leaf
*
*********************************************************************/
obj_template_t *
    ncx_get_gen_string (void)
{
    if (!stage2_init_done) {
        return NULL;
    }
    return gen_string;

} /* ncx_get_gen_string */


/********************************************************************
* FUNCTION ncx_get_gen_empty
* 
* Get the object template for the NCX generic empty leaf
*
*********************************************************************/
obj_template_t *
    ncx_get_gen_empty (void)
{
    if (!stage2_init_done) {
        return NULL;
    }
    return gen_empty;

} /* ncx_get_gen_empty */


/********************************************************************
* FUNCTION ncx_get_gen_root
* 
* Get the object template for the NCX generic root container
*
*********************************************************************/
obj_template_t *
    ncx_get_gen_root (void)
{
    if (!stage2_init_done) {
        return NULL;
    }
    return gen_root;

} /* ncx_get_gen_root */


/********************************************************************
* FUNCTION ncx_get_gen_binary
* 
* Get the object template for the NCX generic binary leaf
*
*********************************************************************/
obj_template_t *
    ncx_get_gen_binary (void)
{
    if (!stage2_init_done) {
        return NULL;
    }
    return gen_binary;

} /* ncx_get_gen_binary */


/********************************************************************
* FUNCTION ncx_get_layer
*
* Get the ncx_layer_t string
*
* INPUTS:
*   layer == ncx_layer_t to convert to a string
*
* RETURNS:
*  const pointer to the string value
*********************************************************************/
const xmlChar *
    ncx_get_layer (ncx_layer_t  layer)
{
    switch (layer) {
    case NCX_LAYER_NONE:
        return (const xmlChar *)"none";
    case NCX_LAYER_TRANSPORT:
        return (const xmlChar *)"transport";
    case NCX_LAYER_RPC:
        return (const xmlChar *)"rpc";
    case NCX_LAYER_OPERATION:
        return (const xmlChar *)"protocol";
    case NCX_LAYER_CONTENT:
        return (const xmlChar *)"application";
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return (const xmlChar *)"--";
    }
}  /* ncx_get_layer */


/********************************************************************
* FUNCTION ncx_get_name_segment
* 
* Get the name string between the dots
*
* INPUTS:
*    str == scoped string
*    buff == address of return buffer
*    buffsize == buffer size
*
* OUTPUTS:
*    buff is filled in with the namestring segment
*
* RETURNS:
*    current string pointer after operation
*********************************************************************/
const xmlChar *
    ncx_get_name_segment (const xmlChar *str,
                          xmlChar  *buff,
                          uint32 buffsize)
{
    const xmlChar *teststr;

#ifdef DEBUG
    if (!str || !buff) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    teststr = str;
    while (*teststr && *teststr != NCX_SCOPE_CH) {
        teststr++;
    }

    if ((uint32)(teststr - str) >= buffsize) {
        SET_ERROR(ERR_BUFF_OVFL);
        return NULL;
    }

    while (*str && *str != NCX_SCOPE_CH) {
        *buff++ = *str++;
    }
    *buff = 0;
    return str;

} /* ncx_get_name_segment */


/********************************************************************
* FUNCTION ncx_get_cvttyp_enum
* 
* Get the enum for the string name of a ncx_cvttyp_t enum
* 
* INPUTS:
*   str == string name of the enum value 
*
* RETURNS:
*   enum value
*********************************************************************/
ncx_cvttyp_t
    ncx_get_cvttyp_enum (const char *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_CVTTYP_NONE;
    }
#endif

    if (!xml_strcmp(NCX_EL_XSD, (const xmlChar *)str)) {
        return NCX_CVTTYP_XSD;
    } else if (!xml_strcmp(NCX_EL_SQL, (const xmlChar *)str)) {
        return NCX_CVTTYP_SQL;
    } else if (!xml_strcmp(NCX_EL_SQLDB, (const xmlChar *)str)) {
        return NCX_CVTTYP_SQLDB;
    } else if (!xml_strcmp(NCX_EL_HTML, (const xmlChar *)str)) {
        return NCX_CVTTYP_HTML;
    } else if (!xml_strcmp(NCX_EL_H, (const xmlChar *)str)) {
        return NCX_CVTTYP_H;
    } else if (!xml_strcmp(NCX_EL_C, (const xmlChar *)str)) {
        return NCX_CVTTYP_C;
    } else if (!xml_strcmp(NCX_EL_YANG, (const xmlChar *)str)) {
        return NCX_CVTTYP_YANG;
    } else if (!xml_strcmp(NCX_EL_COPY, (const xmlChar *)str)) {
        return NCX_CVTTYP_COPY;
    } else if (!xml_strcmp(NCX_EL_YIN, (const xmlChar *)str)) {
        return NCX_CVTTYP_YIN;
    } else {
        return NCX_CVTTYP_NONE;
    }
    /*NOTREACHED*/

}  /* ncx_get_cvttype_enum */


/********************************************************************
* FUNCTION ncx_get_status_enum
* 
* Get the enum for the string name of a ncx_status_t enum
* 
* INPUTS:
*   str == string name of the enum value 
*
* RETURNS:
*   enum value
*********************************************************************/
ncx_status_t
    ncx_get_status_enum (const xmlChar *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_STATUS_NONE;
    }
#endif

    if (!xml_strcmp(NCX_EL_CURRENT, str)) {
        return NCX_STATUS_CURRENT;
    } else if (!xml_strcmp(NCX_EL_DEPRECATED, str)) {
        return NCX_STATUS_DEPRECATED;
    } else if (!xml_strcmp(NCX_EL_OBSOLETE, str)) {
        return NCX_STATUS_OBSOLETE;
    } else {
        return NCX_STATUS_NONE;
    }
    /*NOTREACHED*/

}  /* ncx_get_status_enum */


/********************************************************************
* FUNCTION ncx_get_status_string
* 
* Get the string for the enum value of a ncx_status_t enum
* 
* INPUTS:
*   status == enum value
*
* RETURNS:
*   string name of the enum value 
*********************************************************************/
const xmlChar *
    ncx_get_status_string (ncx_status_t status)
{
    switch (status) {
    case NCX_STATUS_CURRENT:
    case NCX_STATUS_NONE:
        return NCX_EL_CURRENT;
    case NCX_STATUS_DEPRECATED:
        return NCX_EL_DEPRECATED;
    case NCX_STATUS_OBSOLETE:
        return NCX_EL_OBSOLETE;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return (const xmlChar *)"none";
    }
    /*NOTREACHED*/

}  /* ncx_get_status_string */


/********************************************************************
* FUNCTION ncx_check_yang_status
* 
* Check the backward compatibility of the 2 YANG status fields
* 
* INPUTS:
*   mystatus == enum value for the node to be tested
*   depstatus == status value of the dependency
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t
    ncx_check_yang_status (ncx_status_t mystatus,
                           ncx_status_t depstatus)
{
    switch (mystatus) {
    case NCX_STATUS_CURRENT:
        /* current definition can use another
         * current definition 
         */
        switch (depstatus) {
        case NCX_STATUS_CURRENT:
            return NO_ERR;
        case NCX_STATUS_DEPRECATED:
            return ERR_NCX_USING_DEPRECATED;
        case NCX_STATUS_OBSOLETE:
            return ERR_NCX_USING_OBSOLETE;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        /*NOTRECHED*/
    case NCX_STATUS_DEPRECATED:
        /* deprecated definition can use anything but an 
         * an obsolete definition
         */
        switch (depstatus) {
        case NCX_STATUS_CURRENT:
        case NCX_STATUS_DEPRECATED:
            return NO_ERR;
        case NCX_STATUS_OBSOLETE:
            return ERR_NCX_USING_OBSOLETE;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        /*NOTREACHED*/
    case NCX_STATUS_OBSOLETE:
        /* obsolete definition can use any definition */
        return NO_ERR;
    case NCX_STATUS_NONE:
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    /*NOTREACHED*/

}  /* ncx_check_yang_status */


/********************************************************************
* FUNCTION ncx_save_descr
* 
* Get the value of the save description strings variable
*
* RETURNS:
*    TRUE == descriptive strings should be save
*    FALSE == descriptive strings should not be saved
*********************************************************************/
boolean 
    ncx_save_descr (void)
{
    return save_descr;
}  /* ncx_save_descr */


/********************************************************************
* FUNCTION ncx_print_errormsg
* 
*   Print an parse error message to STDOUT
*
* INPUTS:
*   tkc == token chain   (may be NULL)
*   mod == module in progress  (may be NULL)
*   res == error status
*
* RETURNS:
*   none
*********************************************************************/
void
    ncx_print_errormsg (tk_chain_t *tkc,
                        ncx_module_t  *mod,
                        status_t     res)
{
    ncx_print_errormsg_ex(tkc, mod, res, NULL, 0, TRUE);

} /* ncx_print_errormsg */


/********************************************************************
* FUNCTION ncx_print_errormsg_ex
* 
*   Print an parse error message to STDOUT (Extended)
*
* INPUTS:
*   tkc == token chain   (may be NULL)
*   mod == module in progress  (may be NULL)
*   res == error status
*   filename == script finespec
*   linenum == script file number
*   fineoln == TRUE if finish with a newline, FALSE if not
*
* RETURNS:
*   none
*********************************************************************/
void
    ncx_print_errormsg_ex (tk_chain_t *tkc,
                           ncx_module_t  *mod,
                           status_t     res,
                           const char *filename,
                           uint32 linenum,
                           boolean fineoln)
{
    boolean      iserr;

    if (res == NO_ERR) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }

    iserr = (res <= ERR_LAST_USR_ERR) ? TRUE : FALSE;

    if (!iserr && !ncx_warning_enabled(res)) {
        if (LOGDEBUG3) {
            log_debug3("\nSuppressed warning %d (%s.%u)",
                       res, 
                       get_error_string(res),
                       mod->name,
                       linenum);
        }
        return;
    }

    if (mod) {
        if (iserr) {
            mod->errors++;
        } else {
            mod->warnings++;
        }
    }

    if (iserr) {
        if (!LOGERROR) {
            /* errors turned off by the user! */
            return;
        }
    } else if (!LOGWARN) {
        /* warnings turned off by the user */
        return;
    }

    if (tkc && tkc->curerr && tkc->curerr->mod) {
        log_write("\n%s:", (tkc->curerr->mod->sourcefn) ? 
                  (const char *)tkc->curerr->mod->sourcefn : "--");
    } else if (mod && mod->sourcefn) {
        log_write("\n%s:", (mod->sourcefn) ? 
                  (const char *)mod->sourcefn : "--");
    } else if (tkc && tkc->filename) {
        log_write("\n%s:", tkc->filename);
    } else if (filename) {
        log_write("\n%s:", filename);
        if (linenum) {
            log_write("line %u:", linenum);
        }
    } else {
        log_write("\n");
    }

    if (tkc) {
        if (tkc->curerr && tkc->curerr->mod) {
            log_write("%u.%u:", 
                      tkc->curerr->linenum, 
                      tkc->curerr->linepos);
        } else if (tkc->cur && 
                   (tkc->cur != (tk_token_t *)&tkc->tkQ) &&
                   TK_CUR_VAL(tkc)) {
            log_write("%u.%u:", 
                      TK_CUR_LNUM(tkc), 
                      TK_CUR_LPOS(tkc));
        } else {
            log_write("%u.%u:", 
                      tkc->linenum, 
                      tkc->linepos);

        }
        tkc->curerr = NULL;
    }

    if (iserr) {
        log_write(" error(%u): %s", res, get_error_string(res));
    } else {
        log_write(" warning(%u): %s", res, get_error_string(res));
    }

    if (fineoln) {
        log_write("\n");
    }

} /* ncx_print_errormsg_ex */


/********************************************************************
* FUNCTION ncx_conf_exp_err
* 
* Print an error for wrong token, expected a different token
* 
* INPUTS:
*   tkc == token chain
*   result == error code
*   expstr == expected token description
*
*********************************************************************/
void
    ncx_conf_exp_err (tk_chain_t  *tkc,
                      status_t result,
                      const char *expstr)
{
    ncx_print_errormsg_ex(tkc, 
                          NULL, 
                          result, 
                          NULL, 
                          0,
                          (expstr) ? FALSE : TRUE);
    if (expstr) {
        log_write("  Expected: %s\n", expstr);
    }

}  /* ncx_conf_exp_err */


/********************************************************************
* FUNCTION ncx_mod_exp_err
* 
* Print an error for wrong token, expected a different token
* 
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   result == error code
*   expstr == expected token description
*
*********************************************************************/
void
    ncx_mod_exp_err (tk_chain_t  *tkc,
                     ncx_module_t *mod,
                     status_t result,
                     const char *expstr)
{
    const char *gotval;
    tk_type_t   tktyp;
    boolean     skip, done;
    uint32      skipcount;
    status_t    res;

    skip = FALSE;
    if (TK_CUR(tkc)) {
        tktyp = TK_CUR_TYP(tkc);
    } else {
        tktyp = TK_TT_NONE;
    }

    if (tktyp == TK_TT_NONE) {
        gotval = NULL;
    } else if (TK_CUR_TYP(tkc)==TK_TT_TSTRING || TK_CUR_NUM(tkc)) {
        gotval = (const char *)TK_CUR_VAL(tkc);
    } else if (TK_CUR_TYP(tkc) == TK_TT_LBRACE) {
        gotval = "left brace, skipping to closing right brace";
        skip = TRUE;
    } else {
        gotval = tk_get_token_name(tktyp);
    }

    if (LOGERROR) {
        if (gotval && expstr) {
            log_write("\nError:  Got '%s', Expected: %s", gotval, expstr);
        } else if (expstr) {
            log_write("\nError:  Expected: %s", expstr);
        }
        ncx_print_errormsg_ex(tkc, 
                              mod, 
                              result, 
                              NULL, 
                              0,
                              (expstr) ? FALSE : TRUE);
        log_error("\n");
    }

    if (skip) {
        /* got an unexpected left brace, so skip to the
         * end of this unknown section to resynch;
         * otherwise the first unknown closing right brace
         * will end the parent section, which causes
         * a false 'unexpected EOF' error
         */
        skipcount = 1;
        done = FALSE;
        res = NO_ERR;
        while (!done && res == NO_ERR) {
            res = TK_ADV(tkc);
            if (res == NO_ERR) {
                tktyp = TK_CUR_TYP(tkc);
                if (tktyp == TK_TT_LBRACE) {
                    skipcount++;
                } else if (tktyp == TK_TT_RBRACE) {
                    skipcount--;
                }
                if (!skipcount) {
                    done = TRUE;
                }
            }
        }
    }

}  /* ncx_mod_exp_err */


/********************************************************************
* FUNCTION ncx_free_node
* 
* Delete a node based on its type
*
* INPUTS:
*     nodetyp == NCX node type
*     node == node top free
*
*********************************************************************/
void
    ncx_free_node (ncx_node_t nodetyp,
                   void *node)
{
#ifdef DEBUG
    if (!node) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    switch (nodetyp) {
    case NCX_NT_NONE:                          /* uninitialized */
        m__free(node);
        break;
    case NCX_NT_TYP:                          /* typ_template_t */
        typ_free_template(node);
        break;
    case NCX_NT_GRP:                          /* grp_template_t */
        grp_free_template(node);
        break;
    case NCX_NT_VAL:                             /* val_value_t */
        val_free_value(node);
        break;
    case NCX_NT_OBJ:                          /* obj_template_t */
        obj_free_template(node);
        break;
    case NCX_NT_STRING:                       /* xmlChar string */
        m__free(node);
        break;
    case NCX_NT_CFG:                          /* cfg_template_t */
        cfg_free_template(node);
        break;
    case NCX_NT_QNAME:                         /* xmlns_qname_t */
        xmlns_free_qname(node);
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        m__free(node);
    }

} /* ncx_free_node */


/********************************************************************
* FUNCTION ncx_get_data_class_enum
* 
* Get the enum for the string name of a ncx_data_class_t enum
* 
* INPUTS:
*   str == string name of the enum value 
*
* RETURNS:
*   enum value
*********************************************************************/
ncx_data_class_t
    ncx_get_data_class_enum (const xmlChar *str)
{
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_DC_NONE;
    } else if (!xml_strcmp(NCX_EL_CONFIG, str)) {
        return NCX_DC_CONFIG;
    } else if (!xml_strcmp(NCX_EL_STATE, str)) {
        return NCX_DC_STATE;
    } else {
        /* SET_ERROR(ERR_INTERNAL_VAL); */
        return NCX_DC_NONE;
    }
    /*NOTREACHED*/

}  /* ncx_get_data_class_enum */


/********************************************************************
* FUNCTION ncx_get_data_class_str
* 
* Get the string value for the ncx_data_class_t enum
* 
* INPUTS:
*   dataclass == enum value to convert
*
* RETURNS:
*   striong value for the enum
*********************************************************************/
const xmlChar *
    ncx_get_data_class_str (ncx_data_class_t dataclass)
{
    switch (dataclass) {
    case NCX_DC_NONE:
        return NULL;
    case NCX_DC_CONFIG:
        return NCX_EL_CONFIG;
    case NCX_DC_STATE:
        return NCX_EL_STATE;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }
    /*NOTREACHED*/

}  /* ncx_get_data_class_str */


/********************************************************************
* FUNCTION ncx_get_access_str
* 
* Get the string name of a ncx_access_t enum
* 
* INPUTS:
*   access == enum value
*
* RETURNS:
*   string value
*********************************************************************/
const xmlChar * 
    ncx_get_access_str (ncx_access_t max_access)
{
    switch (max_access) {
    case NCX_ACCESS_NONE:    return (const xmlChar *) "not set";
    case NCX_ACCESS_RO:      return NCX_EL_ACCESS_RO;
    case NCX_ACCESS_RW:      return NCX_EL_ACCESS_RW;
    case NCX_ACCESS_RC:      return NCX_EL_ACCESS_RC;
    default:                 return (const xmlChar *) "illegal";
    }
    /*NOTREACHED*/

}  /* ncx_get_access_str */


/********************************************************************
* FUNCTION ncx_get_access_enum
* 
* Get the enum for the string name of a ncx_access_t enum
* 
* INPUTS:
*   str == string name of the enum value 
*
* RETURNS:
*   enum value
*********************************************************************/
ncx_access_t
    ncx_get_access_enum (const xmlChar *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_ACCESS_NONE;
    }
#endif

    if (!xml_strcmp(NCX_EL_ACCESS_RO, str)) {
        return NCX_ACCESS_RO;
    } else if (!xml_strcmp(NCX_EL_ACCESS_RW, str)) {
        return NCX_ACCESS_RW;
    } else if (!xml_strcmp(NCX_EL_ACCESS_RC, str)) {
        return NCX_ACCESS_RC;
    } else {
        SET_ERROR(ERR_INTERNAL_VAL);
        return NCX_ACCESS_NONE;
    }
    /*NOTREACHED*/

}  /* ncx_get_access_enum */


/********************************************************************
* FUNCTION ncx_get_tclass
* 
* Get the token class
*
* INPUTS:
*     btyp == base type enum
* RETURNS:
*     tclass enum
*********************************************************************/
ncx_tclass_t
    ncx_get_tclass (ncx_btype_t btyp)
{
    switch (btyp) {
    case NCX_BT_NONE:
        return NCX_CL_NONE;
    case NCX_BT_ANY:
    case NCX_BT_BOOLEAN:
    case NCX_BT_BITS:
    case NCX_BT_ENUM:
    case NCX_BT_EMPTY:
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
    case NCX_BT_INT64:
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
    case NCX_BT_UINT64:
    case NCX_BT_DECIMAL64:
    case NCX_BT_FLOAT64:
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_LEAFREF:
    case NCX_BT_SLIST:
    case NCX_BT_UNION:
        return NCX_CL_SIMPLE;
    case NCX_BT_CONTAINER:
    case NCX_BT_LIST:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
        return NCX_CL_COMPLEX;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return NCX_CL_NONE;
    }
}  /* ncx_get_tclass */


/********************************************************************
* FUNCTION ncx_valid_name_ch
* 
* Check if an xmlChar is a valid NCX name string char
* INPUTS:
*   ch == xmlChar to check
* RETURNS:
*   TRUE if a valid name char, FALSE otherwise
*********************************************************************/
boolean
    ncx_valid_name_ch (uint32 ch)
{
    char c;

    if (ch & bit7) {
        return FALSE;       /* TEMP -- handling ASCII only */
    } else {
        c = (char)ch;
        return (isalpha(c) || isdigit(c) || c=='_' || c=='-' || c=='.')
            ? TRUE : FALSE;
    }
    /*NOTREACHED*/
} /* ncx_valid_name_ch */


/********************************************************************
* FUNCTION ncx_valid_fname_ch
* 
* Check if an xmlChar is a valid NCX name string first char
*
* INPUTS:
*   ch == xmlChar to check
* RETURNS:
*   TRUE if a valid first name char, FALSE otherwise
*********************************************************************/
boolean
    ncx_valid_fname_ch (uint32 ch)
{
    char c;

    if (ch & bit7) {
        return FALSE;       /* TEMP -- handling ASCII only */
    } else {
        c = (char)ch;
        return (isalpha(c) || (c=='_')) ? TRUE : FALSE;
    }
    /*NOTREACHED*/
} /* ncx_valid_fname_ch */


/********************************************************************
* FUNCTION ncx_valid_name
* 
* Check if an xmlChar string is a valid YANG identifier value
*
* INPUTS:
*   str == xmlChar string to check
*   len == length of the string to check (in case of substr)
* RETURNS:
*   TRUE if a valid name string, FALSE otherwise
*********************************************************************/
boolean
    ncx_valid_name (const xmlChar *str, 
                    uint32 len)
{
    uint32  i;

#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (len == 0 || len > NCX_MAX_NLEN) {
        return FALSE;
    }
    if (!ncx_valid_fname_ch(*str)) {
        return FALSE;
    }
    for (i=1; i<len; i++) {
        if (!ncx_valid_name_ch(str[i])) {
            return FALSE;
        }
    }

    if (len >= 3 &&
        ((*str == 'X' || *str == 'x') &&
         (str[1] == 'M' || str[1] == 'm') &&
         (str[2] == 'L' || str[2] == 'l'))) {
        return FALSE;
    }

    return TRUE;

} /* ncx_valid_name */


/********************************************************************
* FUNCTION ncx_valid_name2
* 
* Check if an xmlChar string is a valid NCX name
*
* INPUTS:
*   str == xmlChar string to check (zero-terminated)

* RETURNS:
*   TRUE if a valid name string, FALSE otherwise
*********************************************************************/
boolean
    ncx_valid_name2 (const xmlChar *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return ncx_valid_name(str, xml_strlen(str));

} /* ncx_valid_name2 */


/********************************************************************
* FUNCTION ncx_parse_name
* 
* Check if the next N chars represent a valid NcxName
* Will end on the first non-name char
*
* INPUTS:
*   str == xmlChar string to check
*   len == address of name length
*
* OUTPUTS:
*   *len == 0 if no valid name parsed
*         > 0 for the numbers of chars in the NcxName
*
* RETURNS:
*   status_t  (error if name too long)
*********************************************************************/
status_t
    ncx_parse_name (const xmlChar *str,
                    uint32 *len)
{
    const xmlChar *s;

#ifdef DEBUG
    if (!str || !len) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (!ncx_valid_fname_ch(*str)) {
        *len = 0;
        return ERR_NCX_INVALID_NAME;
    }

    s = str+1;

    while (ncx_valid_name_ch(*s)) {
        s++;
    }
    *len = (uint32)(s - str);
    if (*len > NCX_MAX_NLEN) {
        return ERR_NCX_TOO_BIG;
    } else {
        return NO_ERR;
    }

} /* ncx_parse_name */


/********************************************************************
* FUNCTION ncx_is_true
* 
* Check if an xmlChar string is a string OK for XSD boolean
*
* INPUTS:
*   str == xmlChar string to check
*
* RETURNS:
*   TRUE if a valid boolean value indicating true
*   FALSE otherwise
*********************************************************************/
boolean
    ncx_is_true (const xmlChar *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (!xml_strcmp(str, (const xmlChar *)"true") ||
        !xml_strcmp(str, (const xmlChar *)"1")) {
        return TRUE;
    } else {
        return FALSE;
    }

} /* ncx_is_true */


/********************************************************************
* FUNCTION ncx_is_false
* 
* Check if an xmlChar string is a string OK for XSD boolean
*
* INPUTS:
*   str == xmlChar string to check
*
* RETURNS:
*   TRUE if a valid boolean value indicating false
*   FALSE otherwise
*********************************************************************/
boolean
    ncx_is_false (const xmlChar *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (!xml_strcmp(str, (const xmlChar *)"false") ||
        !xml_strcmp(str, (const xmlChar *)"0")) {
        return TRUE;
    } else {
        return FALSE;
    }

} /* ncx_is_false */


/*********   P A R S E R   H E L P E R   F U N C T I O N S   *******/


/********************************************************************
* FUNCTION ncx_consume_tstring
* 
* Consume a TK_TT_TSTRING with the specified value
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain 
*   mod == module in progress (NULL if none)
*   name == token name
*   opt == TRUE for optional param
*       == FALSE for mandatory param
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_consume_tstring (tk_chain_t *tkc,
                         ncx_module_t *mod,
                         const xmlChar *name,
                         ncx_opt_t opt)
{
    status_t     res;

#ifdef DEBUG
    if (!tkc || !name) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 
#endif

    res = TK_ADV(tkc);

    if (res == NO_ERR) {
        if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
            if (opt==NCX_OPT) {
                TK_BKUP(tkc);
                return ERR_NCX_SKIPPED;
            } else {
                res = ERR_NCX_WRONG_TKTYPE;
            }
        } else {
            if (xml_strcmp(TK_CUR_VAL(tkc), name)) {
                if (opt==NCX_OPT) {
                    TK_BKUP(tkc);
                    return ERR_NCX_SKIPPED;
                } else {
                    res = ERR_NCX_WRONG_TKVAL;
                }
            }
        }
    }

    if (res != NO_ERR) {
        ncx_print_errormsg(tkc, mod, res);
    }

    return res;

} /* ncx_consume_tstring */


/********************************************************************
* FUNCTION ncx_consume_name
* 
* Consume a TK_TSTRING that matches the 'name', then
* retrieve the next TK_TSTRING token into the namebuff
* If ctk specified, then consume the specified close token
*
* Store the results in a malloced buffer
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain 
*   mod == module in progress (NULL if none)
*   name == first token name
*   namebuff == ptr to output name string
*   opt == NCX_OPT for optional param
*       == NCX_REQ for mandatory param
*   ctyp == close token (use TK_TT_NONE to skip this part)
*
* OUTPUTS:
*   *namebuff points at the malloced name string
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_consume_name (tk_chain_t *tkc,
                      ncx_module_t *mod,
                      const xmlChar *name,
                      xmlChar **namebuff,
                      ncx_opt_t opt,
                      tk_type_t  ctyp)
{
    const char  *expstr;
    status_t     res, retres;

#ifdef DEBUG
    if (!tkc || !name || !namebuff) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    retres = NO_ERR;
    expstr = "name string";

    /* check 'name' token */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        ncx_mod_exp_err(tkc, mod, res, expstr);
        return res;
    }
    if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
        if (opt==NCX_OPT) {
            TK_BKUP(tkc);
            return ERR_NCX_SKIPPED;
        } else {
            res = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
    }
    
    if (res==NO_ERR && xml_strcmp(TK_CUR_VAL(tkc), name)) {
        if (opt==NCX_OPT) {
            TK_BKUP(tkc);
            return ERR_NCX_SKIPPED;
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
    }

    retres = res;
    expstr = "name string";

    /* check string value token */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        ncx_mod_exp_err(tkc, mod, res, expstr);
        return res;
    } else {
        if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
            res = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        } else {
            *namebuff = xml_strdup(TK_CUR_VAL(tkc));
            if (!*namebuff) {
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
                return res;
            }
        }
    }

    retres = res;
    expstr = "closing token";

    /* check for a closing token */
    if (ctyp != TK_TT_NONE) {
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_mod_exp_err(tkc, mod, res, expstr);
        } else {
            if (TK_CUR_TYP(tkc) != ctyp) {
                res = ERR_NCX_WRONG_TKTYPE;
                ncx_mod_exp_err(tkc, mod, res, expstr);
            }
        }
        CHK_EXIT(res, retres);
    }

    return retres;

} /* ncx_consume_name */


/********************************************************************
* FUNCTION ncx_consume_token
* 
* Consume the next token which should be a 1 or 2 char token
* without any value. However this function does not check the value,
* just the token type.
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain 
*   mod == module in progress (NULL if none)
*   ttyp == token type
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_consume_token (tk_chain_t *tkc,
                       ncx_module_t *mod,
                       tk_type_t  ttyp)
{
    const char  *tkname;
    status_t     res;

#ifdef DEBUG
    if (!tkc) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    res = (TK_CUR_TYP(tkc) == ttyp) ? 
        NO_ERR : ERR_NCX_WRONG_TKTYPE;

    if (res != NO_ERR) {
        tkname = tk_get_token_name(ttyp);
        switch (tkc->source) {
        case TK_SOURCE_YANG:
            ncx_mod_exp_err(tkc, mod, res, tkname);

            /* if a token is missing and the token
             * parsed instead looks like the continuation
             * of a statement, or the end of a section,
             * then backup and let parsing continue
             */
            switch (ttyp) {
            case TK_TT_SEMICOL:
            case TK_TT_LBRACE:
                switch (TK_CUR_TYP(tkc)) {
                case TK_TT_TSTRING:
                case TK_TT_MSTRING:
                case TK_TT_RBRACE:
                    TK_BKUP(tkc);
                    break;
                default:
                    ;
                }
                break;
            case TK_TT_RBRACE:
                switch (TK_CUR_TYP(tkc)) {
                case TK_TT_TSTRING:
                case TK_TT_MSTRING:
                    TK_BKUP(tkc);
                    break;
                default:
                    ;
                }
                break;

            default:
                ;
            }
            break;
        default:
            ;
        }
    }

    return res;

} /* ncx_consume_token */


/********************************************************************
* FUNCTION ncx_new_errinfo
* 
* Malloc and init a new ncx_errinfo_t
*
* RETURNS:
*    pointer to malloced ncx_errinfo_t, or NULL if memory error
*********************************************************************/
ncx_errinfo_t *
    ncx_new_errinfo (void)
{
    ncx_errinfo_t *err;

    err = m__getObj(ncx_errinfo_t);
    if (!err) {
        return NULL;
    }
    ncx_init_errinfo(err);
    return err;

}  /* ncx_new_errinfo */


/********************************************************************
* FUNCTION ncx_init_errinfo
* 
* Init the fields in an ncx_errinfo_t struct
*
* INPUTS:
*    err == ncx_errinfo_t data structure to init
*********************************************************************/
void 
    ncx_init_errinfo (ncx_errinfo_t *err)
{
#ifdef DEBUG
    if (!err) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    memset(err, 0x0, sizeof(ncx_errinfo_t));

}  /* ncx_init_errinfo */


/********************************************************************
* FUNCTION ncx_clean_errinfo
* 
* Scrub the memory in a ncx_errinfo_t by freeing all
* the sub-fields
*
* INPUTS:
*    err == ncx_errinfo_t data structure to clean
*********************************************************************/
void 
    ncx_clean_errinfo (ncx_errinfo_t *err)
{
#ifdef DEBUG
    if (!err) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (err->descr) {
        m__free(err->descr);
        err->descr = NULL;
    }
    if (err->ref) {
        m__free(err->ref);
        err->ref = NULL;
    }
    if (err->error_app_tag) {
        m__free(err->error_app_tag);
        err->error_app_tag = NULL;
    }
    if (err->error_message) {
        m__free(err->error_message);
        err->error_message = NULL;
    }

}  /* ncx_clean_errinfo */


/********************************************************************
* FUNCTION ncx_free_errinfo
* 
* Scrub the memory in a ncx_errinfo_t by freeing all
* the sub-fields, then free the errinfo struct
*
* INPUTS:
*    err == ncx_errinfo_t data structure to free
*********************************************************************/
void 
    ncx_free_errinfo (ncx_errinfo_t *err)
{
#ifdef DEBUG
    if (!err) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    ncx_clean_errinfo(err);
    m__free(err);

}  /* ncx_free_errinfo */


/********************************************************************
* FUNCTION ncx_errinfo_set
* 
* Check if the errinfo struct is set or empty
* Checks only the error_app_tag and error_message fields
*
* INPUTS:
*    errinfo == ncx_errinfo_t struct to check
*
* RETURNS:
*   TRUE if at least one field set
*   FALSE if the errinfo struct is empty

*********************************************************************/
boolean
    ncx_errinfo_set (const ncx_errinfo_t *errinfo)
{

#ifdef DEBUG
    if (!errinfo) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    if (errinfo->error_app_tag || errinfo->error_message) {
        return TRUE;
    } else {
        return FALSE;
    }

}  /* ncx_errinfo_set */


/********************************************************************
* FUNCTION ncx_copy_errinfo
* 
* Copy the fields from one errinfo to a blank errinfo
*
* INPUTS:
*    src == struct with starting contents
*    dest == struct to get copy of src contents
*
* OUTPUTS:
*    *dest fields set which are set in src
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ncx_copy_errinfo (const ncx_errinfo_t *src,
                      ncx_errinfo_t *dest)
{
#ifdef DEBUG
    if (!src || !dest) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (src->descr) {
        if (dest->descr) {
            m__free(dest->descr);
        }
        dest->descr = xml_strdup(src->descr);
        if (!dest->descr) {
            return ERR_INTERNAL_MEM;
        }
    }

    if (src->ref) {
        if (dest->ref) {
            m__free(dest->ref);
        }
        dest->ref = xml_strdup(src->ref);
        if (!dest->ref) {
            return ERR_INTERNAL_MEM;
        }
    }

    if (src->error_app_tag) {
        if (dest->error_app_tag) {
            m__free(dest->error_app_tag);
        }
        dest->error_app_tag = xml_strdup(src->error_app_tag);
        if (!dest->error_app_tag) {
            return ERR_INTERNAL_MEM;
        }
    }

    if (src->error_message) {
        if (dest->error_message) {
            m__free(dest->error_message);
        }
        dest->error_message = xml_strdup(src->error_message);
        if (!dest->error_message) {
            return ERR_INTERNAL_MEM;
        }
    }

    return NO_ERR;

}  /* ncx_copy_errinfo */


/********************************************************************
* FUNCTION ncx_get_source
* 
* Get a malloced buffer containing the complete filespec
* for the given input string.  If this is a complete dirspec,
* this this will just strdup the value.
*
* This is just a best effort to get the full spec.
* If the full spec is greater than 1500 bytes,
* then a NULL value (error) will be returned
*
*   - Change ./ --> cwd/
*   - Remove ~/  --> $HOME
*   - add trailing '/' if not present
*
* INPUTS:
*    fspec == input filespec
*    res == address of return status
*
* OUTPUTS:
*   *res == return status, NO_ERR if return is non-NULL
*
* RETURNS:
*   malloced buffer containing possibly expanded full filespec
*********************************************************************/
xmlChar *
    ncx_get_source (const xmlChar *fspec,
                    status_t *res)
{
    const xmlChar  *p, *start, *user;
    xmlChar        *buff, *bp;
    uint32          bufflen, userlen, len;
    
#define DIRBUFF_SIZE 1500

#ifdef DEBUG
    if (!fspec || !res) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
    if (!*fspec) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }
#endif

    *res = NO_ERR;
    buff = NULL;
    user = NULL;
    len = 0;
    p = fspec;

    if (*p == NCXMOD_PSCHAR) {
        /* absolute path */
        buff = xml_strdup(fspec);
        if (!buff) {
            *res = ERR_INTERNAL_MEM;
        }
    } else if (*p == NCXMOD_HMCHAR) {
        /* starts with ~[username]/some/path */
        if (p[1] && p[1] != NCXMOD_PSCHAR) {
            /* explicit user name */
            start = &p[1];   
            p = &p[2];
            while (*p && *p != NCXMOD_PSCHAR) {
                p++;
            }
            userlen = (uint32)(p-start);
            user = ncxmod_get_userhome(start, userlen);
        } else {
            /* implied current user */
            p++;   /* skip ~ char */
            /* get current user home dir */
            user = ncxmod_get_userhome(NULL, 0);
        }

        if (user == NULL) {
            log_error("\nError: invalid user name in path string (%s)",
                      fspec);
            *res = ERR_NCX_INVALID_VALUE;
            return NULL;
        }

        /* string pointer 'p' stopped on the PSCHAR to start the
         * rest of the path string
         */
        len = xml_strlen(user) + xml_strlen(p);
        buff = m__getMem(len+1);
        if (buff == NULL) {
            *res = ERR_INTERNAL_MEM;
            return NULL;
        }

        bp = buff;
        bp += xml_strcpy(bp, user);
        xml_strcpy(bp, p);
    } else if (*p == NCXMOD_ENVCHAR) {
        /* should start with $ENVVAR/some/path */
        start = ++p;  /* skip dollar sign */
        while (*p && *p != NCXMOD_PSCHAR) {
                p++;
        }
        userlen = (uint32)(p-start);
        if (userlen) {
            user = ncxmod_get_envvar(start, userlen);
        }
        if (!user) {
            log_error("\nError: environment variable in path string (%s)",
                      fspec);
            *res = ERR_NCX_INVALID_VALUE;
            return NULL;
        }

        /* string pointer 'p' stopped on the PSCHAR to start the
         * rest of the path string
         */
        len = xml_strlen(user) + xml_strlen(p);
        buff = m__getMem(len+1);
        if (buff == NULL) {
            *res = ERR_INTERNAL_MEM;
            return NULL;
        }

        bp = buff;
        bp += xml_strcpy(bp, user);
        xml_strcpy(bp, p);
    } else if (*p == NCXMOD_DOTCHAR && p[1] == NCXMOD_PSCHAR) {
        /* check for ./some/path */
        p++;

        /* prepend string with current directory */
        buff = m__getMem(DIRBUFF_SIZE);
        if (buff == NULL) {
            *res = ERR_INTERNAL_MEM;
            return NULL;
        }

        if (!getcwd((char *)buff, DIRBUFF_SIZE)) {
            SET_ERROR(ERR_BUFF_OVFL);
            m__free(buff);
            return NULL;
        }
            
        bufflen = xml_strlen(buff);

        if ((bufflen + xml_strlen(p) + 1) >= DIRBUFF_SIZE) {
            *res = ERR_BUFF_OVFL;
            m__free(buff);
            return NULL;
        }

        xml_strcpy(&buff[bufflen], p);
    } else {
        buff = xml_strdup(fspec);
        if (buff == NULL) {
            *res = ERR_INTERNAL_MEM;
        }
    }

    return buff;

}  /* ncx_get_source */


/********************************************************************
* FUNCTION ncx_set_cur_modQ
* 
* Set the current module Q to an alternate (for yangdiff)
* This will be used for module searches usually in ncx_modQ
*
* INPUTS:
*    que == Q of ncx_module_t to use
*********************************************************************/
void
    ncx_set_cur_modQ (dlq_hdr_t *que)
{
#ifdef DEBUG
    if (!que) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    ncx_curQ = que;

}  /* ncx_set_cur_modQ */


/********************************************************************
* FUNCTION ncx_reset_modQ
* 
* Set the current module Q to the original ncx_modQ
*
*********************************************************************/
void
    ncx_reset_modQ (void)
{
    ncx_curQ = &ncx_modQ;

}  /* ncx_reset_modQ */


/********************************************************************
* FUNCTION ncx_set_session_modQ
* 
* !!! THIS HACK IS NEEDED BECAUSE val.c
* !!! USES ncx_find_module sometimes, and
* !!! yangcli sessions are not loaded into the
* !!! main database of modules.
* !!! THIS DOES NOT WORK FOR MULTIPLE CONCURRENT PROCESSES
*
* Set the current session module Q to an alternate (for yangdiff)
* This will be used for module searches usually in ncx_modQ
*
* INPUTS:
*    que == Q of ncx_module_t to use
*********************************************************************/
void
    ncx_set_session_modQ (dlq_hdr_t *que)
{
#ifdef DEBUG
    if (!que) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    ncx_sesmodQ = que;

}  /* ncx_set_sesion_modQ */


/********************************************************************
* FUNCTION ncx_clear_session_modQ
* 
* !!! THIS HACK IS NEEDED BECAUSE val.c
* !!! USES ncx_find_module sometimes, and
* !!! yangcli sessions are not loaded into the
* !!! main database of modules.
* !!! THIS DOES NOT WORK FOR MULTIPLE CONCURRENT PROCESSES
*
* Clear the current session module Q
*
*********************************************************************/
void
    ncx_clear_session_modQ (void)
{

    ncx_sesmodQ = NULL;

}  /* ncx_clear_sesion_modQ */


/********************************************************************
* FUNCTION ncx_set_load_callback
* 
* Set the callback function for a load-module event
*
* INPUT:
*   cbfn == callback function to use
*
*********************************************************************/
void
    ncx_set_load_callback (ncx_load_cbfn_t cbfn)
{

    mod_load_callback = cbfn;

}  /* ncx_set_load_callback */


/********************************************************************
* FUNCTION ncx_prefix_different
* 
* Check if the specified prefix pair reference different modules
* 
* INPUT:
*   prefix1 == 1st prefix to check (may be NULL)
*   prefix2 == 2nd prefix to check (may be NULL)
*   modprefix == module prefix to check (may be NULL)
*
* RETURNS:
*   TRUE if prefix1 and prefix2 reference different modules
*   FALSE if prefix1 and prefix2 reference the same modules
*********************************************************************/
boolean
    ncx_prefix_different (const xmlChar *prefix1,
                          const xmlChar *prefix2,
                          const xmlChar *modprefix)
{
    if (!prefix1) {
        prefix1 = modprefix;
    }
    if (!prefix2) {
        prefix2 = modprefix;
    }
    if (prefix1 == prefix2) {
        return FALSE;
    }
    if (prefix1 == NULL || prefix2 == NULL) {
        return TRUE;
    }
    return (xml_strcmp(prefix1, prefix2)) ? TRUE : FALSE;

}  /* ncx_prefix_different */


/********************************************************************
* FUNCTION ncx_get_baddata_enum
* 
* Check if the specified string matches an ncx_baddata_t enum
* 
* INPUT:
*   valstr == value string to check
*
* RETURNS:
*   enum value if OK
*   NCX_BAD_DATA_NONE if an error
*********************************************************************/
ncx_bad_data_t
    ncx_get_baddata_enum (const xmlChar *valstr)
{
#ifdef DEBUG
    if (!valstr) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_BAD_DATA_NONE;
    }
#endif

    if (!xml_strcmp(valstr, E_BAD_DATA_IGNORE)) {
        return NCX_BAD_DATA_IGNORE;
    } else if (!xml_strcmp(valstr, E_BAD_DATA_WARN)) {
        return NCX_BAD_DATA_WARN;
    } else if (!xml_strcmp(valstr, E_BAD_DATA_CHECK)) {
        return NCX_BAD_DATA_CHECK;
    } else if (!xml_strcmp(valstr, E_BAD_DATA_ERROR)) {
        return NCX_BAD_DATA_ERROR;
    } else {
        return NCX_BAD_DATA_NONE;
    }
    /*NOTREACHED*/

}  /* ncx_get_baddata_enum */


/********************************************************************
* FUNCTION ncx_get_baddata_string
* 
* Get the string for the specified enum value
* 
* INPUT:
*   baddatar == enum value to check
*
* RETURNS:
*   string pointer if OK
*   NULL if an error
*********************************************************************/
const xmlChar *
    ncx_get_baddata_string (ncx_bad_data_t baddata)
{
    switch (baddata) {
    case NCX_BAD_DATA_NONE:
        return NCX_EL_NONE;
    case NCX_BAD_DATA_IGNORE:
        return E_BAD_DATA_IGNORE;
    case NCX_BAD_DATA_WARN:
        return E_BAD_DATA_WARN;
    case NCX_BAD_DATA_CHECK:
        return E_BAD_DATA_CHECK;
    case NCX_BAD_DATA_ERROR:
        return E_BAD_DATA_ERROR;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }

}  /* ncx_get_baddata_string */


/********************************************************************
* FUNCTION ncx_get_withdefaults_string
* 
* Get the string for the specified enum value
* 
* INPUT:
*   withdef == enum value to check
*
* RETURNS:
*   string pointer if OK
*   NULL if an error
*********************************************************************/
const xmlChar *
    ncx_get_withdefaults_string (ncx_withdefaults_t withdef)
{
    switch (withdef) {
    case NCX_WITHDEF_NONE:
        return NCX_EL_NONE;
    case NCX_WITHDEF_REPORT_ALL:
        return NCX_EL_REPORT_ALL;
    case NCX_WITHDEF_TRIM:
        return NCX_EL_TRIM;
    case NCX_WITHDEF_EXPLICIT:
        return NCX_EL_EXPLICIT;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }

}  /* ncx_get_withdefaults_string */


/********************************************************************
* FUNCTION ncx_get_withdefaults_enum
* 
* Get the enum for the specified string value
* 
* INPUT:
*   withdefstr == string value to check
*
* RETURNS:
*   enum value for the string
*   NCX_WITHDEF_NONE if invalid value
*********************************************************************/
ncx_withdefaults_t
    ncx_get_withdefaults_enum (const xmlChar *withdefstr)
{
#ifdef DEBUG
    if (!withdefstr) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_WITHDEF_NONE;
    }
#endif

    if (!xml_strcmp(withdefstr, NCX_EL_REPORT_ALL)) {
        return NCX_WITHDEF_REPORT_ALL;
    } else if (!xml_strcmp(withdefstr, NCX_EL_TRIM)) {
        return NCX_WITHDEF_TRIM;
    } else if (!xml_strcmp(withdefstr, NCX_EL_EXPLICIT)) {
        return NCX_WITHDEF_EXPLICIT;
    } else {
        return NCX_WITHDEF_NONE;
    }

}  /* ncx_get_withdefaults_enum */


/********************************************************************
* FUNCTION ncx_get_mod_prefix
* 
* Get the module prefix for the specified module
* 
* INPUT:
*   mod == module to check
*
* RETURNS:
*   pointer to module YANG prefix
*********************************************************************/
const xmlChar *
    ncx_get_mod_prefix (const ncx_module_t *mod)
{
#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    return mod->prefix;

}  /* ncx_get_mod_prefix */


/********************************************************************
* FUNCTION ncx_get_mod_xmlprefix
* 
* Get the module XML prefix for the specified module
* 
* INPUT:
*   mod == module to check
*
* RETURNS:
*   pointer to module XML prefix
*********************************************************************/
const xmlChar *
    ncx_get_mod_xmlprefix (const ncx_module_t *mod)
{
#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    if (mod->xmlprefix) {
        return mod->xmlprefix;
    } else {
        return mod->prefix;
    }

}  /* ncx_get_mod_xmlprefix */



/********************************************************************
* FUNCTION ncx_get_dec64_base
* 
* Get the base part of a decimal64 number
* 
* INPUT:
*   num == number to check (expected to be NCX_BT_DECIMAL64)
*
* RETURNS:
*   base part of the number
*********************************************************************/
int64
    ncx_get_dec64_base (const ncx_num_t *num)
{
    int64 temp1;

#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif
    temp1 = num->dec.val;
    if (num->dec.digits) {
         temp1 = temp1 / (10 * num->dec.digits);
    }
    return temp1;

}  /* ncx_get_dec64_base */


/********************************************************************
* FUNCTION ncx_get_dec64_fraction
* 
* Get the fraction part of a decimal64 number
* 
* INPUT:
*   num == number to check (expected to be NCX_BT_DECIMAL64)
*
* RETURNS:
*   fraction part of the number
*********************************************************************/
int64
    ncx_get_dec64_fraction (const ncx_num_t *num)
{
    int64 temp1;

#ifdef DEBUG
    if (!num) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    if (num->dec.digits) {
        temp1 = num->dec.val % (10 * num->dec.digits);
    } else {
        temp1 = 0;
    }

    return temp1;

}  /* ncx_get_dec64_fraction */


/********************************************************************
* FUNCTION ncx_get_display_mode_enum
* 
* Get the enum for the specified string value
* 
* INPUT:
*   dmstr == string value to check
*
* RETURNS:
*   enum value for the string
*   NCX_DISPLAY_MODE_NONE if invalid value
*********************************************************************/
ncx_display_mode_t
    ncx_get_display_mode_enum (const xmlChar *dmstr)
{
#ifdef DEBUG
    if (!dmstr) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_DISPLAY_MODE_NONE;
    }
#endif

    if (!xml_strcmp(dmstr, NCX_EL_PLAIN)) {
        return NCX_DISPLAY_MODE_PLAIN;
    } else if (!xml_strcmp(dmstr, NCX_EL_PREFIX)) {
        return NCX_DISPLAY_MODE_PREFIX;
    } else if (!xml_strcmp(dmstr, NCX_EL_MODULE)) {
        return NCX_DISPLAY_MODE_MODULE;
    } else if (!xml_strcmp(dmstr, NCX_EL_XML)) {
        return NCX_DISPLAY_MODE_XML;
    } else {
        return NCX_DISPLAY_MODE_NONE;
    }

}  /* ncx_get_display_mode_enum */


/********************************************************************
* FUNCTION ncx_get_display_mode_str
* 
* Get the string for the specified enum value
* 
* INPUT:
*   dmode == enum display mode value to check
*
* RETURNS:
*   string value for the enum
*   NULL if none found
*********************************************************************/
const xmlChar *
    ncx_get_display_mode_str (ncx_display_mode_t dmode)
{
    switch (dmode) {
    case NCX_DISPLAY_MODE_NONE:
        return NULL;
    case NCX_DISPLAY_MODE_PLAIN:
        return NCX_EL_PLAIN;
    case NCX_DISPLAY_MODE_PREFIX:
        return NCX_EL_PREFIX;
    case NCX_DISPLAY_MODE_MODULE:
        return NCX_EL_MODULE;
    case NCX_DISPLAY_MODE_XML:
        return NCX_EL_XML;
    default:
        return NULL;
    }

}  /* ncx_get_display_mode_str */


/********************************************************************
* FUNCTION ncx_set_warn_idlen
* 
* Set the warning length for identifiers
* 
* INPUT:
*   warnlen == warning length to use
*
*********************************************************************/
void
    ncx_set_warn_idlen (uint32 warnlen)
{
    warn_idlen = warnlen;

}  /* ncx_set_warn_idlen */


/********************************************************************
* FUNCTION ncx_get_warn_idlen
* 
* Get the warning length for identifiers
* 
* RETURNS:
*   warning length to use
*
*********************************************************************/
uint32
    ncx_get_warn_idlen (void)
{
    return warn_idlen;

}  /* ncx_get_warn_idlen */


/********************************************************************
* FUNCTION ncx_set_warn_linelen
* 
* Set the warning length for YANG file lines
* 
* INPUT:
*   warnlen == warning length to use
*
*********************************************************************/
void
    ncx_set_warn_linelen (uint32 warnlen)
{
    warn_linelen = warnlen;

}  /* ncx_set_warn_linelen */


/********************************************************************
* FUNCTION ncx_get_warn_linelen
* 
* Get the warning length for YANG file lines
* 
* RETURNS:
*   warning length to use
*
*********************************************************************/
uint32
    ncx_get_warn_linelen (void)
{
    return warn_linelen;

}  /* ncx_get_warn_linelen */


/********************************************************************
* FUNCTION ncx_check_warn_idlen
* 
* Check if the identifier length is greater than
* the specified amount.
*
* INPUTS:
*   tkc == token chain to use (for warning message only)
*   mod == module (for warning message only)
*   id == identifier string to check; must be Z terminated
*
* OUTPUTS:
*    may generate log_warn ouput
*********************************************************************/
void
    ncx_check_warn_idlen (tk_chain_t *tkc,
                          ncx_module_t *mod,
                          const xmlChar *id)
{
    uint32   idlen;

#ifdef DEBUG
    if (!id) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (!warn_idlen) {
        return;
    }

    idlen = xml_strlen(id);
    if (idlen > warn_idlen) {
        log_warn("\nWarning: identifier '%s' length is %u chars, "
                 "limit is %u chars",
                 id,
                 idlen,
                 warn_idlen);
        ncx_print_errormsg(tkc, mod, ERR_NCX_IDLEN_EXCEEDED);
    }

} /* ncx_check_warn_idlen */


/********************************************************************
* FUNCTION ncx_check_warn_linelen
* 
* Check if the line display length is greater than
* the specified amount.
*
* INPUTS:
*   tkc == token chain to use
*   mod == module (used in warning message only
*   linelen == line length to check
*
* OUTPUTS:
*    may generate log_warn ouput
*********************************************************************/
void
    ncx_check_warn_linelen (tk_chain_t *tkc,
                            ncx_module_t *mod,
                            const xmlChar *line)
{
    const xmlChar   *str;
    uint32           len;

#ifdef DEBUG
    if (!line) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (!warn_linelen) {
        return;
    }

    str = line;
    len = 0;

    /* check for line starting with newline */
    if (*str == '\n') {
        str++;
    }

    /* get the display length */
    while (*str && *str != '\n') {
        if (*str == '\t') {
            len += 8;
        } else {
            len++;
        }
        str++;
    }

    if (len > warn_linelen) {
        log_warn("\nWarning: line is %u chars, limit is %u chars",
                 len,
                 warn_linelen);
        ncx_print_errormsg(tkc, mod, ERR_NCX_LINELEN_EXCEEDED);
    }

} /* ncx_check_warn_linelen */


/********************************************************************
* FUNCTION ncx_turn_off_warning
* 
* Add ar warning suppression entry
*
* INPUTS:
*   res == internal status code to suppress
*
* RETURNS:
*   status (duplicates are silently dropped)
*********************************************************************/
status_t
    ncx_turn_off_warning (status_t res)
{
    warnoff_t         *warnoff;

    if (res == NO_ERR) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res < ERR_WARN_BASE) {
        return ERR_NCX_INVALID_VALUE;
    }

    /* check if 'res' already entered */
    for (warnoff = (warnoff_t *)dlq_firstEntry(&warnoffQ);
         warnoff != NULL;
         warnoff = (warnoff_t *)dlq_nextEntry(warnoff)) {
        if (warnoff->res == res) {
            return NO_ERR;
        }
    }

    warnoff = m__getObj(warnoff_t);
    if (!warnoff) {
        return ERR_INTERNAL_MEM;
    }
    memset(warnoff, 0x0, sizeof(warnoff_t));
    warnoff->res = res;
    dlq_enque(warnoff, &warnoffQ);
    return NO_ERR;

} /* ncx_turn_off_warning */


/********************************************************************
* FUNCTION ncx_warning_enabled
* 
* Check if a specific status_t code is enabled
*
* INPUTS:
*   res == internal status code to check
*
* RETURNS:
*   TRUE if warning is enabled
*   FALSE if warning is suppressed
*********************************************************************/
boolean
    ncx_warning_enabled (status_t res)
{
    const warnoff_t   *warnoff;

    if (res < ERR_WARN_BASE) {
        return TRUE;
    }

    if (!LOGWARN) {
        return FALSE;
    }

    /* check if 'res' already entered */
    for (warnoff = (const warnoff_t *)dlq_firstEntry(&warnoffQ);
         warnoff != NULL;
         warnoff = (const warnoff_t *)dlq_nextEntry(warnoff)) {
        if (warnoff->res == res) {
            return FALSE;
        }
    }
    return TRUE;

} /* ncx_warning_enabled */


/********************************************************************
* FUNCTION ncx_get_version
* 
* Get the the Yuma version ID string
*
* INPUT:
*    buffer == buffer to hold the version string
*    buffsize == number of bytes in buffer
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ncx_get_version (xmlChar *buffer,
                     uint32 buffsize)
{
    xmlChar    *str;
    uint32      versionlen;

#ifdef DEBUG
    if (!buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    versionlen = xml_strlen(YUMA_VERSION) +
        xml_strlen((const xmlChar *)SVNVERSION) + 1;

    if (versionlen >= buffsize) {
        return ERR_BUFF_OVFL;
    }

    str = buffer;
    str += xml_strcpy(str, YUMA_VERSION);
    *str++ = '.';
    xml_strcpy(str, (const xmlChar *)SVNVERSION);
    return NO_ERR;

}  /* ncx_get_version */


/********************************************************************
* FUNCTION ncx_new_save_deviations
* 
* create a deviation save structure
*
* INPUTS:
*   devmodule == deviations module name
*   devrevision == deviation module revision (optional)
*   devnamespace == deviation module namespace URI value
*   devprefix == local module prefix (optional)
*
* RETURNS:
*   malloced and initialized save_deviations struct, 
*   or NULL if malloc error
*********************************************************************/
ncx_save_deviations_t *
    ncx_new_save_deviations (const xmlChar *devmodule,
                             const xmlChar *devrevision,
                             const xmlChar *devnamespace,
                             const xmlChar *devprefix)
{
    ncx_save_deviations_t  *savedev;

#ifdef DEBUG
    if (!devmodule || !devnamespace) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    savedev = m__getObj(ncx_save_deviations_t);
    if (savedev == NULL) {
        return NULL;
    }

    memset(savedev, 0x0, sizeof(ncx_save_deviations_t));
    dlq_createSQue(&savedev->importQ);
    dlq_createSQue(&savedev->deviationQ);

    savedev->devmodule = xml_strdup(devmodule);
    if (savedev->devmodule == NULL) {
        ncx_free_save_deviations(savedev);
        return NULL;
    }

    if (devprefix) {
        savedev->devprefix = xml_strdup(devprefix);
        if (savedev->devprefix == NULL) {
            ncx_free_save_deviations(savedev);
            return NULL;
        }
    }

    if (devrevision) {
        savedev->devrevision = xml_strdup(devrevision);
        if (savedev->devrevision == NULL) {
            ncx_free_save_deviations(savedev);
            return NULL;
        }
    }

    savedev->devnamespace = xml_strdup(devnamespace);
    if (savedev->devnamespace == NULL) {
        ncx_free_save_deviations(savedev);
        return NULL;
    }
    
    return savedev;

}  /* ncx_new_save_deviations */


/********************************************************************
* FUNCTION ncx_free_save_deviations
* 
* free a deviation save struct
*
* INPUT:
*    savedev == struct to clean and delete
*********************************************************************/
void
    ncx_free_save_deviations (ncx_save_deviations_t *savedev)
{
    ncx_import_t      *import;
    obj_deviation_t   *deviation;

#ifdef DEBUG
    if (!savedev) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    while (!dlq_empty(&savedev->importQ)) {
        import = (ncx_import_t *)
            dlq_deque(&savedev->importQ);
        ncx_free_import(import);
    }

    while (!dlq_empty(&savedev->deviationQ)) {
        deviation = (obj_deviation_t *)
            dlq_deque(&savedev->deviationQ);
        obj_free_deviation(deviation);
    }

    if (savedev->devmodule) {
        m__free(savedev->devmodule);
    }

    if (savedev->devrevision) {
        m__free(savedev->devrevision);
    }

    if (savedev->devnamespace) {
        m__free(savedev->devnamespace);
    }

    if (savedev->devprefix) {
        m__free(savedev->devprefix);
    }

    m__free(savedev);

}  /* ncx_free_save_deviations */


/********************************************************************
* FUNCTION ncx_clean_save_deviationsQ
* 
* clean a Q of deviation save structs
*
* INPUT:
*    savedevQ == Q of ncx_save_deviations_t to clean
*********************************************************************/
void
    ncx_clean_save_deviationsQ (dlq_hdr_t *savedevQ)
{
    ncx_save_deviations_t *savedev;

    while (!dlq_empty(savedevQ)) {
        savedev = (ncx_save_deviations_t *)dlq_deque(savedevQ);
        ncx_free_save_deviations(savedev);
    }
} /* ncx_clean_save_deviationsQ */


/********************************************************************
* FUNCTION ncx_set_error
* 
* Set the fields in an ncx_error_t struct
* When called from NACM or <get> internally, there is no
* module or line number info
*
* INPUTS:
*   tkerr== address of ncx_error_t struct to set
*   mod == [sub]module containing tkerr
*   linenum == current linenum
*   linepos == current column position on the current line
*
* OUTPUTS:
*   *tkerr is filled in
*********************************************************************/
void
    ncx_set_error (ncx_error_t *tkerr,
                   ncx_module_t *mod,
                   uint32 linenum,
                   uint32 linepos)
{
#ifdef DEBUG
    if (!tkerr) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }   
#endif

    tkerr->mod = mod;
    tkerr->linenum = linenum;
    tkerr->linepos = linepos;

}  /* ncx_set_error */


/********************************************************************
* FUNCTION ncx_set_temp_modQ
* 
* Set the temp_modQ for yangcli session-specific module list
*
* INPUTS:
*   modQ == new Q pointer to use
*
*********************************************************************/
void
    ncx_set_temp_modQ (dlq_hdr_t *modQ)
{
#ifdef DEBUG
    if (!modQ) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }   
#endif

    temp_modQ = modQ;

}  /* ncx_set_temp_modQ */


/********************************************************************
* FUNCTION ncx_get_temp_modQ
* 
* Get the temp_modQ for yangcli session-specific module list
*
* RETURNS:
*   pointer to the temp modQ, if set
*********************************************************************/
dlq_hdr_t *
    ncx_get_temp_modQ (void)
{
    return temp_modQ;

}  /* ncx_get_temp_modQ */


/********************************************************************
* FUNCTION ncx_clear_temp_modQ
* 
* Clear the temp_modQ for yangcli session-specific module list
*
*********************************************************************/
void
    ncx_clear_temp_modQ (void)
{
    temp_modQ = NULL;

}  /* ncx_clear_temp_modQ */


/********************************************************************
* FUNCTION ncx_get_display_mode
* 
* Get the current default display mode
*
*********************************************************************/
ncx_display_mode_t
    ncx_get_display_mode (void)
{
    return display_mode;

}  /* ncx_get_display_mode */


/********************************************************************
* FUNCTION ncx_get_confirm_event_str
* 
* Get the string for the specified enum value
* 
* INPUT:
*   event == enum confirm event value to convert
*
* RETURNS:
*   string value for the enum
*   NULL if none found
*********************************************************************/
const xmlChar *
    ncx_get_confirm_event_str (ncx_confirm_event_t event)
{
    switch (event) {
    case NCX_CC_EVENT_NONE:
        return NULL;
    case NCX_CC_EVENT_START:
        return NCX_EL_START;
    case NCX_CC_EVENT_CANCEL:
        return NCX_EL_CANCEL;
    case NCX_CC_EVENT_TIMEOUT:
        return NCX_EL_TIMEOUT;
    case NCX_CC_EVENT_EXTEND:
        return NCX_EL_EXTEND;
    case NCX_CC_EVENT_COMPLETE:
        return NCX_EL_COMPLETE;
    default:
        return NULL;
    }

}  /* ncx_get_confirm_event_str */

/********************************************************************
* FUNCTION ncx_copy_c_safe_str
* 
* Copy the string to the buffer, changing legal YANG identifier
* chars that cannot be used in C function names to underscore
*
* !!! DOES NOT CHECK BUFFER OVERRUN !!!
* !!! LENGTH OF STRING IS NOT CHANGED WHEN COPIED TO THE BUFFER !!!
*
* INPUTS:
*   buffer == buffer to write into
*   strval == string value to copy
*
* RETURNS
*   number of chars copied
*********************************************************************/
uint32
    ncx_copy_c_safe_str (xmlChar *buffer,
                         const xmlChar *strval)
{
    const xmlChar *s;
    uint32         count;

    count = 0;
    s = strval;

    while (*s) {
        if (*s == '.' || *s == '-' || *s == NCXMOD_PSCHAR) {
            *buffer++ = '_';
        } else {
            *buffer++ = *s;
        }
        s++;
        count++;
    }
    *buffer = 0;

    return count;

}  /* ncx_copy_c_safe_str */


/* END file ncx.c */
