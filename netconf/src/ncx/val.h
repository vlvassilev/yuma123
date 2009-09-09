#ifndef _H_val
#define _H_val

/*  FILE: val.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Value Node Basic Support

  Value nodes used in thoughout the system are complex
  'automation depositories', which contain all the glue
  to automate the NETCONF functions.

  Almost all value node variants provide user callback
  hooks for CRUD operations on the node.  The read
  operations are usually driven from centrally stored
  data, unless the value node is a 'virtual' value.

  Basic Value Node Usage:
  -----------------------

  1a) Malloc a new value with val_new_value()
        or
  1b)  Initialize a static val_value_t with val_init_value

  2) Bind the value to an object template:
      val_init_from_template

      or use val_make_simval to combine steps 1a and 2

  3) set simple values with various functions, such as
     val_set_simval

  4) When constructing complex values, use val_add_child
     to add them to the parent

  5a) Use val_free_value to free the memory for a value
 
  5b) Use val_clean_value to clean and reuse a value struct

  Internal Value Nodes
  --------------------

  A special developer-level feature to assign arbitrary
  internal values, not in the encoded format. 
  Not used within the configuration database.
  (More TBD)

  External Value Nodes
  --------------------

  The yangcli program allows the user to assign values
  from user and system defined script variables to a
  value node.    Not used within the configuration database.
  (More TBD)

  Virtual Value Nodes
  -------------------

  If a value node does not store its data locally, then it
  is called a virtual node.  Callback functions are used
  for almost all protocol operation support.


*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
19-dec-05    abb      Begun
21jul08      abb      start obj-based rewrite

*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
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

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/


#define VAL_MAX_NUMLEN  NCX_MAX_NUMLEN

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

#define VAL_XPATH_INDEX_SEPSTR (const xmlChar *)"]["
#define VAL_XPATH_INDEX_SEPLEN 2

/* display instead of readl password contents */
#define VAL_PASSWORD_STRING  (const xmlChar *)"****"

/* val_value_t flags field */

/* if set the duplicates-ok test has been done */
#define VAL_FL_DUPDONE   bit0

/* if set the duplicates-ok test was OK */
#define VAL_FL_DUPOK     bit1

/* if set, this value was added by val_add_defaults */
#define VAL_FL_DEFSET    bit2

/* if set, value is actually for an XML attribute */
#define VAL_FL_META      bit3

/* if set, value has been edited or added */
#define VAL_FL_DIRTY     bit4

/* if set, value is a list which has unique-stmt already failed */
#define VAL_FL_UNIDONE   bit5


/* macros to access simple value types */
#define VAL_BOOL(V)    ((V)->v.bool)

#define VAL_DOUBLE(V)  ((V)->v.num.d)

#define VAL_ENU(V)     (&(V)->v.enu)

#define VAL_ENUM(V)    ((V)->v.enu.val)

#define VAL_ENUM_NAME(V)    ((V)->v.enu.name)

#define VAL_FLAG(V)    ((V)->v.bool)

#define VAL_FLOAT(V)   ((V)->v.num.f)

#define VAL_LONG(V)    ((V)->v.num.l)

#define VAL_INT(V)     ((V)->v.num.i)

#define VAL_INT8(V)     ((int8)((V)->v.num.i))

#define VAL_INT16(V)    ((int16)((V)->v.num.i))

#define VAL_STR(V)     ((V)->v.str)

#define VAL_INSTANCE_ID(V)    ((V)->v.str)

#define VAL_IDREF(V)    (&(V)->v.idref)

#define VAL_IDREF_NSID(V)    ((V)->v.idref.nsid)

#define VAL_IDREF_NAME(V)    ((V)->v.idref.name)

#define VAL_UINT(V)    ((V)->v.num.u)

#define VAL_UINT8(V)    ((uint8)((V)->v.num.u))

#define VAL_UINT16(V)   ((uint16)((V)->v.num.u))

#define VAL_ULONG(V)   ((V)->v.num.ul)

#define VAL_LIST(V)    ((V)->v.list)

#define VAL_BITS VAL_LIST


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* one QName for the NCX_BT_IDREF value */
typedef struct val_idref_t_ {
    xmlns_id_t  nsid;
    /* if nsid == INV_ID then this is entire QName */
    xmlChar    *name;
    const ncx_identity_t  *identity;  /* ID back-ptr if found */
} val_idref_t;


