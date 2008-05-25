#ifndef _H_val
#define _H_val

/*  FILE: val.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Parameter Value Handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
19-dec-05    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_status
#include "status.h"
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


#define VAL_MAX_NUMLEN  48

/* constants used in generating C and Xpath instance ID strings */
#define VAL_BINDEX_CH     '['
#define VAL_EINDEX_CH     ']'

#define VAL_BENUM_CH      '('
#define VAL_EENUM_CH      ')'

#define VAL_INST_SEPCH    '.'
#define VAL_INDEX_SEPCH   ','
#define VAL_INDEX_CLI_SEPCH   ' '
#define VAL_QUOTE_CH      '\''
#define VAL_DBLQUOTE_CH   '\"'
#define VAL_EQUAL_CH      '='
#define VAL_XPATH_SEPCH   '/'

#ifdef USE_AND_EXPR_IN_XPATH
#define VAL_XPATH_INDEX_SEPSTR ((const xmlChar *)" and ")
#define VAL_XPATH_INDEX_SEPLEN 5
#else
#define VAL_XPATH_INDEX_SEPSTR ((const xmlChar *)"][")
#define VAL_XPATH_INDEX_SEPLEN 2
#endif

/* val_value_t flags field */
#define VAL_FL_CONTAB    bit0
#define VAL_FL_DUPDONE   bit1
#define VAL_FL_DUPOK     bit2

/* macros to access simple value types */
#define VAL_BOOL(V)    ((V)->v.bool)

#define VAL_DOUBLE(V)  ((V)->v.num.d)

#define VAL_ENUM(V)    ((V)->v.enu.val)

#define VAL_ENUM_NAME(V)    ((V)->v.enu.name)

#define VAL_FLAG(V)    ((V)->v.bool)

#define VAL_FLOAT(V)   ((V)->v.num.f)

#define VAL_LONG(V)    ((V)->v.num.l)

#define VAL_INT(V)     ((V)->v.num.i)

#define VAL_INT8(V)     ((int8)((V)->v.num.i))

#define VAL_INT16(V)    ((int16)((V)->v.num.i))

#define VAL_STR(V)     ((V)->v.str)

#define VAL_USTR       VAL_STR

#define VAL_INSTANCE_ID(V)    ((V)->v.str)

#define VAL_UINT(V)    ((V)->v.num.u)

#define VAL_UINT8(V)    ((uint8)((V)->v.num.u))

#define VAL_UINT16(V)   ((uint16)((V)->v.num.u))

#define VAL_ULONG(V)   ((V)->v.num.ul)

#define VAL_LIST(V)    ((V)->v.list)

#define VAL_BITS VAL_LIST

#define VAL_XLIST(V)   ((V)->v.xlist)


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/* error record for one meta data variable */
typedef struct val_metaerr_t_ {
    dlq_hdr_t       qhdr;
    const xmlChar *name;  
    xmlChar       *dname; 
    xmlns_id_t     nsid;
} val_metaerr_t;


/* one value to match one type */
typedef struct val_value_t_ {
    dlq_hdr_t      qhdr;

    /* common fields */
    const typ_def_t *typdef;                 /* back ptr to typdef */
    const xmlChar *name;                 /* back pointer to elname */
    xmlChar       *dname;           /* AND malloced name if needed */
    void          *parent;             /* ps_parm_t or val_value_t */
    xmlns_id_t     nsid;
    ncx_btype_t    btyp;                /* base type of this value */
    uint32         seqid;         /* instance of this sibling node */
    uint32         flags; 
    ncx_data_class_t dataclass;
    ncx_node_t     parent_typ;        /* NCX_NT_PARM or NCX_NT_VAL */
    dlq_hdr_t      metaQ;                      /* Q of val_value_t */

    /* Used by Agent only:
     * if this field is non-NULL, then the entire value node
     * is actually a placeholder for a dynamic read-only object
     * and all read access is done via this callback function;
     * the real data type is getcb_fn_t *
     */
    void          *getcb;

    /* these fields are only used in new values before they are 
     * actually added to the config database (TBD: remove)
     */
    void          *curparent;      /* parent of curnode for merge */
    op_editop_t    editop;            /* effective edit operation */
    status_t       res;      /* edit result for continue-on-error */
    dlq_hdr_t      metaerrQ;                /* Q of val_metaerr_t */

    /* these fields are used for NCX_BT_LIST and NCX_BT_XCONTAINER */
    struct val_index_t_ *index;   /* back-ptr/flag in use as index */
    dlq_hdr_t       indexQ;    /* Q of val_index_t or ncx_filptr_t */

    /* these fields are used for NCX_BT_UNION */
    typ_template_t *untyp;               /* actual union node type */
    ncx_btype_t     unbtyp;              /* union member base type */

    /* union of all the NCX-specific sub-types */
    union v_ {
        dlq_hdr_t   appQ;        /* NCX_BT_ROOT - Q of cfg_app_t */
        dlq_hdr_t   childQ;         /* NCX_CL_COMPLEX, NCX_BT_ANY  
                                    *   Q of val_value_t           */
	ncx_num_t   num;         /* NCX_BT_INT8, NCX_BT_INT16,
                                  * NCX_BT_INT32, NCX_BT_INT64
                                  * NCX_BT_UINT8, NCX_BT_UINT16
                                  * NCX_BT_UINT32, NCX_BT_UINT64
 		                  * NCX_BT_FLOAT32, NCX_BT_FLOAT64 */
	ncx_str_t  str;       /* NCX_BT_ENAME, NCX_BT_STRING, 
			       * NCX_BT_INSTANCE_ID, NCX_BT_BINARY */
	ncx_list_t list;              /* NCX_BT_BITS, NCX_BT_SLIST */
	ncx_xlist_t xlist;                         /* NCX_BT_XLIST */
	boolean    bool;           /* NCX_BT_EMPTY, NCX_BT_BOOLEAN */
	ncx_enum_t enu;               /* NCX_BT_UNION, NCX_BT_ENUM */
	xmlChar   *fname;                         /* NCX_BT_EXTERN */
	xmlChar   *intbuff;                       /* NCX_BT_INTERN */
    } v;
} val_value_t;


