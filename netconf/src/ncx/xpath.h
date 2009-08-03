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
#include <xmlreader.h>
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

#ifndef _H_var
#include "var.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* max size of the pcb->result_cacheQ */
#define XPATH_RESULT_CACHE_MAX     16

/* max size of the pcb->resnode_cacheQ */
#define XPATH_RESNODE_CACHE_MAX     64


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
#define XP_FN_BOOLEAN              (const xmlChar *)"boolean"
#define XP_FN_CEILING              (const xmlChar *)"ceiling"
#define XP_FN_CONCAT               (const xmlChar *)"concat"
#define XP_FN_CONTAINS             (const xmlChar *)"contains"
#define XP_FN_COUNT                (const xmlChar *)"count"
#define XP_FN_CURRENT              (const xmlChar *)"current"
#define XP_FN_FALSE                (const xmlChar *)"false"
#define XP_FN_FLOOR                (const xmlChar *)"floor"
#define XP_FN_ID                   (const xmlChar *)"id"
#define XP_FN_LANG                 (const xmlChar *)"lang"
#define XP_FN_LAST                 (const xmlChar *)"last"
#define XP_FN_LOCAL_NAME           (const xmlChar *)"local-name"
#define XP_FN_NAME                 (const xmlChar *)"name"
#define XP_FN_NAMESPACE_URI        (const xmlChar *)"namespace-uri"
#define XP_FN_NORMALIZE_SPACE      (const xmlChar *)"normalize-space"
#define XP_FN_NOT                  (const xmlChar *)"not"
#define XP_FN_NUMBER               (const xmlChar *)"number"
#define XP_FN_POSITION             (const xmlChar *)"position"
#define XP_FN_ROUND                (const xmlChar *)"round"
#define XP_FN_STARTS_WITH          (const xmlChar *)"starts-with"
#define XP_FN_STRING               (const xmlChar *)"string"
#define XP_FN_STRING_LENGTH        (const xmlChar *)"string-length"
#define XP_FN_SUBSTRING            (const xmlChar *)"substring"
#define XP_FN_SUBSTRING_AFTER      (const xmlChar *)"substring-after"
#define XP_FN_SUBSTRING_BEFORE     (const xmlChar *)"substring-before"
#define XP_FN_SUM                  (const xmlChar *)"sum"
#define XP_FN_TRANSLATE            (const xmlChar *)"translate"
#define XP_FN_TRUE                 (const xmlChar *)"true"



/* XPath NodeType values */
#define XP_NT_COMMENT              (const xmlChar *)"comment"
#define XP_NT_TEXT                 (const xmlChar *)"text"
#define XP_NT_PROCESSING_INSTRUCTION \
    (const xmlChar *)"processing-instruction"
#define XP_NT_NODE                 (const xmlChar *)"node"

/* XPath 1.0 operator names */
#define XP_OP_AND                  (const xmlChar *)"and"
#define XP_OP_OR                   (const xmlChar *)"or"
#define XP_OP_DIV                  (const xmlChar *)"div"
#define XP_OP_MOD                  (const xmlChar *)"mod"

/* XPath control block flag definitions */

/* If dynnode is present, then at least one component
 * within the entire XPath expression is variable.
 * (e.g  ../parent == 'fred'  or $foo + 1
 *
 * If not set, then the entire expression is constant
 * (e.g.,  34 mod 2 or 48 and 'fred')
 */
#define XP_FL_DYNNODE            bit0


/* during XPath evaluation, skipping the rest of a
 * FALSE AND expression
 */
#define XP_FL_SKIP_FAND          bit1


/* during XPath evaluation, skipping the rest of a
 * TRUE OR expression
 */
#define XP_FL_SKIP_TOR           bit2


/* used by xpath_leafref.c to keep track of path type */
#define XP_FL_ABSPATH            bit3


/* used for YANG/NETCONF to auto-filter any non-config nodes
 * that are matched by an XPath wildcard mechanism
 */
#define XP_FL_CONFIGONLY         bit4

/* used to indicate the top object node is set
 * FALSE to indicate that all the ncx_module_t datadefQs
 * need to be searched instead
 */
#define XP_FL_USEROOT             bit5

/* used to restrict the XPath expression to the YANG
 * instance-identifier syntax
 */
