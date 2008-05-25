#ifndef _H_db_atble
#define _H_db_table
/*  FILE: db_table.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    database generic table handling code

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
23-apr-05    abb      Begun; started from gr8cxt2/db.h

*/

#ifndef _H_procdefs
#include "procdefs.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_db
#include "db.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/


/* db_val_t flags definitions */
#define  FL_VAL_COPY          bit0


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* the type of string copy that will be done */
typedef enum db_colcopy_t_ {
    DB_COPYTYP_NULL,       
    DB_COPYTYP_VALUE,
    DB_COPYTYP_REF
} db_colcopy_t;

/* the type of column definition */
typedef enum db_coltyp_t_ {
    DB_COLTYP_NULL,
    DB_COLTYP_INDEX,
    DB_COLTYP_COLUMN
} db_coltyp_t;

/* the column value type */
typedef enum db_col_valtyp_t_ {
    DB_VALTYP_NULL,
    DB_VALTYP_CHAR,
    DB_VALTYP_INT,
    DB_VALTYP_UINT,
    DB_VALTYP_LONG,
    DB_VALTYP_ULONG,
    DB_VALTYP_STRING,
    DB_VALTYP_BINARY
} db_col_valtyp_t;

/* the column value */    
typedef union db_col_val_t_ {
    char            c;
    int32           i;
    uint32          u;
    int32           l;
    uint32         ul;
    char           *s;
    unsigned char  *b;
} db_col_val_t;

/* the column control structure */    
typedef struct db_col_t_ {
    char            col_name[DB_MAX_COLNAME_LEN+1];
    uint32          col_flags;
    uint32    col_pos;     /* 1 .. N */
    uint32    col_str_maxlen;
    uint32    col_strlen;
    db_coltyp_t     col_type;
    db_col_valtyp_t col_valtyp;
    db_col_val_t    col_val;
    db_colcopy_t    col_copytyp;
} db_col_t;

/* the table control structure */
typedef struct db_table_t_ {
    char            tbl_name[DB_MAX_TBLNAME_LEN+1];
    uint32    tbl_flags;
    uint32    tbl_index_cnt;
    uint32    tbl_column_cnt;
    db_col_t        *tbl_indices;
    db_col_t        *tbl_columns;
} db_table_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* USE FIRST
 * call the db_table_init_tbl function to fill in the db_table_t
 * header and malloc the required index and column fields
 * At least 1 index or column must be indicated:
 *   (index_cnt + column_cnt) > 0
 */
extern status_t  db_table_init_tbl (
	    const char *tbl_name,
	    uint32 index_cnt,  /* 0 .. N */
	    uint32 column_cnt, /* 0 .. N */
	    uint32 tbl_flags,
	    db_table_t   *tbl);

extern void  db_table_free_tbl (db_table_t *tbl);

/* set a numeric index or normal column */

extern status_t  db_table_set_numcol (
	    db_table_t *tbl,
	    uint32 col_num,      /* 1..N position */
	    const char *col_name,
	    db_coltyp_t  col_typ,
	    db_col_valtyp_t  col_valtyp,
	    db_col_val_t *col_val,
	    uint32 col_flags);

/* set a text string index or normal column
 * copy_by_value == TRUE : malloc, FALSE : point 
 */
extern status_t  db_table_set_strcol (
	    db_table_t     *tbl,
	    uint32    col_num,
	    const char     *col_name,
	    db_coltyp_t     col_typ,
	    char           *col_val,
	    db_colcopy_t    copy_typ,
	    uint32    bufflen,
	    uint32          col_flags);

/* set a binary string index or normal column
 * copy_by_value == TRUE : malloc, FALSE : point 
 * value stored in the database will be converted
 * first with bin2blob.
 */
extern status_t  db_table_set_bincol (
	    db_table_t     *tbl,
	    uint32    col_num,
	    const char     *col_name,
	    db_coltyp_t     col_typ,
	    unsigned char   *col_val,
	    uint32    col_len,
	    db_colcopy_t    copy_typ,
	    uint32    bufflen,
	    uint32          col_flags);


#endif	    /* _H_db_table */
