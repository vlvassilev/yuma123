#ifndef _H_xpath
#define _H_xpath

/*  FILE: xpath.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Schema and data model Xpath search support

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
30-dec-07    abb      Begun

*/

#include <xmlstring.h>
#include <xmlregexp.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_val
#include "val.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/


/* XPath 1.0 sec 2.2 AxisName */
#define XP_AXIS_ANCESTOR           (const xmlChar *)"ancestor"
#define XP_AXIS_ANCESTOR_OR_SELF   (const xmlChar *)"ancestor-or-self"
#define XP_AXIS_ATTRIBUTE          (const xmlChar *)"attribute"
#define XP_AXIS_CHILD              (const xmlChar *)"child"
#define XP_AXIS_DESCENDANT         (const xmlChar *)"descendant"
#define XP_AXIS_DESCENDANT_OR_SELF (const xmlChar *)"descendant-or-self"
#define XP_AXIS_FOLLOWING          (const xmlChar *)"following"
#define XP_AXIS_FOLLOWING_SIBLING  (const xmlChar *)"following-sibling"
#define XP_AXIS_NAMESPACE          (const xmlChar *)"namespace"
#define XP_AXIS_PARENT             (const xmlChar *)"parent"
#define XP_AXIS_PRECEDING          (const xmlChar *)"preceding"
#define XP_AXIS_PRECEDING_SIBLING  (const xmlChar *)"preceding-sibling"
#define XP_AXIS_SELF               (const xmlChar *)"self"

/* Xpath 1.0 Function library + current() from XPath 2.0 */
#define XP_FN_LAST                 (const xmlChar *)"last"
#define XP_FN_POSITION             (const xmlChar *)"position"
#define XP_FN_COUNT                (const xmlChar *)"count"
#define XP_FN_ID                   (const xmlChar *)"id"
#define XP_FN_LOCAL_NAME           (const xmlChar *)"local-name"
#define XP_FN_NAMESPACE_URI        (const xmlChar *)"namespace-uri"
#define XP_FN_NAME                 (const xmlChar *)"name"
#define XP_FN_STRING               (const xmlChar *)"string"
#define XP_FN_CONCAT               (const xmlChar *)"concat"
#define XP_FN_STARTS_WITH          (const xmlChar *)"starts-with"
#define XP_FN_CONTAINS             (const xmlChar *)"contains"
#define XP_FN_SUBSTRING_BEFORE     (const xmlChar *)"substring-before"
#define XP_FN_SUBSTRING_AFTER      (const xmlChar *)"substring-after"
#define XP_FN_SUBSTRING            (const xmlChar *)"substring"
#define XP_FN_STRING_LENGTH        (const xmlChar *)"string-length"
#define XP_FN_NORMALIZE_SPACE      (const xmlChar *)"normalize-space"
#define XP_FN_TRANSLATE            (const xmlChar *)"translate"
#define XP_FN_BOOLEAN              (const xmlChar *)"boolean"
#define XP_FN_NOT                  (const xmlChar *)"not"
#define XP_FN_TRUE                 (const xmlChar *)"true"
#define XP_FN_FALSE                (const xmlChar *)"false"
#define XP_FN_LANG                 (const xmlChar *)"lang"
#define XP_FN_NUMBER               (const xmlChar *)"number"
#define XP_FN_SUM                  (const xmlChar *)"sum"
#define XP_FN_FLOOR                (const xmlChar *)"floor"
#define XP_FN_CEILING              (const xmlChar *)"ceiling"
#define XP_FN_ROUND                (const xmlChar *)"round"
#define XP_FN_CURRENT              (const xmlChar *)"current"


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* XPath nodeset result */
typedef struct xpath_nodeset_t_ {
    dlq_hdr_t            nodeQ;        /* Q of val_value_t */
    const tk_token_t    *errtoken;
    uint32               errpos;
    status_t             res;
} xpath_nodeset_t;

typedef enum xpath_curmode_t_ {
    XP_CM_NONE,
    XP_CM_TARGET,
    XP_CM_ALT,
    XP_CM_KEYVAR
} xpath_curmode_t;

typedef enum xpath_source_t_ {
    XP_SRC_NONE,
    XP_SRC_KEYREF,
    XP_SRC_LEAFREF,
    XP_SRC_MUST,
    XP_SRC_WHEN
} xpath_source_t;

/* XPath parser control block */
typedef struct xpath_pcb_t_ {
    tk_chain_t          *tkc;
    xmlChar             *exprstr;
    ncx_module_t        *mod;         /* bptr to exprstr context */
    boolean              abspath;
    xpath_source_t       source;

    /* these 2 parms are used to parse keyref path-arg 
     * limited object tree syntax allowed only
     */
    ncx_module_t          *objmod;        /* module containing obj */
    const obj_template_t  *docroot;        /* bptr to <config> obj */
    const obj_template_t  *obj;            /* bptr to start object */
    const obj_template_t  *targobj;       /* bptr to result object */
    const obj_template_t  *altobj;       /* bptr to result object */
    const obj_template_t  *varobj;       /* bptr to key-expr object */
    xpath_curmode_t        curmode;   

    /* these parms are used for must and when processing
     * against a target database ; full XPath 1.0 allowed
     */
    val_value_t         *cxtnode;
    xpath_nodeset_t      result;
    uint32               cxtpos;
    uint32               cxtsize;
    dlq_hdr_t            varbindQ;        /* Q of val_value_t */
    status_t             parseres;
    status_t             validateres;
    
} xpath_pcb_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* malloc and init an NCX database object template */
extern status_t
    xpath_find_schema_target (tk_chain_t *tkc,
			      ncx_module_t *mod,
			      obj_template_t *obj,
			      dlq_hdr_t  *datadefQ,
			      const xmlChar *target,
			      obj_template_t **targobj,
			      dlq_hdr_t **targQ);

extern status_t
    xpath_find_schema_target_err (tk_chain_t *tkc,
				  ncx_module_t *mod,
				  obj_template_t *obj,
				  dlq_hdr_t  *datadefQ,
				  const xmlChar *target,
				  obj_template_t **targobj,
				  dlq_hdr_t **targQ,
				  tk_token_t *errtk);


extern status_t
    xpath_find_schema_target_int (const xmlChar *target,
				  obj_template_t **targobj);

extern status_t
    xpath_find_val_target (val_value_t *startval,
			   ncx_module_t *mod,
			   const xmlChar *target,
			   val_value_t **targval);


extern xpath_pcb_t *
    xpath_new_pcb (const xmlChar *xpathstr);


extern void
    xpath_free_pcb (xpath_pcb_t *pcb);

extern status_t
    xpath_get_curmod_from_prefix (const xmlChar *prefix,
				  ncx_module_t *mod,
				  ncx_module_t **targmod);

#endif	    /* _H_xpath */
