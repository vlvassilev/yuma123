
/*
 * Copyright (c) 2018, Vladimir Vassilev, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: yangtree.c

  Generate YANG tree output for a specific YANG module

/********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#include <xmlstring.h>
#include <xmlreader.h>
#include  "procdefs.h"
#include "c_util.h"
#include "dlq.h"
#include "ext.h"
#include "grp.h"
#include "ncx.h"
#include "ncxconst.h"
#include "ncxmod.h"
#include "obj.h"
#include "py_util.h"
#include "ses.h"
#include "yangtree.h"
#include "status.h"
#include "tstamp.h"
#include "typ.h"
#include "xmlns.h"
#include "xml_util.h"
#include "xml_val.h"
#include "xpath.h"
#include "xsd_util.h"
#include "yang.h"
#include "yangconst.h"
#include "yangdump.h"
#include "yangdump_util.h"

unsigned int get_data_obj_type_offset_max(const obj_template_t *obj)
{
    obj_template_t* chobj;
    dlq_hdr_t* childdatadefQ;
    unsigned int j;
    unsigned int type_offset;
    unsigned int type_offset_max;
    char* child_line_prefix;

    childdatadefQ = obj_get_datadefQ(obj);
    assert(childdatadefQ);
    type_offset_max=0;
    for (chobj = (obj_template_t *)dlq_firstEntry(childdatadefQ);
         chobj != NULL;
         chobj = (obj_template_t *)dlq_nextEntry(chobj)) {
        unsigned int type_offset_cur;
        /*<status>*/
        type_offset_cur=strlen("+--");
        /*<flags>*/
        if(chobj->objtype!=OBJ_TYP_CASE /*why pyang prints this for choice?!*/) {
            type_offset_cur+=snprintf(NULL,0,"%s ", obj_get_config_flag(chobj)?"rw":"ro");
        }
        /*<name>*/
        if(chobj->objtype==OBJ_TYP_CHOICE) {
            type_offset_cur+=snprintf(NULL,0,"(%s)", obj_get_name(chobj));
        } else if(chobj->objtype==OBJ_TYP_CASE) {
            type_offset_cur+=snprintf(NULL,0,":(%s)", obj_get_name(chobj));
        } else {
            type_offset_cur+=snprintf(NULL,0,"%s", obj_get_name(chobj));
        }
        /*<opts>*/
        if((chobj->objtype == OBJ_TYP_LEAF || chobj->objtype == OBJ_TYP_CHOICE || chobj->objtype == OBJ_TYP_ANYDATA || chobj->objtype == OBJ_TYP_ANYXML) && !obj_is_mandatory(chobj)) {
            /* ?  for an optional leaf, choice, anydata or anyxml */
            type_offset_cur+=strlen("?");
        } else if((chobj->objtype == OBJ_TYP_CONTAINER) && !obj_is_np_container(chobj)) {
            /* !  for a presence container */
            type_offset_cur+=strlen("!");
        } else if((chobj->objtype == OBJ_TYP_LEAF_LIST)) {
            /* *  for a leaf-list or list */
            type_offset_cur+=strlen("?");
        } else if(chobj->objtype == OBJ_TYP_LIST) {
            /* [<keys>] for a list's keys */
            obj_key_t *key;
            type_offset_cur+=strlen("* [");
            for (key = obj_first_key(chobj);
                 key != NULL;
                 key = obj_next_key(key)) {
                type_offset_cur+=snprintf(NULL,0,"%s%s",obj_get_name(key->keyobj),obj_next_key(key)?" ":"");
            }
            type_offset_cur+=strlen("]");

        }
        if((chobj->objtype == OBJ_TYP_LEAF || chobj->objtype == OBJ_TYP_LEAF_LIST) && (type_offset_max<type_offset_cur)) {
            type_offset_max=type_offset_cur;
        }
    }
    return type_offset_max;
}

