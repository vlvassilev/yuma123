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
#include <math.h>
#include <ctype.h>
#include <unistd.h>

/* this is not getting included due to -Wextra in make rules */
#ifndef MACOSX
extern float strtof (const char *str, char **err);
#endif

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
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

#ifndef _H_psd
#include "psd.h"
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

#ifndef _H_xml_util
#include "xml_util.h"
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
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static dlq_hdr_t     ncx_modQ;

static dlq_hdr_t     ncx_appQ;

static dlq_hdr_t     ncx_filptrQ;

static uint32        ncx_max_filptrs;

static uint32        ncx_cur_filptrs;

static typ_child_t   operation_attr;

/* 1st stage init */
static boolean       ncx_init_done = FALSE;

/* 2nd stage init */
static boolean       operation_attr_init_done = FALSE;

static boolean       save_descr = FALSE;


/********************************************************************
* FUNCTION gen_modns
*
* Generate a namespace ID that should be unique from
* various fields in the module header
*
* INPUTS:
*   modhdr == pointer to ncx_module_t containing info
*
* RETURNS:
*    malloced string containing the constructed NS URI
*    !!! This MUST be freed by the caller.!!!
*    Returns NULL if out of memory
*********************************************************************/
static xmlChar *
    gen_modns (const ncx_module_t *modhdr)
{
    xmlChar *str;
    uint32   len;
    int      ret;

#ifdef DEBUG
    if (!modhdr) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* Constructed URI format is not allowed to be
     * 'urn' format unless it is IETF approved org-name,
     *  so 'owner' is not allowed unless IETF-approved.
     *
     * This hack is used:
     *
     *    http://<LIB-LOC>/<owner>/<modname>
     *
     * This NS is expecting in the application element
     * under an 'root' type of node
     */
    len = xml_strlen(NCX_URNP1) + 
	xml_strlen(NCX_HOSTNAME) + 1 +
        xml_strlen(modhdr->owner) + 1 +
        xml_strlen(modhdr->name);

    str = m__getMem(len+1);
    if (!str) {
        return NULL;
    }

    ret = sprintf((char *)str, "%s%s/%s/%s",
		  (const char *)NCX_URNP1,
		  (const char *)NCX_HOSTNAME,
		  (const char *)modhdr->owner,
		  (const char *)modhdr->name);
    if (ret == -1) {
        m__free(str);
        return NULL;
    }

    return str;

}   /* gen_modns */


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
*   modname == module to look up or load
*   defname == name of the app-specific definition to find
* OUTPUTS:
*   *dtyp == NCX_NT_PARMSET  (if NO_ERR)
*   *dptr == pointer to data struct or NULL if not found
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_moddef (const xmlChar  *modname, 
		  const xmlChar  *defname,
		  ncx_node_t      *dtyp,
		  void          **dptr)
{
    ncx_module_t  *imod;
    status_t       res, retres;

    retres = NO_ERR;

    /* First find or load the module */
    imod = def_reg_find_module(modname);
    if (!imod) {
        res = ncxmod_load_module(modname);
	CHK_EXIT;

        /* try again to find the module; should not fail */
        imod = def_reg_find_module(modname);
        if (!imod) {
            return ERR_NCX_MOD_NOT_FOUND;
        }
    }

    /* have a module loaded that might contain this def 
     * look in the def_reg for the defname
     * the module may be loaded with non-fatal errors
     */
    *dptr = def_reg_find_moddef(imod->name, defname, dtyp);
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
*   nobrace == TRUE if not looking for a right brace
*              FALSE if right brace should be checked
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_appinfo_entry (tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   dlq_hdr_t     *appinfoQ,
			   boolean nobrace)
{
    ncx_appinfo_t   *appinfo;
    status_t         res, retres;

    /* right brace means appinfo is done */
    if ((tkc->source == TK_SOURCE_NCX) ||
	(tkc->source == TK_SOURCE_YANG && !nobrace)) {
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
    res = yang_consume_pid_string(tkc, mod,
				  &appinfo->prefix,
				  &appinfo->name);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT) {
	    ncx_free_appinfo(appinfo);
	    return retres;
	}
    }

    appinfo->tk = TK_CUR(tkc);

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
	    if (NEED_EXIT) {
		ncx_free_appinfo(appinfo);
		return retres;
	    }
	}
    }

    /* go around and get nested extension statements or semi-colon */
    res = yang_consume_semiapp(tkc, mod, appinfo->appinfoQ);
    if (res != NO_ERR) {
	retres = res;
	if (NEED_EXIT) {
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

    if (tkc->source == TK_SOURCE_NCX) {
	/* check if optional appinfo keyword is present */
	res = ncx_consume_tstring(tkc, mod, NCX_EL_APPINFO, TRUE);
	if (res!=NO_ERR) {
	    /* may be a real error or just skipped */
	    return res;
	}

	/* got the token, so the left brace must be present */
	res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
	if (res != NO_ERR) {
	    return res;
	}
    } else if (tkc->source == TK_SOURCE_YANG && bkup) {
	/* hack: all the YANG fns that call this function
	 * already parsed the MSTRING since extensions
	 * can be spread out throughout the file
	 */
	TK_BKUP(tkc);
    }

    done = FALSE;
    while (!done) {
	res = consume_appinfo_entry(tkc, mod, appinfoQ, bkup);
	if (res != NO_ERR || tkc->source == TK_SOURCE_YANG) {
	    done = TRUE;
	}
    }

    if (res == ERR_NCX_SKIPPED && tkc->source == TK_SOURCE_NCX) {
	res = NO_ERR;
    }

    /* get the closing right brace */
    if (res == NO_ERR && tkc->source == TK_SOURCE_NCX) {
	res = ncx_consume_token(tkc, mod, TK_TT_RBRACE);
    }

    return res;

}  /* consume_appinfo */


/********************************************************************
* FUNCTION add_to_registry
*
* Add all the definitions stored in an ncx_module_t to the registry
* This step is deferred to keep the registry stable as possible
* and only add modules in an all-or-none fashion.
* 
* INPUTS:
*   mod == module to add to registry
*   modname == name of main module being added
*   nsid == namespace ID of the main module being added
*   needapp == address of return appnode status
*
* OUTPUTS:
*  *needapp == TRUE if appnode needed, FALSE otherwise
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    add_to_registry (ncx_module_t *mod,
		     const xmlChar *modname,
		     xmlns_id_t    nsid,
		     boolean *needapp)
{
    typ_template_t *typ;
    grp_template_t *grp;
    obj_template_t *obj;
    psd_template_t *psd;
    rpc_template_t *rpc;
    ext_template_t *ext;
    status_t        res;

    *needapp = FALSE;

    /* add the type definitions to the def_reg hash table */
    for (typ = (typ_template_t *)dlq_firstEntry(&mod->typeQ);
         typ != NULL;
         typ = (typ_template_t *)dlq_nextEntry(typ)) {

	/* register the top-level type */
	typ->nsid = nsid;
	res = def_reg_add_moddef(modname, typ->name, NCX_NT_TYP, typ);
        if (res != NO_ERR) {
	    /* this type registration failed */
	    log_error("\nncx reg: Module '%s' registering "
		      "type '%s' failed (%s)",
		      modname, typ->name, get_error_string(res));
            return res;
        }
    }

    /* add the grouping definitions to the def_reg hash table */
    for (grp = (grp_template_t *)dlq_firstEntry(&mod->groupingQ);
         grp != NULL;
         grp = (grp_template_t *)dlq_nextEntry(grp)) {

	/* register the top-level grouping */
	grp->nsid = nsid;
	res = def_reg_add_moddef(modname,
				 grp->name, NCX_NT_GRP, grp);
        if (res != NO_ERR) {
	    /* this type registration failed */
	    log_error("\nncx reg: Module '%s' registering "
		      "grouping '%s' failed (%s)",
		      modname, grp->name, get_error_string(res));
            return res;
        }
    }

    /* add the top-level object definitions to the def_reg hash table */
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {
	if (obj->objtype == OBJ_TYP_USES ||
	    obj->objtype == OBJ_TYP_AUGMENT) {
	    /* these are not real objects, and do not have names */
	    continue;
	}

	obj->nsid = nsid;
	res = def_reg_add_moddef(modname,
				 obj_get_name(obj), NCX_NT_OBJ, obj);
        if (res != NO_ERR) {
	    /* this object registration failed */
	    log_error("\nncx reg: Module '%s' registering "
		      "object '%s' failed (%s)",
		      modname, obj_get_name(obj),
		      get_error_string(res));
            return res;
        }
    }

    /* add the PSD definitions to the def_reg hash table */
    for (psd = (psd_template_t *)dlq_firstEntry(&mod->psdQ);
         psd != NULL;
         psd = (psd_template_t *)dlq_nextEntry(psd)) {
	if (psd->psd_type == PSD_TYP_DATA) {
	    *needapp = TRUE;
	}
	if (!psd->nsid) {
	    psd->nsid = nsid;
	}
	res = def_reg_add_moddef(modname,
				 psd->name, NCX_NT_PSD, psd);
        if (res != NO_ERR) {
	    /* this PSD registration failed */
	    log_error("\nncx reg: Module '%s' registering "
		      "parmset '%s' failed (%s)",
		      modname, psd->name,
		      get_error_string(res));
            return res;
        }
    }

    /* add the RPC definitions to the def_reg hash table */
    for (rpc = (rpc_template_t *)dlq_firstEntry(&mod->rpcQ);
         rpc != NULL;
         rpc = (rpc_template_t *)dlq_nextEntry(rpc)) {
	rpc->nsid = nsid;
	res = def_reg_add_moddef(modname,
				 rpc->name, NCX_NT_RPC, rpc);
        if (res != NO_ERR) {
	    /* this RPC registration failed */
	    log_error("\nncx reg: Module '%s' registering "
		      "RPC method '%s' failed (%s)",
		      modname, rpc->name,
		      get_error_string(res));
            return res;
        }
    }

    /* jsut set the extension namespace ID */
    for (ext = (ext_template_t *)dlq_firstEntry(&mod->extensionQ);
         ext != NULL;
         ext = (ext_template_t *)dlq_nextEntry(ext)) {
	ext->nsid = nsid;
    }

    return NO_ERR;

}  /* add_to_registry */


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
*    removereg == TRUE if should if all defs be removed 
*                 from registry, FALSE if not
*********************************************************************/
static void 
    free_module (ncx_module_t *mod,
		 boolean removereg)
{
    ncx_revhist_t  *revhist;
    ncx_import_t   *import;
    typ_template_t *typ;
    grp_template_t *grp;
    psd_template_t *psd;
    rpc_template_t *rpc;
    obj_template_t *obj;

#ifdef DEBUG
    if (!mod) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* unregister the module */
    if (mod->ismod && removereg) {
	def_reg_del_module(mod->name);
    }

    /* clear the import Que */
    while (!dlq_empty(&mod->importQ)) {
	import = (ncx_import_t *)dlq_deque(&mod->importQ);
        ncx_free_import(import);
    }

    /* clear the type Que */
    while (!dlq_empty(&mod->typeQ)) {
	typ = (typ_template_t *)dlq_deque(&mod->typeQ);
	if (typ->name && removereg) {
	    def_reg_del_moddef(mod->name, typ->name, NCX_NT_TYP);
	}
	typ_free_template(typ);
    }

    /* clear the PSD Que */
    while (!dlq_empty(&mod->psdQ)) {
	psd = (psd_template_t *)dlq_deque(&mod->psdQ);
	if (removereg) {
	    def_reg_del_moddef(mod->name, psd->name, NCX_NT_PSD);
	}
	psd_free_template(psd);
    }

    /* clear the RPC Que */
    while (!dlq_empty(&mod->rpcQ)) {
	rpc = (rpc_template_t *)dlq_deque(&mod->rpcQ);
	if (removereg) {
	    def_reg_del_moddef(mod->name, rpc->name, NCX_NT_RPC);
	}
	rpc_free_template(rpc);
    }

    /* clear the grouping Que */
    while (!dlq_empty(&mod->groupingQ)) {
	grp = (grp_template_t *)dlq_deque(&mod->groupingQ);
	if (grp->name && removereg) {
	    def_reg_del_moddef(mod->name, grp->name, NCX_NT_GRP);
	}
	grp_free_template(grp);
    }

    /* clear the datadefQ */
    while (!dlq_empty(&mod->datadefQ)) {
	obj = (obj_template_t *)dlq_deque(&mod->datadefQ);
	if (obj_has_name(obj) && removereg) {
	    def_reg_del_moddef(mod->name, obj_get_name(obj), NCX_NT_OBJ);
	}
	obj_free_template(obj);
    }

    /* clear the extension Que */
    ext_clean_extensionQ(&mod->extensionQ);

    /* clear the header last */
    while (!dlq_empty(&mod->revhistQ)) {
	revhist = (ncx_revhist_t *)dlq_deque(&mod->revhistQ);
        ncx_free_revhist(revhist);
    }

    ncx_clean_appinfoQ(&mod->appinfoQ);

    ncx_clean_typnameQ(&mod->typnameQ);

    yang_clean_import_ptrQ(&mod->saveimpQ);

    yang_clean_nodeQ(&mod->saveincQ);

    if (mod->name) {
	m__free(mod->name);
    }
    if (mod->version) {
	m__free(mod->version);
    }
    if (mod->owner) {
	m__free(mod->owner);
    }
    if (mod->organization) {
	m__free(mod->organization);
    }
    if (mod->app) {
	m__free(mod->app);
    }
    if (mod->copyright) {
	m__free(mod->copyright);
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
    if (mod->last_update) {
	m__free(mod->last_update);
    }
    if (mod->ismod) {
	if (mod->ns) {
	    m__free(mod->ns);
	}
	if (mod->prefix) {
	    m__free(mod->prefix);
	}
    }
    if (mod->source) {
	m__free(mod->source);
    }
    if (mod->belongs) {
	m__free(mod->belongs);
    }

    m__free(mod);

}  /* free_module */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION ncx_init
* 
* Initialize the NCX module
*
* INPUTS:
*   savestr == TRUE if parsed description strings that are
*              not needed by the agent at runtime should
*              be saved anyway.  Converters should use this value.
*                 
*           == FALSE if uneeded strings should not be saved.
*              Embedded agents should use this value
*
*    dlevel == desired debug output level
*
*  startmsg == log_debug2 message to print before starting;
*              NULL if not used;
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    ncx_init (boolean savestr,
	      log_debug_t dlevel,
	      const char *startmsg)
{
    status_t     res;
    xmlns_id_t   nsid;

    if (ncx_init_done) {
        return NO_ERR;
    }

    save_descr = savestr;
    log_set_debug_level(dlevel);

    if (startmsg) {
	log_debug2(startmsg);
    }

    /* create the module and appnode queues */
    dlq_createSQue(&ncx_modQ);
    dlq_createSQue(&ncx_appQ);
    dlq_createSQue(&ncx_filptrQ);
    ncx_max_filptrs = NCX_DEF_FILPTR_CACHESIZE;
    ncx_cur_filptrs = 0;

    /* check that the correct version of libxml2 is installed */
    LIBXML_TEST_VERSION;

    /* init nmodule handler */
    ncxmod_init();

    /* init runstack script support */
    runstack_init();

    /* init top level msg dispatcher */
    top_init();

    /* initialize the definition resistry */
    def_reg_init();

    /* initialize the namespace registry */
    xmlns_init();

    /* Initialize the INVALID namespace to help filter handling */
    res = xmlns_register_ns(INVALID_URN, INV_PREFIX, NCX_MODULE, &nsid);
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the XML namespace for NETCONF */
    res = xmlns_register_ns(NC_URN, NC_PREFIX, NC_MODULE, &nsid);
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the NCX namespace for NCX specific extensions */
    res = xmlns_register_ns(NCX_URN, NCX_PREFIX, NCX_MODULE, &nsid);
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the XMLNS namespace for xmlns attributes */
    res = xmlns_register_ns(NS_URN, NS_PREFIX, NCX_MODULE, &nsid);
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the XSD namespace for ncxdump program */
    res = xmlns_register_ns(XSD_URN, XSD_PREFIX, NCX_MODULE, &nsid);
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the XSI namespace for ncxdump program */
    res = xmlns_register_ns(XSI_URN, XSI_PREFIX, NCX_MODULE, &nsid);
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the Notifications namespace for ncxdump program */
    res = xmlns_register_ns(NCN_URN, NCN_PREFIX, NCX_MODULE, &nsid);
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

    ncx_init_done = TRUE;

    return NO_ERR;

}  /* ncx_init */


/********************************************************************
* FUNCTION ncx_cleanup
*
*  cleanup NCX module
*********************************************************************/
void
    ncx_cleanup (void)
{
    ncx_module_t   *mod;
    ncx_appnode_t  *app;
    ncx_filptr_t   *filptr;

    if (!ncx_init_done) {
	return;
    }

    while (!dlq_empty(&ncx_modQ)) {
	mod = (ncx_module_t *)dlq_deque(&ncx_modQ);
	free_module(mod, TRUE);
    }

    while (!dlq_empty(&ncx_appQ)) {
	app = (ncx_appnode_t *)dlq_deque(&ncx_appQ);
	ncx_free_appnode(app);
    }

    while (!dlq_empty(&ncx_filptrQ)) {
	filptr = (ncx_filptr_t *)dlq_deque(&ncx_filptrQ);
	m__free(filptr);
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
    dlq_createSQue(&mod->psdQ);
    dlq_createSQue(&mod->rpcQ);
    dlq_createSQue(&mod->groupingQ);
    dlq_createSQue(&mod->datadefQ);
    dlq_createSQue(&mod->extensionQ);
    dlq_createSQue(&mod->appinfoQ);
    dlq_createSQue(&mod->typnameQ);
    dlq_createSQue(&mod->saveimpQ);
    dlq_createSQue(&mod->saveincQ);
    return mod;

}  /* ncx_new_module */


/********************************************************************
* FUNCTION ncx_find_module
*
* Find a ncx_module_t in the ncx_modQ
* These are the modules that are already loaded
* This search is done instead of the def_reg directly
* to force the selection specified by the <import>
* instead of what might be loaded into the registry
*
* INPUTS:
*   modname == module name
* RETURNS:
*  module pointer if found or NULL if not
*********************************************************************/
ncx_module_t *
    ncx_find_module (const xmlChar *modname)
{
    ncx_module_t  *mod;

#ifdef DEBUG
    if (!modname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;    /* error */
    }
#endif

    for (mod = (ncx_module_t *)dlq_firstEntry(&ncx_modQ);
         mod != NULL;
         mod = (ncx_module_t *)dlq_nextEntry(mod)) {
        if (!xml_strcmp(mod->name, modname)) {
            return mod;
        }
    }
    return NULL;

}   /* ncx_find_module */


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
    free_module(mod, FALSE);

}  /* ncx_free_module */


/********************************************************************
* FUNCTION ncx_remove_module
* 
* Scrub the memory in a ncx_module_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* Also remove all module definitions from the registry
*
* INPUTS:
*    modname == module name of the NCX module to remove
*********************************************************************/
void 
    ncx_remove_module (const xmlChar *modname)
{
    ncx_module_t  *mod;

#ifdef DEBUG
    if (!modname) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* if the module isn't in the hash table,
     * then it was never registered
     */
    mod = def_reg_find_module(modname);
    if (mod) {
        dlq_remove(mod);
        free_module(mod, TRUE);
    }

}  /* ncx_remove_module */


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

    for (mod = (ncx_module_t *)dlq_firstEntry(&ncx_modQ);
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
    const ncx_import_t       *imp;
    const yang_import_ptr_t  *impptr;
    const dlq_hdr_t          *impQ;

    impQ = (mod->allimpQ) ? mod->allimpQ : &mod->saveimpQ;

    if (mod->isyang) {
	/* Only YANG modules supported at this time */
	for (impptr = (yang_import_ptr_t *)dlq_firstEntry(impQ);
	     impptr != NULL;
	     impptr = (yang_import_ptr_t *)dlq_nextEntry(impptr)) {

	    testmod = ncx_find_module(impptr->modname);
	    if (!testmod) {
		/* missing import */
		return TRUE;
	    }
	    
	    if (testmod->status != NO_ERR) {
		return TRUE;
	    }
	}
    } else {
	/* NCX module -- not really supported , not checking nested imports */
	for (imp = (ncx_import_t *)dlq_firstEntry(&mod->importQ);
	     imp != NULL;
	     imp = (ncx_import_t *)dlq_nextEntry(imp)) {

	    testmod = ncx_find_module(imp->module);
	    if (!testmod) {
		/* this must be an unused import or the NCX parse would have failed */
		continue;
	    }
	    
	    if (testmod->status != NO_ERR) {
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
	    node = yang_find_node(que, inc->submodule);
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
	    node = yang_find_node(que, inc->submodule);
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
* FUNCTION ncx_find_psd
*
* Check if a psd_template_t in the mod->psdQ
*
* INPUTS:
*   mod == ncx_module to check
*   psdname == PSD name
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
psd_template_t *
    ncx_find_psd (const ncx_module_t *mod,
		  const xmlChar *psdname)
{
    psd_template_t *psd;

#ifdef DEBUG
    if (!mod || !psdname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (psd = (psd_template_t *)dlq_firstEntry(&mod->psdQ);
         psd != NULL;
         psd = (psd_template_t *)dlq_nextEntry(psd)) {
        if (!xml_strcmp(psd->name, psdname)) {
            return psd;
        }
    }
    return NULL;

}   /* ncx_find_psd */


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
rpc_template_t *
    ncx_find_rpc (const ncx_module_t *mod,
		  const xmlChar *rpcname)
{
    rpc_template_t *rpc;

#ifdef DEBUG
    if (!mod || !rpcname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (rpc = (rpc_template_t *)dlq_firstEntry(&mod->rpcQ);
         rpc != NULL;
         rpc = (rpc_template_t *)dlq_nextEntry(rpc)) {
        if (!xml_strcmp(rpc->name, rpcname)) {
            return rpc;
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
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
rpc_template_t *
    ncx_match_rpc (const ncx_module_t *mod,
		   const xmlChar *rpcname)
{
    rpc_template_t *rpc;
    uint32          len;

#ifdef DEBUG
    if (!mod || !rpcname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    len = xml_strlen(rpcname);
    for (rpc = (rpc_template_t *)dlq_firstEntry(&mod->rpcQ);
         rpc != NULL;
         rpc = (rpc_template_t *)dlq_nextEntry(rpc)) {
        if (!xml_strncmp(rpc->name, rpcname, len)) {
            return rpc;
        }
    }
    return NULL;

}   /* ncx_match_rpc */


/********************************************************************
* FUNCTION ncx_match_any_rpc
*
* Check if a rpc_template_t in in any module that
* matches the rpc name string and maybe the owner
*
* INPUTS:
*   owner == owner name to check (NULL == check all)
*   rpcname == RPC name to match
* RETURNS:
*  pointer to struct if present, NULL otherwise
*********************************************************************/
rpc_template_t *
    ncx_match_any_rpc (const xmlChar *owner,
		       const xmlChar *rpcname)
{
    rpc_template_t *rpc;
    ncx_module_t   *mod;

#ifdef DEBUG
    if (!rpcname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (mod = ncx_get_first_module();
         mod != NULL;
         mod =  ncx_get_next_module(mod)) {

	/* check if only one owner is requested */
	if (owner && xml_strcmp(owner, mod->owner)) {
	    continue;  /* not the requested owner */
	}

	rpc = ncx_match_rpc(mod, rpcname);
	if (rpc) {
	    return rpc;
        }
    }
    return NULL;

}   /* ncx_match_any_rpc */


/********************************************************************
* FUNCTION ncx_add_to_registry
*
* Add all the definitions stored in an ncx_module_t to the registry
* This step is deferred to keep the registry stable as possible
* and only add modules in an all-or-none fashion.
* 
* HACK:
*    Also used to set the namespace ID of duplicate modules
*    for the yangdump application.  In subtree mode, multiple
*    parser control blocks are used and destroyed, but the
*    def_reg is not reset
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
    xmlns_t        *ns;
    ncx_appnode_t  *app;
    boolean         needapp, needns;
    status_t        res;

#ifdef DEBUG
    if (!mod) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check module parse code */
    if (mod->status != NO_ERR) {
	res = mod->status;
	if (NEED_EXIT) {
	    /* should not happen */
	    log_error("\nError: cannot add module '%s' to registry"
		      " with fatal errors", mod->name);
	    return SET_ERROR(ERR_INTERNAL_VAL);
	} else {
	    log_debug2("\nAdding module '%s' to registry"
		       " with errors", mod->name);
	    res = NO_ERR;
	}
    }

    /* setup local vars */
    needapp = FALSE;
    needns = mod->ismod;

    /* if this is the XSD module, then use the NS ID already registered */
    if (!xml_strcmp(mod->name, NCX_EL_XSD)) {
	mod->nsid = xmlns_xs_id();
	needns = FALSE;
    }

    /* first add the application namespace
     * Multiple NS URIs are allowed to map to the same app 
     */
    if (needns) {
	if (!mod->ns) {
	    /* construct a namespace value from the hdr info */
	    mod->ns = gen_modns(mod);
	    if (!mod->ns) {
		return ERR_INTERNAL_MEM;
	    }
	}
    
	ns = def_reg_find_ns(mod->ns);
	if (ns) {
	    if (xml_strcmp(mod->name, ns->ns_module) &&
		xml_strcmp(ns->ns_module, NCX_OWNER)) {
		/* this NS string already registered to another module */
		log_error("\nncx reg: Module '%s' registering "
			  "duplicate namespace '%s'\n    "
			  "registered by module '%s'",
			  mod->name, mod->ns, ns->ns_module);
		return ERR_DUP_NS;
	    } else {
		/* same owner so okay */
		mod->nsid = ns->ns_id;
	    }
	} else {
	    res = xmlns_register_ns(mod->ns, mod->prefix, 
				    mod->name, &mod->nsid);
	    if (res != NO_ERR) {
		/* this NS registration failed */
		log_error("\nncx reg: Module '%s' registering "
			  "namespace '%s' failed (%s)",
			  mod->name, mod->ns,
			  get_error_string(res));
		return res;
	    }
	}
    }

    /* add the main module definitions */
    res = add_to_registry(mod, mod->name, mod->nsid, &needapp);
    if (res != NO_ERR) {
	return res;
    }

    /* add all the submodules included in this module */
    for (node = (yang_node_t *)dlq_firstEntry(&mod->saveincQ);
	 node != NULL;
	 node = (yang_node_t *)dlq_nextEntry(node)) {

	node->submod->nsid = mod->nsid;
	res = add_to_registry(node->submod, mod->name, 
			      mod->nsid, &needapp);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* add the application node only if there are data parmsets
     * actually defined in this module
     * Add a pointer to the module appnode to the registry
     */
    if (needapp) {
	app = ncx_find_appnode(mod->name, mod->app);
	if (!app) {
	    /* create a new appnode, register it, ans save it
	     * in the ncx_appQ
	     */
	    app = ncx_new_appnode(mod->name, mod->app, mod->nsid);
	    if (!app) {
		res = ERR_INTERNAL_MEM;
	    } else {
		res = def_reg_add_cfgapp(mod->name, app->appname, 
					 NCX_CFGID_RUNNING, app);
	    }
	    if (res != NO_ERR) {
		/* this APP node registration failed */
		log_error("\nncx reg: Module '%s' registering "
			  "APP node '%s' failed (%s)",
			  mod->name, mod->app,
			  get_error_string(res));
		return res;
	    } else {
		dlq_enque(app, &ncx_appQ);
	    }
	}
    }

    /* add the module itself for fast lookup in imports
     * of other modules
     */
    if (mod->ismod) {
	res = def_reg_add_module(mod);
	if (res != NO_ERR) {
	    /* this module registration failed */
	    log_error("\nncx reg: Module '%s' registration failed (%s)",
		      mod->name, get_error_string(res));
	    return res;
	}

	/* save the module in the module Q */
	dlq_enque(mod, &ncx_modQ);
    }

    return NO_ERR;

}  /* ncx_add_to_registry */


/********************************************************************
* FUNCTION ncx_is_duplicate
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
    if (ncx_find_psd(mod, defname)) {
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
*   pointer to the first entry of NULL if empty Q
*********************************************************************/
ncx_module_t *
    ncx_get_first_module (void)
{
    return (ncx_module_t *)dlq_firstEntry(&ncx_modQ);

}  /* ncx_get_first_module */


/********************************************************************
* FUNCTION ncx_get_next_module
* 
* Get the next module in the ncx_modQ
* 
* RETURNS:
*   pointer to the first entry of NULL if empty Q
*********************************************************************/
ncx_module_t *
    ncx_get_next_module (const ncx_module_t *mod)
{
    return (mod) ? 
	(ncx_module_t *)dlq_nextEntry(mod) : NULL;

}  /* ncx_get_next_module */


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
    dlq_createSQue(&import->itemQ);
    dlq_createSQue(&import->appinfoQ);
    return import;

}  /* ncx_new_import */


/********************************************************************
* FUNCTION ncx_new_import_item
* 
* Malloc and initialize the fields in a ncx_import_item_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
ncx_import_item_t * 
    ncx_new_import_item (void)
{
    ncx_import_item_t  *item;

    item = m__getObj(ncx_import_item_t);
    if (!item) {
	return NULL;
    }
    (void)memset(item, 0x0, sizeof(ncx_import_item_t));
    return item;

}  /* ncx_new_import_item */


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
    ncx_import_item_t  *item;

#ifdef DEBUG
    if (!import) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (import->module) {
	m__free(import->module);
    }

    /* YANG imports only */
    if (import->prefix) {
	m__free(import->prefix);
    }

    /* NCX imports only */
    while (!dlq_empty(&import->itemQ)) {
	item = (ncx_import_item_t *)dlq_deque(&import->itemQ);
	ncx_free_import_item(item);
    }

    /* YANG only */
    ncx_clean_appinfoQ(&import->appinfoQ);

    m__free(import);

}  /* ncx_free_import */


/********************************************************************
* FUNCTION ncx_free_import_item
* 
* Scrub the memory in a ncx_import_item_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    item == ncx_import_item_t data structure to free
*********************************************************************/
void 
    ncx_free_import_item (ncx_import_item_t *item)
{
#ifdef DEBUG
    if (!item) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (item->name) {
	m__free(item->name);
    }
    m__free(item);

}  /* ncx_free_import_item */


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
	    import->used = TRUE;
	    return import;
	}
    }
    return NULL;

} /* ncx_find_import */


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
	    import->used = TRUE;
	    return import;
	}
    }
    return NULL;

} /* ncx_find_pre_import */


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


#if 0
/********************************************************************
* FUNCTION ncx_find_pre_import_saveQ
* 
* Search the specified importQ for a specified prefix value
* Test only, do not set used flag
*
* INPUTS:
*   que == Q of ncx_import_t to search
*   prefix == prefix string to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ncx_import_t * 
    ncx_find_pre_import_saveQ (const ncx_module_t *mod,
			       const xmlChar *prefix)
{
    ncx_import_t  *import;
    yang_import_ptr_t  *import;

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
#endif


/********************************************************************
* FUNCTION ncx_locate_import
* 
* Search the module import path for the specified definition name.
*
* NCX only!!!
* Do not use in YANG!!!
*
* INPUTS:
*     mod == ncx_module_t for the construct using this def name
*     defname == name of definition to find
*     *deftyp == specified type or NCX_NT_NONE if any will do
*
* OUTPUTS:
*    *deftyp == type retrieved if NO_ERR
* RETURNS:
*    pointer to the located definition or NULL if not found
*********************************************************************/
void *
    ncx_locate_import (const ncx_module_t  *mod,
		       const xmlChar *defname,
		       ncx_node_t     *deftyp)
{
    ncx_import_t      *imp;
    ncx_import_item_t *item;
    void              *dptr;
    status_t           res;

#ifdef DEBUG
    if (!mod || !defname || !deftyp) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* Go through the imports list for any match in an items list */
    for (imp = (ncx_import_t *)dlq_firstEntry(&mod->importQ);
         imp != NULL;
         imp = (ncx_import_t *)dlq_nextEntry(imp)) {
        /* check the item list for this import */
        for (item = (ncx_import_item_t *)dlq_firstEntry(&imp->itemQ);
             item != NULL;
             item = (ncx_import_item_t *)dlq_nextEntry(item)) {
            if (!xml_strcmp(item->name, defname)) {
		dptr = NULL;
                res = check_moddef(imp->module, defname, deftyp, &dptr);
                if (res==NO_ERR) {
                    return dptr;
                } else if (res != ERR_NCX_DEF_NOT_FOUND) {
		    return NULL;  /*  !! res is lost !! */
		}
            }
        }
    }
        
    /* Definition still not found. 
     * Now try any module in the imports list that does not
     * have an items list.  
     * Fish in the registry for the definition 
     */
    for (imp = (ncx_import_t *)dlq_firstEntry(&mod->importQ);
         imp != NULL;
         imp = (ncx_import_t *)dlq_nextEntry(imp)) {
        /* check only if there is no item list this time */
        if (dlq_empty(&imp->itemQ)) {
            res = check_moddef(imp->module, defname, deftyp, &dptr);
            if (res == NO_ERR) {
                return dptr;
	    } else if (res != ERR_NCX_DEF_NOT_FOUND) {
		return NULL;  /*  !! res is lost !! */
	    }
        }
    }

    /* Either not a definition, definition is a forward reference,
     * or the import for the definition is missing
     */
    return NULL;

}  /* ncx_locate_import */


/********************************************************************
* FUNCTION ncx_locate_modqual_import
* 
* Search the specific module for the specified definition name.
*
* Okay for YANG or NCX
*
* YANG && NCX:
*   - typ_template_t (NCX_NT_TYP)
*
* YANG:
*  - grp_template_t (NCX_NT_GRP)
*  - obj_template_t (NCX_NT_OBJ)
*
* NCX:
*  - psd_template_t  (NCX_NT_PSD)
*  - rpc_template_t  (NCX_NT_RPC)
*
* INPUTS:
*     modstr == module name to check
*     defname == name of definition to find
*     *deftyp == specified type or NCX_NT_NONE if any will do
* OUTPUTS:
*    *deftyp == type retrieved if NO_ERR
* RETURNS:
*    pointer to the located definition or NULL if not found
*********************************************************************/
void *
    ncx_locate_modqual_import (const xmlChar *modstr,
			       const xmlChar *defname,
			       ncx_node_t     *deftyp)
{
    void *dptr;
    status_t  res;

#ifdef DEBUG
    if (!modstr || !defname || !deftyp) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    res = check_moddef(modstr, defname, deftyp, &dptr);
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
* FUNCTION ncx_free_inlude
* 
* Scrub the memory in a ncx_iclude_t by freeing all
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
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
	num->f = 0.0;
#else
	if (num->f) {
	    m__free(num->f);
	    num->f = NULL;
	}
#endif   /* HAS_FLOAT */
	break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
	num->d = 0.0;
#else
	if (num->d) {
	    m__free(num->d);
	    num->d = NULL;
	}
#endif   /* HAS_FLOAT */
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
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
        if (num1->f < num2->f) {
            return -1;
        } else if (num1->f == num2->f) {
            return 0;
        } else {
            return 1;
        }
#else
        /**** TBD: REAL NUMBER IS NOT NORMALIZED YET *****/
        if (num1->f && num2->f) {
            return xml_strcmp(num1->f, num2->f);
        } else if (num1->f) {
            return -1;
        } else {
            return 1;
        }
#endif
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        if (num1->d < num2->d) {
            return -1;
        } else if (num1->d == num2->d) {
            return 0;
        } else {
            return 1;
        }
#else
        /**** TBD: REAL NUMBER IS NOT NORMALIZED YET *****/
        if (num1->d && num2->d) {
            return xml_strcmp(num1->d, num2->d);
        } else if (num1->d) {
            return -1;
        } else {
            return 1;
        }
#endif
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
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
	num->f = -1;  /* INFINITY */
#else
	if (num->f) {
	    m__free(num->f);
	}
	num->f = xml_strdup((const xmlChar *) NCX_MIN_FLOAT);
	if (!num->f) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	}
#endif
	break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
	num->d = -1;  /* INFINITY */
#else
	if (num->d) {
	    m__free(num->d);
	}
	num->d = xml_strdup((const xmlChar *) NCX_MIN_DOUBLE);
	if (!num->d) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	}
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
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
	num->f = 1; /* INFINITY */
#else
	if (num->f) {
	    m__free(num->f);
	}
	num->f = xml_strdup((const xmlChar *) NCX_MAX_FLOAT);
	if (!num->f) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	}
#endif
	break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
	num->d = 1;   /* INFINITY */
#else
	if (num->d) {
	    m__free(num->d);
	}
	num->d = xml_strdup((const xmlChar *) NCX_MAX_DOUBLE);
	if (!num->d) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	}
#endif
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* ncx_set_num_max */


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
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
	num->f = 0;
#else
	if (num->f) {
	    m__free(num->f);
	}
	num->f = xml_strdup((const xmlChar *)"0");
	if (!num->f) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	}
