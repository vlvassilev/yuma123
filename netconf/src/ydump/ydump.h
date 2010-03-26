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
#ifndef _H_yangdump
#define _H_yangdump

/*  FILE: yangdump.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

  Exports yangdump.ncx conversion CLI parameter struct
 
*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
01-mar-08    abb      Begun; moved from ncx/ncxtypes.h

*/

#include <xmlstring.h>

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/
#define PROGNAME            "yangdump"

#define EMPTY_STRING        (const xmlChar *)""

#define EL_A                (const xmlChar *)"a"
#define EL_BODY             (const xmlChar *)"body"
#define EL_DIV              (const xmlChar *)"div"
#define EL_H1               (const xmlChar *)"h1"
#define EL_PRE              (const xmlChar *)"pre"
#define EL_SPAN             (const xmlChar *)"span"
#define EL_UL               (const xmlChar *)"ul"
#define EL_LI               (const xmlChar *)"li"

#define CL_YANG             (const xmlChar *)"yang"
#define CL_TOCMENU          (const xmlChar *)"tocmenu"
#define CL_TOCPLAIN         (const xmlChar *)"tocplain"
#define CL_DADDY            (const xmlChar *)"daddy"

#define ID_NAV              (const xmlChar *)"nav"

#define OBJVIEW_RAW     "raw"
#define OBJVIEW_COOKED  "cooked"


/* this should match the buffer size in ncx/tk.h */
#define YANGDUMP_BUFFSIZE   0xffff

#define YANGDUMP_DEF_OUTPUT   "stdout"

#define YANGDUMP_DEF_CONFIG   (const xmlChar *)"/etc/yuma/yangdump.conf"

#define YANGDUMP_DEF_TOC      (const xmlChar *)"menu"

#define YANGDUMP_DEF_OBJVIEW  OBJVIEW_RAW

#define YANGDUMP_MOD          (const xmlChar *)"yangdump"
#define YANGDUMP_CONTAINER    (const xmlChar *)"yangdump"

#define YANGDUMP_PARM_CONFIG        (const xmlChar *)"config"
#define YANGDUMP_PARM_DEFNAMES      (const xmlChar *)"defnames"
#define YANGDUMP_PARM_DEPENDENCIES  (const xmlChar *)"dependencies"
#define YANGDUMP_PARM_DEVIATION     (const xmlChar *)"deviation"
#define YANGDUMP_PARM_EXPORTS       (const xmlChar *)"exports"
#define YANGDUMP_PARM_FORMAT        (const xmlChar *)"format"
#define YANGDUMP_PARM_HTML_DIV      (const xmlChar *)"html-div"
#define YANGDUMP_PARM_HTML_TOC      (const xmlChar *)"html-toc"
#define YANGDUMP_PARM_IDENTIFIERS   (const xmlChar *)"identifiers"
#define YANGDUMP_PARM_INDENT        (const xmlChar *)"indent"
#define YANGDUMP_PARM_MODULE        (const xmlChar *)"module"
#define YANGDUMP_PARM_MODVERSION    (const xmlChar *)"modversion"
#define YANGDUMP_PARM_VERSIONNAMES  (const xmlChar *)"versionnames"
#define YANGDUMP_PARM_OUTPUT        (const xmlChar *)"output"
#define YANGDUMP_PARM_OBJVIEW       (const xmlChar *)"objview"
#define YANGDUMP_PARM_XSD_SCHEMALOC (const xmlChar *)"xsd-schemaloc"
#define YANGDUMP_PARM_SIMURLS       (const xmlChar *)"simurls"
#define YANGDUMP_PARM_SUBTREE       (const xmlChar *)"subtree"
#define YANGDUMP_PARM_URLSTART      (const xmlChar *)"urlstart"
#define YANGDUMP_PARM_UNIFIED       (const xmlChar *)"unified"

/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/

/* struct of yangdump conversion parameters */
typedef struct yangdump_cvtparms_t_ {
    /* external parameters */
    char           *module;   /* malloced due to subtree design */
    char           *curmodule;  /* not malloced when 2+ modules */
    const char     *output;
    const char     *subtree;
    const char     *objview;
    const xmlChar  *schemaloc;
    const xmlChar  *urlstart;
    const xmlChar  *modpath;
    const xmlChar  *config;
    const xmlChar  *html_toc;
    const xmlChar  *css_file;
    int32           indent;
    uint32          modcount;
    uint32          subtreecount;
    ncx_cvttyp_t    format;
    boolean         helpmode;
    help_mode_t     helpsubmode;
    boolean         defnames;
    boolean         dependencies;
    boolean         exports;
    boolean         showerrorsmode;
    boolean         identifiers;
    boolean         html_div;
    boolean         modversion;
    boolean         subdirs;
    boolean         versionnames;
    boolean         output_isdir;
    boolean         simurls;
    boolean         unified;
    boolean         versionmode;
    boolean         rawmode;
    boolean         allowcode;

    /* internal vars */
    xmlChar        *full_output;
    ncx_module_t   *mod;
    char           *srcfile;
    char           *buff;
    uint32          bufflen;
    boolean         firstdone;
} yangdump_cvtparms_t;


#endif            /* _H_yangdump */