#define XP_FL_INSTANCEID          bit6


/* used to restrict the XPath expression to an 
 * ncx:schema-instance string syntax
 */
#define XP_FL_SCHEMA_INSTANCEID          bit7


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* XPath expression result type */
typedef enum xpath_restype_t_ {
    XP_RT_NONE,
    XP_RT_NODESET,
    XP_RT_NUMBER,
    XP_RT_STRING,
    XP_RT_BOOLEAN
} xpath_restype_t;

/* XPath dynamic parsing mode for leafref */
typedef enum xpath_curmode_t_ {
    XP_CM_NONE,
    XP_CM_TARGET,
    XP_CM_ALT,
    XP_CM_KEYVAR
} xpath_curmode_t;


/* document root type */
typedef enum xpath_document_t_ {
    XP_DOC_NONE,
    XP_DOC_DATABASE,
    XP_DOC_RPC,
    XP_DOC_RPC_REPLY,
    XP_DOC_NOTIFICATION
} xpath_document_t;


/* XPath expression source type */
typedef enum xpath_source_t_ {
    XP_SRC_NONE,
    XP_SRC_LEAFREF,
    XP_SRC_YANG,
    XP_SRC_INSTANCEID,
    XP_SRC_XML
} xpath_source_t;


/* XPath expression operation type */
typedef enum xpath_exop_t_ {
    XP_EXOP_NONE,
    XP_EXOP_AND,         /* keyword 'and' */
    XP_EXOP_OR,          /* keyword 'or' */
    XP_EXOP_EQUAL,       /* equals '=' */
    XP_EXOP_NOTEQUAL,    /* bang equals '!=' */
    XP_EXOP_LT,          /* left angle bracket '<' */
    XP_EXOP_GT,          /* right angle bracket '>' */
    XP_EXOP_LEQUAL,      /* l. angle-equals '<= */
    XP_EXOP_GEQUAL,      /* r. angle-equals '>=' */
    XP_EXOP_ADD,         /* plus sign '+' */
    XP_EXOP_SUBTRACT,    /* minus '-' */
    XP_EXOP_MULTIPLY,    /* asterisk '*' */
    XP_EXOP_DIV,         /* keyword 'div' */
    XP_EXOP_MOD,         /* keyword 'mod' */
    XP_EXOP_NEGATE,      /* unary '-' */
    XP_EXOP_UNION,       /* vert. bar '|' */
    XP_EXOP_FILTER1,     /* fwd slash '/' */
    XP_EXOP_FILTER2      /* double fwd slash (C++ comment) */
} xpath_exop_t;


/* XPath expression node types */
typedef enum xpath_nodetype_t_ {
    XP_EXNT_NONE,
    XP_EXNT_COMMENT,
    XP_EXNT_TEXT,
    XP_EXNT_PROC_INST,
    XP_EXNT_NODE
} xpath_nodetype_t;


/* XPath result node struct */
typedef struct xpath_resnode_t_ {
    dlq_hdr_t             qhdr;
    boolean               dblslash;
    int64                 position;
    int64                 last;   /* only set in context node */
    union node_ {
	const obj_template_t *objptr;
	val_value_t          *valptr;
    } node;
} xpath_resnode_t;


/* XPath expression result */
typedef struct xpath_result_t_ {
    dlq_hdr_t            qhdr;        /* in case saved in a Q */
    xpath_restype_t      restype;
    boolean              isval;   /* matters if XP_RT_NODESET */
    int64                last;    /* used with XP_RT_NODESET */
    union r_ {
	dlq_hdr_t         nodeQ;       /* Q of xpath_resnode_t */
	boolean           bool; 
	ncx_num_t         num;
	xmlChar          *str;
    } r;

    status_t             res;
} xpath_result_t;


