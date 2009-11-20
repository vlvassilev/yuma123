#ifndef _H_ncx
#define _H_ncx

/*  FILE: ncx.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************



    NCX Module Library Utility Functions


                      Container of Definitions
                        +-----------------+ 
                        |   ncx_module_t  |
                        |   ncxtypes.h    |
                        +-----------------+

            Template/Schema 
          +-----------------+ 
          | obj_template_t  |
          |     obj.h       |
          +-----------------+
             ^          |
             |          |       Value Instances
             |          |     +-----------------+
             |          +---->|   val_value_t   |
             |                |      val.h      |
             |                +-----------------+
             |                               
             |       Data Types
             |   +--------------------+      
             +---|   typ_template_t   |      
                 |       typ.h        |      
                 +--------------------+      

 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
29-oct-05    abb      Begun
20-jul-08    abb      Start YANG rewrite; remove PSD and PS
23-aug-09    abb      Update diagram in header
*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_grp
#include "grp.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
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

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    ncx_init (boolean savestr,
	      log_debug_t dlevel,
	      boolean logtstamps,
	      const char *startmsg,
	      int argc,
	      const char *argv[]);


extern status_t
    ncx_stage2_init (void);

extern void 
    ncx_cleanup (void);

/**************** ncx_module_t *********************/
extern ncx_module_t * 
    ncx_new_module (void);

extern ncx_module_t *
    ncx_find_module (const xmlChar *modname,
		     const xmlChar *revision);

extern ncx_module_t *
    ncx_find_module_que (dlq_hdr_t *modQ,
                         const xmlChar *modname,
                         const xmlChar *revision);


extern ncx_module_t *
    ncx_find_module_que_nsid (dlq_hdr_t *modQ,
                              xmlns_id_t nsid);

/* use if module was not added to registry */
extern void 
    ncx_free_module (ncx_module_t *mod);


/* check if any YANG modules loaded with non-fatal errors
 * NCX files are loaded with no errors or nothing
 */
extern boolean
    ncx_any_mod_errors (void);

/* check if any imported modules that the specified module uses
 * has any errors
 */
extern boolean
    ncx_any_dependency_errors (const ncx_module_t *mod);

extern typ_template_t * 
    ncx_find_type (ncx_module_t *mod,
		   const xmlChar *typname);

extern typ_template_t *
    ncx_find_type_que (const dlq_hdr_t *typeQ,
		       const xmlChar *typname);

extern grp_template_t * 
    ncx_find_grouping (ncx_module_t *mod,
		       const xmlChar *grpname);

extern grp_template_t *
    ncx_find_grouping_que (const dlq_hdr_t *groupingQ,
			   const xmlChar *grpname);

extern obj_template_t * 
    ncx_find_rpc (const ncx_module_t *mod,
		  const xmlChar *rpcname);

extern obj_template_t * 
    ncx_match_rpc (const ncx_module_t *mod,
		   const xmlChar *rpcname,
                   uint32 *retcount);

extern obj_template_t * 
    ncx_match_any_rpc (const xmlChar *module,
		       const xmlChar *rpcname,
                       uint32 *retcount);

extern obj_template_t *
    ncx_find_any_object (const xmlChar *objname);

extern obj_template_t *
    ncx_find_any_object_que (dlq_hdr_t *modQ,
                             const xmlChar *objname);

extern obj_template_t *
    ncx_find_object (ncx_module_t *mod,
		     const xmlChar *objname);

extern status_t 
    ncx_add_namespace_to_registry (ncx_module_t *mod,
                                   boolean tempmod);

extern status_t 
    ncx_add_to_registry (ncx_module_t *mod);

extern status_t 
    ncx_add_to_modQ (ncx_module_t *mod);

extern boolean
    ncx_is_duplicate (ncx_module_t *mod,
		      const xmlChar *defname);

extern ncx_module_t *
    ncx_get_first_module (void);

extern ncx_module_t *
    ncx_get_first_session_module (void);

extern ncx_module_t *
    ncx_get_next_session_module (const ncx_module_t *mod);

extern ncx_module_t *
    ncx_get_next_module (const ncx_module_t *mod);

/* returns mod->name for main mod, mod->belongs for submod */
extern const xmlChar *
    ncx_get_modname (const ncx_module_t *mod);