/* Struct marking the parsing of an instance identifier
 * The position of this record in the val_value_t indexQ
 * represents the order the identifers were parsed
 * Since table and container data structures must always
 * appear in the specified field order, this will be the
 * same order for all well-formed entries.
 *
 * Each of these records will point to one nested val_value_t
 * record in the val_value_t childQ
 */
typedef struct val_index_t_ {
    dlq_hdr_t     qhdr;
    val_value_t  *val;      /* points to somewhere in the subtree */
} val_index_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern val_value_t *
    val_new_value (void);

extern void 
    val_init_value (val_value_t *val);

extern void
    val_init_complex (val_value_t *val, 
		      ncx_btype_t btyp);

extern void
    val_init_root (val_value_t *val);

extern void
    val_init_virtual (val_value_t *val,
		      void *cbfn,
		      typ_template_t *typ);

extern void
    val_init_from_template (val_value_t *val,
			    typ_template_t *typ);

extern void 
    val_free_value (val_value_t *val);

extern void 
    val_clean_value (val_value_t *val);

extern void 
    val_set_name (val_value_t *val,
		  const xmlChar *name,
		  uint32 namelen);

extern void 
    val_set_qname (val_value_t *val,
		   xmlns_id_t   nsid,
		   const xmlChar *name,
		   uint32 namelen);

extern status_t
    val_string_ok (const typ_def_t *typdef,
		   ncx_btype_t  btyp,
		   const xmlChar *strval);


/* validate all the ncx_lstr_t entries in the xlist
 * against the specified typdef.  Mark any errors
 * in the ncx_lstr_t flags field of each string 
 * in the list with an error
 */
extern status_t
    val_xlist_ok (const typ_def_t *typdef,
		  ncx_xlist_t *list);

/* validate all the ncx_lmem_t entries in the list
 * against the specified typdef.  Mark any errors
 * in the ncx_lmem_t flags field of each member
 * in the list with an error
 */
extern status_t
    val_list_ok (const typ_def_t *typdef,
		 ncx_list_t *list);

extern status_t
    val_enum_ok (const typ_def_t *typdef,
		 const xmlChar *enumval,
		 int32 *retval,
		 const xmlChar **retstr);

extern status_t
    val_range_ok (const typ_def_t *typdef,
		  ncx_btype_t  btyp,
		  const ncx_num_t *num);

/* check any simple type to see if it is valid,
 * but do not retrieve the value; used to check the
 * default parameter for example
 */
extern status_t
    val_simval_ok (const typ_def_t *typdef,
		   const xmlChar *simval);
		   
extern val_metaerr_t * 
    val_new_metaerr (xmlns_id_t nsid,
		     const xmlChar *name,
		     boolean copy);

extern void
    val_free_metaerr (val_metaerr_t *merr);


/* print a val_value_t struct contents to logfile or stdout */
extern void
    val_dump_value (const val_value_t *val,
		    int32 startindent);

/* print a val_value_t struct contents to stdout */
extern void
    val_stdout_value (const val_value_t *val,
		      int32 startindent);


extern status_t 
    val_set_string (val_value_t  *val,
		    const xmlChar *valname,
		    const xmlChar *valstr);

extern status_t 
    val_set_string2 (val_value_t  *val,
		     const xmlChar *valname,
		     const typ_def_t *typdef,
		     const xmlChar *valstr,
		     uint32 valstrlen);

extern status_t 
    val_set_simval (val_value_t  *val,
		    const typ_def_t    *typdef,
		    xmlns_id_t    nsid,
		    const xmlChar *valname,
		    const xmlChar *valstr);

extern status_t 
    val_set_simval_str (val_value_t  *val,
			const typ_def_t    *typdef,
			xmlns_id_t    nsid,
			const xmlChar *valname,
			uint32 valnamelen,
			const xmlChar *valstr);

extern val_value_t *
    val_make_simval (typ_def_t    *typdef,
		     xmlns_id_t    nsid,
		     const xmlChar *valname,
		     const xmlChar *valstr,
		     status_t  *res);

