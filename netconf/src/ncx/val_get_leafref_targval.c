#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <xmlstring.h>

#include "ncx.h"
#include "ncx_list.h"
#include "ncx_num.h"
#include "ncx_str.h"
#include "ncxconst.h"
#include "obj.h"
#include "val.h"
#include "val_parse.h"
#include "val_util.h"
#include "xml_util.h"
#include "xml_wr.h"
#include "xml_rd.h"
#include "xpath.h"
#include "xpath1.h"
#include "xpath_yang.h"
#include "yangconst.h"

val_value_t* val_get_leafref_targval_parent(val_value_t* root_val, obj_template_t* targobj)
{
    val_value_t* val;
    val_value_t* parent_val;
    if(targobj->parent!=NULL) {
        parent_val = val_get_leafref_targval_parent(root_val, targobj->parent);
        val = val_find_child(parent_val,obj_get_mod_name(targobj), obj_get_name(targobj));
    } else {
        val = val_find_child(root_val,obj_get_mod_name(targobj), obj_get_name(targobj));
    }
    return val;
}

val_value_t* val_get_leafref_targval(val_value_t *leafref_val, val_value_t *root_val)
{
    status_t res;
    val_value_t* target_val;
#if 0
    val_value_t* root_val;
    val_value_t* parent_val;
    val_value_t* val;
    obj_template_t* targobj;

    assert(leafref_val!=NULL);
    targobj = obj_get_leafref_targobj(leafref_val->obj);

    root_val = leafref_val->parent;
    assert(root_val);
    while(root_val->parent) {
        root_val = root_val->parent;
    }

    parent_val = val_get_leafref_targval_parent(root_val, targobj);
    for (val = val_get_first_child(parent_val);
         val != NULL;
         val = val_get_next_child(val)) {
        if(0==strcmp(VAL_STRING(val), VAL_STRING(leafref_val))) {
            return val;
        }
    }
    return NULL;
#else
    /*
      Workaround using the available function xpath_find_val_target.
      TODO: To resolve more complicated cases of leafrefs depending on
      leafs relevant to the leafref value needs both the
      root_val and the leafref_val
     */
#if 0
    root_val = leafref_val->parent;
    assert(root_val);
    while(root_val->parent) {
        root_val = root_val->parent; 
    }
#endif

    res = xpath_find_val_target((leafref_val->xpathpcb->exprstr[0] == '/')?root_val:leafref_val, NULL/*mod*/,leafref_val->xpathpcb->exprstr, &target_val);
    assert(res==NO_ERR);
    while(target_val) {
        char* target_str;
        target_str = val_make_sprintf_string(target_val);
        if(0==strcmp(target_str,VAL_STRING(leafref_val))) {
            free(target_str);
            break;
        }
        free(target_str);
        target_val = val_get_next_child(target_val);
    }
    return target_val;
#endif
}