#endif
	break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
	num->d = 0;
#else
	if (num->d) {
	    m__free(num->d);
	}
	num->d = xml_strdup((const xmlChar *)"0");
	if (!num->d) {
	    SET_ERROR(ERR_INTERNAL_MEM);
	}
#endif
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

} /* ncx_set_num_zero */


/********************************************************************
* FUNCTION ncx_num_zero
* 
* Compare a ncx_num_t to zero
*
* INPUTS:
*     num == number to check
*     btyp == expected data type (NCX_BT_INT, UINT, FLOAT, DOUBLE)
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
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
        return (num->f) ? FALSE : TRUE;
#else
        /**** TBD: REAL NUMBER IS NOT NORMALIZED YET *****/
        if (num->f) {
            return (xml_strcmp(num->f, NCX_EL_ZERO)) ? FALSE : TRUE;
	} else {
	    return FALSE;
	}
#endif
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        return (num->d) ? FALSE : TRUE;
#else
        /**** TBD: REAL NUMBER IS NOT NORMALIZED YET *****/
        if (num->d) {
            return (xml_strcmp(num->d, NCX_EL_ZERO)) ? FALSE : TRUE;
        } else {
            return FALSE;
        }
#endif
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return FALSE;
    }
    /*NOTREACHED*/
}  /* ncx_num_zero */