/* returns highest revision date or NULL if none */
extern const xmlChar *
    ncx_get_modversion (const ncx_module_t *mod);

/* get the module namespace URI */
extern const xmlChar *
    ncx_get_modnamespace (const ncx_module_t *mod);

extern ncx_module_t *
    ncx_get_mainmod (ncx_module_t *mod);


/************* top obj_template_t in module **************/
extern obj_template_t *
    ncx_get_first_object (ncx_module_t *mod);

extern obj_template_t *
    ncx_get_next_object (ncx_module_t *mod,
			 obj_template_t *curobj);

extern obj_template_t *
    ncx_get_first_data_object (ncx_module_t *mod);

extern obj_template_t *
    ncx_get_next_data_object (ncx_module_t *mod,
			      obj_template_t *curobj);

/******************** ncx_import_t ******************/

extern ncx_import_t * 
    ncx_new_import (void);

extern void 
    ncx_free_import (ncx_import_t *import);

extern ncx_import_t * 
    ncx_find_import (const ncx_module_t *mod,
		     const xmlChar *module);

extern ncx_import_t * 
    ncx_find_import_que (const dlq_hdr_t *importQ,
                         const xmlChar *module);

extern ncx_import_t * 
    ncx_find_import_test (const ncx_module_t *mod,
			  const xmlChar *module);

extern ncx_import_t * 
    ncx_find_pre_import (const ncx_module_t *mod,
			 const xmlChar *prefix);

extern ncx_import_t * 
    ncx_find_pre_import_que (const dlq_hdr_t *importQ,
                             const xmlChar *prefix);

extern ncx_import_t * 
    ncx_find_pre_import_test (const ncx_module_t *mod,
			      const xmlChar *prefix);

extern void *
    ncx_locate_modqual_import (yang_pcb_t *pcb,
                               ncx_import_t *imp,
			       const xmlChar *defname,
			       ncx_node_t *deftyp);

/******************** ncx_include_t ******************/

extern ncx_include_t * 
    ncx_new_include (void);

extern void 
    ncx_free_include (ncx_include_t *inc);

extern ncx_include_t * 
    ncx_find_include (const ncx_module_t *mod,
		      const xmlChar *submodule);

/********************** ncx_num_t *********************/

extern void
    ncx_init_num (ncx_num_t *num);

extern void 
    ncx_clean_num (ncx_btype_t btyp,
		   ncx_num_t *num);

extern int32
    ncx_compare_nums (const ncx_num_t *num1,
		      const ncx_num_t *num2,
		      ncx_btype_t  btyp);

extern void
    ncx_set_num_min (ncx_num_t *num,
		     ncx_btype_t  btyp);

extern void
    ncx_set_num_max (ncx_num_t *num,
		     ncx_btype_t  btyp);

extern void
    ncx_set_num_one (ncx_num_t *num,
		     ncx_btype_t  btyp);

extern void
    ncx_set_num_zero (ncx_num_t *num,
		      ncx_btype_t  btyp);

extern void
    ncx_set_num_nan (ncx_num_t *num,
		     ncx_btype_t  btyp);

extern boolean
    ncx_num_is_nan (ncx_num_t *num,
		    ncx_btype_t  btyp);

extern boolean
    ncx_num_zero (const ncx_num_t *num,
		  ncx_btype_t  btyp);

extern status_t
    ncx_convert_num (const xmlChar *numstr,
		     ncx_numfmt_t numfmt,
		     ncx_btype_t  btyp,
		     ncx_num_t    *val);

extern status_t
    ncx_convert_dec64 (const xmlChar *numstr,
		       ncx_numfmt_t numfmt,
		       uint8 digits,
		       ncx_num_t *val);

extern status_t 
    ncx_decode_num (const xmlChar *numstr,
		    ncx_btype_t  btyp,
		    ncx_num_t  *retnum);

extern status_t 
    ncx_decode_dec64 (const xmlChar *numstr,
		      uint8 digits,
		      ncx_num_t  *retnum);

extern status_t
    ncx_copy_num (const ncx_num_t *num1,
		  ncx_num_t *num2,
		  ncx_btype_t  btyp);

extern status_t
    ncx_cast_num (const ncx_num_t *num1,
		  ncx_btype_t  btyp1,
		  ncx_num_t *num2,
		  ncx_btype_t  btyp2);

