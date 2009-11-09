/*  FILE: h.c

    Generate H file output from YANG module

   Identifier type codes:

      Features        --> 'F'
      Identities      --> 'I'
      Data Nodes      --> 'N'
      Typedef         --> 'T'

   Identifier format:

     <cvt-module-name>_<identifier-type>_<cvt-identifier>

   Identifiers are converted as follows:

     '.' -->  '_'
     '-' -->  '_'

   Collisions on pathalogical collisions are not supported yet


*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
28mar09      abb      begun

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

#ifndef _H_c_util
#include "c_util.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ext
#include "ext.h"
#endif

#ifndef _H_h
#include "h.h"
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
*                       V A R I A B L E S                            *
*                                                                   *
*********************************************************************/



/********************************************************************
* FUNCTION write_h_iffeature_start
* 
* Generate the start C for 1 if-feature conditional;
*
* INPUTS:
*   scb == session control block to use for writing
*   iffeatureQ == Q of ncx_feature_t to use
*
*********************************************************************/
static void
    write_h_iffeature_start (ses_cb_t *scb,
                             const dlq_hdr_t *iffeatureQ)
{
    ncx_iffeature_t   *iffeature, *nextif;
    uint32             iffeaturecnt;

    iffeaturecnt = dlq_count(iffeatureQ);

    /* check if conditional wrapper needed */
    if (iffeaturecnt == 1) {
        iffeature = (ncx_iffeature_t *)
            dlq_firstEntry(iffeatureQ);

        ses_putstr(scb, POUND_IFDEF);
        write_identifier(scb, 
                         iffeature->feature->tkerr.mod->name,
                         BAR_FEAT,
                         iffeature->feature->name);
    } else if (iffeaturecnt > 1) {
        ses_putstr(scb, POUND_IF);
        ses_putchar(scb, '(');

        for (iffeature = (ncx_iffeature_t *)
                 dlq_firstEntry(iffeatureQ);
             iffeature != NULL;
             iffeature = nextif) {

            nextif = (ncx_iffeature_t *)dlq_nextEntry(iffeature);

            ses_putstr(scb, START_DEFINED);
            write_identifier(scb, 
                             iffeature->feature->tkerr.mod->name,
                             BAR_FEAT,
                             iffeature->feature->name);
            ses_putchar(scb, ')');

            if (nextif) {
                ses_putstr(scb, (const xmlChar *)" && ");
            }
        }
        ses_putchar(scb, ')');
    }

}  /* write_h_iffeature_start */


/********************************************************************
* FUNCTION write_h_iffeature_end
* 
* Generate the end C for 1 if-feature conditiona;
*
* INPUTS:
*   scb == session control block to use for writing
*   iffeatureQ == Q of ncx_feature_t to use
*
*********************************************************************/
static void
    write_h_iffeature_end (ses_cb_t *scb,
                           const dlq_hdr_t *iffeatureQ)
{
    if (!dlq_empty(iffeatureQ)) {
        ses_putstr(scb, POUND_ENDIF);
    }

}  /* write_h_iffeature_end */


