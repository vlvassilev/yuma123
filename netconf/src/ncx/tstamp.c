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
#include <ctype.h>

#define __USE_XOPEN 1
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

#ifndef _H_xml_util
#include  "xml_util.h"
#endif


/********************************************************************
* FUNCTION time_to_string
*
* Convert the tm to a string in YANG canonical format
*
* INPUTS:
*   curtime == time struct to use
*   buff == pointer to buffer to hold output
*           MUST BE AT LEAST 21 CHARS
* OUTPUTS:
*   buff is filled in
*********************************************************************/
static void 
    time_to_string (const struct tm *curtime,
		       xmlChar *buff)
{
    (void)sprintf((char *)buff, 
		  "%04u-%02u-%02uT%02u:%02u:%02uZ",
		  (uint32)(curtime->tm_year+1900),
		  (uint32)(curtime->tm_mon+1),
		  (uint32)curtime->tm_mday,
		  (uint32)curtime->tm_hour,
		  (uint32)curtime->tm_min,
		  (uint32)curtime->tm_sec);

} /* time_to_string */


/********************************************************************
* FUNCTION tstamp_datetime
*
* Set the current date and time in an XML dateTime string format
*
* INPUTS:
*   buff == pointer to buffer to hold output
*           MUST BE AT LEAST 21 CHARS
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
    curtime = gmtime(&utime);
    time_to_string(curtime, buff);

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


/********************************************************************
* FUNCTION tstamp_convert_to_utctime
*
* Check if the specified string is a valid dateTime or 
* date-and-time string is valid and if so, convert it
* to 
*
* INPUTS:
*   buff == pointer to buffer to check
*   isNegative == address of return negative date flag
*   res == address of return status
*
* OUTPUTS:
*   *isNegative == TRUE if a negative dateTime string is given
*                  FALSE if no starting '-' sign found
*   *res == return status
*
* RETURNS:
*   malloced pointer to converted date time string
*   or NULL if some error
*********************************************************************/
xmlChar *
    tstamp_convert_to_utctime (const xmlChar *timestr,
			       boolean *isNegative,
			       status_t *res)
{
    const char *retptr;
    xmlChar    *buffer;
    time_t      utime;
    struct tm   convertedtime, *curtime;
    uint32      len;

#ifdef DEBUG
    if (!timestr || !isNegative || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    *res = NO_ERR;

    memset(&convertedtime, 0x0, sizeof(struct tm));

    if (*timestr == '-') {
	*isNegative = TRUE;
	timestr++;
    } else {
	*isNegative = FALSE;
    }

    len = xml_strlen(timestr);

    if (len == 20) {
	/* could be in canonical form */
	retptr = strptime((const char *)timestr,
			  "%FT%TZ",
			  &convertedtime);
	if (retptr && *retptr == '\0') {
	    buffer = xml_strdup(timestr);
	    if (!buffer) {
		*res = ERR_INTERNAL_MEM;
		return NULL;
	    } else {
		return buffer;
	    }
	} else {
	    *res = ERR_NCX_INVALID_VALUE;
	    return NULL;
	}
    } else if (len > 20) {
	retptr = strptime((const char *)timestr,
			  "%FT%T",
			  &convertedtime);
	if (retptr == NULL || *retptr == '\0') {
	    *res = ERR_NCX_INVALID_VALUE;
	    return NULL;
	}

	/* check is frac-seconds entered, and skip it */
	if (*retptr == '.') {
	    retptr++;
	    if (!isdigit(*retptr)) {
		*res = ERR_NCX_INVALID_VALUE;
		return NULL;
	    }

	    retptr++;  /* got a start digit */
	    while (isdigit((char)*retptr)) {
		retptr++;
	    }
	}

	/* check if a timezone offset is present */
	retptr = strptime(retptr, "%z", &convertedtime);
	if (retptr == NULL || *retptr != '\0') {
	    *res = ERR_NCX_INVALID_VALUE;
	    return NULL;
	}
	
	buffer = m__getMem(TSTAMP_MIN_SIZE);
	if (!buffer) {
	    *res = ERR_INTERNAL_MEM;
	    return NULL;
	}

	utime = mktime(&convertedtime);
	if (utime == (utime)-1) {
	    *res = ERR_INTERNAL_VAL;
	    m__free(buffer);
	    return NULL;
	}

	curtime = gmtime(&utime);
	time_to_string(curtime, buffer);
	return buffer;
    } else {
	/* improper length */
	*res = ERR_NCX_INVALID_VALUE;
	return NULL;
    }
    
} /* tstamp_convert_to_utctime */


/* END file tstamp.c */
