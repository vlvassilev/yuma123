/*  FILE: agt_acm.c

		
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


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define AGT_ACM_DEBUG 1

#define AGT_ACM_MODULE      (const xmlChar *)"nacm"

#define AGT_ACM_CBPATH    (const xmlChar *)"/nacm:accessControl"

#define AGT_ACM_DEF_MODE  AGT_ACM_CM_LOOSE

#define AGT_ACM_CONTAINER   (const xmlChar *)"accessControl"
#define AGT_ACM_INSTANCE_ID (const xmlChar *)"/accessControl"

#define PARM_CONFIGCAPS (const xmlChar *)"configCapabilities"
#define PARM_PROFILE    (const xmlChar *)"profile"
#define PARM_GROUPS     (const xmlChar *)"groups"
#define PARM_RPCRULES   (const xmlChar *)"rpcRules"
#define PARM_DATARULES  (const xmlChar *)"dataRules"
#define PARM_NOTIFRULES (const xmlChar *)"notificationRules"

#define LEAF_AC_MODE (const xmlChar *)"accessControlMode"
#define LEAF_GLOBAL_CONFIG (const xmlChar *)"globalConfig"
#define LEAF_GROUP_CONFIG (const xmlChar *)"groupConfig"
#define LEAF_RPC_CONFIG (const xmlChar *)"rpcAccessConfig"
#define LEAF_RPCTYPE_CONFIG (const xmlChar *)"rpcTypeAccessConfig"
#define LEAF_DATA_CONFIG (const xmlChar *)"databaseAccessConfig"
#define LEAF_NOTIF_CONFIG (const xmlChar *)"notificationAccessConfig"

#define LEAF_USERS        (const xmlChar *)"users"
#define LEAF_USER         (const xmlChar *)"user"
#define LEAF_ROLE         (const xmlChar *)"role"
#define LEAF_RULETYPE     (const xmlChar *)"ruleType"
#define LEAF_NAME         (const xmlChar *)"name"

#define LEAF_GROUPLIST    (const xmlChar *)"groupList"
#define LEAF_XPATHEXPR    (const xmlChar *)"xpathExpr"
#define LEAF_ALL          (const xmlChar *)"all"

#define LEAF_RPCTARGET    (const xmlChar *)"rpcTarget"
#define LEAF_RPCTLIST     (const xmlChar *)"rpcTypeList"
#define LEAF_RPCLIST      (const xmlChar *)"rpcMethodList"

#define LEAF_NSURI        (const xmlChar *)"namespaceUri"
#define LEAF_ELNAMES      (const xmlChar *)"elementNames"

#define ROLE_ROOT         (const xmlChar *)"root"
#define ROLE_ADMIN        (const xmlChar *)"admin"
#define ROLE_GUEST        (const xmlChar *)"guest"

#define ACCESS_PERMIT     (const xmlChar *)"permit"
#define ACCESS_DENY       (const xmlChar *)"deny"


#define MAX_GROUPS 16

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

typedef struct user_ent_t_ {
    dlq_hdr_t hdr;
    xmlChar *user;
    const xmlChar *group;
    const xmlChar *role;
} user_ent_t;

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean agt_acm_init_done = FALSE;

static agt_acm_config_caps_t configCaps;

static agt_acm_control_mode_t accessControlMode;
static val_value_t   *accessControl;
static boolean        acStale;
static boolean        acModeStale;
static boolean        userQStale;
static dlq_hdr_t      userQ;

#if 0
/********************************************************************
* FUNCTION new_user_ent
*
* Create a new user entry
*
* INPUTS:
*    name == user name
*
* RETURNS:
*    pointer to malloced user ent (NOT FILLED IN COMPLETELY)
*********************************************************************/
static user_ent_t *
    new_user_ent (const xmlChar *user)
{

    user_ent_t  *userent;

    userent = m__getObj(user_ent_t);
    if (!userent) {
	return NULL;
    }

    memset(userent, 0x0, sizeof(user_ent_t));
    userent->user = xml_strdup(user);
    if (!userent->user) {
	m__free(userent);
	return NULL;
    }

    return userent;

} /* new_user_ent */


/********************************************************************
* FUNCTION free_user_ent
*
* Free a user entry
*
* INPUTS:
*    userent == user entry to delete
*********************************************************************/
static void
    free_user_ent (user_ent_t *userent)
{

    if (userent->user) {
	m__free(userent->user);
    }
    m__free(userent);

} /* free_user_ent */


