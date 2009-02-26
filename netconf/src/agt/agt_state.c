/*  FILE: agt_state.c

   NETCONF State Data Model implementation: Agent Side Support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
24feb09      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>
#include  <unistd.h>
#include  <errno.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include  "agt.h"
#endif

#ifndef _H_agt_cap
#include  "agt_cap.h"
#endif

#ifndef _H_agt_cb
#include  "agt_cb.h"
#endif

#ifndef _H_agt_ses
#include  "agt_ses.h"
#endif

#ifndef _H_agt_state
#include  "agt_state.h"
#endif

#ifndef _H_agt_util
#include  "agt_util.h"
#endif

#ifndef _H_cfg
#include  "cfg.h"
#endif

#ifndef _H_getcb
#include  "getcb.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncxmod
#include  "ncxmod.h"
#endif

#ifndef _H_ncxtypes
#include  "ncxtypes.h"
#endif

#ifndef _H_rpc
#include  "rpc.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_ses_msg
#include  "ses_msg.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define AGT_STATE_DEBUG 1
#endif

#define AGT_STATE_TOP_CONTAINER (const xmlChar *)"netconf"


#define AGT_STATE_OBJ_CONFIGURATION   (const xmlChar *)"configuration"
#define AGT_STATE_OBJ_CONFIGURATIONS  (const xmlChar *)"configurations"
#define AGT_STATE_OBJ_LOCKS           (const xmlChar *)"locks"

/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

static boolean       agt_state_init_done = FALSE;
static ncx_module_t *statemod = NULL;


/********************************************************************
* FUNCTION make_configuration_val
*
* make a val_value_t struct for a specified configuration
*
INPUTS:
*   confname == config name
*   confobj == config object to use
*   res == address of return status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*   malloced value struct or NULL if some error
*********************************************************************/
static val_value_t *
    make_configuration_val (const xmlChar *confname,
			    const obj_template_t *confobj,
			    status_t *res)
{
    const obj_template_t  *nameobj, *testobj;
    val_value_t           *confval, *nameval, *leafval;

    nameobj = obj_find_child(confobj, AGT_STATE_MODULE, 
			     NCX_EL_NAME);
    if (!nameobj) {
	*res = SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    /* create configuration node */
    confval = val_new_value();
    if (!confval) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    val_init_from_template(confval, confobj);

    /* create configuration/name */
    nameval = val_new_value();
    if (!nameval) {
	val_free_value(confval);
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    val_init_from_template(nameval, nameobj);
    val_add_child(nameval, confval);
    
    /* create configuration/name/<config-name> */
    testobj = obj_find_child(nameobj, 
			     AGT_STATE_MODULE,
			     confname);
    if (!testobj) {
	val_free_value(confval);
	*res = SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    leafval = val_new_value();
    if (!leafval) {
	val_free_value(confval);
	*res = ERR_INTERNAL_MEM;
	return NULL;
    } else {
	val_init_from_template(leafval, testobj);
	val_add_child(leafval, nameval);
    }

    /* create configuration/locks */
    testobj = obj_find_child(confobj, AGT_STATE_MODULE, 
			     AGT_STATE_OBJ_LOCKS);
    if (!testobj) {
	val_free_value(confval);
	*res = SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
    leafval = val_new_value();
    if (!leafval) {
	val_free_value(confval);
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }
    val_init_from_template(leafval, testobj);
    val_add_child(leafval, confval);

    *res = NO_ERR;
    return confval;

} /* make_configuration_val */


/************* E X T E R N A L    F U N C T I O N S ***************/


/********************************************************************
* FUNCTION agt_state_init
*
* INIT 1:
*   Initialize the agent state monitor module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_state_init (void)
{
    status_t   res;

    if (agt_state_init_done) {
	return ERR_INTERNAL_INIT_SEQ;
    }

#ifdef AGT_STATE_DEBUG
    log_debug2("\nagt: Loading netconf-state module");
#endif

    /* load the netconf-state module */
    res = ncxmod_load_module(AGT_STATE_MODULE, NULL, &statemod);
    if (res != NO_ERR) {
	return res;
    }

    agt_state_init_done = TRUE;
    return NO_ERR;

}  /* agt_state_init */


/********************************************************************
* FUNCTION agt_state_init2
*
* INIT 2:
*   Initialize the monitoring data structures
*   This must be done after the <running> config is loaded
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_state_init2 (void)
{
    const obj_template_t  *topobj, *confsobj, *confobj;
    val_value_t           *topval, *capsval, *confsval, *confval;
    cfg_template_t        *runningcfg;
    status_t  res;

    if (!agt_state_init_done) {
	return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    runningcfg = cfg_get_config(NCX_EL_RUNNING);
    if (!runningcfg || !runningcfg->root) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* create the /netconf node */
    topobj = obj_find_template_top(statemod, 
				   AGT_STATE_MODULE,
				   AGT_STATE_TOP_CONTAINER);
    if (!topobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    confsobj = obj_find_child(topobj, 
			      AGT_STATE_MODULE, 
			      AGT_STATE_OBJ_CONFIGURATIONS);
    if (!confsobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    confobj = obj_find_child(confsobj,
			     AGT_STATE_MODULE,
			     AGT_STATE_OBJ_CONFIGURATION);
    if (!confobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* add /netconf */
    topval = val_new_value();
    if (!topval) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(topval, topobj);
    val_add_child(topval, runningcfg->root);

    /* add /netconf/capabilities */
    capsval = val_clone(agt_cap_get_capsval());
    if (!capsval) {
	return ERR_INTERNAL_MEM;
    } else {
	/* change the namespace to this module, 
	 * and get rid of the netconf NSID 
	 */
	val_change_nsid(capsval, statemod->nsid);
	val_add_child(capsval, topval);
    }

    /* add /netconf/configurations */
    confsval = val_new_value();
    if (!confsval) {
	return ERR_INTERNAL_MEM;
    } else {
	val_init_from_template(confsval, confsobj);
	val_add_child(confsval, topval);
    }

    /* add /netconf/configurations/configuration[1] */
    if (agt_cap_std_set(CAP_STDID_CANDIDATE)) {
	confval = make_configuration_val(NCX_EL_CANDIDATE,
					 confobj,
					 &res);
	if (!confval) {
	    return res;
	}
	val_add_child(confval, confsval);
    }

    /* add /netconf/configurations/configuration[2] */
    confval = make_configuration_val(NCX_EL_RUNNING,
				     confobj,
				     &res);
    if (!confval) {
	return res;
    }
    val_add_child(confval, confsval);

    /* add /netconf/configurations/configuration[3] */
    if (agt_cap_std_set(CAP_STDID_STARTUP)) {
	confval = make_configuration_val(NCX_EL_STARTUP,
					 confobj,
					 &res);
	if (!confval) {
	    return res;
	}
	val_add_child(confval, confsval);
    }

#if 0
    /* register callback functions for the parameters */
    res = agt_cb_register_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_LINESIZE,
					FORALL,
					AGT_CB_APPLY,
					set_linesize);
    if (res != NO_ERR) {
	return res;
    }

    res = agt_cb_register_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_WITHDEF_DEFAULT,
					FORALL,
					AGT_CB_APPLY,
					set_withDefDefault);
    if (res != NO_ERR) {
	return res;
    }

    res = agt_cb_register_parm_callback(AGT_SES_MODULE,
					AGT_SES_MY_SESSION,
					AGT_SES_WITHMETA_DEFAULT,
					FORALL,
					AGT_CB_APPLY,
					set_withMetaDefault);

    return res;
#endif


    return NO_ERR;

}  /* agt_state_init2 */


/********************************************************************
* FUNCTION agt_state_cleanup
*
* Cleanup the module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    agt_state_cleanup (void)
{
    if (agt_state_init_done) {
	statemod = NULL;
	agt_state_init_done = FALSE;
    }

}  /* agt_state_cleanup */



/* END file agt_state.c */
