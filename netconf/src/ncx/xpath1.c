/*  FILE: xpath1.c

    Xpath 1.0 search support
                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
13nov08      abb      begun

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
*           F O R W A R D   D E C L A R A T I O N S                 *
*                                                                   *
*********************************************************************/
static xpath_result_t *
    parse_expr (xpath_pcb_t *pcb,
                status_t  *res);


static xpath_result_t *
    boolean_fn (xpath_pcb_t *pcb,
		dlq_hdr_t *parmQ,
		status_t  *res);

static xpath_result_t *
    ceiling_fn (xpath_pcb_t *pcb,
		dlq_hdr_t *parmQ,
		status_t  *res);

static xpath_result_t *
    concat_fn (xpath_pcb_t *pcb,
	       dlq_hdr_t *parmQ,
	       status_t  *res);

static xpath_result_t *
    contains_fn (xpath_pcb_t *pcb,
		 dlq_hdr_t *parmQ,
		 status_t  *res);

static xpath_result_t *
    count_fn (xpath_pcb_t *pcb,
	      dlq_hdr_t *parmQ,
	      status_t  *res);

static xpath_result_t *
    current_fn (xpath_pcb_t *pcb,
		dlq_hdr_t *parmQ,
		status_t  *res);

static xpath_result_t *
    false_fn (xpath_pcb_t *pcb,
	      dlq_hdr_t *parmQ,
	      status_t  *res);

static xpath_result_t *
    floor_fn (xpath_pcb_t *pcb,
	      dlq_hdr_t *parmQ,
	      status_t  *res);

static xpath_result_t *
    id_fn (xpath_pcb_t *pcb,
	   dlq_hdr_t *parmQ,
	   status_t  *res);

static xpath_result_t *
    lang_fn (xpath_pcb_t *pcb,
	     dlq_hdr_t *parmQ,
	     status_t  *res);

static xpath_result_t *
    last_fn (xpath_pcb_t *pcb,
	     dlq_hdr_t *parmQ,
	     status_t  *res);

static xpath_result_t *
    local_name_fn (xpath_pcb_t *pcb,
		   dlq_hdr_t *parmQ,
		   status_t  *res);

static xpath_result_t *
    namespace_uri_fn (xpath_pcb_t *pcb,
		      dlq_hdr_t *parmQ,
		      status_t  *res);

static xpath_result_t *
    name_fn (xpath_pcb_t *pcb,
	     dlq_hdr_t *parmQ,
	     status_t  *res);

static xpath_result_t *
    normalize_space_fn (xpath_pcb_t *pcb,
			dlq_hdr_t *parmQ,
			status_t  *res);

static xpath_result_t *
    not_fn (xpath_pcb_t *pcb,
	    dlq_hdr_t *parmQ,
	    status_t  *res);

static xpath_result_t *
    number_fn (xpath_pcb_t *pcb,
	       dlq_hdr_t *parmQ,
	       status_t  *res);

static xpath_result_t *
    position_fn (xpath_pcb_t *pcb,
		 dlq_hdr_t *parmQ,
		 status_t  *res);

static xpath_result_t *
    round_fn (xpath_pcb_t *pcb,
	      dlq_hdr_t *parmQ,
	      status_t  *res);

static xpath_result_t *
    starts_with_fn (xpath_pcb_t *pcb,
		 dlq_hdr_t *parmQ,
		    status_t  *res);

static xpath_result_t *
    string_fn (xpath_pcb_t *pcb,
	       dlq_hdr_t *parmQ,
	       status_t  *res);

static xpath_result_t *
    string_length_fn (xpath_pcb_t *pcb,
		      dlq_hdr_t *parmQ,
		      status_t  *res);

static xpath_result_t *
    substring_fn (xpath_pcb_t *pcb,
		  dlq_hdr_t *parmQ,
		  status_t  *res);

static xpath_result_t *
    substring_after_fn (xpath_pcb_t *pcb,
			dlq_hdr_t *parmQ,
			status_t  *res);

static xpath_result_t *
    substring_before_fn (xpath_pcb_t *pcb,
			 dlq_hdr_t *parmQ,
			 status_t  *res);

static xpath_result_t *
    sum_fn (xpath_pcb_t *pcb,
	    dlq_hdr_t *parmQ,
	    status_t  *res);

static xpath_result_t *
    translate_fn (xpath_pcb_t *pcb,
		  dlq_hdr_t *parmQ,
		  status_t  *res);

static xpath_result_t *
    true_fn (xpath_pcb_t *pcb,
	     dlq_hdr_t *parmQ,
	     status_t  *res);


/********************************************************************
*                                                                   *
*                         V A R I A B L E S                         *
*                                                                   *
*********************************************************************/

static xpath_fncb_t functions [] = {
    { XP_FN_BOOLEAN, XP_RT_BOOLEAN, 1, boolean_fn },
    { XP_FN_CEILING, XP_RT_NUMBER, 1, ceiling_fn },
    { XP_FN_CONCAT, XP_RT_STRING, -1, concat_fn },
    { XP_FN_CONTAINS, XP_RT_BOOLEAN, 2, contains_fn },
    { XP_FN_COUNT, XP_RT_NUMBER, 1, count_fn },
    { XP_FN_CURRENT, XP_RT_NODESET, 0, current_fn },
    { XP_FN_FALSE, XP_RT_BOOLEAN, 0, false_fn },
    { XP_FN_FLOOR, XP_RT_NUMBER, 1, floor_fn },
    { XP_FN_ID, XP_RT_NODESET, 1, id_fn },
    { XP_FN_LANG, XP_RT_BOOLEAN, 1, lang_fn },
    { XP_FN_LAST, XP_RT_NUMBER, 0, last_fn },
    { XP_FN_LOCAL_NAME, XP_RT_STRING, -1, local_name_fn },
    { XP_FN_NAME, XP_RT_STRING, -1, name_fn },
    { XP_FN_NAMESPACE_URI, XP_RT_STRING, -1, namespace_uri_fn },
    { XP_FN_NORMALIZE_SPACE, XP_RT_STRING, -1, normalize_space_fn },
    { XP_FN_NOT, XP_RT_BOOLEAN, 1, not_fn },
    { XP_FN_NUMBER, XP_RT_NUMBER, -1, number_fn },
    { XP_FN_POSITION, XP_RT_NUMBER, 0, position_fn },
    { XP_FN_ROUND, XP_RT_NUMBER, 1, round_fn },
    { XP_FN_STARTS_WITH, XP_RT_BOOLEAN, 2, starts_with_fn },
    { XP_FN_STRING, XP_RT_STRING, -1, string_fn },
    { XP_FN_STRING_LENGTH, XP_RT_NUMBER, -1, string_length_fn },
    { XP_FN_SUBSTRING, XP_RT_STRING, -1, substring_fn },
    { XP_FN_SUBSTRING_AFTER, XP_RT_STRING, 2, substring_after_fn },
    { XP_FN_SUBSTRING_BEFORE, XP_RT_STRING, 2, substring_before_fn },
    { XP_FN_SUM, XP_RT_NUMBER, 1, sum_fn },
    { XP_FN_TRANSLATE, XP_RT_STRING, 3, translate_fn },
    { XP_FN_TRUE, XP_RT_BOOLEAN, 0, true_fn },
    { NULL, XP_RT_NONE, 0, NULL }   /* last entry marker */
};


/********************************************************************
* FUNCTION set_uint32_num
* 
* Set an ncx_num_t with a uint32 number
*
* INPUTS:
*    innum == number value to use
*    outnum == address ofoutput number
*
* OUTPUTS:
*   *outnum is set with the converted value
*    
*********************************************************************/
static void
    set_uint32_num (uint32 innum,
		    ncx_num_t  *outnum)
{
#ifdef HAS_FLOAT
    outnum->d = (double)innum;
#else
    outnum->d = (int64)innum;
#endif

}  /* set_uint32_num */


/********************************************************************
* FUNCTION restype_string
* 
* Get the result type enumeration name
*
* INPUTS:
*    restyoe == the result type 
*
* RETURNS:
*    name of the enum
*********************************************************************/
static const xmlChar *
    restype_string (xpath_restype_t restype)
{
    switch (restype) {
    case XP_RT_NONE:
	return (const xmlChar *)"none";
    case XP_RT_NODESET:
	return (const xmlChar *)"nodeset";
    case XP_RT_NUMBER:
	return (const xmlChar *)"number";
    case XP_RT_STRING:
	return (const xmlChar *)"string";
    case XP_RT_BOOLEAN:
	return (const xmlChar *)"boolean";
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return (const xmlChar *)"INVALID";
    }
    /*NOTREACHED*/

}  /* restype_string */


/********************************************************************
* FUNCTION malloc_failed_error
* 
* Generate a malloc failed error if OK
*
* INPUTS:
*    pcb == parser control block to use
*********************************************************************/
static void
    malloc_failed_error (xpath_pcb_t *pcb)
{
    if (pcb->logerrors) {
	if (pcb->exprstr) {
	    log_error("\nError: malloc failed in "
		      "Xpath expression '%s'.",
		      pcb->exprstr);
	} else {
	    log_error("\nError: malloc failed in "
		      "Xpath expression");
	}
	ncx_print_errormsg(pcb->tkc, pcb->mod, 
			   ERR_INTERNAL_MEM);
    }

}  /* malloc_failed_error */


/********************************************************************
* FUNCTION no_parent_warning
* 
* Generate a no parent available error if OK
*
* INPUTS:
*    pcb == parser control block to use
*********************************************************************/
static void
    no_parent_warning (xpath_pcb_t *pcb)
{
    if (pcb->logerrors) {
	log_warn("\nWarning: no parent found "
		  "in XPath expr '%s'", pcb->exprstr);
	ncx_print_errormsg(pcb->tkc, 
			   pcb->objmod, 
			   ERR_NCX_NO_XPATH_PARENT);
    }

}  /* no_parent_warning */


/********************************************************************
* FUNCTION unexpected_error
* 
* Generate an unexpected token error if OK
*
* INPUTS:
*    pcb == parser control block to use
*********************************************************************/
static void
    unexpected_error (xpath_pcb_t *pcb)
{
    if (pcb->logerrors) {
	if (TK_CUR(pcb->tkc)) {
	    log_error("\nError: Unexpected token '%s' in "
		      "XPath expression '%s'",
		      tk_get_token_name(TK_CUR_TYP(pcb->tkc)),
		      pcb->exprstr);
	} else {
	    log_error("\nError: End reached in "
		      "XPath expression '%s'",  pcb->exprstr);
	}
	ncx_print_errormsg(pcb->tkc, pcb->mod, 
			   ERR_NCX_WRONG_TKTYPE);
    }

}  /* unexpected_error */


/********************************************************************
* FUNCTION wrong_type_error
* 
* Generate a wrong function parameter type error if OK
*
* INPUTS:
*    pcb == parser control block to use
*    gottyoe == the result type encountered
*    exptyoe == the result type expected
*********************************************************************/
static void
    wrong_type_error (xpath_pcb_t *pcb,
		      xpath_restype_t gottype,
		      xpath_restype_t exptype)
{
    if (pcb->logerrors) {
	log_error("\nError: Got arg type '%s', expected '%s' in "
		  "Xpath expression '%s'",
		  restype_string(gottype),
		  restype_string(exptype),
		  pcb->exprstr);
	ncx_print_errormsg(pcb->tkc, pcb->mod, 
			   ERR_NCX_WRONG_DATATYP);
    }

}  /* wrong_type_error */


/********************************************************************
* FUNCTION wrong_parmcnt_error
* 
* Generate a wrong function parameter count error if OK
*
* INPUTS:
*    pcb == parser control block to use
*    parmcnt == the number of parameters received
*    res == error result to use
*********************************************************************/
static void
    wrong_parmcnt_error (xpath_pcb_t *pcb,
			 uint32 parmcnt,
			 status_t res)
{
    if (pcb->logerrors) {
	log_error("\nError: wrong function arg count '%u' in "
		  "Xpath expression '%s'",
		  parmcnt,
		  pcb->exprstr);
	ncx_print_errormsg(pcb->tkc, pcb->mod, res);
    }

}  /* wrong_parmcnt_error */


/********************************************************************
* FUNCTION new_result
* 
* Get a new result from the cache or malloc if none available
*
* INPUTS:
*    pcb == parser control block to use
*    restype == desired result type
*
* RETURNS:
*    result from the cache or malloced; NULL if malloc fails
*********************************************************************/
static xpath_result_t *
    new_result (xpath_pcb_t *pcb,
		xpath_restype_t restype)
{
    xpath_result_t  *result;

    result = (xpath_result_t *)dlq_deque(&pcb->result_cacheQ);
    if (result) {
	pcb->result_count--;
	xpath_init_result(result, restype);
    } else {
	result = xpath_new_result(restype);
    }

    if (!result) {
	malloc_failed_error(pcb);
    }

    return result;

} /* new_result */


/********************************************************************
* FUNCTION free_result
* 
* Free a result struct: put in cache or free if cache maxed out
*
* INPUTS:
*    pcb == parser control block to use
*    result == result struct to free
*
*********************************************************************/
static void
    free_result (xpath_pcb_t *pcb,
		xpath_result_t *result)
{
    xpath_resnode_t *resnode;

    if (result->restype == XP_RT_NODESET) {
	while (!dlq_empty(&result->r.nodeQ) &&
	       pcb->resnode_count < XPATH_RESNODE_CACHE_MAX) {

	    resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);
	    xpath_clean_resnode(resnode);
	    dlq_enque(resnode, &pcb->resnode_cacheQ);
	    pcb->resnode_count++;
	}
    }

    if (pcb->result_count < XPATH_RESULT_CACHE_MAX) {
	xpath_clean_result(result);
	dlq_enque(result, &pcb->result_cacheQ);
	pcb->result_count++;
    } else {
	xpath_free_result(result);
    }

} /* free_result */


/********************************************************************
* FUNCTION new_obj_resnode
* 
* Get a new result node from the cache or malloc if none available
*
* INPUTS:
*    pcb == parser control block to use
*           if pcb->val set then node.valptr will be used
*           else node.objptr will be used instead
*    position == position within the current search
*    dblslash == TRUE if // present on this result node
*                and has not been converted to separate
*                nodes for all descendants instead
*             == FALSE if '//' is not in effect for the 
*                current step
*    objptr == object pointer value to use
*
* RETURNS:
*    result from the cache or malloced; NULL if malloc fails
*********************************************************************/
static xpath_resnode_t *
    new_obj_resnode (xpath_pcb_t *pcb,
		     int64 position,
		     boolean dblslash,
		     const obj_template_t *objptr)
{
    xpath_resnode_t  *resnode;

    resnode = (xpath_resnode_t *)dlq_deque(&pcb->resnode_cacheQ);
    if (resnode) {
	pcb->resnode_count--;
    } else {
	resnode = xpath_new_resnode();
    }

    if (!resnode) {
	malloc_failed_error(pcb);
    } else  {
	resnode->position = position;
	resnode->dblslash = dblslash;
	resnode->node.objptr = objptr;
    }

    return resnode;

} /* new_obj_resnode */


/********************************************************************
* FUNCTION new_val_resnode
* 
* Get a new result node from the cache or malloc if none available
*
* INPUTS:
*    pcb == parser control block to use
*           if pcb->val set then node.valptr will be used
*           else node.objptr will be used instead
*    position == position within the search context
*    dblslash == TRUE if // present on this result node
*                and has not been converted to separate
*                nodes for all descendants instead
*             == FALSE if '//' is not in effect for the 
*                current step
*    valptr == variable pointer value to use
*
* RETURNS:
*    result from the cache or malloced; NULL if malloc fails
*********************************************************************/
static xpath_resnode_t *
    new_val_resnode (xpath_pcb_t *pcb,
		     int64 position,
		     boolean dblslash,
		     val_value_t *valptr)
{
    xpath_resnode_t  *resnode;

    resnode = (xpath_resnode_t *)dlq_deque(&pcb->resnode_cacheQ);
    if (resnode) {
	pcb->resnode_count--;
    } else {
	resnode = xpath_new_resnode();
    }

    if (!resnode) {
	malloc_failed_error(pcb);
    } else {
	resnode->position = position;
	resnode->dblslash = dblslash;
	resnode->node.valptr = valptr;
    }

    return resnode;

} /* new_val_resnode */


/********************************************************************
* FUNCTION free_resnode
* 
* Free a result node struct: put in cache or free if cache maxed out
*
* INPUTS:
*    pcb == parser control block to use
*    resnode == result node struct to free
*
*********************************************************************/
static void
    free_resnode (xpath_pcb_t *pcb,
		  xpath_resnode_t *resnode)
{
    if (pcb->resnode_count < XPATH_RESNODE_CACHE_MAX) {
	xpath_clean_resnode(resnode);
	dlq_enque(resnode, &pcb->resnode_cacheQ);
	pcb->resnode_count++;
    } else {
	xpath_free_resnode(resnode);
    }

} /* free_resnode */


#if 0
/********************************************************************
* FUNCTION clone_resnode
* 
* Clone a result node struct
*
* INPUTS:
*    pcb == parser control block to use
*    resnode == result node struct to clone
*
* RETURNS:
*    malloced data structure to mirror the 'resnode' or NULL
*    if out-of-memory error
*********************************************************************/
static xpath_resnode_t *
    clone_resnode (xpath_pcb_t *pcb,
		   xpath_resnode_t *resnode)
{
    if (pcb->val) {
	return new_val_resnode(pcb, 
			       resnode->position,
			       resnode->dblslash,
			       resnode->node.valptr);
    } else if (pcb->obj) {
	return new_obj_resnode(pcb, 
			       resnode->position,
			       resnode->dblslash,
			       resnode->node.objptr);
    } else {
	return NULL;
    }

}  /* clone_resnode */
#endif


#if 0
/********************************************************************
* FUNCTION clone_result
* 
* Clone a result struct
*
* INPUTS:
*    pcb == parser control block to use
*    result == result struct to clone
*
* RETURNS:
*    malloced data structure to mirror the 'result' or NULL
*    if out-of-memory error
*********************************************************************/
static xpath_result_t *
    clone_result (xpath_pcb_t *pcb,
		  const xpath_result_t *result)
{
    xpath_result_t  *newresult;
    xpath_resnode_t *resnode, *newresnode;
    status_t         res;
    
    newresult = new_result(pcb, result->restype);
    if (!newresult) {
	return NULL;
    }

    switch (result->restype) {
    case XP_RT_NONE:
	break;
    case XP_RT_NODESET:
	for (resnode = (xpath_resnode_t *)
		 dlq_firstEntry(&result->r.nodeQ);
	     resnode != NULL;
	     resnode = (xpath_resnode_t *)dlq_nextEntry(resnode)) {

	    newresnode = clone_resnode(pcb, resnode);
	    if (!newresnode) {
		xpath_free_result(newresult);
		return NULL;
	    } else {
		dlq_enque(newresnode, &newresult->r.nodeQ);
	    }
	}
	break;
    case XP_RT_NUMBER:
	res = ncx_copy_num(&result->r.num, &newresult->r.num,
			   NCX_BT_FLOAT64);
	break;
    case XP_RT_STRING:
	newresult->r.str = xml_strdup(result->r.str);
	if (!newresult->r.str) {
	    xpath_free_result(newresult);
	    return NULL;
	}
	break;
    case XP_RT_BOOLEAN:
	newresult->r.bool = result->r.bool;
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	xpath_free_result(newresult);
	return NULL;
    }

    return newresult;

} /* clone_result */
#endif