/********************************************************************
* FUNCTION write_h_object_typdef
* 
* Generate the H file typdefs definitions for 1 data node
*
* INPUTS:
*   scb == session control block to use for writing
*   obj == obj_template_t to use
*   cdefQ == Q of c_define_t structs to check for identity binding
**********************************************************************/
static void
    write_h_object_typdef (ses_cb_t *scb,
                           obj_template_t *obj,
                           dlq_hdr_t *cdefQ)
{
    c_define_t              *cdef;
    obj_template_t          *childobj;
    ncx_btype_t              btyp;
    boolean                  isleaflist;

    isleaflist = FALSE;

    /* typedefs not needed for simple objects */
    if (obj->objtype == OBJ_TYP_LEAF ||
        obj->objtype == OBJ_TYP_ANYXML) {
        return;
    } else if (obj->objtype == OBJ_TYP_LEAF_LIST) {
        isleaflist = TRUE;
    }

    cdef = find_path_cdefine(cdefQ, obj);
    if (cdef == NULL) {
        SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
        return;
    }

    btyp = obj_get_basetype(obj);

    if (obj->objtype == OBJ_TYP_CONTAINER &&
        !obj_has_children(obj)) {
        /* skip empty containers */
        return;
    }

    /* generate the YOID as a comment */
    write_c_oid_comment(scb, obj);

    /* start a new line and a C type definition */
    ses_putstr(scb, START_TYPEDEF);

    if (isleaflist) {
        ses_putstr(scb, STRUCT);
    } else {
        /* print the C top-level data type */
        switch (btyp) {
        case NCX_BT_CONTAINER:
        case NCX_BT_CASE:
        case NCX_BT_LIST:
            ses_putstr(scb, STRUCT);
            break;
        case NCX_BT_CHOICE:
            ses_putstr(scb, UNION);
            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
            return;
        }
    }
    ses_putchar(scb, ' ');

    /* the first 'real' name has an underscore on the end */
    ses_putstr(scb, cdef->idstr);
    ses_putchar(scb, '_');

    /* start the guts of the typedef */
    ses_putstr(scb, START_BLOCK);

    /* generate a line for a Q header or a Queue */
    if (obj->objtype == OBJ_TYP_LIST || 
        obj->objtype == OBJ_TYP_LEAF_LIST) {
        ses_putstr(scb, QHEADER);
    }

    if (isleaflist) { 
        /* generate a line for the leaf-list data type */
        ses_indent(scb, ses_indent_count(scb));
        write_c_objtype(scb, obj);
    } else {
        /* generate a line for each child node */
        for (childobj = obj_first_child(obj);
             childobj != NULL;
             childobj = obj_next_child(childobj)) {

            if (!obj_has_name(obj) || 
                obj_is_cli(obj) ||
                !obj_is_enabled(obj) ||
                obj_is_abstract(obj)) {
                continue;
            }

            if (childobj->objtype == OBJ_TYP_LEAF_LIST) {
                ses_putstr(scb, START_LINE);
                ses_putstr(scb, QUEUE);
                ses_putchar(scb, ' ');
                ses_putstr(scb, cdef->valstr);
                ses_putchar(scb, ';');
            } else {
                ses_indent(scb, ses_indent_count(scb));
                write_c_objtype(scb, childobj);
            }
        }
    }

    ses_putstr(scb, END_BLOCK);
    ses_putchar(scb, ' ');
    ses_putstr(scb, cdef->idstr);
    ses_putchar(scb, ';');

}  /* write_h_object_typdef */


