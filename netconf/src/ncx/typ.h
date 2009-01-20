#ifndef _H_typ
#define _H_typ

/*  FILE: typ.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Parameter Type Handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
22-oct-05    abb      Begun
13-oct-08    abb      Moved pattern from typ_sval_t to ncx_pattern_t 
                      to support N patterns per typdef
*/

#include <xmlstring.h>

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

#ifndef _H_xmlns
#include "xmlns.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* typ_rangedef_t flags field */
#define TYP_FL_LBINF     bit0         /* lower bound = -INF */
#define TYP_FL_LBINF2    bit1         /* lower bound = INF */
#define TYP_FL_UBINF     bit2         /* upper bound = INF */
#define TYP_FL_UBINF2    bit3         /* upper bound = -INF */
#define TYP_FL_LBMIN     bit4         /* lower bound is set to 'min' */
#define TYP_FL_LBMAX     bit5         /* lower bound is set to 'max' */
#define TYP_FL_UBMAX     bit6         /* upper bound is set to 'max' */
#define TYP_FL_UBMIN     bit7         /* upper bound is set to 'min' */

#define TYP_RANGE_FLAGS  0xff

/* typ_enum_t flags field */
#define TYP_FL_ESET      bit0         /* value explicitly set */
#define TYP_FL_ISBITS    bit1         /* enum really used in bits */
#define TYP_FL_SEEN      bit2         /* used by yangdiff */

/* typ_sval_t flags field */
#define TYP_FL_USTRING   bit0         /* value is ustring, not string */

/* typ_named_t flags field */
#define TYP_FL_REPLACE   bit0         /* Replace if set; extend if not */



/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* type parser used in 3 separate modes */
typedef enum typ_pmode_t_ {
    TYP_PM_NONE,
    TYP_PM_NORMAL,    /* normal parse mode */
    TYP_PM_INDEX,     /* index clause parse mode */
    TYP_PM_MDATA      /* metadata clause parse mode */
} typ_pmode_t;


/* one list member
 * stored in simple.queue of instance-qualified strings 
 */
typedef struct typ_listval_t_ {
    dlq_hdr_t     qhdr;
    dlq_hdr_t     strQ;                        /* Q of typ_sval_t */
    ncx_iqual_t  iqual;   /* instance qualifier for this listval */
} typ_listval_t;


/* one member of a range definition -- stored in simple.rangeQ 
 *
 * range components in YANG are not allowed to appear out of order
 * and they are not allowed to overlap.
 *
 * A single number component is represented
 * as a range where lower bound == upper bound
 *
 *  (1 | 5 | 10) saved as (1..1 | 5..5 | 10..10)
 *
 * symbolic range terms are stored in flags, and processed
 * in the 2nd pass of the compiler:
 *
 * lower bound:
 *    min   TYP_FL_LBMIN
 *    max   TYP_FL_LBMAX
 *   -INF   TYP_FL_LBINF
 *    INF   TYP_FL_LBINF2
 *
 * upper bound:
 *    min   TYP_FL_UBMIN
 *    max   TYP_FL_UBMAX
 *    INF   TYP_FL_UBINF
 *   -INF   TYP_FL_UBINF2
 *
 * If range parsed with btyp == NCX_BT_NONE, then lbstr
 * and ubstr will be used (for numbers).  Otherwise
 * the number bounds of the range part are stored in lb and ub
 *
 * The entire range string is saved in rangestr for ncxdump
 * and yangcli help text
 *
 * The token associated with the range part is saved for
 * error messages in YANG phase 2 compiling
 */
typedef struct typ_rangedef_t_ {
    dlq_hdr_t   qhdr;
    xmlChar    *rangestr;        /* saved in YANG only */
    ncx_num_t   lb;              /* lower bound */
    ncx_num_t   ub;              
    ncx_btype_t btyp;
    uint32      flags;
    tk_token_t *tk;              /* saved in YANG only */
    xmlChar    *lbstr;           /* saved if range deferred */
    xmlChar    *ubstr;           /* saved if range deferred */
} typ_rangedef_t;