/********************************************************************
* FUNCTION compare_results
* 
* Compare 2 results
*
* INPUTS:
*    pcb == parser control block to use
*    val1 == first result struct to compare
*    val2 == second result struct to compare
*    res == address of resturn status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*    -1 if val1 < val2
*     0 if val1 == val2
*     1 if val1 > val2
*********************************************************************/
static int32
    compare_results (xpath_pcb_t *pcb,
		     xpath_result_t *val1,
		     xpath_result_t *val2,
		     status_t *res)
{
    xmlChar         *str1, *str2;
    ncx_num_t        num1, num2;
    boolean          bool1, bool2;
    int32            retval;

    *res = NO_ERR;

    if (!pcb->val) {
	return 0;
    }

    str1 = NULL;
    str2 = NULL;
    retval = 0;

    if (val1->restype == XP_RT_NODESET) {
	*res = xpath_cvt_string(val1, &str1);
	if (*res != NO_ERR) {
	    return retval;
	}
    }

    if (val2->restype == XP_RT_NODESET) {
	*res = xpath_cvt_string(val2, &str2);
	if (*res != NO_ERR) {
	    if (str1) {
		m__free(str1);
	    }
	    return retval;
	}
    }

    /* handle converted nodesets */
    if (str1 && str2) {
	retval = xml_strcmp(str1, str2);
	m__free(str1);
	m__free(str2);
	return retval;
    } else if (str1) {
	if (val2->restype == XP_RT_STRING) {
	    retval = xml_strcmp(str1, val2->r.str);
	} else {
	    *res = xpath_cvt_string(val2, &str2);
	    if (*res == NO_ERR) {
		retval = xml_strcmp(str1, str2);
	    }
	    if (str2) {
		m__free(str2);
	    }
	}
	m__free(str1);
	return retval;
    } else if (str2) {
	if (val1->restype == XP_RT_STRING) {
	    retval = xml_strcmp(val1->r.str, str2);
	} else {
	    *res = xpath_cvt_string(val1, &str1);
	    if (*res == NO_ERR) {
		retval = xml_strcmp(str1, str2);
	    }
	    if (str1) {
		m__free(str1);
	    }
	}
	m__free(str2);
	return retval;
    }

    /* no nodesets involved, so compare if same result type */
    if (val1->restype == val2->restype) {
	switch (val1->restype) {
	case XP_RT_NUMBER:
	    retval = ncx_compare_nums(&val1->r.num,
				      &val2->r.num,
				      NCX_BT_FLOAT64);
	    break;
	case XP_RT_STRING:
	    retval = xml_strcmp(val1->r.str, val2->r.str);
	    break;
	case XP_RT_BOOLEAN:
	    if (val1->r.bool && val2->r.bool) {
		retval = 0;
	    } else if (val1->r.bool) {
		retval = 1;
	    } else {
		retval = -1;
	    }
	    break;
	default:
	    *res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	return retval;
    }

    /* not the same result types, so figure out the
     * best comparision to make.  Priority is:
     *
     *  1) string
     *  2) number
     *  3) boolean
     */
    if (val1->restype == XP_RT_STRING) {
	str2 = NULL;
	*res = xpath_cvt_string(val2, &str2);
	if (*res == NO_ERR) {
	    retval = xml_strcmp(val1->r.str, str2);
	}
	if (str2) {
	    m__free(str2);
	}
    } else if (val2->restype == XP_RT_STRING) {
	str1 = NULL;
	*res = xpath_cvt_string(val1, &str1);
	if (*res == NO_ERR) {
	    retval = xml_strcmp(str1, val2->r.str);
	}
	if (str1) {
	    m__free(str1);
	}
    } else if (val1->restype == XP_RT_NUMBER) {
	ncx_init_num(&num2);
	xpath_cvt_number(val2, &num2);
	retval = ncx_compare_nums(&val1->r.num, &num2,
				  NCX_BT_FLOAT64);
	ncx_clean_num(NCX_BT_FLOAT64, &num2);
    } else if (val2->restype == XP_RT_NUMBER) {
	ncx_init_num(&num1);
	xpath_cvt_number(val1, &num1);
	retval = ncx_compare_nums(&num1, &val2->r.num,
				  NCX_BT_FLOAT64);
	ncx_clean_num(NCX_BT_FLOAT64, &num1);
    } else if (val1->restype == XP_RT_BOOLEAN) {
	bool2 = xpath_cvt_boolean(val2);
	if (val1->r.bool && bool2) {
	    retval = 0;
	} else if (val1->r.bool) {
	    retval = 1;
	} else {
	    retval = -1;
	}
    }  else if (val2->restype == XP_RT_BOOLEAN) {
	bool1 = xpath_cvt_boolean(val1);
	if (bool1 && val2->r.bool) {
	    retval = 0;
	} else if (bool1) {
	    retval = 1;
	} else {
	    retval = -1;
	}
    } else {
	*res = ERR_NCX_INVALID_VALUE;
    }

    return retval;

} /* compare_results */


/********************************************************************
* FUNCTION new_nodeset
* 
* Start a nodeset result with a specified object or value ptr
* Only one of obj or val will really be stored,
* depending on the current parsing mode
*
* INPUTS:
*    pcb == parser control block to use
*    obj == object ptr to store as the first resnode
*    val == value ptr to store as the first resnode
*    dblslash == TRUE if unprocessed dblslash in effect
*                FALSE if not
*
* RETURNS:
*    malloced data structure or NULL if out-of-memory error
*********************************************************************/
static xpath_result_t *
    new_nodeset (xpath_pcb_t *pcb,
		 const obj_template_t *obj,
		 val_value_t *val,
		 int64 position,
		 boolean dblslash)
{
    xpath_result_t  *result;
    xpath_resnode_t *resnode;
    
    result = new_result(pcb, XP_RT_NODESET);
    if (!result) {
	return NULL;
    }

    if (obj || val) {
	if (pcb->val) {
	    resnode = new_val_resnode(pcb, position, dblslash, val);
	    result->isval = TRUE;
	} else {
	    resnode = new_obj_resnode(pcb, position, dblslash, obj);
	    result->isval = FALSE;
	}
	if (!resnode) {
	    xpath_free_result(result);
	    return NULL;
	}
	dlq_enque(resnode, &result->r.nodeQ);
    }

    result->last = 1;

    return result;

} /* new_nodeset */


/********************************************************************
* FUNCTION dump_result
* 
* Generate log output displaying the contents of a result
*
* INPUTS:
*    pcb == parser control block to use
*    result == result to dump
*    banner == optional first banner string to use
*********************************************************************/
static void
    dump_result (xpath_pcb_t *pcb,
		 xpath_result_t *result,
		 const char *banner)
{
    xpath_resnode_t       *resnode;
    val_value_t           *val;
    const obj_template_t  *obj;

    if (banner) {
	log_write("\n%s", banner);
    }
    log_write(" mod:%s, line:%u", 
	      ncx_get_modname(pcb->mod),
	      pcb->tk->linenum);
    log_write("\nxpath result for '%s'", pcb->exprstr);

    switch (result->restype) {
    case XP_RT_NONE:
	log_write("\n  typ: none");
	break;
    case XP_RT_NODESET:
	log_write("\n  typ: nodeset = ");
	for (resnode = (xpath_resnode_t *)
		 dlq_firstEntry(&result->r.nodeQ);
	     resnode != NULL;
	     resnode = (xpath_resnode_t *)dlq_nextEntry(resnode)) {

	    log_write("\n   node ");
	    if (result->isval) {
		val = resnode->node.valptr;
		if (val) {
		    log_write("%s:%s [L:%u]", 
			      xmlns_get_ns_prefix(val->nsid),
			      val->name,
			      val_get_nest_level(val));
		} else {
		    log_write("NULL");
		}
	    } else {
		obj = resnode->node.objptr;
		if (obj) {
		    log_write("%s:%s [L:%u]", 
			      obj_get_mod_prefix(obj),
			      obj_get_name(obj),
			      obj_get_level(obj));
		} else {
		    log_write("NULL");
		}
	    }
	}
	break;
    case XP_RT_NUMBER:
	log_write("\n  typ: number = ");
	ncx_printf_num(&result->r.num, NCX_BT_FLOAT64);
	break;
    case XP_RT_STRING:
	log_write("\n  typ: string = %s", result->r.str);
	break;
    case XP_RT_BOOLEAN:
	log_write("\n  typ: boolean = %s",
		  (result->r.bool) ? "true" : "false");
	break;
    default:
	log_write("\n  typ: INVALID (%d)", result->restype);
	SET_ERROR(ERR_INTERNAL_VAL);
    }
    log_write("\n");

}  /* dump_result */


/********************************************************************
* FUNCTION get_axis_id
* 
* Check a string token tfor a match of an AxisName
*
* INPUTS:
*    name == name string to match
*
* RETURNS:
*   enum of axis name or XP_AX_NONE (0) if not an axis name
*********************************************************************/
static ncx_xpath_axis_t
    get_axis_id (const xmlChar *name)
{
    if (!name || !*name) {
	return XP_AX_NONE;
    }

    if (!xml_strcmp(name, XP_AXIS_ANCESTOR)) {
	return XP_AX_ANCESTOR;
    }
    if (!xml_strcmp(name, XP_AXIS_ANCESTOR_OR_SELF)) {
	return XP_AX_ANCESTOR_OR_SELF;
    }
    if (!xml_strcmp(name, XP_AXIS_ATTRIBUTE)) {
	return XP_AX_ATTRIBUTE;
    }
    if (!xml_strcmp(name, XP_AXIS_CHILD)) {
	return XP_AX_CHILD;
    }
    if (!xml_strcmp(name, XP_AXIS_DESCENDANT)) {
	return XP_AX_DESCENDANT;
    }
    if (!xml_strcmp(name, XP_AXIS_DESCENDANT_OR_SELF)) {
	return XP_AX_DESCENDANT_OR_SELF;
    }
    if (!xml_strcmp(name, XP_AXIS_FOLLOWING)) {
	return XP_AX_FOLLOWING;
    }
    if (!xml_strcmp(name, XP_AXIS_FOLLOWING_SIBLING)) {
	return XP_AX_FOLLOWING_SIBLING;
    }
    if (!xml_strcmp(name, XP_AXIS_NAMESPACE)) {
	return XP_AX_NAMESPACE;
    }
    if (!xml_strcmp(name, XP_AXIS_PARENT)) {
	return XP_AX_PARENT;
    }
    if (!xml_strcmp(name, XP_AXIS_PRECEDING)) {
	return XP_AX_PRECEDING;
    }
    if (!xml_strcmp(name, XP_AXIS_PRECEDING_SIBLING)) {
	return XP_AX_PRECEDING_SIBLING;
    }
    if (!xml_strcmp(name, XP_AXIS_SELF)) {
	return XP_AX_SELF;
    }
    return XP_AX_NONE;

} /* get_axis_id */


/********************************************************************
* FUNCTION get_nodetype_id
* 
* Check a string token tfor a match of a NodeType
*
* INPUTS:
*    name == name string to match
*
* RETURNS:
*   enum of node type or XP_EXNT_NONE (0) if not a node type name
*********************************************************************/
static xpath_nodetype_t
    get_nodetype_id (const xmlChar *name)
{
    if (!name || !*name) {
	return XP_EXNT_NONE;
    }

    if (!xml_strcmp(name, XP_NT_COMMENT)) {
	return XP_EXNT_COMMENT;
    }
    if (!xml_strcmp(name, XP_NT_TEXT)) {
	return XP_EXNT_TEXT;
    }
    if (!xml_strcmp(name, XP_NT_PROCESSING_INSTRUCTION)) {
	return XP_EXNT_PROC_INST;
    }
    if (!xml_strcmp(name, XP_NT_NODE)) {
	return XP_EXNT_NODE;
    }
    return XP_EXNT_NONE;

} /* get_nodetype_id */


/********************************************************************
* FUNCTION location_path_end
* 
* Check if the current location path is ending
* by checking if the next token is going to start some
* sub-expression (or end the expression)
*
* INPUTS:
*    pcb == parser control block to check
*
* RETURNS:
*   TRUE if the location path is ended
*   FALSE if the next token is part of the continuing
*   location path
*********************************************************************/
static boolean
    location_path_end (xpath_pcb_t *pcb)
{
    tk_type_t nexttyp;

    /* check corner-case path '/' */
    nexttyp = tk_next_typ(pcb->tkc);
    if (nexttyp == TK_TT_NONE ||
	nexttyp == TK_TT_EQUAL ||
	nexttyp == TK_TT_BAR ||
	nexttyp == TK_TT_PLUS ||
	nexttyp == TK_TT_MINUS ||
	nexttyp == TK_TT_LT ||
	nexttyp == TK_TT_GT ||
	nexttyp == TK_TT_NOTEQUAL ||
	nexttyp == TK_TT_LEQUAL ||
	nexttyp == TK_TT_GEQUAL) {

	return TRUE;
    } else {
	return FALSE;
    }

} /* location_path_end */


/**********   X P A T H    F U N C T I O N S   ************/


/********************************************************************
* FUNCTION boolean_fn
* 
* boolean boolean(object) function [4.3]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 object to convert to boolean
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    boolean_fn (xpath_pcb_t *pcb,
		dlq_hdr_t *parmQ,
		status_t  *res)
{
    xpath_result_t  *parm, *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (!parm) {
	/* should already be reported in obj mode */
	*res = ERR_NCX_MISSING_PARM;
	return NULL;
    }

    result = new_result(pcb, XP_RT_BOOLEAN);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    result->r.bool = xpath_cvt_boolean(parm);
    *res = NO_ERR;
    return result;

}  /* boolean_fn */


/********************************************************************
* FUNCTION ceiling_fn
* 
* number ceiling(number) function [4.4]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 number to convert to ceiling(number)
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    ceiling_fn (xpath_pcb_t *pcb,
		dlq_hdr_t *parmQ,
		status_t  *res)
{
    xpath_result_t  *parm, *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (!parm) {
	*res = ERR_NCX_MISSING_PARM;
	return NULL;
    }

    result = new_result(pcb, XP_RT_NUMBER);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    if (parm->restype == XP_RT_NUMBER) {
	*res = ncx_num_ceiling(&parm->r.num,
			       &result->r.num,
			       NCX_BT_FLOAT64);
			   
    } else {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_NUMBER);
    }

    if (*res != NO_ERR) {
	free_result(pcb, result);
	result = NULL;
    }

    return result;

}  /* ceiling_fn */


/********************************************************************
* FUNCTION concat_fn
* 
* string concat(string, string, string*) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 2 or more strings to concatenate
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    concat_fn (xpath_pcb_t *pcb,
	       dlq_hdr_t *parmQ,
	       status_t  *res)
{
    xpath_result_t  *parm, *result;
    xmlChar         *str;
    uint32           parmcnt, len;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    *res = NO_ERR;
    len = 0;

    /* check at least 2 strings to concat */
    parmcnt = dlq_count(parmQ);
    if (parmcnt < 2) {
	*res = ERR_NCX_MISSING_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    /* check all parms are strings and get total len */
    for (parm = (xpath_result_t *)dlq_firstEntry(parmQ);
	 parm != NULL;
	 parm = (xpath_result_t *)dlq_nextEntry(parm)) {
	if (parm->restype != XP_RT_STRING) {
	    *res = ERR_NCX_WRONG_DATATYP;
	    wrong_type_error(pcb, parm->restype, XP_RT_STRING);
	} else if (parm->r.str) {
	    len += xml_strlen(parm->r.str);
	}
    }

    if (*res != NO_ERR) {
	return NULL;
    }

    result = new_result(pcb, XP_RT_STRING);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    result->r.str = m__getMem(len+1);
    if (!result->r.str) {
	free_result(pcb, result);
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    str = result->r.str;

    for (parm = (xpath_result_t *)dlq_firstEntry(parmQ);
	 parm != NULL;
	 parm = (xpath_result_t *)dlq_nextEntry(parm)) {
	if (parm->r.str) {
	    str += xml_strcpy(str, parm->r.str);
	}
    }

    return result;

}  /* concat_fn */


/********************************************************************
* FUNCTION contains_fn
* 
* boolean contains(string, string) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 2 strings
*             returns true if the 1st string contains the 2nd string
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    contains_fn (xpath_pcb_t *pcb,
		 dlq_hdr_t *parmQ,
		 status_t  *res)
{
    xpath_result_t  *parm1, *parm2, *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm1 = (xpath_result_t *)dlq_firstEntry(parmQ);
    parm2 = (xpath_result_t *)dlq_nextEntry(parm1);

    if (parm1->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm1->restype, XP_RT_STRING);
    } else if (parm2->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm2->restype, XP_RT_STRING);
    } else {
	result = new_result(pcb, XP_RT_BOOLEAN);
	if (!result) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    if (strstr((const char *)parm1->r.str, 
		       (const char *)parm2->r.str)) {
		result->r.bool = TRUE;
	    } else {
		result->r.bool = FALSE;
	    }
	    *res = NO_ERR;
	    return result;
	}
    }
    return NULL;

}  /* contains_fn */


/********************************************************************
* FUNCTION count_fn
* 
* number count(nodeset) function [4.1]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 parm (nodeset to get node count for)
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    count_fn (xpath_pcb_t *pcb,
	      dlq_hdr_t *parmQ,
	      status_t  *res)
{
    xpath_result_t *parm, *result;
    ncx_num_t       tempnum;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }
    parm = (xpath_result_t *)dlq_firstEntry(parmQ);

    if (parm->restype != XP_RT_NODESET) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_NODESET);
    } else {
	result = new_result(pcb, XP_RT_NUMBER);
	if (!result) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    if (pcb->val) {
		ncx_init_num(&tempnum);
		tempnum.u = dlq_count(&parm->r.nodeQ);
		*res = ncx_cast_num(&tempnum, 
				    NCX_BT_UINT32,
				    &result->r.num,
				    NCX_BT_FLOAT64);
		ncx_clean_num(NCX_BT_UINT32, &tempnum);
	    } else {
		ncx_set_num_zero(&result->r.num, NCX_BT_FLOAT64);
		*res = NO_ERR;
	    }

	    if (*res != NO_ERR) {
		free_result(pcb, result);
		result = NULL;
	    }

	    return result;
	}
    }
    return NULL;

}  /* count_fn */


/********************************************************************
* FUNCTION current_fn
* 
* number current() function [XPATH 2.0 used in YANG]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == empty parmQ
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    current_fn (xpath_pcb_t *pcb,
		dlq_hdr_t *parmQ,
		status_t  *res)
{
    xpath_result_t *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    (void)parmQ;

    result = new_nodeset(pcb, 
			 pcb->context.node.objptr,
			 pcb->context.node.valptr,
			 1, FALSE);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
    } else {
	*res = NO_ERR;
    }

    return result;

}  /* current_fn */


/********************************************************************
* FUNCTION false_fn
* 
* boolean false() function [4.3]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == empty parmQ
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    false_fn (xpath_pcb_t *pcb,
	      dlq_hdr_t *parmQ,
	      status_t  *res)
{
    xpath_result_t *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    (void)parmQ;

    result = new_result(pcb, XP_RT_BOOLEAN);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
    } else {
	result->r.bool = FALSE;
	*res = NO_ERR;
    }

    return result;

}  /* false_fn */


/********************************************************************
* FUNCTION floor_fn
* 
* number floor(number) function [4.4]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 number to convert to floor(number)
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    floor_fn (xpath_pcb_t *pcb,
	      dlq_hdr_t *parmQ,
	      status_t  *res)
{
    xpath_result_t  *parm, *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (!parm) {
	*res = ERR_NCX_MISSING_PARM;
	return NULL;
    }

    result = new_result(pcb, XP_RT_NUMBER);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    if (parm->restype == XP_RT_NUMBER) {
	*res = ncx_num_floor(&parm->r.num,
			     &result->r.num,
			     NCX_BT_FLOAT64);
    } else {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_NUMBER);
    }	
			   
    if (*res != NO_ERR) {
	free_result(pcb, result);
	result = NULL;
    }

    return result;

}  /* floor_fn */


/********************************************************************
* FUNCTION id_fn
* 
* nodeset id(object) function [4.1]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 parm, which is the object to match
*             against the current result in progress
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    id_fn (xpath_pcb_t *pcb,
	   dlq_hdr_t *parmQ,
	   status_t  *res)
{
    xpath_result_t  *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    (void)parmQ;  /*****/

    result = new_result(pcb, XP_RT_NODESET);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
    } else {
	*res = NO_ERR;
    }

    /**** get all nodes with the specified ID ****/
    /**** No IDs in YANG so return empty nodeset ****/
    /**** Change this later if there are standard IDs ****/

    return result;

}  /* id_fn */


/********************************************************************
* FUNCTION lang_fn
* 
* boolean lang(string) function [4.3]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 parm; lang string to match
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    lang_fn (xpath_pcb_t *pcb,
	     dlq_hdr_t *parmQ,
	     status_t  *res)
{
    xpath_result_t  *parm, *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_STRING);
	return NULL;
    }

    /* no attributes in YANG, so no lang attr either */
    result = new_result(pcb, XP_RT_NODESET);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
    } else {
	*res = NO_ERR;
    }
    return result;

}  /* lang_fn */


/********************************************************************
* FUNCTION last_fn
* 
* number last() function [4.1]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == empty parmQ
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    last_fn (xpath_pcb_t *pcb,
	     dlq_hdr_t *parmQ,
	     status_t  *res)
{
    xpath_result_t *result;
    ncx_num_t       tempnum;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    (void)parmQ;
    *res = NO_ERR;
    result = new_result(pcb, XP_RT_NUMBER);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
    } else {
	ncx_init_num(&tempnum);
	tempnum.l = pcb->context.last;
	*res = ncx_cast_num(&tempnum, 
			    NCX_BT_INT64,
			    &result->r.num,
			    NCX_BT_FLOAT64);

	ncx_clean_num(NCX_BT_INT64, &tempnum);

	if (*res != NO_ERR) {
	    free_result(pcb, result);
	    result = NULL;
	}
    }

    return result;

}  /* last_fn */


/********************************************************************
* FUNCTION local_name_fn
* 
* string local-name(nodeset?) function [4.1]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with optional node-set parm 
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    local_name_fn (xpath_pcb_t *pcb,
		   dlq_hdr_t *parmQ,
		   status_t  *res)
{
    xpath_result_t  *parm, *result;
    xpath_resnode_t *resnode;
    const xmlChar   *str;
    uint32           parmcnt;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parmcnt = dlq_count(parmQ);
    if (parmcnt > 1) {
	*res = ERR_NCX_EXTRA_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm && parm->restype != XP_RT_NODESET) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_NODESET);
	return NULL;
    }

    str = NULL;
    if (parm) {
	resnode = (xpath_resnode_t *)
	    dlq_firstEntry(&parm->r.nodeQ);
	if (resnode) {
	    if (pcb->val) {
		str = resnode->node.valptr->name;
	    } else {
		str = obj_get_name(resnode->node.objptr);
	    }
	}
    } else {
	if (pcb->val) {
	    str = pcb->context.node.valptr->name;
	} else {
	    str = obj_get_name(pcb->context.node.objptr);
	}
    }
	    
    result = new_result(pcb, XP_RT_STRING);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    
    if (str) {
	result->r.str = xml_strdup(str);
    } else {
	result->r.str = xml_strdup(EMPTY_STRING);
    }

    if (!result->r.str) {
	*res = ERR_INTERNAL_MEM;
	free_result(pcb, result);
	return NULL;
    }

    return result;

}  /* local_name_fn */


