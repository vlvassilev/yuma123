/*  FILE: cyang.c

    Generate YANG file output in canonical format

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
08apr08      abb      begun; started from html.c

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

#ifndef _H_cyang
#include "cyang.h"
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


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define START_SEC         (const xmlChar *)" {"

#define END_SEC           (const xmlChar *)"}"

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

static void
    write_cyang_groupings (ses_cb_t *scb,
			   const ncx_module_t *mod,
			   const yangdump_cvtparms_t *cp,
			   const dlq_hdr_t *groupingQ,
			   int32 startindent);

static void
    write_cyang_type_clause (ses_cb_t *scb,
			     const ncx_module_t *mod,
			     const yangdump_cvtparms_t *cp,
			     const typ_def_t *typdef,
			     int32 startindent);

static void
    write_cyang_objects (ses_cb_t *scb,
			 const ncx_module_t *mod,
			 const yangdump_cvtparms_t *cp,
			 const dlq_hdr_t *datadefQ,
			 int32 startindent);


/********************************************************************
* FUNCTION write_cyang_extkw
* 
* Generate a language extension keyword at the current line location
*
* INPUTS:
*   scb == session control block to use for writing
*   kwpfix == keyword prefix to use
*   kwname == keyword name
*
*********************************************************************/
static void
    write_cyang_extkw (ses_cb_t *scb,
		       const xmlChar *kwpfix,
		       const xmlChar *kwname)
{
    ses_putstr(scb, kwpfix);
    ses_putchar(scb, ':');
    ses_putstr(scb, kwname);

}  /* write_cyang_extkw */


/********************************************************************
* FUNCTION write_cyang_banner_cmt
* 
* Generate a comment at the current location for a section banner
* Check that only the first one is printed
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   cmval == comment value string
*   indent == current indent count
*********************************************************************/
static void
    write_cyang_banner_cmt (ses_cb_t *scb,
			    const ncx_module_t *mod,
			    const yangdump_cvtparms_t *cp,
			    const xmlChar *cmval,
			    int32 indent)
{
    boolean needed;

    needed = FALSE;

    if (cp->unified) {
	if (mod->ismod) {
	    needed = TRUE;
	}
    } else {
	needed = TRUE;
    }
    if (needed) {
	ses_putchar(scb, '\n');
	ses_indent(scb, indent);
	ses_putstr(scb, (const xmlChar *)"// ");
	ses_putstr(scb, cmval);
    }

}  /* write_cyang_banner_cmt */


/********************************************************************
* FUNCTION write_cyang_endsec_cmt
* 
* Generate a comment at the current location for an end of a section
* No comment tokens are given
*
* INPUTS:
*   scb == session control block to use for writing
*   cmtype == comment type string
*   cmname == comment value string
*
*********************************************************************/
static void
    write_cyang_endsec_cmt (ses_cb_t *scb,
			    const xmlChar *cmtype,
			    const xmlChar *cmname)
{
    ses_putstr(scb, (const xmlChar *)" // ");
    if (cmtype) {
	ses_putstr(scb, cmtype);
	ses_putchar(scb, ' ');
    }
    if (cmname) {
	ses_putstr(scb, cmname);
    }

}  /* write_cyang_endsec_cmt */


/********************************************************************
* FUNCTION write_cyang_str
* 
* Generate a string token at the current line location
*
* INPUTS:
*   scb == session control block to use for writing
*   strval == string value
*   quotes == quotes style (0, 1, 2)
*   indent == current indent count
*********************************************************************/
static void
    write_cyang_str (ses_cb_t *scb,
		     const xmlChar *strval,
		     uint32 quotes,
		     int32 indent)
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

    ses_putcstr(scb, strval, indent);

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

}  /* write_cyang_str */


/********************************************************************
* FUNCTION write_cyang_id
* 
* Generate a simple clause with an identifier, on 1 line
*
* INPUTS:
*   scb == session control block to use for writing
*   kwname == keyword name
*   idname == identifier name (may be NULL)
*   indent == indent count to use
*   finsemi == TRUE if end in ';', FALSE if '{'
*   newln == TRUE if a newline should be output first
*            FALSE if newline should not be output first
*********************************************************************/
static void
    write_cyang_id (ses_cb_t *scb,
		    const xmlChar *kwname,
		    const xmlChar *idname,
		    int32 indent,
		    boolean finsemi,
		    boolean newln)
{
    if (newln) {
	ses_putchar(scb, '\n');
    }
    ses_putstr_indent(scb, kwname, indent);
    if (idname) {
	ses_putchar(scb, ' ');
	ses_putstr(scb, idname);
    }
    if (finsemi) {
	ses_putchar(scb, ';');
    } else {
	ses_putstr(scb, START_SEC);
    }

}  /* write_cyang_id */


/********************************************************************
* FUNCTION write_cyang_simple_str
* 
* Generate a simple clause on 1 line
*
* INPUTS:
*   scb == session control block to use for writing
*   kwname == keyword name
*   strval == string value
*   indent == indent count to use
*   quotes == quotes style (0, 1, 2)
*   finsemi == TRUE if end in ';', FALSE if '{'
*********************************************************************/
static void
    write_cyang_simple_str (ses_cb_t *scb,
			    const xmlChar *kwname,
			    const xmlChar *strval,
			    int32 indent,
			    uint32 quotes,
			    boolean finsemi)
{
    ses_putstr_indent(scb, kwname, indent);
    if (strval) {
	if (xml_strlen(strval) < 39) {
	    ses_putchar(scb, ' ');
	} else {
	    indent += ses_indent_count(scb);
	    ses_indent(scb, indent);
	}
	write_cyang_str(scb, strval, quotes, indent);
    }
    if (finsemi) {
	ses_putchar(scb, ';');
    } else {
	ses_putstr(scb, START_SEC);
    }

}  /* write_cyang_simple_str */


/********************************************************************
* FUNCTION write_cyang_status
* 
* Generate the YANG for a status clause only if the value
* is other than current
*   
* INPUTS:
*   scb == session control block to use for writing
*   status == status field
*   indent == start indent count
*
*********************************************************************/
static void
    write_cyang_status (ses_cb_t *scb,
		  ncx_status_t status,
		  int32 indent)
{
    if (status != NCX_STATUS_CURRENT && status != NCX_STATUS_NONE) {
	write_cyang_simple_str(scb, 
			       YANG_K_STATUS,
			       ncx_get_status_string(status),
			       indent, 
			       2, 
			       TRUE);
    }
}  /* write_cyang_status */


/********************************************************************
* FUNCTION write_cyang_type
* 
* Generate the YANG for the specified type name
* Does not generate the entire clause, just the type name
*   
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   typdef == typ_def_t to use
*   indent == start indent count
*
*********************************************************************/
static void
    write_cyang_type (ses_cb_t *scb,
		      const ncx_module_t *mod,
		      const typ_def_t *typdef,
		      int32 indent)
{
    ses_putstr_indent(scb, YANG_K_TYPE, indent);
    ses_putchar(scb, ' ');

    if (typdef->prefix && xml_strcmp(typdef->prefix, mod->prefix)) {
	write_cyang_extkw(scb, typdef->prefix, typdef->typename);
    } else {
	ses_putstr(scb, typdef->typename);
    }

}  /* write_cyang_type */


/********************************************************************
* FUNCTION write_cyang_errinfo
* 
* Generate the YANG for the specified error info struct
*
* INPUTS:
*   scb == session control block to use for writing
*   errinfo == ncx_errinfo_t struct to use
*   indent == start indent count
*
*********************************************************************/
static void
    write_cyang_errinfo (ses_cb_t *scb,
			 const ncx_errinfo_t *errinfo,
			 int32 indent)
{
    if (errinfo->error_message) {
	write_cyang_simple_str(scb, 
			       YANG_K_ERROR_MESSAGE,
			       errinfo->error_message, 
			       indent, 
			       2, 
			       TRUE);
    }
    if (errinfo->error_app_tag) {
	write_cyang_simple_str(scb, 
			       YANG_K_ERROR_APP_TAG,
			       errinfo->error_app_tag, 
			       indent, 
			       2, 
			       TRUE);
    }
    if (errinfo->descr) {
	write_cyang_simple_str(scb, 
			       YANG_K_DESCRIPTION,
			       errinfo->descr, 
			       indent, 
			       2, 
			       TRUE);
    }
    if (errinfo->ref) {
	write_cyang_simple_str(scb, 
			       YANG_K_REFERENCE,
			       errinfo->ref, 
			       indent, 
			       2, 
			       TRUE);
    }
}  /* write_cyang_errinfo */


