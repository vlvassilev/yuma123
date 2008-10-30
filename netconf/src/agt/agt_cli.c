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

#ifndef _H_cli
#include "cli.h"
#endif

#ifndef _H_conf
#include "conf.h"
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

#ifndef _H_obj
#include "obj.h"
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
static val_value_t *cli_val = NULL;


/********************************************************************
* FUNCTION set_agent_profile
* 
* Get the initial agent profile variables for now
* Set the agt_profile data structure
*
* INPUTS:
*   valset == value set for CLI parsing or NULL if none
*   agt_profile == pointer to profile struct to fill in
*
* OUTPUTS:
*   *agt_profile is filled in with params of defaults
*
* RETURNS:
*   none
*********************************************************************/
static void
    set_agent_profile (val_value_t *valset,
		       agt_profile_t *agt_profile)
{
    val_value_t  *val;
    uint32        i;
    boolean       done;

    /* check if there is any CLI data to read */
    if (!valset) {
	/* assumes agt_profile already has default values */
	return;
    }

    /* check all the netconfd CLI parameters;
     * follow the order in netconfd.yang since
     * no action will be taken until all params are collected
     */

    /* config param:
     * do not look for config file param in this function
     * so it can be called for the config file itself
     */

    /* get datapath param */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_DATAPATH);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_datapath = VAL_STR(val);
    }

    /* help parameter checked externally */

    /* get log param */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_LOG);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_logfile = VAL_STR(val);
    }

    /* get log-append param */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_LOGAPPEND);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_logappend = TRUE;
    }

    /* get log-level param */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_LOGLEVEL);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_loglevel = 
	    log_get_debug_level_enum((const char *)VAL_STR(val));
    }

    /* get modpath param */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_MODPATH);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_modpath = VAL_STR(val);
    }

    /* get leaf-list port parameter */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_PORT);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_ports[0] = VAL_UINT16(val);

	val = val_find_next_child(valset, AGT_CLI_MODULE,
				  NCX_EL_PORT, val);
	while (val) {
	    done = FALSE;
	    for (i = 0;	 i < AGT_MAX_PORTS && !done; i++) {
		if (agt_profile->agt_ports[i] == VAL_UINT16(val)) {
		    done = TRUE;
		} else if (agt_profile->agt_ports[i] == 0) {
		    agt_profile->agt_ports[i] = VAL_UINT16(val);
		    done = TRUE;
		}
	    }
	    val = val_find_next_child(valset, AGT_CLI_MODULE,
				      NCX_EL_PORT, val);
	}
    }

    /* get runpath param */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_RUNPATH);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_runpath = VAL_STR(val);
    }

    /* start choice: get no-startup startup param choice */
    val = val_find_child(valset, AGT_CLI_MODULE, AGT_CLI_NOSTARTUP);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_usestartup = FALSE;
    }

    /* start choice: OR get startup param */
    val = val_find_child(valset, AGT_CLI_MODULE, AGT_CLI_STARTUP);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_startup = VAL_STR(val);
    }

    /* get target param */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_TARGET);
    if (val && val->res == NO_ERR) {
	if (!xml_strcmp(VAL_ENUM_NAME(val), NCX_EL_RUNNING)) {
	    agt_profile->agt_targ = NCX_AGT_TARG_RUNNING;
	    agt_profile->agt_start = NCX_AGT_START_DISTINCT;
	    agt_profile->agt_del_startup = TRUE;
	} else if (!xml_strcmp(VAL_ENUM_NAME(val), 
			       NCX_EL_CANDIDATE)) {
	    agt_profile->agt_targ = NCX_AGT_TARG_CANDIDATE;
	    agt_profile->agt_start = NCX_AGT_START_MIRROR;
	    agt_profile->agt_del_startup = FALSE;
	}
    }

    /* version param handled externally */

    /* get xmlorder param */
    val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_XMLORDER);
    if (val && val->res == NO_ERR) {
	agt_profile->agt_xmlorder = TRUE;
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
    const obj_template_t  *obj;
    val_value_t           *valset, *val;
    ncx_node_t             dtyp;
    status_t               res;

    /* find the parmset definition in the registry */
    dtyp = NCX_NT_OBJ;
    obj = (const obj_template_t *)
	def_reg_find_moddef(AGT_CLI_MODULE, 
			    AGT_CLI_CONTAINER, &dtyp);
    if (!obj) {
	log_error("\nError: netconfd module with CLI definitions not loaded");
	return ERR_NCX_NOT_FOUND;
    }

    /* parse the command line against the object template */
    res = NO_ERR;
    valset = NULL;
    if (argc > 1) {
	valset = cli_parse(argc, argv, obj,
			   FULLTEST, PLAINMODE, TRUE, &res);
	if (res != NO_ERR) {
	    return res;
	}
    }

    if (valset) {
	/* transfer the parmset values */
	set_agent_profile(valset, agt_profile);

	/* next get any params from the conf file */
	val = val_find_child(valset, AGT_CLI_MODULE, 
			     NCX_EL_CONFIG);
	if (val) {
	    if (val->res == NO_ERR) {
		/* try the specified config location */
		agt_profile->agt_conffile = VAL_STR(val);
		res = conf_parse_val_from_filespec(VAL_STR(val), 
						   valset, 
						   TRUE, TRUE);
		if (res != NO_ERR) {
		    return res;
		} else {
		    /* transfer the parmset values again */
		    set_agent_profile(valset, agt_profile);
		}
	    }
	} /* else no default config location */

	/* check if version mode requested */
	val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_VERSION);
	*showver = (val) ? TRUE : FALSE;

	/* check if help mode requested */
	val = val_find_child(valset, AGT_CLI_MODULE, NCX_EL_HELP);
	*showhelp = (val) ? TRUE : FALSE;
    } else {
	*showver = FALSE;
	*showhelp = FALSE;
    }

    /* cleanup and exit */
    cli_val = valset;

    return res;

} /* agt_cli_process_input */


/********************************************************************
* FUNCTION agt_cli_get_valset
*
*   Retrieve the command line parameter set from boot time
*
* RETURNS:
*    pointer to parmset or NULL if none
*********************************************************************/
const val_value_t *
    agt_cli_get_valset (void)
{
    return cli_val;
} /* agt_cli_get_valset */


/********************************************************************
* FUNCTION agt_cli_cleanup
*
*   Cleanup the module static data
*
*********************************************************************/
void
    agt_cli_cleanup (void)
{
    if (cli_val) {
	val_free_value(cli_val);
	cli_val = NULL;
    }

} /* agt_cli_cleanup */

/* END file agt_cli.c */
