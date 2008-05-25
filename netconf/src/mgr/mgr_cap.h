#ifndef _H_mgr_cap
#define _H_mgr_cap

/*  FILE: mgr_cap.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Manager capabilities handler

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

extern void 
    mgr_cap_cleanup (void);

extern status_t 
    mgr_cap_set_caps (void);

extern cap_list_t * 
    mgr_cap_get_caps (void);

extern val_value_t * 
    mgr_cap_get_capsval (void);


#endif	    /* _H_mgr_cap */
