/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: c.c

    Generate C file output from YANG module

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
27oct09      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>
#include <ctype.h>

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_c
#include "c.h"
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

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
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


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/



/********************************************************************
* FUNCTION write_if_res
* 
* Write if (res != NO_ERR) ... statements
*
* INPUTS:
*   scb == session control block to use for writing
*   cp == conversion parameters to use
*   startindent == start indent amount
*
*********************************************************************/
static void
    write_if_res (ses_cb_t *scb,
                  const yangdump_cvtparms_t *cp,
                  int32 startindent)
{
    ses_putstr_indent(scb, 
                      (const xmlChar *)"if (res != NO_ERR) {",
                      startindent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"return res;",
                      startindent + cp->indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"}\n",
                      startindent);

} /* write_if_res */


/********************************************************************
* FUNCTION write_if_record_error
* 
* Write if (...) { agt_record_error } stmts
*
* INPUTS:
*   scb == session control block to use for writing
*   cp == conversion parameters to use
*   startindent == start indent amount
*   oper_layer == TRUE if operation layer
*                 FALSE if application layer
*   with_return == TRUE if return res; added
*                  FALSE if not added
*   with_methnode == TRUE if methnode should be in error call
*                    FALSE to use NULL instead of methnode
*
*   
*********************************************************************/
static void
    write_if_record_error (ses_cb_t *scb,
                           const yangdump_cvtparms_t *cp,
                           int32 startindent,
                           boolean oper_layer,
                           boolean with_return,
                           boolean with_methnode)
{
    int32 indent;

    indent = cp->indent;

    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"/* if error: set the res, errorstr, "
                      "and errorval parms */",
                      startindent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"if (res != NO_ERR) {",
                      startindent);

    ses_putstr_indent(scb, 
                      (const xmlChar *)"agt_record_error(",
                      startindent + indent);

    indent += cp->indent;

    ses_putstr_indent(scb, 
                      (const xmlChar *)"scb,",
                      startindent + indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"&msg->mhdr,",
                      startindent + indent);
    if (oper_layer) {
        ses_putstr_indent(scb, 
                          (const xmlChar *)"NCX_LAYER_OPERATION,",
                          startindent + indent);
    } else {
        ses_putstr_indent(scb, 
                          (const xmlChar *)"NCX_LAYER_CONTENT,",
                          startindent + indent);
    }
    ses_putstr_indent(scb, 
                      (const xmlChar *)"res,",
                      startindent + indent);
    ses_putstr_indent(scb, 
                      (with_methnode) ?
                      (const xmlChar *)"methnode," :
                      (const xmlChar *)"NULL,",
                      startindent + indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"NCX_NT_STRING,",
                      startindent + indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"errorstr,",
                      startindent + indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"NCX_NT_VAL,",
                      startindent + indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"errorval);",
                      startindent + indent);

    if (with_return) {
        indent -= cp->indent;

        ses_putstr_indent(scb, 
                          (const xmlChar *)"return res;",
                          startindent + indent);
    }

    ses_putstr_indent(scb, 
                      (const xmlChar *)"}\n",
                      startindent);

} /* write_if_record_error */


/********************************************************************
* FUNCTION write_c_includes
* 
* Write the C file #include statements
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*
*********************************************************************/
static void
    write_c_includes (ses_cb_t *scb,
                      const ncx_module_t *mod,
                      const yangdump_cvtparms_t *cp)
{
#ifdef LEAVE_OUT_FOR_NOW
    const ncx_include_t      *inc;
#endif
    boolean                   needrpc, neednotif;

    needrpc = need_rpc_includes(mod, cp);
    neednotif = need_notif_includes(mod, cp);

    /* add xmlChar include */
    write_ext_include(scb, (const xmlChar *)"xmlstring.h");

    /* add procdefs include first NCX include */
    write_ncx_include(scb, (const xmlChar *)"procdefs");

    /* add includes even if they may not get used
     * TBD: prune all unused include files
     */
    write_ncx_include(scb, (const xmlChar *)"agt");

    if (needrpc) {
        write_ncx_include(scb, (const xmlChar *)"agt_cb");
    }

    if (neednotif) {
        write_ncx_include(scb, (const xmlChar *)"agt_not");
    }
        
    if (needrpc) {
        write_ncx_include(scb, (const xmlChar *)"agt_rpc");
    }

    write_ncx_include(scb, (const xmlChar *)"agt_timer");
    write_ncx_include(scb, (const xmlChar *)"agt_util");
    write_ncx_include(scb, (const xmlChar *)"dlq");
    write_ncx_include(scb, (const xmlChar *)"ncx");
    write_ncx_include(scb, (const xmlChar *)"ncxmod");
    write_ncx_include(scb, (const xmlChar *)"ncxtypes");

    if (needrpc) {
        write_ncx_include(scb, (const xmlChar *)"rpc");
    }

    if (needrpc || neednotif) {
        write_ncx_include(scb, (const xmlChar *)"ses");
    }

    write_ncx_include(scb, (const xmlChar *)"status");

    if (needrpc || neednotif) {
        write_ncx_include(scb, (const xmlChar *)"val");
        write_ncx_include(scb, (const xmlChar *)"val_util");
        write_ncx_include(scb, (const xmlChar *)"xml_util");
    }

    /* include for the module H file */
    write_ncx_include(scb, ncx_get_modname(mod));

#ifdef LEAVE_OUT_FOR_NOW
    /* includes for submodules */
    if (!cp->unified) {
        for (inc = (const ncx_include_t *)
                 dlq_firstEntry(&mod->includeQ);
             inc != NULL;
             inc = (const ncx_include_t *)dlq_nextEntry(inc)) {

            write_ncx_include(scb, inc->submodule);
        }
    }
#endif

} /* write_c_includes */


/********************************************************************
* FUNCTION write_c_object_var
* 
* Write the C file object variable name
*
* INPUTS:
*   scb == session control block to use for writing
*   objname == object name to use
*
*********************************************************************/
static void
    write_c_object_var (ses_cb_t *scb,
                        const xmlChar *objname)
{

    write_c_safe_str(scb, objname);
    ses_putstr(scb, (const xmlChar *)"_obj");

} /* write_c_object_var */


/********************************************************************
* FUNCTION write_c_value_var
* 
* Write the C file value node variable name
*
* INPUTS:
*   scb == session control block to use for writing
*   valname == value name to use
*
*********************************************************************/
static void
    write_c_value_var (ses_cb_t *scb,
                       const xmlChar *valname)
{

    write_c_safe_str(scb, valname);
    ses_putstr(scb, (const xmlChar *)"_val");

} /* write_c_value_var */


/********************************************************************
* FUNCTION write_c_static_vars
* 
* Write the C file static variable declarations
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*
*********************************************************************/
static void
    write_c_static_vars (ses_cb_t *scb,
                         ncx_module_t *mod)
{
    obj_template_t           *obj;

#ifdef LEAVE_OUT_FOR_NOW
    const ncx_include_t      *inc;
#endif

    if (mod->ismod) {
        /* print a banner */
        ses_putstr(scb, (const xmlChar *)"\n/* module static variables */");

        /* create a static module back-pointer */
        ses_putstr(scb, (const xmlChar *)"\nstatic ncx_module_t *");
        write_c_safe_str(scb, ncx_get_modname(mod));
        ses_putstr(scb, (const xmlChar *)"_mod;");
    }

    /* create a static object template pointer for
     * each top-level real object, RPC, or notification
     */
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) || 
            obj_is_abstract(obj)) {
            continue;
        }

        ses_putstr(scb, (const xmlChar *)"\nstatic obj_template_t *");
        write_c_object_var(scb, obj_get_name(obj));
        ses_putchar(scb, ';');
    }
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) || 
            obj_is_abstract(obj) ||
            obj_is_rpc(obj) ||
            obj_is_notif(obj)) {
            continue;
        }

        ses_putstr(scb, (const xmlChar *)"\nstatic val_value_t *");
        write_c_value_var(scb, obj_get_name(obj));
        ses_putchar(scb, ';');
    }
    ses_putchar(scb, '\n');