/********************************************************************
* FUNCTION write_cyang_musts
* 
* Generate the YANG for a Q of ncx_errinfo_t representing
* must-stmts, not just error info
*
* INPUTS:
*   scb == session control block to use for writing
*   mustQ == Q of xpath_pcb_t to use
*   indent == start indent count
*
*********************************************************************/
static void
    write_cyang_musts (ses_cb_t *scb,
		       const dlq_hdr_t *mustQ,
		       int32 indent)
{
    const xpath_pcb_t    *must;
    const ncx_errinfo_t  *errinfo;

    for (must = (const xpath_pcb_t *)dlq_firstEntry(mustQ);
	 must != NULL;
	 must = (const xpath_pcb_t *)dlq_nextEntry(must)) {

	errinfo = &must->errinfo;
	if (errinfo->descr || errinfo->ref ||
	    errinfo->error_app_tag || errinfo->error_message) {
	    write_cyang_simple_str(scb, 
				   YANG_K_MUST,
				   must->exprstr, 
				   indent, 
				   2, 
				   FALSE);
	    write_cyang_errinfo(scb, 
				errinfo, 
				indent + ses_indent_count(scb));
	    ses_putstr_indent(scb, END_SEC, indent);
	} else {
	    write_cyang_simple_str(scb, 
				   YANG_K_MUST,
				   must->exprstr, 
				   indent, 
				   2, 
				   TRUE);
	}
    }

}  /* write_cyang_musts */


/********************************************************************
* FUNCTION write_cyang_appinfoQ
* 
* Generate the YANG for a Q of ncx_appinfo_t representing
* vendor extensions entered with the YANG statements
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   appinfoQ == Q of ncx_appinfo_t to use
*   indent == start indent count
*
*********************************************************************/
static void
    write_cyang_appinfoQ (ses_cb_t *scb,
			  const ncx_module_t *mod,
			  const yangdump_cvtparms_t *cp,
			  const dlq_hdr_t *appinfoQ,
			  int32 indent)
{
    const ncx_appinfo_t *appinfo;

    for (appinfo = (const ncx_appinfo_t *)dlq_firstEntry(appinfoQ);
	 appinfo != NULL;
	 appinfo = (const ncx_appinfo_t *)dlq_nextEntry(appinfo)) {

	ses_indent(scb, indent);
	write_cyang_extkw(scb, appinfo->prefix, appinfo->name);

	if (appinfo->value) {
	    ses_putchar(scb, ' ');
	    write_cyang_str(scb, appinfo->value, 2,
			    indent+ses_indent_count(scb));
	}

	if (!dlq_empty(appinfo->appinfoQ)) {
	    ses_putstr(scb, START_SEC);
	    write_cyang_appinfoQ(scb, mod, cp, appinfo->appinfoQ,
				 indent+ses_indent_count(scb));
	    ses_putstr_indent(scb, END_SEC, indent);
	} else {
	    ses_putchar(scb, ';');
	}
    }

}  /* write_cyang_appinfoQ */


/********************************************************************
* FUNCTION write_type_contents
* 
* Generate the YANG for the specified type
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   typdef == typ_def_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_type_contents (ses_cb_t *scb,
			       const ncx_module_t *mod,
			       const yangdump_cvtparms_t *cp,
			       const typ_def_t *typdef,
			       int32 startindent)
{
    const typ_unionnode_t *un;
    const typ_enum_t      *bit, *enu;
    const xmlChar         *str;
    const typ_range_t     *range;
    const typ_pattern_t   *pat;
    char                   buff[NCX_MAX_NUMLEN];
    int32                  indent;
    boolean                errinfo_set, constrained_set;

    indent = startindent + ses_indent_count(scb);

    switch (typdef->class) {
    case NCX_CL_BASE:
	break;
    case NCX_CL_NAMED:
	if (typdef->def.named.newtyp) {
	    typdef = typdef->def.named.newtyp;
	} else {
	    break;
	}
	/* fall through if typdef set */
    case NCX_CL_SIMPLE:
	switch (typdef->def.simple.btyp) {
	case NCX_BT_UNION:
	    for (un = typ_first_unionnode(typdef);
		 un != NULL;
		 un = (typ_unionnode_t *)dlq_nextEntry(un)) {
		if (un->typdef) {
		    write_cyang_type_clause(scb, mod, cp,
					    un->typdef, startindent);
		} else if (un->typ) {
		    write_cyang_type_clause(scb, mod, cp, 
					    &un->typ->typdef, startindent);
		} else {
		    SET_ERROR(ERR_INTERNAL_VAL);
		}
	    }
	    break;
	case NCX_BT_BITS:
	    for (bit = typ_first_con_enumdef(typdef);
		 bit != NULL;
		 bit = (const typ_enum_t *)dlq_nextEntry(bit)) {

		write_cyang_simple_str(scb, 
				       YANG_K_BIT, 
				       bit->name,
				       startindent, 
				       2, 
				       FALSE);

		sprintf(buff, "%u", bit->pos);
		write_cyang_simple_str(scb, 
				       YANG_K_POSITION,
				       (const xmlChar *)buff,
				       indent, 
				       0, 
				       TRUE);

		write_cyang_status(scb, bit->status, indent);

		if (bit->descr) {
		    write_cyang_simple_str(scb, 
					   YANG_K_DESCRIPTION,
					   bit->descr, 
					   indent, 
					   2, 
					   TRUE);
		}

		if (bit->ref) {
		    write_cyang_simple_str(scb, 
					   YANG_K_REFERENCE,
					   bit->ref, 
					   indent, 
					   2, 
					   TRUE);
		}

		write_cyang_appinfoQ(scb, 
				     mod, 
				     cp, 
				     &bit->appinfoQ, 
				     indent);

		ses_putstr_indent(scb, END_SEC, startindent);
	    }
	    break;
	case NCX_BT_ENUM:
	    for (enu = typ_first_con_enumdef(typdef);
		 enu != NULL;
		 enu = (const typ_enum_t *)dlq_nextEntry(enu)) {

		write_cyang_simple_str(scb, 
				       YANG_K_ENUM, 
				       enu->name,
				       startindent, 
				       2, 
				       FALSE);

		sprintf(buff, "%d", enu->val);
		write_cyang_simple_str(scb, 
				       YANG_K_VALUE,
				       (const xmlChar *)buff,
				       indent, 
				       0, 
				       TRUE);

		write_cyang_status(scb, enu->status, indent);

		if (enu->descr) {
		    write_cyang_simple_str(scb, 
					   YANG_K_DESCRIPTION,
					   enu->descr, 
					   indent, 
					   2, 
					   TRUE);
		}

		if (enu->ref) {
		    write_cyang_simple_str(scb, 
					   YANG_K_REFERENCE,
					   enu->ref, 
					   indent, 
					   2, 
					   TRUE);
		}

		write_cyang_appinfoQ(scb, 
				     mod, 
				     cp, 
				     &enu->appinfoQ, 
				     indent);

		ses_putstr_indent(scb, END_SEC,  startindent);
	    }
	    break;
	case NCX_BT_EMPTY:
	case NCX_BT_BOOLEAN:
	    /* appinfo only */
	    break;
	case NCX_BT_DECIMAL64:
	    sprintf(buff, "%d", typ_get_fraction_digits(typdef));
	    write_cyang_simple_str(scb,
				   YANG_K_FRACTION_DIGITS,
				   (const xmlChar *)buff,
				   startindent,
				   0,
				   FALSE);
	    /* fall through to check range */
	case NCX_BT_INT8:
	case NCX_BT_INT16:
	case NCX_BT_INT32:
	case NCX_BT_INT64:
	case NCX_BT_UINT8:
	case NCX_BT_UINT16:
	case NCX_BT_UINT32:
	case NCX_BT_UINT64:
	case NCX_BT_FLOAT64:
	    range = typ_get_crange_con(typdef);
	    if (range && range->rangestr) {
		errinfo_set = ncx_errinfo_set(&range->range_errinfo);
		write_cyang_simple_str(scb, 
				       YANG_K_RANGE,
				       range->rangestr,
				       startindent, 
				       2, 
				       !errinfo_set);
		if (errinfo_set) {
		    write_cyang_errinfo(scb, &range->range_errinfo, indent);
		    ses_putstr_indent(scb, END_SEC, startindent);
		}
	    }
	    break;
	case NCX_BT_STRING:
	case NCX_BT_BINARY:
	    range = typ_get_crange_con(typdef);
	    if (range && range->rangestr) {
		errinfo_set = ncx_errinfo_set(&range->range_errinfo);
		write_cyang_simple_str(scb, 
				       YANG_K_LENGTH,
				       range->rangestr,
				       startindent, 
				       2, 
				       !errinfo_set);
		if (errinfo_set) {
		    write_cyang_errinfo(scb, &range->range_errinfo, indent);
		    ses_putstr_indent(scb, END_SEC, startindent);
		}
	    }
	    for (pat = typ_get_first_cpattern(typdef);
		 pat != NULL;
		 pat = typ_get_next_cpattern(pat)) {

		errinfo_set = ncx_errinfo_set(&pat->pat_errinfo);
		write_cyang_simple_str(scb, 
				       YANG_K_PATTERN, 
				       pat->pat_str,
				       startindent, 
				       1, 
				       !errinfo_set);
		if (errinfo_set) {		
		    write_cyang_errinfo(scb, 
					&pat->pat_errinfo, 
					indent);
		    ses_putstr_indent(scb, END_SEC, startindent);
		}
	    }
	    break;
	case NCX_BT_SLIST:
	    break;
	case NCX_BT_LEAFREF:
	    str = typ_get_leafref_path(typdef);
	    if (str) {
		write_cyang_simple_str(scb, 
				       YANG_K_PATH, 
				       str,
				       startindent, 
				       2, 
				       TRUE);
	    }
	    /* fall through */
	case NCX_BT_INSTANCE_ID:
	    constrained_set = typ_get_constrained(typdef);
	    write_cyang_simple_str(scb, 
				   YANG_K_REQUIRE_INSTANCE,
				   (constrained_set) 
				   ? NCX_EL_TRUE : NCX_EL_FALSE,
				   startindent, 
				   2, 
				   TRUE);
	    break;
	default:
	    break;
	}
	break;
    case NCX_CL_COMPLEX:
	SET_ERROR(ERR_INTERNAL_VAL);
	break;
    case NCX_CL_REF:
	SET_ERROR(ERR_INTERNAL_VAL);
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    write_cyang_appinfoQ(scb, mod, cp, &typdef->appinfoQ, startindent);

}  /* write_cyang_type_contents */