/* one set of edit-in-progress variables for one value node */
typedef struct val_editvars_t_ {
    /* these fields are only used in modified values before they are 
     * actually added to the config database (TBD: move into struct)
     * curparent == parent of curnode for merge
     */
    struct val_value_t_  *curparent;      
    op_editop_t    editop;            /* effective edit operation */
    op_insertop_t  insertop;             /* YANG insert operation */
    xmlChar       *insertstr;          /* saved value or key attr */
    struct xpath_pcb_t_ *insertxpcb;       /* key attr for insert */
    struct val_value_t_ *insertval;                   /* back-ptr */
    boolean        iskey;                     /* T: key, F: value */
    boolean        operset;                  /* nc:operation here */
} val_editvars_t;


/* one value to match one type */
typedef struct val_value_t_ {
    dlq_hdr_t      qhdr;

    /* common fields */
    struct obj_template_t_ *obj;        /* bptr to object def */
    typ_def_t *typdef;              /* bptr to typdef if leaf */
    const xmlChar   *name;                /* back pointer to elname */
    xmlChar         *dname;          /* AND malloced name if needed */
    struct val_value_t_ *parent;       /* back-ptr to parent if any */
    xmlns_id_t     nsid;              /* namespace ID for this node */
    ncx_btype_t    btyp;                 /* base type of this value */

    uint32         flags;                  /* internal status flags */
    ncx_data_class_t dataclass;             /* config or state data */

    /* YANG does not support user-defined meta-data but NCX does.
     * The <edit-config>, <get> and <get-config> operations 
     * use attributes in the RPC parameters, the metaQ is still used
     *
     * The ncx:metadata extension allows optional attributes
     * to be added to object nodes for anyxml, leaf, leaf-list,
     * list, and container nodes.  The config property will
     * be inherited from the object that contains the metadata
     *
     * This is used mostly for RPC input parameters
     * and is strongly discouraged.  Full edit-config
     * support is not provided for metdata
     */
    dlq_hdr_t        metaQ;                      /* Q of val_value_t */

    /* value editing variables */
    val_editvars_t  *editvars;              /* edit-in-progress vars */
    status_t         res;                      /* validationt result */

    /* Used by Agent only:
     * if this field is non-NULL, then the entire value node
     * is actually a placeholder for a dynamic read-only object
     * and all read access is done via this callback function;
     * the real data type is getcb_fn_t *
     */
    void *getcb;

    /* if this field is non-NULL, then a malloced value struct
     * representing the real value retrieved by 
     * val_get_virtual_value, is cached here for XPath filtering
     * TBD: add timestamp to reuse cached entries for some time
     * period
     */
    struct val_value_t_ *virtualval;

    /* these fields are used for NCX_BT_LIST */
    struct val_index_t_ *index;   /* back-ptr/flag in use as index */
    dlq_hdr_t       indexQ;    /* Q of val_index_t or ncx_filptr_t */

    /* this field is used for NCX_BT_CHOICE 
     * If set, the object path for this node is really:
     *    $this --> casobj --> casobj.parent --> $this.parent
     * the OBJ_TYP_CASE and OBJ_TYP_CHOICE nodes are skipped
     * inside an XML instance document
     */
    struct obj_template_t_   *casobj;

    /* these fields are for NCX_BT_LEAFREF
     * NCX_BT_INSTANCE_ID, or tagged ncx:xpath 
     * value stored in v union as a string
     */
    struct xpath_pcb_t_            *xpathpcb;

    /* union of all the NCX-specific sub-types
     * note that the following invisible constructs should
     * never show up in this struct:
     *     NCX_BT_CHOICE
     *     NCX_BT_CASE
     *     NCX_BT_UNION
     */
    union v_ {
	/* complex types have a Q of val_value_t representing
	 * the child nodes with values
	 *   NCX_BT_CONTAINER
	 *   NCX_BT_LIST
	 */
        dlq_hdr_t   childQ;         

        /* Numeric data types:
	 *   NCX_BT_INT8, NCX_BT_INT16,
	 *   NCX_BT_INT32, NCX_BT_INT64
	 *   NCX_BT_UINT8, NCX_BT_UINT16
	 *   NCX_BT_UINT32, NCX_BT_UINT64
	 *   NCX_BT_DECIMAL64, NCX_BT_FLOAT64 
	 */
	ncx_num_t   num; 

	/* String data types:
	 *   NCX_BT_STRING
	 *   NCX_BT_INSTANCE_ID
	 */
	ncx_str_t  str; 

	val_idref_t idref;

	ncx_binary_t binary;              /* NCX_BT_BINARY */
	ncx_list_t list;      /* NCX_BT_BITS, NCX_BT_SLIST */
	boolean    bool;   /* NCX_BT_EMPTY, NCX_BT_BOOLEAN */
	ncx_enum_t enu;       /* NCX_BT_UNION, NCX_BT_ENUM */
	xmlChar   *fname;                 /* NCX_BT_EXTERN */
	xmlChar   *intbuff;               /* NCX_BT_INTERN */
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
    val_value_t  *val;      /* points to a child node */
} val_index_t;


/* one unique-stmt component test value node */
typedef struct val_unique_t_ {
    dlq_hdr_t     qhdr;
    val_value_t  *valptr;
} val_unique_t;


/* test callback function to check if a value node 
 * should be cloned 
 *
 * INPUTS:
 *   val == value node to check
 *
 * RETURNS:
 *   TRUE if OK to be cloned
 *   FALSE if not OK to be cloned (skipped instead)
 */
typedef boolean
    (*val_test_fn_t) (const val_value_t *val);


/* child or descendent node search walker function
 *
 * INPUTS:
 *   val == value node found in descendent search
 *   cookie1 == cookie1 value passed to start of walk
 *   cookie2 == cookie2 value passed to start of walk
 *
 * RETURNS:
 *   TRUE if walk should continue
 *   FALSE if walk should terminate 
 */
typedef boolean
    (*val_walker_fn_t) (val_value_t *val,
			void *cookie1,
			void *cookie2);


typedef enum val_dumpvalue_mode_t_ {
    DUMP_VAL_NONE,
    DUMP_VAL_STDOUT,
    DUMP_VAL_LOG,
    DUMP_VAL_ALT_LOG
} val_dumpvalue_mode_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern val_value_t *
    val_new_value (void);

/* this is deprecated and should only be called 
 * by val_init_from_template
 */
extern void
    val_init_complex (val_value_t *val, 
		      ncx_btype_t btyp);

extern void
    val_init_virtual (val_value_t *val,
		      void *cbfn,
		      struct obj_template_t_ *obj);

extern void
    val_init_from_template (val_value_t *val,
			    struct obj_template_t_ *obj);

extern void 
    val_free_value (val_value_t *val);

extern status_t
    val_new_editvars (val_value_t *val);

extern void
    val_free_editvars (val_value_t *val);

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
    val_string_ok (typ_def_t *typdef,
		   ncx_btype_t  btyp,
		   const xmlChar *strval);

/* retrieve the YANG custom error info if any */
extern status_t
    val_string_ok_errinfo (typ_def_t *typdef,
			   ncx_btype_t  btyp,
			   const xmlChar *strval,
			   ncx_errinfo_t **errinfo);


/* validate all the ncx_lmem_t entries in the list
 * against the specified typdef.  Mark any errors
 * in the ncx_lmem_t flags field of each member
 * in the list with an error
 */
extern status_t
    val_list_ok (typ_def_t *typdef,
		 ncx_btype_t btyp,
		 ncx_list_t *list);

extern status_t
    val_list_ok_errinfo (typ_def_t *typdef,
			 ncx_btype_t btyp,
			 ncx_list_t *list,
			 ncx_errinfo_t **errinfo);

extern status_t
    val_enum_ok (typ_def_t *typdef,
		 const xmlChar *enumval,
		 int32 *retval,
		 const xmlChar **retstr);

extern status_t
    val_bit_ok (typ_def_t *typdef,
		const xmlChar *bitname,
		uint32 *position);

extern status_t
    val_idref_ok (typ_def_t *typdef,
		  const xmlChar *qname,
		  xmlns_id_t nsid,
		  const xmlChar **name,
		  const ncx_identity_t **id);

extern status_t
    val_parse_idref (ncx_module_t *mod,
		     const xmlChar *qname,
		     xmlns_id_t  *nsid,
		     const xmlChar **name,
		     const ncx_identity_t **id);

extern status_t
    val_range_ok (typ_def_t *typdef,
		  ncx_btype_t  btyp,
		  const ncx_num_t *num);

extern status_t
    val_pattern_ok (typ_def_t *typdef,
		    const xmlChar *strval);

extern status_t
    val_pattern_ok_errinfo (typ_def_t *typdef,
			    const xmlChar *strval,
			    ncx_errinfo_t **errinfo);

extern status_t
    val_range_ok_errinfo (typ_def_t *typdef,
			  ncx_btype_t  btyp,
			  const ncx_num_t *num,
			  ncx_errinfo_t **errinfo);



/* check any simple type to see if it is valid,
 * but do not retrieve the value; used to check the
 * default parameter for example
 */
extern status_t
    val_simval_ok (typ_def_t *typdef,
		   const xmlChar *simval);
		   
extern status_t
    val_simval_ok_errinfo (typ_def_t *typdef,
			   const xmlChar *simval,
			   ncx_errinfo_t **errinfo);

extern status_t
    val_union_ok (typ_def_t *typdef,
		  const xmlChar *strval,
		  val_value_t *retval);

extern status_t
    val_union_ok_errinfo (typ_def_t *typdef,
			  const xmlChar *strval,
			  val_value_t *retval,
			  ncx_errinfo_t **errinfo);


extern dlq_hdr_t *
    val_get_metaQ (val_value_t  *val);

extern val_value_t *
    val_get_first_meta (dlq_hdr_t *queue);

extern val_value_t *
    val_get_first_meta_val (val_value_t *val);

extern val_value_t *
    val_get_next_meta (val_value_t *curmeta);

extern boolean
    val_meta_empty (val_value_t *val);

extern val_value_t *
    val_find_meta (val_value_t *val,
		   xmlns_id_t   nsid,
		   const xmlChar *name);

extern boolean
    val_meta_match (val_value_t *val,
		    val_value_t *metaval);


extern uint32
    val_metadata_inst_count (val_value_t  *val,
			     xmlns_id_t nsid,
			     const xmlChar *name);



/* print a val_value_t struct contents to logfile or stdout */
extern void
    val_dump_value (val_value_t *val,
		    int32 startindent);

extern void
    val_dump_value_ex (val_value_t *val,
                       int32 startindent,
                       ncx_display_mode_t display_mode);

/* print a val_value_t struct contents to alternate logfile */
extern void
    val_dump_alt_value (val_value_t *val,
			int32 startindent);

/* print a val_value_t struct contents to stdout */
extern void
    val_stdout_value (val_value_t *val,
		      int32 startindent);

extern void
    val_stdout_value_ex (val_value_t *val,
                         int32 startindent,
                         ncx_display_mode_t display_mode);

extern void
    val_dump_value_max (val_value_t *val,
                        int32 startindent,
                        int32 indent_amount,
                        val_dumpvalue_mode_t dumpmode,
                        ncx_display_mode_t display_mode,
                        boolean with_meta);

/* use next 4 functions after calling
 * val_new_value().
 */

/* set a generic string using the builtin string typdef */
extern status_t 
    val_set_string (val_value_t  *val,
		    const xmlChar *valname,
		    const xmlChar *valstr);

/* set a string with any typdef */
extern status_t 
    val_set_string2 (val_value_t  *val,
		     const xmlChar *valname,
		     typ_def_t *typdef,
		     const xmlChar *valstr,
		     uint32 valstrlen);

/* clean a value and set it to empty type
 * used by yangcli to delete leafs
 */
extern status_t 
    val_reset_empty (val_value_t  *val);

/* set any simple value with any typdef */
extern status_t 
    val_set_simval (val_value_t  *val,
		    typ_def_t *typdef,
		    xmlns_id_t    nsid,
		    const xmlChar *valname,
		    const xmlChar *valstr);

/* set any simple value with any typdef, and a counted string */
extern status_t 
    val_set_simval_str (val_value_t  *val,
			typ_def_t *typdef,
			xmlns_id_t    nsid,
			const xmlChar *valname,
			uint32 valnamelen,
			const xmlChar *valstr);

/* same as val_set_simval, but malloc the value first */
extern val_value_t *
    val_make_simval (typ_def_t *typdef,
		     xmlns_id_t    nsid,
		     const xmlChar *valname,
		     const xmlChar *valstr,
		     status_t  *res);

extern val_value_t *
    val_make_string (xmlns_id_t nsid,
		     const xmlChar *valname,
		     const xmlChar *valstr);

extern boolean
    val_merge (val_value_t *src,
	       val_value_t *dest);

extern val_value_t *
    val_clone (const val_value_t *val);

extern val_value_t *
    val_clone_test (const val_value_t *val,
		    val_test_fn_t  testfn,
		    status_t *res);

/* pass in a config node, such as <config> root
 * will call val_clone_test with the val_is_config_data
 * callbacck function
 */
extern val_value_t *
    val_clone_config_data (const val_value_t *val,
			   status_t *res);

extern status_t
    val_replace (val_value_t *val,
		 val_value_t *copy);

extern void
    val_add_child (val_value_t *child,
		   val_value_t *parent);

/* add an object and delete any extra cases */
extern void
    val_add_child_clean (val_value_t *child,
			 val_value_t *parent,
			 dlq_hdr_t *cleanQ);


extern void
    val_add_child_clean_editvars (val_editvars_t *editvars,
				  val_value_t *child,
				  val_value_t *parent,
				  dlq_hdr_t *cleanQ);

extern void
    val_insert_child (val_value_t *child,
		      val_value_t *current,
		      val_value_t *parent);

extern void
    val_remove_child (val_value_t *child);

extern void
    val_swap_child (val_value_t *newchild,
		    val_value_t *curchild);

extern val_value_t *
    val_first_child_match (val_value_t *parent,
			   val_value_t *child);

extern val_value_t *
    val_next_child_match (val_value_t *parent,
			  val_value_t *child,
			  val_value_t *curmatch);

extern val_value_t *
    val_get_first_child (const val_value_t *parent);

extern val_value_t *
    val_get_next_child (const val_value_t *curchild);

extern val_value_t *
    val_find_child (const val_value_t  *parent,
		    const xmlChar  *modname,
		    const xmlChar *childname);

extern val_value_t *
    val_match_child (const val_value_t  *parent,
		     const xmlChar  *modname,
		     const xmlChar *childname);

extern val_value_t *
    val_find_next_child (const val_value_t  *parent,
			 const xmlChar  *modname,
			 const xmlChar *childname,
			 const val_value_t *curchild);


/* find first -- really for resolve index function */
extern val_value_t *
    val_first_child_name (val_value_t *parent,
			  const xmlChar *name);

extern val_value_t *
    val_first_child_qname (val_value_t *parent,
			   xmlns_id_t   nsid,
			   const xmlChar *name);

extern val_value_t *
    val_next_child_qname (val_value_t *parent,
			  xmlns_id_t   nsid,
			  const xmlChar *name,
			  val_value_t *curchild);


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
    val_child_inst_cnt (const val_value_t *parent,
			const xmlChar *modname,
			const xmlChar *name);


extern uint32
    val_get_child_inst_id (const val_value_t *parent,
			   const val_value_t *child);


extern boolean
    val_find_all_children (val_walker_fn_t walkerfn,
			   void *cookie1,
			   void *cookie2,
			   val_value_t *startnode,
			   const xmlChar *modname,
			   const xmlChar *name,
			   boolean configonly,
			   boolean textmode);

extern boolean
    val_find_all_ancestors (val_walker_fn_t walkerfn,
			    void *cookie1,
			    void *cookie2,
			    val_value_t *startnode,
			    const xmlChar *modname,
			    const xmlChar *name,
			    boolean configonly,			    
			    boolean textmode,
			    boolean orself);

extern boolean
    val_find_all_descendants (val_walker_fn_t walkerfn,
			      void *cookie1,
			      void *cookie2,
			      val_value_t *startnode,
			      const xmlChar *modname,
			      const xmlChar *name,
			      boolean configonly,
			      boolean textmode,
			      boolean orself,
			      boolean forceall);

			      
extern boolean
    val_find_all_pfaxis (val_walker_fn_t walkerfn,
			 void *cookie1,
			 void *cookie2,
			 val_value_t *startnode,
			 const xmlChar *modname,
			 const xmlChar *name,
			 boolean configonly,
			 boolean dblslash,
			 boolean textmode,
			 ncx_xpath_axis_t axis);
			      

extern boolean
    val_find_all_pfsibling_axis (val_walker_fn_t  walkerfn,
				 void *cookie1,
				 void *cookie2,
				 val_value_t *startnode,
				 const xmlChar *modname,
				 const xmlChar *name,
				 boolean configonly,
				 boolean dblslash,
				 boolean textmode,
				 ncx_xpath_axis_t axis);
			      
extern val_value_t *
    val_get_axisnode (val_value_t *startnode,
		      const xmlChar *modname,
		      const xmlChar *name,
		      boolean configonly,
		      boolean dblslash,
		      boolean textmode,
		      ncx_xpath_axis_t axis,
		      int64 position);

		    
extern uint32
    val_liststr_count (const val_value_t *val);


extern boolean
    val_index_match (val_value_t *val1,
		     val_value_t *val2);

extern int32
    val_compare_ex (val_value_t *val1,
                    val_value_t *val2,
                    boolean configonly);

extern int32
    val_compare (val_value_t *val1,
		 val_value_t *val2);

extern int32
    val_compare_for_replace (val_value_t *val1,
                             val_value_t *val2);


extern int32
    val_compare_to_string (val_value_t *val1,
			   const xmlChar *strval2,
			   status_t *res);

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


extern boolean
    val_duplicates_allowed (val_value_t *val);

extern boolean
    val_has_content (const val_value_t *val);

extern boolean
    val_has_index (const val_value_t *val);

extern val_index_t *
    val_get_first_index (const val_value_t *val);

extern val_index_t *
    val_get_next_index (const val_index_t *valindex);

extern status_t
    val_parse_meta (typ_def_t *typdef,
		    xml_attr_t *attr,
		    val_value_t *retval);

extern void
    val_set_extern (val_value_t  *val,
		    xmlChar *fname);

extern void
    val_set_intern (val_value_t  *val,
		    xmlChar *intbuff);

extern boolean
    val_fit_oneline (const val_value_t *val,
                     uint32 linesize);


extern boolean
    val_create_allowed (const val_value_t *val);

extern boolean
    val_delete_allowed (const val_value_t *val);

extern boolean
    val_is_config_data (const val_value_t *val);

extern boolean
    val_is_virtual (const val_value_t *val);

/* must free the return val; not cached */
extern val_value_t *
    val_get_virtual_value (void *session,  /* really ses_cb_t *   */
			   const val_value_t *val,
			   status_t *res);

/* get + cache as val->virtualval; DO NOT FREE the return val */
extern val_value_t *
    val_cache_virtual_value (void *session,  /* really ses_cb_t *   */
                             val_value_t *val,
                             status_t *res);

extern boolean
    val_is_default (const val_value_t *val);

extern boolean
    val_is_real (const val_value_t *val);

extern xmlns_id_t 
    val_get_parent_nsid (const val_value_t *val);

/* count child instances of modname:objname within parent 'val' */
extern uint32
    val_instance_count (val_value_t  *val,
			const xmlChar *modname,
			const xmlChar *objname);

/* mark ERR_NCX_EXTRA_VAL_INST errors for nodes > 'maxelems' */
extern void
    val_set_extra_instance_errors (val_value_t  *val,
				   const xmlChar *modname,
				   const xmlChar *objname,
				   uint32 maxelems);


extern boolean
    val_need_quotes (const xmlChar *str);


extern boolean
    val_all_whitespace (const xmlChar *str);


extern boolean
    val_match_metaval (const xml_attr_t *attr,
		       xmlns_id_t  nsid,
		       const xmlChar *name);


extern boolean
    val_get_dirty_flag (const val_value_t *val);

extern void
    val_set_dirty_flag (val_value_t *val);

extern void
    val_clear_dirty_flag (val_value_t *val);

extern void
    val_clean_tree (val_value_t *val);

extern uint32
    val_get_nest_level (val_value_t *val);

extern val_value_t *
    val_get_first_leaf (val_value_t *val);

extern const xmlChar *
    val_get_mod_name (const val_value_t *val);

extern const xmlChar *
    val_get_mod_prefix (const val_value_t *val);

extern xmlns_id_t
    val_get_nsid (const val_value_t *val);

extern void
    val_change_nsid (val_value_t *val,
		     xmlns_id_t nsid);

extern val_value_t *
    val_make_from_insertxpcb (val_value_t  *sourceval,
			      status_t *res);

extern val_unique_t * 
    val_new_unique (void);

extern void
    val_free_unique (val_unique_t *valuni);

extern const typ_def_t *
    val_get_typdef (const val_value_t *val);

extern boolean
    val_set_by_default (const val_value_t *val);

extern boolean
    val_is_metaval (const val_value_t *val);

extern void
    val_move_children (val_value_t *srcval,
                       val_value_t *destval);

#endif	    /* _H_val */