#ifdef LEAVE_OUT_FOR_NOW
    /* includes for submodules */
    if (!cp->unified) {
        for (inc = (const ncx_include_t *)
                 dlq_firstEntry(&mod->includeQ);
             inc != NULL;
             inc = (const ncx_include_t *)dlq_nextEntry(inc)) {

            write_c_static_vars(scb, inc->submodule);
        }
    }
#endif

    ses_putstr(scb, 
               (const xmlChar *)"\n/* put your static variables here */\n");

} /* write_c_static_vars */


/********************************************************************
* FUNCTION write_c_init_static_vars_fn
* 
* Write the C file init static variable function
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*********************************************************************/
static void
    write_c_init_static_vars_fn (ses_cb_t *scb,
                                 ncx_module_t *mod,
                                 const yangdump_cvtparms_t *cp)
{
    const xmlChar            *modname;
    obj_template_t           *obj;
    int32                     indent;

    indent = cp->indent;
    modname = mod->name;

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init_static_vars");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"initialize module static variables");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_END);

    ses_putstr(scb, (const xmlChar *)"\nstatic void");
    ses_indent(scb, indent);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init_static_vars");
    ses_putstr(scb, (const xmlChar *)" (void)\n{");

    if (mod->ismod) {
        ses_indent(scb, indent);
        write_c_safe_str(scb, ncx_get_modname(mod));
        ses_putstr(scb, (const xmlChar *)"_mod = NULL;");
    }
        
    /* create an init line for each top-level 
     * real object, RPC, or notification
     */
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) || 
            obj_is_abstract(obj)) {
            continue;
        }

        ses_indent(scb, indent);
        write_c_object_var(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)" = NULL;");
    }

    /* create an init line for each top-level 
     * real value cache pointer for a database object
     */
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) || 
            obj_is_abstract(obj) ||
            obj_is_rpc(obj) ||
            obj_is_notif(obj)) {
            continue;
        }

        ses_indent(scb, indent);
        write_c_value_var(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)" = NULL;");
    }

    ses_putchar(scb, '\n');
    ses_putstr_indent(scb, 
                      (const xmlChar *)"/* init your "
                      "static variables here */",
                      indent);
    ses_putchar(scb, '\n');

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n} /* ");
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init_static_vars");
    ses_putstr(scb, (const xmlChar *)" */\n");

} /* write_c_init_static_vars_fn */


/********************************************************************
* FUNCTION write_c_edit_cbfn
* 
* Generate the C code for the foo_object_edit function
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   obj == object struct for the database object
*   objnameQ == Q of c_define_t structs to search for this object
*********************************************************************/
static void
    write_c_edit_cbfn (ses_cb_t *scb,
                       ncx_module_t *mod,
                       const yangdump_cvtparms_t *cp,
                       obj_template_t *obj,
                       dlq_hdr_t *objnameQ)
{
    const xmlChar   *modname;
    c_define_t      *cdef;
    int32            indent;
    boolean          toplevel;

    modname = ncx_get_modname(mod);
    indent = cp->indent;
    toplevel = obj_is_top(obj);

    cdef = find_path_cdefine(objnameQ, obj);
    if (cdef == NULL) {
        SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
        return;
    }

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    ses_putstr(scb, cdef->idstr);
    ses_putstr(scb, EDIT_SUFFIX);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"Edit database object callback");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"Path: ");
    ses_putstr(scb, cdef->valstr);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, 
               (const xmlChar *)"Add object instrumentation "
               "in COMMIT phase.");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_INPUT);
    ses_putstr(scb, (const xmlChar *)"    see agt/agt_cb.h for details");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_RETURN_STATUS);
    ses_putstr(scb, FN_BANNER_END);

    /* generate the function prototype lines */
    ses_putstr(scb, (const xmlChar *)"\nstatic status_t");
    ses_putstr_indent(scb, cdef->idstr, indent);
    ses_putstr(scb, EDIT_SUFFIX);
    ses_putstr(scb, (const xmlChar *)" (");

    ses_putstr_indent(scb, 
                      (const xmlChar *)"ses_cb_t *scb,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"rpc_msg_t *msg,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"agt_cbtyp_t cbtyp,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"op_editop_t editop,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"val_value_t *newval,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"val_value_t *curval)",
                      indent+indent);
    ses_putstr(scb, (const xmlChar *)"\n{");

    /* generate static vars */
    ses_putstr_indent(scb, 
                      (const xmlChar *)"status_t res;",
                      indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"val_value_t *errorval;",
                      indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"const xmlChar *errorstr;",
                      indent);

    /* initialize the static vars */
    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"res = NO_ERR;",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"errorval = NULL;",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"errorstr = NULL;",
                      indent);

    if (!obj_is_top(obj)) {
        ses_putchar(scb, '\n');
        ses_putstr_indent(scb,
                          (const xmlChar *)"/* remove the next line "
                          "if newval is used */",
                          indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"(void)newval;",
                          indent);

        ses_putchar(scb, '\n');
        ses_putstr_indent(scb,
                          (const xmlChar *)"/* remove the next line "
                          "if curval is used */",
                          indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"(void)curval;",
                          indent);
    }

    /* generate switch on callback type */
    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"switch (cbtyp) {",
                      indent);

    ses_putstr_indent(scb,
                      (const xmlChar *)"case AGT_CB_VALIDATE:",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"/* description-stmt "
                      "validation here */",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent+indent);

    ses_putstr_indent(scb,
                      (const xmlChar *)"case AGT_CB_APPLY:",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"/* database manipulation "
                      "done here */",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent+indent);

    /* commit case arm */
    ses_putstr_indent(scb,
                      (const xmlChar *)"case AGT_CB_COMMIT:",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"/* device instrumentation "
                      "done here */",
                      indent+indent);

    /* generate switch on editop */
    ses_putstr_indent(scb,
                      (const xmlChar *)"switch (editop) {",
                      indent+indent);

    ses_putstr_indent(scb,
                      (const xmlChar *)"case OP_EDITOP_LOAD:",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent*3);
    ses_putstr_indent(scb,
                      (const xmlChar *)"case OP_EDITOP_MERGE:",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent*3);
    ses_putstr_indent(scb,
                      (const xmlChar *)"case OP_EDITOP_REPLACE:",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent*3);
    ses_putstr_indent(scb,
                      (const xmlChar *)"case OP_EDITOP_CREATE:",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent*3);
    ses_putstr_indent(scb,
                      (const xmlChar *)"case OP_EDITOP_DELETE:",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent*3);
    ses_putstr_indent(scb,
                      (const xmlChar *)"default:",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"res = SET_ERROR(ERR_INTERNAL_VAL);",
                      indent*3);
    ses_putstr_indent(scb,
                      (const xmlChar *)"}",
                      indent+indent);

    /* generate cache object check */
    if (obj_is_top(obj)) {
        ses_putchar(scb, '\n');
        ses_putstr_indent(scb,
                          (const xmlChar *)"if (res == NO_ERR) {",
                          indent+indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"res = agt_check_cache(",
                          indent*3);
        ses_putstr_indent(scb,
                          (const xmlChar *)"&",
                          indent*4);
        write_c_value_var(scb, obj_get_name(obj));
        ses_putchar(scb, ',');
        ses_putstr_indent(scb,
                          (const xmlChar *)"newval,",
                          indent*4);
        ses_putstr_indent(scb,
                          (const xmlChar *)"curval,",
                          indent*4);
        ses_putstr_indent(scb,
                          (const xmlChar *)"editop);",
                          indent*4);
        ses_putstr_indent(scb,
                          (const xmlChar *)"}\n",
                          indent+indent);
    }


    /* generate check on read-only child nodes */
    if (obj_has_ro_children(obj)) {
        ses_putstr_indent(scb,
                          (const xmlChar *)"if (res == NO_ERR &&",
                          indent+indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"(editop == OP_EDITOP_LOAD || "
                          "editop == OP_EDITOP_CREATE)) {",
                          indent*3);
        ses_putstr_indent(scb,
                          (const xmlChar *)"res = ",
                          indent*3);
        ses_putstr(scb, cdef->idstr);
        ses_putstr(scb, MRO_SUFFIX);
        ses_putstr(scb, (const xmlChar *)"(newval);");
        ses_indent(scb, indent+indent);
        ses_putchar(scb, '}');
    }

    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent+indent);

    /* rollback case arm */
    ses_putstr_indent(scb,
                      (const xmlChar *)"case AGT_CB_ROLLBACK:",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"/* undo device instrumentation "
                      "here */",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"break;",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"default:",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"res = SET_ERROR(ERR_INTERNAL_VAL);",
                      indent+indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"}",
                      indent);

    write_if_record_error(scb,
                          cp,
                          indent,
                          FALSE,
                          FALSE,
                          FALSE);

    /* return result */
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"return res;");

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n\n} /* ");
    ses_putstr(scb, cdef->idstr);
    ses_putstr(scb, EDIT_SUFFIX);
    ses_putstr(scb, (const xmlChar *)" */\n");

} /* write_c_edit_cbfn */


