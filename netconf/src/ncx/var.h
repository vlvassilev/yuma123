#ifndef _H_var
#define _H_var

/*  FILE: var.h
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
23-aug-07    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_val
#include "val.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/


/* top or parm values for the istop parameter */
#define  ISTOP   TRUE
#define  ISPARM  FALSE

/* bits for ncx_var_t flags field */
#define VAR_FL_GLOBAL bit0
#define VAR_FL_SYSTEM bit1

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* struct of NCX user variable mapping for yangcli */
typedef struct ncx_var_t_ {
    dlq_hdr_t     hdr;
    uint32        flags;
    xmlChar      *name;
    val_value_t  *val;
} ncx_var_t;


/* values for isleft parameter in var_check_ref */

typedef enum var_side_t_ {
    ISRIGHT,
    ISLEFT
} var_side_t;



/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    var_set_str (const xmlChar *name,
		 uint32 namelen,
		 const val_value_t *value,
		 boolean isglobal);

extern status_t
    var_set (const xmlChar *name,
	     const val_value_t *value,
	     boolean isglobal);

extern void
    var_free (ncx_var_t *var);

extern status_t
    var_set_move (const xmlChar *name,
		  uint32 namelen,
		  boolean isglobal,
		  val_value_t *value);

extern status_t
    var_set_sys (const xmlChar *name,
		 const val_value_t *value);

extern const val_value_t *
    var_get_str (const xmlChar *name,
		 uint32 namelen,
		 boolean isglobal);

extern const val_value_t *
    var_get (const xmlChar *name,
	     boolean isglobal);

extern void
    var_unset (const xmlChar *name,
	       uint32 namelen,
	       boolean isglobal);

extern status_t
    var_check_ref (const xmlChar *line,
		   var_side_t side,
		   uint32   *len,
		   boolean *isglobal,
		   const xmlChar **name,
		   uint32 *namelen);

extern val_value_t *
    var_get_script_val (const obj_template_t *obj,
			val_value_t *val,
			const xmlChar *strval,
			boolean istop,
			status_t *res);


extern val_value_t *
    var_check_script_val (const obj_template_t *obj,
			  const xmlChar *strval,
			  boolean istop,
			  status_t *res);


extern status_t
    var_set_from_string (const xmlChar *name,
			 const xmlChar *valstr,
			 boolean isglobal);

#endif	    /* _H_var */
