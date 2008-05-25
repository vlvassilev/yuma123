/*  FILE: agt_cli.c

	Set agent CLI parameters

        This module is called before any other agent modules
        have been initialized.  Only core ncx library functions
        can be called while processing agent CLI parameters

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
03feb06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt_cli
#include "agt_cli.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_ps_parse
#include "ps_parse.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define AGT_CLI_DEBUG 1 */

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static ps_parmset_t *cli_ps = NULL;


/********************************************************************
* FUNCTION set_agent_profile
* 
* Get the initial agent profile variables for now
* Set the agt_profile data structure
*
* INPUTS:
*   ps == parmset from CLI parsing or NULL if none
*   agt_profile == pointer to profile struct to fill in
*
* OUTPUTS:
*  *agt_profile is filled in with params of defaults
*
* RETURNS:
*   none
*********************************************************************/
static void
    set_agent_profile (ps_parmset_t *ps,
		       agt_profile_t *agt_profile)
{
    val_value_t  *val;
    status_t      res;

    /* check if there is any CLI data to read */
    if (!ps) {
	return;
    }

    /* get log param */
    res = ps_get_parmval(ps, NCX_EL_LOG, &val);
    if (res == NO_ERR) {
	agt_profile->agt_logfile = VAL_STR(val);
    }

    /* get log-append param */
    if (ps_find_parm(ps, NCX_EL_LOGAPPEND)) {
	agt_profile->agt_logappend = TRUE;
    }

    /* get log-level param */
    res = ps_get_parmval(ps, NCX_EL_LOGLEVEL, &val);
    if (res == NO_ERR) {
	agt_profile->agt_loglevel = 
	    log_get_debug_level_enum((const char *)VAL_STR(val));
    }

    /* get no-startup startup param choice */
    if (ps_find_parm(ps, AGT_CLI_NOSTARTUP)) {
	agt_profile->agt_usestartup = FALSE;
    } else {
	/* get startup param */
	res = ps_get_parmval(ps, AGT_CLI_STARTUP, &val);
	if (res == NO_ERR) {
	    agt_profile->agt_startup = VAL_STR(val);
	}
    }

    /* get modpath param */
    res = ps_get_parmval(ps, NCX_EL_MODPATH, &val);
    if (res == NO_ERR) {
	agt_profile->agt_modpath = VAL_STR(val);
    }

    /* get datapath param */
    res = ps_get_parmval(ps, NCX_EL_DATAPATH, &val);
    if (res == NO_ERR) {
	agt_profile->agt_datapath = VAL_STR(val);
    }

    /* get runpath param */
    res = ps_get_parmval(ps, NCX_EL_RUNPATH, &val);
    if (res == NO_ERR) {
	agt_profile->agt_runpath = VAL_STR(val);
    }


} /* set_agent_profile */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_cli_process_input
*
* Process the param line parameters against the hardwired
* parmset for the netconfd program
*
* INPUTS:
*    argc == argument count
*    argv == array of command line argument strings
*    agt_profile == agent profile struct to fill in
*    showver == address of version return quick-exit status
*    showhelp == address of help return quick-exit status
*
* OUTPUTS:
*    *agt_profile is filled in, with parms gathered or defaults
*    *showver == TRUE if user requsted version quick-exit mode
*    *showhelp == TRUE if user requsted help quick-exit mode
*
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
status_t
    agt_cli_process_input (int argc,
			   const char *argv[],
			   agt_profile_t *agt_profile,
			   boolean *showver,
			   boolean *showhelp)
{
    psd_template_t  *psd;
    ps_parmset_t    *ps;
    ps_parm_t       *parm;
    ncx_node_t       dtyp;
    status_t         res;

    /* find the parmset definition in the registry */
    dtyp = NCX_NT_PSD;
    psd = (psd_template_t *)
	def_reg_find_moddef(AGT_CLI_MODULE, AGT_CLI_PSD, &dtyp);
    if (!psd) {
	return ERR_NCX_NOT_FOUND;
    }

    /* parse the command line against the PSD */
    res = NO_ERR;
    ps = NULL;
    if (argc > 1) {
	ps = ps_parse_cli(argc, argv, psd,
			  FULLTEST, PLAINMODE, TRUE, &res);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* transfer the parmset values */
    set_agent_profile(ps, agt_profile);

    /* check the quick exit parameters */
    if (ps) {
	/* check if version mode requested */
	parm = ps_find_parm(ps, NCX_EL_VERSION);
	*showver = (parm) ? TRUE : FALSE;

	/* check if help mode requested */
	parm = ps_find_parm(ps, NCX_EL_HELP);
	*showhelp = (parm) ? TRUE : FALSE;
    } else {
	*showver = FALSE;
	*showhelp = FALSE;
    }

    /* cleanup and exit */
    if (ps) {
	cli_ps = ps;
    }

    return res;

} /* agt_cli_process_input */


/********************************************************************
* FUNCTION agt_cli_get_parmset
*
*   Retrieve the command line parameter set from boot time
*
* RETURNS:
*    pointer to parmset or NULL if none
*********************************************************************/
const ps_parmset_t *
    agt_cli_get_parmset (void)
{
    return cli_ps;
} /* agt_cli_get_parmset */


/********************************************************************
* FUNCTION agt_cli_cleanup
*
*   Cleanup the module static data
*
*********************************************************************/
void
    agt_cli_cleanup (void)
{
    if (cli_ps) {
	ps_free_parmset(cli_ps);
	cli_ps = NULL;
    }

} /* agt_cli_cleanup */

/* END file agt_cli.c */
