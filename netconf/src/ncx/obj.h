#ifndef _H_obj
#define _H_obj

/*  FILE: obj.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Data Object Support

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
09-dec-07    abb      Begun
21jul08      abb      start obj-based rewrite

*/

#include <xmlstring.h>
#include <xmlregexp.h>

#ifndef _H_grp
#include "grp.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* default vaule for config statement */
#define OBJ_DEF_CONFIG      TRUE

/* default vaule for mandatory statement */
#define OBJ_DEF_MANDATORY   FALSE


/* flags field in obj_template_t */

/* object is cloned from a grouping, for a uses statement */
#define OBJ_FL_CLONE        bit0

 /* def is cloned flag
  *   == 0 : obj.def.foo is malloced, but the typdef is cloned
  *   == 1 : obj.def.foo: foo is cloned
  */
#define OBJ_FL_DEFCLONE     bit1   

/* clone source
 *   == 0 : cloned object from uses
 *   == 1 : cloned object from augment
 */
#define OBJ_FL_AUGCLONE     bit2

/* object is a refinement within a uses, not a real object */
#define OBJ_FL_REFINE       bit3

/* object is conditional, via a when-stmt expression */
#define OBJ_FL_CONDITIONAL  bit4

/* object is a top-level definition within a module or submodule */
#define OBJ_FL_TOP          bit5

/* object was entered with a 'kw name;' format and is 
 * considered empty by the yangdump program
 */
#define OBJ_FL_EMPTY        bit6

/* object has been visited by the yangdiff program */
#define OBJ_FL_SEEN         bit7

/* object marked as changed by the yangdiff program */
#define OBJ_FL_DIFF         bit8

/* object is marked as ncx:hidden */
#define OBJ_FL_HIDDEN       bit9

/* object is marked as ncx:root */
#define OBJ_FL_ROOT         bit10

/* object is marked as a password */
#define OBJ_FL_PASSWD       bit11

/* object is marked as a CLI-only node */
#define OBJ_FL_CLI          bit12

/* object is marked as an XSD list data type */
#define OBJ_FL_XSDLIST      bit13

/* OBJ_TYP_LEAF object is being uses as a key */
#define OBJ_FL_KEY          bit14

/* object is marked as abstract: not CLI or config data */
#define OBJ_FL_ABSTRACT     bit15

/* object is marked as config set */
#define OBJ_FL_CONFSET      bit16

/* object config value */
#define OBJ_FL_CONFIG       bit17

/* object is marked as mandatory set */
#define OBJ_FL_MANDSET      bit18

/* object mandatory value */
#define OBJ_FL_MANDATORY    bit19

/* object used in a unique-stmt within a list */
#define OBJ_FL_UNIQUE       bit20

/* object data type is an XPath string */
#define OBJ_FL_XPATH        bit21

/* object data type is a QName string */
#define OBJ_FL_QNAME        bit22

/* object data type is a schema-instance string */
#define OBJ_FL_SCHEMAINST   bit23

/* object is tagged ncx:secure */
#define OBJ_FL_SECURE       bit24

/* object is tagged ncx:very-secure */
#define OBJ_FL_VERY_SECURE  bit25

/* object is marked for deletion */
#define OBJ_FL_DELETED      bit26


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/* enumeration for different YANG data def statement types
 * the enum order is significant!!! do not change!!!
 */
typedef enum obj_type_t_ {
    OBJ_TYP_NONE,
    OBJ_TYP_ANYXML,
    OBJ_TYP_CONTAINER,
    OBJ_TYP_LEAF,
    OBJ_TYP_LEAF_LIST,
    OBJ_TYP_LIST,       /* last real database object */
    OBJ_TYP_CHOICE,
    OBJ_TYP_CASE,       /* last named database object */
    OBJ_TYP_USES,
    OBJ_TYP_REFINE,            /* child of uses only */
    OBJ_TYP_AUGMENT,
    OBJ_TYP_RPC,
    OBJ_TYP_RPCIO,
    OBJ_TYP_NOTIF
} obj_type_t;


