/*
 * Copyright (c) 2009, Andy Bierman
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: agt_expr.c

object identifiers:


                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
05oct08      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/

#if 0

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
static boolean agt_exprd_init_done = FALSE;

static ncx_module_t  *exprmod;


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
* FUNCTION get_expr_root
*
* get the /expr root object
*
* RETURNS:
*   pointer to root or NULL if none
*********************************************************************/
static val_value_t *
    get_expr_root (void)
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

}  /* get_expr_root */



/********************************************************************
* FUNCTION check_expressions
*
* Check the configured 
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
    check_expressions (agt_acm_cache_t *cache,
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

} /* check_expressions */


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
* FUNCTION agt_expr_init
* 
* Initialize the NCX Agent access control module
* 
* INPUTS:
*   none
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    agt_expr_init (void)
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
    return NO_ERR;

}  /* agt_expr_init */


/********************************************************************
* FUNCTION agt_expr_init2
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
    agt_expr_init2 (void)
{
    const agt_profile_t   *profile;
    obj_template_t        *nacmobj;
    cfg_template_t        *runningcfg;
    val_value_t           *nacmval;
    status_t               res;

    if (!agt_acm_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    profile = agt_get_profile();

    superuser = profile->agt_superuser;

    if (profile->agt_accesscontrol_enum != AGT_ACMOD_NONE) {
        acmode = profile->agt_accesscontrol_enum;
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
        /* add /nacm/noRule*Default if they
         * are not already set
         */
        res = val_add_defaults(nacmval, FALSE);
        
        /* minimum init done OK, so just exit */
        return res;
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

}  /* agt_expr_init2 */


/********************************************************************
* FUNCTION agt_expr_cleanup
*
* Cleanup the NCX Agent expression module
* 
*********************************************************************/
void
    agt_expr_cleanup (void)
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

}   /* agt_expr_cleanup */


#endif

/* END file agt_expr.c */
