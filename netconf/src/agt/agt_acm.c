/*  FILE: agt_acm.c

object identifiers:

container /nacm
leaf /nacm/noRuleDefault
container /nacm/groups
list /nacm/groups/group
leaf /nacm/groups/group/groupIdentity
leaf-list /nacm/groups/group/userName
container /nacm/rules
list /nacm/rules/moduleRule
leaf /nacm/rules/moduleRule/moduleName
leaf /nacm/rules/moduleRule/allowedRights
leaf-list /nacm/rules/moduleRule/allowedGroup
leaf /nacm/rules/moduleRule/comment
list /nacm/rules/rpcRule
leaf /nacm/rules/rpcRule/rpcModuleName
leaf /nacm/rules/rpcRule/rpcName
leaf /nacm/rules/rpcRule/allowedRights
leaf-list /nacm/rules/rpcRule/allowedGroup
leaf /nacm/rules/rpcRule/comment
list /nacm/rules/dataRule
leaf /nacm/rules/dataRule/name
leaf /nacm/rules/dataRule/path
leaf /nacm/rules/dataRule/allowedRights
leaf-list /nacm/rules/dataRule/allowedGroup
leaf /nacm/rules/dataRule/comment

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
04feb06      abb      begun
01aug08      abb      convert from NCX PSD to YANG OBJ design

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_acm
#include "agt_acm.h"
#endif

#ifndef _H_agt_cb
#include  "agt_cb.h"
#endif

#ifndef _H_agt_util
#include  "agt_util.h"
#endif

#ifndef _H_agt_val
#include  "agt_val.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
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

#ifndef _H_val_util
#include "val_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define AGT_ACM_DEBUG 1

#define AGT_ACM_MODULE      (const xmlChar *)"nacm"

#define nacm_I_nacmGroups (const xmlChar *)"nacmGroups"
#define nacm_I_superuser (const xmlChar *)"superuser"
#define nacm_I_admin (const xmlChar *)"admin"
#define nacm_I_guest (const xmlChar *)"guest"

#define nacm_N_allowedGroup (const xmlChar *)"allowedGroup"
#define nacm_N_allowedRights (const xmlChar *)"allowedRights"
#define nacm_N_comment (const xmlChar *)"comment"
#define nacm_N_dataRule (const xmlChar *)"dataRule"
#define nacm_N_group (const xmlChar *)"group"
#define nacm_N_groupIdentity (const xmlChar *)"groupIdentity"
#define nacm_N_groups (const xmlChar *)"groups"
#define nacm_N_moduleName (const xmlChar *)"moduleName"
#define nacm_N_moduleRule (const xmlChar *)"moduleRule"
#define nacm_N_nacm (const xmlChar *)"nacm"
#define nacm_N_name (const xmlChar *)"name"
#define nacm_N_noRuleDefault (const xmlChar *)"noRuleDefault"
#define nacm_N_path (const xmlChar *)"path"
#define nacm_N_rpcModuleName (const xmlChar *)"rpcModuleName"
#define nacm_N_rpcName (const xmlChar *)"rpcName"
#define nacm_N_rpcRule (const xmlChar *)"rpcRule"
#define nacm_N_rules (const xmlChar *)"rules"
#define nacm_N_userName (const xmlChar *)"userName"


#define nacm_OID_group (const xmlChar *)"/nacm/groups/group"
#define nacm_OID_moduleRule  (const xmlChar *)"/nacm/rules/moduleRule"

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/



/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean agt_acm_init_done = FALSE;

static ncx_module_t  *nacmmod;


#if 0
/********************************************************************
* FUNCTION get_nacm_root
*
* get the /nacm root object
*
* RETURNS:
*   pointer to root or NULL if none
*********************************************************************/
static val_value_t *
    get_nacm_root (void)
{
    cfg_template_t        *runningcfg;
    val_value_t           *nacmval;

    /* make sure the running config root is set */
    runningcfg = cfg_get_config(NCX_EL_RUNNING);
    if (!runningcfg || !runningcfg->root) {
	return NULL;
    }
    
    nacmval = val_find_child(runningcfg->root,
			     AGT_ACM_MODULE,
			     nacm_N_nacm);

    return nacmval;

}  /* get_nacm_root */