/********************************************************************
* FUNCTION write_c_get_cbfn
* 
* Generate the C code for the foo_object_get function
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   obj == object struct for the database object
*   objnameQ == Q of c_define_t structs to search for this object
*********************************************************************/
static void
    write_c_get_cbfn (ses_cb_t *scb,
                      ncx_module_t *mod,
                      const yangdump_cvtparms_t *cp,
                      obj_template_t *obj,
                      dlq_hdr_t *objnameQ)
{
    const xmlChar   *modname, *defval;
    c_define_t      *cdef;
    typ_enum_t      *typ_enum;
    int32            indent;
    ncx_btype_t      btyp;

    modname = ncx_get_modname(mod);
    indent = cp->indent;

    cdef = find_path_cdefine(objnameQ, obj);
    if (cdef == NULL) {
        SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
        return;
    }

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    ses_putstr(scb, cdef->idstr);
    ses_putstr(scb, GET_SUFFIX);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"Get database object callback");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"Path: ");
    ses_putstr(scb, cdef->valstr);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"Fill in 'dstval' contents");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_INPUT);
    ses_putstr(scb, (const xmlChar *)"    see ncx/getcb.h for details");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_RETURN_STATUS);
    ses_putstr(scb, FN_BANNER_END);

    /* generate the function prototype lines */
    ses_putstr(scb, (const xmlChar *)"\nstatic status_t");
    ses_putstr_indent(scb, cdef->idstr, indent);
    ses_putstr(scb, GET_SUFFIX);
    ses_putstr(scb, (const xmlChar *)" (");

    ses_putstr_indent(scb, 
                      (const xmlChar *)"ses_cb_t *scb,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"getcb_mode_t cbmode,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"const val_value_t *virval,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"val_value_t *dstval)",
                      indent+indent);
    ses_putstr(scb, (const xmlChar *)"\n{");

    /* generate static vars */
    ses_putstr_indent(scb, 
                      (const xmlChar *)"status_t res;",
                      indent);

    ses_indent(scb, indent);
    write_c_objtype_ex(scb, obj, ';', TRUE);


    /* initialize the static vars */
    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"res = NO_ERR;",
                      indent);

    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"/* remove the next line "
                      "if scb is used */",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"(void)scb;",
                      indent);

    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"/* remove the next line "
                      "if virval is used */",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"(void)virval;",
                      indent);

    /* generate if-stmt to screen out any other mode except 'get' */
    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"if (cbmode != GETCB_GET_VALUE) {",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"return ERR_NCX_OPERATION"
                      "_NOT_SUPPORTED;",
                      indent+indent);
    ses_putstr_indent(scb, (const xmlChar *)"}", indent);


    ses_putchar(scb, '\n');
    ses_putstr_indent(scb, 
                      (const xmlChar *)"/* set the ", 
                      indent);
    write_c_safe_str(scb, obj_get_name(obj));
    ses_putstr(scb, (const xmlChar *)" var here, ");

    btyp = obj_get_basetype(obj);
    defval = obj_get_default(obj);

    if (defval) {
        ses_putstr(scb, (const xmlChar *)"replace default value */");
        ses_indent(scb, indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)" = (const xmlChar *)\"");
        ses_putstr(scb, defval);
        ses_putstr(scb, (const xmlChar *)"\";");
        ses_putstr_indent(scb, 
                          (const xmlChar *)"res = val_set_simval_obj(",
                          indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"dstval,",
                          indent+indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"dstval->obj,",
                          indent+indent);
        ses_indent(scb, indent+indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)");");
    } else if (typ_is_string(btyp) || 
        btyp == NCX_BT_IDREF ||
        btyp == NCX_BT_INSTANCE_ID ||
        btyp == NCX_BT_UNION ||
        btyp == NCX_BT_LEAFREF) {

        ses_putstr(scb, (const xmlChar *)"change EMPTY_STRING */");
        ses_indent(scb, indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)" = EMPTY_STRING;");
        ses_putstr_indent(scb, 
                          (const xmlChar *)"res = val_set_simval_obj(",
                          indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"dstval,",
                          indent+indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"dstval->obj,",
                          indent+indent);
        ses_indent(scb, indent+indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)");");
    } else if (btyp == NCX_BT_DECIMAL64) {
        ses_putstr(scb, (const xmlChar *)"change zero */");
        ses_indent(scb, indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)" = (const xmlChar *)\"0.0\";");
        ses_putstr_indent(scb, 
                          (const xmlChar *)"res = val_set_simval_obj(",
                          indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"dstval,",
                          indent+indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"dstval->obj,",
                          indent+indent);
        ses_indent(scb, indent+indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)");");
    } else if (typ_is_number(btyp)) {
        ses_putstr(scb, (const xmlChar *)"change zero */");
        ses_indent(scb, indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)" = 0;");

        ses_indent(scb, indent);
        write_c_val_macro_type(scb, obj);
        ses_putstr(scb, (const xmlChar *)"(dstval) = ");
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putchar(scb, ';');
    } else {
        switch (btyp) {
        case NCX_BT_ENUM:
        case NCX_BT_BITS:
            ses_putstr(scb, (const xmlChar *)"change enum */");
            ses_indent(scb, indent);
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)" = ");

            typ_enum = typ_first_enumdef(obj_get_typdef(obj));
            if (typ_enum != NULL) {
                ses_putstr(scb, (const xmlChar *)"(const xmlChar *)\"");
                ses_putstr(scb, typ_enum->name);
                ses_putstr(scb, (const xmlChar *)"\";");
            } else {
                ses_putstr(scb, (const xmlChar *)"EMPTY_STRING;");
            }
            ses_putstr_indent(scb, 
                              (const xmlChar *)"res = val_set_simval_obj(",
                              indent);
            ses_putstr_indent(scb, 
                              (const xmlChar *)"dstval,",
                              indent+indent);
            ses_putstr_indent(scb, 
                              (const xmlChar *)"dstval->obj,",
                              indent+indent);
            ses_indent(scb, indent+indent);
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)");");
            break;
        case NCX_BT_BOOLEAN:
        case NCX_BT_EMPTY:
            ses_putstr(scb, (const xmlChar *)"change TRUE */");
            ses_indent(scb, indent);
            write_c_val_macro_type(scb, obj);
            ses_putchar(scb, '(');
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)") = TRUE;");
            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    /* return result */
    ses_putchar(scb, '\n');
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"return res;");

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n\n} /* ");
    ses_putstr(scb, cdef->idstr);
    ses_putstr(scb, GET_SUFFIX);
    ses_putstr(scb, (const xmlChar *)" */\n");

} /* write_c_get_cbfn */


