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

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
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

/* object is conditional, expanded via an augment clause
 * the obj_when_t clause is non-empty 
 */
#define OBJ_FL_CONDITIONAL  bit4

/* object is a top-level definition within a module or submodule */
#define OBJ_FL_TOP          bit5

/* object was entered with a 'kw name;' format and is 
 * considered empty by the yangdump program
 */
#define OBJ_FL_EMPTY        bit6

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
    OBJ_TYP_CONTAINER,
    OBJ_TYP_LEAF,
    OBJ_TYP_LEAF_LIST,
    OBJ_TYP_LIST,       /* last real database object */
    OBJ_TYP_CHOICE,
    OBJ_TYP_CASE,       /* last named database object */
    OBJ_TYP_USES,
    OBJ_TYP_AUGMENT,
    OBJ_TYP_RPC,
    OBJ_TYP_RPCIO,
    OBJ_TYP_NOTIF
} obj_type_t;

/* enumeration for different Queue types that hold obj_template_t structs */
typedef enum obj_qtype_t_ {
    OBJ_QUE_NONE,
    OBJ_QUE_DATA,                    /* data node datadefQ */
    OBJ_QUE_GRP,                 /* grouping node datadefQ */
    OBJ_QUE_USES,                    /* uses node datadefQ */
    OBJ_QUE_AUG,                  /* augment node datadefQ */
    OBJ_QUE_RPC,                           /* RPC datadefQ */
    OBJ_QUE_RPCIO,         /* RPC input or output datadefQ */
    OBJ_QUE_NOTIF                 /* notification datadefQ */
} obj_qtype_t;


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
    tk_token_t     *tk;
    xmlChar        *xpath;       /* complete saved unique str */
    uint32          linenum;
    dlq_hdr_t       compQ;          /* Q of obj_unique_comp_t */
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
    boolean        confset;
    boolean        config;
    dlq_hdr_t      mustQ;           /* Q of ncx_errinfo_t */
    dlq_hdr_t      appinfoQ;        /* Q of ncx_appinfo_t */
} obj_container_t;


/* One YANG 'leaf' definition */
typedef struct obj_leaf_t_ {
    xmlChar       *name;
    xmlChar       *units;
    xmlChar       *defval;
    xmlChar       *descr;
    xmlChar       *ref;
    typ_def_t     *typdef;
    boolean        confset;
    boolean        config;
    boolean        mandset;
    boolean        mandatory;
    ncx_status_t   status;
    dlq_hdr_t      mustQ;            /* Q of ncx_errinfo_t */
    dlq_hdr_t      appinfoQ;         /* Q of ncx_appinfo_t */
} obj_leaf_t;


/* One YANG 'leaf-list' definition */
typedef struct obj_leaflist_t_ {
    xmlChar       *name;
    xmlChar       *units;
    xmlChar       *descr;
    xmlChar       *ref;
    typ_def_t     *typdef;
    boolean        ordersys; /* ordered-by system or user */
    boolean        confset;
    boolean        config;
    boolean        minset;
    uint32         minelems;
    boolean        maxset;
    uint32         maxelems;
    ncx_status_t   status;
    dlq_hdr_t      mustQ;            /* Q of ncx_errinfo_t */
    dlq_hdr_t      appinfoQ;         /* Q of ncx_appinfo_t */
} obj_leaflist_t;


