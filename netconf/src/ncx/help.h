#ifndef _H_help
#define _H_help

/*  FILE: help.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Print help text for various templates

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
05-oct-07    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    help_program_module (const xmlChar *modname,
			 const xmlChar *cliname,
			 boolean full);

extern void
    help_data_module (const ncx_module_t *mod,
		      boolean full);

extern void
    help_type (const typ_template_t *typ,
	       boolean full);

extern void
    help_parmset (const psd_template_t *psd,
		  boolean full);


extern void
    help_rpc (const rpc_template_t *rpc,
	      boolean full);

#ifdef NOT_YET
extern void
    help_notif (const obj_template_t *notif,
		boolean full);
#endif

extern void
    help_write_lines (const xmlChar *str,
		      uint32 indent,
		      boolean startnl);

#endif	    /* _H_help */
