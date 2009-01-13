/*  FILE: val.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
19dec05      abb      begun
21jul08      abb      start obj-based rewrite

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>
#include  <string.h>
#include  <ctype.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_b64
#include "b64.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_getcb
#include "getcb.h"
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

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_val_util
#include "val_util.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define VAL_DEBUG  1 */

/********************************************************************
*                                                                   *
*                          T Y P E S                                *
*                                                                   *
*********************************************************************/

typedef struct finderparms_t_ {
    val_value_t  *findval;
    val_value_t  *foundval;
    int64         findpos;
    int64         foundpos;
} finderparms_t;

/* pick a log output function for dump_value */
typedef void (*dumpfn_t) (const char *fstr, ...);

/* pick a log indent function for dump_value */
typedef void (*indentfn_t) (int32 indentcnt);


/********************************************************************
* FUNCTION dump_num
* 
* Printf the specified ncx_num_t 
*
* INPUTS:
*    btyp == base type of the number
*    num == number to printf
*
*********************************************************************/
static void
    dump_num (ncx_btype_t btyp,
	      const ncx_num_t *num)
{
    /* dump the value, depending on the base type */
    switch (btyp) {
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
	SET_ERROR(ERR_INTERNAL_VAL);
    }
} /* dump_num */


/********************************************************************
* FUNCTION stdout_num
* 
* Printf the specified ncx_num_t to stdout
*
* INPUTS:
*    btyp == base type of the number
*    num == number to printf
*
*********************************************************************/
static void
    stdout_num (ncx_btype_t btyp,
		const ncx_num_t *num)
{
    /* dump the value, depending on the base type */
    switch (btyp) {
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
	log_stdout("%d", num->i);
	break;
    case NCX_BT_INT64:
	log_stdout("%lld", num->l);
	break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
	log_stdout("%u", num->u);
	break;
    case NCX_BT_UINT64:
	log_stdout("%llu", num->ul);
	break;
    case NCX_BT_FLOAT32:
#ifdef HAS_FLOAT
	log_stdout("%f", num->f);
#else
	log_stdout("%s", (num->f) ? num->f : "--");
#endif
	break;
    case NCX_BT_FLOAT64:
#ifdef HAS_FLOAT
	log_stdout("%lf", num->d);
#else
	log_stdout("%s", (num->d) ? num->d : "--");
#endif
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }
} /* stdout_num */


/********************************************************************
* FUNCTION dump_extern
* 
* Printf the specified external file
*
* INPUTS:
*    fspec == filespec to printf
*
*********************************************************************/
static void
    dump_extern (const xmlChar *fname)
{
    FILE               *fil;
    boolean             done;
    int                 ch;

    if (!fname) {
	log_error("\nval: No extern fname");
	return;
    }

    fil = fopen((const char *)fname, "r");
    if (!fil) {
	log_error("\nval: Open extern failed (%s)", fname);
	return;
    } 

    done = FALSE;
    while (!done) {
	ch = fgetc(fil);
	if (ch == EOF) {
	    fclose(fil);
	    done = TRUE;
	} else {
	    log_write("%c", ch);
	}
    }

} /* dump_extern */


/********************************************************************
* FUNCTION stdout_extern
* 
* Printf the specified external file to stdout
*
* INPUTS:
*    fspec == filespec to printf
*
*********************************************************************/
static void
    stdout_extern (const xmlChar *fname)
{
    FILE               *fil;
    boolean             done;
    int                 ch;

    if (!fname) {
	log_stdout("\nval: No extern fname");
	return;
    }

    fil = fopen((const char *)fname, "r");
    if (!fil) {
	log_stdout("\nval: Open extern failed (%s)", fname);
	return;
    } 

    done = FALSE;
    while (!done) {
	ch = fgetc(fil);
	if (ch == EOF) {
	    fclose(fil);
	    done = TRUE;
	} else {
	    log_stdout("%c", ch);
	}
    }

} /* stdout_extern */


/********************************************************************
* FUNCTION dump_intern
* 
* Printf the specified internal XML buffer
*
* INPUTS:
*    intbuff == internal buffer to printf
*
*********************************************************************/
static void
    dump_intern (const xmlChar *intbuff)
{
    const xmlChar      *ch;

    if (!intbuff) {
	log_error("\nval: No internal buffer");
	return;
    }

    ch = intbuff;
    while (*ch) {
	log_write("%c", *ch++);
    }

} /* dump_intern */


/********************************************************************
* FUNCTION stdout_intern
* 
* Printf the specified internal XML buffer to stdout
*
* INPUTS:
*    intbuff == internal buffer to printf
*
*********************************************************************/
static void
    stdout_intern (const xmlChar *intbuff)
{
    const xmlChar      *ch;

    if (!intbuff) {
	log_stdout("\nval: No internal buffer");
	return;
    }

    ch = intbuff;
    while (*ch) {
	log_stdout("%c", *ch++);
    }

} /* stdout_intern */