/* One YANG 'list' definition */
typedef struct obj_list_t_ {
    xmlChar       *name;
    xmlChar       *keystr;
    tk_token_t    *keytk;
    xmlChar       *descr;
    xmlChar       *ref;
    dlq_hdr_t     *typedefQ;         /* Q of typ_template_t */
    dlq_hdr_t     *groupingQ;       /* Q of grp_template_t */
    dlq_hdr_t     *keyQ;                 /* Q of obj_key_t */
    dlq_hdr_t     *uniqueQ;           /* Q of obj_unique_t */
    dlq_hdr_t     *datadefQ;        /* Q of obj_template_t */
    boolean        datadefclone;
    boolean        ordersys;   /* ordered-by system or user */
    boolean        confset;
    boolean        config;
    boolean        minset;
    uint32         minelems;
    boolean        maxset;
    uint32         maxelems;
    uint32         keylinenum;
    ncx_status_t   status;
    dlq_hdr_t      mustQ;            /* Q of ncx_errinfo_t */
    dlq_hdr_t      appinfoQ;         /* Q of ncx_appinfo_t */
} obj_list_t;


/* One YANG 'choice' definition */
typedef struct obj_choice_t_ {
    xmlChar       *name;
    xmlChar       *defval;
    xmlChar       *descr;
    xmlChar       *ref;
    dlq_hdr_t     *caseQ;             /* Q of obj_template_t */
    boolean        caseQclone;
    boolean        confset;
    boolean        config;
    boolean        mandset;
    boolean        mandatory;
    ncx_status_t   status;
    dlq_hdr_t      appinfoQ;           /* Q of ncx_appinfo_t */
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
    dlq_hdr_t       appinfoQ;          /* Q of ncx_appinfo_t */
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
    dlq_hdr_t         appinfoQ;          /* Q of ncx_appinfo_t */
} obj_uses_t;


/* YANG input-stmt or output-stmt struct */
typedef struct obj_rpcio_t_ {
    xmlChar           *name;                 /* input or output */
    dlq_hdr_t          typedefQ;         /* Q of typ_template_t */
    dlq_hdr_t          groupingQ;        /* Q of gtp_template_t */
    dlq_hdr_t          datadefQ;         /* Q of obj_template_t */
    dlq_hdr_t          appinfoQ;          /* Q of ncx_appinfo_t */
} obj_rpcio_t;


/* YANG rpc-stmt struct; used for augment and name collision detect */
typedef struct obj_rpc_t_ {
    xmlChar           *name;
    xmlChar           *descr;
    xmlChar           *ref;
    void              *rpc;          /* back-ptr rpc_template_t */
    ncx_status_t       status;
    dlq_hdr_t          typedefQ;         /* Q of typ_template_t */
    dlq_hdr_t          groupingQ;        /* Q of gtp_template_t */
    dlq_hdr_t          datadefQ;         /* Q of obj_template_t */
    dlq_hdr_t          appinfoQ;          /* Q of ncx_appinfo_t */
} obj_rpc_t;


/* One concatenated YANG 'when' clause' definition */
typedef struct obj_when_t_ {
    xmlChar       *xpath;
} obj_when_t;


/* YANG augment statement struct */
typedef struct obj_augment_t_ {
    xmlChar          *target;
    xmlChar          *descr;
    xmlChar          *ref;
    struct obj_template_t_ *targobj;
    obj_when_t        when;
    obj_augtype_t     augtype;
    ncx_status_t      status;
    dlq_hdr_t         datadefQ;         /* Q of obj_template_t */
    dlq_hdr_t         appinfoQ;          /* Q of ncx_appinfo_t */
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
    dlq_hdr_t         appinfoQ;          /* Q of ncx_appinfo_t */
} obj_notif_t;


