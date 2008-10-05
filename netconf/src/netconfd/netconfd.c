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
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>

#define _C_main 1

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include  "agt.h"
#endif

#ifndef _H_agt_cb
#include  "agt_cb.h"
#endif

#ifndef _H_agt_ncxserver
#include  "agt_ncxserver.h"
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
/* #define NETCONFD_DEBUG_TEST 1 */
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
static char progver[] = "0.8.1";


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
    log_debug2("\nnetconfd: Loading netconfd core module parmset");
#endif

    /* load in the agent boot parameter definition file */
    res = ncxmod_load_module(NCXMOD_NETCONFD);
    if (res != NO_ERR) {
	return res;
    }

#ifdef NETCONFD_DEBUG
    log_debug2("\nnetconfd: Loading NETCONF Module");
#endif

    /* load in the NETCONF data types and RPC methods */
    res = ncxmod_load_module((const xmlChar *) NCXMOD_NETCONF);
    if (res != NO_ERR) {
	return res;
    }

    /* initialize the NETCONF operation attribute 
     * MUST be after the netconf.ncx module is loaded
     */
    res = ncx_stage2_init();
    if (res != NO_ERR) {
	return res;
    }

#ifdef NETCONFD_DEBUG
    log_debug2("\nnetconfd: Loading Debug Test Module");
#endif

#ifdef REMOVED_USE_LOAD_COMMAND
#ifdef NETCONFD_DEBUG_TEST
    /* Load test module */
    res = ncxmod_load_module((const xmlChar *) "test");
    if (res != NO_ERR) {
	return res;
    }
#endif
#endif

    return NO_ERR;

}  /* load_core_schema */


#ifdef NETCONFD_DEBUG_TEST
static status_t 
    test_callback (ses_cb_t  *scb,
		   rpc_msg_t *msg,
		   agt_cbtyp_t cbtyp,
		   op_editop_t  editop,
		   val_value_t  *newval,
		   val_value_t  *curval)
{
    (void)scb;
    (void)msg;
    (void)cbtyp;
    (void)editop;
    (void)newval;
    (void)curval;
    return NO_ERR;
}
#endif



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
	      boolean *showhelp)
{
    status_t   res;
#ifdef NETCONFD_DEBUG_TEST
    agt_cb_fnset_t cbset;
#endif

#ifdef NETCONFD_DEBUG
    int   i;
#endif

    log_debug_t  dlevel;

    /* set the default debug output level */
#ifdef DEBUG
    dlevel = LOG_DEBUG_DEBUG;
#else
    dlevel = LOG_DEBUG_WARN;
#endif

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     */
    res = ncx_init(FALSE, dlevel, "\nStarting netconfd...");
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
    log_debug2("\nnetconfd: Starting Netconf Agent Library");
#endif

    /* at this point, modules that need to read config
     * params can be initialized
     */

    /* Load all the supported core modules */
    res = load_core_schema();
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the Netconf Agent Library
     * CMD-P1) Get any command line parameters 
     */
    res = agt_init(argc, argv, showver, showhelp);
    if (res != NO_ERR) {
	return res;
    }

    /* check quick-exit mode */
    if (*showver || *showhelp) {
	return NO_ERR;
    }

    /* CMD-P2) Get any environment string parameters */
    /***/

    /* CMD-P3) Get any PS file parameters */
    /***/


#ifdef NETCONFD_DEBUG_TEST

    memset(&cbset, 0x0, sizeof(agt_cb_fnset_t));
    cbset.cbfn[AGT_CB_LOAD_MOD] = test_callback;
    cbset.cbfn[AGT_CB_UNLOAD_MOD] = test_callback;
    cbset.cbfn[AGT_CB_VALIDATE] = test_callback;
    cbset.cbfn[AGT_CB_APPLY] = test_callback;

    res = agt_cb_register_callbacks((const xmlChar *)"test",
				    (const xmlChar *)"/t:test1",
				    (const xmlChar *)"2007-04-01",
				    &cbset);

    if (res != NO_ERR) {
	SET_ERROR(res);
	return res;
    }

#endif

#ifdef NETCONFD_DEBUG
    log_debug("\nnetconfd init OK, ready for sessions\n");
#endif

    return NO_ERR;

}  /* cmn_init */


/********************************************************************
 * FUNCTION netconfd_run
 * 
 * 
 * 
 *********************************************************************/
static void
    netconfd_run (void)
{

    status_t  res;

#ifdef NETCONFD_DEBUG
    log_debug2("\nStart running netconfd agent\n");
#endif

    res = agt_ncxserver_run();
    if (res != NO_ERR) {
	log_error("\nncxserver failed (%s)",
		  get_error_string(res));
    }
    
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
    log_debug2("\nShutting down netconf agent\n");
#endif

#ifdef NETCONFD_DEBUG_TEST
    agt_cb_unregister_callbacks((const xmlChar *)"test",
				(const xmlChar *)"/t:test1");
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
    status_t   res;
    boolean    showver, showhelp, stdlog;

    malloc_cnt = 0;
    free_cnt = 0;


    res = cmn_init(argc, argv, &showver, &showhelp);

    stdlog = !log_is_open();
    
#ifdef NETCONFD_DEBUG
    log_debug2("\nnetconfd: init returned (%d)", res);
#endif

    if (res==NO_ERR) {
	if (showver) {
	    log_write("\nnetconfd version %s\n", progver);
	} else if (showhelp) {
	    help_program_module(NETCONFD_MOD,
				NETCONFD_CLI,
				HELP_MODE_FULL);
	} else {
	    netconfd_run();
	}
    }

    netconfd_cleanup();

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