/********************************************************************
* FUNCTION write_c_mro_fn
* 
* Generate the C code for the foo_object_mro function
* Make read-only children
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   obj == object struct for the database object
*   objnameQ == Q of c_define_t structs to search for this object
*********************************************************************/
static void
    write_c_mro_fn (ses_cb_t *scb,
                    ncx_module_t *mod,
                    const yangdump_cvtparms_t *cp,
                    obj_template_t *obj,
                    dlq_hdr_t *objnameQ)
{
    const xmlChar   *modname;
    c_define_t      *cdef, *childcdef;
    obj_template_t  *childobj;
    int32            indent;
    boolean          needif;

    modname = ncx_get_modname(mod);
    indent = cp->indent;

    cdef = find_path_cdefine(objnameQ, obj);
    if (cdef == NULL) {
        SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
        return;
    }

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    ses_putstr(scb, cdef->idstr);
    ses_putstr(scb, MRO_SUFFIX);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"Make read-only child nodes");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"Path: ");
    ses_putstr(scb, cdef->valstr);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_INPUT);
    ses_putstr(scb, 
               (const xmlChar *)"    parentval == the parent struct to "
               "use for new child nodes");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_RETURN_STATUS);
    ses_putstr(scb, FN_BANNER_END);

    /* generate the function prototype lines */
    ses_putstr(scb, (const xmlChar *)"\nstatic status_t");
    ses_putstr_indent(scb, cdef->idstr, indent);
    ses_putstr(scb, MRO_SUFFIX);
    ses_putstr(scb, (const xmlChar *)" (val_value_t *parentval)");
    ses_putstr(scb, (const xmlChar *)"\n{");

    /* generate static vars */
    ses_putstr_indent(scb, 
                      (const xmlChar *)"status_t res;",
                      indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"val_value_t *childval;",
                      indent);

    /* initialize the static vars */
    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"res = NO_ERR;",
                      indent);

    /* create read-only child nodes as needed */
    for (childobj = obj_first_child(obj);
         childobj != NULL;
         childobj = obj_next_child(childobj)) {

        if (!obj_has_name(childobj) ||
            !obj_is_enabled(childobj) ||
            obj_is_cli(childobj) ||
            obj_is_abstract(childobj) ||
            obj_get_config_flag(childobj)) {
            continue;
        }

        childcdef = find_path_cdefine(objnameQ, childobj);
        if (childcdef == NULL) {
            SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
            return;
        }

        needif = FALSE;

        ses_putchar(scb, '\n');
        switch (childobj->objtype) {
        case OBJ_TYP_LEAF:
            ses_putstr_indent(scb, 
                              (const xmlChar *)"/* add ",
                              indent);
            ses_putstr(scb, childcdef->valstr);
            ses_putstr(scb, (const xmlChar *)" */");
            ses_putstr_indent(scb, 
                              (const xmlChar *)"childval = agt_make_"
                              "virtual_leaf(",
                              indent);
            ses_putstr_indent(scb, 
                              (const xmlChar *)"parentval->obj,",
                              indent+indent);
            ses_indent(scb, indent+indent);
            write_identifier(scb,
                             modname,
                             BAR_NODE,
                             obj_get_name(childobj));
            ses_putchar(scb, ',');
            ses_indent(scb, indent+indent);
            ses_putstr(scb, childcdef->idstr);
            ses_putstr(scb, GET_SUFFIX);
            ses_putchar(scb, ',');            
            ses_putstr_indent(scb, 
                              (const xmlChar *)"&res);",
                              indent+indent);
            needif = TRUE;
            break;
        case OBJ_TYP_LEAF_LIST:

            break;
        case OBJ_TYP_LIST:

            break;
        case OBJ_TYP_CONTAINER:

            break;
        case OBJ_TYP_ANYXML:

            break;
        case OBJ_TYP_CHOICE:

            break;
        case OBJ_TYP_CASE:

            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (needif) {
            ses_putstr_indent(scb, 
                              (const xmlChar *)"if (childval != NULL) {",
                              indent);
            ses_putstr_indent(scb, 
                              (const xmlChar *)"val_add_child(childval, "
                              "parentval);",
                              indent+indent);
            ses_putstr_indent(scb, 
                              (const xmlChar *)"} else {",
                              indent);
            ses_putstr_indent(scb, 
                              (const xmlChar *)"return res;",
                              indent+indent);

            ses_indent(scb, indent);
            ses_putchar(scb, '}');
        }
    }

    /* return result */
    ses_putchar(scb, '\n');
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"return res;");

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n\n} /* ");
    ses_putstr(scb, cdef->idstr);
    ses_putstr(scb, MRO_SUFFIX);
    ses_putstr(scb, (const xmlChar *)" */\n");

} /* write_c_mro_fn */