/* One YANG data-def-stmt */
typedef struct obj_template_t_ {
    dlq_hdr_t      qhdr;
    obj_type_t     objtype;
    obj_qtype_t    qtype;
    uint32         flags;
    uint32         linenum;
    xmlns_id_t     nsid;
    ncx_module_t  *mod;         /* mod or submod containing obj */
    tk_token_t    *tk;          /* tk valid only during parsing */
    grp_template_t *grp;        /* non-NULL == in a grp.datadefQ */
    struct obj_template_t_ *parent;
    struct obj_template_t_ *usesobj;
    const obj_when_t  *augwhen;   /* augment when clause backptr */
    union def_ {
	obj_container_t   *container;
	obj_leaf_t        *leaf;
	obj_leaflist_t    *leaflist;
	obj_list_t        *list;
	obj_choice_t      *choic;
	obj_case_t        *cas;
	obj_uses_t        *uses;
	obj_augment_t     *augment;
	obj_rpc_t         *rpc;
	obj_rpcio_t       *rpcio;
	obj_notif_t       *notif;
    } def;

} obj_template_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/****************** ALLOCATION FUNCTIONS **********************/

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
    obj_find_template_con (const dlq_hdr_t  *que,
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

extern const xmlChar * 
    obj_get_name (const obj_template_t *obj);

extern boolean
    obj_has_name (const obj_template_t *obj);

extern ncx_status_t
    obj_get_status (const obj_template_t *obj);

extern const xmlChar *
    obj_get_description (const obj_template_t *obj);

extern const xmlChar *
    obj_get_reference (const obj_template_t *obj);

extern boolean
    obj_get_config_flag (const obj_template_t *obj);

extern boolean
    obj_get_config_flag2 (const obj_template_t *obj,
			  boolean *setflag);

extern dlq_hdr_t *
    obj_get_appinfoQ (const obj_template_t *obj);

extern dlq_hdr_t *
    obj_get_mustQ (const obj_template_t *obj);

extern const xmlChar *
    obj_get_typestr (const obj_template_t *obj);


extern typ_template_t *
    obj_find_type (const obj_template_t *obj,
		   const xmlChar *typname);

extern grp_template_t *
    obj_find_grouping (const obj_template_t *obj,
		       const xmlChar *grpname);

extern status_t 
    obj_set_named_type (tk_chain_t *tkc,
			ncx_module_t *mod,
			const xmlChar *typname,
			typ_def_t *typdef,
			obj_template_t *parent,
			grp_template_t *grp);

extern boolean
    obj_is_required (const obj_template_t *obj);

extern boolean
    obj_is_cloned (const obj_template_t *obj);

extern boolean
    obj_is_augclone (const obj_template_t *obj);

extern boolean
    obj_is_refine (const obj_template_t *obj);

extern obj_template_t *
    obj_clone_template (ncx_module_t *mod,
			obj_template_t *srcobj,
			obj_template_t *mobj);

/* create an OBJ_TYP_CASE wrapper if needed,
 * for a short-case-stmt data def 
 */
extern obj_template_t *
    obj_clone_template_case (ncx_module_t *mod,
			     obj_template_t *srcobj,
			     obj_template_t *mobj);

extern dlq_hdr_t *
    obj_get_datadefQ (obj_template_t *obj);

extern const dlq_hdr_t *
    obj_get_cdatadefQ (const obj_template_t *obj);

extern void
    obj_dump_datadefQ (const dlq_hdr_t *datadefQ);

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

extern obj_key_t *
    obj_new_key (void);

extern void
    obj_free_key (obj_key_t *key);

extern obj_key_t *
    obj_find_key (dlq_hdr_t *que,
		  const xmlChar *keycompname);

extern obj_key_t *
    obj_find_key2 (dlq_hdr_t *que,
		   const obj_template_t *keyobj);

extern boolean
    obj_any_notifs (dlq_hdr_t *datadefQ);

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

extern uint32
    obj_get_object_id_len (const obj_template_t *obj);

extern status_t
    obj_gen_aughook_id (const obj_template_t *obj,
			xmlChar  **buff);

extern boolean
    obj_is_data (const obj_template_t *obj);

extern boolean
    obj_is_data_db (const obj_template_t *obj);

extern const xmlChar *
    obj_get_defval (const obj_template_t *obj);

extern uint32
    obj_get_level (const obj_template_t *obj);

extern boolean
    obj_has_typedefs (const obj_template_t *obj);

extern boolean
    obj_is_empty (const obj_template_t *obj);

#endif	    /* _H_obj */
