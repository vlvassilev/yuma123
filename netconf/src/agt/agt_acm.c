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

#define nacm_E_noRuleDefault_permit (const xmlChar *)"permit"
#define nacm_E_noRuleDefault_deny   (const xmlChar *)"deny"

#define nacm_E_allowedRights_read  (const xmlChar *)"read"
#define nacm_E_allowedRights_write (const xmlChar *)"write"
#define nacm_E_allowedRights_exec  (const xmlChar *)"exec"


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* 1 group that the user is a member */
typedef struct group_ptr_t_ {
    dlq_hdr_t         qhdr;
    xmlns_id_t        groupnsid;
    const xmlChar    *groupname;
} group_ptr_t;


/* list of group identities that the user is a member */
typedef struct user_groups_t_ {
    dlq_hdr_t         qhdr;
    const xmlChar    *username;
    dlq_hdr_t         groupQ;   /* Q of group_ptr_t */
} user_groups_t;


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/
static boolean agt_acm_init_done = FALSE;

static ncx_module_t  *nacmmod;


/********************************************************************
* FUNCTION new_group_ptr
*
* create a group pointer
*
* INPUTS:
*   groupnsid == group identity namspace ID
*   groupname == group identity name to use
*
* RETURNS:
*   filled in, malloced struct or NULL if malloc error
*********************************************************************/
static group_ptr_t *
    new_group_ptr (xmlns_id_t groupnsid,
		   const xmlChar *groupname)
{
    group_ptr_t *grptr;

    grptr = m__getObj(group_ptr_t);
    if (!grptr) {
	return NULL;
    }
    memset(grptr, 0x0, sizeof(group_ptr_t));
    grptr ->groupnsid = groupnsid;
    grptr->groupname = groupname;
    return grptr;

}  /* new_group_ptr */


/********************************************************************
* FUNCTION free_group_ptr
*
* free a group pointer
*
* INPUTS:
*   groupnsid == group identity namspace ID
*   groupname == group identity name to use
*
* RETURNS:
*   filled in, malloced struct or NULL if malloc error
*********************************************************************/
static void
    free_group_ptr (group_ptr_t *grptr)
{
    m__free(grptr);

}  /* free_group_ptr */


/********************************************************************
* FUNCTION find_group_ptr
*
* find a group pointer in a user group record
*
* INPUTS:
*   usergroups == user-to-groups struct to check
*   groupnsid == group identity namspace ID to find
*   groupname == group identity name to find
*
* RETURNS:
*   pointer to found record or NULL if not found
*********************************************************************/
static group_ptr_t *
    find_group_ptr (user_groups_t *usergroups,
		    xmlns_id_t  groupnsid,
		    const xmlChar *groupname)
{
    group_ptr_t   *grptr;

    for (grptr = (group_ptr_t *)
	     dlq_firstEntry(&usergroups->groupQ);
	 grptr != NULL;
	 grptr = (group_ptr_t *)dlq_nextEntry(grptr)) {

	if (grptr->groupnsid == groupnsid &&
	    !xml_strcmp(grptr->groupname, groupname)) {
	    return grptr;
	}
    }
    return NULL;

}  /* find_group_ptr */


/********************************************************************
* FUNCTION add_group_ptr
*
* add a group pointer in a user group record
* if it is not already there
*
* INPUTS:
*   usergroups == user-to-groups struct to use
*   groupnsid == group identity namspace ID to add
*   groupname == group identity name to add
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    add_group_ptr (user_groups_t *usergroups,
		    xmlns_id_t  groupnsid,
		    const xmlChar *groupname)
{
    group_ptr_t   *grptr;

    grptr = find_group_ptr(usergroups, groupnsid, groupname);
    if (grptr) {
	return NO_ERR;
    }

    grptr = new_group_ptr(groupnsid, groupname);
    if (!grptr) {
	return ERR_INTERNAL_MEM;
    }

    dlq_enque(grptr, &usergroups->groupQ);
    return NO_ERR;

}  /* add_group_ptr */


/********************************************************************
* FUNCTION new_usergroups
*
* create a user-to-groups struct
*
* INPUTS:
*   username == name of user to use
*
* RETURNS:
*   filled in, malloced struct or NULL if malloc error
*********************************************************************/
static user_groups_t *
    new_usergroups (const xmlChar *username)
{
    user_groups_t *usergroups;


    usergroups = m__getObj(user_groups_t);
    if (!usergroups) {
	return NULL;
    }

    memset(usergroups, 0x0, sizeof(user_groups_t));
    dlq_createSQue(&usergroups->groupQ);
    usergroups->username = username;

    return usergroups;

}  /* new_usergroups */


