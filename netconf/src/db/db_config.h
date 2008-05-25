#ifndef _H_db_config
#define _H_db_config
/*  FILE: db_config.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    database configuration handler code

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
22-apr-05    abb      Begun; started from gr8cxt2/db.h

*/

#ifndef _H_status
#include "status.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/
#define   DB_MAX_USERNAME_LEN    15
#define   DB_MAX_HOSTNAME_LEN    63
#define   DB_MAX_DBNAME_LEN      63
#define   DB_MAX_PASSWD_LEN      31


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/
typedef struct db_config_settings_t_
{
    uchar   odbc_server_name[DB_MAX_HOSTNAME_LEN+1];
    uchar   odbc_user_name[DB_MAX_USERNAME_LEN+1];
    uchar   odbc_user_passwd[DB_MAX_PASSWD_LEN+1];
    uchar   odbc_def_db[DB_MAX_DBNAME_LEN+1];
    uint32  odbc_timeout;

} db_config_settings_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/
extern status_t db_config_init (void);

extern db_config_settings_t *db_config_get_settings (void);

#endif	    /* _H_dbconfig */