/********************************************************************
* FUNCTION write_cyang_type_clause
* 
* Generate the YANG for the specified type clause
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   typdef == typ_def_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_type_clause (ses_cb_t *scb,
			     const ncx_module_t *mod,
			     const yangdump_cvtparms_t *cp,
			     const typ_def_t *typdef,
			     int32 startindent)
{
    write_cyang_type(scb, mod, typdef, startindent);
    if (typ_has_subclauses(typdef)) {
	ses_putstr(scb, START_SEC);
	write_cyang_type_contents(scb, mod, cp, typdef,
				  startindent + ses_indent_count(scb));
	ses_putstr_indent(scb, END_SEC, startindent);
    } else {
	ses_putchar(scb, ';');
    }
}  /* write_cyang_type_clause */


/********************************************************************
* FUNCTION write_cyang_typedef
* 
* Generate the YANG for 1 typedef
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   typ == typ_template_t to use
*   startindent == start indent count
*   first == TRUE if this is the first typedef at this indent level
*            FALSE if not the first typedef at this indent level
**********************************************************************/
static void
    write_cyang_typedef (ses_cb_t *scb,
			 const ncx_module_t *mod,
			 const yangdump_cvtparms_t *cp,
			 const typ_template_t *typ,
			 int32 startindent,
			 boolean first)
{
    int32                    indent;

    indent = startindent + ses_indent_count(scb);

    write_cyang_id(scb, YANG_K_TYPEDEF, typ->name, startindent, 
		   FALSE, !first);

    /* type field */
    write_cyang_type_clause(scb, mod, cp, &typ->typdef, indent);

    /* units field */
    if (typ->units) {
	write_cyang_simple_str(scb, 
			       YANG_K_UNITS, 
			       typ->units, 
			       indent, 
			       2, 
			       TRUE);
    }

    /* default field */
    if (typ->defval) {
	write_cyang_simple_str(scb, 
			       YANG_K_DEFAULT, 
			       typ->defval, 
			       indent, 
			       2, 
			       TRUE);
    }

    /* status field */
    write_cyang_status(scb, typ->status, indent);

    /* description field */
    if (typ->descr) {
	write_cyang_simple_str(scb, 
			       YANG_K_DESCRIPTION, 
			       typ->descr, 
			       indent, 
			       2, 
			       TRUE);
    }

    /* reference field */
    if (typ->ref) {
	write_cyang_simple_str(scb, 
			       YANG_K_REFERENCE, 
			       typ->ref, 
			       indent, 
			       2, 
			       TRUE);
    }

    /* appinfoQ */
    write_cyang_appinfoQ(scb, mod, cp, &typ->appinfoQ, indent);

    /* end typedef clause */
    ses_putstr_indent(scb, END_SEC, startindent);

}  /* write_cyang_typedef */



/********************************************************************
* FUNCTION write_cyang_typedefs
* 
* Generate the YANG for the specified typedefQ
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   typedefQ == que of typ_template_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_typedefs (ses_cb_t *scb,
			  const ncx_module_t *mod,
			  const yangdump_cvtparms_t *cp,
			  const dlq_hdr_t *typedefQ,
			  int32 startindent)
{
    const typ_template_t    *typ;
    boolean                  first;

    if (dlq_empty(typedefQ)) {
	return;
    }

    if (typedefQ == &mod->typeQ) {
	write_cyang_banner_cmt(scb, mod, cp,
			       (const xmlChar *)"typedefs", startindent);
    }

    first = TRUE;
    for (typ = (const typ_template_t *)dlq_firstEntry(typedefQ);
	 typ != NULL;
	 typ = (const typ_template_t *)dlq_nextEntry(typ)) {

	write_cyang_typedef(scb, mod, cp, typ, startindent, first);
	first = FALSE;
    }

}  /* write_cyang_typedefs */


/********************************************************************
* FUNCTION write_cyang_grouping
* 
* Generate the YANG for 1 grouping
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   grp == grp_template_t to use
*   startindent == start indent count
*   first == TRUE if this is the first grouping at this indent level
*            FALSE if not the first grouping at this indent level
**********************************************************************/
static void
    write_cyang_grouping (ses_cb_t *scb,
			  const ncx_module_t *mod,
			  const yangdump_cvtparms_t *cp,
			  const grp_template_t *grp,
			  int32 startindent,
			  boolean first)
{
    int32                    indent;
    boolean                  cooked;

    cooked = (strcmp(cp->objview, OBJVIEW_COOKED)) ? FALSE : TRUE;
    indent = startindent + ses_indent_count(scb);

    if (cooked && !grp_has_typedefs(grp)) {
	return;
    }
	
    write_cyang_id(scb, YANG_K_GROUPING, grp->name, startindent,
		   FALSE, !first);

    /* status field */
    write_cyang_status(scb, grp->status, indent);

    /* description field */
    if (grp->descr) {
	write_cyang_simple_str(scb, 
			       YANG_K_DESCRIPTION, 
			       grp->descr, 
			       indent, 
			       2, 
			       TRUE);
    }

    /* reference field */
    if (grp->ref) {
	write_cyang_simple_str(scb, 
			       YANG_K_REFERENCE, 
			       grp->ref, 
			       indent, 
			       2, 
			       TRUE);
    }

    write_cyang_typedefs(scb, mod, cp, &grp->typedefQ, indent);

    write_cyang_groupings(scb, mod, cp, &grp->groupingQ, indent);

    if (!cooked) {
	write_cyang_objects(scb, mod, cp, &grp->datadefQ, indent);
    }

    /* appinfoQ */
    write_cyang_appinfoQ(scb, mod, cp, &grp->appinfoQ, indent);

    /* end grouping clause */
    ses_putstr_indent(scb, END_SEC, startindent);

    /* end grouping comment */
    write_cyang_endsec_cmt(scb, YANG_K_GROUPING, grp->name);

}  /* write_cyang_grouping */


/********************************************************************
* FUNCTION write_cyang_groupings
* 
* Generate the YANG for the specified groupingQ
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   groupingQ == que of grp_template_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_groupings (ses_cb_t *scb,
			   const ncx_module_t *mod,
			   const yangdump_cvtparms_t *cp,
			   const dlq_hdr_t *groupingQ,
			   int32 startindent)
{
    const grp_template_t    *grp;
    boolean                  needed, cooked, first;

    if (dlq_empty(groupingQ)) {
	return;
    }

    cooked = (strcmp(cp->objview, OBJVIEW_COOKED)) ? FALSE : TRUE;

    /* groupings are only generated in cooked mode if they have
     * typedefs, and then just the typedefs are generated
     */
    if (cooked) {
	needed = FALSE;
	for (grp = (const grp_template_t *)dlq_firstEntry(groupingQ);
	     grp != NULL && needed==FALSE;
	     grp = (const grp_template_t *)dlq_nextEntry(grp)) {
	    needed = grp_has_typedefs(grp);
	}
	if (!needed) {
	    return;
	}
    }

    /* put comment for first grouping only */
    if (groupingQ == &mod->groupingQ) {
	write_cyang_banner_cmt(scb, mod, cp,
			       (const xmlChar *)"groupings", startindent);
    }

    first = TRUE;
    for (grp = (const grp_template_t *)dlq_firstEntry(groupingQ);
	 grp != NULL;
	 grp = (const grp_template_t *)dlq_nextEntry(grp)) {

	write_cyang_grouping(scb, mod, cp, grp, startindent, first);
	first = FALSE;
    }

}  /* write_cyang_groupings */


/********************************************************************
* FUNCTION write_cyang_iffeature
* 
* Generate the YANG for 1 if-feature statement
*
* INPUTS:
*   scb == session control block to use for writing
*   iffeature == ncx_iffeature_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_iffeature (ses_cb_t *scb,
			   const ncx_iffeature_t *iffeature,
			   int32 startindent)
{
    ses_putstr_indent(scb, YANG_K_IF_FEATURE, startindent);
    ses_putchar(scb, ' ');
    if (iffeature->prefix) {
	ses_putstr(scb, iffeature->prefix);
	ses_putchar(scb, ':');
    }
    ses_putstr(scb, iffeature->name);
    ses_putchar(scb, ';');

}  /* write_cyang_iffeature */