/********************************************************************
* FUNCTION namespace_uri_fn
* 
* string namespace-uri(nodeset?) function [4.1]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with optional node-set parm 
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    namespace_uri_fn (xpath_pcb_t *pcb,
		      dlq_hdr_t *parmQ,
		      status_t  *res)
{
    xpath_result_t  *parm, *result;
    xpath_resnode_t *resnode;
    const xmlChar   *str;
    uint32           parmcnt;
    xmlns_id_t       nsid;


    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parmcnt = dlq_count(parmQ);
    if (parmcnt > 1) {
	*res = ERR_NCX_EXTRA_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm && parm->restype != XP_RT_NODESET) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_NODESET);
	return NULL;
    }

    nsid = 0;
    if (parm) {
	resnode = (xpath_resnode_t *)
	    dlq_firstEntry(&parm->r.nodeQ);
	if (resnode) {
	    if (pcb->val) {
		nsid = obj_get_nsid(resnode->node.valptr->obj);
	    } else {
		nsid = obj_get_nsid(resnode->node.objptr);
	    }
	}
    } else {
	if (pcb->val) {
	    nsid = obj_get_nsid(pcb->context.node.valptr->obj);
	} else {
	    nsid = obj_get_nsid(pcb->context.node.objptr);
	}
    }
		
    result = new_result(pcb, XP_RT_STRING);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    
    if (nsid) {
	str = xmlns_get_ns_name(nsid);
	result->r.str = xml_strdup(str);
    } else {
	result->r.str = xml_strdup(EMPTY_STRING);
    }

    if (!result->r.str) {
	*res = ERR_INTERNAL_MEM;
	free_result(pcb, result);
	return NULL;
    }

    return result;

}  /* namespace_uri_fn */


/********************************************************************
* FUNCTION name_fn
* 
* string name(nodeset?) function [4.1]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with optional node-set parm 
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    name_fn (xpath_pcb_t *pcb,
	     dlq_hdr_t *parmQ,
	     status_t  *res)
{
    xpath_result_t  *parm, *result;
    xpath_resnode_t *resnode;
    const xmlChar   *prefix, *name;
    xmlChar         *str;
    uint32           len, parmcnt;
    xmlns_id_t       nsid;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parmcnt = dlq_count(parmQ);
    if (parmcnt > 1) {
	*res = ERR_NCX_EXTRA_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm && parm->restype != XP_RT_NODESET) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_NODESET);
	return NULL;
    }

    nsid = 0;
    name = NULL;

    if (parm) {
	resnode = (xpath_resnode_t *)
	    dlq_firstEntry(&parm->r.nodeQ);
	if (resnode) {
	    if (pcb->val) {
		nsid = obj_get_nsid(resnode->node.valptr->obj);
		name = resnode->node.valptr->name;
	    } else {
		nsid = obj_get_nsid(resnode->node.objptr);
		name = obj_get_name(resnode->node.objptr);
	    }
	}
    } else {
	if (pcb->val) {
	    nsid = obj_get_nsid(pcb->context.node.valptr->obj);
	    name = pcb->context.node.valptr->name;
	} else {
	    nsid = obj_get_nsid(pcb->context.node.objptr);
	    name = obj_get_name(pcb->context.node.objptr);
	}
    }
		
    result = new_result(pcb, XP_RT_STRING);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    
    if (nsid) {
	prefix = xmlns_get_ns_prefix(nsid);
    }

    len = 0;
    if (prefix) {
	len += xml_strlen(prefix);
	len++;
    }
    if (name) {
	len += xml_strlen(name);
    } else {
	name = (const xmlChar *)"none";
	len += 4;
    }

    result->r.str = m__getMem(len+1);
    if (!result->r.str) {
	*res = ERR_INTERNAL_MEM;
	free_result(pcb, result);
	return NULL;
    }

    str = result->r.str;
    if (prefix) {
	str += xml_strcpy(str, prefix);
	*str++ = ':';
    }
    xml_strcpy(str, name);

    return result;

}  /* name_fn */


/********************************************************************
* FUNCTION normalize_space_fn
* 
* string normalize-space(string?) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with optional string to convert to normalized
*             string
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    normalize_space_fn (xpath_pcb_t *pcb,
			dlq_hdr_t *parmQ,
			status_t  *res)
{
    xpath_result_t  *parm, *result, *tempresult;
    const xmlChar   *teststr;
    xmlChar         *mallocstr, *str;
    uint32           parmcnt;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parmcnt = dlq_count(parmQ);
    if (parmcnt > 1) {
	*res = ERR_NCX_EXTRA_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm && parm->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_STRING);
	return NULL;
    }

    teststr = NULL;
    str = NULL;
    mallocstr = NULL;

    if (parm) {
	teststr = parm->r.str;
    } else {
	tempresult= new_nodeset(pcb,
				pcb->context.node.objptr,
				pcb->context.node.valptr,
				1, FALSE);

	if (!tempresult) {
	    *res = ERR_INTERNAL_MEM;
	    return NULL;
	} else {
	    *res = xpath_cvt_string(tempresult, &mallocstr);
	    if (*res == NO_ERR) {
		teststr = mallocstr;
	    }
	    free_result(pcb, tempresult);
	}
    }
    
    if (*res != NO_ERR) {
	return NULL;
    }

    result = new_result(pcb, XP_RT_STRING);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	if (mallocstr) {
	    m__free(mallocstr);
	}
	return NULL;
    }
    
    if (!teststr) {
	teststr = EMPTY_STRING;
    }

    result->r.str = m__getMem(xml_strlen(teststr)+1);
    if (!result->r.str) {
	*res = ERR_INTERNAL_MEM;
	free_result(pcb, result);
	if (mallocstr) {
	    m__free(mallocstr);
	}
	return NULL;
    }

    str = result->r.str;

    while (*teststr && xml_isspace(*teststr)) {
	teststr++;
    }

    while (*teststr) {
	if (xml_isspace(*teststr)) {
	    *str++ = ' ';
	    teststr++;
	    while (*teststr && xml_isspace(*teststr)) {
		teststr++;
	    }

	} else {
	    *str++ = *teststr++;
	}
    }

    if (str > result->r.str && *(str-1) == ' ') {
	str--;
    }

    *str = 0;

    if (mallocstr) {
	m__free(mallocstr);
    }

    return result;

}  /* normalize_space_fn */


/********************************************************************
* FUNCTION not_fn
* 
* boolean not(object) function [4.3]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 object to perform NOT boolean conversion
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    not_fn (xpath_pcb_t *pcb,
	    dlq_hdr_t *parmQ,
	    status_t  *res)
{
    xpath_result_t  *parm, *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm->restype != XP_RT_BOOLEAN) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_BOOLEAN);
	return NULL;
    }

    result = new_result(pcb, XP_RT_BOOLEAN);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
    } else {
	*res = NO_ERR;
    }

    result->r.bool = !parm->r.bool;

    return result;

}  /* not_fn */


/********************************************************************
* FUNCTION number_fn
* 
* number number(object?) function [4.4]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 optional object to convert to a number
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    number_fn (xpath_pcb_t *pcb,
	       dlq_hdr_t *parmQ,
	       status_t  *res)
{
    xpath_result_t  *parm, *result, *tempresult;
    uint32           parmcnt;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parmcnt = dlq_count(parmQ);
    if (parmcnt > 1) {
	*res = ERR_NCX_EXTRA_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    result = new_result(pcb, XP_RT_NUMBER);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    *res = NO_ERR;

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm) {
	xpath_cvt_number(parm, &result->r.num);
    } else {
	/* convert the context node value */
	if (pcb->val) {
	    /* get the value of the context node
	     * and convert it to a string first
	     */
	    tempresult= new_nodeset(pcb,
				    pcb->context.node.objptr,
				    pcb->context.node.valptr,
				    1, FALSE);

	    if (!tempresult) {
		*res = ERR_INTERNAL_MEM;
	    } else {
		xpath_cvt_number(tempresult, &result->r.num);
		free_result(pcb, tempresult);
	    }
	} else {
	    /* nothing to check; conversion to NaN is
	     * not an XPath error
	     */
	    ncx_set_num_zero(&result->r.num, NCX_BT_FLOAT64);
	}
    }

    if (*res != NO_ERR) {
	free_result(pcb, result);
	result = NULL;
    }

    return result;

}  /* number_fn */


/********************************************************************
* FUNCTION position_fn
* 
* number position() function [4.1]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == empty parmQ
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    position_fn (xpath_pcb_t *pcb,
		 dlq_hdr_t *parmQ,
		 status_t  *res)
{
    xpath_result_t *result;
    ncx_num_t       tempnum;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    (void)parmQ;
    *res = NO_ERR;
    result = new_result(pcb, XP_RT_NUMBER);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
    } else {
	ncx_init_num(&tempnum);
	tempnum.l = pcb->context.position;
	*res = ncx_cast_num(&tempnum, 
			    NCX_BT_INT64,
			    &result->r.num,
			    NCX_BT_FLOAT64);

	ncx_clean_num(NCX_BT_INT64, &tempnum);

	if (*res != NO_ERR) {
	    free_result(pcb, result);
	    result = NULL;
	}
    }

    return result;

}  /* position_fn */


/********************************************************************
* FUNCTION round_fn
* 
* number round(number) function [4.4]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 number to convert to round(number)
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    round_fn (xpath_pcb_t *pcb,
	      dlq_hdr_t *parmQ,
	      status_t  *res)
{
    xpath_result_t  *parm, *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (!parm) {
	*res = ERR_NCX_MISSING_PARM;
	return NULL;
    }

    result = new_result(pcb, XP_RT_NUMBER);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    if (parm->restype == XP_RT_NUMBER) {
	*res = ncx_round_num(&parm->r.num,
			     &result->r.num,
			     NCX_BT_FLOAT64);
    } else {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_NUMBER);
    }

    if (*res != NO_ERR) {
	free_result(pcb, result);
	result = NULL;
    }

    return result;

}  /* round_fn */


/********************************************************************
* FUNCTION starts_with_fn
* 
* boolean starts-with(string, string) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 2 strings
*             returns true if the 1st string starts with the 2nd string
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    starts_with_fn (xpath_pcb_t *pcb,
		    dlq_hdr_t *parmQ,
		    status_t  *res)
{
    xpath_result_t  *parm1, *parm2, *result;
    uint32           len1, len2;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm1 = (xpath_result_t *)dlq_firstEntry(parmQ);
    parm2 = (xpath_result_t *)dlq_nextEntry(parm1);

    if (parm1->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm1->restype, XP_RT_STRING);
    } else if (parm2->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm2->restype, XP_RT_STRING);
    } else {
	result = new_result(pcb, XP_RT_BOOLEAN);
	if (!result) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    len1 = xml_strlen(parm1->r.str);
	    len2 = xml_strlen(parm2->r.str);

	    if (len2 > len1) {
		result->r.bool = FALSE;
	    } else if (!xml_strncmp(parm1->r.str, 
				    parm2->r.str,
				    len2)) {
		result->r.bool = TRUE;
	    } else {
		result->r.bool = FALSE;
	    }
	    *res = NO_ERR;
	    return result;
	}
    }
    return NULL;

}  /* starts_with_fn */


/********************************************************************
* FUNCTION string_fn
* 
* string string(object?) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with optional object to convert to string
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    string_fn (xpath_pcb_t *pcb,
	       dlq_hdr_t *parmQ,
	       status_t  *res)
{
    xpath_result_t  *parm, *result, *tempresult;
    uint32           parmcnt;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parmcnt = dlq_count(parmQ);
    if (parmcnt > 1) {
	*res = ERR_NCX_EXTRA_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    result = new_result(pcb, XP_RT_STRING);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    *res = NO_ERR;

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm) {
	xpath_cvt_string(parm, &result->r.str);
    } else {
	/* convert the context node value */
	if (pcb->val) {
	    /* get the value of the context node
	     * and convert it to a string first
	     */
	    tempresult= new_nodeset(pcb,
				    pcb->context.node.objptr,
				    pcb->context.node.valptr,
				    1, FALSE);

	    if (!tempresult) {
		*res = ERR_INTERNAL_MEM;
	    } else {
		*res = xpath_cvt_string(tempresult, &result->r.str);
		free_result(pcb, tempresult);
	    }
	} else {
	    result->r.str = xml_strdup(EMPTY_STRING);
	    if (!result->r.str) {
		*res = ERR_INTERNAL_MEM;
	    }
	}
    }

    if (*res != NO_ERR) {
	free_result(pcb, result);
	result = NULL;
    }

    return result;

}  /* string_fn */


/********************************************************************
* FUNCTION string_length_fn
* 
* number string-length(string?) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with optional string to check length
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    string_length_fn (xpath_pcb_t *pcb,
		      dlq_hdr_t *parmQ,
		      status_t  *res)
{
    xpath_result_t  *parm, *result, *tempresult;
    xmlChar         *tempstr;
    uint32           parmcnt, len;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parmcnt = dlq_count(parmQ);
    if (parmcnt > 1) {
	*res = ERR_NCX_EXTRA_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    result = new_result(pcb, XP_RT_NUMBER);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    *res = NO_ERR;
    len = 0;

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm) {
	len = xml_strlen(result->r.str);
    } else {
	/* convert the context node value */
	if (pcb->val) {
	    /* get the value of the context node
	     * and convert it to a string first
	     */
	    tempresult= new_nodeset(pcb,
				    pcb->context.node.objptr,
				    pcb->context.node.valptr,
				    1, FALSE);

	    if (!tempresult) {
		*res = ERR_INTERNAL_MEM;
	    } else {
		tempstr = NULL;
		*res = xpath_cvt_string(tempresult, &tempstr);
		free_result(pcb, tempresult);
		if (*res == NO_ERR && tempstr) {
		    len = xml_strlen(tempstr);
		    m__free(tempstr);
		}
	    }
	} else {
	    len = 0;
	}
    }

    if (*res == NO_ERR) {
	set_uint32_num(len, &result->r.num);
    }

    if (*res != NO_ERR) {
	free_result(pcb, result);
	result = NULL;
    }

    return result;

}  /* string_length_fn */


/********************************************************************
* FUNCTION substring_fn
* 
* string substring-after(string, number, number?) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 2 or 3 parms
*             returns substring of 1st string starting at the
*             position indicated by the first number; copies
*             only N chars if the 2nd number is present
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    substring_fn (xpath_pcb_t *pcb,
		  dlq_hdr_t *parmQ,
		  status_t  *res)
{
    xpath_result_t  *parm1, *parm2, *parm3, *result;
    ncx_num_t        tempnum;
    uint32           parmcnt, maxlen;
    int64            startpos, copylen, slen;
    status_t         myres;
    boolean          copylenset;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    *res = NO_ERR;

    /* check at least 2 strings to concat */
    parmcnt = dlq_count(parmQ);
    if (parmcnt < 2) {
	*res = ERR_NCX_MISSING_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    } else if (parmcnt > 3) {
	*res = ERR_NCX_EXTRA_PARM;
	wrong_parmcnt_error(pcb, parmcnt, *res);
	return NULL;
    }

    parm1 = (xpath_result_t *)dlq_firstEntry(parmQ);
    parm2 = (xpath_result_t *)dlq_nextEntry(parm1);
    parm3 = (xpath_result_t *)dlq_nextEntry(parm2);

    if (parm1->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm1->restype, XP_RT_STRING);
    }

    if (parm2->restype != XP_RT_NUMBER) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm2->restype, XP_RT_NUMBER);
    }

    if (parm3 && parm3->restype != XP_RT_NUMBER) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm3->restype, XP_RT_NUMBER);
    }

    if (*res != NO_ERR) {
	return NULL;
    }

    startpos = 0;
    ncx_init_num(&tempnum);
    myres = ncx_round_num(&parm2->r.num, &tempnum, NCX_BT_FLOAT64);
    if (myres == NO_ERR) {
	startpos = ncx_cvt_to_int64(&tempnum, NCX_BT_FLOAT64);
    }
    ncx_clean_num(NCX_BT_FLOAT64, &tempnum);

    copylenset = FALSE;
    copylen = 0;
    if (parm3) {
	copylenset = TRUE;
	myres = ncx_round_num(&parm3->r.num, &tempnum, NCX_BT_FLOAT64);
	if (myres == NO_ERR) {
	    copylen = ncx_cvt_to_int64(&tempnum, NCX_BT_FLOAT64);

	}
	ncx_clean_num(NCX_BT_FLOAT64, &tempnum);
    }

    slen = (int64)xml_strlen(parm1->r.str);

    result = new_result(pcb, XP_RT_STRING);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    if (startpos > (slen+1) || (copylenset && copylen <= 0)) {
	/* starting after EOS */
	result->r.str = xml_strdup(EMPTY_STRING);
    } else if (copylenset) {
	if (startpos < 1) {
	    /* adjust startpos, then copy N chars */
	    copylen += (startpos-1);
	    startpos = 1;
	}
	if (copylen >= 1) {
	    if (copylen >= NCX_MAX_UINT) {
		/* internal max of strings way less than 4G
		 * so copy the whole string
		 */
		result->r.str = xml_strdup(&parm1->r.str[startpos-1]);
	    } else {
		/* copy at most N chars of whole string */
		maxlen = (uint32)copylen;
		result->r.str = xml_strndup(&parm1->r.str[startpos-1],
					    maxlen);
	    }
	} else {
	    result->r.str = xml_strdup(EMPTY_STRING);
	}
    } else if (startpos <= 1) {
	/* copying whole string */
	result->r.str = xml_strdup(parm1->r.str);
    } else {
	/* copying rest of string */
	result->r.str = xml_strdup(&parm1->r.str[startpos-1]);
    }

    if (!result->r.str) {
	*res = ERR_INTERNAL_MEM;
	free_result(pcb, result);
	result = NULL;
    }
	
    return result;

}  /* substring_fn */


/********************************************************************
* FUNCTION substring_after_fn
* 
* string substring-after(string, string) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 2 strings
*             returns substring of 1st string after
*             the occurance of the 2nd string
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    substring_after_fn (xpath_pcb_t *pcb,
			dlq_hdr_t *parmQ,
			status_t  *res)
{
    xpath_result_t  *parm1, *parm2, *result;
    const xmlChar   *str, *retstr;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm1 = (xpath_result_t *)dlq_firstEntry(parmQ);
    parm2 = (xpath_result_t *)dlq_nextEntry(parm1);

    if (parm1->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm1->restype, XP_RT_STRING);
    } else if (parm2->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm2->restype, XP_RT_STRING);
    } else {
	result = new_result(pcb, XP_RT_STRING);
	if (result) {
	    str = (const xmlChar *)
		strstr((const char *)parm1->r.str, 
		       (const char *)parm2->r.str);
	    if (str) {
		retstr = str + xml_strlen(parm2->r.str);
		result->r.str = m__getMem(xml_strlen(retstr)+1);
		if (result->r.str) {
		    xml_strcpy(result->r.str, retstr);
		}
	    } else {
		result->r.str = xml_strdup(EMPTY_STRING);
	    }

	    if (!result->r.str) {
		*res = ERR_INTERNAL_MEM;
	    }

	    if (*res != NO_ERR) {
		free_result(pcb, result);
		result = NULL;
	    }

	    return result;
	} else {
	    *res = ERR_INTERNAL_MEM;
	}
    }
    return NULL;

}  /* substring_after_fn */


/********************************************************************
* FUNCTION substring_before_fn
* 
* string substring-before(string, string) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 2 strings
*             returns substring of 1st string that precedes
*             the occurance of the 2nd string
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    substring_before_fn (xpath_pcb_t *pcb,
			 dlq_hdr_t *parmQ,
			 status_t  *res)
{
    xpath_result_t  *parm1, *parm2, *result;
    const xmlChar   *str;
    uint32           retlen;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm1 = (xpath_result_t *)dlq_firstEntry(parmQ);
    parm2 = (xpath_result_t *)dlq_nextEntry(parm1);

    if (parm1->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm1->restype, XP_RT_STRING);
    } else if (parm2->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm2->restype, XP_RT_STRING);
    } else {
	result = new_result(pcb, XP_RT_STRING);
	if (result) {
	    str = (const xmlChar *)
		strstr((const char *)parm1->r.str, 
		       (const char *)parm2->r.str);
	    if (str) {
		retlen = (uint32)(str - parm1->r.str);
		result->r.str = m__getMem(retlen+1);
		if (result->r.str) {
		    xml_strncpy(result->r.str,
				parm1->r.str,
				retlen);
		}
	    } else {
		result->r.str = xml_strdup(EMPTY_STRING);
	    }

	    if (!result->r.str) {
		*res = ERR_INTERNAL_MEM;
	    }

	    if (*res != NO_ERR) {
		free_result(pcb, result);
		result = NULL;
	    }

	    return result;
	} else {
	    *res = ERR_INTERNAL_MEM;
	}
    }
    return NULL;

}  /* substring_before_fn */


/********************************************************************
* FUNCTION sum_fn
* 
* number sum(nodeset) function [4.4]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 1 nodeset to convert to numbers
*             and add together to resurn the total
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    sum_fn (xpath_pcb_t *pcb,
	    dlq_hdr_t *parmQ,
	    status_t  *res)
{
    xpath_result_t   *parm, *result;
    xpath_resnode_t  *resnode;
    val_value_t      *val;
    ncx_num_t         tempnum, mysum;
    status_t          myres;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    parm = (xpath_result_t *)dlq_firstEntry(parmQ);
    if (parm->restype != XP_RT_NODESET) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm->restype, XP_RT_NODESET);
	return NULL;
    }

    result = new_result(pcb, XP_RT_NUMBER);
    if (result) {
	if (pcb->val) {

	    ncx_init_num(&tempnum);
	    ncx_set_num_zero(&tempnum, NCX_BT_FLOAT64);
	    ncx_init_num(&mysum);
	    ncx_set_num_zero(&mysum, NCX_BT_FLOAT64);
	    myres = NO_ERR;

	    for (resnode = (xpath_resnode_t *)
		     dlq_firstEntry(&parm->r.nodeQ);
		 resnode != NULL && myres == NO_ERR;
		 resnode = (xpath_resnode_t *)
		     dlq_nextEntry(resnode)) {

		val = val_get_first_leaf(resnode->node.valptr);
		if (val && typ_is_number(val->btyp)) {
		    myres = ncx_cast_num(&val->v.num,
				       val->btyp,
				       &tempnum,
				       NCX_BT_FLOAT64);
		    if (myres == NO_ERR) {
			mysum.d += tempnum.d;
		    }
		} else {
		    myres = ERR_NCX_INVALID_VALUE;
		}
	    }

	    if (myres == NO_ERR) {
		result->r.num.d = mysum.d;
	    } else {
		ncx_set_num_nan(&result->r.num, NCX_BT_FLOAT64);
	    }
	} else {
	    ncx_set_num_zero(&result->r.num, NCX_BT_FLOAT64);
	}

	return result;
    } else {
	*res = ERR_INTERNAL_MEM;
    }
    return result;

}  /* sum_fn */


