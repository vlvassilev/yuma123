#ifndef _H_xpath1
#define _H_xpath1

/*  FILE: xpath1.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    XPath 1.0 expression support

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
13-nov-08    abb      Begun

*/

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
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
    xpath1_parse_expr (tk_chain_t *tkc,
		       ncx_module_t *mod,
		       xpath_pcb_t *pcb,
		       xpath_source_t source);

extern status_t
    xpath1_validate_expr (ncx_module_t *mod,
			  const obj_template_t *obj,
			  xpath_pcb_t *pcb);


extern xpath_result_t *
    xpath1_eval_expr (xpath_pcb_t *pcb,
		      val_value_t *val,
		      val_value_t *docroot,
		      boolean logerrors,
		      boolean configonly,
		      status_t *res);

extern xpath_result_t *
    xpath1_eval_xmlexpr (xmlTextReaderPtr reader,
			 xpath_pcb_t *pcb,
			 val_value_t *val,
			 val_value_t *docroot,
			 boolean logerrors,
			 boolean configonly,
			 status_t *res);

extern const xpath_fncb_t *
    xpath1_get_functions_ptr (void);

#endif	    /* _H_xpath1 */