/********************************************************************
* FUNCTION write_cyang_iffeatureQ
* 
* Generate the YANG for a Q of if-feature statements
*
* INPUTS:
*   scb == session control block to use for writing
*   iffeatureQ == Q of ncx_iffeature_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_iffeatureQ (ses_cb_t *scb,
			    const dlq_hdr_t *iffeatureQ,
			    int32 startindent)
{
    const ncx_iffeature_t   *iffeature;

    for (iffeature = (const ncx_iffeature_t *)
	     dlq_firstEntry(iffeatureQ);
	 iffeature != NULL;
	 iffeature = (const ncx_iffeature_t *)
	     dlq_nextEntry(iffeature)) {

	write_cyang_iffeature(scb, iffeature, startindent);
    }

}  /* write_cyang_iffeatureQ */


/********************************************************************
* FUNCTION write_cyang_object
* 
* Generate the YANG for 1 datadef
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   obj == obj_template_t to use
*   startindent == start indent count
*   first == TRUE if this is the first object at this indent level
*            FALSE if not the first object at this indent level
**********************************************************************/
static void
    write_cyang_object (ses_cb_t *scb,
			const ncx_module_t *mod,
			const yangdump_cvtparms_t *cp,
			const obj_template_t *obj,
			int32 startindent,
			boolean first)
{
    const obj_container_t   *con;
    const obj_leaf_t        *leaf;
    const obj_leaflist_t    *leaflist;
    const obj_list_t        *list;
    const obj_choice_t      *choic;
    const obj_case_t        *cas;
    const obj_uses_t        *uses;
    const obj_augment_t     *aug;
    const obj_rpc_t         *rpc;
    const obj_rpcio_t       *rpcio;
    const obj_notif_t       *notif;
    const obj_key_t         *key, *nextkey;
    const obj_unique_t      *uni;
    const obj_unique_comp_t *unicomp, *nextunicomp;
    int32                    indent;
    char                     buff[NCX_MAX_NUMLEN];
    boolean                  notrefined, isanyxml, isempty, rawmode;

    isanyxml = FALSE;
    indent = startindent + ses_indent_count(scb);
    rawmode = strcmp(cp->objview, OBJVIEW_RAW) ? FALSE : TRUE;

    if (obj_is_cloned(obj) && rawmode) {
	/* skip cloned objects in 'raw' object view mode */
	return;
    }

    notrefined = !obj_is_refine(obj);
    isempty = obj_is_empty(obj);

    switch (obj->objtype) {
    case OBJ_TYP_CONTAINER:
	con = obj->def.container;
	write_cyang_id(scb, YANG_K_CONTAINER, con->name, startindent, 
		       isempty, !first);
	if (isempty && rawmode) {
	    return;
	}

	write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);

	/* 0 or more must-stmts */
	write_cyang_musts(scb, &con->mustQ, indent);

	/* presence field */
	if (con->presence) {
	    write_cyang_simple_str(scb, 
				   YANG_K_PRESENCE, 
				   con->presence, 
				   indent, 
				   2, 
				   TRUE);
	}

	/* config field, only if actually set (no default) */
	if (obj->flags & OBJ_FL_CONFSET) {
	    write_cyang_simple_str(scb, 
				   YANG_K_CONFIG, 
				   (obj->flags & OBJ_FL_CONFIG) 
				   ? NCX_EL_TRUE : NCX_EL_FALSE,
				   indent, 
				   2, 
				   TRUE);
	}
	    
	/* status field */
	if (notrefined) {
	    write_cyang_status(scb, con->status, indent);
	}

	/* description field */
	if (con->descr) {
	    write_cyang_simple_str(scb, 
				   YANG_K_DESCRIPTION, 
				   con->descr, 
				   indent, 
				   2, 
				   TRUE);
	}
	    
	/* reference field */
	if (con->ref) {
	    write_cyang_simple_str(scb, 
				   YANG_K_REFERENCE, 
				   con->ref, 
				   indent, 
				   2,
				   TRUE);
	}

	if (notrefined) {
	    write_cyang_typedefs(scb, mod, cp, con->typedefQ, indent);
	    write_cyang_groupings(scb, mod, cp, con->groupingQ, indent);
	}

	write_cyang_objects(scb, mod, cp, con->datadefQ, indent);

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);

	/* end container comment */
	write_cyang_endsec_cmt(scb, YANG_K_CONTAINER, con->name);
	break;
    case OBJ_TYP_ANYXML:
        isanyxml = TRUE;
        /* fall through */
    case OBJ_TYP_LEAF:
	leaf = obj->def.leaf;
	if (isanyxml) {
	    write_cyang_id(scb, YANG_K_ANYXML, leaf->name, startindent, 
			   isempty, !first);
	    if (isempty) {
		return;
	    }
	} else {
	    write_cyang_id(scb, YANG_K_LEAF, leaf->name, startindent, 
			   isempty, !first);
	    if (isempty) {
		return;
	    }

	    write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);

	    /* type field */
	    if (notrefined) {
		write_cyang_type_clause(scb, mod, cp, leaf->typdef, indent);
	    }

	    /* units clause */
	    if (notrefined && leaf->units) {
		write_cyang_simple_str(scb,
				       YANG_K_UNITS,
				       leaf->units,
				       indent, 
				       2, 
				       TRUE);
	    }

	    /* 0 or more must-stmts */
	    write_cyang_musts(scb, &leaf->mustQ, indent);

	    /* default field */
	    if (leaf->defval) {
		write_cyang_simple_str(scb, 
				       YANG_K_DEFAULT, 
				       leaf->defval, 
				       indent, 
				       2, 
				       TRUE);
	    }
	}

	/* config field, only if actually set (no default) */
	if (obj->flags & OBJ_FL_CONFSET) {
	    write_cyang_simple_str(scb, 
				   YANG_K_CONFIG, 
				   (obj->flags & OBJ_FL_CONFIG) ? 
				   NCX_EL_TRUE : NCX_EL_FALSE,
				   indent, 
				   2, 
				   TRUE);
	}

	/* mandatory field, only if actually set (no default) */
	if (obj->flags & OBJ_FL_MANDSET) {
	    write_cyang_simple_str(scb, 
				   YANG_K_MANDATORY, 
				   (obj->flags & OBJ_FL_MANDATORY) ? 
				   NCX_EL_TRUE : NCX_EL_FALSE,
				   indent, 
				   2, 
				   TRUE);
	}

	/* status field */
	if (notrefined) {
	    write_cyang_status(scb, leaf->status, indent);
	}

	/* description field */
	if (leaf->descr) {
	    write_cyang_simple_str(scb, 
				   YANG_K_DESCRIPTION, 
				   leaf->descr, 
				   indent, 
				   2, 
				   TRUE);
	}

	/* reference field */
	if (leaf->ref) {
	    write_cyang_simple_str(scb, 
				   YANG_K_REFERENCE, 
				   leaf->ref, 
				   indent, 
				   2, 
				   TRUE);
	}

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);
	break;
    case OBJ_TYP_LEAF_LIST:
	leaflist = obj->def.leaflist;
	write_cyang_id(scb, YANG_K_LEAF_LIST, leaflist->name, startindent, 
		       isempty, !first);
	if (isempty) {
	    return;
	}

	write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);

	/* type field */
	if (notrefined) {
	    write_cyang_type_clause(scb, mod, cp, leaflist->typdef, indent);
	}

	/* units clause */
	if (notrefined && leaflist->units) {
	    write_cyang_simple_str(scb, 
				   YANG_K_UNITS, 
				   leaflist->units,
				   indent, 
				   2, 
				   TRUE);
	}

	/* 0 or more must-stmts */
	write_cyang_musts(scb, &leaflist->mustQ, indent);

	/* config field, only if actually set (no default) */
	if (obj->flags & OBJ_FL_CONFSET) {
	    write_cyang_simple_str(scb, 
				   YANG_K_CONFIG, 
				   (obj->flags & OBJ_FL_CONFIG) ? 
				   NCX_EL_TRUE : NCX_EL_FALSE,
				   indent, 
				   2, 
				   TRUE);
	}

	/*  min-elements */
	if (leaflist->minset && leaflist->minelems) {
	    sprintf(buff, "%u", leaflist->minelems);
	    write_cyang_simple_str(scb, 
				   YANG_K_MIN_ELEMENTS, 
				   (const xmlChar *)buff,
				   indent, 
				   2, 
				   TRUE);
	}

	/*  max-elements */
	if (leaflist->maxset && leaflist->maxelems) {
	    sprintf(buff, "%u", leaflist->maxelems);
	    write_cyang_simple_str(scb, 
				   YANG_K_MAX_ELEMENTS, 
				   (const xmlChar *)buff,
				   indent, 
				   2, 
				   TRUE);
	}

	/* ordered-by field */
	if (notrefined) {
	    write_cyang_simple_str(scb, 
				   YANG_K_ORDERED_BY, 
				   (leaflist->ordersys) ? 
				   YANG_K_SYSTEM : YANG_K_USER,
				   indent, 
				   2, 
				   TRUE);
	}

	/* status field */
	if (notrefined) {
	    write_cyang_status(scb, leaflist->status, indent);
	}

	/* description field */
	if (leaflist->descr) {
	    write_cyang_simple_str(scb, 
				   YANG_K_DESCRIPTION, 
				   leaflist->descr, 
				   indent, 
				   2, 
				   TRUE);
	}

	/* reference field */
	if (leaflist->ref) {
	    write_cyang_simple_str(scb, 
				   YANG_K_REFERENCE, 
				   leaflist->ref, 
				   indent, 
				   2, 
				   TRUE);
	}

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);
	break;
    case OBJ_TYP_LIST:
	list = obj->def.list;
	write_cyang_id(scb, YANG_K_LIST, list->name, startindent, 
		       isempty, !first);
	if (isempty) {
	    return;
	}

	write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);

	/* 0 or more must-stmts */
	write_cyang_musts(scb, &list->mustQ, indent);

	/* key field, manual generation to make links */
	if (notrefined && !dlq_empty(&list->keyQ)) {
	    ses_indent(scb, indent);
	    ses_putstr(scb, YANG_K_KEY);
	    ses_putstr(scb, (const xmlChar *)" \"");

	    for (key = obj_first_ckey(obj);
		 key != NULL; 
		 key = nextkey) {
		nextkey = obj_next_ckey(key);
		ses_putstr(scb, obj_get_name(key->keyobj));
		if (nextkey) {
		    ses_putchar(scb, ' ');
		}
	    }
	    ses_putstr(scb, (const xmlChar *)"\";");
	}

	/* unique fields, manual generation to make links */
	if (notrefined && !dlq_empty(&list->uniqueQ)) {
	    for (uni = (const obj_unique_t *)
		     dlq_firstEntry(&list->uniqueQ);
		 uni != NULL;
		 uni = (const obj_unique_t *)dlq_nextEntry(uni)) {

		ses_indent(scb, indent);
		ses_putstr(scb, YANG_K_UNIQUE);
		ses_putstr(scb, (const xmlChar *)" \"");

		for (unicomp = (const obj_unique_comp_t *)
			 dlq_firstEntry(&uni->compQ);
		     unicomp != NULL; unicomp = nextunicomp) {
		    nextunicomp = (const obj_unique_comp_t *)
			dlq_nextEntry(unicomp);
		    ses_putstr(scb, unicomp->xpath);
		    if (nextunicomp) {
			ses_putstr(scb, (const xmlChar *)", ");
		    }
		}
		ses_putstr(scb, (const xmlChar *)"\";");
	    }
	}

	/* config field, only if actually set (no default) */
	if (obj->flags & OBJ_FL_CONFSET) {
	    write_cyang_simple_str(scb, 
				   YANG_K_CONFIG, 
				   (obj->flags & OBJ_FL_CONFIG) ? 
				   NCX_EL_TRUE : NCX_EL_FALSE,
				   indent, 
				   2, 
				   TRUE);
	}

	/*  min-elements */
	if (list->minset && list->minelems) {
	    sprintf(buff, "%u", list->minelems);
	    write_cyang_simple_str(scb, 
				   YANG_K_MIN_ELEMENTS, 
				   (const xmlChar *)buff,
				   indent,
				   2,
				   TRUE);
	}

	/*  max-elements */
	if (list->maxset && list->maxelems) {
	    sprintf(buff, "%u", list->maxelems);
	    write_cyang_simple_str(scb,
				   YANG_K_MAX_ELEMENTS, 
				   (const xmlChar *)buff,
				   indent,
				   2,
				   TRUE);
	}

	/* ordered-by field */
	if (notrefined) {
	    write_cyang_simple_str(scb,
				   YANG_K_ORDERED_BY, 
				   (list->ordersys) ? 
				   YANG_K_SYSTEM : YANG_K_USER,
				   indent,
				   2,
				   TRUE);
	}

	/* status field */
	if (notrefined) {
	    write_cyang_status(scb, list->status, indent);
	}

	/* description field */
	if (list->descr) {
	    write_cyang_simple_str(scb,
				   YANG_K_DESCRIPTION, 
				   list->descr,
				   indent,
				   2,
				   TRUE);
	}

	/* reference field */
	if (list->ref) {
	    write_cyang_simple_str(scb,
				   YANG_K_REFERENCE, 
				   list->ref,
				   indent,
				   2,
				   TRUE);
	}



	if (notrefined) {
	    write_cyang_typedefs(scb, mod, cp, list->typedefQ, indent);
	    write_cyang_groupings(scb, mod, cp, list->groupingQ, indent);
	}

	write_cyang_objects(scb, mod, cp, list->datadefQ, indent);

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);

	/* end list section comment */
	write_cyang_endsec_cmt(scb, YANG_K_LIST, list->name);
	break;
    case OBJ_TYP_CHOICE:
	choic = obj->def.choic;
	write_cyang_id(scb, YANG_K_CHOICE, choic->name, startindent, 
		       isempty, !first);
	if (isempty) {
	    return;
	}

	write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);

	/* default case field */
	if (choic->defval) {
	    write_cyang_simple_str(scb, 
				   YANG_K_DEFAULT, 
				   choic->defval, 
				   indent, 
				   2, 
				   TRUE);
	}

	/* mandatory field, only if actually set (no default) */
	if (obj->flags & OBJ_FL_MANDSET) {
	    write_cyang_simple_str(scb, 
				   YANG_K_MANDATORY, 
				   (obj->flags & OBJ_FL_MANDATORY) ? 
				   NCX_EL_TRUE : NCX_EL_FALSE,
				   indent, 
				   2, 
				   TRUE);
	}

	/* status field */
	if (notrefined) {
	    write_cyang_status(scb, choic->status, indent);
	}

	/* description field */
	if (choic->descr) {
	    write_cyang_simple_str(scb, 
				   YANG_K_DESCRIPTION, 
				   choic->descr, 
				   indent, 
				   2, 
				   TRUE);
	}

	/* reference field */
	if (choic->ref) {
	    write_cyang_simple_str(scb, 
				   YANG_K_REFERENCE, 
				   choic->ref, 
				   indent, 
				   2, 
				   TRUE);
	}

	write_cyang_objects(scb, mod, cp, choic->caseQ, indent);

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);

	/* end choice comment */
	write_cyang_endsec_cmt(scb, YANG_K_CHOICE, choic->name);
	break;
    case OBJ_TYP_CASE:
	cas = obj->def.cas;
	write_cyang_id(scb, YANG_K_CASE, cas->name, startindent, 
		       isempty, !first);
	if (isempty) {
	    return;
	}

	write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);

	/* status field */
	if (notrefined) {
	    write_cyang_status(scb, cas->status, indent);
	}

	/* description field */
	if (cas->descr) {
	    write_cyang_simple_str(scb, 
				   YANG_K_DESCRIPTION, 
				   cas->descr, 
				   indent, 
				   2, 
				   TRUE);
	}

	/* reference field */
	if (cas->ref) {
	    write_cyang_simple_str(scb,
				   YANG_K_REFERENCE, 
				   cas->ref, 
				   indent, 
				   2,
				   TRUE);
	}

	write_cyang_objects(scb, mod, cp, cas->datadefQ, indent);

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);

	/* end case comment */
	write_cyang_endsec_cmt(scb, YANG_K_CASE, cas->name);
	break;
    case OBJ_TYP_USES:
	if (strcmp(cp->objview, OBJVIEW_RAW)) {
	    return;
	}

	uses = obj->def.uses;
	if (!first) {
	    ses_putchar(scb, '\n');
	}
	ses_indent(scb, startindent);
	ses_putstr(scb, YANG_K_USES);
	ses_putchar(scb, ' ');
	if (uses->prefix && xml_strcmp(uses->prefix, mod->prefix)) {
	    write_cyang_extkw(scb, uses->prefix, uses->name);
	} else {
	    ses_putstr(scb, uses->name);
	}

	if (uses->descr || uses->ref || 
	    uses->status != NCX_STATUS_CURRENT ||
	    !dlq_empty(&obj->iffeatureQ) ||
	    !dlq_empty(uses->datadefQ) ||
	    !dlq_empty(&obj->appinfoQ)) {

	    ses_putstr(scb, START_SEC);

	    write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);


	    /* status field */
	    write_cyang_status(scb, uses->status, indent);

	    /* description field */
	    if (uses->descr) {
		write_cyang_simple_str(scb,
				       YANG_K_DESCRIPTION, 
				       uses->descr,
				       indent, 
				       2, 
				       TRUE);
	    }

	    /* reference field */
	    if (uses->ref) {
		write_cyang_simple_str(scb, 
				       YANG_K_REFERENCE, 
				       uses->ref, 
				       indent, 
				       2, 
				       TRUE);
	    }

	    write_cyang_objects(scb, mod, cp, uses->datadefQ, indent);

	    write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	    /* end object definition clause */
	    ses_putstr_indent(scb, END_SEC, startindent);
	} else {
	    ses_putchar(scb, ';');
	}
	break;
    case OBJ_TYP_AUGMENT:
	if (strcmp(cp->objview, OBJVIEW_RAW)) {
	    return;
	}

	aug = obj->def.augment;
	if (!first) {
	    ses_putchar(scb, '\n');
	}
	ses_indent(scb, startindent);
	ses_putstr(scb, YANG_K_AUGMENT);
	ses_putchar(scb, ' ');
	ses_putstr(scb, aug->target);

	if (isempty) {
	    ses_putchar(scb, ';');
	    return;
	}

	ses_putstr(scb, START_SEC);

	write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);

	/* when field */
	if (obj->when && obj->when->exprstr) {
	    write_cyang_simple_str(scb, 
				   YANG_K_WHEN, 
				   obj->when->exprstr,
				   indent, 
				   2, 
				   TRUE);
	}		

	/* status field */
	write_cyang_status(scb, aug->status, indent);

	/* description field */
	if (aug->descr) {
	    write_cyang_simple_str(scb, 
				   YANG_K_DESCRIPTION, 
				   aug->descr, 
				   indent, 
				   2, 
				   TRUE);
	}

	/* reference field */
	if (aug->ref) {
	    write_cyang_simple_str(scb, 
				   YANG_K_REFERENCE, 
				   aug->ref, 
				   indent, 
				   2, 
				   TRUE);
	}

	if (!dlq_empty(&aug->datadefQ)) {
	    write_cyang_objects(scb, mod, cp, &aug->datadefQ, indent);
	}

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);
	break;
    case OBJ_TYP_RPC:
	rpc = obj->def.rpc;
	write_cyang_id(scb, YANG_K_RPC, rpc->name, startindent, 
		       isempty, !first);
	if (isempty) {
	    return;
	}

	write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);


	/* status field */
	write_cyang_status(scb, rpc->status, indent);

	/* description field */
	if (rpc->descr) {
	    write_cyang_simple_str(scb, 
				   YANG_K_DESCRIPTION, 
				   rpc->descr,
				   indent,
				   2,
				   TRUE);
	}

	/* reference field */
	if (rpc->ref) {
	    write_cyang_simple_str(scb,
				   YANG_K_REFERENCE, 
				   rpc->ref,
				   indent,
				   2,
				   TRUE);
	}

	write_cyang_typedefs(scb, mod, cp, &rpc->typedefQ, indent);

	write_cyang_groupings(scb, mod, cp, &rpc->groupingQ, indent);

	write_cyang_objects(scb, mod, cp, &rpc->datadefQ, indent);

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);

	/* end RPC section comment */
	write_cyang_endsec_cmt(scb, YANG_K_RPC, rpc->name);
	break;
    case OBJ_TYP_RPCIO:
	rpcio = obj->def.rpcio;

	if (!dlq_empty(&rpcio->typedefQ) ||
	    !dlq_empty(&rpcio->groupingQ) ||
	    !dlq_empty(&rpcio->datadefQ) ||
	    !dlq_empty(&obj->appinfoQ)) {

	    write_cyang_id(scb, obj_get_name(obj), NULL, startindent, 
			   FALSE, !first);

	    write_cyang_typedefs(scb, mod, cp, &rpcio->typedefQ, indent);

	    write_cyang_groupings(scb, mod, cp, &rpcio->groupingQ, indent);

	    write_cyang_objects(scb, mod, cp, &rpcio->datadefQ, indent);

	    write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	    ses_putstr_indent(scb, END_SEC, startindent);
	}
	break;
    case OBJ_TYP_NOTIF:
	notif = obj->def.notif;
	write_cyang_id(scb, YANG_K_NOTIFICATION, notif->name, startindent, 
		       isempty, !first);
	if (isempty) {
	    return;
	}

	write_cyang_iffeatureQ(scb, &obj->iffeatureQ, indent);

	/* status field */
	write_cyang_status(scb, notif->status, indent);

	/* description field */
	if (notif->descr) {
	    write_cyang_simple_str(scb,
				   YANG_K_DESCRIPTION, 
				   notif->descr,
				   indent,
				   2,
				   TRUE);
	}

	/* reference field */
	if (notif->ref) {
	    write_cyang_simple_str(scb,
				   YANG_K_REFERENCE, 
				   notif->ref,
				   indent,
				   2,
				   TRUE);
	}

	write_cyang_typedefs(scb, mod, cp, &notif->typedefQ, indent);

	write_cyang_groupings(scb, mod, cp, &notif->groupingQ, indent);

	write_cyang_objects(scb, mod, cp, &notif->datadefQ, indent);

	write_cyang_appinfoQ(scb, mod, cp, &obj->appinfoQ, indent);

	/* end object definition clause */
	ses_putstr_indent(scb, END_SEC, startindent);

	/* end notification section comment */
	write_cyang_endsec_cmt(scb, YANG_K_NOTIFICATION, notif->name);
	break;
    case OBJ_TYP_REFINE:
	break;   /*****/
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }
    
}  /* write_cyang_object */


