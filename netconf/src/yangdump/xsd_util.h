#ifndef _H_xsd_util
#define _H_xsd_util

/*  FILE: xsd_util.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Utility functions for converting NCX modules to XSD format
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
24-nov-06    abb      Begun; split from xsd.c

*/

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ext
#include "ext.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_yangdump
#include "yangdump.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

#define XSD_ABSTRACT       (const xmlChar *)"abstract"
#define XSD_AF_DEF         (const xmlChar *)"attributeFormDefault"
#define XSD_ANNOTATION     (const xmlChar *)"annotation"
#define XSD_ANY            (const xmlChar *)"any"
#define XSD_ANY_TYPE       (const xmlChar *)"anyType"
#define XSD_APP_SUFFIX     (const xmlChar *)"AppType"
#define XSD_ATTRIBUTE      (const xmlChar *)"attribute"
#define XSD_BANNER_0       (const xmlChar *)"Converted from NCX file '"
#define XSD_BANNER_0Y      (const xmlChar *)"Converted from YANG file '"
#define XSD_BANNER_0END    (const xmlChar *)"' by yangdump version "
#define XSD_BANNER_1       (const xmlChar *)"\n\nModule: "
#define XSD_BANNER_1S      (const xmlChar *)"\n\nSubmodule: "
#define XSD_BANNER_1B      (const xmlChar *)"\nBelongs to module: "
#define XSD_BANNER_2       (const xmlChar *)"\nOrganization: "
#define XSD_BANNER_3       (const xmlChar *)"\nVersion: "
#define XSD_BANNER_4       (const xmlChar *)"\nCopyright: "
#define XSD_BANNER_5       (const xmlChar *)"\nContact: "
#define XSD_BASE           (const xmlChar *)"base"
#define XSD_BASE64         (const xmlChar *)"base64Binary"
#define XSD_BT             (const xmlChar *)"BT"
#define XSD_CPX_CON        (const xmlChar *)"complexContent"
#define XSD_CPX_TYP        (const xmlChar *)"complexType"
#define XSD_DATA_INLINE    (const xmlChar *)"dataInlineType"
#define XSD_DOCUMENTATION  (const xmlChar *)"documentation"
#define XSD_DRAFT_URL   \
    (const xmlChar *)"http://www.ietf.org/internet-drafts/"
#define XSD_EF_DEF         (const xmlChar *)"elementFormDefault"
#define XSD_ELEMENT        (const xmlChar *)"element"
#define XSD_EN             (const xmlChar *)"en"
#define XSD_EXTENSION      (const xmlChar *)"extension"
#define XSD_ENUMERATION    (const xmlChar *)"enumeration"
#define XSD_FALSE          (const xmlChar *)"false"
#define XSD_FIELD          (const xmlChar *)"field"
#define XSD_FIXED          (const xmlChar *)"fixed"
#define XSD_GROUP          (const xmlChar *)"group"
#define XSD_HEX_BINARY     (const xmlChar *)"hexBinary"
#define XSD_ITEMTYPE       (const xmlChar *)"itemType"
#define XSD_KEY            (const xmlChar *)"key"
#define XSD_KEY_SUFFIX     (const xmlChar *)"_Key"
#define XSD_LANG           (const xmlChar *)"xml:lang"
#define XSD_LAX            (const xmlChar *)"lax"
#define XSD_LENGTH         (const xmlChar *)"length"
#define XSD_LIST           (const xmlChar *)"list"
#define XSD_LOC            (const xmlChar *)"schemaLocation"
#define XSD_MEMBERTYPES    (const xmlChar *)"memberTypes"
#define XSD_MAX_INCL       (const xmlChar *)"maxInclusive"
#define XSD_MAX_LEN        (const xmlChar *)"maxLength"
#define XSD_MAX_OCCURS     (const xmlChar *)"maxOccurs"
#define XSD_MIN_INCL       (const xmlChar *)"minInclusive"
#define XSD_MIN_LEN        (const xmlChar *)"minLength"
#define XSD_MIN_OCCURS     (const xmlChar *)"minOccurs"
#define XSD_NOTIF_CONTENT  (const xmlChar *)"notificationContent"
#define XSD_NOTIF_CTYPE    (const xmlChar *)"NotificationContentType"
#define XSD_OPTIONAL       (const xmlChar *)"optional"
#define XSD_OTHER          (const xmlChar *)"##other"
#define XSD_OUTPUT_TYPEEXT (const xmlChar *)"_output_type__"
#define XSD_PROC_CONTENTS  (const xmlChar *)"processContents"
#define XSD_PS_SUFFIX      (const xmlChar *)"PSType"
#define XSD_QUAL           (const xmlChar *)"qualified"
#define XSD_REQUIRED       (const xmlChar *)"required"
#define XSD_REF            (const xmlChar *)"ref"
#define XSD_RFC_URL        (const xmlChar *)"http://www.ietf.org/rfc/rfc"
#define XSD_RPC_OP         (const xmlChar *)"rpcOperation"
#define XSD_RPC_OPTYPE     (const xmlChar *)"rpcOperationType"
#define XSD_RESTRICTION    (const xmlChar *)"restriction"
#define XSD_SCHEMA         (const xmlChar *)"schema"
#define XSD_SELECTOR       (const xmlChar *)"selector"
#define XSD_SEQUENCE       (const xmlChar *)"sequence"
#define XSD_SIM_CON        (const xmlChar *)"simpleContent"
#define XSD_SIM_TYP        (const xmlChar *)"simpleType"
#define XSD_SUB_GRP        (const xmlChar *)"substitutionGroup"
#define XSD_TARG_NS        (const xmlChar *)"targetNamespace"
#define XSD_TRUE           (const xmlChar *)"true"
#define XSD_UNBOUNDED      (const xmlChar *)"unbounded"
#define XSD_UNION          (const xmlChar *)"union"
#define XSD_UNIQUE         (const xmlChar *)"unique"
#define XSD_UNIQUE_SUFFIX  (const xmlChar *)"_Unique"
#define XSD_UNQUAL         (const xmlChar *)"unqualified"
#define XSD_USE            (const xmlChar *)"use"
#define XSD_VALUE          (const xmlChar *)"value"
#define XSD_ZERO           (const xmlChar *)"0"


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/
typedef struct xsd_keychain_t_ {
    dlq_hdr_t    qhdr;
    val_value_t *val;
} xsd_keychain_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* Build output value: add child node to a struct node */

