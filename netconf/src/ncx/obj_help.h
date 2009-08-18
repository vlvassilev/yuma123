#ifndef _H_obj_help
#define _H_obj_help

/*  FILE: obj_help.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Help command support for obj_template_t

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
17-aug-08    abb      Begun; split from obj.h to prevent H file loop

*/


#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    obj_dump_template (obj_template_t *obj,
		       help_mode_t mode,
		       uint32 nestlevel,
		       uint32 indent);

extern void
    obj_dump_datadefQ (dlq_hdr_t *datadefQ,
		       help_mode_t mode,
		       uint32 nestlevel,
		       uint32 indent);

#endif	    /* _H_obj_help */
