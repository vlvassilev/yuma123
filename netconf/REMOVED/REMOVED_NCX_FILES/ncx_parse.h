#ifndef _H_ncx_parse
#define _H_ncx_parse

/*  FILE: ncx_parse.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Module parser module

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
13-nov-05    abb      Begun
22-oct-07    abb      Moved parser utility functions to ncx.h

*/

#include <xmlstring.h>

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

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

extern ncx_module_t *
    ncx_parse_from_filespec (const xmlChar *filespec,
			     status_t  *res);


#endif	    /* _H_ncx_parse */