/********************************************************************
* FUNCTION get_group_for_user
*
* get the group name associated with this user name
*
* First call:
*   SET group to NULL
*  2nd to Nth call:
*   Leave *group set to its previous value
*
* This initial implementation uses a brute-force lookup
* instead of a cache approach.  May change later if
* this is too slow
*
* INPUTS:
*    groupsval == root value struct of all group entries
*    username == username
*    group == address of return group value struct
*    groupid == address of return groupIdentity value struct
*
* OUTPUTS:
*   *group == group value struct, if found
*   *groupid == groupIdentity value struct, if found
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_group_for_user (val_value_t *groupsval,
			const xmlChar *username,
			val_value_t **group,
			val_value_t **groupid)
{
    val_value_t   *nacmval, *groupval, *usernameval;
    
    *groupid = NULL;

    if (*group == NULL) {
	*group = val_get_first_child(groupsval);
    } else {
	*group = val_get_next_child(*group);
    }

    /* go through the groups in order and check the user list */
    for (groupval = *group;
	 groupval != NULL;
	 groupval = val_get_next_child(groupval)) {

	usernameval = val_find_child(group, 
				     AGT_ACM_MODULE, 
				     nacm_N_userName);
	while (usernameval != NULL) {
	    if (!xml_strcmp(username, VAL_STR(usernameval))) {
		*groupid = val_find_child(groupval,
					  AGT_ACM_MODULE,
					  nacm_N_groupIdentity);
		*group = groupval;
		return NO_ERR;
	    } else {
		usernameval = 
		    val_find_next_child(groupval,
					AGT_ACM_MODULE,
					nacm_N_groupIdentity,
					usernameval);
	    }
	}
    }

    return ERR_NCX_DEF_NOT_FOUND;

} /* get_group_for_user */