/********************************************************************
* FUNCTION free_usergroups
*
* free a user-to-groups struct
*
* INPUTS:
*   usergroups == struct to free
*
*********************************************************************/
static void
    free_usergroups (user_groups_t *usergroups)
{
    group_ptr_t *grptr;

    while (!dlq_empty(&usergroups->groupQ)) {
	grptr = (group_ptr_t *)
	    dlq_deque(&usergroups->groupQ);
	free_group_ptr(grptr);
    }
    m__free(usergroups);

}  /* free_usergroups */


/********************************************************************
* FUNCTION get_usergroups_entry
*
* create (TBD: cache) a user-to-groups entry
* for the specified username, based on the /nacm/groups
* contents at this time
*
* INPUTS:
*   nacmroot == root of the nacm tree, already fetched
*   username == user name to create mapping for
*   groupcount == address of return group count field
*
* OUTPUTS:
*   *groupcount == number of groups that the specified
*                  user is part of (i.e., number of 
*                  group_ptr_t structs in the groups Q
*
* RETURNS:
*  malloced usergroups entry for the specified user
*********************************************************************/
static user_groups_t *
    get_usergroups_entry (val_value_t *nacmroot,
			  const xmlChar *username,
			  uint32 *groupcount)
{
    user_groups_t  *usergroups;
    val_value_t    *groupsval, *groupval, *groupid, *userval;
    boolean         done;
    status_t        res;

    *groupcount = 0;
    res = NO_ERR;

    /*** no cache yet -- just create entry each time for now ***/

    usergroups = new_usergroups(username);
    if (!usergroups) {
	return NULL;
    }

    /* get /nacm/groups node */
    groupsval = val_find_child(nacmroot,
			       AGT_ACM_MODULE,
			       nacm_N_groups);
    if (!groupsval) {
	return usergroups;
    }

    /* check each /nacm/groups/group node */
    for (groupval = val_get_first_child(groupsval);
	 groupval != NULL && res == NO_ERR;
	 groupval = val_get_next_child(groupval)) {

	done = FALSE;

	/* check each /nacm/groups/group/userName node */
	for (userval = val_find_child(groupval,
				      AGT_ACM_MODULE,
				      nacm_N_userName);
	     userval != NULL && !done;
	     userval = val_find_next_child(groupval,
					   AGT_ACM_MODULE,
					   nacm_N_userName,
					   userval)) {

	    if (!xml_strcmp(username, VAL_STR(userval))) {
		/* user is a member of this group
		 * get the groupIdentity key leaf
		 */
		groupid = val_find_child(groupval,
					 AGT_ACM_MODULE,
					 nacm_N_groupIdentity);
		done = TRUE;
		if (!groupid) {
		    res = SET_ERROR(ERR_INTERNAL_VAL);
		} else {
		    res = add_group_ptr(usergroups,
					VAL_IDREF_NSID(groupid),
					VAL_IDREF_NAME(groupid));
		    (*groupcount)++;
		}
	    }
	}
    }

    if (res != NO_ERR) {
	log_error("\nError: agt_acm add user2group entry failed");
    }

    return usergroups;
    
}  /* get_usergroups_entry */


