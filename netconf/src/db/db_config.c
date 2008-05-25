/*  FILE: db_config.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
22apr05      abb      start with gr8cxt2/db.c and convert it

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>

#if 0
#include  <sys/time.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <limits.h>
#include  <errno.h>
#endif

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_db_config
#include  "db_config.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define DEF_ODBC_HOST            "localhost"
#define DEF_ODBC_USER            "nc_client"
#define DEF_ODBC_PASSWD          ""
#define DEF_ODBC_TIMEOUT         5
#define DEF_ODBC_DB              "netconf_env"


/* #define DB_CONFIG_DEBUG 1 */

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean              db_config_init_done = 0;
static db_config_settings_t db_config;


/********************************************************************
* FUNCTION db_config_init
* 
* Initialize the db_config module (get theODBC and other settings)
*
* INPUTS:
*   none
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
extern status_t db_config_init (void)
{
    if (db_config_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    } 

    /* fill in the db_config_settings struct with default values */
    xml_strcpy(db_config.odbc_server_name, (const uchar *)DEF_ODBC_HOST);
    xml_strcpy(db_config.odbc_user_name, (const uchar *)DEF_ODBC_USER);
    xml_strcpy(db_config.odbc_user_passwd, (const uchar *)DEF_ODBC_PASSWD);
    xml_strcpy(db_config.odbc_def_db, (const uchar *)DEF_ODBC_DB);
    db_config.odbc_timeout = DEF_ODBC_TIMEOUT;

    /* placeholder -- will get config variables from a config file */
    /***/

    db_config_init_done = TRUE;
    return NO_ERR;

}  /* db_config_init */


/********************************************************************
* FUNCTION db_config_get_settings
* 
* Retrieve the db_config settings
*
* INPUTS:
*   none
* RETURNS:
*   read-only pointer to the db_config_settings_t block
*********************************************************************/
extern db_config_settings_t *db_config_get_settings (void)
{
    if (!db_config_init_done) {
        (void)SET_ERROR(ERR_INTERNAL_INIT_SEQ);
	return NULL;
    } 
    return &db_config;

}  /* db_config_get_settings */


/* END file db_config.c */
