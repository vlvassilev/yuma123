/*
 * Copyright (c) 2009 - 2011, Andy Bierman
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: yang_obj.c

    YANG module parser, data-def-stmt support

        /ns1:value/ns2:value/ns3:value/...

    An obj_template_t is essentially a QName node in the
    conceptual <config> element,

    Every leaf/leaf-list definition node has a typ_def_t,
    and every value instance node has a val_value_t struct.
    This allows engine callbacks to process arbitrarily complex
    data structues with the same code.

    There are 13 types of objects:

      enum constant          has value node
      ----------------------------------------
      OBJ_TYP_ANYXML            Y (1)
      OBJ_TYP_CONTAINER         Y
      OBJ_TYP_LEAF              Y
      OBJ_TYP_LEAF_LIST         Y
      OBJ_TYP_LIST              Y
      OBJ_TYP_CHOICE            N
      OBJ_TYP_CASE              N
      OBJ_TYP_USES              N
      OBJ_TYP_REFINE            N
      OBJ_TYP_AUGMENT           N
      OBJ_TYP_RPC               N
      OBJ_TYP_RPCIO             Y (2)
      OBJ_TYP_NOTIF             N

   (1) ANYXML is not stored in the value tree as type anyxml.
       It is converted as follows:
           Complex Node -> NCX_BT_CONTAINER
           Simple Node  -> NCX_BT_STRING
           Empty Node   -> NCX_BT_EMPTY

   (2) RPCIO nodes are instantiated only within the implementation,
       to act as a container for collected parameters or results.
       It is not found under the <config> element.

   These objects are grouped as follows:
      * concrete data node objects (anyxml, container - list)
      * meta grouping constructs (choice, case) 
        and (uses, refine, augment)
      * RPC method objects (rpc, input, output)
      * notification objects (notification)

    5 Pass Validation Process
    --------------------------

    In pass 1, the source file is parsed into YANG tokens.
    String concatentation and quoted string adjustment are
    handled in this pass.

    In pass 2, the objects are parsed via yang_obj_consume_datadef.
    Syntax errors and any other static errors are reported.

    In pass 3, the object definitions are validated for correctness,
    via the yang_obj_resolve_datadefs function.  This is mixed with
    calls to yang_typ_resolve_typedefs and yang_grp_resolve_groupings.

    Uses and augments are not expanded in pass 3, so some details
    like key validation for a list cannot be done, since the
    contents may depend on the expanded uses or descendant form
    augment statement.
   
    In pass 4, groupings are completed with yang_grp_resolve_complete.
    Then all the uses-based data is cloned and placed into
    the tree, via yang_obj_resolve_uses

    In pass 5, all the augment-based data is cloned and placed into
    the tree, via yang_obj_resolve_augments

    The uses and augment objects are kept for
    XSD and other translation, and needed for internal data sharing.
    In a cloned object, a minimal amount of data is copied,
    and the rest is shadowed with back-pointers.

    For the 'uses' statement, refined objects are merged into
    the cloned tree as specified by the grouping and any
    refine statements within the uses statement.

    For the 'augment' statement, one exact clone of each augmenting
    node is placed in the target, based on the schema node target
    for the augment clause.


*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
09dec07      abb      begun; start from yang_typ.c
29nov08      abb      added when-stmt support as per yang-02
05def11      abb      update docs; fix skipped resolve_xpath bug

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_grp
#include  "grp.h"
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

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncx_appinfo
#include "ncx_appinfo.h"
#endif

#ifndef _H_ncx_feature
#include "ncx_feature.h"
#endif

#ifndef _H_ncx_list
#include "ncx_list.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xpath1
#include "xpath1.h"
#endif

#ifndef _H_xpath_yang
#include "xpath_yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yang_grp
#include "yang_grp.h"
#endif

#ifndef _H_yang_obj
#include "yang_obj.h"
#endif

#ifndef _H_yang_typ
#include "yang_typ.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define YANG_OBJ_DEBUG 1
/* #define YANG_OBJ_DEBUG_USES 1 */
/* #define YANG_OBJ_DEBUG_MEMORY 1 */
#endif



/********************************************************************
*                                                                   *
*                          M A C R O S                              *
*                                                                   *
*********************************************************************/

/* used in parser routines to decide if processing can continue
 * will exit the function if critical error or continue if not
 * In all uses, there is a new object being constructed,
 * called 'obj', which must be freed before exit
 *
 * Unless the error is considered fatal, processing
 * continues in order to validate as much of the input
 * module as possible
 */
#define CHK_OBJ_EXIT(obj, res, retres)\
    if (res != NO_ERR) {\
        if (res < ERR_LAST_SYS_ERR || res==ERR_NCX_EOF) {\
            obj_free_template(obj);\
            return res;\
        } else {\
            retres = res;\
        }\
    }


#define CHK_DEV_EXIT(dev, res, retres)\
    if (res != NO_ERR) {\
        if (res < ERR_LAST_SYS_ERR || res==ERR_NCX_EOF) {\
            obj_free_deviation(dev);\
            return res;\
        } else {\
            retres = res;\
        }\
    }


#define CHK_DEVI_EXIT(devi, res, retres)\
    if (res != NO_ERR) {\
        if (res < ERR_LAST_SYS_ERR || res==ERR_NCX_EOF) {\
            obj_free_deviate(devi);\
            return res;\
        } else {\
            retres = res;\
        }\
    }


#define SET_OBJ_CURERR(tkc, obj)                 \
    if (obj->usesobj) {\
         tkc->curerr = &obj->usesobj->tkerr;\
    } else {\
        tkc->curerr = &obj->tkerr;\
    }


/********************************************************************
*                                                                   *
*              F O R W A R D    D E C L A R A T I O N S             *
*                                                                   *
*********************************************************************/

/* local functions called recursively */
static status_t 
    consume_datadef (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     dlq_hdr_t *que,
                     obj_template_t *parent,
                     grp_template_t *grp);

static status_t 
    consume_case_datadef (yang_pcb_t *pcb,
                          tk_chain_t *tkc,
                          ncx_module_t  *mod,
                          dlq_hdr_t *que,
                          obj_template_t *parent);

static status_t 
    consume_refine (tk_chain_t *tkc,
                    ncx_module_t  *mod,
                    dlq_hdr_t *que,
                    obj_template_t *parent);

static status_t 
    consume_augment (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     dlq_hdr_t *que,
                     obj_template_t *parent,
                     grp_template_t *grp);

static status_t 
    expand_augment (yang_pcb_t *pcb,
                    tk_chain_t *tkc,
                    ncx_module_t  *mod,
                    obj_template_t *obj,
                    dlq_hdr_t *datadefQ);

static status_t 
    resolve_datadef (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     obj_template_t *testobj,
                     boolean redo);

static status_t 
    resolve_datadefs (yang_pcb_t *pcb,
                      tk_chain_t *tkc,
                      ncx_module_t  *mod,
                      dlq_hdr_t *datadefQ,
                      boolean redo);

static status_t 
    resolve_iffeatureQ (yang_pcb_t *pcb,
                        tk_chain_t *tkc,
                        ncx_module_t  *mod,
                        obj_template_t *obj);

/*************    P A R S E   F U N C T I O N S    *************/


/********************************************************************
* FUNCTION finish_config_flag
* 
* Finish the internal settings for the config-stmt
*
* INPUTS:
*   obj == object to process
*
*********************************************************************/
static void
    finish_config_flag (obj_template_t *obj)
{
    boolean flag;

    if (!(obj->flags & OBJ_FL_CONFSET)) {
        if (obj->parent && !obj_is_root(obj->parent)) {
            flag = obj_get_config_flag_deep(obj->parent);
            if (flag) {
                obj->flags |= OBJ_FL_CONFIG;
            } else {
                obj->flags &= ~OBJ_FL_CONFIG;
            }
        } else if (OBJ_DEF_CONFIG) {
            obj->flags |= OBJ_FL_CONFIG;
        }
    }

} /* finish_config_flag */


/********************************************************************
* FUNCTION add_object
* 
* Check if an object already exists, and add it if not
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* 'obj' is either deleted or added at the end of this fn
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   obj == object to add
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    add_object (tk_chain_t *tkc,
                ncx_module_t  *mod,
                dlq_hdr_t  *que,
                obj_template_t *obj)
{
    obj_template_t  *testobj;
    const xmlChar   *name;
    yang_stmt_t     *stmt;
    status_t         res;

    res = NO_ERR;
    name = obj_get_name(obj);

    if (que == &mod->datadefQ) {
        testobj = obj_find_template_top(mod, 
                                        obj_get_mod_name(obj), 
                                        name);
    } else {
        testobj = obj_find_template_test(que, 
                                         obj_get_mod_name(obj), 
                                         name);
    }
    if (testobj == NULL && 
        obj_is_top(obj) &&
        obj_is_data_db(obj)) {
        testobj = obj_find_template_all(mod, 
                                        obj_get_mod_name(obj), 
                                        name);
    }        
    if (testobj) {
        if (testobj->tkerr.mod != mod) {
            log_error("\nError: object '%s' already defined "
                      "in [sub]module '%s' at line %u",
                      name, 
                      testobj->tkerr.mod->name, 
                      testobj->tkerr.linenum);
        } else {
            log_error("\nError: object '%s' already defined at line %u",
                      name, 
                      testobj->tkerr.linenum);
        }
        res = ERR_NCX_DUP_ENTRY;
        SET_OBJ_CURERR(tkc, obj);
        ncx_print_errormsg(tkc, mod, res);
        obj_free_template(obj);
    } else {
        obj_set_ncx_flags(obj);
        dlq_enque(obj, que);  /* may have some errors */
        if (mod->stmtmode && que==&mod->datadefQ) {
            /* save top-level object order only */
            stmt = yang_new_obj_stmt(obj);
            if (stmt) {
                dlq_enque(stmt, &mod->stmtQ);
            } else {
                log_error("\nError: malloc failure for obj_stmt");
                res = ERR_INTERNAL_MEM;
                SET_OBJ_CURERR(tkc, obj);
                ncx_print_errormsg(tkc, mod, res);
            }
        }
    }
    return res;

}  /* add_object */


/********************************************************************
* FUNCTION consume_semi_lbrace
* 
* Parse the next token as a semi-colon or a left brace
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'anyxml' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == object in progress
*   done == address of boolean done flag
*
*
OUTPUTS:
*  *done will be set on exit to TRUE or FALSE
*
* RETURNS:
*   status of the operation;
*********************************************************************/
static status_t 
    consume_semi_lbrace (tk_chain_t *tkc,
                         ncx_module_t  *mod,
                         obj_template_t *obj,
                         boolean *done)
{
    const char *expstr;
    status_t res;

    /* Get the starting left brace for the sub-clauses
     * or a semi-colon to end the case-stmt
     */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        *done = TRUE;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
        obj->flags |= OBJ_FL_EMPTY;
        *done = TRUE;
        break;
    case TK_TT_LBRACE:
        *done = FALSE;
        break;
    default:
        res = ERR_NCX_WRONG_TKTYPE;
        expstr = "semi-colon or left brace";
        ncx_mod_exp_err(tkc, mod, res, expstr);
        *done = TRUE;
    }
    return res;

}  /* consume_semi_lbrace */


/********************************************************************
* FUNCTION consume_anyxml
* 
* Parse the next N tokens as an anyxml-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'anyxml' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level anyxml-stmt
*   grp == parent grp_template_t or NULL if not child of grp
*   
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_anyxml (tk_chain_t *tkc,
                    ncx_module_t  *mod,
                    dlq_hdr_t  *que,
                    obj_template_t *parent,
                    grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_leaf_t      *leaf;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, conf, flagset, mand, stat, desc, ref;
    status_t         res, retres;

    obj = NULL;
    leaf = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    conf = FALSE;
    mand = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_ANYXML);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->grp = grp;
    obj->nsid = mod->nsid;

    leaf = obj->def.leaf;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    if (leaf->typdef) {
        typ_free_typdef(leaf->typdef);
    }
    leaf->typdef = typ_get_basetype_typdef(NCX_BT_ANY);

    /* Get the mandatory anyxml name */
    res = yang_consume_id_string(tkc, mod, &leaf->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the anyxml statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);
        flagset = FALSE;

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_CONFIG)) {
            res = yang_consume_boolean(tkc, 
                                       mod,
                                       &flagset,
                                       &conf, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_CONFSET;
            if (flagset) {
                obj->flags |= OBJ_FL_CONFIG;
            } else {
                obj->flags &= ~OBJ_FL_CONFIG;
            }
        } else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
            res = yang_consume_boolean(tkc, 
                                       mod,
                                       &flagset,
                                       &mand, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_MANDSET;
            if (flagset) {
                obj->flags |= OBJ_FL_MANDATORY;
            }
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &leaf->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &leaf->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &leaf->ref,
                                     &ref, 
                                     &obj->appinfoQ);

        } else {
            res = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (leaf->name && ncx_valid_name2(leaf->name)) {
        res = add_object(tkc, mod, que, obj);
        CHK_EXIT(res, retres);
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_anyxml */


/********************************************************************
* FUNCTION consume_container
* 
* Parse the next N tokens as a container-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'container' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_container (yang_pcb_t *pcb,
                       tk_chain_t *tkc,
                       ncx_module_t  *mod,
                       dlq_hdr_t  *que,
                       obj_template_t *parent,
                       grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_container_t *con;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, pres, conf, flagset, stat, desc, ref;
    status_t         res, retres;

    obj = NULL;
    con = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    pres = FALSE;
    conf = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_CONTAINER);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->grp = grp;
    obj->nsid = mod->nsid;

    con = obj->def.container;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }
        
    /* Get the mandatory container name */
    res = yang_consume_id_string(tkc, mod, &con->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);
        flagset = FALSE;

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
            res = yang_typ_consume_typedef(pcb,
                                           tkc, 
                                           mod,
                                           con->typedefQ);
        } else if (!xml_strcmp(val, YANG_K_GROUPING)) {
            res = yang_grp_consume_grouping(pcb,
                                            tkc, 
                                            mod, 
                                            con->groupingQ, 
                                            obj);
        } else if (!xml_strcmp(val, YANG_K_MUST)) {
            res = yang_consume_must(tkc, 
                                    mod, 
                                    &con->mustQ,
                                    &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_PRESENCE)) {
            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &con->presence,
                                         &pres, 
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_CONFIG)) {
            res = yang_consume_boolean(tkc, 
                                       mod,
                                       &flagset,
                                       &conf, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_CONFSET;
            if (flagset) {
                obj->flags |= OBJ_FL_CONFIG;
            } else {
                obj->flags &= ~OBJ_FL_CONFIG;
            }
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &con->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &con->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &con->ref,
                                     &ref, 
                                     &obj->appinfoQ);
        } else {
            res = yang_obj_consume_datadef(pcb,
                                           tkc,
                                           mod,
                                           con->datadefQ, 
                                           obj);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (con->name && ncx_valid_name2(con->name)) {
        res = add_object(tkc, mod, que, obj);
        CHK_EXIT(res, retres);
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_container */


/********************************************************************
* FUNCTION consume_leaf
* 
* Parse the next N tokens as a leaf-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'leaf' keyword
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == parent grp_template_t or NULL if not child of grp
*   
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_leaf (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  dlq_hdr_t  *que,
                  obj_template_t *parent,
                  grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_leaf_t      *leaf;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, typ, units, def, conf;
    boolean          mand, stat, desc, ref, typeok, flagset;
    status_t         res, retres;

    obj = NULL;
    leaf = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    typ = FALSE;
    units = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    typeok = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_LEAF);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->grp = grp;
    obj->nsid = mod->nsid;

    leaf = obj->def.leaf;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    /* Get the mandatory leaf name */
    res = yang_consume_id_string(tkc, mod, &leaf->name);
    CHK_OBJ_EXIT(obj, res, retres);

    /* Get the mandatory left brace */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the leaf statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);
        flagset = FALSE;

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_TYPE)) {
            if (typ) {
                retres = ERR_NCX_DUP_ENTRY;
                typeok = FALSE;
                ncx_print_errormsg(tkc, mod, retres);

                /* toss the old typdef because this is a fatal
                 * error anyway, and need to skip past the new
                 * typedef; replace the old typedef!
                 */
                typ_clean_typdef(leaf->typdef);
                res = yang_typ_consume_type(pcb,
                                            tkc, 
                                            mod, 
                                            leaf->typdef);
            } else {
                typ = TRUE;
                res = yang_typ_consume_type(pcb,
                                            tkc, 
                                            mod, 
                                            leaf->typdef);
                if (res == NO_ERR) {
                    typeok = TRUE;
                }
            }
        } else if (!xml_strcmp(val, YANG_K_UNITS)) {
            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &leaf->units,
                                         &units, 
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MUST)) {
            res = yang_consume_must(tkc, 
                                    mod, 
                                    &leaf->mustQ,
                                    &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DEFAULT)) {
            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &leaf->defval,
                                         &def, &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_CONFIG)) {
            res = yang_consume_boolean(tkc, 
                                       mod,
                                       &flagset,
                                       &conf, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_CONFSET;
            if (flagset) {
                obj->flags |= OBJ_FL_CONFIG;
            } else {
                obj->flags &= ~OBJ_FL_CONFIG;
            }
        } else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
            res = yang_consume_boolean(tkc, 
                                       mod,
                                       &flagset,
                                       &mand, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_MANDSET;
            if (flagset) {
                obj->flags |= OBJ_FL_MANDATORY;
            }
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &leaf->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &leaf->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &leaf->ref,
                                     &ref, 
                                     &obj->appinfoQ);
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* check mandatory params */
    if (!typ) {
        retres = ERR_NCX_DATA_MISSING;
        ncx_mod_missing_err(tkc, 
                            mod, 
                            "leaf", 
                            "type");
    }

    /* save or delete the obj_template_t struct */
    if (leaf->name && ncx_valid_name2(leaf->name) && typeok) {
        res = add_object(tkc, mod, que, obj);
        CHK_EXIT(res, retres);
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_leaf */


/********************************************************************
* FUNCTION consume_leaflist
* 
* Parse the next N tokens as a leaf-list-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'leaf-list' keyword
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_leaflist (yang_pcb_t *pcb,
                      tk_chain_t *tkc,
                      ncx_module_t  *mod,
                      dlq_hdr_t  *que,
                      obj_template_t *parent,
                      grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_leaflist_t  *llist;
    const xmlChar   *val;
    const char      *expstr;
    xmlChar         *str;
    tk_type_t        tktyp;
    boolean          done, when, typ, units, conf;
    boolean          minel, maxel, ord, stat, desc, ref, typeok, flagset;
    status_t         res, retres;

    obj = NULL;
    llist = NULL;
    val = NULL;
    expstr = "keyword";
    str = NULL;
    done = FALSE;
    when = FALSE;
    typ = FALSE;
    units = FALSE;
    conf = FALSE;
    minel = FALSE;
    maxel = FALSE;
    ord = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    typeok = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_LEAF_LIST);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,                  
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->grp = grp;
    obj->nsid = mod->nsid;

    llist = obj->def.leaflist;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    /* Get the mandatory leaf-list name */
    res = yang_consume_id_string(tkc, mod, &llist->name);
    CHK_OBJ_EXIT(obj, res, retres);

    /* Get the mandatory left brace */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the leaf-list statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_TYPE)) {
            if (typ) {
                retres = ERR_NCX_DUP_ENTRY;
                typeok = FALSE;
                ncx_print_errormsg(tkc, mod, retres);
                typ_clean_typdef(llist->typdef);
                res = yang_typ_consume_type(pcb,
                                            tkc, 
                                            mod, 
                                            llist->typdef);
            } else {
                typ = TRUE;
                res = yang_typ_consume_type(pcb,
                                            tkc, 
                                            mod, 
                                            llist->typdef);
                if (res == NO_ERR) {
                    typeok = TRUE;
                }
            }
        } else if (!xml_strcmp(val, YANG_K_UNITS)) {
            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &llist->units,
                                         &units, 
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MUST)) {
            res = yang_consume_must(tkc, 
                                    mod, 
                                    &llist->mustQ,
                                    &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_CONFIG)) {
            flagset = FALSE;
            res = yang_consume_boolean(tkc, 
                                       mod,
                                       &flagset,
                                       &conf, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_CONFSET;
            if (flagset) {
                obj->flags |= OBJ_FL_CONFIG;
            } else {
                obj->flags &= ~OBJ_FL_CONFIG;
            }
        } else if (!xml_strcmp(val, YANG_K_MIN_ELEMENTS)) {
            res = yang_consume_uint32(tkc, 
                                      mod,
                                      &llist->minelems,
                                      &minel, 
                                      &obj->appinfoQ);
            llist->minset = TRUE;
        } else if (!xml_strcmp(val, YANG_K_MAX_ELEMENTS)) {
            res = yang_consume_max_elements(tkc, 
                                            mod,
                                            &llist->maxelems,
                                            &maxel, 
                                            &obj->appinfoQ);
            llist->maxset = TRUE;
        } else if (!xml_strcmp(val, YANG_K_ORDERED_BY)) {
            res = yang_consume_ordered_by(tkc, 
                                          mod, 
                                          &llist->ordersys,
                                          &ord, 
                                          &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &llist->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &llist->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &llist->ref,
                                     &ref, 
                                     &obj->appinfoQ);
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* check mandatory params */
    if (!typ) {
        retres = ERR_NCX_DATA_MISSING;
        ncx_mod_missing_err(tkc, 
                            mod, 
                            "leaf-list",
                            "type");
    }

    /* save or delete the obj_template_t struct */
    if (llist->name && ncx_valid_name2(llist->name) && typeok) {
        res = add_object(tkc, mod, que, obj);
        CHK_EXIT(res, retres);
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_leaflist */


/********************************************************************
* FUNCTION consume_list
* 
* Parse the next N tokens as a list-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'list' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_list (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  dlq_hdr_t  *que,
                  obj_template_t *parent,
                  grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_list_t      *list;
    obj_unique_t    *objuniq;
    const xmlChar   *val;
    const char      *expstr;
    xmlChar         *str;
    tk_type_t        tktyp;
    boolean          done, when, key, conf;
    boolean          minel, maxel, ord, stat, desc, ref, flagset, ingrp;
    status_t         res, retres;
    ncx_error_t      savetkerr;

    obj = NULL;
    list = NULL;
    val = NULL;
    expstr = "keyword";
    str = NULL;
    done = FALSE;
    when = FALSE;
    key = FALSE;
    conf = FALSE;
    minel = FALSE;
    maxel = FALSE;
    ord = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_LIST);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->grp = grp;
    obj->nsid = mod->nsid;

    list = obj->def.list;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    /* Get the mandatory list name */
    res = yang_consume_id_string(tkc, mod, &list->name);
    CHK_OBJ_EXIT(obj, res, retres);

    /* Get the mandatory left brace */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the list statements and any appinfo extensions */
    while (!done) {

        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
            res = yang_typ_consume_typedef(pcb,
                                           tkc, 
                                           mod,
                                           list->typedefQ);
        } else if (!xml_strcmp(val, YANG_K_GROUPING)) {
            res = yang_grp_consume_grouping(pcb,
                                            tkc, 
                                            mod,
                                            list->groupingQ, 
                                            obj);
        } else if (!xml_strcmp(val, YANG_K_MUST)) {
            res = yang_consume_must(tkc, 
                                    mod, 
                                    &list->mustQ,
                                    &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_KEY)) {
            ncx_set_error(&savetkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &list->keystr,
                                         &key, 
                                         &obj->appinfoQ);
            if (res == NO_ERR) {
                ncx_set_error(&list->keytkerr,
                              savetkerr.mod,
                              savetkerr.linenum,
                              savetkerr.linepos);
            }
        } else if (!xml_strcmp(val, YANG_K_UNIQUE)) {
            objuniq = obj_new_unique();
            if (!objuniq) {
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
                obj_free_template(obj);
                return res;
            }

            ncx_set_error(&objuniq->tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            res = yang_consume_strclause(tkc, 
                                         mod,
                                         &objuniq->xpath,
                                         NULL, 
                                         &obj->appinfoQ);
            if (res == NO_ERR) {
                dlq_enque(objuniq, &list->uniqueQ);
            } else {
                obj_free_unique(objuniq);
            }
        } else if (!xml_strcmp(val, YANG_K_CONFIG)) {
            flagset = FALSE;
            res = yang_consume_boolean(tkc, 
                                       mod,
                                       &flagset,
                                       &conf, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_CONFSET;
            if (flagset) {
                obj->flags |= OBJ_FL_CONFIG;
            } else {
                obj->flags &= ~OBJ_FL_CONFIG;
            }
        } else if (!xml_strcmp(val, YANG_K_MIN_ELEMENTS)) {
            res = yang_consume_uint32(tkc, 
                                      mod,
                                      &list->minelems,
                                      &minel, 
                                      &obj->appinfoQ);
            list->minset = TRUE;
        } else if (!xml_strcmp(val, YANG_K_MAX_ELEMENTS)) {
            res = yang_consume_max_elements(tkc, 
                                            mod,
                                            &list->maxelems,
                                            &maxel, 
                                            &obj->appinfoQ);
            list->maxset = TRUE;
        } else if (!xml_strcmp(val, YANG_K_ORDERED_BY)) {
            res = yang_consume_ordered_by(tkc, 
                                          mod,
                                          &list->ordersys,
                                          &ord, 
                                          &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &list->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &list->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &list->ref,
                                     &ref, 
                                     &obj->appinfoQ);
        } else {
            res = yang_obj_consume_datadef(pcb,
                                           tkc, 
                                           mod,
                                           list->datadefQ, 
                                           obj);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    ingrp = FALSE;
    if (!list->keystr && obj_get_config_flag_check(obj, &ingrp)) {
        if (!ingrp) {
            log_error("\nError: No key present for list '%s' on line %u",
                      list->name, 
                      obj->tkerr.linenum);
            retres = ERR_NCX_DATA_MISSING;
            ncx_print_errormsg(tkc, mod, retres);
        }
    }

    /* save or delete the obj_template_t struct */
    if (list->name && ncx_valid_name2(list->name)) {
        res = add_object(tkc, mod, que, obj);
        CHK_EXIT(res, retres);
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_list */


/********************************************************************
* FUNCTION consume_case
* 
* Parse the next N tokens as a case-stmt
* Create and fill in an obj_template_t struct
* and add it to the caseQ for a choice, in progress
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'case' keyword (if withcase==TRUE)
* Current token is a data-def keyword (if withcase==FALSE)
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   choic == obj_choice_t in progress, add case arm to this caseQ
*   caseQ == Que to store the obj_template_t generated
*   parent == the obj_template_t containing the 'choic' param
*             In YANG, a top-level object cannot be a 'choice',
*             so this param should not be NULL
*   withcase == TRUE if a case arm was entered and the normal
*               (full) syntax for a case arm is used
*            == FALSE if the case arm is the shorthand implied
*               kind. The start token is the limited data-def-stmt
*               keyword and only one data-def-stmt will be parsed
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_case (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  dlq_hdr_t *caseQ,
                  obj_template_t *parent,
                  boolean withcase)
{
    obj_case_t      *cas, *testcas;
    obj_template_t  *obj, *testobj, *test2obj, *casobj;
    const xmlChar   *val, *namestr;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, stat, desc, ref, anydone;
    status_t         res, retres;

    obj = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    anydone = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_CASE);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->nsid = mod->nsid;
    cas = obj->def.cas;

    /* Get the mandatory case name */
    if (withcase) {
        res = yang_consume_id_string(tkc, mod, &cas->name);
        CHK_OBJ_EXIT(obj, res, retres);

        res = consume_semi_lbrace(tkc, mod, obj, &done);
        CHK_OBJ_EXIT(obj, res, retres);
    } else {
        /* shoarthand version, just 1 data-def-stmt per case */
        anydone = TRUE;
        res = consume_case_datadef(pcb,
                                   tkc, 
                                   mod,
                                   cas->datadefQ, 
                                   obj);
        CHK_OBJ_EXIT(obj, res, retres);
        done = TRUE;
    }

    /* get the case statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &cas->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &cas->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &cas->ref,
                                     &ref, 
                                     &obj->appinfoQ);
        } else {
            res = consume_case_datadef(pcb,
                                       tkc, 
                                       mod,
                                       cas->datadefQ, 
                                       obj);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* if shorthand version, copy leaf name to case name */
    if (!withcase && retres==NO_ERR) {
        res = NO_ERR;
        testobj = (obj_template_t *)dlq_firstEntry(cas->datadefQ);
        if (testobj) {
            val = obj_get_name(testobj);
            if (val) {
                cas->name = xml_strdup(val);
                if (!cas->name) {
                    res = ERR_INTERNAL_MEM;
                }
            } else {
                res = SET_ERROR(ERR_INTERNAL_VAL);
            }
        } else {
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }
    }

    /* check case arm already defined
     * if not, check if any objects in the new case
     * are already defined in a different case
     */
    if (retres == NO_ERR) {
        res = NO_ERR;
        for (casobj = (obj_template_t *)dlq_firstEntry(caseQ);
             casobj != NULL && res==NO_ERR;
             casobj = (obj_template_t *)dlq_nextEntry(casobj)) {

            testcas = casobj->def.cas;

            if (!xml_strcmp(cas->name, testcas->name)) {
                /* case arm name already used  error */
                res = retres = ERR_NCX_DUP_ENTRY;
                log_error("\nError: case name '%s' already used"
                          " on line %u", 
                          testcas->name,
                          casobj->tkerr.linenum);
                ncx_print_errormsg(tkc, mod, retres);
            } else {
                /* check object named within case arm already used */
                for (testobj = (obj_template_t *)
                         dlq_firstEntry(cas->datadefQ);
                     testobj != NULL;
                     testobj = (obj_template_t *)
                         dlq_nextEntry(testobj)) {

                    namestr = obj_get_name(testobj);
                    test2obj = 
                        obj_find_template_test(testcas->datadefQ,
                                               obj_get_mod_name(testobj),
                                               namestr);
                    if (test2obj) {
                        /* duplicate in another case arm error */
                        res = retres = ERR_NCX_DUP_ENTRY;
                        log_error("\nError: object name '%s' already used"
                                  " in case '%s', on line %u", 
                                  namestr, 
                                  testcas->name, 
                                  test2obj->tkerr.linenum);
                        ncx_print_errormsg(tkc, mod, retres);
                    } 
                }
            }
        }
    }
        
    /* save or delete the obj_template_t struct */
    if (res==NO_ERR && cas->name && ncx_valid_name2(cas->name)) {
        obj_set_ncx_flags(obj);
        dlq_enque(obj, caseQ);
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_case */


/********************************************************************
* FUNCTION consume_choice
* 
* Parse the next N tokens as a choice-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'choice' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_choice (yang_pcb_t *pcb,
                    tk_chain_t *tkc,
                    ncx_module_t  *mod,
                    dlq_hdr_t  *que,
                    obj_template_t *parent,
                    grp_template_t *grp)
{
    obj_template_t  *obj, *testobj, *test2obj, *casobj;
    obj_choice_t    *choic;
    obj_case_t      *testcas;
    const xmlChar   *val, *namestr;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, when, def, mand, conf, stat, desc, ref, flagset;
    status_t         res, retres;

    obj = NULL;
    choic = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    when = FALSE;
    def = FALSE;
    mand = FALSE;
    conf = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_CHOICE);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->grp = grp;
    obj->nsid = mod->nsid;

    choic = obj->def.choic;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    /* Get the mandatory choice name */
    res = yang_consume_id_string(tkc, mod, &choic->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);
        flagset = FALSE;

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DEFAULT)) {
            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &choic->defval,
                                         &def, 
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
            res = yang_consume_boolean(tkc,
                                       mod,
                                       &flagset,
                                       &mand,
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_MANDSET;
            if (flagset) {
                obj->flags |= OBJ_FL_MANDATORY;
            }
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc,
                                      mod,
                                      &choic->status,
                                      &stat,
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc,
                                     mod,
                                     &choic->descr,
                                     &desc,
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc,
                                     mod,
                                     &choic->ref,
                                     &ref,
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_CONFIG)) {
            res = yang_consume_boolean(tkc,
                                       mod,
                                       &flagset,
                                       &conf,
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_CONFSET;
            if (flagset) {
                obj->flags |= OBJ_FL_CONFIG;
            } else {
                obj->flags &= OBJ_FL_CONFIG;
            }
        } else if (!xml_strcmp(val, YANG_K_CASE)) {
            res = consume_case(pcb,
                               tkc, 
                               mod,
                               choic->caseQ,
                               obj, 
                               TRUE);
        } else if (!xml_strcmp(val, YANG_K_ANYXML) ||
                   !xml_strcmp(val, YANG_K_CONTAINER) ||
                   !xml_strcmp(val, YANG_K_LEAF) ||
                   !xml_strcmp(val, YANG_K_LEAF_LIST) ||
                   !xml_strcmp(val, YANG_K_LIST)) {
            /* create an inline 1-obj case statement */
            res = consume_case(pcb,
                               tkc, 
                               mod,
                               choic->caseQ,
                               obj, 
                               FALSE);
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (choic->name && ncx_valid_name2(choic->name)) {
        res = NO_ERR;

        /* make sure sibling choice is not already defined with same name */
        if (que == &mod->datadefQ) {
            testobj = obj_find_template_top(mod, 
                                            obj_get_mod_name(obj), 
                                            choic->name);
        } else {
            testobj = obj_find_template_test(que, 
                                             obj_get_mod_name(obj),
                                             choic->name);
        }
        if (testobj) {
            if (testobj->tkerr.mod != mod) {
                log_error("\nError: object '%s' already defined "
                          "in [sub]module '%s' at line %u",
                          choic->name, 
                          testobj->tkerr.mod->name,
                          testobj->tkerr.linenum);
            } else {
                log_error("\nError: choice '%s' already defined at line %u",
                          choic->name, 
                          testobj->tkerr.linenum);
            }
            res = retres = ERR_NCX_DUP_ENTRY;
            ncx_print_errormsg(tkc, mod, retres);
        }

        /* since the choice and case nodes do not really exist,
         * the objects within each case datadefQ must not conflict
         * with any sibling nodes of the choice itself
         */
        for (casobj = (obj_template_t *)dlq_firstEntry(choic->caseQ);
             casobj != NULL;
             casobj = (obj_template_t *)dlq_nextEntry(casobj)) {

            testcas = casobj->def.cas;

            /* check object named within choice sibling objects */
            for (testobj = (obj_template_t *)
                     dlq_firstEntry(testcas->datadefQ);
                 testobj != NULL;
                 testobj = (obj_template_t *)dlq_nextEntry(testobj)) {
                    
                namestr = obj_get_name(testobj);
                test2obj = 
                    obj_find_template_test(que, 
                                           obj_get_mod_name(testobj), 
                                           namestr);
                if (test2obj) {
                    /* duplicate in the same Q as the choice */
                    res = retres = ERR_NCX_DUP_ENTRY;
                    log_error("\nError: object name '%s' in case '%s'"
                              " already used in sibling node, on line %u", 
                              namestr, 
                              obj_get_name(casobj),
                              test2obj->tkerr.linenum);
                    ncx_print_errormsg(tkc, mod, retres);
                } 
            }
        }

        if (res==NO_ERR) {
            obj_set_ncx_flags(obj);
            dlq_enque(obj, que);  /* may have some errors */        
        } else {
            obj_free_template(obj);
        }
    } else {
        /* choice name was not valid */
        obj_free_template(obj);
    }

    return retres;

}  /* consume_choice */


/********************************************************************
* FUNCTION consume_refine
* 
* Parse the next N tokens as a refinement-stmt
* Create and fill in an obj_template_t struct
* and add it to the datadefQ for a uses in progress
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'refine' keyword
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t struct that get created
*   parent == the obj_template_t containing the 'uses' param
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_refine (tk_chain_t *tkc,
                    ncx_module_t  *mod,
                    dlq_hdr_t *que,
                    obj_template_t *parent)
{
    obj_template_t  *obj;
    obj_refine_t    *refine;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, targ, desc, ref, pres, def;
    boolean          conf, mand, minel, maxel, flagset;
    status_t         res, retres;

    obj = NULL;
    refine = NULL;
    val = NULL;
    expstr = "refine target";
    done = FALSE;
    targ = FALSE;
    desc = FALSE;
    ref = FALSE;
    pres = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    minel = FALSE;
    maxel = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_REFINE);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }
    refine = obj->def.refine;

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->nsid = mod->nsid;

    /* Get the mandatory refine target */
    res = yang_consume_string(tkc, mod, &refine->target);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);
        flagset = FALSE;

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            ncx_set_error(&refine->descr_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &refine->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            ncx_set_error(&refine->ref_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &refine->ref,
                                     &ref, 
                                     &obj->appinfoQ);

        } else if (!xml_strcmp(val, YANG_K_PRESENCE)) {
            ncx_set_error(&refine->presence_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));
            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &refine->presence,
                                         &pres, 
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DEFAULT)) {
            ncx_set_error(&refine->def_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));
            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &refine->def,
                                         &def, 
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_CONFIG)) {
            flagset = FALSE;
            ncx_set_error(&refine->config_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));
            res = yang_consume_boolean(tkc, 
                                       mod, 
                                       &flagset,
                                       &conf, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_CONFSET;
            if (flagset) {
                obj->flags |= OBJ_FL_CONFIG;
            } else {
                obj->flags &= ~OBJ_FL_CONFIG;
            }
        } else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
            flagset = FALSE;
            ncx_set_error(&refine->mandatory_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));
            res = yang_consume_boolean(tkc, 
                                       mod,
                                       &flagset,
                                       &mand, 
                                       &obj->appinfoQ);
            obj->flags |= OBJ_FL_MANDSET;
            if (flagset) {
                obj->flags |= OBJ_FL_MANDATORY;
            } else {
                obj->flags &= ~OBJ_FL_MANDATORY;
            }
        } else if (!xml_strcmp(val, YANG_K_MIN_ELEMENTS)) {
            ncx_set_error(&refine->minelems_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));
            res = yang_consume_uint32(tkc, 
                                      mod,
                                      &refine->minelems,
                                      &minel, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MAX_ELEMENTS)) {
            ncx_set_error(&refine->maxelems_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));
            res = yang_consume_max_elements(tkc, 
                                            mod,
                                            &refine->maxelems,
                                            &maxel, 
                                            &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MUST)) {
            res = yang_consume_must(tkc, 
                                    mod, 
                                    &refine->mustQ,
                                    &obj->appinfoQ);
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (refine->target) {
        res = add_object(tkc, mod, que, obj);
        CHK_EXIT(res, retres);
    } else {
        obj_free_template(obj);
    }

    return res;

}  /* consume_refine */


