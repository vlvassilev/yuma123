/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: log.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
08jan06      abb      begun, borrowed from openssh code

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tstamp
#include "tstamp.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/

/* global logging only at this time
 * per-session logging is TBD, and would need
 * to be in the agent or mgr specific session handlers,
 * not in the ncx directory
 */
static log_debug_t   debug_level = LOG_DEBUG_NONE;

static boolean       use_tstamps;

static FILE *logfile = NULL;

static FILE *altlogfile = NULL;


/********************************************************************
* FUNCTION log_open
*
*   Open a logfile for writing
*   DO NOT use this function to send log entries to STDOUT
*   Leave the logfile NULL instead.
*
* INPUTS:
*   fname == full filespec string for logfile
*   append == TRUE if the log should be appended
*          == FALSE if it should be rewriten
*   tstamps == TRUE if the datetime stamp should be generated
*             at log-open and log-close time
*          == FALSE if no open and close timestamps should be generated
*
* RETURNS:
*    status
*********************************************************************/
status_t
    log_open (const char *fname,
              boolean append,
              boolean tstamps)
{
    const char *str;
    xmlChar buff[TSTAMP_MIN_SIZE];

#ifdef DEBUG
    if (!fname) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (logfile) {
        return ERR_NCX_DATA_EXISTS;
    }

    if (append) {
        str="a";
    } else {
        str="w";
    }

    logfile = fopen(fname, str);
    if (!logfile) {
        return ERR_FIL_OPEN;
    }

    use_tstamps = tstamps;
    if (tstamps) {
        tstamp_datetime(buff);
        fprintf(logfile, "\n*** log open at %s ***\n", buff);
    }

    return NO_ERR;

}  /* log_open */


/********************************************************************
* FUNCTION log_close
*
*   Close the logfile
*
* RETURNS:
*    none
*********************************************************************/
void
    log_close (void)
{
    xmlChar buff[TSTAMP_MIN_SIZE];

    if (!logfile) {
        return;
    }

    if (use_tstamps) {
        tstamp_datetime(buff);
        fprintf(logfile, "\n*** log close at %s ***\n", buff);
    }

    fclose(logfile);
    logfile = NULL;

}  /* log_close */


/********************************************************************
* FUNCTION log_alt_open
*
*   Open an alternate logfile for writing
*   DO NOT use this function to send log entries to STDOUT
*   Leave the logfile NULL instead.
*
* INPUTS:
*   fname == full filespec string for logfile
*
* RETURNS:
*    status
*********************************************************************/
status_t
    log_alt_open (const char *fname)
{
    const char *str;

#ifdef DEBUG
    if (!fname) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (altlogfile) {
        return ERR_NCX_DATA_EXISTS;
    }

    str="w";

    altlogfile = fopen(fname, str);
    if (!altlogfile) {
        return ERR_FIL_OPEN;
    }

    return NO_ERR;

}  /* log_alt_open */


/********************************************************************
* FUNCTION log_alt_close
*
*   Close the alternate logfile
*
* RETURNS:
*    none
*********************************************************************/
void
    log_alt_close (void)
{
    if (!altlogfile) {
        return;
    }

    fclose(altlogfile);
    altlogfile = NULL;

}  /* log_alt_close */


/********************************************************************
* FUNCTION log_stdout
*
*   Write lines of text to STDOUT, even if the logfile
*   is open, unless the debug mode is set to NONE
*   to indicate silent batch mode
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_stdout (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() == LOG_DEBUG_NONE) {
        return;
    }

    va_start(args, fstr);
    vprintf(fstr, args);
    fflush(stdout);
    va_end(args);

}  /* log_stdout */


/********************************************************************
* FUNCTION log_write
*
*   Generate a log entry, regardless of log level
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_write (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() == LOG_DEBUG_NONE) {
        return;
    }

    va_start(args, fstr);

    if (logfile) {
        vfprintf(logfile, fstr, args);
        fflush(logfile);
    } else {
        vprintf(fstr, args);
        fflush(stdout);
    }

    va_end(args);

}  /* log_write */


/********************************************************************
* FUNCTION log_alt_write
*
*   Write to the alternate log file
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for fprintf
*
*********************************************************************/
void 
    log_alt_write (const char *fstr, ...)
{
    va_list args;

    va_start(args, fstr);

    if (altlogfile) {
        vfprintf(altlogfile, fstr, args);
        fflush(altlogfile);
    }

    va_end(args);

}  /* log_alt_write */


