/*  FILE: var.c

    User variable utilities

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
23-aug-07    abb      begun


*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef _H_procdefs
#include "procdefs.h"
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

#ifndef _H_runstack
#include "runstack.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_var
#include "var.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define VAR_DEBUG   1
#endif

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION new_var
* 
* Malloc and fill in a new var struct
*
* INPUTS:
*    val == variable value
*    isglobal == TRUE if global var
*             == FALSE if runstack var
*    res == address of status result if returning NULL
*
* OUTPUTS:
*    *res == status
*
* RETURNS:
*    malloced struct with direct 'val' string added
*    to be freed later
*********************************************************************/
static ncx_var_t *
    new_var (val_value_t *val,
	     boolean isglobal,
	     boolean issystem,
	     status_t *res)
{
    ncx_var_t  *var;

    var = m__getObj(ncx_var_t);
    if (!var) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    memset(var, 0x0, sizeof(ncx_var_t));

    var->val = val;
    if (isglobal) {
	var->flags |= VAR_FL_GLOBAL;
    }
    if (issystem) {
	var->flags |= VAR_FL_SYSTEM;
    }

    *res = NO_ERR;
    return var;
	
} /* new_var */


/********************************************************************
* FUNCTION free_var
* 
* Free a previously malloced user var struct
*
* INPUTS:
*    var == user var struct to free
*********************************************************************/
static void
    free_var (ncx_var_t *var)
{
    if (var->val) {
	val_free_value(var->val);
    }
    m__free(var);

} /* free_var */


/********************************************************************
* FUNCTION get_que
* 
* Get the correct queue for the ncx_var_t struct
*
* INPUTS:
*    isglobal == TRUE for global var
*    name == name string (only checks first char
*            does not have to be zero terminated!!
*
* RETURNS:
*   correct que header or NULL if internal error
*********************************************************************/
static dlq_hdr_t *
    get_que (boolean isglobal,
	     const xmlChar *name)
{
    if (isdigit((int)*name)) {
	return runstack_get_parm_que();
    } else {
	return runstack_get_que(isglobal);
    }
} /* get_que */


/********************************************************************
* FUNCTION var_insert
* 
* Insert a user var 
* 
* INPUTS:
*   var == var to insert
*    que == queue header to contain the var struct
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    insert_var (ncx_var_t *var,
		dlq_hdr_t *que)
{
    ncx_var_t  *cur;
    int         ret;

    for (cur = (ncx_var_t *)dlq_firstEntry(que);
	 cur != NULL;
	 cur = (ncx_var_t *)dlq_nextEntry(cur)) {
	ret = xml_strcmp(var->val->name, cur->val->name);
	if (ret < 0) {
	    dlq_insertAhead(var, cur);
	    return NO_ERR;
	} else if (ret == 0) {
	    return SET_ERROR(ERR_NCX_DUP_ENTRY);
	} /* else keep going */
    }

    /* if we get here, then new first entry */
    dlq_enque(var, que);
    return NO_ERR;

}  /* insert_var */


/********************************************************************
* FUNCTION remove_var
* 
* Remove a user var 
* 
* INPUTS:
*   name == var name to remove
*   namelen == length of name
*   isglobal == TRUE if global var, FALSE if local var
*
* RETURNS:
*   found var struct or NULL if not found
*********************************************************************/
static ncx_var_t *
    remove_var (const xmlChar *name,
		uint32 namelen,
		boolean isglobal)
{
    dlq_hdr_t  *que;
    ncx_var_t  *cur;
    int         ret;

    que = get_que(isglobal, name);
    if (!que) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    for (cur = (ncx_var_t *)dlq_firstEntry(que);
	 cur != NULL;
	 cur = (ncx_var_t *)dlq_nextEntry(cur)) {
	ret = xml_strncmp(name, cur->val->name, namelen);
	if (ret == 0 && xml_strlen(cur->val->name)==namelen) {
	    dlq_remove(cur);
	    return cur;
	} /* else keep going */
    }

    return NULL;

}  /* remove_var */