/********************************************************************
* FUNCTION check_access_bit
*
*
* INPUTS:
*    ruleval == /nacm/rules/<foo>Rule node, pre-fetched
*    access == string (enum name) for the requested access
*              (read, write, exec)
*    usergroups == user-to-group mapping for access processing
*    done == address of return done processing flag
*
* OUTPUTS:
*    *done == TRUE if a matching group was found, 
*             so return value is the final answer
*          == FALSE if no matching group was found
*
* RETURNS:
*    only valid if *done == TRUE:
*      TRUE if requested access right found 
*      FALSE if requested access right not found
*********************************************************************/
static boolean
    check_access_bit (val_value_t *ruleval,
		      const xmlChar *access,
		      user_groups_t *usergroups,
		      boolean *done)
{
    val_value_t      *group, *rights;
    ncx_list_t       *bits;
    boolean           granted;

    *done = FALSE;
    granted = FALSE;

    for (group = val_find_child(ruleval, 
				AGT_ACM_MODULE, 
				nacm_N_allowedGroup);
	 group != NULL && !*done;
	 group = val_find_next_child(ruleval,
				     AGT_ACM_MODULE,
				     nacm_N_allowedGroup,
				     group)) {
	if (find_group_ptr(usergroups,
			   VAL_IDREF_NSID(group),
			   VAL_IDREF_NAME(group))) {
	    /* this group is a match */
	    *done = TRUE;

	    /* get the allowedRights leaf
	     * and check if the exec bit is set
	     */
	    rights = val_find_child(ruleval,
				    AGT_ACM_MODULE,
				    nacm_N_allowedRights);
	    if (!rights) {
		SET_ERROR(ERR_INTERNAL_VAL);
	    } else {
		bits = &VAL_BITS(rights);
		granted = ncx_string_in_list(access, bits);
	    }
	}
    }
				   
    return granted;

} /* check_accesse_bit */


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
* FUNCTION get_default_rpc_response
*
* get the default response for the specified RPC object
* there are no rules that match any groups with this user
*
*  INPUTS:
*    nacmroot == pre-fectched NACM root
*    rpcobj == RPC template for this request
*    
* RETURNS:
*   TRUE if access granted
*   FALSE if access denied
*********************************************************************/
static boolean
    get_default_rpc_response (val_value_t *nacmroot,
			      const obj_template_t *rpcobj)
{
    val_value_t  *noRule;

    /* check if the RPC method is tagged as 
     * ncx:secure or ncx:very-secure and
     * deny access if so
     */
    if (obj_is_secure(rpcobj) ||
	obj_is_very_secure(rpcobj)) {
	return FALSE;
    }

    /* check the noDefaultRule setting on this agent */
    noRule = val_find_child(nacmroot,
			    AGT_ACM_MODULE,
			    nacm_N_noRuleDefault);
    if (!noRule) {
	return TRUE;  /* default is TRUE */
    }

    if (!xml_strcmp(VAL_ENUM_NAME(noRule),
		    nacm_E_noRuleDefault_permit)) {
	return TRUE;
    } else {
	return FALSE;
    }

}  /* get_default_rpc_response */


/********************************************************************
* FUNCTION check_rpc_rules
*
* Check the configured /nacm/rules/rpcRule list to see if the
* user is allowed to invoke the specified RPC method
*
* INPUTS:
*    rulesval == /nacm/rules node, pre-fetched
*    rpcobj == RPC template requested
*    usergroups == user-to-group mapping for access processing
*    done == address of return done processing flag
*
* OUTPUTS:
*    *done == TRUE if a rule was found, so return value is
*             the final answer
*          == FALSE if no rpcRule was found to match
*
* RETURNS:
*    only valid if *done == TRUE:
*      TRUE if authorization to invoke the RPC op is granted
*      FALSE if authorization to invoke the RPC op is not granted
*********************************************************************/
static boolean
    check_rpc_rules (val_value_t *rulesval,
		     const obj_template_t *rpcobj,
		     user_groups_t *usergroups,
		     boolean *done)
{
    val_value_t      *rpcrule, *modname, *rpcname;
    boolean           granted, done2;

    *done = FALSE;
    granted = FALSE;

    /* check all the rpcRule entries */
    for (rpcrule = val_find_child(rulesval, 
				  AGT_ACM_MODULE, 
				  nacm_N_rpcRule);
	 rpcrule != NULL && !*done;
	 rpcrule = val_find_next_child(rulesval,
				       AGT_ACM_MODULE,
				       nacm_N_rpcRule,
				       rpcrule)) {

	/* get the module name key */
	modname = val_find_child(rpcrule,
				 AGT_ACM_MODULE,
				 nacm_N_rpcModuleName);
	if (!modname) {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    continue;
	}

	/* get the rpc operation name key */
	rpcname = val_find_child(rpcrule,
				 AGT_ACM_MODULE,
				 nacm_N_rpcModuleName);
	if (!rpcname) {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    continue;
	}

	/* check if this is the right module */
	if (xml_strcmp(obj_get_mod_name(rpcobj),
		       VAL_STR(modname))) {
	    continue;
	}

	/* check if this is the right RPC operation */
	if (xml_strcmp(obj_get_name(rpcobj),
		       VAL_STR(rpcname))) {
	    continue;
	}

	/* this rpcRule is for the specified RPC operation
	 * check if any of the groups in the usergroups
	 * list for this user match any of the groups in
	 * the allowedGroup leaf-list
	 */
	done2 = FALSE;
	granted = check_access_bit(rpcrule,
				   nacm_E_allowedRights_exec,
				   usergroups,
				   &done2);
	if (done2) {
	    *done = TRUE;
	} else {
	    granted = FALSE;
	}
    }
				   
    return granted;

} /* check_rpc_rules */


