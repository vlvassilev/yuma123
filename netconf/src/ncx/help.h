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

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif


/********************************************************************
*                                                                   *
*                            T Y P E S                              *
*                                                                   *
*********************************************************************/

typedef enum help_mode_t_ {
    HELP_MODE_NONE,
    HELP_MODE_BRIEF,
    HELP_MODE_NORMAL,
    HELP_MODE_FULL
} help_mode_t;



/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    help_program_module (const xmlChar *modname,
			 const xmlChar *cliname,
			 help_mode_t mode);

extern void
    help_data_module (const ncx_module_t *mod,
		      help_mode_t mode);

extern void
    help_type (const typ_template_t *typ,
	       help_mode_t mode);

extern void
    help_object (const obj_template_t *obj,
		 help_mode_t mode);

extern void
    help_write_lines (const xmlChar *str,
		      uint32 indent,
		      boolean startnl);

#endif	    /* _H_help */