/********************************************************************
* FUNCTION find_user_ent
*
* Find a user entry
*
* INPUTS:
*    name == user name to find
*
* RETURNS:
*    pointer to found user ent or NULL if not found
*********************************************************************/
static user_ent_t *
    find_user_ent (const xmlChar *user)
{

    user_ent_t  *userent;

    for (userent = (user_ent_t *)dlq_firstEntry(&userQ);
	 userent != NULL;
	 userent = (user_ent_t *)dlq_nextEntry(userent)) {

	if (!xml_strcmp(user, userent->user)) {
	    return userent;
	}
    }
    return NULL;

} /* find_user_ent */


/********************************************************************
* FUNCTION flush_user_entQ
*
* Delete all the userQ cache entries
*
*********************************************************************/
static void
    flush_user_entQ (void)
{

    user_ent_t  *user;

    while (!dlq_empty(&userQ)) {
	user = (user_ent_t *)dlq_deque(&userQ);
	free_user_ent(user);
    }


} /* find_user_ent */


/********************************************************************
* FUNCTION get_ac_valset
*
* get the cached value set pointer
*
* RETURNS:
*    accessControl pointer
*********************************************************************/
static val_value_t *
    get_ac_valset (void)
{
    if (!accessControl || acStale) {

	accessControl = 
	    cfg_find_datanode(AGT_ACM_INSTANCE_ID,
			      NCX_CFGID_RUNNING);
	if (accessControl) {
	    acStale = FALSE;
	    acModeStale = TRUE;
	    userQStale = TRUE;
	}
    }

    return accessControl;

} /* get_ac_valset */


/********************************************************************
* FUNCTION get_ac_mode
*
* get the access control mode
*
* RETURNS:
*    accessContro.accessControlMode
*********************************************************************/
static agt_acm_control_mode_t
    get_ac_mode (void)
{
    val_value_t            *valset, *val, *chval;

    if (acModeStale) {
	/* get the accessControl parmset */
	valset = get_ac_valset();
	if (!valset) {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return AGT_ACM_CM_NONE;
	}

	/* get the accessControlMode value struct */
	val = val_find_child(valset, AGT_ACM_MODULE, PARM_PROFILE);
	if (val) {
	    chval = val_find_child(val, AGT_ACM_MODULE, LEAF_AC_MODE);
	    if (!chval) {
		SET_ERROR(ERR_INTERNAL_VAL);
		return AGT_ACM_CM_NONE;
	    }
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return AGT_ACM_CM_NONE;
	}	    

	/* check the string value and convert to enum */
	if (!xml_strcmp(VAL_STR(chval),
			(const xmlChar *)"off")) {
	    accessControlMode = AGT_ACM_CM_OFF;
	} else if (!xml_strcmp(VAL_STR(chval),
			       (const xmlChar *)"warn")) {
	    accessControlMode = AGT_ACM_CM_WARN;
	} else if (!xml_strcmp(VAL_STR(chval),
			       (const xmlChar *)"loose")) {
	    accessControlMode = AGT_ACM_CM_LOOSE;
	} else if (!xml_strcmp(VAL_STR(chval),
			       (const xmlChar *)"strict")) {
	    accessControlMode = AGT_ACM_CM_STRICT;
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return AGT_ACM_CM_NONE;
	}
	acModeStale = FALSE;
    }

    return accessControlMode;

} /* get_ac_mode */


/********************************************************************
* FUNCTION get_group_from_user
*
* get the group name associated with this user name
*
* INPUTS:
*    username == username
*    groupname == address of return group name string
*    rolestr == address of return admin role
*
* OUTPUTS:
*   *groupname == group name, if NO_ERR
*   *rolestr == group admin role, if NO_ERR
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_group_from_user (const xmlChar *username,
			 const xmlChar **groupname,
			 const xmlChar **rolestr)
{
    val_value_t   *valset, *groups, *group, *users, *user, *role, *name;
    user_ent_t    *userent;

    /* first check the userQ cache */
    userent = find_user_ent(username);
    if (userent) {
	*groupname = userent->group;
	*rolestr = userent->role;
	return NO_ERR;
    }

    /* not in the queue, so check the groups value struct
     * get tha accessControl parmset
     */
    valset = get_ac_valset();
    if (!valset) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    
    /* get the groups parameter */
    groups = val_find_child(valset, AGT_ACM_MODULE, PARM_GROUPS);
    if (!groups) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* go through the groups in order and check the user list */
    for (group = val_get_first_child(groups);
	 group != NULL;
	 group = val_get_next_child(group)) {

	name = val_find_child(group, AGT_ACM_MODULE, LEAF_NAME);
	if (!name) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	    
	role = val_find_child(group, AGT_ACM_MODULE, LEAF_ROLE);
	if (!role) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}

	users = val_find_child(group, AGT_ACM_MODULE, LEAF_USERS);
	if (users) {
	    for (user = val_get_first_child(users);
		 user != NULL;
		 user = val_get_next_child(user)) {

		if (!xml_strcmp(VAL_STR(user), username)) {

		    *groupname = VAL_STR(name);
		    *rolestr = VAL_STR(role);

		    /* add a new userent struct to the userQ cache */
		    userent = new_user_ent(username);
		    if (userent) {
			userent->group = *groupname;
			userent->role = *rolestr;
			dlq_enque(userent, &userQ);
		    } /* else memory error, skip cache add ! */

		    return NO_ERR;
		}
	    }
	}
    }

    return ERR_NCX_DEF_NOT_FOUND;

} /* get_group_from_user */


