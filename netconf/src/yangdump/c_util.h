#ifndef _H_c_util
#define _H_c_util

/*  FILE: c_util.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

  YANGDUMP C code generation utilities
 
*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
24-oct-09    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_yangdump
#include "yangdump.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
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

#define IDREF         (const xmlChar *)"val_idref_t *"

#define BOOLEAN       (const xmlChar *)"boolean"
#define FLOAT         (const xmlChar *)"float"
#define DOUBLE        (const xmlChar *)"double"

#define STRUCT        (const xmlChar *)"struct"
#define UNION         (const xmlChar *)"union"

#define QHEADER       (const xmlChar *)"\n    dlq_hdr_t qhdr;"

#define QUEUE         (const xmlChar *)"dlq_hdr_t"

#define START_LINE    (const xmlChar *)"\n    "

#define Y_PREFIX      (const xmlChar *)"y_"

#define EDIT_SUFFIX   (const xmlChar *)"_edit"
#define GET_SUFFIX    (const xmlChar *)"_get"
#define MRO_SUFFIX    (const xmlChar *)"_mro"

#define FN_BANNER_START (const xmlChar *)\
    "\n\n/********************************************************************\n* FUNCTION "

#define FN_BANNER_LN (const xmlChar *)"\n* "

#define FN_BANNER_INPUT (const xmlChar *)"\n* INPUTS:\n* "

#define FN_BANNER_RETURN (const xmlChar *)"\n* RETURNS:\n* "

#define FN_BANNER_RETURN_STATUS (const xmlChar *)\
    "\n* RETURNS:\n*     error status"

#define FN_BANNER_END (const xmlChar *)\
    "\n********************************************************************/"

/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/

typedef enum c_mode_t_ {
    C_MODE_NONE,
    C_MODE_OID,
    C_MODE_TYPEDEF,
    C_MODE_CALLBACK
} c_mode_t;

/* ID to name string binding for #define statements */
typedef struct c_define_t_ {
    dlq_hdr_t        qhdr;
    xmlChar         *idstr;
    xmlChar         *valstr;
    obj_template_t  *obj;   /* back-ptr to object for typdef */
} c_define_t;


/********************************************************************
*                                                                   *
*                         F U N C T I O N S                         *
*                                                                   *
*********************************************************************/

extern boolean
    need_rpc_includes (const ncx_module_t *mod,
                       const yangdump_cvtparms_t *cp);

extern boolean
    need_notif_includes (const ncx_module_t *mod,
                         const yangdump_cvtparms_t *cp);

extern void
    write_c_safe_str (ses_cb_t *scb,
                      const xmlChar *strval);

extern uint32
    copy_c_safe_str (xmlChar *buffer,
                     const xmlChar *strval);

extern void
    write_c_str (ses_cb_t *scb,
                 const xmlChar *strval,
                 uint32 quotes);

extern void
    write_c_simple_str (ses_cb_t *scb,
                        const xmlChar *kwname,
                        const xmlChar *strval,
                        int32 indent,
                        uint32 quotes);

extern void
    write_identifier (ses_cb_t *scb,
                      const xmlChar *modname,
                      const xmlChar *defpart,
                      const xmlChar *idname);

extern void
    write_ext_include (ses_cb_t *scb,
                       const xmlChar *hfile);

extern void
    write_ncx_include (ses_cb_t *scb,
                       const xmlChar *modname);

extern status_t
    save_oid_cdefine (dlq_hdr_t *cdefineQ,
                      const xmlChar *modname,
                      const xmlChar *defname);

extern status_t
    save_path_cdefine (dlq_hdr_t *cdefineQ,
                       const xmlChar *modname,
                       obj_template_t *obj,
                       c_mode_t cmode);

extern c_define_t *
    find_path_cdefine (dlq_hdr_t *cdefineQ,
                       obj_template_t *obj);

extern void
    clean_cdefineQ (dlq_hdr_t *cdefineQ);

extern void
    write_c_header (ses_cb_t *scb,
                    const ncx_module_t *mod,
                    const yangdump_cvtparms_t *cp);

extern void
    write_c_footer (ses_cb_t *scb,
                    const ncx_module_t *mod);

extern void
    write_c_objtype (ses_cb_t *scb,
                     const obj_template_t *obj);

extern void
    write_c_objtype_ex (ses_cb_t *scb,
                        const obj_template_t *obj,
                        xmlChar endchar,
                        boolean isconst);

extern void
    write_c_val_macro_type (ses_cb_t *scb,
                            const obj_template_t *obj);

extern void
    write_c_oid_comment (ses_cb_t *scb,
                         const obj_template_t *obj);

extern status_t
    save_c_objects (ncx_module_t *mod,
                    dlq_hdr_t *datadefQ,
                    dlq_hdr_t *savecdefQ,
                    c_mode_t cmode);

#endif            /* _H_c_util */