void print_data_obj(const obj_template_t *obj, char* line_prefix)
{
    obj_template_t* chobj;
    dlq_hdr_t* childdatadefQ;
    int j;
    unsigned int type_offset;
    char* child_line_prefix;

    childdatadefQ = obj_get_datadefQ(obj);
    if (!childdatadefQ) {
        return;
    }

    type_offset = get_data_obj_type_offset_max(obj);

    for (chobj = (obj_template_t *)dlq_firstEntry(childdatadefQ);
         chobj != NULL;
         chobj = (obj_template_t *)dlq_nextEntry(chobj)) {
        unsigned int type_offset_cur=0;


        printf(line_prefix);
        /*<status>*/
        type_offset_cur+=printf("+--");
        /*<flags>*/
        if(chobj->objtype!=OBJ_TYP_CASE /*why pyang prints this for choice?!*/) {
            type_offset_cur+=printf("%s ", obj_get_config_flag(chobj)?"rw":"ro");
        }
        /*<name>*/
        if(chobj->objtype==OBJ_TYP_CHOICE) {
            type_offset_cur+=printf("(%s)", obj_get_name(chobj));
        } else if(chobj->objtype==OBJ_TYP_CASE) {
            type_offset_cur+=printf(":(%s)", obj_get_name(chobj));
        } else {
            type_offset_cur+=printf("%s", obj_get_name(chobj));
        }
 
        /*<opts>*/
        if((chobj->objtype == OBJ_TYP_LEAF || chobj->objtype == OBJ_TYP_CHOICE || chobj->objtype == OBJ_TYP_ANYDATA || chobj->objtype == OBJ_TYP_ANYXML) && !obj_is_mandatory(chobj)) {
            /* ?  for an optional leaf, choice, anydata or anyxml */
            type_offset_cur+=printf("?");
        } else if((chobj->objtype == OBJ_TYP_CONTAINER) && !obj_is_np_container(chobj)) {
            /* !  for a presence container */
            type_offset_cur+=printf("!");
        } else if((chobj->objtype == OBJ_TYP_LEAF_LIST)) {
            /* *  for a leaf-list or list */
            type_offset_cur+=printf("*");
        } else if(chobj->objtype == OBJ_TYP_LIST) {
            /* [<keys>] for a list's keys */
            obj_key_t *key;
            type_offset_cur+=printf("* [");
            for (key = obj_first_key(chobj);
                 key != NULL;
                 key = obj_next_key(key)) {
                type_offset_cur+=printf("%s%s",obj_get_name(key->keyobj),obj_next_key(key)?" ":"");
            }
            type_offset_cur+=printf("]");
        }
        /* /  for a top-level data node in a mounted module */
        /* @  for a top-level data node in a parent referenced module */

        if(chobj->objtype == OBJ_TYP_LIST || chobj->objtype == OBJ_TYP_CONTAINER || chobj->objtype == OBJ_TYP_CHOICE || chobj->objtype == OBJ_TYP_CASE) {
            child_line_prefix=malloc(strlen(line_prefix)+3+1);
            if(dlq_nextEntry(chobj)) {
                sprintf(child_line_prefix, "%s|  ",line_prefix);
            } else {
                sprintf(child_line_prefix, "%s   ",line_prefix);
            }
            print_data_obj(chobj, child_line_prefix);
            free(child_line_prefix);
            continue;
        }
        /*type alignment*/
        for(j=0;j<((int)type_offset-(int)type_offset_cur);j++) {
            printf(" ");
        }
        printf("  ");

        /*<type>*/
        {
            typ_def_t * typdef;
            typdef = obj_get_typdef(chobj);
            if(typdef && typdef->typenamestr) {
                if(typdef->prefix) {
                    printf(" %s:%s", typdef->prefix, typdef->typenamestr);
                } else {
                    printf(" %s", typdef->typenamestr);
                }
            }
        }
//        printf(obj_get_type_name(chobj));
//        printf(" %s",obj_get_basetype(obj));

        /*<if-features>*/
        if(!dlq_empty(&chobj->iffeatureQ)){
            ncx_iffeature_t  *iff;
            printf(" {");

            for (iff = (ncx_iffeature_t *)
                           dlq_firstEntry(&chobj->iffeatureQ);
                 iff != NULL;
                 iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

                printf("%s%s",iff->name,dlq_nextEntry(iff)?" ":"");
            }
            printf("}?");
        }
    }
}

void print_data(const ncx_module_t    *mod)
{
    const yang_node_t     *node;
    obj_template_t* obj;
    if (dlq_empty(&mod->datadefQ)) {
        return NO_ERR;
    }

    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) || obj_is_cli(obj) || obj_is_abstract(obj) || obj->objtype==OBJ_TYP_AUGMENT || obj->objtype==OBJ_TYP_RPC || obj->objtype==OBJ_TYP_NOTIF) {
            continue;
        }
        printf("\n    +--");
        printf("%s ", obj_get_config_flag(obj)?"rw":"ro");
        printf("%s", obj_get_name(obj));
        if(dlq_nextEntry(obj)) {
            print_data_obj(obj, "\n    |  ");
        } else {
            print_data_obj(obj, "\n       ");
        }
    }
}

void print_augmentations(const ncx_module_t    *mod)
{
    const yang_node_t     *node;
    obj_template_t* obj;
    if (dlq_empty(&mod->datadefQ)) {
        return NO_ERR;
    }

    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (obj->objtype!=OBJ_TYP_AUGMENT) {
            continue;
        }

        printf("\n  augment %s:", obj->def.augment->target);
        print_data_obj(obj, "\n    ");
    }
}
#if 0
        if (obj->objtype == OBJ_TYP_NOTIF) {
            return TRUE;
        }
#endif
/*********     E X P O R T E D   F U N C T I O N S    **************/


/********************************************************************
* FUNCTION yangtree_convert_module
* 
*  Convert the YANG module to YANG tree format
*
* INPUTS:
*   pcb == parser control block of module to convert
*          This is returned from ncxmod_load_module_ex
*   cp == convert parms struct to use
*   scb == session to use for output
*
* RETURNS:
*   status
*********************************************************************/
status_t
    yangtree_convert_module(const yang_pcb_t *pcb,
                              const yangdump_cvtparms_t *cp,
                              ses_cb_t *scb)
{
    const ncx_module_t    *mod;

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
        return SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
    }
    //ses_putstr(scb, "module: ");
    printf("module: %s", mod->name);

    /*draft-ietf-netmod-yang-tree-diagrams-04 sec.2*/
    print_data(mod);
    print_augmentations(mod);
#if 0
    print_rpcs();
    print_notifications();
    print_groupings();
    print_yangdata();
#endif
    return NO_ERR;

}   /* yangtree_convert_module */


/* END file yangtree.c */