/********************************************************************
* FUNCTION write_c_objects
* 
* Generate the callback functions for each object found
*
* INPUTS:
*   scb == session to use
*   mod == module in progress
*   cp == conversion parameters to use
*   datadefQ == que of obj_template_t to use
*   objnameQ == Q of c_define_t structs to use
*
*********************************************************************/
static void
    write_c_objects (ses_cb_t *scb,
                     ncx_module_t *mod,
                     const yangdump_cvtparms_t *cp,
                     dlq_hdr_t *datadefQ,
                     dlq_hdr_t *objnameQ)
{
    obj_template_t    *obj;
    dlq_hdr_t         *childdatadefQ;

    for (obj = (obj_template_t *)dlq_firstEntry(datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) ||
            !obj_is_data_db(obj) ||
            obj_is_cli(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        /* generate functions in reverse order so parent functions
         * will be able to access decendant functions without
         * any forward function declarations
         */
        childdatadefQ = obj_get_datadefQ(obj);
        if (childdatadefQ) {
            write_c_objects(scb,
                            mod,
                            cp,
                            childdatadefQ,
                            objnameQ);
        }

        /* generate the callback function: edit or get */
        if (obj_get_config_flag(obj)) {
            /* check if this node has any non-config children */
            if (obj_has_ro_children(obj)) {
                write_c_mro_fn(scb, mod, cp, obj, objnameQ);
            }

            /* generate the foo_edit function */
            write_c_edit_cbfn(scb, mod, cp, obj, objnameQ);
        } else {
            if (obj_is_leafy(obj)) {
                /* generate the foo_get function */
                write_c_get_cbfn(scb, mod, cp, obj, objnameQ);
            }
        }
    }

}  /* write_c_objects */


/********************************************************************
* FUNCTION write_c_rpc_fn
* 
* Generate the C code for the foo_rpc_fn_validate function
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   rpcobj == object struct for the RPC method
*   is_validate == TUE for _validate, FALSE for _invoke
*********************************************************************/
static void
    write_c_rpc_fn (ses_cb_t *scb,
                    ncx_module_t *mod,
                    const yangdump_cvtparms_t *cp,
                    obj_template_t *rpcobj,
                    boolean is_validate)
{
    obj_template_t  *inputobj, *outputobj, *obj;
    const xmlChar   *modname;
    int32            indent;
    boolean          hasinput, hasoutput;

    modname = ncx_get_modname(mod);
    indent = cp->indent;

    inputobj = obj_find_child(rpcobj, NULL, YANG_K_INPUT);
    hasinput = 
        (inputobj != NULL && obj_has_children(inputobj)) ?
        TRUE : FALSE;

    outputobj = obj_find_child(rpcobj, NULL, YANG_K_OUTPUT);
    hasoutput = 
        (outputobj != NULL && obj_has_children(outputobj)) ?
        TRUE : FALSE;

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    write_identifier(scb, 
                     modname,
                     NULL,
                     obj_get_name(rpcobj));
    if (is_validate) {
        ses_putstr(scb, (const xmlChar *)"_validate");
    } else {
        ses_putstr(scb, (const xmlChar *)"_invoke");
    }
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_LN);
    if (is_validate) {
        ses_putstr(scb, (const xmlChar *)"RPC validation phase");
        ses_putstr(scb, FN_BANNER_LN);
        ses_putstr(scb, (const xmlChar *)"All YANG constriants have "
                   "passed at this point.");
        ses_putstr(scb, FN_BANNER_LN);
        ses_putstr(scb, (const xmlChar *)"Add description-stmt checks "
                   "in this function.");
    } else {
        ses_putstr(scb, (const xmlChar *)"RPC invocation phase");
        ses_putstr(scb, FN_BANNER_LN);
        ses_putstr(scb, (const xmlChar *)"All constriants have "
                   "passed at this point.");
        ses_putstr(scb, FN_BANNER_LN);
        ses_putstr(scb, (const xmlChar *)"Call "
                   "device instrumentation code "
                   "in this function.");
    }
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_INPUT);
    ses_putstr(scb, (const xmlChar *)"    see agt/agt_rpc.h for details");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_RETURN_STATUS);
    ses_putstr(scb, FN_BANNER_END);

    /* generate the function prototype lines */
    ses_putstr(scb, (const xmlChar *)"\nstatic status_t");
    ses_indent(scb, indent);
    write_identifier(scb, 
                     modname,
                     NULL,
                     obj_get_name(rpcobj));
    if (is_validate) {
        ses_putstr(scb, (const xmlChar *)"_validate (");
    } else {
        ses_putstr(scb, (const xmlChar *)"_invoke (");
    }
    ses_putstr_indent(scb, 
                      (const xmlChar *)"ses_cb_t *scb,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"rpc_msg_t *msg,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"xml_node_t *methnode)",
                      indent+indent);
    ses_putstr(scb, (const xmlChar *)"\n{");

    ses_putstr_indent(scb, 
                      (const xmlChar *)"status_t res;",
                      indent);

    if (is_validate) {
        ses_putstr_indent(scb, 
                          (const xmlChar *)"val_value_t *errorval;",
                          indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"const xmlChar *errorstr;",
                          indent);
    }

    if (hasinput) {
        /* declare value pointer node variables for 
         * the RPC input parameters 
         */
        for (obj = obj_first_child(inputobj);
             obj != NULL;
             obj = obj_next_child(obj)) {

            if (obj_is_abstract(obj)) {
                continue;
            }

            ses_putstr_indent(scb, 
                              (const xmlChar *)"val_value_t *",
                              indent);
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)"_val;");
        }

        /* declare variables for the RPC input parameters */
        for (obj = obj_first_child(inputobj);
             obj != NULL;
             obj = obj_next_child(obj)) {

            if (obj_is_abstract(obj)) {
                continue;
            }

            ses_indent(scb, indent);
            write_c_objtype(scb, obj);
        }
    }

    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"res = NO_ERR;",
                      indent);

    if (is_validate) {
        ses_putstr_indent(scb,
                          (const xmlChar *)"errorval = NULL;",
                          indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"errorstr = NULL;",
                          indent);
    }

    if (hasinput) {
        /* retrieve the parameter from the input */
        for (obj = obj_first_child(inputobj);
             obj != NULL;
             obj = obj_next_child(obj)) {

            if (obj_is_abstract(obj)) {
                continue;
            }

            ses_putchar(scb, '\n');
            ses_indent(scb, indent);
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)"_val = val_find_child(");

            ses_putstr_indent(scb,
                              (const xmlChar *)"msg->rpc_input,",
                              indent+indent);
            ses_indent(scb, indent+indent);
            write_identifier(scb,
                             modname,
                             BAR_MOD,
                             modname);
            ses_putchar(scb, ',');

            ses_indent(scb, indent+indent);
            write_identifier(scb,
                             modname,
                             BAR_NODE,
                             obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)");");
            ses_putstr_indent(scb,
                              (const xmlChar *)"if (",
                              indent);
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)"_val != NULL && ");
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)"_val->res == NO_ERR) {");

            ses_indent(scb, indent+indent);
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)" = ");
            write_c_val_macro_type(scb, obj);
            ses_putchar(scb, '(');
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)"_val);");
            ses_indent(scb, indent);
            ses_putchar(scb, '}');

        }
    }

    if (is_validate) {
        /* write generic record error code */
        write_if_record_error(scb, 
                              cp, 
                              indent, 
                              TRUE, 
                              FALSE,
                              TRUE);
    } else {
        ses_putchar(scb, '\n');
        ses_putstr_indent(scb,
                          (const xmlChar *)"/* remove the next line "
                          "if scb is used */",
                          indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"(void)scb;",
                          indent);

        if (!hasinput) {
            ses_putchar(scb, '\n');
            ses_putstr_indent(scb,
                              (const xmlChar *)"/* remove the next line "
                              "if msg is used */",
                              indent);
            ses_putstr_indent(scb,
                              (const xmlChar *)"(void)msg;",
                              indent);
        }

        ses_putchar(scb, '\n');
        ses_putstr_indent(scb,
                          (const xmlChar *)"/* remove the next line "
                          "if methnode is used */",
                          indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"(void)methnode;",
                          indent);

        ses_putchar(scb, '\n');
        ses_putstr_indent(scb,
                          (const xmlChar *)"/* invoke your device "
                          "instrumentation code here */\n",
                          indent);
    }

    /* return NO_ERR */
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"return res;");

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n\n} /* ");
    write_identifier(scb, 
                     modname,
                     NULL,
                     obj_get_name(rpcobj));
    if (is_validate) {
        ses_putstr(scb, (const xmlChar *)"_validate */\n");
    } else {
        ses_putstr(scb, (const xmlChar *)"_invoke */\n");
    }

} /* write_c_rpc_fn */


/********************************************************************
* FUNCTION write_c_rpcs
* 
* Generate the C file decls for the RPC methods in the 
* specified module
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == que of obj_template_t to use
*   cp == conversion parameters to use
*********************************************************************/
static void
    write_c_rpcs (ses_cb_t *scb,
                  ncx_module_t *mod,
                  const yangdump_cvtparms_t *cp)                  
{
    obj_template_t    *obj;
    /* dlq_hdr_t         *childdatadefQ; */



    (void)scb;


    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_is_rpc(obj) || 
            !obj_is_enabled(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        write_c_rpc_fn(scb, mod, cp, obj, TRUE);
        write_c_rpc_fn(scb, mod, cp, obj, FALSE);
    }

}  /* write_c_rpcs */


