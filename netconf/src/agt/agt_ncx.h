#ifndef _H_agt_ncx
#define _H_agt_ncx

/*  FILE: agt_ncx.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent standard method routines

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
04-feb-06    abb      Begun

*/

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_ncx_init (void);

extern void 
    agt_ncx_cleanup (void);

extern status_t
    agt_ncx_cfg_load (cfg_template_t *cfg,
		      cfg_location_t cfgloc,
		      const xmlChar *cfgparm);

extern status_t
    agt_ncx_cfg_save (cfg_template_t *cfg,
		      boolean bkup);


#endif	    /* _H_agt_ncx */
