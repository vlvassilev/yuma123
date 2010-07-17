/*
 * Copyright (c) 2009, Andy Bierman
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: tg2.c

  Generate turbogears 2 files for a specific YANG module

  For module foo and bar:
  The TG2 format is:

     tg2env/myproject/myproject/
        templates/
          foo.html      Genshi
          bar.html
        controllers/
          root.py
          foo.py        TG2::RootController
          bar.py
        model/
          __init__.py
          foo.py        SQLAlchemy::DeclarativeBase
          bar.py


  Mapping from YANG to SQL:

  Everything maps to a table class, even a leaf.
  Nested lists and leaf-lists need to be split out
  into their own table, inherited all the ancestor keys
  as unique columns.  Foreign keys link the tables
  to manage fate-sharing.

  Choice nodes are collapsed so the first visible descendant
  node within each case is raised to the the same level
  as the highest-level choice.  This is OK since node names
  are not allowed to collide within a choice/case meta-containers.

  Container nodes are collapsed to the child nodes, but the container
  name is retained as a prefix because child nodes of the
  container may collide with siblings of the container.
  
  Nested lists and leaf-lists are unfolded as follows:

                     list   top
                        |
             +----------+-----------+
             |          |           |
           key a     list b       leaf c
                       |
             +----------+-----------+-------+
             |          |           |       |
           key d      key e       leaf f   leaf-list g

   table: top:
       id: primary key;
       a: unique column;    
       c: column;
  
   table top_b :
       id: primary key;
       top_id: foreign key;
       top_a: unique column;
       d: unique column;
       e: unique column;
       f: column;

   table top_b_g :
       id: primary key;
       top_id: foreign key;
       top_b_id: foreign key;
       top_a: unique column;
       top_b_d: unique column;
       top_b_e: unique column;
       g: column;



*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
11mar10      abb      begun

*********************************************************************
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

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_c_util
#include "c_util.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ext
#include "ext.h"
#endif

#ifndef _H_grp
#include "grp.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_py_util
#include "py_util.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_tg2
#include "tg2.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include "tstamp.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xml_val
#include "xml_val.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xsd_util
#include "xsd_util.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yangdump
#include "yangdump.h"
#endif

#ifndef _H_yangdump_util
#include "yangdump_util.h"
#endif


#if 0

/********************************************************************
* FUNCTION write_object_entry
* 
* Generate the TG2 code to add the obj_template_t to 
* the 'ncobject' table
*
* INPUTS:
*   mod == module in progress
*   obj == object template to process
*   cp == conversion parameters to use
*   scb == session control block to use for writing
*   buff == scratch buff to use
*
*********************************************************************/
static void
    write_object_entry (const ncx_module_t *mod,
                        obj_template_t *obj,
                        const yangdump_cvtparms_t *cp,
                        ses_cb_t *scb,
                        char *buff)
{
    dlq_hdr_t              *datadefQ;
    obj_template_t         *chobj;
    const xmlChar          *name, *defval;

    if (!obj_has_name(obj)) {
        return;
    }

    name = obj_get_name(obj);
    datadefQ = obj_get_datadefQ(obj);

    ses_putstr(scb, (const xmlChar *)"\n\nINSERT INTO ncobject VALUES (");

    /* columns: ID, modname, submodname, version, name, linenum */
    write_first_tuple(scb, mod, name, obj->tkerr.linenum, buff);

    /* column: objectid */
    write_object_id(scb, obj, buff);

    /* columns: description, reference */
    write_descr_ref(scb, obj_get_description(obj),
                    obj_get_reference(obj));

    /* column: docurl */
    write_docurl(scb, mod, cp, name, obj->tkerr.linenum);

    /* column: objtyp */
    sprintf(buff, "\n    '%s', ", (const char *)obj_get_typestr(obj));
    ses_putstr(scb, (const xmlChar *)buff);

    /* column: parentid */
    if (obj->parent && !obj_is_root(obj->parent)) {
        write_object_id(scb, obj->parent, buff);
    } else {
        write_empty_col(scb);
    }

    /* column: istop */
    sprintf(buff, 
            "\n    '%u',",  
            (obj->parent && !obj_is_root(obj->parent)) ? 0 : 1);
    ses_putstr(scb, (const xmlChar *)buff);

    /* column: isdata -- config DB data only */
    sprintf(buff, "\n    '%u',", obj_is_data_db(obj) ? 1 : 0);
    ses_putstr(scb, (const xmlChar *)buff);

    /* column: typename, or groupname if uses */
    switch (obj->objtype) {
    case OBJ_TYP_LEAF:
        sprintf(buff, "\n    '%s',", 
                typ_get_name(obj->def.leaf->typdef));
        break;
    case OBJ_TYP_LEAF_LIST:
        sprintf(buff, "\n    '%s',", 
                typ_get_name(obj->def.leaflist->typdef));
        break;
    case OBJ_TYP_USES:
        sprintf(buff, "\n    '%s',", obj->def.uses->grp->name);
        break;
    default:
        sprintf(buff, "\n    '',");
    }
    ses_putstr(scb, (const xmlChar *)buff);

    /* column: augwhen */
    if (obj->augobj && obj->augobj->when &&
        obj->augobj->when->exprstr) {
        ses_putstr(scb, (const xmlChar *)"\n    '");
        write_cstring(scb, obj->augobj->when->exprstr);
        ses_putstr(scb, (const xmlChar *)"',");
    } else {
        write_empty_col(scb);
    }

    /* column: childlist */
    if (datadefQ) {
        write_object_list(scb, datadefQ);
    } else {
        write_empty_col(scb);
    }

    /* column: defval */
    defval = obj_get_default(obj);
    if (defval) {
        sprintf(buff, "\n    '%s',",  defval);  
        ses_putstr(scb, (const xmlChar *)buff);
    } else {
        write_empty_col(scb);
    }

    /* column: listkey */
    if (obj->objtype == OBJ_TYP_LIST && obj->def.list->keystr) {
        sprintf(buff, "\n    '%s',", obj->def.list->keystr);
        ses_putstr(scb, (const xmlChar *)buff);
    } else {
        write_empty_col(scb);
    }   
   
    /* columns: config, mandatory, level */
    sprintf(buff, "\n    '%u', '%u', '%u',",
            obj_get_config_flag(obj) ? 1 : 0,
            obj_is_mandatory(obj) ? 1 : 0,
            obj_get_level(obj));
    ses_putstr(scb, (const xmlChar *)buff);

    /* columns: minelements, maxelements */
    if (obj->objtype == OBJ_TYP_LEAF_LIST) {
        sprintf(buff, "\n    '%u', '%u',",
                obj->def.leaflist->minelems,
                obj->def.leaflist->maxelems);
        ses_putstr(scb, (const xmlChar *)buff);
    } else if (obj->objtype == OBJ_TYP_LIST) {
        sprintf(buff, "\n    '%u', '%u',",
                obj->def.list->minelems,
                obj->def.list->maxelems);
        ses_putstr(scb, (const xmlChar *)buff);
    } else {
        ses_putstr(scb, (const xmlChar *)"\n    '0', '0',");
    }
    
    /* columns: iscurrent, islatest **** !!!! ****/
    ses_putstr(scb, (const xmlChar *)"\n    '1', '1',");

    /* columns: created_on, updated_on */
    write_end_tstamps(scb, buff);

    /* end the VALUES clause */
    ses_putstr(scb, (const xmlChar *)");");

    /* write an entry for each child node */
    if (datadefQ) {
        for (chobj = (obj_template_t *)dlq_firstEntry(datadefQ);
             chobj != NULL;
             chobj = (obj_template_t *)dlq_nextEntry(chobj)) {
            write_object_entry(mod, chobj, cp, scb, buff);
        }
    }

}  /* write_object_entry */