/********************************************************************
* FUNCTION find_var
* 
* Find a user var 
* 
* INPUTS:
*   name == var name to find
*   namelen == name length
*   isglobal = TRUE if global var, FALSE if local
*
* RETURNS:
*   found var struct or NULL if not found
*********************************************************************/
static ncx_var_t *
    find_var (const xmlChar *name,
	      uint32  namelen,
	      boolean isglobal)
{
    dlq_hdr_t  *que;
    ncx_var_t  *cur;
    int         ret;

    que = get_que(isglobal, name);
    if (!que) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    for (cur = (ncx_var_t *)dlq_firstEntry(que);
	 cur != NULL;
	 cur = (ncx_var_t *)dlq_nextEntry(cur)) {
	ret = xml_strncmp(name, cur->val->name, namelen);
	if (ret == 0 && xml_strlen(cur->val->name)==namelen) {
	    return cur;
	} /* else keep going */
    }

    return NULL;

}  /* find_var */


/********************************************************************
* FUNCTION set_str
* 
* Find and set (or create a new) global user variable
* Common portions only!!!
*
* Force caller to deallocate var if there is an error
*
* INPUTS:
*   name == var name to set
*   namelen == length of name
*   val == var value to set
*   isglobal == TRUE for global var, 
*   issystem == TRUE for global system var, 
* 
* RETURNS:
*   status
*********************************************************************/
static status_t
    set_str (const xmlChar *name,
	     uint32 namelen,
	     val_value_t *val,
	     boolean isglobal,
	     boolean issystem)
{
    ncx_var_t    *var;
    dlq_hdr_t    *que;
    status_t      res;

    res = NO_ERR;

    que = get_que(isglobal, name);
    if (!que) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    val_set_name(val, name, namelen);

    /* try to find this var */
    var = find_var(name, xml_strlen(name), isglobal);

    if (var) {
	if (var->flags & VAR_FL_SYSTEM) {
	    /* found system var, replace only if same type */
	    if (var->val->typdef != val->typdef) {
		return ERR_NCX_WRONG_TYPE;
	    }
	}
	val_free_value(var->val);
	var->val = val;
    } else {
	/* create a new value */
	var = new_var(val, isglobal, issystem, &res);
	if (!var || res != NO_ERR) {
	    return res;
	}
	res = insert_var(var, que);
	if (res != NO_ERR) {
	    var->val = NULL;
	    free_var(var);
	}
    }
    return res;

}  /* set_str */



/*************   E X T E R N A L   F U N C T I O N S   ************/


/********************************************************************
* FUNCTION var_set_str
* 
* Find and set (or create a new) global user variable
* 
* INPUTS:
*   name == var name to set
*   namelen == length of name
*   value == var value to set
*   isglobal == TRUE for global var, 
* 
* RETURNS:
*   status
*********************************************************************/
status_t
    var_set_str (const xmlChar *name,
		 uint32 namelen,
		 const val_value_t *value,
		 boolean isglobal)
{
    val_value_t  *val;
    status_t      res;

#ifdef DEBUG
    if (!name || !value) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!namelen) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    val = val_clone(value);
    if (!val) {
	return ERR_INTERNAL_MEM;
    }

    res = set_str(name, namelen, val, isglobal, FALSE);
    if (res != NO_ERR) {
	val_free_value(val);
    }
    return res;

}  /* var_set_str */


/********************************************************************
* FUNCTION var_set
* 
* Find and set (or create a new) global user variable
* 
* INPUTS:
*   name == var name to set
*   value == var value to set
*   isglobal == TRUE if global var
* 
* RETURNS:
*   status
*********************************************************************/
status_t
    var_set (const xmlChar *name,
	     const val_value_t *value,
	     boolean isglobal)
{
#ifdef DEBUG
    if (!name) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return var_set_str(name, xml_strlen(name),
		       value, isglobal);

}  /* var_set */