/********************************************************************
* FUNCTION write_c_notif
* 
* Generate the C file decls for 1 notification
*
* INPUTS:
*   scb == session control block to use for writing
*   datadefQ == que of obj_template_t to use
*   cp == conversion parms to use
*********************************************************************/
static void
    write_c_notif (ses_cb_t *scb,
                   obj_template_t *notifobj,
                   const yangdump_cvtparms_t *cp)
{
    obj_template_t  *obj, *nextobj;
    const xmlChar   *modname;
    int32            indent;
    boolean          haspayload, anydone;

    modname = obj_get_mod_name(notifobj);
    indent = cp->indent;
    haspayload = obj_has_children(notifobj);

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    write_identifier(scb, 
                     modname,
                     NULL,
                     obj_get_name(notifobj));
    ses_putstr(scb, (const xmlChar *)"_send");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"Send a ");
    write_identifier(scb,
                     obj_get_mod_name(notifobj),
                     NULL,
                     obj_get_name(notifobj));
    ses_putstr(scb, (const xmlChar *)" notification");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, 
               (const xmlChar *)"Called by your code when "
               "notification event occurs");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_END);

    /* generate the function prototype lines */
    ses_putstr(scb, (const xmlChar *)"\nvoid");
    ses_indent(scb, indent);
    write_identifier(scb,
                     obj_get_mod_name(notifobj),
                     NULL,
                     obj_get_name(notifobj));
    ses_putstr(scb, (const xmlChar *)"_send (");

    anydone = FALSE;
    for (obj = obj_first_child(notifobj);
         obj != NULL;
         obj = nextobj) {

        nextobj = obj_next_child(obj);
        anydone = TRUE;
        ses_indent(scb, indent+indent);
        write_c_objtype_ex(scb, 
                           obj,
                           (nextobj == NULL) ? ')' : ',',
                           TRUE);
    }
    if (!anydone) {
        ses_putstr(scb, (const xmlChar *)"void)");
    }

    ses_putchar(scb, '\n');
    ses_putchar(scb, '{');

    /* print a debug message */
    ses_putstr_indent(scb, 
                      (const xmlChar *)"agt_not_msg_t *notif;",
                      indent);

    if (haspayload) {
        ses_putstr_indent(scb, 
                          (const xmlChar *)"val_value_t *parmval;",
                          indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"status_t res;",
                          indent);

        ses_putchar(scb, '\n');
        ses_putstr_indent(scb,
                          (const xmlChar *)"res = NO_ERR;",
                          indent);
    }

    ses_putchar(scb, '\n');
    ses_putstr_indent(scb,
                      (const xmlChar *)"if (LOGDEBUG) {",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"log_debug(\"\\nGenerating <",
                      indent+indent);
    ses_putstr(scb, obj_get_name(notifobj));
    ses_putstr(scb,
               (const xmlChar *)"> notification\");");
    ses_putstr_indent(scb,
                      (const xmlChar *)"}\n",
                      indent);

    /* allocate a new notification */
    ses_putstr_indent(scb,
                      (const xmlChar *)"notif = agt_not_new_notification(",
                      indent);
    write_c_safe_str(scb, obj_get_name(notifobj));
    ses_putstr(scb, (const xmlChar *)"_obj);");
    ses_putstr_indent(scb,
                      (const xmlChar *)"if (notif == NULL) {",
                      indent);
    ses_putstr_indent(scb,
                      (const xmlChar *)"log_error(\"\\nError: "
                      "malloc failed, cannot send <",
                      indent+indent);
    ses_putstr(scb, obj_get_name(notifobj));
    ses_putstr(scb, (const xmlChar *)"> notification\");");
    ses_putstr_indent(scb, 
                      (const xmlChar *)"return;",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"}\n",
                      indent);


    for (obj = obj_first_child(notifobj);
         obj != NULL;
         obj = obj_next_child(obj)) {

        if (obj_is_abstract(obj) ||
            obj_is_cli(obj)) {
            continue;
        }

        if (obj->objtype != OBJ_TYP_LEAF) {
            /* TBD: need to handle non-leafs automatically */
            ses_putstr_indent(scb,
                              (const xmlChar *)"/* add ",
                              indent);
            ses_putstr(scb, obj_get_typestr(obj));
            ses_putchar(scb, ' ');
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)" to payload");
            ses_putstr_indent(scb,
                              (const xmlChar *)"* replace following line"
                              "with real code",
                              indent);
            ses_putstr_indent(scb,
                              (const xmlChar *)"(void)",
                              indent);
            write_c_safe_str(scb, obj_get_name(obj));
            ses_putstr(scb, (const xmlChar *)";\n");
            continue;
        }

        /* this is a leaf parameter -- add starting comment */
        ses_putstr_indent(scb,
                          (const xmlChar *)"/* add ",
                          indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)" to payload */");

        /* agt_make_leaf stmt */
        ses_putstr_indent(scb, 
                          (const xmlChar *)"parmval = agt_make_leaf(",
                          indent);
        ses_indent(scb, indent+indent);
        write_c_safe_str(scb, obj_get_name(notifobj));
        ses_putstr(scb, (const xmlChar *)"_obj,");
        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_NODE,
                         obj_get_name(obj));
        ses_putchar(scb, ',');
        ses_indent(scb, indent+indent);
        write_c_safe_str(scb, obj_get_name(obj));
        ses_putchar(scb, ',');
        ses_putstr_indent(scb, 
                          (const xmlChar *)"&res);",
                          indent+indent);

        /* check result stmt, Q leaf if non-NULL */
        ses_putstr_indent(scb, 
                          (const xmlChar *)"if (parmval == NULL) {",
                          indent);

        ses_putstr_indent(scb,
                          (const xmlChar *)"log_error(",
                          indent+indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"\"\\nError: "
                          "make leaf failed (%s), cannot send <",
                          indent*3);
        ses_putstr(scb, obj_get_name(notifobj));
        ses_putstr(scb, (const xmlChar *)"> notification\",");
        ses_putstr_indent(scb,
                          (const xmlChar *)"get_error_string(res));",
                          indent*3);
        ses_putstr_indent(scb,
                          (const xmlChar *)"} else {",
                          indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"agt_not_add_to_payload"
                          "(notif, parmval);",
                          indent+indent);
        ses_putstr_indent(scb,
                          (const xmlChar *)"}\n",
                          indent);
    }

    /* save the malloced notification struct */
    ses_putstr_indent(scb,
                      (const xmlChar *)"agt_not_queue_notification(notif);\n",
                      indent);

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n} /* ");
    write_identifier(scb, 
                     modname,
                     NULL,
                     obj_get_name(notifobj));
    ses_putstr(scb, (const xmlChar *)"_send */\n");

}  /* write_c_notif */


/********************************************************************
* FUNCTION write_c_notifs
* 
* Generate the C file decls for the notifications in the 
* specified datadefQ
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parms to use
*********************************************************************/
static void
    write_c_notifs (ses_cb_t *scb,
                    ncx_module_t *mod,
                    const yangdump_cvtparms_t *cp)
{
    obj_template_t    *obj;

    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_is_notif(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        write_c_notif(scb, obj, cp);
    }

}  /* write_c_notifs */


