/*  FILE: xpath.c

    Schema and data model Xpath search support
		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
31dec07      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_grp
#include "grp.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                         V A R I A B L E S                         *
*                                                                   *
*********************************************************************/



/********************************************************************
* FUNCTION do_errmsg
* 
* Generate the errormsg
*
* INPUTS:
*    tkc == token chain in progress (may be NULL)
*    mod == module in progress
*    errtk == error tk_token_t to temp. set
*    res == error enumeration
*
*********************************************************************/
static void
    do_errmsg (tk_chain_t *tkc,
	       ncx_module_t *mod,
	       tk_token_t *errtk,
	       status_t  res)
{
    tk_token_t  *savetk;

    if (tkc) {
	savetk = tkc->cur;
	tkc->cur = errtk;
    }
    ncx_print_errormsg(tkc, mod, res);
    if (tkc) {
	tkc->cur = savetk;
    }
}  /* do_errmsg */


/********************************************************************
* FUNCTION next_nodeid
* 
* Get the next Name of QName segment of an Xpath schema-nodeid
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain in progress  (may be NULL)
*    mod == module in progress
*    obj == object calling this fn (for error purposes)
*    target == Xpath expression string to evaluate
*    prefix == buffer to store prefix portion of QName (if any)
*    name == buffer to store name portion of segment
*    len == address of return byte count
*
* OUTPUTS:
*   prefix[] == prefix portion of QName
*   name[] == name portion of QName
*   *cnt == number of bytes used in target
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    next_nodeid (tk_chain_t *tkc,
		 ncx_module_t *mod,
		 obj_template_t *obj,
		 const xmlChar *target,
		 xmlChar *prefix,
		 xmlChar *name,
		 uint32 *len)
{
    const xmlChar *p, *q;
    uint32         cnt;
    status_t       res;

    /* find the EOS or a separator */
    p = target;
    while (*p && *p != ':' && *p != '/') {
	p++;
    }
    cnt = (uint32)(p-target);

    if (!ncx_valid_name(target, cnt)) {
	xml_strncpy(prefix, target, min(cnt, NCX_MAX_NLEN));
	log_error("\nError: invalid name string (%s)", prefix);
	res = ERR_NCX_INVALID_NAME;
	do_errmsg(tkc, mod, obj->tk, res);
	return res;
    }

    if (*p==':') {
	/* copy prefix, then get name portion */
	xml_strncpy(prefix, target, cnt);

	q = ++p;
	while (*q && *q != '/') {
	    q++;
	}
	cnt = (uint32)(q-p);

	if (!ncx_valid_name(p, cnt)) {
	    xml_strncpy(name, p, min(cnt, NCX_MAX_NLEN));
	    log_error("\nError: invalid name string (%s)", name);
	    res = ERR_NCX_INVALID_NAME;
	    do_errmsg(tkc, mod, obj->tk, res);
	    return res;
	}

	xml_strncpy(name, p, cnt);
	*len = (uint32)(q-target);
    } else  {
	/* found EOS or pathsep, got just one 'name' string */
	xml_strncpy(name, target, cnt);
	*prefix = '\0';
	*len = cnt;
    }
    return NO_ERR;

}  /* next_nodeid */


/********************************************************************
* FUNCTION next_nodeid_noerr
* 
* Get the next Name of QName segment of an Xpath schema-nodeid
*
* Error messages are not printed by this function!!
*
* INPUTS:
*    target == Xpath expression string in progress to evaluate
*    prefix == buffer to store prefix portion of QName (if any)
*    name == buffer to store name portion of segment
*    len == address of return byte count
*
* OUTPUTS:
*   prefix[] == prefix portion of QName
*   name[] == name portion of QName
*   *cnt == number of bytes used in target
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    next_nodeid_noerr (const xmlChar *target,
		       xmlChar *prefix,
		       xmlChar *name,
		       uint32 *len)
{
    const xmlChar *p, *q;
    uint32         cnt;

    /* find the EOS or a separator */
    p = target;
    while (*p && *p != ':' && *p != '/') {
	p++;
    }
    cnt = (uint32)(p-target);

    if (!ncx_valid_name(target, cnt)) {
	return ERR_NCX_INVALID_NAME;
    }

    if (*p==':') {
	/* copy prefix, then get name portion */
	xml_strncpy(prefix, target, cnt);

	q = ++p;
	while (*q && *q != '/') {
	    q++;
	}
	cnt = (uint32)(q-p);

	if (!ncx_valid_name(p, cnt)) {
	    return ERR_NCX_INVALID_NAME;
	}

	xml_strncpy(name, p, cnt);
	*len = (uint32)(q-target);
    } else  {
	/* found EOS or pathsep, got just one 'name' string */
	xml_strncpy(name, target, cnt);
	*prefix = '\0';
	*len = cnt;
    }
    return NO_ERR;

}  /* next_nodeid_noerr */


