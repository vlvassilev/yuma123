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
28marr09      abb      begun

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


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define START_COMMENT (const xmlChar *)"\n\n/* "
#define END_COMMENT   (const xmlChar *)" */"

#define START_BLOCK  (const xmlChar *)" {"
#define END_BLOCK    (const xmlChar *)"\n}"

#define BAR_H         (const xmlChar *)"_H_"
#define DOT_H         (const xmlChar *)".h"
#define BAR_FEAT      (const xmlChar *)"F"
#define BAR_ID        (const xmlChar *)"I"
#define BAR_NODE      (const xmlChar *)"N"
#define DEF_TYPE      (const xmlChar *)"T"

#define BAR_CONST     (const xmlChar *)"(const xmlChar *)"

#define POUND_DEFINE  (const xmlChar *)"\n#define "
#define POUND_ENDIF   (const xmlChar *)"\n#endif"
#define POUND_IF      (const xmlChar *)"\n#if "
#define POUND_IFDEF   (const xmlChar *)"\n#ifdef "
#define POUND_IFNDEF  (const xmlChar *)"\n#ifndef "
#define POUND_INCLUDE (const xmlChar *)"\n#include "

#define START_DEFINED (const xmlChar *)"defined("
#define START_TYPEDEF (const xmlChar *)"\ntypedef "


#define INT8          (const xmlChar *)"int8"
#define INT16         (const xmlChar *)"int16"
#define INT32         (const xmlChar *)"int32"
#define INT64         (const xmlChar *)"int64"

#define UINT8         (const xmlChar *)"uint8"
#define UINT16        (const xmlChar *)"uint16"
#define UINT32        (const xmlChar *)"uint32"
#define UINT64        (const xmlChar *)"uint64"

#define STRING        (const xmlChar *)"xmlChar *"

#define BOOLEAN       (const xmlChar *)"boolean"
#define FLOAT         (const xmlChar *)"float"
#define DOUBLE        (const xmlChar *)"double"

#define STRUCT        (const xmlChar *)"struct"
#define UNION         (const xmlChar *)"union"

#define QHEADER       (const xmlChar *)"\n    dlq_hdr_t qhdr;"

#define QUEUE         (const xmlChar *)"dlq_hdr_t"

#define START_LINE    (const xmlChar *)"\n    "

/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION write_h_safe_str
* 
* Generate a string token at the current line location
*
* INPUTS:
*   scb == session control block to use for writing
*   strval == string value
*********************************************************************/
static void
    write_h_safe_str (ses_cb_t *scb,
		      const xmlChar *strval)
{
    const xmlChar *s;

    s = strval;
    while (*s) {
	if (*s == '.' || *s == '-') {
	    ses_putchar(scb, '_');
	} else {
	    ses_putchar(scb, *s);
	}
	s++;
    }

}  /* write_h_safe_str */


/********************************************************************
* FUNCTION write_h_str
* 
* Generate a string token at the current line location
*
* INPUTS:
*   scb == session control block to use for writing
*   strval == string value
*   quotes == quotes style (0, 1, 2)
*********************************************************************/
static void
    write_h_str (ses_cb_t *scb,
		 const xmlChar *strval,
		 uint32 quotes)
{
    switch (quotes) {
    case 1:
	ses_putchar(scb, '\'');
	break;
    case 2:
	ses_putchar(scb, '"');
	break;
    default:
	;
    }

    ses_putstr(scb, strval);

    switch (quotes) {
    case 1:
	ses_putchar(scb, '\'');
	break;
    case 2:
	ses_putchar(scb, '"');
	break;
    default:
	;
    }

}  /* write_h_str */


/********************************************************************
* FUNCTION write_h_simple_str
* 
* Generate a simple clause on 1 line
*
* INPUTS:
*   scb == session control block to use for writing
*   kwname == keyword name
*   strval == string value
*   indent == indent count to use
*   quotes == quotes style (0, 1, 2)
*********************************************************************/
static void
    write_h_simple_str (ses_cb_t *scb,
			const xmlChar *kwname,
			const xmlChar *strval,
			int32 indent,
			uint32 quotes)
{
    ses_putstr_indent(scb, kwname, indent);
    if (strval) {
	ses_putchar(scb, ' ');
	write_h_str(scb, strval, quotes);
    }

}  /* write_h_simple_str */