/* enumeration for different YANG augment statement types */
typedef enum obj_augtype_t_ {
    OBJ_AUGTYP_NONE,
    OBJ_AUGTYP_RPCIN,
    OBJ_AUGTYP_RPCOUT,
    OBJ_AUGTYP_CASE,
    OBJ_AUGTYP_DATA
} obj_augtype_t;


/* One YANG list key component */
typedef struct obj_key_t_ {
    dlq_hdr_t       qhdr;
    struct obj_template_t_ *keyobj;
    boolean         seen;   /* used by yangdiff */
} obj_key_t;


/* One component in a YANG list unique target */
typedef struct obj_unique_comp_t_ {
    dlq_hdr_t               qhdr;
    struct obj_template_t_ *unobj;
    xmlChar                 *xpath;       /* saved unique str for this obj */
} obj_unique_comp_t;


/* One component in a YANG list unique target */
typedef struct obj_unique_t_ {
    dlq_hdr_t       qhdr;
    xmlChar        *xpath;       /* complete saved unique str */
    dlq_hdr_t       compQ;          /* Q of obj_unique_comp_t */
    boolean         seen;               /* needed by yangdiff */
    ncx_error_t     tkerr;
} obj_unique_t;


/* One YANG 'container' definition */
typedef struct obj_container_t_ {
    xmlChar       *name;
    xmlChar       *descr;
    xmlChar       *ref;
    xmlChar       *presence;
    dlq_hdr_t     *typedefQ;       /* Q of typ_template_t */
    dlq_hdr_t     *groupingQ;      /* Q of grp_template_t */
    dlq_hdr_t     *datadefQ;       /* Q of obj_template_t */
    boolean        datadefclone;
    ncx_status_t   status;
    dlq_hdr_t      mustQ;             /* Q of xpath_pcb_t */
    struct obj_template_t_ *defaultparm;
} obj_container_t;


/* One YANG 'leaf' or 'anyxml' definition */
typedef struct obj_leaf_t_ {
    xmlChar       *name;
    xmlChar       *units;
    xmlChar       *defval;
    xmlChar       *descr;
    xmlChar       *ref;
    typ_def_t     *typdef;
    ncx_status_t   status;
    dlq_hdr_t      mustQ;              /* Q of xpath_pcb_t */
    struct obj_template_t_ *leafrefobj;
} obj_leaf_t;


/* One YANG 'leaf-list' definition */
typedef struct obj_leaflist_t_ {
    xmlChar       *name;
    xmlChar       *units;
    xmlChar       *descr;
    xmlChar       *ref;
    typ_def_t     *typdef;
    boolean        ordersys; /* ordered-by system or user */
    boolean        minset;
    uint32         minelems;
    boolean        maxset;
    uint32         maxelems;
    ncx_status_t   status;
    dlq_hdr_t      mustQ;              /* Q of xpath_pcb_t */
    struct obj_template_t_ *leafrefobj;
} obj_leaflist_t;


/* One YANG 'list' definition */
typedef struct obj_list_t_ {
    xmlChar       *name;
    xmlChar       *keystr;
    xmlChar       *descr;
    xmlChar       *ref;
    dlq_hdr_t     *typedefQ;         /* Q of typ_template_t */
    dlq_hdr_t     *groupingQ;       /* Q of grp_template_t */
    dlq_hdr_t     *datadefQ;        /* Q of obj_template_t */
    dlq_hdr_t      keyQ;                 /* Q of obj_key_t */
    dlq_hdr_t      uniqueQ;           /* Q of obj_unique_t */
    boolean        datadefclone;
    boolean        ordersys;   /* ordered-by system or user */
    boolean        minset;
    uint32         minelems;
    boolean        maxset;
    uint32         maxelems;
    ncx_status_t   status;
    dlq_hdr_t      mustQ;              /* Q of xpath_pcb_t */
    ncx_error_t    keytkerr;
} obj_list_t;