/********************************************************************
* FUNCTION next_val_nodeid
* 
* Get the next Name of QName segment of an Xpath schema-nodeid
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    target == Xpath expression string to evaluate
*    prefix == buffer to store prefix portion of QName (if any)
*    name == buffer to store name portion of segment
*    len == address of return byte count
*
* OUTPUTS:
*   prefix[] == prefix portion of QName
*   name[] == name portion of QName
*   *cnt == number of bytes used in target
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    next_val_nodeid (const xmlChar *target,
		     xmlChar *prefix,
		     xmlChar *name,
		     uint32 *len)
{
    const xmlChar *p, *q;
    uint32         cnt;

    /* find the EOS or a separator */
    p = target;
    while (*p && *p != ':' && *p != '/') {
	p++;
    }
    cnt = (uint32)(p-target);

    if (!ncx_valid_name(target, cnt)) {
	xml_strncpy(prefix, target, min(cnt, NCX_MAX_NLEN));
	log_error("\nError: invalid name string (%s)", prefix);
	return ERR_NCX_INVALID_NAME;
    }

    if (*p==':') {
	/* copy prefix, then get name portion */
	xml_strncpy(prefix, target, cnt);

	q = ++p;
	while (*q && *q != '/') {
	    q++;
	}
	cnt = (uint32)(q-p);

	if (!ncx_valid_name(p, cnt)) {
	    xml_strncpy(name, p, min(cnt, NCX_MAX_NLEN));
	    log_error("\nError: invalid name string (%s)", name);
	    return ERR_NCX_INVALID_NAME;
	}

	xml_strncpy(name, p, cnt);
	*len = (uint32)(q-target);
    } else  {
	/* found EOS or pathsep, got just one 'name' string */
	xml_strncpy(name, target, cnt);
	*prefix = '\0';
	*len = cnt;
    }
    return NO_ERR;

}  /* next_val_nodeid */