/********************************************************************
* FUNCTION write_cyang_objects
* 
* Generate the YANG for the specified datadefQ
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   datadefQ == que of obj_template_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_objects (ses_cb_t *scb,
			 const ncx_module_t *mod,
			 const yangdump_cvtparms_t *cp,
			 const dlq_hdr_t *datadefQ,
			 int32 startindent)
{
    const obj_template_t    *obj;
    boolean                  first;

    if (dlq_empty(datadefQ)) {
	return;
    }

    if (datadefQ == &mod->datadefQ) {
	write_cyang_banner_cmt(scb, mod, cp,
			       (const xmlChar *)"objects", 
			       startindent);
    }

    first = TRUE;
    for (obj = (const obj_template_t *)dlq_firstEntry(datadefQ);
	 obj != NULL;
	 obj = (const obj_template_t *)dlq_nextEntry(obj)) {

	if (obj_is_hidden(obj)) {
	    continue;
	}

	write_cyang_object(scb, mod, cp, obj, startindent, first);
	first = FALSE;
    }

}  /* write_cyang_objects */


/********************************************************************
* FUNCTION write_cyang_extension
* 
* Generate the YANG for 1 extension
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == ncx_module_t struct in progress
*   cp == conversion parameters in use
*   ext == ext_template_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_extension (ses_cb_t *scb,
			   const ncx_module_t *mod,
			   const yangdump_cvtparms_t *cp,
			   const ext_template_t *ext,
			   int32 startindent)
{
    int32              indent;

    indent = startindent + ses_indent_count(scb);

    write_cyang_id(scb, YANG_K_EXTENSION, 
		   ext->name, startindent, 
		   FALSE, FALSE);

    /* argument sub-clause */
    if (ext->arg) {
	write_cyang_simple_str(scb,
			       YANG_K_ARGUMENT, 
			       ext->arg, 
			       indent, 
			       2, 
			       FALSE);
	write_cyang_simple_str(scb, 
			       YANG_K_YIN_ELEMENT, 
			       ext->argel ? NCX_EL_TRUE : NCX_EL_FALSE,
			       indent + ses_indent_count(scb), 
			       2, 
			       FALSE);
	ses_putstr_indent(scb, END_SEC, indent);
    }

    /* status field */
    write_cyang_status(scb, ext->status, indent);

    /* description field */
    if (ext->descr) {
	write_cyang_simple_str(scb, 
			       YANG_K_DESCRIPTION, 
			       ext->descr, 
			       indent, 
			       2, 
			       TRUE);
    }

    /* reference field */
    if (ext->ref) {
	write_cyang_simple_str(scb, 
			       YANG_K_REFERENCE, 
			       ext->ref, 
			       indent, 
			       2, 
			       TRUE);
    }

    write_cyang_appinfoQ(scb, mod, cp, &ext->appinfoQ, indent);

    /* end extension clause */
    ses_putstr_indent(scb, END_SEC, startindent);

}  /* write_cyang_extension */


