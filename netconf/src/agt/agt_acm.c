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
/*  FILE: agt_acm.c

object identifiers:

container /nacm
leaf /nacm/noRuleReadDefault
leaf /nacm/noRuleWriteDefault
leaf /nacm/noRuleExecDefault
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

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xpath1
#include "xpath1.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define AGT_ACM_DEBUG 1

#define AGT_ACM_MODULE      (const xmlChar *)"yuma-nacm"

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
#define nacm_N_noRuleReadDefault (const xmlChar *)"noRuleReadDefault"
#define nacm_N_noRuleWriteDefault (const xmlChar *)"noRuleWriteDefault"
#define nacm_N_noRuleExecDefault (const xmlChar *)"noRuleExecDefault"
#define nacm_N_path (const xmlChar *)"path"
#define nacm_N_rpcModuleName (const xmlChar *)"rpcModuleName"
#define nacm_N_rpcName (const xmlChar *)"rpcName"
#define nacm_N_rpcRule (const xmlChar *)"rpcRule"
#define nacm_N_rules (const xmlChar *)"rules"
#define nacm_N_userName (const xmlChar *)"userName"

#define nacm_OID_nacm (const xmlChar *)"/nacm"

#define nacm_E_noRuleDefault_permit (const xmlChar *)"permit"
#define nacm_E_noRuleDefault_deny   (const xmlChar *)"deny"

#define nacm_E_allowedRights_read  (const xmlChar *)"read"
#define nacm_E_allowedRights_write (const xmlChar *)"write"
#define nacm_E_allowedRights_exec  (const xmlChar *)"exec"


/********************************************************************
*                                                                    *
*                             T Y P E S                                    *
*                                                                    *
*********************************************************************/



/********************************************************************
*                                                                   *
*                       V A R I A B L E S                            *
*                                                                   *
*********************************************************************/
static boolean agt_acm_init_done = FALSE;

static ncx_module_t  *nacmmod;

static const xmlChar *superuser;

static agt_acmode_t   acmode;


/********************************************************************
* FUNCTION is_superuser
*
* Check if the specified user name is the superuser
*
* INPUTS:
*   username == username to check
*
* RETURNS:
*   TRUE if username is the superuser
*   FALSE if username is not the superuser
*********************************************************************/
static boolean
    is_superuser (const xmlChar *username)
{
    if (!superuser || !*superuser) {
        return FALSE;
    }
    if (!username || !*username) {
        return FALSE;
    }
    return (xml_strcmp(superuser, username)) ? FALSE : TRUE;

}  /* is_superuser */


/********************************************************************
* FUNCTION check_mode
*
* Check the access-control mode being used
* 
* INPUTS:
*   cache == cache to use
*   access == requested access mode
*   obj == object template to check
*
* RETURNS:
*   TRUE if access granted
*   FALSE to check the rules and find out
*********************************************************************/
static boolean 
    check_mode (agt_acm_cache_t *cache,
                const xmlChar *access,
                const obj_template_t *obj)
{
    boolean    isread;

    /* check if this is a read or a write */
    if (!xml_strcmp(access, nacm_E_allowedRights_write)) {
        isread = FALSE;
    } else if (!xml_strcmp(access, 
                           nacm_E_allowedRights_exec)) {
        isread = FALSE;
    } else {
        isread = TRUE;
    }

    switch (cache->mode) {
    case AGT_ACMOD_ENFORCING:
        break;
    case AGT_ACMOD_PERMISSIVE:
        if (isread && !obj_is_very_secure(obj)) {
            return TRUE;
        }
        break;
    case AGT_ACMOD_DISABLED:
        if (isread) {
            if (!obj_is_very_secure(obj)) {
                return TRUE;
            }
        } else {
            if (!obj_is_secure(obj)) {
                return TRUE;
            }
        }
        break;
    case AGT_ACMOD_OFF:
        return TRUE;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }
    return FALSE;

}  /* check_mode */


/********************************************************************
* FUNCTION new_modrule
*
* create a moduleRule cache entry
*
* INPUTS:
*   nsid == module namspace ID
*   rule == back-ptr to moduleRule entry
*
* RETURNS:
*   filled in, malloced struct or NULL if malloc error
*********************************************************************/
static agt_acm_modrule_t *
    new_modrule (xmlns_id_t nsid,
                 val_value_t *rule)
{
    agt_acm_modrule_t *modrule;

    modrule = m__getObj(agt_acm_modrule_t);
    if (!modrule) {
        return NULL;
    }
    memset(modrule, 0x0, sizeof(agt_acm_modrule_t));

    modrule->nsid = nsid;
    modrule->modrule = rule;
    return modrule;

}  /* new_modrule */