/********************************************************************
* FUNCTION group_in_list
*
* Check the groupList value to see if the groupname is present
*
* INPUTS:
*    groupname == group name for this user
*    ruleval == value struct representing the current *Rule
*
* RETURNS:
*    TRUE if the group in the groupList
*    FALSE if the group is not in the groupList, or some error
*********************************************************************/
static boolean
    group_in_list (const xmlChar *groupname,
		   val_value_t *ruleval)
{
    val_value_t *grouplist;

    /* get the groupList field */
    grouplist = val_find_child(ruleval, AGT_ACM_MODULE, 
			       LEAF_GROUPLIST);
    if (!grouplist) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    /* see if this rule applies to the groupname */
    return ncx_string_in_list(groupname, &(VAL_LIST(grouplist)));

} /* group_in_list */


/********************************************************************
* FUNCTION node_in_list
*
* Check the NcxAccessNodeList type value to see if the 
* node namespace and name are in the list
*
* INPUTS:
*    nsid == namespace ID of the node
*    nodename == node name to find
*    listval == value struct representing the current nodeList
*
* RETURNS:
*    TRUE if the node in the list
*    FALSE if the node is not in the list, or some error
*********************************************************************/
static boolean
    node_in_list (xmlns_id_t  nsid,
		  const xmlChar *nodename,
		  val_value_t *listval)
{
    val_value_t *val;
    xmlns_id_t   valnsid;

    /* get the namespace URI */
    val = val_find_child(listval, AGT_ACM_MODULE, LEAF_NSURI);
    if (!val) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    /* compare the NS ID to the node NSID */
    valnsid = xmlns_find_ns_by_name(VAL_STR(val));
    if (valnsid != nsid) {
	return FALSE;  /* no match */
    }

    /* get the optional element names list */
    val = val_find_child(listval, AGT_ACM_MODULE, LEAF_ELNAMES);
    if (!val) {
	return TRUE;  /* matched namespace URI */
    }

    /* check the list */
    return ncx_string_in_list(nodename, &(VAL_LIST(val)));

} /* node_in_list */


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
		    const xmlChar *groupname,
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

    /* return TRUE if this is the root user */
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


