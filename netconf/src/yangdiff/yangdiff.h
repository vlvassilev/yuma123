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


/* this should match the buffer size in ncx/tk.h */
#define YANGDIFF_BUFFSIZE           0xffff
#define YANGDIFF_MOD_BUFFSIZE       0x1fff
#define YANGDIFF_DEF_MAXSIZE       12

#define YANGDIFF_DIFFTYPE_TERSE    (const xmlChar *)"terse"
#define YANGDIFF_DIFFTYPE_NORMAL   (const xmlChar *)"normal"
#define YANGDIFF_DIFFTYPE_REVISION (const xmlChar *)"revision"

#define YANGDIFF_DEF_OUTPUT        (const xmlChar *)"stdout"
#define YANGDIFF_DEF_DIFFTYPE      (const xmlChar *)"summary"
#define YANGDIFF_DEF_DT            YANGDIFF_DT_NORMAL
#define YANGDIFF_DEF_CONFIG        (const xmlChar *)"/etc/yangdiff.conf"
#define YANGDIFF_DEF_FILENAME      (const xmlChar *)"yangdiff.log"

#define YANGDIFF_MOD               (const xmlChar *)"yangdiff"
#define YANGDIFF_PARMSET           (const xmlChar *)"yangdiff"

#define YANGDIFF_PARM_CONFIG        (const xmlChar *)"config"
#define YANGDIFF_PARM_OLD           (const xmlChar *)"old"
#define YANGDIFF_PARM_NEW           (const xmlChar *)"new"
#define YANGDIFF_PARM_DIFFTYPE      (const xmlChar *)"difftype"
#define YANGDIFF_PARM_OUTPUT        (const xmlChar *)"output"
#define YANGDIFF_PARM_INDENT        (const xmlChar *)"indent"
#define YANGDIFF_PARM_NO_SUBDIRS    (const xmlChar *)"no-subdirs"
#define YANGDIFF_PARM_NO_HEADER     (const xmlChar *)"no-header"
#define YANGDIFF_LINE (const xmlChar *)\
"\n==================================================================="

#define FROM_STR (const xmlChar *)"from "
#define TO_STR (const xmlChar *)"to "
#define A_STR (const xmlChar *)"A "
#define D_STR (const xmlChar *)"D "
#define M_STR (const xmlChar *)"M "
#define ADD_STR (const xmlChar *)"- Added "
#define DEL_STR (const xmlChar *)"- Removed "
#define MOD_STR (const xmlChar *)"- Changed "



/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/
typedef enum yangdiff_difftype_t_ {
    YANGDIFF_DT_NONE,
    YANGDIFF_DT_TERSE,
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
    uint32          maxlen;

    /* internal vars */
    ncx_module_t   *mod;
    ses_cb_t       *scb;
    xmlChar        *curold;
    xmlChar        *curnew;
    xmlChar        *buff;
    xmlChar        *modbuff;
    ncx_module_t   *oldmod;
    ncx_module_t   *newmod;
    uint32          bufflen;
    uint32          modbufflen;
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