/********************************************************************
* FUNCTION write_cyang_extensions
* 
* Generate the YANG for the specified extensionQ
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == ncx_module_t struct in progress
*   cp == conversion parameters in use
*   extensionQ == que of ext_template_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_extensions (ses_cb_t *scb,
			    const ncx_module_t *mod,
			    const yangdump_cvtparms_t *cp,
			    const dlq_hdr_t *extensionQ,
			    int32 startindent)
{
    const ext_template_t *ext;
    int32              indent;

    if (dlq_empty(extensionQ)) {
	return;
    }

    write_cyang_banner_cmt(scb, mod, cp,
			   (const xmlChar *)"extensions", 
			   startindent);

    indent = startindent + ses_indent_count(scb);

    for (ext = (const ext_template_t *)dlq_firstEntry(extensionQ);
	 ext != NULL;
	 ext = (const ext_template_t *)dlq_nextEntry(ext)) {

	write_cyang_extension(scb, mod, cp, ext, startindent);
    }

}  /* write_cyang_extensions */


/********************************************************************
* FUNCTION write_cyang_identity
* 
* Generate the YANG for 1 identity
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == ncx_module_t struct in progress
*   cp == conversion parameters in use
*   identity == ncx_identity_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_identity (ses_cb_t *scb,
			  const ncx_module_t *mod,
			  const yangdump_cvtparms_t *cp,
			  const ncx_identity_t *identity,
			  int32 startindent)
{
    int32              indent;

    indent = startindent + ses_indent_count(scb);

    write_cyang_id(scb, YANG_K_IDENTITY, 
		   identity->name, 
		   startindent, 
		   FALSE, FALSE);

    /* base sub-clause */
    if (identity->base) {
	ses_putstr_indent(scb, YANG_K_BASE, indent);
	ses_putchar(scb, ' ');
	if (identity->baseprefix) {
	    write_cyang_extkw(scb, 
			      identity->baseprefix, 
			      identity->basename);
	} else {
	    ses_putstr(scb, identity->basename);
	}
	ses_putchar(scb, ';');
    }

    /* status field */
    write_cyang_status(scb, identity->status, indent);

    /* description field */
    if (identity->descr) {
	write_cyang_simple_str(scb, 
			       YANG_K_DESCRIPTION, 
			       identity->descr, 
			       indent, 
			       2, 
			       TRUE);
    }

    /* reference field */
    if (identity->ref) {
	write_cyang_simple_str(scb, 
			       YANG_K_REFERENCE, 
			       identity->ref, 
			       indent, 
			       2, 
			       TRUE);
    }

    write_cyang_appinfoQ(scb, 
			 mod, 
			 cp, 
			 &identity->appinfoQ, 
			 indent);

    /* end identity clause */
    ses_putstr_indent(scb, END_SEC, startindent);

}  /* write_cyang_identity */