/********************************************************************
* FUNCTION set_accessControl
*
* <edit-config> operation handler for the accessControl parmset
*
* When the <accessControl> parmset is created, it needs a read-only
* parm called 'configCapabilities' filled in by the agent
*
* INPUTS:
*    see agt/agt_.h agt_cb_pscb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    set_accessControl (ses_cb_t  *scb,
		       rpc_msg_t *msg,
		       agt_cbtyp_t cbtyp,
		       op_editop_t  editop,
		       val_value_t *newval,
		       val_value_t *curval)
{
    val_value_t           *caps, *val;
    const obj_template_t  *obj, *chobj;
    typ_template_t *typ;
    typ_def_t   *typdef;
    const xmlChar *str;
    status_t     res;

#ifdef AGT_ACM_DEBUG
    log_debug2("\nagt_acm: set accessControl for session %d", scb->sid);
#endif

    res = NO_ERR;

    if (cbtyp != AGT_CB_APPLY) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    acStale = TRUE;

    /* need to add the configCapabilities object instance */
    switch (editop) {
    case OP_EDITOP_NONE:
	res = NO_ERR;
	break;
    case OP_EDITOP_MERGE:
    case OP_EDITOP_REPLACE:
    case OP_EDITOP_CREATE:
    case OP_EDITOP_LOAD:
	/* check if the current valset already has an
	 * entry for the configCapabilities
	 */
	if (curval && val_find_child(curval, AGT_ACM_MODULE, 
				     PARM_CONFIGCAPS)) {
	    res = NO_ERR;
	    break;
	} else if (!newval) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    break;
	}

	/* hardwired code !! expecting only boolean objects
	 * in the configCapabilities struct
	 */
	obj = obj_find_child(newval->obj, AGT_ACM_MODULE, 
			     PARM_CONFIGCAPS);
	if (!obj) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    break;
	}

	/* get the nacm:globalCapabilities object */
	chobj = obj_find_child(obj, AGT_ACM_MODULE,
			       LEAF_GLOBAL_CONFIG);
	if (!chobj) {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	    break;
	} 


	/************ STOPPED CONVERION HERE ************/

	/* start the configCapabilities parm */
	caps = val_new_value()
	if (!caps) {
	    res = ERR_INTERNAL_MEM;
	    break;
	}
	
	/* add globalConfig field */
	str = (configCaps.globalConfig) ? NCX_EL_TRUE : 
	    NCX_EL_FALSE;
	val = val_make_simval(obj_get_typdef(chobj), 
			      caps->val->nsid,
			      LEAF_GLOBAL_CONFIG, str, &res);
	if (val) {
	    val_add_child(val, caps->val);
	} else {
	    break;
	}

	/* add groupConfig field */
	str = (configCaps.groupConfig) ? NCX_EL_TRUE : 
	    NCX_EL_FALSE;
	val = val_make_simval(typdef, caps->val->nsid,
			      LEAF_GROUP_CONFIG, str, &res);
	if (val) {
	    val_add_child(val, caps->val);
	} else {
	    break;
	}

	/* add rpcAccessConfig field */
	str = (configCaps.rpcAccessConfig) ? NCX_EL_TRUE : 
	    NCX_EL_FALSE;
	val = val_make_simval(typdef, caps->val->nsid,
			      LEAF_RPC_CONFIG, str, &res);
	if (val) {
	    val_add_child(val, caps->val);
	} else {
	    break;
	}

	/* add rpcTypeAccessConfig field */
	str = (configCaps.rpcTypeAccessConfig) ? NCX_EL_TRUE : 
	    NCX_EL_FALSE;
	val = val_make_simval(typdef, caps->val->nsid,
			      LEAF_RPCTYPE_CONFIG, str, &res);
	if (val) {
	    val_add_child(val, caps->val);
	} else {
	    break;
	}

	/* add databaseAccessConfig field */
	str = (configCaps.databaseAccessConfig) ? NCX_EL_TRUE : 
	    NCX_EL_FALSE;
	val = val_make_simval(typdef, caps->val->nsid,
			      LEAF_DATA_CONFIG, str, &res);
	if (val) {
	    val_add_child(val, caps->val);
	} else {
	    break;
	}

	/* add notificationAccessConfig field */
	str = (configCaps.notificationAccessConfig) 
	    ? NCX_EL_TRUE : NCX_EL_FALSE;
	val = val_make_simval(typdef, caps->val->nsid,
			      LEAF_NOTIF_CONFIG, str, &res);
	if (val) {
	    val_add_child(val, caps->val);
	} else {
	    break;
	}

	/* add the configCapabilities parm to the accessControl
	 * parmset
	 */
	ps_add_parm(caps, newps, NCX_MERGE_FIRST);
	break;
    case OP_EDITOP_DELETE:
	res = ERR_NCX_NO_ACCESS_MAX;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (res != NO_ERR) {
	agt_record_error(scb, &msg->mhdr.errQ, 
			 NCX_LAYER_CONTENT, res, NULL,
			 NCX_NT_NONE, NULL,
			 (caps) ? NCX_NT_PARM : NCX_NT_NONE, caps);
	if (caps) {
	    ps_free_parm(caps);
	}
    }

    return res;

}  /* set_accessControl */


/********************************************************************
* FUNCTION validate_profile
*
* <edit-config> operation handler for the profile parm
*
* INPUTS:
*    see agt/agt_.h agt_cb_pcb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    validate_profile (ses_cb_t  *scb,
		      rpc_msg_t *msg,
		      agt_cbtyp_t cbtyp,
		      op_editop_t  editop,
		      ps_parm_t *newp,
		      ps_parm_t *curp)
{
    status_t  res;

    /* get rid of compiler warnings about unused parameters */
    (void)editop;
    (void)curp;

    res = NO_ERR;
    if (cbtyp==AGT_CB_VALIDATE) {
	if (!configCaps.globalConfig) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    agt_record_error(scb, &msg->mhdr.errQ,
			     NCX_LAYER_CONTENT,
			     res,
			     NULL,
			     NCX_NT_NONE, NULL,
			     NCX_NT_PARM, newp);
	}
    }
    return res;
			     
} /* validate_profile */