/********************************************************************
* FUNCTION log_alt_indent
* 
* Printf a newline to the alternate logfile,
* then the specified number of space chars
*
* INPUTS:
*    indentcnt == number of indent chars, -1 == skip everything
*
*********************************************************************/
void
    log_alt_indent (int32 indentcnt)
{
    int32  i;

    if (indentcnt >= 0) {
        log_alt_write("\n");
        for (i=0; i<indentcnt; i++) {
            log_alt_write(" ");
        }
    }

} /* log_alt_indent */


/********************************************************************
* FUNCTION log_error
*
*   Generate a LOG_DEBUG_ERROR log entry
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_error (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() < LOG_DEBUG_ERROR) {
        return;
    }

    va_start(args, fstr);

    if (logfile) {
        vfprintf(logfile, fstr, args);
        fflush(logfile);
    } else {
        vprintf(fstr, args);
        fflush(stdout);
    }

    va_end(args);

}  /* log_error */


/********************************************************************
* FUNCTION log_warn
*
*   Generate LOG_DEBUG_WARN log output
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_warn (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() < LOG_DEBUG_WARN) {
        return;
    }

    va_start(args, fstr);

    if (logfile) {
        vfprintf(logfile, fstr, args);
        fflush(logfile);
    } else {
        vprintf(fstr, args);
        fflush(stdout);
    }

    va_end(args);

}  /* log_warn */


/********************************************************************
* FUNCTION log_info
*
*   Generate a LOG_DEBUG_INFO log entry
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_info (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() < LOG_DEBUG_INFO) {
        return;
    }

    va_start(args, fstr);

    if (logfile) {
        vfprintf(logfile, fstr, args);
        fflush(logfile);
    } else {
        vprintf(fstr, args);
        fflush(stdout);
    }

    va_end(args);

}  /* log_info */


/********************************************************************
* FUNCTION log_debug
*
*   Generate a LOG_DEBUG_DEBUG log entry
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_debug (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() < LOG_DEBUG_DEBUG) {
        return;
    }

    va_start(args, fstr);

    if (logfile) {
        vfprintf(logfile, fstr, args);
        fflush(logfile);
    } else {
        vprintf(fstr, args);
        fflush(stdout);
    }

    va_end(args);

}  /* log_debug */


/********************************************************************
* FUNCTION log_debug2
*
*   Generate LOG_DEBUG_DEBUG2 log trace output
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_debug2 (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() < LOG_DEBUG_DEBUG2) {
        return;
    }

    va_start(args, fstr);

    if (logfile) {
        vfprintf(logfile, fstr, args);
        fflush(logfile);
    } else {
        vprintf(fstr, args);
        fflush(stdout);
    }

    va_end(args);

}  /* log_debug2 */


/********************************************************************
* FUNCTION log_debug3
*
*   Generate LOG_DEBUG_DEBUG3 log trace output
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_debug3 (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() < LOG_DEBUG_DEBUG3) {
        return;
    }

    va_start(args, fstr);

    if (logfile) {
        vfprintf(logfile, fstr, args);
        fflush(logfile);
    } else {
        vprintf(fstr, args);
        fflush(stdout);
    }

    va_end(args);

}  /* log_debug3 */


/********************************************************************
* FUNCTION log_debug4
*
*   Generate LOG_DEBUG_DEBUG4 log trace output
*
* INPUTS:
*   fstr == format string in printf format
*   ... == any additional arguments for printf
*
*********************************************************************/
void 
    log_debug4 (const char *fstr, ...)
{
    va_list args;

    if (log_get_debug_level() < LOG_DEBUG_DEBUG4) {
        return;
    }

    va_start(args, fstr);

    if (logfile) {
        vfprintf(logfile, fstr, args);
        fflush(logfile);
    } else {
        vprintf(fstr, args);
        fflush(stdout);
    }

    va_end(args);

}  /* log_debug4 */


/********************************************************************
* FUNCTION log_set_debug_level
* 
* Set the global debug filter threshold level
* 
* INPUTS:
*   dlevel == desired debug level
*
*********************************************************************/
void
    log_set_debug_level (log_debug_t dlevel)
{
#ifdef DEBUG
    if (dlevel > LOG_DEBUG_DEBUG4) {
        SET_ERROR(ERR_INTERNAL_VAL);
        dlevel = LOG_DEBUG_DEBUG4;
    }
#endif

    debug_level = dlevel;

}  /* log_set_debug_level */


/********************************************************************
* FUNCTION log_get_debug_level
* 
* Get the global debug filter threshold level
* 
* RETURNS:
*   the global debug level
*********************************************************************/
log_debug_t
    log_get_debug_level (void)
{
    return debug_level;

}  /* log_get_debug_level */


