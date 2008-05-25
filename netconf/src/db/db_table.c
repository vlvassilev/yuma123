/*  FILE: db_table.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
22apr05      abb      generic simple table processing 

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


/* macro to access a column node C in table T by 1..N column number */
#define get_col(T,C) 	&((T)->tbl_columns[(C)-1])

/* macro to access an index node I in table T by 1..N index number */
#define get_idx(T,I) 	&((T)->tbl_indices[(I)-1])

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define DB_TABLE_DEBUG 1 */

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION db_table_init_tbl
*
* first, call the init_tbl function to fill in the db_table_t
* header and malloc the required value fields
*
* INPUTS:
*  
* RETURNS:
*  
*********************************************************************/
status_t  db_table_init_tbl (
	    const char *tbl_name,
	    uint32 index_cnt,
	    uint32 column_cnt,
	    uint32 tbl_flags,
	    db_table_t   *tbl)
{
    unsigned int  len;

    /* check the parameters */
    if (!tbl_name || !tbl) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (index_cnt+column_cnt==0) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    if (index_cnt>DB_MAX_INDEX_CNT || column_cnt>DB_MAX_COLUMN_CNT) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    len = strlen(tbl_name);
    if (!len || len>DB_MAX_TBLNAME_LEN) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* initialize the db_table_t struct */
    (void)memset(tbl, 0x0, sizeof(db_table_t));
    (void)strcpy(tbl->tbl_name, tbl_name);
    tbl->tbl_flags = tbl_flags;
    tbl->tbl_index_cnt = index_cnt;
    tbl->tbl_column_cnt = column_cnt;

    /* allocate memory for the indices if any */
    len = sizeof(db_col_t) * index_cnt;
    if (len) {
	tbl->tbl_indices = (db_col_t *)m__getMem(len);
	if (!tbl->tbl_indices) {
	    return SET_ERROR(ERR_INTERNAL_MEM);
	}
	(void)memset(tbl->tbl_indices, 0x0, len);
    }

    /* allocate memory for the columns if any */
    len = sizeof(db_col_t) * column_cnt;
    if (len) {
	tbl->tbl_columns = (db_col_t *)m__getMem(len);
	if (!tbl->tbl_columns) {
	    if (tbl->tbl_indices) {
		m__free(tbl->tbl_indices);
		tbl->tbl_indices = NULL;
	    }
	    return SET_ERROR(ERR_INTERNAL_MEM);
	}
	(void)memset(tbl->tbl_columns, 0x0, len);
    }

    return NO_ERR;

} /* db_table_init_tbl */



/********************************************************************
* FUNCTION db_table_free_tbl
*
* free the db_table_t malloced fields and clear the memory
* does not free the memory used by the db_table_t itself
*
* INPUTS:
*    tbl == table header block to clean
* RETURNS:
*    none
*********************************************************************/
void  db_table_free_tbl (db_table_t *tbl)
{
    /* check the parameters */
    if (!tbl) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;   /* don't really care if this error is undetected */
    }
    if (tbl->tbl_indices) {
	m__free(tbl->tbl_indices);
    }
    if (tbl->tbl_columns) {
	m__free(tbl->tbl_columns);
    }
    (void)memset(tbl, 0x0, sizeof(db_table_t));

} /* db_table_free_tbl */