/* one ENUM typdef value -- stored in simple.valQ
 * Used for NCX_BT_ENUM and NCX_BT_BITS data type
 */
typedef struct typ_enum_t_ {
    dlq_hdr_t     qhdr;
    xmlChar      *name;
    xmlChar      *descr;
    xmlChar      *ref;
    ncx_status_t  status;
    int32         val;
    uint32        pos;
    uint32        order;
    uint32        flags;
    dlq_hdr_t     appinfoQ;
} typ_enum_t;


/* one STRING typdef value, pattern value 
 *  -- stored in simple.valQ 
 */
typedef struct typ_sval_t_ {
    dlq_hdr_t  qhdr;
    ncx_str_t val;
    uint32    flags;
} typ_sval_t;

/* one range description */
typedef struct typ_range_t_ {
    xmlChar         *rangestr;
    tk_token_t      *tk;
    dlq_hdr_t        rangeQ;            /* Q of typ_rangedef_t */
    ncx_errinfo_t    range_errinfo;
    ncx_status_t     res;
} typ_range_t;

/* YANG pattern struct : N per typedef and also
 * across N typdefs in a chain: all are ANDed together like RelaxNG
 * instead of ORed together within the same type step like XSD
 */
typedef struct typ_pattern_t_ {
    dlq_hdr_t       qhdr;
    xmlRegexpPtr    pattern;
    xmlChar        *pat_str;
    ncx_errinfo_t   pat_errinfo;
    ncx_status_t    res;
} typ_pattern_t;


/* YANG identityref struct
 * the value is an identity-stmt QName
 * that has a base-stmt that resolves to the same value
 */
typedef struct typ_idref_t {
    xmlChar        *baseprefix;
    xmlChar        *basename;
    ncx_identity_t *base;     /* back-ptr to base (if found ) */
    const xmlChar  *modname;   /* back-ptr to the main mod name */
} typ_idref_t;

/* NCX_CL_SIMPLE
 *
 * The following enums defined in ncxconst.h are supported in this struct
 *  NCX_BT_BITS -- set of bit definitions (like list of enum)
 *  NCX_BT_BOOLEAN -- true, 1, false, 0
 *  NCX_BT_ENUM -- XSD like enumeration
 *  NCX_BT_EMPTY -- empty element type like <ok/>
 *  NCX_BT_INT8 -- 8 bit signed integer value
 *  NCX_BT_INT16 -- 16 bit signed integer value
 *  NCX_BT_INT32 -- 32 bit signed integer value
 *  NCX_BT_INT64 -- 64 bit signed integer value
 *  NCX_BT_UINT8 -- 8 bit unsigned integer value
 *  NCX_BT_UINT16 -- 16 bit unsigned integer value
 *  NCX_BT_UINT32 -- 32 bit unsigned integer value
 *  NCX_BT_UINT64 -- 64 bit unsigned integer value
 *  NCX_BT_FLOAT32 -- 32 bit float number value
 *  NCX_BT_FLOAT64 -- 64 bit float number value
 *  NCX_BT_STRING -- char string
 *  NCX_BT_BINARY -- binary string (base64 from RFC 4648)
 *  NCX_BT_KEYREF -- YANG keyref (XPath expression)
 *  NCX_BT_IDREF -- YANG identityref (QName)
 *  NCX_BT_INSTANCE_ID -- YANG instance-identifier (XPath expression)
 *  NCX_BT_SLIST -- simple list of string or number type (xsd:list)
 *  NCX_BT_UNION -- C-type union of any simtype except keyref and empty
 */