/********************************************************************
* FUNCTION consume_uses
* 
* Parse the next N tokens as a uses-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'uses' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_uses (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  dlq_hdr_t  *que,
                  obj_template_t *parent,
                  grp_template_t *grp)
{
    obj_template_t  *obj, *testobj;
    obj_uses_t      *uses;
    grp_template_t  *impgrp;
    const xmlChar   *val;
    const char      *expstr;
    xmlChar         *str;
    yang_stmt_t     *stmt;
    tk_type_t        tktyp;
    boolean          done, when, stat, desc, ref;
    status_t         res, retres;

    obj = NULL;
    uses = NULL;
    val = NULL;
    expstr = "keyword";
    str = NULL;
    done = FALSE;
    when = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_USES);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->nsid = mod->nsid;
    obj->grp = grp;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    uses = obj->def.uses;

    /* Get the mandatory uses target [prefix:]name */
    res = yang_consume_pid_string(tkc, 
                                  mod,
                                  &uses->prefix,
                                  &uses->name);
    CHK_OBJ_EXIT(obj, res, retres);

    /* attempt to find grouping only if it is from another module */
    if (uses->prefix && xml_strcmp(uses->prefix, mod->prefix)) {
        impgrp = NULL;
        res = yang_find_imp_grouping(pcb,
                                     tkc, 
                                     mod, 
                                     uses->prefix,
                                     uses->name, 
                                     &obj->tkerr, 
                                     &impgrp);
        CHK_OBJ_EXIT(obj, res, retres);
        uses->grp = impgrp;
    }

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    expstr = "uses sub-statement";

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &uses->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &uses->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &uses->ref,
                                     &ref, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_AUGMENT)) {
            res = consume_augment(pcb,
                                  tkc, 
                                  mod, 
                                  uses->datadefQ,
                                  obj, 
                                  NULL);
        } else if (!xml_strcmp(val, YANG_K_REFINE)) {
            res = consume_refine(tkc, mod, uses->datadefQ, obj);
        } else {
           res = ERR_NCX_WRONG_TKVAL;
           ncx_mod_exp_err(tkc, mod, res, expstr);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (uses->name && ncx_valid_name2(uses->name)) {
        testobj = obj_find_template_test(que, 
                                         obj_get_mod_name(obj),
                                         uses->name);
        if (testobj) {
            log_error("\nError: object '%s' already defined at line %u",
                      uses->name, 
                      testobj->tkerr.linenum);
            retres = ERR_NCX_DUP_ENTRY;
            ncx_print_errormsg(tkc, mod, retres);
            obj_free_template(obj);
        } else {
            obj_set_ncx_flags(obj);
            dlq_enque(obj, que);  /* may have some errors */
            if (mod->stmtmode && que==&mod->datadefQ) {
                /* save top-level object order only */
                stmt = yang_new_obj_stmt(obj);
                if (stmt) {
                    dlq_enque(stmt, &mod->stmtQ);
                } else {
                    log_error("\nError: malloc failure for obj_stmt");
                    res = ERR_INTERNAL_MEM;
                    ncx_print_errormsg(tkc, mod, res);
                }
            }
        }
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_uses */


/********************************************************************
* FUNCTION consume_rpcio
* 
* Parse the next N tokens as an input-stmt or output-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'input' or 'output' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent RPC object
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_rpcio (yang_pcb_t *pcb,
                   tk_chain_t *tkc,
                   ncx_module_t  *mod,
                   dlq_hdr_t  *que,
                   obj_template_t *parent)
{
    obj_template_t  *obj, *testobj;
    obj_rpcio_t     *rpcio;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, anydone;
    status_t         res, retres;

    obj = NULL;
    rpcio = NULL;
    val = NULL;
    expstr = "typedef, grouping, or data-def keyword";
    done = FALSE;
    anydone = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_RPCIO);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->nsid = mod->nsid;
    rpcio = obj->def.rpcio;
        
    /* Get the mandatory RPC method name */
    rpcio->name = xml_strdup(TK_CUR_VAL(tkc));
    if (!rpcio->name) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        obj_free_template(obj);
        return res;
    }

    /* Get the starting left brace for the sub-clauses */
    res = ncx_consume_token(tkc, mod, TK_TT_LBRACE);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
            res = yang_typ_consume_typedef(pcb,
                                           tkc, 
                                           mod, 
                                           &rpcio->typedefQ);
        } else if (!xml_strcmp(val, YANG_K_GROUPING)) {
            res = yang_grp_consume_grouping(pcb,
                                            tkc, 
                                            mod, 
                                            &rpcio->groupingQ, 
                                            obj);
        } else {
            res = yang_obj_consume_datadef(pcb,
                                           tkc, 
                                           mod,
                                           &rpcio->datadefQ, 
                                           obj);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    testobj = obj_find_template_test(que, 
                                     obj_get_mod_name(obj),
                                     rpcio->name);
    if (testobj) {
        log_error("\nError: '%s' statement already defined at line %u",
                  rpcio->name, 
                  testobj->tkerr.linenum);
        retres = ERR_NCX_DUP_ENTRY;
        ncx_print_errormsg(tkc, mod, retres);
        obj_free_template(obj);
    } else {
        obj_set_ncx_flags(obj);
        dlq_enque(obj, que);  /* may have some errors */
    }

    return retres;

}  /* consume_rpcio */


/********************************************************************
* FUNCTION consume_augdata
* 
* Parse the next N tokens as a case-stmt
* Create and fill in an obj_template_t struct
* and add it to the datadefQ for the augment in progress
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the starting keyword of an object
* that can be refined in a uses statement:
*
*   container
*   leaf
*   leaf-list
*   list
*   choice
*   uses
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t struct that get created
*   parent == the obj_template_t containing the 'augment' param
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_augdata (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     dlq_hdr_t *que,
                     obj_template_t *parent)
{
    const xmlChar   *val;
    tk_type_t        tktyp;
    status_t         res;
    boolean          errdone;

    errdone = TRUE;
    res = NO_ERR;
    tktyp = TK_CUR_TYP(tkc);
    val = TK_CUR_VAL(tkc);

    /* check the current token type */
    if (tktyp != TK_TT_TSTRING) {
        errdone = FALSE;
        res = ERR_NCX_WRONG_TKTYPE;
    } else {
        /* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_ANYXML)) {
            res = consume_anyxml(tkc, mod, que, parent, NULL);
        } else if (!xml_strcmp(val, YANG_K_CONTAINER)) {
            res = consume_container(pcb,
                                    tkc, 
                                    mod, 
                                    que, 
                                    parent, 
                                    NULL);
        } else if (!xml_strcmp(val, YANG_K_LEAF)) {
            res = consume_leaf(pcb,
                               tkc, 
                               mod, 
                               que, 
                               parent, 
                               NULL);
        } else if (!xml_strcmp(val, YANG_K_LEAF_LIST)) {
            res = consume_leaflist(pcb,
                                   tkc, 
                                   mod, 
                                   que, 
                                   parent, 
                                   NULL);
        } else if (!xml_strcmp(val, YANG_K_LIST)) {
            res = consume_list(pcb,
                               tkc, 
                               mod, 
                               que, 
                               parent, 
                               NULL);
        } else if (!xml_strcmp(val, YANG_K_CHOICE)) {
            res = consume_choice(pcb,
                                 tkc, 
                                 mod, 
                                 que, 
                                 parent, 
                                 NULL);
        } else if (!xml_strcmp(val, YANG_K_USES)) {
            res = consume_uses(pcb,
                               tkc, 
                               mod, 
                               que, 
                               parent, 
                               NULL);
        } else {
            errdone = FALSE;
            res = ERR_NCX_WRONG_TKVAL;
        }
    }

    if (res != NO_ERR && !errdone) {
        ncx_mod_exp_err(tkc, mod, res,
                        "container, leaf, leaf-list, list, "
                        "choice, or uses keyword");
    }

    return res;

}  /* consume_augdata */


/********************************************************************
* FUNCTION consume_augment
* 
* Parse the next N tokens as an augment-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'augment' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level augment
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_augment (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     dlq_hdr_t *que,
                     obj_template_t *parent,
                     grp_template_t *grp)
{

    obj_template_t  *obj;
    obj_augment_t   *aug;
    const xmlChar   *val;
    const char      *expstr;
    xmlChar         *str;
    yang_stmt_t     *stmt;
    tk_type_t        tktyp;
    boolean          done, when, stat, desc, ref;
    boolean          rpcin, rpcout;
    status_t         res, retres;

    obj = NULL;
    aug = NULL;
    val = NULL;
    expstr = "keyword";
    str = NULL;
    done = FALSE;
    when = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    rpcin = FALSE;
    rpcout = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_AUGMENT);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->nsid = mod->nsid;
    obj->grp = grp;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    aug = obj->def.augment;

    /* Get the mandatory augment target */
    res = yang_consume_string(tkc, mod, &aug->target);
    CHK_OBJ_EXIT(obj, res, retres);

    /* Get the semi-colon or starting left brace for the sub-clauses */
    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_WHEN)) {
            res = yang_consume_when(tkc, mod, obj, &when);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &aug->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &aug->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &aug->ref,
                                     &ref, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_CASE)) {
            res = consume_case(pcb,
                               tkc, 
                               mod, 
                               &aug->datadefQ,
                               obj, 
                               TRUE);
        } else {
            res = consume_augdata(pcb,
                                  tkc, 
                                  mod, 
                                  &aug->datadefQ, 
                                  obj);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (aug->target) {
        obj_set_ncx_flags(obj);
        dlq_enque(obj, que);
        if (mod->stmtmode && que==&mod->datadefQ) {
            /* save top-level object order only */
            stmt = yang_new_obj_stmt(obj);
            if (stmt) {
                dlq_enque(stmt, &mod->stmtQ);
            } else {
                log_error("\nError: malloc failure for obj_stmt");
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
            }
        }
    } else {
        obj_free_template(obj);
    }

    return retres;

} /* consume_augment */


