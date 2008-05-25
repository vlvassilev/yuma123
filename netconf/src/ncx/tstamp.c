/*  FILE: tstamp.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
17apr06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif


/********************************************************************
* FUNCTION tstamp_datetime
*
* Set the current date and time in an XML dateTime string format
*
* INPUTS:
*   buff == pointer to buffer to hold output
*           MUST BE AT LEAST 50 CHARS
* OUTPUTS:
*   buff is filled in
*********************************************************************/
void 
    tstamp_datetime (xmlChar *buff)
{
    time_t  utime;
    struct tm  *curtime;

#ifdef DEBUG
    if (!buff) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    (void)time(&utime);
    curtime = localtime(&utime);
    (void)sprintf((char *)buff, 
		  "%04u-%02u-%02uT%02u:%02u:%02u.%03u%c%02u:%02u",
		  (uint32)(curtime->tm_year+1900),
		  (uint32)(curtime->tm_mon+1),
		  (uint32)curtime->tm_mday,
		  (uint32)curtime->tm_hour,
		  (uint32)curtime->tm_min,
		  (uint32)curtime->tm_sec,
		  0,    /***  milliseconds TBD ***/
		  (curtime->tm_gmtoff < 0) ? '-' : '+',
		  (uint32)abs((curtime->tm_gmtoff / 3600)),
		  (uint32)abs((curtime->tm_gmtoff % 60)));

} /* tstamp_datetime */


/********************************************************************
* FUNCTION tstamp_date
*
* Set the current date in an XML dateTime string format
*
* INPUTS:
*   buff == pointer to buffer to hold output
*           MUST BE AT LEAST 11 CHARS
* OUTPUTS:
*   buff is filled in
*********************************************************************/
void 
    tstamp_date (xmlChar *buff)
{
    time_t  utime;
    struct tm  *curtime;

#ifdef DEBUG
    if (!buff) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    (void)time(&utime);
    curtime = localtime(&utime);
    (void)sprintf((char *)buff, 
		  "%04u-%02u-%02u",
		  (uint32)(curtime->tm_year+1900),
		  (uint32)(curtime->tm_mon+1),
		  (uint32)curtime->tm_mday);

} /* tstamp_date */


/********************************************************************
* FUNCTION tstamp_datetime_sql
*
* Set the current date and time in an XML dateTime string format
*
* INPUTS:
*   buff == pointer to buffer to hold output
*           MUST BE AT LEAST 20 CHARS
* OUTPUTS:
*   buff is filled in
*********************************************************************/
void 
    tstamp_datetime_sql (xmlChar *buff)
{
    time_t  utime;
    struct tm  *curtime;

#ifdef DEBUG
    if (!buff) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    (void)time(&utime);
    curtime = localtime(&utime);
    /***  milliseconds not returned, hardwired to '00' ***/
    (void)sprintf((char *)buff, 
		  "%04u-%02u-%02u %02u:%02u:%02u",
		  (uint32)(curtime->tm_year+1900),
		  (uint32)(curtime->tm_mon+1),
		  (uint32)curtime->tm_mday,
		  (uint32)curtime->tm_hour,
		  (uint32)curtime->tm_min,
		  (uint32)curtime->tm_sec);
    
} /* tstamp_datetime_sql */

/* END file tstamp.c */
