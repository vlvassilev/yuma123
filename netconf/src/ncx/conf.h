#ifndef _H_conf
#define _H_conf

/*  FILE: conf.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Text Config file parser

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
22-oct-07    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/
extern status_t 
    conf_parse_ps_from_filespec (const xmlChar *filespec,
				 ps_parmset_t *ps,
				 boolean keepvals,
				 boolean fileerr);


#endif	    /* _H_conf */