/********************************************************************
* FUNCTION find_schema_node
* 
* Follow the absolute-path or descendant node path expression
* and return the obj_template_t that it indicates, and the
* que that the object is in
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain in progress
*    mod == module in progress
*    obj == object calling this fn (for error purposes)
*    datadefQ == Q of obj_template_t containing 'obj'
*    target == Xpath expression string to evaluate
*    targobj == address of return object  (may be NULL)
*    targQ == address of return target queue (may be NULL)
*    errtk == error token to use
*
* OUTPUTS:
*   if non-NULL inputs:
*      *targobj == target object
*      *targQ == datadefQ header for targobj
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    find_schema_node (tk_chain_t *tkc,
		      ncx_module_t *mod,
		      obj_template_t *obj,
		      dlq_hdr_t *datadefQ,
		      const xmlChar *target,
		      obj_template_t **targobj,
		      dlq_hdr_t **targQ,
		      tk_token_t *errtk)
{
    ncx_import_t   *imp;
    ncx_module_t   *impmod;
    obj_template_t *curobj, *nextobj;
    dlq_hdr_t      *curQ;
    const xmlChar  *str;
    uint32          len;
    status_t        res;
    ncx_node_t      dtyp;
    xmlChar         prefix[NCX_MAX_NLEN+1];
    xmlChar         name[NCX_MAX_NLEN+1];

    imp = NULL;
    impmod = NULL;
    dtyp = NCX_NT_OBJ;

    /* skip the first fwd slash, if any */
    if (*target == '/') {
	str = ++target;
    } else {
	str = target;
    }

    /* get the first QName (prefix, name) */
    res = next_nodeid(tkc, mod, obj, str, prefix, name, &len);
    if (res != NO_ERR) {
	return res;
    } else {
	str += len;
    }

    /* get the import if there is a real prefix entered */
    if (*prefix && xml_strcmp(prefix, mod->prefix)) {
	imp = ncx_find_pre_import(mod, prefix);
	if (!imp) {
	    log_error("\nError: prefix '%s' not found in module imports"
		      " in Xpath target %s", prefix, target);
	    res = ERR_NCX_INVALID_NAME;
	    do_errmsg(tkc, mod, errtk, res);
	    return res;
	}
	impmod = ncx_find_module(imp->module);
	if (!impmod) {
	    log_error("\nError: module '%s' not found for prefix %s"
		      " in Xpath target %s",
		      imp->module, prefix, target);
	    res = ERR_NCX_MOD_NOT_FOUND;
	    do_errmsg(tkc, mod, errtk, res);
	    return res;
	}
    }

    /* get the first object template */
    if (imp) {
	curobj = ncx_locate_modqual_import(imp->module, name,
					   mod->diffmode, &dtyp);
    } else if (*target == '/') {
	curobj = obj_find_template_top(mod, mod->name, name);
    } else {
	curobj = obj_find_template(datadefQ, mod->name, name);
    }

    if (!curobj) {
	if (ncx_valid_name2(name)) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else {
	    res = ERR_NCX_INVALID_NAME;
	}
	log_error("\nError: object '%s' not found in module %s"
		  " in Xpath target %s",
		  name, (imp) ? imp->module : mod->name,
		  target);
	do_errmsg(tkc, mod, errtk, res);
	return res;
    } else {
	curQ = datadefQ;
    }

    if (obj_is_augclone(curobj)) {
	res = ERR_NCX_INVALID_VALUE;
	log_error("\nError: augment is external: node '%s'"
		  " from module %s, line %u in Xpath target %s",
		  name, curobj->mod->name,
		  curobj->tk->linenum, target);
	do_errmsg(tkc, mod, errtk, res);
	return res;
    }

    /* got the first object; keep parsing node IDs
     * until the Xpath expression is done or an error occurs
     */
    while (*str == '/') {
	str++;
	/* get the next QName (prefix, name) */
	res = next_nodeid(tkc, mod, obj, str, prefix, name, &len);
	if (res != NO_ERR) {
	    return res;
	} else {
	    str += len;
	}

	/* make sure the prefix is valid, if present */
	if (*prefix && xml_strcmp(prefix, mod->prefix)) {
	    imp = ncx_find_pre_import(mod, prefix);
	    if (!imp) {
		log_error("\nError: prefix '%s' not found in module"
			  " imports in Xpath target %s",
			  prefix, target);
		res = ERR_NCX_INVALID_NAME;
		do_errmsg(tkc, mod, errtk, res);
		return res;
	    }
	} else {
	    imp = NULL;
	}

	/* make sure the name is a valid name string */
	if (!ncx_valid_name2(name)) {
	    log_error("\nError: object name '%s' not a valid "
		      "identifier in Xpath target %s",
		      name, target);
	    res = ERR_NCX_INVALID_NAME;
	    do_errmsg(tkc, mod, errtk, res);
	    return res;
	}

	/* determine 'nextval' based on [curval, prefix, name] */
	curQ = obj_get_datadefQ(curobj);

	if (curQ) {
	    nextobj = obj_find_template(curQ,
					(imp) ? imp->module : 
					mod->name, name);
	} else {
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    log_error("\nError: '%s' in Xpath target '%s' invalid: "
		      "%s on line %u is a %s",
		      name, target, obj_get_name(curobj),
		      curobj->tk->linenum, obj_get_typestr(curobj));
	    do_errmsg(tkc, mod, errtk, res);
	    return res;
	}

	if (nextobj) {
	    curobj = nextobj;
	} else {
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    log_error("\nError: object '%s' not found in module %s",
		      name, (imp) ? imp->module : mod->name);
	    do_errmsg(tkc, mod, errtk, res);
	    return res;
	}
    }

    if (targobj) {
	*targobj = curobj;
    }
    if (targQ) {
	*targQ = curQ;
    }

    return NO_ERR;

}  /* find_schema_node */


