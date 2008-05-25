#ifndef _H_tstamp
#define _H_tstamp
/*  FILE: tstamp.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Timestamp utilities

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
17-apr-06    abb      begun
*/

#define TSTAMP_MIN_SIZE   50
#define TSTAMP_DATE_SIZE  12
#define TSTAMP_SQL_SIZE   20

/*
 * Set the current date and time in an XML dateTime string format
 *
 * INPUTS:
 *   buff == pointer to buffer to hold output
 *           MUST BE AT LEAST 50 CHARS
 */
extern void 
    tstamp_datetime (xmlChar *buff);


/* get just the current date string 
 * BUFFER MUST BE AT LEAST 12 chars
 */
extern void 
    tstamp_date (xmlChar *buff);


/*
 * Set the current date and time in an SQL datetime string format
 *
 * INPUTS:
 *   buff == pointer to buffer to hold output
 *           MUST BE AT LEAST 20 CHARS
 */
extern void 
    tstamp_datetime_sql (xmlChar *buff);

#endif	    /* _H_tstamp */
