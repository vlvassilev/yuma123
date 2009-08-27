#ifndef _H_yangcli_autoload
#define _H_yangcli_autoload

/*  FILE: yangcli_autoload.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
13-augr-09    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_yangcli
#include "yangcli.h"
#endif


/********************************************************************
*								    *
*		      F U N C T I O N S 			    *
*								    *
*********************************************************************/

extern status_t
    autoload_setup_tempdir (server_cb_t *server_cb,
                            ses_cb_t *scb);

extern status_t
    autoload_start_get_modules (server_cb_t *server_cb,
                                ses_cb_t *scb);

extern status_t
    autoload_handle_rpc_reply (server_cb_t *server_cb,
                               ses_cb_t *scb,
                               val_value_t *reply,
                               boolean anyerrors);

extern status_t
    autoload_compile_modules (server_cb_t *server_cb,
                              ses_cb_t *scb);

extern void
    autoload_handle_timeout_cleanup (server_cb_t *server_cb,
                                     ses_cb_t *scb);


#endif	    /* _H_yangcli_autoload */