/********************************************************************
* FUNCTION find_schema_node_int
* 
* Follow the absolute-path expression
* and return the obj_template_t that it indicates
* A missing prefix means check any namespace for the symbol
*
* !!! Internal version !!!
* !!! Error messages are not printed by this function!!
*
* INPUTS:
*    target == Absolute Xpath expression string to evaluate
*    targobj == address of return object  (may be NULL)
*
* OUTPUTS:
*   if non-NULL inputs:
*      *targobj == target object
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    find_schema_node_int (const xmlChar *target,
			  obj_template_t **targobj)
{
    obj_template_t *curobj, *nextobj;
    dlq_hdr_t      *curQ;
    ncx_module_t   *mod;
    const xmlChar  *str, *modname;
    uint32          len;
    status_t        res;
    ncx_node_t      dtyp;
    xmlChar         prefix[NCX_MAX_NLEN+1];
    xmlChar         name[NCX_MAX_NLEN+1];


    /* skip the first fwd slash, if any
     * the target must be from the config root
     * so if the first fwd-slash is missing then
     * just keep going and assume the config root anyway
     */
    if (*target == '/') {
	str = ++target;
    } else {
	str = target;
    }

    /* get the first QName (prefix, name) */
    res = next_nodeid_noerr(str, prefix, name, &len);
    if (res != NO_ERR) {
	return res;
    } else {
	str += len;
    }

    /* get the import if there is a real prefix entered */
    if (*prefix) {
	mod = def_reg_find_module_prefix(prefix);
	if (!mod) {
	    return ERR_NCX_INVALID_NAME;
	}
	/* get the first object template */
	curobj = obj_find_template_top(mod, mod->name, name);
    } else {
	/* no prefix given, check all top-level objects */
	dtyp = NCX_NT_OBJ;
        curobj = (obj_template_t *)
	    def_reg_find_any_moddef(&modname, name, &dtyp);
    }

    /* check if first level object found */
    if (!curobj) {
	if (ncx_valid_name2(name)) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else {
	    res = ERR_NCX_INVALID_NAME;
	}
	return res;
    }

    if (obj_is_augclone(curobj)) {
	return ERR_NCX_INVALID_VALUE;
    }

    /* got the first object; keep parsing node IDs
     * until the Xpath expression is done or an error occurs
     */
    while (*str == '/') {
	str++;
	/* get the next QName (prefix, name) */
	res = next_nodeid_noerr(str, prefix, name, &len);
	if (res != NO_ERR) {
	    return res;
	} else {
	    str += len;
	}

	/* make sure the name is a valid name string */
	if (!ncx_valid_name2(name)) {
	    return ERR_NCX_INVALID_NAME;
	}

	/* determine 'nextval' based on [curval, prefix, name] */
	curQ = obj_get_datadefQ(curobj);
	if (!curQ) {
	    return ERR_NCX_DEFSEG_NOT_FOUND;
	}

	/* make sure the prefix is valid, if present */
	if (*prefix) {
	    mod = def_reg_find_module_prefix(prefix);
	    if (!mod) {
		return ERR_NCX_INVALID_NAME;
	    }
	    nextobj = obj_find_template(curQ, mod->name, name);
	} else {
	    /* no prefix given; try current module first */
	    nextobj = obj_find_template(curQ, obj_get_mod_name(curobj), 
					name); 
	    if (!nextobj) {
		nextobj = obj_find_template(curQ, NULL, name); 
	    }
	}

	/* setup next loop or error exit because last node not found */
	if (nextobj) {
	    curobj = nextobj;
	} else {
	    return ERR_NCX_DEFSEG_NOT_FOUND;
	}
    }

    if (targobj) {
	*targobj = curobj;
    }

    return NO_ERR;

}  /* find_schema_node_int */


