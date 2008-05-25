#ifndef _H_agt_cli
#define _H_agt_cli

/*  FILE: agt_cli.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Command Line Interface handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
27-oct-06    abb      Begun

*/

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_status
#include "status.h"
#endif


/********************************************************************
*								    *
*			C O N S T A N T S
*								    *
*********************************************************************/

#define AGT_CLI_MODULE    (const xmlChar *)"netconfd"
#define AGT_CLI_APP       (const xmlChar *)"netconfd"
#define AGT_CLI_PSD       (const xmlChar *)"netconfd"


#define AGT_CLI_NOSTARTUP (const xmlChar *)"no-startup"
#define AGT_CLI_STARTUP   (const xmlChar *)"startup"


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    agt_cli_process_input (int argc,
			   const char *argv[],
			   agt_profile_t *agt_profile,
			   boolean *showver,
			   boolean *showhelp);

extern const ps_parmset_t *
    agt_cli_get_parmset (void);

extern void
    agt_cli_cleanup (void);

#endif	    /* _H_agt_cli */