/* One YANG 'choice' definition */
typedef struct obj_choice_t_ {
    xmlChar       *name;
    xmlChar       *defval;
    xmlChar       *descr;
    xmlChar       *ref;
    dlq_hdr_t     *caseQ;             /* Q of obj_template_t */
    boolean        caseQclone;
    ncx_status_t   status;
} obj_choice_t;


/* One YANG 'case' definition */
typedef struct obj_case_t_ {
    xmlChar        *name;
    xmlChar        *descr;
    xmlChar        *ref;
    dlq_hdr_t      *datadefQ;         /* Q of obj_template_t */
    boolean         nameclone;
    boolean         datadefclone;
    ncx_status_t    status;
} obj_case_t;


/* YANG uses statement struct */
typedef struct obj_uses_t_ {
    xmlChar          *prefix;
    xmlChar          *name;
    xmlChar          *descr;
    xmlChar          *ref;
    grp_template_t   *grp;      /* const back-ptr to grouping */
    dlq_hdr_t        *datadefQ;         /* Q of obj_template_t */
    ncx_status_t      status;
} obj_uses_t;


/* YANG refine statement struct */
typedef struct obj_refine_t_ {
    xmlChar          *target;
    struct obj_template_t_ *targobj;

    /* the token for each sub-clause is saved because
     * when the refine-stmt is parsed, the target is not
     * known yet so picking the correct variant
     * such as refine-leaf-stmts or refine-list-stmts
     * needs to wait until the resolve phase
     */
    xmlChar          *descr;
    ncx_error_t       descr_tkerr;
    xmlChar          *ref;
    ncx_error_t       ref_tkerr;
    xmlChar          *presence;
    ncx_error_t       presence_tkerr;
    xmlChar          *def;
    ncx_error_t       def_tkerr;
    /* config and confset are in the object flags */
    ncx_error_t       config_tkerr;
    /* mandatory and mandset are in the object flags */
    ncx_error_t       mandatory_tkerr;
    uint32            minelems;
    ncx_error_t       minelems_tkerr;   /* also minset */
    uint32            maxelems;
    ncx_error_t       maxelems_tkerr;   /* also maxset */
    dlq_hdr_t         mustQ;
} obj_refine_t;


/* YANG input-stmt or output-stmt struct */
typedef struct obj_rpcio_t_ {
    xmlChar           *name;                 /* input or output */
    dlq_hdr_t          typedefQ;         /* Q of typ_template_t */
    dlq_hdr_t          groupingQ;        /* Q of gtp_template_t */
    dlq_hdr_t          datadefQ;         /* Q of obj_template_t */
    struct obj_template_t_ *defaultparm;
} obj_rpcio_t;


/* YANG rpc-stmt struct; used for augment and name collision detect */
typedef struct obj_rpc_t_ {
    xmlChar           *name;
    xmlChar           *descr;
    xmlChar           *ref;
    ncx_status_t       status;
    dlq_hdr_t          typedefQ;         /* Q of typ_template_t */
    dlq_hdr_t          groupingQ;        /* Q of gtp_template_t */
    dlq_hdr_t          datadefQ;         /* Q of obj_template_t */

    /* internal fields for manager and agent */
    rpc_type_t      rpc_typ;
    xmlns_id_t      nsid;
    rpc_outtyp_t    out_datatyp;
    boolean          supported;    /* mod loaded, not implemented */    
} obj_rpc_t;


/* YANG augment statement struct */
typedef struct obj_augment_t_ {
    xmlChar          *target;
    xmlChar          *descr;
    xmlChar          *ref;
    struct obj_template_t_ *targobj;
    obj_augtype_t     augtype;
    ncx_status_t      status;
    dlq_hdr_t         datadefQ;         /* Q of obj_template_t */
} obj_augment_t;


/* One YANG 'notification' clause definition */
typedef struct obj_notif_t_ {
    xmlChar          *name;
    xmlChar          *descr;
    xmlChar          *ref;
    ncx_status_t      status;
    dlq_hdr_t         typedefQ;         /* Q of typ_template_t */
    dlq_hdr_t         groupingQ;        /* Q of gtp_template_t */
    dlq_hdr_t         datadefQ;          /* Q of obj_template_t */
} obj_notif_t;


