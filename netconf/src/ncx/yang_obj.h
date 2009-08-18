#ifndef _H_yang_obj
#define _H_yang_obj

/*  FILE: yang_obj.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    YANG Module parser support:

        - data-def-stmt support
          - container-stmt
          - leaf-stmt
          - leaf-list-stmt
          - list-stmt
          - choice-stmt
          - uses-stmt
          - augment-stmt


*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
11-def-07    abb      Begun; start from yang_parse.c

*/

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_grp
#include "grp.h"
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

#ifndef _H_yang
#include "yang.h"
#endif


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
    yang_obj_consume_datadef (yang_pcb_t *pcb,
                              tk_chain_t *tkc,
			      ncx_module_t  *mod,
			      dlq_hdr_t *que,
			      obj_template_t *parent);

extern status_t 
    yang_obj_consume_datadef_grp (yang_pcb_t *pcb,
                                  tk_chain_t *tkc,
				  ncx_module_t  *mod,
				  dlq_hdr_t *que,
				  obj_template_t *parent,
				  grp_template_t *grp);


extern status_t 
    yang_obj_consume_rpc (yang_pcb_t *pcb,
                          tk_chain_t *tkc,
			  ncx_module_t  *mod);

extern status_t 
    yang_obj_consume_notification (yang_pcb_t *pcb,
                                   tk_chain_t *tkc,
				   ncx_module_t  *mod);
extern status_t 
    yang_obj_consume_augment (yang_pcb_t *pcb,
                              tk_chain_t *tkc,
			      ncx_module_t  *mod);

extern status_t 
    yang_obj_consume_deviation (yang_pcb_t *pcb,
                                tk_chain_t *tkc,
				ncx_module_t  *mod);

extern status_t 
    yang_obj_resolve_datadefs (yang_pcb_t *pcb,
                               tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *datadefQ);


extern status_t 
    yang_obj_resolve_uses (yang_pcb_t *pcb,
                           tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   dlq_hdr_t *datadefQ);

extern status_t 
    yang_obj_resolve_augments (yang_pcb_t *pcb,
                               tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *datadefQ);

extern status_t 
    yang_obj_resolve_deviations (yang_pcb_t *pcb,
				 tk_chain_t *tkc,
				 ncx_module_t  *mod);

extern status_t 
    yang_obj_resolve_final (yang_pcb_t *pcb,
                            tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    dlq_hdr_t *datadefQ);

extern status_t 
    yang_obj_resolve_xpath (tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    dlq_hdr_t *datadefQ);

extern status_t 
    yang_obj_check_leafref_loops (tk_chain_t *tkc,
				  ncx_module_t  *mod,
				  dlq_hdr_t *datadefQ);

extern status_t
    yang_obj_remove_deleted_nodes (yang_pcb_t *pcb,
                                   tk_chain_t *tkc,
                                   ncx_module_t *mod,
                                   dlq_hdr_t *datadefQ);


#endif	    /* _H_yang_obj */
