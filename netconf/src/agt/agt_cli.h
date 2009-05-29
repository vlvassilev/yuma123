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
01-aug-08    abb      Convert from NCX PSD to YANG OBJ

*/

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_status
#include "status.h"
#endif


/********************************************************************
*								    *
*			C O N S T A N T S
*								    *
*********************************************************************/

#define AGT_CLI_MODULE     (const xmlChar *)"netconfd"
#define AGT_CLI_CONTAINER  (const xmlChar *)"netconfd"


#define AGT_CLI_NOSTARTUP (const xmlChar *)"no-startup"
#define AGT_CLI_STARTUP   (const xmlChar *)"startup"

#define AGT_CLI_SUPERUSER NCX_EL_SUPERUSER

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
			   help_mode_t *showhelpmode);

extern const val_value_t *
    agt_cli_get_valset (void);

extern void
    agt_cli_cleanup (void);

#endif	    /* _H_agt_cli */