/* One YANG data-def-stmt */
typedef struct obj_template_t_ {
    dlq_hdr_t      qhdr;
    obj_type_t     objtype;
    uint32         flags;              /* see OBJ_FL_* definitions */
    ncx_error_t    tkerr;
    grp_template_t *grp;          /* non-NULL == in a grp.datadefQ */

    /* 4 back pointers */
    struct obj_template_t_ *parent;
    struct obj_template_t_ *usesobj;
    struct obj_template_t_ *augobj;
    struct xpath_pcb_t_    *when;           /* optional when clause */
    dlq_hdr_t               metadataQ;       /* Q of obj_metadata_t */
    dlq_hdr_t               appinfoQ;         /* Q of ncx_appinfo_t */
    dlq_hdr_t               iffeatureQ;     /* Q of ncx_iffeature_t */

    /* cbset is agt_rpc_cbset_t for RPC or agt_cb_fnset_t for OBJ */
    void                   *cbset;   

    union def_ {
	obj_container_t   *container;
	obj_leaf_t        *leaf;
	obj_leaflist_t    *leaflist;
	obj_list_t        *list;
	obj_choice_t      *choic;
	obj_case_t        *cas;
	obj_uses_t        *uses;
	obj_refine_t      *refine;
	obj_augment_t     *augment;
	obj_rpc_t         *rpc;
	obj_rpcio_t       *rpcio;
	obj_notif_t       *notif;
    } def;

} obj_template_t;


/* One YANG metadata (XML attribute) node */
typedef struct obj_metadata_t_ {
    dlq_hdr_t      qhdr;
    struct obj_template_t_ *parent;     /* obj containing metadata */
    xmlChar       *name;
    typ_def_t     *typdef;
    xmlns_id_t     nsid;                 /* in case parent == NULL */
    ncx_error_t    tkerr;
} obj_metadata_t;


/* type of deviation for each deviate entry */
typedef enum obj_deviate_arg_t_ {
    OBJ_DARG_NONE,
    OBJ_DARG_ADD,
    OBJ_DARG_DELETE,
    OBJ_DARG_REPLACE,
    OBJ_DARG_NOT_SUPPORTED
}  obj_deviate_arg_t;


/* YANG deviate statement struct */
typedef struct obj_deviate_t_ {
    dlq_hdr_t          qhdr;

    /* the error info for each sub-clause is saved because
     * when the deviation-stmt is parsed, the target is not
     * known yet so picking the correct variant
     * such as type-stmt or refine-list-stmts
     * needs to wait until the resolve phase
     *
     */
    ncx_error_t       tkerr;
    boolean           empty;
    obj_deviate_arg_t arg;
    ncx_error_t       arg_tkerr;
    typ_def_t        *typdef;
    ncx_error_t       type_tkerr;
    xmlChar          *units;
    ncx_error_t       units_tkerr;
    xmlChar          *defval;
    ncx_error_t       default_tkerr;
    boolean           config;
    ncx_error_t       config_tkerr;
    boolean           mandatory;
    ncx_error_t       mandatory_tkerr;
    uint32            minelems;
    ncx_error_t       minelems_tkerr;   /* also minset */
    uint32            maxelems;
    ncx_error_t       maxelems_tkerr;   /* also maxset */
    dlq_hdr_t         mustQ;     /* Q of xpath_pcb_t */
    dlq_hdr_t         uniqueQ;  /* Q of obj_unique_t */
    dlq_hdr_t         appinfoQ;  /* Q of ncx_appinfo_t */
} obj_deviate_t;