/********************************************************************
* FUNCTION write_cyang_identities
* 
* Generate the YANG for the specified identityQ
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == ncx_module_t struct in progress
*   cp == conversion parameters in use
*   identityQ == que of ncx_identity_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_identities (ses_cb_t *scb,
			    const ncx_module_t *mod,
			    const yangdump_cvtparms_t *cp,
			    const dlq_hdr_t *identityQ,
			    int32 startindent)
{
    const ncx_identity_t *identity;
    int32                 indent;

    if (dlq_empty(identityQ)) {
	return;
    }

    write_cyang_banner_cmt(scb, mod, cp,
			   (const xmlChar *)"identities", 
			   startindent);

    indent = startindent + ses_indent_count(scb);

    for (identity = (const ncx_identity_t *)
	     dlq_firstEntry(identityQ);
	 identity != NULL;
	 identity = (const ncx_identity_t *)
	     dlq_nextEntry(identity)) {

	write_cyang_identity(scb, mod, cp, 
			     identity, startindent);
    }

}  /* write_cyang_identities */


/********************************************************************
* FUNCTION write_cyang_feature
* 
* Generate the YANG for 1 feature statement
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == ncx_module_t struct in progress
*   cp == conversion parameters in use
*   feature == ncx_feature_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_feature (ses_cb_t *scb,
			 const ncx_module_t *mod,
			 const yangdump_cvtparms_t *cp,
			 const ncx_feature_t *feature,
			 int32 startindent)
{
    int32              indent;

    indent = startindent + ses_indent_count(scb);

    write_cyang_id(scb, YANG_K_FEATURE, 
		   feature->name, 
		   startindent, 
		   FALSE, FALSE);

    /* optional Q of if-feature statements */
    write_cyang_iffeatureQ(scb, &feature->iffeatureQ, indent);

    /* status field */
    write_cyang_status(scb, feature->status, indent);

    /* description field */
    if (feature->descr) {
	write_cyang_simple_str(scb, 
			       YANG_K_DESCRIPTION, 
			       feature->descr, 
			       indent, 
			       2, 
			       TRUE);
    }

    /* reference field */
    if (feature->ref) {
	write_cyang_simple_str(scb, 
			       YANG_K_REFERENCE, 
			       feature->ref, 
			       indent, 
			       2, 
			       TRUE);
    }

    write_cyang_appinfoQ(scb, 
			 mod, 
			 cp, 
			 &feature->appinfoQ, 
			 indent);

    /* end feature clause */
    ses_putstr_indent(scb, END_SEC, startindent);

}  /* write_cyang_feature */


/********************************************************************
* FUNCTION write_cyang_features
* 
* Generate the YANG for the specified featureQ
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == ncx_module_t struct in progress
*   cp == conversion parameters in use
*   featureQ == que of ncx_feature_t to use
*   startindent == start indent count
*
*********************************************************************/
static void
    write_cyang_features (ses_cb_t *scb,
			  const ncx_module_t *mod,
			  const yangdump_cvtparms_t *cp,
			  const dlq_hdr_t *featureQ,
			  int32 startindent)
{
    const ncx_feature_t *feature;

    if (dlq_empty(featureQ)) {
	return;
    }

    write_cyang_banner_cmt(scb, mod, cp,
			   (const xmlChar *)"features", 
			   startindent);

    for (feature = (const ncx_feature_t *)
	     dlq_firstEntry(featureQ);
	 feature != NULL;
	 feature = (const ncx_feature_t *)
	     dlq_nextEntry(feature)) {

	write_cyang_feature(scb, mod, cp, 
			    feature, startindent);
    }

}  /* write_cyang_features */


/********************************************************************
* FUNCTION write_cyang_import
* 
* Generate the YANG for an import statement
*
* INPUTS:
*   scb == session control block to use for writing
*   mod == module in progress
*   cp == conversion parameters to use
*   modprefix == module prefix value for import
*   modname == module name to use in import clause
*   modrevision == module revision date (may be NULL)
*   appinfoQ  == import appinfo Q (may be NULL)
*   indent == start indent count
*
*********************************************************************/
static void
    write_cyang_import (ses_cb_t *scb,
			const ncx_module_t *mod,
			const yangdump_cvtparms_t *cp,
			const xmlChar *modprefix,
			const xmlChar *modname,
			const xmlChar *modrevision,
			const dlq_hdr_t *appinfoQ,
			int32 indent)
{
    ses_indent(scb, indent);
    ses_putstr(scb, YANG_K_IMPORT);
    ses_putchar(scb, ' ');
    ses_putstr(scb, modname);
    ses_putstr(scb, START_SEC);
    write_cyang_simple_str(scb, 
			   YANG_K_PREFIX,
			   modprefix,
			   indent + ses_indent_count(scb), 
			   2, 
			   TRUE);
    if (modrevision) {
	write_cyang_simple_str(scb, 
			       YANG_K_REVISION, 
			       modrevision,
			       indent + ses_indent_count(scb), 
			       2, 
			       TRUE);

    }
    if (appinfoQ) {
	write_cyang_appinfoQ(scb, 
			     mod, 
			     cp, 
			     appinfoQ,
			     indent + ses_indent_count(scb));
    }
    ses_putstr_indent(scb, END_SEC, indent);

}  /* write_cyang_import */


/********************************************************************
* FUNCTION write_cyang_header
* 
* Generate the YANG for the module header info
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
    write_cyang_header (ses_cb_t *scb,
			const ncx_module_t *mod,
			const yangdump_cvtparms_t *cp)
{
    const ncx_import_t       *imp;
    const yang_import_ptr_t  *impptr;
    const ncx_include_t      *inc;
    const ncx_revhist_t      *rev;
    char                      buff[NCX_MAX_NUMLEN];
    int                       indent;

    /* [sub]module name { */
    if (mod->ismod) {
	ses_putstr(scb, YANG_K_MODULE);
    } else {
	ses_putstr(scb, YANG_K_SUBMODULE);
    }
    ses_putchar(scb, ' ');
    ses_putstr(scb, mod->name);
    ses_putstr(scb, START_SEC);
    ses_putchar(scb, '\n');

    /* set indent count to one level */
    indent = ses_indent_count(scb);

    /* yang-version */
    sprintf(buff, "%u", mod->langver);
    write_cyang_simple_str(scb, 
			   YANG_K_YANG_VERSION, 
			   (const xmlChar *)buff, 
			   indent, 
			   0, 
			   TRUE);
    ses_putchar(scb, '\n');

    /* namespace or belongs-to */
    if (mod->ismod) {
	write_cyang_simple_str(scb, 
			       YANG_K_NAMESPACE, 
			       mod->ns,
			       indent, 
			       2, 
			       TRUE);
    } else {
	write_cyang_simple_str(scb, 
			       YANG_K_BELONGS_TO, 
			       mod->belongs,
			       indent, 
			       2, 
			       TRUE);
    }
    ses_putchar(scb, '\n');

    /* prefix for module only */ 
    if (mod->ismod) {
	write_cyang_simple_str(scb, 
			       YANG_K_PREFIX,
			       mod->prefix,
			       indent,
			       2,
			       TRUE);
    }

    /* blank line */
    ses_putchar(scb, '\n');

    /* imports section */
    if (cp->unified) {
	for (impptr = (const yang_import_ptr_t *)
		 dlq_firstEntry(&mod->saveimpQ);
	     impptr != NULL;
	     impptr = (const yang_import_ptr_t *)
		 dlq_nextEntry(impptr)) {

	    /* the appinfoQ info is not saved in unified mode ouput!! */
	    write_cyang_import(scb, 
			       mod, 
			       cp, 
			       impptr->modprefix,
			       impptr->modname, 
			       impptr->revision, 
			       NULL, 
			       indent);
	}
	if (!dlq_empty(&mod->saveimpQ)) {
	    ses_putchar(scb, '\n');
	}
    } else {
	for (imp = (const ncx_import_t *)
		 dlq_firstEntry(&mod->importQ);
	     imp != NULL;
	     imp = (const ncx_import_t *)
		 dlq_nextEntry(imp)) {

	    write_cyang_import(scb, 
			       mod, 
			       cp, 
			       imp->prefix,
			       imp->module, 
			       imp->revision,
			       &imp->appinfoQ, 
			       indent);

	}
	if (!dlq_empty(&mod->importQ)) {
	    ses_putchar(scb, '\n');
	}
    }

    /* includes section	*/
    if (!cp->unified) {
	for (inc = (const ncx_include_t *)
		 dlq_firstEntry(&mod->includeQ);
	     inc != NULL;
	     inc = (const ncx_include_t *)dlq_nextEntry(inc)) {
	    ses_putstr_indent(scb, YANG_K_INCLUDE, indent);
	    ses_putchar(scb, ' ');
	    ses_putstr(scb, inc->submodule);
	    if (inc->revision || !dlq_empty(&inc->appinfoQ)) {
		ses_putstr(scb, START_SEC);
		if (inc->revision) {
		    write_cyang_simple_str(scb, 
					   YANG_K_REVISION, 
					   inc->revision,
					   indent + ses_indent_count(scb), 
					   2, 
					   TRUE);
		}
		write_cyang_appinfoQ(scb, 
				     mod, 
				     cp, 
				     &inc->appinfoQ,
				     indent + ses_indent_count(scb));
		ses_putstr_indent(scb, END_SEC, indent);
	    } else {
		ses_putchar(scb, ';');
	    }
	}
	if (!dlq_empty(&mod->includeQ)) {
	    ses_putchar(scb, '\n');
	}
    }

    /* organization */
    if (mod->organization) {
	write_cyang_simple_str(scb, 
			       YANG_K_ORGANIZATION,
			       mod->organization, 
			       indent, 
			       2, 
			       TRUE);
	ses_putchar(scb, '\n');	
    }

    /* contact */
    if (mod->contact_info) {
	write_cyang_simple_str(scb, 
			       YANG_K_CONTACT,
			       mod->contact_info, 
			       indent, 
			       2, 
			       TRUE);
	ses_putchar(scb, '\n');
    }

    /* description */
    if (mod->descr) {
	write_cyang_simple_str(scb, 
			       YANG_K_DESCRIPTION,
			       mod->descr, 
			       indent, 
			       2, 
			       TRUE);
	ses_putchar(scb, '\n');
    }

    /* reference */
    if (mod->ref) {
	write_cyang_simple_str(scb, 
			       YANG_K_REFERENCE,
			       mod->ref,
			       indent,
			       2,
			       TRUE);
	ses_putchar(scb, '\n');
    }

    /* revision history section	*/
    for (rev = (const ncx_revhist_t *)
	     dlq_firstEntry(&mod->revhistQ);
	 rev != NULL;
	 rev = (const ncx_revhist_t *)dlq_nextEntry(rev)) {

	write_cyang_simple_str(scb, 
			       YANG_K_REVISION,
			       rev->version,
			       indent, 
			       2, 
			       FALSE);

	write_cyang_simple_str(scb, 
			       YANG_K_DESCRIPTION,
			       (rev->descr) ? rev->descr : EMPTY_STRING,
			       indent + ses_indent_count(scb), 
			       2, 
			       TRUE);

	ses_putstr_indent(scb, END_SEC, indent);
    }
    if (!dlq_empty(&mod->revhistQ)) {
	ses_putchar(scb, '\n');
    }

} /* write_cyang_header */