extern status_t
    ncx_num_floor (const ncx_num_t *num1,
		   ncx_num_t *num2,
		   ncx_btype_t  btyp);

extern status_t
    ncx_num_ceiling (const ncx_num_t *num1,
		     ncx_num_t *num2,
		     ncx_btype_t  btyp);

extern status_t
    ncx_round_num (const ncx_num_t *num1,
		   ncx_num_t *num2,
		   ncx_btype_t  btyp);

extern boolean
    ncx_num_is_integral (const ncx_num_t *num,
			 ncx_btype_t  btyp);

extern int64
    ncx_cvt_to_int64 (const ncx_num_t *num,
		      ncx_btype_t  btyp);

extern ncx_numfmt_t
    ncx_get_numfmt (const xmlChar *numstr);

extern void
    ncx_printf_num (const ncx_num_t *num,
		    ncx_btype_t  btyp);

extern void
    ncx_alt_printf_num (const ncx_num_t *num,
			ncx_btype_t  btyp);

extern status_t
    ncx_sprintf_num (xmlChar *buff,
		     const ncx_num_t *num,
		     ncx_btype_t  btyp,
		     uint32  *len);

extern boolean
    ncx_is_min (const ncx_num_t *num,
		ncx_btype_t btyp);

extern boolean
    ncx_is_max (const ncx_num_t *num,
		ncx_btype_t btyp);

extern status_t
    ncx_convert_tkcnum (tk_chain_t *tkc,
			ncx_btype_t  btyp,
			ncx_num_t *val);

extern status_t
    ncx_convert_tkc_dec64 (tk_chain_t *tkc,
			   uint8 digits,
			   ncx_num_t *val);

/********************** ncx_str_t *********************/

extern int32
    ncx_compare_strs (const ncx_str_t *str1,
		      const ncx_str_t *str2,
		      ncx_btype_t  btyp);

extern status_t
    ncx_copy_str (const ncx_str_t *str1,
		  ncx_str_t *str2,
		  ncx_btype_t  btyp);

extern void 
    ncx_clean_str (ncx_str_t *str);

/********************** ncx_list_t *********************/

extern ncx_list_t *
    ncx_new_list (ncx_btype_t btyp);

extern void
    ncx_init_list (ncx_list_t *list,
		   ncx_btype_t btyp);

extern void 
    ncx_clean_list (ncx_list_t *list);

extern void
    ncx_free_list (ncx_list_t *list);

extern uint32
    ncx_list_cnt (const ncx_list_t *list);

extern boolean
    ncx_list_empty (const ncx_list_t *list);

extern boolean
    ncx_string_in_list (const xmlChar *str,
			const ncx_list_t *list);

extern int32
    ncx_compare_lists (const ncx_list_t *list1,
		       const ncx_list_t *list2);

extern status_t
    ncx_copy_list (const ncx_list_t *list1,
		   ncx_list_t *list2);

extern void
    ncx_merge_list (ncx_list_t *src,
		    ncx_list_t *dest,
		    ncx_merge_t mergetyp,
		    boolean allow_dups);

/* consume a generic string list with no type checking */
extern status_t
    ncx_set_strlist (const xmlChar *liststr,
		     ncx_list_t *list);

/* consume a generic string list with base type checking */
extern status_t 
    ncx_set_list (ncx_btype_t btyp,
		  const xmlChar *strval,
		  ncx_list_t  *list);

/* 2nd pass of parsing a ncx_list_t */
extern status_t
    ncx_finish_list (typ_def_t *typdef,
		     ncx_list_t *list);

/********************** ncx_lmem_t *********************/
extern ncx_lmem_t *
    ncx_new_lmem (void);

extern void
    ncx_clean_lmem (ncx_lmem_t *lmem,
		    ncx_btype_t btyp);

extern void
    ncx_free_lmem (ncx_lmem_t *lmem,
		   ncx_btype_t btyp);

extern ncx_lmem_t *
    ncx_find_lmem (ncx_list_t *list,
		   const ncx_lmem_t *memval);

extern void
    ncx_insert_lmem (ncx_list_t *list,
		     ncx_lmem_t *memval,
		     ncx_merge_t mergetyp);

extern ncx_lmem_t *
    ncx_first_lmem (ncx_list_t *list);

/********************** ncx_binary_t *********************/
extern ncx_binary_t *
    ncx_new_binary (void);