/* YANG deviate statement struct */
typedef struct obj_deviation_t_ {
    dlq_hdr_t             qhdr;
    xmlChar              *target;
    xmlChar              *targmodname;
    obj_template_t       *targobj;
    xmlChar              *descr;
    xmlChar              *ref;
    ncx_error_t           tkerr;
    xmlChar              *devmodname;  /* set if not the targmod */
    boolean               empty;
    status_t              res;
    dlq_hdr_t             deviateQ;   /* Q of obj_deviate_t */
    dlq_hdr_t             appinfoQ;  /* Q of ncx_appinfo_t */
} obj_deviation_t;


/* child or descendant node search walker function
 *
 * INPUTS:
 *   obj == object node found in descendant search
 *   cookie1 == cookie1 value passed to start of walk
 *   cookie2 == cookie2 value passed to start of walk
 *
 * RETURNS:
 *   TRUE if walk should continue
 *   FALSE if walk should terminate 
 */
typedef boolean
    (*obj_walker_fn_t) (obj_template_t *obj,
			void *cookie1,
			void *cookie2);


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/******************  obj_template_t  **********************/

/* malloc and init an NCX database object template */
extern obj_template_t *
    obj_new_template (obj_type_t objtype);

/* free an NCX database object template */
extern void 
    obj_free_template (obj_template_t *obj);

extern obj_template_t *
    obj_find_template (dlq_hdr_t  *que,
		       const xmlChar *modname,
		       const xmlChar *objname);

extern const obj_template_t *
    obj_find_template_con (dlq_hdr_t  *que,
			   const xmlChar *modname,
			   const xmlChar *objname);

extern obj_template_t *
    obj_find_template_str (dlq_hdr_t  *que,
			   const xmlChar *modname,
			   const xmlChar *objname,
			   uint32 objnamelen);

extern obj_template_t *
    obj_find_template_test (dlq_hdr_t  *que,
			    const xmlChar *modname,
			    const xmlChar *objname);

extern obj_template_t *
    obj_find_template_top (ncx_module_t *mod,
			   const xmlChar *modname,
			   const xmlChar *objname);

extern obj_template_t *
    obj_find_child (obj_template_t  *obj,
		    const xmlChar *modname,
		    const xmlChar *objname);

extern obj_template_t *
    obj_find_child_str (obj_template_t  *obj,
			const xmlChar *modname,
			const xmlChar *objname,
			uint32 objnamelen);

extern obj_template_t *
    obj_match_child_str (obj_template_t *obj,
			 const xmlChar *modname,
			 const xmlChar *objname,
			 uint32 objnamelen,
			 uint32 *matchcount);

/* skips augment and uses */
extern obj_template_t *
    obj_first_child (obj_template_t *obj);

/* skips augment and uses */
extern obj_template_t *
    obj_last_child (obj_template_t *obj);

/* skips augment and uses */
extern obj_template_t *
    obj_next_child (obj_template_t *obj);

/* skips augment and uses */
extern obj_template_t *
    obj_previous_child (obj_template_t *obj);

/* skips augment and uses, dives into choice, case */
extern obj_template_t *
    obj_first_child_deep (obj_template_t *obj);

/* skips augment and uses, dives into choice, case */
extern obj_template_t *
    obj_next_child_deep (obj_template_t *obj);

extern boolean
    obj_find_all_children (ncx_module_t *exprmod,
			   obj_walker_fn_t walkerfn,
			   void *cookie1,
			   void *cookie2,
			   obj_template_t *startnode,
			   const xmlChar *modname,
			   const xmlChar *childname,
			   boolean configonly, 
			   boolean textmode,
			   boolean useroot);


extern boolean
    obj_find_all_ancestors (ncx_module_t *exprmod,
			    obj_walker_fn_t walkerfn,
			    void *cookie1,
			    void *cookie2,
			    obj_template_t *startnode,
			    const xmlChar *modname,
			    const xmlChar *name,
			    boolean configonly,
			    boolean textmode,
			    boolean useroot,
			    boolean orself,
			    boolean *fncalled);