/********************************************************************
* FUNCTION pattern_match
* 
* Check the specified string against the specified pattern
*
* INPUTS:
*    pattern == compiled pattern to use 
*    strval == string to check
*
* RETURNS:
*    TRUE is string matches pattern; FALSE otherwise
*********************************************************************/
static boolean
    pattern_match (const xmlRegexpPtr pattern,
		   const xmlChar *strval)
{
    int ret;

    ret = xmlRegexpExec(pattern, strval);
    if (ret==1) {
	return TRUE;
    } else if (ret==0) {
	return FALSE;
    } else if (ret < 0) {
	/* pattern match execution error, but compiled ok */
	SET_ERROR(ERR_NCX_INVALID_PATTERN);
	return FALSE;
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
    /*NOTREACHED*/

} /* pattern_match */


/********************************************************************
* FUNCTION check_svalQ_enum
* 
* Check all the ncx_enum_t structs in a queue of typ_sval_t 
*
* INPUTS:
*    intval == enum integer value
*    useval == TRUE if the intval is valid
*    name == enum len (use if non-NULL)
*    namelen == length of name string to use
*    checkQ == pointer to Q of typ_sval_t to check
* 
* OUTPUTS:
*    *reten == typ_enum_t found if matched (res == NO_ERR)
* RETURNS:
*    status, NO_ERR if string value is in the set
*********************************************************************/
static status_t
    check_svalQ_enum (int32 intval,
		      boolean useint,
		      const xmlChar *name,
		      uint32  namelen,
		      const dlq_hdr_t *checkQ,
		      typ_enum_t  **reten)
{
    typ_enum_t  *en;

    /* check this typdef for restrictions */
    for (en = (typ_enum_t *)dlq_firstEntry(checkQ);
	 en != NULL;
	 en = (typ_enum_t *)dlq_nextEntry(en)) {
	if (useint) {
	    if (intval == en->val) {
		if (name && namelen) {
		    if (!xml_strncmp(name, en->name, namelen)) {
			/* both name and num match */
			*reten = en;  
			return NO_ERR;
		    }
		} else {
		    /* just num used and it matches */
		    *reten = en;
		    return NO_ERR;
		}
	    }
	} else if (name && namelen) {
	    if (!xml_strncmp(name, en->name, namelen)) {
		/* just name used and it matches */
		*reten = en;
		return NO_ERR;
	    }
	}
    }
    return ERR_NCX_NOT_FOUND;

} /* check_svalQ_enum */


/********************************************************************
* FUNCTION clean_value
* 
* Scrub the memory in a ncx_value_t by freeing all
* the sub-fields. DOES NOT free the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    val == val_value_t data structure to clean
*   redo == TRUE if this value might be used over again
*           so val will be reset to all cleared values
*        == FALSE if this value is about to be freed
*           !!! many fields will be left with invalid values !!!
*********************************************************************/
static void 
    clean_value (val_value_t *val,
		 boolean redo)
{
    val_value_t   *cur;
    val_index_t   *in;
    ncx_btype_t    btyp;

    if (val->btyp==NCX_BT_UNION) {
	btyp = val->unbtyp;
    } else {
	btyp = val->btyp;
    }

    /* clean the val->v union, depending on base type */
    switch (btyp) {
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
	ncx_clean_num(btyp, &val->v.num);
	break;
    case NCX_BT_ENUM:
	ncx_clean_enum(&val->v.enu);
	break;
    case NCX_BT_BINARY:
	ncx_clean_binary(&val->v.binary);
	break;
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:
	ncx_clean_str(&val->v.str);
	break;
    case NCX_BT_IDREF:
	if (val->v.idref.name) {
	    m__free(val->v.idref.name);
	}
	break;
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
	ncx_clean_list(&val->v.list);
	break;
    case NCX_BT_LIST:
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
	while (!dlq_empty(&val->v.childQ)) {
	    cur = (val_value_t *)dlq_deque(&val->v.childQ);
	    val_free_value(cur);
	}
	break;
    case NCX_BT_EXTERN:
	if (val->v.fname) { 
	    m__free(val->v.fname);
	}
	break;
    case NCX_BT_INTERN:
	if (val->v.intbuff) { 
	    m__free(val->v.intbuff);
	}
	break;
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
    case NCX_BT_NONE:
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (val->dname) {
	m__free(val->dname);
	val->dname = NULL;
    }

    while (!dlq_empty(&val->metaQ)) {
	cur = (val_value_t *)dlq_deque(&val->metaQ);
	val_free_value(cur);
    }

    while (!dlq_empty(&val->indexQ)) {
	in = (val_index_t *)dlq_deque(&val->indexQ);
	m__free(in);
    }

    if (val->insertstr) {
	m__free(val->insertstr);
    }

    if (redo) {
	val_init_value(val);
    }

}  /* clean_value */


/********************************************************************
* FUNCTION merge_simple
* 
* Merge simple src val into dest val (! MUST be same type !)
* Instance qualifiers have already been checked,
* and only zero or 1 instance is allowed, so replace
* the current one
*
* INPUTS:
*    src == val to merge from
*       (destructive -- entries will be moved, not copied)
*    dest == val to merge into
*
*********************************************************************/
static void
    merge_simple (ncx_btype_t btyp,
		  val_value_t *src,
		  val_value_t *dest)
{
    /* need to replace the current value or merge a list, etc. */
    switch (btyp) {
    case NCX_BT_ENUM:
	dest->v.enu.name = src->v.enu.name;
	dest->v.enu.val = src->v.enu.val;
	break;
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
	dest->v.bool = src->v.bool;
	break;
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
	dest->v.num.i = src->v.num.i;
	break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
	dest->v.num.u = src->v.num.u;
	break;
    case NCX_BT_INT64:
	dest->v.num.l = src->v.num.l;
	break;
    case NCX_BT_UINT64:
	dest->v.num.ul = src->v.num.ul;
	break;
    case NCX_BT_FLOAT32:
	ncx_clean_num(btyp, &dest->v.num);
	dest->v.num.f = src->v.num.f;
#ifndef HAS_FLOAT
	src->v.num.f = NULL;
#endif
	break;
    case NCX_BT_FLOAT64:
	ncx_clean_num(btyp, &dest->v.num);
	dest->v.num.d = src->v.num.d;
#ifndef HAS_FLOAT
	src->v.num.d = NULL;
#endif
	break;
    case NCX_BT_BINARY:
	ncx_clean_binary(&dest->v.binary);
	dest->v.binary.ustr = src->v.binary.ustr;
	dest->v.binary.ubufflen = src->v.binary.ubufflen;
	dest->v.binary.ustrlen = src->v.binary.ustrlen;
	src->v.binary.ustr = NULL;
	src->v.binary.ustrlen = 0;
	src->v.binary.ubufflen = 0;
	break;
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /****/
	ncx_clean_str(&dest->v.str);
	dest->v.str = src->v.str;
	src->v.str = NULL;
	break;
    case NCX_BT_IDREF:
	dest->v.idref.nsid = src->v.idref.nsid;	
	if (dest->v.idref.name) {
	    m__free(dest->v.idref.name);
	}
	dest->v.idref.name = src->v.idref.name;
	src->v.idref.name = NULL;
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* merge_simple */


/********************************************************************
* FUNCTION index_match
* 
* Check 2 val_value structs for the same instance ID
* 
* The node data types must match, and must be
*    NCX_BT_LIST
*
* All index components must exactly match.
* 
* INPUTS:
*    val1 == first value to index match
*    val2 == second value to index match
*
* RETURNS:
*  -2 if some error
*   -1 if val1 < val2 index
*   0 if val1 == val2 index
*   1 if val1 > val2 index
*********************************************************************/
static int32
    index_match (const val_value_t *val1,
		 const val_value_t *val2)
{
    const val_index_t *c1, *c2;
    int32              cmp;

    /* only lists have index chains */
    if (val1->obj->objtype != OBJ_TYP_LIST) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return -2;
    }

    /* object templates must exactly match */
    if (val1->obj != val2->obj) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return -2;
    }

    /* get the first pair of index nodes to check */
    c1 = (val_index_t *)dlq_firstEntry(&val1->indexQ);
    c2 = (val_index_t *)dlq_firstEntry(&val2->indexQ);

    /* match index values, 1 for 1, left to right */
    for (;;) {

	/* check if 1 table has index values but not the other */
	if (!c1 && !c2) {
	    return 0;
	} else if (!c1) {
	    return -1;
	} else if (!c2) {
	    return 1;
	}

	/* check the node if the object then value matches */
	if (c1->val->obj != c2->val->obj) {
	    return 1;  
	} else {
	    /* same name in the same namespace */
	    cmp = val_compare(c1->val, c2->val);
	    if (cmp) {
		return cmp;
	    }
	}

	/* node matched, get next node */
	c1 = (val_index_t *)dlq_nextEntry(c1);
	c2 = (val_index_t *)dlq_nextEntry(c2);
    }
    /*NOTREACHED*/

}  /* index_match */


/********************************************************************
* FUNCTION init_from_template
* 
* Initialize a value node from its object template
*
* MUST CALL val_new_value OR val_init_value FIRST
*
* INPUTS:
*   val == pointer to the malloced struct to initialize
*   obj == object template to use
*   btyp == builtin type to use
*********************************************************************/
static void
    init_from_template (val_value_t *val,
			const obj_template_t *obj,
			ncx_btype_t  btyp)
{
    const typ_template_t  *listtyp;
    ncx_btype_t            listbtyp;

    val->obj = obj;
    val->typdef = obj_get_ctypdef(obj);
    val->btyp = btyp;
    val->nsid = obj_get_nsid(obj);

    /* the agent new_child_val function may have already
     * set the name field in agt_val_parse.c
     */
    if (!val->name) {
	val->name = obj_get_name(obj);
    }

    /* set the case object field if this is a node from a
     * choice.  This will be used to validate and manipulate
     * the choice that is not actually taking up a node
     * in the value tree
     */
    val->dataclass = obj_get_config_flag(obj) ?
	NCX_DC_CONFIG : NCX_DC_STATE;
    if (obj->parent && obj->parent->objtype==OBJ_TYP_CASE) {
	val->casobj = obj->parent;
    }
    if (!typ_is_simple(val->btyp)) {
	val_init_complex(val, btyp);
    } else if (val->btyp == NCX_BT_SLIST) {
	listtyp = typ_get_clisttyp(val->typdef);
	if (!listtyp) {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    listbtyp = NCX_BT_STRING;
	} else {
	    listbtyp = typ_get_basetype(&listtyp->typdef);
	}
	ncx_init_list(&val->v.list, listbtyp);
    } else if (val->btyp == NCX_BT_BITS) {
	ncx_init_list(&val->v.list, NCX_BT_BITS);
    }

}  /* init_from_template */


/********************************************************************
* FUNCTION check_rangeQ
* 
* Check all the typ_rangedef_t structs in the queue
* For search qualifier NCX_SQUAL_RANGE
*
* INPUTS:
*    btyp == number base type
*    num == number to check
*    checkQ == pointer to Q of typ_rangedef_t to check
*
* RETURNS:
*    status, NO_ERR if string value is in the set
*********************************************************************/
static status_t
    check_rangeQ (ncx_btype_t  btyp,
		  const ncx_num_t *num,
		  const dlq_hdr_t *checkQ)
{
    const typ_rangedef_t  *rv;
    int32            cmp;
    boolean          lbok, ubok;

    for (rv = (const typ_rangedef_t *)dlq_firstEntry(checkQ);
	 rv != NULL;
	 rv = (const typ_rangedef_t *)dlq_nextEntry(rv)) {

	lbok = FALSE;
	ubok = FALSE;

	/* make sure the range numbers are really this type */
	if (rv->btyp != btyp) {
	    return SET_ERROR(ERR_NCX_WRONG_NUMTYP);
	}

	/* check lower bound first, only if there is one */
	if (!(rv->flags & TYP_FL_LBINF)) {
	    cmp = ncx_compare_nums(num, &rv->lb, btyp);
	    if (cmp >= 0) {
		lbok = TRUE;
	    }
	} else {
	    /* LB == -INF, always passes the test */
	    lbok = TRUE;
	} 

	/* check upper bound last, only if there is one */
	if (!(rv->flags & TYP_FL_UBINF)) {
	    cmp = ncx_compare_nums(num, &rv->ub, btyp);
	    if (cmp <= 0) {
		ubok = TRUE;
	    }
	} else {
	    /* UB == INF, always passes the test */
	    ubok = TRUE;
	}

	if (lbok && ubok) {
	    /* num is >= LB and <= UB */
	    return NO_ERR;
	}
    }

    /* ran out of rangedef segments to check */
    return ERR_NCX_NOT_IN_RANGE;

} /* check_rangeQ */


/********************************************************************
* FUNCTION dump_value
* 
* Printf the specified val_value_t struct to 
* the logfile, or stdout if none set
* Uses conf file format (see ncx/conf.h)
*
* INPUTS:
*    val == value to dump
*    startindent == start indent char count
*    tolog == TRUE if use log for output
*             FALSE if use STDOUT for output
*********************************************************************/
static void
    dump_value (const val_value_t *val,
		int32 startindent,
		boolean tolog)
{
    dumpfn_t            dumpfn, errorfn;
    indentfn_t          indentfn;
    const val_value_t  *chval;
    const ncx_lmem_t   *listmem;
    const val_idref_t  *idref;
    xmlChar            *buff;
    ncx_btype_t         btyp, lbtyp;
    uint32              len;
    status_t            res;
    boolean             quotes;

#ifdef VAL_INCLUDE_META
    const dlq_hdr_t    *metaQ;
    const val_value_t  *metaval;
#endif

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (tolog) {
	dumpfn = log_write;
	errorfn = log_error;
	indentfn = log_indent;
    } else {
	dumpfn = log_stdout;
	errorfn = log_stdout;
	indentfn = log_stdout_indent;
    }

    /* indent and print the val name */
    (*indentfn)(startindent);
    if (val->btyp == NCX_BT_EXTERN) {
	(*dumpfn)("%s (extern=%s) ", 
		  (val->name) ? (const char *)val->name : "--",
		  (val->v.fname) ? (const char *)val->v.fname : "--");
    } else if (val->btyp == NCX_BT_INTERN) {
	(*dumpfn)("%s (intern) ",
		  (val->name) ? (const char *)val->name : "--");
    } else {
	(*dumpfn)("%s ", (val->name) ? (const char *)val->name : "--");
    }

    /* get the real base type */
    if (val->btyp == NCX_BT_UNION) {
	btyp = val->unbtyp;
    } else {
	btyp = val->btyp;
    }

    /* check if an index clause needs to be printed next */
    if (!dlq_empty(&val->indexQ)) {
	res = val_get_index_string(NULL, NCX_IFMT_CLI, val, NULL, &len);
	if (res == NO_ERR) {
	    buff = m__getMem(len+1);
	    if (buff) {
		res = val_get_index_string(NULL, NCX_IFMT_CLI, 
					   val, buff, &len);
		if (res == NO_ERR) {
		    (dumpfn)("%s ", buff);
		} else {
		    SET_ERROR(res);
		}
		m__free(buff);
	    } else {
		(*errorfn)("\nval: malloc failed for %u bytes", len+1);
	    }
	}

	/* check if there is no more to print for this node */
	if (typ_is_simple(btyp)) {
	    return;
	}
    }

    /* dump the value, depending on the base type */
    switch (btyp) {
    case NCX_BT_NONE:
	SET_ERROR(ERR_INTERNAL_VAL);
	break;
    case NCX_BT_ANY:
	(*dumpfn)("(any)");
	break;
    case NCX_BT_ENUM:
	if (val->v.enu.name) {
	    (*dumpfn)("%s", (const char *)val->v.enu.name);
	}
	break;
    case NCX_BT_EMPTY:
	if (!val->v.bool) {
	    (*dumpfn)("(not set)");   /* should not happen */
	}
	break;
    case NCX_BT_BOOLEAN:
	if (val->v.bool) {
	    (*dumpfn)("true");
	} else {
	    (*dumpfn)("false");
	}
	break;
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
	if (tolog) {
	    dump_num(btyp, &val->v.num);
	} else {
	    stdout_num(btyp, &val->v.num);
	}
	break;
    case NCX_BT_BINARY:
	(*dumpfn)("binary string, length '%u'",
		  val->v.binary.ustrlen);
	break;
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /*******/
	if (VAL_STR(val)) {
	    quotes = val_need_quotes(VAL_STR(val));
	    if (quotes) {
		(*dumpfn)("%c", VAL_QUOTE_CH);
	    }
	    if (obj_is_password(val->obj)) {
		(*dumpfn)("%s", (const char *)"****");
	    } else {
		(*dumpfn)("%s", (const char *)VAL_STR(val));
	    }
	    if (quotes) {
		(*dumpfn)("%c", VAL_QUOTE_CH);
	    }
	}
	break;
    case NCX_BT_IDREF:
	idref = VAL_IDREF(val);
	(*dumpfn)("%s:%s",
		  xmlns_get_ns_prefix(idref->nsid),
		  idref->name);
	break;
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
	if (dlq_empty(&val->v.list.memQ)) {
	    (*dumpfn)("{ }");
	} else {
	    lbtyp = val->v.list.btyp;
	    (*dumpfn)("{");
	    for (listmem = (const ncx_lmem_t *)
		     dlq_firstEntry(&val->v.list.memQ);
		 listmem != NULL;
		 listmem = (const ncx_lmem_t *)dlq_nextEntry(listmem)) {

		if (startindent >= 0) {
		    (*indentfn)(startindent+NCX_DEF_INDENT);
		}

		if (typ_is_string(lbtyp)) {
		    if (listmem->val.str) {
			quotes = val_need_quotes(listmem->val.str);
			if (quotes) {
			    (*dumpfn)("%c", VAL_QUOTE_CH);
			}
			(*dumpfn)("%s ", (const char *)listmem->val.str);
			if (quotes) {
			    (*dumpfn)("%c", VAL_QUOTE_CH);
			}
		    }
		} else if (typ_is_number(lbtyp)) {
		    if (tolog) {
			dump_num(lbtyp, &listmem->val.num);
		    } else {
			stdout_num(lbtyp, &listmem->val.num);
		    }
		    (*dumpfn)(" ");
		} else {
		    switch (lbtyp) {
		    case NCX_BT_ENUM:
			if (listmem->val.enu.name) {
			    (*dumpfn)("%s ",
				      (const char *)listmem->val.enu.name);
			}
			break;
		    case NCX_BT_BITS:
			(*dumpfn)("%s ", (const char *)listmem->val.str);
			break;
		    case NCX_BT_BOOLEAN:
			(*dumpfn)("%s ",
				  (listmem->val.bool) ? 
				  NCX_EL_TRUE : NCX_EL_FALSE);
			break;
		    default:
			SET_ERROR(ERR_INTERNAL_VAL);
		    }
		}
	    }
	    (*indentfn)(startindent);
	    (*dumpfn)("}");
	}
	break;
    case NCX_BT_LIST:
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
	(*dumpfn)("{");
	for (chval = (const val_value_t *)dlq_firstEntry(&val->v.childQ);
	     chval != NULL;
	     chval = (const val_value_t *)dlq_nextEntry(chval)) {
	    dump_value(chval, (startindent >= 0) ?
		       startindent+NCX_DEF_INDENT : startindent,
		       tolog);
	}
	(*indentfn)(startindent);
	(*dumpfn)("}");
	break;
    case NCX_BT_EXTERN:
	(*dumpfn)("{");
	(*indentfn)(startindent);
	if (tolog) {
	    dump_extern(val->v.fname);
	} else {
	    stdout_extern(val->v.fname);
	}
	(*indentfn)(startindent);
	(*dumpfn)("}");
	break;
    case NCX_BT_INTERN:
	(*dumpfn)("{");
	(*indentfn)(startindent);
	if (tolog) {
	    dump_intern(val->v.intbuff);
	} else {
	    stdout_intern(val->v.intbuff);
	}
	(*indentfn)(startindent);
	(*dumpfn)("}");
	break;
    default:
	(*errorfn)("\nval: illegal btype (%d)", btyp);
    }    

#ifdef VAL_INCLUDE_META
    /* dump the metadata queue if non-empty */
    metaQ = val_get_metaQ(val);
    if (metaQ && !dlq_empty(metaQ)) {
	if (startindent >= 0) {
	    (*indentfn)(startindent+NCX_DEF_INDENT);
	}
	(*dumpfn)("%s.metaQ ", val->name);
	for (metaval = val_get_first_meta(metaQ);
	     metaval != NULL;
	     metaval = val_get_next_meta(metaval)) {
	    dump_value(metaval, (startindent >= 0) ?
		       startindent+(2*NCX_DEF_INDENT) : startindent,
		       tolog);
	}
    }
#endif

}   /* dump_value */


/********************************************************************
* FUNCTION position_walker
* 
* Position finder val_walker_fn for XPath support
* Follows val_walker_fn_t template
*
* INPUTS:
*   val == value node being processed in the tree walk
*   cookie1 == cookie1 value passed to start fn
*   cookie2 == cookie2 value passed to start fn
*
* RETURNS:
*   TRUE if walk should continue, FALSE if not
*********************************************************************/
static boolean
    position_walker (val_value_t *val,
		     void *cookie1,
		     void *cookie2)
{
    finderparms_t *finderparms;

    finderparms = (finderparms_t *)cookie1;
    (void)cookie2;

    finderparms->foundval = val;
    finderparms->foundpos++;

    if (finderparms->findval && 
	(finderparms->findval == val)) {
	return FALSE;
    }

    if (finderparms->findpos && 
	(finderparms->findpos == finderparms->foundpos)) {
	return FALSE;
    }

    return TRUE;

}  /* position_walker */


/********************************************************************
* FUNCTION process_one_valwalker
* 
* Process one child object node for
* the obj_find_all_* functions
*
* INPUTS:
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    obj == object to process
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    name == name of node to filter
*              == NULL to match any node name
*    configonly = TRUE for config=true only
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    fncalled == address of return function called flag
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
static boolean
    process_one_valwalker (val_walker_fn_t walkerfn,
			   void *cookie1,
			   void *cookie2,
			   val_value_t *val,
			   const xmlChar *modname,
			   const xmlChar *name,
			   boolean configonly,
			   boolean textmode,
			   boolean *fncalled)
{
    boolean         fnresult;

    *fncalled = FALSE;
    if (configonly && !name && !obj_is_config(val->obj)) {
	return TRUE;
    }

    fnresult = TRUE;
    if (textmode) {
	if ((val->obj->objtype == OBJ_TYP_LEAF ||
	     val->obj->objtype == OBJ_TYP_LEAF_LIST) &&
	    val->btyp != NCX_BT_ANY) {

	    if (walkerfn) {
		fnresult = (*walkerfn)(val, cookie1, cookie2);
	    }
	    *fncalled = TRUE;
	}
    } else if (modname && name) {
	if (!xml_strcmp(modname, 
			obj_get_mod_name(val->obj)) &&
	    !xml_strcmp(name, val->name)) {

	    if (walkerfn) {
		fnresult = (*walkerfn)(val, cookie1, cookie2);
	    }
	    *fncalled = TRUE;
	}
    } else if (modname) {
	if (!xml_strcmp(modname, obj_get_mod_name(val->obj))) {
	    if (walkerfn) {
		fnresult = (*walkerfn)(val, cookie1, cookie2);
	    }
	    *fncalled = TRUE;
	}
    } else if (name) {
	if (!xml_strcmp(name, val->name)) {
	    if (walkerfn) {
		fnresult = (*walkerfn)(val, cookie1, cookie2);
	    }
	    *fncalled = TRUE;
	}
    } else {
	if (walkerfn) {
	    fnresult = (*walkerfn)(val, cookie1, cookie2);
	}
	*fncalled = TRUE;
    }

    return fnresult;

}  /* process_one_valwalker */


/*************** E X T E R N A L    F U N C T I O N S  *************/


/********************************************************************
* FUNCTION val_new_value
* 
* Malloc and initialize the fields in a val_value_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
val_value_t * 
    val_new_value (void)
{
    val_value_t  *val;

    val = m__getObj(val_value_t);
    if (!val) {
	return NULL;
    }
    val_init_value(val);
    return val;

}  /* val_new_value */


/********************************************************************
* FUNCTION val_init_value
* 
* Initialize the fields in a val_value_t
*
* INPUTS:
*   val == pointer to the malloced struct to initialize
*********************************************************************/
void
    val_init_value (val_value_t *val)
{
    (void)memset(val, 0x0, sizeof(val_value_t));
    dlq_createSQue(&val->metaQ);
    dlq_createSQue(&val->indexQ);

}  /* val_init_value */


/********************************************************************
* FUNCTION val_init_complex
* 
* Initialize the fields in a complex val_value_t
*
* MUST CALL val_init_value FIRST
*
* INPUTS:
*   val == pointer to the malloced struct to initialize
*********************************************************************/
void
    val_init_complex (val_value_t *val, 
		      ncx_btype_t btyp)
{
    val->btyp = btyp;
    dlq_createSQue(&val->v.childQ);

}  /* val_init_complex */


/********************************************************************
* FUNCTION val_init_virtual
* 
* Special function to initialize a virtual value node
*
* MUST CALL val_init_value FIRST
*
* INPUTS:
*   val == pointer to the malloced struct to initialize
*   cbfn == get callback function to use
*   obj == object template to use
*********************************************************************/
void
    val_init_virtual (val_value_t *val,
		      void  *cbfn,
		      const obj_template_t *obj)
{
    val_init_from_template(val, obj);
    val->getcb = cbfn;

}  /* val_init_virtual */


/********************************************************************
* FUNCTION val_init_from_template
* 
* Initialize a value node from its object template
*
* MUST CALL val_new_value OR val_init_value FIRST
*
* INPUTS:
*   val == pointer to the initialized value struct to bind
*   obj == object template to use
*********************************************************************/
void
    val_init_from_template (val_value_t *val,
			    const obj_template_t *obj)
{

    init_from_template(val, obj, obj_get_basetype(obj));

}  /* val_init_from_template */


/********************************************************************
* FUNCTION val_free_value
* 
* Scrub the memory in a val_value_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    val == val_value_t to delete
*********************************************************************/
void 
    val_free_value (val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif
    clean_value(val, FALSE);
    m__free(val);

}  /* val_free_value */


/********************************************************************
* FUNCTION val_clean_value
* 
* Scrub the memory in a ncx_value_t by freeing all
* the sub-fields. DOES NOT free the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    val == val_value_t data structure to clean
*********************************************************************/
void 
    val_clean_value (val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;  
    }
#endif
    clean_value(val, TRUE);

}  /* val_clean_value */


/********************************************************************
* FUNCTION val_set_name
* 
* Set (or reset) the name of a value struct
*
* INPUTS:
*    val == val_value_t data structure to check
*    name == name string to set
*    namelen == length of name string
*********************************************************************/
void 
    val_set_name (val_value_t *val,
		  const xmlChar *name,
		  uint32 namelen)
{
#ifdef DEBUG
    if (!val || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;  
    }
#endif

    /* check no change to name */
    if (val->name && (xml_strlen(val->name) == namelen) &&
	!xml_strncmp(val->name, name, namelen)) {
	return;
    }

    /* replace the name field */
    if (val->dname) {
	m__free(val->dname);
    }
    val->dname = xml_strndup(name, namelen);
    if (!val->dname) {
	SET_ERROR(ERR_INTERNAL_MEM);
    } 
    val->name = val->dname;

}  /* val_set_name */


/********************************************************************
* FUNCTION val_set_qname
* 
* Set (or reset) the name and namespace ID of a value struct
*
* INPUTS:
*    val == val_value_t data structure to check
*    nsid == namespace ID to set
*    name == name string to set
*    namelen == length of name string
*********************************************************************/
void 
    val_set_qname (val_value_t *val,
		   xmlns_id_t   nsid,
		   const xmlChar *name,
		   uint32 namelen)
{
#ifdef DEBUG
    if (!val || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;  
    }
#endif

    val->nsid = nsid;

    /* check no change to name */
    if (val->name && (xml_strlen(val->name) == namelen) &&
	!xml_strncmp(val->name, name, namelen)) {
	return;
    }

    /* replace the name field */
    if (val->dname) {
	m__free(val->dname);
    }
    val->dname = xml_strndup(name, namelen);
    if (!val->dname) {
	SET_ERROR(ERR_INTERNAL_MEM);
    } 
    val->name = val->dname;

}  /* val_set_qname */


/********************************************************************
* FUNCTION val_string_ok
* 
* Check a string to make sure the value is valid based
* on the restrictions in the specified typdef
*
* INPUTS:
*    typdef == typ_def_t for the designated string type
*    btyp == basetype of the string
*    strval == string value to check
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_string_ok (const typ_def_t *typdef,
		   ncx_btype_t btyp,
		   const xmlChar *strval)
{

    return val_string_ok_errinfo(typdef, btyp, strval, NULL);

} /* val_string_ok */


/********************************************************************
* FUNCTION val_string_ok_errinfo
* 
* Check a string to make sure the value is valid based
* on the restrictions in the specified typdef
* Retrieve the configured error info struct if any error
*
* INPUTS:
*    typdef == typ_def_t for the designated string type
*    btyp == basetype of the string
*    strval == string value to check
*    errinfo == address of return errinfo block (may be NULL)
*
* OUTPUTS:
*   if non-NULL: 
*       *errinfo == error record to use if return error
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_string_ok_errinfo (const typ_def_t *typdef,
			   ncx_btype_t btyp,
			   const xmlChar *strval,
			   const ncx_errinfo_t **errinfo)
{
    status_t        res;
    ncx_num_t       len;

#ifdef DEBUG
    if (!typdef || !strval) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (errinfo) {
	*errinfo = NULL;
    }

    /* make sure the data type is correct */
    switch (btyp) {
    case NCX_BT_STRING:
	len.u = xml_strlen(strval);
	res = val_range_ok_errinfo(typdef, NCX_BT_UINT32, &len, errinfo);
	if (res != NO_ERR) {
	    return res;
	}

	/* the range-test, if any, has succeeded
	 * check if there is a pattern test for a string only
	 */
	if (btyp == NCX_BT_STRING) {
	    res = val_pattern_ok_errinfo(typdef, strval, errinfo);
	}
	break;
    case NCX_BT_BINARY:
	/* just get the length of the decoded binary string */
	res = b64_decode(strval, xml_strlen(strval),
			 NULL, NCX_MAX_UINT, &len.u);
	if (res == NO_ERR) {
	    res = val_range_ok_errinfo(typdef, NCX_BT_UINT32, 
				       &len, errinfo);
	}
	break;
    case NCX_BT_INSTANCE_ID:
	return NO_ERR;  /*** BUG: MISSING INSTANCE ID VALIDATION ***/
    case NCX_BT_KEYREF:
	return NO_ERR;  /*** BUG: MISSING KEYREF VALIDATION ***/
    default:
	return ERR_NCX_WRONG_DATATYP;
    }

    /* check the range against the string length */

    return res;

} /* val_string_ok_errinfo */


/********************************************************************
* FUNCTION val_list_ok
* 
* Check a list to make sure the all the strings are valid based
* on the specified typdef
*
* INPUTS:
*    typdef == typ_def_t for the designated list type
*    btyp == base type (NCX_BT_SLIST or NCX_BT_BITS)
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
    val_list_ok (const typ_def_t *typdef,
		 ncx_btype_t  btyp,
		 ncx_list_t *list)
{

    return val_list_ok_errinfo(typdef, btyp, list, NULL);

} /* val_list_ok */