/********************************************************************
* FUNCTION check_rpc_rule
*
* Check the configured <rpcRule> element to see if the
* group list is allowed to invoke the specified RPC method
*
* INPUTS:
*    rpcobj == RPC object template requested
*    groupname == group name for this user
*    ruleval == value struct representing the current rpcRule
*    granted == address of return access granted status
*
* OUTPUTS:
*   If rule applies (returns TRUE)
*     *granted == TRUE if access to invoke RPC granted
*              == FALSE if access is not granted to invoke the RPC
*
* RETURNS:
*    TRUE if the rule applied to this group list
*    FALSE if the rule did not apply to this group list
*********************************************************************/
static boolean
    check_rpc_rule (const obj_template_t *rpcobj,
		    val_value_t groupval,
		    val_value_t *ruleval,
		    boolean *granted)
{
    const obj_rpc_t   *rpc;
    val_value_t       *target, *chval, *actype;
    boolean            match;

    rpc = rpcobj->def.rpc;

    /* check if this rule applies to the groupname */
    if (!group_in_list(groupname, ruleval)) {
	return FALSE;
    }

    /* group applies to check the rpcTarget
     * get the rpcTarget choice 
     */
    target = val_find_child(ruleval, AGT_ACM_MODULE, LEAF_RPCTARGET);
    if (!target) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
    chval = val_get_first_child(target);

    /* check if the RPC rule matches the target choice */
    if (!xml_strcmp(chval->name, LEAF_RPCTLIST)) {
	/* check if rpc-type in the rpcTypeList */
	match = ncx_string_in_list(rpc_get_rpctype_str(rpc->rpc_typ), 
				   &(VAL_LIST(chval)));
    } else if (!xml_strcmp(chval->name, LEAF_RPCLIST)) {
	match = node_in_list(rpc->nsid, rpc->name, chval);
    } else if (!xml_strcmp(chval->name, LEAF_XPATHEXPR)) {
	match = FALSE; /***/
    } else if (!xml_strcmp(chval->name, LEAF_ALL)) {
	match = TRUE;
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    /* check if rule target applies to this RPC method */
    if (!match) {
	return FALSE;
    }

    /* rules does apply to this group and target
     * check if the rule is permit or deny and set the
     * *granted return value
     */
    actype = val_find_child(ruleval, AGT_ACM_MODULE, LEAF_RULETYPE);
    if (!actype) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
    if (!xml_strcmp(VAL_STR(actype), ACCESS_PERMIT)) {
	*granted = TRUE;
    } else if (!xml_strcmp(VAL_STR(actype), ACCESS_DENY)) {
	*granted = FALSE;
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    return TRUE;

} /* check_rpc_rule */


/********************************************************************
* FUNCTION check_rpc_rules
*
* Check the configured <rpcRules> element to see if the
* user is allowed to invoke the specified RPC method
*
* INPUTS:
*    rpcobj == RPC template requested
*    user == username requesting access
*    cmode == current access control mode (warn, loose, strict)
*
* RETURNS:
*    TRUE if the user is allowed to invoke the RPC
*    FALSE if the user is not allowed to invoke the RPC (or some error)
*********************************************************************/
static boolean
    check_rpc_rules (const obj_template_t *rpcobj,
		     const xmlChar *user,
		     agt_acm_control_mode_t cmode)
{
    val_value_t      *valset, *val, *chval;
    const xmlChar    *group, *role;
    const obj_rpc_t  *rpc;
    status_t          res;
    boolean           granted, done;

    done = FALSE;
    granted = FALSE;
    rpc = rpcobj->def.rpc;

    /* get the accessControl parmset */
    valset = get_ac_valset();
    if (!valset) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    /* get the group and role for this user */
    res = get_group_from_user(user, &group, &role);
    if (res != NO_ERR) {
	log_warn("\nagt_acm: User %s not in any groups", user);
	return FALSE;
    }

    /* return TRUE if this is the super user */
    if (!xml_strcmp(role, ROLE_ROOT)) {
	return TRUE;
    }

    /* get the rpcRules value struct */
    val = val_find_child(accessControl, AGT_ACM_MODULE, PARM_RPCRULES);
    if (!val) {
	log_warn("\nagt_acm: No RPC Rules found");
	return FALSE;
    }

    /* find the first rule that matches this RPC method */
    for (chval = val_get_first_child(val);
	 chval != NULL && !done;
	 chval = val_get_next_child(chval)) {
	done = check_rpc_rule(rpcobj, group, chval, &granted);
    }

    /* check if any rule found */
    if (!done) {
	log_warn("\nagt_acm: RPC %s -- no rule found for user %s",
		 obj_get_name(rpcobj), user);
    }

    /* generate return value based on global control mode */
    switch (cmode) {
    case AGT_ACM_CM_WARN:
	if (!done || !granted) {
	    /*  gen_rpc_warning(user, rpc)  */  ;
	}
	/* fall through */
    case AGT_ACM_CM_LOOSE:
	if (!done) {
	    switch (rpc->rpc_typ) {
	    case RPC_TYP_OTHER:
	    case RPC_TYP_CONFIG:
		granted = FALSE;
		break;
	    case RPC_TYP_EXEC:
	    case RPC_TYP_MONITOR:
	    case RPC_TYP_DEBUG:
		granted = TRUE;
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
		granted = FALSE;
	    }
	}
	break;
    case AGT_ACM_CM_STRICT:
	if (!done) {
	    /* always allow the close-session command to be invoked */
	    if (obj_get_nsid(rpcobj) == xmlns_nc_id() &&
		!xml_strcmp(rpc->name, NCX_EL_CLOSE_SESSION)) {
		granted = TRUE;
	    } else {
		/* no rule for any other RPC method is denied access */
		granted = FALSE;
	    }
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	granted = FALSE;
    }

    return granted;

} /* check_rpc_rules */
#endif



/********************************************************************
* FUNCTION nacm_group_callback
*
* nacm callback function [test]
*
* INPUTS:
*    see agt/agt_cb.h  (agt_cb_pscb_t)
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    nacm_group_callback (ses_cb_t  *scb,
			 rpc_msg_t  *msg,
			 agt_cbtyp_t cbtyp,
			 op_editop_t  editop,
			 val_value_t  *newval,
			 val_value_t  *curval)
{

    (void)scb;
    (void)msg;
    (void)editop;
    (void)curval;

#ifdef AGT_ACM_DEBUG
    if (LOGDEBUG) {
	log_debug("\n\n**** agt_acm_cb: (G) op:%s, n:%s s:%s\n", 
		  op_editop_name(editop),
		  newval->name,
		  agt_cbtype_name(cbtyp));
    }
#endif

    switch (cbtyp) {
    case AGT_CB_LOAD_MOD:
    case AGT_CB_UNLOAD_MOD:
    case AGT_CB_VALIDATE:
    case AGT_CB_APPLY:
	break;
    case AGT_CB_COMMIT:
	break;
    case AGT_CB_ROLLBACK:
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    
    return NO_ERR;

} /* nacm_group_callback */


/********************************************************************
* FUNCTION nacm_rule_callback
*
* nacm callback function [test]
*
* INPUTS:
*    see agt/agt_cb.h  (agt_cb_pscb_t)
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    nacm_rule_callback (ses_cb_t  *scb,
			rpc_msg_t  *msg,
			agt_cbtyp_t cbtyp,
			op_editop_t  editop,
			val_value_t  *newval,
			val_value_t  *curval)
{

    (void)scb;
    (void)msg;
    (void)editop;
    (void)curval;

#ifdef AGT_ACM_DEBUG
    if (LOGDEBUG) {
	log_debug("\n\n**** agt_acm_cb: (R) op:%s, n:%s s:%s\n", 
		  op_editop_name(editop),
		  newval->name,
		  agt_cbtype_name(cbtyp));
    }
#endif

    switch (cbtyp) {
    case AGT_CB_LOAD_MOD:
    case AGT_CB_UNLOAD_MOD:
    case AGT_CB_VALIDATE:
    case AGT_CB_APPLY:
	break;
    case AGT_CB_COMMIT:
	break;
    case AGT_CB_ROLLBACK:
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    
    return NO_ERR;

} /* nacm_rule_callback */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_acm_init
* 
* Initialize the NCX Agent access control module
* 
* INPUTS:
*   none
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    agt_acm_init (void)
{
    status_t  res;

    if (agt_acm_init_done) {
	return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

#ifdef AGT_ACM_DEBUG
    log_debug2("\nagt: Loading NCX Access Control module");
#endif

    nacmmod = NULL;

    /* load in the access control parameters */
    res = ncxmod_load_module(AGT_ACM_MODULE, 
			     NULL, 
			     &nacmmod);
    if (res != NO_ERR) {
	return res;
    }

    agt_acm_init_done = TRUE;
    return NO_ERR;

}  /* agt_acm_init */


/********************************************************************
* FUNCTION agt_acm_init2
* 
* Phase 2:
*   Initialize the nacm.yang configuration data structures
* 
* INPUTS:
*   none
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    agt_acm_init2 (void)
{
    const obj_template_t  *nacmobj;
    cfg_template_t        *runningcfg;
    val_value_t           *nacmval;
    status_t               res;

    if (!agt_acm_init_done) {
	return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    res = agt_cb_register_callback(AGT_ACM_MODULE,
				   nacm_OID_group,
				   NULL,
				   nacm_group_callback);
    if (res != NO_ERR) {
	return res;
    }

    res = agt_cb_register_callback(AGT_ACM_MODULE,
				   nacm_OID_moduleRule,
				   NULL,
				   nacm_rule_callback);
    if (res != NO_ERR) {
	return res;
    }

    /* make sure the running config root is set */
    runningcfg = cfg_get_config(NCX_EL_RUNNING);
    if (!runningcfg || !runningcfg->root) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check to see if the /nacm branch already exists
     * if so, then skip all this init stuff
     */
    nacmval = val_find_child(runningcfg->root,
			     AGT_ACM_MODULE,
			     nacm_N_nacm);
    if (nacmval) {
	/* minimum init done OK, so just exit */
	agt_acm_init_done = TRUE;
	return NO_ERR;
    }
	
    /* did not find the /nacm node so create one;
     * get all the static object nodes first 
     */
    nacmobj = obj_find_template_top(nacmmod, 
				    AGT_ACM_MODULE,
				    nacm_N_nacm);
    if (!nacmobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* create the static structure for the /nacm data model
     * start with the top node /nacm
     */
    nacmval = val_new_value();
    if (!nacmval) {
	res = ERR_INTERNAL_MEM;
    } else {
	val_init_from_template(nacmval, nacmobj);

	/* handing off the malloced memory here */
	val_add_child(nacmval, runningcfg->root);

	/* add /nacm/noRuleDefault */
	res = val_add_defaults(nacmval, FALSE);
    }

    return res;

}  /* agt_acm_init2 */


/********************************************************************
* FUNCTION agt_acm_cleanup
*
* Cleanup the NCX Agent access control module
* 
*********************************************************************/
void
    agt_acm_cleanup (void)
{
    if (!agt_acm_init_done) {
	return;
    }

    agt_cb_unregister_callbacks(AGT_ACM_MODULE,	
				nacm_OID_group);
    agt_cb_unregister_callbacks(AGT_ACM_MODULE,	
				nacm_OID_moduleRule);
    nacmmod = NULL;
    agt_acm_init_done = FALSE;

}   /* agt_acm_cleanup */


/********************************************************************
* FUNCTION agt_acm_rpc_allowed
*
* Check if the specified user is allowed to invoke an RPC
* 
* INPUTS:
*   user == user name string
*   rpcobj == obj_template_t for the RPC method to check
*
* RETURNS:
*   TRUE if user allowed invoke this RPC; FALSE otherwise
*********************************************************************/
boolean 
    agt_acm_rpc_allowed (const xmlChar *user,
			 const obj_template_t *rpcobj)
{

#ifdef DEBUG
    if (!user || !rpcobj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    /* super user is allowed to access anything */
    if (!xml_strcmp(user, NCX_SUPERUSER)) {
	return TRUE;
    }

    /* everybody is allowed to close their own session */
    if (obj_get_nsid(rpcobj) == xmlns_nc_id() &&
	!xml_strcmp(obj_get_name(rpcobj),
		    NCX_EL_CLOSE_SESSION)) {
	return TRUE;
    }


    /*** TEMP ***/
    /* return check_rpc_rules(rpcobj, user, acmode); */
    return TRUE;

}   /* agt_acm_rpc_allowed */


/********************************************************************
* FUNCTION agt_acm_val_write_allowed
*
* Check if the specified user is allowed to access a value node
* The val->obj template will be checked against the val->editop
* requested access and the user's configured max-access
* 
* INPUTS:
*   user == user name string
*   val  == val_value_t in progress to check
*
* RETURNS:
*   TRUE if user allowed this level of access to the value node
*********************************************************************/
boolean 
    agt_acm_val_write_allowed (const xmlChar *user,
			       const val_value_t *val)
{
#ifdef DEBUG
    if (!user || !val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    /* super user is allowed to access anything */
    if (!xml_strcmp(user, NCX_SUPERUSER)) {
	return TRUE;
    }

    return TRUE;   /* !!! TEMP !!! */

}   /* agt_acm_val_write_allowed */


/********************************************************************
* FUNCTION agt_acm_val_read_allowed
*
* Check if the specified user is allowed to read a value node
* 
* INPUTS:
*   user == user name string
*   val  == val_value_t in progress to check
*
* RETURNS:
*   TRUE if user allowed read access to the value node
*********************************************************************/
boolean 
    agt_acm_val_read_allowed (const xmlChar *user,
			     const val_value_t *val)
{
#ifdef DEBUG
    if (!user || !val) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    /* super user is allowed to access anything */
    if (!xml_strcmp(user, NCX_SUPERUSER)) {
	return TRUE;
    }

    return TRUE;   /* !!! TEMP !!! */

}   /* agt_acm_val_read_allowed */


/* END file agt_acm.c */
