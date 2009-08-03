#ifndef _H_yang_typ
#define _H_yang_typ

/*  FILE: yang_typ.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    YANG Module parser typedef and type statement support

    
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
15-nov-07    abb      Begun; start from yang_parse.c

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
    yang_typ_consume_type (yang_pcb_t *pcb,
                           tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   typ_def_t *typdef);

extern status_t 
    yang_typ_consume_metadata_type (yang_pcb_t *pcb,
                                    tk_chain_t *tkc,
				    ncx_module_t  *mod,
				    typ_def_t *intypdef);

extern status_t 
    yang_typ_consume_typedef (yang_pcb_t *pcb,
                              tk_chain_t *tkc,
			      ncx_module_t  *mod,
			      dlq_hdr_t *que);


extern status_t 
    yang_typ_resolve_typedefs (yang_pcb_t *pcb,
                               tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *typeQ,
			       obj_template_t *parent);


extern status_t 
    yang_typ_resolve_typedefs_grp (yang_pcb_t *pcb,
                                   tk_chain_t *tkc,
				   ncx_module_t  *mod,
				   dlq_hdr_t *typeQ,
				   obj_template_t *parent,
				   grp_template_t *grp);

extern status_t 
    yang_typ_resolve_type (yang_pcb_t *pcb,
                           tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   typ_def_t *typdef,
			   const xmlChar *defval,
			   obj_template_t *obj);

extern status_t 
    yang_typ_rangenum_ok (typ_def_t *typdef,
			  const ncx_num_t *num);


#endif	    /* _H_yang_typ */