extern boolean
    obj_find_all_descendants (ncx_module_t *exprmod,
			      obj_walker_fn_t walkerfn,
			      void *cookie1,
			      void *cookie2,
			      obj_template_t *startnode,
			      const xmlChar *modname,
			      const xmlChar *name,
			      boolean configonly,
			      boolean textmode,
			      boolean useroot,
			      boolean orself,
			      boolean *fncalled);


extern boolean
    obj_find_all_pfaxis (ncx_module_t *exprmod,
			 obj_walker_fn_t walkerfn,
			 void *cookie1,
			 void *cookie2,
			 obj_template_t *startnode,
			 const xmlChar *modname,
			 const xmlChar *name,
			 boolean configonly,
			 boolean dblslash,
			 boolean textmode,
			 boolean useroot,
			 ncx_xpath_axis_t axis,
			 boolean *fncalled);


extern obj_case_t *
    obj_find_case (obj_choice_t *choic,
		   const xmlChar *modname,
		   const xmlChar *casname);


extern obj_template_t * 
    obj_new_rpcio (obj_template_t *rpcobj,
		   const xmlChar *name);

/* clean Q of obj_template_t */
extern void
    obj_clean_datadefQ (dlq_hdr_t *que);

extern typ_template_t *
    obj_find_type (obj_template_t *obj,
		   const xmlChar *typname);

extern grp_template_t *
    obj_find_grouping (obj_template_t *obj,
		       const xmlChar *grpname);

extern status_t 
    obj_set_named_type (tk_chain_t *tkc,
			ncx_module_t *mod,
			const xmlChar *typname,
			typ_def_t *typdef,
			obj_template_t *parent,
			grp_template_t *grp);

extern obj_template_t *
    obj_clone_template (ncx_module_t *mod,
			obj_template_t *srcobj,
			dlq_hdr_t *mobjQ);

/* create an OBJ_TYP_CASE wrapper if needed,
 * for a short-case-stmt data def 
 */
extern obj_template_t *
    obj_clone_template_case (ncx_module_t *mod,
			     obj_template_t *srcobj,
			     dlq_hdr_t *mobjQ);


/********************    obj_unique_t   ********************/

/* malloc and init a unique list-node descriptor */
extern obj_unique_t *
    obj_new_unique (void);

extern void
    obj_init_unique (obj_unique_t *un);

/* free a unique list-node descriptor */
extern void
    obj_free_unique (obj_unique_t *un);

extern void
    obj_clean_unique (obj_unique_t *un);

extern obj_unique_comp_t *
    obj_new_unique_comp (void);

extern void
    obj_free_unique_comp (obj_unique_comp_t *unc);

extern obj_unique_t *
    obj_find_unique (dlq_hdr_t *que,
		     const xmlChar *xpath);

extern obj_unique_t *
    obj_first_unique (obj_template_t *listobj);

extern obj_unique_t *
    obj_next_unique (obj_unique_t *un);

extern obj_unique_comp_t *
    obj_first_unique_comp (obj_unique_t *un);

extern obj_unique_comp_t *
    obj_next_unique_comp (obj_unique_comp_t *uncomp);

/********************    obj_key_t   ********************/
extern obj_key_t *
    obj_new_key (void);

extern void
    obj_free_key (obj_key_t *key);

extern obj_key_t *
    obj_find_key (dlq_hdr_t *que,
		  const xmlChar *keycompname);

extern obj_key_t *
    obj_find_key2 (dlq_hdr_t *que,
		   obj_template_t *keyobj);

extern obj_key_t *
    obj_first_key (obj_template_t *obj);

extern const obj_key_t *
    obj_first_ckey (const obj_template_t *obj);

extern obj_key_t *
    obj_next_key (obj_key_t *objkey);

extern const obj_key_t *
    obj_next_ckey (const obj_key_t *objkey);

extern uint32
    obj_key_count (const obj_template_t *obj);

extern boolean
    obj_any_rpcs (const dlq_hdr_t *datadefQ);

extern boolean
    obj_any_notifs (const dlq_hdr_t *datadefQ);


/********************    obj_deviate_t   *******************/

extern obj_deviate_t *
    obj_new_deviate (void);

