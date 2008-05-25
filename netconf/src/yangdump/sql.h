#ifndef _H_sql
#define _H_sql

/*  FILE: sql.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Convert NCX module to SQL format for the netconfcentral.com DB
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
18-feb-08    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yangdump
#include "yangdump.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* SQL database definition limits */
#define MAX_VERSION_LEN   32
#define MAX_URL_LEN       255
#define MAX_OBJECTID_LEN  900


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

extern status_t 
    sql_convert_module (const yang_pcb_t *pcb,
			const yangdump_cvtparms_t *cp,
			ses_cb_t *scb);

#endif	    /* _H_sql */