/********************************************************************
* FUNCTION consume_case_datadef
* 
* Parse the next N tokens as a case-data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_case_datadef (yang_pcb_t *pcb,
                          tk_chain_t *tkc,
                          ncx_module_t  *mod,
                          dlq_hdr_t *que,
                          obj_template_t *parent)
{
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          errdone;
    status_t         res;

    expstr = "container, leaf, leaf-list, list, uses,"
        "or augment keyword";
    errdone = TRUE;
    res = NO_ERR;
    tktyp = TK_CUR_TYP(tkc);
    val = TK_CUR_VAL(tkc);

    /* check the current token type */
    if (tktyp != TK_TT_TSTRING) {
        res = ERR_NCX_WRONG_TKTYPE;
        errdone = FALSE;
    } else {
        /* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_ANYXML)) {
            res = consume_anyxml(tkc, 
                                 mod, 
                                 que,
                                 parent, 
                                 NULL);
        } else if (!xml_strcmp(val, YANG_K_CONTAINER)) {
            res = consume_container(pcb,
                                    tkc, 
                                    mod, 
                                    que,
                                    parent,
                                    NULL);
        } else if (!xml_strcmp(val, YANG_K_LEAF)) {
            res = consume_leaf(pcb,
                               tkc, 
                               mod,
                               que,
                               parent,
                               NULL);
        } else if (!xml_strcmp(val, YANG_K_LEAF_LIST)) {
            res = consume_leaflist(pcb,
                                   tkc,
                                   mod,
                                   que,
                                   parent,
                                   NULL);
        } else if (!xml_strcmp(val, YANG_K_LIST)) {
            res = consume_list(pcb,
                               tkc,
                               mod,
                               que,
                               parent,
                               NULL);
        } else if (!xml_strcmp(val, YANG_K_CHOICE)) {
            res = consume_choice(pcb,
                                 tkc,
                                 mod,
                                 que,
                                 parent,
                                 NULL);
        } else if (!xml_strcmp(val, YANG_K_USES)) {
            res = consume_uses(pcb,
                               tkc, 
                               mod, 
                               que, 
                               parent, 
                               NULL);
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            errdone = FALSE;
        }
    }

    if (res != NO_ERR && !errdone) {
        ncx_mod_exp_err(tkc, mod, res, expstr);
    }

    return res;

}  /* consume_case_datadef */


/********************************************************************
* FUNCTION consume_rpc
* 
* Parse the next N tokens as an rpc-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'rpc' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_rpc (yang_pcb_t *pcb,
                 tk_chain_t *tkc,
                 ncx_module_t  *mod,
                 dlq_hdr_t  *que,
                 obj_template_t *parent,
                 grp_template_t *grp)
{
    obj_template_t        *obj, *chobj;
    const obj_template_t  *testobj;
    obj_rpc_t             *rpc;
    const xmlChar         *val;
    const char            *expstr;
    tk_type_t              tktyp;
    boolean                done, stat, desc, ref;
    status_t               res, retres;

    obj = NULL;
    rpc = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_RPC);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->nsid = mod->nsid;
    obj->grp = grp;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    rpc = obj->def.rpc;
        
    /* Get the mandatory RPC method name */
    res = yang_consume_id_string(tkc, mod, &rpc->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
            res = yang_typ_consume_typedef(pcb,
                                           tkc, 
                                           mod, 
                                           &rpc->typedefQ);
        } else if (!xml_strcmp(val, YANG_K_GROUPING)) {
            res = yang_grp_consume_grouping(pcb,
                                            tkc, 
                                            mod, 
                                            &rpc->groupingQ, 
                                            obj);
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &rpc->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &rpc->descr,
                                     &desc, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &rpc->ref,
                                     &ref, 
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_INPUT) ||
                   !xml_strcmp(val, YANG_K_OUTPUT)) {
            res = consume_rpcio(pcb,
                                tkc, 
                                mod, 
                                &rpc->datadefQ, 
                                obj);
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (rpc->name && ncx_valid_name2(rpc->name)) {

        /* make sure the rpc node has an input and output node
         * for augment purposes
         */
        testobj = obj_find_child(obj, NULL, YANG_K_INPUT);
        if (!testobj) {
            chobj = obj_new_template(OBJ_TYP_RPCIO);
            if (!chobj) {
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
                obj_free_template(obj);
                return res;
            }

            ncx_set_error(&chobj->tkerr,
                          mod,
                          obj->tkerr.linenum,
                          obj->tkerr.linepos);

            chobj->parent = obj;

            chobj->def.rpcio->name = xml_strdup(YANG_K_INPUT);
            if (!chobj->def.rpcio->name) {
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
                obj_free_template(chobj);
                obj_free_template(obj);
                return res;
            }
            obj_set_ncx_flags(chobj);
            dlq_enque(chobj, &obj->def.rpc->datadefQ);
        }

        testobj = obj_find_child(obj, NULL, YANG_K_OUTPUT);
        if (!testobj) {
            chobj = obj_new_template(OBJ_TYP_RPCIO);
            if (!chobj) {
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
                obj_free_template(obj);
                return res;
            }

            ncx_set_error(&chobj->tkerr,
                          mod,
                          obj->tkerr.linenum,
                          obj->tkerr.linepos);

            chobj->parent = obj;

            chobj->def.rpcio->name = xml_strdup(YANG_K_OUTPUT);
            if (!chobj->def.rpcio->name) {
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
                obj_free_template(chobj);
                obj_free_template(obj);
                return res;
            }
            obj_set_ncx_flags(chobj);
            dlq_enque(chobj, &obj->def.rpc->datadefQ);
        }
        
        res = add_object(tkc, mod, que, obj);
        CHK_EXIT(res, retres);
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_rpc */


/********************************************************************
* FUNCTION consume_notif
* 
* Parse the next N tokens as a notification-stmt
* Create and fill in an obj_template_t struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'notification' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == Q to hold the obj_template_t that gets created
*   parent == parent object or NULL if top-level
*   grp == parent grp_template_t or NULL if not child of grp
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_notif (yang_pcb_t *pcb,
                   tk_chain_t *tkc,
                   ncx_module_t  *mod,
                   dlq_hdr_t  *que,
                   obj_template_t *parent,
                   grp_template_t *grp)
{
    obj_template_t  *obj;
    obj_notif_t     *notif;
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          done, stat, desc, ref;
    status_t         res, retres;

    obj = NULL;
    notif = NULL;
    val = NULL;
    expstr = "keyword";
    done = FALSE;
    stat = FALSE;
    desc = FALSE;
    ref = FALSE;
    res = NO_ERR;
    retres = NO_ERR;

    /* Get a new obj_template_t to fill in */
    obj = obj_new_template(OBJ_TYP_NOTIF);
    if (!obj) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    ncx_set_error(&obj->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    obj->parent = parent;
    obj->grp = grp;
    obj->nsid = mod->nsid;
    if (que == &mod->datadefQ) {
        obj->flags |= (OBJ_FL_TOP | OBJ_FL_CONFSET | OBJ_FL_CONFIG);
    }

    notif = obj->def.notif;
        
    /* Get the mandatory RPC method name */
    res = yang_consume_id_string(tkc, mod, &notif->name);
    CHK_OBJ_EXIT(obj, res, retres);

    res = consume_semi_lbrace(tkc, mod, obj, &done);
    CHK_OBJ_EXIT(obj, res, retres);

    /* get the container statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_template(obj);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &obj->appinfoQ);
            CHK_OBJ_EXIT(obj, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_TYPEDEF)) {
            res = yang_typ_consume_typedef(pcb,
                                           tkc, 
                                           mod, 
                                           &notif->typedefQ);
        } else if (!xml_strcmp(val, YANG_K_GROUPING)) {
            res = yang_grp_consume_grouping(pcb,
                                            tkc, 
                                            mod, 
                                            &notif->groupingQ, 
                                            obj);
        } else if (!xml_strcmp(val, YANG_K_IF_FEATURE)) {
            res = yang_consume_iffeature(tkc, 
                                         mod, 
                                         &obj->iffeatureQ,
                                         &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_STATUS)) {
            res = yang_consume_status(tkc, 
                                      mod, 
                                      &notif->status,
                                      &stat, 
                                      &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc,
                                     mod, 
                                     &notif->descr,
                                     &desc,
                                     &obj->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc,
                                     mod,
                                     &notif->ref,
                                     &ref,
                                     &obj->appinfoQ);
        } else {
            res = consume_datadef(pcb,
                                  tkc,
                                  mod,
                                  &notif->datadefQ,
                                  obj,
                                  NULL);
        }
        CHK_OBJ_EXIT(obj, res, retres);
    }

    /* save or delete the obj_template_t struct */
    if (notif->name && ncx_valid_name2(notif->name)) {
        res = add_object(tkc, mod, que, obj);
        CHK_EXIT(res, retres);
    } else {
        obj_free_template(obj);
    }

    return retres;

}  /* consume_notif */


/********************************************************************
* FUNCTION apply_object_deviations
* 
* Apply the Q of deviations to the specified object
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   targobj == target object to check for deviations pending
*   deviation == deviation to apply
*   deleted == address of return deleted flag
*
* OUTPUTS:
*   *deleted == TRUE if the targobj was deleted and freed
*               FALSE if not
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    apply_object_deviations (yang_pcb_t *pcb,
                             tk_chain_t *tkc,
                             ncx_module_t  *mod,
                             obj_template_t *targobj,
                             obj_deviation_t *deviation,
                             boolean *deleted)
{
    obj_deviate_t      *devi;
    xmlChar           **strarg;
    dlq_hdr_t          *targQ, datadefQ;
    xpath_pcb_t        *must, *targmust;
    obj_unique_t       *unique, *targunique;
    status_t            res;
    boolean             retest, done;

    *deleted = FALSE;
    res = NO_ERR;
    retest = FALSE;
    done = FALSE;
    strarg = NULL;
    dlq_createSQue(&datadefQ);

    for (devi = (obj_deviate_t *)
             dlq_firstEntry(&deviation->deviateQ);
         devi != NULL && res == NO_ERR && !done;
         devi = (obj_deviate_t *)dlq_nextEntry(devi)) {

        switch (devi->arg) {
        case OBJ_DARG_NOT_SUPPORTED:
            /* make sure not deleting a key leaf */
            if (obj_is_key(targobj)) {
                res = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: cannot remove key leaf %s:%s",
                          obj_get_mod_name(targobj),
                          obj_get_name(targobj));
                tkc->curerr = &devi->tkerr;
                ncx_print_errormsg(tkc, mod, res);
                continue;
            }

            /* remove the node and toss it out */
#ifdef YANG_OBJ_DEBUG
            if (LOGDEBUG2) {
                log_debug2("\napply_dev: mark target obj %s:%s for removal",
                           obj_get_mod_name(targobj),
                           obj_get_name(targobj));
            }
#endif
            targobj->flags |= OBJ_FL_DELETED;
            *deleted = TRUE;
            done = TRUE;
            break;
        case OBJ_DARG_ADD:
        case OBJ_DARG_REPLACE:
        case OBJ_DARG_DELETE:
            /* type-stmt */
            if (devi->typdef) {
                /* only allowed arg is replace, so
                 * replace the typdef; minor alteration :-) 
                 */
                retest = TRUE;
#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG3) {
                    log_debug3("\napply_dev: replacing type in "
                               "target obj %s:%s",
                               obj_get_mod_name(targobj),
                               obj_get_name(targobj));
                }
#endif
                switch (targobj->objtype) {
                case OBJ_TYP_ANYXML:
                case OBJ_TYP_LEAF:
                    typ_free_typdef(targobj->def.leaf->typdef);
                    targobj->def.leaf->typdef = devi->typdef;
                    devi->typdef = NULL;
                    break;
                case OBJ_TYP_LEAF_LIST:
                    typ_free_typdef(targobj->def.leaflist->typdef);
                    targobj->def.leaflist->typdef = devi->typdef;
                    devi->typdef = NULL;
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
            }

            /* units-stmt */
            if (devi->units) {
                switch (targobj->objtype) {
                case OBJ_TYP_LEAF:
                    strarg = &targobj->def.leaf->units;
                    break;
                case OBJ_TYP_LEAF_LIST:
                    strarg = &targobj->def.leaflist->units;
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
                switch (devi->arg) {
                case OBJ_DARG_ADD:
#ifdef YANG_OBJ_DEBUG
                    if (LOGDEBUG3) {
                        log_debug3("\napply_dev: adding units to "
                                   "target obj %s:%s",
                                   obj_get_mod_name(targobj),
                                   obj_get_name(targobj));
                    }
#endif
                    if (*strarg != NULL) {
                        SET_ERROR(ERR_INTERNAL_VAL);
                        m__free(*strarg);
                    }
                    *strarg = devi->units;
                    devi->units = NULL;
                    break;
                case OBJ_DARG_DELETE:
#ifdef YANG_OBJ_DEBUG
                    if (LOGDEBUG3) {
                        log_debug3("\napply_dev: deleting units in "
                                   "target obj %s:%s",
                                   obj_get_mod_name(targobj),
                                   obj_get_name(targobj));
                    }
#endif
                    if (*strarg == NULL) {
                        SET_ERROR(ERR_INTERNAL_VAL);
                    }
                    m__free(*strarg);
                    *strarg = NULL;
                    break;
                case OBJ_DARG_REPLACE:
                    if (*strarg == NULL) {
                        SET_ERROR(ERR_INTERNAL_VAL);
                    } else {
#ifdef YANG_OBJ_DEBUG
                        if (LOGDEBUG3) {
                            log_debug3("\napply_dev: replacing units in "
                                       "target obj %s:%s",
                                       obj_get_mod_name(targobj),
                                       obj_get_name(targobj));
                        }
#endif
                        m__free(*strarg);
                        *strarg = devi->units;
                        devi->units = NULL;
                    }
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
            }

            /* default-stmt */
            if (devi->defval) {
                retest = TRUE;
                switch (targobj->objtype) {
                case OBJ_TYP_LEAF:
                    strarg = &targobj->def.leaf->defval;
                    break;
                case OBJ_TYP_CHOICE:
                    strarg = &targobj->def.choic->defval;
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
                switch (devi->arg) {
                case OBJ_DARG_ADD:
                    if (*strarg != NULL) {
                        SET_ERROR(ERR_INTERNAL_VAL);
                    } else {
#ifdef YANG_OBJ_DEBUG
                        if (LOGDEBUG3) {
                            log_debug3("\napply_dev: adding default to "
                                       "target obj %s:%s",
                                       obj_get_mod_name(targobj),
                                       obj_get_name(targobj));
                        }
#endif
                        *strarg = devi->defval;
                        devi->defval = NULL;
                    }
                    break;
                case OBJ_DARG_DELETE:
                    if (*strarg == NULL) {
                        SET_ERROR(ERR_INTERNAL_VAL);
                    } else {
#ifdef YANG_OBJ_DEBUG
                        if (LOGDEBUG3) {
                            log_debug3("\napply_dev: deleting default in "
                                       "target obj %s:%s",
                                       obj_get_mod_name(targobj),
                                       obj_get_name(targobj));
                        }
#endif
                        m__free(*strarg);
                        *strarg = NULL;
                    }
                    break;
                case OBJ_DARG_REPLACE:
                    if (*strarg == NULL) {
                        SET_ERROR(ERR_INTERNAL_VAL);
                    } else {
#ifdef YANG_OBJ_DEBUG
                        if (LOGDEBUG3) {
                            log_debug3("\napply_dev: replacing default in "
                                       "target obj %s:%s",
                                       obj_get_mod_name(targobj),
                                       obj_get_name(targobj));
                        }
#endif
                        m__free(*strarg);
                        *strarg = devi->units;
                        devi->units = NULL;
                    }
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
            }

            /* config-stmt */
            if (devi->config_tkerr.mod) {
                retest = TRUE;
#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG3) {
                    log_debug3("\napply_dev: replacing config-stmt in "
                               "target obj %s:%s",
                               obj_get_mod_name(targobj),
                               obj_get_name(targobj));
                }
#endif

                targobj->flags |= OBJ_FL_CONFSET;
                if (devi->config) {
                    targobj->flags |= OBJ_FL_CONFIG;
                } else {
                    targobj->flags &= ~OBJ_FL_CONFIG;
                }
            }

            /* mandatory-stmt */
            if (devi->mandatory_tkerr.mod) {
                retest = TRUE;

#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG3) {
                    log_debug3("\napply_dev: replacing mandatory-stmt in "
                               "target obj %s:%s",
                               obj_get_mod_name(targobj),
                               obj_get_name(targobj));
                }
#endif

                targobj->flags |= OBJ_FL_MANDSET;
                if (devi->mandatory) {
                    targobj->flags |= OBJ_FL_MANDATORY;
                } else {
                    targobj->flags &= ~OBJ_FL_MANDATORY;
                }
            }

            /* min-elements-stmt */
            if (devi->minelems_tkerr.mod) {
                retest = TRUE;

#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG3) {
                    log_debug3("\napply_dev: replacing min-elements in "
                               "target obj %s:%s",
                               obj_get_mod_name(targobj),
                               obj_get_name(targobj));
                }
#endif

                switch (targobj->objtype) {
                case OBJ_TYP_LIST:
                    targobj->def.list->minelems = devi->minelems;
                    break;
                case OBJ_TYP_LEAF_LIST:
                    targobj->def.leaflist->minelems = devi->minelems;
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
            }

            /* max-elements-stmt */
            if (devi->maxelems_tkerr.mod) {
                retest = TRUE;

#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG3) {
                    log_debug3("\napply_dev: replacing max-elements in "
                               "target obj %s:%s",
                               obj_get_mod_name(targobj),
                               obj_get_name(targobj));
                }
#endif

                switch (targobj->objtype) {
                case OBJ_TYP_LIST:
                    targobj->def.list->maxelems = devi->maxelems;
                    break;
                case OBJ_TYP_LEAF_LIST:
                    targobj->def.leaflist->maxelems = devi->maxelems;
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
            }

            /* must-stmt */
            if (!dlq_empty(&devi->mustQ)) {
                targQ = obj_get_mustQ(targobj);
                if (!targQ) {
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
                switch (devi->arg) {
                case OBJ_DARG_ADD:
#ifdef YANG_OBJ_DEBUG
                    if (LOGDEBUG3) {
                        log_debug3("\napply_dev: adding must-stmt(s) to "
                                   "target obj %s:%s",
                                   obj_get_mod_name(targobj),
                                   obj_get_name(targobj));
                    }
#endif
                    dlq_block_enque(&devi->mustQ, targQ);
                    break;
                case OBJ_DARG_DELETE:
#ifdef YANG_OBJ_DEBUG
                    if (LOGDEBUG3) {
                        log_debug3("\napply_dev: removing must-stmt(s) from "
                                   "target obj %s:%s",
                                   obj_get_mod_name(targobj),
                                   obj_get_name(targobj));
                    }
#endif
                    for (must = (xpath_pcb_t *)dlq_firstEntry(&devi->mustQ);
                         must != NULL;
                         must = (xpath_pcb_t *)dlq_nextEntry(must)) {

                        targmust = xpath_find_pcb(targQ,
                                                  must->exprstr);
                        if (targmust) {
                            dlq_remove(targmust);
                            xpath_free_pcb(targmust);
                        } else {
                            res = SET_ERROR(ERR_INTERNAL_VAL);
                        }
                    }
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                }
            }

            /* unique-stmt */
            if (!dlq_empty(&devi->uniqueQ)) {
                if (targobj->objtype != OBJ_TYP_LIST) {
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
                targQ = &targobj->def.list->uniqueQ;
                if (!targQ) {
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
                switch (devi->arg) {
                case OBJ_DARG_ADD:
#ifdef YANG_OBJ_DEBUG
                    if (LOGDEBUG3) {
                        log_debug3("\napply_dev: adding unique-stmt(s) to "
                                   "target obj %s:%s",
                                   obj_get_mod_name(targobj),
                                   obj_get_name(targobj));
                    }
#endif
                    dlq_block_enque(&devi->uniqueQ, targQ);
                    break;
                case OBJ_DARG_DELETE:
#ifdef YANG_OBJ_DEBUG
                    if (LOGDEBUG3) {
                        log_debug3("\napply_dev: removing unique-stmt(s) from "
                                   "target obj %s:%s",
                                   obj_get_mod_name(targobj),
                                   obj_get_name(targobj));
                    }
#endif
                    for (unique = (obj_unique_t *)
                             dlq_firstEntry(&devi->uniqueQ);
                         unique != NULL;
                         unique = (obj_unique_t *)dlq_nextEntry(unique)) {

                        targunique = obj_find_unique(targQ, unique->xpath);
                        if (targunique) {
                            dlq_remove(targunique);
                            obj_free_unique(targunique);
                        } else {
                            res = SET_ERROR(ERR_INTERNAL_VAL);
                        }
                    }
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
                break;
            }
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    if (res == NO_ERR && retest && !deleted) {
#ifdef YANG_OBJ_DEBUG
        if (LOGDEBUG3) {
            log_debug3("\nRechecking %s:%s after "
                       "applying deviation(s)",
                       obj_get_mod_name(targobj),
                       obj_get_name(targobj));
        }
#endif
        res = resolve_datadef(pcb,
                              tkc, 
                              mod, 
                              targobj, 
                              TRUE);
    }

    return res;

}  /* apply_object_deviations */


/********************************************************************
* FUNCTION apply_all_object_deviations
* 
* Apply all the deviations to the specified object
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    apply_all_object_deviations (yang_pcb_t *pcb,
                                 tk_chain_t *tkc,
                                 ncx_module_t *mod)
{
    obj_deviation_t    *deviation;
    obj_template_t     *parentobj, *targobj;
    status_t            res, retres;
    boolean             deleted;

    res = NO_ERR;
    retres = NO_ERR;
    targobj = NULL;

    for (deviation = (obj_deviation_t *)
             dlq_firstEntry(&mod->deviationQ);
         deviation != NULL && res == NO_ERR;
         deviation = (obj_deviation_t *)
             dlq_nextEntry(deviation)) {

        /* make sure deviation is for this module */
        if (xml_strcmp(deviation->targmodname, mod->name)) {
            continue;
        }

        if (deviation->targobj == NULL) {
            continue;
        }

        /* make sure not already processed
         * all the deviate structs from this deviation
         * have been moved to the object deviate Q
         */
         deleted = FALSE;
         parentobj = deviation->targobj->parent;
         res = apply_object_deviations(pcb,
                                       tkc, 
                                       mod,
                                       deviation->targobj,
                                       deviation,
                                       &deleted);
         CHK_EXIT(res, retres);
    }

    if (!pcb->stmtmode) {
        /* don't need these anymore for agent */
        while (!dlq_empty(&mod->deviationQ)) {
            deviation = (obj_deviation_t *)
                dlq_deque(&mod->deviationQ);
            obj_free_deviation(deviation);
        }
    }

    return retres;

}  /* apply_all_object_deviations */


/********************************************************************
* FUNCTION check_deviate_collision
* 
* Check if the specified obj_deviate_t stmt would
* overlap with any of the existing deviate-stmts
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   devi == current obj_deviate_t to validate against the rest
*   deviateQ == Q of obj_deviate_t structs to check against
*        
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_deviate_collision (tk_chain_t *tkc,
                             ncx_module_t  *mod,
                             obj_deviate_t *devi,
                             dlq_hdr_t *deviateQ)
{
    obj_deviate_t     *testdevi;
    status_t           res;

    res = NO_ERR;

    /* check valid deviation-stmt syntax */
    if (devi->arg != OBJ_DARG_NOT_SUPPORTED) {
        for (testdevi = (obj_deviate_t *)dlq_firstEntry(deviateQ);
             testdevi != NULL;
             testdevi = (obj_deviate_t *)dlq_nextEntry(testdevi)) {

            /* check the deviateQ to see if a 'not-supported' clause
             * already entered; if so, call it a fatal error
             */
            if (testdevi->arg == OBJ_DARG_NOT_SUPPORTED) {
                res = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: not-supported deviate-stmt "
                          "already entered on line %u",
                          testdevi->tkerr.linenum);
                tkc->curerr = &devi->tkerr;
                ncx_print_errormsg(tkc, mod, res);
            } else {
                /* make sure none of the same sub-stmts are
                 * touched in these 2 deviate structs
                 */
                if (devi->typdef && testdevi->typdef) {
                    res = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'type' deviate-stmt "
                              "already entered on line %u",
                              testdevi->tkerr.linenum);
                    tkc->curerr = &devi->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                }

                if (devi->units && testdevi->units) {
                    res = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'units' deviate-stmt "
                              "already entered on line %u",
                              testdevi->tkerr.linenum);
                    tkc->curerr = &devi->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                }

                if (devi->defval && testdevi->defval) {
                    res = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'default' deviate-stmt "
                              "already entered on line %u",
                              testdevi->tkerr.linenum);
                    tkc->curerr = &devi->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                }

                if (devi->config_tkerr.mod && 
                    testdevi->config_tkerr.mod) {
                    res = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'config' deviate-stmt "
                              "already entered on line %u",
                              testdevi->tkerr.linenum);
                    tkc->curerr = &devi->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                }

                if (devi->mandatory_tkerr.mod && 
                    testdevi->mandatory_tkerr.mod) {
                    res = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'mandatory' deviate-stmt "
                              "already entered on line %u",
                              testdevi->tkerr.linenum);
                    tkc->curerr = &devi->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                }

                if (devi->minelems_tkerr.mod && 
                    testdevi->minelems_tkerr.mod) {
                    res = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'min-elements' deviate-stmt "
                              "already entered on line %u",
                              testdevi->tkerr.linenum);
                    tkc->curerr = &devi->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                }

                if (devi->maxelems_tkerr.mod && 
                    testdevi->maxelems_tkerr.mod) {
                    res = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'max-elements' deviate-stmt "
                              "already entered on line %u",
                              testdevi->tkerr.linenum);
                    tkc->curerr = &devi->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                }

                /**** check devi->mustQ against testdevi->mustQ
                 **** just ignore them for now
                 ****/

                /**** check devi->uniqueQ against testdevi->uniqueQ
                 **** just ignore them for now
                 ****/
            }
        }
    } else {
        /* adding a not-supported, so make sure the Q is empty */
        if (!dlq_empty(deviateQ)) {
            res = ERR_NCX_INVALID_DEV_STMT;
            log_error("\nError: 'not-supported' deviate-stmt "
                      "not allowed");
            tkc->curerr = &devi->tkerr;
            ncx_print_errormsg(tkc, mod, res);
        }
    }

    return res;

}  /* check_deviate_collision */


/********************************************************************
* FUNCTION normalize_deviationQ
* 
* Check for overlapping deviation statements
* combine any deviation statements for the same target
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*        
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    normalize_deviationQ (tk_chain_t *tkc,
                          ncx_module_t  *mod)
{
    obj_deviation_t  *curdev, *checkdev, *nextdev;
    obj_deviate_t    *deviate;
    status_t          res, retres;

    if (dlq_count(&mod->deviationQ) < 2) {
        return NO_ERR;
    }

    res = NO_ERR;
    retres = NO_ERR;

    /* there are at least 2 entries, so check the whole Q */
    curdev = (obj_deviation_t *)dlq_firstEntry(&mod->deviationQ);
    while (curdev != NULL) {

        if (curdev->targobj == NULL || 
            curdev->targobj->tkerr.mod != mod) {
            curdev = (obj_deviation_t *)dlq_nextEntry(curdev);
            continue;
        }

        for (checkdev = (obj_deviation_t *)dlq_nextEntry(curdev);
             checkdev != NULL;
             checkdev = nextdev) {

            nextdev = (obj_deviation_t *)dlq_nextEntry(checkdev);

            if (checkdev->targobj == curdev->targobj) {
                /* have a match; remove this entry
                 * and combine it with the current deviation
                 */
                dlq_remove(checkdev);
                dlq_block_enque(&checkdev->appinfoQ,
                                &curdev->appinfoQ);
                while (!dlq_empty(&checkdev->deviateQ)) {
                    deviate = (obj_deviate_t *)
                        dlq_deque(&checkdev->deviateQ);

                    res = check_deviate_collision(tkc, 
                                                  mod, 
                                                  deviate,
                                                  &curdev->deviateQ);
                    if (res != NO_ERR) {
                        retres = res;
                        obj_free_deviate(deviate);
                    } else {
                        dlq_enque(deviate, &curdev->deviateQ);
                    }
                }
                obj_free_deviation(checkdev);
            }
        }

        /* move through the Q and keep checking for duplicates */
        curdev = (obj_deviation_t *)dlq_nextEntry(curdev);
    }

    return retres;

} /* normalize_deviationQ */


/********************************************************************
* FUNCTION consume_deviate
* 
* Parse the next N tokens as a deviate-stmt
* Create a obj_deviate_t struct and add it to the 
* specified deviation->deviateQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'deviate' keyword
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   deviation == parent obj_deviation_t to hold the new obj_deviate_t
*                created by this function
*
* OUTPUTS:
*   new deviate struct added to deviation deviateQ
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_deviate (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     obj_deviation_t *deviation)
{
    obj_deviate_t    *devi;
    obj_unique_t     *uniq;
    typ_def_t        *dummy;
    const xmlChar    *val;
    const char       *expstr;
    xmlChar          *str;
    tk_type_t         tktyp;
    boolean           done, type, units, def, conf;
    boolean           mand, minel, maxel;
    status_t          res, retres;

    /* Get a new obj_deviation_t to fill in */
    devi = obj_new_deviate();
    if (!devi) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    val = NULL;
    expstr = "add, replace, delete, or not-supported";
    str = NULL;
    done = FALSE;
    type = FALSE;
    units = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    minel = FALSE;
    maxel = FALSE;
    retres = NO_ERR;

    ncx_set_error(&devi->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    /* Get the mandatory deviation argument */
    res = yang_consume_string(tkc, mod, &str);
    CHK_DEVI_EXIT(devi, res, retres);

    if (res == NO_ERR) {
        /* check the value */
        if (!xml_strcmp(str, YANG_K_ADD)) {
            devi->arg = OBJ_DARG_ADD;
        } else if (!xml_strcmp(str, YANG_K_DELETE)) {
            devi->arg = OBJ_DARG_DELETE;
        } else if (!xml_strcmp(str, YANG_K_REPLACE)) {
            devi->arg = OBJ_DARG_REPLACE;
        } else if (!xml_strcmp(str, YANG_K_NOT_SUPPORTED)) {
            devi->arg = OBJ_DARG_NOT_SUPPORTED;
        } else {
            log_error("\nError: invalid deviate-stmt "
                      "argument '%s'", str);
            retres = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
        }
    }

    if (str) {
        m__free(str);
        str = NULL;
    }

    /* Get the starting left brace or semi-colon */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        ncx_print_errormsg(tkc, mod, res);
        obj_free_deviate(devi);
        return res;
    }

    /* check for semi-colon or left brace */
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
        /* only 1 arg type allowed to be empty */
        if (devi->arg == OBJ_DARG_NOT_SUPPORTED) {
            devi->empty = TRUE;
        } else {
            retres = ERR_NCX_WRONG_TKTYPE;
            expstr = "left brace";
            ncx_mod_exp_err(tkc, mod, retres, expstr);
        }
        done = TRUE;
        break;
    case TK_TT_LBRACE:
        done = FALSE;
        break;
    default:
        retres = ERR_NCX_WRONG_TKTYPE;
        expstr = "semi-colon or left brace";
        ncx_mod_exp_err(tkc, mod, retres, expstr);
        done = TRUE;
    }

    expstr = "deviate-stmt sub-clause";

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_deviate(devi);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_deviate(devi);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &devi->appinfoQ);
            CHK_DEVI_EXIT(devi, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_TYPE)) {
            ncx_set_error(&devi->type_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: type-stmt cannot be added");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_DARG_DELETE:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: type-stmt cannot be deleted");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_DARG_REPLACE:
                /* only allowed verb is 'replace' for type-stmt */
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            if (type) {
                log_error("\nError: type-stmt already entered");
                retres = ERR_NCX_ENTRY_EXISTS;          
                dummy = typ_new_typdef();
                if (!dummy) {
                    res = ERR_INTERNAL_MEM;
                    ncx_print_errormsg(tkc, mod, res);
                    obj_free_deviate(devi);
                    return res;
                } else {
                    res = yang_typ_consume_type(pcb,
                                                tkc, 
                                                mod, 
                                                dummy);
                    typ_free_typdef(dummy);
                }
            } else {
                type = TRUE;
                devi->typdef = typ_new_typdef();
                if (!devi->typdef) {
                    res = ERR_INTERNAL_MEM;
                    ncx_print_errormsg(tkc, mod, res);
                    obj_free_deviate(devi);
                    return res;
                } else {
                    res = yang_typ_consume_type(pcb,
                                                tkc, 
                                                mod, 
                                                devi->typdef);
                }
            }
        } else if (!xml_strcmp(val, YANG_K_UNITS)) {
            ncx_set_error(&devi->units_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                break;
            case OBJ_DARG_DELETE:
                break;
            case OBJ_DARG_REPLACE:
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &devi->units,
                                         &units, 
                                         &devi->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DEFAULT)) {
            ncx_set_error(&devi->default_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                break;
            case OBJ_DARG_DELETE:
                break;
            case OBJ_DARG_REPLACE:
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            res = yang_consume_strclause(tkc, 
                                         mod, 
                                         &devi->defval,
                                         &def, 
                                         &devi->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_CONFIG)) {
            ncx_set_error(&devi->config_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                break;
            case OBJ_DARG_DELETE:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: config-stmt cannot be deleted");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_DARG_REPLACE:
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            res = yang_consume_boolean(tkc,
                                       mod,
                                       &devi->config,
                                       &conf,
                                       &devi->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MANDATORY)) {
            ncx_set_error(&devi->mandatory_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                break;
            case OBJ_DARG_DELETE:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: mandatory-stmt cannot be deleted");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_DARG_REPLACE:
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            res = yang_consume_boolean(tkc,
                                       mod,
                                       &devi->mandatory,
                                       &mand, 
                                       &devi->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MIN_ELEMENTS)) {
            ncx_set_error(&devi->minelems_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                break;
            case OBJ_DARG_DELETE:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: min-elements-stmt cannot be deleted");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_DARG_REPLACE:
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            res = yang_consume_uint32(tkc, 
                                      mod,
                                      &devi->minelems,
                                      &minel, 
                                      &devi->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MAX_ELEMENTS)) {
            ncx_set_error(&devi->maxelems_tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                break;
            case OBJ_DARG_DELETE:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: max-elements-stmt cannot be deleted");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_DARG_REPLACE:
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            res = yang_consume_max_elements(tkc, 
                                            mod,
                                            &devi->maxelems,
                                            &maxel,
                                            &devi->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_MUST)) {
            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                break;
            case OBJ_DARG_DELETE:
                break;
            case OBJ_DARG_REPLACE:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: must-stmt cannot be replaced");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            res = yang_consume_must(tkc, 
                                    mod, 
                                    &devi->mustQ,
                                    &devi->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_UNIQUE)) {
            uniq = obj_new_unique();
            if (!uniq) {
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
                obj_free_deviate(devi);
                return res;
            }

            ncx_set_error(&uniq->tkerr,
                          mod,
                          TK_CUR_LNUM(tkc),
                          TK_CUR_LPOS(tkc));

            switch (devi->arg) {
            case OBJ_DARG_NONE:
                /* argument was invalid so cannot check it further */
                break;
            case OBJ_DARG_ADD:
                break;
            case OBJ_DARG_DELETE:
                break;
            case OBJ_DARG_REPLACE:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: unique-stmt cannot be replaced");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_DARG_NOT_SUPPORTED:
                retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: sub-clauses not allowed "
                          "for 'not-supported'");
                ncx_print_errormsg(tkc, mod, retres);
                break;
            default:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
            }

            res = yang_consume_strclause(tkc,
                                         mod,
                                         &uniq->xpath,
                                         NULL,
                                         &devi->appinfoQ);
            CHK_DEVI_EXIT(devi, res, retres);
            if (res == NO_ERR) {
                dlq_enque(uniq, &devi->uniqueQ);
            } else {
                obj_free_unique(uniq);
            }
        } else {
            retres = ERR_NCX_WRONG_TKVAL;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
        }
        CHK_DEVI_EXIT(devi, res, retres);
    }

    /* check valid deviation-stmt syntax */
    if (retres == NO_ERR) {
        retres = check_deviate_collision(tkc, 
                                         mod, 
                                         devi,
                                         &deviation->deviateQ);
    }

    /* not going to resolve this deviate-stmt if it has errors */
    if (retres != NO_ERR) {
        obj_free_deviate(devi);
    } else {
        dlq_enque(devi, &deviation->deviateQ);
    }

    return retres;

}  /* consume_deviate */


/************   R E S O L V E    F U N C T I O N S   ***************/


/********************************************************************
* FUNCTION check_parent
* 
* Check the node against its parent
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == object to check, only if obj != NULL
*          and obj->parent != NULL
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_parent (tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  obj_template_t *obj)
{
    status_t      res;
    ncx_status_t  stat, parentstat;
    boolean       conf, parentconf, ingrp1, ingrp2;

    res = NO_ERR;

    /* check status stmt against the parent, if any */
    if (obj && obj->parent && !obj_is_root(obj->parent)) {
        if (!obj_is_refine(obj)) {
            stat = obj_get_status(obj);
            parentstat = obj_get_status(obj->parent);

            /* check invalid status warning */
            if (stat < parentstat) {
                if (ncx_warning_enabled(ERR_NCX_INVALID_STATUS)) {
                    log_warn("\nWarning: Invalid status: "
                             "child node '%s' = '%s' and"
                             " parent node '%s' = '%s'",
                             obj_get_name(obj),
                             ncx_get_status_string(stat),
                             obj_get_name(obj->parent),
                             ncx_get_status_string(parentstat));
                    SET_OBJ_CURERR(tkc, obj);
                    ncx_print_errormsg(tkc, mod, ERR_NCX_INVALID_STATUS);
                } else if (mod != NULL) {
                    ncx_inc_warnings(mod);
                }
            }
        }

        /* check invalid config flag error for real object only */
        if (obj->objtype <= OBJ_TYP_CASE &&
            obj->parent->objtype <= OBJ_TYP_CASE) {
            ingrp1 = ingrp2 = FALSE;
            conf = obj_get_config_flag_check(obj, &ingrp1);
            parentconf = obj_get_config_flag_check(obj->parent, &ingrp2);
            if ((!parentconf && conf) && (!ingrp1 && !ingrp2))  {
                if (obj_is_data(obj)) {
                    log_error("\nError: Node '%s' is marked as configuration, "
                              "but parent node '%s' is not",
                              obj_get_name(obj),
                              obj_get_name(obj->parent));
                    SET_OBJ_CURERR(tkc, obj);
                    res = ERR_NCX_INVALID_VALUE;
                    ncx_print_errormsg(tkc, mod, res);
                } else {
                    log_info("\nInfo: Non-data node '%s' "
                             "is marked as configuration : statement ignored",
                             obj_get_name(obj));
                    SET_OBJ_CURERR(tkc, obj);
                    res = ERR_NCX_STMT_IGNORED;
                    ncx_print_errormsg(tkc, mod, res);
                }
            }
        }
    }

    return res;

}  /* check_parent */


/********************************************************************
* FUNCTION resolve_default_parm
* 
* Check the rpc input or container object type
* to see if a CLI default-parm was defined.  If so,
* find the target object.
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == parent object for 'rpcio' or 'list'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_default_parm (tk_chain_t *tkc,
                   ncx_module_t  *mod,
                   obj_template_t *obj)
{
    obj_template_t    *targobj;
    ncx_appinfo_t     *appinfo;
    status_t           res;

    res = NO_ERR;

    if (obj->objtype == OBJ_TYP_CONTAINER ||
        (obj->objtype == OBJ_TYP_RPCIO &&
         !xml_strcmp(obj_get_name(obj), YANG_K_INPUT))) {

        appinfo = ncx_find_appinfo(&obj->appinfoQ,
                                   NCX_PREFIX,
                                   NCX_EL_DEFAULT_PARM);
        if (appinfo) {
            if (appinfo->value) {
                targobj = obj_find_child(obj,
                                         obj_get_mod_name(obj),
                                         appinfo->value);
                if (targobj) {
                    if (obj->objtype == OBJ_TYP_CONTAINER) {
                        obj->def.container->defaultparm = targobj;
                    } else {
                        obj->def.rpcio->defaultparm = targobj;
                    }
                } else {
                    res = ERR_NCX_UNKNOWN_OBJECT;
                }
            } else {
                res = ERR_NCX_MISSING_PARM;
            }

            if (res != NO_ERR) {
                log_error("\nError: invalid 'default-parm' extension");
                ncx_print_errormsg(tkc, mod, res);
            }
        }
    }

    return res;
                                    
}  /* resolve_default_parm */