typedef struct typ_simple_t_ {
    ncx_btype_t      btyp;                             /* NCX base type */
    struct typ_template_t_ *listtyp;       /* template for NCX_BT_SLIST */
    struct xpath_pcb_t_   *xkeyref;     /* saved for NCX_BT_KEYREF only */
    typ_range_t      range;     /* for all num types and string length  */
    typ_idref_t      idref;                    /* for NCX_BT_IDREF only */
    dlq_hdr_t        valQ;     /* bit, enum, string, list vals/patterns */
    dlq_hdr_t        metaQ;              /* Q of obj_template_t structs */
    dlq_hdr_t        unionQ;   /* Q of typ_unionnode_t for NCX_BT_UNION */
    dlq_hdr_t        patternQ;  /* Q of ncx_pattern_t for NCX_BT_STRING */
    ncx_strrest_t    strrest;   /* string/type restriction type in valQ */
    uint32           flags;
} typ_simple_t;


/* NCX_CL_NAMED
 *
 * typ_named_t
 *      - user defined type using another named type (not a base type)
 *        and therefore pointing to another typ_template (typ)
 *      - also used for derived type from another named type or
 *        extending the parent type in some way (newtyp)
 */
typedef struct typ_named_t_ {
    struct typ_template_t_ *typ;               /* derived-from type */
    struct typ_def_t_      *newtyp;    /* opt. additional semantics */
    uint32                  flags;
} typ_named_t;


/* NCX_CL_REF
 *
 * typ_ref_t
 *  struct pointing to another typ_def_t
 *  used internally within inline data types
 *  (child nodes or scoped index nodes of complex types)
 */
typedef struct typ_ref_t_ {
    struct typ_def_t_ *typdef;                 /* fwd pointer */
} typ_ref_t;


/* Union of all the typdef variants */
typedef union typ_def_u_t_ {
    ncx_btype_t     base;
    typ_simple_t    simple;
    typ_named_t     named;
    typ_ref_t       ref;
} typ_def_u_t;


/* Discriminated union for all data typedefs.
 * This data structure is repeated in every
 * node of every data structure, starting with
 * the typ_def_t struct in the type template
 */
typedef struct typ_def_t_ {
    ncx_tclass_t     class;
    ncx_iqual_t      iqual;
    ncx_access_t     maxaccess;
    ncx_data_class_t dataclass;
    ncx_merge_t      mergetype;
    xmlChar         *prefix;            /* pfix used in type field */
    xmlChar         *typename;      /* typename used in type field */
    uint32           linenum;         /* linenum when NCX_CL_NAMED */
    tk_token_t      *tk;            /* const back-ptr for errmsg */
    dlq_hdr_t        appinfoQ;             /* Q of ncx_appinfo_t */
    typ_def_u_t      def;
} typ_def_t;


/* One YANG 'type' definition -- top-level type template */
typedef struct typ_template_t_ {
    dlq_hdr_t    qhdr;
    xmlChar     *name;
    xmlChar     *descr;
    xmlChar     *ref;
    xmlChar     *defval;
    xmlChar     *units;
    ncx_module_t *mod;   /* const back-ptr to module defining type */
    xmlns_id_t   nsid;
    boolean      used;
    ncx_status_t status;
    typ_def_t    typdef;
    dlq_hdr_t    appinfoQ;
    uint32       linenum;
    tk_token_t  *tk;
    void        *grp;       /* const back-ptr to direct grp parent */
    status_t     res;
} typ_template_t;


/* One YANG union node 
 * One of the 2 pointers (typ or typdef will be NULL
 * If a named type is used, then 'typ' is active
 * If an inline type is used, then typdef is active
 */
typedef struct typ_unionnode_t_ {
    dlq_hdr_t     qhdr;
    typ_template_t *typ;      /* not malloced, just back-ptr */
    typ_def_t      *typdef;   /* malloced for unnamed inline type */
    boolean         seen;     /* needed for yangdiff */
} typ_unionnode_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/***************** INITIALIZATION FUNCTIONS *******************/

/* load the typ_template_t structs for the ncx_btype_t types
 * MUST be called during ncx_init startup
 */
extern status_t
    typ_load_basetypes (void);

