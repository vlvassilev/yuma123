/*
 * Copyright (c) 2013 - 2018, Vladimir Vassilev, All Rights Reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*  FILE: val123.h

  Support functions not part of the original libyumancx API

/********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include "val.h"
#include "obj.h"
#include "cli.h"

val_value_t* val123_find_match(val_value_t* haystack_root_val, val_value_t* needle_val);
status_t val123_clone_instance(val_value_t* root_val, val_value_t* original_val, val_value_t** clone_val);
val_value_t* val123_get_first_obj_instance(val_value_t* top_val, obj_template_t* obj);
val_value_t* val123_get_next_obj_instance(val_value_t* top_val, val_value_t* cur_val);
boolean ncx123_identity_is_derived_from(const ncx_identity_t * identity, const ncx_identity_t *identity_base);
ncx_identity_t* ncx123_identity_get_first_base(const ncx_identity_t* identity);
ncx_identity_t* ncx123_identity_get_next_base(const ncx_identity_t* identity, const ncx_identity_t *identity_base);
val_value_t* val123_deref(val_value_t* leafref_val);
bool val123_bit_is_set(val_value_t* bits_val, const char* bit_str);
status_t cli123_parse_value_instance(runstack_context_t *rcxt, val_value_t *topval, obj_template_t *obj, const xmlChar * instance_id_str, const xmlChar *strval, boolean script);
status_t val123_new_value_from_instance_id(obj_template_t* parent_obj, const xmlChar* instance_id_str, boolean schemainst, val_value_t** childval, obj_template_t** targobj, val_value_t** targval);
obj_template_t* obj123_get_first_data_parent(obj_template_t* obj);
status_t cli123_parse_next_child_obj_from_path(obj_template_t* obj, boolean autocomp, const char* parmname, unsigned int* len_out, obj_template_t** chobj_out);
status_t cli123_parse_parm_assignment(obj_template_t* obj, boolean autocomp, const char* cli_str, unsigned int* len_out, val_value_t** chval_out);
status_t cli123_parse_value_string(const char* cli_str, unsigned int* len, char** valstr);
status_t val123_merge_cplx(val_value_t* dst, val_value_t* src);
val_value_t* val123_select_obj(val_value_t* parent_val, obj_template_t* child_obj);
val_value_t* val123_clone_real(val_value_t* val);
obj_template_t* obj123_get_top_uses(obj_template_t* obj);
typ_def_t* typ123_get_first_named_typdef(typ_def_t* typdef);
unsigned int
    ncx123_find_all_homonym_top_objs(dlq_hdr_t *modQ,
                             const xmlChar *objname,
                             obj_template_t **matched_objs,
                             unsigned int matched_objs_limit);
unsigned int ncx123_find_matching_identities(ncx_module_t* mod,
                 const xmlChar * qname,
                 const typ_idref_t *idref,
                 ncx_identity_t **ids,
                 unsigned int matched_ids_limit);
status_t
    val123_parse_idref_ex (ncx_module_t *mod,
                     const xmlChar *qname,
                     const typ_idref_t *idref,
                     ncx_identity_t **id);
unsigned int
    obj123_find_all_homonym_child_objs (obj_template_t *parent,
                             const xmlChar *objname,
                             obj_template_t **matched_objs,
                             unsigned int matched_objs_limit);


ncx_module_t*
    obj123_find_child_mod_from_name(obj_template_t *parent, const char* modname);
ncx_module_t*
    obj123_find_child_mod_from_prefix(obj_template_t *parent, const char* modprefix);
void
    val123_add_virtual_cb (val_value_t *val,
                      void  *cbfn);
void
    tstamp123_datetime_nsec (xmlChar *buff);
void
    ncx123_set_vtimeout_value (uint32 sec);
obj_template_t*
    val123_get_case_for_choice(obj_template_t* choicobj, val_value_t* testval);