/********************************************************************
* FUNCTION resolve_mustQ
* 
* Check any must-stmts for this node
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain in progress
*   mod == module in progress containing obj
*   obj == object to check (from the cooked module,
*                           not from any grouping or augment)
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_mustQ (tk_chain_t *tkc,
                   ncx_module_t  *mod,
                   obj_template_t *obj)
{
    xpath_pcb_t    *must;
    dlq_hdr_t      *mustQ;
    status_t        res, retres;

    mustQ = obj_get_mustQ(obj);
    if (!mustQ) {
        return NO_ERR;
    }

    retres = NO_ERR;
    for (must = (xpath_pcb_t *)dlq_firstEntry(mustQ);
         must != NULL;
         must = (xpath_pcb_t *)dlq_nextEntry(must)) {

        if (must->tkc == NULL) {
            /* this is a clone object and the xpath PCB
             * is a bare-minimum copy; need to parse
             * the expression again,
             */

            /* if the must is from a grouping in a different
             * module, then the must->tk value will be
             * garbage at this point  !!!!
             */
            tkc->curerr = &must->tkerr;
            res = xpath1_parse_expr(tkc, mod, must, XP_SRC_YANG);
        }

        if (must->parseres != NO_ERR) {
            /* some errors already reported so do not
             * duplicate messages; just skip 2nd pass
             */
            continue;
        }

        res = xpath1_validate_expr_ex(mod, obj, must, FALSE);
        CHK_EXIT(res, retres);
    }
    return retres;

}  /* resolve_mustQ */


/********************************************************************
* FUNCTION resolve_when
* 
* Check any when-stmt for this node
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   mod == module in progress containing obj
*   when == XPath control block to use
*   obj == object to check (from the cooked module,
*                           not from any grouping or augment)
*
* OUTPUTS:
*   pcb->validateres is set
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_when (ncx_module_t  *mod,
                  xpath_pcb_t *when,
                  obj_template_t *obj)
{
    if (when->parseres != NO_ERR) {
        /* some errors already reported so do not
         * duplicate messages; just skip 2nd pass
         */
        return NO_ERR;
    }
    return xpath1_validate_expr_ex(mod, obj, when, FALSE);

}  /* resolve_when */


/********************************************************************
* FUNCTION resolve_metadata
* 
* Check the object for ncx:metadata definitions
* Convert any clauses to metadata nodes within
* the the object struct
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   obj == object to check for ncx:metadata clauses
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_metadata (yang_pcb_t *pcb,
                      tk_chain_t *tkc,
                      ncx_module_t  *mod,
                      obj_template_t *obj)
{
    dlq_hdr_t            *que;
    ncx_appinfo_t        *appinfo;
    tk_chain_t           *newchain;
    obj_metadata_t       *meta;
    status_t              res, retres;
    boolean               usewarning;

    retres = NO_ERR;
    
    que = obj_get_appinfoQ(obj);
    if (!que) {
        return NO_ERR;
    }

    usewarning = ncx_warning_enabled(ERR_NCX_USING_RESERVED_NAME);

    for (appinfo = 
             ncx_find_appinfo(que, 
                              NCX_PREFIX, 
                              NCX_EL_METADATA);
         appinfo != NULL;
         appinfo = ncx_find_next_appinfo2(appinfo, 
                                          NCX_PREFIX,
                                          NCX_EL_METADATA)) {

        /* parse the value string into 2 or 3 fields */
        newchain = NULL;
        meta = NULL;
        res = NO_ERR;

        /* turn the string into a Q of tokens */
        newchain = tk_tokenize_metadata_string(mod,
                                               appinfo->value,
                                               &res);
        if (res != NO_ERR) {
            log_error("\nError: Invalid metadata value string");
        } else {
            meta = obj_new_metadata();
            if (!meta) {
                res = ERR_INTERNAL_MEM;
            } else {
                /* check the tokens that are in the chain for
                 * a YANG QName for the datatype and a YANG
                 * identifier for the XML attribute name
                 */
                res = yang_typ_consume_metadata_type(pcb,
                                                     newchain, 
                                                     mod, 
                                                     meta->typdef);
                if (res == NO_ERR) {
                    /* make sure type OK for XML attribute */
                    if (!typ_ok_for_metadata
                        (typ_get_basetype(meta->typdef))) {
                        log_error("\nError: Builtin type %s not "
                                  "allowed for metadata in object %s",
                                  tk_get_btype_sym
                                  (typ_get_basetype(meta->typdef)),
                                  obj_get_name(obj));
                        res = ERR_NCX_WRONG_TYPE;
                    }
                }

                if (res == NO_ERR) {
                    /* got a type for the attribute
                     * now need to get a valid name
                     */
                    res = yang_consume_id_string(newchain,
                                                 mod,
                                                 &meta->name);
                    if (res == NO_ERR) {
                        /* check if the name clashes
                         * with any standard attributes
                         */
                        if (!xml_strcmp(meta->name, 
                                        NC_OPERATION_ATTR_NAME)) {
                            if (usewarning) {
                                log_warn("\nWarning: metadata using "
                                         "reserved name 'operation' "
                                         "for object %s",
                                         obj_get_name(obj));
                            } else {
                                ncx_inc_warnings(mod);
                            }
                        } else if (!xml_strcmp(meta->name, 
                                               YANG_K_KEY)) {
                            if (usewarning) {
                                log_warn("\nWarning: metadata using "
                                         "reserved name 'key' "
                                         "for object %s",
                                         obj_get_name(obj));
                            } else {
                                ncx_inc_warnings(mod);
                            }

                        } else if (!xml_strcmp(meta->name, 
                                               YANG_K_INSERT)) {
                            if (usewarning) {
                                log_warn("\nWarning: metadata using "
                                         "reserved name 'insert' "
                                         "for object %s",
                                         obj_get_name(obj));
                            } else {
                                ncx_inc_warnings(mod);
                            }
                        } else if (!xml_strcmp(meta->name, 
                                               YANG_K_VALUE)) {
                            if (usewarning) {
                                log_warn("\nWarning: metadata using "
                                         "reserved name 'value' "
                                         "for object %s",
                                         obj_get_name(obj));
                            } else {
                                ncx_inc_warnings(mod);
                            }
                        }

                        /* save the metadata even if the name clashes
                         * because it is supposed to be used
                         * with a namespace;  However, the
                         * standard attribbutes are often used
                         * without any prefix
                         */
                        res = obj_add_metadata(meta, obj);
                    }
                }
            }
        }

        if (res != NO_ERR) {
            log_error("\nError: Invalid ncx:metadata string");
            res = ERR_NCX_INVALID_VALUE;
            tkc->curerr = &appinfo->tkerr;
            ncx_print_errormsg(tkc, mod, res);
            retres = res;
        }

        if (newchain) {
            tk_free_chain(newchain);
        }

        if (res != NO_ERR && meta) {
            obj_free_metadata(meta);
        }
    }

    return retres;

} /* resolve_metadata */


/********************************************************************
* FUNCTION resolve_container
* 
* Check the container object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   con == obj_container_t to check
*   obj == parent object for 'con'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_container (yang_pcb_t *pcb,
                       tk_chain_t *tkc,
                       ncx_module_t  *mod,
                       obj_container_t *con,
                       obj_template_t *obj,
                       boolean redo)
{
    status_t              res, retres;

    retres = NO_ERR;

    if (!redo) {
        res = resolve_metadata(pcb,
                               tkc, 
                               mod, 
                               obj);
        CHK_EXIT(res, retres);
    }

    if (!obj_is_refine(obj) && !redo) {
        res = yang_typ_resolve_typedefs(pcb,
                                        tkc, 
                                        mod, 
                                        con->typedefQ, 
                                        obj);
        CHK_EXIT(res, retres);

        res = yang_grp_resolve_groupings(pcb,
                                         tkc, 
                                         mod, 
                                         con->groupingQ, 
                                         obj);
        CHK_EXIT(res, retres);
    }

    finish_config_flag(obj);

    res = resolve_datadefs(pcb,
                           tkc, 
                           mod, 
                           con->datadefQ, 
                           redo);
    CHK_EXIT(res, retres);

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    return retres;
                                    
}  /* resolve_container */


/********************************************************************
* FUNCTION resolve_container_final
* 
* Check the final container placement for any mandatory
* top-level NP containers

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == container object to check
*   ingrouping == TRUE if this object being resolved
*                 from yang_grp_resolve_final
*              == FALSE otherwise
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_container_final (tk_chain_t *tkc,
                             ncx_module_t  *mod,
                             obj_template_t *obj,
                             boolean ingrouping)
{
    const xmlChar *errstr;
    status_t       res;
    boolean        ingrp;

    res = NO_ERR;
    ingrp = FALSE;

    if (!ingrouping && 
        !obj_is_abstract(obj) &&
        (obj->def.container->presence == NULL) &&
        obj_get_config_flag_check(obj, &ingrp) &&
        ((obj->parent != NULL && obj_is_root(obj->parent)) ||
         (obj->parent == NULL && obj->grp == NULL)) &&
        obj_is_mandatory_when_ex(obj, TRUE)) {
        
        if (ncx_warning_enabled(ERR_NCX_TOP_LEVEL_MANDATORY)) {

            errstr = (obj_has_when_stmts(obj)) ? 
                (const xmlChar *)"conditional " : EMPTY_STRING;

            log_warn("\nWarning: top-level %sNP container "
                     "'%s' is mandatory",
                     errstr,
                     obj_get_name(obj));
            res = ERR_NCX_TOP_LEVEL_MANDATORY;
            SET_OBJ_CURERR(tkc, obj);
            ncx_print_errormsg(tkc, mod, res);
            res = NO_ERR;
        } else if (mod != NULL) {
            ncx_inc_warnings(mod);
        }
    }

    if (obj_is_cli(obj)) {
        obj_sort_children(obj);
    }

    return res;
                                    
}  /* resolve_container_final */


/********************************************************************
* FUNCTION resolve_leaf
* 
* Check the leaf object type
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   leaf == obj_leaf_t to check
*   obj == parent object for 'leaf'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_leaf (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  obj_leaf_t *leaf,
                  obj_template_t *obj,
                  boolean redo)
{
    status_t res, retres;

    retres = NO_ERR;

    if (!redo) {
        res = resolve_metadata(pcb,
                               tkc, 
                               mod, 
                               obj);
        CHK_EXIT(res, retres);
    }

    if (!obj_is_refine(obj) || !redo) {
        res = yang_typ_resolve_type(pcb,
                                    tkc, 
                                    mod, 
                                    leaf->typdef,
                                    leaf->defval, 
                                    obj);
        CHK_EXIT(res, retres);
    }

    finish_config_flag(obj);

    if (obj->flags & OBJ_FL_MANDATORY) {
        if (leaf->defval) {
            log_error("\nError: both mandatory and default "
                      "statements present"
                      "'%s'", obj_get_name(obj));
            retres = ERR_NCX_INVALID_VALUE;
            SET_OBJ_CURERR(tkc, obj);
            ncx_print_errormsg(tkc, mod, retres);
        }
    }

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    return retres;
                                    
}  /* resolve_leaf */


/********************************************************************
* FUNCTION resolve_leaf_final
* 
* Check the final leaf placements for any mandatory
* top-level leafs

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == leaf object to check
*   ingrouping == TRUE if this object being resolved
*                 from yang_grp_resolve_final
*              == FALSE otherwise
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_leaf_final (tk_chain_t *tkc,
                        ncx_module_t  *mod,
                        obj_template_t *obj,
                        boolean ingrouping)
{
    const xmlChar *errstr;
    status_t       res;
    boolean        ingrp;

    res = NO_ERR;
    ingrp = FALSE;

    if (!ingrouping &&
        !obj_is_abstract(obj) &&
        obj_is_mandatory_when(obj) &&
        obj_get_config_flag_check(obj, &ingrp) &&
        ((obj->parent && obj_is_root(obj->parent)) || 
         (obj->parent == NULL && obj->grp == NULL))) {

        if (ncx_warning_enabled(ERR_NCX_TOP_LEVEL_MANDATORY)) {

            errstr = (obj_has_when_stmts(obj)) ? 
                (const xmlChar *)"conditional " : EMPTY_STRING;
            
            log_warn("\nWarning: top-level %sleaf '%s' is mandatory",
                     errstr,
                     obj_get_name(obj));
            res = ERR_NCX_TOP_LEVEL_MANDATORY;
            SET_OBJ_CURERR(tkc, obj);
            ncx_print_errormsg(tkc, mod, res);
            res = NO_ERR;
        } else if (mod != NULL) {
            ncx_inc_warnings(mod);
        }
    }

    return res;
                                    
}  /* resolve_leaf_final */


/********************************************************************
* FUNCTION resolve_leaflist
* 
* Check the leaf-list object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   llist == obj_leaflist_t to check
*   obj == parent object for 'llist'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_leaflist (yang_pcb_t *pcb,
                      tk_chain_t *tkc,
                      ncx_module_t  *mod,
                      obj_leaflist_t *llist,
                      obj_template_t *obj,
                      boolean redo)
{
    status_t res, retres;

    retres = NO_ERR;

    if (!redo) {
        res = resolve_metadata(pcb,
                               tkc, 
                               mod, 
                               obj);
        CHK_EXIT(res, retres);
    }

    if (!obj_is_refine(obj) && !redo) {
        res = yang_typ_resolve_type(pcb,
                                    tkc, 
                                    mod,
                                    llist->typdef, 
                                    NULL, 
                                    obj);
        CHK_EXIT(res, retres);
    }

    /* mark default as zero or more entries
     * the min-elements and max-elements will override
     * this property at runtime
     */
    llist->typdef->iqual = NCX_IQUAL_ZMORE;

    finish_config_flag(obj);

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* check if minelems and maxelems are valid */
    if (llist->minelems && llist->maxelems) {
        if (llist->minelems > llist->maxelems) {
            log_error("\nError: leaf-list '%s' min-elements > max-elements",
                      obj_get_name(obj));
            retres = ERR_NCX_INVALID_VALUE;
            SET_OBJ_CURERR(tkc, obj);
            ncx_print_errormsg(tkc, mod, retres);
        }
    }

    return retres;
                                    
}  /* resolve_leaflist */


/********************************************************************
* FUNCTION get_list_key
* 
* Get the key components and validate, save them

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   list == obj_list_t to check
*   obj == parent object for 'list'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    get_list_key (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  obj_list_t    *list,
                  obj_template_t *obj)
{
    obj_template_t    *keyobj;
    xmlChar           *str, *p, savech;
    ncx_error_t       *tkerr;
    obj_key_t         *objkey;
    status_t           retres;
    ncx_btype_t        btyp;
    boolean            keyconfig, listconfig, ingrp1, ingrp2;

    retres = NO_ERR;
    tkerr = (list->keytkerr.mod) ? &list->keytkerr : &obj->tkerr;

    /* skip all leading whitespace */
    p = list->keystr;
    while (*p && xml_isspace(*p)) {
        p++;
    }

    /* check whitespace only string */
    if (!*p) {
        log_error("\nError: no identifiers entered in key '%s'",
                  list->keystr);
        retres = ERR_NCX_INVALID_VALUE;
        tkc->curerr = tkerr;
        ncx_print_errormsg(tkc, mod, retres);
        return retres;
    }

    /* keep parsing Xpath strings until EOS reached
     * they will be checked to make sure only proper
     * child nodes are used as keys
     */
    while (*p) {
        /* save start of identifier */
        str = p;       

        /* find end of the identifier string */
        while (*p && !xml_isspace(*p)) {
            p++;
        }

        /* make Zstring for 1 key identifier */
        savech = *p;
        *p = 0;

        /* check for a valid descendant-schema-nodeid string */
        retres = xpath_find_schema_target_err(pcb,
                                              tkc, 
                                              mod, 
                                              obj,
                                              list->datadefQ,
                                              str, 
                                              &keyobj, 
                                              NULL, 
                                              tkerr);


        /* check identifier is bogus, nothing found */
        if (retres != NO_ERR) {
            log_error("\nError: invalid identifier in key"
                      " for list '%s' (%s)", 
                      list->name, str);
            tkc->curerr = tkerr;
            ncx_print_errormsg(tkc, mod, retres);

            /* waited to restore string so it could be used 
             * in the log_error msg above
             */
            *p = savech;
            while (*p && xml_isspace(*p)) {
                p++;
            }
            continue;
        }

        /* mark the object as a key leaf for obj_is_key() fn */
        keyobj->flags |= OBJ_FL_KEY;

        /* restore string and skip any whitespace between key components */
        *p = savech;
        while (*p && xml_isspace(*p)) {
            p++;
        }

        /* get the base type of the object */
        btyp = obj_get_basetype(keyobj);

        /* make sure the key is a leaf */
        if (keyobj->objtype != OBJ_TYP_LEAF) {
            /* found the key node, but it is not a leaf */
            log_error("\nError: node '%s' on line %u not a leaf in key"
                      " for list '%s' (%s)",
                      obj_get_name(keyobj), 
                      keyobj->tkerr.linenum,
                      list->name, 
                      obj_get_typestr(keyobj));
            retres = ERR_NCX_TYPE_NOT_INDEX;
            tkc->curerr = tkerr;
            ncx_print_errormsg(tkc, mod, retres);
            continue;
        } 

        /* make sure the leaf is a child of the list object
         * and not a deep key; this is a CLR in YANG but it
         * is supported by Yuma
         */
        if (keyobj->parent != obj) {
            log_error("\nError: leaf node '%s' on line %u not child "
                      "of list '%s'",
                      obj_get_name(keyobj),
                      keyobj->tkerr.linenum,
                      list->name);
            retres = ERR_NCX_WRONG_INDEX_TYPE;
            tkc->curerr = tkerr;
            ncx_print_errormsg(tkc, mod, retres);
        }

        /* make sure the base type is OK for an index */
        if (!typ_ok_for_inline_index(btyp)) {
            log_error("\nError: leaf node '%s' on line %u not valid type "
                      "in key, for list '%s' (%s)",
                      obj_get_name(keyobj),
                      keyobj->tkerr.linenum,
                      list->name,
                      tk_get_btype_sym(btyp));
            retres = ERR_NCX_TYPE_NOT_INDEX;
            tkc->curerr = tkerr;
            ncx_print_errormsg(tkc, mod, retres);
        }

        /* make sure madatory=false is not set for the key leaf */
        if ((keyobj->flags & OBJ_FL_MANDSET) && 
            !(keyobj->flags & OBJ_FL_MANDATORY)) {
            if (ncx_warning_enabled(ERR_NCX_STMT_IGNORED)) {
                log_warn("\nWarning: 'mandatory false;' "
                         "ignored in leaf '%s' "
                         "on line %u for list '%s'",
                         obj_get_name(keyobj), 
                         keyobj->tkerr.linenum, 
                         list->name);
                tkc->curerr = tkerr;
                ncx_print_errormsg(tkc, mod, ERR_NCX_STMT_IGNORED);
            } else if (mod != NULL) {
                ncx_inc_warnings(mod);
            }
        }

        /* make sure config has same setting as the list parent */
        ingrp1 = ingrp2 = FALSE;
        keyconfig = obj_get_config_flag_check(keyobj, &ingrp1);
        listconfig = obj_get_config_flag_check(obj, &ingrp2);

        if ((keyconfig != listconfig) && (!ingrp1 && !ingrp2)) {
            retres = ERR_NCX_WRONG_INDEX_TYPE;
            log_error("\nError: 'config-stmt for key leaf '%s' "
                      "on line %u must match list '%s'",
                      obj_get_name(keyobj), 
                      keyobj->tkerr.linenum, 
                      list->name);
            tkc->curerr = tkerr;
            ncx_print_errormsg(tkc, mod, retres);
        }

        /* make sure key component not already used */
        objkey = obj_find_key2(&list->keyQ, keyobj);
        if (objkey) {
            log_error("\nError: duplicate key node '%s' on line %u "
                      "for list '%s'",
                      obj_get_name(keyobj), 
                      keyobj->tkerr.linenum, 
                      list->name);
            retres = ERR_NCX_DUP_ENTRY;
            tkc->curerr = tkerr;
            ncx_print_errormsg(tkc, mod, retres);
            continue;
        }

        /* get a new key record struct */
        objkey = obj_new_key();
        if (!objkey) {
            retres = ERR_INTERNAL_MEM;
            tkc->curerr = tkerr;
            ncx_print_errormsg(tkc, mod, retres);
            return retres;
        }

        /* everything OK so save the key
         * a backptr to 'objkey' in 'key' cannot be maintained
         * because 'key' may be inside a grouping, and a simple
         * uses foo; will cause the groupingQ to be used directly
         */
        objkey->keyobj = keyobj;
        dlq_enque(objkey, &list->keyQ);
    }

    return retres;

}  /* get_list_key */


/********************************************************************
* FUNCTION resolve_list
* 
* Check the list object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   list == obj_list_t to check
*   obj == parent object for 'list'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_list (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  obj_list_t *list,
                  obj_template_t *obj,
                  boolean redo)
{
    status_t res, retres;

    retres = NO_ERR;

    if (!redo) {
        res = resolve_metadata(pcb,
                               tkc, 
                               mod, 
                               obj);
        CHK_EXIT(res, retres);
    }

    if (!obj_is_refine(obj) && !redo) {
        res = yang_typ_resolve_typedefs(pcb,
                                        tkc, 
                                        mod, 
                                        list->typedefQ, 
                                        obj);
        CHK_EXIT(res, retres);

        res = yang_grp_resolve_groupings(pcb,
                                         tkc, 
                                         mod, 
                                         list->groupingQ, 
                                         obj);
        CHK_EXIT(res, retres);
    }

    finish_config_flag(obj);

    res = resolve_datadefs(pcb, 
                           tkc, 
                           mod, 
                           list->datadefQ, 
                           redo);
    CHK_EXIT(res, retres);

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* check if minelems and maxelems are valid */
    if (list->minelems && list->maxelems) {
        if (list->minelems > list->maxelems) {
            log_error("\nError: list '%s' min-elements > max-elements",
                      obj_get_name(obj));
            retres = ERR_NCX_INVALID_VALUE;
            SET_OBJ_CURERR(tkc, obj);
            ncx_print_errormsg(tkc, mod, retres);
        }
    }

    return retres;
                                    
}  /* resolve_list */


/********************************************************************
* FUNCTION get_unique_comps
* 
* Get the unique-stmt components and validate, save them

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   list == obj_list_t to check
*   obj == parent object for 'list'
*   uni == unique statement collected in this struct
*          needs to be validated and finalized
*
* OUTPUTS:
*   uni->compQ is filled with obj_unique_comp_t structs
*         each one represents one leaf in the unique tuple
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    get_unique_comps (yang_pcb_t *pcb,
                      tk_chain_t *tkc,
                      ncx_module_t  *mod,
                      obj_list_t *list,
                      obj_template_t *obj,
                      obj_unique_t *uni)
{
    obj_template_t    *uniobj, *testobj;
    xmlChar           *str, *p, *savestr, savech;
    ncx_error_t       *tkerr;
    obj_unique_comp_t *unicomp, *testcomp;
    status_t           res, retres;
    boolean            firstset, ingrp;

    firstset = FALSE;
    savestr = NULL;
    retres = NO_ERR;
    tkerr = (uni->tkerr.mod) ? &uni->tkerr : &obj->tkerr;

    /* skip all leading whitespace */
    p = uni->xpath;
    while (*p && xml_isspace(*p)) {
        p++;
    }
    if (!*p) {
        log_error("\nError: no identifiers entered in unique statement '%s'",
                  uni->xpath);
        retres = ERR_NCX_INVALID_VALUE;
        tkc->curerr = tkerr;
        ncx_print_errormsg(tkc, mod, retres);
        return retres;
    }

    /* keep parsing Xpath strings until EOS reached */
    while (*p) {
        /* find end of non-whitespace string */
        str = p;
        while (*p && !xml_isspace(*p)) {
            p++;
        }
        savech = *p;
        *p = 0;

        /* check for a valid descendant-schema-nodeid string */
        res = xpath_find_schema_target_err(pcb,
                                           tkc, 
                                           mod, 
                                           obj,
                                           list->datadefQ,
                                           str, 
                                           &uniobj,
                                           NULL,
                                           tkerr);
        CHK_EXIT(res, retres);
        if (res == NO_ERR) {
            savestr = xml_strdup(str);
            if (!savestr) {
                retres = ERR_INTERNAL_MEM;
                tkc->curerr = tkerr;
                ncx_print_errormsg(tkc, mod, retres);
                return retres;
            }
        }

        *p = savech;
        while (*p && xml_isspace(*p)) {
            p++;   /* skip whitespace between strings */
        }
        if (res != NO_ERR) {
            continue;
        }

        /* got a valid Xpath expression which points to a
         * child node in the obj_list_t datadefQ
         * make sure the unique target is a leaf
         */
        if (uniobj->objtype != OBJ_TYP_LEAF) {
            log_error("\nError: node '%s' on line %u not leaf in "
                      "list '%s' unique-stmt",
                      obj_get_name(uniobj),
                      uniobj->tkerr.linenum,
                      list->name);
            retres = ERR_NCX_INVALID_UNIQUE_NODE;
            tkc->curerr = tkerr;
            ncx_print_errormsg(tkc, mod, retres);
            m__free(savestr);
            continue;
        } 

        /* make sure there is a no config mismatch */
        if (firstset) {
            ingrp = FALSE;
            if (obj_get_config_flag_check(obj, &ingrp) 
                && !uni->isconfig) {
                /* mix of config and non-config leafs
                 * in the unique-stmt
                 */
                if (!ingrp) {
                    log_error("\nError: leaf '%s' on line "
                              "%u; unique-stmt config mismatch in "
                              "list '%s'",
                              obj_get_name(uniobj),
                              uniobj->tkerr.linenum,
                              list->name);
                    retres = ERR_NCX_INVALID_UNIQUE_NODE;
                    tkc->curerr = tkerr;
                    ncx_print_errormsg(tkc, mod, retres);
                    m__free(savestr);
                    continue;
                }
            }
        } else {
            /* unique-stmt can be for all config leafs or
             * all non-config leafs, but no mix
             */
            uni->isconfig = obj_get_config_flag_deep(obj);
            firstset = TRUE;
        }

        /* the final target seems to be a valid leaf
         * so check that its path back to the original
         * object is all static object types
         *   container, leaf, choice, case
         */
        testobj = uniobj->parent;
        res = NO_ERR;
        while (testobj && (testobj != obj) && (res == NO_ERR)) {
            switch (testobj->objtype) {
            case OBJ_TYP_CONTAINER:
            case OBJ_TYP_CHOICE:
            case OBJ_TYP_CASE:
                break;
            default:
                res = ERR_NCX_INVALID_UNIQUE_NODE;
                log_error("\nError: multi-instance node (%s) "
                          "within unique stmt '%s'",
                          obj_get_typestr(testobj),
                          uni->xpath);
                tkc->curerr = tkerr;
                ncx_print_errormsg(tkc, mod, res);
            }

            testobj = testobj->parent;
        }
        if (res != NO_ERR) {
            m__free(savestr);
            CHK_EXIT(res, retres);
            continue;
        }

        uniobj->flags |= OBJ_FL_UNIQUE;

        /* make sure this leaf component not already used */
        for (testcomp = (obj_unique_comp_t *)dlq_firstEntry(&uni->compQ);
             testcomp != NULL && res==NO_ERR;
             testcomp = (obj_unique_comp_t *)dlq_nextEntry(testcomp)) {
            if (testcomp->unobj == uniobj) {
                if (ncx_warning_enabled(ERR_NCX_DUP_UNIQUE_COMP)) {
                    log_warn("\nWarning: duplicate unique "
                             "node '%s' on line %u "
                             "for list '%s'",
                             obj_get_name(uniobj),
                             uniobj->tkerr.linenum, 
                             list->name);
                    tkc->curerr = tkerr;
                    ncx_print_errormsg(tkc, 
                                       mod,
                                       ERR_NCX_DUP_UNIQUE_COMP);
                } else if (mod != NULL) {
                    ncx_inc_warnings(mod);
                }
            }
        }

        /* try to save the info in a new unicomp struct */
        if (retres == NO_ERR) {
            /* get a new unique component struct */
            unicomp = obj_new_unique_comp();
            if (!unicomp) {
                retres = ERR_INTERNAL_MEM;
                tkc->curerr = tkerr;
                ncx_print_errormsg(tkc, mod, retres);
                m__free(savestr);
                return retres;
            } else {
                /* everything OK so save the unique component
                 * pass off the malloced savestr to the
                 * unicomp record
                 */
                unicomp->unobj = uniobj;
                unicomp->xpath = savestr;
                dlq_enque(unicomp, &uni->compQ);
            }
        } else {
            m__free(savestr);
        }
    }

    return retres;

}  /* get_unique_comps */


