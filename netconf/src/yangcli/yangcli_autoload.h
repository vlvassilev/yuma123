#ifndef _H_yangcli_autoload
#define _H_yangcli_autoload

/*  FILE: yangcli_autoload.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
13-augr-09    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_yangcli
#include "yangcli.h"
#endif


/********************************************************************
*								    *
*		      F U N C T I O N S 			    *
*								    *
*********************************************************************/


extern status_t
    autoload_module (const xmlChar *modname,
                     const xmlChar *revision,
                     ncx_list_t *devlist,
                     ncx_module_t **retmod);


#endif	    /* _H_yangcli_autoload */