/********************************************************************
 *
* FUNCTION write_identifier
* 
* Generate an identifier
*
*  #module_DEFTYPE_idname
*
* INPUTS:
*   scb == session control block to use for writing
*   modname == module name start-string to use
*   defpart == string for deftype part
*   idname == identifier name
*
*********************************************************************/
static void
    write_identifier (ses_cb_t *scb,
		      const xmlChar *modname,
		      const xmlChar *defpart,
		      const xmlChar *idname)
{
    write_h_safe_str(scb, modname);
    ses_putchar(scb, '_');
    ses_putstr(scb, defpart);
    ses_putchar(scb, '_');
    write_h_safe_str(scb, idname);

}  /* write_identifier */


/********************************************************************
* FUNCTION write_h_iffeature_start
* 
* Generate the C for 1 if-feature conditiona;
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


/*******************************************************************
* FUNCTION write_h_objtype
* 
* Generate the C definitions for 1 datadef child type line
*
* INPUTS:
*   scb == session control block to use for writing
*   obj == obj_template_t to use
*
**********************************************************************/
static void
    write_h_objtype (ses_cb_t *scb,
		     const obj_template_t *obj)

{
    ncx_btype_t    btyp;
    boolean        needspace;

    needspace = TRUE;

    ses_putstr(scb, START_LINE);
    btyp = obj_get_basetype(obj);

    switch (btyp) {
    case NCX_BT_BOOLEAN:
	ses_putstr(scb, BOOLEAN);
	break;
    case NCX_BT_INT8:
	ses_putstr(scb, INT8);
	break;
    case NCX_BT_INT16:
	ses_putstr(scb, INT16);
	break;
    case NCX_BT_INT32:
	ses_putstr(scb, INT32);
	break;
    case NCX_BT_INT64:
	ses_putstr(scb, INT64);
	break;
    case NCX_BT_UINT8:
	ses_putstr(scb, UINT8);
	break;
    case NCX_BT_UINT16:
	ses_putstr(scb, UINT16);
	break;
    case NCX_BT_UINT32:
	ses_putstr(scb, UINT32);
	break;
    case NCX_BT_UINT64:
	ses_putstr(scb, UINT64);
	break;
    case NCX_BT_DECIMAL64:
	ses_putstr(scb, INT64);
	break;
    case NCX_BT_FLOAT64:
	ses_putstr(scb, DOUBLE);
	break;
    case NCX_BT_ENUM:
    case NCX_BT_STRING:
    case NCX_BT_BINARY:
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_LEAFREF:
    case NCX_BT_IDREF:
    case NCX_BT_SLIST:
	ses_putstr(scb, STRING);
	needspace = FALSE;
	break;
    case NCX_BT_LIST:
	ses_putstr(scb, QUEUE);
	break;
    default:
	/* assume complex type */
	write_identifier(scb,
			 obj_get_mod_name(obj),
			 DEF_TYPE,
			 obj_get_name(obj));
    }

    if (needspace) {
	ses_putchar(scb, ' ');
    }

    write_h_safe_str(scb, obj_get_name(obj));
    ses_putchar(scb, ';');

}  /* write_h_objtype */


/********************************************************************
* FUNCTION write_h_object
* 
* Generate the C definitions for 1 datadef
*
* INPUTS:
*   scb == session control block to use for writing
*   obj == obj_template_t to use
*
**********************************************************************/
static void
    write_h_object (ses_cb_t *scb,
		    obj_template_t *obj)