/********************************************************************
* FUNCTION resolve_list_final
* 
* Check the list object type after all uses and augments are expanded

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   list == obj_list_t to check
*   obj == parent object for 'list'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_list_final (yang_pcb_t *pcb,
                        tk_chain_t *tkc,
                        ncx_module_t  *mod,
                        obj_list_t *list,
                        obj_template_t *obj)
{
    obj_unique_t    *uni;
    status_t         res, retres;

    retres = NO_ERR;

    /* augment is processed before resolve_list_final and the list keys
     * are filled into the orginal list (under augment, not in the
     * expanded list under the augment target
     * THIS DOES NOT ALWAYS WORK FOR augment /obj-in-submod-a with
     *  obj-in-submod-b if submod-a processed before submod-b, 
     * this step will get skipped
     *
     * The function yang_obj_top_resolve_final will be called for the
     * main module and all submodules will attempt this code again
     * in case the submod datadefQ was filled in by another submod
     * after the first submod called yang_obj_resolve_final
     *
     * For modules augmenting other modules, this step could
     * be skipped, if the augmenting module is compiled after the
     * augmented module, which is always the case, since the
     * augmented module has to be imported, then resolve_list_final
     * for the augmenting list was not done for the cloned list
     */

    /* validate key clause only if this has probably not 
     * been attempted yet 
     */
    if (list->keystr && dlq_empty(&list->keyQ)) {
        res = get_list_key(pcb, tkc, mod, list, obj);
        CHK_EXIT(res, retres);
    }

    /* validate Q of unique clauses only if probably not attempted yet */
    for (uni = (obj_unique_t *)dlq_firstEntry(&list->uniqueQ);
         uni != NULL;
         uni = (obj_unique_t *)dlq_nextEntry(uni)) {

        if (!dlq_empty(&uni->compQ)) {
            /* this list was processed already and some or all
             * unique components were found already
             */
            continue;
        }

        res = get_unique_comps(pcb, tkc, mod, list, obj, uni);
        CHK_EXIT(res, retres);
    }

    return retres;

}  /* resolve_list_final */


/********************************************************************
* FUNCTION resolve_case
* 
* Check the case object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   cas == obj_case_t to check
*   obj == parent object for 'cas'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_case (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  obj_case_t *cas,
                  obj_template_t *obj,
                  boolean redo)
{
    status_t res, retres;

    retres = NO_ERR;

    res = resolve_datadefs(pcb, tkc, mod, cas->datadefQ, redo);
    CHK_EXIT(res, retres);

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    return retres;
                                    
}  /* resolve_case */


/********************************************************************
* FUNCTION resolve_choice
* 
* Check the choice object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   choic == obj_choice_t to check
*   obj == parent object for 'choic'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_choice (yang_pcb_t *pcb,
                    tk_chain_t *tkc,
                    ncx_module_t  *mod,
                    obj_choice_t *choic,
                    obj_template_t *obj,
                    boolean redo)
{
    obj_case_t *cas;
    obj_template_t *cobj;
    status_t res, retres;

    retres = NO_ERR;

    /* not in draft yet !!! */
    finish_config_flag(obj);

    if ((obj->flags & OBJ_FL_MANDATORY) && choic->defval) {
        log_error("\nError: both mandatory and default statements present"
                  "'%s'", 
                  obj_get_name(obj));
        retres = ERR_NCX_INVALID_VALUE;
        SET_OBJ_CURERR(tkc, obj);
        ncx_print_errormsg(tkc, mod, retres);
    }

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* finish up the data-def-stmts in each case arm */
    res = resolve_datadefs(pcb, tkc, mod, choic->caseQ, redo);
    CHK_EXIT(res, retres);

    /* check defval is valid case name */
    if (choic->defval) {
        cas = obj_find_case(choic, 
                            obj_get_mod_name(obj), 
                            choic->defval);
        if (!cas) {
            /* default is not a valid case name */
            SET_OBJ_CURERR(tkc, obj);
            retres = ERR_NCX_INVALID_VALUE;
            log_error("\nError: Choice default '%s' "
                      "not a valid case name", 
                      choic->defval);
            ncx_print_errormsg(tkc, mod, retres);
        } else {
            /* valid case name, 
             * make sure 'cas' contains only optional data nodes
             */
            for (cobj = (obj_template_t *)dlq_firstEntry(cas->datadefQ);
                 cobj != NULL;
                 cobj = (obj_template_t *)dlq_nextEntry(cobj)) {
                if (obj_is_mandatory(cobj)) {
                    tkc->curerr = &cobj->tkerr;
                    retres = ERR_NCX_DEFCHOICE_NOT_OPTIONAL;
                    ncx_print_errormsg(tkc, mod, retres);
                }
            }
        }
    }
        
    return retres;
                                    
}  /* resolve_choice */


/********************************************************************
* FUNCTION resolve_choice_final
* 
* Check the final choice placement for any mandatory
* top-level choices

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   obj == choice object to check
*   ingrouping == TRUE if this object being resolved
*                 from yang_grp_resolve_final
*              == FALSE otherwise
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_choice_final (tk_chain_t *tkc,
                          ncx_module_t  *mod,
                          obj_template_t *obj,
                          boolean ingrouping)
{
    const xmlChar *errstr;
    status_t       res;
    boolean        ingrp;

    res = NO_ERR;
    ingrp = FALSE;

    if (!ingrouping &&
        !obj_is_abstract(obj) &&
        obj_is_mandatory_when(obj) &&
        obj_get_config_flag_check(obj, &ingrp) &&
        ((obj->parent && obj_is_root(obj->parent)) ||
         (obj->parent == NULL && obj->grp == NULL))) {

        if (ncx_warning_enabled(ERR_NCX_TOP_LEVEL_MANDATORY)) {

            errstr = (obj_has_when_stmts(obj)) ? 
                (const xmlChar *)"conditional " : EMPTY_STRING;

            log_warn("\nWarning: top-level %schoice '%s' is mandatory",
                     errstr,
                     obj_get_name(obj));
            res = ERR_NCX_TOP_LEVEL_MANDATORY;
            SET_OBJ_CURERR(tkc, obj);
            ncx_print_errormsg(tkc, mod, res);
            res = NO_ERR;
        } else if (mod != NULL) {
            ncx_inc_warnings(mod);
        }
    }

    return res;
                                    
}  /* resolve_choice_final */


/********************************************************************
* FUNCTION check_refine_allowed
* 
* Check the uses object type against the target node found
*
* Only checks if extra refine clauses are present
* which are not allowed for that 
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   refineobj == refine object to check
*   targobj == target object to check against
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_refine_allowed (tk_chain_t *tkc,
                          ncx_module_t  *mod,
                          obj_template_t *refineobj,
                          obj_template_t *targobj)
{
    obj_refine_t   *refine;
    xpath_pcb_t    *must;
    boolean         pres, def, conf, mand, minel, maxel, bmust;
    status_t        res;

    refine = refineobj->def.refine;

    pres = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    minel = FALSE;
    maxel = FALSE;
    bmust = FALSE;

    res = NO_ERR;

    switch (targobj->objtype) {
    case OBJ_TYP_LEAF:
        conf = TRUE;
        mand = TRUE;
        bmust = TRUE;
        def = TRUE;
        break;
    case OBJ_TYP_ANYXML:
        mand = TRUE;
        break;
    case OBJ_TYP_LEAF_LIST:
        conf = TRUE;
        bmust = TRUE;
        minel = TRUE;
        maxel = TRUE;
        break;
    case OBJ_TYP_CONTAINER:
        bmust = TRUE;
        pres = TRUE;
        conf = TRUE;
        break;
    case OBJ_TYP_LIST:
        bmust = TRUE;
        conf = TRUE;
        minel = TRUE;
        maxel = TRUE;
        break;
    case OBJ_TYP_CHOICE:
        def = TRUE;
        mand = TRUE;
        break;
    case OBJ_TYP_CASE:
        break;
    default:
        return NO_ERR;  /* error: should already be reported */
    }

    /* check all the fields except description and reference
     * since they are allowed to appear in every variant
     */
    if (refine->presence && !pres) {
        res = ERR_NCX_REFINE_NOT_ALLOWED;
        log_error("\nError: 'presence' refinement on %s '%s'",
                  obj_get_typestr(targobj),
                  obj_get_name(targobj));
        tkc->curerr = &refine->presence_tkerr;
        ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->def && !def) {
        res = ERR_NCX_REFINE_NOT_ALLOWED;
        log_error("\nError: 'default' refinement on %s '%s'",
                  obj_get_typestr(targobj),
                  obj_get_name(targobj));
        tkc->curerr = &refine->def_tkerr;
        ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->config_tkerr.mod && !conf) {
        res = ERR_NCX_REFINE_NOT_ALLOWED;
        log_error("\nError: 'config' refinement on %s '%s'",
                  obj_get_typestr(targobj),
                  obj_get_name(targobj));
        tkc->curerr = &refine->config_tkerr;
        ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->mandatory_tkerr.mod && !mand) {
        res = ERR_NCX_REFINE_NOT_ALLOWED;
        log_error("\nError: 'mandatory' refinement on %s '%s'",
                  obj_get_typestr(targobj),
                  obj_get_name(targobj));
        tkc->curerr = &refine->config_tkerr;
        ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->minelems_tkerr.mod && !minel) {
        res = ERR_NCX_REFINE_NOT_ALLOWED;
        log_error("\nError: 'min-elements' refinement on %s '%s'",
                  obj_get_typestr(targobj),
                  obj_get_name(targobj));
        tkc->curerr = &refine->minelems_tkerr;
        ncx_print_errormsg(tkc, mod, res);
    }

    if (refine->maxelems_tkerr.mod && !maxel) {
        res = ERR_NCX_REFINE_NOT_ALLOWED;
        log_error("\nError: 'max-elements' refinement on %s '%s'",
                  obj_get_typestr(targobj),
                  obj_get_name(targobj));
        tkc->curerr = &refine->maxelems_tkerr;
        ncx_print_errormsg(tkc, mod, res);
    }

    if (!dlq_empty(&refine->mustQ) && !bmust) {
        res = ERR_NCX_REFINE_NOT_ALLOWED;
        for (must = (xpath_pcb_t *)dlq_firstEntry(&refine->mustQ);
             must != NULL;
             must = (xpath_pcb_t *)dlq_nextEntry(must)) {
            log_error("\nError: 'must' refinement on %s '%s'",
                      obj_get_typestr(targobj),
                      obj_get_name(targobj));
            tkc->curerr = &must->tkerr;
            ncx_print_errormsg(tkc, mod, res);
        }
    }

    return res;

}  /* check_refine_allowed */


/********************************************************************
* FUNCTION combine_refine_objects
* 
* Combine two refine objects with the same target
* Add fields from the mergeobj into the keepobj.
*
* The mergeobj should be freed after this call
*
* Only checks if extra refine clauses are present
* which are not allowed for that 
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   keepobj == refine object to keep
*   mergeobj == refine object to merge into 'keepobj'
*   targobj == refine object target to check type
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    combine_refine_objects (tk_chain_t *tkc,
                            ncx_module_t  *mod,
                            obj_template_t *keepobj,
                            obj_template_t *mergeobj,
                            obj_template_t *targobj)
{
    obj_refine_t   *krefine, *mrefine;
    boolean         pres, def, conf, mand, minel, maxel, bmust;
    status_t        res;

    krefine = keepobj->def.refine;
    mrefine = mergeobj->def.refine;

    pres = FALSE;
    def = FALSE;
    conf = FALSE;
    mand = FALSE;
    minel = FALSE;
    maxel = FALSE;
    bmust = FALSE;

    res = NO_ERR;

    switch (targobj->objtype) {
    case OBJ_TYP_LEAF:
        conf = TRUE;
        mand = TRUE;
        bmust = TRUE;
        def = TRUE;
        break;
    case OBJ_TYP_ANYXML:
        mand = TRUE;
        break;
    case OBJ_TYP_LEAF_LIST:
        conf = TRUE;
        bmust = TRUE;
        minel = TRUE;
        maxel = TRUE;
        break;
    case OBJ_TYP_CONTAINER:
        bmust = TRUE;
        pres = TRUE;
        conf = TRUE;
        break;
    case OBJ_TYP_LIST:
        bmust = TRUE;
        conf = TRUE;
        minel = TRUE;
        maxel = TRUE;
        break;
    case OBJ_TYP_CHOICE:
        def = TRUE;
        mand = TRUE;
        break;
    case OBJ_TYP_CASE:
        break;
    default:
        return NO_ERR;  /* error: should already be reported */
    }


    if (mrefine->descr) {
        if (krefine->descr) {
            res = ERR_NCX_DUP_REFINE_STMT;
            log_error("\nError: description-stmt set in refine on line %u",
                      krefine->descr_tkerr.linenum);
            tkc->curerr = &mrefine->descr_tkerr;
            ncx_print_errormsg(tkc, mod, res);
        } else {
            krefine->descr = mrefine->descr;
            ncx_set_error(&krefine->descr_tkerr,
                          mrefine->descr_tkerr.mod,
                          mrefine->descr_tkerr.linenum,
                          mrefine->descr_tkerr.linepos);
            mrefine->descr = NULL;
        }
    }

    if (mrefine->ref) {
        if (krefine->ref) {
            res = ERR_NCX_DUP_REFINE_STMT;
            log_error("\nError: reference-stmt set in refine on line %u",
                      krefine->ref_tkerr.linenum);
            tkc->curerr = &mrefine->ref_tkerr;
            ncx_print_errormsg(tkc, mod, res);
        } else {
            krefine->ref = mrefine->ref;
            ncx_set_error(&krefine->ref_tkerr,
                          mrefine->ref_tkerr.mod,
                          mrefine->ref_tkerr.linenum,
                          mrefine->ref_tkerr.linepos);
            mrefine->ref = NULL;
        }
    }

    if (mrefine->presence && pres) {
        if (krefine->presence) {
            res = ERR_NCX_DUP_REFINE_STMT;
            log_error("\nError: presence-stmt set in refine on line %u",
                      krefine->presence_tkerr.linenum);
            tkc->curerr = &mrefine->presence_tkerr;
            ncx_print_errormsg(tkc, mod, res);
        } else {
            krefine->presence = mrefine->presence;
            ncx_set_error(&krefine->presence_tkerr,
                          mrefine->presence_tkerr.mod,
                          mrefine->presence_tkerr.linenum,
                          mrefine->presence_tkerr.linepos);
            mrefine->presence = NULL;
        }
    }

    if (mrefine->def && def) {
        if (krefine->def) {
            res = ERR_NCX_DUP_REFINE_STMT;
            log_error("\nError: default-stmt set in refine on line %u",
                      krefine->def_tkerr.linenum);
            tkc->curerr = &mrefine->def_tkerr;
            ncx_print_errormsg(tkc, mod, res);
        } else {
            krefine->def = mrefine->def;
            ncx_set_error(&krefine->def_tkerr,
                          mrefine->def_tkerr.mod,
                          mrefine->def_tkerr.linenum,
                          mrefine->def_tkerr.linepos);
            mrefine->def = NULL;
        }
    }

    if (mrefine->config_tkerr.mod && conf) {
        if (krefine->config_tkerr.mod) {
            res = ERR_NCX_DUP_REFINE_STMT;
            log_error("\nError: config-stmt set in refine on line %u",
                      krefine->config_tkerr.linenum);
            tkc->curerr = &mrefine->config_tkerr;
            ncx_print_errormsg(tkc, mod, res);
        } else {
            ncx_set_error(&krefine->config_tkerr,
                          mrefine->config_tkerr.mod,
                          mrefine->config_tkerr.linenum,
                          mrefine->config_tkerr.linepos);
            keepobj->flags |= OBJ_FL_CONFSET;
            if (mergeobj->flags & OBJ_FL_CONFIG) {
                keepobj->flags |= OBJ_FL_CONFIG;
            } else {
                keepobj->flags &= ~OBJ_FL_CONFIG;
            }
        }
    }

    if (mrefine->mandatory_tkerr.mod && mand) {
        if (krefine->mandatory_tkerr.mod) {
            res = ERR_NCX_DUP_REFINE_STMT;
            log_error("\nError: mandatory-stmt set in refine on line %u",
                      krefine->mandatory_tkerr.linenum);
            tkc->curerr = &mrefine->mandatory_tkerr;
            ncx_print_errormsg(tkc, mod, res);
        } else {
            ncx_set_error(&krefine->mandatory_tkerr,
                          mrefine->mandatory_tkerr.mod,
                          mrefine->mandatory_tkerr.linenum,
                          mrefine->mandatory_tkerr.linepos);
            keepobj->flags |= OBJ_FL_MANDSET;
            if (mergeobj->flags & OBJ_FL_MANDATORY) {
                keepobj->flags |= OBJ_FL_MANDATORY;
            }
        }
    }

    if (mrefine->minelems_tkerr.mod && minel) {
        if (krefine->minelems_tkerr.mod) {
            res = ERR_NCX_DUP_REFINE_STMT;
            log_error("\nError: min-elements-stmt set in refine on line %u",
                      krefine->minelems_tkerr.linenum);
            tkc->curerr = &mrefine->minelems_tkerr;
            ncx_print_errormsg(tkc, mod, res);
        } else {
            krefine->minelems = mrefine->minelems;
            ncx_set_error(&krefine->minelems_tkerr,
                          mrefine->minelems_tkerr.mod,
                          mrefine->minelems_tkerr.linenum,
                          mrefine->minelems_tkerr.linepos);
        }
    }

    if (mrefine->maxelems_tkerr.mod && maxel) {
        if (krefine->maxelems_tkerr.mod) {
            res = ERR_NCX_DUP_REFINE_STMT;
            log_error("\nError: max-elements-stmt set in refine on line %u",
                      krefine->maxelems_tkerr.linenum);
            tkc->curerr = &mrefine->maxelems_tkerr;
            ncx_print_errormsg(tkc, mod, res);
        } else {
            krefine->maxelems = mrefine->maxelems;
            ncx_set_error(&krefine->maxelems_tkerr,
                          mrefine->maxelems_tkerr.mod,
                          mrefine->maxelems_tkerr.linenum,
                          mrefine->maxelems_tkerr.linepos);
        }
    }

    if (!dlq_empty(&mrefine->mustQ) && bmust) {
        dlq_block_enque(&mrefine->mustQ, &krefine->mustQ);
    }

    if (!dlq_empty(&mergeobj->appinfoQ)) {
        dlq_block_enque(&mergeobj->appinfoQ, &keepobj->appinfoQ);
    }

    return res;

}  /* combine_refine_objects */


/********************************************************************
* FUNCTION resolve_uses
* 
* Check the uses object type
* This is done before the groupings are expanded
*
*   - Find the grouping being used
*   - Check for uses loop errors
*   - check all the local augment statements
*   - check all the refine statements
*   - patch the objects with the refinements
*   - change refine-stmts to canonical form; no duplicate targets
*   
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   uses == obj_uses_t to check
*   obj == parent object for 'uses'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_uses (yang_pcb_t *pcb,
                  tk_chain_t *tkc,
                  ncx_module_t  *mod,
                  obj_uses_t *uses,
                  obj_template_t *obj)
{
    obj_template_t *chobj, *testobj, *cobj, *targobj, *nextobj;
    obj_case_t     *cas;
    obj_refine_t   *refine;
    status_t        res, retres;

    retres = NO_ERR;

    /* find the grouping that this uses references if this is
     * a local grouping, and grp has not been set yet
     */
    if (!uses->grp) {
        uses->grp = obj_find_grouping(obj, uses->name);
        if (!uses->grp) {
            uses->grp = ncx_find_grouping(mod, uses->name, FALSE);
            if (!uses->grp) {
                log_error("\nError: grouping '%s' not found",
                          uses->name);
                retres = ERR_NCX_DEF_NOT_FOUND;
                SET_OBJ_CURERR(tkc, obj);
                ncx_print_errormsg(tkc, mod, retres);
            }
        }
    }

    /* check for nested uses -- a uses AA within grouping AA */
    if (uses->grp) {
        uses->grp->used = TRUE;
        res = yang_grp_check_nest_loop(tkc, mod, obj, uses->grp);
        CHK_EXIT(res, retres);
        if (res != NO_ERR) {
            uses->grp = NULL;   /* prevent recursive crash later */
        }
    }


    /* resolve all the grouping augments, skip the refines */
    res = yang_obj_resolve_datadefs(pcb,
                                    tkc, 
                                    mod, 
                                    uses->datadefQ);
    if (res != NO_ERR) {
        retres = res;
    }
    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* make sure all the refinements really match a child
     * in the grouping
     */
    for (chobj = (obj_template_t *)dlq_firstEntry(uses->datadefQ);
         chobj != NULL;
         chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

        if (chobj->objtype != OBJ_TYP_REFINE) {
            continue;
        }

        refine = chobj->def.refine;

        /* find schema-nodeid target
         * the node being refined MUST exist in the grouping
         */
        res = xpath_find_schema_target(pcb,
                                       tkc, 
                                       uses->grp->tkerr.mod, 
                                       obj,
                                       &uses->grp->datadefQ,
                                       refine->target, 
                                       &targobj, 
                                       NULL);
        if (res != NO_ERR || !targobj) {
            /* error: refined obj not in the grouping */
            log_error("\nError: refinement node '%s' not found"
                      " in grouping '%s'",
                      obj_get_name(chobj),
                      uses->grp->name);
            retres = ERR_NCX_MISSING_REFTARGET;
            tkc->curerr = &chobj->tkerr;
            ncx_print_errormsg(tkc, mod, retres);
        } else {
            /* refine target is valid, so save it */
            refine->targobj = targobj;

            /* check any extra refinements not allowed for
             * the target object type
             */
            res = check_refine_allowed(tkc, 
                                       mod,
                                       chobj,
                                       targobj);
            CHK_EXIT(res, retres);

            /* check if any default statements are present,
             * and if they are OK for the target data type
             */
            if (targobj->objtype == OBJ_TYP_LEAF) {
                if (refine->def) {
                    res = val_simval_ok_ex(targobj->def.leaf->typdef,
                                           refine->def,
                                           NULL,
                                           mod);
                    if (res != NO_ERR) {
                        retres = res;
                        log_error("\nError: Leaf refinement '%s' has "
                                  "invalid default value (%s)",
                                  obj_get_name(targobj),
                                  refine->def);
                        tkc->curerr = &refine->def_tkerr;
                        ncx_print_errormsg(tkc, mod, retres);
                    }
                }
            } else if (targobj->objtype == OBJ_TYP_CHOICE) {
                if (refine->def) {
                    cas = obj_find_case(targobj->def.choic,
                                        obj_get_mod_name(targobj),
                                        refine->def);
                    if (!cas) {
                        /* default is not a valid case name */
                        tkc->curerr = &refine->def_tkerr;
                        retres = ERR_NCX_INVALID_VALUE;
                        log_error("\nError: Refined choice default '%s' "
                                  "is not a valid case name",
                                  refine->def);
                        ncx_print_errormsg(tkc, mod, retres);
                    } else {
                        /* valid case name, 
                         * make sure 'cas' contains only optional data nodes
                         */
                        for (cobj = (obj_template_t *)
                                 dlq_firstEntry(cas->datadefQ);
                             cobj != NULL;
                             cobj = (obj_template_t *)dlq_nextEntry(cobj)) {
                            if (obj_has_name(cobj) &&
                                obj_is_mandatory(cobj)) {
                                tkc->curerr = &cobj->tkerr;
                                retres = ERR_NCX_DEFCHOICE_NOT_OPTIONAL;
                                ncx_print_errormsg(tkc, mod, retres);
                            }
                        }
                    }
                }
            }
        }
    }

    /* go through the refinement objects one more time
     * and combine the ones with the same target (if any)
     * Generate an error for duplicate sub-clauses entered
     */
    for (chobj = (obj_template_t *)dlq_firstEntry(uses->datadefQ);
         chobj != NULL;
         chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

        if (chobj->objtype != OBJ_TYP_REFINE) {
            continue;
        }

        cobj = chobj->def.refine->targobj;
        if (!cobj) {
            continue;
        }

        /* look through rest of Q for any refine w/ same target */
        for (testobj = (obj_template_t *)dlq_nextEntry(chobj);
             testobj != NULL; 
             testobj = nextobj) {

            nextobj = (obj_template_t *)dlq_nextEntry(testobj);

            if (testobj->objtype != OBJ_TYP_REFINE) {
                continue;
            }

            if (testobj->def.refine->targobj == cobj) {
                /* duplicate refine found, remove and combine */
                dlq_remove(testobj);
                res = combine_refine_objects(tkc,
                                             mod,
                                             chobj,
                                             testobj,
                                             cobj);
                obj_free_template(testobj);
                CHK_EXIT(res, retres);
            }
        }
    }

    /* at this point there should be one refine object for
     * each target specified, and no duplicates, unless fatal
     * error so will not continue anyways
     */
    return retres;
                                    
}  /* resolve_uses */


/********************************************************************
* FUNCTION expand_uses
* 
* Expand the indicated grouping inline, inserted into 
* datadefQ just before the uses object node

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   obj == obj_template_t that contains the obj_uses_t to check
*   datadefQ == Q that obj is stored in (needed to check for dup. err)
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    expand_uses (yang_pcb_t *pcb,
                 tk_chain_t *tkc,
                 ncx_module_t  *mod,
                 obj_template_t *obj,
                 dlq_hdr_t *datadefQ)
{
    obj_uses_t     *uses;
    obj_template_t *chobj, *newobj, *testobj;
    const xmlChar  *name;
    status_t        res, retres;
    boolean         config;

    retres = NO_ERR;
    uses = obj->def.uses;

    if (!uses->grp) {
        /* this node has errors, currently proccessing for
         * errors only, so just keep going
         */
        if (LOGDEBUG) {
            log_debug("\nSkipping uses w/errors in mod %s on line %u",
                      mod->name,
                      obj->tkerr.linenum);
        }
                      
        return NO_ERR;
    }

    if (obj_get_status(obj) == NCX_STATUS_OBSOLETE) {
        if (LOGDEBUG) {
            log_debug("\nSkip expand of obsolete uses '%s' "
                      "in %smodule '%s'",
                      uses->grp,
                      mod->ismod ? "" : "sub",
                      mod->name);
        }
        return NO_ERR;
    }

#ifdef YANG_OBJ_DEBUG_USES
    if (LOGDEBUG4) {
        log_debug4("\nexpand_uses: uses '%s' in mod '%s' on line %u",
                   uses->grp->name,
                   mod->name,
                   obj->tkerr.linenum);
    }
#endif

    if (!uses->grp->expand_done) {
        /* go through the grouping and make sure all the
         * nested uses-stmts are expanded first
         */

#ifdef YANG_OBJ_DEBUG_USES
        if (LOGDEBUG4) {
            log_debug4("\nexpand_uses: need expand of grouping %s",
                       uses->grp->name);
            }
#endif

        res = yang_obj_resolve_uses(pcb,
                                    tkc,
                                    mod,
                                    &uses->grp->datadefQ);
        CHK_EXIT(res, retres);
        uses->grp->expand_done = TRUE;
    }

    /* go through each node in the grouping
     * make sure it is not already in the same datadefQ
     * as the uses; don't check the module name since
     * augments has not been expanded yet
     * clone the object and add it inline to the datadefQ
     */
    for (chobj = (obj_template_t *)
             dlq_firstEntry(&uses->grp->datadefQ);
         chobj != NULL;
         chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

#ifdef YANG_OBJ_DEBUG_USES
        if (LOGDEBUG4) {
            log_debug4("\nexpand_uses: object %s in mod %s on line %u",
                       obj_get_name(chobj),
                       mod->name,
                       chobj->tkerr.linenum);
        }
#endif

        switch (chobj->objtype) {
        case OBJ_TYP_USES:    /* expand should already be done */
        case OBJ_TYP_AUGMENT:
        case OBJ_TYP_REFINE:
            break;
        default:
            name = obj_get_name(chobj);
            testobj = obj_find_template_test(datadefQ, NULL, name);
            if (testobj) {
                log_error("\nError: object '%s' already defined at line %u",
                          name,
                          testobj->tkerr.linenum);
                retres = ERR_NCX_DUP_ENTRY;
                tkc->curerr = &chobj->tkerr;
                ncx_print_errormsg(tkc, mod, retres);
            } else {
                newobj = obj_clone_template(mod,
                                            chobj,
                                            uses->datadefQ);
                if (!newobj) {
                    retres = ERR_INTERNAL_MEM;
                    tkc->curerr = &chobj->tkerr;
                    ncx_print_errormsg(tkc, mod, retres);
                    return retres;
                } else {
                    /* set the object module (and namespace)
                     * to the target, not the module w/ grouping
                     * !!! this does not work -- it just sets
                     * !!! the top-level node being expanded;
                     * !!! all the children get the old module name
                     *
                     * !!! newobj->tkerr.mod = obj->tkerr.mod;
                     */
                    newobj->parent = obj->parent;
                    newobj->usesobj = obj;

                    if (!(newobj->flags & OBJ_FL_CONFSET)) {
                        config = obj_get_config_flag_deep(newobj);
                        if (config) {
                            newobj->flags |= OBJ_FL_CONFIG;
                        } else {
                            newobj->flags &= ~OBJ_FL_CONFIG;
                        }
                    }
                    
                    dlq_insertAhead(newobj, obj);

#ifdef YANG_OBJ_DEBUG_USES
                    if (LOGDEBUG4) {
                        log_debug4("\nexpand_uses: add new "
                                   "obj '%s' to parent '%s',"
                                   " uses.%u",
                                   obj_get_name(newobj),
                                   (obj->grp) ? obj->grp->name :
                                   ((obj->parent) ? 
                                    obj_get_name(obj->parent) : NCX_EL_NONE),
                                   obj->tkerr.linenum);
                    }
#endif
                }
            }
        }
    }

    /* go through each node in the uses datadefQ
     * looking for augments within the uses
     * expand the augment within the same Q
     * as the uses
     */
    for (chobj = (obj_template_t *)
             dlq_firstEntry(uses->datadefQ);
         chobj != NULL;
         chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

        if (chobj->objtype != OBJ_TYP_AUGMENT) {
            continue;
        }

#ifdef YANG_OBJ_DEBUG_USES
        if (LOGDEBUG3) {
            log_debug3("\nexpand_uses_augment: "
                       "mod %s, augment on line %u",
                       mod->name, 
                       chobj->tkerr.linenum);
        }
#endif

        res = expand_augment(pcb,
                             tkc, 
                             mod, 
                             chobj, 
                             datadefQ);
        CHK_EXIT(res, retres);
    }

