#ifndef _H_agt_val_wr
#define _H_agt_val_wr

/*  FILE: agt_val_wr.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Write Value to session functions

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
20-may-06    abb      Begun

*/

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_op
#include "op.h"
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

#ifndef _H_xml_msg
#include "xml_msg.h"
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

/* output data node */
extern void
    agt_val_check_write_app_nc (ses_cb_t *scb,
				xml_msg_hdr_t *msg,
				const cfg_app_t *app,
				int32  indent,
				boolean useacl,
				ncx_nodetest_fn_t testfn);


/* output entire application node */
extern void
    agt_val_write_app_nc (ses_cb_t *scb,
			  xml_msg_hdr_t *msg,
			  const cfg_app_t *app,
			  int32  indent,
			  boolean useacl);


/* output val_value_t node contents only (w/filter) */
extern void
    agt_val_check_write_nc (ses_cb_t *scb,
			    xml_msg_hdr_t *msg,
			    const val_value_t *val,
			    int32  indent,
			    boolean useacl,
			    ncx_nodetest_fn_t testfn);

/* output val_value_t node contents only */
extern void
    agt_val_write_nc (ses_cb_t *scb,
		      xml_msg_hdr_t *msg,
		      const val_value_t *val,
		      int32 indent,
		      boolean useacl);

/* check if value can fit on 1 line */
extern boolean
    agt_val_oneline_nc (const val_value_t *val);


/* generate entire val_value_t *w/filter) */
extern void
    agt_val_full_check_write_nc (ses_cb_t *scb,
				 xml_msg_hdr_t *msg,
				 const val_value_t *val,
				 int32  indent,
				 boolean useacl,
				 ncx_nodetest_fn_t testfn);

extern void
    agt_val_full_write_nc (ses_cb_t *scb,
			   xml_msg_hdr_t *msg,
			   const val_value_t *val,
			   int32  indent,
			   boolean useacl);
		      
#endif	    /* _H_agt_val_wr */