{
    obj_template_t          *childobj;
    xmlChar                 *buffer;
    ncx_btype_t              btyp;
    status_t                 res;
    boolean                  isleaflist;

    isleaflist = FALSE;

    /* typedefs not needed for simple objects */
    if (obj->objtype == OBJ_TYP_LEAF ||
        obj->objtype == OBJ_TYP_ANYXML) {
	return;
    } else if (obj->objtype == OBJ_TYP_LEAF_LIST) {
	isleaflist = TRUE;
    }

    btyp = obj_get_basetype(obj);

    /* generate the YOID as a comment */
    res = obj_gen_object_id(obj, &buffer);
    if (res != NO_ERR) {
	SET_ERROR(res);
	return;
    }
    ses_putstr(scb, START_COMMENT);
    ses_putstr(scb, obj_get_typestr(obj));
    ses_putchar(scb, ' ');
    ses_putstr(scb, buffer);
    ses_putstr(scb, END_COMMENT);
    m__free(buffer);

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
    write_identifier(scb,
		     obj_get_mod_name(obj),
		     DEF_TYPE,
		     obj_get_name(obj));
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
	write_h_objtype(scb, obj);
    } else {
	/* generate a line for each child node */
	for (childobj = obj_first_child(obj);
	     childobj != NULL;
	     childobj = obj_next_child(childobj)) {

	    if (!obj_has_name(childobj) ||
		!obj_is_data_db(childobj)) {
		continue;
	    }
	    if (childobj->objtype == OBJ_TYP_LEAF_LIST) {
		ses_putstr(scb, START_LINE);
		ses_putstr(scb, QUEUE);
		ses_putchar(scb, ' ');
		write_h_safe_str(scb, obj_get_name(childobj));
		ses_putchar(scb, ';');
	    } else {
		write_h_objtype(scb, childobj);
	    }
	}
    }

    ses_putstr(scb, END_BLOCK);
    ses_putchar(scb, ' ');
    write_identifier(scb,
		     obj_get_mod_name(obj),
		     DEF_TYPE,
		     obj_get_name(obj));
    ses_putchar(scb, ';');

}  /* write_h_object */


