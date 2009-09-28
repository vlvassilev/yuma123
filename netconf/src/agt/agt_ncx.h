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

#ifndef _H_ses
#include "ses.h"
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

extern status_t
    agt_ncx_cfg_save_inline (const xmlChar *source_url,
                             val_value_t *newroot);

extern status_t
    agt_ncx_load_backup (const xmlChar *filespec,
                         cfg_template_t *cfg,
                         ses_id_t  use_sid);

extern boolean
    agt_ncx_cc_active (void);

extern ses_id_t
    agt_ncx_cc_ses_id (void);

extern void
    agt_ncx_check_cc_timeout (void);

extern void
    agt_ncx_cancel_confirmed_commit (void);

#endif	    /* _H_agt_ncx */