/********************************************************************
* FUNCTION val_list_ok_errinfo
* 
* Check a list to make sure the all the strings are valid based
* on the specified typdef
*
* INPUTS:
*    typdef == typ_def_t for the designated list type
*    btyp == base type (NCX_BT_SLIST or NCX_BT_BITS)
*    list == list struct with ncx_lmem_t structs to check
*    errinfo == address of return rpc-error info struct
*
* OUTPUTS:
*   If return other than NO_ERR:
*     *errinfo contains the YANG specified error info, if any*   
*     each list->lmem.flags field may contain bits set
*     for errors:
*        NCX_FL_RANGE_ERR: size out of range
*        NCX_FL_VALUE_ERR  value not permitted by value set, 
*                          or pattern
* RETURNS:
*    status
*********************************************************************/
status_t
    val_list_ok_errinfo (const typ_def_t *typdef,
			 ncx_btype_t  btyp,
			 ncx_list_t *list,
			 const ncx_errinfo_t **errinfo)
{
    const typ_template_t *listtyp;
    const typ_def_t      *listdef;
    const ncx_lmem_t     *lmem;
    status_t              res;

#ifdef DEBUG
    if (!typdef || !list) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (errinfo) {
	*errinfo = NULL;
    }

    /* listtyp is for the list members, not the list itself */
    if (btyp == NCX_BT_SLIST) {
	listtyp = typ_get_clisttyp(typdef);
	listdef = &listtyp->typdef;
    }

    /* go through all the list members and check them */
    for (lmem = (ncx_lmem_t *)dlq_firstEntry(&list->memQ);
	 lmem != NULL;
	 lmem = (ncx_lmem_t *)dlq_nextEntry(lmem)) {

	/* stop on first error -- if 1 list member is invalid
	 * then the entire list is invalid
	 */
	if (btyp == NCX_BT_SLIST) {
	    res = val_simval_ok_errinfo(listdef,
					lmem->val.str,
					errinfo);
	} else {
	    res = val_bit_ok(typdef, 
			     lmem->val.str, NULL, NULL);
	}
	if (res != NO_ERR) {
	    return res;
	}
    }

    return NO_ERR;

} /* val_list_ok_errinfo */

  
/********************************************************************
* FUNCTION val_enum_ok
* 
* Check an enumerated integer string to make sure the value 
* is valid based on the specified typdef
*
* INPUTS:
*    typdef == typ_def_t for the designated enum type
*    enumval == enum string value to check
*    retval == pointer to return integer variable
*    retstr == pointer to return string name variable
* 
* OUTPUTS:
*    *retval == integer value of enum
*    *retstr == pointer to return string name variable 
* 
* RETURNS:
*    status
*********************************************************************/
status_t
    val_enum_ok (const typ_def_t *typdef,
		 const xmlChar *enumval,
		 int32 *retval,
		 const xmlChar **retstr)
{
    ncx_btype_t    btyp;
    const dlq_hdr_t  *checkQ;
    status_t       res;
    int32          i;
    boolean        iset, last;
    uint32         elen;
    typ_enum_t    *en;

#ifdef DEBUG
    if (!typdef || !enumval ||!retval ||!retstr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* make sure the data type is correct */
    btyp = typ_get_basetype(typdef);
    if (btyp != NCX_BT_ENUM) {
	return ERR_NCX_WRONG_DATATYP;
    }

    /* split the enum string into its parts */
    res = ncx_decode_enum(enumval, &i, &iset, &elen);
    if (res != NO_ERR) {
	return res;
    }

    /* check which string Q to use for further processing */
    switch (typdef->class) {
    case NCX_CL_SIMPLE:
	checkQ = &typdef->def.simple.valQ;
	last = TRUE;
	break;
    case NCX_CL_NAMED:
	/* the restrictions in the newtyp override anything
	 * in the parent typedef(s)
	 */
	last = FALSE;
	if (typdef->def.named.newtyp) {
	    checkQ = &typdef->def.named.newtyp->def.simple.valQ;
	} else {
	    checkQ = NULL;
	}
	break;
    default:
	return ERR_NCX_WRONG_DATATYP;  /* should not happen */
    }

    /* check typdefs until the final one in the chain is reached */
    for (;;) {
	if (checkQ) {
	    res = check_svalQ_enum(i, iset, (elen) ? enumval : NULL, 
				   elen, checkQ, &en);
	    if (res == NO_ERR) {
		/* return both the name and number of the found enum */
		*retval = en->val;
		*retstr = en->name;
		return NO_ERR;
	    }
	}

	/* check if any more typdefs to search 
         * NOT SUPPORTED IN YANG, WILL NOT FIND ANY MORE ENUMS
	 */
	if (last) {
	    return ERR_NCX_VAL_NOTINSET;
	}

	/* setup the typdef and checkQ for the next loop */
	typdef = typ_get_next_typdef(&typdef->def.named.typ->typdef);
	if (!typdef) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	switch (typdef->class) {
	case NCX_CL_SIMPLE:
	    checkQ = &typdef->def.simple.valQ;
	    last = TRUE;
	    break;
	case NCX_CL_NAMED:
	    if (typdef->def.named.newtyp) {
		checkQ = &typdef->def.named.newtyp->def.simple.valQ;
	    } else {
		checkQ = NULL;
	    }
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
    /*NOTREACHED*/

} /* val_enum_ok */


/********************************************************************
* FUNCTION val_bit_ok
* 
* Check a bit name is valid for the typedef
*
* INPUTS:
*    typdef == typ_def_t for the designated bits type
*    bitname == bit name value to check
*    position == address of return bit struct position value
*    order == address of return bit struct order
*
* OUTPUTS:
*  if non-NULL:
*     *position == bit position value
*     *order == bit struct typedef order
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_bit_ok (const typ_def_t *typdef,
		const xmlChar *bitname,
		uint32 *position,
		uint32 *order)
{
    const dlq_hdr_t  *checkQ;
    status_t       res;
    boolean        last;
    typ_enum_t    *en;

#ifdef DEBUG
    if (!typdef || !bitname) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check which string Q to use for further processing */
    switch (typdef->class) {
    case NCX_CL_SIMPLE:
	checkQ = &typdef->def.simple.valQ;
	last = TRUE;
	break;
    case NCX_CL_NAMED:
	/* the restrictions in the newtyp override anything
	 * in the parent typedef(s)
	 */
	last = FALSE;
	if (typdef->def.named.newtyp) {
	    checkQ = &typdef->def.named.newtyp->def.simple.valQ;
	} else {
	    checkQ = NULL;
	}
	break;
    default:
	return ERR_NCX_WRONG_DATATYP;  /* should not happen */
    }

    /* check typdefs until the final one in the chain is reached */
    for (;;) {
	if (checkQ) {
	    res = check_svalQ_enum(0, FALSE, bitname,
				   xml_strlen(bitname),
				   checkQ, &en);
	    if (res == NO_ERR) {
		if (position) {
		    *position = en->pos;
		}
		if (order) {
		    *order = en->order;
		}
		return NO_ERR;
	    }
	}

	/* check if any more typdefs to search 
         * NOT SUPPORTED IN YANG, WILL NOT FIND ANY MORE BITS 
	 */	
	if (last) {
	    return ERR_NCX_VAL_NOTINSET;
	}

	/* setup the typdef and checkQ for the next loop */
	typdef = typ_get_next_typdef(&typdef->def.named.typ->typdef);
	if (!typdef) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	switch (typdef->class) {
	case NCX_CL_SIMPLE:
	    checkQ = &typdef->def.simple.valQ;
	    last = TRUE;
	    break;
	case NCX_CL_NAMED:
	    if (typdef->def.named.newtyp) {
		checkQ = &typdef->def.named.newtyp->def.simple.valQ;
	    } else {
		checkQ = NULL;
	    }
	    break;
	default:
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    }
    /*NOTREACHED*/

} /* val_bit_ok */


/********************************************************************
* FUNCTION val_idref_ok
* 
* Check if an identityref QName is valid for the typedef
* The QName must match an identity that has the same base
* as specified in the typdef
*
* INPUTS:
*    typdef == typ_def_t for the designated identityref type
*    qname == QName or local-name string to check
*    nsid == namespace ID from XML node for NS of QName
*            this NSID will be used and not the prefix in qname
*            it was parsed in the XML node and is not a module prefix
*    name == address of return local name part of QName
*    id == address of return identity, if found
*
* OUTPUTS:
*  if non-NULL:
*     *name == pointer into the qname string at the start of
*              the local name part
*     *id == pointer to ncx_identity_t found
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_idref_ok (const typ_def_t *typdef,
		  const xmlChar *qname,
		  xmlns_id_t nsid,
		  const xmlChar **name,
		  const ncx_identity_t **id)
{
    const typ_idref_t     *idref;
    const xmlChar         *str, *modname;
    ncx_module_t          *mod;
    const ncx_identity_t  *identity;
    boolean                found;

#ifdef DEBUG
    if (!typdef || !qname) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (typ_get_basetype(typdef) != NCX_BT_IDREF) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    found = FALSE;

    if (nsid == XMLNS_NULL_NS_ID) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    idref = typ_get_idref(typdef);
    if (!idref) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    
    /* find the local-name in the prefix:local-name combo */
    str = qname;
    while (*str && *str != ':') {
	str++;
    }
    if (*str == ':') {
	str++;
    }

    /* str should point to the local name now */
    modname = xmlns_get_module(nsid);
    if (!modname) {
	return ERR_NCX_DEF_NOT_FOUND;
    }
    mod = ncx_find_module(modname);
    if (!mod) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* the namespace produced a module which should have
     * the identity-stmt with this name
     */
    identity = ncx_find_identity(mod, str);
    if (!identity) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* got some identity match; make sure this identity
     * has an ancestor-or-self node that is the same base
     * as the base specified in the typdef
     */

    while (identity && !found) {
	if (!xml_strcmp(ncx_get_modname(identity->mod), 
			idref->modname) &&
	    !xml_strcmp(identity->name, idref->basename)) {
	    found = TRUE;
	} else {
	    identity = identity->base;
	}
    }

    if (found) {
	if (name) {
	    *name = str;
	}
	if (id) {
	    *id = identity;
	}
	return NO_ERR;
    }

    return ERR_NCX_INVALID_VALUE;

} /* val_idref_ok */


/********************************************************************
* FUNCTION val_parse_idref
* 
* Parse a CLI BASED identityref QName into its various parts
*
* INPUTS:
*    mod == module containing the default-stmt (or NULL if N/A)
*    qname == QName or local-name string to parse
*    nsid == address of return namespace ID of the module
*            indicated by the prefix. If mod==NULL then
*            a prefix MUST be present
*    name == address of return local name part of QName
*    id == address of return identity, if found
*
* OUTPUTS:
*  if non-NULL:
*     *nsid == namespace ID for the prefix part of the QName
*     *name == pointer into the qname string at the start of
*              the local name part
*     *id == pointer to ncx_identity_t found (if any, not an error)
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_parse_idref (ncx_module_t *mod,
		     const xmlChar *qname,
		     xmlns_id_t  *nsid,
		     const xmlChar **name,
		     const ncx_identity_t **id)
{
    const xmlChar         *str, *modname;
    ncx_module_t          *impmod;
    const ncx_identity_t  *identity;
    const ncx_import_t    *import;
    xmlChar               *prefixbuff;
    status_t               res;
    uint32                 prefixlen;
    xmlns_id_t             prefixnsid;

#ifdef DEBUG
    if (!qname) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif
    
    res = NO_ERR;
    impmod = NULL;
    
    if (nsid) {
	*nsid = 0;
    }
    if (name) {
	*name = NULL;
    }

    /* find the local-name in the prefix:local-name combo */
    str = qname;
    while (*str && *str != ':') {
	str++;
    }

    /* figure out the prefix namespace ID */
    if (*str == ':') {
	/* prefix found */
	prefixnsid = 0;
	prefixlen = (uint32)(str++ - qname);
	prefixbuff = m__getMem(prefixlen+1);
	if (!prefixbuff) {
	    return ERR_INTERNAL_MEM;
	} else {
	    xml_strncpy(prefixbuff, qname, prefixlen);
	}

	if (mod) {
	    if (xml_strcmp(mod->prefix, prefixbuff)) {
		import = ncx_find_pre_import(mod, prefixbuff);
		if (import) {
		    impmod = ncx_find_module(import->module);
		    if (impmod) {
			prefixnsid = impmod->nsid;
			identity = ncx_find_identity(impmod, str);
		    } else {
			res = ERR_NCX_MOD_NOT_FOUND;
		    }
		} else {
		    res = ERR_NCX_IMP_NOT_FOUND;
		}
	    } else {
		prefixnsid = mod->nsid;
		identity = ncx_find_identity(mod, str);
	    }
	} else {
	    /* look up the default prefix for a module */
	    prefixnsid = xmlns_find_ns_by_prefix(prefixbuff);
	    if (prefixnsid) {
		modname = xmlns_get_module(prefixnsid);
		if (modname) {
		    impmod = ncx_find_module(modname);
		    if (impmod) {
			prefixnsid = impmod->nsid;
			identity = ncx_find_identity(impmod, str);
		    } else {
			res = ERR_NCX_MOD_NOT_FOUND;
		    }
		} else {
		    res = ERR_NCX_MOD_NOT_FOUND;
		}
	    } else {
		res = ERR_NCX_PREFIX_NOT_FOUND;
	    }
	}
	m__free(prefixbuff);
	if (name) {
	    *name = str;
	}
    } else {
	/* no prefix entered */
	if (name) {
	    *name = qname;
	}
	if (mod) {
	    prefixnsid = mod->nsid;
	    identity = ncx_find_identity(mod, qname);
	} else {
	    res = ERR_NCX_INVALID_VALUE;
	}
    }

    if (id) {
	*id = identity;
    }
    if (nsid) {
	*nsid = prefixnsid;
    }
    return res;

}  /* val_parse_idref */


/********************************************************************
* FUNCTION val_range_ok
* 
* Check a number to see if it is in range or not
* Could be a number or size range
*
* INPUTS:
*    typdef == typ_def_t for the simple type to check
*    btyp == base type of num
*    num == number to check
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_range_ok (const typ_def_t *typdef,
		  ncx_btype_t  btyp,
		  const ncx_num_t *num)
{

    return val_range_ok_errinfo(typdef, btyp, num, NULL);

} /* val_range_ok */


/********************************************************************
* FUNCTION val_range_ok_errinfo
* 
* Check a number to see if it is in range or not
* Could be a number or size range
*
* INPUTS:
*    typdef == typ_def_t for the simple type to check
*    btyp == base type of num
*    num == number to check
*    errinfo == address of return error struct
*
* OUTPUTS:
*   if non-NULL:
*     *errinfo == errinfo record on error exit
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_range_ok_errinfo (const typ_def_t *typdef,
			  ncx_btype_t  btyp,
			  const ncx_num_t *num,
			  const ncx_errinfo_t **errinfo)
{
    const typ_def_t       *testdef;
    const dlq_hdr_t       *checkQ;
    const ncx_errinfo_t   *range_errinfo;
    status_t          res;


#ifdef DEBUG
    if (!typdef || !num) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (errinfo) {
	*errinfo = NULL;
    }

    /* find the real typdef to check */
    testdef = typ_get_cqual_typdef(typdef, NCX_SQUAL_RANGE);
    if (!testdef) {
	/* assume this means no range specified and
	 * not an internal PTR or VAL error
	 */
	return NO_ERR;   
    }

    /* there can only be one active range, which is
     * the most derived typdef that declares a range
     */
    range_errinfo = typ_get_range_errinfo(testdef);
    checkQ = typ_get_crangeQ_con(testdef);

    res = check_rangeQ(btyp, num, checkQ);
    if (res != NO_ERR && errinfo && range_errinfo &&
	ncx_errinfo_set(range_errinfo)) {
	*errinfo = range_errinfo;
    }
    return res;

} /* val_range_ok_errinfo */


/********************************************************************
* FUNCTION val_pattern_ok
* 
* Check a string against all the patterns in a big AND expression
*
* INPUTS:
*    typdef == typ_def_t for the designated enum type
*    strval == string value to check
* 
* RETURNS:
*    NO_ERR if pattern OK or no patterns found to check; error otherwise
*********************************************************************/
status_t
    val_pattern_ok (const typ_def_t *typdef,
		    const xmlChar *strval)
{

    return val_pattern_ok_errinfo(typdef, strval, NULL);

}  /* val_pattern_ok */


/********************************************************************
* FUNCTION val_pattern_ok_errinfo
* 
* Check a string against all the patterns in a big AND expression
*
* INPUTS:
*    typdef == typ_def_t for the designated enum type
*    strval == string value to check
*    errinfo == address of return errinfo struct for err-pattern
*
* OUTPUTS:
*   *errinfo set to error info struct if any, and if error exit
*
* RETURNS:
*    NO_ERR if pattern OK or no patterns found to check; 
*    error otherwise, and *errinfo will be set if the pattern
*    that failed has any errinfo defined in it
*********************************************************************/
status_t
    val_pattern_ok_errinfo (const typ_def_t *typdef,
			    const xmlChar *strval,
			    const ncx_errinfo_t **errinfo)
{
    const typ_pattern_t   *pat;

#ifdef DEBUG
    if (!typdef) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (!strval) {
	strval = (const xmlChar *)"";
    }

    if (typ_get_basetype(typdef) != NCX_BT_STRING) {
	return ERR_NCX_WRONG_DATATYP;
    }

    if (errinfo) {
	*errinfo = NULL;
    }

    while (typdef) {
	for (pat = typ_get_first_cpattern(typdef);
	     pat != NULL;
	     pat = typ_get_next_cpattern(pat)) {

	    if (pat->res == NO_ERR) {
		if (!pattern_match(pat->pattern, strval)) {
		    if (errinfo && 
			ncx_errinfo_set(&pat->pat_errinfo)) {

			*errinfo = &pat->pat_errinfo;
			return ERR_NCX_PATTERN_FAILED;
		    }
		}
	    } else {
		/* return SET_ERROR(ERR_INTERNAL_VAL) */;
	    }
	}

	typdef = typ_get_cparent_typdef(typdef);
    }

    return NO_ERR;

} /* val_pattern_ok_errinfo */


/********************************************************************
* FUNCTION val_simval_ok
* 
* check any simple type to see if it is valid,
* but do not retrieve the value; used to check the
* default parameter for example
*
* INPUTS:
*    typdef == typ_def_t for the simple type to check
*    simval == value string to check (NULL means empty string)
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_simval_ok (const typ_def_t *typdef,
		   const xmlChar *simval)
{
    return val_simval_ok_errinfo(typdef, simval, NULL);

} /* val_simval_ok */



/********************************************************************
* FUNCTION val_simval_ok_errinfo
* 
* check any simple type to see if it is valid,
* but do not retrieve the value; used to check the
* default parameter for example
*
* INPUTS:
*    typdef == typ_def_t for the simple type to check
*    simval == value string to check (NULL means empty string)
*    errinfo == address of return error struct
*
* OUTPUTS:
*   if non-NULL:
*      *errinfo == error struct on error exit
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_simval_ok_errinfo (const typ_def_t *typdef,
			   const xmlChar *simval,
			   const ncx_errinfo_t **errinfo)
{
    const xmlChar          *retstr, *name;
    val_value_t            *unval;
    const typ_template_t   *listtyp;
    const ncx_identity_t   *identity;
    ncx_num_t               num;
    ncx_list_t              list;
    status_t                res;
    ncx_btype_t             btyp, listbtyp;
    int32                   retval;
    boolean                 forced;
    xmlns_id_t              nsid;

#ifdef DEBUG
    if (!typdef) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (errinfo) {
	*errinfo = NULL;
    }

    if (!simval) {
	forced = TRUE;
	simval = (const xmlChar *)"";
    } else {
	forced = FALSE;
    }

    btyp = typ_get_basetype(typdef);

    switch (btyp) {
    case NCX_BT_NONE:
	res = ERR_NCX_DEF_NOT_FOUND;
	break;
    case NCX_BT_ANY:
	res = ERR_NCX_INVALID_VALUE;
	break;
    case NCX_BT_ENUM:
	res = val_enum_ok(typdef, simval, &retval, &retstr);
	break;
    case NCX_BT_EMPTY:
	if (forced || !*simval) {
	    res = NO_ERR;
	} else {
	    res = ERR_NCX_INVALID_VALUE;
	}
	break;
    case NCX_BT_BOOLEAN:
	if (ncx_is_true(simval) || ncx_is_false(simval)) {
	    res = NO_ERR;
	} else {
	    res = ERR_NCX_INVALID_VALUE;
	}
	break;
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
    case NCX_BT_UINT64:
	if (*simval == '-') {
	    res = ERR_NCX_INVALID_VALUE;
	    break;
	}
	/* fall through */
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
    case NCX_BT_INT64:
    case NCX_BT_FLOAT32:
    case NCX_BT_FLOAT64:
	res = ncx_decode_num(simval, btyp, &num);
	if (res == NO_ERR) {
	    res = val_range_ok_errinfo(typdef, btyp, &num, errinfo);
	}
	ncx_clean_num(btyp, &num);
	break;
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
	res = val_string_ok_errinfo(typdef, btyp, simval, errinfo);
	break;
    case NCX_BT_INSTANCE_ID:
	/*****/
	res = val_string_ok_errinfo(typdef, btyp, simval, errinfo);
	break;
    case NCX_BT_UNION:
	unval = val_new_value();
	if (!unval) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    unval->btyp = NCX_BT_UNION;
	    unval->typdef = typdef;
	    res = val_union_ok(typdef, simval, unval);
	    val_free_value(unval);
	}
	break;
    case NCX_BT_KEYREF:
	/*****/
	res = val_string_ok_errinfo(typdef, btyp, simval, errinfo);
	break;
    case NCX_BT_IDREF:
	res = val_parse_idref(NULL, simval, &nsid, &name, &identity);
	if (res == NO_ERR) {
	    res = val_idref_ok(typdef, simval, nsid, &name, &identity);
	}
	break;
    case NCX_BT_BITS:
	ncx_init_list(&list, NCX_BT_BITS);
	res = ncx_set_list(NCX_BT_BITS, simval, &list);
	if (res == NO_ERR) {
	    res = ncx_finish_list(typdef, &list);
	    if (res == NO_ERR) {
		res = val_list_ok_errinfo(typdef, btyp, 
					  &list, errinfo);
	    }
	}
	ncx_clean_list(&list);
	break;
    case NCX_BT_SLIST:
	listtyp = typ_get_clisttyp(typdef);
	listbtyp = typ_get_basetype(&listtyp->typdef);
	ncx_init_list(&list, listbtyp);
	res = ncx_set_list(listbtyp, simval, &list);
	if (res == NO_ERR) {
	    res = ncx_finish_list(&listtyp->typdef, &list);
	    if (res == NO_ERR) {
		res = val_list_ok_errinfo(typdef, btyp, 
					  &list, errinfo);
	    }
	}
	ncx_clean_list(&list);
	break;
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_LIST:
    case NCX_BT_EXTERN:
    case NCX_BT_INTERN:
	res = ERR_NCX_INVALID_VALUE;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

} /* val_simval_ok_errinfo */


/********************************************************************
* FUNCTION val_union_ok
* 
* Check a union to make sure the string is valid based
* on the specified typdef, and convert the string to
* an NCX internal format
*
* INPUTS:
*    typdef == typ_def_t for the designated union type
*    strval == the value to check against the member typ defs
*    retval == pointer to output struct for converted value
*
* OUTPUTS:
*   If return NO_ERR:
*   retval->str or retval->num will be set with the converted value
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_union_ok (const typ_def_t *typdef,
		  const xmlChar *strval,
		  val_value_t *retval)
{

    return val_union_ok_errinfo(typdef, strval, retval, NULL);

} /* val_union_ok */