/********************************************************************
* FUNCTION write_h_objects
* 
* Generate the YANG for the specified datadefQ
*
* INPUTS:
*   scb == session control block to use for writing
*   datadefQ == Q of obj_template_t to use
*   typcdefQ == Q of typename binding c_define_t structs
*********************************************************************/
static void
    write_h_objects (ses_cb_t *scb,
                     dlq_hdr_t *datadefQ,
                     dlq_hdr_t *typcdefQ)
{
    obj_template_t    *obj;
    dlq_hdr_t         *childdatadefQ;
    boolean            dowrite;

    if (dlq_empty(datadefQ)) {
        return;
    }

    for (obj = (obj_template_t *)dlq_firstEntry(datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_has_name(obj) || 
            obj_is_cli(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        childdatadefQ = obj_get_datadefQ(obj);
        if (childdatadefQ) {
            write_h_objects(scb, childdatadefQ, typcdefQ);
        }

        dowrite = TRUE;

        switch (obj->objtype) {
        case OBJ_TYP_RPC:
            dowrite = FALSE;
            break;
        case OBJ_TYP_RPCIO:
        case OBJ_TYP_NOTIF:
        case OBJ_TYP_CONTAINER:
        case OBJ_TYP_LIST:
            if (obj_first_child(obj) == NULL) {
                dowrite = FALSE;
            }
            break;
        default:
            ;
        }
        if (dowrite) {
            write_h_object_typdef(scb, obj, typcdefQ);
        }
    }

}  /* write_h_objects */


/********************************************************************
* FUNCTION write_h_rpcs
* 
* Generate the H file decls for the RPC methods in the 
* specified datadefQ
*
* INPUTS:
*   scb == session control block to use for writing
*   datadefQ == que of obj_template_t to use
*
*********************************************************************/
static void
    write_h_rpcs (ses_cb_t *scb,
                  dlq_hdr_t *datadefQ)
{
    obj_template_t    *obj;
    /* dlq_hdr_t         *childdatadefQ; */



    (void)scb;


    if (dlq_empty(datadefQ)) {
        return;
    }

    for (obj = (obj_template_t *)dlq_firstEntry(datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_is_rpc(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        /*
        childdatadefQ = obj_get_datadefQ(obj);
        if (childdatadefQ) {
            write_h_objects(scb, childdatadefQ);
        }
        write_h_rpc(scb, obj);
        */
    }

}  /* write_h_rpcs */


/********************************************************************
* FUNCTION write_h_notif
* 
* Generate the H file decls for 1 notification
*
* INPUTS:
*   scb == session control block to use for writing
*   datadefQ == que of obj_template_t to use
*   cp == conversion parms to use
*********************************************************************/
static void
    write_h_notif (ses_cb_t *scb,
                   obj_template_t *notifobj,
                   const yangdump_cvtparms_t *cp)
{
    obj_template_t    *obj, *nextobj;

    ses_putstr(scb, (const xmlChar *)"\n/* send a ");
    write_identifier(scb,
                     obj_get_mod_name(notifobj),
                     NULL,
                     obj_get_name(notifobj));
    ses_putstr(scb, (const xmlChar *)" notification */");
    ses_putstr(scb, (const xmlChar *)"\nextern void");
    ses_indent(scb, cp->indent);
    write_identifier(scb,
                     obj_get_mod_name(notifobj),
                     NULL,
                     obj_get_name(notifobj));
    ses_putstr(scb, (const xmlChar *)"_send (");

    if (obj_has_children(notifobj)) {
        for (obj = obj_first_child(notifobj);
             obj != NULL;
             obj = nextobj) {

            nextobj = obj_next_child(obj);
            ses_indent(scb, cp->indent);
            write_c_objtype_ex(scb, 
                               obj,
                               (nextobj == NULL) ? ')' : ',',
                               TRUE);
        }
    } else {
        ses_putstr(scb, (const xmlChar *)"void)");
    }

    ses_putstr(scb, (const xmlChar *)";\n");

}  /* write_h_notif */


/********************************************************************
* FUNCTION write_h_notifs
* 
* Generate the H file decls for the notifications in the 
* specified datadefQ
*
* INPUTS:
*   scb == session control block to use for writing
*   datadefQ == que of obj_template_t to use
*   cp == conversion parms to use
*********************************************************************/
static void
    write_h_notifs (ses_cb_t *scb,
                    dlq_hdr_t *datadefQ,
                    const yangdump_cvtparms_t *cp)
{
    obj_template_t    *obj;
    boolean            first;

    first = TRUE;
    for (obj = (obj_template_t *)dlq_firstEntry(datadefQ);
         obj != NULL;
         obj = (obj_template_t *)dlq_nextEntry(obj)) {

        if (!obj_is_notif(obj) ||
            !obj_is_enabled(obj) ||
            obj_is_abstract(obj)) {
            continue;
        }

        if (first) {
            ses_putchar(scb, '\n');
            first = FALSE;
        }

        write_h_notif(scb, obj, cp);
    }

}  /* write_h_notifs */


/********************************************************************
* FUNCTION write_h_identity
* 
* Generate the #define for 1 identity
*
* INPUTS:
*   scb == session control block to use for writing
*   identity == ncx_identity_t to use
*
*********************************************************************/
static void
    write_h_identity (ses_cb_t *scb,
                      const ncx_identity_t *identity)
{
    /* define the identity constant */
    ses_putstr(scb, POUND_DEFINE);
    write_identifier(scb, 
                     identity->tkerr.mod->name,
                     BAR_ID,
                     identity->name);
    ses_putchar(scb, ' ');
    ses_putstr(scb, BAR_CONST);
    ses_putchar(scb, '"');
    ses_putstr(scb, identity->name);
    ses_putchar(scb, '"');

}  /* write_h_identity */


/********************************************************************
* FUNCTION write_h_identities
* 
* Generate the H file delcs for the specified identityQ
*
* INPUTS:
*   scb == session control block to use for writing
*   identityQ == que of ncx_identity_t to use
*
*********************************************************************/
static void
    write_h_identities (ses_cb_t *scb,
                        const dlq_hdr_t *identityQ)
{

    const ncx_identity_t *identity;

    if (dlq_empty(identityQ)) {
        return;
    }

    for (identity = (const ncx_identity_t *)
             dlq_firstEntry(identityQ);
         identity != NULL;
         identity = (const ncx_identity_t *)
             dlq_nextEntry(identity)) {

        write_h_identity(scb, identity);
    }
    ses_putchar(scb, '\n');

}  /* write_h_identities */


/********************************************************************
* FUNCTION write_h_oid
* 
* Generate the #define for 1 object node identifier
*
* INPUTS:
*   scb == session control block to use for writing
*   cdef == c_define_t struct to use
*
*********************************************************************/
static void
    write_h_oid (ses_cb_t *scb,
                 const c_define_t *cdef)
{
    /* define the identity constant */
    ses_putstr(scb, POUND_DEFINE);
    ses_putstr(scb, cdef->idstr);
    ses_putchar(scb, ' ');
    ses_putstr(scb, BAR_CONST);
    write_c_str(scb, cdef->valstr, 2);

}  /* write_h_oid */


/********************************************************************
* FUNCTION write_h_oids
* 
* Generate the #defines for the specified #define statements
* for name identifier strings
*
* INPUTS:
*   scb == session control block to use for writing
*   cdefineQ == que of c_define_t structs to use
*
*********************************************************************/
static void
    write_h_oids (ses_cb_t *scb,
                  const dlq_hdr_t *cdefineQ)
{
    const c_define_t     *cdef;

    for (cdef = (const c_define_t *)dlq_firstEntry(cdefineQ);
         cdef != NULL;
         cdef = (const c_define_t *)dlq_nextEntry(cdef)) {

        write_h_oid(scb, cdef);
    }

}  /* write_h_oids */


/********************************************************************
* FUNCTION save_h_oids
* 
* Save the identity to name bindings for the object names
*
* INPUTS:
*   scb == session control block to use for writing
*   cdefineQ == que of c_define_t structs to use
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    save_h_oids (ses_cb_t *scb,
                 const dlq_hdr_t *datadefQ,
                 dlq_hdr_t *cdefineQ)
{
    const dlq_hdr_t      *childQ;
    const obj_template_t *obj;
    status_t              res;

    if (dlq_empty(datadefQ)) {
	return NO_ERR;
    }

    for (obj = (const obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (const obj_template_t *)dlq_nextEntry(obj)) {

	if (obj_has_name(obj) && 
            obj_is_enabled(obj) &&
            !obj_is_cli(obj) &&
            !obj_is_abstract(obj)) {

            if (obj->objtype != OBJ_TYP_RPCIO) {
                res = save_oid_cdefine(cdefineQ,
                                       obj_get_mod_name(obj),
                                       obj_get_name(obj));
                if (res != NO_ERR) {
                    return res;
                }
            }

	    childQ = obj_get_cdatadefQ(obj);
	    if (childQ) {
		res = save_h_oids(scb, childQ, cdefineQ);
                if (res != NO_ERR) {
                    return res;
                }
	    }
	}
    }

    return NO_ERR;

}  /* save_h_oids */


/********************************************************************
* FUNCTION write_h_feature
* 
* Generate the #define for 1 feature statement
*
* INPUTS:
*   scb == session control block to use for writing
*   feature == ncx_feature_t to use
*
*********************************************************************/
static void
    write_h_feature (ses_cb_t *scb,
                     const ncx_feature_t *feature)
{

#ifdef NOT_IMPLEMENTED_YET
    if (!feature->enabled) {
        return;
    }
#endif

    write_h_iffeature_start(scb, &feature->iffeatureQ);

    /* define feature constant */
    ses_putstr(scb, POUND_DEFINE);
    write_identifier(scb, 
                     feature->tkerr.mod->name,
                     BAR_FEAT,
                     feature->name);
    ses_putchar(scb, ' ');
    ses_putchar(scb, '1');

    write_h_iffeature_end(scb, &feature->iffeatureQ);
    
}  /* write_h_feature */


/********************************************************************
* FUNCTION write_h_features
* 
* Generate the H file decls for the specified featureQ
*
* INPUTS:
*   scb == session control block to use for writing
*   featureQ == que of ncx_feature_t to use
*
*********************************************************************/
static void
    write_h_features (ses_cb_t *scb,
                      const dlq_hdr_t *featureQ)
{
    const ncx_feature_t *feature;

    if (dlq_empty(featureQ)) {
        return;
    }

    for (feature = (const ncx_feature_t *)
             dlq_firstEntry(featureQ);
         feature != NULL;
         feature = (const ncx_feature_t *)
             dlq_nextEntry(feature)) {

        write_h_feature(scb, feature);
    }
    ses_putchar(scb, '\n');

}  /* write_h_features */


/********************************************************************
* FUNCTION write_h_includes
* 
* Write the H file #include statements
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*
*********************************************************************/
static void
    write_h_includes (ses_cb_t *scb,
                      const ncx_module_t *mod,
                      const yangdump_cvtparms_t *cp)
{
    const ncx_include_t      *inc;

#ifdef LEAVE_OUT_FOR_NOW
    boolean                   needrpc, neednotif;


    needrpc = need_rpc_includes(mod, cp);
    neednotif = need_notif_includes(mod, cp);
#endif

    /* add xmlChar include */
    write_ext_include(scb, (const xmlChar *)"xmlstring.h");

#ifdef LEAVE_OUT_FOR_NOW
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

    write_ncx_include(scb, (const xmlChar *)"agt_util");
#endif

    write_ncx_include(scb, (const xmlChar *)"dlq");
    write_ncx_include(scb, (const xmlChar *)"ncxtypes");

#ifdef LEAVE_OUT_FOR_NOW
    if (needrpc) {
        write_ncx_include(scb, (const xmlChar *)"rpc");
    }

    if (needrpc || neednotif) {
        write_ncx_include(scb, (const xmlChar *)"ses");
    }
#endif

    write_ncx_include(scb, (const xmlChar *)"status");

#ifdef LEAVE_OUT_FOR_NOW
    if (needrpc || neednotif) {
        write_ncx_include(scb, (const xmlChar *)"val");
        write_ncx_include(scb, (const xmlChar *)"val_util");
    }
#endif


    /* includes for submodules */
    if (!cp->unified) {
        for (inc = (const ncx_include_t *)
                 dlq_firstEntry(&mod->includeQ);
             inc != NULL;
             inc = (const ncx_include_t *)dlq_nextEntry(inc)) {

            write_ncx_include(scb, inc->submodule);
        }
    }

} /* write_h_includes */


/********************************************************************
* FUNCTION write_h_file
* 
* Generate the module start and header
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
    write_h_file (ses_cb_t *scb,
                  ncx_module_t *mod,
                  const yangdump_cvtparms_t *cp)
{
    yang_node_t *node;
    int32        indent;
    dlq_hdr_t    cdefineQ, typenameQ;
    status_t     res;

    res = NO_ERR;
    indent = cp->indent;
    dlq_createSQue(&cdefineQ);
    dlq_createSQue(&typenameQ);

    /* Write the start of the H file */
    ses_putstr(scb, POUND_IFNDEF);
    ses_putstr(scb, BAR_H);
    write_c_safe_str(scb, mod->name);
    ses_putstr(scb, POUND_DEFINE);
    ses_putstr(scb, BAR_H);
    write_c_safe_str(scb, mod->name);

    write_c_header(scb, mod, cp);

    write_h_includes(scb, mod, cp);

    /* 1) features */
    write_h_features(scb, &mod->featureQ);
    if (cp->unified && mod->ismod) {
        for (node = (yang_node_t *)
                 dlq_firstEntry(&mod->saveincQ);
             node != NULL;
             node = (yang_node_t *)dlq_nextEntry(node)) {
            if (node->submod) {
                write_h_features(scb, &node->submod->featureQ);
            }
        }
    }

    /* 2) identities */
    write_h_identities(scb, &mod->identityQ);
    if (cp->unified && mod->ismod) {
        for (node = (yang_node_t *)
                 dlq_firstEntry(&mod->saveincQ);
             node != NULL;
             node = (yang_node_t *)dlq_nextEntry(node)) {
            if (node->submod) {
                write_h_identities(scb, &node->submod->identityQ);
            }
        }
    }

    /* 3) object node identifiers */
    res = save_h_oids(scb, &mod->datadefQ, &cdefineQ);
    if (res == NO_ERR) {
        if (cp->unified && mod->ismod) {
            for (node = (yang_node_t *)
                     dlq_firstEntry(&mod->saveincQ);
                 node != NULL && res == NO_ERR;
                 node = (yang_node_t *)dlq_nextEntry(node)) {
                if (node->submod) {
                    res = save_h_oids(scb, 
                                      &node->submod->datadefQ, 
                                      &cdefineQ);
                }
            }
        }
    }

    if (res == NO_ERR) {
        write_h_oids(scb, &cdefineQ);
    }

    /* 4) typedefs for objects */
    if (res == NO_ERR) {
        res = save_c_objects(mod, 
                             &mod->datadefQ, 
                             &typenameQ,
                             C_MODE_TYPEDEF);
        if (res == NO_ERR) {
            if (cp->unified && mod->ismod) {
                for (node = (yang_node_t *)
                         dlq_firstEntry(&mod->saveincQ);
                     node != NULL && res == NO_ERR;
                     node = (yang_node_t *)dlq_nextEntry(node)) {
                    if (node->submod) {
                        res = save_c_objects(node->submod,
                                             &node->submod->datadefQ, 
                                             &typenameQ,
                                             C_MODE_TYPEDEF);
                    }
                }
            }
        }
    }

    if (res == NO_ERR) {
        write_h_objects(scb, &mod->datadefQ, &typenameQ);
        if (cp->unified && mod->ismod) {
            for (node = (yang_node_t *)
                     dlq_firstEntry(&mod->saveincQ);
                 node != NULL;
                 node = (yang_node_t *)dlq_nextEntry(node)) {
                if (node->submod) {
                    write_h_objects(scb, &node->submod->datadefQ, &typenameQ);
                }
            }
        }
    }

    /* 5) typedefs and function prototypes for RPCs */
    if (res == NO_ERR) {
        write_h_rpcs(scb, &mod->datadefQ);
        if (cp->unified && mod->ismod) {
            for (node = (yang_node_t *)
                     dlq_firstEntry(&mod->saveincQ);
                 node != NULL;
                 node = (yang_node_t *)dlq_nextEntry(node)) {
                if (node->submod) {
                    write_h_rpcs(scb, &node->submod->datadefQ);
                }
            }
        }
    }

    /* 6) typedefs and function prototypes for notifications */
    if (res == NO_ERR) {
        write_h_notifs(scb, &mod->datadefQ, cp);
        if (cp->unified && mod->ismod) {
            for (node = (yang_node_t *)
                     dlq_firstEntry(&mod->saveincQ);
                 node != NULL;
                 node = (yang_node_t *)dlq_nextEntry(node)) {
                if (node->submod) {
                    write_h_notifs(scb, &node->submod->datadefQ, cp);
                }
            }
        }
    }

    /* 5) init and cleanup functions */
    if (mod->ismod && res == NO_ERR) {
        /* extern status_t y_<module>_init (void); */
        ses_putstr(scb, START_COMMENT);
        ses_putstr(scb, mod->name);
        ses_putstr(scb, (const xmlChar *)" module init 1");
        ses_putstr(scb, END_COMMENT);
        ses_putstr(scb, (const xmlChar *)"\nextern status_t");
        ses_indent(scb, indent);
        write_identifier(scb,
                         mod->name,
                         NULL,
                         (const xmlChar *)"init");
        ses_putstr(scb, (const xmlChar *)" (void);");

        /* extern status_t y_<module>_init2 (void); */
        ses_putstr(scb, START_COMMENT);
        ses_putstr(scb, mod->name);
        ses_putstr(scb, (const xmlChar *)" module init 2");
        ses_putstr(scb, END_COMMENT);
        ses_putstr(scb, (const xmlChar *)"\nextern status_t");
        ses_indent(scb, indent);
        write_identifier(scb,
                         mod->name,
                         NULL,
                         (const xmlChar *)"init2");
        ses_putstr(scb, (const xmlChar *)" (void);");

        /* extern void y_<module>_cleanup (void); */
        ses_putstr(scb, START_COMMENT);
        ses_putstr(scb, mod->name);
        ses_putstr(scb, (const xmlChar *)" module cleanup");
        ses_putstr(scb, END_COMMENT);
        ses_putstr(scb, (const xmlChar *)"\nextern void");
        ses_indent(scb, indent);
        write_identifier(scb,
                         mod->name,
                         NULL,
                         (const xmlChar *)"cleanup");
        ses_putstr(scb, (const xmlChar *)" (void);");
    }

    /* Write the end of the H file */
    ses_putchar(scb, '\n');
    ses_putstr(scb, POUND_ENDIF);

    clean_cdefineQ(&cdefineQ);
    clean_cdefineQ(&typenameQ);

    return res;

} /* write_h_file */


/*********     E X P O R T E D   F U N C T I O N S    **************/


/********************************************************************
* FUNCTION h_convert_module
* 
*
* INPUTS:
*   pcb == parser control block of module to convert
*          This is returned from ncxmod_load_module_xsd
*   cp == conversion parms to use
*   scb == session control block for writing output
*
* RETURNS:
*   status
*********************************************************************/
status_t
    h_convert_module (yang_pcb_t *pcb,
                      const yangdump_cvtparms_t *cp,
                      ses_cb_t *scb)
{
    ncx_module_t  *mod;
    status_t       res;

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
        return SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
    }

    res = write_h_file(scb, mod, cp);

    return res;

}   /* h_convert_module */

/* END file h.c */