/********************************************************************
* FUNCTION write_h_objects
* 
* Generate the YANG for the specified datadefQ
*
* INPUTS:
*   scb == session control block to use for writing
*   datadefQ == que of obj_template_t to use
*
*********************************************************************/
static void
    write_h_objects (ses_cb_t *scb,
		     dlq_hdr_t *datadefQ)
{
    obj_template_t    *obj;
    dlq_hdr_t         *childdatadefQ;

    if (dlq_empty(datadefQ)) {
	return;
    }

    for (obj = (obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (obj_template_t *)dlq_nextEntry(obj)) {

	if (!obj_has_name(obj) || !obj_is_data_db(obj)) {
	    continue;
	}

	childdatadefQ = obj_get_datadefQ(obj);
	if (childdatadefQ) {
	    write_h_objects(scb, childdatadefQ);
	}
	write_h_object(scb, obj);
    }

}  /* write_h_objects */


/********************************************************************
* FUNCTION write_h_identity
* 
* Generate the C definition for 1 identity
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
* Generate the YANG for the specified identityQ
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
* Generate the C definition for 1 object node identifier
*
* INPUTS:
*   scb == session control block to use for writing
*   obj == object template to use
*
*********************************************************************/
static void
    write_h_oid (ses_cb_t *scb,
		 const obj_template_t *obj)
{
    /* define the identity constant */
    ses_putstr(scb, POUND_DEFINE);
    write_identifier(scb, 
		     obj_get_mod_name(obj),
		     BAR_NODE,
		     obj_get_name(obj));
    ses_putchar(scb, ' ');
    ses_putstr(scb, BAR_CONST);
    write_h_str(scb, obj_get_name(obj), 2);

}  /* write_h_oid */


/********************************************************************
* FUNCTION write_h_oids
* 
* Generate the YANG for the specified identityQ
*
* INPUTS:
*   scb == session control block to use for writing
*   identityQ == que of ncx_identity_t to use
*
*********************************************************************/
static void
    write_h_oids (ses_cb_t *scb,
		  const dlq_hdr_t *datadefQ)
{
    const dlq_hdr_t      *childQ;
    const obj_template_t *obj;

    if (dlq_empty(datadefQ)) {
	return;
    }

    for (obj = (const obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (const obj_template_t *)dlq_nextEntry(obj)) {

	if (obj_has_name(obj) && obj_is_data_db(obj)) {
	    write_h_oid(scb, obj);

	    childQ = obj_get_cdatadefQ(obj);
	    if (childQ) {
		write_h_oids(scb, childQ);
	    }
	}
    }

}  /* write_h_oids */



/********************************************************************
* FUNCTION write_h_feature
* 
* Generate the C for 1 feature statement
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
* Generate the C definitions for the specified featureQ
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
* FUNCTION write_h_header
* 
* Write the H file header
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*
* OUTPUTS:
*  current indent count will be ses_indent_count(scb) upon exit
*********************************************************************/
static void
    write_h_header (ses_cb_t *scb,
			const ncx_module_t *mod,
			const yangdump_cvtparms_t *cp)
{
    const ncx_include_t      *inc;
    status_t                  res;
    xmlChar                   buffer[NCX_VERSION_BUFFSIZE];


    /* banner comments */
    ses_putstr(scb, START_COMMENT);    
    ses_putchar(scb, '\n');

    /* generater tag */
    ses_putstr(scb, (const xmlChar *)
	       "\n  Generated by yangdump version ");
    res = ncx_get_version(buffer, NCX_VERSION_BUFFSIZE);
    if (res == NO_ERR) {
        ses_putstr(scb, buffer);
    } else {
        SET_ERROR(res);
    }
    ses_putchar(scb, '\n');

    /* copyright section */
    ses_putstr(scb, (const xmlChar *)
	       "\n  *** Put copyright info here ***\n");

    /* module name */
    if (mod->ismod) {
	ses_putstr_indent(scb, YANG_K_MODULE, NCX_DEF_INDENT);
    } else {
	ses_putstr_indent(scb, YANG_K_SUBMODULE, NCX_DEF_INDENT);
    }
    ses_putchar(scb, ' ');
    ses_putstr(scb, mod->name);

    /* version */
    if (mod->version) {
	write_h_simple_str(scb, 
			   YANG_K_REVISION,
			   mod->version, 
			   NCX_DEF_INDENT, 0);
	ses_putchar(scb, '\n');
    }

    /* namespace or belongs-to */
    if (mod->ismod) {
	write_h_simple_str(scb, 
			   YANG_K_NAMESPACE, 
			   mod->ns,
			   NCX_DEF_INDENT, 0);
    } else {
	write_h_simple_str(scb, 
			   YANG_K_BELONGS_TO, 
			   mod->belongs,
			   NCX_DEF_INDENT, 0);
    }

    /* organization */
    if (mod->organization) {
	write_h_simple_str(scb, 
			   YANG_K_ORGANIZATION,
			   mod->organization, 
			   NCX_DEF_INDENT, 0);
    }

    ses_putchar(scb, '\n');
    ses_putchar(scb, '\n');
    ses_putstr(scb, END_COMMENT);
    ses_putchar(scb, '\n');

    /* includes section	*/
    if (!cp->unified) {
	for (inc = (const ncx_include_t *)
		 dlq_firstEntry(&mod->includeQ);
	     inc != NULL;
	     inc = (const ncx_include_t *)dlq_nextEntry(inc)) {

	    ses_putstr(scb, POUND_IFNDEF);
	    ses_putstr(scb, BAR_H);
	    write_h_safe_str(scb, inc->submodule);

	    ses_putstr(scb, POUND_INCLUDE);
	    ses_putchar(scb, ' ');
	    ses_putchar(scb, '"');
	    write_h_safe_str(scb, inc->submodule);
	    ses_putstr(scb, DOT_H);
	    ses_putchar(scb, '"');
	    ses_putstr(scb, POUND_ENDIF);
	    ses_putchar(scb, '\n');
	}
    }

} /* write_h_header */


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
*********************************************************************/
static void
    write_h_file (ses_cb_t *scb,
		  ncx_module_t *mod,
		  const yangdump_cvtparms_t *cp)
{
    yang_node_t *node;

    /* Write the start of the H file */
    ses_putstr(scb, POUND_IFNDEF);
    ses_putstr(scb, BAR_H);
    write_h_safe_str(scb, mod->name);
    ses_putstr(scb, POUND_DEFINE);
    ses_putstr(scb, BAR_H);
    write_h_safe_str(scb, mod->name);

    write_h_header(scb, mod, cp);


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
    write_h_oids(scb, &mod->datadefQ);
    if (cp->unified && mod->ismod) {
	for (node = (yang_node_t *)
		 dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_h_oids(scb, &node->submod->datadefQ);
	    }
	}
    }

    /* 4) typedefs for objects */
    write_h_objects(scb, &mod->datadefQ);

    if (cp->unified && mod->ismod) {
	for (node = (yang_node_t *)
		 dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_h_objects(scb, &node->submod->datadefQ);
	    }
	}
    }

    /* Write the end of the H file */
    ses_putchar(scb, '\n');
    ses_putstr(scb, POUND_ENDIF);

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

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
	return SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
    }

    write_h_file(scb, mod, cp);

    return NO_ERR;

}   /* h_convert_module */


/* END file cyang.c */