/********************************************************************
* FUNCTION write_c_init_fn
* 
* Generate the C code for the foo_init function
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   objnameQ == Q of c_define_t structs to use to find
*               the mapped C identifier for each object
*********************************************************************/
static void
    write_c_init_fn (ses_cb_t *scb,
                     ncx_module_t *mod,
                     const yangdump_cvtparms_t *cp,
                     dlq_hdr_t *objnameQ)
{
    const xmlChar   *modname, *modversion;
    obj_template_t  *obj;
    c_define_t      *cdef;
    int32            indent;

    modname = ncx_get_modname(mod);
    indent = cp->indent;

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"initialize the ");
    ses_putstr(scb, mod->name);
    ses_putstr(scb, (const xmlChar *)" server instrumentation library");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_INPUT);
    ses_putstr(scb, (const xmlChar *)"   modname == requested module name");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, (const xmlChar *)"   revision == requested "
               "version (NULL for any)");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_RETURN_STATUS);
    ses_putstr(scb, FN_BANNER_END);

    /* generate the function prototype lines */
    ses_putstr(scb, (const xmlChar *)"\nstatus_t");
    ses_indent(scb, indent);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init");
    ses_putstr(scb, (const xmlChar *)" (");
    ses_putstr_indent(scb, 
                      (const xmlChar *)"const xmlChar *modname,",
                      indent+indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"const xmlChar *revision)",
                      indent+indent);
    ses_putstr(scb, (const xmlChar *)"\n{");

    /* declare the function variables */
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"agt_profile_t *agt_profile;");

    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"status_t res;");

    ses_putchar(scb, '\n');

    /* call the init_static_vars function */
    ses_indent(scb, indent);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init_static_vars");
    ses_putstr(scb, (const xmlChar *)"();\n");

    /* check the input variables */
    ses_putstr_indent(scb, 
                      (const xmlChar *)"/* change if custom handling done */",
                      indent);
    ses_putstr_indent(scb, 
                      (const xmlChar *)"if (xml_strcmp(modname, ",
                      indent);
    write_identifier(scb, modname, BAR_MOD, modname);
    ses_putstr(scb, (const xmlChar *)")) {");
    ses_putstr_indent(scb, 
                      (const xmlChar *)"return ERR_NCX_UNKNOWN_MODULE;",
                      indent+indent);
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"}\n");

    ses_putstr_indent(scb, 
                      (const xmlChar *)"if (revision && xml_strcmp(revision, ",
                      indent);
    write_identifier(scb, modname, BAR_REV, modname);
    ses_putstr(scb, (const xmlChar *)")) {");
    ses_putstr_indent(scb, 
                      (const xmlChar *)"return ERR_NCX_WRONG_VERSION;",
                      indent+indent);
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"}\n");
    
    /* init the function variables */
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"agt_profile = agt_get_profile();");

    /* load the module */
    if (mod->ismod) {
        ses_putchar(scb, '\n');
        ses_indent(scb, indent);
        ses_putstr(scb, (const xmlChar *)"res = ncxmod_load_module(");

        ses_indent(scb, indent+indent);
        write_identifier(scb, 
                         modname,
                         BAR_MOD,
                         modname);
        ses_putchar(scb, ',');

        if (mod->version) {
            ses_indent(scb, indent+indent);
            write_identifier(scb,
                             modname,
                             BAR_REV,
                             modname);
            ses_putchar(scb, ',');
        } else {
            ses_putstr_indent(scb,
                              (const xmlChar *)"NULL,",
                              indent+indent);
        }

        ses_putstr_indent(scb,
                          (const xmlChar *)"&agt_profile->agt_savedevQ,",
                          indent+indent);
        ses_indent(scb, indent+indent);
        ses_putchar(scb, '&');
        write_c_safe_str(scb, modname);
        ses_putstr(scb, (const xmlChar *)"_mod);");
        write_if_res(scb, cp, indent);
    } else {
        ses_putstr_indent(scb,
                          (const xmlChar *)"res = NO_ERR;",
                          indent);
    }

    /* load the static object variable pointers */
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_cli(obj) || 
            obj_is_abstract(obj)) {
            continue;
        }

        ses_indent(scb, indent);
        write_c_object_var(scb, obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)" = ncx_find_object(");
        ses_indent(scb, indent+indent);
        write_c_safe_str(scb, modname);
        ses_putstr(scb, (const xmlChar *)"_mod,");
        ses_indent(scb, indent+indent);
        write_identifier(scb, 
                         modname,
                         BAR_NODE,
                         obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)");");
        ses_putstr_indent(scb, 
                          (const xmlChar *)"if (", indent);
        write_c_safe_str(scb, modname);
        ses_putstr(scb, (const xmlChar *)"_mod == NULL) {");
        ses_putstr_indent(scb, 
                          (const xmlChar *)"return SET_ERROR("
                          "ERR_NCX_DEF_NOT_FOUND);", 
                          indent+indent);
        ses_putstr_indent(scb, 
                          (const xmlChar *)"}\n",
                          indent);
        
    }

    /* initialize any RPC methods */
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_is_rpc(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        /* register validate function */
        ses_putstr_indent(scb, 
                          (const xmlChar *)"res = agt_rpc_register_method(",
                          indent);

        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_MOD,
                         modname);
        ses_putchar(scb, ',');

        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_NODE,
                         obj_get_name(obj));
        ses_putchar(scb, ',');

        ses_putstr_indent(scb, 
                          (const xmlChar *)"AGT_RPC_PH_VALIDATE,",
                          indent+indent);
        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         NULL,
                         obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)"_validate);");

        write_if_res(scb, cp, indent);

        /* register invoke function */
        ses_putstr_indent(scb, 
                          (const xmlChar *)"res = agt_rpc_register_method(",
                          indent);

        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_MOD,
                         modname);
        ses_putchar(scb, ',');

        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_NODE,
                         obj_get_name(obj));
        ses_putchar(scb, ',');

        ses_putstr_indent(scb, 
                          (const xmlChar *)"AGT_RPC_PH_INVOKE,",
                          indent+indent);
        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         NULL,
                         obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)"_invoke);");

        write_if_res(scb, cp, indent);
    }

    /* initialize any object callbacks here */
    for (cdef = (c_define_t *)dlq_firstEntry(objnameQ);
         cdef != NULL;
         cdef = (c_define_t *)dlq_nextEntry(cdef)) {

        if (!obj_is_data_db(cdef->obj) ||
            obj_is_cli(cdef->obj) ||
            obj_is_abstract(cdef->obj)) {
            continue;
        }

        if (obj_get_config_flag(cdef->obj)) {
            /* register the edit callback */
            ses_putstr_indent(scb, 
                              (const xmlChar *)"res = agt_cb_"
                              "register_callback(",
                          indent);

            ses_indent(scb, indent+indent);
            write_identifier(scb,
                             modname,
                             BAR_MOD,
                             modname);
            ses_putchar(scb, ',');

            ses_indent(scb, indent+indent);
            ses_putstr(scb, (const xmlChar *)"(const xmlChar *)\"");
            ses_putstr(scb, cdef->valstr);
            ses_putchar(scb, '"');
            ses_putchar(scb, ',');
            modversion = obj_get_mod_version(cdef->obj);
            if (modversion == NULL) {
                ses_putstr_indent(scb,
                                  (const xmlChar *)"NULL,",
                                  indent+indent);
            } else {
                ses_indent(scb, indent+indent);
                ses_putstr(scb, (const xmlChar *)"(const xmlChar *)\"");
                ses_putstr(scb, modversion);
                ses_putchar(scb, '"');
                ses_putchar(scb, ',');
            }
            ses_putstr_indent(scb, cdef->idstr, indent+indent);
            ses_putstr(scb, (const xmlChar *)"_edit);");
            write_if_res(scb, cp, indent);
        } else {
            /* nothing to do now for read-only objects */
        }
    }

    ses_putstr_indent(scb,
                      (const xmlChar *)"/* put your module "
                      "initialization code here */\n",
                      indent);


    /* return res; */
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"return res;");

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n} /* ");
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init");
    ses_putstr(scb, (const xmlChar *)" */\n");

} /* write_c_init_fn */


