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

#ifndef _H_xpath1
#include "xpath1.h"
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

    savetk = NULL;
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
*    prefix == address for return buffer to store 
*          prefix portion of QName (if any)
*    name == address for return buffer to store 
*          name portion of segment
*    len == address of return byte count
*
* OUTPUTS:
*   *prefix == malloced buffer with prefix portion of QName
*   *name == malloced name name portion of QName
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
		 xmlChar **prefix,
		 xmlChar **name,
		 uint32 *len)
{
    const xmlChar *p, *q;
    uint32         cnt;
    status_t       res;

    *prefix = NULL;
    *name = NULL;
    *len = 0;

    /* find the EOS or a separator */
    p = target;
    while (*p && *p != ':' && *p != '/') {
	p++;
    }
    cnt = (uint32)(p-target);

    if (!ncx_valid_name(target, cnt)) {
	log_error("\nError: invalid name string (%s)", 
		  target);
	res = ERR_NCX_INVALID_NAME;
	do_errmsg(tkc, mod, obj->tk, res);
	return res;
    }

    if (*p==':') {
	/* copy prefix, then get name portion */
	*prefix = m__getMem(cnt+1);
	if (!*prefix) {
	    log_error("\nError: malloc failed");
	    res = ERR_INTERNAL_MEM;
	    do_errmsg(tkc, mod, obj->tk, res);
	    return res;
	}
	xml_strncpy(*prefix, target, cnt);

	q = ++p;
	while (*q && *q != '/') {
	    q++;
	}
	cnt = (uint32)(q-p);

	if (!ncx_valid_name(p, cnt)) {
	    log_error("\nError: invalid name string (%s)", 
		      target);
	    res = ERR_NCX_INVALID_NAME;
	    do_errmsg(tkc, mod, obj->tk, res);
	    if (*prefix) {
		m__free(*prefix);
		*prefix = NULL;
	    }
	    return res;
	}

	*name = m__getMem(cnt+1);
	if (!*name) {
	    log_error("\nError: malloc failed");
	    res = ERR_INTERNAL_MEM;
	    do_errmsg(tkc, mod, obj->tk, res);
	    if (*prefix) {
		m__free(*prefix);
		*prefix = NULL;
	    }
	    return res;
	}

	xml_strncpy(*name, p, cnt);
	*len = (uint32)(q-target);
    } else  {
	/* found EOS or pathsep, got just one 'name' string */
	*name = m__getMem(cnt+1);
	if (!*name) {
	    log_error("\nError: malloc failed");
	    res = ERR_INTERNAL_MEM;
	    do_errmsg(tkc, mod, obj->tk, res);
	    return res;
	}
	xml_strncpy(*name, target, cnt);
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
*    prefix == address for return buffer to store 
*          prefix portion of QName (if any)
*    name == address for return buffer to store 
*          name portion of segment
*    len == address of return byte count
*
* OUTPUTS:
*   *prefix == malloced buffer with prefix portion of QName
*   *name == malloced name name portion of QName
*   *cnt == number of bytes used in target
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    next_nodeid_noerr (const xmlChar *target,
		       xmlChar **prefix,
		       xmlChar **name,
		       uint32 *len)
{
    const xmlChar *p, *q;
    uint32         cnt;

    *prefix = NULL;
    *name = NULL;
    *len = 0;

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
	*prefix = m__getMem(cnt+1);
	if (!*prefix) {
	    return ERR_INTERNAL_MEM;
	}
	/* copy prefix, then get name portion */
	xml_strncpy(*prefix, target, cnt);

	q = ++p;
	while (*q && *q != '/') {
	    q++;
	}
	cnt = (uint32)(q-p);

	if (!ncx_valid_name(p, cnt)) {
	    if (*prefix) {
		m__free(*prefix);
		*prefix = NULL;
	    }
	    return ERR_NCX_INVALID_NAME;
	}

	*name = m__getMem(cnt+1);
	if (!*name) {
	    if (*prefix) {
		m__free(*prefix);
		*prefix = NULL;
	    }
	    return ERR_INTERNAL_MEM;
	}
	    
	xml_strncpy(*name, p, cnt);
	*len = (uint32)(q-target);
    } else  {
	/* found EOS or pathsep, got just one 'name' string */
	*name = m__getMem(cnt+1);
	if (!*name) {
	    return ERR_INTERNAL_MEM;
	}
	xml_strncpy(*name, target, cnt);
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
*    logerrors = TRUE to use log_error, FALSE to skip it
*    prefix == address for return buffer to store 
*          prefix portion of QName (if any)
*    name == address for return buffer to store 
*          name portion of segment
*    len == address of return byte count
*
* OUTPUTS:
*   *prefix == malloced buffer with prefix portion of QName
*   *name == malloced name name portion of QName
*   *cnt == number of bytes used in target
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    next_val_nodeid (const xmlChar *target,
		     boolean logerrors,
		     xmlChar **prefix,
		     xmlChar **name,
		     uint32 *len)
{
    const xmlChar *p, *q;
    uint32         cnt;

    *prefix = NULL;
    *name = NULL;
    *len = 0;

    /* find the EOS or a separator */
    p = target;
    while (*p && *p != ':' && *p != '/') {
	p++;
    }
    cnt = (uint32)(p-target);

    if (!ncx_valid_name(target, cnt)) {
	if (logerrors) {
	    log_error("\nError: invalid name string (%s)", 
		      target);
	}
	return ERR_NCX_INVALID_NAME;
    }

    if (*p==':') {
	*prefix = m__getMem(cnt+1);
	if (!*prefix) {
	    return ERR_INTERNAL_MEM;
	}
	/* copy prefix, then get name portion */
	xml_strncpy(*prefix, target, cnt);

	q = ++p;
	while (*q && *q != '/') {
	    q++;
	}
	cnt = (uint32)(q-p);

	if (!ncx_valid_name(p, cnt)) {
	    if (logerrors) {
		log_error("\nError: invalid name string (%s)", 
			  target);
	    }
	    m__free(*prefix);
	    *prefix = NULL;
	    return ERR_NCX_INVALID_NAME;
	}

	*name = m__getMem(cnt+1);
	if (!*name) {
	    if (*prefix) {
		m__free(*prefix);
		*prefix = NULL;
	    }
	    return ERR_INTERNAL_MEM;
	}
	xml_strncpy(*name, p, cnt);
	*len = (uint32)(q-target);
    } else  {
	*name = m__getMem(cnt+1);
	if (!*name) {
	    return ERR_INTERNAL_MEM;
	}
	/* found EOS or pathsep, got just one 'name' string */
	xml_strncpy(*name, target, cnt);
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
    xmlChar        *prefix;
    xmlChar        *name;

    imp = NULL;
    impmod = NULL;
    dtyp = NCX_NT_OBJ;
    curQ = NULL;
    prefix = NULL;
    name = NULL;

    /* skip the first fwd slash, if any */
    if (*target == '/') {
	str = target+1;
    } else {
	str = target;
    }

    /* get the first QName (prefix, name) */
    res = next_nodeid(tkc, mod, obj, str, &prefix, &name, &len);
    if (res != NO_ERR) {
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    } else {
	str += len;
    }

    /* get the import if there is a real prefix entered */
    if (prefix && xml_strcmp(prefix, mod->prefix)) {
	imp = ncx_find_pre_import(mod, prefix);
	if (!imp) {
	    log_error("\nError: prefix '%s' not found in module imports"
		      " in Xpath target %s", prefix, target);
	    res = ERR_NCX_INVALID_NAME;
	    do_errmsg(tkc, mod, errtk, res);
	    m__free(prefix);
	    if (name) {
		m__free(name);
	    }
	    return res;
	}
	impmod = ncx_find_module(imp->module, imp->revision);
	if (!impmod) {
	    log_error("\nError: module '%s' not found for prefix %s"
		      " in Xpath target %s",
		      imp->module, prefix, target);
	    res = ERR_NCX_MOD_NOT_FOUND;
	    do_errmsg(tkc, mod, errtk, res);
	    m__free(prefix);
	    if (name) {
		m__free(name);
	    }
	    return res;
	}
    }

    /* get the first object template */
    if (imp) {
	curobj = ncx_locate_modqual_import(imp, name, &dtyp);
    } else if (*target == '/') {
	curobj = obj_find_template_top(mod,
				       ncx_get_modname(mod),
				       name);
    } else {
	curobj = obj_find_template(datadefQ, 
				   ncx_get_modname(mod), 
				   name);
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
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    } else {
	curQ = datadefQ;
    }

    if (obj_is_augclone(curobj)) {
	res = ERR_NCX_INVALID_VALUE;
	log_error("\nError: augment is external: node '%s'"
		  " from module %s, line %u in Xpath target %s",
		  (name) ? name : NCX_EL_NONE,
		  curobj->mod->name,
		  curobj->tk->linenum, 
		  target);
	do_errmsg(tkc, mod, errtk, res);
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    }

    if (prefix) {
	m__free(prefix);
	prefix = NULL;
    }
    if (name) {
	m__free(name);
	name = NULL;
    }

    /* got the first object; keep parsing node IDs
     * until the Xpath expression is done or an error occurs
     */
    while (*str == '/') {
	str++;
	/* get the next QName (prefix, name) */
	res = next_nodeid(tkc, mod, obj, str, &prefix, &name, &len);
	if (res != NO_ERR) {
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	} else {
	    str += len;
	}

	/* make sure the prefix is valid, if present */
	if (prefix && xml_strcmp(prefix, mod->prefix)) {
	    imp = ncx_find_pre_import(mod, prefix);
	    if (!imp) {
		log_error("\nError: prefix '%s' not found in module"
			  " imports in Xpath target '%s'",
			  prefix, target);
		res = ERR_NCX_INVALID_NAME;
		do_errmsg(tkc, mod, errtk, res);
		m__free(prefix);
		if (name) {
		    m__free(name);
		}
		return res;
	    }
	} else {
	    imp = NULL;
	}

	/* make sure the name is a valid name string */
	if (name && !ncx_valid_name2(name)) {
	    log_error("\nError: object name '%s' not a valid "
		      "identifier in Xpath target '%s'",
		      name, target);
	    res = ERR_NCX_INVALID_NAME;
	    do_errmsg(tkc, mod, errtk, res);
	    if (prefix) {
		m__free(prefix);
	    }
	    m__free(name);
	    return res;
	}

	/* determine 'nextval' based on [curval, prefix, name] */
	curQ = obj_get_datadefQ(curobj);

	if (name && curQ) {
	    nextobj = obj_find_template(curQ,
					(imp) ? imp->module : 
					ncx_get_modname(mod), name);
	} else {
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    log_error("\nError: '%s' in Xpath target '%s' invalid: "
		      "%s on line %u is a %s",
		      name, target, obj_get_name(curobj),
		      curobj->tk->linenum, obj_get_typestr(curobj));
	    do_errmsg(tkc, mod, errtk, res);
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	}

	if (nextobj) {
	    curobj = nextobj;
	} else {
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    log_error("\nError: object '%s' not found in module %s",
		      name, (imp) ? imp->module : mod->name);
	    do_errmsg(tkc, mod, errtk, res);
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	}

	if (prefix) {
	    m__free(prefix);
	    prefix = NULL;
	}
	if (name) {
	    m__free(name);
	    name = NULL;
	}
    }

    if (targobj) {
	*targobj = curobj;
    }
    if (targQ) {
	*targQ = curQ;
    }

    if (prefix) {
	m__free(prefix);
    }
    if (name) {
	m__free(name);
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
    const xmlChar  *str;
    xmlChar        *prefix;
    xmlChar        *name;
    uint32          len;
    status_t        res;

    prefix = NULL;
    name = NULL;

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
    res = next_nodeid_noerr(str, &prefix, &name, &len);
    if (res != NO_ERR) {
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    } else {
	str += len;
    }

    /* get the import if there is a real prefix entered */
    if (prefix) {
	mod = (ncx_module_t *)xmlns_get_modptr
	    (xmlns_find_ns_by_prefix(prefix));
	if (!mod) {
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return ERR_NCX_INVALID_NAME;
	}
	/* get the first object template */
	curobj = obj_find_template_top(mod, 
				       ncx_get_modname(mod), 
				       name);
    } else {
	/* no prefix given, check all top-level objects */
        curobj = ncx_find_any_object(name);
    }

    /* check if first level object found */
    if (!curobj) {
	if (ncx_valid_name2(name)) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else {
	    res = ERR_NCX_INVALID_NAME;
	}
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    }

    if (prefix) {
	m__free(prefix);
	prefix = NULL;
    }
    if (name) {
	m__free(name);
	name = NULL;
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
	res = next_nodeid_noerr(str, &prefix, &name, &len);
	if (res != NO_ERR) {
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	} else {
	    str += len;
	}

	/* make sure the name is a valid name string */
	if (!name || !ncx_valid_name2(name)) {
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return ERR_NCX_INVALID_NAME;
	}

	/* determine 'nextval' based on [curval, prefix, name] */
	curQ = obj_get_datadefQ(curobj);
	if (!curQ) {
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return ERR_NCX_DEFSEG_NOT_FOUND;
	}

	/* make sure the prefix is valid, if present */
	if (prefix && name) {
	    mod = (ncx_module_t *)xmlns_get_modptr
		(xmlns_find_ns_by_prefix(prefix));
	    if (!mod) {
		m__free(prefix);
		if (name) {
		    m__free(name);
		}
		return ERR_NCX_INVALID_NAME;
	    }
	    nextobj = obj_find_template(curQ, 
					ncx_get_modname(mod), 
					name);
	} else if (name) {
	    /* no prefix given; try current module first */
	    nextobj = obj_find_template(curQ, 
					obj_get_mod_name(curobj), 
					name); 
	    if (!nextobj) {
		nextobj = obj_find_template(curQ, 
					    NULL, 
					    name); 
	    }
	} else {
	    nextobj = NULL;
	}

	if (prefix) {
	    m__free(prefix);
	    prefix = NULL;
	}
	if (name) {
	    m__free(name);
	    name = NULL;
	}

	/* setup next loop or error exit because last node not found */
	if (nextobj) {
	    curobj = nextobj;
	} else {
	    return ERR_NCX_DEFSEG_NOT_FOUND;
	}
    }

    if (prefix) {
	m__free(prefix);
    }
    if (name) {
	m__free(name);
    }
    if (targobj) {
	*targobj = curobj;
    }

    return NO_ERR;

}  /* find_schema_node_int */


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
    xmlChar        *prefix;
    xmlChar        *name;
    uint32          len;
    status_t        res;

    prefix = NULL;
    name = NULL;

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
    res = next_val_nodeid(str, TRUE, &prefix, &name, &len);
    if (res != NO_ERR) {
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    } else {
	str += len;
    }

    res = xpath_get_curmod_from_prefix(prefix, mod, &usemod);
    if (res != NO_ERR) {
	if (prefix) {
	    log_error("\nError: module not found for prefix %s"
		      " in Xpath target %s",
		      prefix, target);
	    m__free(prefix);
	} else {
	    log_error("\nError: no module prefix specified"
			  " in Xpath target %s", target);
	}
	if (name) {
	    m__free(name);
	}
	return ERR_NCX_MOD_NOT_FOUND;
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
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    }

    if (prefix) {
	m__free(prefix);
	prefix = NULL;
    }
    if (name) {
	m__free(name);
	name = NULL;
    }

    /* got the first object; keep parsing node IDs
     * until the Xpath expression is done or an error occurs
     */
    while (*str == '/') {
	str++;
	/* get the next QName (prefix, name) */
	res = next_val_nodeid(str, TRUE, &prefix, &name, &len);
	if (res != NO_ERR) {
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	} else {
	    str += len;
	}

	res = xpath_get_curmod_from_prefix(prefix, mod, &usemod);
	if (res != NO_ERR) {
	    if (prefix) {
		log_error("\nError: module not found for prefix %s"
			  " in Xpath target %s",
			  prefix, target);
		m__free(prefix);
	    } else {
		log_error("\nError: no module prefix specified"
			  " in Xpath target %s", target);
	    }
	    if (name) {
		m__free(name);
	    }
	    return ERR_NCX_MOD_NOT_FOUND;
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
		if (prefix) {
		    m__free(prefix);
		}
		if (name) {
		    m__free(name);
		}
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
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    do_errmsg(NULL, mod, NULL, res);
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	}
    }


    if (prefix) {
	m__free(prefix);
    }
    if (name) {
	m__free(name);
    }

    if (targval) {
	*targval = curval;
    }
    return NO_ERR;

}  /* find_val_node */


/********************************************************************
* FUNCTION find_val_node_unique
* 
* Follow the relative-path schema-nodeid expression
* and return the val_value_t that it indicates, if any
* Used in the YANG unique-stmt component
*
*  [/] foo/bar/baz
*
* No predicates are expected, but choice and case nodes
* are expected and will be accounted for if present
*
* XML mode - unique-smmt processing
* check first before logging errors;
*
* If logerrors:
*   Error messages are printed by this function!!
*   Do not duplicate error messages upon error return
*
* INPUTS:
*    startval == val_value_t node to start search
*    mod == module to use for the default context
*           and prefixes will be relative to this module's
*           import statements.
*        == NULL and the default registered prefixes
*           will be used
*    target == relative path expr to evaluate
*    logerrors == TRUE to log_error, FALSE to skip
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
    find_val_node_unique (val_value_t *startval,
			  ncx_module_t *mod,
			  const xmlChar *target,
			  boolean logerrors,
			  val_value_t **targval)
{
    val_value_t    *curval;
    ncx_module_t   *usemod;
    const xmlChar  *str;
    xmlChar        *prefix;
    xmlChar        *name;
    uint32          len;
    status_t        res;

    prefix = NULL;
    name = NULL;

    /* skip the first fwd slash, if any */
    if (*target == '/') {
	str = ++target;
    } else {
	str = target;
    }

    /* get the first QName (prefix, name) */
    res = next_val_nodeid(str, logerrors, &prefix, &name, &len);
    if (res != NO_ERR) {
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    } else {
	str += len;
    }

    res = xpath_get_curmod_from_prefix(prefix, mod, &usemod);
    if (res != NO_ERR) {
	if (prefix) {
	    if (logerrors) {
		log_error("\nError: module not found for prefix %s"
			  " in Xpath target %s",
			  prefix, target);
	    }
	    m__free(prefix);
	} else {
	    if (logerrors) {
		log_error("\nError: no module prefix specified"
			  " in Xpath target %s", target);
	    }
	}
	if (name) {
	    m__free(name);
	}
	return ERR_NCX_MOD_NOT_FOUND;
    }

    /* get the first value node */
    curval = val_find_child(startval, usemod->name, name);
    if (!curval) {
	if (ncx_valid_name2(name)) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else {
	    res = ERR_NCX_INVALID_NAME;
	}
	if (logerrors) {
	    log_error("\nError: value node '%s' not found for module %s"
		      " in Xpath target %s",
		      name, usemod->name, target);
	}
	if (prefix) {
	    m__free(prefix);
	}
	if (name) {
	    m__free(name);
	}
	return res;
    }

    if (prefix) {
	m__free(prefix);
	prefix = NULL;
    }
    if (name) {
	m__free(name);
	name = NULL;
    }
    
    /* got the first object; keep parsing node IDs
     * until the Xpath expression is done or an error occurs
     */
    while (*str == '/') {
	str++;
	/* get the next QName (prefix, name) */
	res = next_val_nodeid(str, logerrors, &prefix, &name, &len);
	if (res != NO_ERR) {
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	} else {
	    str += len;
	}

	res = xpath_get_curmod_from_prefix(prefix, mod, &usemod);
	if (res != NO_ERR) {
	    if (prefix) {
		if (logerrors) {
		    log_error("\nError: module not found for prefix %s"
			      " in Xpath target %s",
			      prefix, target);
		}
		m__free(prefix);
	    } else {
		if (logerrors) {
		    log_error("\nError: no module prefix specified"
			      " in Xpath target %s", target);
		}
	    }
	    if (name) {
		m__free(name);
	    }
	    return ERR_NCX_MOD_NOT_FOUND;
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
		if (logerrors) {
		    log_error("\nError: value node '%s' not found for module %s"
			      " in Xpath target %s",
			      (name) ? name : NCX_EL_NONE, 
			      usemod->name, 
			      target);
		}
		if (prefix) {
		    m__free(prefix);
		}
		if (name) {
		    m__free(name);
		}
		return res;
	    }
	    break;
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    res = ERR_NCX_DEFSEG_NOT_FOUND;
	    if (logerrors) {
		log_error("\nError: '%s' in Xpath target '%s' invalid: "
			  "%s is a %s",
			  (name) ? name : NCX_EL_NONE, 
			  target, 
			  curval->name,
			  obj_get_typestr(curval->obj));
	    }
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    if (logerrors) {
		do_errmsg(NULL, mod, NULL, res);
	    }
	    if (prefix) {
		m__free(prefix);
	    }
	    if (name) {
		m__free(name);
	    }
	    return res;
	}

	if (prefix) {
	    m__free(prefix);
	    prefix = NULL;
	}
	if (name) {
	    m__free(name);
	    name = NULL;
	}
    }

    if (prefix) {
	m__free(prefix);
    }
    if (name) {
	m__free(name);
    }

    if (targval) {
	*targval = curval;
    }
    return NO_ERR;

}  /* find_val_node_unique */


/************    E X T E R N A L   F U N C T I O N S    ************/


/*********    S C H E M A  N O D E  I D    S U P P O R T  ***********/


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
	if (obj && obj->parent && !obj_is_root(obj->parent)) {
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
* FUNCTION xpath_find_schema_target_int
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
* and return the val_value_t that it indicates
*
* Expression must be the node-path from root for
* the desired node.
*
* Error messages are logged by this function
*
* INPUTS:
*    startval == top-level start element to search
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
* FUNCTION xpath_find_val_unique
* 
* Follow the relative-path Xpath expression as used
* internally to identify a config DB node
* and return the val_value_t that it indicates
*
* Expression must be the node-path from root for
* the desired node.
*
* Error messages are logged by this function
* only if logerrors is TRUE
*
* INPUTS:
*    cfg == configuration to search
*    mod == module to use for the default context
*           and prefixes will be relative to this module's
*           import statements.
*        == NULL and the default registered prefixes
*           will be used
*    target == Xpath expression string to evaluate
*    logerrors == TRUE to use log_error, FALSE to skip it
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
    xpath_find_val_unique (val_value_t *startval,
			   ncx_module_t *mod,
			   const xmlChar *target,
			   boolean logerrors,
			   val_value_t **targval)
{

#ifdef DEBUG
    if (!startval || !target) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return find_val_node_unique(startval, mod, 
				target, logerrors, targval);

}  /* xpath_find_val_unique */


/*******    X P A T H   and   K E Y R E F    S U P P O R T   *******/


/********************************************************************
* FUNCTION xpath_new_pcb
* 
* Create and initialize an XPath parser control block
*
* INPUTS:
*   xpathstr == XPath expression string to save (a copy will be made)
*            == NULL if this step should be skipped
*
* RETURNS:
*   pointer to malloced struct, NULL if malloc error
*********************************************************************/
xpath_pcb_t *
    xpath_new_pcb (const xmlChar *xpathstr)
{
    xpath_pcb_t *pcb;

    pcb = m__getObj(xpath_pcb_t);
    if (!pcb) {
	return NULL;
    }

    memset(pcb, 0x0, sizeof(xpath_pcb_t));

    if (xpathstr) {
	pcb->exprstr = xml_strdup(xpathstr);
	if (!pcb->exprstr) {
	    m__free(pcb);
	    return NULL;
	}
    }

    ncx_init_errinfo(&pcb->errinfo);

    pcb->functions = xpath1_get_functions_ptr();

    dlq_createSQue(&pcb->result_cacheQ);
    dlq_createSQue(&pcb->resnode_cacheQ);

    return pcb;

}  /* xpath_new_pcb */


/********************************************************************
* FUNCTION xpath_clone_pcb
* 
* Clone an XPath PCB for a  must clause copy
*
* INPUTS:
*    srcpcb == struct with starting contents
*
* RETURNS:
*   new xpatyh_pcb_t clone of the srcmust, NULL if malloc error
*   It will not be processed or parsed.  Only the starter
*   data will be set
*********************************************************************/
xpath_pcb_t *
    xpath_clone_pcb (const xpath_pcb_t *srcpcb)
{
    xpath_pcb_t *newpcb;
    status_t     res;

#ifdef DEBUG
    if (!srcpcb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    newpcb = xpath_new_pcb(srcpcb->exprstr);
    if (!newpcb) {
	return NULL;
    }

    /*** clone tkc ***/
    newpcb->tk = srcpcb->tk;
    newpcb->reader = srcpcb->reader;
    newpcb->mod = srcpcb->mod;
    newpcb->source = srcpcb->source;

    res = ncx_copy_errinfo(&srcpcb->errinfo, &newpcb->errinfo);
    if (res != NO_ERR) {
	xpath_free_pcb(newpcb);
	return NULL;
    }
    newpcb->logerrors = srcpcb->logerrors;
    newpcb->targobj = srcpcb->targobj;
    newpcb->altobj = srcpcb->altobj;
    newpcb->varobj = srcpcb->varobj;
    newpcb->curmode = srcpcb->curmode;
    newpcb->obj = srcpcb->obj;
    newpcb->objmod = srcpcb->objmod;
    newpcb->docroot = srcpcb->docroot;
    newpcb->doctype = srcpcb->doctype;
    newpcb->val = srcpcb->val;
    newpcb->val_docroot = srcpcb->val_docroot;
    newpcb->flags = srcpcb->flags;
    /*** skip copying the scratch result ***/
    /*** ??? context ??? ***/
    /*** ??? varbindQ ??? ***/
    newpcb->functions = srcpcb->functions;
    /* result_cacheQ not copied */
    /* resnode_cacheQ not copied */
    /* result_count not copied */
    /* resnode_count not copied */
    newpcb->parseres = srcpcb->parseres;
    newpcb->validateres = srcpcb->validateres;
    newpcb->valueres = srcpcb->valueres;
    newpcb->errtoken = srcpcb->errtoken;
    newpcb->errpos = srcpcb->errpos;
    newpcb->seen = srcpcb->seen;

    return newpcb;

}  /* xpath_clone_pcb */


/********************************************************************
* FUNCTION xpath_find_pcb
* 
* Find an XPath PCB
*
* INPUTS:
*    pcbQ == Q of xpath_pcb_t structs to check
*    exprstr == XPath expression string to find
*
* RETURNS:
*   pointer to found xpath_pcb_t or NULL if not found
*********************************************************************/
xpath_pcb_t *
    xpath_find_pcb (dlq_hdr_t *pcbQ,
		    const xmlChar *exprstr)
{
    xpath_pcb_t *pcb;

#ifdef DEBUG
    if (!pcbQ || !exprstr) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (pcb = (xpath_pcb_t *)dlq_firstEntry(pcbQ);
	 pcb != NULL;
	 pcb = (xpath_pcb_t *)dlq_nextEntry(pcb)) {

	if (pcb->exprstr && 
	    !xml_strcmp(exprstr, pcb->exprstr)) {
	    return pcb;
	}
    }
    return NULL;

}  /* xpath_find_pcb */


/********************************************************************
* FUNCTION xpath_free_pcb
* 
* Free a malloced XPath parser control block
*
* INPUTS:
*   pcb == pointer to parser control block to free
*********************************************************************/
void
    xpath_free_pcb (xpath_pcb_t *pcb)
{
    xpath_result_t   *result;
    xpath_resnode_t  *resnode;

#ifdef DEBUG
    if (!pcb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (pcb->tkc) {
	tk_free_chain(pcb->tkc);
    }

    if (pcb->exprstr) {
	m__free(pcb->exprstr);
    }

    if (pcb->result) {
	xpath_free_result(pcb->result);
    }

    ncx_clean_errinfo(&pcb->errinfo);

    while (!dlq_empty(&pcb->result_cacheQ)) {
	result = (xpath_result_t *)
	    dlq_deque(&pcb->result_cacheQ);
	xpath_free_result(result);
    }

    while (!dlq_empty(&pcb->resnode_cacheQ)) {
	resnode = (xpath_resnode_t *)
	    dlq_deque(&pcb->resnode_cacheQ);
	xpath_free_resnode(resnode);
    }


    m__free(pcb);

}  /* xpath_free_pcb */


/********************************************************************
* FUNCTION xpath_new_result
* 
* Create and initialize an XPath result struct
*
* INPUTS:
*   restype == the desired result type
*
* RETURNS:
*   pointer to malloced struct, NULL if malloc error
*********************************************************************/
xpath_result_t *
    xpath_new_result (xpath_restype_t  restype)
{
    xpath_result_t *result;

    result = m__getObj(xpath_result_t);
    if (!result) {
	return NULL;
    }
    
    xpath_init_result(result, restype);
    return result;

}  /* xpath_new_result */


/********************************************************************
* FUNCTION xpath_init_result
* 
* Initialize an XPath result struct
*
* INPUTS:
*   result == pointer to result struct to initialize
*   restype == the desired result type
*********************************************************************/
void 
    xpath_init_result (xpath_result_t *result,
		       xpath_restype_t  restype)
{
#ifdef DEBUG
    if (!result) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(result, 0x0, sizeof(xpath_result_t));
    result->restype = restype;

    switch (restype) {
    case XP_RT_NODESET:
	dlq_createSQue(&result->r.nodeQ);
	break;
    case XP_RT_NUMBER:
	ncx_init_num(&result->r.num);
	ncx_set_num_zero(&result->r.num, NCX_BT_FLOAT64);
	break;
    case XP_RT_STRING:
    case XP_RT_BOOLEAN:
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* xpath_init_result */


/********************************************************************
* FUNCTION xpath_free_result
* 
* Free a malloced XPath result struct
*
* INPUTS:
*   result == pointer to result struct to free
*********************************************************************/
void
    xpath_free_result (xpath_result_t *result)
{
#ifdef DEBUG
    if (!result) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    xpath_clean_result(result);
    m__free(result);

}  /* xpath_free_result */


/********************************************************************
* FUNCTION xpath_clean_result
* 
* Clean an XPath result struct
*
* INPUTS:
*   result == pointer to result struct to clean
*********************************************************************/
void
    xpath_clean_result (xpath_result_t *result)
{
    xpath_resnode_t *resnode;

#ifdef DEBUG
    if (!result) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    switch (result->restype) {
    case XP_RT_NODESET:
	while (!dlq_empty(&result->r.nodeQ)) {
	    resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);
	    xpath_free_resnode(resnode);
	}
	break;
    case XP_RT_NUMBER:
	ncx_clean_num(NCX_BT_FLOAT64, &result->r.num);
	break;
    case XP_RT_STRING:
	if (result->r.str) {
	    m__free(result->r.str);
	    result->r.str = NULL;
	}
	break;
    case XP_RT_BOOLEAN:
	result->r.bool = FALSE;
	break;
    case XP_RT_NONE:
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    result->restype = XP_RT_NONE;
    result->res = NO_ERR;

}  /* xpath_clean_result */


/********************************************************************
* FUNCTION xpath_new_resnode
* 
* Create and initialize an XPath result node struct
*
* INPUTS:
*   restype == the desired result type
*
* RETURNS:
*   pointer to malloced struct, NULL if malloc error
*********************************************************************/
xpath_resnode_t *
    xpath_new_resnode (void)
{
    xpath_resnode_t *resnode;

    resnode = m__getObj(xpath_resnode_t);
    if (!resnode) {
	return NULL;
    }
    
    xpath_init_resnode(resnode);
    return resnode;

}  /* xpath_new_resnode */


/********************************************************************
* FUNCTION xpath_init_resnode
* 
* Initialize an XPath result node struct
*
* INPUTS:
*   resnode == pointer to result node struct to initialize
*********************************************************************/
void 
    xpath_init_resnode (xpath_resnode_t *resnode)
{
#ifdef DEBUG
    if (!resnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(resnode, 0x0, sizeof(xpath_resnode_t));

}  /* xpath_init_resnode */


/********************************************************************
* FUNCTION xpath_free_resnode
* 
* Free a malloced XPath result node struct
*
* INPUTS:
*   resnode == pointer to result node struct to free
*********************************************************************/
void
    xpath_free_resnode (xpath_resnode_t *resnode)
{
#ifdef DEBUG
    if (!resnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    xpath_clean_resnode(resnode);
    m__free(resnode);

}  /* xpath_free_resnode */


/********************************************************************
* FUNCTION xpath_clean_resnode
* 
* Clean an XPath result node struct
*
* INPUTS:
*   resnode == pointer to result node struct to clean
*********************************************************************/
void
    xpath_clean_resnode (xpath_resnode_t *resnode)
{

#ifdef DEBUG
    if (!resnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(resnode, 0x0, sizeof(xpath_resnode_t));

}  /* xpath_clean_resnode */


/********************************************************************
* FUNCTION xpath_get_curmod_from_prefix
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
status_t
    xpath_get_curmod_from_prefix (const xmlChar *prefix,
				  ncx_module_t *mod,
				  ncx_module_t **targmod)
{
    ncx_import_t   *imp;
    status_t        res;

#ifdef DEBUG
    if (!mod || !targmod) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    /* get the import if there is a real prefix entered */
    if (prefix && *prefix) {
	if (!mod) {
	    *targmod = (ncx_module_t *)xmlns_get_modptr
		(xmlns_find_ns_by_prefix(prefix));
	    if (!*targmod) {
		res = ERR_NCX_MOD_NOT_FOUND;
	    }
	} else if (xml_strcmp(prefix, mod->prefix)) {
	    imp = ncx_find_pre_import(mod, prefix);
	    if (!imp) {
		res = ERR_NCX_INVALID_NAME;
	    } else {
		*targmod = ncx_find_module(imp->module, 
					   imp->revision);
		if (!*targmod) {
		    res = ERR_NCX_MOD_NOT_FOUND;
		}
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

}   /* xpath_get_curmod_from_prefix */


/********************************************************************
* FUNCTION xpath_get_curmod_from_prefix_str
* 
* Get the correct module to use for a given prefix
* Unended string version
*
* INPUTS:
*    prefix == string to check
*    prefixlen == length of prefix
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
status_t
    xpath_get_curmod_from_prefix_str (const xmlChar *prefix,
				      uint32 prefixlen,
				      ncx_module_t *mod,
				      ncx_module_t **targmod)
{
    xmlChar        *buff;
    status_t        res;

#ifdef DEBUG
    if (!mod || !targmod) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (prefix && prefixlen) {
	buff = m__getMem(prefixlen+1);
	if (!buff) {
	    return ERR_INTERNAL_MEM;
	}
	xml_strncpy(buff, prefix, prefixlen);

	res = xpath_get_curmod_from_prefix(buff, mod, targmod);
	
	m__free(buff);

	return res;
    } else {
	return xpath_get_curmod_from_prefix(NULL, mod, targmod);
    }
    /*NOTREACHED*/

}   /* xpath_get_curmod_from_prefix_str */


/********************************************************************
* FUNCTION xpath_parse_token
* 
* Parse the XPath token sequence for a specific token type
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*    tktyp == expected token type
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath_parse_token (xpath_pcb_t *pcb,
		       tk_type_t  tktype)
{
    status_t     res;

#ifdef DEBUG
    if (!pcb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* get the next token */
    res = TK_ADV(pcb->tkc);
    if (res != NO_ERR) {
	if (pcb->logerrors) {
	    ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	}
	return res;
    }

    if (TK_CUR_TYP(pcb->tkc) != tktype) {
	res = ERR_NCX_WRONG_TKTYPE;
	if (pcb->logerrors) {
	    ncx_mod_exp_err(pcb->tkc, pcb->mod, res,
			    tk_get_token_name(tktype));
	}
	return res;
    }

    return NO_ERR;

}  /* xpath_parse_token */


/********************************************************************
* FUNCTION xpath_cvt_boolean
* 
* Convert an XPath result to a boolean answer
*
* INPUTS:
*    result == result struct to convert to boolean
*
* RETURNS:
*   TRUE or FALSE depending on conversion
*********************************************************************/
boolean
    xpath_cvt_boolean (const xpath_result_t *result)
{
#ifdef DEBUG
    if (!result) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    switch (result->restype) {
    case XP_RT_NONE:
	return FALSE;
    case XP_RT_NODESET:
	return (dlq_empty(&result->r.nodeQ)) ? FALSE : TRUE;
    case XP_RT_NUMBER:
	return (ncx_num_zero(&result->r.num, NCX_BT_FLOAT64)) ?
	    FALSE : TRUE;
    case XP_RT_STRING:
	return (result->r.str && xml_strlen(result->r.str)) ?
	    TRUE : FALSE;
    case XP_RT_BOOLEAN:
	return result->r.bool;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
    /*NOTREACHED*/

}  /* xpath_cvt_boolean */


/********************************************************************
* FUNCTION xpath_cvt_number
* 
* Convert an XPath result to a number answer
*
* INPUTS:
*    result == result struct to convert to a number
*    num == pointer to ncx_num_t to hold the conversion result
*
* OUTPUTS:
*   *num == numeric result from conversion
*
*********************************************************************/
void
    xpath_cvt_number (const xpath_result_t *result,
		      ncx_num_t *num)
{
    const xpath_resnode_t   *resnode;
    val_value_t             *val;
    status_t                 res;
    ncx_num_t                testnum;
    ncx_numfmt_t             numformat;

#ifdef DEBUG
    if (!result || !num) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    res = NO_ERR;

    switch (result->restype) {
    case XP_RT_NONE:
	ncx_set_num_nan(num, NCX_BT_FLOAT64);
	break;
    case XP_RT_NODESET:
	if (dlq_empty(&result->r.nodeQ)) {
	    ncx_set_num_nan(num, NCX_BT_FLOAT64);
	} else {
	    if (result->isval) {
		resnode = (const xpath_resnode_t *)
		    dlq_firstEntry(&result->r.nodeQ);
		val = val_get_first_leaf(resnode->node.valptr);
		if (val && typ_is_number(val->btyp)) {
		    res = ncx_cast_num(&val->v.num,
				       val->btyp,
				       num,
				       NCX_BT_FLOAT64);
		    if (res != NO_ERR) {
			ncx_set_num_nan(num, NCX_BT_FLOAT64);
		    }			
		} else {
		    ncx_set_num_nan(num, NCX_BT_FLOAT64);
		}
	    } else {
		/* does not matter */
		ncx_set_num_zero(num, NCX_BT_FLOAT64);
	    }
	}
	break;
    case XP_RT_NUMBER:
	ncx_copy_num(&result->r.num, num, NCX_BT_FLOAT64);
	break;
    case XP_RT_STRING:
	if (result->r.str) {
	    numformat = ncx_get_numfmt(result->r.str);
	    switch (numformat) {
	    case NCX_NF_DEC:
	    case NCX_NF_REAL:
		break;
	    case NCX_NF_NONE:
	    case NCX_NF_OCTAL:
	    case NCX_NF_HEX:
		res = ERR_NCX_WRONG_NUMTYP;
		break;
	    default:
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }

	    if (res == NO_ERR) {
		ncx_init_num(&testnum);
		res = ncx_convert_num(result->r.str,
				      numformat,
				      NCX_BT_FLOAT64,
				      &testnum);
		if (res == NO_ERR) {
		    (void)ncx_copy_num(&testnum, num, NCX_BT_FLOAT64);
		} else {
		    ncx_set_num_nan(num, NCX_BT_FLOAT64);
		}
		ncx_clean_num(NCX_BT_FLOAT64, &testnum);
	    } else {
		ncx_set_num_nan(num, NCX_BT_FLOAT64);
	    }
	} else {
	    ncx_set_num_nan(num, NCX_BT_FLOAT64);
	}
	break;
    case XP_RT_BOOLEAN:
	if (result->r.bool) {
	    ncx_set_num_one(num, NCX_BT_FLOAT64);
	} else {
	    ncx_set_num_zero(num, NCX_BT_FLOAT64);
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* xpath_cvt_number */


/********************************************************************
* FUNCTION xpath_cvt_string
* 
* Convert an XPath result to a string answer
*
* INPUTS:
*    pcb == parser control block to use
*    result == result struct to convert to a number
*    str == pointer to xmlChar * to hold the conversion result
*
* OUTPUTS:
*   *str == pointer to malloced string from conversion
*
* RETURNS:
*   status; could get an ERR_INTERNAL_MEM error or NO_RER
*********************************************************************/
status_t
    xpath_cvt_string (xpath_pcb_t *pcb,
		      const xpath_result_t *result,
		      xmlChar **str)
{
    status_t                 res;
    uint32                   len;

#ifdef DEBUG
    if (!result || !str) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    *str = NULL;

    switch (result->restype) {
    case XP_RT_NONE:
	*str = xml_strdup(EMPTY_STRING);
	break;
    case XP_RT_NODESET:
	if (dlq_empty(&result->r.nodeQ)) {
	    *str = xml_strdup(EMPTY_STRING);
	} else {
	    if (result->isval) {
		res = xpath1_stringify_nodeset(pcb, result, str);
	    } else {
		*str = xml_strdup(EMPTY_STRING);
	    }
	}
	break;
    case XP_RT_NUMBER:
	res = ncx_sprintf_num(NULL, 
			      &result->r.num,
			      NCX_BT_FLOAT64,
			      &len);
	if (res != NO_ERR) {
	    return res;
	}

	*str = m__getMem(len+1);
	if (*str) {
	    res = ncx_sprintf_num(*str,
				  &result->r.num,
				  NCX_BT_FLOAT64,
				  &len);
	    if (res != NO_ERR) {
		m__free(*str);
		*str = NULL;
		return res;
	    }
	}
	break;
    case XP_RT_STRING:
	if (result->r.str) {
	    *str = xml_strdup(result->r.str);
	} else {
	    *str = xml_strdup(EMPTY_STRING);
	}
	break;
    case XP_RT_BOOLEAN:
	if (result->r.bool) {
	    *str = xml_strdup(NCX_EL_TRUE);
	} else {
	    *str = xml_strdup(NCX_EL_FALSE);
	}
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (!*str) {
	res = ERR_INTERNAL_MEM;
    }
    return res;

}  /* xpath_cvt_string */


/********************************************************************
* FUNCTION xpath_get_resnodeQ
* 
* Get the renodeQ from a result struct
*
* INPUTS:
*    result == result struct to check
*
* RETURNS:
*   pointer to resnodeQ or NULL if some error
*********************************************************************/
dlq_hdr_t *
    xpath_get_resnodeQ (xpath_result_t *result)
{
#ifdef DEBUG
    if (!result) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (result->restype != XP_RT_NODESET) {
	return NULL;
    }
    return &result->r.nodeQ;

}  /* xpath_get_resnodeQ */



/* END xpath.c */