/********************************************************************
* FUNCTION check_module_rules
*
* Check the configured /nacm/rules/moduleRule list to see if the
* user is allowed access the specified module
*
* INPUTS:
*    rulesval == /nacm/rules node, pre-fetched
*    obj == RPC or data template requested
*    access == string (enum name) for the requested access
*              (read, write, exec)
*    usergroups == user-to-group mapping for access processing
*    done == address of return done processing flag
*
* OUTPUTS:
*    *done == TRUE if a rule was found, so return value is
*             the final answer
*          == FALSE if no rpcRule was found to match
*
* RETURNS:
*    only valid if *done == TRUE:
*      TRUE if authorization to invoke the RPC op is granted
*      FALSE if authorization to invoke the RPC op is not granted
*********************************************************************/
static boolean
    check_module_rules (val_value_t *rulesval,
			const obj_template_t *obj,
			const xmlChar *access,
			user_groups_t *usergroups,
			boolean *done)
{
    val_value_t      *modrule, *modname;
    boolean           granted, done2;

    *done = FALSE;
    granted = FALSE;

    /* check all the rpcRule entries */
    for (modrule = val_find_child(rulesval, 
				  AGT_ACM_MODULE, 
				  nacm_N_moduleRule);
	 modrule != NULL && !*done;
	 modrule = val_find_next_child(rulesval,
				       AGT_ACM_MODULE,
				       nacm_N_moduleRule,
				       modrule)) {

	/* get the module name key */
	modname = val_find_child(modrule,
				 AGT_ACM_MODULE,
				 nacm_N_moduleName);
	if (!modname) {
	    SET_ERROR(ERR_INTERNAL_VAL);
	    continue;
	}

	/* check if this is the right module */
	if (xml_strcmp(obj_get_mod_name(obj),
		       VAL_STR(modname))) {
	    continue;
	}

	/* this moduleRule is for the specified node
	 * check if any of the groups in the usergroups
	 * list for this user match any of the groups in
	 * the allowedGroup leaf-list
	 */
	done2 = FALSE;
	granted = check_access_bit(modrule,
				   access,
				   usergroups,
				   &done2);
	if (done2) {
	    *done = TRUE;
	} else {
	    granted = FALSE;
	}
    }
				   
    return granted;

} /* check_module_rules */


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
    val_value_t      *nacmroot, *rulesval;
    user_groups_t    *usergroups;
    uint32            groupcnt;
    boolean           retval, done;

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

    /* get the NACM root to decide any more */
    nacmroot = get_nacm_root();
    if (!nacmroot) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

    groupcnt = 0;

    usergroups = get_usergroups_entry(nacmroot, 
				      user, 
				      &groupcnt);
    if (!usergroups) {
	/* out of memory! deny all access! */
	return FALSE;
    }

    /* usergroups malloced at this point */
    retval = FALSE;

    if (groupcnt == 0) {
	/* just check the default for this RPC operation */
	retval = get_default_rpc_response(nacmroot, rpcobj);
    } else {
	/* get the /nacm/rules node to decide any more */
	rulesval = val_find_child(nacmroot,
				  AGT_ACM_MODULE,
				  nacm_N_rules);
	if (!rulesval) {
	    /* no rules at all so use the default */
	    retval = get_default_rpc_response(nacmroot, 
					      rpcobj);
	} else {
	    /* there is a rules node so check the rpcRules */
	    done = FALSE;
	    retval = check_rpc_rules(rulesval,
				     rpcobj,
				     usergroups, 
				     &done);
	    if (!done) {
		/* no RPC rule found;
		 * try a module namespace rule
		 */
		retval = check_module_rules(rulesval,
					    rpcobj,
					    nacm_E_allowedRights_exec,
					    usergroups, 
					    &done);
		if (!done) {
		    /* no module rule so use the default */
		    retval = get_default_rpc_response(nacmroot, 
						      rpcobj);
		}
	    }
	}
    }

    free_usergroups(usergroups);

    return retval;

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