/********************************************************************
* FUNCTION get_curmod_from_prefix
* 
* Get the correct module to use for a given prefix
*
* INPUTS:
*    prefix == string to check
*    mod == module to use for the default context
*           and prefixes will be relative to this module's
*           import statements.
*        == NULL and the default registered prefixes
*           will be used
*    targmod == address of return module
*
* OUTPUTS:
*    *targmod == target moduke to use
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_curmod_from_prefix (const xmlChar *prefix,
			    ncx_module_t *mod,
			    ncx_module_t **targmod)
{
    ncx_import_t   *imp;
    status_t        res;

    /* get the import if there is a real prefix entered */
    if (prefix && *prefix) {
	if (!mod) {
	    *targmod = def_reg_find_module_prefix(prefix);
	    if (!*targmod) {
		res = ERR_NCX_MOD_NOT_FOUND;
	    }
	} else if (xml_strcmp(prefix, mod->prefix)) {
	    imp = ncx_find_pre_import(mod, prefix);
	    if (!imp) {
		res = ERR_NCX_INVALID_NAME;
	    }
	    *targmod = ncx_find_module(imp->module);
	    if (!*targmod) {
		res = ERR_NCX_MOD_NOT_FOUND;
	    }
	} else {
	    *targmod = mod;
	    res = NO_ERR;
	}
    } else if (mod) {
	*targmod = mod;
	res = NO_ERR;
    } else {
	*targmod = NULL;
	res = ERR_NCX_DATA_MISSING;
    }
    return res;

}   /* get_curmod_from_prefix */


/********************************************************************
* FUNCTION find_val_node
* 
* Follow the Xpath expression
* and return the val_value_t that it indicates, if any
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    startval == val_value_t node to start search
*    mod == module to use for the default context
*           and prefixes will be relative to this module's
*           import statements.
*        == NULL and the default registered prefixes
*           will be used
*    target == Xpath expression string to evaluate
*    targval == address of return value  (may be NULL)
*
* OUTPUTS:
*   if non-NULL inputs:
*      *targval == target value node
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    find_val_node (val_value_t *startval,
		   ncx_module_t *mod,
		   const xmlChar *target,
		   val_value_t **targval)
{
    val_value_t    *curval;
    ncx_module_t   *usemod;
    const xmlChar  *str;
    uint32          len;
    status_t        res;
    xmlChar         prefix[NCX_MAX_NLEN+1];
    xmlChar         name[NCX_MAX_NLEN+1];

    /* check absolute path starting with root val */
    if (*target == '/' && !obj_is_root(startval->obj)) {
	log_error("\nError: Absolute path given but startval is not "
		  "root in Xpath target %s", target);
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* check '/' corner-case */
    if (!xml_strcmp(target, (const xmlChar *)"/")) {
	if (targval) {
	    *targval = startval;
	}
	return NO_ERR;
    }

    /* skip the first fwd slash, if any */
    if (*target == '/') {
	str = ++target;
    } else {
	str = target;
    }

    /* get the first QName (prefix, name) */
    res = next_val_nodeid(str, prefix, name, &len);
    if (res != NO_ERR) {
	return res;
    } else {
	str += len;
    }

    res = get_curmod_from_prefix(prefix, mod, &usemod);
    if (res != NO_ERR) {
	if (*prefix) {
	    log_error("\nError: module not found for prefix %s"
		      " in Xpath target %s",
		      prefix, target);
	    return ERR_NCX_MOD_NOT_FOUND;
	} else {
	    log_error("\nError: no module prefix specified"
			  " in Xpath target %s", target);
	    return ERR_NCX_MOD_NOT_FOUND;
	}
    }

    /* get the first value node */
    curval = val_find_child(startval, usemod->name, name);
    if (!curval) {
	if (ncx_valid_name2(name)) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else {
	    res = ERR_NCX_INVALID_NAME;
	}
	log_error("\nError: value node '%s' not found for module %s"
		  " in Xpath target %s",
		  name, usemod->name, target);
	return res;
    }

    /* got the first object; keep parsing node IDs
     * until the Xpath expression is done or an error occurs
     */
    while (*str == '/') {
	str++;
	/* get the next QName (prefix, name) */
	res = next_val_nodeid(str, prefix, name, &len);
	if (res != NO_ERR) {
	    return res;
	} else {
	    str += len;
	}

	res = get_curmod_from_prefix(prefix, mod, &usemod);
	if (res != NO_ERR) {
	    if (*prefix) {
		log_error("\nError: module not found for prefix %s"
			  " in Xpath target %s",
			  prefix, target);
		return ERR_NCX_MOD_NOT_FOUND;
	    } else {
		log_error("\nError: no module prefix specified"
			  " in Xpath target %s", target);
		return ERR_NCX_MOD_NOT_FOUND;
	    }
	}

	/* determine 'nextval' based on [curval, prefix, name] */
	switch (curval->obj->objtype) {
	case OBJ_TYP_CONTAINER:
	case OBJ_TYP_LIST:
	case OBJ_TYP_CHOICE:
	case OBJ_TYP_CASE:
	case OBJ_TYP_RPC:
	case OBJ_TYP_RPCIO:
	case OBJ_TYP_NOTIF:
	    curval = val_find_child(curval, usemod->name, name);
	    if (!curval) {
		if (ncx_valid_name2(name)) {
		    res = ERR_NCX_DEF_NOT_FOUND;
		} else {
		    res = ERR_NCX_INVALID_NAME;
		}
		log_error("\nError: value node '%s' not found for module %s"
			  " in Xpath target %s",
			  name, usemod->name, target);
		return res;
	    }
	    break;
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    log_error("\nError: '%s' in Xpath target '%s' invalid: "
		      "%s is a %s",
		      name, target, curval->name,
		      obj_get_typestr(curval->obj));
	    return res;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    do_errmsg(NULL, mod, NULL, res);
	    return res;
	}
    }

    if (targval) {
	*targval = curval;
    }
    return NO_ERR;

}  /* find_val_node */


