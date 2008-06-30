#ifndef _H_yangdiff
#define _H_yangdiff

/*  FILE: yangdiff.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Exports yangdiff.ncx conversion CLI parameter struct
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
01-mar-08    abb      Begun; moved from ncx/ncxtypes.h

*/

#include <xmlstring.h>

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/
#define YANGDIFF_PROGNAME   (const xmlChar *)"yangdiff"
#define YANGDIFF_PROGVER    (const xmlChar *)"0.1.0"


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/
typedef enum yangdiff_difftype_t_ {
    YANGDIFF_DT_NONE,
    YANGDIFF_DT_TERSE,
    YANGDIFF_DT_BRIEF,
    YANGDIFF_DT_NORMAL,
    YANGDIFF_DT_REVISION
} yangdiff_difftype_t;

/* struct of yangdiff conversion parameters */
typedef struct yangdiff_diffparms_t_ {
    /* external parameters */
    xmlChar        *old;   /* malloced due to subtree design */
    xmlChar        *new;   /* malloced due to subtree design */
    const xmlChar  *output;
    const xmlChar  *difftype;
    const xmlChar  *logfilename;
    const xmlChar  *modpath;
    const xmlChar  *config;
    boolean         helpmode;
    int32           indent;
    boolean         logappend;
    log_debug_t     log_level;
    boolean         noheader;
    boolean         nosubdirs;
    boolean         versionmode;

    /* internal vars */
    ncx_module_t   *mod;
    ses_cb_t       *scb;
    xmlChar        *curold;
    xmlChar        *curnew;
    xmlChar        *buff;
    ncx_module_t   *oldmod;
    ncx_module_t   *newmod;
    uint32          bufflen;
    boolean         firstdone;
    boolean         output_isdir;
    boolean         old_isdir;
    boolean         new_isdir;
    yangdiff_difftype_t edifftype;
    int32           curindent;
} yangdiff_diffparms_t;

/* change description block */
typedef struct yangdiff_cdb_t_ {
    const xmlChar *fieldname;
    const xmlChar *chtyp;
    const xmlChar *oldval;
    const xmlChar *newval;
    const xmlChar *useval;
    boolean        changed;
} yangdiff_cdb_t;


#endif	    /* _H_yangdiff */
