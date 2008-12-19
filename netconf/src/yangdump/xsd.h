#ifndef _H_xsd
#define _H_xsd

/*  FILE: xsd.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Convert NCX module to XSD format
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
15-nov-06    abb      Begun

*/

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yangdump
#include "yangdump.h"
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

/* module entry point: convert an entire module to a value struct
 * representing an XML element which represents an XSD
 */
extern status_t 
    xsd_convert_module (ncx_module_t *mod,
			yangdump_cvtparms_t *cp,
			val_value_t **retval,
			xml_attrs_t  *top_attrs);


extern status_t
    xsd_load_typenameQ (ncx_module_t *mod);

#endif	    /* _H_xsd */