/********************************************************************
* FUNCTION db_table_set_numcol
*
* set a numeric index or normal column 
*
* INPUTS:
*    tbl == table header block to set
* RETURNS:
*    none
*********************************************************************/
status_t  db_table_set_numcol (
	    db_table_t *tbl,
	    uint32 col_num,      /* 1..N position */
	    const char *col_name,
	    db_coltyp_t  col_typ,
	    db_col_valtyp_t  col_valtyp,
	    db_col_val_t *col_val,
	    uint32 col_flags)
{
    int  len;
    db_col_t   *col;

    /* check the parameters */
    if (!tbl || !col_name) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    len = strlen(col_name);
    if (len==0 || len>DB_MAX_COLNAME_LEN) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* get the specified index or column */
    switch (col_typ) {
    case DB_COLTYP_INDEX:
	if (!tbl->tbl_indices || !col_num || col_num > tbl->tbl_index_cnt) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}
	col = get_idx(tbl, col_num);
	break;
    case DB_COLTYP_COLUMN:
	if (!tbl->tbl_columns || !col_num || col_num > tbl->tbl_column_cnt) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}
	col = get_col(tbl, col_num);
	break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check if this db_col_t entry is already set */
    switch (col->col_valtyp) {
    case DB_VALTYP_NULL:  /* setting this struct for the first time */
    case DB_VALTYP_CHAR:  /* rest are resetting a number field */
    case DB_VALTYP_INT:
    case DB_VALTYP_UINT:
    case DB_VALTYP_LONG:
    case DB_VALTYP_ULONG:
	break;  
    case DB_VALTYP_STRING:
    case DB_VALTYP_BINARY:
    default:
	/* calling the wrong function or field not initialized */
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check the val_type parameter and copy the value as needed */
    switch (col_valtyp) {
    case DB_VALTYP_NULL:
        return SET_ERROR(ERR_INTERNAL_VAL);
    case DB_VALTYP_CHAR:
	col->col_val.c = (col_val) ? col_val->c : 0;
	break;
    case DB_VALTYP_INT:
	col->col_val.i = (col_val) ? col_val->i : 0;
	break;
    case DB_VALTYP_UINT:
	col->col_val.u = (col_val) ? col_val->u : 0;
	break;
    case DB_VALTYP_LONG:
	col->col_val.l = (col_val) ? col_val->l : 0;
	break;
    case DB_VALTYP_ULONG:
	col->col_val.ul = (col_val) ? col_val->ul : 0;
	break;
    case DB_VALTYP_STRING:
    case DB_VALTYP_BINARY:
    default:
	/* wrong value for this parameter */
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* set the rest of the column fields */
    strcpy(col->col_name, col_name);
    col->col_flags = col_flags;
    col->col_pos = col_num;
    col->col_str_maxlen = 0;
    col->col_strlen = 0;
    col->col_type = col_typ;
    col->col_valtyp = col_valtyp;
    col->col_copytyp = DB_COPYTYP_NULL;

    return NO_ERR;
} /* db_table_set_numcol */


/********************************************************************
* FUNCTION db_table_set_strcol
*
* set a char string index or normal column for the first time
* Use db_table_update_strcol to change the value of a string 
* column.
*
* INPUTS:
*    tbl == table header block to set
*    col_name == valid SQL column name string
*    col_num == the 1..N position of the index or column
*    col_typ == type of column DB_COLTYP_INDEX or DB_COLTYP_COLUMN
*    col_val == if copy-by-value, then this is an optional initial 
*               value.  If copy-by-reference, then this is a mandatory
*               field to initially set the value pointer
*    copy_typ == DB_COPYTYP_VALUE or DB_COPYTYP_REF: if 'VALUE, then 
*                a buffer of size==bufflen will be malloced
*    bufflen == size of buffer to malloc; ignored unless copy_typ
*               is DB_COPYTYP_VALUE
*    col_flags == user extension flags, set to this initial value
* OUTPUTS:
*    the indicated column field within the 'tbl' data struct is 
*    initialized.
* RETURNS:
*    NO_ERR if all goes well, else error code (malloc or param error)
*********************************************************************/
status_t  db_table_set_strcol (
	    db_table_t     *tbl,
	    uint32    col_num,
	    const char     *col_name,
	    db_coltyp_t     col_typ,
	    char           *col_val,
	    db_colcopy_t    copy_typ,
	    uint32    bufflen,
	    uint32          col_flags)
{
    uint32  len;
    db_col_t     *col;

    /* check the parameters */
    if (!tbl || !col_name) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    len = strlen(col_name);
    if (len==0 || len>DB_MAX_COLNAME_LEN) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* get the specified index or column */
    switch (col_typ) {
    case DB_COLTYP_INDEX:
	if (!tbl->tbl_indices || !col_num || col_num > tbl->tbl_index_cnt) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}
	col = get_idx(tbl, col_num);
	break;
    case DB_COLTYP_COLUMN:
	if (!tbl->tbl_columns || !col_num || col_num > tbl->tbl_column_cnt) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}
	col = get_col(tbl, col_num);
	break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check if this db_col_t entry is already set */
    if (col->col_valtyp != DB_VALTYP_NULL) {
	return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    /* check the copy type and malloc and strcpy as needed */
    switch (copy_typ) {
    case DB_COPYTYP_VALUE:
	if (col_val) {
	    len = strlen(col_val);
	    if (len >= bufflen) {
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}

	/* create a buffer to hold a copy of any string value */
	col->col_val.s = (char *)m__getMem(bufflen);
	if (!col->col_val.s) {
	    /* out of memory */
	    return SET_ERROR(ERR_INTERNAL_MEM);
	} else {
	    /* clear the memory */
	    (void)memset(col->col_val.s, 0x0, bufflen);
	}
	/* copy the optional initial value, if any */
	if (col_val) {    
	    (void)strcpy(col->col_val.s, col_val);
	    col->col_strlen = len;
	} else {
	    col->col_strlen = 0;
	}
	break;
    case DB_COPYTYP_REF:
	/* just save the address of the col_val string or set
	 * the initial value to NULL if col_val is not set 
	 */
	col->col_val.s = col_val;
	if (col_val) {
	    col->col_strlen = strlen(col_val);
	} else {
	    col->col_strlen = 0;
	}
	break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* set the rest of the column fields */
    strcpy(col->col_name, col_name);
    col->col_flags = col_flags;
    col->col_pos = col_num;
    col->col_str_maxlen = bufflen-1;
    col->col_type = col_typ;
    col->col_valtyp = DB_VALTYP_STRING;
    col->col_copytyp = copy_typ;

    return NO_ERR;

} /* db_table_set_strcol */


/********************************************************************
* FUNCTION db_table_set_bincol
*
* set a binary string index or normal column for the first time
* Use db_table_update_bincol to change the value of a binary string 
* column.  Value stored in the database will be converted first 
* with bin2blob.
*
* INPUTS:
*    tbl == table header block to set
*    col_name == valid SQL column name string
*    col_num == the 1..N position of the index or column
*    col_typ == type of column DB_COLTYP_INDEX or DB_COLTYP_COLUMN
*    col_val == if copy-by-value, then this is an optional initial 
*               value.  If copy-by-reference, then this is an optional
*               field to initially set the value pointer.
*    col_len == length of *col_val in bytes
*    copy_typ == DB_COPYTYP_VALUE or DB_COPYTYP_REF: if 'VALUE, then 
*                a buffer of size==bufflen will be malloced
*    bufflen == size of buffer to malloc; ignored unless copy_typ
*               is DB_COPYTYP_VALUE
*    col_flags == user extension flags, set to this initial value
* OUTPUTS:
*    the indicated column field within the 'tbl' data struct is 
*    initialized.
* RETURNS:
*    NO_ERR if all goes well, else error code (malloc or param error)
*********************************************************************/
status_t  db_table_set_bincol (
	    db_table_t     *tbl,
	    uint32    col_num,
	    const char     *col_name,
	    db_coltyp_t     col_typ,
	    unsigned char   *col_val,
	    uint32    col_len,
	    db_colcopy_t    copy_typ,
	    uint32    bufflen,
	    uint32          col_flags)
{
    int         len;
    db_col_t   *col;

    /* check the parameters */
    if (!tbl || !col_name) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    len = strlen(col_name);
    if (len==0 || len>DB_MAX_COLNAME_LEN) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* get the specified index or column */
    switch (col_typ) {
    case DB_COLTYP_INDEX:
	if (!tbl->tbl_indices || !col_num || col_num > tbl->tbl_index_cnt) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}
	col = get_idx(tbl, col_num);
	break;
    case DB_COLTYP_COLUMN:
	if (!tbl->tbl_columns || !col_num || col_num > tbl->tbl_column_cnt) {
	    return SET_ERROR(ERR_INTERNAL_PTR);
	}
	col = get_col(tbl, col_num);
	break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check if this db_col_t entry is already set */
    if (col->col_valtyp != DB_VALTYP_NULL) {
	return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    /* check the copy type and malloc and strcpy as needed */
    switch (copy_typ) {
    case DB_COPYTYP_VALUE:
	if (col_len >= bufflen) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}

	/* create a buffer to hold a copy of any binary string value */
	col->col_val.b = (unsigned char *)m__getMem(bufflen);
	if (!col->col_val.b) {
	    /* out of memory */
	    return SET_ERROR(ERR_INTERNAL_MEM);
	} else {
	    /* clear the memory */
	    (void)memset(col->col_val.b, 0x0, bufflen);
	}
	/* copy the optional initial value, if any */
	if (col_val) {    
	    (void)memcpy(col->col_val.b, col_val, col_len);
	    col->col_strlen = col_len;
	} else {
	    col->col_strlen = 0;
	}
	break;
    case DB_COPYTYP_REF:
	/* just save the address of the col_val string or set
	 * the initial value to NULL if col_val is not set 
	 */
	col->col_val.b = col_val;
	col->col_strlen = col_len;
	break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* set the rest of the column fields */
    strcpy(col->col_name, col_name);
    col->col_flags = col_flags;
    col->col_pos = col_num;
    col->col_str_maxlen = bufflen;
    col->col_type = col_typ;
    col->col_valtyp = DB_VALTYP_BINARY;
    col->col_copytyp = copy_typ;

    return NO_ERR;

} /* db_table_set_bincol */



/* END file db_table.c */
