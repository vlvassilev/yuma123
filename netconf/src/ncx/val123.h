#include "val.h"
#include "obj.h"
#include "cli.h"

val_value_t* val123_find_match(val_value_t* haystack_root_val, val_value_t* needle_val);
status_t val123_clone_instance(val_value_t* root_val, val_value_t* original_val, val_value_t** clone_val);
val_value_t* val123_get_first_obj_instance(val_value_t* top_val, obj_template_t* obj);
val_value_t* val123_get_next_obj_instance(val_value_t* top_val, val_value_t* cur_val);
bool ncx123_identity_is_derived_from(const ncx_identity_t * identity, const ncx_identity_t *identity_base);
ncx_identity_t* ncx123_identity_get_first_base(const ncx_identity_t* identity);
ncx_identity_t* ncx123_identity_get_next_base(const ncx_identity_t* identity, const ncx_identity_t *identity_base);
val_value_t* val123_deref(val_value_t* leafref_val);
bool val123_bit_is_set(val_value_t* bits_val, const char* bit_str);
status_t cli123_parse_value_instance(runstack_context_t *rcxt, val_value_t *topval, obj_template_t *obj, const xmlChar * instance_id_str, const xmlChar *strval, boolean script);
status_t val123_create_descendant_value_chain(obj_template_t* obj, const xmlChar* instance_id_str, const xmlChar* strval, val_value_t** top_val, val_value_t** bottom_val);
obj_template_t* obj123_get_first_data_parent(obj_template_t* obj);
status_t cli123_parse_next_child_obj_from_path(obj_template_t* obj, boolean autocomp, const char* parmname, unsigned int* len_out, obj_template_t** chobj_out);
status_t cli123_parse_parm_assignment(obj_template_t* obj, boolean autocomp, const char* cli_str, unsigned int* len_out, val_value_t** chval_out);
status_t cli123_parse_value_string(char* cli_str, unsigned int* len, char** valstr);
status_t val123_merge_cplx(val_value_t* dst, val_value_t* src);