/********************************************************************
* FUNCTION var_free
* 
* Free a ncx_var_t struct
* 
* INPUTS:
*   var == var struct to free
* 
*********************************************************************/
void
    var_free (ncx_var_t *var)
{
#ifdef DEBUG
    if (!var) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (var->val) {
	val_free_value(var->val);
    }
    m__free(var);

}  /* var_free */


/********************************************************************
* FUNCTION var_set_move
* 
* Find and set (or create a new) global user variable
* Use the provided entry which will be freed later
* This function will not clone the value like var_set
*
* INPUTS:
*   name == var name to set
*   namelen == length of name string
*   isglobal == TRUE if global var
*   value == var value to set
* 
* RETURNS:
*   status
*********************************************************************/
status_t
    var_set_move (const xmlChar *name,
		  uint32 namelen,
		  boolean isglobal,
		  val_value_t *value)
{
#ifdef DEBUG
    if (!name || !value) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!namelen) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    return set_str(name, namelen, value, isglobal, FALSE);

}  /* var_set_move */


/********************************************************************
* FUNCTION var_set_sys
* 
* Find and set (or create a new) global system variable
* 
* INPUTS:
*   name == var name to set
*   value == var value to set
* 
* RETURNS:
*   status
*********************************************************************/
status_t
    var_set_sys (const xmlChar *name,
		 const val_value_t *value)
{
    val_value_t  *val;
    status_t      res;

#ifdef DEBUG
    if (!name || !value) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    val = val_clone(value);
    if (!val) {
	return ERR_INTERNAL_MEM;
    }

    res = set_str(name, xml_strlen(name), val, TRUE, TRUE);
    if (res != NO_ERR) {
	val_free_value(val);
    }
    return res;

}  /* var_set_sys */


