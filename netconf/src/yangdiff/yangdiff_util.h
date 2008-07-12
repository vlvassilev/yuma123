#ifndef _H_yangdiff_util
#define _H_yangdiff_util

/*  FILE: yangdiff_util.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Report differences utilities
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
10-jul-08    abb      Split out from yangdiff.c

*/

#include <xmlstring.h>

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_yangdiff
#include "yangdiff.h"
#endif


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    indent_in (yangdiff_diffparms_t *cp);

extern void
    indent_out (yangdiff_diffparms_t *cp);

extern uint32
    str_field_changed (const xmlChar *fieldname,
		       const xmlChar *oldstr,
		       const xmlChar *newstr,
		       boolean isrev,
		       yangdiff_cdb_t *cdb);

extern uint32
    bool_field_changed (const xmlChar *fieldname,
		       boolean oldbool,
		       boolean newbool,
		       boolean isrev,
			yangdiff_cdb_t *cdb);

extern uint32
    status_field_changed (const xmlChar *fieldname,
			  ncx_status_t oldstat,
			  ncx_status_t newstat,
			  boolean isrev,
			  yangdiff_cdb_t *cdb);

extern uint32
    prefix_field_changed (const ncx_module_t *oldmod,
			  const ncx_module_t *newmod,
			  const xmlChar *oldprefix,
			  const xmlChar *newprefix);

extern void
    output_val (yangdiff_diffparms_t *cp,
		const xmlChar *strval);

extern void
    output_mstart_line (yangdiff_diffparms_t *cp,
			const xmlChar *fieldname,
			const xmlChar *val,
			boolean isid);

extern void
    output_cdb_line (yangdiff_diffparms_t *cp,
		     const yangdiff_cdb_t *cdb);

extern void
    output_diff (yangdiff_diffparms_t *cp,
		 const xmlChar *fieldname,
		 const xmlChar *oldval,
		 const xmlChar *newval,
		 boolean isid);

extern void
    output_errinfo_diff (yangdiff_diffparms_t *cp,
			 const ncx_errinfo_t *olderr,
			 const ncx_errinfo_t *newerr);

extern uint32
    errinfo_changed (const ncx_errinfo_t *olderr,
		     const ncx_errinfo_t *newerr);

#endif	    /* _H_yangdiff_util */