extern const xmlChar *
    xsd_typename (ncx_btype_t btyp);

extern xmlChar *
    xsd_make_basename (const typ_template_t *typ);


extern val_value_t *
    xsd_make_enum_appinfo (int32 enuval,
			   const dlq_hdr_t  *appinfoQ,
			   ncx_status_t status);

extern val_value_t *
    xsd_make_bit_appinfo (uint32 bitpos,
			  const dlq_hdr_t  *appinfoQ,
			  ncx_status_t status);

extern val_value_t *
    xsd_make_err_appinfo (const xmlChar *ref,
			  const xmlChar *errmsg,
			  const xmlChar *errtag);

extern xmlChar *
    xsd_make_schema_location (const ncx_module_t *mod,
			      const xmlChar *schemaloc,
			      boolean versionnames);

extern xmlChar *
    xsd_make_output_filename (const ncx_module_t *mod,
			      const yangdump_cvtparms_t *cp);

extern status_t
    xsd_do_documentation (const xmlChar *descr,
			  val_value_t *val);

extern status_t
    xsd_do_reference (const xmlChar *ref,
		      val_value_t *val);

extern status_t
    xsd_add_mod_documentation (const ncx_module_t *mod,
			       val_value_t *annot);

extern status_t
    xsd_add_imports (const ncx_module_t *mod,
		     const yangdump_cvtparms_t *cp,
		     val_value_t *val);

extern status_t
    xsd_add_includes (const ncx_module_t *mod,
		      const yangdump_cvtparms_t *cp,
		      val_value_t *val);

extern val_value_t *
    xsd_new_element (const ncx_module_t *mod,
		     const xmlChar *name,
		     const typ_def_t *typdef,
		     const typ_def_t *parent,
		     boolean hasnodes,
		     boolean hasindex);

extern val_value_t *
    xsd_new_leaf_element (const ncx_module_t *mod,
			  const obj_template_t *obj,
			  boolean hasnodes,
			  boolean addtype,
			  boolean iskey);

extern status_t
    xsd_add_parmtype_attr (xmlns_id_t targns,
			   const typ_template_t *typ,
			   val_value_t *val);

extern status_t
    xsd_add_key (const xmlChar *name,
		 const typ_def_t *typdef,
		 val_value_t *val);

extern status_t
    xsd_do_annotation (const xmlChar *descr,
		       const xmlChar *condition,
		       const xmlChar *units,
		       ncx_access_t maxacc,
		       ncx_status_t status,
		       const dlq_hdr_t *appinfoQ,
		       val_value_t  *val);


extern status_t
    xsd_do_type_annotation (const typ_template_t *typ,
			    val_value_t *val);

extern val_value_t *
    xsd_make_obj_annotation (const obj_template_t *obj,
			     status_t  *res);

extern val_value_t *
    xsd_make_group_annotation (const grp_template_t *grp,
			       status_t  *res);

#if 0
extern status_t
    xsd_add_any (val_value_t *val);
#endif

extern status_t
    xsd_add_aughook (val_value_t *val,
		     const obj_template_t *obj);

extern xmlChar *
    xsd_make_rpc_output_typename (const obj_template_t *obj);


extern status_t
    xsd_add_type_attr (const ncx_module_t *mod,
		       const typ_def_t *typdef,
		       val_value_t *val);

extern xmlChar *
    xsd_make_default_filename (const ncx_module_t *mod);

#endif	    /* _H_xsd_util */
