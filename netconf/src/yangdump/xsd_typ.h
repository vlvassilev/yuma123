#ifndef _H_xsd_typ
#define _H_xsd_typ

/*  FILE: xsd_typ.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Convert NCX Types to XSD format
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
24-nov-06    abb      Begun; split from xsd.c

*/

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
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

extern status_t
    xsd_add_types (const ncx_module_t *mod,
		   val_value_t *val);

extern status_t
    xsd_add_local_types (const ncx_module_t *mod,
			 val_value_t *val);

extern status_t
    xsd_finish_simpleType (const ncx_module_t *mod,
			   const typ_def_t *typdef,
			   val_value_t *val);

extern status_t
    xsd_finish_namedType (const ncx_module_t *mod,
			  const typ_def_t *typdef,
			  val_value_t *val);

extern status_t
    xsd_finish_union (const ncx_module_t *mod,
		      const typ_def_t *typdef,
		      val_value_t *val);

#endif	    /* _H_xsd_typ */