/************    E X T E R N A L   F U N C T I O N S    ************/


/********************************************************************
* FUNCTION xpath_find_schema_target
* 
* Follow the absolute-path or descendant-node path expression
* and return the obj_template_t that it indicates, and the
* que that the object is in
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain in progress  (may be NULL: errmsg only)
*    mod == module in progress
*    obj == augment object initiating search, NULL to start at top
*    datadefQ == Q of obj_template_t containing 'obj'
*    target == Xpath expression string to evaluate
*    targobj == address of return object  (may be NULL)
*    targQ == address of return target queue (may be NULL)
*
* OUTPUTS:
*   if non-NULL inputs:
*      *targobj == target object
*      *targQ == datadefQ Q header which contains targobj
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_find_schema_target (tk_chain_t *tkc,
			      ncx_module_t *mod,
			      obj_template_t *obj,
			      dlq_hdr_t  *datadefQ,
			      const xmlChar *target,
			      obj_template_t **targobj,
			      dlq_hdr_t **targQ)
{
    return xpath_find_schema_target_err(tkc, mod, obj, datadefQ, 
					target, targobj, targQ, 
					NULL);

}  /* xpath_find_schema_target */


/********************************************************************
* FUNCTION xpath_find_schema_target_err
* 
* Same as xpath_find_schema_target except a token struct
* is provided to use for the error token, instead of 'obj'
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain in progress (may be NULL: errmsg only)
*    mod == module in progress
*    obj == augment object initiating search, NULL to start at top
*    datadefQ == Q of obj_template_t containing 'obj'
*    target == Xpath expression string to evaluate
*    targobj == address of return object  (may be NULL)
*    targQ == address of return target queue (may be NULL)
*    errtk == error token to use if any messages generated
*
* OUTPUTS:
*   if non-NULL inputs:
*      *targobj == target object
*      *targQ == datadefQ header for targobj
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_find_schema_target_err (tk_chain_t *tkc,
				  ncx_module_t *mod,
				  obj_template_t *obj,
				  dlq_hdr_t  *datadefQ,
				  const xmlChar *target,
				  obj_template_t **targobj,
				  dlq_hdr_t **targQ,
				  tk_token_t *errtk)
{
    status_t    res;

#ifdef DEBUG
    if (!mod || !datadefQ || !target) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (*target == '/') {
	/* check error: if nested object is using an abs. Xpath */
	if (obj && obj->parent) {
	    log_error("\nError: Absolute Xpath expression not "
		      "allowed here (%s)", target);
	    res = ERR_NCX_INVALID_VALUE;
	    do_errmsg(tkc, mod, errtk ? errtk : obj->tk, res);
	    return res;
	}
    }

    res = find_schema_node(tkc, mod, obj, datadefQ,
			   target, targobj, targQ, 
			   errtk ? errtk : (obj ? obj->tk : NULL));
    return res;

}  /* xpath_find_schema_target_err */


