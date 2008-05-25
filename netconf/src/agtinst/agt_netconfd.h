#ifndef _H_agt_netconfd
#define _H_agt_netconfd

/*  FILE: agt_netconfd.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Parmset callback routines

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
29-apr-06    abb      Begun

*/

#ifndef _H_status
#include "status.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* application name */
#define NCX_APP_NCAGENT      ((const xmlChar *)"netconfd")

/* parmset names for the netconfd application that are
 * defined in the netconfd module
 */
#define NCX_PS_NC_BOOT      ((const xmlChar *)"netconfd")
#define NCX_PS_NC_SECURITY  ((const xmlChar *)"nc-security")
#define NCX_PS_NC_TRANSPORT ((const xmlChar *)"nc-transport")
#define NCX_PAR_CFG_PATH    ((const xmlChar *)"cfg-path")

#define NCX_TYP_FILELIST    ((const xmlChar *)"FileSpecList")

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_netconfd_init (void);

extern void 
    agt_netconfd_cleanup (void);

#endif	    /* _H_agt_netconfd */
