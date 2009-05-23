/*  FILE: netconfd.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
04-jun-06    abb      begun; cloned from ncxmain.c
250aug-06    abb      renamed from ncxagtd.c to netconfd.c

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

#ifndef _H_agt
#include  "agt.h"
#endif

#ifndef _H_agt_ncxserver
#include  "agt_ncxserver.h"
#endif

#ifndef _H_agt_util
#include  "agt_util.h"
#endif

#ifndef _H_help
#include  "help.h"
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

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define NETCONFD_DEBUG   1
#define NETCONFD_DEBUG_LOAD_TEST 1
#endif

#define NETCONFD_MOD       (const xmlChar *)"netconfd"
#define NETCONFD_CLI       (const xmlChar *)"netconfd"

#define MAX_FILESPEC_LEN  1023

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
/* program version string */
static char progver[] = "0.9.5";


/********************************************************************
 * FUNCTION load_base_schema 
 * 
 * RETURNS:
 *     status
 *********************************************************************/
static status_t
    load_base_schema (void)
{
    status_t res;

    /* load in the agent boot parameter definition file */
    res = ncxmod_load_module(NCXMOD_NETCONFD, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* load in the NETCONF data types and RPC methods */
    res = ncxmod_load_module(NCXMOD_NETCONF, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    return res;

}   /* load_base_schema */


/********************************************************************
 * FUNCTION load_core_schema 
 * 
 * RETURNS:
 *     status
 *********************************************************************/
static status_t
    load_core_schema (void)
{
    status_t   res;

#ifdef NETCONFD_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nnetconfd: Loading NCX Module");
    }
#endif

    /* load in the NCX extensions module */
    res = ncxmod_load_module(NCXMOD_NCX, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* load in the with-defaults extension module */
    res = ncxmod_load_module(NCXMOD_WITH_DEFAULTS, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    }

    /* initialize the NETCONF operation attribute 
     * MUST be after the netconf.yang module is loaded
     */
    res = ncx_stage2_init();
    if (res != NO_ERR) {
	return res;
    }

#ifdef NETCONFD_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nnetconfd: Loading Debug Test Module");
    }
#endif

#ifdef NETCONFD_DEBUG_LOAD_TEST

#define TESTMOD (const xmlChar *)"test"
#define TESTFEATURE (const xmlChar *)"feature1"

    /* Load test module */
    res = ncxmod_load_module(TESTMOD, NULL, NULL);
    if (res != NO_ERR) {
	return res;
    } else {
	agt_enable_feature(TESTMOD, TESTFEATURE);
    }
#endif

    return NO_ERR;

}  /* load_core_schema */


/********************************************************************
 * FUNCTION cmn_init
 * 
 * 
 * 
 *********************************************************************/
static status_t 
    cmn_init (int argc,
	      const char *argv[],
	      boolean *showver,
	      help_mode_t *showhelpmode)
{
    status_t   res;
    log_debug_t  dlevel;

#ifdef NETCONFD_DEBUG
    int   i;
#endif

    /* set the default debug output level */
#ifdef DEBUG
#ifdef NETCONFD_DEBUG
    dlevel = LOG_DEBUG_DEBUG;
#else 
    dlevel = LOG_DEBUG_INFO;
#endif
#else
    dlevel = LOG_DEBUG_WARN;
#endif

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     */
    res = ncx_init(FALSE, 
		   dlevel, 
		   TRUE,
		   "\nStarting netconfd",
		   argc, argv);
		   
    if (res != NO_ERR) {
	return res;
    }

#ifdef NETCONFD_DEBUG
    if (argc>1 && LOGDEBUG2) {
        log_debug2("\nCommand line parameters:");
        for (i=0; i<argc; i++) {
            log_debug2("\n   arg%d: %s", i, argv[i]);
        }
    }
#endif

#ifdef NETCONFD_DEBUG
    if (LOGDEBUG2) {
	log_debug2("\nnetconfd: Starting Netconf Agent Library");
    }
#endif

    /* at this point, modules that need to read config
     * params can be initialized
     */

    /* Load the core modules (netconfd and netconf) */
    res = load_base_schema();
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the Netconf Agent Library
     * with command line and conf file parameters 
     */
    res = agt_init1(argc, argv, showver, showhelpmode);
    if (res != NO_ERR) {
	return res;
    }

    /* check quick-exit mode */
    if (*showver || *showhelpmode != HELP_MODE_NONE) {
	return NO_ERR;
    }

    /* Load the core modules (netconfd and netconf) */
    res = load_core_schema();
    if (res != NO_ERR) {
	return res;
    }

    /* finidh initializing agent data structures */
    res = agt_init2();
    if (res != NO_ERR) {
	return res;
    }

#ifdef NETCONFD_DEBUG
    if (LOGDEBUG) {
	log_debug("\nnetconfd init OK, ready for sessions\n");
    }
#endif

    return NO_ERR;

}  /* cmn_init */


/********************************************************************
 * FUNCTION netconfd_run
 *  
 * Startup and run the NCX server loop
 * 
 * RETURNS:
 *    status:  NO_ERR if startup OK and then run OK
 *             this will be a delayed return code
 *             some error if server startup failed
 *             e.g., socket already in use
 *********************************************************************/
static status_t
    netconfd_run (void)
{

    status_t  res;

#ifdef NETCONFD_DEBUG
    if (LOGDEBUG) {
	log_debug("\nStart running netconfd agent\n");
    }
#endif

    res = agt_ncxserver_run();
    if (res != NO_ERR) {
	log_error("\nncxserver failed (%s)",
		  get_error_string(res));
    }
    return res;
    
}  /* netconfd_run */


/********************************************************************
 * FUNCTION netconfd_cleanup
 * 
 * 
 * 
 *********************************************************************/
static void
    netconfd_cleanup (void)
{

#ifdef NETCONFD_DEBUG
    if (LOGDEBUG) {
	log_debug("\nShutting down netconf agent\n");
    }
#endif

    /* Cleanup the Netconf Agent Library */
    agt_cleanup();

    /* cleanup the NCX engine and registries */
    ncx_cleanup();

}  /* netconfd_cleanup */


/********************************************************************
*                                                                   *
*			FUNCTION main				    *
*                                                                   *
*********************************************************************/
int 
    main (int argc, 
	  const char *argv[])
{
    status_t           res;
    boolean            showver, stdlog, done;
    help_mode_t        showhelpmode;
    ncx_shutdowntyp_t  shutmode;

#ifdef MEMORY_DEBUG
    mtrace();
#endif

    done = FALSE;

    malloc_cnt = 0;
    free_cnt = 0;

    while (!done) {

	res = cmn_init(argc, argv, &showver, &showhelpmode);

	stdlog = !log_is_open();
    
	if (res != NO_ERR) {
	    log_error("\nnetconfd: init returned (%s)", 
		      get_error_string(res));
	    agt_request_shutdown(NCX_SHUT_EXIT);
	} else {
	    if (showver) {
		log_write("\nnetconfd version %s\n", progver);
		agt_request_shutdown(NCX_SHUT_EXIT);
	    } else if (showhelpmode != HELP_MODE_NONE) {
		help_program_module(NETCONFD_MOD,
				    NETCONFD_CLI,
				    showhelpmode);
		agt_request_shutdown(NCX_SHUT_EXIT);
	    } else {
		res = netconfd_run();
		if (res != NO_ERR) {
		    agt_request_shutdown(NCX_SHUT_EXIT);
		}
	    }
	}

	shutmode = agt_shutdown_mode_requested();

	netconfd_cleanup();

	if (shutmode == NCX_SHUT_EXIT) {
	    done = TRUE;
	}
    }

    if (malloc_cnt != free_cnt) {
	printf("\n*** netconfd error: memory leak (m:%u f:%u)\n", 
	       malloc_cnt, free_cnt);
    }

    print_errors();

    if (stdlog) {
	printf("\n");
    }

    return 0;

} /* main */

/* END netconfd.c */