/********************************************************************
* FUNCTION val_union_ok_errinfo
* 
* Check a union to make sure the string is valid based
* on the specified typdef, and convert the string to
* an NCX internal format
*
* INPUTS:
*    typdef == typ_def_t for the designated union type
*    strval == the value to check against the member typ defs
*    retval == pointer to output struct for converted value
*    errinfo == address of error struct
*
* OUTPUTS:
*   If return NO_ERR:
*   retval->str or retval->num will be set with the converted value
*   *errinfo == error struct on error exit
*
* RETURNS:
*    status
*********************************************************************/
status_t
    val_union_ok_errinfo (const typ_def_t *typdef,
			  const xmlChar *strval,
			  val_value_t *retval,
			  const ncx_errinfo_t **errinfo)
{
    const typ_def_t        *undef;
    const typ_unionnode_t  *un;
    status_t                res;
    boolean                 done, retdone;

#ifdef DEBUG
    if (!typdef || !strval || !retval) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (errinfo) {
	*errinfo = NULL;
    }

    /* get the first union member type */
    un = typ_first_unionnode(typdef);

#ifdef DEBUG
    if (!un || (!un->typ && !un->typdef)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    retdone = FALSE;
    done = FALSE;

    /* go through all the union member typdefs until
     * the first match (decodes as a valid value for that typdef)
     */
    while (!done) {

	/* get type info for this member typedef */
	if (un->typ) {
	    undef = &un->typ->typdef;
	} else if (un->typdef) {
	    undef = un->typdef;
	} else {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    done = TRUE;
	    continue;
	}

	res = val_simval_ok_errinfo(undef, strval, errinfo);
	if (res == NO_ERR) {
	    /* the un->typ field may be NULL for YANG unions in progress
	     * When the default is checked this ptr may be NULL, but the
	     * retval is not used by that fn.  After the module is
	     * registered, the un->typ field should be set
	     */
	    if (!retdone) {
		retval->untyp = un->typ;   /****/

		/* what if this is set to NCX_BT_UNION still?
		 * not sure it matters.  Check!!!
		 */
		retval->unbtyp = typ_get_basetype(undef);
	    }
	    done = TRUE;
	} else if (res != ERR_INTERNAL_MEM) {
	    un = (const typ_unionnode_t *)dlq_nextEntry(un);
	    if (!un) {
		res = ERR_NCX_WRONG_NODETYP;
		done = TRUE;
	    }
	} else {
	    done = TRUE;
	}
    }

    return res;

} /* val_union_ok_errinfo */


/********************************************************************
* FUNCTION val_get_metaQ
* 
* Get the meta Q header for the value
* 
* INPUTS:
*    val == value node to check
*
* RETURNS:
*   pointer to the metaQ for this value
*********************************************************************/
const dlq_hdr_t *
    val_get_metaQ (const val_value_t  *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (val->getcb) {
	/***/ return NULL;
    } else {
	return &val->metaQ;
    }

}  /* val_get_metaQ */


/********************************************************************
* FUNCTION val_get_first_meta
* 
* Get the first metaQ entry from the specified Queue
* 
* INPUTS:
*    queue == queue of meta-vals to check
*
* RETURNS:
*   pointer to the first meta-var in the Queue if found, 
*   or NULL if none
*********************************************************************/
const val_value_t *
    val_get_first_meta (const dlq_hdr_t *queue)
{
#ifdef DEBUG
    if (!queue) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const val_value_t *)dlq_firstEntry(queue);

}  /* val_get_first_meta */


/********************************************************************
* FUNCTION val_get_first_meta_val
* 
* Get the first metaQ entry from the specified Queue
* 
* INPUTS:
*    value node to get the metaQ from
*
* RETURNS:
*   pointer to the first meta-var in the Queue if found, 
*   or NULL if none
*********************************************************************/
const val_value_t *
    val_get_first_meta_val (const val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const val_value_t *)dlq_firstEntry(&val->metaQ);

}  /* val_get_first_meta_val */


/********************************************************************
* FUNCTION val_get_next_meta
* 
* Get the next metaQ entry from the specified entry
* 
* INPUTS:
*    curnode == current meta-var node
*
* RETURNS:
*   pointer to the next meta-var in the Queue if found, 
*   or NULL if none
*********************************************************************/
const val_value_t *
    val_get_next_meta (const val_value_t *curnode)
{
#ifdef DEBUG
    if (!curnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const val_value_t *)dlq_nextEntry(curnode);

}  /* val_get_next_meta */


/********************************************************************
* FUNCTION val_meta_empty
* 
* Check if the metaQ is empty for the value node
*
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   TRUE if the metaQ for the value is empty
*   FALSE otherwise
*********************************************************************/
boolean
    val_meta_empty (const val_value_t *val)
{

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return TRUE;
    }
#endif

    if (val->getcb) {
	/***/ return TRUE;
    } else {
	return dlq_empty(&val->metaQ);
    }

}  /* val_meta_empty */


/********************************************************************
* FUNCTION val_find_meta
* 
* Get the corresponding meta data node 
* 
* INPUTS:
*    val == value to check for metadata
*    nsid == namespace ID of 'name'; 0 == don't use
*    name == name of metadata variable name
*
* RETURNS:
*   pointer to the child if found or NULL if not found
*********************************************************************/
val_value_t *
    val_find_meta (const val_value_t  *val,
		   xmlns_id_t    nsid,
		   const xmlChar *name)
{
    val_value_t *metaval;

#ifdef DEBUG
    if (!val || !name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif
	
    for (metaval = (val_value_t *)dlq_firstEntry(&val->metaQ);
	 metaval != NULL;
	 metaval = (val_value_t *)dlq_nextEntry(metaval)) {

	/* check the node if the name matches and
	 * check for position instance match 
	 */
	if (!xml_strcmp(metaval->name, name)) {
	    if (xmlns_ids_equal(nsid, metaval->nsid)) {
		return metaval;
	    }
	}
    }

    return NULL;

}  /* val_find_meta */


/********************************************************************
* FUNCTION val_meta_match
* 
* Return true if the corresponding attribute exists and has
* the same value 
* 
* INPUTS:
*    val == value to check for metadata
*    metaval == value to match in the val->metaQ 
*
* RETURNS:
*   TRUE if the specified attr if found and has the same value
*   FALSE otherwise
*********************************************************************/
boolean
    val_meta_match (const val_value_t *val,
		    const val_value_t *metaval)
{
    const val_value_t *m1;
    const dlq_hdr_t   *queue;
    boolean            ret, done;

#ifdef DEBUG
    if (!val || !metaval) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    queue = val_get_metaQ(val);
    if (!queue) {
	return FALSE;
    }

    ret = FALSE;
    done = FALSE;

    /* check all the metavars in the val->metaQ until
     * the specified entry is found or list ends
     */
    for (m1 = val_get_first_meta(queue);
	 m1 != NULL && !done;
	 m1 = val_get_next_meta(m1)) {

	/* check the node if the name matches and
	 * then if the namespace matches
	 */
	if (!xml_strcmp(metaval->name, m1->name)) {
	    if (!xmlns_ids_equal(metaval->nsid, m1->nsid)) {
		continue;
	    }
	    ret = (val_compare(metaval, m1)) ? FALSE : TRUE;
	    done = TRUE;
	}
    }

    return ret;

}  /* val_meta_match */


/********************************************************************
* FUNCTION val_metadata_inst_count
* 
* Get the number of instances of the specified attribute
*
* INPUTS:
*     val == value to check for metadata instance count
*     nsid == namespace ID of the meta data variable
*     name == name of the meta data variable
*
* RETURNS:
*   number of instances found in val->metaQ
*********************************************************************/
uint32
    val_metadata_inst_count (const val_value_t  *val,
			     xmlns_id_t nsid,
			     const xmlChar *name)
{
    const val_value_t *metaval;
    uint32             cnt;

    cnt = 0;

    for (metaval = (const val_value_t *)dlq_firstEntry(&val->metaQ);
	 metaval != NULL;
	 metaval = (const val_value_t *)dlq_nextEntry(metaval)) {
	if (xml_strcmp(metaval->name, name)) {
	    continue;
	}
	if (nsid && metaval->nsid) {
	    if (metaval->nsid == nsid) {
		cnt++;
	    }
	} else {
	    cnt++;
	}
    }
    return cnt;

} /* val_metadata_inst_count */


/********************************************************************
* FUNCTION val_dump_value
* 
* Printf the specified val_value_t struct to 
* the logfile, or stdout if none set
* Uses conf file format (see ncx/conf.h)
*
* INPUTS:
*    val == value to printf
*    startindent == start indent char count
*
*********************************************************************/
void
    val_dump_value (const val_value_t *val,
		    int32 startindent)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    dump_value(val, startindent, TRUE);

} /* val_dump_value */


/********************************************************************
* FUNCTION val_stdout_value
* 
* Printf the specified val_value_t struct to stdout
* Uses conf file format (see ncx/conf.h)
*
* Brute force clone of val_dump_value
*
* INPUTS:
*    val == value to printf
*    startindent == start indent char count
*
*********************************************************************/
void
    val_stdout_value (const val_value_t *val,
		      int32 startindent)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    dump_value(val, startindent, FALSE);

} /* val_stdout_value */


/********************************************************************
* FUNCTION val_set_string
* 
* Set an initialized val_value_t as a simple type
* namespace set to 0 !!!
*
* INPUTS:
*    val == value to set
*    valname == name of simple value
*    valstr == simple value encoded as a string
*
* OUTPUTS:
*    *val is filled in if return NO_ERR
* RETURNS:
*  status
*********************************************************************/
status_t 
    val_set_string (val_value_t  *val,
		    const xmlChar *valname,
		    const xmlChar *valstr)
{
    if (valname) {
	return val_set_simval_str(val, 
				  typ_get_basetype_typdef(NCX_BT_STRING),
				  0, valname, xml_strlen(valname), valstr);
    } else {
	return val_set_simval_str(val, 
				  typ_get_basetype_typdef(NCX_BT_STRING),
				  0, NULL, 0, valstr);

    }

}  /* val_set_string */


/********************************************************************
* FUNCTION val_set_string2
* 
* Set an initialized val_value_t as a simple type
* namespace set to 0 !!!
*
* INPUTS:
*    val == value to set
*    valname == name of simple value
*    typdef == parm typdef (may be NULL)
*    valstr == simple value encoded as a string
*    valstrlen == length of valstr to use
*
* OUTPUTS:
*    *val is filled in if return NO_ERR
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    val_set_string2 (val_value_t  *val,
		     const xmlChar *valname,
		     const typ_def_t *typdef,
		     const xmlChar *valstr,
		     uint32 valstrlen)
{
    status_t  res;
    xmlChar  *temp;

#ifdef DEBUG
    if (!val || !valstr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!valstrlen) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    if (typdef) {
	val->typdef = typdef;
	val->btyp = typ_get_basetype(typdef);
    } else {
	val->typdef = typ_get_basetype_typdef(NCX_BT_STRING);
	val->btyp = NCX_BT_STRING;
    }
    val->nsid = 0;

    switch (val->btyp) {
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /****/
	if (valname && !val->name) {
	    if (val->dname) {
		SET_ERROR(ERR_INTERNAL_VAL);
		m__free(val->dname);
	    }
	    val->dname = xml_strdup(valname);
	    if (!val->dname) {
		return ERR_INTERNAL_MEM;
	    }
	    val->name = val->dname;
	}

	VAL_STR(val) = xml_strndup(valstr, valstrlen);
	if (!VAL_STR(val)) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    res = NO_ERR;
	}
	break;
    default:
	temp = xml_strndup(valstr, valstrlen);
	if (temp) {
	    res = val_set_simval(val, typdef, 0, NULL, temp);
	    m__free(temp);
	} else {
	    res = ERR_INTERNAL_MEM;
	}
    }
    return res;

}  /* val_set_string2 */


/********************************************************************
* FUNCTION val_set_simval
* 
* Set an initialized val_value_t as a simple type
*
* INPUTS:
*    val == value to set
*    typdef == typdef of expected type
*    nsid == namespace ID of this field
*    valname == name of simple value
*    valstr == simple value encoded as a string
*
* OUTPUTS:
*    *val is filled in if return NO_ERR
* RETURNS:
*  status
*********************************************************************/
status_t 
    val_set_simval (val_value_t  *val,
		    const typ_def_t    *typdef,
		    xmlns_id_t    nsid,
		    const xmlChar *valname,
		    const xmlChar *valstr)
{
    if (valname) {
	return val_set_simval_str(val, typdef, nsid, 
				  valname, xml_strlen(valname), valstr);
    } else {
	return val_set_simval_str(val, typdef, nsid, NULL, 0, valstr);
    }

}  /* val_set_simval */


/********************************************************************
* FUNCTION val_set_simval_str
* 
* Set an initialized val_value_t as a simple type
*
* This function is intended to bypass all typdef
* checking wrt/ ranges, patterns, etc.
* During CLI processing or test PDU generation,
* any well-formed XML value needs to parsed or generated.
*
* Only the base type is used from the typdef parameter
* when converting a string value to val_value_t format
*
* Handles the following data types:
* 
*  NCX_BT_INT8
*  NCX_BT_INT16
*  NCX_BT_INT32
*  NCX_BT_INT64
*  NCX_BT_UINT8
*  NCX_BT_UINT16
*  NCX_BT_UINT32
*  NCX_BT_UINT64
*  NCX_BT_FLOAT32
*  NCX_BT_FLOAT64
*  NCX_BT_BINARY
*  NCX_BT_STRING
*  NCX_BT_INSTANCE_ID
*  NCX_BT_ENUM
*  NCX_BT_EMPTY
*  NCX_BT_BOOLEAN
*  NCX_BT_SLIST
*  NCX_BT_BITS
*  NCX_BT_UNION
*  NCX_BT_KEYREF
*  NCX_BT_IDREF
*
* INPUTS:
*    val == value to set
*    typdef == typdef of expected type
*    nsid == namespace ID of this field
*    valname == name of simple value
*    valnamelen == length of name string to compare
*    valstr == simple value encoded as a string
*
* OUTPUTS:
*    *val is filled in if return NO_ERR
* RETURNS:
*  status
*********************************************************************/
status_t 
    val_set_simval_str (val_value_t  *val,
			const typ_def_t *typdef,
			xmlns_id_t    nsid,
			const xmlChar *valname,
			uint32 valnamelen,
			const xmlChar *valstr)
{
    const xmlChar        *localname;
    const ncx_identity_t *identity;
    status_t              res;
    uint32                ulen;
    xmlns_id_t            qname_nsid;

#ifdef DEBUG
    if (!val || !typdef) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    /* only set name if it is not already set */
    if (!val->name && valname) {
	val->dname = xml_strndup(valname, valnamelen);
	if (!val->dname) {
	    return ERR_INTERNAL_MEM;
	}
	val->name = val->dname;
    }

    /* set the object to the generic string if not set */
    if (!val->obj) {
	val->obj = ncx_get_gen_string();
    }

    /* set these fields even if already set by 
     * val_init_from_template
     */
    val->nsid = nsid;
    val->typdef = typdef;
    val->btyp = typ_get_basetype(typdef);

    /* convert the value string, if any */
    switch (val->btyp) {
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
	if (valstr && *valstr) {
	    res = ncx_convert_num(valstr, NCX_NF_NONE, 
				  val->btyp, &val->v.num);
	} else {
	    res = ERR_NCX_EMPTY_VAL;
	}
	break;
    case NCX_BT_BINARY:
	ncx_init_binary(&val->v.binary);
	ulen = 0;
	if (valstr && *valstr) {
	    ulen = xml_strlen(valstr);
	}
	if (!ulen) {
	    break;
	}

	/* allocate a binary string as big as the input
	 * text string, even though it will be about 25% too big
	 */
	val->v.binary.ustr = m__getMem(ulen+1);
	if (!val->v.binary.ustr) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    /* really a test tool only for binary strings
	     * entered at the CLI; use @foo.jpg for
	     * raw input of binary files in var_get_script_val
	     */
	    memcpy(val->v.binary.ustr, valstr, ulen);
	    val->v.binary.ubufflen = ulen+1;
	    val->v.binary.ustrlen = ulen;
	}
	break;
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /****/
	VAL_STR(val) = xml_strdup(valstr);
	if (!VAL_STR(val)) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case NCX_BT_IDREF:
	res = val_parse_idref(NULL, valstr, &qname_nsid, 
			      &localname, &identity);
	if (res == NO_ERR) {
	    res = val_idref_ok(typdef, valstr, qname_nsid, 
			       &localname, &identity);
	    if (res == NO_ERR) {
		val->v.idref.nsid = qname_nsid;
		val->v.idref.identity = identity;
		val->v.idref.name = xml_strdup(localname);
		if (!val->v.idref.name) {
		    res = ERR_INTERNAL_MEM;
		}
	    }
	}
	break;
    case NCX_BT_ENUM:
	res = ncx_set_enum(valstr, &val->v.enu);
	break;
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
	/* if supplied, match the flag name against the supplied value */
	if (valstr) {
	    if (!xml_strcmp(NCX_EL_TRUE, valstr)) {
		val->v.bool = TRUE;
	    } else if (!xml_strcmp(NCX_EL_FALSE, valstr)) {
		val->v.bool = FALSE;
	    } else if (!xml_strncmp(valstr, valname, valnamelen)) {
		val->v.bool = TRUE;
	    } else {
		res = ERR_NCX_INVALID_VALUE;
	    }
	} else {
	    /* name indicates presence, so set val to TRUE */
	    val->v.bool = TRUE;
	}	    
	break;
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
	res = ncx_set_strlist(valstr, &val->v.list);
	break;
    case NCX_BT_UNION:
	/* set as generic string -- parser as other end will
	 * match against actual union types
	 */
	val->untyp = typ_get_basetype_typ(NCX_BT_STRING);
	val->unbtyp = NCX_BT_STRING;
	VAL_STR(val) = xml_strdup(valstr);
	if (!VAL_STR(val)) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
	val->btyp = NCX_BT_NONE;
    }

    return res;

}  /* val_set_simval_str */


/********************************************************************
* FUNCTION val_make_simval
* 
* Create and set a val_value_t as a simple type
*
* INPUTS:
*    typdef == typdef of expected type
*    nsid == namespace ID of this field
*    valname == name of simple value
*    valstr == simple value encoded as a string
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    pointer to malloced and filled in val_value_t struct
*    NULL if some error
*********************************************************************/
val_value_t *
    val_make_simval (const typ_def_t    *typdef,
		     xmlns_id_t    nsid,
		     const xmlChar *valname,
		     const xmlChar *valstr,
		     status_t  *res)
{
    val_value_t  *val;

#ifdef DEBUG
    if (!typdef || !valname || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    val = val_new_value();
    if (!val) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    if (valname) {
	*res = val_set_simval_str(val, typdef, nsid, 
				  valname, xml_strlen(valname), valstr);
    } else {
	*res = val_set_simval_str(val, typdef, nsid, NULL, 0, valstr);
    }
    return val;

}  /* val_make_simval */



/********************************************************************
* FUNCTION val_merge
* 
* Merge src val into dest val (! MUST be same type !)
* Any meta vars in src are also merged into dest
*
* INPUTS:
*    src == val to merge from
*
*       !!! destructive -- entries will be moved, not copied !!!
*       !!! Must be dequeued before calling this function !!!
*       !!! Must not use src pointer value again if *freesrc == FALSE

*    dest == val to merge into
*
* RETURNS:
*       TRUE if the source value needs to be deleted because the
*          memory was not transfered to the parent val childQ.
*       FALSE if the source value should not be freed because 
*         the memory is still in use, but transferred to the target
*********************************************************************/
boolean
    val_merge (val_value_t *src,
	       val_value_t *dest)
{
    ncx_btype_t      btyp;
    ncx_iqual_t      iqual;
    ncx_merge_t      mergetyp;
    boolean          dupsok;

#ifdef DEBUG
    if (!src || !dest) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return TRUE;
    }
#endif

    btyp = dest->btyp;
    iqual = typ_get_iqualval_def(dest->typdef);

    if (obj_is_system_ordered(dest->obj)) {
	mergetyp = typ_get_mergetype(dest->typdef);
	if (mergetyp == NCX_MERGE_NONE) {
	    mergetyp = NCX_MERGE_LAST;
	}
    } else {
	mergetyp = typ_get_mergetype(dest->typdef);
	if (mergetyp == NCX_MERGE_NONE) {
	    mergetyp = NCX_MERGE_LAST;
	}

	/***************** !!!!!!!!!!! *******************/
    }

    switch (iqual) {
    case NCX_IQUAL_1MORE:
    case NCX_IQUAL_ZMORE:
	switch (src->obj->objtype) {
	case OBJ_TYP_LEAF_LIST:
	case OBJ_TYP_LIST:
	    /* duplicates not allowed in leaf lists 
	     * leave the current value in place
	     */

	    /********** TBD: MOVE via insert attribute *****/

	    return TRUE;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return TRUE;
	}
	/*NOTREACHED*/
    case NCX_IQUAL_ONE:
    case NCX_IQUAL_OPT:
	/* need to replace the current value or merge a list, etc. */
	switch (btyp) {
	case NCX_BT_ENUM:
	case NCX_BT_EMPTY:
	case NCX_BT_BOOLEAN:
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
	case NCX_BT_INSTANCE_ID:
	case NCX_BT_KEYREF:   /*****/
	    merge_simple(btyp, src, dest);
	    break;
	case NCX_BT_UNION:
	    /* first clean the dest */
	    if (typ_is_number(dest->unbtyp)) {
		ncx_clean_num(dest->unbtyp, &dest->v.num);
	    } else if (typ_is_string(dest->unbtyp)) {		
		ncx_clean_str(&dest->v.str);
	    } else {
		switch (dest->unbtyp) {
		case NCX_BT_BINARY:
		    ncx_clean_binary(&dest->v.binary);
		    break;
		case NCX_BT_ENUM:
		    ncx_clean_enum(&dest->v.enu);
		    break;
		case NCX_BT_LIST:
		case NCX_BT_BITS:
		    ncx_clean_list(&dest->v.list);
		    break;
		default:
		    ;
		}
	    }

	    /* set the union member type */
	    dest->untyp = src->untyp;
	    dest->unbtyp = src->unbtyp;
	    merge_simple(src->unbtyp, src, dest);
	    break;
	case NCX_BT_SLIST:
	    dupsok = val_duplicates_allowed(dest);
	    ncx_merge_list(&src->v.list, &dest->v.list,
			   mergetyp, dupsok);
	    break;
	case NCX_BT_BITS:
	    ncx_merge_list(&src->v.list, &dest->v.list,
			   mergetyp, FALSE);
	    break;
	case NCX_BT_ANY:
	case NCX_BT_CONTAINER:
	case NCX_BT_LIST:
	case NCX_BT_CHOICE:
	case NCX_BT_CASE:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return TRUE;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return TRUE;
	}
	return TRUE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return TRUE;
    }
    /*NOTREACHED*/

}  /* val_merge */