/********************************************************************
* FUNCTION convert_one_module_model
* 
*  Convert the YANG or NCX module to TG2 for input to
*  the netconfcentral object dictionary database
*
* INPUTS:
*   mod == module to convert
*   cp == convert parms struct to use
*   scb == session to use for output
*
*********************************************************************/
static void
    convert_one_module_model (const ncx_module_t *mod,
                              const yangdump_cvtparms_t *cp,
                              ses_cb_t *scb)
{
    obj_template_t        *obj;

#ifdef DEBUG
    /* copy namespace ID if this is a submodule */
    if (!mod->ismod && !mod->nsid) {
        SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* write the ncobject table entries */
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (obj_is_hidden(obj)) {
            continue;
        }

        write_object_model_class(mod, obj, cp, scb, cp->buff);
    }


    ses_putchar(scb, '\n');

}   /* convert_one_module_model */

#endif


/*********     E X P O R T E D   F U N C T I O N S    **************/


/********************************************************************
* FUNCTION tg2_convert_module_model
* 
*  Convert the YANG module to TG2 code for the SQL model
*  for the YANG data nodes; used by the Yuma Tools Workbench
*  application
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
    tg2_convert_module_model (const yang_pcb_t *pcb,
                              const yangdump_cvtparms_t *cp,
                              ses_cb_t *scb)
{
    const ncx_module_t    *mod;
    const yang_node_t     *node;


    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
        return SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
    }

    /* start the python file */
    write_py_header(scb, mod, cp);

    /* convert_one_module_model(mod, cp, scb); */

    if (cp->unified && mod->ismod) {
        for (node = (const yang_node_t *)dlq_firstEntry(&mod->saveincQ);
             node != NULL;
             node = (const yang_node_t *)dlq_nextEntry(node)) {
            if (node->submod) {
                /* convert_one_module_model(node->submod, cp, scb); */
            }
        }
    }

    /* end the python file */
    write_py_footer(scb, mod);

    return NO_ERR;

}   /* tg2_convert_module_model */


/* END file tg2.c */