/* XPath parser control block */
typedef struct xpath_pcb_t_ {
    dlq_hdr_t            qhdr;           /* in case saved in a Q */
    tk_chain_t          *tkc;               /* chain for exprstr */
    tk_token_t          *tk;              /* back-ptr for errors */
    xmlChar             *exprstr;           /* YANG XPath string */
    xmlTextReaderPtr     reader;            /* get NS inside XML */

    /* the prefixes in the QNames in the exprstr MUST be resolved
     * in different contexts.  
     *
     * For must/when/leafref XPath, the prefix is a module prefix
     * which must match an import statement in the 'mod' import Q
     *
     * For XML context (NETCONF PDU 'select' attribute)
     * the prefix is part of an extended name, representing
     * XML namespace for the module that defines that node
     */
    ncx_module_t        *mod;         /* bptr to exprstr context */
    xpath_source_t       source;
    ncx_errinfo_t        errinfo;            /* must error extras */
    boolean              logerrors;     /* T: use log_error F: agt */

    /* these parms are used to parse leafref path-arg 
     * limited object tree syntax allowed only
     */
    const obj_template_t  *targobj;       /* bptr to result object */
    const obj_template_t  *altobj;     /* bptr to pred. RHS object */
    const obj_template_t  *varobj;  /* bptr to key-expr LHS object */
    xpath_curmode_t        curmode;     /* select targ/alt/var obj */

    /* these parms are used by leafref and XPath1 parsing */
    const obj_template_t  *obj;            /* bptr to start object */
    ncx_module_t          *objmod;        /* module containing obj */
    const obj_template_t  *docroot;        /* bptr to <config> obj */
    xpath_document_t       doctype;
    val_value_t           *val;                  /* current() node */
    val_value_t           *val_docroot;        /* cfg->root for db */
    /* these parms are used for XPath1 processing
     * against a target database 
     */
    uint32               flags;
    xpath_result_t      *result;

    /* additive XPath1 context back- pointer to current 
     * step results; initially NULL and modified until
     * the expression is done
     */
    xpath_resnode_t      context;

    /* The varbindQ is passed in as a parameter by the app
     * It contains zero or more ncx_var_t structs
     */
    dlq_hdr_t           *varbindQ;         /* Q of ncx_var_t */

    /* The function Q is a copy of the global Q
     * It is not hardwired in case app-specific extensions
     * are added later -- array of xpath_fncb_t
     */
    const struct xpath_fncb_t_ *functions; 

    /* Performance Caches
     * The xpath_result_t and xpath_resnode_t structs
     * are used in many intermediate operations
     *
     * These Qs are used to cache these structs
     * instead of calling malloc and free constantly
     *
     * The XPATH_RESULT_CACHE_MAX and XPATH_RESNODE_CACHE_MAX
     * constants are used to control the max cache sizes
     * This is not user-configurable (TBD).
     */
    dlq_hdr_t           result_cacheQ;  /* Q of xpath_result_t */
    dlq_hdr_t           resnode_cacheQ;  /* Q of xpath_resnode_t */
    uint32              result_count;
    uint32              resnode_count;


    /* first and second pass parsing results
     * the next phase will not execute until
     * all previous phases have a NO_ERR status
     */
    status_t             parseres;
    status_t             validateres;
    status_t             valueres;

    /* saved error info for the agent to process */
    const tk_token_t    *errtoken;
    uint32               errpos;

    boolean              seen;      /* yangdiff support */
} xpath_pcb_t;


/* XPath function prototype */
typedef xpath_result_t *
    (*xpath_fn_t) (xpath_pcb_t *pcb,
		   dlq_hdr_t *parmQ,   /* Q of xpath_result_t */
		   status_t *res);


/* XPath function control block */
typedef struct xpath_fncb_t_ {
    const xmlChar     *name;
    xpath_restype_t    restype;
    int32              parmcnt;   /* -1 == N, 0..N == actual cnt */
    xpath_fn_t         fn;
} xpath_fncb_t;


/* Value or object node walker fn callback parameters */
typedef struct xpath_walkerparms_t_ {
    dlq_hdr_t         *resnodeQ;
    int64              callcount;
    status_t           res;
} xpath_walkerparms_t;


/* Value node compare walker fn callback parameters */
typedef struct xpath_compwalkerparms_t_ {
    xpath_result_t    *result2;
    xmlChar           *cmpstring;
    ncx_num_t         *cmpnum;
    xmlChar           *buffer;
    uint32             buffsize;
    xpath_exop_t       exop;
    boolean            cmpresult;
    status_t           res;
} xpath_compwalkerparms_t;