/********************************************************************
* FUNCTION validate_groups
*
* <edit-config> operation handler for the groups parm
*
* INPUTS:
*    see agt/agt_.h agt_cb_pcb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    validate_groups (ses_cb_t  *scb,
		     rpc_msg_t *msg,
		     agt_cbtyp_t cbtyp,
		     op_editop_t  editop,
		     ps_parm_t *newp,
		     ps_parm_t *curp)
{
    status_t  res;

    (void)editop;
    (void)curp;

    res = NO_ERR;
    if (cbtyp==AGT_CB_VALIDATE) {
	if (!configCaps.groupConfig) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    agt_record_error(scb, &msg->mhdr.errQ,
			     NCX_LAYER_CONTENT,
			     res,
			     NULL,
			     NCX_NT_NONE, NULL,
			     NCX_NT_PARM, newp);
	}
    }
    return res;
			     
} /* validate_groups */


/********************************************************************
* FUNCTION validate_rpcRules
*
* <edit-config> operation handler for the rpcRules parm
*
* INPUTS:
*    see agt/agt_.h agt_cb_pcb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    validate_rpcRules (ses_cb_t  *scb,
		       rpc_msg_t *msg,
		       agt_cbtyp_t cbtyp,
		       op_editop_t  editop,
		       ps_parm_t *newp,
		       ps_parm_t *curp)
{
    status_t  res;

    (void)editop;
    (void)curp;

    res = NO_ERR;
    if (cbtyp==AGT_CB_VALIDATE) {
	if (!configCaps.rpcAccessConfig) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    agt_record_error(scb, &msg->mhdr.errQ,
			     NCX_LAYER_CONTENT,
			     res,
			     NULL,
			     NCX_NT_NONE, NULL,
			     NCX_NT_PARM, newp);
	}
    }
    return res;
			     
} /* validate_rpcRules */


/********************************************************************
* FUNCTION validate_dataRules
*
* <edit-config> operation handler for the dataRules parm
*
* INPUTS:
*    see agt/agt_.h agt_cb_pcb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    validate_dataRules (ses_cb_t  *scb,
			rpc_msg_t *msg,
			agt_cbtyp_t cbtyp,
			op_editop_t  editop,
			ps_parm_t *newp,
			ps_parm_t *curp)
{
    status_t  res;

    (void)editop;
    (void)curp;

    res = NO_ERR;
    if (cbtyp==AGT_CB_VALIDATE) {
	if (!configCaps.databaseAccessConfig) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    agt_record_error(scb, &msg->mhdr.errQ,
			     NCX_LAYER_CONTENT,
			     res,
			     NULL,
			     NCX_NT_NONE, NULL,
			     NCX_NT_PARM, newp);
	}
    }
    return res;
			     
} /* validate_dataRules */


/********************************************************************
* FUNCTION validate_notificationRules
*
* <edit-config> operation handler for the notificationRules parm
*
* INPUTS:
*    see agt/agt_.h agt_cb_pcb_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    validate_notificationRules (ses_cb_t  *scb,
				rpc_msg_t *msg,
				agt_cbtyp_t cbtyp,
				op_editop_t  editop,
				ps_parm_t *newp,
				ps_parm_t *curp)
{
    status_t  res;

    (void)editop;
    (void)curp;

    res = NO_ERR;
    if (cbtyp==AGT_CB_VALIDATE) {
	if (!configCaps.notificationAccessConfig) {
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    agt_record_error(scb, &msg->mhdr.errQ,
			     NCX_LAYER_CONTENT,
			     res,
			     NULL,
			     NCX_NT_NONE, NULL,
			     NCX_NT_PARM, newp);
	}
    }
    return res;
			     
} /* validate_notificationRules */
#endif   /*** 0 ***/


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
#if 0
    status_t  res;
#endif

    accessControlMode = AGT_ACM_DEF_MODE;
    accessControl = NULL;
    dlq_createSQue(&userQ);
    accessControl = NULL;
    acStale = TRUE;
    acModeStale = TRUE;
    userQStale = TRUE;