extern void
    obj_free_deviate (obj_deviate_t *deviate);



/********************    obj_deviation_t   *****************/

extern obj_deviation_t *
    obj_new_deviation (void);

extern void
    obj_free_deviation (obj_deviation_t *deviation);

extern void
    obj_clean_deviationQ (dlq_hdr_t *deviationQ);


/******************** OBJECT ID ************************/
/* malloc an object ID */
extern status_t
    obj_gen_object_id (const obj_template_t *obj,
		       xmlChar  **buff);

/* copy an object ID to a buffer */
extern status_t
    obj_copy_object_id (const obj_template_t *obj,
			xmlChar  *buff,
			uint32 bufflen,
			uint32 *reallen);

extern status_t
    obj_gen_aughook_id (const obj_template_t *obj,
			xmlChar  **buff);

/* set all the ncx.yang extension flags */
extern void
    obj_set_ncx_flags (obj_template_t *obj);

/***************** ACCESS OBJECT PROPERTIES  *******************/

extern const xmlChar * 
    obj_get_name (const obj_template_t *obj);

/* this function is used throughout the code to 
 * filter out uses and augment nodes from the
 * real nodes.  Those are the only YANG nodes that
 * do not have a name assigned to them
 */
extern boolean
    obj_has_name (const obj_template_t *obj);

extern boolean
    obj_has_text_content (const obj_template_t *obj);

extern ncx_status_t
    obj_get_status (const obj_template_t *obj);

extern const xmlChar *
    obj_get_description (const obj_template_t *obj);

extern const xmlChar *
    obj_get_reference (const obj_template_t *obj);

#define obj_is_config obj_get_config_flag

extern boolean
    obj_get_config_flag (const obj_template_t *obj);

extern boolean
    obj_get_config_flag2 (const obj_template_t *obj,
			  boolean *setflag);

extern ncx_access_t
    obj_get_max_access (const obj_template_t *obj);

extern dlq_hdr_t *
    obj_get_appinfoQ (obj_template_t *obj);

extern dlq_hdr_t *
    obj_get_appinfoQ2 (obj_template_t *obj);

extern dlq_hdr_t *
    obj_get_mustQ (const obj_template_t *obj);

extern const xmlChar *
    obj_get_typestr (const obj_template_t *obj);

extern dlq_hdr_t *
    obj_get_datadefQ (obj_template_t *obj);

extern const dlq_hdr_t *
    obj_get_cdatadefQ (const obj_template_t *obj);

extern const xmlChar *
    obj_get_default (const obj_template_t *obj);

extern obj_template_t *
    obj_get_default_case (obj_template_t *obj);

extern uint32
    obj_get_level (const obj_template_t *obj);

extern boolean
    obj_has_typedefs (const obj_template_t *obj);

extern typ_def_t *
    obj_get_typdef (obj_template_t  *obj);

extern const typ_def_t *
    obj_get_ctypdef (const obj_template_t  *obj);

extern ncx_btype_t
    obj_get_basetype (const obj_template_t  *obj);

extern const xmlChar *
    obj_get_mod_prefix (const obj_template_t *obj);

extern const xmlChar *
    obj_get_mod_xmlprefix (const obj_template_t  *obj);

extern const xmlChar *
    obj_get_mod_name (const obj_template_t  *obj);

extern const xmlChar *
    obj_get_type_name (const obj_template_t  *obj);

extern xmlns_id_t
    obj_get_nsid (const obj_template_t *);

extern ncx_iqual_t
    obj_get_iqualval (obj_template_t  *obj);

extern ncx_iqual_t
    obj_get_iqualval_ex (obj_template_t  *obj,
			 boolean required);

/* return TRUE if min is set */
extern boolean
    obj_get_min_elements (obj_template_t  *obj,
			  uint32 *minelems);

/* return TRUE if max is set */
extern boolean
    obj_get_max_elements (obj_template_t  *obj,
			  uint32 *maxelems);


extern const xmlChar *
    obj_get_units (obj_template_t  *obj);