/* Value node stringify walker fn callback parameters */
typedef struct xpath_stringwalkerparms_t_ {
    xmlChar           *buffer;
    uint32             buffsize;
    uint32             buffpos;
    status_t           res;
} xpath_stringwalkerparms_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* find target, save in *targobj */
extern status_t
    xpath_find_schema_target (yang_pcb_t *pcb,
                              tk_chain_t *tkc,
			      ncx_module_t *mod,
			      obj_template_t *obj,
			      dlq_hdr_t  *datadefQ,
			      const xmlChar *target,
			      obj_template_t **targobj,
			      dlq_hdr_t **targQ);

/* find target module name, return malloced module name string */
extern xmlChar *
    xpath_find_schema_target_modname (tk_chain_t *tkc,
                                      ncx_module_t *mod,
                                      const xmlChar *target,
                                      status_t *res);

/* find target, save in *targobj, use the errtk if error */
extern status_t
    xpath_find_schema_target_err (yang_pcb_t *pcb,
                                  tk_chain_t *tkc,
				  ncx_module_t *mod,
				  obj_template_t *obj,
				  dlq_hdr_t  *datadefQ,
				  const xmlChar *target,
				  obj_template_t **targobj,
				  dlq_hdr_t **targQ,
				  tk_token_t *errtk);


/* internal find target, without any error reporting */
extern status_t
    xpath_find_schema_target_int (const xmlChar *target,
				  obj_template_t **targobj);

/* used by cfg.c to find parms in the value struct for
 * a config file (ncx:cli)
 */
extern status_t
    xpath_find_val_target (val_value_t *startval,
			   ncx_module_t *mod,
			   const xmlChar *target,
			   val_value_t **targval);

/* called by agent to find a descendant value node
 * based  on a relative-path sub-clause of a unique-stmt
 */
extern status_t
    xpath_find_val_unique (val_value_t *startval,
			   ncx_module_t *mod,
			   const xmlChar *target,
			   boolean logerrors,
			   val_value_t **targval);

/* malloc a new XPath parser control block
 * xpathstr is allowed to be NULL, otherwise
 * a strdup will be made and exprstr will be set
 */
extern xpath_pcb_t *
    xpath_new_pcb (const xmlChar *xpathstr);

/* copy from typdef to object for leafref
 * of object to value for NETCONF PDU processing
 */
extern xpath_pcb_t *
    xpath_clone_pcb (const xpath_pcb_t *srcpcb);

/* find by exact match of the expressions string */
extern xpath_pcb_t *
    xpath_find_pcb (dlq_hdr_t *pcbQ, 
		    const xmlChar *exprstr);

/* free an XPath parser control block */
extern void
    xpath_free_pcb (xpath_pcb_t *pcb);

/* malloc an XPath result */
extern xpath_result_t *
    xpath_new_result (xpath_restype_t restype);

/* malloc an XPath result node */
extern void 
    xpath_init_result (xpath_result_t *result,
		       xpath_restype_t restype);

/* free an XPath result */
extern void
    xpath_free_result (xpath_result_t *result);

extern void
    xpath_clean_result (xpath_result_t *result);

extern xpath_resnode_t *
    xpath_new_resnode (void);

extern void 
    xpath_init_resnode (xpath_resnode_t *resnode);

extern void
    xpath_free_resnode (xpath_resnode_t *resnode);

extern void
    xpath_clean_resnode (xpath_resnode_t *resnode);


extern status_t
    xpath_get_curmod_from_prefix (const xmlChar *prefix,
				  ncx_module_t *mod,
				  ncx_module_t **targmod);

extern status_t
    xpath_get_curmod_from_prefix_str (const xmlChar *prefix,
				      uint32 prefixlen,
				      ncx_module_t *mod,
				      ncx_module_t **targmod);

extern status_t
    xpath_parse_token (xpath_pcb_t *pcb,
		       tk_type_t  tktype);

extern boolean
    xpath_cvt_boolean (const xpath_result_t *result);

extern void
    xpath_cvt_number (const xpath_result_t *result,
		      ncx_num_t *num);

extern status_t
    xpath_cvt_string (xpath_pcb_t *pcb,
		      const xpath_result_t *result,
		      xmlChar **str);

extern dlq_hdr_t *
    xpath_get_resnodeQ (xpath_result_t *result);

#endif	    /* _H_xpath */
