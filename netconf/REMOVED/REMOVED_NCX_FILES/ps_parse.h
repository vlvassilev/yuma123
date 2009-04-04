#ifndef _H_ps_parse
#define _H_ps_parse

/*  FILE: ps_parse.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    common parameter set XML parser module

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
21-oct-05    abb      Begun
09-feb-06    abb      Change from xmlTextReader to rpc_agt callback
                      API format
10-feb-07    abb      Split out common functions from agt_ps_parse.h

*/

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* value only of full test values for the valonly parameter */
#define  VALONLY   TRUE
#define  FULLTEST  FALSE


/* plain or script values for the climode parameter */
#define  SCRIPTMODE    TRUE
#define  PLAINMODE     FALSE

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

extern ps_parmset_t *
    ps_parse_cli (int argc, 
		  const char *argv[],
		  const psd_template_t *psd,
		  boolean valonly,
		  boolean script,
		  boolean autocomp,
		  status_t  *status);


extern status_t
    ps_parse_cli_parm (ps_parmset_t *ps,
		       psd_parm_t *parm,
		       const xmlChar *strval,
		       boolean script);


extern status_t
    ps_parse_add_defaults (ps_parmset_t *ps);

extern status_t
    ps_parse_add_clone (ps_parmset_t *ps,
			const ps_parm_t *curparm);

#endif	    /* _H_ps_parse */
