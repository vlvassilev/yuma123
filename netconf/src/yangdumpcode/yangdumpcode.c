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

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_yangdump
#include  "yangdump.h"
#endif

#ifndef _H_ydump
#include  "ydump.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                            *
*                                                                   *
*********************************************************************/

static yangdump_cvtparms_t   mycvtparms;


/********************************************************************
*                                                                   *
*                        FUNCTION main                              *
*                                                                   *
*********************************************************************/
int 
    main (int argc, 
          const char *argv[])
{
    status_t      res;

#ifdef MEMORY_DEBUG
    mtrace();
#endif

    malloc_cnt = 0;
    free_cnt = 0;

    res = ydump_init(argc, argv, TRUE, &mycvtparms);

    if (res == NO_ERR) {
	res = ydump_main(&mycvtparms);
    }

    /* if warnings+ are enabled, then res could be NO_ERR and still
     * have output to STDOUT
     */
    if (res == NO_ERR) {
        log_warn("\n");   /*** producing extra blank lines ***/
    }

    print_errors();

    print_error_count();

    ydump_cleanup(&mycvtparms);

    print_error_count();

    return (res == NO_ERR) ? 0 : 1;

} /* main */

/* END yangdumpcode.c */