extern void
    ncx_init_binary (ncx_binary_t *binary);

extern void
    ncx_clean_binary (ncx_binary_t *binary);

extern void
    ncx_free_binary (ncx_binary_t *binary);


/********************** ncx_appinfo_t *********************/

extern ncx_appinfo_t * 
    ncx_new_appinfo (boolean isclone);

extern void 
    ncx_free_appinfo (ncx_appinfo_t *appinfo);

extern ncx_appinfo_t *
    ncx_find_appinfo (dlq_hdr_t *appinfoQ,
		      const xmlChar *prefix,
		      const xmlChar *varname);

extern const ncx_appinfo_t *
    ncx_find_const_appinfo (const dlq_hdr_t *appinfoQ,
                            const xmlChar *prefix,
                            const xmlChar *varname);

extern const ncx_appinfo_t *
    ncx_find_next_appinfo (const ncx_appinfo_t *current,
			   const xmlChar *prefix,
			   const xmlChar *varname);

extern ncx_appinfo_t *
    ncx_find_next_appinfo2 (ncx_appinfo_t *current,
                            const xmlChar *prefix,
                            const xmlChar *varname);

extern ncx_appinfo_t *
    ncx_clone_appinfo (ncx_appinfo_t *appinfo);

extern void 
    ncx_clean_appinfoQ (dlq_hdr_t *appinfoQ);

extern status_t 
    ncx_consume_appinfo (tk_chain_t *tkc,
			 ncx_module_t  *mod,
			 dlq_hdr_t *appinfoQ);

extern status_t 
    ncx_consume_appinfo2 (tk_chain_t *tkc,
			  ncx_module_t  *mod,
			  dlq_hdr_t *appinfoQ);

extern status_t 
    ncx_resolve_appinfoQ (yang_pcb_t *pcb,
                          tk_chain_t *tkc,
			  ncx_module_t  *mod,
			  dlq_hdr_t *appinfoQ);


/********************** ncx_iffeature_t *********************/

extern ncx_iffeature_t * 
    ncx_new_iffeature (void);

extern void 
    ncx_free_iffeature (ncx_iffeature_t *iffeature);

extern void 
    ncx_clean_iffeatureQ (dlq_hdr_t *iffeatureQ);

extern ncx_iffeature_t *
    ncx_find_iffeature (dlq_hdr_t *iffeatureQ,
			const xmlChar *prefix,
			const xmlChar *name,
			const xmlChar *modprefix);


/********************** ncx_feature_t *********************/

extern ncx_feature_t * 
    ncx_new_feature (void);

extern void 
    ncx_free_feature (ncx_feature_t *feature);

extern ncx_feature_t *
    ncx_find_feature (ncx_module_t *mod,
		      const xmlChar *name);

extern ncx_feature_t *
    ncx_find_feature_que (dlq_hdr_t *featureQ,
			  const xmlChar *name);

extern void
    ncx_for_all_features (const ncx_module_t *mod,
			  ncx_feature_cbfn_t  cbfn,
			  void *cookie,
			  boolean enabledonly);

extern uint32
    ncx_feature_count (const ncx_module_t *mod,
		       boolean enabledonly);

extern boolean
    ncx_feature_enabled (const ncx_feature_t *feature);

/********************** ncx_identity_t *********************/

extern ncx_identity_t * 
    ncx_new_identity (void);

extern void 
    ncx_free_identity (ncx_identity_t *identity);

extern ncx_identity_t *
    ncx_find_identity (ncx_module_t *mod,
		       const xmlChar *name);

extern ncx_identity_t *
    ncx_find_identity_que (dlq_hdr_t *identityQ,
			   const xmlChar *name);

/********************** ncx_filptr_t *********************/

extern ncx_filptr_t *
    ncx_new_filptr (void);

extern void 
    ncx_free_filptr (ncx_filptr_t *filptr);

/********************** ncx_revhist_t *********************/


extern ncx_revhist_t * 
    ncx_new_revhist (void);

extern void 
    ncx_free_revhist (ncx_revhist_t *revhist);

extern ncx_revhist_t * 
    ncx_find_revhist (const ncx_module_t *mod,
		      const xmlChar *ver);

/********************** ncx_enum_t *********************/

extern void
    ncx_init_enum (ncx_enum_t *enu);

extern void
    ncx_clean_enum (ncx_enum_t *enu);