/********************************************************************
* FUNCTION val_clone
* 
* Clone a specified val_value_t struct and sub-trees
*
* INPUTS:
*    val == value to clone
*
* RETURNS:
*   clone of val, or NULL if a malloc failure
*********************************************************************/
val_value_t *
    val_clone (const val_value_t *val)
{
    status_t res;

    return val_clone_test(val, NULL, &res);

}  /* val_clone */


/********************************************************************
* FUNCTION val_clone_test
* 
* Clone a specified val_value_t struct and sub-trees
* Only clone the nodes that pass the test function callback
*
* INPUTS:
*    val == value to clone
*    testcb  == filter test callback function to use
*               NULL means no filter
*    res == address of return status
*
* OUTPUTS:
*    *res == resturn status
*
* RETURNS:
*   clone of val, or NULL if a malloc failure or entire node filtered
*********************************************************************/
val_value_t *
    val_clone_test (const val_value_t *val,
		    val_test_fn_t  testfn,
		    status_t *res)
{
    const val_value_t *ch;
    val_value_t       *copy, *copych;
    boolean            testres;

#ifdef DEBUG
    if (!val || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (testfn) {
	testres = (*testfn)(val);
	if (!testres) {
	    *res = ERR_NCX_SKIPPED;
	    return NULL;
	}
    }

    copy = val_new_value();
    if (!copy) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    /* copy all the fields */
    copy->obj = val->obj;
    copy->typdef = val->typdef;

    if (val->dname) {
	copy->dname = xml_strdup(val->dname);
	if (!copy->dname) {
	    *res = ERR_INTERNAL_MEM;
	    val_free_value(copy);
	    return NULL;
	}
	copy->name = copy->dname;
    } else {
	copy->dname = NULL;
	copy->name = val->name;
    }

    copy->parent = val->parent;
    copy->nsid = val->nsid;
    copy->btyp = val->btyp;
    copy->flags = val->flags;
    copy->dataclass = val->dataclass;

    /* copy meta-data */
    for (ch = (const val_value_t *)dlq_firstEntry(&val->metaQ);
	 ch != NULL;
	 ch = (const val_value_t *)dlq_nextEntry(ch)) {
	copych = val_clone_test(ch, testfn, res);
	if (!copych) {
	    if (*res == ERR_NCX_SKIPPED) {
		*res = NO_ERR;
	    } else {
		val_free_value(copy);
		return NULL;
	    }
	} else {
	    dlq_enque(copych, &copy->metaQ);
	}
    }

    /* set the copy->insertstr */
    if (val->insertstr) {
	copy->insertstr = xml_strdup(val->insertstr);
	if (!copy->insertstr) {
	    *res = ERR_INTERNAL_MEM;
	    val_free_value(copy);
	    return NULL;
	}
    }

    copy->curparent = val->curparent;
    copy->editop = val->editop;
    copy->insertop = val->insertop;
    copy->res = val->res;
    copy->getcb = val->getcb;
    /* DO NOT COPY copy->index = val->index; */
    /* set copy->indexQ after cloning child nodes is done */
    copy->untyp = val->untyp;
    copy->unbtyp = val->unbtyp;
    copy->casobj = val->casobj;

    /* assume OK return for now */
    *res = NO_ERR;

    /* v_ union: copy the actual value or children for complex types */
    switch (val->btyp) {
    case NCX_BT_ENUM:
	copy->v.enu.name = val->v.enu.name;
	VAL_ENUM(copy) = VAL_ENUM(val);
	break;
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
	copy->v.bool = val->v.bool;
	break;
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
	*res = ncx_copy_num(&val->v.num, &copy->v.num, val->btyp);
	break;
    case NCX_BT_BINARY:
	ncx_init_binary(&copy->v.binary);
	if (val->v.binary.ustr) {
	    copy->v.binary.ustr = m__getMem(val->v.binary.ustrlen);
	    if (!copy->v.binary.ustr) {
		*res = ERR_INTERNAL_MEM;
	    } else {
		memcpy(copy->v.binary.ustr, 
		       val->v.binary.ustr, 
		       val->v.binary.ustrlen);
		copy->v.binary.ustrlen = val->v.binary.ustrlen;
		copy->v.binary.ubufflen = val->v.binary.ustrlen;
	    }
	}
	break;
    case NCX_BT_STRING:	
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /*****/
	*res = ncx_copy_str(&val->v.str, &copy->v.str, val->btyp);
	break;
    case NCX_BT_IDREF:
	copy->v.idref.name = xml_strdup(val->v.idref.name);
	if (!copy->v.idref.name) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    *res = NO_ERR;
	}
	copy->v.idref.nsid = val->v.idref.nsid;
	copy->v.idref.identity = val->v.idref.identity;
	break;
    case NCX_BT_BITS:
    case NCX_BT_SLIST:
	*res = ncx_copy_list(&val->v.list, &copy->v.list);
	break;
    case NCX_BT_ANY:
    case NCX_BT_LIST:
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
	val_init_complex(copy, val->btyp);
	for (ch = (const val_value_t *)dlq_firstEntry(&val->v.childQ);
	     ch != NULL && *res==NO_ERR;
	     ch = (const val_value_t *)dlq_nextEntry(ch)) {
	    copych = val_clone_test(ch, testfn, res);
	    if (!copych) {
		if (*res == ERR_NCX_SKIPPED) {
		    *res = NO_ERR;
		}
	    } else {
		copych->parent = copy;
		dlq_enque(copych, &copy->v.childQ);
	    }
	}
	break;
    case NCX_BT_EXTERN:
	if (val->v.fname) {
	    copy->v.fname = xml_strdup(val->v.fname);
	    if (!copy->v.fname) {
		*res = ERR_INTERNAL_MEM;
	    }
	}
	break;
    case NCX_BT_INTERN:
	if (val->v.intbuff) {
	    copy->v.intbuff = xml_strdup(val->v.intbuff);
	    if (!copy->v.intbuff) {
		*res = ERR_INTERNAL_MEM;
	    }
	}
	break;
    default:
	*res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* reconstruct index records if needed */
    if (*res==NO_ERR && !dlq_empty(&val->indexQ)) {
	*res = val_gen_index_chain(val->obj, copy);
    }

    if (*res != NO_ERR) {
	val_free_value(copy);
	copy = NULL;
    }

    return copy;

}  /* val_clone_test */


/********************************************************************
* FUNCTION val_clone_config_data
* 
* Clone a specified val_value_t struct and sub-trees
* Filter with the val_is_config_data callback function
*
* INPUTS:
*    val == config data value to clone
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*   clone of val, or NULL if a malloc failure
*********************************************************************/
val_value_t *
    val_clone_config_data (const val_value_t *val,
			   status_t *res)
{
    return val_clone_test(val, val_is_config_data, res);

}  /* val_clone_config_data */


/********************************************************************
* FUNCTION val_replace
* 
* Replace a specified val_value_t struct and sub-trees
*
* INPUTS:
*    val == value to clone from
*    copy == value to replace
*
* RETURNS:
*   status
*********************************************************************/
status_t
    val_replace (const val_value_t *val,
		 val_value_t *copy)
{
    const val_value_t *ch;
    val_value_t       *copych;
    status_t           res;

#ifdef DEBUG
    if (!val|| !copy) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    val_clean_value(copy);
    val_init_from_template(copy, val->obj);

    copy->editop = val->editop;
    copy->res = val->res;
    copy->flags = val->flags;

    /**** THIS MAY NEED TO CHANGE !!! ****/
    copy->parent = val->parent;

    /* copy meta-data */
    for (ch = (const val_value_t *)dlq_firstEntry(&val->metaQ);
	 ch != NULL;
	 ch = (const val_value_t *)dlq_nextEntry(ch)) {
	copych = val_clone(ch);
	if (!copych) {
	    return ERR_INTERNAL_MEM;
	}
	dlq_enque(copych, &copy->metaQ);
    }

    /* copy the actual value or children for complex types */
    res = NO_ERR;
    switch (val->btyp) {
    case NCX_BT_ENUM:
	VAL_ENUM(copy) = VAL_ENUM(val);
	VAL_ENUM_NAME(copy) = VAL_ENUM_NAME(val);
	break;
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
	copy->v.bool = val->v.bool;
	break;
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
	res = ncx_copy_num(&val->v.num, &copy->v.num, val->btyp);
	break;
    case NCX_BT_BINARY:
	ncx_clean_binary(&copy->v.binary);
	if (val->v.binary.ustr) {
	    copy->v.binary.ustr = m__getMem(val->v.binary.ustrlen);
	    if (!copy->v.binary.ustr) {
		res = ERR_INTERNAL_MEM;
	    } else {
		memcpy(copy->v.binary.ustr, val->v.binary.ustr,
		       val->v.binary.ustrlen);
		copy->v.binary.ustrlen = val->v.binary.ustrlen;
		copy->v.binary.ubufflen = val->v.binary.ustrlen;
	    }
	}
	break;
    case NCX_BT_STRING:	
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /*****/
	res = ncx_copy_str(&val->v.str, &copy->v.str, val->btyp);
	break;
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
	res = ncx_copy_list(&val->v.list, &copy->v.list);
	break;
    case NCX_BT_ANY:
    case NCX_BT_LIST:
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
	for (ch = (const val_value_t *)dlq_firstEntry(&val->v.childQ);
	     ch != NULL && res==NO_ERR;
	     ch = (const val_value_t *)dlq_nextEntry(ch)) {
	    copych = val_clone(ch);
	    if (!copych) {
		res = ERR_INTERNAL_MEM;
	    } else {
		val_add_child(copych, copy);
	    }
	}
	break;
    case NCX_BT_EXTERN:
	if (val->v.fname) {
	    copy->v.fname = xml_strdup(val->v.fname);
	    if (!copy->v.fname) {
		res = ERR_INTERNAL_MEM;
	    }
	}
	break;
    case NCX_BT_INTERN:
	if (val->v.intbuff) {
	    copy->v.intbuff = xml_strdup(val->v.intbuff);
	    if (!copy->v.intbuff) {
		res = ERR_INTERNAL_MEM;
	    }
	}
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* reconstruct index records if needed */
    if (res==NO_ERR && !dlq_empty(&val->indexQ)) {
	res = val_gen_index_chain(val->obj, copy);
    }

    return res;

}  /* val_replace */


/********************************************************************
* FUNCTION val_clear_editvars
* 
*   Clean the edit-config variables in the value node
*   Do not clear the dirty flag though
*
* INPUTS:
*    val == node to clear
*
*********************************************************************/
void
    val_clear_editvars (val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    val->curparent = NULL;
    val->editop = OP_EDITOP_NONE;
    val->insertop = OP_INSOP_NONE;
    if (val->insertstr) {
	m__free(val->insertstr);
	val->insertstr = NULL;
    }

}   /* val_clear_editvars */


/********************************************************************
* FUNCTION val_add_child
* 
*   Add a child value node to a parent value node
*
* INPUTS:
*    child == node to store in the parent
*    parent == complex value node with a childQ
*
*********************************************************************/
void
    val_add_child (val_value_t *child,
		   val_value_t *parent)
{
#ifdef DEBUG
    if (!child || !parent) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    child->parent = parent;
    dlq_enque(child, &parent->v.childQ);

}   /* val_add_child */


/********************************************************************
* FUNCTION val_add_child_clean
* 
*  Add a child value node to a parent value node
*  This is only called by the agent when adding nodes
*  to a target database.
*
*  If the child node being added is part of a choice/case,
*  then all sibling nodes in other cases within the same
*  choice will be deleted
*
*  The insert operation will also be check to see
*  if the child is a list oo a leaf-list, which is ordered-by user
*
*  The default insert mode is always 'last'
*
* INPUTS:
*    child == node to store in the parent
*    parent == complex value node with a childQ
*    cleanQ == address of Q to receive any deleted sibling nodes
*
* OUTPUTS:
*    cleanQ may have nodes added if the child being added
*    is part of a case.  All other cases will be deleted
*    from the parent Q and moved to the cleanQ
*
*********************************************************************/
void
    val_add_child_clean (val_value_t *child,
			 val_value_t *parent,
			 dlq_hdr_t *cleanQ)
{
    val_value_t  *testval, *nextval, *simval;
    boolean       doins, islist;
    status_t      res;

#ifdef DEBUG
    if (!child || !parent || !cleanQ) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (child->casobj) {
	for (testval = val_get_first_child(parent);
	     testval != NULL;
	     testval = nextval) {

	    nextval = val_get_next_child(testval);
	    if (testval->casobj && 
		(testval->casobj->parent == child->casobj->parent)) {

		if (testval->casobj != child->casobj) {
		    log_debug3("\nagt_val: clean old case member '%s'"
			       " from parent '%s'",
			       testval->name, parent->name);
		    dlq_remove(testval);
		    testval->parent = NULL;
		    dlq_enque(testval, cleanQ);
		}
	    }
	}
    }

    child->parent = parent;

    doins = FALSE;
    if (child->obj->objtype == OBJ_TYP_LIST) {
	doins = TRUE;
	islist = TRUE;
    } else if (child->obj->objtype == OBJ_TYP_LEAF_LIST) {
	doins = TRUE;
	islist = FALSE;
    }
    if (doins && child->insertop != OP_INSOP_NONE) {
	switch (child->insertop) {
	case OP_INSOP_FIRST:
	    testval = val_find_child(parent, 
				     obj_get_mod_name(child->obj),
				     child->name);
	    if (testval) {
		dlq_insertAhead(child, testval);
	    } else {
		dlq_enque(child, &parent->v.childQ);
	    }
	    break;
	case OP_INSOP_LAST:
	case OP_INSOP_NONE:
	    dlq_enque(child, &parent->v.childQ);
	    break;
	case OP_INSOP_BEFORE:
	case OP_INSOP_AFTER:
	    /* find the entry specified by the val->insertstr value
	     * this is value='foo' for leaf-list and
	     * key="[x:foo='bar'][x:foo2=7]" for list
	     */
	    if (child->obj->objtype == OBJ_TYP_LEAF_LIST) {
		simval = val_make_simval(child->typdef,
					 child->nsid,
					 child->name,
					 child->insertstr,
					 &res);
		if (res != NO_ERR) {
		    SET_ERROR(res);
		    dlq_enque(child, &parent->v.childQ);
		} else {
		    testval = 
			val_first_child_match(child->curparent,
					      simval);
		    if (!testval) {
			/* sibling leaf-list with the specified
			 * value was not found
			 */
			SET_ERROR(ERR_NCX_INSERT_MISSING_INSTANCE);
			dlq_enque(child, &parent->v.childQ);
		    } else if (child->insertop == OP_INSOP_BEFORE) {
			dlq_insertAhead(child, testval);
		    } else {
			dlq_insertAfter(child, testval);
		    }
		}
		    
		if (simval) {
		    val_free_value(simval);
		}
	    } else if (child->obj->objtype == OBJ_TYP_LIST) {
		/***** TBD ******/
		dlq_enque(child, &parent->v.childQ);
	    } else {
		/* wrong object type */
		SET_ERROR(ERR_INTERNAL_VAL);
		dlq_enque(child, &parent->v.childQ);
	    }
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    dlq_enque(child, &parent->v.childQ);
	}
    } else {
	dlq_enque(child, &parent->v.childQ);
    }

}   /* val_add_child_clean */


/********************************************************************
* FUNCTION val_insert_child
* 
*   Insert a child value node to a parent value node
*
* INPUTS:
*    child == node to store in the parent
*    current == current node to insert after; 
*               NULL to make new first entry
*    parent == complex value node with a childQ
*
*********************************************************************/
void
    val_insert_child (val_value_t *child,
		      val_value_t *current,
		      val_value_t *parent)
{
#ifdef DEBUG
    if (!child || !parent) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    child->parent = parent;
    if (current) {
	dlq_insertAfter(child, current);
    } else {
	dlq_insertAfter(child, &parent->v.childQ);
    }

}   /* val_insert_child */


/********************************************************************
* FUNCTION val_remove_child
* 
*   Remove a child value node from its parent value node
*
* INPUTS:
*    child == node to store in the parent
*    parent == complex value node with a childQ
*
*********************************************************************/
void
    val_remove_child (val_value_t *child)
{
#ifdef DEBUG
    if (!child) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    dlq_remove(child);
    child->parent = NULL;

}   /* val_remove_child */


/********************************************************************
* FUNCTION val_swap_child
* 
*   Swap a child value node with a current value node
*
* INPUTS:
*    newchild == node to store in the place of the curchild
*    curchild == node to remove from the parent
*
*********************************************************************/
void
    val_swap_child (val_value_t *newchild,
		    val_value_t *curchild)
{
#ifdef DEBUG
    if (!newchild || !curchild) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    newchild->parent = curchild->parent;
    newchild->getcb = curchild->getcb;

    dlq_swap(newchild, curchild);

    curchild->parent = NULL;

}   /* val_swap_child */


/********************************************************************
* FUNCTION val_first_child_match
* 
* Get the first instance of the corresponding child node 
* 
* INPUTS:
*    parent == parent value to check
*    child == child value to find (e.g., from a NETCONF PDU) 
*
* RETURNS:
*   pointer to the child if found or NULL if not found
*********************************************************************/
val_value_t *
    val_first_child_match (val_value_t  *parent,
			   val_value_t *child)
{
    val_value_t *val;

#ifdef DEBUG
    if (!parent || !child) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return NULL;
    }

    for (val = (val_value_t *)dlq_firstEntry(&parent->v.childQ);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {

	/* check the node if the QName matches */
	if (val->nsid == child->nsid &&
	    !xml_strcmp(val->name, child->name)) {

	    if (val->btyp == NCX_BT_LIST) {
		/* match the instance identifiers, if any */
		if (val_index_match(child, val)) {
		    return val;
		}
	    } else if (val->obj->objtype == OBJ_TYP_LEAF_LIST) {
		/* find the leaf-list with the same value */
		if (!val_compare(val, child)) {
		    return val;
		}
	    } else {
		/* can only be this one instance */
		return val;
	    }
	}
    }

    return NULL;

}  /* val_first_child_match */


/********************************************************************
* FUNCTION val_get_first_child
* 
* Get the child node
* 
* INPUTS:
*    parent == parent complex type to check
*
* RETURNS:
*   pointer to the child if found or NULL if not found
*********************************************************************/
val_value_t *
    val_get_first_child (const val_value_t  *parent)
{
#ifdef DEBUG
    if (!parent) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return NULL;
    }

    return (val_value_t *)dlq_firstEntry(&parent->v.childQ);

}  /* val_get_first_child */


/********************************************************************
* FUNCTION val_get_next_child
* 
* Get the next child node
* 
* INPUTS:
*    curchild == current child node
*
* RETURNS:
*   pointer to the next child if found or NULL if not found
*********************************************************************/
val_value_t *
    val_get_next_child (const val_value_t  *curchild)
{
#ifdef DEBUG
    if (!curchild) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (val_value_t *)dlq_nextEntry(curchild);

}  /* val_get_next_child */


/********************************************************************
* FUNCTION val_find_child
* 
* Find the first instance of the specified child node
*
* INPUTS:
*    parent == parent complex type to check
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    childname == name of child node to find
*
* RETURNS:
*   pointer to the child if found or NULL if not found
*********************************************************************/
val_value_t *
    val_find_child (const val_value_t  *parent,
		    const xmlChar *modname,
		    const xmlChar *childname)
{
    val_value_t *val;

#ifdef DEBUG
    if (!parent || !childname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return NULL;
    }

    for (val = (val_value_t *)dlq_firstEntry(&parent->v.childQ);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {
	if (modname && 
	    xml_strcmp(modname, 
		       obj_get_mod_name(val->obj))) {
	    continue;
	}
	if (!xml_strcmp(val->name, childname)) {
	    return val;
	}
    }
    return NULL;

}  /* val_find_child */



/********************************************************************
* FUNCTION val_match_child
* 
* Match the first instance of the specified child node
*
* INPUTS:
*    parent == parent complex type to check
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    childname == name of child node to find
*
* RETURNS:
*   pointer to the child if found or NULL if not found
*********************************************************************/
val_value_t *
    val_match_child (const val_value_t  *parent,
		     const xmlChar *modname,
		     const xmlChar *childname)
{
    val_value_t *val;

#ifdef DEBUG
    if (!parent || !childname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return NULL;
    }

    for (val = (val_value_t *)dlq_firstEntry(&parent->v.childQ);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {
	if (modname && 
	    xml_strcmp(modname, obj_get_mod_name(val->obj))) {
	    continue;
	}
	if (!xml_strncmp(val->name, childname,
			 xml_strlen(childname))) {
	    return val;
	}
    }
    return NULL;

}  /* val_match_child */


/********************************************************************
* FUNCTION val_find_next_child
* 
* Find the next instance of the specified child node
* 
* INPUTS:
*    parent == parent complex type to check
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    childname == name of child node to find
*    curchild == current child of this object type to start search
*
* RETURNS:
*   pointer to the child if found or NULL if not found
*********************************************************************/
val_value_t *
    val_find_next_child (const val_value_t  *parent,
			 const xmlChar *modname,
			 const xmlChar *childname,
			 const val_value_t *curchild)
{
    val_value_t *val;

#ifdef DEBUG
    if (!parent || !childname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return NULL;
    }

    for (val = (val_value_t *)dlq_nextEntry(curchild);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {
	if (modname && 
	    xml_strcmp(modname, 
		       obj_get_mod_name(val->obj))) {
	    continue;
	}
	if (!xml_strcmp(val->name, childname)) {
	    return val;
	}
    }
    return NULL;

}  /* val_find_next_child */



/********************************************************************
* FUNCTION val_first_child_name
* 
* Get the first corresponding child node instance, by name
* 
* INPUTS:
*    parent == parent complex type to check
*    name == child name to find
*
* RETURNS:
*   pointer to the FIRST match if found, or NULL if not found
*********************************************************************/
val_value_t *
    val_first_child_name (val_value_t  *parent,
			  const xmlChar *name)
{
    val_value_t *val;

#ifdef DEBUG
    if (!parent || !name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return NULL;
    }
	
    for (val = (val_value_t *)dlq_firstEntry(&parent->v.childQ);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {

	/* check the node if the name matches */
	if (!xml_strcmp(val->name, name)) {
	    return val;
	}
    }

    return NULL;

}  /* val_first_child_name */


/********************************************************************
* FUNCTION val_first_child_string
* 
* Get the first corresponding child node instance, by name
* and by string value.
* Child node must be a base type of 
*   NCX_BT_STRING
*   NCX_BT_INSTANCE_ID
*   NCX_BT_KEYREF
*
* INPUTS:
*    parent == parent complex type to check
*    name == child name to find
*    strval == string value to find 
* RETURNS:
*   pointer to the FIRST match if found, or NULL if not found
*********************************************************************/
val_value_t *
    val_first_child_string (val_value_t  *parent,
			    const xmlChar *name,
			    const xmlChar *strval)
{
    val_value_t *val;

#ifdef DEBUG
    if (!parent || !name || !strval) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return NULL;
    }
	
    for (val = (val_value_t *)dlq_firstEntry(&parent->v.childQ);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {

	/* check the node if the name matches */
	if (!xml_strcmp(val->name, name)) {
	    if (typ_is_string(val->btyp)) {
		if (!xml_strcmp(val->v.str, strval)) {
		    return val;
		}
	    } else {
		/* requested child node is wrong type */
		return NULL;
	    }
	}
    }

    return NULL;

}  /* val_first_child_string */


/********************************************************************
* FUNCTION val_child_cnt
* 
* Get the number of child nodes present
* 
* INPUTS:
*    parent == parent complex type to check
*
* RETURNS:
*   the number of child nodes found
*********************************************************************/
uint32
    val_child_cnt (val_value_t  *parent)
{
    val_value_t *val;
    uint32       cnt;

#ifdef DEBUG
    if (!parent) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return 0;
    }
	
    cnt = 0;
    for (val = (val_value_t *)dlq_firstEntry(&parent->v.childQ);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {
	cnt++;
    }
    return cnt;

}  /* val_child_cnt */


/********************************************************************
* FUNCTION val_child_inst_cnt
* 
* Get the corresponding child instance count by name
* 
* INPUTS:
*    parent == parent complex type to check
*    modname == module name defining the child (may be NULL)
*    name == child name to find and count
*
* RETURNS:
*   the instance count
*********************************************************************/
uint32
    val_child_inst_cnt (const val_value_t  *parent,
			const xmlChar *modname,
			const xmlChar *name)
{
    const val_value_t *val;
    uint32       cnt;

#ifdef DEBUG
    if (!parent || !name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    if (!typ_has_children(parent->btyp)) {
	return 0;
    }
	
    cnt = 0;
    for (val = (const val_value_t *)dlq_firstEntry(&parent->v.childQ);
	 val != NULL;
	 val = (const val_value_t *)dlq_nextEntry(val)) {

	if (modname &&
	    xml_strcmp(modname,
		       obj_get_mod_name(val->obj))) {
	    continue;
	}

	/* check the node if the name matches */
	if (!xml_strcmp(val->name, name)) {
	    cnt++;
	} 
    }

    return cnt;

}  /* val_child_inst_cnt */


/********************************************************************
* FUNCTION val_find_all_children
* 
* Find all occurances of the specified node(s)
* within the children of the current node. 
* The walker fn will be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* INPUTS:
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startnode == node to check
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    name == name of child node to find
*              == NULL to match any child name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if childname == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    val_find_all_children (val_walker_fn_t walkerfn,
			   void *cookie1,
			   void *cookie2,
			   val_value_t *startnode,
			   const xmlChar *modname,
			   const xmlChar *name,
			   boolean configonly,
			   boolean textmode)
{
    val_value_t *val;
    boolean      fnresult, fncalled;

#ifdef DEBUG
    if (!startnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (!typ_has_children(startnode->btyp)) {
	return FALSE;
    }

    for (val = (val_value_t *)dlq_firstEntry(&startnode->v.childQ);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {

	fnresult = process_one_valwalker(walkerfn,
					 cookie1,
					 cookie2,
					 val, 
					 modname,
					 name,
					 configonly,
					 textmode,
					 &fncalled);
	if (!fnresult) {
	    return FALSE;
	}
    }
    return TRUE;

}  /* val_find_all_children */


/********************************************************************
* FUNCTION val_find_all_ancestors
* 
* Find all the ancestor instances of the specified node
* within the path to root from the current node;
* use the filter criteria provided
*
* INPUTS:
*
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startnode == node to start search at
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    name == name of ancestor node to find
*              == NULL to match any node name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if name == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    orself == TRUE if axis is really ancestor-or-self
*              FALSE if axis is ancestor
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    val_find_all_ancestors (val_walker_fn_t  walkerfn,
			    void *cookie1,
			    void *cookie2,
			    val_value_t *startnode,
			    const xmlChar *modname,
			    const xmlChar *name,
			    boolean configonly,
			    boolean textmode,
			    boolean orself)
{
    val_value_t *val;
    boolean      fnresult, fncalled;

#ifdef DEBUG
    if (!startnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (orself) {
	val = startnode;
    } else {
	val = startnode->parent;
    }

    while (val) {
	fnresult = process_one_valwalker(walkerfn,
					 cookie1,
					 cookie2,
					 val, 
					 modname,
					 name,
					 configonly,
					 textmode,
					 &fncalled);
	if (!fnresult) {
	    return FALSE;
	}
	val = val->parent;
    }
    return TRUE;

}  /* val_find_all_ancestors */


/********************************************************************
* FUNCTION val_find_all_descendants
* 
* Find all occurances of the specified node
* within the current subtree. The walker fn will
* be called for each match.  
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* INPUTS:
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startnode == startnode complex type to check
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    name == name of descendent node to find
*              == NULL to match any node name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if decname == NULL
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    orself == TRUE if axis is really ancestor-or-self
*              FALSE if axis is ancestor
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    val_find_all_descendants (val_walker_fn_t walkerfn,
			      void *cookie1,
			      void *cookie2,
			      val_value_t *startnode,
			      const xmlChar *modname,
			      const xmlChar *name,
			      boolean configonly,
			      boolean textmode,
			      boolean orself)
{
    val_value_t *val;
    boolean      fncalled, fnresult;

#ifdef DEBUG
    if (!startnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (orself) {
	fnresult = process_one_valwalker(walkerfn,
					 cookie1,
					 cookie2,
					 startnode,
					 modname, 
					 name,
					 configonly,
					 textmode,
					 &fncalled);
	if (!fnresult) {
	    return FALSE;
	}
    }

    if (!typ_has_children(startnode->btyp)) {
	return TRUE;
    }

    for (val = (val_value_t *)dlq_firstEntry(&startnode->v.childQ);
	 val != NULL;
	 val = (val_value_t *)dlq_nextEntry(val)) {

	fncalled = FALSE;
	fnresult = process_one_valwalker(walkerfn,
					 cookie1,
					 cookie2,
					 val, 
					 modname,
					 name,
					 configonly,
					 textmode,
					 &fncalled);
	if (!fnresult) {
	    return FALSE;
	}
	if (!fncalled) {
	    fnresult = val_find_all_descendants(walkerfn,
						cookie1,
						cookie2,
						val,
						modname,
						name, 
						configonly,
						textmode,
						FALSE);
	    if (!fnresult) {
		return FALSE;
	    }
	}
    }
    return TRUE;

}  /* val_find_all_descendants */


/********************************************************************
* FUNCTION val_find_all_pfaxis
* 
* Find all occurances of the specified node
* for the specified preceding or following axis
*
*   preceding::*
*   following::*
*
* within the current subtree. The walker fn will
* be called for each match.  Because the callbacks
* will be done in sequential order, starting from
* the 
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* INPUTS:
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    topnode == topnode used as the relative root for
*               calculating node position
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    name == name of preceding or following node to find
*              == NULL to match any node name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if decname == NULL
*    dblslash == TRUE if all decendents of the preceding
*                 or following nodes should be checked
*                FALSE only 1 level is checked, not their descendants
*    textmode == TRUE if just testing for text() nodes
*                name modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    axis == axis enum to use
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    val_find_all_pfaxis (val_walker_fn_t  walkerfn,
			 void *cookie1,
			 void *cookie2,
			 val_value_t  *startnode,
			 const xmlChar *modname,
			 const xmlChar *name,
			 boolean configonly,
			 boolean dblslash,
			 boolean textmode,
			 ncx_xpath_axis_t axis)
{
    val_value_t      *val, *child;
    boolean           fncalled, fnresult, forward;

#ifdef DEBUG
    if (!startnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    /* check the Q containing the startnode
     * for preceding or following nodes;
     * could be sibling node check or any node check
     */
    switch (axis) {
    case XP_AX_PRECEDING:
	forward = FALSE;
	val = (val_value_t *)dlq_prevEntry(startnode);
	break;
    case XP_AX_FOLLOWING:
	forward = TRUE;
	val = (val_value_t *)dlq_nextEntry(startnode);
	break;
    case XP_AX_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    while (val) {
	if (configonly && !name && !obj_is_config(val->obj)) {
	    if (forward) {
		val = (val_value_t *)dlq_nextEntry(val);
	    } else {
		val = (val_value_t *)dlq_prevEntry(val);
	    }
	    continue;
	}

	fnresult = process_one_valwalker(walkerfn,
					 cookie1,
					 cookie2,
					 val, 
					 modname,
					 name,
					 configonly,
					 textmode,
					 &fncalled);
	if (!fnresult) {
	    return FALSE;
	}

	if (!fncalled && dblslash) {
	    /* if /foo did not get added, than 
	     * try /foo/bar, /foo/baz, etc.
	     * check all the child nodes even if
	     * one of them matches, because all
	     * matches are needed with the '//' operator
	     */
	    for (child = val_get_first_child(val);
		 child != NULL;
		 child = val_get_next_child(child)) {

		fnresult = 
		    val_find_all_pfaxis(walkerfn, 
					cookie1, 
					cookie2,
					child, 
					modname, 
					name, 
					configonly,
					dblslash,
					textmode,
					axis);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}

	if (forward) {
	    val = (val_value_t *)dlq_nextEntry(val);
	} else {
	    val = (val_value_t *)dlq_prevEntry(val);
	}
    }
    return TRUE;

}  /* val_find_all_pfaxis */


/********************************************************************
* FUNCTION val_find_all_pfsibling_axis
* 
* Find all occurances of the specified node
* for the specified axis
*
*   preceding-sibling::*
*   following-sibling::*
*
* within the current subtree. The walker fn will
* be called for each match.  Because the callbacks
* will be done in sequential order, starting from
* the 
*
* If the walker function returns TRUE, then the 
* walk will continue; If FALSE it will terminate right away
*
* INPUTS:
*    walkerfn == callback function to use
*    cookie1 == cookie1 value to pass to walker fn
*    cookie2 == cookie2 value to pass to walker fn
*    startnode == starting sibling node to check
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    name == name of preceding or following node to find
*              == NULL to match any node name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if decname == NULL
*    dblslash == TRUE if all decendents of the preceding
*                 or following nodes should be checked
*                FALSE only 
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    axis == axis enum to use
*
* RETURNS:
*   TRUE if normal termination occurred
*   FALSE if walker fn requested early termination
*********************************************************************/
boolean
    val_find_all_pfsibling_axis (val_walker_fn_t  walkerfn,
				 void *cookie1,
				 void *cookie2,
				 val_value_t  *startnode,
				 const xmlChar *modname,
				 const xmlChar *name,
				 boolean configonly,
				 boolean dblslash,
				 boolean textmode,
				 ncx_xpath_axis_t axis)
{
    val_value_t *val, *child;
    boolean      fncalled, fnresult, forward;

#ifdef DEBUG
    if (!startnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    /* check the Q containing the startnode
     * for preceding or following nodes;
     */
    switch (axis) {
    case XP_AX_PRECEDING_SIBLING:
	/* execute the callback for all preceding nodes
	 * that match the filter criteria 
	 */
	val = (val_value_t *)dlq_prevEntry(startnode);
	forward = FALSE;
	break;
    case XP_AX_FOLLOWING_SIBLING:
	val = (val_value_t *)dlq_nextEntry(startnode);
	forward = TRUE;
	break;
    case XP_AX_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    while (val) {
	if (configonly && !name && !obj_is_config(val->obj)) {
	    if (forward) {
		val = (val_value_t *)dlq_nextEntry(val);
	    } else {
		val = (val_value_t *)dlq_prevEntry(val);
	    }
	    continue;
	}

	fnresult = process_one_valwalker(walkerfn,
					 cookie1,
					 cookie2,
					 val, 
					 modname,
					 name,
					 configonly,
					 textmode,
					 &fncalled);
	if (!fnresult) {
	    return FALSE;
	}

	if (!fncalled && dblslash) {
	    /* if /foo did not get added, than 
	     * try /foo/bar, /foo/baz, etc.
	     * check all the child nodes even if
	     * one of them matches, because all
	     * matches are needed with the '//' operator
	     */
	    for (child = val_get_first_child(val);
		 child != NULL;
		 child = val_get_next_child(child)) {

		fnresult = 
		    val_find_all_pfsibling_axis(walkerfn, 
						cookie1, 
						cookie2,
						child, 
						modname, 
						name, 
						configonly,
						dblslash,
						textmode,
						axis);
		if (!fnresult) {
		    return FALSE;
		}
	    }
	}

	if (forward) {
	    val = (val_value_t *)dlq_nextEntry(val);
	} else {
	    val = (val_value_t *)dlq_prevEntry(val);
	}
    }
    return TRUE;

}  /* val_find_all_pfsibling_axis */


/********************************************************************
* FUNCTION val_get_axisnode
* 
* Find the specified node based on the context node,
* a context position and an axis
*
*   ancestor::*
*   ancestor-or-self::*
*   child::*
*   descendant::*
*   descendant-or-self::*
*   following::*
*   following-sibling::*
*   preceding::*
*   preceding-sibling::*
*   self::*
*
* INPUTS:
*    startnode == context node to run tests from
*    modname == module name; 
*                the first match in this module namespace
*                will be returned
*            == NULL:
*                 the first match in any namespace will
*                 be  returned;
*    name == name of preceding or following node to find
*              == NULL to match any node name
*    configonly == TRUE to skip over non-config nodes
*                  FALSE to check all nodes
*                  Only used if decname == NULL
*    dblslash == TRUE if all decendents of the preceding
*                 or following nodes should be checked
*                FALSE only 
*    textmode == TRUE if just testing for text() nodes
*                name and modname will be ignored in this mode
*                FALSE if using name and modname to filter
*    axis == axis enum to use
*    position == position to find in the specified axis
*
* RETURNS:
*   pointer to found value or NULL if none
*********************************************************************/
val_value_t *
    val_get_axisnode (val_value_t *startnode,
		      const xmlChar *modname,
		      const xmlChar *name,
		      boolean configonly,
		      boolean dblslash,
		      boolean textmode,
		      ncx_xpath_axis_t axis,
		      int64 position)
{
    finderparms_t    finderparms;
    boolean          fnresult, fncalled, orself;

#ifdef DEBUG
    if (!startnode) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (position <= 0) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    orself = FALSE;
    memset(&finderparms, 0x0, sizeof(finderparms_t));
    finderparms.findpos = position;

    /* check the Q containing the startnode
     * for preceding or following nodes;
     * could be sibling node check or any node check
     */
    switch (axis) {
    case XP_AX_ANCESTOR_OR_SELF:
	orself = TRUE;
	/* fall through */
    case XP_AX_ANCESTOR:
	fnresult = val_find_all_ancestors(position_walker,
					  &finderparms,
					  NULL,
					  startnode,
					  modname,
					  name,
					  configonly,
					  textmode,
					  orself);
	if (fnresult) {
	    return NULL;
	} else {
	    return finderparms.foundval;
	}
    case XP_AX_ATTRIBUTE:
	/* TBD: attributes not supported */
	return NULL;
    case XP_AX_CHILD:
	fnresult = val_find_all_children(position_walker,
					 &finderparms,
					 NULL,
					 startnode,
					 modname,
					 name,
					 configonly,
					 textmode);
	if (fnresult) {
	    return NULL;
	} else {
	    return finderparms.foundval;
	}
    case XP_AX_DESCENDANT_OR_SELF:
	orself = TRUE;
	/* fall through */
    case XP_AX_DESCENDANT:
	fnresult = val_find_all_descendants(position_walker,
					    &finderparms,
					    NULL,
					    startnode,
					    modname,
					    name,
					    configonly,
					    textmode,
					    orself);
	if (fnresult) {
	    return NULL;
	} else {
	    return finderparms.foundval;
	}
    case XP_AX_PRECEDING:
    case XP_AX_FOLLOWING:
	fnresult = val_find_all_pfaxis(position_walker,
				       &finderparms,
				       NULL,
				       startnode,
				       modname,
				       name,
				       configonly,
				       dblslash,
				       textmode,
				       axis);
	if (fnresult) {
	    return NULL;
	} else {
	    return finderparms.foundval;
	}
    case XP_AX_PRECEDING_SIBLING:
    case XP_AX_FOLLOWING_SIBLING:
	fnresult = val_find_all_pfsibling_axis(position_walker,
					       &finderparms,
					       NULL,
					       startnode,
					       modname,
					       name,
					       configonly,
					       dblslash,
					       textmode,
					       axis);
	if (fnresult) {
	    return NULL;
	} else {
	    return finderparms.foundval;
	}
    case XP_AX_NAMESPACE:
	return NULL;
    case XP_AX_PARENT:
	if (!startnode->parent) {
	    return NULL;
	}
	/* there can only be one node in this axis,
	 * so if the startnode isn't it, then return NULL
	 */
	fnresult = process_one_valwalker(position_walker,
					 &finderparms,
					 NULL,
					 startnode->parent,
					 modname,
					 name,
					 configonly,
					 textmode,
					 &fncalled);
	if (fnresult) {
	    return NULL;
	} else {
	    return finderparms.foundval;
	}
    case XP_AX_NONE:
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    /*NOTREACHED*/

}  /* val_get_axisnode */


/********************************************************************
* FUNCTION val_get_child_inst_id
* 
* Get the instance ID for this child node within the parent context
* 
* INPUTS:
*    parent == parent complex type to check
*    child == child node to find ID for
*
* RETURNS:
*   the instance ID num (1 .. N), or 0 if some error
*********************************************************************/
uint32
    val_get_child_inst_id (const val_value_t  *parent,
			   const val_value_t *child)
{
    const val_value_t *val;
    uint32             cnt;

#ifdef DEBUG
    if (!parent || !child) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
    if (!typ_has_children(parent->btyp)) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return 0;
    }
#endif

    cnt = 0;
    for (val = (const val_value_t *)dlq_firstEntry(&parent->v.childQ);
	 val != NULL;
	 val = (const val_value_t *)dlq_nextEntry(val)) {

	if (xml_strcmp(obj_get_mod_name(child->obj),
		       obj_get_mod_name(val->obj))) {
	    continue;
	}

	/* check the node if the name matches */
	if (!xml_strcmp(val->name, child->name)) {
	    cnt++;
	    if (val == child) {
		return cnt;
	    }
	}
    }

    SET_ERROR(ERR_INTERNAL_VAL);
    return 0;

}  /* val_get_child_inst_id */


/********************************************************************
* FUNCTION val_liststr_count
* 
* Get the number of strings in the list type
* 
* INPUTS:
*    val == value to check
*
* RETURNS:
*  number of list entries; also zero for error
*********************************************************************/
uint32
    val_liststr_count (const val_value_t  *val)
{
    uint32             cnt;

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    cnt = 0;
    switch (val->btyp) {
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
	cnt = ncx_list_cnt(&val->v.list);
	break;
    default:
        SET_ERROR(ERR_NCX_WRONG_TYPE);
    }

    return cnt;

}  /* val_liststr_count */




/********************************************************************
* FUNCTION val_index_match
* 
* Check 2 val_value structs for the same instance ID
* 
* The node data types must match, and must be
*    NCX_BT_LIST
*
* All index components must exactly match.
* 
* INPUTS:
*    val1 == first value to index match
*    val2 == second value to index match
*
* RETURNS:
*   TRUE if the index chains match
*********************************************************************/
boolean
    val_index_match (const val_value_t *val1,
		     const val_value_t *val2)
{
    int32 ret;

    ret = index_match(val1, val2);
    return (ret) ? FALSE : TRUE;

}  /* val_index_match */


/********************************************************************
* FUNCTION val_compare
* 
* Compare 2 val_value_t struct value contents
*
* Handles NCX_CL_BASE and NCX_CL_SIMPLE data classes
* by comparing the simple value.
*
* Handle NCX_CL_COMPLEX by checking the index if needed
* and then checking all the child nodes recursively
*
* !!!! Meta-value contents are ignored for this test !!!!
* 
* INPUTS:
*    val1 == first value to check
*    val2 == second value to check
*
* RETURNS:
*   compare result
*     -1: val1 is less than val2 (if complex just different or error)
*      0: val1 is the same as val2 
*      1: val1 is greater than val2
*********************************************************************/
int32
    val_compare (const val_value_t *val1,
		 const val_value_t *val2)
{
    ncx_btype_t  btyp;
    val_value_t *ch1, *ch2;
    int32 ret;

#ifdef DEBUG
    if (!val1 || !val2) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return -1;
    }
    if (val1->btyp != val2->btyp) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return -1;
    }
#endif

    if (val1->btyp == NCX_BT_UNION) {
	btyp = val1->unbtyp;
    } else {
	btyp = val1->btyp;
    }

    switch (btyp) {
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
	if (val1->v.bool == val2->v.bool) {
	    return 0;
	} else if (val1->v.bool) {
	    return 1;
	} else {
	    return -1;
	}
	/*NOTREACHED*/
    case NCX_BT_ENUM:
	if (VAL_ENUM(val1) == VAL_ENUM(val2)) {
	    return 0;
	} else if (VAL_ENUM(val1) < VAL_ENUM(val2)) {
	    return -1;
	} else {
	    return 1;
	}
	/*NOTREACHED*/
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
	return ncx_compare_nums(&val1->v.num, &val2->v.num, btyp);
    case NCX_BT_BINARY:
	if (!val1->v.binary.ustr) {
	    return -1;
	} else if (!val2->v.binary.ustr) {
	    return 1;
	} else if (val1->v.binary.ustrlen <
		   val2->v.binary.ustrlen) {
	    return -1;
	} else if (val1->v.binary.ustrlen >
		   val2->v.binary.ustrlen) {
	    return 1;
	} else {
	    return memcmp(val1->v.binary.ustr,
			  val2->v.binary.ustr,
			  val1->v.binary.ustrlen);
	}
	/*NOTREACHED*/
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /*****/
	return ncx_compare_strs(&val1->v.str, &val2->v.str, btyp);
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
	return ncx_compare_lists(&val1->v.list, &val2->v.list);
    case NCX_BT_LIST:
	ret = index_match(val1, val2);
	if (ret) {
	    return ret;
	} /* else drop though and check values */
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
	ch1 = (val_value_t *)dlq_firstEntry(&val1->v.childQ);
	ch2 = (val_value_t *)dlq_firstEntry(&val2->v.childQ);
	for (;;) {
	    /* check if both child nodes exist */
	    if (!ch1 && !ch2) {
		return 0;
	    } else if (!ch1) {
		return -1;
	    } else if (!ch2) {
		return 1;
	    }
	    
	    /* check if both child nodes have the same name */
	    ret = xml_strcmp(ch1->name, ch2->name);
	    if (ret) {
		return ret;
	    } else {
		/* check if they have same value */
		ret = val_compare(ch1, ch2);
		if (ret) {
		    return ret;
		}
	    }

	    /* get the next pair of child nodes to check */
	    ch1 = (val_value_t *)dlq_nextEntry(ch1);
	    ch2 = (val_value_t *)dlq_nextEntry(ch2);
	}
	/*NOTREACHED*/
    case NCX_BT_EXTERN:
	SET_ERROR(ERR_INTERNAL_VAL);
	return -1;
    case NCX_BT_INTERN:
	SET_ERROR(ERR_INTERNAL_VAL);
	return -1;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return -1;
    }
    /*NOTREACHED*/

}  /* val_compare */


/********************************************************************
* FUNCTION val_sprintf_simval_nc
* 
* Sprintf the xmlChar string NETCONF representation of a simple value
*
* buff is allowed to be NULL; if so, then this fn will
* just return the length of the string (w/o EOS ch) 
*
* USAGE:
*  call 1st time with a NULL buffer to get the length
*  call the 2nd time with a buffer of the returned length
*
* !!!! DOES NOT CHECK BUFF OVERRUN IF buff is non-NULL !!!!
*
* INPUTS:
*    buff == buffer to write (NULL means get length only)
*    val == value to check
*
* OUTPUTS:
*   *len == number of bytes written (or just length if buff == NULL)
*
* RETURNS:
*   status
*********************************************************************/
status_t
    val_sprintf_simval_nc (xmlChar *buff,
			   const val_value_t *val,
			   uint32 *len)
{
    const ncx_lmem_t  *lmem;
    const xmlChar     *s;
    ncx_btype_t        btyp;
    status_t           res;
    int32              icnt;
    uint32             bufflen;
    char               numbuff[VAL_MAX_NUMLEN];

#ifdef DEBUG
    if (!val || !len) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    btyp = val->btyp;

    switch (btyp) {
    case NCX_BT_EMPTY:
	/* flag is element name : <foo/>  */
	if (val->v.bool) {
	    if (buff) {
		if ((xml_strlen(val->name)+3) < bufflen) {
		    icnt = sprintf((char *)buff, "<%s/>", val->name);
		    if (icnt < 0) {
			return SET_ERROR(ERR_INTERNAL_VAL);
		    } else {
			*len = (uint32)icnt;
		    }
		}
	    } else {
		*len = xml_strlen(val->name) + 3;
	    }
	} else {
	    *len = 0;
	}
	break;
    case NCX_BT_BOOLEAN:
	if (val->v.bool) {
	    if (buff) {
		sprintf((char *)buff, "true");
	    }
	    *len = 4;
	} else {
	    if (buff) {
		sprintf((char *)buff, "false");
	    }
	    *len = 5;
	}
	break;
    case NCX_BT_ENUM:
	if (buff) {
	    if (val->v.enu.name) {
		icnt = sprintf((char *)buff, "%s", val->v.enu.name);
		if (icnt < 0 ) {
		    return SET_ERROR(ERR_INTERNAL_VAL);
		} else {
		    *len = (uint32)icnt;
		}
	    } else {
		*len = 0;
	    }
	} else if (val->v.enu.name) {
	    *len = xml_strlen(val->v.enu.name);
	} else {
	    *len = 0;
	}
	break;
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
	res = ncx_sprintf_num(buff, &val->v.num, btyp, len);
	if (res != NO_ERR) {
	    return SET_ERROR(res);
	}
	break;
    case NCX_BT_STRING:	
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:  /****/
	s = VAL_STR(val);
	if (buff) {
	    if (s) {
		*len = xml_strcpy(buff, s);
	    } else {
		*len = 0;
	    }
	} else {
	    *len = (s) ? xml_strlen(s) : 0;
	}
	break;
    case NCX_BT_BINARY:
	s = val->v.binary.ustr;
	if (buff) {
	    if (s) {
		/* !!! do not know the real buffer length
		 * !!! to send; assume call to this fn
		 * !!! to retrieve the length was done OK
		 */
		res = b64_encode(s, val->v.binary.ustrlen,
				 buff, NCX_MAX_UINT,
				 NCX_DEF_LINELEN, len);
	    } else {
		*len = 0;
	    }
	} else if (s) {
	    res = b64_encode(s, val->v.binary.ustrlen,
			     NULL, NCX_MAX_UINT,
			     NCX_DEF_LINELEN, len);
	} else {
	    *len = 0;
	}
	break;
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
	*len = 0;
	for (lmem = (const ncx_lmem_t *)dlq_firstEntry(&val->v.list.memQ);
	     lmem != NULL;
	     lmem = (const ncx_lmem_t *)dlq_nextEntry(lmem)) {

	    if (typ_is_string(val->v.list.btyp)) {
		s = lmem->val.str;
	    } else if (typ_is_number(val->v.list.btyp)) {
		res = ncx_sprintf_num((xmlChar *)numbuff, 
				      &lmem->val.num, 
				      val->v.list.btyp, len);
		if (res != NO_ERR) {
		    return SET_ERROR(res);
		}
		s = (const xmlChar *)numbuff;
	    } else {
		switch (val->v.list.btyp) {
		case NCX_BT_ENUM:
		    s = VAL_ENUM_NAME(val);
		    break;
		case NCX_BT_BOOLEAN:
		    if (val->v.bool) {
			s = NCX_EL_TRUE;
		    } else {
			s = NCX_EL_FALSE;
		    }
		    break;
		default:
		    s = NULL;
		}
	    }

	    if (buff) {
		/* hardwire double quotes to wrapper list strings */
		icnt = sprintf((char *)buff, "\"%s\"", 
			       (s) ? (const char *)s : "");
		if (icnt < 0) {
		    return SET_ERROR(ERR_INTERNAL_VAL);
		} else {
		    *len += (uint32)icnt;
		}
	    } else {
		*len += (2 + ((s) ? xml_strlen(s) : 0));
	    }
	}
	break;
    case NCX_BT_LIST:
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
	return SET_ERROR(ERR_NCX_OPERATION_NOT_SUPPORTED);
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    return NO_ERR;

}  /* val_sprintf_simval_nc */


/********************************************************************
* FUNCTION val_resolve_scoped_name
* 
* Find the scoped identifier in the specified complex value
* 
* E.g.: foo.bar.baz
*
* INPUTS:
*    cpx == complex type to check
*    name == scoped name string of a nested node to find
* OUTPUTS:
*    *chval is set to the value of the found local scoped 
*      child member, if NO_ERR
* RETURNS:
*   status
*********************************************************************/
status_t 
    val_resolve_scoped_name (val_value_t *val,
			     const xmlChar *name,
			     val_value_t **chval)
{
    xmlChar        *buff;
    const xmlChar  *next;
    val_value_t    *ch, *nextch;

    buff = m__getMem(NCX_MAX_NLEN+1);
    if (!buff) {
	return SET_ERROR(ERR_INTERNAL_MEM);
    }

    /* get the top-level definition name and look for it
     * in the child queue.  This is going to work because
     * the token was already parsed as a scoped token string
     */
    next = ncx_get_name_segment(name, buff);

    /* the first segment is the start value */
    if (xml_strcmp(buff, val->name)) {
	return SET_ERROR(ERR_NCX_NOT_FOUND);
    }

    /* Each time get_name_segment is called, the next pointer
     * will be a dot or end of string.
     *
     * Keep looping until there are no more name segments left
     * or an error occurs.  The first time the loop is entered
     * the *next char should be non-zero.
     */
    ch = val;
    while (*next) {
        /* there is a next child, this better be a complex value */

	nextch = NULL;
        if (typ_has_children(ch->btyp)) {
	    next = ncx_get_name_segment(++next, buff);	    
	    nextch = val_first_child_name(ch, buff);
	}
	if (!nextch) {
	    m__free(buff);
	    return SET_ERROR(ERR_NCX_DEFSEG_NOT_FOUND);
	}
	ch = nextch;
    }

    m__free(buff);
    *chval = ch;
    return NO_ERR;

} /* val_resolve_scoped_name */


/********************************************************************
* FUNCTION val_get_iqualval
* 
* Get the effective instance qualifier value for this value
* 
* INPUTS:
*    val == value construct to check
*
* RETURNS:
*   iqual value
*********************************************************************/
ncx_iqual_t 
    val_get_iqualval (const val_value_t *val)
{
    return obj_get_iqualval(val->obj);

} /* val_get_iqualval */


/********************************************************************
* FUNCTION val_duplicates_allowed
* 
* Determine if duplicates are allowed for the given val type
* The entire definition chain is checked to see if a 'no-duplicates'
*
* The default is config, so some sort of named type or parameter
* must be declared to create a non-config data element
*
* Fishing order:
*  1) typdef chain
*  2) parm definition
*  3) parmset definition
*
* INPUTS:
*     val == value node to check
*
* RETURNS:
*     TRUE if the value is classified as configuration
*     FALSE if the value is not classified as configuration
*********************************************************************/
boolean
    val_duplicates_allowed (val_value_t *val)
{
    /* see if info already cached */
    if (val->flags & VAL_FL_DUPDONE) {
	return (val->flags & VAL_FL_DUPOK) ? TRUE : FALSE;
    }

    /* check for no-duplicates in the type appinfo */
    if (typ_find_appinfo(val->typdef, NCX_PREFIX, 
			 NCX_EL_NODUPLICATES)) {
	val->flags |= VAL_FL_DUPDONE;
	return FALSE;
    }

    /* default is to allow duplicates */
    val->flags |= (VAL_FL_DUPDONE | VAL_FL_DUPOK);
    return TRUE;

}  /* val_duplicates_allowed */


/********************************************************************
* FUNCTION val_has_content
* 
* Determine if there is a value or any child nodes for this val
*
* INPUTS:
*     val == value node to check
*
* RETURNS:
*     TRUE if the value has some content
*     FALSE if the value does not have any content
*********************************************************************/
boolean
    val_has_content (const val_value_t *val)
{
    ncx_btype_t  btyp;

    if (!val_is_real(val)) {
	return TRUE;
    }

    if (val->btyp==NCX_BT_UNION) {
	btyp = val->unbtyp;
    } else {
	btyp = val->btyp;
    }

    if (typ_has_children(btyp)) {
	return !dlq_empty(&val->v.childQ);
    } else if (btyp == NCX_BT_EMPTY && !val->v.bool) {
	return FALSE;
    } else if ((btyp == NCX_BT_SLIST || btyp==NCX_BT_BITS)
	       && ncx_list_empty(&val->v.list)) {
	return FALSE;
    } else {
	return TRUE;
    }

}  /* val_has_content */


/********************************************************************
* FUNCTION val_has_index
* 
* Determine if this value has an index
*
* INPUTS:
*     val == value node to check
*
* RETURNS:
*     TRUE if the value has an index
*     FALSE if the value does not have an index
*********************************************************************/
boolean
    val_has_index (const val_value_t *val)
{
    return (dlq_empty(&val->indexQ)) ? FALSE : TRUE;

}  /* val_has_index */


/********************************************************************
* FUNCTION val_parse_meta
* 
* Parse the metadata descriptor against the typdef
* Check only that the value is ok, not instance count
*
* INPUTS:
*     typdef == typdef to check
*     nsid   == attribute namespace ID
*     attrname == attribute name string
*     attrval == attribute value string
*     retval == initialized val_value_t to fill in
*
* OUTPUTS:
*   *retval == filled in if return is NO_ERR
* RETURNS:
*   status of the operation
*********************************************************************/
status_t
    val_parse_meta (const typ_def_t *typdef,
		xmlns_id_t    nsid,
		const xmlChar *attrname,
		const xmlChar *attrval,
		val_value_t *retval)
{
    ncx_btype_t  btyp;
    int32        enuval;
    const xmlChar *enustr;
    status_t     res;

#ifdef DEBUG
    if (!typdef || !attrname || !attrval) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    btyp = typ_get_basetype(typdef);

    /* setup the return value */
    retval->flags |= VAL_FL_META;  /* obj field is NULL in meta */
    retval->btyp = btyp;
    retval->typdef = typdef;
    retval->dname = xml_strdup(attrname);
    if (!retval->dname) {
	return ERR_INTERNAL_MEM;
    }
    retval->name = retval->dname;
    retval->nsid = nsid;

    /* handle the attr string according to its base type */
    switch (btyp) {
    case NCX_BT_BOOLEAN:
	if (attrval && !xml_strcmp(attrval, NCX_EL_TRUE)) {
	    retval->v.bool = TRUE;
	} else if (attrval && 
		   !xml_strcmp(attrval,
			       (const xmlChar *)"1")) {
	    retval->v.bool = TRUE;
	} else if (attrval && !xml_strcmp(attrval, NCX_EL_FALSE)) {
	    retval->v.bool = FALSE;
	} else if (attrval && 
		   !xml_strcmp(attrval,
			       (const xmlChar *)"0")) {
	    retval->v.bool = FALSE;
	} else {
	    res = ERR_NCX_INVALID_VALUE;
	}
	break;
    case NCX_BT_ENUM:
	res = val_enum_ok(typdef, attrval, &enuval, &enustr);
	if (res == NO_ERR) {
	    retval->v.enu.name = enustr;
	    retval->v.enu.val = enuval;
	}
	break;
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
	res = ncx_decode_num(attrval, btyp, &retval->v.num);
	if (res == NO_ERR) {
	    res = val_range_ok(typdef, btyp, &retval->v.num);
	}
	break;
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_INSTANCE_ID:
	res = val_string_ok(typdef, btyp, attrval);
	if (res == NO_ERR) {
	    retval->v.str = xml_strdup(attrval);
	    if (!retval->v.str) {
		res = ERR_INTERNAL_MEM;
	    }
	}
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

} /* val_parse_meta */


/********************************************************************
* FUNCTION val_set_extern
* 
* Setup an NCX_BT_EXTERN value
*
* INPUTS:
*     val == value to setup
*     fname == filespec string to set as the value
*********************************************************************/
void
    val_set_extern (val_value_t  *val,
		    xmlChar *fname)
{
    val->btyp = NCX_BT_EXTERN;
    val->v.fname = fname;

} /* val_set_extern */


/********************************************************************
* FUNCTION val_set_intern
* 
* Setup an NCX_BT_INTERN value
*
* INPUTS:
*     val == value to setup
*     intbuff == internal buffer to set as the value
*********************************************************************/
void
    val_set_intern (val_value_t  *val,
		    xmlChar *intbuff)
{
    val->btyp = NCX_BT_INTERN;
    val->v.intbuff = intbuff;

} /* val_set_intern */


/********************************************************************
* FUNCTION val_fit_oneline
* 
* Check if the XML encoding for the specified val_value_t
* should take one line or more than one line
*
* Simple types should not use more than one line or introduce
* any extra whitespace in any simple content element
* 
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   TRUE if the val is a type that should or must fit on one line
*   FALSE otherwise
*********************************************************************/
boolean
    val_fit_oneline (const val_value_t *val)
{
    ncx_btype_t       btyp;
    const xmlChar    *str;
    uint32            cnt;
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return TRUE;
    }
#endif

    if (val->btyp == NCX_BT_UNION) {
	btyp = val->unbtyp;
    } else {
	btyp = val->btyp;
    }

    switch (btyp) {
    case NCX_BT_ENUM:
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
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
	return TRUE;
    case NCX_BT_BINARY:
	return (val->v.binary.ustrlen < 20) ? TRUE : FALSE;
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_KEYREF:   /*****/
	if (!VAL_STR(val)) {
	    /* empty string */
	    return TRUE;
	}

	/* hack: assume some length more than a URI,
	 * and less than enough to cause line wrap
	 */
	if (xml_strlen(VAL_STR(val)) > 20) {
	    return FALSE;
	}

	/* check if multiple new-lines are entered, if not,
	 * then treat it as if it will fit on one line
	 * the session linesize and current linepos 
	 * are not known here.
	 */
	str = VAL_STR(val);
	cnt = 0;
	while (*str && cnt < 2) {
	    if (*str++ == '\n') {
		cnt++;
	    }
	}
	return (cnt < 2) ? TRUE : FALSE;
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
	return TRUE;  /***/
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
    case NCX_BT_LIST:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
	return dlq_empty(&val->v.childQ);
    case NCX_BT_EXTERN:
    case NCX_BT_INTERN:
	return FALSE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return TRUE;
    }
    /*NOTREACHED*/

}  /* val_fit_oneline */


/********************************************************************
* FUNCTION val_create_allowed
* 
* Check if the specified value is allowed to have a
* create edit-config operation attribute
* 
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   TRUE if the val is allowed to have the edit-op
*   FALSE otherwise
*********************************************************************/
boolean
    val_create_allowed (const val_value_t *val)
{

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    return (val->index) ? FALSE : TRUE;

}  /* val_create_allowed */


/********************************************************************
* FUNCTION val_delete_allowed
* 
* Check if the specified value is allowed to have a
* delete edit-config operation attribute
* 
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   TRUE if the val is allowed to have the edit-op
*   FALSE otherwise
*********************************************************************/
boolean
    val_delete_allowed (const val_value_t *val)
{

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    return (val->index) ? FALSE : TRUE;

}  /* val_delete_allowed */


/********************************************************************
* FUNCTION val_is_config_data
* 
* Check if the specified value is a config DB object instance
* 
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   TRUE if the val is a config DB object instance
*   FALSE otherwise
*********************************************************************/
boolean
    val_is_config_data (const val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (obj_is_data_db(val->obj) && 
	obj_get_config_flag(val->obj)) {
	return TRUE;
    } else {
	return FALSE;
    }

}  /* val_is_config_data */


/********************************************************************
* FUNCTION val_is_virtual
* 
* Check if the specified value is a virtual value
* such that a 'get' callback function is required
* to access the real value contents
* 
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   TRUE if the val is a virtual value
*   FALSE otherwise
*********************************************************************/
boolean
    val_is_virtual (const val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    return (val->getcb) ? TRUE : FALSE;

}  /* val_is_virtual */


/********************************************************************
* FUNCTION val_get_virtual_value
* 
* Get the value of a value node
* The top-level value is provided by the caller
* and must be malloced with val_new_value
* before calling this function
* 
* If the val->getcb is NULL, then an error will be returned
*
* INPUTS:
*   session == session CB ptr cast as void *
*              that is getting the virtual value
*   val == virtual value to get value for
*   res == pointer to output function return status value
*
* OUTPUTS:
*    *res == the function return status
*
* RETURNS:
*   A malloced and filled in val_value_t struct
*   The val_free_value function must be called if the
*   return value is non-NULL
*********************************************************************/
val_value_t *
    val_get_virtual_value (const void *session,
			   val_value_t *val,
			   status_t *res)
{
    const ses_cb_t *scb;
    getcb_fn_t  getcb;
    val_value_t *retval;

#ifdef DEBUG
    if (!val || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!val->getcb) {
	*res = ERR_NCX_OPERATION_FAILED;
	return NULL;
    }

    scb = (const ses_cb_t *)session;
    getcb = (getcb_fn_t)val->getcb;

    retval = val_new_value();
    if (!retval) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    *res = (*getcb)(scb, GETCB_GET_VALUE, NULL, val, retval);
    if (*res != NO_ERR) {
	val_free_value(retval);
	retval = NULL;
    }

    return retval;

}  /* val_get_virtual_value */


/********************************************************************
* FUNCTION val_setup_virtual_retval
* 
* Set an initialized val_value_t as a virtual return val
*
* INPUTS:
*    virval == virtual value to set from
*    realval == value to set to
*
* OUTPUTS:
*    *realval is setup for return if NO_ERR
*********************************************************************/
void
    val_setup_virtual_retval (val_value_t  *virval,
			      val_value_t *realval)
{
    const typ_template_t  *listtyp;
    ncx_btype_t            btyp;

#ifdef DEBUG
    if (!virval || !realval) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    realval->name = virval->name;
    realval->nsid = virval->nsid;
    realval->obj = virval->obj;
    realval->typdef = virval->typdef;
    realval->flags = virval->flags;
    realval->btyp = virval->btyp;
    realval->dataclass = virval->dataclass;
    realval->parent = virval->parent;

    if (virval->btyp==NCX_BT_UNION) {
	realval->untyp = virval->untyp;
	realval->unbtyp = virval->unbtyp;
    } else if (!typ_is_simple(virval->btyp)) {
	val_init_complex(realval, virval->btyp);
    } else if (virval->btyp == NCX_BT_SLIST ||
	       virval->btyp == NCX_BT_BITS) {
	listtyp = typ_get_listtyp(realval->typdef);
	btyp = typ_get_basetype(&listtyp->typdef);
	ncx_init_list(&realval->v.list, btyp);
    }

}  /* val_setup_virtual_retval */


/********************************************************************
* FUNCTION val_new_virtual_chval
* 
* Create and initialize a val_value_t as a child node
* of a virtual return val
*
* INPUTS:
*    name == name of child node
*    nsid == namespace ID of child node
*    typdef == typdef struct for the child node
*    parent == the parent struct to add the child node to
*              (real value node, not the virtual node!)
* OUTPUTS:
*    new child node added to parent childQ
*
* RETURNS:
*    pointer to new child value, actual value not set yet!!!
*********************************************************************/
val_value_t *
    val_new_virtual_chval (const xmlChar *name,
			   xmlns_id_t nsid,
			   typ_def_t *typdef,
			   val_value_t *parent)
{
    const typ_template_t *listtyp;
    val_value_t          *retval;
    ncx_btype_t           btyp;

#ifdef DEBUG
    if (!name || !typdef || !parent) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    retval = val_new_value();
    if (!retval) {
	return NULL;
    }

    retval->name = name;
    retval->nsid = nsid;
    retval->typdef = typdef;
    retval->btyp = typ_get_basetype(typdef);
    retval->dataclass = NCX_DC_STATE;

    if (!typ_is_simple(retval->btyp)) {
	val_init_complex(retval, retval->btyp);
    } else if (retval->btyp == NCX_BT_SLIST ||
	       retval->btyp == NCX_BT_BITS) {
	listtyp = typ_get_listtyp(retval->typdef);
	btyp = typ_get_basetype(&listtyp->typdef);
	ncx_init_list(&retval->v.list, btyp);
    }

    val_add_child(retval, parent);
    return retval;

}  /* val_new_virtual_chval */


/********************************************************************
* FUNCTION val_is_default
* 
* Check if the specified value is set to the default value
* 
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   TRUE if the val is set to the default value
*   FALSE otherwise
*********************************************************************/
boolean
    val_is_default (const val_value_t *val)
{
    const xmlChar *def;
    xmlChar       *binbuff;
    ncx_enum_t     enu;
    ncx_num_t      num;
    boolean        ret;
    ncx_btype_t    btyp;
    status_t       res;
    uint32         len, deflen;

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    /* check general corner-case: if the value is part of
     * an index, then return FALSE, even if the data type
     * for the index node has a default
     */
    if (val->index) {
	return FALSE;
    }

    /* check if the data type has a default */
    def = typ_get_default(val->typdef);
    if (!def) {
	return FALSE;
    }

    /* check if this is a virtual value, return FALSE instead
     * of retrieving the value!!! Used for monitoring only!!!
     */
    if (val->getcb) {
	return FALSE;
    }

    ret = FALSE;

    if (val->btyp == NCX_BT_UNION) {
	btyp = val->unbtyp;
    } else {
	btyp = val->btyp;
    }

    switch (btyp) {
    case NCX_BT_EMPTY:
    case NCX_BT_BOOLEAN:
	if (ncx_is_true(def)) {
	    /* default is true */
	    if (val->v.bool) {
		ret = TRUE;
	    }
	} else if (ncx_is_false(def)) {
	    /* default is false */
	    if (!val->v.bool) {
		ret = TRUE;
	    }
	}
	break;
    case NCX_BT_ENUM:
	ncx_init_enum(&enu);
	res = ncx_set_enum(def, &enu);
	if (res == NO_ERR && !ncx_compare_enums(&enu, &val->v.enu)) {
	    ret = TRUE;
	}
	ncx_clean_enum(&enu);
	break;
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
	ncx_init_num(&num);
	res = ncx_decode_num(def, btyp, &num);
	if (res == NO_ERR && 
	    !ncx_compare_nums(&num, &val->v.num, btyp)) {
	    ret = TRUE;
	}
	ncx_clean_num(btyp, &num);
	break;
    case NCX_BT_BINARY:
	deflen = xml_strlen(def);
	res = b64_decode(def, deflen,
			 NULL, NCX_MAX_UINT, &len);
	if (res == NO_ERR) {
	    binbuff = m__getMem(len);
	    if (!binbuff) {
		SET_ERROR(ERR_INTERNAL_MEM);
		return FALSE;
	    } 
	    res = b64_decode(def, deflen,
			       binbuff, len, &len);
	    if (res == NO_ERR) {
		ret = memcmp(binbuff, val->v.binary.ustr, len) 
		    ? FALSE : TRUE;
	    }
	    m__free(binbuff);
	}
	break;
    case NCX_BT_STRING:
    case NCX_BT_INSTANCE_ID:
	if (!ncx_compare_strs((const ncx_str_t *)def, 
			      &val->v.str, btyp)) {
	    ret = TRUE;
	}
	break;
    case NCX_BT_KEYREF:
    case NCX_BT_SLIST:
    case NCX_BT_BITS:
    case NCX_BT_LIST:
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
    case NCX_BT_EXTERN:
    case NCX_BT_INTERN:
	/*** not supported for default value ***/
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    return ret;
    
}  /* val_is_default */


/********************************************************************
* FUNCTION val_is_real
* 
* Check if the specified value is a real value
*
*  return TRUE if not virtual or NCX_BT_EXTERN or NCX_BT_INTERN)
* 
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   TRUE if the val is a real value
*   FALSE otherwise
*********************************************************************/
boolean
    val_is_real (const val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    return (val->getcb || val->btyp==NCX_BT_EXTERN ||
	    val->btyp==NCX_BT_INTERN) ? FALSE : TRUE;

}  /* val_is_real */


/********************************************************************
* FUNCTION val_get_parent_nsid
* 
*    Try to get the parent namespace ID
* 
* INPUTS:
*   val == value to check
*   
* RETURNS:
*   namespace ID of parent, or 0 if not found or not a value parent
*********************************************************************/
xmlns_id_t
    val_get_parent_nsid (const val_value_t *val)
{
    const val_value_t  *v;

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    if (!val->parent) {
	return 0;
    }

    v = (const val_value_t *)val->parent;
    return v->nsid;

}  /* val_get_parent_nsid */


/********************************************************************
* FUNCTION val_instance_count
* 
* Count the number of instances of the specified object name
* in the parent value struct.  This only checks the first
* level under the parent, not the entire subtree
*
*
* INPUTS:
*   val == value to check
*   modname == name of module which defines the object to count
*              NULL (do not check module names)
*   objname == name of object to count
*
* RETURNS:
*   number of instances found
*********************************************************************/
uint32
    val_instance_count (val_value_t  *val,
			const xmlChar *modname,
			const xmlChar *objname)
{
    val_value_t *chval;
    uint32       cnt;

#ifdef DEBUG
    if (!val || !objname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    cnt = 0;

    for (chval = val_get_first_child(val);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {

	if (modname && 
	    xml_strcmp(modname,
		       obj_get_mod_name(chval->obj))) {
	    continue;
	}

	if (!xml_strcmp(objname, chval->name)) {
	    cnt++;
	}
    }
    return cnt;
    
}  /* val_instance_count */


/********************************************************************
* FUNCTION val_set_extra_instance_errors
* 
* Count the number of instances of the specified object name
* in the parent value struct.  This only checks the first
* level under the parent, not the entire subtree
* Set the val-res status for all instances beyond the 
* specified 'maxelems' count to ERR_NCX_EXTRA_VAL_INST
*
* INPUTS:
*   val == value to check
*   modname == name of module which defines the object to count
*              NULL (do not check module names)
*   objname == name of object to count
*   maxelems == number of allowed instances
*
*********************************************************************/
void
    val_set_extra_instance_errors (val_value_t  *val,
				   const xmlChar *modname,
				   const xmlChar *objname,
				   uint32 maxelems)
{
    val_value_t *chval;
    uint32       cnt;

#ifdef DEBUG
    if (!val || !objname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    if (maxelems == 0) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    cnt = 0;

    for (chval = val_get_first_child(val);
	 chval != NULL;
	 chval = val_get_next_child(chval)) {

	if (modname && 
	    xml_strcmp(modname,
		       obj_get_mod_name(chval->obj))) {
	    continue;
	}

	if (!xml_strcmp(objname, chval->name)) {
	    if (++cnt > maxelems) {
		chval->res = ERR_NCX_EXTRA_VAL_INST;
	    }
	}
    }
    
}  /* val_set_extra_instance_errors */


/********************************************************************
* FUNCTION val_need_quotes
* 
* Check if a string needs to be quoted to be output
* within a conf file or ncxcli stdout output
*
* INPUTS:
*    str == string to check
*
* RETURNS:
*    TRUE if double quoted string is needed
*    FALSE if not needed
*********************************************************************/
boolean
    val_need_quotes (const xmlChar *str)
{
    /* any whitespace or newline needs quotes */
    while (*str) {
	if (isspace(*str) || *str == '\n') {
	    return TRUE;
	}
	str++;
    }
    return FALSE;

}  /* val_need_quotes */


/********************************************************************
* FUNCTION val_all_whitespace
* 
* Check if a string is all whitespace
*
* INPUTS:
*    str == string to check
*
* RETURNS:
*    TRUE if string is all whitespace or empty length
*    FALSE if non-whitespace char found
*********************************************************************/
boolean
    val_all_whitespace (const xmlChar *str)
{
    /* any whitespace or newline needs quotes */
    while (*str) {
	if (!(isspace(*str) || *str == '\n')) {
	    return FALSE;
	}
	str++;
    }
    return TRUE;

}  /* val_all_whitespace */


/********************************************************************
* FUNCTION val_match_metaval
* 
* Match the specific attribute value and namespace ID
*
* INPUTS:
*     attr == attr to check
*     nsid == mamespace ID to match against
*     name == attribute name to match against
*
* RETURNS:
*     TRUE if attr is a match; FALSE otherwise
*********************************************************************/
boolean
    val_match_metaval (const xml_attr_t *attr,
		       xmlns_id_t  nsid,
		       const xmlChar *name)
{
    if (xml_strcmp(attr->attr_name, name)) {
	return FALSE;
    }
    if (attr->attr_ns) {
	return (attr->attr_ns==nsid);
    } else {
	/* unqualified match */
	return TRUE;
    }
} /* val_match_metaval */


/********************************************************************
* FUNCTION val_get_dirty_flag
* 
* Get the dirty flag for this value node
*
* INPUTS:
*     val == value node to check
*
* RETURNS:
*     TRUE if value is dirty, false otherwise
*********************************************************************/
boolean
    val_get_dirty_flag (const val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
#endif

    return (val->flags & VAL_FL_DIRTY) ? TRUE : FALSE;

} /* val_get_dirty_flag */


/********************************************************************
* FUNCTION val_set_dirty_flag
* 
* Set the dirty flag for this value node
*
* INPUTS:
*     val == value node to check
*
* RETURNS:
*     TRUE if value is dirty, false otherwise
*********************************************************************/
void
    val_set_dirty_flag (val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    val->flags |= VAL_FL_DIRTY;

} /* val_set_dirty_flag */


/********************************************************************
* FUNCTION val_clear_dirty_flag
* 
* Clear the dirty flag for this value node
*
* INPUTS:
*     val == value node to check
*
* RETURNS:
*     TRUE if value is dirty, false otherwise
*********************************************************************/
void
    val_clear_dirty_flag (val_value_t *val)
{
#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    val->flags &= ~VAL_FL_DIRTY;

} /* val_clear_dirty_flag */


/********************************************************************
 * FUNCTION val_clean_tree
 * 
 * Clear the dirty flag and the operation for all
 * nodes within a value struct
 *
 * INPUTS:
 *   val == value node to clean
 *
 * OUTPUTS:
 *   val and all its child nodes (if any) are cleaned
 *     val->flags: VAL_FL_DIRTY bit cleared to 0
 *     val->editop: cleared to OP_EDITOP_NONE
 *     val->curparent: cleared to NULL
 *********************************************************************/
void
    val_clean_tree (val_value_t *val)
{
    val_value_t           *chval;


#ifdef DEBUG
    if (!val || !val->obj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (obj_is_data_db(val->obj) &&
	obj_is_config(val->obj)) {

	for (chval = val_get_first_child(val);
	     chval != NULL;
	     chval = val_get_next_child(chval)) {
	    val_clean_tree(chval);
	}

	val->flags &= ~VAL_FL_DIRTY;
	val->editop = OP_EDITOP_NONE;
	val->curparent = NULL;
    }

}  /* val_clean_tree */


/********************************************************************
* FUNCTION val_get_nest_level
* 
* Get the next level of the value
*
* INPUTS:
*     val == value node to check
*
* RETURNS:
*     nest level from the root
*********************************************************************/
uint32
    val_get_nest_level (val_value_t *val)
{
    uint32 level;

#ifdef DEBUG
    if (!val) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return 0;
    }
#endif

    level = 1;
    while (val->parent) {
	level++;
	val = val->parent;
    }
    return level;

} /* val_get_nest_level */



/* END file val.c */
