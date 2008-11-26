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
static xpath_axis_t
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




/**********   X P A T H    F U N C T I O N S   ************/

/* XPath callback functions -- in progress */


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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

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
    return NULL;

}  /* true_fn */


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
* FUNCTION varbind_exists
* 
* Check if the specified variable binding exists
*
* INPUTS:
*    pcb == parser control block in progress
*    prefix == prefix string of module with the varbind
*           == NULL for current module (pcb->mod)
*    name == variable name string
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    varbind_exists (xpath_pcb_t *pcb,
		    const xmlChar *prefix,
		    const xmlChar *name)
{
    ncx_module_t *targmod;
    status_t      res;

    /* check if prefix set and specifies an import */
    if (prefix && *prefix && 
	xml_strcmp(prefix, pcb->mod->prefix)) {

        res = xpath_get_curmod_from_prefix(prefix,
                                           pcb->mod,
                                           &targmod);
        if (res != NO_ERR) {
            log_error("\nError: Module for prefix '%s' not found",
                      prefix);
	    ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
	    return res;
        }

	/**** module variables TBD *******/
	return ERR_NCX_DEF_NOT_FOUND;
    } else {
	/**** module variables TBD *******/
	return ERR_NCX_DEF_NOT_FOUND;
    }
    /*NOTREACHED*/

}  /* varbind_exists */


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
* FUNCTION eval_xpath_op
* 
* Evaluate the XPath operation on the 1 or 2 operands
* Store the result in the first operand
*
* INPUTS:
*    op1 == operand 1
*    op2 == operand 2
*    exop == expression operation enum
*
* OUTPUTS:
*   op1 is updated to contain the result of the operation
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    eval_xpath_op (xpath_result_t *op1,
                   xpath_result_t *op2,
                   xpath_exop_t    exop)
{

    /*****/
    return NO_ERR;

}  /* eval_xpath_op */


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
    parse_node_test (xpath_pcb_t *pcb,
		     status_t  *res)
{
    xpath_result_t    *val1;
    const xmlChar     *literal;
    tk_type_t          nexttyp;
    xpath_nodetype_t   nodetyp;

    val1 = NULL;
    literal = NULL;

    *res = TK_ADV(pcb->tkc);
    if (*res != NO_ERR) {
	log_error("\nError: Invalid XPath expression");
	ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
	return NULL;
    }


    switch (TK_CUR_TYP(pcb->tkc)) {
    case TK_TT_STAR:
	/* match all element children of the context node */
	break;
    case TK_TT_NCNAME_STAR:
	/* match all nodes in the namespace w/ specified prefix */
	break;
    case TK_TT_MSTRING:
	/* match all element children with same QName */
	break;
    case TK_TT_TSTRING:
	/* check the ID token for a NodeType name */
	nodetyp = get_nodetype_id(TK_CUR_VAL(pcb->tkc));

	if (nodetyp == XP_EXNT_NONE) {
	    /* match all element children with same NCName */

	} else {
	    /* get the node test left paren */
	    *res = xpath_parse_token(pcb, TK_TT_LPAREN);
	    if (*res != NO_ERR) {
		return NULL;
	    }

	    /* check if a literal param can be present */
	    if (nodetyp == XP_EXNT_PROC_INST) {
		/* check if a literal param is present */
		nexttyp = tk_next_typ(pcb->tkc);
		if (nexttyp==TK_TT_QSTRING ||
		    nexttyp==TK_TT_SQSTRING) {
		    /* temp save the literal string */
		    *res = xpath_parse_token(pcb, nexttyp);
		    if (*res != NO_ERR) {
			return NULL;
		    }

		    literal = TK_CUR_VAL(pcb->tkc);
		}
	    }

	    /* get the node test right paren */
	    *res = xpath_parse_token(pcb, TK_TT_RPAREN);
	    if (*res != NO_ERR) {
		return NULL;
	    }

	    /* process the result based on the node type test */
	    switch (nodetyp) {
	    case XP_EXNT_COMMENT:
		/* no comments to match */
		break;
	    case XP_EXNT_TEXT:
		/* match leaf content */
		break;
	    case XP_EXNT_PROC_INST:
		/* no processing instructions to match */
		break;
	    case XP_EXNT_NODE:
		/* match any database node */
		break;
	    default:
		*res = SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
	break;
    default:
	/* wrong token type found */
	*res = ERR_NCX_WRONG_TKTYPE;
	log_error("\nError: Unexpected token in "
		  "XPath expression '%s'",
		  tk_get_token_name(TK_CUR_TYP(pcb->tkc)));
	ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
	return NULL;
    }

    return val1;

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
    parse_predicate (xpath_pcb_t *pcb,
		     status_t *res)
{
    xpath_result_t  *val1;

    *res = xpath_parse_token(pcb, TK_TT_LBRACK);

    val1 = parse_expr(pcb, res);

    if (*res == NO_ERR) {
	*res = xpath_parse_token(pcb, TK_TT_RBRACK);
    }

    if (*res == NO_ERR) {
	/****/
	/* prune pcb->curfilter based on expr value */
	;
    }

    return val1;

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
    parse_step (xpath_pcb_t *pcb,
		status_t *res)
{
    xpath_result_t  *val1, *val2, *save;
    const xmlChar  *nextval;
    tk_type_t       nexttyp, nexttyp2;
    xpath_axis_t    saveaxis, axis;

    val1 = NULL;
    val2 = NULL;
    nexttyp = tk_next_typ(pcb->tkc);
    saveaxis = pcb->curaxis;

    switch (nexttyp) {
    case TK_TT_PERIOD:
	/* abbrev step '.': current target node stays the same */
	*res = xpath_parse_token(pcb, TK_TT_PERIOD);
	return NULL;  /****/
    case TK_TT_RANGESEP:
	/* abbrev step '..': current target node becomes parent */
	*res = xpath_parse_token(pcb, TK_TT_RANGESEP);
	return NULL;  /****/
    case TK_TT_ATSIGN:
	pcb->curaxis = XP_AX_ATTRIBUTE;
	*res = xpath_parse_token(pcb, TK_TT_ATSIGN);
	if (*res != NO_ERR) {
	    pcb->curaxis = saveaxis;
	    return NULL;
	}
	break;
    case TK_TT_STAR:
    case TK_TT_NCNAME_STAR:
    case TK_TT_MSTRING:
	/* set the axis to default child, hit node test */
	pcb->curaxis = XP_AX_CHILD;
	break;
    case TK_TT_TSTRING:
	/* check the ID token for an axis name */
	nexttyp2 = tk_next_typ2(pcb->tkc);
	nextval = tk_next_val(pcb->tkc);
	axis = get_axis_id(nextval);
	if (axis != XP_AX_NONE && nexttyp2==TK_TT_DBLCOLON) {
	    *res = xpath_parse_token(pcb, TK_TT_TSTRING);
	    if (*res != NO_ERR) {
		return NULL;
	    }
	    *res = xpath_parse_token(pcb, TK_TT_DBLCOLON);
	    if (*res != NO_ERR) {
		return NULL;
	    }
	    pcb->curaxis = axis;
	} else {
	    pcb->curaxis = XP_AX_CHILD;
	}
	break;
    default:
	/* wrong token type found */
	*res = TK_ADV(pcb->tkc);
	if (*res == NO_ERR) {
	    *res = ERR_NCX_WRONG_TKTYPE;
	}
	log_error("\nError: Unexpected token in "
		  "XPath expression '%s'",
		  tk_get_token_name(TK_CUR_TYP(pcb->tkc)));
	ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
	return NULL;
    }

    /* axis or default child parsed OK, get node test */
    val1 = parse_node_test(pcb, res);
    if (*res == NO_ERR) {
	nexttyp = tk_next_typ(pcb->tkc);
	while (nexttyp == TK_TT_LBRACK && *res==NO_ERR) {
	    save = pcb->curfilter;
	    pcb->curfilter = val1;
	    val2 = parse_predicate(pcb, res);
	    pcb->curfilter = save;
	    if (val1) {
		xpath_free_result(val1);
	    }
	    val1 = val2;
	    nexttyp = tk_next_typ(pcb->tkc);
	}
    }


    pcb->curaxis = saveaxis;
    return val1;

}  /* parse_step */


/********************************************************************
* FUNCTION parse_relative_location_path
* 
* Parse the XPath relativeLocationPath sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
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
    parse_relative_location_path (xpath_pcb_t *pcb,
				  status_t *res)
{
    xpath_result_t  *val1;
    tk_type_t        nexttyp;
    boolean          abbrev, done;

    val1 = NULL;
    abbrev = FALSE;
    done = FALSE;
    
    while (!done && *res == NO_ERR) {
        val1 = parse_step(pcb, res);

        if (*res == NO_ERR) {

	    /***** add in step to result *****/
	    /***/

            nexttyp = tk_next_typ(pcb->tkc);
            if (nexttyp == TK_TT_FSLASH) {
                *res = xpath_parse_token(pcb, TK_TT_FSLASH);
            } else if (nexttyp == TK_TT_DBLFSLASH) {
                abbrev = TRUE;
                *res = xpath_parse_token(pcb, TK_TT_DBLFSLASH);
            } else {
                done = TRUE;
            }
        }
    }
            
    return val1;

}  /* parse_relative_location_path */


/********************************************************************
* FUNCTION parse_absolute_location_path
* 
* Parse the AbsoluteLocationPath sequence
* It has already been tokenized
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* [2] AbsoluteLocationPath ::= '/' RelativeLocationPath?
*                     | AbbreviatedAbsoluteLocationPath        
*
* [10] AbbreviatedAbsoluteLocationPath ::= '//' RelativeLocationPath
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
    parse_absolute_location_path (xpath_pcb_t *pcb,
				  status_t *res)
{
    tk_type_t    nexttyp;
    boolean      abbrev;

    abbrev = FALSE;

    /* get  the first token  '/' or '//' */
    nexttyp = tk_next_typ(pcb->tkc); 
    if (nexttyp == TK_TT_FSLASH) {
        *res = xpath_parse_token(pcb, TK_TT_FSLASH);
    } else {
        abbrev = TRUE;
        *res = xpath_parse_token(pcb, TK_TT_DBLFSLASH);
    }

    if (*res != NO_ERR) {
        return NULL;
    }

    /* check if there is anything else to do;
     * OK to terminate an absolute path right here
     */
    if (!abbrev) {
        nexttyp = tk_next_typ(pcb->tkc);
        if (nexttyp == TK_TT_NONE) {
            /* exprstr is simply docroot '/' */
            *res = NO_ERR;

	    /* set the return value */
	    /****/
	    return NULL;
        }
    }

    /* some more input expected, which is a relative
     * path location in either case
     */
    return parse_relative_location_path(pcb, res);

}  /* parse_absolute_location_path */


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

    tk_type_t  nexttyp;

    nexttyp = tk_next_typ(pcb->tkc);
    if (nexttyp == TK_TT_FSLASH ||
        nexttyp == TK_TT_DBLFSLASH) {
        pcb->abspath = TRUE;
        return parse_absolute_location_path(pcb, res);
    } else {
        return parse_relative_location_path(pcb, res);
    }

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
	    log_error("\nError: wrong number of parameters got %d, need %d"
		      " for function '%s'",
		      parmcnt, fncb->parmcnt,
		      fncb->name);
	    ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
	} else {
	    /* make the function call */
	    val1 = (*fncb->fn)(pcb, &parmQ, res);
	}
    } else {
	*res = ERR_NCX_UNKNOWN_PARM;
	log_error("\nError: Invalid XPath function name '%s'",
		  TK_CUR_VAL(pcb->tkc));
	ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
    }

    /* clean up any function parameters */
    while (!dlq_empty(&parmQ)) {
	val2 = (xpath_result_t *)dlq_deque(&parmQ);
	xpath_free_result(val2);
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
    tk_type_t               nexttyp;

    val1 = NULL;
    nexttyp = tk_next_typ(pcb->tkc);

    switch (nexttyp) {
    case TK_TT_VARBIND:
	*res = xpath_parse_token(pcb, TK_TT_VARBIND);

	/* get NCName variable reference */
	if (*res == NO_ERR) {
	    *res = varbind_exists(pcb, NULL,
				  TK_CUR_VAL(pcb->tkc));
	    if (*res != NO_ERR) {
		if (*res == ERR_NCX_DEF_NOT_FOUND) {
		    log_error("\nError: unknown variable binding '%s'",
			      TK_CUR_VAL(pcb->tkc));
		    ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
		}
	    } else {
		;  /* make varptr */
	    }
	}
	break;
    case TK_TT_QVARBIND:
	*res = xpath_parse_token(pcb, TK_TT_QVARBIND);

	/* get QName variable reference */
	if (*res == NO_ERR) {
	    *res = varbind_exists(pcb, 
				  TK_CUR_MOD(pcb->tkc),
				  TK_CUR_VAL(pcb->tkc));
	    if (*res != NO_ERR) {
		if (*res == ERR_NCX_DEF_NOT_FOUND) {
		    log_error("\nError: unknown variable binding '%s:%s'",
			      TK_CUR_MOD(pcb->tkc),
			      TK_CUR_VAL(pcb->tkc));
		    ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
		}
	    } else {
		;  /* make varptr */
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
	    val1 = xpath_new_result(XP_RT_NUMBER);
	    if (!val1) {
		*res = ERR_INTERNAL_MEM;
		log_error("\nError: malloc failed in XPath parsing");
		ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
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
	    val1 = xpath_new_result(XP_RT_STRING);
	    if (!val1) {
		*res = ERR_INTERNAL_MEM;
		log_error("\nError: malloc failed in XPath parsing");
		ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
		return NULL;
	    }

	    val1->r.str = xml_strdup(TK_CUR_VAL(pcb->tkc));
	    if (!val1->r.str) {
		*res = ERR_INTERNAL_MEM;
		log_error("\nError: malloc failed in XPath parsing");
		ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
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
    default:
	/* unexpected token error */
	*res = ERR_NCX_WRONG_TKTYPE;
	log_error("\nError: wrong token type '%s', "
		  "value '%s' in primary expr",
		  tk_get_token_sym(nexttyp),
		  TK_CUR_VAL(pcb->tkc));
	ncx_print_errormsg(pcb->tkc, pcb->mod, *res);
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
    xpath_result_t  *val1, *val2, *save;
    tk_type_t        nexttyp;

    val1 = parse_primary_expr(pcb, res);

    if (*res != NO_ERR) {
	return val1;
    }

    /* peek ahead to check the possible next chars */
    nexttyp = tk_next_typ(pcb->tkc);
    while (nexttyp == TK_TT_LBRACK && *res==NO_ERR) {
	save = pcb->curfilter;
	pcb->curfilter = val1;
	val2 = parse_predicate(pcb, res);
	pcb->curfilter = save;
	if (val1) {
	    xpath_free_result(val1);
	}
	val1 = val2;
	nexttyp = tk_next_typ(pcb->tkc);
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

    /* peek ahead to check the possible next chars */
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
	val2 = val1;
	val1 = NULL;

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

	if (curop != XP_EXOP_NONE) {
	    val1 = parse_relative_location_path(pcb, res);
	    if (*res == NO_ERR) {
                *res = eval_xpath_op(val2, val1, curop);
            }
        }
    }

    if (val1) {
        xpath_free_result(val1);
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
                *res = eval_xpath_op(val2, val1, XP_EXOP_UNION);
                if (val1) {
                    xpath_free_result(val1);
                }
            } else {
                val2 = val1;
            }
            val1 = NULL;

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
        xpath_free_result(val1);
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
    xpath_result_t  *val1;
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
	/* odd number of negate ops requested */
	*res = eval_xpath_op(val1, NULL, XP_EXOP_NEGATE);
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
    xpath_result_t  *val1, *val2;
    xpath_exop_t     curop;
    boolean          done;
    tk_type_t        nexttyp;

    val1 = NULL;
    val2 = NULL;
    curop = XP_EXOP_NONE;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_unary_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
                *res = eval_xpath_op(val2, val1, curop);
                if (val1) {
                    xpath_free_result(val1);
                }
            } else {
                val2 = val1;
            }
            val1 = NULL;

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
        xpath_free_result(val1);
    }

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
    xpath_result_t  *val1, *val2;
    xpath_exop_t     curop;
    boolean          done;
    tk_type_t        nexttyp;

    val1 = NULL;
    val2 = NULL;
    curop = XP_EXOP_NONE;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_multiplicative_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
                *res = eval_xpath_op(val2, val1, curop);
                if (val1) {
                    xpath_free_result(val1);
                }
            } else {
                val2 = val1;
            }
            val1 = NULL;

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
        xpath_free_result(val1);
    }

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
    xpath_result_t  *val1, *val2;
    xpath_exop_t     curop;
    boolean          done;
    tk_type_t        nexttyp;

    val1 = NULL;
    val2 = NULL;
    curop = XP_EXOP_NONE;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_additive_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
                *res = eval_xpath_op(val2, val1, curop);
                if (val1) {
                    xpath_free_result(val1);
                }
            } else {
                val2 = val1;
            }
            val1 = NULL;

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
        xpath_free_result(val1);
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
    xpath_result_t  *val1, *val2;
    xpath_exop_t     curop;
    boolean          done;

    val1 = NULL;
    val2 = NULL;
    curop = XP_EXOP_NONE;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_relational_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
                *res = eval_xpath_op(val2, val1, curop);
                if (val1) {
                    xpath_free_result(val1);
                }
            } else {
                val2 = val1;
            }
            val1 = NULL;

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
        xpath_free_result(val1);
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
    xpath_result_t  *val1, *val2;
    boolean          done;

    val1 = NULL;
    val2 = NULL;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_equality_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
                *res = eval_xpath_op(val2, val1, XP_EXOP_AND);
                if (val1) {
                    xpath_free_result(val1);
                }
            } else {
                val2 = val1;
            }
            val1 = NULL;

	    if (*res != NO_ERR) {
		continue;
	    }

            if (match_next_token(pcb, TK_TT_STRING, XP_OP_AND)) {
                *res = xpath_parse_token(pcb, TK_TT_STRING);
            } else {
                done = TRUE;
            }
        }
    }

    if (val1) {
        xpath_free_result(val1);
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
    xpath_result_t  *val1, *val2;
    boolean          done;

    val1 = NULL;
    val2 = NULL;
    done = FALSE;

    while (!done && *res == NO_ERR) {
        val1 = parse_and_expr(pcb, res);

        if (*res == NO_ERR) {
            if (val2) {
                *res = eval_xpath_op(val2, val1, XP_EXOP_OR);
                if (val1) {
                    xpath_free_result(val1);
                }
            } else {
                val2 = val1;
            }
            val1 = NULL;

	    if (*res != NO_ERR) {
		continue;
	    }

            if (match_next_token(pcb, TK_TT_STRING, XP_OP_OR)) {
                *res = xpath_parse_token(pcb, TK_TT_STRING);
            } else {
                done = TRUE;
            }
        }
    }

    if (val1) {
        xpath_free_result(val1);
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
* Parse the XPATH 1.0 expression string
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
*
* INPUTS:
*    tkc == parent token chain
*    mod == module in progress
*    pcb == initialized xpath parser control block
*           for the keyref path; use xpath_new_pcb
*           to initialize before calling this fn
*    source == enum indicating source of this expression
*
* OUTPUTS:
*   pcb->tkc is filled and then validated
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

    /* the module that contains the keyref is the one
     * that will always be used to resolve prefixes
     * within the XPath expression
     */
    pcb->mod = mod;
    pcb->source = source;
    pcb->obj = NULL;
    pcb->objmod = NULL;
    pcb->parseres = NO_ERR;

    /* since the pcb->obj is not set, this validation
     * phase will skip identifier tests, predicate tests
     * and completeness tests
     */
    result = parse_expr(pcb, &pcb->parseres);
    if (result) {
	xpath_free_result(result);
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
*   - QNames are valid
*   - object structure referenced is valid
*   - objects are all 'config true'
*   - target object is a leaf
*   - keyref represents a single instance
*
* Called after all 'uses' and 'augment' expansion
* so validation against cooked object tree can be done
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*    mod == module containing the 'obj' (in progress)
*    obj == object using the keyref data type
*    pcb == the keyref parser control block from the typdef
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath1_validate_expr (ncx_module_t *mod,
                          const obj_template_t *obj,
                          xpath_pcb_t *pcb)
{
#if 0
    status_t  res;

#ifdef DEBUG
    if (!mod || !obj || !pcb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!pcb->tkc) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    if (pcb->parseres != NO_ERR) {
        /* errors already reported, skip this one */
        return NO_ERR;
    }

    pcb->docroot = ncx_get_gen_root();
    if (!pcb->docroot) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    tk_reset_chain(pcb->tkc);
    pcb->objmod = mod;
    pcb->obj = obj;
    pcb->source = XP_SRC_KEYREF;
    pcb->targobj = NULL;
    pcb->altobj = NULL;
    pcb->varobj = NULL;
    pcb->curmode = XP_CM_TARGET;

    /* validate the XPath expression against the 
     * full cooked object tree
     */
    pcb->validateres = parse_path_arg(pcb);

    /* check keyref is config but target is not */
    if (pcb->validateres == NO_ERR && pcb->targobj) {
        if (obj_get_config_flag(obj) &&
            !obj_get_config_flag(pcb->targobj)) {
            res = ERR_NCX_NOT_CONFIG;
            log_error("\nError: XPath target '%s' for keyref '%s' must be "
                      "a config object",
                      obj_get_name(pcb->targobj),
                      obj_get_name(obj));
            ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
            pcb->validateres = res;
        }

        switch (pcb->source) {
        case XP_SRC_KEYREF:
            if (!obj_is_key(pcb->targobj)) {
                res = ERR_NCX_TYPE_NOT_INDEX;
                log_error("\nError: path target '%s' is "
                          "not a key leaf",
                          obj_get_name(pcb->targobj));
                ncx_mod_exp_err(pcb->tkc, 
                                pcb->objmod, 
                                res, "key leaf");
                pcb->validateres = res;
            }
            break;
        case XP_SRC_LEAFREF:
            break;
        default:
            ;
        }

        if (pcb->targobj == pcb->obj) {
            res = ERR_NCX_DEF_LOOP;
            log_error("\nError: path target '%s' is set to "
                      "the target object",
                      obj_get_name(pcb->targobj));
            ncx_print_errormsg(pcb->tkc, pcb->objmod, res);
        }
    }

    return pcb->validateres;
#else
    return NO_ERR;
#endif
}  /* xpath1_validate_expr */


/********************************************************************
* FUNCTION xpath1_get_value
* 
* Get the expression nodeset result
*
* INPUTS:
*    mod == module in progress
*    val == value node initiating search, which contains the XPath expr
*    pcb == XPath parser control block to use
*
* OUTPUTS:
*   if non-NULL:
*      pcb->result == filles with the result nodeset
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xpath1_get_value (ncx_module_t *mod,
                      val_value_t *val,
                      xpath_pcb_t *pcb)
{

#ifdef DEBUG
    if (!mod || !val || !pcb) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /*****/

    return NO_ERR;

}  /* xpath1_get_value */


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