extern int32
    ncx_compare_enums (const ncx_enum_t *enu1,
		       const ncx_enum_t *enu2);

extern status_t
    ncx_decode_enum (const xmlChar *enumval,
		     int32 *retval,
		     boolean *retset,
		     uint32 *retlen);

extern status_t
    ncx_set_enum (const xmlChar *enumval,
		  ncx_enum_t *retenu);


/********************** ncx_bit_t *********************/

extern void
    ncx_init_bit (ncx_bit_t *bit);

extern void
    ncx_clean_bit (ncx_bit_t *bit);

extern int32
    ncx_compare_bits (const ncx_bit_t *bitone,
		      const ncx_bit_t *bittwo);

/********************** ncx_typname_t *********************/

extern ncx_typname_t *
    ncx_new_typname (void);

extern void
    ncx_free_typname (ncx_typname_t *typnam);

extern const xmlChar *
    ncx_find_typname (const typ_template_t *typ,
		      const dlq_hdr_t *que);


extern const typ_template_t *
    ncx_find_typname_type (const dlq_hdr_t *que,
			   const xmlChar *typname);

extern void
    ncx_clean_typnameQ (dlq_hdr_t *que);

/************** General NCX Utilities ******************/

extern void
    ncx_printf_indent (int32 indentcnt);

extern void
    ncx_stdout_indent (int32 indentcnt);

/* 4 internal objects for subtree filter processing */
extern obj_template_t *
    ncx_get_gen_anyxml (void);

extern obj_template_t *
    ncx_get_gen_container (void);

extern obj_template_t *
    ncx_get_gen_string (void);

extern obj_template_t *
    ncx_get_gen_empty (void);

extern obj_template_t *
    ncx_get_gen_root (void);

extern obj_template_t *
    ncx_get_gen_binary (void);

/* translate ncx_layer_t enum to a string */
extern const xmlChar *
    ncx_get_layer (ncx_layer_t  layer);


extern const xmlChar *
    ncx_get_name_segment (const xmlChar *str,
			  xmlChar  *buff,
			  uint32 buffsize);

extern boolean
    ncx_save_descr (void);

extern void
    ncx_print_errormsg (tk_chain_t *tkc,
			ncx_module_t  *mod,
			status_t     res);

extern void
    ncx_print_errormsg_ex (tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   status_t     res,
			   const char *filename,
			   uint32 linenum,
			   boolean fineoln);


extern void
    ncx_conf_exp_err (tk_chain_t  *tkc,
		      status_t result,
		      const char *expstr);

extern void
    ncx_mod_exp_err (tk_chain_t  *tkc,
		     ncx_module_t *mod,
		     status_t result,
		     const char *expstr);

extern void
    ncx_free_node (ncx_node_t nodetyp,
		   void *node);

extern ncx_data_class_t 
    ncx_get_data_class_enum (const xmlChar *str);

extern const xmlChar *
    ncx_get_data_class_str (ncx_data_class_t dataclass);

extern const xmlChar * 
    ncx_get_access_str (ncx_access_t max_access);

extern ncx_access_t
    ncx_get_access_enum (const xmlChar *str);

extern ncx_cvttyp_t
    ncx_get_cvttyp_enum (const char *str);

extern ncx_status_t
    ncx_get_status_enum (const xmlChar *str);

extern const xmlChar *
    ncx_get_status_string (ncx_status_t status);

extern status_t
    ncx_check_yang_status (ncx_status_t mystatus,
			   ncx_status_t depstatus);

extern ncx_tclass_t
    ncx_get_tclass (ncx_btype_t btyp);

extern boolean
    ncx_valid_name_ch (uint32 ch);

extern boolean
    ncx_valid_fname_ch (uint32 ch);

extern boolean
    ncx_valid_name (const xmlChar *str, 
		    uint32 len);

extern boolean
    ncx_valid_name2 (const xmlChar *str);

extern status_t
    ncx_parse_name (const xmlChar *str,
		    uint32 *len);

extern boolean
    ncx_is_true (const xmlChar *str);

extern boolean
    ncx_is_false (const xmlChar *str);


/** parse utilities **/
extern status_t 
    ncx_consume_tstring (tk_chain_t *tkc,
			 ncx_module_t  *mod,
			 const xmlChar *name,
			 ncx_opt_t opt);