/********************************************************************
* FUNCTION translate_fn
* 
* string translate(string, string, string) function [4.2]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == parmQ with 3 strings to translate
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    translate_fn (xpath_pcb_t *pcb,
		  dlq_hdr_t *parmQ,
		  status_t  *res)
{
    xpath_result_t  *parm1, *parm2, *parm3, *result;
    xmlChar         *p1str, *p2str, *p3str, *writestr;
    uint32           parmcnt, p1len, p2len, p3len, curpos;
    boolean          done;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    *res = NO_ERR;

    /* check at least 2 strings to concat */
    parmcnt = dlq_count(parmQ);

    parm1 = (xpath_result_t *)dlq_firstEntry(parmQ);
    parm2 = (xpath_result_t *)dlq_nextEntry(parm1);
    parm3 = (xpath_result_t *)dlq_nextEntry(parm2);

    if (parm1->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm1->restype, XP_RT_STRING);
    } else {
	p1str = parm1->r.str;
	p1len = xml_strlen(p1str);
    }
	
    if (parm2->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm2->restype, XP_RT_STRING);
    } else {
	p2str = parm2->r.str;
	p2len = xml_strlen(p2str);
    }

    if (parm3->restype != XP_RT_STRING) {
	*res = ERR_NCX_WRONG_DATATYP;
	wrong_type_error(pcb, parm3->restype, XP_RT_NUMBER);
    } else {
	p3str = parm3->r.str;
	p3len = xml_strlen(p3str);
    }

    if (*res != NO_ERR) {
	return NULL;
    }

    result = new_result(pcb, XP_RT_STRING);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    result->r.str = m__getMem(p1len+1);
    if (!result->r.str) {
	*res = ERR_INTERNAL_MEM;
	free_result(pcb, result);
	result = NULL;
    }

    writestr = result->r.str;

    /* translate p1str into the result string */
    while (*p1str) {
	curpos = 0;
	done = FALSE;

	/* look for a match char in p2str */
	while (!done && curpos < p2len) {
	    if (p2str[curpos] == *p1str) {
		done = TRUE;
	    } else {
		curpos++;
	    }
	}

	if (done) {
	    if (curpos < p3len) {
		/* replace p1char with p3str translation char */
		*writestr++ = p3str[curpos];
	    } /* else drop p1char from result */
	} else {
	    /* copy p1char to result */
	    *writestr++ = *p1str;
	}

	p1str++;
    }

    *writestr = 0;

    return result;

}  /* translate_fn */


/********************************************************************
* FUNCTION true_fn
* 
* boolean true() function [4.3]
*
* INPUTS:
*    pcb == parser control block to use
*    parmQ == empty parmQ
*    res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced xpath_result_t if no error and results being processed
*    NULL if error
*********************************************************************/
static xpath_result_t *
    true_fn (xpath_pcb_t *pcb,
	     dlq_hdr_t *parmQ,
	     status_t  *res)
{
    xpath_result_t *result;

    if (!pcb->val && !pcb->obj) {
	return NULL;
    }

    (void)parmQ;
    result = new_result(pcb, XP_RT_BOOLEAN);
    if (!result) {
	*res = ERR_INTERNAL_MEM;
    } else {
	result->r.bool = TRUE;
	*res = NO_ERR;
    }
    return result;

}  /* true_fn */


/****************   U T I L I T Y    F U N C T I O N S   ***********/


/********************************************************************
* FUNCTION find_fncb
* 
* Find an XPath function control block
*
* INPUTS:
*    pcb == parser control block to check
*    name == name string to check
*
* RETURNS:
*   pointer to found control block
*   NULL if not found
*********************************************************************/
static const xpath_fncb_t *
    find_fncb (xpath_pcb_t *pcb,
	       const xmlChar *name)
{
    const xpath_fncb_t  *fncb;
    uint32               i;

    i = 0;
    fncb = &pcb->functions[i];
    while (fncb && fncb->name) {
	if (!xml_strcmp(name, fncb->name)) {
	    return fncb;
	} else {
	    fncb = &pcb->functions[++i];
	}
    }
    return NULL;

} /* find_fncb */


/********************************************************************
* FUNCTION get_varbind
* 
* Get the specified variable binding
*
* INPUTS:
*    pcb == parser control block in progress
*    prefix == prefix string of module with the varbind
*           == NULL for current module (pcb->mod)
*    prefixlen == length of prefix string
*    name == variable name string
*     res == address of return status
*
* OUTPUTS:
*   *res == resturn status
*
* RETURNS:
*   pointer to found varbind, NULL if not found
*********************************************************************/
static ncx_var_t *
    get_varbind (xpath_pcb_t *pcb,
		 const xmlChar *prefix,
		 uint32 prefixlen,
		 const xmlChar *name,
		 status_t  *res)
{
    ncx_module_t *targmod;
    ncx_var_t    *var;
    xmlns_id_t    nsid;

    nsid = 0;
    *res = NO_ERR;

    /* check if prefix set and specifies an import */
    if (pcb->source != XP_SRC_XML) {
	if (prefix && prefixlen && 
	    xml_strncmp(pcb->mod->prefix, prefix, prefixlen)
	    && (xml_strlen(pcb->mod->prefix) != prefixlen)) {

	    *res = xpath_get_curmod_from_prefix_str(prefix,
						    prefixlen,
						    pcb->mod,
						    &targmod);
	    if (*res != NO_ERR) {
		return NULL;
	    }
	    nsid = targmod->nsid;
	}

	/* try to find the variable */
	var = var_get_que_raw(pcb->varbindQ, nsid, name);
	if (!var) {
	    *res = ERR_NCX_DEF_NOT_FOUND;
	}
	return var;
    } else {
	/*** agent TBD ***/
	*res = ERR_NCX_DEF_NOT_FOUND;
	return NULL;
    }
    /*NOTREACHED*/

}  /* get_varbind */


/********************************************************************
* FUNCTION match_next_token
* 
* Match the next token in the chain with a type and possibly value
*
* INPUTS:
*    pcb == parser control block in progress
*    tktyp == token type to match
*    tkval == string val to match (or NULL to just match tktyp)
*
* RETURNS:
*   TRUE if the type and value (if non-NULL) match
*   FALSE if next token is not a match
*********************************************************************/
static boolean
    match_next_token (xpath_pcb_t *pcb,
                      tk_type_t tktyp,
                      const xmlChar *tkval)
{
    const xmlChar  *nextval;
    tk_type_t       nexttyp;

    nexttyp = tk_next_typ(pcb->tkc);
    if (nexttyp == tktyp) {
        if (tkval) {
            nextval = tk_next_val(pcb->tkc);
            if (nextval && !xml_strcmp(tkval, nextval)) {
                return TRUE;
            } else {
                return FALSE;
            }
        } else {
            return TRUE;
        }
    } else {
        return FALSE;
    }
    /*NOTREACHED*/

}  /* match_next_token */


/********************************************************************
* FUNCTION check_qname_prefix
* 
* Check the prefix for the current node against the proper context
* and report any unresolved prefix errors
*
* Do not check the local-name part of the QName because
* the module is still being parsed and out of order
* nodes will not be found yet
*
* INPUTS:
*    pcb == parser control block in progress
*    nsid == address of return NS ID (may be NULL)
*
* OUTPUTS:
*   if non-NULL:
*    *nsid == namespace ID for the prefix, if found
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_qname_prefix (xpath_pcb_t *pcb,
			xmlns_id_t *nsid)
{
    const xmlChar  *prefix;
    ncx_module_t   *targmod;
    uint32          prefixlen;
    status_t        res;

    res = NO_ERR;
    if (pcb->source != XP_SRC_XML) {
	prefix = TK_CUR_MOD(pcb->tkc);
	prefixlen = TK_CUR_MODLEN(pcb->tkc);

	res = xpath_get_curmod_from_prefix_str(prefix, prefixlen,
					       pcb->mod, &targmod);

	if (res != NO_ERR) {
	    if (pcb->logerrors) {
		log_error("\nError: Module for prefix '%s' not found",
			  (prefix) ? prefix : EMPTY_STRING);
		ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	    }
	} else if (nsid) {
	    *nsid = targmod->nsid;
	}
    } else {
	/*** check agent prefix ***/;
    }

    return res;
    
}  /* check_qname_prefix */


/********************************************************************
* FUNCTION cvt_from_value
* 
* Convert a val_value_t into an XPath result
*
* INPUTS:
*    pcb == parser control block to use
*    val == value struct to convert
*
* RETURNS:
*   malloced and filled in xpath_result_t or NULL if malloc error
*********************************************************************/
static xpath_result_t *
    cvt_from_value (xpath_pcb_t *pcb,
		    const val_value_t *val)
{
    xpath_result_t   *result;
    status_t          res;
    uint32            len;

    result = NULL;
    if (typ_is_string(val->btyp)) {
	result = new_result(pcb, XP_RT_STRING);
	if (result) {
	    if (VAL_STR(val)) {
		result->r.str = xml_strdup(VAL_STR(val));
	    } else {
		result->r.str = xml_strdup(EMPTY_STRING);
	    }
	    if (!result->r.str) {
		free_result(pcb, result);
		result = NULL;
	    }
	}
    } else if (typ_is_number(val->btyp)) {
	result = new_result(pcb, XP_RT_NUMBER);
	if (result) {
	    res = ncx_cast_num(&val->v.num, 
			       val->btyp,
			       &result->r.num,
			       NCX_BT_FLOAT64);
	    if (res != NO_ERR) {
		free_result(pcb, result);
		result = NULL;
	    }
	}
    } else if (typ_is_simple(val->btyp)) {
	len = 0;
	res = val_sprintf_simval_nc(NULL, val, &len);
	if (res == NO_ERR) {
	    result = new_result(pcb, NCX_BT_STRING);
	    if (result) {
		result->r.str = m__getMem(len+1);
		if (!result->r.str) {
		    free_result(pcb, result);
		    result = NULL;
		} else {
		    res = val_sprintf_simval_nc
			(result->r.str, val, &len);
		    if (res != NO_ERR) {
			free_result(pcb, result);
			result = NULL;
		    }
		}
	    }
	}
    }

    return result;

}  /* cvt_from_value */


/********************************************************************
* FUNCTION find_resnode
* 
* Check if the specified resnode ptr is already in the Q
*
* INPUTS:
*    pcb == parser control block to use
*    resultQ == Q of xpath_resnode_t structs to check
*               DOES NOT HAVE TO BE WITHIN A RESULT NODE Q
*    ptr   == pointer value to find
*
* RETURNS:
*    found resnode or NULL if not found
*********************************************************************/
static xpath_resnode_t *
    find_resnode (xpath_pcb_t *pcb,
		  dlq_hdr_t *resultQ,
		  const void *ptr)
{
    xpath_resnode_t  *resnode;

    for (resnode = (xpath_resnode_t *)dlq_firstEntry(resultQ);
	 resnode != NULL;
	 resnode = (xpath_resnode_t *)dlq_nextEntry(resnode)) {

	if (pcb->val) {
	    if (resnode->node.valptr == ptr) {
		return resnode;
	    }
	} else {
	    if (resnode->node.objptr == ptr) {
		return resnode;
	    }
	}
    }
    return NULL;

}  /* find_resnode */

#if 0
/********************************************************************
* FUNCTION check_node_exists
* 
* Check if any ancestor-ot-self node is already in the specified Q
*
* This is only done after all the nodes have been processed
* and the nodeset is complete.  For NETCONF purposes,
* the entire path to root is added for the context node,
* and the entire context node contexts are always returned
*
* INPUTS:
*    pcb == parser control block to use
*    resultQ == Q of xpath_resnode_t structs to check
*               DOES NOT HAVE TO BE WITHIN A RESULT NODE Q
*    ptr   == node pointer value to find
*
* RETURNS:
*    TRUE if found, FALSE otherwise
*********************************************************************/
static boolean
    check_node_exists (xpath_pcb_t *pcb,
		       dlq_hdr_t *resultQ,
		       void *ptr)
{
    obj_template_t   *obj;
    val_value_t      *val;

    /* quick test -- see if docroot is already in the Q
     * which means nothing else is needed
     */
    if (pcb->val) {
	if (find_resnode(pcb, resultQ, pcb->val_docroot)) {
	    return TRUE;
	}
    } else if (pcb->obj) {
	if (find_resnode(pcb, resultQ, pcb->docroot)) {
	    return TRUE;
	}
    } else {
	return FALSE;
    }

    /* no docroot in the Q so check the node itself */
    if (pcb->val) {
	val = (val_value_t *)ptr;
	if (val == pcb->val_docroot) {
	    return FALSE;
	}
	
	while (val) {
	    if (find_resnode(pcb, resultQ, val)) {
		return TRUE;
	    }

	    if (val->parent && 
		val->parent != pcb->val_docroot) {
		val = val->parent;
	    } else {
		return FALSE;
	    }
	}
    } else {
	obj = (obj_template_t *)ptr;
	if (obj == pcb->docroot) {
	    return FALSE;
	}

	while (obj) {
	    if (!(obj->objtype == OBJ_TYP_CHOICE ||
		  obj->objtype == OBJ_TYP_CASE)) {
		if (find_resnode(pcb, resultQ, obj)) {
		    return TRUE;
		}
	    }

	    if (obj->parent && obj->parent != pcb->docroot) {
		obj = obj->parent;
	    } else {
		return FALSE;
	    }
	}
    }
    return FALSE;

}  /* check_node_exists */
#endif


/********************************************************************
* FUNCTION merge_nodeset
* 
* Add the nodes from val1 into val2
* that are not already there
*
* INPUTS:
*    pcb == parser control block to use
*    result == address of return XPath result nodeset
*
* OUTPUTS:
*    result->nodeQ contents adjusted
*
*********************************************************************/
static void
    merge_nodeset (xpath_pcb_t *pcb,
		   xpath_result_t *val1,
		   xpath_result_t *val2)
{
    xpath_resnode_t        *resnode, *findnode;

    if (!pcb->val && !pcb->obj) {
	return;
    }

    if (val1->restype != XP_RT_NODESET ||
	val2->restype != XP_RT_NODESET) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    while (!dlq_empty(&val1->r.nodeQ)) {
	resnode = (xpath_resnode_t *)dlq_deque(&val1->r.nodeQ);

	if (pcb->val) {
	    findnode = find_resnode(pcb, &val2->r.nodeQ,
				    resnode->node.valptr);
	} else {
	    findnode = find_resnode(pcb, &val2->r.nodeQ,
				    resnode->node.objptr);
	}
	if (findnode) {
	    if (resnode->dblslash) {
		findnode->dblslash = TRUE;
	    }
	    findnode->position = resnode->position;
	    free_resnode(pcb, resnode);
	} else {
	    dlq_enque(resnode, &val2->r.nodeQ);
	}
    }

}  /* merge_nodeset */


/********************************************************************
* FUNCTION set_nodeset_dblslash
* 
* Set the current nodes dblslash flag
* to indicate that all unchecked descendants
* are also represented by this node, for
* further node-test or predicate testing
*
* INPUTS:
*    pcb == parser control block to use
*    result == address of return XPath result nodeset
*
* OUTPUTS:
*    result->nodeQ contents adjusted
*
*********************************************************************/
static void
    set_nodeset_dblslash (xpath_pcb_t *pcb,
			  xpath_result_t *result)
{
    xpath_resnode_t        *resnode;

    if (!pcb->val && !pcb->obj) {
	return;
    }

    for (resnode = (xpath_resnode_t *)
	     dlq_firstEntry(&result->r.nodeQ);
	 resnode != NULL;
	 resnode = (xpath_resnode_t *)dlq_nextEntry(resnode)) {

	resnode->dblslash = TRUE;
    }

}  /* set_nodeset_dblslash */


/********************************************************************
* FUNCTION value_walker_fn
* 
* Check the current found value node, based on the
* criteria passed to the search function
*
* Matches val_walker_fn_t template in val.h
*
* INPUTS:
*    val == value node found in the search
*    cookie1 == xpath_pcb_t * : parser control block to use
*    cookie2 == xpath_walkerparms_t *: walker parms to use
*
* OUTPUTS:
*    result->nodeQ contents adjusted or replaced
*
* RETURNS:
*    TRUE to keep walk going
*    FALSE to terminate walk
*********************************************************************/
static boolean
    value_walker_fn (val_value_t *val,
		     void *cookie1,
		     void *cookie2)
{
    xpath_pcb_t          *pcb;
    xpath_walkerparms_t  *parms;
    xpath_resnode_t      *newresnode;

    pcb = (xpath_pcb_t *)cookie1;
    parms = (xpath_walkerparms_t *)cookie2;

    /* check if this node is already in the result */
    if (find_resnode(pcb, parms->resnodeQ, val)) {
	return TRUE;
    }

    /* need to add this node */
    newresnode = new_val_resnode(pcb, 
				 ++parms->callcount, 
				 FALSE,
				 val);
    if (!newresnode) {
	parms->res = ERR_INTERNAL_MEM;
	return FALSE;
    }

    dlq_enque(newresnode, parms->resnodeQ);
    return TRUE;

}  /* value_walker_fn */


/********************************************************************
* FUNCTION object_walker_fn
* 
* Check the current found object node, based on the
* criteria passed to the search function
*
* Matches obj_walker_fn_t template in obj.h
*
* INPUTS:
*    val == value node found in the search
*    cookie1 == xpath_pcb_t * : parser control block to use
*    cookie2 == xpath_walkerparms_t *: walker parms to use
*
* OUTPUTS:
*    result->nodeQ contents adjusted or replaced
*
* RETURNS:
*    TRUE to keep walk going
*    FALSE to terminate walk
*********************************************************************/
static boolean
    object_walker_fn (const obj_template_t *obj,
		      void *cookie1,
		      void *cookie2)
{
    xpath_pcb_t          *pcb;
    xpath_walkerparms_t  *parms;
    xpath_resnode_t      *newresnode;

    pcb = (xpath_pcb_t *)cookie1;
    parms = (xpath_walkerparms_t *)cookie2;

    /* check if this node is already in the result */
    if (find_resnode(pcb, parms->resnodeQ, obj)) {
	return TRUE;
    }

    /* need to add this child node */
    newresnode = new_obj_resnode(pcb, 
				 ++parms->callcount, 
				 FALSE,
				 obj);
    if (!newresnode) {
	parms->res = ERR_INTERNAL_MEM;
	return FALSE;
    }

    dlq_enque(newresnode, parms->resnodeQ);
    return TRUE;

}  /* object_walker_fn */


/********************************************************************
* FUNCTION set_nodeset_self
* 
* Check the current result nodeset and keep each
* node, or remove it if filter tests fail
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*    result == address of return XPath result nodeset
*    nsid == 0 if any namespace is OK
*              else only this namespace will be checked
*    name == name of parent to find
*         == NULL to find any parent name
*    textmode == TRUE if just selecting text() nodes
*                FALSE if ignored
*
* OUTPUTS:
*    result->nodeQ contents adjusted or removed
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    set_nodeset_self (xpath_pcb_t *pcb,
		      xpath_result_t *result,
		      xmlns_id_t nsid,
		      const xmlChar *name,
		      boolean textmode)
{
    xpath_resnode_t        *resnode, *findnode;
    const obj_template_t   *testobj;
    const xmlChar           *modname;
    val_value_t            *testval;
    boolean                 keep, cfgonly, fnresult, fncalled, useroot;
    dlq_hdr_t               resnodeQ;
    status_t                res;
    xpath_walkerparms_t     walkerparms;

    if (!pcb->val && !pcb->obj) {
	return NO_ERR;
    }

    if (dlq_empty(&result->r.nodeQ)) {
	return NO_ERR;
    }

    dlq_createSQue(&resnodeQ);

    walkerparms.resnodeQ = &resnodeQ;
    walkerparms.res = NO_ERR;
    walkerparms.callcount = 0;

    modname = (nsid) ? xmlns_get_module(nsid) : NULL;
    useroot = (pcb->flags & XP_FL_USEROOT) ? TRUE : FALSE;

    res = NO_ERR;

    /* the resnodes need to be deleted or moved to a tempQ
     * to correctly track duplicates and remove them
     */
    while (!dlq_empty(&result->r.nodeQ) && res == NO_ERR) {

	resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);

	if (resnode->dblslash) {
	    if (pcb->val) {
		testval = resnode->node.valptr;
		cfgonly = obj_is_config(testval->obj);

		fnresult = 
		    val_find_all_descendants(value_walker_fn,
					     pcb, 
					     &walkerparms,
					     testval, 
					     modname,
					     name,
					     cfgonly,
					     textmode,
					     TRUE);
	    } else {
		testobj = resnode->node.objptr;
		cfgonly = obj_is_config(testobj);

		fnresult = 
		    obj_find_all_descendants(pcb->objmod,
					     object_walker_fn,
					     pcb, 
					     &walkerparms,
					     testobj,
					     modname,
					     name,
					     cfgonly,
					     textmode,
					     useroot,
					     TRUE,
					     &fncalled);
	    }

	    if (!fnresult || walkerparms.res != NO_ERR) {
		res = walkerparms.res;
	    }
	    free_resnode(pcb, resnode);
	} else {
	    keep = FALSE;

	    if (pcb->val) {
		testval = resnode->node.valptr;
	    
		if (textmode) {
		    if (obj_has_text_content(testval->obj)) {
			keep = TRUE;
		    }
		} else if (modname && name) {
		    if (!xml_strcmp(modname,
				    obj_get_mod_name(testval->obj)) &&
			!xml_strcmp(name, testval->name)) {
			keep = TRUE;
		    }

		} else if (modname) {
		    if (!xml_strcmp(modname,
				    obj_get_mod_name(testval->obj))) {
			keep = TRUE;
		    }
		} else if (name) {
		    if (!xml_strcmp(name, testval->name)) {
			keep = TRUE;
		    }
		} else {
		    keep = TRUE;
		}

		if (keep) {
		    findnode = find_resnode(pcb, 
					    &resnodeQ, 
					    testval);
		    if (findnode) {
			if (resnode->dblslash) {
			    findnode->dblslash = TRUE;
			    findnode->position = 
				++walkerparms.callcount;
			}
			free_resnode(pcb, resnode);
		    } else {
			/* set the resnode to its parent */
			resnode->node.valptr = testval;
			resnode->position = 
			    ++walkerparms.callcount;
			dlq_enque(resnode, &resnodeQ);
		    }
		} else {
		    free_resnode(pcb, resnode);
		}
	    } else {
		testobj = resnode->node.objptr;

		if (textmode) {
		    if (obj_has_text_content(testobj)) {
			keep = TRUE;
		    }
		} else if (modname && name) {
		    if (!xml_strcmp(modname,
				    obj_get_mod_name(testobj)) &&
			!xml_strcmp(name, obj_get_name(testobj))) {
			keep = TRUE;
		    }
		} else if (modname) {
		    if (!xml_strcmp(modname,
				    obj_get_mod_name(testobj))) {
			keep = TRUE;
		    }
		} else if (name) {
		    if (!xml_strcmp(name, obj_get_name(testobj))) {
			keep = TRUE;
		    }
		} else {
		    keep = TRUE;
		}

		if (keep) {
		    findnode = find_resnode(pcb, 
					    &resnodeQ, 
					    testobj);
		    if (findnode) {
			if (resnode->dblslash) {
			    findnode->dblslash = TRUE;
			    findnode->position = 
				++walkerparms.callcount;
			}
			free_resnode(pcb, resnode);
		    } else {
			/* set the resnode to its parent */
			resnode->node.objptr = testobj;
			resnode->position =
			    ++walkerparms.callcount;
			dlq_enque(resnode, &resnodeQ);
		    }
		} else {
		    if (pcb->logerrors) {
			log_warn("\nWarning: no self node found "
				 "in XPath expr '%s'", pcb->exprstr);
			ncx_print_errormsg(pcb->tkc, 
					   pcb->objmod, 
					   ERR_NCX_NO_XPATH_NODES);
		    }
		    free_resnode(pcb, resnode);
		}
	    }
	}
    }

    /* put the resnode entries back where they belong */
    if (!dlq_empty(&resnodeQ)) {
	dlq_block_enque(&resnodeQ, &result->r.nodeQ);
    }

    result->last = walkerparms.callcount;

    return res;

}  /* set_nodeset_self */