/********************************************************************
* FUNCTION xpath_find_schema_int
* 
* Follow the absolute-path expression
* and return the obj_template_t that it indicates
*
* Internal access version
* Error messages are not printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    target == absolute Xpath expression string to evaluate
*    targobj == address of return object  (may be NULL)
*
* OUTPUTS:
*   if non-NULL inputs:
*      *targobj == target object
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_find_schema_target_int (const xmlChar *target,
				  obj_template_t **targobj)
{
#ifdef DEBUG
    if (!target) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return find_schema_node_int(target, targobj);

}  /* xpath_find_schema_target_int */


/********************************************************************
* FUNCTION xpath_find_val_target
* 
* Follow the absolute-path Xpath expression as used
* internally to identify a config DB node
* and return the obj_template_t that it indicates
*
* Expression must be the node-path from root for
* the desired node.
*
* Error messages are logged by this function
*
* INPUTS:
*    cfg == configuration to search
*    mod == module to use for the default context
*           and prefixes will be relative to this module's
*           import statements.
*        == NULL and the default registered prefixes
*           will be used
*    target == Xpath expression string to evaluate
*    targval == address of return value  (may be NULL)
*
* OUTPUTS:
*   if non-NULL inputs and value node found:
*      *targval == target value node
*   If non-NULL targval and error exit:
*      *targval == last good node visited in expression (if any)
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_find_val_target (val_value_t *startval,
			   ncx_module_t *mod,
			   const xmlChar *target,
			   val_value_t **targval)
{

#ifdef DEBUG
    if (!startval || !target) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return find_val_node(startval, mod, target, targval);

}  /* xpath_find_val_target */


/********************************************************************
* FUNCTION xpath_get_keyref_path
* 
* Parse the keyref path as a keyref path
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* path-arg-str           = < a string which matches the rule
*                            path-arg >
*
* path-arg               = absolute-path / relative-path
*
* absolute-path          = 1*("/" (node-identifier *path-predicate))
*
* relative-path          = descendant-path /
*                          (".." "/"
*                          *relative-path)
* 
* descendant-path        = node-identifier *path-predicate
*                          absolute-path
*
* path-predicate         = "[" *WSP path-equality-expr *WSP "]"
* 
* path-equality-expr     = node-identifier *WSP "=" *WSP path-key-expr
*
* path-key-expr          = this-variable-keyword "/" rel-path-keyexpr
*
* rel-path-keyexpr       = 1*(".." "/") *(node-identifier "/")
*                         node-identifier
*
* INPUTS:
*    tkc == token chain in progress (may be NULL: errmsg only)
*    mod == module in progress
*    obj == object initiating search, which contains the keyref type
*    target == Xpath expression string to evaluate
*    errtk == error token to use if any messages generated (may be NULL)
*    targobj == address of return target object (may be NULL)
*
* OUTPUTS:
*   *targobj == pointer to return object target
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_get_keyref_path (tk_chain_t *tkc,
			   ncx_module_t *mod,
			   obj_template_t *obj,
			   const xmlChar *target,
			   tk_token_t *errtk,
			   obj_template_t **targobj)
{
    status_t    res;

#ifdef DEBUG
    if (!mod || !obj || !target) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;  /******/

    return res;

}  /* xpath_get_keyref_path */
			      

/* END xpath.c */
