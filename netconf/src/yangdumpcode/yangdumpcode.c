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
/*  FILE: yangdumpcode.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
24-mar-10    abb      begun; split out from yangdump.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* #define MEMORY_DEBUG 1 */

#ifdef MEMORY_DEBUG
#include <mcheck.h>
#endif

#define _C_main 1

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_c
#include  "c.h"
#endif

#ifndef _H_cli
#include  "cli.h"
#endif

#ifndef _H_conf
#include  "conf.h"
#endif

#ifndef _H_cyang
#include  "cyang.h"
#endif

#ifndef _H_h
#include  "h.h"
#endif

#ifndef _H_help
#include  "help.h"
#endif

#ifndef _H_html
#include  "html.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include  "ncxmod.h"
#endif

#ifndef _H_sql
#include  "sql.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tg2
#include  "tg2.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_val_util
#include  "val_util.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_xml_wr
#include  "xml_wr.h"
#endif

#ifndef _H_xsd
#include  "xsd.h"
#endif

#ifndef _H_xsd_util
#include  "xsd_util.h"
#endif

#ifndef _H_yang
#include  "yang.h"
#endif

#ifndef _H_yangconst
#include  "yangconst.h"
#endif

#ifndef _H_yangdump
#include  "yangdump.h"
#endif

#ifndef _H_yangdump_util
#include  "yangdump_util.h"
#endif

#ifndef _H_yangyin
#include  "yangyin.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
/* #define YANGDUMP_DEBUG   1 */


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                            *
*                                                                   *
*********************************************************************/

static val_value_t          *cli_val;
static yangdump_cvtparms_t   cvtparms;
static dlq_hdr_t             savedevQ;


/********************************************************************
*                                                                   *
*                        FUNCTION main                              *
*                                                                   *
*********************************************************************/
int 
    main (int argc, 
          const char *argv[])
{
    val_value_t  *val;
    status_t      res;
    boolean       done, quickexit;
    xmlChar       buffer[NCX_VERSION_BUFFSIZE];

#ifdef MEMORY_DEBUG
    mtrace();
#endif

    done = FALSE;
    res = main_init(argc, argv);

    if (res == NO_ERR) {

        quickexit = cvtparms.helpmode ||
            cvtparms.versionmode ||
            cvtparms.showerrorsmode;

        if (cvtparms.versionmode || cvtparms.showerrorsmode) {
            res = ncx_get_version(buffer, NCX_VERSION_BUFFSIZE);
            if (res == NO_ERR) {
                log_write("\nyangdump %s", buffer);
                if (cvtparms.versionmode) {
                    log_write("\n");
                }
            } else {
                SET_ERROR(res);
            }
        }
        if (cvtparms.helpmode) {
            help_program_module(YANGDUMP_MOD, 
                                YANGDUMP_CONTAINER, 
                                cvtparms.helpsubmode);
        }
        if (cvtparms.showerrorsmode) {
            log_write(" errors and warnings\n");
            print_error_messages();
        }

        if (!quickexit) {
            /* check if subdir search suppression is requested */
            if (!cvtparms.subdirs) {
                ncxmod_set_subdirs(FALSE);
            }

            if (LOGINFO) {
                /* generate banner everytime yangdump runs */
                write_banner();
            } else {
                if (!(cvtparms.format == NCX_CVTTYP_XSD ||
                      cvtparms.format == NCX_CVTTYP_HTML ||
                      cvtparms.format == NCX_CVTTYP_H ||
                      cvtparms.format == NCX_CVTTYP_C)) {
                    write_banner();
                }
            }

            /* first check if there are any deviations to load */
            res = NO_ERR;
            val = val_find_child(cli_val, 
                                 YANGDUMP_MOD, 
                                 YANGDUMP_PARM_DEVIATION);
            while (val) {
                res = ncxmod_load_deviation(VAL_STR(val), &savedevQ);
                if (NEED_EXIT(res)) {
                    val = NULL;
                } else {
                    val = val_find_next_child(cli_val,
                                              YANGDUMP_MOD,
                                              YANGDUMP_PARM_DEVIATION,
                                              val);
                }
            }

            /* convert one file or N files or 1 subtree */
            res = NO_ERR;
            val = val_find_child(cli_val, 
                                 YANGDUMP_MOD, 
                                 YANGDUMP_PARM_MODULE);
            while (val) {
                done = TRUE;
                cvtparms.curmodule = (char *)VAL_STR(val);
                res = convert_one(&cvtparms);
                if (NEED_EXIT(res)) {
                    val = NULL;
                } else {
                    val = val_find_next_child(cli_val,
                                              YANGDUMP_MOD,
                                              YANGDUMP_PARM_MODULE,
                                              val);
                }
            }

            if (res == NO_ERR &&
                cvtparms.subtreecount >= 1) {
                if (cvtparms.format == NCX_CVTTYP_XSD ||
                    cvtparms.format == NCX_CVTTYP_HTML ||
                    cvtparms.format == NCX_CVTTYP_YANG ||
                    cvtparms.format == NCX_CVTTYP_COPY) {
                    /* force separate file names in subtree mode */
                    cvtparms.defnames = TRUE;
                }
                   
                val = val_find_child(cli_val, 
                                     YANGDUMP_MOD, 
                                     YANGDUMP_PARM_SUBTREE);
                while (val) {
                    done = TRUE;
                    cvtparms.subtree = (const char *)VAL_STR(val);
                    res = ncxmod_process_subtree(cvtparms.subtree,
                                                 subtree_callback,
                                                 &cvtparms);
                    if (NEED_EXIT(res)) {
                        val = NULL;
                    } else {
                        val = val_find_next_child(cli_val,
                                                  YANGDUMP_MOD,
                                                  YANGDUMP_PARM_SUBTREE,
                                                  val);
                    }
                }
            }

            if (res == NO_ERR && !done) {
                res = ERR_NCX_MISSING_PARM;
                log_error("\nyangdump: Error: missing parameter (%s or %s)\n",
                          NCX_EL_MODULE, 
                          NCX_EL_SUBTREE);
            }
        }
    }

    /* if warnings+ are enabled, then res could be NO_ERR and still
     * have output to STDOUT
     */
    if (res == NO_ERR) {
        log_warn("\n");   /*** producing extra blank lines ***/
    }

    print_errors();

    print_error_count();

    main_cleanup();

    print_error_count();

    return (res == NO_ERR) ? 0 : 1;

} /* main */

/* END yangdump.c */