#ifdef YANG_OBJ_DEBUG_USES
    if (LOGDEBUG4) {
        log_debug4("\nyang_obj: uses '%s'; datadefQ after expand",
                   uses->grp->name);
        obj_dump_child_list(datadefQ,
                            NCX_DEF_INDENT,
                            NCX_DEF_INDENT);
    }
#endif

    return retres;
                                    
}  /* expand_uses */


/********************************************************************
* FUNCTION resolve_augment
* 
* Check the augment object type

* Error messages are printed by this function!!
 Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   aug == obj_augment_t to check
*   obj == parent object for 'aug'
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_augment (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     obj_augment_t *aug,
                     obj_template_t *obj)
{
    status_t       res, retres;

    
    retres = NO_ERR;

    res = check_parent(tkc, mod, obj);
    CHK_EXIT(res, retres);

    /* figure out augment target later */

    /* check if correct target Xpath string form is present */
    if (obj->parent && !obj_is_root(obj->parent) && 
        aug->target && *aug->target == '/') {
        /* absolute-schema-nodeid target not allowed */
        log_error("\nError: absolute schema-nodeid form"
                  " not allowed in nested augment statement");
        retres = ERR_NCX_INVALID_VALUE;
        SET_OBJ_CURERR(tkc, obj);
        ncx_print_errormsg(tkc, mod, retres);
    }

    /* check if correct target Xpath string form is present */
    if ((!obj->parent || obj_is_root(obj->parent)) && 
        (aug->target && *aug->target != '/')) {
        /* absolute-schema-nodeid target must be used */
        log_error("\nError: descendant schema-nodeid form"
                  " not allowed in top-level augment statement");
        retres = ERR_NCX_INVALID_AUGTARGET;
        SET_OBJ_CURERR(tkc, obj);
        ncx_print_errormsg(tkc, mod, retres);
    }

    /* resolve augment contents */
    res = yang_obj_resolve_datadefs(pcb, 
                                    tkc, 
                                    mod, 
                                    &aug->datadefQ);
    CHK_EXIT(res, retres);

    return retres;
                                    
}  /* resolve_augment */


/********************************************************************
* FUNCTION expand_augment
* 
* Expand the indicated top-level or nested augment inline,
* inserted into the tree at the specified node
*
* Note that nested augment clauses are only allowed to
* use the descendant-schema-nodeid form of Xpath expression,
* and the target must therefore be within the sibling sub-trees
* contained in the datadefQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   obj == obj_template_t that contains the obj_augment_t to check
*   datadefQ == Q of obj_template_t that contains 'obj'
*        
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    expand_augment (yang_pcb_t *pcb,
                    tk_chain_t *tkc,
                    ncx_module_t  *mod,
                    obj_template_t *obj,
                    dlq_hdr_t *datadefQ)
{
    obj_augment_t     *aug;
    obj_template_t    *targobj, *chobj, *newobj, *testobj;
    const xmlChar     *name;
    dlq_hdr_t         *targQ, *augQ;
    status_t           res, retres;
    boolean            augextern, augdefcase, config;
    
    aug = obj->def.augment;
    if (!aug->target) {
        /* this node has errors, currently proccessing for
         * errors only, so just keep going
         */
        return NO_ERR;
    }

    if (obj_get_status(obj) == NCX_STATUS_OBSOLETE) {
        if (LOGDEBUG) {
            log_debug("\nSkip expand of obsolete augment '%s' "
                      "in %smodule '%s'",
                      aug->target,
                      mod->ismod ? "" : "sub",
                      mod->name);
        }
        return NO_ERR;
    }

    augQ = &aug->datadefQ;
    retres = NO_ERR;
    targobj = NULL;
    targQ = NULL;

    /* find schema-nodeid target
     * the node being augmented MUST exist to be valid
     */
    res = xpath_find_schema_target(pcb,
                                   tkc, 
                                   mod, 
                                   obj, 
                                   datadefQ,
                                   aug->target, 
                                   &targobj,
                                   NULL);
    if (res != NO_ERR) {
        return res;
    }
        
    aug->targobj = targobj;

    augextern = xml_strcmp(obj_get_mod_name(obj),
                           obj_get_mod_name(targobj));

    augdefcase = (targobj->objtype == OBJ_TYP_CASE && targobj->parent
                  && targobj->parent->def.choic->defval &&
                  !xml_strcmp(obj_get_name(targobj),
                              targobj->parent->def.choic->defval)) 
        ? TRUE : FALSE;
    
    /* check external augment for mandatory nodes */
    if (augextern || augdefcase) {

        /* check that all the augment nodes are optional */
        for (testobj = (obj_template_t *)dlq_firstEntry(augQ);
             testobj != NO_ERR;
             testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

            if (!obj_has_name(testobj)) {
                continue;
            }
        
            if (obj_is_mandatory(testobj)) {
                if (augextern) {
                    log_error("\nError: Mandatory object '%s' not allowed "
                              "in external augment statement",
                              obj_get_name(testobj));
                } else {
                    log_error("\nError: Mandatory object '%s' not allowed "
                              "in default case '%s'",
                              obj_get_name(testobj), 
                              obj_get_name(targobj));
                }
                
                retres = ERR_NCX_MANDATORY_NOT_ALLOWED;
                tkc->curerr = &testobj->tkerr;
                ncx_print_errormsg(tkc, mod, retres);
            }
        }
    }

    /* make sure the objects augmented the target node
     * are OK for that object type
     */
    switch (targobj->objtype) {
    case OBJ_TYP_RPC:
        retres = ERR_NCX_INVALID_AUGTARGET;
        log_error("\nError: cannot augment rpc node '%s'; use 'input' "
                  "or 'output' instead", 
                  obj_get_name(targobj));
        tkc->curerr = &obj->tkerr;
        ncx_print_errormsg(tkc, mod, retres);
        break;
    case OBJ_TYP_CHOICE:
        for (testobj = (obj_template_t *)dlq_firstEntry(augQ);
             testobj != NULL;
             testobj = (obj_template_t *)dlq_nextEntry(testobj)) {
            switch (testobj->objtype) {
            case OBJ_TYP_RPC:
            case OBJ_TYP_RPCIO:
            case OBJ_TYP_NOTIF:
            case OBJ_TYP_CHOICE:
                retres = ERR_NCX_INVALID_AUGTARGET;
                log_error("\nError: invalid %s '%s' augmenting choice node",
                          obj_get_typestr(testobj),
                          obj_get_name(testobj));
                tkc->curerr = &obj->tkerr;
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_TYP_NONE:
            case OBJ_TYP_USES:
            case OBJ_TYP_AUGMENT:
            case OBJ_TYP_REFINE:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
                break;
            default:
                ;
            }
        }
        break;
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
        break;
    case OBJ_TYP_ANYXML:
        retres = ERR_NCX_INVALID_AUGTARGET;
        log_error("\nError: cannot augment anyxml node '%s'",
                  obj_get_name(targobj));
        tkc->curerr = &obj->tkerr;
        ncx_print_errormsg(tkc, mod, retres);
        break;
    default:
        for (testobj = (obj_template_t *)dlq_firstEntry(augQ);
             testobj != NULL;
             testobj = (obj_template_t *)dlq_nextEntry(testobj)) {
            switch (testobj->objtype) {
            case OBJ_TYP_RPC:
            case OBJ_TYP_RPCIO:
            case OBJ_TYP_NOTIF:
            case OBJ_TYP_CASE:
                retres = ERR_NCX_INVALID_AUGTARGET;
                log_error("\nError: invalid %s '%s' augmenting data node",
                          obj_get_typestr(testobj),
                          obj_get_name(testobj));
                tkc->curerr = &obj->tkerr;
                ncx_print_errormsg(tkc, mod, retres);
                break;
            case OBJ_TYP_NONE:
            case OBJ_TYP_AUGMENT:
            case OBJ_TYP_REFINE:
                retres = SET_ERROR(ERR_INTERNAL_VAL);
                break;
            default:
                ;
            }
        }
    }

    /* get the augment target datadefQ */
    targQ = obj_get_datadefQ(targobj);
    if (!targQ) {
        log_error("\nError: %s '%s' cannot be augmented",
                  obj_get_typestr(targobj),
                  obj_get_name(targobj));
        retres = ERR_NCX_INVALID_AUGTARGET;
        tkc->curerr = &targobj->tkerr;
        ncx_print_errormsg(tkc, mod, retres);
        return retres;
    }

    /* go through each node in the augment
     * make sure it is not already in the same datadefQ
     * if not, then clone the grouping object and add it
     * to the augment target
     */
    for (chobj = (obj_template_t *)dlq_firstEntry(augQ);
         chobj != NULL;
         chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

#ifdef YANG_OBJ_DEBUG
        if (LOGDEBUG4) {
            log_debug4("\nexpand_aug: mod %s, object %s, on line %u",
                       mod->name, obj_get_name(chobj),
                       chobj->tkerr.linenum);
        }
#endif

        switch (chobj->objtype) {
        case OBJ_TYP_USES:    /* expand should already be done */
            break;
        case OBJ_TYP_AUGMENT:
            res = expand_augment(pcb,
                                 tkc, 
                                 mod, 
                                 chobj, 
                                 &aug->datadefQ);
            CHK_EXIT(res, retres);
            break;
        default:
            name = obj_get_name(chobj);

            /* try to find the node in any namespace (warning) */
            testobj = obj_find_template_test(targQ, NULL, name);
            if (testobj && xml_strcmp(obj_get_mod_name(testobj),
                                      obj_get_mod_name(chobj))) {
                if (ncx_warning_enabled(ERR_NCX_DUP_AUGNODE)) {
                    log_warn("\nWarning: sibling object '%s' "
                             "already defined "
                             "in %smodule '%s' at line %u",
                             name, 
                             (testobj->tkerr.mod->ismod) ? "" : "sub",
                             testobj->tkerr.mod->name,
                             testobj->tkerr.linenum);
                    res = ERR_NCX_DUP_AUGNODE;
                    tkc->curerr = &chobj->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                } else if (mod != NULL) {
                    ncx_inc_warnings(mod);
                }
            }

            /* try to find the node in the target namespace (error) */
            testobj = obj_find_template_test(targQ, 
                                             obj_get_mod_name(targobj),
                                             name);
            if (testobj) {
                log_error("\nError: object '%s' already defined "
                          "in %smodule '%s' at line %u",
                          name, 
                          (testobj->tkerr.mod->ismod) ? "" : "sub",
                          testobj->tkerr.mod->name,
                          testobj->tkerr.linenum);
                retres = ERR_NCX_DUP_ENTRY;
                tkc->curerr = &chobj->tkerr;
                ncx_print_errormsg(tkc, mod, retres);
            } else {
                /* OK to create the new name
                 * make a clone of the augment object in
                 * case this augment is inside a grouping
                 */
                if (targobj->objtype == OBJ_TYP_CHOICE) {
                    /* make sure all the child nodes are wrapped
                     * in a OBJ_TYP_CASE node -- this has not
                     * been checked yet
                     */
                    newobj = obj_clone_template_case(mod, chobj, NULL);
                } else {
                    /* create a cloned object with the namespace of the
                     * module defining the augment
                     */
                    newobj = obj_clone_template(mod, chobj, NULL);
                }
                if (!newobj) {
                    res = ERR_INTERNAL_MEM;
                    tkc->curerr = &chobj->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                    return res;
                } else {
                    newobj->parent = targobj;
                    newobj->flags |= OBJ_FL_AUGCLONE;
                    newobj->augobj = obj;
                    obj_set_ncx_flags(newobj);
                    dlq_enque(newobj, targQ);

                    /* may need to set the config flag now, under the context
                     * of the actual target, not within the grouping
                     */
                    if (!(newobj->flags & OBJ_FL_CONFSET)) {
                        config = obj_get_config_flag_deep(newobj);
                        if (config) {
                            newobj->flags |= OBJ_FL_CONFIG;
                        } else {
                            newobj->flags &= ~OBJ_FL_CONFIG;
                        }
                    }

#ifdef YANG_OBJ_DEBUG
                    if (LOGDEBUG4) {
                        log_debug4("\nexpand_aug: add new obj '%s' "
                                   "to target %s.%u, aug.%u",
                                   obj_get_name(newobj),
                                   obj_get_name(targobj),
                                   targobj->tkerr.linenum,
                                   obj->tkerr.linenum);
                    }
#endif
                }
            }
        }
    }

    return retres;
                                    
}  /* expand_augment */


/********************************************************************
* FUNCTION resolve_augextern_final
* 
* Check for cloned lists that need final internal data structure
* modifications
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   datadefQ == Q of obj_template_t to check
*        
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_augextern_final (yang_pcb_t *pcb,
                             tk_chain_t *tkc,
                             ncx_module_t *mod,
                             dlq_hdr_t *datadefQ)
{
    obj_template_t    *chobj;
    dlq_hdr_t         *child_datadefQ;
    status_t           res, retres;

    res = NO_ERR;
    retres = NO_ERR;

    for (chobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         chobj != NULL;
         chobj = (obj_template_t *)dlq_nextEntry(chobj)) {

        if (!obj_has_name(chobj) ||
            obj_get_status(chobj) == NCX_STATUS_OBSOLETE) {
            continue;
        }

        if (chobj->objtype == OBJ_TYP_LIST) {
            res = resolve_list_final(pcb, 
                                     tkc, 
                                     mod, 
                                     chobj->def.list,
                                     chobj);
            CHK_EXIT(res, retres);
        }

        child_datadefQ = obj_get_datadefQ(chobj);
        if (child_datadefQ != NULL) {
            res = resolve_augextern_final(pcb,
                                          tkc,
                                          mod,
                                          child_datadefQ);
            CHK_EXIT(res, retres);
        }
    }

    return retres;
                                    
}  /* resolve_augextern_final */


/********************************************************************
* FUNCTION resolve_augment_final
* 
* Check for cloned lists that need final internal data structure
* modifications
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   obj == obj_template_t of the augment target (augmented obj)
*        
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_augment_final (yang_pcb_t *pcb,
                           tk_chain_t *tkc,
                           ncx_module_t  *mod,
                           obj_template_t *obj)
{
    obj_augment_t     *aug;
    dlq_hdr_t         *targQ;
    status_t           res;
    boolean            augextern;
    
    aug = obj->def.augment;
    if (aug->target == NULL) {
        /* this node has errors, currently proccessing for
         * errors only, so just keep going
         */
        return NO_ERR;
    }

    if (obj_get_status(obj) == NCX_STATUS_OBSOLETE) {
        /* already reported in expand_augment */
        return NO_ERR;
    }

    if (aug->targobj == NULL) {
        return ERR_NCX_OPERATION_FAILED;
    }

    augextern = xml_strcmp(obj_get_mod_name(obj),
                           obj_get_mod_name(aug->targobj));
    if (!augextern) {
        return NO_ERR;
    }

    /* get the augment target datadefQ */
    targQ = obj_get_datadefQ(aug->targobj);
    if (targQ == NULL) {
        return ERR_NCX_OPERATION_FAILED;
    }

    /* go through each node in the augment
     * make sure it is not already in the same datadefQ
     * if not, then clone the grouping object and add it
     * to the augment target
     */
    res = resolve_augextern_final(pcb, tkc, mod, targQ);
    return res;
                                    
}  /* resolve_augment_final */


/********************************************************************
* FUNCTION resolve_deviation
* 
* Resolve and possibly expand the indicated top-level deviation-stmt
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   deviation == obj_deviation_t to validate
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_deviation (yang_pcb_t *pcb,
                       tk_chain_t *tkc,
                       ncx_module_t  *mod,
                       obj_deviation_t *deviation)
{
    obj_deviate_t     *devi, *nextdevi;
    obj_template_t    *targobj;
    const xmlChar     *curval;
    tk_token_t        *curtk;
    xpath_pcb_t       *must, *targmust;
    obj_unique_t      *unique, *targunique;
    dlq_hdr_t         *targQ;
    status_t           res, retres;
    boolean            instancetest, curexists, ingrp;

    retres = NO_ERR;
    targobj = NULL;
    instancetest = FALSE;
    curtk = NULL;

    /* find schema-nodeid target
     * the node being augmented MUST exist to be valid
     */
    res = xpath_find_schema_target(pcb,
                                   tkc, 
                                   mod, 
                                   NULL, 
                                   &mod->datadefQ,
                                   deviation->target, 
                                   &targobj, 
                                   NULL);
    if (res != NO_ERR) {
        return res;
    }
        
    deviation->targobj = targobj;

    deviation->targmodname = xml_strdup(obj_get_mod_name(targobj));
    if (deviation->targmodname == NULL) {
        return ERR_INTERNAL_MEM;
    }

    /* make sure all the deviate statements are 
     * are OK for that object type
     */
    for (devi = (obj_deviate_t *)
             dlq_firstEntry(&deviation->deviateQ);
         devi != NULL;
         devi = nextdevi) {

        nextdevi = (obj_deviate_t *)dlq_nextEntry(devi);

        res = NO_ERR;

        /* check if no sub-clauses are expected */
        if (devi->arg == OBJ_DARG_NOT_SUPPORTED) {
            continue;
        } else if (devi->arg == OBJ_DARG_ADD) {
            /* current clause must not exist */
            instancetest = FALSE;
        } else {
            /* current clause must exist */
            instancetest = TRUE;
        }

        /* check the object type against the clauses
         * in this deviate statement;
         * check type-stmt first
         */
        if (devi->typdef) {
            /* replace type statement entered */
            if (!obj_is_leafy(targobj)) {
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "type-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            } else {
                res = yang_typ_resolve_type(pcb,
                                            tkc,
                                            mod,
                                            devi->typdef,
                                            obj_get_default(targobj),
                                            targobj);
                CHK_EXIT(res, retres);
            }
        }

        /* check if units-stmt was entered */
        if (devi->units) {
            if (!obj_is_leafy(targobj)) {
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "type-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            } else {
                switch (targobj->objtype) {
                case OBJ_TYP_LEAF:
                    curexists = (targobj->def.leaf->units) 
                        ? TRUE : FALSE;
                    break;
                case OBJ_TYP_LEAF_LIST:
                    curexists = (targobj->def.leaflist->units) 
                        ? TRUE : FALSE;
                    break;
                default:
                    curexists = FALSE;
                    res = retres = SET_ERROR(ERR_INTERNAL_VAL);
                }

                if (instancetest && !curexists) {
                    res = retres = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'units' must exist in "
                              "deviate target '%s'",
                              deviation->target);
                    if (tkc) {
                        tkc->curerr = &devi->tkerr;
                    }
                    ncx_print_errormsg(tkc, mod, retres);
                } else if (!instancetest && curexists) {
                    res = retres = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'units' must not exist in "
                              "deviate target '%s'",
                              deviation->target);
                    if (tkc) {
                        tkc->curerr = &devi->tkerr;
                    }
                    ncx_print_errormsg(tkc, mod, retres);
                } else if (devi->arg == OBJ_DARG_DELETE &&
                           xml_strcmp(devi->units,
                                      obj_get_units(targobj))) {
                    res = retres = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'units' value '%s' must match in "
                              "deviate target '%s'",
                              devi->units,
                              deviation->target);
                    if (tkc) {
                        tkc->curerr = &devi->tkerr;
                    }
                    ncx_print_errormsg(tkc, mod, retres);
                }
            }
        }

        /* check if default-stmt entered */
        if (devi->defval) {
            if ((targobj->objtype == OBJ_TYP_LEAF) ||
                targobj->objtype == OBJ_TYP_CHOICE) {

                if (targobj->objtype == OBJ_TYP_CHOICE) {
                    curval = targobj->def.choic->defval;
                } else {
                    curval = targobj->def.leaf->defval;
                }
                curexists = (curval) ? TRUE : FALSE;

                if (instancetest && !curexists) {
                    res = retres = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'default' must exist in "
                              "deviate target '%s'",
                              deviation->target);
                    if (tkc) {
                        tkc->curerr = &devi->tkerr;
                    }
                    ncx_print_errormsg(tkc, mod, retres);
                } else if (!instancetest && curexists) {
                    res = retres = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'default' must not exist in "
                              "deviate target '%s'",
                              deviation->target);
                    if (tkc) {
                        tkc->curerr = &devi->tkerr;
                    }
                    ncx_print_errormsg(tkc, mod, retres);
                } else if (devi->arg == OBJ_DARG_DELETE &&
                           xml_strcmp(devi->defval, curval)) {
                    res = retres = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: 'default' value '%s' must match in "
                              "deviate target '%s'",
                              devi->defval,
                              deviation->target);
                    if (tkc) {
                        tkc->curerr = &devi->tkerr;
                    }
                    ncx_print_errormsg(tkc, mod, retres);
                }
            } else {
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "default-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            }
        }

        /* check if config-stmt entered */
        if (devi->config_tkerr.mod) {
            switch (targobj->objtype) {
            case OBJ_TYP_LEAF:
                ingrp = FALSE;
                if (!devi->config && 
                    obj_is_key(targobj) &&
                    obj_get_config_flag_check(targobj, &ingrp)) {
                    if (!ingrp) {
                        res = retres = ERR_NCX_INVALID_DEV_STMT;
                        log_error("\nError: leaf %s:%s is a key; "
                                  "cannot change config to false",
                                  obj_get_mod_name(targobj),
                                  obj_get_name(targobj));
                        if (tkc) {
                            tkc->curerr = &devi->tkerr;
                        }
                        ncx_print_errormsg(tkc, mod, retres);                   
                    }
                }
                break;
            case OBJ_TYP_CONTAINER:
            case OBJ_TYP_LEAF_LIST:
            case OBJ_TYP_CHOICE:
                break;
            case OBJ_TYP_LIST:
                if (!obj_first_key(targobj) && devi->config) {
                    res = retres = ERR_NCX_INVALID_DEV_STMT;
                    log_error("\nError: list %s:%s has no key; "
                              "cannot change config to true",
                              obj_get_mod_name(targobj),
                              obj_get_name(targobj));
                    if (tkc) {
                        tkc->curerr = &devi->tkerr;
                    }
                    ncx_print_errormsg(tkc, mod, retres);                   
                }
                break;
            default:
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "config-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            }
        }

        /* check if mandatory-stmt entered */
        if (devi->mandatory_tkerr.mod) {
            switch (targobj->objtype) {
            case OBJ_TYP_CHOICE:
            case OBJ_TYP_LEAF:
            case OBJ_TYP_ANYXML:
                break;
            default:
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "mandatory-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            }
        }

        /* check if min-elements stmt entered */
        if (devi->minelems_tkerr.mod) {
            switch (targobj->objtype) {
            case OBJ_TYP_LEAF_LIST:
            case OBJ_TYP_LIST:
                break;
            default:
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "min-elements-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            }
        }

        /* check if max-elements stmt entered */
        if (devi->maxelems_tkerr.mod) {
            switch (targobj->objtype) {
            case OBJ_TYP_LEAF_LIST:
            case OBJ_TYP_LIST:
                break;
            default:
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "max-elements-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            }
        }

        /* check if any must-stmts entered */
        if (!dlq_empty(&devi->mustQ)) {
            switch (targobj->objtype) {
            case OBJ_TYP_CONTAINER:
            case OBJ_TYP_ANYXML:
            case OBJ_TYP_LEAF:
            case OBJ_TYP_LEAF_LIST:
            case OBJ_TYP_LIST:
                targQ = obj_get_mustQ(targobj);
                if (!targQ) {
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
                for (must = (xpath_pcb_t *)dlq_firstEntry(&devi->mustQ);
                     must != NULL;
                     must = (xpath_pcb_t *)dlq_nextEntry(must)) {

                    targmust = xpath_find_pcb(targQ,
                                              must->exprstr);
                    curexists = (targmust) ? TRUE : FALSE;

                    if (instancetest && !curexists) {
                        res = retres = ERR_NCX_INVALID_DEV_STMT;
                        log_error("\nError: 'must %s' must exist in "
                                  "deviate target '%s'",
                                  must->exprstr,
                                  deviation->target);
                        if (tkc) {
                            tkc->curerr = &devi->tkerr;
                        }
                        ncx_print_errormsg(tkc, mod, retres);
                    } else if (!instancetest && curexists) {
                        res = retres = ERR_NCX_INVALID_DEV_STMT;
                        log_error("\nError: 'must %s' must not exist in "
                                  "deviate target '%s'",
                                  must->exprstr,
                                  deviation->target);
                        if (tkc) {
                            tkc->curerr = &devi->tkerr;
                        }
                        ncx_print_errormsg(tkc, mod, retres);
                    }
                }
                break;
            default:
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "must-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            }
        }

        /* check if any unique-stmts entered */
        if (!dlq_empty(&devi->uniqueQ)) {
            switch (targobj->objtype) {
            case OBJ_TYP_LIST:
                targQ = &targobj->def.list->uniqueQ;
                if (!targQ) {
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                    continue;
                }
                for (unique = (obj_unique_t *)
                         dlq_firstEntry(&devi->uniqueQ);
                     unique != NULL;
                     unique = (obj_unique_t *)dlq_nextEntry(unique)) {

                    targunique = obj_find_unique(targQ, unique->xpath);
                    curexists = (targunique) ? TRUE : FALSE;

                    if (instancetest && !curexists) {
                        res = retres = ERR_NCX_INVALID_DEV_STMT;
                        log_error("\nError: 'unique %s' must exist in "
                                  "deviate target '%s'",
                                  unique->xpath,
                                  deviation->target);
                        if (tkc) {
                            tkc->curerr = &devi->tkerr;
                        }
                        ncx_print_errormsg(tkc, mod, retres);
                    } else if (!instancetest && curexists) {
                        res = retres = ERR_NCX_INVALID_DEV_STMT;
                        log_error("\nError: 'unique %s' must not exist in "
                                  "deviate target '%s'",
                                  unique->xpath,
                                  deviation->target);
                        if (tkc) {
                            tkc->curerr = &devi->tkerr;
                        }
                        ncx_print_errormsg(tkc, mod, retres);
                    }
                }
                break;
            default:
                res = retres = ERR_NCX_INVALID_DEV_STMT;
                log_error("\nError: target '%s' is a '%s': "
                          "unique-stmt not allowed",
                          deviation->target,
                          obj_get_typestr(targobj));
                if (tkc) {
                    tkc->curerr = &devi->tkerr;
                }
                ncx_print_errormsg(tkc, mod, retres);
            }
        }

        /* finally, if entire deviate-stmt is OK save it
         * or else toss it
         */
        if (res != NO_ERR) {
            /* toss this deviate-stmt; it is no good */
            dlq_remove(devi);
            obj_free_deviate(devi);
        } /* else leave in this Q for HTML or YANG output */
    }

    return retres;
                                    
}  /* resolve_deviation */


/********************************************************************
* FUNCTION transfer_my_deviations
* 
* Find any deviations for the specified module
* Resolve them, and stored the resolved obj_deviation_t
* in the module deviationQ
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token parse chain to use for errors
*   savedev == save deviations struct to check
*   mod == module in progress; transfer to this deviationQ
*   runningtotal == address of return total deviation count
*                    this will not be zeroed before using!!!
* 
* OUTPUTS:
*   *runningtotal == running total of deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    transfer_my_deviations (yang_pcb_t *pcb,
                            tk_chain_t *tkc,
                            ncx_save_deviations_t *savedev,
                            ncx_module_t *mod,
                            uint32 *runningtotal)
{
    ncx_import_t      *myimport;
    obj_deviation_t   *deviation, *nextdeviation;
    ncx_module_t      *dummymod;
    status_t           res;
    boolean            anydone;

    /* if 'mod->name' import does not show up, then this deviation
     * module cannot possibly contain and deviations for 'mod'
     */
    myimport = ncx_find_import_que(&savedev->importQ,
                                   mod->name);
    if (myimport == NULL) {
        return NO_ERR;
    }

    /* check the revision if it is present to make sure
     * it is not for a different version of 'mod'
     */
    if (myimport->revision) {
        if (mod->version == NULL ||
            xml_strcmp(myimport->revision, mod->version)) {
            return NO_ERR;
        }
    }

    /* mock-up a dummy module for the deviations
     * so the resolve_deviation function can be
     * used directly
     */
    dummymod = ncx_new_module();
    if (dummymod == NULL) {
        return ERR_INTERNAL_MEM;
    }

    /* borrow some pointers from the savedev */
    dummymod->name = savedev->devmodule;

#ifdef YANG_OBJ_DEBUG_MEMORY
    if (LOGDEBUG3) {
        log_debug3("\n   malloced %p module '%s'",
                   dummymod,
                   dummymod->name);
    }
#endif

    dummymod->ismod = TRUE;
    dummymod->prefix = savedev->devprefix;
    myimport->mod = mod;
    anydone = FALSE;

    /* temp move the imports from 1 Q to another */
    dlq_block_enque(&savedev->importQ, 
                    &dummymod->importQ);

    /* the deviations file imported 'mod';
     * check all the deviation targets to find
     * any for 'mod'; remove and resolve if found
     */
    res = NO_ERR;
    for (deviation = (obj_deviation_t *)
             dlq_firstEntry(&savedev->deviationQ);
         deviation != NULL && res == NO_ERR;
         deviation = nextdeviation) {

        nextdeviation = (obj_deviation_t *)
            dlq_nextEntry(deviation);

        if (deviation->targobj == NULL) {
            /* deviation has not been resolved yet;
             * or the target was never found
             * and this fn call will fail
             */
            res = resolve_deviation(pcb,
                                    tkc,
                                    dummymod,
                                    deviation);
        }

        if (res == NO_ERR) {
            if (deviation->targobj->tkerr.mod == mod) {
                if (LOGDEBUG) {
                    log_debug("\nAdding external deviation "
                              "to '%s', from '%s' to '%s'",
                              obj_get_name(deviation->targobj),
                              savedev->devmodule,
                              mod->name);
                }

                /* transfer the deviation to the target module */
                dlq_remove(deviation);
                dlq_enque(deviation, &mod->deviationQ);
                anydone = TRUE;
                (*runningtotal)++;
            }
        }
    }

    /* check if any devmodlist entry needed */
    if (anydone) {
        res = ncx_set_list(NCX_BT_STRING,
                           savedev->devmodule,
                           &mod->devmodlist);
        if (res != NO_ERR) {
            log_error("\nError: set list failed (%s), cannot add deviation",
                      get_error_string(res));
        }
    }

    /* restore the savedev structure */
    myimport->mod = NULL;
    dlq_block_enque(&dummymod->importQ, &savedev->importQ);
    dummymod->name = NULL;
    dummymod->prefix = NULL;
    ncx_free_module(dummymod);

    return res;

} /* transfer_my_deviations */


/********************************************************************
* FUNCTION resolve_rpc
* 
* Check the rpc method object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   rpc == obj_rpc_t to check
*   obj == parent object for 'rpc'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_rpc (yang_pcb_t *pcb,
                 tk_chain_t *tkc,
                 ncx_module_t  *mod,
                 obj_rpc_t *rpc,
                 obj_template_t *obj,
                 boolean redo)
{
    status_t          res, retres;

    retres = NO_ERR;

    if (!redo) {
        res = yang_typ_resolve_typedefs(pcb,
                                        tkc, 
                                        mod, 
                                        &rpc->typedefQ, obj);
        CHK_EXIT(res, retres);

        res = yang_grp_resolve_groupings(pcb, 
                                         tkc, 
                                         mod, 
                                         &rpc->groupingQ, 
                                         obj);
        CHK_EXIT(res, retres);
    }

    res = resolve_datadefs(pcb, 
                           tkc, 
                           mod, 
                           &rpc->datadefQ, 
                           redo);
    CHK_EXIT(res, retres);

    return retres;
                                    
}  /* resolve_rpc */


/********************************************************************
* FUNCTION resolve_rpcio
* 
* Check the rpc input or output object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   rpcio == obj_rpcio_t to check
*   obj == parent object for 'rpcio'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_rpcio (yang_pcb_t *pcb,
                   tk_chain_t *tkc,
                   ncx_module_t  *mod,
                   obj_rpcio_t *rpcio,
                   obj_template_t *obj,
                   boolean redo)
{
    status_t              res, retres;

    retres = NO_ERR;

    if (!redo) {
        res = yang_typ_resolve_typedefs(pcb,
                                        tkc, 
                                        mod, 
                                        &rpcio->typedefQ, 
                                        obj);
        CHK_EXIT(res, retres);

        res = yang_grp_resolve_groupings(pcb,
                                         tkc, 
                                         mod, 
                                         &rpcio->groupingQ, 
                                         obj);
        CHK_EXIT(res, retres);
    }

    res = resolve_datadefs(pcb, 
                           tkc, 
                           mod, 
                           &rpcio->datadefQ, 
                           redo);
    CHK_EXIT(res, retres);

    return retres;
                                    
}  /* resolve_rpcio */


/********************************************************************
* FUNCTION resolve_notif
* 
* Check the notification object type

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   notif == obj_notif_t to check
*   obj == parent object for 'notif'
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_notif (yang_pcb_t *pcb,
                   tk_chain_t *tkc,
                   ncx_module_t  *mod,
                   obj_notif_t *notif,
                   obj_template_t *obj,
                   boolean redo)
{
    status_t          res, retres;

    retres = NO_ERR;

    if (!redo) {
        res = yang_typ_resolve_typedefs(pcb,
                                        tkc, 
                                        mod, 
                                        &notif->typedefQ, 
                                        obj);
        CHK_EXIT(res, retres);

        res = yang_grp_resolve_groupings(pcb,
                                         tkc, 
                                         mod, 
                                         &notif->groupingQ, 
                                         obj);
        CHK_EXIT(res, retres);
    }

    /* resolve notification contents */
    res = resolve_datadefs(pcb, 
                           tkc, 
                           mod, 
                           &notif->datadefQ, 
                           redo);
    CHK_EXIT(res, retres);

    return retres;
                                    
}  /* resolve_notif */


/********************************************************************
* FUNCTION resolve_datadef
* 
* First pass object validation
*
* Analyze the entire datadefQ within the module struct
* Finish all the clauses within this struct that
* may have been defered because of possible forward references
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   testobj == obj_template_t to check
*   redo == TRUE if this is a 2nd pass due to deviations added
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_datadef (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     obj_template_t *testobj,
                     boolean redo)
{
    status_t         res, retres;

    res = NO_ERR;
    retres = NO_ERR;

#ifdef YANG_OBJ_DEBUG
    if (LOGDEBUG4) {
        log_debug4("\nyang_obj_resolve: %s", obj_get_name(testobj));
    }
#endif

    if (!redo) {
        res = ncx_resolve_appinfoQ(pcb,
                                   tkc, 
                                   mod, 
                                   &testobj->appinfoQ);
        CHK_EXIT(res, retres);

        res = resolve_iffeatureQ(pcb,
                                 tkc, 
                                 mod, 
                                 testobj);
        CHK_EXIT(res, retres);
    }

    switch (testobj->objtype) {
    case OBJ_TYP_CONTAINER:
        res = resolve_container(pcb,
                                tkc, 
                                mod,
                                testobj->def.container, 
                                testobj, 
                                redo);
        break;
    case OBJ_TYP_LEAF:
    case OBJ_TYP_ANYXML:
        res = resolve_leaf(pcb,
                           tkc, 
                           mod,
                           testobj->def.leaf, 
                           testobj, 
                           redo);
        break;
    case OBJ_TYP_LEAF_LIST:
        res = resolve_leaflist(pcb,
                               tkc, 
                               mod,
                               testobj->def.leaflist, 
                               testobj, 
                               redo);
        break;
    case OBJ_TYP_LIST:
        res = resolve_list(pcb,
                           tkc, 
                           mod,
                           testobj->def.list, 
                           testobj, 
                           redo);
        break;
    case OBJ_TYP_CHOICE:
        res = resolve_choice(pcb,
                             tkc, 
                             mod,
                             testobj->def.choic, 
                             testobj, 
                             redo);
        break;
    case OBJ_TYP_CASE:
        res = resolve_case(pcb,
                           tkc, 
                           mod,
                           testobj->def.cas, 
                           testobj, 
                           redo);
        break;
    case OBJ_TYP_USES:
        if (!redo) {
            res = resolve_uses(pcb,
                               tkc, 
                               mod,
                               testobj->def.uses, 
                               testobj);
        }
        break;
    case OBJ_TYP_AUGMENT:
        if (!redo) {
            res = resolve_augment(pcb,
                                  tkc, 
                                  mod,
                                  testobj->def.augment, 
                                  testobj);
        }
        break;
    case OBJ_TYP_RPC:
        res = resolve_rpc(pcb,
                          tkc, 
                          mod,
                          testobj->def.rpc, 
                          testobj, 
                          redo);
        break;
    case OBJ_TYP_RPCIO:
        res = resolve_rpcio(pcb,
                            tkc, 
                            mod,
                            testobj->def.rpcio, 
                            testobj, 
                            redo);
        break;
    case OBJ_TYP_NOTIF:
        res = resolve_notif(pcb,
                            tkc, 
                            mod,
                            testobj->def.notif, 
                            testobj, 
                            redo);
        break;
    case OBJ_TYP_REFINE:
        res = NO_ERR;
        break;
    case OBJ_TYP_NONE:
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* set the flags again; they were already
     * set once during the consume_foo function
     * which means that named typedefs and other
     * references are not resolved yet; need to
     * go through again and check some flags
     */
    obj_set_ncx_flags(testobj);

    CHK_EXIT(res, retres);
    return retres;

}  /* resolve_datadef */


/********************************************************************
* FUNCTION resolve_datadefs
* 
* First pass object validation
*
* Analyze the entire datadefQ within the module struct
* Finish all the clauses within this struct that
* may have been defered because of possible forward references
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_datadefs (yang_pcb_t *pcb,
                      tk_chain_t *tkc,
                      ncx_module_t  *mod,
                      dlq_hdr_t *datadefQ,
                      boolean redo)
{
    obj_template_t  *testobj;
    status_t         res, retres;

    retres = NO_ERR;

    /* first resolve all the local type names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

        res = resolve_datadef(pcb, tkc, mod, testobj, redo);
        CHK_EXIT(res, retres);
    }

    return retres;

}  /* resolve_datadefs */


/********************************************************************
* FUNCTION consume_datadef
* 
* Parse the next N tokens as a data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
*   
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == grp_template_t parent or NULL if parent is not a grouping
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    consume_datadef (yang_pcb_t *pcb,
                     tk_chain_t *tkc,
                     ncx_module_t  *mod,
                     dlq_hdr_t *que,
                     obj_template_t *parent,
                     grp_template_t *grp)
{
    const xmlChar   *val;
    const char      *expstr;
    tk_type_t        tktyp;
    boolean          errdone;
    status_t         res;

    expstr = "anyxml, container, leaf, leaf-list, list, choice, uses,"
        "or augment keyword";
    errdone = TRUE;
    res = NO_ERR;
    tktyp = TK_CUR_TYP(tkc);
    val = TK_CUR_VAL(tkc);

    /* check the current token type */
    if (tktyp != TK_TT_TSTRING) {
        res = ERR_NCX_WRONG_TKTYPE;
        errdone = FALSE;
    } else {
        /* Got a token string so check the value */
        if (!xml_strcmp(val, YANG_K_ANYXML)) {
            res = consume_anyxml(tkc, 
                                 mod, 
                                 que, 
                                 parent, 
                                 grp);
        } else if (!xml_strcmp(val, YANG_K_CONTAINER)) {
            res = consume_container(pcb,
                                    tkc, 
                                    mod, 
                                    que,
                                    parent,
                                    grp);
        } else if (!xml_strcmp(val, YANG_K_LEAF)) {
            res = consume_leaf(pcb,
                               tkc, 
                               mod,
                               que,
                               parent,
                               grp);
        } else if (!xml_strcmp(val, YANG_K_LEAF_LIST)) {
            res = consume_leaflist(pcb,
                                   tkc,
                                   mod,
                                   que,
                                   parent, 
                                   grp);
        } else if (!xml_strcmp(val, YANG_K_LIST)) {
            res = consume_list(pcb,
                               tkc,
                               mod,
                               que,
                               parent, 
                               grp);
        } else if (!xml_strcmp(val, YANG_K_CHOICE)) {
            res = consume_choice(pcb,
                                 tkc,
                                 mod,
                                 que,
                                 parent, 
                                 grp);
        } else if (!xml_strcmp(val, YANG_K_USES)) {
            res = consume_uses(pcb,
                               tkc, 
                               mod, 
                               que, 
                               parent, 
                               grp);
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            errdone = FALSE;
        }
    }

    if (res != NO_ERR && !errdone) {
        log_error("\nError: '%s' token not allowed here", val);
        ncx_mod_exp_err(tkc, mod, res, expstr);
        yang_skip_statement(tkc, mod);
    }

    return res;

}  /* consume_datadef */