/********************************************************************
* FUNCTION var_get_str
* 
* Find a global user variable
* 
* INPUTS:
*   name == var name to get
*   namelen == length of name
*   isglobal == TRUE for global var, 
*
* RETURNS:
*   pointer to value, or NULL if not found
*********************************************************************/
const val_value_t *
    var_get_str (const xmlChar *name,
		 uint32 namelen,
		 boolean isglobal)
{
    ncx_var_t    *var;


#ifdef DEBUG
    if (!name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (!namelen) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    var = find_var(name, namelen, isglobal);
    if (var) {
	return var->val;
    } else {
	return NULL;
    }

}  /* var_get_str */


/********************************************************************
* FUNCTION var_get
* 
* Find a local or global user variable
* 
* INPUTS:
*   name == var name to get
*   isglobal == TRUE for global var, 
* 
* RETURNS:
*   pointer to value, or NULL if not found
*********************************************************************/
const val_value_t *
    var_get (const xmlChar *name,
	     boolean isglobal)
{
    const ncx_var_t  *var;

#ifdef DEBUG
    if (!name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    var = find_var(name, xml_strlen(name), isglobal);
    if (var) {
	return var->val;
    } else {
	return NULL;
    }

}  /* var_get */


/********************************************************************
* FUNCTION var_unset
* 
* Find and remove a local user variable
* 
* INPUTS:
*   name == var name to unset
*   namelen == length of name string
*   isglobal == TRUE is global var, FALSE if local var
* 
*********************************************************************/
void
    var_unset (const xmlChar *name,
	       uint32 namelen,
	       boolean isglobal)
{

    ncx_var_t *var;

#ifdef DEBUG
    if (!name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    if (!namelen) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    var = remove_var(name, namelen, isglobal);
    if (var) {
	free_var(var);
    } else {
	log_error("\nunset: Variable %s not found", name);
    }

}  /* var_unset */


/********************************************************************
* FUNCTION var_check_ref
* 
* Check if the immediate command sub-string is a variable
* reference.  If so, return the (isglobal, name, namelen)
* tuple that identifies the reference.  Also return
* the total number of chars consumed from the input line.
* 
* E.g.,
*
*   $foo = get-config filter=@filter.xml
*
* INPUTS:
*   line == command line string to expand
*   isleft == TRUE if left hand side of an expression
*          == FALSE if right hand side ($1 type vars allowed)
*   len  == address of number chars parsed so far in line
*   isglobal == address of global/local flag
*   name == address of string start return val
*   namelen == address of name length return val
*
* OUTPUTS:
*   *len == number chars consumed by this function
*   *isglobal == TRUE=global/FALSE=local
*   *name == start of name string
*   *namelen == length of *name string
*
* RETURNS:
*    status   
*********************************************************************/
status_t
    var_check_ref (const xmlChar *line,
		   var_side_t side,
		   uint32   *len,
		   boolean *isglobal,
		   const xmlChar **name,
		   uint32 *namelen)
{
    const xmlChar  *str;
    int             num;
    status_t        res;

#ifdef DEBUG
    if (!line || !len || !isglobal || !name || !namelen) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* save start point in line */
    str = line;

    /* skip leading whitespace */
    while (*str && isspace(*str)) {
	str++;
    }

    /* check if this is not the start var char */
    if (*str != NCX_VAR_CH) {
	*len = 0;
	return NO_ERR;
    }

    /* is a var, check $$global or $local variable */
    if (str[1] == NCX_VAR_CH) {
	*isglobal = TRUE;
	str += 2;
    } else {
	*isglobal = FALSE;
	str++;
    }

    /* check if this is a number variable reference */
    if (isdigit((int)*str)) {
	if (side==ISLEFT || *isglobal) {
	    *len = 0;
	    return ERR_NCX_INVALID_VALUE;
	}
	num = atoi((const char *)str);
	if (num < 0 || num > RUNSTACK_MAX_PARMS) {
	    *len = 0;
	    return ERR_NCX_INVALID_VALUE;
	}
	*namelen = 1;
    } else {
	/* parse the variable name */
	res = ncx_parse_name(str, namelen);
	if (res != NO_ERR) {
	    *len = 0;
	    return res;
	}
    }

    /* return the name string */
    *name = str;
    str += *namelen;
    *len = str - line;
    return NO_ERR;

}  /* var_check_ref */


/********************************************************************
* FUNCTION var_get_script_val
* 
* Create or fill in a val_value_t struct for a parameter assignment
* within the script processing mode
*
* See ncxcli.c for details on the script syntax
*
* INPUTS:
*   typdef == expected type template 
*          == NULL and will be set to NCX_BT_STRING for simple
*             and NCX_BT_ANY for complex types
*   val == value to fill in 
*          == NULL to create a new one
*   nsid == namespace ID of the 'val', can be zero
*   varname == name of the 'val', can be NULL
*   strval == string value to check
*   istop == TRUE if calling from top level assignment
*            An unquoted string is the start of a command
*         == FALSE if calling from a parameter parse
*            An unquoted string is just a string
*   res == address of status result
*
* OUTPUTS:
*   *res == status
*
* RETURNS:
*   If error, then returns NULL;
*   If no error, then returns pointer to new val or filled in 'val'
*********************************************************************/
val_value_t *
    var_get_script_val (const typ_def_t *typdef,
			val_value_t *val,
			xmlns_id_t  nsid,
			const xmlChar *varname,
			const xmlChar *strval,
			boolean istop,
			status_t *res)
{
    const typ_def_t    *usetypdef;
    const val_value_t  *varval;
    const xmlChar      *str, *name;
    val_value_t        *newval, *useval;

    xmlChar            *fname, *intbuff;
    uint32              namelen, len;
    boolean             isglobal;

#ifdef DEBUG
    if (!res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    newval = NULL;
    useval = NULL;
    usetypdef = NULL;

    *res = NO_ERR;

    /* get a new value struct if one is not provided */
    if (val) {
	useval = val;
    } else {
	newval = val_new_value();
	if (!newval) {
	    *res = ERR_INTERNAL_MEM;
	    return NULL;
	}
	useval = newval;
    }

    /* set the typdef to 'string' if none is provided */
    usetypdef = typdef;
    if (!usetypdef) {
	if (useval->typdef) {
	    usetypdef = useval->typdef;
	} else {
	    usetypdef = typ_get_basetype_typdef(NCX_BT_STRING);
	}
    }

    if (!usetypdef) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    /* check if strval is NULL */
    if (!strval) {
	if (typ_get_basetype(usetypdef) == NCX_BT_EMPTY) {
	    *res = val_set_simval(useval, usetypdef, 0, NULL, NULL);
	} else {
	    *res = ERR_NCX_EMPTY_VAL;
	}
    } else if (*strval==NCX_AT_CH) {
	/* this is a NCX_BT_EXTERNAL value
	 * find the file with the raw XML data
	 */
	fname = ncxmod_find_data_file(&strval[1], TRUE, res);
	if (fname) {
	    /* hand off the malloced 'fname' to be freed later */
	    val_set_extern(useval, fname);
	    
	} /* else res already set */
    } else if (*strval == NCX_VAR_CH) {
	/* this is a variable reference
	 * get the value and clone it for the new value
	 * flag an error if variable not found
	 */
	*res = var_check_ref(strval, ISRIGHT, &len, &isglobal, 
			     &name, &namelen);
	if (*res == NO_ERR) {
	    varval = var_get_str(name, namelen, isglobal);
	    if (!varval) {
		*res = ERR_NCX_DEF_NOT_FOUND;
	    } else {
		*res = val_replace(varval, useval);
	    }
	}
    } else if (*strval == NCX_QUOTE_CH) {
	/* this is a quoted string literal */
	/* set the start after quote */
	str = ++strval;

	/* find the end of the quoted string */
	while (*str && *str != NCX_QUOTE_CH) {
	    str++;
	}
	*res = val_set_string2(useval, NULL, usetypdef, 
			       strval, (uint32)(str-strval)); 
    } else if ((*strval == NCX_XML1a_CH) &&
	       (strval[1] == NCX_XML1b_CH)) {

	/* this is a bracketed inline XML sequence */
	str = strval+2;
			    
	/* find the end of the inline XML */
	while (*str && 
	       !((*str==NCX_XML2a_CH) && (str[1]==NCX_XML2b_CH))) {
	    str++;
	}
	intbuff = xml_strndup(strval+1, (uint32)(str-strval));
	if (!intbuff) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    val_set_intern(useval, intbuff);
	}
    } else if (istop && ncx_valid_fname_ch(*strval)) {
	/* this is a regular string, treated as a function
	 * call at the top level
	 */
	*res = NO_ERR;
	return NULL;
    } else {
	/* this is a regular string, treated as a string
	 * when used within an RPC function  parameter
	 */
	*res = val_set_simval(useval, usetypdef, 0, NULL, strval);
    }

    if (*res != NO_ERR) {
	if (newval) {
	    val_free_value(newval);
	}
	return NULL;
    } else {
	if (varname) {
	    val_set_qname(useval, nsid, varname, xml_strlen(varname));
	} else {
	    useval->nsid = nsid;
	}
	return useval;
    }
    /*NOTREACHED*/

}  /* var_get_script_val */


/********************************************************************
* FUNCTION var_set_from_string
* 
* Find and set (or create a new) global user variable
* from a string value instead of a val_value_t struct
*
* INPUTS:
*   name == var name to set
*   valstr == value string to set
*   isglobal == TRUE for global var, 
* 
* RETURNS:
*   status
*********************************************************************/
status_t
    var_set_from_string (const xmlChar *name,
			 const xmlChar *valstr,
			 boolean isglobal)
{
    val_value_t  *val;
    status_t      res;

#ifdef DEBUG
    if (!name || !valstr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* create a value struct to store */
    val = val_new_value();
    if (!val) {
	return ERR_INTERNAL_MEM;
    }
    
    /* create a string value */
    res = val_set_string(val, name, valstr);
    if (res != NO_ERR) {
	val_free_value(val);
	return res;
    }

    /* save the variable */
    res = set_str(name, xml_strlen(name), val, isglobal, FALSE);
    if (res != NO_ERR) {
	val_free_value(val);
    }

    return res;

}  /* var_set_from_string */


/* END var.c */