/********************************************************************
* FUNCTION set_nodeset_parent
* 
* Check the current result nodeset and move each
* node to its parent, or remove it if no parent
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*    result == address of return XPath result nodeset
*    nsid == 0 if any namespace is OK
*              else only this namespace will be checked
*    name == name of parent to find
*         == NULL to find any parent name
*
* OUTPUTS:
*    result->nodeQ contents adjusted or removed
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    set_nodeset_parent (xpath_pcb_t *pcb,
			xpath_result_t *result,
			xmlns_id_t nsid,
			const xmlChar *name)
{
    xpath_resnode_t        *resnode, *findnode;
    const obj_template_t   *testobj, *useobj;
    const xmlChar           *modname;
    val_value_t            *testval;
    boolean                 keep, cfgonly, fnresult, fncalled, useroot;
    dlq_hdr_t               resnodeQ;
    status_t                res;
    xpath_walkerparms_t     walkerparms;

    if (!pcb->val && !pcb->obj) {
	return NO_ERR;
    }

    if (dlq_empty(&result->r.nodeQ)) {
	return NO_ERR;
    }

    dlq_createSQue(&resnodeQ);

    walkerparms.resnodeQ = &resnodeQ;
    walkerparms.res = NO_ERR;
    walkerparms.callcount = 0;

    modname = (nsid) ? xmlns_get_module(nsid) : NULL;
    useroot = (pcb->flags & XP_FL_USEROOT) ? TRUE : FALSE;

    res = NO_ERR;

    /* the resnodes need to be deleted or moved to a tempQ
     * to correctly track duplicates and remove them
     */
    while (!dlq_empty(&result->r.nodeQ) && res == NO_ERR) {

	resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);

	if (resnode->dblslash) {
	    if (pcb->val) {
		testval = resnode->node.valptr;
		cfgonly = obj_is_config(testval->obj);

		fnresult = 
		    val_find_all_descendants(value_walker_fn,
					     pcb, 
					     &walkerparms,
					     testval, 
					     modname,
					     name,
					     cfgonly,
					     FALSE,
					     TRUE);
	    } else {
		testobj = resnode->node.objptr;
		cfgonly = obj_is_config(testobj);

		fnresult = 
		    obj_find_all_descendants(pcb->objmod,
					     object_walker_fn,
					     pcb, 
					     &walkerparms,
					     testobj,
					     modname,
					     name,
					     cfgonly,
					     FALSE,
					     useroot,
					     TRUE,
					     &fncalled);
	    }

	    if (!fnresult || walkerparms.res != NO_ERR) {
		res = walkerparms.res;
	    }
	    free_resnode(pcb, resnode);
	} else {
	    keep = FALSE;

	    if (pcb->val) {
		testval = resnode->node.valptr;
	    
		if (testval == pcb->val_docroot) {
		    if (resnode->dblslash) {
			/* just move this node to the result */
			resnode->position = 
			    ++walkerparms.callcount;
			dlq_enque(resnode, &resnodeQ);
		    } else {
			/* no parent available error
			 * remove node from result 
			 */
			no_parent_warning(pcb);
			free_resnode(pcb, resnode);
		    }
		} else {
		    testval = testval->parent;
		    
		    if (modname && name) {
			if (!xml_strcmp(modname,
					obj_get_mod_name(testval->obj)) &&
			    !xml_strcmp(name, testval->name)) {
			    keep = TRUE;
			}
		    } else if (modname) {
			if (!xml_strcmp(modname,
					obj_get_mod_name(testval->obj))) {
			    keep = TRUE;
			}
		    } else if (name) {
			if (!xml_strcmp(name, testval->name)) {
			    keep = TRUE;
			}
		    } else {
			keep = TRUE;
		    }

		    if (keep) {
			findnode = find_resnode(pcb, 
						&resnodeQ, 
						testval);
			if (findnode) {
			    /* parent already in the Q
			     * remove node from result 
			     */
			    if (resnode->dblslash) {
				findnode->position = 
				    ++walkerparms.callcount;
				findnode->dblslash = TRUE;
			    }
			    free_resnode(pcb, resnode);
			} else {
			    /* set the resnode to its parent */
			    resnode->position = 
				++walkerparms.callcount;
			    resnode->node.valptr = testval->parent;
			    dlq_enque(resnode, &resnodeQ);
			}
		    } else {
			/* no parent available error
			 * remove node from result 
			 */
			no_parent_warning(pcb);
			free_resnode(pcb, resnode);
		    }
		}
	    } else {
		testobj = resnode->node.objptr;
		if (testobj == pcb->docroot) {
		    if (resnode->dblslash) {
			resnode->position = 
			    ++walkerparms.callcount;
			dlq_enque(resnode, &resnodeQ);
		    } else {
			/* this is an RPC or notification */
			no_parent_warning(pcb);
			free_resnode(pcb, resnode);
		    }
		} else if (!testobj->parent) {
		    if (!resnode->dblslash && (modname || name)) {
			no_parent_warning(pcb);
			free_resnode(pcb, resnode);
		    } else {
			/* this is a databd node */
			findnode = find_resnode(pcb, 
						&resnodeQ, 
						pcb->docroot);
			if (findnode) {
			    if (resnode->dblslash) {
				findnode->position = 
				    ++walkerparms.callcount;
				findnode->dblslash = TRUE;
			    }
			    free_resnode(pcb, resnode);
			} else {
			    resnode->position = 
				++walkerparms.callcount;
			    resnode->node.objptr = pcb->docroot;
			    dlq_enque(resnode, &resnodeQ);
			}
		    }
		} else {
		    /* find a parent but not a case or choice */
		    testobj = testobj->parent;
		    while (testobj && testobj->parent && 
			   !obj_is_toproot(testobj->parent) &&
			   (testobj->objtype==OBJ_TYP_CHOICE ||
			    testobj->objtype==OBJ_TYP_CASE)) {
			testobj = testobj->parent;
		    }

		    /* check if stopped on top-level choice
		     * a top-level case should not happen
		     * but check it anyway
		     */
		    if (testobj->objtype==OBJ_TYP_CHOICE ||
			testobj->objtype==OBJ_TYP_CASE) {
			useobj = pcb->docroot;
		    } else {
			useobj = testobj;
		    }

		    if (modname && name) {
			if (!xml_strcmp(modname,
					obj_get_mod_name(useobj)) &&
			    !xml_strcmp(name, obj_get_name(useobj))) {
			    keep = TRUE;
			}
		    } else if (modname) {
			if (!xml_strcmp(modname,
					obj_get_mod_name(useobj))) {
			    keep = TRUE;
			}
		    } else if (name) {
			if (!xml_strcmp(name, obj_get_name(useobj))) {
			    keep = TRUE;
			}
		    } else {
			keep = TRUE;
		    }

		    if (keep) {
			/* replace this node with the useobj */
			findnode = find_resnode(pcb, &resnodeQ, useobj);
			if (findnode) {
			    if (resnode->dblslash) {
				findnode->position =
				    ++walkerparms.callcount;
				findnode->dblslash = TRUE;
			    }
			    free_resnode(pcb, resnode);
			} else {
			    resnode->node.objptr = useobj;
			    resnode->position = 
				++walkerparms.callcount;
			    dlq_enque(resnode, &resnodeQ);
			}
		    } else {
			no_parent_warning(pcb);
			free_resnode(pcb, resnode);
		    }
		}
	    }
	}
    }

    /* put the resnode entries back where they belong */
    if (!dlq_empty(&resnodeQ)) {
	dlq_block_enque(&resnodeQ, &result->r.nodeQ);
    }

    result->last = walkerparms.callcount;

    return res;

}  /* set_nodeset_parent */


/********************************************************************
* FUNCTION set_nodeset_child
* 
* Check the current result nodeset and replace
* each node with a node for every child instead
*
* Handles child and descendant nodes
*
* If a child is specified then any node not
* containing this child will be removed
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*    result == address of return XPath result nodeset
*    childnsid == 0 if any namespace is OK
*                 else only this namespace will be checked
*    childname == name of child to find
*              == NULL to find all child nodes
*                 In this mode, if the context object
*                 is config=true, then config=false
*                 children will be skipped
*    textmode == TRUE if just selecting text() nodes
*                FALSE if ignored
*     axis == actual axis used
*
* OUTPUTS:
*    result->nodeQ contents adjusted or replaced
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    set_nodeset_child (xpath_pcb_t *pcb,
		       xpath_result_t *result,
		       xmlns_id_t  childnsid,
		       const xmlChar *childname,
		       boolean textmode,
		       ncx_xpath_axis_t axis)
{
    xpath_resnode_t      *resnode;
    const obj_template_t *testobj;
    val_value_t          *testval;
    const xmlChar        *modname;
    status_t              res;
    boolean               fnresult, fncalled, cfgonly;
    boolean               orself, myorself, useroot;
    dlq_hdr_t             resnodeQ;
    xpath_walkerparms_t   walkerparms;

    if (!pcb->val && !pcb->obj) {
	return NO_ERR;
    }

    if (dlq_empty(&result->r.nodeQ)) {
	return NO_ERR;
    }

    dlq_createSQue(&resnodeQ);
    res = NO_ERR;
    cfgonly = (pcb->flags & XP_FL_CONFIGONLY) ?	TRUE : FALSE;
    orself = (axis == XP_AX_DESCENDANT_OR_SELF) ? TRUE : FALSE;
    useroot = (pcb->flags & XP_FL_USEROOT) ? TRUE : FALSE;

    if (childnsid) {
	modname = xmlns_get_module(childnsid);
    } else {
	modname = NULL;
    }

    walkerparms.resnodeQ = &resnodeQ;
    walkerparms.res = NO_ERR;
    walkerparms.callcount = 0;

    /* the resnodes need to be deleted or moved to a tempQ
     * to correctly track duplicates and remove them
     */
    while (!dlq_empty(&result->r.nodeQ) && res == NO_ERR) {

	resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);
	myorself = (orself || resnode->dblslash) ? TRUE : FALSE;

	/* select 1 or all children of the resnode 
	 * special YANG support; skip over nodes that
	 * are not config nodes if they were selected
	 * by the wildcard '*' operator
	 */
	if (pcb->val) {

	    testval = resnode->node.valptr;

	    if (axis != XP_AX_CHILD || resnode->dblslash) {
		fnresult = 
		    val_find_all_descendants(value_walker_fn,
					     pcb, 
					     &walkerparms,
					     testval, 
					     modname,
					     childname,
					     cfgonly,
					     textmode,
					     myorself);
		if (!fnresult || walkerparms.res != NO_ERR) {
		    res = walkerparms.res;
		}
	    } else {
		fnresult = 
		    val_find_all_children(value_walker_fn,
					  pcb, 
					  &walkerparms,
					  testval, 
					  modname,
					  childname,
					  cfgonly,
					  textmode);
		if (!fnresult || walkerparms.res != NO_ERR) {
		    res = walkerparms.res;
		}
	    }
	} else {
	    testobj = resnode->node.objptr;

	    if (axis != XP_AX_CHILD || resnode->dblslash) {
		fnresult = 
		    obj_find_all_descendants(pcb->objmod,
					     object_walker_fn,
					     pcb, 
					     &walkerparms,
					     testobj,
					     modname,
					     childname,
					     cfgonly,
					     textmode,
					     useroot,
					     myorself,
					     &fncalled);
		if (!fnresult || walkerparms.res != NO_ERR) {
		    res = walkerparms.res;
		}
	    } else {
		fnresult = 
		    obj_find_all_children(pcb->objmod,
					  object_walker_fn,
					  pcb,
					  &walkerparms,
					  testobj,
					  modname,
					  childname,
					  cfgonly,
					  textmode,
					  useroot);
		if (!fnresult || walkerparms.res != NO_ERR) {
		    res = walkerparms.res;
		}
	    }
	}

	free_resnode(pcb, resnode);
    }

    /* put the resnode entries back where they belong */
    if (!dlq_empty(&resnodeQ)) {
	dlq_block_enque(&resnodeQ, &result->r.nodeQ);
    } else if (!pcb->val && pcb->obj) {
	if (pcb->logerrors) {
	    if (axis != XP_AX_CHILD) {
		res = ERR_NCX_NO_XPATH_DESCENDANT;
		log_warn("\nWarning: no descendant nodes found "
			 "in XPath expr '%s'", pcb->exprstr);
	    } else {
		res = ERR_NCX_NO_XPATH_CHILD;
		log_warn("\nWarning: no child nodes found "
			 "in XPath expr '%s'", pcb->exprstr);
	    }
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	    res = NO_ERR;
	}
    }

    result->last = walkerparms.callcount;

    return res;

}  /* set_nodeset_child */


/********************************************************************
* FUNCTION set_nodeset_pfaxis
*
* Handles these axis node tests
*
*   preceding::*
*   preceding-sibling::*
*   following::*
*   following-sibling::*
*
* Combinations with '//' are handled as well
*
* Check the current result nodeset and replace
* each node with all the requested nodes instead,
* based on the axis used
*
* The result set is changed to the selected axis
* Since the current node is no longer going to
* be part of the result set
*
* After this initial step, the desired filtering
* is performed on each of the result nodes (if any)
*
* At this point, if a 'nsid' and/or 'name' is 
* specified then any selected node not
* containing this namespace and/or node name
* will be removed
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*    result == address of return XPath result nodeset
*    nsid == 0 if any namespace is OK
*            else only this namespace will be checked
*    name == name of preceding node to find
*            == NULL to find all child nodes
*               In this mode, if the context object
*               is config=true, then config=false
*               preceding nodes will be skipped
*    textmode == TRUE if just selecting text() nodes
*                FALSE if ignored
*    axis == axis in use for this current step
*
* OUTPUTS:
*    result->nodeQ contents adjusted or replaced
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    set_nodeset_pfaxis (xpath_pcb_t *pcb,
			xpath_result_t *result,
			xmlns_id_t  nsid,
			const xmlChar *name,
			boolean textmode,
			ncx_xpath_axis_t axis)
{
    xpath_resnode_t      *resnode;
    const obj_template_t *testobj;
    val_value_t          *testval;
    const xmlChar        *modname;
    status_t              res;
    boolean               fnresult, fncalled, cfgonly, useroot;
    dlq_hdr_t             resnodeQ;
    xpath_walkerparms_t   walkerparms;
    
    if (!pcb->val && !pcb->obj) {
	return NO_ERR;
    }

    if (dlq_empty(&result->r.nodeQ)) {
	return NO_ERR;
    }

    dlq_createSQue(&resnodeQ);
    res = NO_ERR;
    cfgonly = (pcb->flags & XP_FL_CONFIGONLY) ?	TRUE : FALSE;

    modname = (nsid) ? xmlns_get_module(nsid) : NULL;
    useroot = (pcb->flags & XP_FL_USEROOT) ? TRUE : FALSE;

    walkerparms.resnodeQ = &resnodeQ;
    walkerparms.res = NO_ERR;
    walkerparms.callcount = 0;

    /* the resnodes need to be deleted or moved to a tempQ
     * to correctly track duplicates and remove them
     */
    while (!dlq_empty(&result->r.nodeQ) && res == NO_ERR) {

	resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);

	/* select 1 or all children of the resnode 
	 * special YANG support; skip over nodes that
	 * are not config nodes if they were selected
	 * by the wildcard '*' operator
	 */
	if (pcb->val) {
	    testval = resnode->node.valptr;

	    if (axis==XP_AX_PRECEDING || axis==XP_AX_FOLLOWING) {
		fnresult = 
		    val_find_all_pfaxis(value_walker_fn,
					pcb, 
					&walkerparms,
					testval, 
					modname,
					name, 
					cfgonly, 
					resnode->dblslash, 
					textmode,
					axis);
	    } else {
		fnresult = 
		    val_find_all_pfsibling_axis(value_walker_fn,
						pcb, 
						&walkerparms,
						testval, 
						modname,
						name, 
						cfgonly, 
						resnode->dblslash, 
						textmode,
						axis);

	    }
	    if (!fnresult || walkerparms.res != NO_ERR) {
		res = walkerparms.res;
	    }
	} else {
	    testobj = resnode->node.objptr;
	    fnresult = obj_find_all_pfaxis(pcb->objmod,
					   object_walker_fn,
					   pcb, 
					   &walkerparms,
					   testobj, 
					   modname,
					   name, 
					   cfgonly,
					   resnode->dblslash,
					   textmode,
					   useroot,
					   axis, 
					   &fncalled);
	    if (!fnresult || walkerparms.res != NO_ERR) {
		res = walkerparms.res;
	    }
	}
	free_resnode(pcb, resnode);
    }

    /* put the resnode entries back where they belong */
    if (!dlq_empty(&resnodeQ)) {
	dlq_block_enque(&resnodeQ, &result->r.nodeQ);
    } else if (!pcb->val && pcb->obj) {
	res = ERR_NCX_NO_XPATH_NODES;
	if (pcb->logerrors) {
	    log_warn("\nWarning: no axis nodes found "
		     "in XPath expr '%s'", 
		     pcb->exprstr);
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	}
	res = NO_ERR;
    }

    result->last = walkerparms.callcount;

    return res;

}  /* set_nodeset_pfaxis */


