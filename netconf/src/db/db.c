/*  FILE: db.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
22apr05      abb      start with gr8cxt2/db.c and convert it
                      to use ODBC instead of mySQL directly

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
/* #include  <sys/time.h> */
/* #include  <sys/types.h> */
/* #include  <sys/stat.h> */
/* #include  <limits.h> */
/* #include  <errno.h> */
#include  <memory.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_db_config
#include "db_config.h"
#endif

#ifndef _H_db
#include "db.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define ODBC_ERR_BUFF_LEN    256

/* #define DB_DEBUG 1 */

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean        db_init_done = FALSE;


static SQLHENV	      h_odbc_env;     /* Handle ODBC environment */
static SQLHDBC	      h_odbc_hdbc;    /* Handle connection */


/* unixODBC error info */
static SQLCHAR     odbc_stat[ODBC_ERR_BUFF_LEN];
static SQLCHAR     odbc_msg[ODBC_ERR_BUFF_LEN];
static SQLINTEGER  odbc_err;
static SQLSMALLINT odbc_mlen;


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION db_init
* 
* Initialize the db module (get theODBC and other settings)
*
* INPUTS:
*   none
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
extern status_t db_init (void)
{
    status_t   res;
    db_config_settings_t  *pconfig;
    long   odbc_res;    

    if (db_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    } 

    /* init module static variables */
    odbc_stat[0] = 0;
    odbc_msg[0] = 0;
    odbc_err = 0;
    odbc_mlen = 0;

    /* get the DB server config settings */
    res = db_config_init();
    if (res != NO_ERR) {
	return res;
    }

    pconfig = db_config_get_settings();
    if (!pconfig) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* 1. allocate Environment handle and register version */
    odbc_res = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &h_odbc_env);
    if ((odbc_res != SQL_SUCCESS) && (odbc_res != SQL_SUCCESS_WITH_INFO)) {
        return SET_ERROR(ERR_DB_INIT_FAILED);
    }
	
    odbc_res = SQLSetEnvAttr(h_odbc_env, SQL_ATTR_ODBC_VERSION, 
			     (void*)SQL_OV_ODBC3, 0); 
    if ((odbc_res != SQL_SUCCESS) && (odbc_res != SQL_SUCCESS_WITH_INFO)) {
	SQLFreeHandle(SQL_HANDLE_ENV, h_odbc_env);
        return SET_ERROR(ERR_DB_INIT_FAILED);
    }

    /* 2. allocate connection handle, set timeout */
    odbc_res = SQLAllocHandle(SQL_HANDLE_DBC, h_odbc_env, &h_odbc_hdbc); 
    if ((odbc_res != SQL_SUCCESS) && (odbc_res != SQL_SUCCESS_WITH_INFO)) {
	SQLFreeHandle(SQL_HANDLE_ENV, h_odbc_env);
        return SET_ERROR(ERR_DB_INIT_FAILED);
    }

    SQLSetConnectAttr(h_odbc_hdbc, SQL_LOGIN_TIMEOUT, 
		      (SQLPOINTER *)pconfig->odbc_timeout, 0);

    /* 3. Connect to the datasource */
    odbc_res = SQLConnect(h_odbc_hdbc, 
			  pconfig->odbc_def_db, SQL_NTS,
			  pconfig->odbc_user_name, SQL_NTS,
			  pconfig->odbc_user_passwd, SQL_NTS);
    if ((odbc_res != SQL_SUCCESS) && (odbc_res != SQL_SUCCESS_WITH_INFO)) {
	db_capture_error();
	SQLFreeHandle(SQL_HANDLE_DBC, h_odbc_hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, h_odbc_env);
        return SET_DB_ERROR(ERR_DB_CONNECT_FAILED);
    }

    db_init_done = TRUE;

    return NO_ERR;

}  /* db_init */


/********************************************************************
* FUNCTION db_cleanup
*
*  close ODBC server connection
*********************************************************************/
void  db_cleanup (void)
{
    if (db_init_done) {
	SQLDisconnect(h_odbc_hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, h_odbc_hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, h_odbc_env);
	db_init_done = 0;
    }
}   /* db_cleanup */


/********************************************************************
* FUNCTION db_capture_error
* 
* Capture the ODBC server error info (if any)
*
* INPUTS:
*   none
* RETURNS:
*   none
*********************************************************************/
extern void db_capture_error (void)
{
    if (db_init_done) {
        SQLGetDiagRec(SQL_HANDLE_DBC, h_odbc_hdbc, 1, 
		  odbc_stat, &odbc_err, odbc_msg, 100, &odbc_mlen);
    }

} /* db_capture_error */


/********************************************************************
* FUNCTION db_print_error
* 
* Print the ODBC server error info (if any)
*
* INPUTS:
*   none
* RETURNS:
*   none
*********************************************************************/
extern void db_print_error (void)
{
    if (db_init_done) {
        printf("\nODBC Error (%ld): %s\n", odbc_err, odbc_msg);
    }

} /* db_print_error */


/* END file db.c */