extern status_t 
    ncx_consume_name (tk_chain_t *tkc,
		      ncx_module_t *mod,
		      const xmlChar *name,
		      xmlChar **namebuff,
		      ncx_opt_t opt,
		      tk_type_t  ctyp);

extern status_t 
    ncx_consume_token (tk_chain_t *tkc,
		       ncx_module_t *mod,
		       tk_type_t  ttyp);


extern ncx_errinfo_t *
    ncx_new_errinfo (void);

extern void 
    ncx_init_errinfo (ncx_errinfo_t *err);

extern void 
    ncx_clean_errinfo (ncx_errinfo_t *err);

extern void 
    ncx_free_errinfo (ncx_errinfo_t *err);

/* check if error-app-tag or error-message set */
extern boolean
    ncx_errinfo_set (const ncx_errinfo_t *errinfo);

extern status_t
    ncx_copy_errinfo (const ncx_errinfo_t *src,
		      ncx_errinfo_t *dest);

extern xmlChar *
    ncx_get_source (const xmlChar *fspec,
                    status_t *res);

extern void
    ncx_set_cur_modQ (dlq_hdr_t *que);

extern void
    ncx_reset_modQ (void);

extern void
    ncx_set_session_modQ (dlq_hdr_t *que);

extern void
    ncx_set_load_callback (ncx_load_cbfn_t cbfn);

extern void
    ncx_clear_session_modQ (void);

extern boolean
    ncx_prefix_different (const xmlChar *prefix1,
			  const xmlChar *prefix2,
			  const xmlChar *modprefix);

extern ncx_bad_data_t
    ncx_get_baddata_enum (const xmlChar *valstr);

extern const xmlChar *
    ncx_get_baddata_string (ncx_bad_data_t baddata);

extern const xmlChar *
    ncx_get_withdefaults_string (ncx_withdefaults_t withdef);

extern ncx_withdefaults_t
    ncx_get_withdefaults_enum (const xmlChar *withdefstr);

extern const xmlChar *
    ncx_get_mod_prefix (const ncx_module_t *mod);

extern const xmlChar *
    ncx_get_mod_xmlprefix (const ncx_module_t *mod);

extern int64
    ncx_get_dec64_base (const ncx_num_t *num);

extern int64
    ncx_get_dec64_fraction (const ncx_num_t *num);

extern ncx_display_mode_t
    ncx_get_display_mode_enum (const xmlChar *dmstr);

extern const xmlChar *
    ncx_get_display_mode_str (ncx_display_mode_t dmode);

extern void
    ncx_set_warn_idlen (uint32 warnlen);

extern uint32
    ncx_get_warn_idlen (void);

extern void
    ncx_set_warn_linelen (uint32 warnlen);

extern uint32
    ncx_get_warn_linelen (void);

extern void
    ncx_check_warn_idlen (tk_chain_t *tkc,
                          ncx_module_t *mod,
                          const xmlChar *id);

extern void
    ncx_check_warn_linelen (tk_chain_t *tkc,
                            ncx_module_t *mod,
                            const xmlChar *line);

extern status_t
    ncx_turn_off_warning (status_t res);

extern boolean
    ncx_warning_enabled (status_t res);

extern status_t
    ncx_get_version (xmlChar *buffer,
                     uint32 buffsize);

extern ncx_save_deviations_t *
    ncx_new_save_deviations (const xmlChar *devmodule,
                             const xmlChar *devrevision,
                             const xmlChar *devnamespace,
                             const xmlChar *devprefix);

extern void
    ncx_free_save_deviations (ncx_save_deviations_t *savedev);

extern void
    ncx_clean_save_deviationsQ (dlq_hdr_t *savedevQ);

extern void
    ncx_set_error (ncx_error_t *tkerr,
                   ncx_module_t *mod,
                   uint32 linenum,
                   uint32 linepos);


extern void
    ncx_set_temp_modQ (dlq_hdr_t *modQ);

extern dlq_hdr_t *
    ncx_get_temp_modQ (void);

extern void
    ncx_clear_temp_modQ (void);

extern ncx_display_mode_t
    ncx_get_display_mode (void);

extern const xmlChar *
    ncx_get_confirm_event_str (ncx_confirm_event_t event);

extern uint32
    ncx_copy_c_safe_str (xmlChar *buffer,
                         const xmlChar *strval);

#endif	    /* _H_ncx */
