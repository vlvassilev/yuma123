/*  FILE: dbtest.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
22-apr-05    abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_db
#include  "db.h"
#endif

#ifndef _H_db_table
#include  "db_table.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define  APP_NAME    "ncxmain"


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static db_table_t   test_tbl;


/********************************************************************
 * FUNCTION setup_table
 * 
 * Simple test for the db_table functionality based on the
 * 'testac' table in the 'test' database within mySQL
 *********************************************************************/
static status_t setup_table (void)
{
    status_t   res;

    res = db_table_init_tbl("testac", 0, 2, 0, &test_tbl);
    if (res != NO_ERR) {
	return res;
    }

    res = db_table_set_numcol(&test_tbl, 1, "object_id",
	      DB_COLTYP_COLUMN, DB_VALTYP_INT, NULL, 0);
    if (res != NO_ERR) {
	db_table_free_tbl(&test_tbl);
	return res;
    }

    res = db_table_set_strcol(&test_tbl, 2, "object_title",
	      DB_COLTYP_COLUMN, NULL, DB_COPYTYP_VALUE, 65, 0);
    if (res != NO_ERR) {
	db_table_free_tbl(&test_tbl);
	return res;
    }

    return NO_ERR;

}


/********************************************************************
 * FUNCTION cleanup_table
 * 
 * Simple test for the db_table functionality based on the
 * 'testac' table in the 'test' database within mySQL
 *********************************************************************/
static void cleanup_table (void)
{
    db_table_free_tbl(&test_tbl);
}


/********************************************************************
*                                                                   *
*			FUNCTION main				    *
*                                                                   *
*********************************************************************/
/*ARGSUSED*/
int main (int argc, char *argv[])
{
    status_t   res;

    res = db_init();

    if (res != NO_ERR) {
	print_errors();
	db_print_error();
    } else {
	printf("\nODBC Connect OK\n");
    }


    if (setup_table()==NO_ERR) {
	cleanup_table();
    }
    db_cleanup();

    return 0;
}

/* END dbtest.c */