/********************************************************************
* FUNCTION write_cyang_module
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
    write_cyang_module (ses_cb_t *scb,
			const ncx_module_t *mod,
			const yangdump_cvtparms_t *cp)
{
    const yang_node_t     *node;
    const yang_stmt_t     *stmt;
    boolean                stmtmode;

    write_cyang_header(scb, mod, cp);

    /* if the top-level statement order was saved, it was only for
     * the YANG_PT_TOP module, and none of the sub-modules
     */
    stmtmode = dlq_empty(&mod->stmtQ) ? FALSE : TRUE;

    /* 1) features */
    if (!stmtmode) {
	write_cyang_features(scb, mod, cp, 
			     &mod->featureQ, 
			     2*cp->indent);
    }

    if (cp->unified && mod->ismod) {
	for (node = (const yang_node_t *)dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (const yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_cyang_features(scb, 
				     node->submod, 
				     cp, 
				     &node->submod->featureQ, 
				     2*cp->indent);
	    }
	}
    }

    /* 2) identities */
    if (!stmtmode) {
	write_cyang_identities(scb, 
			       mod, 
			       cp, 
			       &mod->identityQ, 
			       cp->indent);
    }

    if (cp->unified && mod->ismod) {
	for (node = (const yang_node_t *)
		 dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (const yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_cyang_identities(scb, 
				       node->submod, 
				       cp, 
				       &node->submod->identityQ, 
				       cp->indent);
	    }
	}
    }

    /* 3) typedefs */
    if (!stmtmode) {
	write_cyang_typedefs(scb, 
			     mod, 
			     cp, 
			     &mod->typeQ, 
			     cp->indent);
    }

    if (cp->unified && mod->ismod) {
	for (node = (const yang_node_t *)
		 dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (const yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_cyang_typedefs(scb, 
				     node->submod, 
				     cp, 
				     &node->submod->typeQ, 
				     cp->indent);
	    }
	}
    }

    /* 4) groupings */
    if (!stmtmode) {
	write_cyang_groupings(scb, 
			      mod, 
			      cp, 
			      &mod->groupingQ, 
			      cp->indent);
    }

    if (cp->unified && mod->ismod) {
	for (node = (const yang_node_t *)
		 dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (const yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_cyang_groupings(scb, 
				      node->submod, 
				      cp, 
				      &node->submod->groupingQ, 
				      cp->indent);
	    }
	}
    }

    /* 5) extensions */
    if (!stmtmode) {
	write_cyang_extensions(scb, 
			       mod, 
			       cp, 
			       &mod->extensionQ, 
			       cp->indent);
    }

    if (cp->unified && mod->ismod) {
	for (node = (const yang_node_t *)
		 dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (const yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_cyang_extensions(scb, 
				       node->submod, 
				       cp, 
				       &node->submod->extensionQ, 
				       cp->indent);
	    }
	}
    }

    /* 6) objects */
    if (!stmtmode) {
	write_cyang_objects(scb, 
			    mod, 
			    cp, 
			    &mod->datadefQ, 
			    cp->indent);
    }

    if (cp->unified && mod->ismod) {
	for (node = (const yang_node_t *)
		 dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (const yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_cyang_objects(scb, 
				    node->submod,
				    cp, 
				    &node->submod->datadefQ, 
				    cp->indent);
	    }
	}
    }

    /* check statement mode on top-level module only */
    if (stmtmode) {
	for (stmt = (const yang_stmt_t *)
		 dlq_firstEntry(&mod->stmtQ);
	     stmt != NULL;
	     stmt = (const yang_stmt_t *)dlq_nextEntry(stmt)) {
	    switch (stmt->stmttype) {
	    case YANG_ST_NONE:
		SET_ERROR(ERR_INTERNAL_VAL);
		break;
	    case YANG_ST_TYPEDEF:
		write_cyang_typedef(scb,
				    mod,
				    cp, 
				    stmt->s.typ,
				    cp->indent, 
				    FALSE);
		break;
	    case YANG_ST_GROUPING:
		write_cyang_grouping(scb,
				     mod,
				     cp, 
				     stmt->s.grp, 
				     cp->indent, 
				     FALSE);
		break;
	    case YANG_ST_EXTENSION:
		write_cyang_extension(scb,
				      mod,
				      cp, 
				      stmt->s.ext,
				      cp->indent);
		break;
	    case YANG_ST_OBJECT:
		write_cyang_object(scb,
				   mod,
				   cp, 
				   stmt->s.obj, 
				   cp->indent,
				   FALSE);
		break;
	    case YANG_ST_IDENTITY:
		write_cyang_identity(scb,
				     mod,
				     cp, 
				     stmt->s.identity, 
				     cp->indent);
		break;
	    case YANG_ST_FEATURE:
		write_cyang_feature(scb, 
				    mod,
				    cp, 
				    stmt->s.feature, 
				    cp->indent);
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
    }
    
    /* TBD: need a better way to generate all the top-level
     * extensions.  This approach is broken because it gathers
     * them and puts them at the end
     *
     * will need to check line numbers as other constructs
     * are generated to find any extensions that 'fit'
     * in particuolar line number ranges
     */
    write_cyang_appinfoQ(scb, mod, cp, &mod->appinfoQ, cp->indent);

    if (cp->unified && mod->ismod) {
	for (node = (const yang_node_t *)dlq_firstEntry(&mod->saveincQ);
	     node != NULL;
	     node = (const yang_node_t *)dlq_nextEntry(node)) {
	    if (node->submod) {
		write_cyang_appinfoQ(scb, 
				     node->submod,
				     cp, 
				     &node->submod->appinfoQ,
				     cp->indent);
	    }
	}
    }

    /* end module */
    ses_putstr(scb, (const xmlChar *)"\n}\n");

} /* write_cyang_module */


/*********     E X P O R T E D   F U N C T I O N S    **************/


/********************************************************************
* FUNCTION cyang_convert_module
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
    cyang_convert_module (const yang_pcb_t *pcb,
			  const yangdump_cvtparms_t *cp,
			  ses_cb_t *scb)
{
    const ncx_module_t  *mod;

    /* the module should already be parsed and loaded */
    mod = pcb->top;
    if (!mod) {
	return SET_ERROR(ERR_NCX_MOD_NOT_FOUND);
    }

    write_cyang_module(scb, mod, cp);

    return NO_ERR;

}   /* cyang_convert_module */


/* END file cyang.c */