/********************************************************************
* FUNCTION ncx_convert_num
* 
* Convert a number string to an int32, uint32, or float
*
* INPUTS:
*     numstr == number string
*     numfmt == NCX_NF_DEC, NCX_NF_HEX, or NCX_NF_REAL
*     btyp == desired number type 
*             (e.g., NCX_BT_INT, NCX_BT_UINT, NCX_BT_FLOAT)
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
    float f;
    double d;
#else
    xmlChar *str;
#endif

#ifdef DEBUG
    if (!numstr || !val) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!*numstr) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* check the number format set to don't know */
    if (numfmt==NCX_NF_NONE) {
	numfmt = ncx_get_numfmt(numstr);
    }

    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
        switch (numfmt) {
        case NCX_NF_DEC:
            l = strtol((const char *)numstr, &err, 10);
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
        case NCX_NF_HEX:
            l = strtol((const char *)numstr, &err, 16);
            if (err && *err) {
                return ERR_NCX_INVALID_HEXNUM;
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
        case NCX_NF_REAL:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case NCX_BT_INT64:
        switch (numfmt) {
        case NCX_NF_DEC:
            ll = strtoll((const char *)numstr, &err, 10);
            if (err && *err) {
                return ERR_NCX_INVALID_NUM;
            }
            val->l = (int64)ll;
            break;
        case NCX_NF_HEX:
            ll = strtoll((const char *)numstr, &err, 16);
            if (err && *err) {
                return ERR_NCX_INVALID_HEXNUM;
            }
            val->l = (int64)ll;
            break;
        case NCX_NF_REAL:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
        switch (numfmt) {
        case NCX_NF_DEC:
            ul = strtoul((const char *)numstr, &err, 10);
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

            val->u = (uint32)ul;
            break;
        case NCX_NF_HEX:
            ul = strtoul((const char *)numstr, &err, 16);
            if (err && *err) {
                return ERR_NCX_INVALID_HEXNUM;
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

            val->u = (uint32)ul;
            break;
        case TK_TT_RNUM:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case NCX_BT_UINT64:
        switch (numfmt) {
        case NCX_NF_DEC:
            ull = strtoull((const char *)numstr, &err, 10);
            if (err && *err) {
                return ERR_NCX_INVALID_NUM;
            }
            val->ul = (uint64)ull;
            break;
        case NCX_NF_HEX:
            ull = strtoull((const char *)numstr, &err, 16);
            if (err && *err) {
                return ERR_NCX_INVALID_HEXNUM;
            }
            val->ul = (uint64)ull;
            break;
        case NCX_NF_REAL:
            return ERR_NCX_WRONG_TKTYPE;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
        switch (numfmt) {
        case NCX_NF_DEC:
        case NCX_NF_REAL:
            f = strtof((const char *)numstr, &err);
            if (err && *err) {
                return ERR_NCX_INVALID_NUM;
            }
            val->f = f;
            break;
        case NCX_NF_HEX:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
#else
        switch (numfmt) {
        case NCX_NF_DEC:
        case NCX_NF_REAL:
	    if (val->f) {
		m__free(val->f);
	    }
            val->f = xml_strdup(numstr);
            if (!val->f) {
                return ERR_INTERNAL_MEM;
            }
            break;
        case NCX_NF_HEX:
            return ERR_NCX_WRONG_NUMTYP;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
#endif
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
        switch (numfmt) {
        case NCX_NF_DEC:
        case NCX_NF_REAL:
            d = strtod((const char *)numstr, &err);
            if (err && *err) {
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
        case NCX_NF_DEC:
        case NCX_NF_REAL:
	    if (val->d) {
		m__free(val->d);
	    }
            val->d = xml_strdup(numstr);
            if (!val->d) {
                return ERR_INTERNAL_MEM;
            }
            break;
        case NCX_NF_HEX:
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
	return ncx_convert_num(numstr+2, NCX_NF_HEX, btyp, retnum);
    }

    /* check if this is a real number */
    str = numstr;
    while (*str && (*str != '.')) {
        str++;
    }
    if (*str) {
	return ncx_convert_num(numstr, NCX_NF_REAL, btyp, retnum);
    }

    /* else assume this is a decimal number */
    return ncx_convert_num(numstr, NCX_NF_DEC, btyp, retnum);

}  /* ncx_decode_num */


/********************************************************************
* FUNCTION ncx_copy_num
* 
* Copy the contents of num1 to num2
*
* Supports all NCX numeric types:
*    NCX_BT_INT*
*    NCX_BT_UINT*
*    NCX_BT_FLOAT*
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
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT        
        num2->f = num1->f;
#else
        if (str1->f) {
	    if (num2->f) {
		m__free(num2->f);
	    }
            num2->f = xml_strdup(str1->f);
            if (!num2->f) {
                return ERR_INTERNAL_MEM;
            }
        } else {
            return SET_ERROR(ERR_INTERNAL_PTR);
        }            
#endif
        break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT        
        num2->d = num1->d;
#else
        if (str1->d) {
	    if (num2->d) {
		m__free(num2->d);
	    }
            num2->d = xml_strdup(str1->d);
            if (!num2->d) {
                return ERR_INTERNAL_MEM;
            }
        } else {
            return SET_ERROR(ERR_INTERNAL_PTR);
        }
#endif
        break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    return NO_ERR;
}  /* ncx_copy_num */


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
    if (!numstr || !*numstr) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NCX_NF_NONE;
    }
#endif

    /* check for a HEX string first */
    if (*numstr=='0' && (numstr[1]=='x' || numstr[1]=='X')) {
	return NCX_NF_HEX;
    }

    while (*numstr && (*numstr != '.')) {
	numstr++;
    }
    return (*numstr) ? NCX_NF_REAL : NCX_NF_DEC;

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
#ifdef DEBUG
    if (!num) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    switch (btyp) {
    case NCX_BT_NONE:
	log_write("--");
	break;
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
	log_write("%d", num->i);
	break;
    case NCX_BT_INT64:
	log_write("%lld", num->l);
	break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
	log_write("%u", num->u);
	break;
    case NCX_BT_UINT64:
	log_write("%llu", num->ul);
	break;
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
	log_write("%f", num->f);
#else
	log_write("%s", (num->f) ? num->f : "--");
#endif
	break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
	log_write("%lf", num->d);
#else
	log_write("%s", (num->d) ? num->d : "--");
#endif
	break;
    default:
	log_write("--");
    }

} /* ncx_printf_num */


/********************************************************************
* FUNCTION ncx_sprintf_num
* 
* Sprintf a ncx_num_t contents
*
* INPUTS:
*    buff == buffer to write; NULL means just get length
*    num == number to printf
*    btyp == number base type
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
    int32  ilen;
    xmlChar dumbuff[VAL_MAX_NUMLEN];

#ifdef DEBUG
    if (!num || !len) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (!buff) {
	buff = dumbuff;
    }

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
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
	ilen = sprintf((char *)buff, "%f", num->f);
#else
	ilen = sprintf((char *)buff, "%s", (num->f) ? num->f : "");
#endif
	break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
	ilen = sprintf((char *)buff, "%lf", num->d);
#else
	ilen = sprintf((char *)buff, "%s", (num->d) ? num->d : "");
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
    case NCX_BT_FLOAT32:
    case NCX_BT_FLOAT64:
	/* there is no actual min value, just MIN and -INF */
	return FALSE;
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
    case NCX_BT_FLOAT32:
    case NCX_BT_FLOAT64:
	/* float does not have a max, MAX or INF is used instead */
	return FALSE;
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
*             (e.g., NCX_BT_INT, NCX_BT_UINT, NCX_BT_FLOAT)
* OUTPUTS:
*     *val == converted number value (0..2G-1), if NO_ERR
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_convert_tkcnum (tk_chain_t *tkc,
			ncx_btype_t  btyp,
			ncx_num_t    *val)
{
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_DNUM:
	return ncx_convert_num(TK_CUR_VAL(tkc), 
			       NCX_NF_DEC, btyp, val);
    case TK_TT_HNUM:
	return ncx_convert_num(TK_CUR_VAL(tkc), 
			       NCX_NF_HEX, btyp, val);
    case TK_TT_RNUM:
	return ncx_convert_num(TK_CUR_VAL(tkc), 
			       NCX_NF_REAL, btyp, val);
    default:
	/* if this is a string, then this might work */
	return ncx_decode_num(TK_CUR_VAL(tkc), btyp, val);
    }
}  /* ncx_convert_tkcnum */


/********************************************************************
* FUNCTION ncx_compare_strs
* 
* Compare 2 ncx_str_t union contents
*
* INPUTS:
*     str1 == first number
*     str2 == second number
*     btyp == expected data type 
*             (NCX_BT_STRING, NCX_BT_BINARY, NCX_BT_ENAME)
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
#endif

    switch (btyp) {
    case NCX_BT_STRING:
    case NCX_BT_ENAME:
    case NCX_BT_BINARY:
	return xml_strcmp(*str1, *str2);
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return 0;
    }
    /*NOTREACHED*/
}  /* ncx_compare_strs */


/********************************************************************
* FUNCTION ncx_copy_str
* 
* Copy the contents of str1 to str2
* Supports base types:
*     NCX_BT_STRING
*     NCX_BT_BINARY
*     NCX_BT_ENAME
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
#endif

    switch (btyp) {
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_ENAME:
	if (*str1) {
	    *str2 = xml_strdup(*str1);
	    if (!*str2) {
		return ERR_INTERNAL_MEM;
	    }
	} else {
	    *str2 = NULL;
	}
        break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
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
	switch (list->btyp) {
	case NCX_BT_STRING:
	case NCX_BT_BINARY:
	    ncx_clean_str(&lmem->val.str);
	    break;
	case NCX_BT_ENUM:
	    ncx_clean_enum(&lmem->val.enu);
	    break;
	default:
	    /* must be a number */
	    ncx_clean_num(list->btyp, &lmem->val.num);
	}
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
    case NCX_BT_BINARY:
    case NCX_BT_ENAME:
    case NCX_BT_ENUM:
	break;
    default:
	SET_ERROR(ERR_NCX_WRONG_TYPE);
	return FALSE;
    }

    /* search the list for a match */
    for (lmem = (const ncx_lmem_t *)dlq_firstEntry(&list->memQ);
	 lmem != NULL;
	 lmem = (const ncx_lmem_t *)dlq_nextEntry(lmem)) {

	if (list->btyp == NCX_BT_ENUM) {
	    if (!xml_strcmp(str, lmem->val.enu.name)) {
		return TRUE;
	    }
	} else {
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
	switch (list1->btyp) {
	case NCX_BT_STRING:
	case NCX_BT_BINARY:
	    switch (ncx_compare_strs(&s1->val.str, &s2->val.str, 
				     list1->btyp)) {
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
	    break;
	default:
	    switch (ncx_compare_nums(&s1->val.num, &s2->val.num,
				     list1->btyp)) {
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
	case NCX_BT_BINARY:
	    res = ncx_copy_str(&lmem->val.str, &lcopy->val.str, list1->btyp);
	    break;
	default:
	    res = ncx_copy_num(&lmem->val.num, &lcopy->val.num, list1->btyp);
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
* For each member in the src list {
*
*   If rangeQ not NULL:
*      check if merging will violate the range restrictions
*      as each entry is added. If so, then merge type will
*      take note of this condition and overwrite dest list
*      members as needed to keep the length within the maxLength
*
*   Merge src list member into dest, based on mergetyp enum
* }
*
* OK exit
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
		    boolean allow_dups,
		    const dlq_hdr_t *rangeQ)
{
    ncx_lmem_t      *lmem, *dest_lmem;
    const typ_rangedef_t *rdef;
    const ncx_num_t *ub;
    uint32           destcnt;
    boolean          done;
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

    ub = NULL;
    destcnt = 0;

    /* get rid of dups in the src list if duplicates not allowed */
    if (!allow_dups) {
	for (dest_lmem = (ncx_lmem_t *)dlq_firstEntry(&dest->memQ);
	     dest_lmem != NULL;
	     dest_lmem = (ncx_lmem_t *)dlq_nextEntry(dest_lmem)) {
	    done = FALSE;
	    while (!done) {
		lmem = ncx_find_lmem(src, dest_lmem);
		if (lmem) {
		    dlq_remove(lmem);
		    ncx_free_lmem(lmem, dest->btyp);
		} else {
		    done = TRUE;
		}
	    }
	}
    }

    /* get range upper bound if rangeQ non-empty */
    if (rangeQ) {
	rdef = (const typ_rangedef_t *)dlq_lastEntry(rangeQ);
	if (rdef) {
	    ub = &rdef->ub;
	    destcnt = ncx_list_cnt(dest);
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

	/* check if upper bound controls size of list */
	if (ub && destcnt+1 >= ub->u) {
	    /* drop the src entry instead of adding it */
	    ncx_free_lmem(lmem, src->btyp);
	} else {
	    /* merge lmem into the dest list */
	    ncx_insert_lmem(dest, lmem, mergetyp);
	    destcnt++;
	}
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

    if (!*strval) {
	return NO_ERR;
    }

    str1 = strval;
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
	    ncx_free_lmem(lmem, btyp);
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
    ncx_finish_list (const typ_def_t *typdef,
		     ncx_list_t *list)
{
    ncx_lmem_t      *lmem;
    xmlChar         *str;
    ncx_btype_t      btyp;
    status_t         res, retres;

    btyp = typ_get_basetype(typdef);
    retres = NO_ERR;

    /* check if any work to do */
    switch (btyp) {
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
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
	    res = val_enum_ok(typdef, str,
			      &lmem->val.enu.val,
			      &lmem->val.enu.name);
	} else {
	    res = ncx_decode_num(str, btyp, &lmem->val.num);
	}
	m__free(str);
	lmem->val.str = NULL;

	if (res != NO_ERR) {
	    /* the string did not match this pattern */
	    retres = res;
	    lmem->flags |= NCX_FL_VALUE_ERR;
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

    switch (btyp) {
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
	ncx_clean_str(&lmem->val.str);
	break;
    case NCX_BT_ENUM:
	ncx_clean_enum(&lmem->val.enu);
	break;
    default:
	/* must be a number */
	ncx_clean_num(btyp, &lmem->val.num);
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
    ncx_lmem_t      *lmem;
    const ncx_num_t *num;
    const ncx_str_t *str;
    const ncx_enum_t *enu;
    int32            cmpval;

#ifdef DEBUG
    if (!list || !memval) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    num = NULL;
    str = NULL;
    enu = NULL;

    if (typ_is_number(list->btyp)) {
	num = &memval->val.num;
    } else if (typ_is_string(list->btyp)) {
	str = &memval->val.str;
    } else if (list->btyp == NCX_BT_ENUM) {
	enu = &memval->val.enu;
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
	} else {
	    cmpval = ncx_compare_enums(&lmem->val.enu, enu);
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
    ncx_lmem_t      *lmem;
    const ncx_num_t *num;
    const ncx_str_t *str;
    const ncx_enum_t *enu;
    int32            cmpval;

#ifdef DEBUG
    if (!list || !memval) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    num = NULL;
    str = NULL;
    enu = NULL;

    if (typ_is_number(list->btyp)) {
	num = &memval->val.num;
    } else if (typ_is_string(list->btyp)) {
	str = &memval->val.str;
    } else if (list->btyp == NCX_BT_ENUM) {
	enu = &memval->val.enu;
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

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
	for (lmem = (ncx_lmem_t *)dlq_firstEntry(&list->memQ);
	     lmem != NULL;
	     lmem = (ncx_lmem_t *)dlq_nextEntry(lmem)) {
	    if (num) {
		cmpval = ncx_compare_nums(&lmem->val.num, num, list->btyp);
	    } else if (str) {
		cmpval = ncx_compare_strs(&lmem->val.str, str, list->btyp);
	    } else {
		cmpval = ncx_compare_enums(&lmem->val.enu, enu);
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
* FUNCTION ncx_new_xlist
* 
* Malloc Initialize an allocated ncx_xlist_t
*
* RETURNS:
*   pointer to new entry, or NULL if memory error
*********************************************************************/
ncx_xlist_t *
    ncx_new_xlist (void)
{
    ncx_xlist_t *list;

    list = m__getObj(ncx_xlist_t);
    if (list) {
	ncx_init_xlist(list);
    }
    return list;

} /* ncx_new_xlist */


/********************************************************************
* FUNCTION ncx_init_xlist
* 
* Initialize an allocated ncx_xlist_t
*
* INPUTS:
*    list == pointer to ncx_xlist_t memory
*********************************************************************/
void
    ncx_init_xlist (ncx_xlist_t *list)
{
#ifdef DEBUG
    if (!list) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    list->btyp = NCX_BT_STRING;
    dlq_createSQue(&list->strQ);

} /* ncx_init_xlist */


/********************************************************************
* FUNCTION ncx_clean_xlist
* 
* Scrub the memory of a ncx_xlist_t but do not delete it
*
* INPUTS:
*    list == ncx_xlist_t struct to clean
*********************************************************************/
void
    ncx_clean_xlist (ncx_xlist_t *list)
{
    ncx_lstr_t  *lstr;

#ifdef DEBUG
    if (!list) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* clean the string Q */
    while (!dlq_empty(&list->strQ)) {
	lstr = (ncx_lstr_t *)dlq_deque(&list->strQ);
	if (lstr->str) {
	    m__free(lstr->str);
	}
	m__free(lstr);
    }

    list->btyp = NCX_BT_NONE;
    /* leave the list->strQ ready to use again */

} /* ncx_clean_xlist */


/********************************************************************
* FUNCTION ncx_free_xlist
* 
* Clean and free an allocated ncx_xlist_t
*
* INPUTS:
*    list == pointer to ncx_xlist_t memory
*********************************************************************/
void
    ncx_free_xlist (ncx_xlist_t *list)
{
#ifdef DEBUG
    if (!list) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ncx_clean_xlist(list);
    m__free(list);

} /* ncx_free_xlist */


/********************************************************************
* FUNCTION ncx_xlist_cnt
* 
* Get the number of entries in the xlist
*
* INPUTS:
*    list == pointer to ncx_xlist_t memory
* RETURNS:
*    number of entries counted
*********************************************************************/
uint32
    ncx_xlist_cnt (const ncx_xlist_t *list)
{
    const ncx_lstr_t *lstr;
    uint32      cnt;

#ifdef DEBUG
    if (!list) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    cnt = 0;
    for (lstr = (const ncx_lstr_t *)dlq_firstEntry(&list->strQ);
	 lstr != NULL;
	 lstr = (const ncx_lstr_t *)dlq_nextEntry(lstr)) {
	cnt++;
    }
    return cnt;

} /* ncx_xlist_cnt */


/********************************************************************
* FUNCTION ncx_compare_xlists
* 
* Compare 2 ncx_xlist_t struct contents
*
* Expected data type (NCX_BT_XLIST)
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
    ncx_compare_xlists (const ncx_xlist_t *list1,
			const ncx_xlist_t *list2)
{
    const ncx_lstr_t  *s1, *s2;

#ifdef DEBUG
    if (!list1 || !list2) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
    if (list1->btyp != list2->btyp) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return 0;
    }	
#endif

    /* get start strings */
    s1 = (const ncx_lstr_t *)dlq_firstEntry(&list1->strQ);
    s2 = (const ncx_lstr_t *)dlq_firstEntry(&list2->strQ);
	
    /* have 2 start strings to compare */
    for (;;) {
	if (!s1 && !s2) {
	    return 0;
	} else if (!s1) {
	    return -1;
	} else if (!s2) {
	    return 1;
	}
	switch (ncx_compare_strs(&s1->str, &s2->str, list1->btyp)) {
	case -1:
	    return -1;
	case 0:
	    s1 = (const ncx_lstr_t *)dlq_nextEntry(s1);
	    s2 = (const ncx_lstr_t *)dlq_nextEntry(s2);
	    break;
	case 1:
	    return 1;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return 0;
	}
    }
    /*NOTREACHED*/

}  /* ncx_compare_xlists */


/********************************************************************
* FUNCTION ncx_copy_xlist
* 
* Copy the contents of list1 to list2
* Supports base type NCX_BT_XLIST
*
* INPUTS:
*     list1 == first list
*     list2 == second list
*
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_copy_xlist (const ncx_xlist_t *list1,
		    ncx_xlist_t *list2)
{
    const ncx_lstr_t *lstr;
    ncx_lstr_t       *copylstr;
    status_t          res;

#ifdef DEBUG
    if (!list1 || !list2) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    list2->btyp = list1->btyp;
    dlq_createSQue(&list2->strQ);

    for (lstr = (const ncx_lstr_t *)dlq_firstEntry(&list1->strQ);
	 lstr != NULL && res==NO_ERR;
	 lstr = (const ncx_lstr_t *)dlq_nextEntry(lstr)) {

	copylstr = ncx_new_lstr();
	if (!copylstr) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    res = ncx_copy_str(&lstr->str, &copylstr->str, NCX_BT_STRING);
	    if (res == NO_ERR) {
		copylstr->flags = lstr->flags;
		dlq_enque(copylstr, &list2->strQ);
	    } else {
		ncx_free_lstr(copylstr);
	    }
	}
    }
    return res;
}  /* ncx_copy_xlist */


/********************************************************************
* FUNCTION ncx_merge_xlist
* 
* !!! xlist is deprecated!!! Use list instead!!!
*
* The merge function is handled specially for lists.
* The contents are not completely replaced like a string.
* Instead, only new entries from src are added to the dest list.
*
* NCX merge algorithm for xlists:
*
* If allow_dups == FALSE:
*    check if entry exists; if so, exit;
*
* For each member in the src list {
*
*   If rangeQ not NULL:
*      check if merging will violate the range restrictions
*      as each entry is added. If so, then merge type will
*      take note of this condition and overwrite dest list
*      members as needed to keep the length within the maxLength
*
*   Merge src list member into dest, based on mergetyp enum
* }
*
* OK exit
*
* INPUTS:
*    src == ncx_xlist_t struct to merge from
*    dest == ncx_xlist_t struct to merge into
*    mergetyp == type of merge used for this list
*    allow_dups == TRUE if this list allows duplicate values
*
* OUTPUTS:
*    ncx_lstr_t structs will be moved from the src to dest as needed
*
* RETURNS:
*   none
*********************************************************************/
void
    ncx_merge_xlist (ncx_xlist_t *src,
		     ncx_xlist_t *dest,
		     ncx_merge_t mergetyp)
{
    ncx_lstr_t      *lstr;

#ifdef DEBUG
    if (!src || !dest) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* transfer the source members to the dest list */
    while (!dlq_empty(&src->strQ)) {

	/* pick an entry to merge, reverse of the merge type
	 * to preserve the source order in the dest list
	 */
	switch (mergetyp) {
	case NCX_MERGE_FIRST:
	    lstr = (ncx_lstr_t *)dlq_lastEntry(&src->strQ);
	    break;
	case NCX_MERGE_LAST:
	case NCX_MERGE_SORT:
	    lstr = (ncx_lstr_t *)dlq_firstEntry(&src->strQ);
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return;
	}
	dlq_remove(lstr);

	/* merge lmem into the dest list */
	ncx_insert_lstr(dest, lstr, mergetyp);
    }

}  /* ncx_merge_xlist */


/********************************************************************
* FUNCTION ncx_set_xlist
* 
* Parse the XML input as an NCX_BT_XLIST
* Do not check the individual strings against any restrictions
*
* INPUTS:
*     strval == cleaned XML string to parse into ncx_str_t values
*     list == ncx_xlist_t in progress that will get the ncx_lstr_t 
*             structs added to it, as they are parsed
*
* OUTPUTS:
*     list->strQ has 0 or more ncx_lstr_t structs appended to it
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    ncx_set_xlist (const xmlChar *strval,
		   ncx_xlist_t  *list)
{
    status_t           res;
    boolean            done;
    const xmlChar     *str1, *str2;
    ncx_lstr_t        *lstr;

    if (!*strval) {
	return NO_ERR;
    }

    str1 = strval;
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
	lstr = ncx_new_lstr();
	if (!lstr) {
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
	    if (!*str2) {
		/*** add warning for missing EOS marker ***/
		    
	    }
	} else {
	    /* consume string until a WS, str-start, or EOS seen */
	    str2 = str1+1;
	    while (*str2 && !xml_isspace(*str2) && 
		   (*str2 != NCX_STR_START)) {
		str2++;
	    }
	}

	/* copy the string just parsed */
	res = NO_ERR;
	lstr->str = xml_strndup(str1, (uint32)(str2-str1));
	if (!lstr->str) {
	    res = ERR_INTERNAL_MEM;
	} 
	if (res != NO_ERR) {
	    ncx_free_lstr(lstr);
	    return res;
	}

	/* save the string in the list string Q */
	dlq_enque(lstr, &list->strQ);

	/* reset the string pointer and loop */
	str1 = str2;
    }

    return NO_ERR;

}  /* ncx_set_xlist */


/********************************************************************
* FUNCTION ncx_new_lstr
*
* Malloc and fill in a new ncx_lstr_t struct
*
* INPUTS:
*   none
* RETURNS:
*   pointer to malloced and initialized ncx_lstr_t struct
*   NULL if malloc error
*********************************************************************/
ncx_lstr_t *
    ncx_new_lstr (void)
{
    ncx_lstr_t  *lstr;
    

    lstr = m__getObj(ncx_lstr_t);
    if (!lstr) {
	return NULL;
    }
    memset(lstr, 0x0, sizeof(ncx_lstr_t));
    return lstr;

}  /* ncx_new_lstr */


/********************************************************************
* FUNCTION ncx_free_lstr
*
* Free all the memory in a  ncx_lstr_t struct
*
* INPUTS:
*   lstr == struct to clean and free
*   btyp == base type of the string
*
*********************************************************************/
void
    ncx_free_lstr (ncx_lstr_t *lstr)
{
#ifdef DEBUG
    if (!lstr) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (lstr->str) {
	m__free(lstr->str);
    } 
    m__free(lstr);

}  /* ncx_free_lstr */


/********************************************************************
* FUNCTION ncx_insert_lstr
*
* !!! xlist is deprecated!!! Use list instead!!!
*
* Insert an xlist entry into the specified xlist
*
* INPUTS:
*   list == xlist to insert into
*   strval == value to insert, based on list->btyp
*   mergetyp == requested merge type for the insertion
*
* RETURNS:
*   none
*********************************************************************/
void
    ncx_insert_lstr (ncx_xlist_t *list,
		     ncx_lstr_t *strval,
		     ncx_merge_t mergetyp)
{
    ncx_lstr_t      *lstr;
    int32            cmpval;

#ifdef DEBUG
    if (!list || !strval) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    switch (mergetyp) {
    case NCX_MERGE_FIRST:
	lstr = (ncx_lstr_t *)dlq_firstEntry(&list->strQ);
	if (lstr) {
	    dlq_insertAhead(strval, lstr);
	} else {
	    dlq_enque(strval, &list->strQ);
	}
	break;
    case NCX_MERGE_LAST:
	dlq_enque(lstr, &list->strQ);
	break;
    case NCX_MERGE_SORT:
	for (lstr = (ncx_lstr_t *)dlq_firstEntry(&list->strQ);
	     lstr != NULL;
	     lstr = (ncx_lstr_t *)dlq_nextEntry(lstr)) {
	    cmpval = ncx_compare_strs(&lstr->str, &strval->str, list->btyp);
	    if (cmpval >= 0) {
		dlq_insertAhead(strval, lstr);
		return;
	    }
	}

	/* make new last entry */
	dlq_enque(strval, &list->strQ);
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }	

}  /* ncx_insert_lstr */


/********************************************************************
* FUNCTION ncx_new_appnode
*
* Malloc and fill in a new ncx_appnode_t struct
*
* INPUTS:
*   owner == owner name
*   appname == application name
*   nsid  == application namespace ID
* RETURNS:
*  const pointer to the string value
*********************************************************************/
ncx_appnode_t *
    ncx_new_appnode (const xmlChar *owner,
		     const xmlChar *appname,
		     xmlns_id_t  nsid)
{
    ncx_appnode_t  *app;
    

    app = m__getObj(ncx_appnode_t);
    if (!app) {
	return NULL;
    }
    app->owner = xml_strdup(owner);
    if (!app->owner) {
	m__free(app);
	return NULL;
    }
    app->appname = xml_strdup(appname);
    if (!app->appname) {
	m__free(app->owner);
	m__free(app);
	return NULL;
    }
    app->nsid = nsid;

    return app;

}  /* ncx_new_appnode */


/********************************************************************
* FUNCTION ncx_free_appnode
*
* Free all the memory in a new ncx_appnode_t struct
*
* INPUTS:
*   app == ncx_appnode_t to destroy
*
*********************************************************************/
void
    ncx_free_appnode (ncx_appnode_t *app)
{
#ifdef DEBUG
    if (!app) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (app->owner && app->appname) {
	def_reg_del_cfgapp(app->owner, app->appname, NCX_CFGID_RUNNING);
    }
    if (app->owner) {
	m__free(app->owner);
    }
    if (app->appname) {
	m__free(app->appname);
    }
    m__free(app);

}  /* ncx_free_appnode */


/********************************************************************
* FUNCTION ncx_find_appnode
*
* Find an ncx_appnode_t struct in the ncx_appQ
*
* INPUTS:
*   owner == owner name to find
*   appname == application name to find
*
* RETURNS:
*  pointer to the struct if found
*********************************************************************/
ncx_appnode_t *
    ncx_find_appnode (const xmlChar *owner,
		      const xmlChar *appname)
{
    ncx_appnode_t  *app;

#ifdef DEBUG
    if (!owner|| !appname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (app = (ncx_appnode_t *)dlq_firstEntry(&ncx_appQ);
	 app != NULL;
	 app = (ncx_appnode_t *)dlq_nextEntry(app)) {
	if (!xml_strcmp(app->owner, owner) &&
	    !xml_strcmp(app->appname, appname)) {
	    return app;
	}
    }
    return NULL;

}  /* ncx_find_appnode */


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
*    varname == name string of the appinfo variable to find
*
* RETURNS:
*    pointer to the ncx_appinfo_t struct for the entry if found
*    NULL if the entry is not found
*********************************************************************/
const ncx_appinfo_t *
    ncx_find_appinfo (const dlq_hdr_t *appinfoQ,
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
	if (!xml_strcmp(varname, appinfo->name)) {
	    return appinfo;
	}
    }
    return NULL;

}  /* ncx_find_appinfo */


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
*   tkc == token chain
*   mod == ncx_module_t in progress
*   appinfoQ == queue to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_resolve_appinfoQ (tk_chain_t *tkc,
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


	if (appinfo->prefix &&
	    xml_strcmp(appinfo->prefix, mod->prefix)) {

	    res = yang_find_imp_extension(tkc, mod, appinfo->prefix,
					  appinfo->name, appinfo->tk,
					  &ext);
	    CHK_EXIT;
	} else {

	    ext = ext_find_extension(&mod->extensionQ, appinfo->name);
	    if (!ext) {
		log_error("\nError: Local module extension '%s' not found",
			  appinfo->name);
		res = retres = ERR_NCX_DEF_NOT_FOUND;
		tkc->cur = appinfo->tk;
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
		tkc->cur = appinfo->tk;
		ncx_print_errormsg(tkc, mod, retres);
	    } else if (!ext->arg && appinfo->value) {
		retres = ERR_NCX_EXTRA_PARM;
		log_error("\nError: argument '%s' provided for"
			  " extension '%s:%s' is not allowed",
			  appinfo->value, appinfo->prefix, ext->name);
		tkc->cur = appinfo->tk;
		ncx_print_errormsg(tkc, mod, retres);
	    }
	}

	/* recurse through any nested appinfo statements */
	res = ncx_resolve_appinfoQ(tkc, mod, appinfo->appinfoQ);
	CHK_EXIT;
    }

    return retres;

}  /* ncx_resolve_appinfoQ */


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

    /* check the cache first */
    if (ncx_cur_filptrs < ncx_max_filptrs) {
	memset(filptr, 0x0, sizeof(ncx_filptr_t));
	dlq_createSQue(&filptr->childQ);
	dlq_enque(filptr, &ncx_filptrQ);
	ncx_cur_filptrs++;
    } else {
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

    while (!dlq_empty(que)) {
	tn = (ncx_typname_t *)dlq_deque(que);
	ncx_free_typname(tn);
    }

}  /* ncx_clean_typnameQ */


/********************************************************************
* FUNCTION ncx_printf_indent
* 
* Printf a newline, then the specified number of chars
*
* INPUTS:
*    indentcnt == number of indent chars, -1 == skip everything
*
*********************************************************************/
void
    ncx_printf_indent (int32 indentcnt)
{
    int32  i;

    if (indentcnt >= 0) {
	log_write("\n");
	for (i=0; i<indentcnt; i++) {
	    log_write(" ");
	}
    }

} /* ncx_printf_indent */


/********************************************************************
* FUNCTION ncx_stdout_indent
* 
* Printf a newline to stdout, then the specified number of chars
*
* INPUTS:
*    indentcnt == number of indent chars, -1 == skip everything
*
*********************************************************************/
void
    ncx_stdout_indent (int32 indentcnt)
{
    int32  i;

    if (indentcnt >= 0) {
	log_stdout("\n");
	for (i=0; i<indentcnt; i++) {
	    log_stdout(" ");
	}
    }

} /* ncx_stdout_indent */


/********************************************************************
* FUNCTION ncx_init_operation_attr
* 
* Create typ_child_t struct for the NETCONF operation attribute
* Must be called after NETCONF module is loaded
*
* RETURNS:
*     status
*********************************************************************/
status_t
    ncx_init_operation_attr (void)
{
    typ_template_t  *typ;
    ncx_node_t        deftyp;

    if (operation_attr_init_done) {
	return NO_ERR;
    }

    deftyp = NCX_NT_TYP;
    typ = (typ_template_t *)
	def_reg_find_moddef(NC_MODULE, NC_OPERATION_ATTR_TYPE, &deftyp);
    if (!typ) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    memset(&operation_attr, 0x0, sizeof(typ_child_t));
    operation_attr.name = xml_strdup(NC_OPERATION_ATTR_NAME);
    if (!operation_attr.name) {
	return ERR_INTERNAL_MEM;
    }

    operation_attr.typdef.iqual = NCX_IQUAL_ONE;
    operation_attr.typdef.class = NCX_CL_NAMED;
    operation_attr.typdef.def.named.typ = typ;
    operation_attr.typdef.def.named.newtyp = NULL;
    operation_attr.typdef.def.named.flags = 0;

    operation_attr_init_done = TRUE;
    return NO_ERR;

} /* ncx_init_operation_attr */


/********************************************************************
* FUNCTION ncx_clean_operation_attr
* 
* Clean the typ_child_t struct for the NETCONF operation attribute
*
*********************************************************************/
void
    ncx_clean_operation_attr (void)
{
    if (!operation_attr_init_done) {
	return;
    }
    if (operation_attr.name) {
	m__free(operation_attr.name);
    }
    memset(&operation_attr, 0x0, sizeof(typ_child_t));
    operation_attr_init_done = FALSE;

} /* ncx_clean_operation_attr */


/********************************************************************
* FUNCTION ncx_get_operation_attr
* 
* Get the typ_child_t struct for the NETCONF operation attribute
*
*********************************************************************/
typ_child_t *
    ncx_get_operation_attr (void)
{
    status_t  res;

    if (!operation_attr_init_done) {
	res = ncx_init_operation_attr();
	if (res != NO_ERR) {
	    return NULL;
	}
    }
    return &operation_attr;

} /* ncx_get_operation_attr */


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
* OUTPUTS:
*    buff is filled in with the namestring segment
* RETURNS:
*    current string pointer after operation
*********************************************************************/
const xmlChar *
    ncx_get_name_segment (const xmlChar *str,
			  xmlChar  *buff)
{
#ifdef DEBUG
    if (!str || !buff) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

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
    } else if (!xml_strcmp(NCX_EL_YANG, (const xmlChar *)str)) {
        return NCX_CVTTYP_YANG;
    } else if (!xml_strcmp(NCX_EL_COPY, (const xmlChar *)str)) {
        return NCX_CVTTYP_COPY;
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
			   uint linenum,
			   boolean fineoln)
{
    boolean      iserr;

    iserr = (res <= ERR_LAST_USR_ERR) ? TRUE : FALSE;

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

    if (mod && mod->sourcefn) {
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

    if (tkc && tkc->cur && TK_CUR_VAL(tkc)) {
	log_write("%u.%u:", TK_CUR_LNUM(tkc), TK_CUR_LPOS(tkc));
    }

    if (iserr) {
	log_write(" error(%u): %s", res, get_error_string(res));
    } else {
	log_write(" warning(%u): %s", res, get_error_string(res));
    }

#ifdef TOO_VERBOSE_FOR_NOW
    if (tkc && tkc->cur && TK_CUR_VAL(tkc)) {
	if (TK_CUR_MOD(tkc)) {
	    if (xml_strlen(TK_CUR_VAL(tkc)) < 20) {
		log_write(": Cur: '%s:%s'",
			  TK_CUR_MOD(tkc), TK_CUR_VAL(tkc));
	    } else {
		log_write(": Cur: '%s:%20s...'", 
			  TK_CUR_MOD(tkc), TK_CUR_VAL(tkc));
	    }
	} else {
	    if (xml_strlen(TK_CUR_VAL(tkc)) < 20) {
		log_write(": Cur: '%s'", TK_CUR_VAL(tkc));
	    } else {
		log_write(": Cur: '%20s...'", TK_CUR_VAL(tkc));
	    }
	}
    }
#endif

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
    ncx_print_errormsg_ex(tkc, NULL, result, NULL, 0,
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
    ncx_print_errormsg_ex(tkc, mod, result, NULL, 0,
			  (expstr) ? FALSE : TRUE);
    if (expstr) {
	log_write("  Expected: %s\n", expstr);
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
    switch (nodetyp) {
    case NCX_NT_NONE:                          /* uninitialized */
	m__free(node);
	break;
    case NCX_NT_RPC:                          /* rpc_template_t */
	rpc_free_template(node);
	break;
    case NCX_NT_TYP:                          /* typ_template_t */
	typ_free_template(node);
	break;
    case NCX_NT_GRP:                          /* grp_template_t */
	grp_free_template(node);
	break;
    case NCX_NT_PSDPARM:                          /* psd_parm_t */
	psd_free_parm(node);
	break;
    case NCX_NT_PSD:                          /* psd_template_t */
	psd_free_template(node);              
	break;
    case NCX_NT_VAL:                             /* val_value_t */
	val_free_value(node);
	break;
    case NCX_NT_PARM:                              /* ps_parm_t */
	ps_free_parm(node);
	break;
    case NCX_NT_PARMSET:                        /* ps_parmset_t */   
	ps_free_parmset(node);
	break;
    case NCX_NT_APP:                               /* cfg_app_t */
	cfg_free_appnode(node);
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
    case NCX_NT_INDEX:                          /* typ_index_t  */
	typ_free_index(node);
	break;
    case NCX_NT_QNAME:                         /* xmlns_qname_t */
	xmlns_free_qname(node);
	break;
    case NCX_NT_TOP:                            /* ncx_filptr_t */
	m__free(node);    /***/
	break;
    case NCX_NT_CHILD:                          /* ncx_filptr_t */
	m__free(node);    /***/
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
    } else if (!xml_strcmp(NCX_EL_TCONFIG, str)) {
        return NCX_DC_TCONFIG;
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
    case NCX_DC_TCONFIG:
	return NCX_EL_TCONFIG;
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
    case NCX_BT_ROOT:
    case NCX_BT_ENUM:
    case NCX_BT_ENAME:
    case NCX_BT_EMPTY:
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
    case NCX_BT_INT64:
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
    case NCX_BT_UINT64:
    case NCX_BT_FLOAT32:
    case NCX_BT_FLOAT64:
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_SLIST:
    case NCX_BT_XLIST:
        return NCX_CL_SIMPLE;
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_LIST:
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
* Check if an xmlChar string is a valid NCX name
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

    if (!len || len>NCX_MAX_NLEN) {
        return FALSE;
    }
    if (!ncx_valid_fname_ch(*str)) {
        return FALSE;
    }
    for (i=1;i<len;i++) {
        if (!ncx_valid_name_ch(str[i])) {
            return FALSE;
        }
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
    uint32  i, len;

#ifdef DEBUG
    if (!str) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    len = xml_strlen(str);
    if (!len || len>NCX_MAX_NLEN) {
        return FALSE;
    }
    if (!ncx_valid_fname_ch(*str)) {
        return FALSE;
    }
    for (i=1;i<len;i++) {
        if (!ncx_valid_name_ch(str[i])) {
            return FALSE;
        }
    }
    return TRUE;
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
	!xml_strcmp(str, (const xmlChar *)"1") ||
	!xml_strcmp(str, (const xmlChar *)"true(1)")) {
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
	!xml_strcmp(str, (const xmlChar *)"0") ||
	!xml_strcmp(str, (const xmlChar *)"false(0)")) {
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
* FUNCTION ncx_consume_dyn_string
* 
* Consume a TK_TT_TSTRING identifier matching 'name', then
* Consume any kind of string token:
*   TK_TT_STRING, TK_TT_SSTRING, TK_TT_TSTRING, or TK_TT_QSTRING 
* If ctk specified, then consume the specified close token
*
* Store the results in the specified buffer
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain 
*   mod == module in progress (NULL if none)
*   name == token name
*   strbuff == address of pointer to buffer to store the string value
*           == NULL if the string should not be saved, just parsed
*   opt == NCX_OPT for optional param
*       == NCX_REQ for mandatory param
*   wsp == NCX_WSP for whitespace allowed (TK_TT_QSTRING allowed)
*       == NCX_NO_WSP for whitespace not allowed
*   ctyp == close token (use TK_TT_NONE to skip this part)
*
* OUTPUTS:
*   strbuff is filled in, if NO_ERR
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_consume_dyn_string (tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    const xmlChar *name,
			    xmlChar **strbuff,
			    ncx_opt_t opt,
			    ncx_strtyp_t wsp,
			    tk_type_t  ctyp)
{
    status_t     res;

#ifdef DEBUG
    if (!tkc || !name) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check the identifier token */
    res = TK_ADV(tkc);
    if (res == NO_ERR) {
	if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
	    if (opt == NCX_OPT) {
		TK_BKUP(tkc);
		return ERR_NCX_SKIPPED;
	    } else {
		res = ERR_NCX_WRONG_TKTYPE;
	    }
	} else if (xml_strcmp(TK_CUR_VAL(tkc), name)) {
	    if (opt == NCX_OPT) {
		TK_BKUP(tkc);
		return ERR_NCX_SKIPPED;
	    } else {
		res = ERR_NCX_WRONG_TKVAL;
	    }
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
	return res;
    }

    /* get the string token */
    res = TK_ADV(tkc);
    if (res == NO_ERR) {
	if (wsp==NCX_WSP) {
	    if (!TK_CUR_WSTR(tkc)) {
		res = ERR_NCX_WRONG_TKTYPE;
	    }
	} else if (!TK_CUR_NOWSTR(tkc)) {
	    res = ERR_NCX_WRONG_TKTYPE;
	}

	if (res == NO_ERR && strbuff) {
	    *strbuff = xml_strdup(TK_CUR_VAL(tkc));
	    if (!*strbuff) {
		res = ERR_INTERNAL_MEM;
	    }
	}
    }

    /* check for a closing token */
    if (res == NO_ERR && ctyp != TK_TT_NONE) {
        res = TK_ADV(tkc);
        if (res == NO_ERR) {
	    if (TK_CUR_TYP(tkc) != ctyp) {
		if (strbuff && *strbuff) {
		    m__free(*strbuff);
		    *strbuff = NULL;
		}
		res = ERR_NCX_WRONG_TKTYPE;
	    }
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
    }
    return res;

} /* ncx_consume_dyn_string */


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
    }

    return res;

} /* ncx_consume_name */


/********************************************************************
* FUNCTION ncx_consume_mname
* 
* Consume a TK_TSTRING that matches the 'name', then
* retrieve the 'value' (TK_TSTRING or TK_TT_MSTRING) token 
* into the namebuff.
*
* If ctk specified, then consume the specified close token
*
* Store the results in the specified buffers
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* If name is NULL, then the opt parameter is not used,
* and an mname token must be present. Otherwise, the
* name string is checked against the first token, and
* if optional, the token chain will back up and return NO_ERR.

* INPUTS:
*   tkc == token chain 
*   mod == module in progress (NULL if none)
*   name == first token name
*   namebuff == pointer to output name string
*   modstr == pointer to output module name string
*   opt == NCX_OPT for optional param
*       == NCX_REQ for mandatory param
*   ctyp == close token (use TK_TT_NONE to skip this part)
*
* OUTPUTS:
*   *namebuff points at the malloced name buffer
*   *modstr points to a strdup of the module name if this
*      is a TK_TT_MSTRING token
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    ncx_consume_mname (tk_chain_t *tkc,
		       ncx_module_t *mod,
		       const xmlChar *name,
		       xmlChar **namebuff,
		       xmlChar **modstr,
		       ncx_opt_t opt,
		       tk_type_t  ctyp)
{
    status_t     res;

#ifdef DEBUG
    if (!tkc || !name || !namebuff || !modstr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    /* check 'name' token if it is supplied */
    if (name) {
	res = TK_ADV(tkc);
	if (res != NO_ERR) {
	    ncx_print_errormsg(tkc, mod, res);
	    return res;
	} else {
	    if (TK_CUR_TYP(tkc) != TK_TT_TSTRING) {
		/* wrong token type -- backup if this is optional */
		if (opt==NCX_OPT) {
		    TK_BKUP(tkc);
		    return ERR_NCX_SKIPPED;
		} else {
		    res = ERR_NCX_WRONG_TKTYPE;
		}
	    }
	}
	if (res==NO_ERR && xml_strcmp(TK_CUR_VAL(tkc), name)) {
	    /* wrong name value -- backup if this is optional */
	    if (opt==NCX_OPT) {
		TK_BKUP(tkc);
		return ERR_NCX_SKIPPED;
	    } else {
		res = ERR_NCX_WRONG_TKVAL;
	    }
	}
    }

    /* check string value token */
    if (res == NO_ERR) {
	res = TK_ADV(tkc);
	if (res == NO_ERR) {
	    if (!TK_CUR_ID(tkc)) {
		res = ERR_NCX_WRONG_TKTYPE;
	    } else {
		*namebuff = xml_strdup(TK_CUR_VAL(tkc));
		if (!*namebuff) {
		    res = ERR_INTERNAL_MEM;
		}
	    }
	}
    }
    
    if (res==NO_ERR && TK_CUR_MOD(tkc)) {
        /* module-qualified identifier */
        *modstr = xml_strdup(TK_CUR_MOD(tkc));
        if (!*modstr) {
	    if (*namebuff) {
		m__free(*namebuff);
		*namebuff = NULL;
	    }
            res = ERR_INTERNAL_MEM;
        }
    }

    /* check for a closing token */
    if (res==NO_ERR && ctyp != TK_TT_NONE) {
        res = TK_ADV(tkc);
        if (res == NO_ERR) {
	    if (TK_CUR_TYP(tkc) != ctyp) {
		if (*namebuff) {
		    m__free(*namebuff);
		    *namebuff = NULL;
		}
		if (*modstr) {
		    m__free(*modstr);
		    *modstr = NULL;
		}
		res = ERR_NCX_WRONG_TKTYPE;
	    }
	}
    }

    if (res != NO_ERR) {
	ncx_print_errormsg(tkc, mod, res);
    }

    return res;

} /* ncx_consume_mname */


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
	case TK_SOURCE_NCX:
	    ncx_mod_exp_err(tkc, mod, res, tkname);
	    break;
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

    if (err->xpath) {
	m__free(err->xpath);
	err->xpath = NULL;
    }
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
    ncx_clean_errinfo(err);
    m__free(err);

}  /* ncx_free_errinfo */


/********************************************************************
* FUNCTION ncx_get_source
* 
* Get a malloced buffer containing the complete filespec
* for the given into string.  If this is a complete dirspec,
* this this will just strdup the value.
*
* This is just a best effort to get the full spec.
* If the full spec is greater than 1500 bytes,
* then only the filespec will be copied, if the
* current directory needs to be prepended to the filespec
*
* INPUTS:
*    fspec == input filespec
*
* RETURNS:
*   malloced buffer containing possibly expanded full filespec
*********************************************************************/
xmlChar *
    ncx_get_source (const xmlChar *fspec)
{
    xmlChar *buff;
    uint32   bufflen;

#define DIRBUFF_SIZE 1500

    if (*fspec == NCXMOD_PSCHAR) {
	return xml_strdup(fspec);
    }

    buff = m__getMem(DIRBUFF_SIZE);
    if (!buff) {
	return NULL;
    }

    if (!getcwd((char *)buff, DIRBUFF_SIZE)) {
	SET_ERROR(ERR_BUFF_OVFL);
	m__free(buff);
	return xml_strdup(fspec);
    }

    bufflen = xml_strlen(buff);

    if ((bufflen + xml_strlen(fspec) + 1) >= DIRBUFF_SIZE) {
	SET_ERROR(ERR_BUFF_OVFL);
	m__free(buff);
	return xml_strdup(fspec);
    }

    buff[bufflen] = NCXMOD_PSCHAR;
    xml_strcpy(&buff[bufflen+1], fspec);

    return buff;

}  /* ncx_get_source */


/* END file ncx.c */
