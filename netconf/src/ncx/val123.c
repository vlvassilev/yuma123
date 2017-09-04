#include <assert.h>
#include "obj.h"
#include "val.h"
#include "val_util.h"
#include "val_get_leafref_targval.h"
#include "cli.h"
#include "val123.h"
#include "xpath_yang.h"


val_value_t* val123_deref(val_value_t* leafref_val)
{
    val_value_t* val;
    val_value_t* parent_val;
    val_value_t* root_val;

    assert(leafref_val->parent);
    parent_val=leafref_val->parent;
    while(parent_val->parent) {
        parent_val = parent_val->parent;
    }

    root_val=parent_val;

    val=val_get_leafref_targval(leafref_val, root_val);
    return val;
}

val_value_t* val123_find_match(val_value_t* haystack_root_val, val_value_t* needle_val)
{
    val_value_t* val=NULL;
    char* pathbuff;
    if(haystack_root_val==NULL) {
        return NULL;
    }
    status_t res = val_gen_instance_id(NULL, needle_val, NCX_IFMT_XPATH1, (xmlChar **) &pathbuff);
    assert(res==NO_ERR);
    if(obj_is_root(haystack_root_val->obj)) {
        /* absolute mode */
        res = xpath_find_val_target(haystack_root_val, NULL/*mod*/, pathbuff, &val);
    } else {
        /* relative mode */
        char* root_pathbuff;
        val_value_t* needle_root_val;
        needle_root_val = needle_val->parent;
        while(needle_root_val->obj != haystack_root_val->obj && needle_root_val->parent) {
            needle_root_val = needle_root_val->parent;
        }

        assert(needle_root_val->obj == haystack_root_val->obj);
        res = val_gen_instance_id(NULL, needle_root_val, NCX_IFMT_XPATH1, (xmlChar **) &root_pathbuff);
        assert(res==NO_ERR);
        assert(strlen(pathbuff)>strlen(root_pathbuff));
        res = xpath_find_val_target(haystack_root_val, obj_get_mod(haystack_root_val->obj)/*mod*/, pathbuff+strlen(root_pathbuff)+1, &val);
        free(root_pathbuff);
    }
    free(pathbuff);
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
    if(original_val->parent->obj==clone_root_val->obj) {
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

ncx_identity_t* ncx123_identity_get_first_base(const ncx_identity_t* identity)
{
    ncx_identity_base_t *base;
    base=(ncx_identity_base_t *)dlq_firstEntry(&identity->baseQ);
    if(base) {
        return base->identity;
    } else {
        return NULL;
    }
}

ncx_identity_t* ncx123_identity_get_next_base(const ncx_identity_t* identity, const ncx_identity_t *identity_base)
{
    ncx_identity_base_t *base;
    ncx_identity_base_t *next_base;
    assert(identity);
    assert(identity_base);
    base=(ncx_identity_base_t *)dlq_firstEntry(&identity->baseQ);
    assert(base);
    for(;base!=NULL;base=(ncx_identity_base_t *)dlq_nextEntry(base)) {
        if(base->identity==identity_base) {
            next_base=(ncx_identity_base_t *)dlq_nextEntry(base);
            if(next_base) {
                return next_base->identity;
            } else {
                return NULL;
            }
        }
    }
    assert(NULL); /*never found identity_base in baseQ*/
}

bool ncx123_identity_is_derived_from(const ncx_identity_t * identity, const ncx_identity_t *identity_base)
{
    ncx_identity_t * b;
    assert(identity);
    assert(identity_base);

    for(b = ncx123_identity_get_first_base(identity);
        b != NULL;
        b = ncx123_identity_get_next_base(identity, b)) {
        if(identity_base==b) {
            return TRUE;
        }
        if(ncx123_identity_is_derived_from(b,identity_base)) {
            return TRUE;
        }
    }

    return FALSE;
}

bool val123_bit_is_set(val_value_t* bits_val, const char* bit_str)
{
    ncx_lmem_t         *listmem;
    assert(bits_val);
    assert(bit_str);

    if(dlq_empty(&bits_val->v.list.memQ)) {
        return FALSE;
    }

    assert(NCX_BT_BITS == bits_val->v.list.btyp);
    for (listmem = (ncx_lmem_t *)
         dlq_firstEntry(&bits_val->v.list.memQ);
         listmem != NULL;
        listmem = (ncx_lmem_t *)dlq_nextEntry(listmem)) {
        assert(listmem->val.str);
        if(0==strcmp(listmem->val.str, bit_str)) {
            return TRUE;
        }
    }

    return FALSE;
}

/********************************************************************
* FUNCTION cli123_parse_value_instance
*
* Create a val_value_t struct for the specified parm value,
* and insert it into the parent container value
*
* ONLY CALLED FROM CLI PARSING FUNCTIONS IN ncxcli.c
* ALLOWS SCRIPT EXTENSIONS TO BE PRESENT
*
* INPUTS:
*   rcxt == runstack context to use
*   val == parent value struct to adjust
*   parm == obj_template_t descriptor for the missing parm
*   instance_id_str == instance identifier string of the parameter value
*        e.g foo or foo[name='bar']/foo etc.
*   strval == string representation of the parm value
*             (may be NULL if parm btype is NCX_BT_EMPTY
*   script == TRUE if CLI script mode
*          == FALSE if CLI plain mode
*
* OUTPUTS:
*   A new val_value_t will be inserted in the val->v.childQ or of a child
*   sub-container or list value as required to fill in the parm.
*
* RETURNS:
*   status
*********************************************************************/
status_t cli123_parse_value_instance(runstack_context_t *rcxt, val_value_t *parent_val, obj_template_t *obj, const xmlChar * instance_id_str, const xmlChar *strval, boolean script)
{
    val_value_t* temp_parent_val;
    if(obj_is_cli(obj) || (parent_val->obj->objtype!=OBJ_TYP_CONTAINER && parent_val->obj->objtype!=OBJ_TYP_LIST) ||  parent_val->obj==obj123_get_first_data_parent(obj)) {
        return cli_parse_parm(rcxt, parent_val, obj, strval, script);
    } else {
        status_t res;
        val_value_t* val;
        val_value_t* childval;
        obj_template_t* targobj;
        val_value_t* targval;
        res = val123_new_value_from_instance_id(parent_val->obj, instance_id_str, FALSE, &childval, &targobj, &targval);
        if(res!=NO_ERR) {
            return res;
        }
        if(targobj->objtype!=OBJ_TYP_LIST && targobj->objtype!=OBJ_TYP_CONTAINER) {
            res = val_set_simval_obj(targval,targobj,strval);
            if(res!=NO_ERR) {
                val_free_value(childval);
                return res;
            }
        }
        temp_parent_val = val_new_value();
        assert(temp_parent_val);
        val_init_from_template(temp_parent_val, parent_val->obj);
        val_add_child(childval,temp_parent_val);
        res = val123_merge_cplx(parent_val, temp_parent_val);
        val_free_value(temp_parent_val);
        return res;
    }
}  /* cli123_parse_value_instance */

/********************************************************************
 * FUNCTION val123_new_value_from_instance_id
 *
 * Validate an instance identifier parameter
 * Return the target object
 * Return a value struct from root/parent containing
 * all the predicate assignments in the stance identifier
 *
 * INPUTS:
 *    parent_obj == template of the context/parent node in case of
 *                  relative instance identifier, NULL in case of
 *                  absolute e.g. /.../...
 *    instance_id_str == XPath expression for the instance-identifier
 *    schemainst == TRUE if ncx:schema-instance string
 *                  FALSE if instance-identifier
 *
 * OUTPUTS:
 *    childval == address of return pointer to child of parent_obj value
 *               (first/top node) in the chain leading to targval
 *    targobj == address of return pointer to target obj template
 *               node. Only useful if targval is a simple type leaf that
 *               the user must initialize with value.
 *    targval == address of return pointer to target value
 *               node within the value subtree returned
 *
 * RETURNS:
 *   If NO_ERR:
 *     malloced value node chain with keys representing the instance-identifier
 *     from childval to the targval
 *********************************************************************/
status_t val123_new_value_from_instance_id(obj_template_t* parent_obj, const xmlChar* instance_id_str, boolean schemainst, val_value_t** childval, obj_template_t** targobj, val_value_t** targval)
{
    xpath_pcb_t           *xpathpcb;
    status_t               res;

    *targobj = NULL;
    *childval = NULL;
    *targval = NULL;

    /* get a parser block for the instance-id */
    xpathpcb = xpath_new_pcb(instance_id_str, NULL);
    assert(xpathpcb);

    /* initial parse into a token chain
     * this is only for parsing leafref paths!
     */
    res = xpath_yang_parse_path(NULL,
                                NULL,
                                schemainst?XP_SRC_SCHEMA_INSTANCEID :
                                XP_SRC_INSTANCEID,
                                xpathpcb);
    if (res != NO_ERR) {
        log_error("\nError: parse XPath target '%s' failed",
                  xpathpcb->exprstr);
        xpath_free_pcb(xpathpcb);
        return res;
    }

    /* validate against the object tree */
    res = xpath_yang_validate_path(parent_obj?obj_get_mod(parent_obj):NULL,
                                   parent_obj?parent_obj:ncx_get_gen_root(),
                                   xpathpcb,
                                   schemainst,
                                   targobj);
    if (res != NO_ERR) {
        log_error("\nError: validate XPath target '%s' failed",
                  xpathpcb->exprstr);
        xpath_free_pcb(xpathpcb);
        return res;
    }

    /* have a valid target object, so follow the
     * parser chain and build a value subtree
     * from the XPath expression
     */
    *childval = xpath_yang_make_instanceid_val(xpathpcb,
                                            &res,
                                            targval);

    xpath_free_pcb(xpathpcb);

    return res;
}

status_t val123_merge_cplx(val_value_t* dst, val_value_t* src)
{
    val_value_t* chval;
    val_value_t* match_val;
    for (chval = val_get_first_child(src);
         chval != NULL;
         chval = val_get_next_child(chval)) {

#if 0
        if(obj_is_key(chval->obj)) {
            continue;
        }
#endif
        match_val = val123_find_match(dst, chval);
        if(match_val==NULL) {
            val_add_child(val_clone(chval),dst);
        } else {
            if(typ_is_simple(match_val->btyp)) {
                val_merge(match_val, chval);
            } else {
                val123_merge_cplx(match_val, chval);
            }
        }
    }
    return NO_ERR;
}

obj_template_t* obj123_get_first_data_parent(obj_template_t* obj)
{
    obj_template_t* parent_obj;
    do {
        parent_obj = obj_get_parent(obj);
        if(parent_obj->objtype==OBJ_TYP_CONTAINER || parent_obj->objtype==OBJ_TYP_LIST) {
            break;
        }
        obj = parent_obj;
    } while(parent_obj);
    return parent_obj;
}

status_t cli123_parse_value_string(const char* cli_str, unsigned int* len, char** valstr)
{
    char* ptr = cli_str;
    *valstr=NULL;
    if(*ptr == '\'') {
        do {
            ptr++;
        } while(*ptr!='\'' && *ptr!=' ');

        if(*ptr=='\'') {
            char* buf;
            unsigned int len_wo_quotes;
            len_wo_quotes=ptr-cli_str-1;
            buf=(char*)malloc(len_wo_quotes+1);
            memcpy(buf,cli_str+1,len_wo_quotes);
            buf[len_wo_quotes]=0;
            *len = ptr-cli_str+1;
            *valstr=buf;
        } else {
            assert(0);
        }
    } else {
        /*TODO*/
        assert(0);
    }
    return NO_ERR;
}

/********************************************************************
* FUNCTION cli123_parse_parm_assignment
*
*  Parses cli parameter assignment tuple e.g. foo="bar" and creates value
*
* INPUTS:
*   obj == parent container
*   autocomp == attempt to autocomplete parameter identifiers
*   cli_str == start of the parameter identifier, not necessary 0 terminated
*
* OUTPUTS:
*   len_out == length of detected parameter identifier
*   chval_out == allocated val for the parameter[=value] expression
*
* RETURNS:
*   NO_ERR, ERR_NCX_AMBIGUOUS_CMD, other
*********************************************************************/
status_t cli123_parse_parm_assignment(obj_template_t* obj, boolean autocomp, const char* cli_str, unsigned int* len_out, val_value_t** chval_out)
{
    status_t res;
    obj_template_t* chobj;
    val_value_t* chval;
    const char* ptr;
    unsigned int len;
    char* valstr;

    *len_out = 0;
    *chval_out = NULL;
    ptr = cli_str;

    res = cli123_parse_next_child_obj_from_path(obj, autocomp, ptr, &len, &chobj);
    if(res!=NO_ERR) {
        return res;
    }
    ptr+=len;
    if(*ptr!='=') {
        return ERR_NCX_WRONG_TKVAL;
    }
    ptr++;
    res = cli123_parse_value_string(ptr, &len, &valstr);
    if(res!=NO_ERR) {
        return res;
    }
    ptr+=len;
    chval=val_new_value();
    assert(chval);
    res = val_set_simval_obj(chval,chobj,valstr);
    free(valstr);

    if(res==NO_ERR) {
        *len_out = ptr-cli_str;
        *chval_out = chval;
    }
    return res;

} /* cli123_parse_parm_assignment */

/********************************************************************
* FUNCTION cli123_parse_next_child_obj_from_path
*
*  Attempts to find child object from parent obj and instance identifier
*  in a private case a simple parameter name string.
*  Optionally autocompletion can be attempted.
*
* INPUTS:
*   obj == parent container
*   autocomp == attempt to autocomplete parameter identifiers
*   parmname == start of the parameter identifier, not necessary 0 terminated
*
* OUTPUTS:
*   len_out == length of detected parameter identifier
*   chobj_out == obj template of the detected parameter
*
* RETURNS:
*   NO_ERR, ERR_NCX_AMBIGUOUS_CMD, other
*********************************************************************/
status_t cli123_parse_next_child_obj_from_path(obj_template_t* obj, boolean autocomp, const char* parmname, unsigned int* len_out, obj_template_t** chobj_out)
{
    status_t res;
    unsigned int parmnamelen, copylen, matchcount;
    boolean gotmatch;
    const char* str;
    unsigned int len;
    obj_template_t* chobj;

    res=NO_ERR;
    len = 0;
    chobj=NULL;
    gotmatch=FALSE;

    /* check the parmname string for a terminating char */
    parmnamelen = 0;

    if (ncx_valid_fname_ch(*parmname)) {
        str = &parmname[1];
        while (*str && ncx_valid_name_ch(*str)) {
            str++;
        }
        parmnamelen = (uint32)(str - parmname);
        len = parmnamelen;

        /* check if this parameter name is in the parmset def */
        chobj = obj_find_child_str(obj, NULL,
                                   (const xmlChar *)parmname,
                                   parmnamelen);

        /* check if parm was found, try partial name if not */
        if (!chobj && autocomp) {
            matchcount = 0;
            chobj = obj_match_child_str(obj, NULL,
                                        (const xmlChar *)parmname,
                                        parmnamelen,
                                        &matchcount);
            if (chobj) {
                if (matchcount > 1) {
                    res = ERR_NCX_AMBIGUOUS_CMD;
                }
            } else {
                len = 0;
            }
        }

    }  /* else it could be a default-parm value */

    *chobj_out = chobj;
    *len_out = len;

    return res;

} /* cli123_parse_next_child_obj_from_path */


/********************************************************************
* FUNCTION val123_select_obj
*
*  Finds all child values of the specified object template. Returns
*  value of the original parent_val obj template with all descendant
*  node instances of matching obj template. e.g. selecting all in-octets
*  in /interfaces-state
*
* INPUTS:
*   parent_val == top level node to search in
*   child_obj  == obj template instances to select
*
* RETURNS:
*   NULL in case there were no matches, allocated value of identical
*   parent_val->obj containing all selected matches.
*********************************************************************/
val_value_t* val123_select_obj(val_value_t* parent_val, obj_template_t* child_obj)
{
    status_t res;
    val_value_t* match_val;
    val_value_t* parent_select_val;
    val_value_t* clone_val;

    match_val = val123_get_first_obj_instance(parent_val, child_obj);

    if(match_val==NULL) {
        return NULL;
    }
    parent_select_val = val_new_value();
    val_init_from_template(parent_select_val, parent_val->obj);
    while(match_val) {
        res = val123_clone_instance(parent_select_val, match_val, &clone_val);
        assert(res==NO_ERR);
        match_val = val123_get_next_obj_instance(parent_val, match_val);
    }
    return parent_select_val;
} /* val123_select_obj */