/********************************************************************
* FUNCTION resolve_iffeatureQ
* 
* Check the Q of if-feature statements for the specified object

* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*   obj == object to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_iffeatureQ (yang_pcb_t *pcb,
                        tk_chain_t *tkc,
                        ncx_module_t  *mod,
                        obj_template_t *obj)
{
    ncx_feature_t    *testfeature;
    ncx_iffeature_t  *iff;
    status_t          res, retres;
    boolean           errdone;

    retres = NO_ERR;

    /* check if there are any if-feature statements inside
     * this object that need to be resolved
     */
    for (iff = (ncx_iffeature_t *)
             dlq_firstEntry(&obj->iffeatureQ);
         iff != NULL;
         iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

        testfeature = NULL;
        errdone = FALSE;
        res = NO_ERR;

        if (iff->prefix &&
            xml_strcmp(iff->prefix, mod->prefix)) {
            /* find the feature in another module */
            res = yang_find_imp_feature(pcb,
                                        tkc, 
                                        mod, 
                                        iff->prefix,
                                        iff->name, 
                                        &iff->tkerr,
                                        &testfeature);
            if (res != NO_ERR) {
                retres = res;
                errdone = TRUE;
            }
        } else {
            testfeature = ncx_find_feature(mod, iff->name);
        }

        if (!testfeature && !errdone) {
            log_error("\nError: Feature '%s' not found "
                      "for if-feature statement in object '%s'",
                      iff->name, 
                      obj_get_name(obj));
            res = retres = ERR_NCX_DEF_NOT_FOUND;
            tkc->curerr = &iff->tkerr;
            ncx_print_errormsg(tkc, mod, retres);
        }

        if (testfeature) {
            iff->feature = testfeature;
        }
    }

    /* check the feature mismatch corner cases later,
     * after the OBJ_FL_KEY flags have been set
     */
    return retres;
                                    
}  /* resolve_iffeatureQ */


/********************************************************************
* FUNCTION check_iffeature_mismatch
* 
* Check the child object against the ancestor node for 1 if-feature
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   ancestor == ancestor node of child to compare against
*               and stop the check
*   testobj == current object being checked
*   iff == current if-feature record within the testobj to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_iffeature_mismatch (tk_chain_t *tkc,
                              ncx_module_t  *mod,
                              obj_template_t *ancestor,
                              obj_template_t *testobj,
                              ncx_iffeature_t *iff)

{
    status_t  res;

    res = NO_ERR;

    if (!ncx_find_iffeature(&ancestor->iffeatureQ,
                            iff->prefix,
                            iff->name,
                            mod->prefix)) {
        res = ERR_NCX_INVALID_CONDITIONAL;
        log_error("\nError: 'if-feature %s' present for "
                  "%s %s, but not in list %s",
                  iff->name, 
                  obj_get_typestr(testobj),
                  obj_get_name(testobj),
                  obj_get_name(ancestor));
        tkc->curerr = &iff->tkerr;
        ncx_print_errormsg(tkc, mod, res);
    }
    return res;
}  /* check_iffeature_mismatch */


/********************************************************************
* FUNCTION check_one_when_mismatch
* 
* Check one object when clause(s) against another
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   test1 == node to check
*   test2 == node to check
* 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_one_when_mismatch (tk_chain_t *tkc,
                             ncx_module_t  *mod,
                             obj_template_t *test1,
                             obj_template_t *test2)
{
    if (!test1->when) {
        return NO_ERR;
    }

    if (test2->when) {
        if (!xml_strcmp(test1->when->exprstr,
                        test2->when->exprstr)) {
            return NO_ERR;
        }
    }
        
    if (test2->usesobj && test2->usesobj->when) {
        if (!xml_strcmp(test1->when->exprstr,
                        test2->usesobj->when->exprstr)) {
            return NO_ERR;
        }
    }
        
    if (test2->augobj && test2->augobj->when) {
        if (!xml_strcmp(test1->when->exprstr,
                        test2->augobj->when->exprstr)) {
            return NO_ERR;
        }
    }

    log_error("\nError: when-stmt '%s' not in affect "
              "for list %s",
              test1->when, obj_get_name(test2));
    tkc->curerr = &test1->when->tkerr;
    ncx_print_errormsg(tkc, mod, ERR_NCX_INVALID_CONDITIONAL);

    return ERR_NCX_INVALID_CONDITIONAL;
                                    
}  /* check_one_conditional_mismatch */


/********************************************************************
* FUNCTION check_conditional_mismatch
* 
* Check the child object against the ancestor node to see
* if any child conditionals are present that are not
* present in the path to the ancestor.  Treat this
* as an error
*
* Do not call for every node!
*    - (parent-list, key-leaf)
*    - (unique-list, unique-node)
*    - (parent-choice, default-case)
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain
*   mod == module in progress
*   ancestor == ancestor node of child to compare against
*               and stop the check
*   child == child object to check
* 
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    check_conditional_mismatch (tk_chain_t *tkc,
                                ncx_module_t  *mod,
                                obj_template_t *ancestor,
                                obj_template_t *child)
{
    obj_template_t   *testobj;
    ncx_iffeature_t  *iff;
    status_t          res, retres;
    boolean           done;

    retres = NO_ERR;

    testobj = child->parent;
    if (!testobj) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (ancestor->objtype==OBJ_TYP_CHOICE && 
        child->objtype==OBJ_TYP_CASE) {


    } else {
        /* make sure that the child node does not introduce
         * any if-features that are not in the ancestor node
         */
        testobj = child;
        done = FALSE;

        /* go up the tree from the child to the level before
         * the ancestor
         */
        while (!done) {
            /* check all possible when clauses in the testobj
             * against the ancestor node
             */
            res = check_one_when_mismatch(tkc, 
                                          mod, 
                                          testobj,
                                          ancestor);
            CHK_EXIT(res, retres);

            if (testobj->usesobj) {
                res = check_one_when_mismatch(tkc, 
                                              mod, 
                                              testobj->usesobj,
                                              ancestor);
                CHK_EXIT(res, retres);
            }

            if (testobj->augobj) {
                res = check_one_when_mismatch(tkc, 
                                              mod, 
                                              testobj->augobj,
                                              ancestor);
                CHK_EXIT(res, retres);
            }

            /* check all the if-features in the testnode
             * against the ones in the ancestor;
             * a missing if-feature in the ancestor is an error
             */
            for (iff = (ncx_iffeature_t *)
                     dlq_firstEntry(&testobj->iffeatureQ);
                 iff != NULL;
                 iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

                res = check_iffeature_mismatch(tkc, 
                                               mod, 
                                               ancestor,
                                               testobj, 
                                               iff);
                CHK_EXIT(res, retres);
            }


            /* check any extra if-features from the uses object */
            if (testobj->usesobj) {
                for (iff = (ncx_iffeature_t *)
                         dlq_firstEntry(&testobj->usesobj->iffeatureQ);
                     iff != NULL;
                     iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

                    if (!ncx_find_iffeature(&testobj->iffeatureQ,
                                            iff->prefix,
                                            iff->name,
                                            mod->prefix)) {

                        res = check_iffeature_mismatch(tkc, 
                                                       mod, 
                                                       ancestor,
                                                       testobj, 
                                                       iff);
                        CHK_EXIT(res, retres);
                    }
                }
            }

            /* check any extra if-features from the augment object */
            if (testobj->augobj) {
                for (iff = (ncx_iffeature_t *)
                         dlq_firstEntry(&testobj->augobj->iffeatureQ);
                     iff != NULL;
                     iff = (ncx_iffeature_t *)dlq_nextEntry(iff)) {

                    if (!ncx_find_iffeature(&testobj->iffeatureQ,
                                            iff->prefix,
                                            iff->name,
                                            mod->prefix)) {

                        res = check_iffeature_mismatch(tkc, 
                                                       mod, 
                                                       ancestor,
                                                       testobj, 
                                                       iff);
                        CHK_EXIT(res, retres);
                    }
                }
            }

            testobj = testobj->parent;
            if (!testobj) {
                return SET_ERROR(ERR_INTERNAL_VAL);
            }
            if (testobj == ancestor) {
                done = TRUE;
            }
        }
    }

    return retres;
                                    
}  /* check_conditional_mismatch */


/********************************************************************
* FUNCTION resolve_xpath
* 
* Fifth (and final) pass object validation
*
* Check all leafref, must, and when XPath expressions
* to make sure they are well-formed
*
* Checks the cooked objects, and skips all groupings
* uses, and augment nodes
*
* MUST BE CALLED AFTER yang_obj_resolve_final
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == [sub]module in progress within the tree walking
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
    resolve_xpath (tk_chain_t *tkc,
                   ncx_module_t  *mod,
                   dlq_hdr_t *datadefQ)
{
    obj_template_t        *testobj;
    obj_template_t        *leafobj;
    obj_key_t             *key;
    obj_unique_t          *uniq;
    obj_unique_comp_t     *uncomp;
    typ_def_t             *typdef;
    xpath_pcb_t           *pcb;
    xpath_pcb_t           *pcbclone;
    status_t               res, retres;
    boolean                is_targetmod;

    res = NO_ERR;
    retres = NO_ERR;

    /* check the must and when stmts in the entire subtree */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

        /* check corner case -- submodules augmenting other
         * submodules; only evaluate in the context of the
         * submodule that defined the objects (augmentor, 
         * not augmentee)
         */

        is_targetmod = (testobj->tkerr.mod == mod);

        if (LOGDEBUG4) {
            if (!obj_has_name(testobj)) {
                log_debug4("\nresolve_xpath: %s", 
                           obj_get_typestr(testobj));
            } else {
                xmlChar *mybuff = NULL;
                res = obj_gen_object_id(testobj, &mybuff);
                if (res == NO_ERR) {
                    log_debug4("\nresolve_xpath: %s", mybuff);
                } else {
                    log_debug4("\nresolve_xpath: %s", obj_get_name(testobj));
                }
                if (mybuff) {
                    m__free(mybuff);
                }
            }
            if (is_targetmod) {
                log_debug4(" (targmod)");
            }
        }


        /* check the when-stmt in the object itself 
         * check uses and augment since they can have
         * their own when statements
         */
        if (testobj->when && is_targetmod) {
            res = resolve_when(mod, testobj->when, testobj);
            CHK_EXIT(res, retres);
        }

        if (!obj_has_name(testobj)) {
            /* skip augment and uses for the rest of the tests */
            continue;
        }

        /* validate correct Xpath in must clauses */
        if (is_targetmod) {
            res = resolve_mustQ(tkc, mod, testobj);
            CHK_EXIT(res, retres);
        }
        
        switch (testobj->objtype) {
        case OBJ_TYP_CONTAINER:
            /* check container children */
            res = resolve_xpath(tkc, 
                                mod, 
                                testobj->def.container->datadefQ);
            break;
        case OBJ_TYP_ANYXML:
            break;
        case OBJ_TYP_LEAF:
        case OBJ_TYP_LEAF_LIST:
            if (!is_targetmod) {
                break;
            }
            if (obj_get_basetype(testobj) == NCX_BT_LEAFREF) {

#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG3) {
                    log_debug3("\nresolve_xpath: leafref in mod %s, "
                               "object %s, on line %u",
                               mod->name,
                               obj_get_name(testobj), 
                               testobj->tkerr.linenum);
                }
#endif

                /* need to make a copy of the XPath PCB because
                 * typedefs are treated as read-only when referenced
                 * from a val_value_t node
                 */
                typdef = obj_get_typdef(testobj);
                pcb = typ_get_leafref_pcb(typdef);

#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG3) {
                    log_debug3(" expr: %s", pcb->exprstr);
                }
#endif

                pcbclone = xpath_clone_pcb(pcb);
                if (!pcbclone) {
                    res = ERR_INTERNAL_MEM;
                } else {
                    tkc->curerr = &pcb->tkerr;
                    res = xpath_yang_parse_path(tkc, 
                                                mod, 
                                                XP_SRC_LEAFREF,
                                                pcbclone);
                    if (res == NO_ERR) {
                        leafobj = NULL;
                        res = xpath_yang_validate_path(mod, 
                                                       testobj, 
                                                       pcbclone, 
                                                       FALSE,
                                                       &leafobj);
                        if (res == NO_ERR && leafobj) {
                            typ_set_xref_typdef(typdef, 
                                                obj_get_typdef(leafobj));
                            if (testobj->objtype == OBJ_TYP_LEAF) {
                                testobj->def.leaf->leafrefobj = leafobj;
                            } else {
                                testobj->def.leaflist->leafrefobj = leafobj;
                            }
                        }
                    }
                    xpath_free_pcb(pcbclone);
                }
            }

#ifdef YANG_OBJ_DEBUG
            if (LOGDEBUG3 && res != NO_ERR) {
                log_debug3("\nresolve_xpath: FAILED (%s)",
                           get_error_string(res));
            }
#endif

            break;
        case OBJ_TYP_LIST:
            /* check that none of the key leafs have more
             * conditionals than their list parent
             */
            if (is_targetmod) {
                for (key = obj_first_key(testobj);
                     key != NULL;
                     key = obj_next_key(key)) {

                    if (key->keyobj) {
                        res = check_conditional_mismatch(tkc, 
                                                         mod,
                                                         testobj,
                                                         key->keyobj);
                        CHK_EXIT(res, retres);
                    }
                }
            }

            /* check that none of the unique set leafs have more
             * conditionals than their list parent
             */
            if (is_targetmod) {
                for (uniq = obj_first_unique(testobj);
                     uniq != NULL;
                     uniq = obj_next_unique(uniq)) {

                    for (uncomp = obj_first_unique_comp(uniq);
                         uncomp != NULL;
                         uncomp = obj_next_unique_comp(uncomp)) {

                        if (uncomp->unobj) {
                            res = check_conditional_mismatch(tkc, 
                                                             mod,
                                                             testobj,
                                                             uncomp->unobj);
                            CHK_EXIT(res, retres);
                        }
                    }
                }
            }

            /* check list children */
            res = resolve_xpath(tkc, 
                                mod, 
                                testobj->def.list->datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_CHOICE:
            res = resolve_xpath(tkc, 
                                mod, 
                                testobj->def.choic->caseQ);
            break;
        case OBJ_TYP_CASE:
            res = resolve_xpath(tkc, 
                                mod, 
                                testobj->def.cas->datadefQ);
            break;
        case OBJ_TYP_RPC:
            res = resolve_xpath(tkc, 
                                mod, 
                                &testobj->def.rpc->datadefQ);
            break;
        case OBJ_TYP_RPCIO:
            res = resolve_xpath(tkc, 
                                mod, 
                                &testobj->def.rpcio->datadefQ);
            break;
        case OBJ_TYP_NOTIF:
            res = resolve_xpath(tkc, 
                                mod, 
                                &testobj->def.notif->datadefQ);
            break;
        case OBJ_TYP_NONE:
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        CHK_EXIT(res, retres);
    }

    return retres;

}  /* resolve_xpath */


/************   E X T E R N A L   F U N C T I O N S   ***************/


/********************************************************************
* FUNCTION yang_obj_consume_datadef
* 
* Parse the next N tokens as a data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_datadef (yang_pcb_t *pcb,
                              tk_chain_t *tkc,
                              ncx_module_t  *mod,
                              dlq_hdr_t *que,
                              obj_template_t *parent)
{
    status_t         res;

#ifdef DEBUG
    if (!pcb || !tkc || !mod || !que) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_datadef(pcb,
                          tkc, 
                          mod, 
                          que, 
                          parent, 
                          NULL);
    return res;

}  /* yang_obj_consume_datadef */


/********************************************************************
* FUNCTION yang_obj_consume_datadef_grp
* 
* Parse the next N tokens as a data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == grp_template_t containing 'que'
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_datadef_grp (yang_pcb_t *pcb,
                                  tk_chain_t *tkc,
                                  ncx_module_t  *mod,
                                  dlq_hdr_t *que,
                                  obj_template_t *parent,
                                  grp_template_t *grp)
{
    status_t         res;

#ifdef DEBUG
    if (!pcb || !tkc || !mod || !que || !grp) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_datadef(pcb, tkc, mod, que, parent, grp);
    return res;

}  /* yang_obj_consume_datadef_grp */


/********************************************************************
* FUNCTION yang_obj_consume_rpc
* 
* Parse the next N tokens as a rpc-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'rpc' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new RPC added to module
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_rpc (yang_pcb_t *pcb,
                          tk_chain_t *tkc,
                          ncx_module_t  *mod)
{
    status_t         res;

#ifdef DEBUG
    if (!pcb || !tkc || !mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_rpc(pcb,
                      tkc, 
                      mod, 
                      &mod->datadefQ, 
                      NULL, 
                      NULL);
    return res;

}  /* yang_obj_consume_rpc */


/********************************************************************
* FUNCTION yang_obj_consume_notification
* 
* Parse the next N tokens as a notification-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'notification' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new notification added to module
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_notification (yang_pcb_t *pcb,
                                   tk_chain_t *tkc,
                                   ncx_module_t  *mod)
{
    status_t         res;

#ifdef DEBUG
    if (!pcb || !tkc || !mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_notif(pcb,
                        tkc, 
                        mod, 
                        &mod->datadefQ, 
                        NULL, 
                        NULL);
    return res;

}  /* yang_obj_consume_notification */


/********************************************************************
* FUNCTION yang_obj_consume_augment
* 
* Parse the next N tokens as a top-level augment-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'augment' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new augment object added to module 
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_augment (yang_pcb_t *pcb,
                              tk_chain_t *tkc,
                              ncx_module_t  *mod)
{
    status_t         res;

#ifdef DEBUG
    if (!pcb || !tkc || !mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = consume_augment(pcb,
                          tkc, 
                          mod, 
                          &mod->datadefQ, 
                          NULL, 
                          NULL);
    return res;

}  /* yang_obj_consume_augment */