/* unload the typ_template_t structs for the ncx_btype_t types
 * SHOULD be called during ncx_init shutdown
 */
extern void
    typ_unload_basetypes (void);


/****************** ALLOCATION FUNCTIONS **********************/

/* malloc and init an NCX type template */
extern typ_template_t *
    typ_new_template (void);

/* free an NCX type template */
extern void 
    typ_free_template (typ_template_t *typ);

/* malloc and init a typdef */
extern typ_def_t *
    typ_new_typdef (void);

/* init a pre-allocated typdef (done first) */
extern void 
    typ_init_typdef (typ_def_t *typdef);


/* init a simple data type after typ_init_typdef */
extern void
    typ_init_simple (typ_def_t  *tdef, 
		     ncx_btype_t btyp);

/* init a named data type after typ_init_typdef */
extern void
    typ_init_named (typ_def_t  *tdef);

/* free a typdef */
extern void 
    typ_free_typdef (typ_def_t *typdef);

/* clean a typdef but do not free it */
extern void
    typ_clean_typdef (typ_def_t  *typdef);

extern void
    typ_set_named_typdef (typ_def_t *typdef,
			  typ_template_t *imptyp);

extern const xmlChar *
    typ_get_named_typename (const typ_def_t  *typdef);

extern status_t
    typ_set_new_named (typ_def_t  *typdef, 
		       ncx_btype_t btyp);

/* access named.newtyp */
extern typ_def_t *
    typ_get_new_named (typ_def_t  *typdef);

extern const typ_def_t *
    typ_cget_new_named (const typ_def_t  *typdef);

extern void
    typ_set_simple_typdef (typ_template_t *typ,
			   ncx_btype_t btyp);

/* malloc and init an enumeration descriptor, strdup name ptr */
extern typ_enum_t *
    typ_new_enum (const xmlChar *name);

/* malloc and init an enumeration descriptor, pass off name ptr */
extern typ_enum_t *
    typ_new_enum2 (xmlChar *name);

/* free an enumeration descriptor */
extern void
    typ_free_enum (typ_enum_t *en);

/* malloc and init a range definition */
extern typ_rangedef_t *
    typ_new_rangedef (void);

/* free a range definition */
extern void
    typ_free_rangedef (typ_rangedef_t *rv, 
		       ncx_btype_t  btyp);

/* concat consecutive rangedef sections for integral numbers */
extern void
    typ_normalize_rangeQ (dlq_hdr_t *rangeQ,
			  ncx_btype_t  btyp);

extern dlq_hdr_t *
    typ_get_rangeQ (typ_def_t *typdef);

extern dlq_hdr_t *
    typ_get_rangeQ_con (typ_def_t *typdef);

extern const dlq_hdr_t *
    typ_get_crangeQ (const typ_def_t *typdef);

extern const dlq_hdr_t *
    typ_get_crangeQ_con (const typ_def_t *typdef);

extern typ_range_t *
    typ_get_range_con (typ_def_t *typdef);

extern const typ_range_t *
    typ_get_crange_con (const typ_def_t *typdef);

extern const xmlChar *
    typ_get_rangestr (const typ_def_t *typdef);

extern const typ_rangedef_t *
    typ_first_rangedef (const typ_def_t *typdef);

extern const typ_rangedef_t *
    typ_first_rangedef_con (const typ_def_t *typdef);


/* deprecated -- does not support multi-part ranges */
extern status_t
    typ_get_rangebounds_con (const typ_def_t *typdef,
			     ncx_btype_t *btyp,
			     const ncx_num_t **lb,
			     const ncx_num_t **ub);


/* get typdef string restriction type */
extern ncx_strrest_t 
    typ_get_strrest (const typ_def_t *typdef);

/* set typdef string restriction type */
extern void
    typ_set_strrest (typ_def_t *typdef,
		     ncx_strrest_t strrest);

/* malloc and init a string descriptor */
extern typ_sval_t *
    typ_new_sval (const xmlChar *str,
		  ncx_btype_t  btyp);

