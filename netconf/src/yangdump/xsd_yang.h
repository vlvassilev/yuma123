#ifndef _H_xsd_yang
#define _H_xsd_yang

/*  FILE: xsd_yang.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Convert YANG-specific constructs to XSD format
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
04-feb-08    abb      Begun; split from xsd_typ.h

*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    xsd_add_groupings (ncx_module_t *mod,
		       val_value_t *val);

extern status_t
    xsd_add_objects (ncx_module_t *mod,
		     val_value_t *val);

extern status_t
    xsd_do_typedefs_groupingQ (ncx_module_t *mod,
			       dlq_hdr_t *groupingQ,
			       dlq_hdr_t *typnameQ);

extern status_t
    xsd_do_typedefs_datadefQ (ncx_module_t *mod,
			      dlq_hdr_t *datadefQ,
			      dlq_hdr_t *typnameQ);

extern void
    xsd_clean_typnameQ (dlq_hdr_t *que);

#endif	    /* _H_xsd_yang */
