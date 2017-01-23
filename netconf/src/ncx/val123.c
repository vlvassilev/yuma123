#include <assert.h>
#include "obj.h"
#include "val.h"

val_value_t* val123_find_match(val_value_t* haystack_root_val, val_value_t* needle_val)
{
    val_value_t* val;
    char* pathbuff;
    status_t res = val_gen_instance_id(NULL, needle_val, NCX_IFMT_XPATH1, (xmlChar **) &pathbuff);
    assert(res==NO_ERR);
    res = xpath_find_val_target(haystack_root_val, NULL/*mod*/, pathbuff, &val);
    return val;
}

static status_t val123_clone_instance_ex(val_value_t* clone_root_val, val_value_t* original_val, val_value_t** return_clone_val, boolean without_non_index_children)
{
    status_t res;
    val_value_t* clone_parent_val;
    val_value_t* clone_val;
    val_index_t* index;
    val_value_t* val;

    if(obj_is_root(original_val->obj) || (original_val->parent==NULL)) {
        return ERR_NCX_INVALID_VALUE;
    }
    if(obj_is_root(original_val->parent->obj)) {
        clone_parent_val = clone_root_val;
    } else {
        res = val123_clone_instance_ex(clone_root_val, original_val->parent, &clone_parent_val, TRUE);
        if(res != NO_ERR) {
            return res;
        }
    }

    clone_val = val_clone(original_val);
    assert(clone_val);
    if(without_non_index_children) {
        clone_val = val_new_value();
        val_init_from_template(clone_val, original_val->obj);
        for(index = val_get_first_index(original_val);
            index != NULL;
            index = val_get_next_index(index)) {
            val = val_clone(index->val);
            assert(val!=NULL);
            val_add_child(val, clone_val);
        }
    } else {
        clone_val = val_clone(original_val);
    }
    val_add_child(clone_val,clone_parent_val);
    *return_clone_val = clone_val;
    return NO_ERR;
}

status_t val123_clone_instance(val_value_t* root_val, val_value_t* original_val, val_value_t** clone_val)
{
    return val123_clone_instance_ex(root_val, original_val, clone_val, FALSE);
}

obj_template_t* obj123_get_child_ancestor_of_descendant(obj_template_t* top_obj, obj_template_t* obj)
{
    obj_template_t* child_obj = obj;
    obj_template_t* last_data_obj = NULL;
    while(child_obj->parent!=NULL) {
        if(obj_is_leafy(child_obj) || child_obj->objtype==OBJ_TYP_CONTAINER || OBJ_TYP_LIST==child_obj->objtype) {
            last_data_obj=child_obj;
        }
        if(child_obj->parent==top_obj || (obj_is_root(child_obj->parent) && obj_is_root(top_obj))) {
            return last_data_obj;
        }
        child_obj=child_obj->parent;
    }
    return NULL;
}

val_value_t* val123_get_first_obj_instance(val_value_t* top_val, obj_template_t* obj)
{
    obj_template_t* child_obj;
    val_value_t* child_val;
    val_value_t* result_val=NULL;
    assert(obj);
    if(top_val==NULL) {
        return NULL;
    }
    if(top_val->obj==obj) {
        return top_val;
    }
    child_obj = obj123_get_child_ancestor_of_descendant(top_val->obj, obj);
    child_val = val_find_child(top_val, obj_get_mod_name(child_obj), obj_get_name(child_obj));
    while(child_val) {
        result_val = val123_get_first_obj_instance(child_val,obj);
        if(result_val!=NULL) {
            break;
        }
        if(child_val->obj->objtype==OBJ_TYP_LIST) {
            child_val = val_find_next_child(top_val, obj_get_mod_name(child_val->obj), obj_get_name(child_val->obj),child_val);
        } else {
            break;
        }
    };
    return result_val;
}

val_value_t* val123_get_next_obj_instance(val_value_t* top_val, val_value_t* cur_val)
{
    val_value_t* next_val;
    val_value_t* ancestor_val;

    if(top_val==cur_val) {
        return NULL;
    }
    if(OBJ_TYP_LIST==cur_val->obj->objtype) {
        next_val = val_find_next_child(cur_val->parent, obj_get_mod_name(cur_val->obj), obj_get_name(cur_val->obj),cur_val);
        if(next_val!=NULL) {
            return next_val;
        }
    }

    /*Climb up and use val123_get_first_obj_instance on every OBJ_TYP_LIST branch*/
    ancestor_val = cur_val->parent;
    while(ancestor_val!=NULL && ancestor_val!=top_val) {
        val_value_t* next_ancestor_val;
        if(ancestor_val->obj->objtype==OBJ_TYP_LIST) {
            next_ancestor_val = val_find_next_child(ancestor_val->parent, obj_get_mod_name(ancestor_val->obj), obj_get_name(ancestor_val->obj), ancestor_val);
            while(next_ancestor_val!=NULL) {
                if(next_ancestor_val!=NULL) {
                    next_val = val123_get_first_obj_instance(next_ancestor_val, cur_val->obj);
                    if(next_val!=NULL) {
                        return next_val;
                    }
                }
                next_ancestor_val = val_find_next_child(next_ancestor_val->parent, obj_get_mod_name(ancestor_val->obj), obj_get_name(ancestor_val->obj), next_ancestor_val);
            }
        }
        ancestor_val = ancestor_val->parent;
    }

    return NULL;
}

