#ifndef _H_agt_ps_wr
#define _H_agt_ps_wr

/*  FILE: agt_ps_wr.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Parmset session output handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
28-apr-06    abb      Begun
28-aug-06    abb      split from agt_ps.c
*/

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    agt_ps_check_write_nc (ses_cb_t *scb,
			   xml_msg_hdr_t *msg,
			   const ps_parmset_t *ps,
			   int32  indent,
			   boolean useacl,
			   ncx_nodetest_fn_t testfn);

extern void
    agt_ps_write_nc (ses_cb_t *scb,
		     xml_msg_hdr_t *msg,
		     const ps_parmset_t *ps,
		     int32  indent,
		     boolean useacl);


#endif	    /* _H_agt_ps_wr */
