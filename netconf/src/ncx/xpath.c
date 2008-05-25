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
* FUNCTION next_nodeid
* 
* Get the next Name of QName segment of an Xpath schema-nodeid
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    tkc == token chain in progress
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
    tk_token_t    *savetk;
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
	savetk = tkc->cur;
	tkc->cur = obj->tk;
	ncx_print_errormsg(tkc, mod, res);
	tkc->cur = savetk;
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
	    savetk = tkc->cur;
	    tkc->cur = obj->tk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
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
    obj_rpc_t      *rpc;
    tk_token_t     *savetk;
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
	    savetk = tkc->cur;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	}
	impmod = ncx_find_module(imp->module);
	if (!impmod) {
	    log_error("\nError: module '%s' not found for prefix %s"
		      " in Xpath target %s",
		      imp->module, prefix, target);
	    res = ERR_NCX_MOD_NOT_FOUND;
	    savetk = tkc->cur;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	}
    }

    /* get the first object template */
    if (imp) {
	curobj = ncx_locate_modqual_import(imp->module, name, &dtyp);
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
	savetk = tkc->cur;
	tkc->cur = errtk;
	ncx_print_errormsg(tkc, mod, res);
	tkc->cur = savetk;
	return res;
    }

    if (obj_is_augclone(curobj)) {
	res = ERR_NCX_INVALID_VALUE;
	log_error("\nError: augment is external: node '%s'"
		  " from module %s, line %u in Xpath target %s",
		  name, curobj->mod->name,
		  curobj->tk->linenum, target);
	savetk = tkc->cur;
	tkc->cur = errtk;
	ncx_print_errormsg(tkc, mod, res);
	tkc->cur = savetk;
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
		savetk = tkc->cur;
		tkc->cur = errtk;
		ncx_print_errormsg(tkc, mod, res);
		tkc->cur = savetk;
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
	    savetk = tkc->cur;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	}

	/* determine 'nextval' based on [curval, prefix, name] */
	switch (curobj->objtype) {
	case OBJ_TYP_CONTAINER:
	    nextobj = obj_find_template(curobj->def.container->datadefQ,
					(imp) ? imp->module : 
					mod->name, name);
	    break;
	case OBJ_TYP_LIST:
	    nextobj = obj_find_template(curobj->def.list->datadefQ,
					(imp) ? imp->module : 
					mod->name, name);
	    break;
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    log_error("\nError: '%s' in Xpath target '%s' invalid: "
		      "%s on line %u is a %s",
		      name, target, obj_get_name(curobj),
		      curobj->tk->linenum, obj_get_typestr(curobj));
	    savetk = tkc->cur;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	case OBJ_TYP_CHOICE:
	    nextobj = obj_find_template(curobj->def.choic->caseQ,
					(imp) ? imp->module : 
					mod->name, name);
	    break;
	case OBJ_TYP_CASE:
	    nextobj = obj_find_template(curobj->def.cas->datadefQ,
					(imp) ? imp->module : 
					mod->name, name);
	    break;
	case OBJ_TYP_RPC:
	    rpc = curobj->def.rpc;
	    if (*prefix) {
		log_error("\nError: prefix '%s' not allowed for "
			  "input/output node in Xpath target %s",
			  prefix, target);
		savetk = tkc->cur;
		tkc->cur = errtk;
		ncx_print_errormsg(tkc, mod, res);
		tkc->cur = savetk;
		return res;
	    }
	    if (!xml_strcmp(name, YANG_K_INPUT) ||
		!xml_strcmp(name, YANG_K_OUTPUT)) {
		nextobj = obj_find_template(&rpc->datadefQ, NULL, name);
	    } else {
		log_error("\nError: object '%s' not allowed for rpc"
			  " in Xpath target %s", name, target);
		savetk = tkc->cur;
		tkc->cur = errtk;
		ncx_print_errormsg(tkc, mod, res);
		tkc->cur = savetk;
		return res;
	    }
	    break;
	case OBJ_TYP_RPCIO:
	    nextobj = obj_find_template(&curobj->def.rpcio->datadefQ,
					(imp) ? imp->module : 
					mod->name, name);
	    break;
	case OBJ_TYP_NOTIF:
	    nextobj = obj_find_template(&curobj->def.notif->datadefQ,
					(imp) ? imp->module : 
					mod->name, name);
	    break;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    savetk = tkc->cur;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	}

	if (nextobj) {
	    curobj = nextobj;
	} else {
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    log_error("\nError: object '%s' not found in module %s",
		      name, (imp) ? imp->module : mod->name);
	    savetk = tkc->cur;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	}
    }

    if (targobj) {
	*targobj = curobj;
    }
    if (targQ) {
	switch (curobj->objtype) {
	case OBJ_TYP_CONTAINER:
	    *targQ = curobj->def.container->datadefQ;
	    break;
	case OBJ_TYP_LIST:
	    *targQ = curobj->def.list->datadefQ;
	    break;
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    log_error("\nError: path '%s' invalid: %s on line %u"
		      " is a %s", name, obj_get_name(curobj),
		      curobj->tk->linenum,
		      obj_get_typestr(curobj));
	    savetk = tkc->cur;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	case OBJ_TYP_CHOICE:
	    *targQ = curobj->def.choic->caseQ;
	    break;
	case OBJ_TYP_CASE:
	    *targQ = curobj->def.cas->datadefQ;
	    break;
	case OBJ_TYP_RPC:
	    *targQ = &curobj->def.rpc->datadefQ;
	    break;
	case OBJ_TYP_RPCIO:
	    *targQ = &curobj->def.rpcio->datadefQ;
	    break;
	case OBJ_TYP_NOTIF:
	    *targQ = &curobj->def.notif->datadefQ;
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    savetk = tkc->cur;
	    tkc->cur = errtk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	}
    }

    return NO_ERR;

}  /* find_schema_node */


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
*    tkc == token chain in progress
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
*      *targQ == datadefQ header for targobj
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
    tk_token_t *savetk;
    status_t    res;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ || !target) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (*target == '/') {
	/* check error: if nested object is using an abs. Xpath */
	if (obj && obj->parent) {
	    log_error("\nError: Absolute Xpath expression not "
		      "allowed here (%s)", target);
	    res = ERR_NCX_INVALID_VALUE;
	    savetk = tkc->cur;
	    tkc->cur = obj->tk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	}
    }

    res = find_schema_node(tkc, mod, obj, datadefQ,
			   target, targobj, targQ, obj->tk);
    return res;

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
*    tkc == token chain in progress
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
    tk_token_t *savetk;
    status_t    res;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ || !target || !errtk) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (*target == '/') {
	/* check error: if nested object is using an abs. Xpath */
	if (obj && obj->parent) {
	    log_error("\nError: Absolute Xpath expression not "
		      "allowed here (%s)", target);
	    res = ERR_NCX_INVALID_VALUE;
	    savetk = tkc->cur;
	    tkc->cur = obj->tk;
	    ncx_print_errormsg(tkc, mod, res);
	    tkc->cur = savetk;
	    return res;
	}
    }

    res = find_schema_node(tkc, mod, obj, datadefQ,
			   target, targobj, targQ, errtk);
    return res;

}  /* xpath_find_schema_target_err */
			      

/* END xpath.c */