/********************************************************************
* FUNCTION log_get_debug_level_enum
* 
* Get the corresponding debug enum for the specified string
* 
* INPUTS:
*   str == string value to convert
*
* RETURNS:
*   the corresponding enum for the specified debug level
*********************************************************************/
log_debug_t
    log_get_debug_level_enum (const char *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return LOG_DEBUG_NONE;
    }
#endif

    if (!xml_strcmp((const xmlChar *)str,
                    LOG_DEBUG_STR_OFF)) {
        return LOG_DEBUG_OFF;
    } else if (!xml_strcmp((const xmlChar *)str,
                           LOG_DEBUG_STR_ERROR)) {
        return LOG_DEBUG_ERROR;
    } else if (!xml_strcmp((const xmlChar *)str,
                           LOG_DEBUG_STR_WARN)) {
        return LOG_DEBUG_WARN;
    } else if (!xml_strcmp((const xmlChar *)str,
                           LOG_DEBUG_STR_INFO)) {
        return LOG_DEBUG_INFO;
    } else if (!xml_strcmp((const xmlChar *)str,
                           LOG_DEBUG_STR_DEBUG)) {
        return LOG_DEBUG_DEBUG;
    } else if (!xml_strcmp((const xmlChar *)str,
                           LOG_DEBUG_STR_DEBUG2)) {
        return LOG_DEBUG_DEBUG2;
    } else if (!xml_strcmp((const xmlChar *)str,
                           LOG_DEBUG_STR_DEBUG3)) {
        return LOG_DEBUG_DEBUG3;
    } else if (!xml_strcmp((const xmlChar *)str,
                           LOG_DEBUG_STR_DEBUG4)) {
        return LOG_DEBUG_DEBUG4;
    } else {
        return LOG_DEBUG_NONE;
    }

}  /* log_get_debug_level_enum */


/********************************************************************
* FUNCTION log_get_debug_level_string
* 
* Get the corresponding string for the debug enum
* 
* INPUTS:
*   level ==  the enum for the specified debug level
*
* RETURNS:
*   the string value for this enum

*********************************************************************/
const xmlChar *
    log_get_debug_level_string (log_debug_t level)
{
    switch (level) {
    case LOG_DEBUG_OFF:
        return LOG_DEBUG_STR_OFF;
    case LOG_DEBUG_ERROR:
        return LOG_DEBUG_STR_ERROR;
    case LOG_DEBUG_WARN:
        return LOG_DEBUG_STR_WARN;
    case LOG_DEBUG_INFO:
        return LOG_DEBUG_STR_INFO;
    case LOG_DEBUG_DEBUG:
        return LOG_DEBUG_STR_DEBUG;
    case LOG_DEBUG_DEBUG2:
        return LOG_DEBUG_STR_DEBUG2;
    case LOG_DEBUG_DEBUG3:
        return LOG_DEBUG_STR_DEBUG3;
    case LOG_DEBUG_DEBUG4:
        return LOG_DEBUG_STR_DEBUG4;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return LOG_DEBUG_STR_OFF;
    }
    /*NOTREACHED*/

}  /* log_get_debug_level_string */


/********************************************************************
* FUNCTION log_is_open
* 
* Check if the logfile is active
* 
* RETURNS:
*   TRUE if logfile open, FALSE otherwise
*********************************************************************/
boolean
    log_is_open (void)
{
    return (logfile) ? TRUE : FALSE;

}  /* log_is_open */


/********************************************************************
* FUNCTION log_indent
* 
* Printf a newline, then the specified number of chars
*
* INPUTS:
*    indentcnt == number of indent chars, -1 == skip everything
*
*********************************************************************/
void
    log_indent (int32 indentcnt)
{
    int32  i;

    if (indentcnt >= 0) {
        log_write("\n");
        for (i=0; i<indentcnt; i++) {
            log_write(" ");
        }
    }

} /* log_indent */


/********************************************************************
* FUNCTION log_stdout_indent
* 
* Printf a newline to stdout, then the specified number of chars
*
* INPUTS:
*    indentcnt == number of indent chars, -1 == skip everything
*
*********************************************************************/
void
    log_stdout_indent (int32 indentcnt)
{
    int32  i;

    if (indentcnt >= 0) {
        log_stdout("\n");
        for (i=0; i<indentcnt; i++) {
            log_stdout(" ");
        }
    }

} /* log_stdout_indent */


/********************************************************************
* FUNCTION log_get_logfile
* 
* Get the open logfile for direct output
* Needed by libtecla to write command line history
*
* RETURNS:
*   pointer to open FILE if any
*   NULL if no open logfile
*********************************************************************/
FILE *
    log_get_logfile (void)
{
    return logfile;

}  /* log_get_logfile */


/* END file log.c */
