#ifndef _H_cli
#define _H_cli

/*  FILE: cli.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    command line interpreter parsing to internal val_value_t format

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
29-jul-08    abb      change to cli.h; remove all by CLI parsing;
                      conversion from NCX parmset to YANG object
07-feb-09    abb      Add cli_rawparm_t and cli_parse_raw
                      for bootstrap CLI support

*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
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

/* used for bootstrap CLI parms only, no validation */
typedef struct cli_rawparm_t_ {
    dlq_hdr_t   qhdr;
    const char *name;
    char       *value;
    int32       count;
} cli_rawparm_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/*** bootstrap CLI support ***/
extern cli_rawparm_t *
    cli_new_rawparm (const xmlChar *name);

extern void
    cli_free_rawparm (cli_rawparm_t *parm);

extern void
    cli_clean_rawparmQ (dlq_hdr_t *parmQ);

extern cli_rawparm_t *
    cli_find_rawparm (const xmlChar *name,
		      dlq_hdr_t *parmQ);

extern status_t
    cli_parse_raw (int argc, 
		   const char *argv[],
		   dlq_hdr_t *rawparmQ);


/*** schema based CLI support ***/
extern val_value_t *
    cli_parse (int argc, 
	       const char *argv[],
	       const obj_template_t *obj,
	       boolean valonly,
	       boolean script,
	       boolean autocomp,
	       status_t  *status);


extern status_t
    cli_parse_parm (val_value_t *val,
		    const obj_template_t *obj,
		    const xmlChar *strval,
		    boolean script);


extern status_t
    cli_parse_parm_ex (val_value_t *val,
		       const obj_template_t *obj,
		       const xmlChar *strval,
		       boolean script,
		       ncx_bad_data_t  bad_data);

#endif	    /* _H_cli */
