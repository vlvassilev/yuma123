#ifndef _H_typ_parse
#define _H_typ_parse

/*  FILE: typ_parse.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Parameter Type Parser Module

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
23-nov-05    abb      Begun; split out from typ.h

*/

#include <xmlstring.h>

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    typ_parse_syntax_contents (ncx_module_t *mod,
			       tk_chain_t *tkc,
			       typ_template_t *typ);

extern status_t 
    typ_parse_metadata_contents (ncx_module_t *mod,
				 tk_chain_t *tkc,
				 typ_template_t *typ);


#endif	    /* _H_typ_parse */
