#ifndef _H_log
#define _H_log
/*  FILE: log.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Logging manager

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
08-jan-06    abb      begun
*/

#include <stdio.h>
#include <xmlstring.h>

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* macros to check the debug level */
#define LOGERROR   (log_get_debug_level() >= LOG_DEBUG_ERROR)
#define LOGWARN    (log_get_debug_level() >= LOG_DEBUG_WARN)
#define LOGINFO    (log_get_debug_level() >= LOG_DEBUG_INFO)
#define LOGDEBUG   (log_get_debug_level() >= LOG_DEBUG_DEBUG)
#define LOGDEBUG2  (log_get_debug_level() >= LOG_DEBUG_DEBUG2)
#define LOGDEBUG3  (log_get_debug_level() >= LOG_DEBUG_DEBUG3)


#define LOG_DEBUG_STR_OFF     (const xmlChar *)"off"
#define LOG_DEBUG_STR_ERROR   (const xmlChar *)"error"
#define LOG_DEBUG_STR_WARN    (const xmlChar *)"warn"
#define LOG_DEBUG_STR_INFO    (const xmlChar *)"info"
#define LOG_DEBUG_STR_DEBUG   (const xmlChar *)"debug"
#define LOG_DEBUG_STR_DEBUG2  (const xmlChar *)"debug2"
#define LOG_DEBUG_STR_DEBUG3  (const xmlChar *)"debug3"

/********************************************************************
*                                                                   *
*                            T Y P E S                              *
*                                                                   *
*********************************************************************/

/* The debug level enumerations used in util/log.c */
typedef enum log_debug_t_ {
    LOG_DEBUG_NONE,                 /* value not set or error */
    LOG_DEBUG_OFF,                      /* logging turned off */ 
    LOG_DEBUG_ERROR,          /* fatal + internal errors only */
    LOG_DEBUG_WARN,                  /* all errors + warnings */
    LOG_DEBUG_INFO,         /* all previous + user info trace */
    LOG_DEBUG_DEBUG,                        /* debug level 1 */
    LOG_DEBUG_DEBUG2,                       /* debug level 2 */
    LOG_DEBUG_DEBUG3                        /* debug level 3 */
}  log_debug_t;

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    log_open (const char *fname,
	      boolean append,
	      boolean tstamps);

extern void
    log_close (void);

extern status_t
    log_alt_open (const char *fname);

extern void
    log_alt_close (void);

extern void 
    log_stdout (const char *fstr, ...);

extern void 
    log_write (const char *fstr, ...);

extern void 
    log_alt_write (const char *fstr, ...);

extern void
    log_alt_indent (int32 indentcnt);

extern void 
    log_fatal (const char *fstr, ...);

extern void 
    log_error (const char *fstr, ...);

extern void 
    log_warn (const char *fstr, ...);

extern void 
    log_info (const char *fstr, ...);

extern void 
    log_debug (const char *fstr, ...);

extern void 
    log_debug2 (const char *fstr, ...);

extern void 
    log_debug3 (const char *fstr, ...);

extern void
    log_set_debug_level (log_debug_t dlevel);

extern log_debug_t
    log_get_debug_level (void);

extern log_debug_t
    log_get_debug_level_enum (const char *str);

extern const xmlChar *
    log_get_debug_level_string (log_debug_t level);

extern boolean
    log_is_open (void);

extern void
    log_indent (int32 indentcnt);

extern void
    log_stdout_indent (int32 indentcnt);

extern FILE *
    log_get_logfile (void);

#endif	    /* _H_log */