#if 0
#ifdef AGT_ACM_DEBUG
    log_debug2("\nagt: Loading NCX Access Control module");
#endif

    /* load in the access control parameters */
    res = ncxmod_load_module(AGT_ACM_MODULE);
    if (res != NO_ERR) {
	return res;
    }

    /* register callback function for the accessControl parmset */
    res = agt_cb_register_callback(AGT_ACM_CBPATH,
				   FORONE,
				   AGT_CB_APPLY,
				   set_accessControl);
    if (res != NO_ERR) {
	return res;
    }

    /* register callback function for the profile parm */
    res = agt_cb_register_parm_callback(AGT_ACM_MODULE,
					AGT_ACM_PARMSET,
					PARM_PROFILE,
					FORONE,
					AGT_CB_VALIDATE,
					validate_profile);
    if (res != NO_ERR) {
	return res;
    }

    /* register callback function for the groups parm */
    res = agt_cb_register_parm_callback(AGT_ACM_MODULE,
					AGT_ACM_PARMSET,
					PARM_GROUPS,
					FORONE,
					AGT_CB_VALIDATE,
					validate_groups);
    if (res != NO_ERR) {
	return res;
    }

    /* register callback function for the rpcRules parm */
    res = agt_cb_register_parm_callback(AGT_ACM_MODULE,
					AGT_ACM_PARMSET,
					PARM_RPCRULES,
					FORONE,
					AGT_CB_VALIDATE,
					validate_rpcRules);
    if (res != NO_ERR) {
	return res;
    }

    /* register callback function for the dataRules parm */
    res = agt_cb_register_parm_callback(AGT_ACM_MODULE,
					AGT_ACM_PARMSET,
					PARM_DATARULES,
					FORONE,
					AGT_CB_VALIDATE,
					validate_dataRules);
    if (res != NO_ERR) {
	return res;
    }

    /* register callback function for the notificationRules parm */
    res = agt_cb_register_parm_callback(AGT_ACM_MODULE,
					AGT_ACM_PARMSET,
					PARM_NOTIFRULES,
					FORONE,
					AGT_CB_VALIDATE,
					validate_notificationRules);
    if (res != NO_ERR) {
	return res;
    }

#endif

    /* set the agent config capabilities by hand here!!! */
    configCaps.globalConfig = TRUE;
    configCaps.groupConfig = TRUE;
    configCaps.rpcAccessConfig = TRUE;
    configCaps.rpcTypeAccessConfig = TRUE;
    configCaps.databaseAccessConfig = FALSE;
    configCaps.notificationAccessConfig = FALSE;

    agt_acm_init_done = TRUE;
    return NO_ERR;

}  /* agt_acm_init */


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

#if 0
    agt_cb_unregister_parm_callback(AGT_ACM_MODULE,
				    AGT_ACM_PARMSET,
				    PARM_PROFILE);
    agt_cb_unregister_parm_callback(AGT_ACM_MODULE,
				    AGT_ACM_PARMSET,
				    PARM_GROUPS);
    agt_cb_unregister_parm_callback(AGT_ACM_MODULE,
				    AGT_ACM_PARMSET,
				    PARM_RPCRULES);
    agt_cb_unregister_parm_callback(AGT_ACM_MODULE,
				    AGT_ACM_PARMSET,
				    PARM_DATARULES);
    agt_cb_unregister_parm_callback(AGT_ACM_MODULE,
				    AGT_ACM_PARMSET,
				    PARM_NOTIFRULES);
    agt_cb_unregister_ps_callback(AGT_ACM_MODULE,
				  AGT_ACM_PARMSET);

    flush_user_entQ();
#endif

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
    /* agt_acm_control_mode_t acmode; */

#ifdef DEBUG
    if (!user || !rpcobj) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

#if 0
    /* check the access control mode */
    acmode = get_ac_mode();
    switch (acmode) {
    case AGT_ACM_CM_NONE:
    case AGT_ACM_CM_OFF:
	return TRUE;
    default:
	;
    }
#endif

    /* root user is allowed to access anything */
    if (!xml_strcmp(user, NCX_ROOT_USER)) {
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

    /* root user is allowed to access anything */
    if (!xml_strcmp(user, NCX_ROOT_USER)) {
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

    /* root user is allowed to access anything */
    if (!xml_strcmp(user, NCX_ROOT_USER)) {
	return TRUE;
    }

    return TRUE;   /* !!! TEMP !!! */

}   /* agt_acm_val_read_allowed */


/* END file agt_acm.c */
