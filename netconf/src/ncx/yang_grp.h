#ifndef _H_yang_grp
#define _H_yang_grp

/*  FILE: yang_grp.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    YANG Module parser grouping statement support

    
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
14-dec-07    abb      Begun; start from yang_typ.h

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

/* 2nd pass parsing */
extern status_t 
    yang_grp_consume_grouping (tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *que,
			       obj_template_t *parent);

/* 3rd pass parsing */
extern status_t 
    yang_grp_resolve_groupings (tk_chain_t *tkc,
				ncx_module_t  *mod,
				dlq_hdr_t *groupingQ,
				obj_template_t *parent);

/* 4th pass parsing */
extern status_t 
    yang_grp_resolve_complete (tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *groupingQ,
			       obj_template_t *parent);

extern status_t 
    yang_grp_resolve_final (tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    dlq_hdr_t *groupingQ);

extern status_t 
    yang_grp_check_nest_loop (tk_chain_t *tkc,
			      ncx_module_t  *mod,
			      obj_template_t *obj,
			      grp_template_t *grp);


#endif	    /* _H_yang_grp */