extern boolean
    val_merge (val_value_t *src,
	       val_value_t *dest);


/* just merge src->metaQ into dest->metaQ */
extern void
    val_merge_meta (val_value_t *src,
		    val_value_t *dest);

extern val_value_t *
    val_clone (const val_value_t *val);

extern status_t
    val_replace (const val_value_t *val,
		 val_value_t *copy);

extern status_t
    val_gen_instance_id (ncx_node_t nodetyp,
			 const void  *node, 
			 ncx_instfmt_t format,
			 boolean full,
			 xmlChar  **buff);

extern status_t 
    val_gen_index_comp  (const typ_index_t *in,
			 val_value_t *val);

extern status_t 
    val_gen_index_chain (const typ_index_t *instart,
			 val_value_t *val);

extern void
    val_add_child (val_value_t *child,
		   val_value_t *parent);

extern void
    val_swap_child (val_value_t *newchild,
		    val_value_t *curchild);

extern val_value_t *
    val_first_child (val_value_t *parent,
		     val_value_t *child);

extern val_value_t *
    val_get_first_child (const val_value_t *parent);

extern val_value_t *
    val_get_next_child (const val_value_t *curchild);

extern val_value_t *
    val_find_child (const val_value_t  *parent,
		    const xmlChar *childname);

extern const dlq_hdr_t *
    val_get_metaQ (const val_value_t  *val);

extern const val_value_t *
    val_get_first_meta (const dlq_hdr_t *queue);

extern const val_value_t *
    val_get_next_meta (const val_value_t *curmeta);

extern boolean
    val_meta_empty (const val_value_t *val);


/* find first -- really for resolve index function */
extern val_value_t *
    val_first_child_name (val_value_t *parent,
			  const xmlChar *name);

/* find first name value pair */
extern val_value_t *
    val_first_child_string (val_value_t *parent,
			    const xmlChar *name,
			    const xmlChar *strval);


/* get number of child nodes present -- for choice checking */
extern uint32
    val_child_cnt (val_value_t *parent);

/* get instance count -- for instance qualifer checking */
extern uint32
    val_child_inst_cnt (val_value_t *parent,
			const xmlChar *name);

extern uint32
    val_liststr_count (const val_value_t *val);

/* strnum is a zero-based index */
extern const xmlChar *
    val_get_liststr (const val_value_t *val,
		     uint32 strnum);

extern val_value_t *
    val_find_meta (const val_value_t *val,
		   xmlns_id_t   nsid,
		   const xmlChar *name);

extern boolean
    val_meta_match (const val_value_t *val,
		    const val_value_t *metaval);

extern boolean
    val_index_match (const val_value_t *val1,
		     const val_value_t *val2);

extern int32
    val_compare (const val_value_t *val1,
		 const val_value_t *val2);

extern status_t
    val_sprintf_simval_nc (xmlChar *buff,
			   const val_value_t  *val,
			   uint32  *len);

extern status_t 
    val_resolve_scoped_name (val_value_t *val,
			     const xmlChar *name,
			     val_value_t **chval);

extern ncx_iqual_t 
    val_get_iqualval (const val_value_t *val);

extern status_t
    val_union_ok (const typ_def_t *typdef,
		  const xmlChar *strval,
		  val_value_t *retval);

extern boolean
    val_duplicates_allowed (val_value_t *val);

extern boolean
    val_has_content (const val_value_t *val);

extern boolean
    val_has_index (const val_value_t *val);

extern status_t
    val_parse_meta (typ_def_t *typdef,
		    xmlns_id_t    nsid,
		    const xmlChar *attrname,
		    const xmlChar *attrval,
		    val_value_t *retval);

extern uint32
    val_metadata_inst_count (const val_value_t  *val,
			     xmlns_id_t nsid,
			     const xmlChar *name);

extern void
    val_set_extern (val_value_t  *val,
		    xmlChar *fname);

extern void
    val_set_intern (val_value_t  *val,
		    xmlChar *intbuff);

extern boolean
    val_fit_oneline (const val_value_t *val);


extern boolean
    val_create_allowed (const val_value_t *val);

extern boolean
    val_delete_allowed (const val_value_t *val);

extern boolean
    val_is_virtual (const val_value_t *val);

extern val_value_t *
    val_get_virtual_value (const void *scb,
			   val_value_t *val,
			   status_t *res);

extern void
    val_setup_virtual_retval (val_value_t  *virval,
			      val_value_t *realval);

extern val_value_t *
    val_new_virtual_chval (const xmlChar *name,
			   xmlns_id_t nsid,
			   typ_def_t *typdef,
			   val_value_t *parent);

extern boolean
    val_is_default (const val_value_t *val);

extern boolean
    val_is_real (const val_value_t *val);

extern xmlns_id_t 
    val_get_parent_nsid (const val_value_t *val);

extern status_t
    val_check_rangeQ (ncx_btype_t  btyp,
		      const ncx_num_t *num,
		      const dlq_hdr_t *checkQ);

#endif	    /* _H_val */