extern obj_template_t *
    obj_get_parent (obj_template_t  *obj);

extern const obj_template_t *
    obj_get_cparent (const obj_template_t  *obj);

extern boolean
    obj_is_leafy (const obj_template_t  *obj);

extern boolean
    obj_is_mandatory (obj_template_t *obj);

extern boolean
    obj_is_cloned (const obj_template_t *obj);

extern boolean
    obj_is_augclone (const obj_template_t *obj);

extern boolean
    obj_is_refine (const obj_template_t *obj);

extern boolean
    obj_is_data (const obj_template_t *obj);

extern boolean
    obj_is_data_db (const obj_template_t *obj);

extern boolean
    obj_in_rpc (const obj_template_t *obj);

extern boolean
    obj_in_rpc_reply (const obj_template_t *obj);

extern boolean
    obj_in_notif (const obj_template_t *obj);

extern boolean
    obj_is_rpc (const obj_template_t *obj);

extern boolean
    obj_is_notif (const obj_template_t *obj);

extern boolean
    obj_is_empty (const obj_template_t *obj);

extern boolean
    obj_is_match (const obj_template_t  *obj1,
		  const obj_template_t *obj2);

extern boolean
    obj_is_hidden (const obj_template_t *obj);

extern boolean
    obj_is_root (const obj_template_t *obj);

extern boolean
    obj_is_password (const obj_template_t *obj);

extern boolean
    obj_is_xsdlist (const obj_template_t *obj);

extern boolean
    obj_is_cli (const obj_template_t *obj);

extern boolean
    obj_is_key (const obj_template_t *obj);

extern boolean
    obj_is_abstract (const obj_template_t *obj);

extern boolean
    obj_is_xpath_string (const obj_template_t *obj);

extern boolean
    obj_is_schema_instance_string (const obj_template_t *obj);

extern boolean
    obj_is_secure (const obj_template_t *obj);

extern boolean
    obj_is_very_secure (const obj_template_t *obj);

extern boolean
    obj_is_system_ordered (const obj_template_t *obj);

extern boolean
    obj_is_np_container (const obj_template_t *obj);

extern const xmlChar *
    obj_get_presence_string (const obj_template_t *obj);

extern boolean
    obj_ok_for_cli (obj_template_t *obj);

/* complex logic for finding the right module namespace
 * and child node, given the current context
 */
extern status_t 
    obj_get_child_node (obj_template_t *obj,
			obj_template_t *chobj,
			const xml_node_t *curnode,
			boolean xmlorder,
                        dlq_hdr_t *force_modQ,
			obj_template_t **rettop,
			obj_template_t **retobj);


extern uint32
    obj_get_child_count (const obj_template_t *obj);

extern boolean
    obj_has_children (obj_template_t *obj);

extern obj_metadata_t * 
    obj_new_metadata (void);

extern void 
    obj_free_metadata (obj_metadata_t *meta);


extern status_t
    obj_add_metadata (obj_metadata_t *meta,
		      obj_template_t *obj);

extern obj_metadata_t *
    obj_find_metadata (const obj_template_t *obj,
		       const xmlChar *name);

extern obj_metadata_t *
    obj_first_metadata (const obj_template_t *obj);

extern obj_metadata_t *
    obj_next_metadata (const obj_metadata_t *meta);

/* yangcli and ncx:cli support */
extern obj_template_t * 
    obj_get_default_parm (obj_template_t *obj);

/* get config flag during augment expand */
extern boolean
    obj_get_config_flag_deep (const obj_template_t *obj);

extern uint8
    obj_get_fraction_digits (const obj_template_t  *obj);

extern const ncx_iffeature_t *
    obj_get_first_iffeature (const obj_template_t *obj);

extern const ncx_iffeature_t *
    obj_get_next_iffeature (const ncx_iffeature_t *iffeature);

extern boolean
    obj_is_enabled (const obj_template_t *obj);

extern boolean
    obj_is_single_instance (obj_template_t *obj);


#endif	    /* _H_obj */
