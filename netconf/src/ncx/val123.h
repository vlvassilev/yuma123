#include "val.h"
#include "obj.h"

val_value_t* val123_find_match(val_value_t* haystack_root_val, val_value_t* needle_val);
status_t val123_clone_instance(val_value_t* root_val, val_value_t* original_val, val_value_t** clone_val);
val_value_t* val123_get_first_obj_instance(val_value_t* top_val, obj_template_t* obj);
val_value_t* val123_get_next_obj_instance(val_value_t* top_val, val_value_t* cur_val);
bool ncx123_identity_is_derived_from(const ncx_identity_t * identity, const ncx_identity_t *identity_base);
ncx_identity_t* ncx123_identity_get_first_base(const ncx_identity_t* identity);
ncx_identity_t* ncx123_identity_get_next_base(const ncx_identity_t* identity, const ncx_identity_t *identity_base);