/********************************************************************
* FUNCTION set_nodeset_ancestor
* 
* Check the current result nodeset and move each
* node to the specified ancestor, or remove it 
* if none found that matches the filter criteria
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    pcb == parser control block in progress
*    result == address of return XPath result nodeset
*    nsid == 0 if any namespace is OK
*                 else only this namespace will be checked
*    name == name of ancestor to find
*              == NULL to find any ancestor nodes
*                 In this mode, if the context object
*                 is config=true, then config=false
*                 children will be skipped
*    textmode == TRUE if just selecting text() nodes
*                FALSE if ignored
*     axis == axis in use for this current step
*             XP_AX_ANCESTOR or XP_AX_ANCESTOR_OR_SELF
*
* OUTPUTS:
*    result->nodeQ contents adjusted or removed
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    set_nodeset_ancestor (xpath_pcb_t *pcb,
			  xpath_result_t *result,
			  xmlns_id_t  nsid,
			  const xmlChar *name,
			  boolean textmode,
			  ncx_xpath_axis_t axis)
{
    xpath_resnode_t        *resnode, *testnode;
    const obj_template_t   *testobj;
    val_value_t            *testval;
    const xmlChar          *modname;
    xpath_result_t         *dummy;
    status_t                res;
    boolean                 cfgonly, fnresult, fncalled, orself, useroot;
    dlq_hdr_t               resnodeQ;
    xpath_walkerparms_t     walkerparms;

    if (!pcb->val && !pcb->obj) {
	return NO_ERR;
    }

    if (dlq_empty(&result->r.nodeQ)) {
	return NO_ERR;
    }

    dlq_createSQue(&resnodeQ);

    res = NO_ERR;

    modname = (nsid) ? xmlns_get_module(nsid) : NULL;

    cfgonly = (pcb->flags & XP_FL_CONFIGONLY) ?	TRUE : FALSE;
    useroot = (pcb->flags & XP_FL_USEROOT) ? TRUE : FALSE;
    orself = (axis == XP_AX_ANCESTOR_OR_SELF) ? TRUE : FALSE;

    walkerparms.resnodeQ = &resnodeQ;
    walkerparms.res = NO_ERR;

    /* the resnodes need to be deleted or moved to a tempQ
     * to correctly track duplicates and remove them
     */
    while (!dlq_empty(&result->r.nodeQ) && res == NO_ERR) {

	resnode = (xpath_resnode_t *)dlq_deque(&result->r.nodeQ);

	walkerparms.callcount = 0;

	if (pcb->val) {
	    testval = resnode->node.valptr;
	    fnresult = val_find_all_ancestors(value_walker_fn,
					      pcb,
					      &walkerparms,
					      testval,
					      modname, 
					      name, 
					      cfgonly,
					      orself,
					      textmode);
	} else {
	    testobj = resnode->node.objptr;
	    fnresult = obj_find_all_ancestors(pcb->objmod,
					      object_walker_fn,
					      pcb,
					      &walkerparms,
					      testobj,
					      modname, 
					      name, 
					      cfgonly,
					      textmode,
					      useroot,
					      orself,
					      &fncalled);

	}
	if (walkerparms.res != NO_ERR) {
	    res = walkerparms.res;
	} else if (!fnresult) {
	    res = ERR_NCX_OPERATION_FAILED;
	} else if (!walkerparms.callcount && resnode->dblslash) {
	    dummy = new_result(pcb, XP_RT_NODESET);
	    if (!dummy) {
		res = ERR_INTERNAL_MEM;
		continue;
	    }

	    walkerparms.resnodeQ = &dummy->r.nodeQ;
	    if (pcb->val) {
		fnresult = val_find_all_descendants(value_walker_fn,
						    pcb,
						    &walkerparms,
						    testval,
						    modname, 
						    name, 
						    cfgonly,
						    TRUE,
						    textmode);
	    } else {
		fnresult = obj_find_all_descendants(pcb->objmod,
						    object_walker_fn,
						    pcb,
						    &walkerparms,
						    testobj,
						    modname, 
						    name, 
						    cfgonly,
						    textmode,
						    useroot,
						    TRUE,
						    &fncalled);
	    }
	    walkerparms.resnodeQ = &resnodeQ;

	    if (walkerparms.res != NO_ERR) {
		res = walkerparms.res;
	    } else if (!fnresult) {
		res = ERR_NCX_OPERATION_FAILED;
	    } else {
		res = set_nodeset_ancestor(pcb, 
					   dummy,
					   nsid, 
					   name, 
					   textmode,
					   axis);
		while (!dlq_empty(&dummy->r.nodeQ)) {
		    testnode = (xpath_resnode_t *)
			dlq_deque(&dummy->r.nodeQ);

		    if (find_resnode(pcb, &resnodeQ,
				     (testnode) ?
				     (const void *)testnode->node.valptr :
				     (const void *)testnode->node.objptr)) {
			free_resnode(pcb, testnode);
		    } else {
			dlq_enque(testnode, &resnodeQ);
		    }
		}
		free_result(pcb, dummy);
	    }
	}

	free_resnode(pcb, resnode);
    }

    /* put the resnode entries back where they belong */
    if (!dlq_empty(&resnodeQ)) {
	dlq_block_enque(&resnodeQ, &result->r.nodeQ);
    } else {
	res = ERR_NCX_NO_XPATH_ANCESTOR;
	if (pcb->logerrors) {
	    if (orself) {
		log_warn("\nWarning: no ancestor-or-self nodes found "
			 "in XPath expr '%s'", pcb->exprstr);
	    } else {
		log_warn("\nWarning: no ancestor nodes found "
			 "in XPath expr '%s'", pcb->exprstr);
	    }
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	}
	res = NO_ERR;
    }

    result->last = walkerparms.callcount;

    return res;

}  /* set_nodeset_ancestor */


/***********   B E G I N    E B N F    F U N C T I O N S *************/


/********************************************************************
* FUNCTION parse_node_test
* 
* Parse the XPath NodeTest sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [7] NodeTest ::= NameTest
*                  | NodeType '(' ')'
*                  | 'processing-instruction' '(' Literal ')'
*
* [37] NameTest ::= '*'
*                  | NCName ':' '*'
*                  | QName
*
* [38] NodeType ::= 'comment'
*                    | 'text'	
*                    | 'processing-instruction'
*                    | 'node'
*
* INPUTS:
*    pcb == parser control block in progress
*    axis  == current axis from first part of Step
*    result == address of pointer to result struct in progress
*
* OUTPUTS:
*   *result is modified
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_node_test (xpath_pcb_t *pcb,
		     ncx_xpath_axis_t axis,
		     xpath_result_t **result)
{
    xpath_result_t    *val1;
    const xmlChar     *literal, *name;
    tk_type_t          nexttyp;
    xpath_nodetype_t   nodetyp;
    status_t           res;
    xmlns_id_t         nsid;
    boolean            emptyresult, textmode;

    val1 = NULL;
    literal = NULL;
    nsid = 0;
    name = NULL;
    textmode = FALSE;

    res = TK_ADV(pcb->tkc);
    if (res != NO_ERR) {
	res = ERR_NCX_INVALID_XPATH_EXPR;
	if (pcb->logerrors) {
	    log_error("\nError: token expected in XPath "
		      "expression '%s'", pcb->exprstr);
	    ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	} else {
	    /*** handle agent error ***/
	}
	return res;
    }

    /* process the tokens but not the result yet */
    switch (TK_CUR_TYP(pcb->tkc)) {
    case TK_TT_STAR:
	break;
    case TK_TT_NCNAME_STAR:
	/* match all nodes in the namespace w/ specified prefix */
	res = check_qname_prefix(pcb, &pcb->tkc->cur->nsid);
	if (res == NO_ERR) {
	    nsid = pcb->tkc->cur->nsid;
	}
	break;
    case TK_TT_MSTRING:
	/* match all nodes in the namespace w/ specified prefix */
	res = check_qname_prefix(pcb, &pcb->tkc->cur->nsid);
	if (res == NO_ERR) {
	    nsid = pcb->tkc->cur->nsid;
	    name = TK_CUR_VAL(pcb->tkc);
	}
	break;
    case TK_TT_TSTRING:
	/* check the ID token for a NodeType name */
	nodetyp = get_nodetype_id(TK_CUR_VAL(pcb->tkc));
	if (nodetyp == XP_EXNT_NONE) {
	    name = TK_CUR_VAL(pcb->tkc);
	    break;
	}

	/* get the node test left paren */
	res = xpath_parse_token(pcb, TK_TT_LPAREN);
	if (res != NO_ERR) {
	    return res;
	}

	/* check if a literal param can be present */
	if (nodetyp == XP_EXNT_PROC_INST) {
	    /* check if a literal param is present */
	    nexttyp = tk_next_typ(pcb->tkc);
	    if (nexttyp==TK_TT_QSTRING ||
		nexttyp==TK_TT_SQSTRING) {
		/* temp save the literal string */
		res = xpath_parse_token(pcb, nexttyp);
		if (res != NO_ERR) {
		    return res;
		}

		literal = TK_CUR_VAL(pcb->tkc);
	    }
	}

	/* get the node test right paren */
	res = xpath_parse_token(pcb, TK_TT_RPAREN);
	if (res != NO_ERR) {
	    return res;
	}

	/* process the result based on the node type test */
	switch (nodetyp) {
	case XP_EXNT_COMMENT:
	    /* no comments to match */
	    emptyresult = TRUE;
	    if (pcb->obj && pcb->logerrors) {
		log_warn("\nWarning: no comment nodes available in "
			 "XPath expr '%s'", pcb->exprstr);
		ncx_print_errormsg(pcb->tkc, pcb->mod,
				   ERR_NCX_EMPTY_XPATH_RESULT);
	    }
	    break;
	case XP_EXNT_TEXT:
	    /* match all leaf of leaf-list content */
	    emptyresult = FALSE;
	    textmode = TRUE;
	    break;
	case XP_EXNT_PROC_INST:
	    /* no processing instructions to match */
	    emptyresult = TRUE;
	    if (pcb->obj && pcb->logerrors) {
		log_warn("\nWarning: no processing instruction "
			 "nodes available in "
			 "XPath expr '%s'", pcb->exprstr);
		ncx_print_errormsg(pcb->tkc, pcb->mod,
				   ERR_NCX_EMPTY_XPATH_RESULT);
	    }
	    break;
	case XP_EXNT_NODE:
	    /* match any node */
	    emptyresult = FALSE;
	    break;
	default:
	    emptyresult = TRUE;
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}

	if (emptyresult) {
	    if (*result) {
		free_result(pcb, *result);
	    }
	    *result = new_result(pcb, XP_RT_NODESET);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	    return res;
	}  /* else go on to the text() or node() test */
	break;
    default:
	/* wrong token type found */
	res = ERR_NCX_WRONG_TKTYPE;
	unexpected_error(pcb);
    }

    /* do not care about result if fatal error occurred */
    if (res != NO_ERR) {
	return res;
    } else if (!pcb->val && !pcb->obj) {
	/* nothing to do in first pass except create
	 * dummy result to flag that a location step
	 * has already started
	 */
	if (!*result) {
	    *result = new_result(pcb, XP_RT_NODESET);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	}
	return res;
    }


    /* if we get here, then the axis and name fields are set
     * or a texttest is needed
     */
    switch (axis) {
    case XP_AX_ANCESTOR:
    case XP_AX_ANCESTOR_OR_SELF:
	if (!*result) {
	    *result = new_nodeset(pcb,
				  pcb->context.node.objptr,
				  pcb->context.node.valptr,
				  1, FALSE);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	}
	if (res == NO_ERR) {
	    res = set_nodeset_ancestor(pcb, 
				       *result,
				       nsid, 
				       name,
				       textmode,
				       axis);
	} 
	break;
    case XP_AX_ATTRIBUTE:
	/* Attribute support in XPath is TBD
	 * YANG does not define them and the ncx:metadata
	 * extension is not fully supported within the 
	 * the edit-config code yet anyway
	 *
	 * just set the result to the empty nodeset
	 */
	if (pcb->obj && pcb->logerrors) {
	    log_warn("\nWarning: attribute axis is empty in "
		     "XPath expr '%s'", pcb->exprstr);
	    ncx_print_errormsg(pcb->tkc, pcb->mod,
			       ERR_NCX_EMPTY_XPATH_RESULT);
	}
	if (*result) {
	    free_result(pcb, *result);
	}
	*result = new_result(pcb, XP_RT_NODESET);
	if (!*result) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case XP_AX_DESCENDANT:
    case XP_AX_DESCENDANT_OR_SELF:
    case XP_AX_CHILD:
	/* select all the child nodes of each node in 
	 * the result node set.
	 * ALSO select all the descendant nodes of each node in 
	 * the result node set. (they are the same in NETCONF)
	 */
	if (!*result) {
	    /* first step is child::* or descendant::*    */
	    *result = new_nodeset(pcb, 
				  pcb->context.node.objptr,
				  pcb->context.node.valptr,
				  1, FALSE);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	}
	if (res == NO_ERR) {
	    res = set_nodeset_child(pcb, 
				    *result, 
				    nsid, 
				    name, 
				    textmode,
				    axis);
	}
	break;
    case XP_AX_FOLLOWING:
    case XP_AX_PRECEDING:
    case XP_AX_FOLLOWING_SIBLING:
    case XP_AX_PRECEDING_SIBLING:
	/* need to set the result to all the objects
	 * or all instances of all value nodes
	 * preceding or following the context node
	 */
	if (!*result) {
	    /* first step is following::* or preceding::* */
	    *result = new_nodeset(pcb, 
				  pcb->context.node.objptr,
				  pcb->context.node.valptr,
				  1, FALSE);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	}

	if (res == NO_ERR) {
	    res = set_nodeset_pfaxis(pcb,
				     *result,
				     nsid, 
				     name,
				     textmode,
				     axis);
	}	 
	break;
    case XP_AX_NAMESPACE:
	/* for NETCONF purposes, there is no need to
	 * provide access to namespace xmlns declarations
	 * within the object or value tree
	 * This can be added later!
	 *
	 *
	 * For now, just turn the result into the empty set
	 */
	if (pcb->obj && pcb->logerrors) {
	    log_warn("Warning: namespace axis is empty in "
		     "XPath expr '%s'", pcb->exprstr);
	    ncx_print_errormsg(pcb->tkc, pcb->mod,
			       ERR_NCX_EMPTY_XPATH_RESULT);
	}
	if (*result) {
	    free_result(pcb, *result);
	}
	*result = new_result(pcb, XP_RT_NODESET);
	if (!*result) {
	    res = ERR_INTERNAL_MEM;
	}
	break;
    case XP_AX_PARENT:
	/* step is parent::*  -- same as .. for nodes  */
	if (textmode) {
	    if (pcb->obj && pcb->logerrors) {
		log_warn("Warning: parent axis contains no text nodes in "
		     "XPath expr '%s'", pcb->exprstr);
		ncx_print_errormsg(pcb->tkc, pcb->mod,
				   ERR_NCX_EMPTY_XPATH_RESULT);
	    }
	    if (*result) {
		free_result(pcb, *result);
	    }
	    *result = new_result(pcb, XP_RT_NODESET);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	    break;
	}
	    
	if (!*result) {
	    *result = new_nodeset(pcb, 
				  pcb->context.node.objptr, 
				  pcb->context.node.valptr,
				  1, 
				  FALSE);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	}

	if (res == NO_ERR) {
	    res = set_nodeset_parent(pcb, 
				     *result, 
				     nsid,
				     name);
	}
	break;
    case XP_AX_SELF:
	/* keep the same context node */
	if (!*result) {
	    /* first step is self::*   */
	    *result = new_nodeset(pcb, 
				  pcb->context.node.objptr,
				  pcb->context.node.valptr,
				  1, FALSE);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	}
	if (res == NO_ERR) {
	    res = set_nodeset_self(pcb,
				   *result,
				   nsid,
				   name,
				   textmode);
	}				   
	break;
    case XP_AX_NONE:
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* parse_node_test */


/********************************************************************
* FUNCTION parse_predicate
* 
* Parse an XPath Predicate sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [8] Predicate ::= '[' PredicateExpr ']'
* [9] PredicateExpr ::= Expr
*
* INPUTS:
*    pcb == parser control block in progress
*    result == address of result in progress to filter
*
* OUTPUTS:
*   *result may be pruned based on filter matches
*            nodes may be updated if descendants are
*            checked and matches are found
*
* RETURNS:
*   malloced result of predicate expression
*********************************************************************/
static status_t
    parse_predicate (xpath_pcb_t *pcb,
		     xpath_result_t **result)
{
    xpath_result_t  *val1, *contextset;
    xpath_resnode_t  lastcontext, *resnode, *nextnode;
    tk_token_t      *leftbrack;
    boolean          bool;
    status_t         res;
    int64            position;

    res = xpath_parse_token(pcb, TK_TT_LBRACK);
    if (res != NO_ERR) {
	return res;
    }

    if (pcb->val) {
	leftbrack = TK_CUR(pcb->tkc);
	contextset = *result;
	if (!contextset) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	} else if (contextset->restype == XP_RT_NODESET &&
		   !dlq_empty(&contextset->r.nodeQ)) {

	    lastcontext.node.valptr = 
		pcb->context.node.valptr;
	    lastcontext.position = pcb->context.position;
	    lastcontext.last = pcb->context.last;
	    lastcontext.dblslash = pcb->context.dblslash;

	    for (resnode = (xpath_resnode_t *)
		     dlq_firstEntry(&(*result)->r.nodeQ);
		 resnode != NULL;
		 resnode = nextnode) {

		/* go over and over the predicate expression
		 * with the resnode as the current context node
		 */
		TK_CUR(pcb->tkc) = leftbrack;
		bool = FALSE;

		nextnode = (xpath_resnode_t *)
		    dlq_nextEntry(resnode);

		pcb->context.node.valptr = resnode->node.valptr;
		pcb->context.position = resnode->position;
		pcb->context.last = contextset->last;
		pcb->context.dblslash = resnode->dblslash;

		val1 = parse_expr(pcb, &res);
		if (res != NO_ERR) {
		    if (val1) {
			free_result(pcb, val1);
		    }
		    return res;
		}
	    
		res = xpath_parse_token(pcb, TK_TT_RBRACK);
		if (res != NO_ERR) {
		    if (val1) {
			free_result(pcb, val1);
		    }
		    return res;
		}

		if (val1->restype == XP_RT_NUMBER) {
		    /* the predicate specifies a context
		     * position and this resnode is
		     * only selected if it is the Nth
		     * instance within the current context
		     */
		    if (ncx_num_is_integral(&val1->r.num,
					    NCX_BT_FLOAT64)) {
			position = 
			    ncx_cvt_to_int64(&val1->r.num,
					     NCX_BT_FLOAT64);
			/* check if the proximity position
			 * of this node matches the position
			 * value from this expression
			 */
			bool = (position == resnode->position) ?
			    TRUE : FALSE;
		    } else {
			bool = FALSE;
		    }
		} else {
		    bool = xpath_cvt_boolean(val1);
		}
		free_result(pcb, val1);

		if (!bool) {
		    /* predicate expression evaluated to false
		     * so delete this resnode from the result
		     */
		    dlq_remove(resnode);
		    free_resnode(pcb, resnode);
		}
	    }

	    pcb->context.node.valptr = 
		lastcontext.node.valptr;
	    pcb->context.position = lastcontext.position;
	    pcb->context.last = lastcontext.last;
	    pcb->context.dblslash = lastcontext.dblslash;

	} else {
	    /* result is from a primary expression and
	     * is not a nodeset.  It will get cleared
	     * if the predicate evaluates to false
	     */
	    val1 = parse_expr(pcb, &res);
	    if (res != NO_ERR) {
		if (val1) {
		    free_result(pcb, val1);
		}
		return res;
	    }
	    
	    res = xpath_parse_token(pcb, TK_TT_RBRACK);
	    if (res != NO_ERR) {
		if (val1) {
		    free_result(pcb, val1);
		}
		return res;
	    }

	    if (val1 && pcb->val) {
		bool = xpath_cvt_boolean(val1);
	    }
	    if (val1) {
		free_result(pcb, val1);
	    }
	    if (pcb->val && !bool && *result) {
		xpath_clean_result(*result);
		xpath_init_result(*result, XP_RT_NONE);
	    }
	}
    } else {
	/* always one pass; do not care about result */
	val1 = parse_expr(pcb, &res);
	if (res == NO_ERR) {
	    res = xpath_parse_token(pcb, TK_TT_RBRACK);
	}
	if (val1) {
	    free_result(pcb, val1);
	}
    }

    return res;

} /* parse_predicate */


/********************************************************************
* FUNCTION parse_step
* 
* Parse the XPath Step sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [4] Step  ::=  AxisSpecifier NodeTest Predicate*        
*                | AbbreviatedStep        
*
* [12] AbbreviatedStep ::= '.'        | '..'
*
* [5] AxisSpecifier ::= AxisName '::'
*                | AbbreviatedAxisSpecifier
*
* [13] AbbreviatedAxisSpecifier ::= '@'?
*
* INPUTS:
*    pcb == parser control block in progress
*    result == address of erturn XPath result nodeset
*

* OUTPUTS:
*   *result pointer is set to malloced result struct
*    if it is NULL, or used if it is non-NULL;
*    MUST be a XP_RT_NODESET result

*
* RETURNS:
*   status
*********************************************************************/
static status_t
    parse_step (xpath_pcb_t *pcb,
		xpath_result_t **result)
{
    const xmlChar    *nextval;
    tk_type_t         nexttyp, nexttyp2;
    ncx_xpath_axis_t  axis;
    status_t          res;

    nexttyp = tk_next_typ(pcb->tkc);
    axis = XP_AX_CHILD;

    /* check start token '/' or '//' */
    if (nexttyp == TK_TT_DBLFSLASH) {
	res = xpath_parse_token(pcb, TK_TT_DBLFSLASH);
	if (res != NO_ERR) {
	    return res;
	}
	if (!*result) {
	    *result = new_nodeset(pcb,
				  pcb->context.node.objptr,
				  pcb->context.node.valptr,
				  1, TRUE);
	    if (!*result) {
		return ERR_INTERNAL_MEM;
	    }
	}
	set_nodeset_dblslash(pcb, *result);
    } else if (nexttyp == TK_TT_FSLASH) {
	res = xpath_parse_token(pcb, TK_TT_FSLASH);
	if (res != NO_ERR) {
	    return res;
	}

	if (!*result) {
	    /* this is the first call */
	    *result = new_nodeset(pcb, 
				  pcb->docroot, 
				  pcb->val_docroot,
				  1,
				  FALSE);
	    if (!*result) {
		return ERR_INTERNAL_MEM;
	    }

	    /* check corner-case path '/' */
	    if (location_path_end(pcb)) {
		/* exprstr is simply docroot '/' */
		return NO_ERR;
	    }
	}
    } else if (*result) {
	/* should not happen */
	SET_ERROR(ERR_INTERNAL_VAL);
	return ERR_NCX_INVALID_XPATH_EXPR;
    }

    /* handle an abbreviated step (. or ..) or
     * handle the axis-specifier for the full form step
     */
    nexttyp = tk_next_typ(pcb->tkc);
    switch (nexttyp) {
    case TK_TT_PERIOD:
	/* abbreviated step '.': 
	 * current target node stays the same unless
	 * this is the first token in the location path,
	 * then the context result needs to be initialized
	 *
	 * first consume the period token
	 */
	res = xpath_parse_token(pcb, TK_TT_PERIOD);
	if (res != NO_ERR) {
	    return res;
	}

	/* first step is simply . */
	if (!*result) {
	    *result = new_nodeset(pcb,
				  pcb->context.node.objptr,
				  pcb->context.node.valptr,
				  1, FALSE);
	    if (!*result) {
		return ERR_INTERNAL_MEM;
	    }
	} /* else leave current result alone */
	return NO_ERR;
    case TK_TT_RANGESEP:
	/* abbrev step '..': 
	 * matches parent of current context */
	res = xpath_parse_token(pcb, TK_TT_RANGESEP);
	if (res != NO_ERR) {
	    return res;
	}

	/* step is .. or  //.. */
	if (!*result) {
	    /* first step is .. or //..     */
	    *result = new_nodeset(pcb, 
				  pcb->context.node.objptr, 
				  pcb->context.node.valptr,
				  1, FALSE);
	    if (!*result) {
		res = ERR_INTERNAL_MEM;
	    }
	}
	if (res == NO_ERR) {
	    res = set_nodeset_parent(pcb, 
				     *result,
				     0,
				     NULL);
	}
	return res;
    case TK_TT_ATSIGN:
	axis = XP_AX_ATTRIBUTE;
	res = xpath_parse_token(pcb, TK_TT_ATSIGN);
	if (res != NO_ERR) {
	    return res;
	}
	break;
    case TK_TT_STAR:
    case TK_TT_NCNAME_STAR:
    case TK_TT_MSTRING:
	/* set the axis to default child, hit node test */
	axis = XP_AX_CHILD;
	break;
    case TK_TT_TSTRING:
	/* check the ID token for an axis name */
	nexttyp2 = tk_next_typ2(pcb->tkc);
	nextval = tk_next_val(pcb->tkc);
	axis = get_axis_id(nextval);
	if (axis != XP_AX_NONE && nexttyp2==TK_TT_DBLCOLON) {
	    /* correct axis-name :: sequence */
	    res = xpath_parse_token(pcb, TK_TT_TSTRING);
	    if (res != NO_ERR) {
		return res;
	    }
	    res = xpath_parse_token(pcb, TK_TT_DBLCOLON);
	    if (res != NO_ERR) {
		return res;
	    }
	} else if (axis == XP_AX_NONE && nexttyp2==TK_TT_DBLCOLON) {
	    /* incorrect axis-name :: sequence */
	    (void)TK_ADV(pcb->tkc);
	    res = ERR_NCX_INVALID_XPATH_EXPR;
	    if (pcb->logerrors) {
		log_error("\nError: invalid axis name '%s' in "
			  "XPath expression '%s'",
			  TK_CUR_VAL(pcb->tkc),
			  pcb->exprstr);
		ncx_print_errormsg(pcb->tkc, pcb->mod, res);
	    } else {
		/*** log agent error ***/
	    }
	    (void)TK_ADV(pcb->tkc);
	    return res;
	} else {
	    axis = XP_AX_CHILD;
	}
	break;
    default:
	/* wrong token type found */
	(void)TK_ADV(pcb->tkc);
	res = ERR_NCX_WRONG_TKTYPE;
	unexpected_error(pcb);
	return res;
    }

    /* axis or default child parsed OK, get node test */
    res = parse_node_test(pcb, axis, result);
    if (res == NO_ERR) {
	nexttyp = tk_next_typ(pcb->tkc);
	while (nexttyp == TK_TT_LBRACK && res==NO_ERR) {
	    res = parse_predicate(pcb, result);
	    nexttyp = tk_next_typ(pcb->tkc);
	}
    }

    return res;

}  /* parse_step */