/********************************************************************
* FUNCTION write_c_init2_fn
* 
* Generate the C code for the foo_init2 function
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   objnameQ == Q of object ID bindings to use
*********************************************************************/
static void
    write_c_init2_fn (ses_cb_t *scb,
                      ncx_module_t *mod,
                      const yangdump_cvtparms_t *cp,
                      dlq_hdr_t *objnameQ)
{
    c_define_t      *cdef;
    const xmlChar   *modname;
    int32            indent;

    modname = ncx_get_modname(mod);
    indent = cp->indent;

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init2");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, 
               (const xmlChar *)"SIL init phase 2: "
               "non-config data structures");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, 
               (const xmlChar *)"Called after running config is loaded");
    ses_putstr(scb, FN_BANNER_LN);

    ses_putstr(scb, FN_BANNER_RETURN_STATUS);
    ses_putstr(scb, FN_BANNER_END);

    /* generate the function prototype lines */
    ses_putstr(scb, (const xmlChar *)"\nstatus_t");
    ses_indent(scb, indent);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init2");
    ses_putstr(scb, (const xmlChar *)" (void)\n{");

    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"status_t res;\n");


    ses_putstr_indent(scb,
                      (const xmlChar *)"res = NO_ERR;",
                      indent);


    /* generate any cache inits here */
    for (cdef = (c_define_t *)dlq_firstEntry(objnameQ);
         cdef != NULL;
         cdef = (c_define_t *)dlq_nextEntry(cdef)) {

        if (!obj_is_data_db(cdef->obj) ||
            !obj_is_top(cdef->obj) ||
            obj_is_cli(cdef->obj) ||
            obj_is_abstract(cdef->obj)) {
            continue;
        }

        /* init the cache value pointer */
        ses_putchar(scb, '\n');
        ses_indent(scb, indent);
        write_c_value_var(scb, obj_get_name(cdef->obj));
        ses_putstr(scb, (const xmlChar *)" = agt_init_cache(");
        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_MOD,
                         modname);
        ses_putchar(scb, ',');
        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_NODE,
                         obj_get_name(cdef->obj));
        ses_putchar(scb, ',');
        ses_putstr_indent(scb, 
                          (const xmlChar *)"&res);",
                          indent+indent);
        write_if_res(scb, cp, indent);
    }

    ses_putstr_indent(scb, 
                      (const xmlChar *)"/* put your init2 code here */\n",
                      indent);

    /* return NO_ERR */
    ses_indent(scb, indent);
    ses_putstr(scb, (const xmlChar *)"return res;");

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n} /* ");
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"init2");
    ses_putstr(scb, (const xmlChar *)" */\n");

} /* write_c_init2_fn */


/********************************************************************
* FUNCTION write_c_cleanup_fn
* 
* Generate the C code for the foo_cleanup function
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   objnameQ == Q of object ID bindings to use
*********************************************************************/
static void
    write_c_cleanup_fn (ses_cb_t *scb,
                        ncx_module_t *mod,
                        const yangdump_cvtparms_t *cp,
                        dlq_hdr_t *objnameQ)                        
{
    obj_template_t  *obj;
    const xmlChar   *modname;
    c_define_t      *cdef;
    int32            indent;

    modname = ncx_get_modname(mod);
    indent = cp->indent;

    /* generate function banner comment */
    ses_putstr(scb, FN_BANNER_START);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"cleanup");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, 
               (const xmlChar *)"   cleanup the server "
               "instrumentation library");
    ses_putstr(scb, FN_BANNER_LN);
    ses_putstr(scb, FN_BANNER_END);

    /* generate the function prototype lines */
    ses_putstr(scb, (const xmlChar *)"\nvoid");
    ses_indent(scb, indent);
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"cleanup");
    ses_putstr(scb, (const xmlChar *)" (void)\n{");

    /* function contents */

    /* cleanup any RPC methods */
    for (obj = (obj_template_t *)dlq_firstEntry(&mod->datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_is_rpc(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        /* unregister all callback fns for this RPC function */
        ses_putstr_indent(scb, 
                          (const xmlChar *)"agt_rpc_unregister_method(",
                          indent);

        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_MOD,
                         modname);
        ses_putchar(scb, ',');

        ses_indent(scb, indent+indent);
        write_identifier(scb,
                         modname,
                         BAR_NODE,
                         obj_get_name(obj));
        ses_putstr(scb, (const xmlChar *)");\n");
    }


    /* cleanup any registered callbacks */
    for (cdef = (c_define_t *)dlq_firstEntry(objnameQ);
         cdef != NULL;
         cdef = (c_define_t *)dlq_nextEntry(cdef)) {

        if (!obj_is_data_db(cdef->obj) ||
            obj_is_cli(cdef->obj) ||
            obj_is_abstract(cdef->obj)) {
            continue;
        }

        if (obj_get_config_flag(cdef->obj)) {
            /* register the edit callback */
            ses_putstr_indent(scb, 
                              (const xmlChar *)"agt_cb_"
                              "unregister_callbacks(",
                          indent);

            ses_indent(scb, indent+indent);
            write_identifier(scb,
                             modname,
                             BAR_MOD,
                             modname);
            ses_putchar(scb, ',');

            ses_indent(scb, indent+indent);
            ses_putstr(scb, (const xmlChar *)"(const xmlChar *)\"");
            ses_putstr(scb, cdef->valstr);
            ses_putstr(scb, (const xmlChar *)"\");\n");
        } else {
            /* nothing to do now for read-only objects */
        }
    }

    ses_putstr_indent(scb, 
                      (const xmlChar *)"/* put your cleanup code here */\n",
                      indent);

    /* end the function */
    ses_putstr(scb, (const xmlChar *)"\n} /* ");
    write_identifier(scb, 
                     modname,
                     NULL,
                     (const xmlChar *)"cleanup");
    ses_putstr(scb, (const xmlChar *)" */\n");

} /* write_c_cleanup_fn */


/********************************************************************
* FUNCTION write_c_file
* 
* Generate the C file for the module
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    write_c_file (ses_cb_t *scb,
                  ncx_module_t *mod,
                  const yangdump_cvtparms_t *cp)
{
    yang_node_t *node;
    dlq_hdr_t    objnameQ;
    status_t     res;

    dlq_createSQue(&objnameQ);
    res = NO_ERR;

    /* name strings for callback fns for objects */
    res = save_c_objects(mod, 
                         &mod->datadefQ, 
                         &objnameQ,
                         C_MODE_CALLBACK);
    if (res == NO_ERR) {
        if (cp->unified && mod->ismod) {
            for (node = (yang_node_t *)
                     dlq_firstEntry(&mod->saveincQ);
                 node != NULL && res == NO_ERR;
                 node = (yang_node_t *)dlq_nextEntry(node)) {
                if (node->submod) {
                    res = save_c_objects(node->submod,
                                         &node->submod->datadefQ, 
                                         &objnameQ,
                                         C_MODE_CALLBACK);
                }
            }
        }
    }

    if (res == NO_ERR) {
        /* file header, meta-data, static data decls */
        write_c_header(scb, mod, cp);
        write_c_includes(scb, mod, cp);
        write_c_static_vars(scb, mod);

        /* static functions */
        write_c_objects(scb, 
                        mod, 
                        cp, 
                        &mod->datadefQ, 
                        &objnameQ);
        if (cp->unified && mod->ismod) {
            for (node = (yang_node_t *)
                     dlq_firstEntry(&mod->saveincQ);
                 node != NULL && res == NO_ERR;
                 node = (yang_node_t *)dlq_nextEntry(node)) {
                if (node->submod) {
                    write_c_objects(scb,
                                    node->submod,
                                    cp,
                                    &node->submod->datadefQ,
                                    &objnameQ);
                }
            }
        }

        write_c_rpcs(scb, mod, cp);

        write_c_init_static_vars_fn(scb, mod, cp);

        /* external functions */
        write_c_notifs(scb, mod, cp);
        write_c_init_fn(scb, mod, cp, &objnameQ);
        write_c_init2_fn(scb, mod, cp, &objnameQ);
        write_c_cleanup_fn(scb, mod, cp, &objnameQ);

        /* end comment */
        write_c_footer(scb, mod);
    }

    clean_cdefineQ(&objnameQ);

    return res;

} /* write_c_file */


/*********     E X P O R T E D   F U N C T I O N S    **************/


/********************************************************************
* FUNCTION c_convert_module
* 
*
* INPUTS:
*   pcb == parser control block of module to convert
*          This is returned from ncxmod_load_module_ex
*   cp == conversion parms to use
*   scb == session control block for writing output
*
* RETURNS:
*   status
*********************************************************************/
status_t
    c_convert_module (yang_pcb_t *pcb,
                      const yangdump_cvtparms_t *cp,
                      ses_cb_t *scb)
{
    ncx_module_t  *mod;
    status_t       res;

    res = NO_ERR;

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
        return SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
    }

    res = write_c_file(scb, mod, cp);
    (void)cp;
    (void)scb;
    return res;

}   /* c_convert_module */


/* END file c.c */
