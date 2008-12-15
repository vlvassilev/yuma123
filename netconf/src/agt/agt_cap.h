#ifndef _H_agt_cap
#define _H_agt_cap

/*  FILE: agt_cap.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent capabilities handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
03-feb-06    abb      Begun; split out from base/cap.h

*/

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/
#ifdef WILL_CHANGE_TO_STD_SDISC_MODULE
extern status_t
    agt_cap_init (void);
#endif

extern void 
    agt_cap_cleanup (void);

extern status_t 
    agt_cap_set_caps (ncx_agttarg_t  agttarg,
		      ncx_agtstart_t agtstart);

extern status_t 
    agt_cap_set_modules (void);

extern status_t 
    agt_cap_add_module (const ncx_module_t *mod);

extern void
    agt_cap_set_modcaps_parmset (void);

extern cap_list_t * 
    agt_cap_get_caps (void);

extern val_value_t * 
    agt_cap_get_capsval (void);

extern boolean
    agt_cap_std_set (cap_stdid_t cap);

#endif	    /* _H_agt_cap */