/********************************************************************
* FUNCTION parse_location_path
* 
* Parse the Location-Path sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [1] LocationPath  ::=  RelativeLocationPath        
*                       | AbsoluteLocationPath        
*
* [2] AbsoluteLocationPath ::= '/' RelativeLocationPath?
*                     | AbbreviatedAbsoluteLocationPath        
*
* [10] AbbreviatedAbsoluteLocationPath ::= '//' RelativeLocationPath
*
* [3] RelativeLocationPath ::= Step
*                       | RelativeLocationPath '/' Step
*                       | AbbreviatedRelativeLocationPath
*
* [11] AbbreviatedRelativeLocationPath ::= 
*               RelativeLocationPath '//' Step
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_location_path (xpath_pcb_t *pcb,
			 status_t *res)
{
    xpath_result_t     *val1;
    tk_type_t           nexttyp;
    boolean             done;

    val1 = NULL;

    done = FALSE;
    while (!done && *res == NO_ERR) {
        *res = parse_step(pcb, &val1);
        if (*res == NO_ERR) {
            nexttyp = tk_next_typ(pcb->tkc);
            if (!(nexttyp == TK_TT_FSLASH ||
		  nexttyp == TK_TT_DBLFSLASH)) {
                done = TRUE;
            }
        }
    }

    return val1;

}  /* parse_location_path */


/********************************************************************
* FUNCTION parse_function_call
* 
* Parse an XPath FunctionCall sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [16] FunctionCall ::= FunctionName 
*                        '(' ( Argument ( ',' Argument )* )? ')'
* [17] Argument ::= Expr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_function_call (xpath_pcb_t *pcb,
			 status_t *res)
{
    xpath_result_t         *val1, *val2;
    const xpath_fncb_t     *fncb;
    dlq_hdr_t               parmQ;
    tk_type_t               nexttyp;
    int32                   parmcnt;
    boolean                 done;

    val1 = NULL;
    parmcnt = 0;
    dlq_createSQue(&parmQ);

    /* get the function name */
    *res = xpath_parse_token(pcb, TK_TT_TSTRING);
    if (*res != NO_ERR) {
	return NULL;
    }

    /* find the function in the library */
    fncb = find_fncb(pcb, TK_CUR_VAL(pcb->tkc));
    if (fncb) {
	/* get the mandatory left paren */
	*res = xpath_parse_token(pcb, TK_TT_LPAREN);
	if (*res != NO_ERR) {
	    return NULL;
	}

	/* get parms until a matching right paren is reached */
	nexttyp = tk_next_typ(pcb->tkc);
	done = (nexttyp == TK_TT_RPAREN) ? TRUE : FALSE;
	while (!done && *res == NO_ERR) {
	    val1 = parse_expr(pcb, res);
	    if (*res == NO_ERR) {
		parmcnt++;
		if (val1) {
		    dlq_enque(val1, &parmQ);
		    val1 = NULL;
		}

		/* check for right paren or else should be comma */
		nexttyp = tk_next_typ(pcb->tkc);
		if (nexttyp == TK_TT_RPAREN) {
		    done = TRUE;
		} else {
		    *res = xpath_parse_token(pcb, TK_TT_COMMA);
		}
	    }
	}

	/* get closing right paren */
	if (*res == NO_ERR) {
	    *res = xpath_parse_token(pcb, TK_TT_RPAREN);
	}

	/* check parameter count */
	if (fncb->parmcnt >= 0 && fncb->parmcnt != parmcnt) {
	    *res = (parmcnt > fncb->parmcnt) ?
		ERR_NCX_EXTRA_PARM : ERR_NCX_MISSING_PARM;

	    if (pcb->logerrors) {	
		log_error("\nError: wrong number of "
			  "parameters got %d, need %d"
			  " for function '%s'",
			  parmcnt, fncb->parmcnt,
			  fncb->name);
		ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
	    } else {
		/*** log agent error ***/
	    }
	} else {
	    /* make the function call */
	    val1 = (*fncb->fn)(pcb, &parmQ, res);
	}
    } else {
	*res = ERR_NCX_UNKNOWN_PARM;

	if (pcb->logerrors) {	
	    log_error("\nError: Invalid XPath function name '%s'",
		      TK_CUR_VAL(pcb->tkc));
	    ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
	} else {
	    /*** log agent error ***/
	}
    }

    /* clean up any function parameters */
    while (!dlq_empty(&parmQ)) {
	val2 = (xpath_result_t *)dlq_deque(&parmQ);
	free_result(pcb, val2);
    }

    return val1;

} /* parse_function_call */


/********************************************************************
* FUNCTION parse_primary_expr
* 
* Parse an XPath PrimaryExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [15] PrimaryExpr ::= VariableReference
*                       | '(' Expr ')'
*                       | Literal
*                       | Number
*                       | FunctionCall
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_primary_expr (xpath_pcb_t *pcb,
			status_t *res)
{
    xpath_result_t         *val1;
    ncx_var_t              *varbind;
    const xmlChar          *errstr;
    tk_type_t               nexttyp;

    val1 = NULL;
    nexttyp = tk_next_typ(pcb->tkc);

    switch (nexttyp) {
    case TK_TT_VARBIND:
    case TK_TT_QVARBIND:
	*res = xpath_parse_token(pcb, nexttyp);

	/* get QName or NCName variable reference */
	if (*res == NO_ERR) {
	    if (TK_CUR_TYP(pcb->tkc) == TK_TT_VARBIND) {
		varbind = get_varbind(pcb, NULL, 0, 
				      TK_CUR_VAL(pcb->tkc), res);
		errstr = TK_CUR_VAL(pcb->tkc);
	    } else {
		varbind = get_varbind(pcb, 
				      TK_CUR_MOD(pcb->tkc),
				      TK_CUR_MODLEN(pcb->tkc), 
				      TK_CUR_VAL(pcb->tkc), res);
		errstr = TK_CUR_MOD(pcb->tkc);
	    }
	    if (!varbind || *res != NO_ERR) {
		if (pcb->logerrors) {
		    if (*res == ERR_NCX_DEF_NOT_FOUND) {
			log_error("\nError: unknown variable binding '%s'",
				  errstr);
			ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
		    } else {
			log_error("\nError: error in variable binding '%s'",
				  errstr);
			ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
		    }
		} else {
		    ;  /**** log agent error ****/
		}
	    } else {
		/* OK: found the variable binding */
		val1 = cvt_from_value(pcb, varbind->val);
		if (!val1) {
		    *res = ERR_INTERNAL_MEM;
		}
	    }
	}
	break;
    case TK_TT_LPAREN:
	/* get ( expr ) */
	*res = xpath_parse_token(pcb, TK_TT_LPAREN);
	if (*res == NO_ERR) {
	    val1 = parse_expr(pcb, res);
	    if (*res == NO_ERR) {
		*res = xpath_parse_token(pcb, TK_TT_RPAREN);
	    }
	}
	break;
    case TK_TT_DNUM:
    case TK_TT_RNUM:
	*res = xpath_parse_token(pcb, nexttyp);

	/* get the Number token */
	if (*res == NO_ERR) {
	    val1 = new_result(pcb, XP_RT_NUMBER);
	    if (!val1) {
		*res = ERR_INTERNAL_MEM;
		return NULL;
	    }

	    *res = ncx_decode_num(TK_CUR_VAL(pcb->tkc),
				  NCX_BT_FLOAT32,
				  &val1->r.num);
	}
	break;
    case TK_TT_QSTRING:             /* double quoted string */
    case TK_TT_SQSTRING:            /* single quoted string */
	/* get the literal token */
	*res = xpath_parse_token(pcb, nexttyp);

	if (*res == NO_ERR) {
	    val1 = new_result(pcb, XP_RT_STRING);
	    if (!val1) {
		*res = ERR_INTERNAL_MEM;
		return NULL;
	    }

	    val1->r.str = xml_strdup(TK_CUR_VAL(pcb->tkc));
	    if (!val1->r.str) {
		*res = ERR_INTERNAL_MEM;
		malloc_failed_error(pcb);
		xpath_free_result(val1);
		val1 = NULL;
	    }
	}
	break;
    case TK_TT_TSTRING:                    /* NCName string */
	/* get the string ID token */
	nexttyp = tk_next_typ2(pcb->tkc);
	if (nexttyp == TK_TT_LPAREN) {
	    val1 = parse_function_call(pcb, res);
	} else {
	    *res = SET_ERROR(ERR_INTERNAL_VAL);
	}
	break;
    case TK_TT_NONE:
	/* unexpected end of token chain */
	(void)TK_ADV(pcb->tkc);
	*res = ERR_NCX_INVALID_XPATH_EXPR;
	if (pcb->logerrors) {
	    log_error("\nError: token expected in XPath expression '%s'",
		      pcb->exprstr);
	    /* hack to get correct error token to print */
	    pcb->tkc->cur = pcb->tk;
	    ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
	    pcb->tkc->cur = NULL;
	} else {
	    ;  /**** log agent error ****/
	}
	break;
    default:
	/* unexpected token error */
	(void)TK_ADV(pcb->tkc);
	*res = ERR_NCX_WRONG_TKTYPE;
	unexpected_error(pcb);
    }

    return val1;

} /* parse_primary_expr */


/********************************************************************
* FUNCTION parse_filter_expr
* 
* Parse an XPath FilterExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [20] FilterExpr ::= PrimaryExpr	
*                     | FilterExpr Predicate	
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_filter_expr (xpath_pcb_t *pcb,
		       status_t *res)
{
    xpath_result_t  *val1, *dummy;
    tk_type_t        nexttyp;

    val1 = parse_primary_expr(pcb, res);
    if (*res != NO_ERR) {
	return val1;
    }

    /* peek ahead to check the possible next chars */
    nexttyp = tk_next_typ(pcb->tkc);

    if (val1) {
	while (nexttyp == TK_TT_LBRACK && *res==NO_ERR) {
	    *res = parse_predicate(pcb, &val1);
	    nexttyp = tk_next_typ(pcb->tkc);
	}
    } else if (nexttyp==TK_TT_LBRACK) {
	dummy = new_result(pcb, XP_RT_NODESET);
	if (dummy) {
	    while (nexttyp == TK_TT_LBRACK && *res==NO_ERR) {
		*res = parse_predicate(pcb, &dummy);
		nexttyp = tk_next_typ(pcb->tkc);
	    }
	    free_result(pcb, dummy);
	} else {
	    *res = ERR_INTERNAL_MEM;
	    return NULL;
	}
    }

    return val1;

} /* parse_filter_expr */


/********************************************************************
* FUNCTION parse_path_expr
* 
* Parse an XPath PathExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [19] PathExpr ::= LocationPath
*                   | FilterExpr
*                   | FilterExpr '/' RelativeLocationPath
*                   | FilterExpr '//' RelativeLocationPath
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_path_expr (xpath_pcb_t *pcb,
		     status_t *res)
{
    xpath_result_t  *val1, *val2;
    const xmlChar   *nextval;
    tk_type_t        nexttyp, nexttyp2;
    xpath_exop_t     curop;

    val1 = NULL;
    val2 = NULL;

    /* peek ahead to check the possible next sequence */
    nexttyp = tk_next_typ(pcb->tkc);
    switch (nexttyp) {
    case TK_TT_FSLASH:                /* abs location path */
    case TK_TT_DBLFSLASH:     /* abbrev. abs location path */
    case TK_TT_PERIOD:                      /* abbrev step */
    case TK_TT_RANGESEP:                    /* abbrev step */
    case TK_TT_ATSIGN:                 /* abbrev axis name */
    case TK_TT_STAR:              /* rel, step, node, name */
    case TK_TT_NCNAME_STAR:       /* rel, step, node, name */
    case TK_TT_MSTRING:          /* rel, step, node, QName */
	return parse_location_path(pcb, res);
    case TK_TT_TSTRING:
	/* some sort of identifier string to check
	 * get the value of the string and the following token type
	 */
	nexttyp2 = tk_next_typ2(pcb->tkc);
	nextval = tk_next_val(pcb->tkc);

	/* check 'axis-name ::' sequence */
	if (nexttyp2==TK_TT_DBLCOLON && get_axis_id(nextval)) {
	    /* this is an axis name */
	    return parse_location_path(pcb, res);
	}		

	/* check 'NodeType (' sequence */
	if (nexttyp2==TK_TT_LPAREN && get_nodetype_id(nextval)) {
	    /* this is an nodetype name */
	    return parse_location_path(pcb, res);
	}

	/* check not a function call, so must be a QName */
	if (nexttyp2 != TK_TT_LPAREN) {
	    /* this is an NameTest QName w/o a prefix */
	    return parse_location_path(pcb, res);
	}
	break;
    default:
	;
    }

    /* if we get here, then a filter expression is expected */
    val1 = parse_filter_expr(pcb, res);

    if (*res == NO_ERR) {
	nexttyp = tk_next_typ(pcb->tkc);
	switch (nexttyp) {
	case TK_TT_FSLASH:
	    curop = XP_EXOP_FILTER1;
	    break;
	case TK_TT_DBLFSLASH:
	    curop = XP_EXOP_FILTER2;
	    break;
	default:
	    curop = XP_EXOP_NONE;
	}


	/*** !!! probably all wrong !!! ****/
	if (curop != XP_EXOP_NONE) {
	    val2 = parse_location_path(pcb, res);
	    if (*res == NO_ERR) {
		/*   *res = eval_xpath_op(val1, val2, curop);  */
            }
        }
    }

    if (val1) {
        free_result(pcb, val1);
    }

    return val2;

} /* parse_path_expr */


/********************************************************************
* FUNCTION parse_union_expr
* 
* Parse an XPath UnionExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [18] UnionExpr ::= PathExpr
*                    | UnionExpr '|' PathExpr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_union_expr (xpath_pcb_t *pcb,
		      status_t *res)
{
    xpath_result_t  *val1, *val2;
    boolean          done;
    tk_type_t        nexttyp;

    val1 = NULL;
    val2 = NULL;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_path_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
		if (pcb->val || pcb->obj) {
		    /* add all the nodes from val1 into val2
		     * that are not already present
		     */
		    merge_nodeset(pcb, val1, val2);
		    
		    if (val1) {
			free_result(pcb, val1);
			val1 = NULL;
		    }
		}
	    } else {
		val2 = val1;
		val1 = NULL;
	    }

	    if (*res != NO_ERR) {
		continue;
	    }

	    nexttyp = tk_next_typ(pcb->tkc);
	    if (nexttyp != TK_TT_BAR) {
		done = TRUE;
	    } else {
                *res = xpath_parse_token(pcb, TK_TT_BAR);
            }
        }
    }

    if (val1) {
        free_result(pcb, val1);
    }

    return val2;

} /* parse_union_expr */


/********************************************************************
* FUNCTION parse_unary_expr
* 
* Parse an XPath UnaryExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [27] UnaryExpr ::= UnionExpr	
*                    | '-' UnaryExpr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_unary_expr (xpath_pcb_t *pcb,
		      status_t *res)
{
    xpath_result_t  *val1, *result;
    tk_type_t        nexttyp;
    uint32           minuscnt;

    val1 = NULL;
    minuscnt = 0;

    nexttyp = tk_next_typ(pcb->tkc);
    while (nexttyp == TK_TT_MINUS) {
	*res = xpath_parse_token(pcb, TK_TT_MINUS);
	if (*res != NO_ERR) {
	    return NULL;
	} else {
	    minuscnt++;
	}
    }

    val1 = parse_union_expr(pcb, res);

    if (*res == NO_ERR && minuscnt/2) {
	if (pcb->val || pcb->obj) {
	    /* odd number of negate ops requested */

	    if (val1->restype == XP_RT_BOOLEAN) {
		val1->r.bool = !val1->r.bool;
		return val1;
	    } else {
		result = new_result(pcb, XP_RT_BOOLEAN);
		if (!result) {
		    *res = ERR_INTERNAL_MEM;
		    return NULL;
		}
		result->r.bool = xpath_cvt_boolean(val1);
		free_result(pcb, val1);
		return result;
	    }
	}
    }

    return val1;

} /* parse_unary_expr */


/********************************************************************
* FUNCTION parse_multiplicative_expr
* 
* Parse an XPath MultiplicativeExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [26] MultiplicativeExpr ::= UnaryExpr	
*              | MultiplicativeExpr MultiplyOperator UnaryExpr
*              | MultiplicativeExpr 'div' UnaryExpr
*              | MultiplicativeExpr 'mod' UnaryExpr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_multiplicative_expr (xpath_pcb_t *pcb,
			       status_t *res)
{
    xpath_result_t  *val1, *val2, *result;
    ncx_num_t        num1, num2;
    xpath_exop_t     curop;
    boolean          done;
    tk_type_t        nexttyp;

    val1 = NULL;
    val2 = NULL;
    result = NULL;
    curop = XP_EXOP_NONE;
    done = FALSE;

    ncx_init_num(&num1);
    ncx_init_num(&num2);

    while (!done && *res == NO_ERR) {
        val1 = parse_unary_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
		if (pcb->val || pcb->obj) {
		    /* val2 holds the 1st operand
		     * val1 holds the 2nd operand
		     */
		    if (val1->restype != XP_RT_NUMBER) {
			xpath_cvt_number(val1, &num1);
		    } else {
			*res = ncx_copy_num(&val1->r.num,
					    &num1, 
					    NCX_BT_FLOAT64);
		    }

		    if (val2->restype != XP_RT_NUMBER) {
			xpath_cvt_number(val2, &num2);
		    } else {
			*res = ncx_copy_num(&val2->r.num,
					    &num2, 
					    NCX_BT_FLOAT64);
		    }

		    if (*res == NO_ERR) {
			result = new_result(pcb, XP_RT_NUMBER);
			if (!result) {
			    *res = ERR_INTERNAL_MEM;
			} else {
			    switch (curop) {
			    case XP_EXOP_MULTIPLY:
				result->r.num.d = num2.d * num1.d;
				break;
			    case XP_EXOP_DIV:
				if (ncx_num_zero(&num2, 
						 NCX_BT_FLOAT64)) {
				    ncx_set_num_max(&result->r.num,
						    NCX_BT_FLOAT64);
				} else {
				    result->r.num.d = num2.d / num1.d;
				}
				break;
			    case XP_EXOP_MOD:
				result->r.num.d = num2.d / num1.d;
#ifdef HAS_FLOAT
				result->r.num.d = trunc(result->r.num.d);
#endif
				break;
			    default:
				*res = SET_ERROR(ERR_INTERNAL_VAL);
			    }
			}
		    }
		}

		if (val1) {
		    free_result(pcb, val1);
		    val1 = NULL;
		}
		if (val2) {
		    free_result(pcb, val2);
		    val2 = NULL;
		}

		if (result) {
		    val2 = result;
		    result = NULL;
		}
            } else {
                val2 = val1;
		val1 = NULL;
            }

	    if (*res != NO_ERR) {
		continue;
	    }

	    nexttyp = tk_next_typ(pcb->tkc);
	    switch (nexttyp) {
	    case TK_TT_STAR:
		curop = XP_EXOP_MULTIPLY;
		break;
	    case TK_TT_TSTRING:
		if (match_next_token(pcb, TK_TT_TSTRING,
				     XP_OP_DIV)) {
		    curop = XP_EXOP_DIV;
		} else if (match_next_token(pcb, 
					    TK_TT_TSTRING,
					    XP_OP_MOD)) {
		    curop = XP_EXOP_MOD;
		} else {
		    done = TRUE;
		}
		break;
	    default:
		done = TRUE;
	    }
	    if (!done) {
                *res = xpath_parse_token(pcb, nexttyp);
            }
        }
    }

    if (val1) {
        free_result(pcb, val1);
    }

    ncx_clean_num(NCX_BT_FLOAT64, &num1);
    ncx_clean_num(NCX_BT_FLOAT64, &num2);

    return val2;

} /* parse_multiplicative_expr */