/********************************************************************
* FUNCTION yang_obj_consume_deviation
* 
* Parse the next N tokens as a top-level deviation-stmt
* Create a obj_deviation_t struct and add it to the 
* specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'deviation' keyword
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new deviation struct added to module deviationQ
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_consume_deviation (yang_pcb_t *pcb,
                                tk_chain_t *tkc,
                                ncx_module_t  *mod)
{
    obj_deviation_t  *dev;
    const xmlChar    *val;
    const char       *expstr;
    xmlChar          *str;
    yang_stmt_t      *stmt;
    tk_type_t         tktyp;
    boolean           done, desc, ref;
    status_t          res, retres;

#ifdef DEBUG
    if (!pcb || !tkc || !mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* Get a new obj_deviation_t to fill in */
    dev = obj_new_deviation();
    if (!dev) {
        res = ERR_INTERNAL_MEM;
        ncx_print_errormsg(tkc, mod, res);
        return res;
    }

    val = NULL;
    expstr = "absolute schema node";
    str = NULL;
    done = FALSE;
    desc = FALSE;
    ref = FALSE;
    retres = NO_ERR;

    ncx_set_error(&dev->tkerr,
                  mod,
                  TK_CUR_LNUM(tkc),
                  TK_CUR_LPOS(tkc));

    /* Get the mandatory deviation target */
    res = yang_consume_string(tkc, mod, &dev->target);
    CHK_DEV_EXIT(dev, res, retres);

    /* Get the starting left brace or semi-colon */
    res = TK_ADV(tkc);
    if (res != NO_ERR) {
        ncx_print_errormsg(tkc, mod, res);
        obj_free_deviation(dev);
        return res;
    }

    /* check for semi-colon or left brace */
    switch (TK_CUR_TYP(tkc)) {
    case TK_TT_SEMICOL:
        dev->empty = TRUE;
        done = TRUE;
        break;
    case TK_TT_LBRACE:
        done = FALSE;
        break;
    default:
        res = ERR_NCX_WRONG_TKTYPE;
        expstr = "semi-colon or left brace";
        ncx_mod_exp_err(tkc, mod, res, expstr);
        done = TRUE;
    }

    /* get the sub-section statements and any appinfo extensions */
    while (!done) {
        /* get the next token */
        res = TK_ADV(tkc);
        if (res != NO_ERR) {
            ncx_print_errormsg(tkc, mod, res);
            obj_free_deviation(dev);
            return res;
        }

        tktyp = TK_CUR_TYP(tkc);
        val = TK_CUR_VAL(tkc);

        /* check the current token type */
        switch (tktyp) {
        case TK_TT_NONE:
            res = ERR_NCX_EOF;
            ncx_print_errormsg(tkc, mod, res);
            obj_free_deviation(dev);
            return res;
        case TK_TT_MSTRING:
            /* vendor-specific clause found instead */
            res = ncx_consume_appinfo(tkc, mod, &dev->appinfoQ);
            CHK_DEV_EXIT(dev, res, retres);
            continue;
        case TK_TT_RBRACE:
            done = TRUE;
            continue;
        case TK_TT_TSTRING:
            break;  /* YANG clause assumed */
        default:
            retres = ERR_NCX_WRONG_TKTYPE;
            ncx_mod_exp_err(tkc, mod, retres, expstr);
            continue;
        }

        /* Got a keyword token string so check the value */
        if (!xml_strcmp(val, YANG_K_DESCRIPTION)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &dev->descr,
                                     &desc, 
                                     &dev->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_REFERENCE)) {
            res = yang_consume_descr(tkc, 
                                     mod, 
                                     &dev->ref,
                                     &ref, 
                                     &dev->appinfoQ);
        } else if (!xml_strcmp(val, YANG_K_DEVIATE)) {
            res = consume_deviate(pcb,
                                  tkc, 
                                  mod, 
                                  dev);
        } else {
            res = ERR_NCX_WRONG_TKVAL;
            expstr = "description, reference, or deviate";
            ncx_mod_exp_err(tkc, mod, res, expstr);
        }
        CHK_DEV_EXIT(dev, res, retres);
    }

    /* save or delete the obj_deviation_t struct */
    if (dev->target) {
        dev->res = retres;
        dlq_enque(dev, &mod->deviationQ);
        if (mod->stmtmode) {
            /* save top-level deviation order */
            stmt = yang_new_deviation_stmt(dev);
            if (stmt) {
                dlq_enque(stmt, &mod->stmtQ);
            } else {
                log_error("\nError: malloc failure for obj_stmt");
                res = ERR_INTERNAL_MEM;
                ncx_print_errormsg(tkc, mod, res);
            }
        }
    } else {
        obj_free_deviation(dev);
    }

    return retres;

}  /* yang_obj_consume_deviation */


/********************************************************************
* FUNCTION yang_obj_resolve_datadefs
* 
* First pass object validation
*
* Analyze the entire datadefQ within the module struct
* Finish all the clauses within this struct that
* may have been defered because of possible forward references
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_datadefs (yang_pcb_t *pcb,
                               tk_chain_t *tkc,
                               ncx_module_t  *mod,
                               dlq_hdr_t *datadefQ)
{
    status_t    retres;

#ifdef DEBUG
    if (!pcb || !tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    retres = resolve_datadefs(pcb, tkc, mod, datadefQ, FALSE);
    return retres;

}  /* yang_obj_resolve_datadefs */


/********************************************************************
* FUNCTION yang_obj_resolve_uses
* 
* Second pass object validation
* This calls expand_uses not resolve_uses!
*
* Refine-stmts have already been patched into objects in phase 1.
* Expand and validate any uses clauses within any objects
* within the datadefQ.
*
* Validate and expand any augments within a uses-stmt
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_uses (yang_pcb_t *pcb,
                           tk_chain_t *tkc,
                           ncx_module_t  *mod,
                           dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj, *casobj;
    obj_case_t      *cas;
    obj_augment_t   *aug;
    status_t         res, retres;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* first resolve all the local type names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

#ifdef YANG_OBJ_DEBUG
        if (LOGDEBUG4) {
            log_debug4("\nresolve_uses: mod %s, object %s, on line %u",
                       mod->name, 
                       obj_get_name(testobj),
                       testobj->tkerr.linenum);
        }
#endif

        /* all node types are just traversing the object tree
         * except OBJ_TYP_USES, which is expanded with expand_uses
         */
        switch (testobj->objtype) {
        case OBJ_TYP_CONTAINER:
            res = 
                yang_grp_resolve_complete(pcb,
                                          tkc, 
                                          mod,
                                          testobj->def.container->groupingQ,
                                          testobj);
            CHK_EXIT(res, retres);

            res = yang_obj_resolve_uses(pcb,
                                        tkc, 
                                        mod,
                                        testobj->def.container->datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_ANYXML:
        case OBJ_TYP_LEAF:
        case OBJ_TYP_LEAF_LIST:
            break;
        case OBJ_TYP_LIST:
            res = yang_grp_resolve_complete(pcb,
                                            tkc, 
                                            mod,
                                            testobj->def.list->groupingQ,
                                            testobj);
            CHK_EXIT(res, retres);

            res = yang_obj_resolve_uses(pcb,
                                        tkc, 
                                        mod,
                                        testobj->def.list->datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_CHOICE:
            for (casobj = (obj_template_t *)
                     dlq_firstEntry(testobj->def.choic->caseQ);
                 casobj != NULL;
                 casobj = (obj_template_t *)dlq_nextEntry(casobj)) {
                cas = casobj->def.cas;
                res = yang_obj_resolve_uses(pcb,
                                            tkc, 
                                            mod, 
                                            cas->datadefQ);
                CHK_EXIT(res, retres);
            }
            break;
        case OBJ_TYP_CASE:
            cas = testobj->def.cas;
            res = yang_obj_resolve_uses(pcb,
                                        tkc, 
                                        mod, 
                                        cas->datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_USES:
            res = expand_uses(pcb,
                              tkc, 
                              mod, 
                              testobj, 
                              datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_AUGMENT:
            aug = testobj->def.augment;
            res = yang_obj_resolve_uses(pcb,
                                        tkc, 
                                        mod, 
                                        &aug->datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_RPC:
            res = yang_grp_resolve_complete(pcb,
                                            tkc, 
                                            mod,
                                            &testobj->def.rpc->groupingQ,
                                            testobj);
            CHK_EXIT(res, retres);

            res = yang_obj_resolve_uses(pcb,
                                        tkc, 
                                        mod,
                                        &testobj->def.rpc->datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_RPCIO:
            res = yang_grp_resolve_complete(pcb,
                                            tkc, 
                                            mod,
                                            &testobj->def.rpcio->groupingQ,
                                            testobj);
            CHK_EXIT(res, retres);

            res = yang_obj_resolve_uses(pcb,
                                        tkc, 
                                        mod,
                                        &testobj->def.rpcio->datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_NOTIF:
            res = yang_grp_resolve_complete(pcb,
                                            tkc,
                                            mod,
                                            &testobj->def.notif->groupingQ,
                                            testobj);
            CHK_EXIT(res, retres);

            res = yang_obj_resolve_uses(pcb,
                                        tkc,
                                        mod,
                                        &testobj->def.notif->datadefQ);
            CHK_EXIT(res, retres);
            break;
        case OBJ_TYP_NONE:
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        CHK_EXIT(res, retres);
    }

    return retres;

}  /* yang_obj_resolve_uses */


/********************************************************************
* FUNCTION yang_obj_resolve_augments
* 
*
* Third pass object validation
*
* Expand and validate any augment clauses within any objects
* within the datadefQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_augments (yang_pcb_t *pcb,
                               tk_chain_t *tkc,
                               ncx_module_t *mod,
                               dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj;
    status_t         res, retres;

#ifdef DEBUG
    if (!pcb || !tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* go through all the object trees and check for augments */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

        if (testobj->objtype == OBJ_TYP_AUGMENT) {
            res = expand_augment(pcb,
                                 tkc, 
                                 mod, 
                                 testobj, 
                                 datadefQ);
            CHK_EXIT(res, retres);
        }
    }

    return retres;

}  /* yang_obj_resolve_augments */


/********************************************************************
* FUNCTION yang_obj_resolve_augments_final
* 
* Fourth pass object expand augments
*
* Clone any list keys missed in yang_obj_resolve_augments
* This only occurs for augments of external objects, so
* only top-level augment-stmts need to be checked.
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_augments_final (yang_pcb_t *pcb,
                                     tk_chain_t *tkc,
                                     ncx_module_t *mod,
                                     dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj;
    status_t         res, retres;

#ifdef DEBUG
    if (!pcb || !tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* only need to check the top-level obkects because only
     * top-level augments can be for other modules
     */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

        if (testobj->objtype == OBJ_TYP_AUGMENT) {
            res = resolve_augment_final(pcb, tkc, mod, testobj);
            CHK_EXIT(res, retres);
        }
    }

    return retres;

}  /* yang_obj_resolve_augments_final */


/********************************************************************
* FUNCTION yang_obj_resolve_deviations
* 
*
* Validate any deviation statements within the 
* module deviationQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_deviations (yang_pcb_t *pcb,
                                 tk_chain_t *tkc,
                                 ncx_module_t  *mod)
{
    obj_deviation_t        *deviation;
    ncx_save_deviations_t  *savedev, *nextdev;
    status_t                res, retres;
    boolean                 anydevs;
    uint32                  extdevcount;

#ifdef DEBUG
    if (!tkc || !mod) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (pcb->deviationmode) {
        /* save any deviations until later 
         * grab all the imports also to resolve
         * the deviation statements later
         */
        if (!dlq_empty(&mod->deviationQ)) {
            savedev = ncx_new_save_deviations(mod->name,
                                              mod->version,
                                              mod->ns,
                                              mod->prefix);
            if (savedev == NULL) {
                return ERR_INTERNAL_MEM;
            }
            if (LOGDEBUG) {
                log_debug("\nSaving %u deviations from deviation module '%s'",
                          dlq_count(&mod->deviationQ),
                          mod->name);
            }
            dlq_block_enque(&mod->importQ, &savedev->importQ);
            dlq_block_enque(&mod->deviationQ, &savedev->deviationQ);
            dlq_enque(savedev, pcb->savedevQ);
        } else if (LOGDEBUG) {
            log_debug("\nNo deviations found in deviation module '%s'",
                      mod->name);
        }
        return NO_ERR;
    }

    res = NO_ERR;
    retres = NO_ERR;
    anydevs = FALSE;
    extdevcount = 0;

    /* first resolve all the local deviations */
    for (deviation = (obj_deviation_t *)
             dlq_firstEntry(&mod->deviationQ);
         deviation != NULL;
         deviation = (obj_deviation_t *)
             dlq_nextEntry(deviation)) {

        if (deviation->res != NO_ERR) {
            continue;
        }

        anydevs = TRUE;

        res = resolve_deviation(pcb,
                                tkc, 
                                mod, 
                                deviation);

        deviation->res = res;
        CHK_EXIT(res, retres);
    }

    /* next gather all the external deviations that apply
     * to this module; they will be moved from the global
     * pcb->savedevQ to the mod->deviationQ
     * Only do this for main modules, not submodules
     */
    if (pcb->savedevQ) {
        for (savedev = (ncx_save_deviations_t *)
                 dlq_firstEntry(pcb->savedevQ);
             savedev != NULL && mod->ismod;
             savedev = nextdev) {

            nextdev = (ncx_save_deviations_t *)dlq_nextEntry(savedev);

            /* check if the deviation module is this module; skip */
            if (!xml_strcmp(savedev->devmodule, mod->name)) {
                continue;
            }

            /* check if there are any deviations for this module
             * in the savedev->deviationQ; if so, resolve them
             * and put them in the mod->deviationQ
             */
            res = transfer_my_deviations(pcb,
                                         tkc,
                                         savedev, 
                                         mod, 
                                         &extdevcount);
            CHK_EXIT(res, retres);
        }
    }

    /* normalize the deviationQ so there are no duplicate
     * target objects; combine any deviations for the
     * same targobj; check errors deferred from resolve_module
     */
    if (retres == NO_ERR && (anydevs || extdevcount)) {
        retres = normalize_deviationQ(tkc, mod);
    }

    /* pick out any deviations for this module and 
     * patch them into the object tree
     */
    if (retres == NO_ERR && (anydevs || extdevcount)) {
        retres = apply_all_object_deviations(pcb, tkc, mod);
    }

    return retres;

}  /* yang_obj_resolve_deviations */


/********************************************************************
* FUNCTION yang_obj_resolve_final
* 
* Fourth pass object validation
*
* Check various final stage errors and warnings
* within a single file
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*   ingrouping == TRUE if this object being resolved
*                 from yang_grp_resolve_final
*              == FALSE otherwise
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_final (yang_pcb_t *pcb,
                            tk_chain_t *tkc,
                            ncx_module_t  *mod,
                            dlq_hdr_t *datadefQ,
                            boolean ingrouping)
{
    obj_template_t  *testobj;
    status_t         res, retres;
    boolean          notclone;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* first resolve all the local object names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

        /* go through all the cloned nodes to find any lists
         * so their keyQ and uniqueQ structures can be
         * set with pointers to the cloned objects
         * instead of pointers to the original objects
         */
        notclone = !obj_is_cloned(testobj);

#ifdef YANG_OBJ_DEBUG
        if (LOGDEBUG4) {
            log_debug4("\nresolve_final: mod %s, object %s, on line %u",
                       mod->name, 
                       obj_get_name(testobj), 
                       testobj->tkerr.linenum);
        }
#endif
        
        switch (testobj->objtype) {
        case OBJ_TYP_CONTAINER:
            res = resolve_container_final(tkc, 
                                          mod, 
                                          testobj,
                                          ingrouping);
            CHK_EXIT(res, retres);

            if (notclone) {
                res = 
                    yang_grp_resolve_final(pcb,
                                           tkc, 
                                           mod,
                                           testobj->def.container->groupingQ);
                CHK_EXIT(res, retres);
            }

            res = yang_obj_resolve_final(pcb,
                                         tkc, 
                                         mod, 
                                         testobj->def.container->datadefQ,
                                         ingrouping);
            CHK_EXIT(res, retres);

            if (notclone) {
                res = resolve_default_parm(tkc, mod, testobj);

                yang_check_obj_used(tkc, 
                                    mod,
                                    testobj->def.container->typedefQ,
                                    testobj->def.container->groupingQ);
            }
            break;
        case OBJ_TYP_LEAF:
            res = resolve_leaf_final(tkc, mod, testobj, ingrouping);
            break;
        case OBJ_TYP_ANYXML:
        case OBJ_TYP_LEAF_LIST:
            break;
        case OBJ_TYP_LIST:
            if (notclone) {
                res = yang_grp_resolve_final(pcb,
                                             tkc, 
                                             mod, 
                                             testobj->def.list->groupingQ);
                CHK_EXIT(res, retres);
            }

            res = yang_obj_resolve_final(pcb,
                                         tkc, 
                                         mod, 
                                         testobj->def.list->datadefQ,
                                         ingrouping);
            CHK_EXIT(res, retres);

            if (notclone) {
                yang_check_obj_used(tkc, 
                                    mod,
                                    testobj->def.list->typedefQ,
                                    testobj->def.list->groupingQ);
            }

            res = resolve_list_final(pcb,
                                     tkc, 
                                     mod, 
                                     testobj->def.list, 
                                     testobj);
            break;
        case OBJ_TYP_CHOICE:
            res = resolve_choice_final(tkc, mod, testobj, ingrouping);
            CHK_EXIT(res, retres);

            res = yang_obj_resolve_final(pcb,
                                         tkc, 
                                         mod, 
                                         testobj->def.choic->caseQ,
                                         ingrouping);
            break;
        case OBJ_TYP_CASE:
            res = yang_obj_resolve_final(pcb,
                                         tkc, 
                                         mod, 
                                         testobj->def.cas->datadefQ,
                                         ingrouping);
            break;
        case OBJ_TYP_USES:
            if (notclone) {
                res = 
                    yang_obj_resolve_final(pcb,
                                           tkc, 
                                           mod, 
                                           testobj->def.uses->datadefQ,
                                           ingrouping);
            }
            break;
        case OBJ_TYP_AUGMENT:
            if (notclone) {
                res = 
                    yang_obj_resolve_final(pcb,
                                           tkc, 
                                           mod, 
                                           &testobj->def.augment->datadefQ,
                                           ingrouping);
            }
            break;
        case OBJ_TYP_RPC:
            if (notclone) {
                res = 
                    yang_grp_resolve_final(pcb,
                                           tkc, 
                                           mod, 
                                           &testobj->def.rpc->groupingQ);
                CHK_EXIT(res, retres);
            }

            res = yang_obj_resolve_final(pcb,
                                         tkc, 
                                         mod, 
                                         &testobj->def.rpc->datadefQ,
                                         ingrouping);

            if (notclone) {
                yang_check_obj_used(tkc, 
                                    mod,
                                    &testobj->def.rpc->typedefQ,
                                    &testobj->def.rpc->groupingQ);
            }
            break;
        case OBJ_TYP_RPCIO:
            if (notclone) {
                res = 
                    yang_grp_resolve_final(pcb,
                                           tkc, 
                                           mod, 
                                           &testobj->def.rpcio->groupingQ);
                CHK_EXIT(res, retres);
            }

            res = yang_obj_resolve_final(pcb,
                                         tkc, 
                                         mod, 
                                         &testobj->def.rpcio->datadefQ,
                                         ingrouping);
            CHK_EXIT(res, retres);

            if (notclone) {
                res = resolve_default_parm(tkc, mod, testobj);
                yang_check_obj_used(tkc, 
                                    mod,
                                    &testobj->def.rpcio->typedefQ,
                                    &testobj->def.rpcio->groupingQ);
            }
            break;
        case OBJ_TYP_NOTIF:
            if (notclone) {
                res = 
                    yang_grp_resolve_final(pcb,
                                           tkc, 
                                           mod, 
                                           &testobj->def.notif->groupingQ);
                CHK_EXIT(res, retres);
            }

            res = 
                yang_obj_resolve_final(pcb,
                                       tkc, 
                                       mod, 
                                       &testobj->def.notif->datadefQ,
                                       ingrouping);

            if (notclone) {
                yang_check_obj_used(tkc,
                                    mod,
                                    &testobj->def.notif->typedefQ,
                                    &testobj->def.notif->groupingQ);
            }
            break;
        case OBJ_TYP_REFINE:
            break;
        case OBJ_TYP_NONE:
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        CHK_EXIT(res, retres);
    }

    return retres;

}  /* yang_obj_resolve_final */


/********************************************************************
* FUNCTION yang_obj_top_resolve_final
* 
* Fourth pass object validation; for top-level module with submodules
*
* Check various final stage errors and warnings
* within a single file
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_top_resolve_final (yang_pcb_t *pcb,
                                tk_chain_t *tkc,
                                ncx_module_t  *mod,
                                dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj;
    dlq_hdr_t       *childdefQ;
    status_t         res, retres;


#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* first resolve all the local object names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

#ifdef YANG_OBJ_DEBUG
        if (LOGDEBUG4) {
            log_debug4("\nresolve_top_final: mod %s, object %s, on line %u",
                       mod->name, 
                       obj_get_name(testobj), 
                       testobj->tkerr.linenum);
        }
#endif

        if (testobj->objtype == OBJ_TYP_LIST) {
            res = resolve_list_final(pcb,
                                     tkc, 
                                     mod, 
                                     testobj->def.list, 
                                     testobj);
            CHK_EXIT(res, retres);
        }
        childdefQ = obj_get_datadefQ(testobj);
        if (childdefQ != NULL) {
            res = yang_obj_top_resolve_final(pcb, tkc, mod, childdefQ);
            CHK_EXIT(res, retres);
        }
    }

    return retres;

}  /* yang_obj_top_resolve_final */


/********************************************************************
* FUNCTION yang_obj_resolve_xpath
* 
* Fifth (and final) pass object validation
*
* Check all leafref, must, and when XPath expressions
* to make sure they are well-formed
*
* Checks the cooked objects, and skips all groupings
* uses, and augment nodes
*
* MUST BE CALLED AFTER yang_obj_resolve_final
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_resolve_xpath (tk_chain_t *tkc,
                            ncx_module_t  *mod,
                            dlq_hdr_t *datadefQ)
{
    //ncx_include_t      *inc;
    yang_node_t        *node, *node2;
    status_t            res, retres;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    if (LOGDEBUG3) {
        log_debug3("\nyang_obj_resolve_xpath for %smodule '%s'",
                   (mod->ismod) ? EMPTY_STRING : (const xmlChar *)"sub",
                   mod->name);
    }

    /* first resolve the main module or top submodule */
    res = resolve_xpath(tkc, mod, datadefQ);
    CHK_EXIT(res, retres);


    for (node = (yang_node_t *)dlq_firstEntry(&mod->allincQ);
         node != NULL;
         node = (yang_node_t *)dlq_nextEntry(node)) {

        if (node->submod == NULL) {
            /* error in processing this submod */
            continue;
        }

        if (LOGDEBUG3) {
            log_debug3("\nyang_obj_resolve_xpath "
                       "for submodule '%s' against main mod '%s'",
                       node->submod->name,
                       mod->name);
        }

        /* check node submod against main module */
        res = resolve_xpath(tkc, 
                            node->submod,
                            datadefQ);
        CHK_EXIT(res, retres);

        for (node2 = (yang_node_t *)dlq_firstEntry(&mod->allincQ);
             node2 != NULL;
             node2 = (yang_node_t *)dlq_nextEntry(node2)) {

            if (node2->submod == NULL) {
                continue;
            }
            if (LOGDEBUG3) {
                log_debug3("\nyang_obj_resolve_xpath "
                           "for submodule '%s' against sub mod '%s'",
                           node->submod->name,
                           node2->submod->name);
            }

            res = resolve_xpath(tkc, 
                                node->submod,
                                &node2->submod->datadefQ);
            CHK_EXIT(res, retres);
        }
    }

    return retres;

}  /* yang_obj_resolve_xpath */


/********************************************************************
* FUNCTION yang_obj_check_leafref_loops
* 
* Check all leafref objects for hard-wired object loops
* Must be done after yang_obj_resolve_xpath
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
    yang_obj_check_leafref_loops (tk_chain_t *tkc,
                                  ncx_module_t *mod,
                                  dlq_hdr_t *datadefQ)
{
    obj_template_t  *testobj, *nextobj, *lastobj;
    dlq_hdr_t       *childdatadefQ;
    status_t         res, retres;
    boolean          isleaf;

#ifdef DEBUG
    if (!tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;

    /* first resolve all the local object names */
    for (testobj = (obj_template_t *)dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = (obj_template_t *)dlq_nextEntry(testobj)) {

        isleaf = FALSE;
        switch (testobj->objtype) {
        case OBJ_TYP_LEAF:
            isleaf = TRUE;
            /* fall through */
        case OBJ_TYP_LEAF_LIST:
            if (obj_get_basetype(testobj) == NCX_BT_LEAFREF) {
#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG4) {
                    log_debug4("\ncheck_leafref_loop: mod %s, "
                               "object %s, on line %u",
                               mod->name, 
                               obj_get_name(testobj), 
                               testobj->tkerr.linenum);
                }
#endif
                
                if (isleaf) {
                    nextobj = testobj->def.leaf->leafrefobj;
                } else {
                    nextobj = testobj->def.leaflist->leafrefobj;
                }

                if (nextobj == testobj) {
                    res = ERR_NCX_LEAFREF_LOOP;
                    log_error("\nError: leafref path in "
                              "%s %s loops with self",
                              obj_get_typestr(testobj),
                              obj_get_name(testobj));
                    tkc->curerr = &testobj->tkerr;
                    ncx_print_errormsg(tkc, mod, res);
                } else {
                    while (nextobj) {
                        if (obj_is_leafy(nextobj) &&
                            obj_get_basetype(nextobj) == NCX_BT_LEAFREF) {

                            lastobj = nextobj;
                            if (isleaf) {
                                nextobj = nextobj->def.leaf->leafrefobj;
                            } else {
                                nextobj = nextobj->def.leaflist->leafrefobj;
                            }

                            if (nextobj == testobj) {
                                res = ERR_NCX_LEAFREF_LOOP;
                                log_error("\nError: leafref path in "
                                          "%s %s loops with %s %s",
                                          obj_get_typestr(testobj),
                                          obj_get_name(testobj),
                                          obj_get_typestr(lastobj),
                                          obj_get_name(lastobj));
                                tkc->curerr = &testobj->tkerr;
                                ncx_print_errormsg(tkc, mod, res);
                                nextobj = NULL;
                            }
                        } else {
                            nextobj = NULL;
                        }
                    }
                }
                CHK_EXIT(res, retres);
            }
            break;
        default:
            childdatadefQ = obj_get_datadefQ(testobj);
            if (childdatadefQ) {
                res = yang_obj_check_leafref_loops(tkc,
                                                   mod,
                                                   childdatadefQ);
                CHK_EXIT(res, retres);
            }
        }
    }

    return retres;

}  /* yang_obj_check_leafref_loops */


/********************************************************************
* FUNCTION yang_obj_remove_deleted_nodes
* 
* Find any nodes marked for deletion and remove them
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token parse chain to use for errors
*   mod == module in progress; transfer to this deviationQ
*   datadefQ == current object datadef Q to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t
    yang_obj_remove_deleted_nodes (yang_pcb_t *pcb,
                                   tk_chain_t *tkc,
                                   ncx_module_t *mod,
                                   dlq_hdr_t *datadefQ)
{
    obj_template_t   *testobj, *nextobj, *parentobj;
    dlq_hdr_t        *child_datadefQ;
    status_t          res, retres;

#ifdef DEBUG
    if (!pcb || !tkc || !mod || !datadefQ) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    retres = NO_ERR;

    for (testobj = (obj_template_t *)
             dlq_firstEntry(datadefQ);
         testobj != NULL;
         testobj = nextobj) {

        nextobj = (obj_template_t *)dlq_nextEntry(testobj);
        parentobj = NULL;

        if (testobj->flags & OBJ_FL_DELETED) {
            dlq_remove(testobj);

#ifdef YANG_OBJ_DEBUG
            if (LOGDEBUG2) {
                log_debug2("\nDeviation caused deletion of object %s:%s",
                           obj_get_mod_name(testobj),
                           obj_get_name(testobj));
            }
#endif

            parentobj = testobj->parent;
            obj_free_template(testobj);

            if (parentobj) {
                /* need to retest the parent to see if it
                 * is still OK; there should not be any other
                 * deviations with the same target object
                 */
#ifdef YANG_OBJ_DEBUG
                if (LOGDEBUG2) {
                    log_debug2("\nRechecking %s:%s after "
                               "applying deviation(s) to child",
                               obj_get_mod_name(parentobj),
                               obj_get_name(parentobj));
                }
#endif
                res = resolve_datadef(pcb,
                                      tkc, 
                                      mod, 
                                      parentobj,
                                      TRUE);
                CHK_EXIT(res, retres);
            }
        } else {
            /* this object was not deleted */
            child_datadefQ = obj_get_datadefQ(testobj);
            if (child_datadefQ != NULL) {
                res = yang_obj_remove_deleted_nodes(pcb,
                                                    tkc,
                                                    mod,
                                                    child_datadefQ);
                if (res != NO_ERR) {
                    retres = res;
                }
            }
        }
    }

    return retres;
        
}  /* yang_obj_remove_deleted_nodes */


/* END file yang_obj.c */