/********************************************************************
* FUNCTION free_modrule
*
* free a moduleRule cache entry
*
* INPUTS:
*   modrule == entry to free
*
*********************************************************************/
static void
    free_modrule (agt_acm_modrule_t *modrule)
{
    m__free(modrule);

}  /* free_modrule */


/********************************************************************
* FUNCTION new_datarule
*
* create a data rule cache entry
*
* INPUTS:
*   pcb == parser control block to cache
*   result == XPath result to cache
*   rule == back-ptr to dataRule entry
*
* RETURNS:
*   filled in, malloced struct or NULL if malloc error
*********************************************************************/
static agt_acm_datarule_t *
    new_datarule (xpath_pcb_t *pcb,
                  xpath_result_t *result,
                  val_value_t *rule)
{
    agt_acm_datarule_t *datarule;

    datarule = m__getObj(agt_acm_datarule_t);
    if (!datarule) {
        return NULL;
    }
    memset(datarule, 0x0, sizeof(agt_acm_datarule_t));

    datarule->pcb = pcb;
    datarule->result = result;
    datarule->datarule = rule;
    return datarule;

}  /* new_datarule */


/********************************************************************
* FUNCTION free_datarule
*
* free a dataRule cache entry
*
* INPUTS:
*   datarule == entry to free
*
*********************************************************************/
static void
    free_datarule (agt_acm_datarule_t *datarule)
{

    if (datarule->result) {
        xpath_free_result(datarule->result);
    }
    if (datarule->pcb) {
        xpath_free_pcb(datarule->pcb);
    }
    m__free(datarule);

}  /* free_datarule */


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
static agt_acm_group_t *
    new_group_ptr (xmlns_id_t groupnsid,
                   const xmlChar *groupname)
{
    agt_acm_group_t *grptr;

    grptr = m__getObj(agt_acm_group_t);
    if (!grptr) {
        return NULL;
    }
    memset(grptr, 0x0, sizeof(agt_acm_group_t));
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
    free_group_ptr (agt_acm_group_t *grptr)
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
static agt_acm_group_t *
    find_group_ptr (agt_acm_usergroups_t *usergroups,
                    xmlns_id_t  groupnsid,
                    const xmlChar *groupname)
{
    agt_acm_group_t   *grptr;

    for (grptr = (agt_acm_group_t *)
             dlq_firstEntry(&usergroups->groupQ);
         grptr != NULL;
         grptr = (agt_acm_group_t *)dlq_nextEntry(grptr)) {

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
    add_group_ptr (agt_acm_usergroups_t *usergroups,
                   xmlns_id_t  groupnsid,
                   const xmlChar *groupname)
{
    agt_acm_group_t   *grptr;

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
static agt_acm_usergroups_t *
    new_usergroups (const xmlChar *username)
{
    agt_acm_usergroups_t *usergroups;

    usergroups = m__getObj(agt_acm_usergroups_t);
    if (!usergroups) {
        return NULL;
    }

    memset(usergroups, 0x0, sizeof(agt_acm_usergroups_t));
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
    free_usergroups (agt_acm_usergroups_t *usergroups)
{
    agt_acm_group_t *grptr;

    while (!dlq_empty(&usergroups->groupQ)) {
        grptr = (agt_acm_group_t *)
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
*                  agt_acm_group_t structs in the groups Q
*
* RETURNS:
*  malloced usergroups entry for the specified user
*********************************************************************/
static agt_acm_usergroups_t *
    get_usergroups_entry (val_value_t *nacmroot,
                          const xmlChar *username,
                          uint32 *groupcount)
{
    agt_acm_usergroups_t  *usergroups;
    val_value_t           *groupsval, *groupval, *groupid, *userval;
    boolean                done;
    status_t               res;

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
                      agt_acm_usergroups_t *usergroups,
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
*    cache == agt_acm cache to check
*    nacmroot == pre-fectched NACM root
*    rpcobj == RPC template for this request
*    
* RETURNS:
*   TRUE if access granted
*   FALSE if access denied
*********************************************************************/
static boolean
    get_default_rpc_response (agt_acm_cache_t *cache,
                              val_value_t *nacmroot,
                              const obj_template_t *rpcobj)
{
    val_value_t  *noRule;
    boolean       retval;

    /* check if the RPC method is tagged as 
     * ncx:secure or ncx:very-secure and
     * deny access if so
     */
    if (obj_is_secure(rpcobj) ||
        obj_is_very_secure(rpcobj)) {
        return FALSE;
    }

    /* check the noDefaultRule setting on this agent */
    if (cache->flags & FL_ACM_DEFEXEC_SET) {
        return (cache->flags & FL_ACM_DEFEXEC_OK) ? 
            TRUE : FALSE;
    }

    noRule = val_find_child(nacmroot,
                            AGT_ACM_MODULE,
                            nacm_N_noRuleExecDefault);
    if (!noRule) {
        cache->flags |= (FL_ACM_DEFEXEC_SET | FL_ACM_DEFEXEC_OK);
        return TRUE;  /* default is TRUE */
    } 

    if (!xml_strcmp(VAL_ENUM_NAME(noRule),
                    nacm_E_noRuleDefault_permit)) {
        retval = TRUE;
    } else {
        retval = FALSE;
    }
    cache->flags |= FL_ACM_DEFEXEC_SET;
    if (retval) {
        cache->flags |= FL_ACM_DEFEXEC_OK;
    }

    return retval;

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
                     agt_acm_usergroups_t *usergroups,
                     boolean *done)
{
    val_value_t      *rpcrule, *modname, *rpcname;
    boolean           granted, done2;
    status_t          res;

    *done = FALSE;
    granted = FALSE;
    res = NO_ERR;

    /* check all the rpcRule entries */
    for (rpcrule = val_find_child(rulesval, 
                                  AGT_ACM_MODULE, 
                                  nacm_N_rpcRule);
         rpcrule != NULL && res == NO_ERR && !*done;
         rpcrule = val_find_next_child(rulesval,
                                       AGT_ACM_MODULE,
                                       nacm_N_rpcRule,
                                       rpcrule)) {

        /* get the module name key */
        modname = val_find_child(rpcrule,
                                 AGT_ACM_MODULE,
                                 nacm_N_rpcModuleName);
        if (!modname) {
            res = SET_ERROR(ERR_INTERNAL_VAL);
            continue;
        }

        /* get the rpc operation name key */
        rpcname = val_find_child(rpcrule,
                                 AGT_ACM_MODULE,
                                 nacm_N_rpcModuleName);
        if (!rpcname) {
            res = SET_ERROR(ERR_INTERNAL_VAL);
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

    if (res != NO_ERR) {
        granted = FALSE;
        *done = TRUE;
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
*    cache == cache to use
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
    check_module_rules (agt_acm_cache_t *cache,
                        val_value_t *rulesval,
                        const obj_template_t *obj,
                        const xmlChar *access,
                        agt_acm_usergroups_t *usergroups,
                        boolean *done)
{
    val_value_t        *modrule, *modname;
    agt_acm_modrule_t  *modrule_cache;
    boolean             granted;
    xmlns_id_t          nsid;
    status_t            res;

    *done = FALSE;
    granted = FALSE;
    res = NO_ERR;

    if (!(cache->flags & FL_ACM_MODRULES_SET)) {
        cache->flags |= FL_ACM_MODRULES_SET;

        /* check all the moduleRule entries */
        for (modrule = val_find_child(rulesval, 
                                      AGT_ACM_MODULE, 
                                      nacm_N_moduleRule);
             modrule != NULL && res == NO_ERR;
             modrule = val_find_next_child(rulesval,
                                           AGT_ACM_MODULE,
                                           nacm_N_moduleRule,
                                           modrule)) {

            /* get the module name key */
            modname = val_find_child(modrule,
                                     AGT_ACM_MODULE,
                                     nacm_N_moduleName);
            if (!modname) {
                res = SET_ERROR(ERR_INTERNAL_VAL);
                continue;
            }

            nsid = xmlns_find_ns_by_module(VAL_STR(modname));
            if (nsid) {
                modrule_cache = new_modrule(nsid, modrule);
                if (!modrule_cache) {
                    res = ERR_INTERNAL_MEM;
                } else {
                    dlq_enque(modrule_cache, &cache->modruleQ);
                }
            } else {
                /* this rule is for a module that is not
                 * loaded into the system at this time
                 * just skip this entry;
                 */
                ;
            }
        }
    }

    /* get the namespace ID to check against */
    nsid = obj_get_nsid(obj);

    /* go through the cache and exit if any matches are found */
    for (modrule_cache = (agt_acm_modrule_t *)
             dlq_firstEntry(&cache->modruleQ);
         modrule_cache != NULL && res == NO_ERR && !*done;
         modrule_cache = (agt_acm_modrule_t *)
             dlq_nextEntry(modrule_cache)) {

            /* check if this is the right module */
        if (nsid != modrule_cache->nsid) {
            continue;
        }

        /* this moduleRule is for the specified node
         * check if any of the groups in the usergroups
         * list for this user match any of the groups in
         * the allowedGroup leaf-list
         */
        granted = check_access_bit(modrule_cache->modrule,
                                   access,
                                   usergroups,
                                   done);
        if (!*done) {
            granted = FALSE;
        }
    }

    if (res != NO_ERR) {
        granted = FALSE;
        *done = TRUE;
    }

    return granted;

} /* check_module_rules */


/********************************************************************
* FUNCTION get_default_data_response
*
* get the default response for the specified data object
* there are no rules that match any groups with this user
*
*  INPUTS:
*    cache == agt_acm cache to use
*    nacmroot == pre-fectched NACM root
*    obj == data object template for this request
*    iswrite == TRUE for write access
*               FALSE for read access
*
* RETURNS:
*   TRUE if access granted
*   FALSE if access denied
*********************************************************************/
static boolean
    get_default_data_response (agt_acm_cache_t *cache,
                               val_value_t *nacmroot,
                               const val_value_t *val,
                               boolean iswrite)
{
    const obj_template_t  *testobj;
    val_value_t           *noRule;
    boolean                retval;

    /* check if the RPC method is tagged as 
     * ncx:secure or ncx:very-secure and
     * deny access if so
     */
    testobj = val->obj;

    /* make sure this is not an nested object within a
     * object tagged as ncx:secure or ncx:very-secure
     */
    while (testobj) {
        if (iswrite) {
            /* reject any ncx:secure or ncx:very-secure object */
            if (obj_is_secure(testobj) ||
                obj_is_very_secure(testobj)) {
                return FALSE;
            }
        } else {
            /* allow ncx:secure to be read; reject ncx:very-secure */
            if (obj_is_very_secure(testobj)) {
                return FALSE;
            }
        }

        /* stop at root */
        if (obj_is_root(testobj)) {
            testobj = NULL;
        } else {
            testobj = testobj->parent;
        }

        /* no need to check further if the parent was the root
         * need to make sure not to go past the
         * config parameter into the rpc input
         * then the secret <load-config> rpc
         */
        if (testobj && obj_is_root(testobj)) {
            testobj = NULL;
        }
    }

    /* check the noDefaultRule setting on this agent */
    if (iswrite) {
        if (cache->flags & FL_ACM_DEFWRITE_SET) {
            return (cache->flags & FL_ACM_DEFWRITE_OK) ?
                TRUE : FALSE;
        }
        noRule = val_find_child(nacmroot,
                                AGT_ACM_MODULE,
                                nacm_N_noRuleWriteDefault);
        if (!noRule) {
            cache->flags |= FL_ACM_DEFWRITE_SET;
            return FALSE;  /* default is FALSE */
        }

        if (!xml_strcmp(VAL_ENUM_NAME(noRule),
                        nacm_E_noRuleDefault_permit)) {
            retval = TRUE;
        } else {
            retval = FALSE;
        }

        cache->flags |= FL_ACM_DEFWRITE_SET;
        if (retval) {
            cache->flags |= FL_ACM_DEFWRITE_OK;
        }
    } else {
        if (cache->flags & FL_ACM_DEFREAD_SET) {
            return (cache->flags & FL_ACM_DEFREAD_OK) ?
                TRUE : FALSE;
        }
        noRule = val_find_child(nacmroot,
                                AGT_ACM_MODULE,
                                nacm_N_noRuleReadDefault);
        if (!noRule) {
            cache->flags |= (FL_ACM_DEFREAD_SET | FL_ACM_DEFREAD_OK);
            return TRUE;  /* default is TRUE */
        }

        if (!xml_strcmp(VAL_ENUM_NAME(noRule),
                        nacm_E_noRuleDefault_permit)) {
            retval = TRUE;
        } else {
            retval = FALSE;
        }

        cache->flags |= FL_ACM_DEFREAD_SET;
        if (retval) {
            cache->flags |= FL_ACM_DEFREAD_OK;
        }
    }

    return retval;

}  /* get_default_data_response */


/********************************************************************
* FUNCTION check_data_rules
*
* Check the configured /nacm/rules/dataRule list to see if the
* user is allowed access the specified data object
*
* INPUTS:
*    cache == agt_acm cache to use
*    nacmroot == /nacm node prefetched
*    rulesval == /nacm/rules node, pre-fetched
*    val == value node requested
*    access == string (enum name) for the requested access
*              (read, write)
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
*      TRUE if authorization to access data is granted
*      FALSE if authorization to access data is not granted
*********************************************************************/
static boolean
    check_data_rules (agt_acm_cache_t *cache,
                      val_value_t *nacmroot,
                      val_value_t *rulesval,
                      const val_value_t *val,
                      const xmlChar *access,
                      agt_acm_usergroups_t *usergroups,
                      boolean *done)
{
    xpath_pcb_t         *pcb;
    xpath_result_t      *result;
    dlq_hdr_t           *resnodeQ;
    agt_acm_datarule_t  *datarule_cache;
    val_value_t         *datarule, *path, *valroot;
    boolean              granted, nodefound;
    status_t             res;

    *done = FALSE;
    granted = FALSE;
    pcb = NULL;
    res = NO_ERR;

    /* the /nacm node is supposed to be a child of <config> */
    valroot = nacmroot->parent;
    if (!valroot || !obj_is_root(valroot->obj)) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return FALSE;
    }

    /* fill the dataruleQ in the cache if needed */
    if (!(cache->flags & FL_ACM_DATARULES_SET)) {
        cache->flags |= FL_ACM_DATARULES_SET;

        /* check all the dataRule entries */
        for (datarule = val_find_child(rulesval, 
                                       AGT_ACM_MODULE, 
                                       nacm_N_dataRule);
             datarule != NULL && res == NO_ERR;
             datarule = val_find_next_child(rulesval,
                                            AGT_ACM_MODULE,
                                            nacm_N_dataRule,
                                            datarule)) {

            /* get the XPath expression leaf */
            path = val_find_child(datarule,
                                  AGT_ACM_MODULE,
                                  nacm_N_path);
            if (!path || !path->xpathpcb) {
                res = SET_ERROR(ERR_INTERNAL_VAL);
                continue;
            }

            pcb = xpath_clone_pcb(path->xpathpcb);
            if (pcb == NULL) {
                res = ERR_INTERNAL_MEM;
                continue;
            }

            /* make sure the source is not XML so the defunct reader
             * does not get accessed; the clone should save the
             * NSID bindings in all the tokens
             */
            pcb->source = XP_SRC_YANG;
            res = NO_ERR;
            result = xpath1_eval_expr(pcb,
                                      valroot,
                                      valroot,
                                      FALSE,
                                      TRUE,
                                      &res);
            if (!result) {
                xpath_free_pcb(pcb);
                pcb = NULL;
                continue;
            }

            if (res == NO_ERR) {
                datarule_cache = new_datarule(pcb, result, datarule);
                if (!datarule_cache) {
                    res = ERR_INTERNAL_MEM;
                    xpath_free_pcb(pcb);
                    xpath_free_result(result);
                    pcb = NULL;
                    result = NULL;
                    continue;
                } else {
                    /* pass off 'pcb' and 'result' memory here */
                    result = NULL;
                    dlq_enque(datarule_cache, &cache->dataruleQ);
                }
            }
        }
    }

    /* go through the cache and exit if any matches are found */
    for (datarule_cache = (agt_acm_datarule_t *)
             dlq_firstEntry(&cache->dataruleQ);
         datarule_cache != NULL && res == NO_ERR && !*done;
         datarule_cache = (agt_acm_datarule_t *)
             dlq_nextEntry(datarule_cache)) {

        resnodeQ = xpath_get_resnodeQ(datarule_cache->result);
        if (!resnodeQ) {
            res = SET_ERROR(ERR_INTERNAL_VAL);
            continue;
        }

        nodefound = xpath1_check_node_exists_slow(datarule_cache->pcb,
                                                  resnodeQ,
                                                  val);
        if (nodefound) {
            /* this dataRule is for the specified node
             * check if any of the groups in the usergroups
             * list for this user match any of the groups in
             * the allowedGroup leaf-list
             */
            granted = check_access_bit(datarule_cache->datarule,
                                       access,
                                       usergroups,
                                       done);
            if (!*done) {
                granted = FALSE;
            }
        }
    }

    if (res != NO_ERR) {
        granted = FALSE;
        *done = TRUE;
    }

    return granted;

} /* check_data_rules */


/********************************************************************
* FUNCTION valnode_access_allowed
*
* Check if the specified user is allowed to access a value node
* The val->obj template will be checked against the val->editop
* requested access and the user's configured max-access
* 
* INPUTS:
*   msg == incoming message in progress
*   user == user name string
*   val  == val_value_t in progress to check
*   access == access enum string requested (read or write)
*
* RETURNS:
*   TRUE if user allowed this level of access to the value node
*********************************************************************/
static boolean 
    valnode_access_allowed (agt_acm_cache_t *cache,
                            const xmlChar *user,
                            const val_value_t *val,
                            const xmlChar *access)
{
    val_value_t             *nacmroot, *rulesval;
    agt_acm_usergroups_t    *usergroups;
    uint32                   groupcnt;
    boolean                  retval, done, iswrite;

    /* super user is allowed to access anything */
    if (is_superuser(user)) {
        return TRUE;
    }

    /* check if this is a read or a write */
    if (!xml_strcmp(access, nacm_E_allowedRights_write)) {
        iswrite = TRUE;
    } else {
        iswrite = FALSE;
    }

    /* check if access granted without any rules */
    if (check_mode(cache, access, val->obj)) {
        return TRUE;
    }

    if (cache->mode == AGT_ACMOD_DISABLED) {
        return TRUE;
    }

    /* get the NACM root to decide any more */
    if (cache->nacmroot) {
        nacmroot = cache->nacmroot;
    } else {
        nacmroot = get_nacm_root();
        if (!nacmroot) {
            SET_ERROR(ERR_INTERNAL_VAL);
            return FALSE;
        } else {
            cache->nacmroot = nacmroot;
        }
    }

    groupcnt = 0;
    if (cache->usergroups) {
        usergroups = cache->usergroups;
        groupcnt = cache->groupcnt;
    } else {
        usergroups = get_usergroups_entry(nacmroot, 
                                          user, 
                                          &groupcnt);
        if (!usergroups) {
            /* out of memory! deny all access! */
            return FALSE;
        } else {
            cache->usergroups = usergroups;
            cache->groupcnt = groupcnt;
        }
    }

    /* usergroups malloced at this point */
    retval = FALSE;

    if (groupcnt == 0) {
        /* just check the default for this RPC operation */
        retval = get_default_data_response(cache,
                                           nacmroot, 
                                           val, 
                                           iswrite);
    } else {
        /* get the /nacm/rules node to decide any more */
        if (cache->rulesval) {
            rulesval = cache->rulesval;
        } else {
            rulesval = val_find_child(nacmroot,
                                      AGT_ACM_MODULE,
                                      nacm_N_rules);
            if (rulesval) {
                cache->rulesval = rulesval;
            } else {
                /* no rules at all so use the default */
                retval = get_default_data_response(cache,
                                                   nacmroot, 
                                                   val, 
                                                   iswrite);
                return retval;
            }
        }

        /* there is a rules node so check the dataRule list */
        done = FALSE;
        retval = check_data_rules(cache,
                                  nacmroot,
                                  rulesval,
                                  val,
                                  access,
                                  usergroups, 
                                  &done);
        if (!done) {
            /* no data rule found;
             * try a module namespace rule
             */
            retval = check_module_rules(cache,
                                        rulesval,
                                        val->obj,
                                        access,
                                        usergroups, 
                                        &done);
            if (!done) {
                /* no module rule so use the default */
                retval = get_default_data_response(cache,
                                                   nacmroot,
                                                   val,
                                                   iswrite);
            }
        }
    }

    return retval;

}   /* valnode_access_allowed */


/********************************************************************
* FUNCTION nacm_callback
*
* top-level nacm callback function
*
* INPUTS:
*    see agt/agt_cb.h  (agt_cb_pscb_t)
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    nacm_callback (ses_cb_t  *scb,
                   rpc_msg_t  *msg,
                   agt_cbtyp_t cbtyp,
                   op_editop_t  editop,
                   val_value_t  *newval,
                   val_value_t  *curval)
{
    status_t   res;

    (void)scb;
    (void)msg;
    (void)curval;

#ifdef AGT_ACM_DEBUG
    if (LOGDEBUG2) {
        log_debug2("\nServer %s callback: t: %s:%s, op:%s\n", 
                   agt_cbtype_name(cbtyp),
                   val_get_mod_name(newval),
                   newval->name,
                   op_editop_name(editop));
    }
#endif

    res = NO_ERR;

    switch (cbtyp) {
    case AGT_CB_VALIDATE:
        break;
    case AGT_CB_APPLY:
        break;
    case AGT_CB_COMMIT:
        switch (editop) {
        case OP_EDITOP_LOAD:
        case OP_EDITOP_MERGE:
        case OP_EDITOP_REPLACE:
        case OP_EDITOP_CREATE:
        case OP_EDITOP_DELETE:
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case AGT_CB_ROLLBACK:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }
    
    return res;

} /* nacm_callback */


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
    agt_profile_t  *agt_profile;

    if (agt_acm_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

#ifdef AGT_ACM_DEBUG
    log_debug2("\nagt: Loading NCX Access Control module");
#endif

    agt_profile = agt_get_profile();

    nacmmod = NULL;

    /* load in the access control parameters */
    res = ncxmod_load_module(AGT_ACM_MODULE, 
                             NULL, 
                             &agt_profile->agt_savedevQ,
                             &nacmmod);
    if (res != NO_ERR) {
        return res;
    }

    superuser = NULL;
    acmode = AGT_ACMOD_ENFORCING;
    agt_acm_init_done = TRUE;

    res = agt_cb_register_callback(AGT_ACM_MODULE,
                                   nacm_OID_nacm,
                                   NULL,
                                   nacm_callback);
    if (res != NO_ERR) {
        return res;
    }

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
    const agt_profile_t   *profile;
    obj_template_t        *nacmobj;
    cfg_template_t        *runningcfg;
    val_value_t           *nacmval;
    status_t               res;

    if (!agt_acm_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    res = NO_ERR;
    profile = agt_get_profile();
    superuser = profile->agt_superuser;

    if (profile->agt_accesscontrol_enum != AGT_ACMOD_NONE) {
        acmode = profile->agt_accesscontrol_enum;
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
        /* add /nacm/noRule*Default if they
         * are not already set
         */
        /* res = val_add_defaults(nacmval, FALSE); */
        
        /* minimum init done OK, so just exit */
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

    agt_cb_unregister_callbacks(AGT_ACM_MODULE, nacm_OID_nacm);
    nacmmod = NULL;
    agt_acm_init_done = FALSE;

}   /* agt_acm_cleanup */


/********************************************************************
* FUNCTION agt_acm_rpc_allowed
*
* Check if the specified user is allowed to invoke an RPC
* 
* INPUTS:
*   msg == XML header in incoming message in progress
*   user == user name string
*   rpcobj == obj_template_t for the RPC method to check
*
* RETURNS:
*   TRUE if user allowed invoke this RPC; FALSE otherwise
*********************************************************************/
boolean 
    agt_acm_rpc_allowed (xml_msg_hdr_t *msg,
                         const xmlChar *user,
                         const obj_template_t *rpcobj)
{
    val_value_t             *nacmroot, *rulesval;
    agt_acm_usergroups_t    *usergroups;
    agt_acm_cache_t         *cache;
    uint32                   groupcnt;
    boolean                  retval, done;

#ifdef DEBUG
    if (!msg || !user || !rpcobj) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    /* super user is allowed to access anything */
    if (is_superuser(user)) {
        return TRUE;
    }

    /* everybody is allowed to close their own session */
    if (obj_get_nsid(rpcobj) == xmlns_nc_id() &&
        !xml_strcmp(obj_get_name(rpcobj),
                    NCX_EL_CLOSE_SESSION)) {
        return TRUE;
    }

    cache = msg->acm_cache;

    /* check if access granted without any rules */
    if (check_mode(cache, 
                   nacm_E_allowedRights_exec, 
                   rpcobj)) {
        return TRUE;
    }

    if (cache->mode == AGT_ACMOD_DISABLED) {
        return TRUE;
    }

    /* get the NACM root to decide any more */
    if (cache->nacmroot) {
        nacmroot = cache->nacmroot;
    } else {
        nacmroot = get_nacm_root();
        if (!nacmroot) {
            SET_ERROR(ERR_INTERNAL_VAL);
            return FALSE;
        } else {
            cache->nacmroot = nacmroot;
        }
    }

    groupcnt = 0;

    if (cache->usergroups) {
        usergroups = cache->usergroups;
        groupcnt = cache->groupcnt;
    } else {
        usergroups = get_usergroups_entry(nacmroot, 
                                          user, 
                                          &groupcnt);
        if (!usergroups) {
            /* out of memory! deny all access! */
            return FALSE;
        } else {
            cache->usergroups = usergroups;
            cache->groupcnt = groupcnt;
        }
    }

    /* usergroups malloced at this point */
    retval = FALSE;

    if (groupcnt == 0) {
        /* just check the default for this RPC operation */
        retval = get_default_rpc_response(cache,
                                          nacmroot, 
                                          rpcobj);
    } else {
        /* get the /nacm/rules node to decide any more */
        rulesval = val_find_child(nacmroot,
                                  AGT_ACM_MODULE,
                                  nacm_N_rules);
        if (!rulesval) {
            /* no rules at all so use the default */
            retval = get_default_rpc_response(cache,
                                              nacmroot, 
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
                retval = check_module_rules(cache,
                                            rulesval,
                                            rpcobj,
                                            nacm_E_allowedRights_exec,
                                            usergroups, 
                                            &done);
                if (!done) {
                    /* no module rule so use the default */
                    retval = get_default_rpc_response(cache,
                                                      nacmroot, 
                                                      rpcobj);
                }
            }
        }
    }

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
*   msg == XML header from incoming message in progress
*   user == user name string
*   val  == val_value_t in progress to check
*
* RETURNS:
*   TRUE if user allowed this level of access to the value node
*********************************************************************/
boolean 
    agt_acm_val_write_allowed (xml_msg_hdr_t *msg,
                               const xmlChar *user,
                               const val_value_t *val)
{
#ifdef DEBUG
    if (!msg || !msg->acm_cache || !user || !val) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return valnode_access_allowed(msg->acm_cache,
                                  user,
                                  val,
                                  nacm_E_allowedRights_write);

}   /* agt_acm_val_write_allowed */


/********************************************************************
* FUNCTION agt_acm_val_read_allowed
*
* Check if the specified user is allowed to read a value node
* 
* INPUTS:
*   msg == XML header from incoming message in progress
*   user == user name string
*   val  == val_value_t in progress to check
*
* RETURNS:
*   TRUE if user allowed read access to the value node
*********************************************************************/
boolean 
    agt_acm_val_read_allowed (xml_msg_hdr_t *msg,
                              const xmlChar *user,
                              const val_value_t *val)
{
#ifdef DEBUG
    if (!msg || !msg->acm_cache || !user || !val) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return valnode_access_allowed(msg->acm_cache,
                                  user,
                                  val,
                                  nacm_E_allowedRights_read);

}   /* agt_acm_val_read_allowed */


/********************************************************************
* FUNCTION agt_acm_init_msg_cache
*
* Malloc and initialize an agt_acm_cache_t struct
* and attach it to the incoming message
*
* INPUTS:
*   msg == message to use
*
* OUTPUTS:
*   msg->acm_cache pointer set
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_acm_init_msg_cache (xml_msg_hdr_t *msg)
{
    agt_acm_cache_t  *acm_cache;

#ifdef DEBUG
    if (!msg) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (msg->acm_cache) {
        SET_ERROR(ERR_INTERNAL_INIT_SEQ);
        agt_acm_clear_msg_cache(msg);
    }

    acm_cache = m__getObj(agt_acm_cache_t);
    if (!acm_cache) {
        return ERR_INTERNAL_MEM;
    }
    memset(acm_cache, 0x0, sizeof(agt_acm_cache_t));
    dlq_createSQue(&acm_cache->modruleQ);
    dlq_createSQue(&acm_cache->dataruleQ);
    acm_cache->mode = acmode;
    msg->acm_cache = acm_cache;
    msg->acm_cbfn = agt_acm_val_read_allowed;
    return NO_ERR;

} /* agt_acm_init_msg_cache */


/********************************************************************
* FUNCTION agt_acm_clear_msg_cache
*
* Clear an agt_acm_cache_t struct
* attached to the specified message
*
* INPUTS:
*   msg == message to use
*
* OUTPUTS:
*   msg->acm_cache pointer is freed and set to NULL
*
*********************************************************************/
void
    agt_acm_clear_msg_cache (xml_msg_hdr_t *msg)
{
    agt_acm_modrule_t    *modrule;
    agt_acm_datarule_t   *datarule;

#ifdef DEBUG
    if (!msg) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    msg->acm_cbfn = NULL;
    if (!msg->acm_cache) {
        return;
    }

    while (!dlq_empty(&msg->acm_cache->modruleQ)) {
        modrule = (agt_acm_modrule_t *)
            dlq_deque(&msg->acm_cache->modruleQ);
        free_modrule(modrule);
    }

    while (!dlq_empty(&msg->acm_cache->dataruleQ)) {
        datarule = (agt_acm_datarule_t *)
            dlq_deque(&msg->acm_cache->dataruleQ);
        free_datarule(datarule);
    }

    if (msg->acm_cache->usergroups) {
        free_usergroups(msg->acm_cache->usergroups);
    }

    m__free(msg->acm_cache);
    msg->acm_cache = NULL;

} /* agt_acm_clear_msg_cache */


/********************************************************************
* FUNCTION agt_acm_session_is_superuser
*
* Check if the specified session is the superuser
*
* INPUTS:
*   scb == session to check
*
* RETURNS:
*   TRUE if session is for the superuser
*   FALSE if session is not for the superuser
*********************************************************************/
boolean
    agt_acm_session_is_superuser (const ses_cb_t *scb)
{
#ifdef DEBUG
    if (!scb) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return FALSE;
    }
#endif

    return is_superuser(scb->username);

}  /* agt_acm_session_is_superuser */


/* END file agt_acm.c */