/********************************************************************
* FUNCTION parse_additive_expr
* 
* Parse an XPath AdditiveExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [25] AdditiveExpr ::= MultiplicativeExpr
*                | AdditiveExpr '+' MultiplicativeExpr
*                | AdditiveExpr '-' MultiplicativeExpr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_additive_expr (xpath_pcb_t *pcb,
			   status_t *res)
{
    xpath_result_t  *val1, *val2, *result;
    ncx_num_t        num1, num2;
    xpath_exop_t     curop;
    boolean          done;
    tk_type_t        nexttyp;

    val1 = NULL;
    val2 = NULL;
    result = NULL;
    curop = XP_EXOP_NONE;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_multiplicative_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
		/* val2 holds the 1st operand
		 * val1 holds the 2nd operand
		 */

		if (pcb->val || pcb->val) {
		    if (val1->restype != XP_RT_NUMBER) {
			xpath_cvt_number(val1, &num1);
		    } else {
			*res = ncx_copy_num(&val1->r.num,
					    &num1, 
					    NCX_BT_FLOAT64);
		    }

		    if (val2->restype != XP_RT_NUMBER) {
			xpath_cvt_number(val2, &num2);
		    } else {
			*res = ncx_copy_num(&val2->r.num,
					    &num2, 
					    NCX_BT_FLOAT64);
		    }

		    if (*res == NO_ERR) {
			result = new_result(pcb, XP_RT_NUMBER);
			if (!result) {
			    *res = ERR_INTERNAL_MEM;
			} else {
			    switch (curop) {
			    case XP_EXOP_ADD:
				result->r.num.d = num2.d + num1.d;
				break;
			    case XP_EXOP_SUBTRACT:
				result->r.num.d = num2.d - num1.d;
				break;
			    default:
				*res = SET_ERROR(ERR_INTERNAL_VAL);
			    }
			}
		    }
		}

		if (val1) {
		    free_result(pcb, val1);
		    val1 = NULL;
		}
		if (val2) {
		    free_result(pcb, val2);
		    val2 = NULL;
		}

		if (result) {
		    val2 = result;
		    result = NULL;
		}
            } else {
                val2 = val1;
		val1 = NULL;
            }

	    if (*res != NO_ERR) {
		continue;
	    }

	    nexttyp = tk_next_typ(pcb->tkc);
	    switch (nexttyp) {
	    case TK_TT_PLUS:
		curop = XP_EXOP_ADD;
		break;
	    case TK_TT_MINUS:
		curop = XP_EXOP_SUBTRACT;
		break;
	    default:
		done = TRUE;
	    }
	    if (!done) {
                *res = xpath_parse_token(pcb, nexttyp);
            }
        }
    }

    if (val1) {
        free_result(pcb, val1);
    }

    ncx_clean_num(NCX_BT_FLOAT64, &num1);
    ncx_clean_num(NCX_BT_FLOAT64, &num2);

    return val2;

} /* parse_additive_expr */


/********************************************************************
* FUNCTION parse_relational_expr
* 
* Parse an XPath RelationalExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [24]  RelationalExpr ::= AdditiveExpr
*             | RelationalExpr '<' AdditiveExpr
*             | RelationalExpr '>' AdditiveExpr
*             | RelationalExpr '<=' AdditiveExpr
*             | RelationalExpr '>=' AdditiveExpr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_relational_expr (xpath_pcb_t *pcb,
			   status_t *res)
{
    xpath_result_t  *val1, *val2, *result;
    xpath_exop_t     curop;
    boolean          done;
    tk_type_t        nexttyp;
    int32            cmpresult;

    val1 = NULL;
    val2 = NULL;
    result = NULL;
    curop = XP_EXOP_NONE;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_additive_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
		if (pcb->val || pcb->obj) {
		    /* val2 holds the 1st operand
		     * val1 holds the 2nd operand
		     */
		    cmpresult = compare_results(pcb, val2, val1, res);

		    if (*res == NO_ERR) {
			result = new_result(pcb, XP_RT_BOOLEAN);
			if (!result) {
			    *res = ERR_INTERNAL_MEM;
			} else {
			    switch (curop) {
			    case XP_EXOP_LT:
				result->r.bool = (cmpresult < 0)
				    ? TRUE : FALSE;
				break;
			    case XP_EXOP_GT:
				result->r.bool = (cmpresult > 0)
				    ? TRUE : FALSE;
				break;
			    case XP_EXOP_LEQUAL:
				result->r.bool = (cmpresult <= 0)
				    ? TRUE : FALSE;
				break;
			    case XP_EXOP_GEQUAL:
				result->r.bool = (cmpresult >= 0)
				    ? TRUE : FALSE;
				break;
			    default:
				*res = SET_ERROR(ERR_INTERNAL_VAL);
			    }
			}
		    }
		}

		if (val1) {
		    free_result(pcb, val1);
		    val1 = NULL;
		}

		if (val2) {
		    free_result(pcb, val2);
		    val2 = NULL;
		}

		if (result) {
		    val2 = result;
		    result = NULL;
		}
            } else {
                val2 = val1;
		val1 = NULL;
            }

	    if (*res != NO_ERR) {
		continue;
	    }

	    nexttyp = tk_next_typ(pcb->tkc);
	    switch (nexttyp) {
	    case TK_TT_LT:
		curop = XP_EXOP_LT;
		break;
	    case TK_TT_GT:
		curop = XP_EXOP_GT;
		break;
	    case TK_TT_LEQUAL:
		curop = XP_EXOP_LEQUAL;
		break;
	    case TK_TT_GEQUAL:
		curop = XP_EXOP_GEQUAL;
		break;
	    default:
		done = TRUE;
	    }
	    if (!done) {
                *res = xpath_parse_token(pcb, nexttyp);
            }
        }
    }

    if (val1) {
        free_result(pcb, val1);
    }

    return val2;

}  /* parse_relational_expr */


/********************************************************************
* FUNCTION parse_equality_expr
* 
* Parse an XPath EqualityExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [23] EqualityExpr  ::=   RelationalExpr        
*                | EqualityExpr '=' RelationalExpr        
*                | EqualityExpr '!=' RelationalExpr        
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_equality_expr (xpath_pcb_t *pcb,
			 status_t *res)
{
    xpath_result_t  *val1, *val2, *result;
    xpath_exop_t     curop;
    boolean          done;
    int32            cmpresult;

    val1 = NULL;
    val2 = NULL;
    result = NULL;
    curop = XP_EXOP_NONE;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_relational_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
		if (pcb->val || pcb->obj) {
		    /* val2 holds the 1st operand
		     * val1 holds the 2nd operand
		     */
		    cmpresult = compare_results(pcb, val2, val1, res);

		    if (*res == NO_ERR) {
			result = new_result(pcb, XP_RT_BOOLEAN);
			if (!result) {
			    *res = ERR_INTERNAL_MEM;
			} else {
			    switch (curop) {
			    case XP_EXOP_EQUAL:
				result->r.bool = (cmpresult == 0) 
				    ? TRUE : FALSE;
				break;
			    case XP_EXOP_NOTEQUAL:
				result->r.bool = (cmpresult != 0)
				    ? TRUE : FALSE;
				break;
			    default:
				*res = SET_ERROR(ERR_INTERNAL_VAL);
			    }
			}
		    }
		}

		if (val1) {
		    free_result(pcb, val1);
		    val1 = NULL;
		}

		if (val2) {
		    free_result(pcb, val2);
		    val2 = NULL;
		}

		if (result) {
		    val2 = result;
		    result = NULL;
		}
            } else {
                val2 = val1;
		val1 = NULL;
            }

	    if (*res != NO_ERR) {
		continue;
	    }

            if (match_next_token(pcb, TK_TT_EQUAL, NULL)) {
                *res = xpath_parse_token(pcb, TK_TT_EQUAL);
		curop = XP_EXOP_EQUAL;
	    } else if (match_next_token(pcb, TK_TT_NOTEQUAL, NULL)) {
                *res = xpath_parse_token(pcb, TK_TT_NOTEQUAL);
		curop = XP_EXOP_NOTEQUAL;
            } else {
                done = TRUE;
            }
        }
    }

    if (val1) {
        free_result(pcb, val1);
    }

    return val2;

}  /* parse_equality_expr */


/********************************************************************
* FUNCTION parse_and_expr
* 
* Parse an XPath AndExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [22] AndExpr  ::= EqualityExpr
*                   | AndExpr 'and' EqualityExpr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_and_expr (xpath_pcb_t *pcb,
                    status_t *res)
{
    xpath_result_t  *val1, *val2, *result;
    boolean          done, bool1, bool2;

    val1 = NULL;
    val2 = NULL;
    result = NULL;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_equality_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
		if (pcb->val || pcb->obj) {
		    /* val2 holds the 1st operand
		     * val1 holds the 2nd operand
		     */
		    bool1 = xpath_cvt_boolean(val1);
		    bool2 = xpath_cvt_boolean(val2);

		    result = new_result(pcb, XP_RT_BOOLEAN);
		    if (!result) {
			*res = ERR_INTERNAL_MEM;
		    } else {
			result->r.bool = (bool1 && bool2) 
			    ? TRUE : FALSE;
		    }
		}

		if (val1) {
		    free_result(pcb, val1);
		    val1 = NULL;
		}

		if (val2) {
		    free_result(pcb, val2);
		    val2 = NULL;
		}

		if (result) {
		    val2 = result;
		    result = NULL;
		}
            } else {
                val2 = val1;
		val1 = NULL;
            }

	    if (*res != NO_ERR) {
		continue;
	    }

            if (match_next_token(pcb, TK_TT_TSTRING, XP_OP_AND)) {
                *res = xpath_parse_token(pcb, TK_TT_TSTRING);
            } else {
                done = TRUE;
            }
        }
    }

    if (val1) {
        free_result(pcb, val1);
    }

    return val2;

}  /* parse_and_expr */


/********************************************************************
* FUNCTION parse_or_expr
* 
* Parse an XPath OrExpr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [21] OrExpr ::= AndExpr        
*                 | OrExpr 'or' AndExpr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_or_expr (xpath_pcb_t *pcb,
                   status_t *res)
{
    xpath_result_t  *val1, *val2, *result;
    boolean          done, bool1, bool2;

    val1 = NULL;
    val2 = NULL;
    result = NULL;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_and_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
		if (pcb->val || pcb->obj) {
		    /* val2 holds the 1st operand
		     * val1 holds the 2nd operand
		     */
		    bool1 = xpath_cvt_boolean(val1);
		    bool2 = xpath_cvt_boolean(val2);

		    result = new_result(pcb, XP_RT_BOOLEAN);
		    if (!result) {
			*res = ERR_INTERNAL_MEM;
		    } else {
			result->r.bool = (bool1 || bool2) 
			    ? TRUE : FALSE;
		    }
		}
		    
		if (val1) {
		    free_result(pcb, val1);
		    val1 = NULL;
		}

		if (val2) {
		    free_result(pcb, val2);
		    val2 = NULL;
		}

		if (result) {
		    val2 = result;
		    result = NULL;
		}
            } else {
                val2 = val1;
		val1 = NULL;
            }

	    if (*res != NO_ERR) {
		continue;
	    }

            if (match_next_token(pcb, TK_TT_TSTRING, XP_OP_OR)) {
                *res = xpath_parse_token(pcb, TK_TT_TSTRING);
            } else {
                done = TRUE;
            }
        }
    }

    if (val1) {
        free_result(pcb, val1);
    }

    return val2;

}  /* parse_or_expr */


/********************************************************************
* FUNCTION parse_expr
* 
* Parse an XPath Expr sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [14] Expr ::= OrExpr
*
* INPUTS:
*    pcb == parser control block in progress
*    res == address of result status
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to malloced result struct or NULL if no
*   result processing in effect 
*********************************************************************/
static xpath_result_t *
    parse_expr (xpath_pcb_t *pcb,
                status_t  *res)
{

    return parse_or_expr(pcb, res);

}  /* parse_expr */


/************    E X T E R N A L   F U N C T I O N S    ************/


/********************************************************************
* FUNCTION xpath1_parse_expr
* 
* Parse the XPATH 1.0 expression string.
*
* This is just a first pass done when the
* XPath string is consumed.  If this is a
* YANG file source then the prefixes will be
* checked against the 'mos' import Q
*
* The expression is parsed into XPath tokens
* and checked for well-formed syntax and function
* invocations. Any variable 
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
*
* INPUTS:
*    tkc == parent token chain
*    mod == module in progress
*    pcb == initialized xpath parser control block
*           for the expression; use xpath_new_pcb
*           to initialize before calling this fn
*           The pcb->exprstr MUST BE SET BEFORE THIS CALL
*    source == enum indicating source of this expression
*
* OUTPUTS:
*   pcb->tkc is filled and then partially validated
*   pcb->parseres is set
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath1_parse_expr (tk_chain_t *tkc,
                       ncx_module_t *mod,
                       xpath_pcb_t *pcb,
                       xpath_source_t source)
{
    xpath_result_t  *result;
    status_t         res;

#ifdef DEBUG
    if (!tkc || !mod || !pcb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* before all objects are known, only simple validation
     * is done, and the token chain is saved for reuse
     * each time the expression is evaluated
     */
    if (pcb->tkc) {
	tk_free_chain(pcb->tkc);
	pcb->tkc = NULL;
    }

    pcb->tkc = tk_tokenize_xpath_string(mod, pcb->exprstr, 
					TK_CUR_LNUM(tkc),
					TK_CUR_LPOS(tkc),
					&res);
    if (!pcb->tkc || res != NO_ERR) {
        log_error("\nError: Invalid XPath string '%s'",
                  pcb->exprstr);
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    /* the module that contains the XPath expr is the one
     * that will always be used to resolve prefixes
     */
    pcb->mod = mod;
    pcb->source = source;
    pcb->logerrors = TRUE;
    pcb->obj = NULL;
    pcb->objmod = NULL;
    pcb->docroot = NULL;
    pcb->doctype = XP_DOC_NONE;
    pcb->val = NULL;
    pcb->val_docroot = NULL;
    pcb->context.node.objptr = NULL;
    pcb->parseres = NO_ERR;

    /* since the pcb->obj is not set, this validation
     * phase will skip identifier tests, predicate tests
     * and completeness tests
     */
    result = parse_expr(pcb, &pcb->parseres);
    if (result) {
	free_result(pcb, result);
    }

    if (pcb->parseres == NO_ERR && pcb->tkc->cur) {
	res = TK_ADV(pcb->tkc);
	if (res == NO_ERR) {
	    if (pcb->logerrors) {
		pcb->parseres = ERR_NCX_INVALID_XPATH_EXPR;
		log_error("\nError: extra tokens in XPath expression '%s'",
			  pcb->exprstr);
		ncx_print_errormsg(pcb->tkc, pcb->mod, pcb->parseres);
	    } else {
		/*** log agent error ***/
	    }
	}
    }

    if (LOGDEBUG3 && pcb->tkc) {
	log_debug3("\n\nParse chain for XPath '%s':\n",
		   pcb->exprstr);
	tk_dump_chain(pcb->tkc);
	log_debug3("\n");
    }

    /* the expression will not be processed further if the
     * parseres is other than NO_ERR
     */
    return pcb->parseres;

}  /* xpath1_parse_expr */


/********************************************************************
* FUNCTION xpath1_validate_expr
* 
* Validate the previously parsed expression string
*   - QName prefixes are valid
*   - function calls are well-formed and exist in
*     the pcb->functions array
*   - variable references exist in the pcb->varbindQ
*
* Called after all 'uses' and 'augment' expansion
* so validation against cooked object tree can be done
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    mod == module containing the 'obj' (in progress)
*    obj == object containing the XPath clause
*    pcb == the XPath parser control block to process
*
* OUTPUTS:
*   pcb->obj and pcb->objmod are set
*   pcb->validateres is set
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath1_validate_expr (ncx_module_t *mod,
                          const obj_template_t *obj,
                          xpath_pcb_t *pcb)
{
    xpath_result_t       *result;
    const obj_template_t *rootobj;
    boolean               rootdone;
 
#ifdef DEBUG
    if (!mod || !obj || !pcb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!pcb->tkc) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    pcb->objmod = mod;
    pcb->obj = obj;
    pcb->logerrors = TRUE;
    pcb->val = NULL;
    pcb->val_docroot = NULL;

    if (pcb->source == XP_SRC_YANG && obj_is_config(obj)) {
	pcb->flags |= XP_FL_CONFIGONLY;
    }

    if (pcb->parseres != NO_ERR) {
        /* errors already reported, skip this one */
        return NO_ERR;
    }

    if (pcb->tkc) {
	tk_reset_chain(pcb->tkc);
    } else {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    pcb->context.node.objptr = obj;

    rootdone = FALSE;
    if (obj_is_data_db(obj)) {
	rootdone = TRUE;
	pcb->doctype = XP_DOC_DATABASE;
	pcb->docroot = ncx_get_gen_root();
	if (!pcb->docroot) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
    } else if (obj_in_notif(obj)) {
	pcb->doctype = XP_DOC_NOTIFICATION;
    } else if (obj_in_rpc(obj)) {
	pcb->doctype = XP_DOC_RPC;
    } else if (obj_in_rpc_reply(obj)) {
	pcb->doctype = XP_DOC_RPC_REPLY;
    } else {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (!rootdone) {
	/* get the rpc/input, rpc/output, or /notif node */
	rootobj = obj;
	while (rootobj->parent && !obj_is_toproot(rootobj->parent) &&
	       rootobj->objtype != OBJ_TYP_RPCIO) {
	    rootobj = rootobj->parent;
	}
	pcb->docroot = rootobj;
    }

    /* validate the XPath expression against the 
     * full cooked object tree
     * !!! not much checking done right now !!!
     */
    result = parse_expr(pcb, &pcb->validateres);


    if (result) {
	if (LOGDEBUG2) {
	    dump_result(pcb, result, "validate_expr");
	}

	free_result(pcb, result);
    }

    return pcb->validateres;


}  /* xpath1_validate_expr */


/********************************************************************
* FUNCTION xpath1_eval_expr
* 
* Evaluate the expression and get the expression nodeset result
*
* INPUTS:
*    pcb == XPath parser control block to use
*    val == start context node for value of current()
*    docroot == ptr to cfg->root or top of rpc/rpc-replay/notif tree
*    logerrors == TRUE if log_error and ncx_print_errormsg
*                  should be used to log XPath errors and warnings
*                 FALSE if internal error info should be recorded
*                 in the xpath_result_t struct instead
*    configonly == 
*           XP_SRC_XML:
*                 TRUE if this is a <get-config> call
*                 and all config=false nodes should be skipped
*                 FALSE if <get> call and non-config nodes
*                 will not be skipped
*           XP_SRC_YANG and XP_SRC_KEYREF:
*                 should be set to false
*     res == address of return status
*
* OUTPUTS:
*   *res is set to the return status
*
*
* RETURNS:
*   malloced result struct with expr result
*   NULL if no result produced (see *res for reason)
*********************************************************************/
xpath_result_t *
    xpath1_eval_expr (xpath_pcb_t *pcb,
                      val_value_t *val,
                      val_value_t *docroot,
		      boolean logerrors,
		      boolean configonly,
		      status_t *res)
{
    xpath_result_t *result;

#ifdef DEBUG
    if (!pcb || !val || !docroot || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    if (pcb->parseres != NO_ERR) {
	*res = pcb->parseres;
	return NULL;
    }

    if (pcb->validateres != NO_ERR) {
	*res = pcb->validateres;
	return NULL;
    }

    if (pcb->tkc) {
	tk_reset_chain(pcb->tkc);
    } else {
	*res = SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;

#if 0
	/* agent XML mode */
	pcb->tkc = tk_tokenize_xpath_string(NULL, pcb->exprstr, 
					    0, 0, res);
	if (!pcb->tkc || *res != NO_ERR) {
	    if (logerrors) {
		log_error("\nError: Invalid XPath string '%s'",
			  pcb->exprstr);
		ncx_print_errormsg(tkc, mod, *res);
	    }
	    return NULL;
	}
#endif

    }

    pcb->val = val;
    pcb->val_docroot = docroot;
    pcb->logerrors = logerrors;
    if (val) {
	pcb->context.node.valptr = val;
    } else {
	pcb->context.node.valptr = docroot;
    }

    if (configonly ||
	(pcb->source == XP_SRC_YANG && obj_is_config(val->obj))) {
	pcb->flags |= XP_FL_CONFIGONLY;
    }

    pcb->flags |= XP_FL_USEROOT;

    result = parse_expr(pcb, &pcb->valueres);

    if (LOGDEBUG2) {
	dump_result(pcb, result, "eval_expr");
    }

    return result;

}  /* xpath1_eval_expr */


/********************************************************************
* FUNCTION xpath1_get_functions_ptr
* 
* Get the start of the function array for XPath 1.0 plus
* the current() function
*
* RETURNS:
*   pointer to functions array
*********************************************************************/
const xpath_fncb_t *
    xpath1_get_functions_ptr (void)
{
    return &functions[0];

}  /* xpath1_get_functions_ptr */


/* END xpath1.c */
