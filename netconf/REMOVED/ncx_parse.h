#ifndef _H_ncx_parse
#define _H_ncx_parse

/*  FILE: ncx_parse.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    <ncx-module> parser module

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
01-nov-05    abb      Begun

*/


#include <xmlreader.h>

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
ncx_parse_from_reader (xmlTextReaderPtr reader,
		       boolean *advance,
                       const char *source);

extern status_t 
ncx_parse_from_filespec (const char *filespec);

#endif	    /* _H_ncx_parse */