/* free a string descriptor */
extern void
    typ_free_sval (typ_sval_t *sv);

/* malloc and init a list descriptor */
extern typ_listval_t *
    typ_new_listval (void);

/* free a list descriptor */
extern void
    typ_free_listval (typ_listval_t *lv);

/***************** ACCESS FUNCTIONS *****************/

/* get the proper range base type to use for a given base type */
extern ncx_btype_t 
    typ_get_range_type (ncx_btype_t btyp);

/* Follow any typdef links and get the actual base type of 
 * the specified typedef 
 */
extern ncx_btype_t
    typ_get_basetype (const typ_def_t  *typdef);

extern const xmlChar *
    typ_get_name (const typ_def_t  *typdef);

extern const xmlChar *
    typ_get_basetype_name (const typ_template_t  *typ);

extern const xmlChar *
    typ_get_parenttype_name (const typ_template_t  *typ);

/* Follow any typdef links and get the class of the base typdef
 * for the specified typedef 
 */
extern ncx_tclass_t
    typ_get_base_class (const typ_def_t  *typdef);

/* Get the default type template for the specified base type */
extern typ_template_t *
    typ_get_basetype_typ (ncx_btype_t  btyp);

/* Get the default typdef for the specified base type */
extern typ_def_t *
    typ_get_basetype_typdef (ncx_btype_t  btyp);


/* get the real typdef that describes the type, if the
 * input is one of the 'pointer' typdef classes. Otherwise,
 * just return the input typdef
 */
extern typ_def_t *
    typ_get_base_typdef (typ_def_t  *typdef);

extern const typ_def_t *
    typ_get_cbase_typdef (const typ_def_t  *typdef);

/* Get the parent typdef for NCX_CL_NAMED and NCX_CL_REF
 * Returns NULL for all other classes
 */
extern typ_def_t *
    typ_get_parent_typdef (typ_def_t  *typdef);

extern const typ_template_t *
    typ_get_parent_type (const typ_template_t  *typ);

extern const typ_def_t *
    typ_get_cparent_typdef (const typ_def_t  *typdef);


/* Get the next typdef in the chain for NCX_CL_NAMED or NCX_CL_REF
 * Returns the input typdef for all other typdef classes
 */
extern typ_def_t *
    typ_get_next_typdef (typ_def_t  *typdef);


/* Get the next typdef in the chain for NCX_CL_NAMED or NCX_CL_REF
 * Skip any named types without the specific restriction defined
 *
 * Returns the input typdef for simple typdef classes
 */
extern typ_def_t *
    typ_get_qual_typdef (typ_def_t  *typdef,
			 ncx_squal_t squal);

extern const typ_def_t *
    typ_get_cqual_typdef (const typ_def_t  *typdef,
			  ncx_squal_t  squal);

/* find a specified appinfo variable */
extern const ncx_appinfo_t *
    typ_find_appinfo (const typ_def_t *typdef,
		      const xmlChar *prefix,
		      const xmlChar *name);

/* find a specified appinfo variable constrined to the typdef */
extern const ncx_appinfo_t *
    typ_find_appinfo_con (const typ_def_t *typdef,
			  const xmlChar *prefix,
			  const xmlChar *name);

extern boolean
    typ_is_xpath_string (const typ_def_t *typdef);


/* get default from template */
extern const xmlChar * 
    typ_get_defval (const typ_template_t *typ);

/* get default from typdef */
extern const xmlChar *
    typ_get_default (const typ_def_t *typdef);

extern ncx_iqual_t 
    typ_get_iqualval (const typ_template_t *typ);

extern ncx_iqual_t
    typ_get_iqualval_def (const typ_def_t *typdef);

extern const xmlChar * 
    typ_get_units (const typ_template_t *typ);

/* get units from named type if any */
extern const xmlChar * 
    typ_get_units_from_typdef (const typ_def_t *typdef);

extern boolean
    typ_has_children (ncx_btype_t btyp);

