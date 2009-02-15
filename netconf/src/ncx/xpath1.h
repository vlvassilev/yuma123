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

#ifndef _H_dlq
#include "dlq.h"
#endif

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

/* parse initial expr with YANG prefixes: must/when
 * the object is left out in case it is in a grouping
 */
extern status_t
    xpath1_parse_expr (tk_chain_t *tkc,
		       ncx_module_t *mod,
		       xpath_pcb_t *pcb,
		       xpath_source_t source);

/* parse expr with YANG prefixes: must/when
 * called from final OBJ xpath check after all
 * cooked objects are in place
 */
extern status_t
    xpath1_validate_expr (ncx_module_t *mod,
			  const obj_template_t *obj,
			  xpath_pcb_t *pcb);

/* use if the prefixes are YANG: must/when */
extern xpath_result_t *
    xpath1_eval_expr (xpath_pcb_t *pcb,
		      val_value_t *val,
		      val_value_t *docroot,
		      boolean logerrors,
		      boolean configonly,
		      status_t *res);

/* use if the prefixes are XML: select */
extern xpath_result_t *
    xpath1_eval_xmlexpr (xmlTextReaderPtr reader,
			 xpath_pcb_t *pcb,
			 val_value_t *val,
			 val_value_t *docroot,
			 boolean logerrors,
			 boolean configonly,
			 status_t *res);

#ifdef KEEP_AROUND_UNTIL_LEAFREF_TESTED
extern xpath_result_t *
    xpath1_eval_xml_instanceid (xmlTextReaderPtr reader,
				xpath_pcb_t *pcb,
				val_value_t *val,
				val_value_t *docroot,
				boolean logerrors,
				status_t *res);
#endif

extern const xpath_fncb_t *
    xpath1_get_functions_ptr (void);

extern void
    xpath1_prune_nodeset (xpath_pcb_t *pcb,
			  xpath_result_t *result);

extern boolean
    xpath1_check_node_exists (xpath_pcb_t *pcb,
			      dlq_hdr_t *resultQ,
			      val_value_t *val);

extern status_t
    xpath1_stringify_nodeset (xpath_pcb_t *pcb,
			      const xpath_result_t *result,
			      xmlChar **str);

extern boolean
    xpath1_compare_result_to_string (xpath_pcb_t *pcb,
				     xpath_result_t *result,
				     xmlChar *strval,
				     status_t *res);


extern boolean
    xpath1_compare_result_to_number (xpath_pcb_t *pcb,
				     xpath_result_t *result,
				     ncx_num_t *numval,
				     status_t *res);

#endif	    /* _H_xpath1 */
