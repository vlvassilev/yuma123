#ifndef _H_db
#define _H_db
/*  FILE: db.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    database front-end code

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
22-apr-05    abb      Begun; started from gr8cxt2/db.h

*/

#ifndef _H_procdefs
#include "procdefs.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/
#define QBUFF_SIZE         0xC000
#define MYSQL_OK           0
#define DB_MAX_TBLNAME_LEN  63
#define DB_MAX_COLNAME_LEN  63
#define DB_MAX_INDEX_CNT    64
#define DB_MAX_COLUMN_CNT   1024


/* db_handle.htyp field values */
#define DB_HTYP_OPEN    1
#define DB_HTYP_CLOSED  2


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* handle struct used by db_getFirstOrder and db_getNextOrder
 * to maintain state between function calls
 */
typedef struct db_handle_t_ {
    int   htyp;            /* DB_HTYP_OPEN or DB_HTYP_CLOSED */
    void  *presult;
    int   num_rows;
    int   pos;   /* row 0 .. N-1 */
} db_handle_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t  db_init (void);

extern void      db_cleanup (void);

extern void      db_capture_error (void);

extern void      db_print_error (void);


#endif	    /* _H_db */