extern boolean
    typ_has_index (ncx_btype_t btyp);

extern boolean
    typ_is_simple (ncx_btype_t btyp);

extern boolean
    typ_is_xsd_simple (ncx_btype_t btyp);

extern const typ_enum_t *
    typ_first_enumdef (const typ_def_t *typdef);

extern const typ_enum_t *
    typ_next_enumdef (const typ_enum_t *enumdef);

extern typ_enum_t *
    typ_first_enumdef2 (typ_def_t *typdef);

extern const typ_enum_t *
    typ_first_con_enumdef (const typ_def_t *typdef);

extern typ_enum_t *
    typ_find_enumdef (dlq_hdr_t *ebQ,
		      const xmlChar *name);

extern uint32
    typ_enumdef_count (const typ_def_t *typdef);

extern const typ_sval_t *
    typ_first_strdef (const typ_def_t *typdef);

extern uint32
    typ_get_maxrows (const typ_def_t *typdef);

extern ncx_access_t
    typ_get_maxaccess (const typ_def_t *typdef);

extern ncx_data_class_t
    typ_get_dataclass (const typ_def_t *typdef);

extern ncx_merge_t
    typ_get_mergetype (const typ_def_t *typdef);

extern xmlns_id_t 
    typ_get_nsid (const typ_template_t *typ);

extern const typ_template_t *
    typ_get_listtyp (const typ_def_t *typdef);

extern const typ_template_t *
    typ_get_clisttyp (const typ_def_t *typdef);

extern typ_unionnode_t *
    typ_new_unionnode (typ_template_t *typ);

extern void
    typ_free_unionnode (typ_unionnode_t *un);

extern typ_def_t *
    typ_get_unionnode_ptr (typ_unionnode_t *un);

extern const typ_unionnode_t *
    typ_first_unionnode (const typ_def_t *typdef);

extern boolean
    typ_is_number (ncx_btype_t btyp);

extern boolean
    typ_is_string (ncx_btype_t btyp);

extern typ_pattern_t *
    typ_new_pattern (const xmlChar *pat_str);

extern void
    typ_free_pattern (typ_pattern_t *pat);

extern status_t
    typ_compile_pattern (typ_pattern_t *pat);

extern typ_pattern_t *
    typ_get_first_pattern (typ_def_t *typdef);

extern typ_pattern_t *
    typ_get_next_pattern (typ_pattern_t *curpat);

extern const typ_pattern_t *
    typ_get_first_cpattern (const typ_def_t *typdef);

extern const typ_pattern_t *
    typ_get_next_cpattern (const typ_pattern_t *curpat);

extern uint32
    typ_get_pattern_count (const typ_def_t *typdef);

extern const ncx_errinfo_t *
    typ_get_range_errinfo (const typ_def_t *typdef);

/* clean Q of typ_template_t */
extern void
    typ_clean_typeQ (dlq_hdr_t *que);

/* clean Q of typ_index_t
 * extern void
 *    typ_clean_indexQ (dlq_hdr_t *que);
 */

extern boolean
    typ_ok_for_inline_index (ncx_btype_t btyp);

extern boolean
    typ_ok_for_metadata (ncx_btype_t btyp);

extern boolean
    typ_ok_for_index (const typ_def_t  *typdef);

extern boolean
    typ_ok_for_union (ncx_btype_t btyp);

extern boolean
    typ_ok_for_xsdlist (ncx_btype_t btyp);

/* check result in typdef chain */
extern boolean
    typ_ok (const typ_def_t *typdef);

extern const xmlChar *
    typ_get_keyref_path (const typ_def_t *typdef);

/* returns xpath_pcb_t but cannot import due to H file loop */
extern void *
    typ_get_keyref_pcb (typ_def_t *typdef);

extern boolean
    typ_has_subclauses (const typ_def_t *typdef);

extern const typ_idref_t *
    typ_get_idref (const typ_def_t  *typdef);

#endif	    /* _H_typ */
