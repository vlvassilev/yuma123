#ifndef _H_yang_parse
#define _H_yang_parse

/*  FILE: yang_parse.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    YANG Module parser module


    Data type conversion

    
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
26-oct-07    abb      Begun; start from ncx_parse.h

*/

#include <xmlstring.h>

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

extern status_t 
    yang_parse_from_filespec (const xmlChar *filespec,
			 yang_pcb_t *pcb,
			      yang_parsetype_t ptyp);

#endif	    /* _H_yang_parse */
