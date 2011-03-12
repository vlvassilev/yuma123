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
/*  FILE: agt_val.c

   Manage Agent callbacks for typedef-based config manipulation

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
20may06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include <memory.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_cap
#include "agt_cap.h"
#endif

#ifndef _H_agt_cb
#include "agt_cb.h"
#endif

#ifndef _H_agt_ncx
#include "agt_ncx.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val
#include "agt_val.h"
#endif

#ifndef _H_agt_val_parse
#include "agt_val_parse.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_log
#include "log.h"
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

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_plock
#include "plock.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_val_util
#include  "val_util.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xpath
#include  "xpath.h"
#endif

#ifndef _H_xpath_yang
#include  "xpath_yang.h"
#endif

#ifndef _H_xpath1
#include  "xpath1.h"
#endif

#ifndef _H_yangconst
#include  "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define AGT_VAL_DEBUG 1
#endif


/* recursive callback function foward decl */
static status_t
    invoke_btype_cb (agt_cbtyp_t cbtyp,
                     op_editop_t editop,
                     ses_cb_t  *scb,
                     rpc_msg_t  *msg,
                     cfg_template_t *target,
                     val_value_t  *newval,
                     val_value_t  *curval,
                     boolean done);


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION handle_audit_record
* 
* Create and store a change-audit record, if needed
*
* !!! ONLY CALLED FOR RUNNING CONFIG !!!!
*
* INPUTS:
*   editop == edit operation requested
*   scb == session control block
*   msg == RPC request message in progress
*   node == top value node involved in edit
*
* OUTPUTS:
*   log message generated if log level set to LOG_INFO or higher
*********************************************************************/
static void
    handle_audit_record (op_editop_t editop,
                         ses_cb_t  *scb,
                         rpc_msg_t *msg,
                         val_value_t *node)
{
    rpc_audit_rec_t   *auditrec;
    xmlChar           *ibuff, tbuff[TSTAMP_MIN_SIZE+1];

    if (editop == OP_EDITOP_LOAD) {
        return;
    }

    /* generate a log record */
    ibuff = NULL;
    tstamp_datetime(tbuff);

    if (node) {
        (void)val_gen_instance_id(NULL, 
                                  node, 
                                  NCX_IFMT_XPATH1, 
                                  &ibuff);
    }

    if (LOGINFO) {
        log_info("\nedit-config: operation %s on session %d by %s@%s"
                 "\n  at %s on target 'running'"
                 "\n  data: %s",
                 op_editop_name(editop),
                 scb->sid, 
                 (scb->username) ? 
                 scb->username : (const xmlChar *)"nobody",
                 (scb->peeraddr) ? 
                 scb->peeraddr : (const xmlChar *)"localhost",
                 tbuff,
                 (ibuff) ? (const char *)ibuff : "--");
    }

    /* generate a sysConfigChange notification */
    if (ibuff) {
        auditrec = rpc_new_auditrec(ibuff, editop);
        if (auditrec == NULL) {
            log_error("\nError: malloc failed for audit record");
        } else {
            dlq_enque(auditrec, &msg->rpc_auditQ);
        }
        m__free(ibuff);
    }

} /* handle_audit_record */


/********************************************************************
* FUNCTION handle_user_callback
* 
* Find the correct user callback function and invoke it
*
* INPUTS:
*    cbtyp == agent callback type
*    editop == edit operation applied to newnode oand/or curnode
*    scb == session control block invoking the callback
*    msg == RPC message in progress
*    newnode == new node in operation
*    curnode == current node in operation
*
* RETURNS:
*   status of the operation (usually returned from the callback)
*   NO USER CALLBACK FOUND == NO_ERR
*********************************************************************/
static status_t
    handle_user_callback (agt_cbtyp_t cbtyp,
                          op_editop_t editop,
                          ses_cb_t  *scb,
                          rpc_msg_t  *msg,
                          val_value_t *newnode,
                          val_value_t *curnode)
{
    agt_cb_fnset_t    *cbset;
    val_value_t       *val;
    status_t           res;
    boolean            done;

    if (newnode) {
        val = newnode;
    } else if (curnode) {
        val = curnode;
    } else {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (obj_is_root(val->obj)) {
        return NO_ERR;
    }

#ifdef AGT_VAL_DEBUG
    if (LOGDEBUG4) {
        log_debug4("\nChecking for %s user callback for %s edit on %s:%s",
                   agt_cbtype_name(cbtyp),
                   op_editop_name(editop),
                   val_get_mod_name(val),
                   val->name);
    }
#endif

    done = FALSE;
    while (!done) {
        cbset = NULL;
        if (val->obj && val->obj->cbset) {
            cbset = val->obj->cbset;
        }
        if (cbset != NULL && cbset->cbfn[cbtyp] != NULL) {

            if (LOGDEBUG2) {
                log_debug2("\nFound %s user callback for %s:%s",
                           agt_cbtype_name(cbtyp),
                           op_editop_name(editop),
                           val_get_mod_name(val),
                           val->name);
            }

            res = (*cbset->cbfn[cbtyp])(scb, 
                                        msg, 
                                        cbtyp, 
                                        editop, 
                                        newnode, 
                                        curnode);
            if (val->res == NO_ERR) {
                val->res = res;
            }

            if (LOGDEBUG2 && res != NO_ERR) {
                log_debug("\n%s user callback failed (%s) for %s on %s:%s",
                          agt_cbtype_name(cbtyp),
                          get_error_string(res),
                          op_editop_name(editop),
                          val_get_mod_name(val),
                          val->name);
            }
            return res;
        } else if (val->parent != NULL &&
                   !obj_is_root(val->parent->obj) &&
                   val_get_nsid(val) == val_get_nsid(val->parent)) {
            val = val->parent;
        } else {
            done = TRUE;
        }
    }

    return NO_ERR;

} /* handle_user_callback */


/********************************************************************
* FUNCTION add_undo_node
* 
* Add an undo node to the msg->undoQ
*
* INPUTS:
*    msg == RPC message in progress
*    editop == edit-config operation attribute value
*    newnode == node from PDU
*    curnode == node from database (if any)
*    parentnode == parent of curnode (or would be)
*    res == result of edit operation
*    result == address of return status
* OUTPUTS:
*    rpc_undo_rec_t struct added to msg->undoQ
*   *result set to resturn status
*
* RETURNS:
*   pointer to new undo record, in case any extra_deleteQ
*   items need to be added; NULL on error
*********************************************************************/
static rpc_undo_rec_t *
    add_undo_node (rpc_msg_t *msg,
                   op_editop_t editop,
                   val_value_t *newnode,
                   val_value_t *curnode,
                   val_value_t *parentnode,
                   status_t  res,
                   status_t  *result)
{
    rpc_undo_rec_t *undo;

    /* create an undo record for this merge */
    undo = rpc_new_undorec();
    if (!undo) {
        *result = ERR_INTERNAL_MEM;
        return NULL;
    }

    /* save a copy of the current value in case it gets modified
     * in a merge operation
     */
    if (curnode != NULL && msg->rpc_need_undo) {
        undo->curnode_clone = val_clone(curnode);
        if (!undo->curnode_clone) {
            rpc_free_undorec(undo);
            *result = ERR_INTERNAL_MEM;
            return NULL;
        }
    }

    undo->ismeta = FALSE;
    undo->editop = editop;
    undo->newnode = newnode;
    undo->curnode = curnode;
    undo->parentnode = parentnode;
    undo->res = res;

    dlq_enque(undo, &msg->rpc_undoQ);
    *result = NO_ERR;
    return undo;

} /* add_undo_node */


/********************************************************************
* FUNCTION apply_this_node
* 
* Check if the write operation applies to the current node
*
* INPUTS:
*    editop == edit operation value
*    curnode == pointer to current value node 
*                   (just used to check if non-NULL)
*
* RETURNS:
*    TRUE if the current node needs the write operation applied
*    FALSE if this is a NO=OP node (either explicit or special merge)
*********************************************************************/
static boolean
    apply_this_node (op_editop_t editop,
                     const val_value_t *curnode)
{
    boolean retval;

    retval = FALSE;
    switch (editop) {
    case OP_EDITOP_NONE:
        /* never apply here when operation is none */
        break;
    case OP_EDITOP_MERGE:
        /* if no current node then always merge here
         * merge child nodes into an existing complex type
         * except for the index nodes, which are kept
         */
        if (!curnode) {
            retval = TRUE;
        } else {
            /* if this is a leaf and not an index leaf, then
             * apply the merge here
             */
            if (curnode && !curnode->index) {
                retval = typ_is_simple
                    (obj_get_basetype(curnode->obj));
            }
        }
        break;
    case OP_EDITOP_REPLACE:
        if (curnode == NULL || !obj_is_root(curnode->obj)) {
            retval = TRUE;
        }
        break;
    case OP_EDITOP_COMMIT:
    case OP_EDITOP_CREATE:
    case OP_EDITOP_LOAD:
    case OP_EDITOP_DELETE:
        retval = TRUE;
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }
    return retval;

} /* apply_this_node */


/********************************************************************
* FUNCTION add_child_node
* 
* Add a child node
*
* INPUTS:
*   child == child value to add
*   parent == parent value to add child to
*   undo == undo record in progress (may be NULL)
*
* OUTPUTS:
*    child added to parent->v.childQ
*    any other cases removed and either added to undo node or deleted
*
*********************************************************************/
static void
    add_child_node (val_value_t  *child,
                    val_value_t  *parent,
                    rpc_undo_rec_t *undo)
{
    val_value_t  *val;
    dlq_hdr_t     cleanQ;

    if (LOGDEBUG3) {
        log_debug3("\nAdd child '%s' to parent '%s'",
                   child->name, 
                   parent->name);
    }

    dlq_createSQue(&cleanQ);

    val_add_child_clean(child, parent, &cleanQ);
    if (undo) {
        dlq_block_enque(&cleanQ, &undo->extra_deleteQ);
    } else {
        while (!dlq_empty(&cleanQ)) {
            val = (val_value_t *)dlq_deque(&cleanQ);
            val_free_value(val);
        }
    }
}  /* add_child_node */


/********************************************************************
* FUNCTION move_child_node
* 
* Move a child list or leaf-list node
*
* INPUTS:
*   newchild == new child value to add
*   curchild == existing child value to move
*   parent == parent value to move child within
*   undo == undo record in progress (may be NULL)
*
* OUTPUTS:
*    child added to parent->v.childQ
*    any other cases removed and either added to undo node or deleted
*
*********************************************************************/
static void
    move_child_node (val_value_t  *newchild,
                     val_value_t  *curchild,
                     val_value_t  *parent,
                     rpc_undo_rec_t *undo)
{
    val_value_t  *val;
    dlq_hdr_t     cleanQ;

    if (LOGDEBUG3) {
        log_debug3("\nMove child '%s' in parent '%s'",
                   newchild->name, 
                   parent->name);
    }

    dlq_createSQue(&cleanQ);

    val_remove_child(curchild);

    val_add_child_clean(newchild, parent, &cleanQ);        
    if (undo) {
        /*** order is not remembered ! ***/
        dlq_enque(curchild, &undo->extra_deleteQ);
    } else {
        val_free_value(curchild);
    }

    /* should not happen, but checking anyway */
    if (undo) {
        dlq_block_enque(&cleanQ, &undo->extra_deleteQ);
    } else {
        while (!dlq_empty(&cleanQ)) {
            val = (val_value_t *)dlq_deque(&cleanQ);
            val_free_value(val);
        }
    }

}  /* move_child_node */


/********************************************************************
* FUNCTION move_mergedlist_node
* 
* Move a list node that was just merged with
* the contents of the newval; now the curval needs
* to be moved
*
* INPUTS:
*   newchild == new child value with the editvars
*               to use to move curchild
*   curchild == existing child value to move
*   parent == parent value to move child within
*   undo == undo record in progress (may be NULL)
*
* OUTPUTS:
*    child added to parent->v.childQ
*    any other cases removed and either added to undo node or deleted
*
*********************************************************************/
static void
    move_mergedlist_node (val_value_t  *newchild,
                          val_value_t  *curchild,
                          val_value_t  *parent,
                          rpc_undo_rec_t *undo)
{
    val_value_t  *val;
    dlq_hdr_t     cleanQ;

    if (LOGDEBUG3) {
        log_debug3("\nMove list '%s' in parent '%s'",
                   newchild->name, 
                   parent->name);
    }

    dlq_createSQue(&cleanQ);

    val_remove_child(curchild);
    
    val_add_child_clean_editvars(newchild->editvars, 
                                 curchild,
                                 parent, 
                                 &cleanQ);        

    if (undo) {
        /*** TBD: remember order! ***/
    }

    /* should not happen, but checking anyway */
    if (undo) {
        dlq_block_enque(&cleanQ, &undo->extra_deleteQ);
    } else {
        while (!dlq_empty(&cleanQ)) {
            val = (val_value_t *)dlq_deque(&cleanQ);
            val_free_value(val);
        }
    }

}  /* move_mergedlist_node */


/********************************************************************
* FUNCTION check_insert_attr
* 
* Check the YANG insert attribute
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   newval == val_value_t from the PDU
*   
* RETURNS:
*   status
*********************************************************************/
static status_t
    check_insert_attr (ses_cb_t  *scb,
                       rpc_msg_t  *msg,
                       val_value_t  *newval)
{
    val_value_t     *testval, *simval, *insertval;
    status_t         res;

    res = NO_ERR;

    if (newval->editvars->editop == OP_EDITOP_DELETE) {
        /* this error already checked in agt_val_parse */
        return NO_ERR;
    }

    if (newval->obj == NULL) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    switch (newval->obj->objtype) {
    case OBJ_TYP_LEAF_LIST:
        /* make sure the insert attr is on a node with a parent
         * this should always be true since the docroot would
         * be the parent of every accessible object instance
         *
         * OK to check insertstr, otherwise errors
         * should already be recorded by agt_val_parse
         */
        if (newval->editvars->insertop == OP_INSOP_NONE) {
            return NO_ERR;
        }

        if (!newval->editvars->insertstr) {
            /* insert op already checked in agt_val_parse */
            return NO_ERR;
        }

        if (obj_is_system_ordered(newval->obj)) {
            res = ERR_NCX_UNEXPECTED_INSERT_ATTRS;
            agt_record_error(scb,
                             &msg->mhdr,
                             NCX_LAYER_CONTENT,
                             res,
                             NULL,
                             NCX_NT_STRING,
                             newval->editvars->insertstr,
                             NCX_NT_VAL,
                             newval);
            return res;
        }

        if (!newval->editvars->curparent) {
            res = SET_ERROR(ERR_INTERNAL_VAL);
        } else {
            /* validate the insert string against siblings
             * make a value node to compare in the
             * value space instead of the lexicographical space
             */
            simval = val_make_simval_obj(newval->obj,
                                         newval->editvars->insertstr,
                                         &res);
            if (res == NO_ERR && simval) {
                testval = 
                    val_first_child_match(newval->editvars->curparent,
                                          simval);
                if (!testval) {
                    /* sibling leaf-list with the specified
                     * value was not found (13.7)
                     */
                    res = ERR_NCX_INSERT_MISSING_INSTANCE;
                } else {
                    newval->editvars->insertval = testval;
                }
            }
                    
            if (simval) {
                val_free_value(simval);
            }
        }
        break;
    case OBJ_TYP_LIST:
        /* there should be a 'key' attribute
         * OK to check insertxpcb, otherwise errors
         * should already be recorded by agt_val_parse
         */
        if (newval->editvars->insertop == OP_INSOP_NONE) {
            return NO_ERR;
        }

        if (!newval->editvars->insertstr) {
            /* insert op already checked in agt_val_parse */
            return NO_ERR;
        }

        if (!newval->editvars->insertxpcb) {
            /* insert op already checked in agt_val_parse */
            return NO_ERR;
        }

        if (obj_is_system_ordered(newval->obj)) {
            res = ERR_NCX_UNEXPECTED_INSERT_ATTRS;
            agt_record_error(scb,
                             &msg->mhdr,
                             NCX_LAYER_CONTENT,
                             res,
                             NULL,
                             NCX_NT_STRING,
                             newval->editvars->insertstr,
                             NCX_NT_VAL,
                             newval);
            return res;
        }

        if (newval->editvars->insertxpcb->validateres != NO_ERR) {
            res = newval->editvars->insertxpcb->validateres;
        } else {
            res = xpath_yang_validate_xmlkey
                (scb->reader,
                 newval->editvars->insertxpcb,
                 newval->obj,
                 FALSE);                
        }
        if (res == NO_ERR) {
            /* get the list entry that the 'key' attribute
             * referenced. It should be valid objects
             * and well-formed from passing the previous test
             */
            testval = val_make_from_insertxpcb(newval, &res);
            if (res == NO_ERR && testval) {
                val_set_canonical_order(testval);
                insertval = val_first_child_match
                    (newval->editvars->curparent,  testval);
                if (!insertval) {
                    /* sibling list with the specified
                     * key value was not found (13.7)
                     */
                    res = ERR_NCX_INSERT_MISSING_INSTANCE;
                } else {
                    newval->editvars->insertval = insertval;
                }
            }
            if (testval) {
                val_free_value(testval);
                testval = NULL;
            }
        }
        break;
    default:
        if (newval->editvars->insertop != OP_INSOP_NONE) {
            res = ERR_NCX_UNEXPECTED_INSERT_ATTRS;
            agt_record_error(scb,
                             &msg->mhdr,
                             NCX_LAYER_CONTENT,
                             res,
                             NULL,
                             NCX_NT_STRING,
                             newval->editvars->insertstr,
                             NCX_NT_VAL,
                             newval);
            return res;
        }
    }
             
    if (res != NO_ERR) {
        agt_record_insert_error(scb, &msg->mhdr, res, newval);
    }

    return res;

}  /* check_insert_attr */


/********************************************************************
* FUNCTION apply_write_val
* 
* Invoke all the AGT_CB_APPLY callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type in case it is AGT_CB_COMMIT_CHECK
*   editop == edit operation in effect on the current node
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == config database target 
*   parent == parent value of curval and newval
*   newval == new value to apply
*   curval == current instance of value (may be NULL if none)
*   *done  == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*
* OUTPUTS:
*   newval and curval may be moved around to
*       different queues, or get modified
*   *done may be changed from FALSE to TRUE if node is applied here
* RETURNS:
*   status
*********************************************************************/
static status_t
    apply_write_val (agt_cbtyp_t cbtyp,
                     op_editop_t  editop,
                     ses_cb_t  *scb,
                     rpc_msg_t  *msg,
                     cfg_template_t *target,
                     val_value_t  *parent,
                     val_value_t  *newval,
                     val_value_t  *curval,
                     boolean      *done)
{
    const xmlChar   *name;
    rpc_undo_rec_t   *undo;
    status_t          res;
    op_editop_t       cur_editop;
    boolean           applyhere, freenew, add_defs_done;
    int               retval;
    uint32            lockid;

    res = NO_ERR;
    freenew = FALSE;
    undo = NULL;
    name = NULL;
    add_defs_done = FALSE;

    if (newval) {
        cur_editop = newval->editvars->editop;
        name = newval->name;
    } else if (curval) {
        cur_editop = editop;
        name = curval->name;
    } else {
        *done = TRUE;
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

#ifdef AGT_VAL_DEBUG
    if (LOGDEBUG4) {
        if (cbtyp == AGT_CB_COMMIT_CHECK) {
            log_debug4("\napply_write_val: commit-check %s start", 
                       name);
        } else {
            log_debug4("\napply_write_val: %s start", name);
        }
    }
#endif

    /* check if this node needs the edit operation applied */
    if (*done || (newval && obj_is_root(newval->obj))) {
        applyhere = FALSE;
    } else if (editop == OP_EDITOP_COMMIT) {
        applyhere = (newval) ? val_get_dirty_flag(newval) : FALSE;
        *done = applyhere;
    } else if (editop == OP_EDITOP_DELETE) {
        applyhere = TRUE;
        *done = TRUE;
    } else {
        applyhere = apply_this_node(cur_editop, curval);
        *done = applyhere;

        /* try to make sure subtree has changed in a replace */
        if (applyhere && (cur_editop == OP_EDITOP_REPLACE)) {
            if (newval && curval) {
                /* for replace, it is safe to add the default
                 * nodes because any missing node in new
                 * is supposed to be deleted in the current node,
                 * and a default will get added right back again
                 */
                if (cbtyp != AGT_CB_COMMIT_CHECK) {
                    res = val_add_defaults(newval, FALSE);
                    if (res != NO_ERR) {
                        *done = TRUE;
                        return res;
                    }
                    add_defs_done = TRUE;
                    val_set_canonical_order(newval);
                }

                retval = val_compare_ex(newval, curval, TRUE);
                if (retval == 0) {
                    /* apply here but nothing to do,
                     * so skip this entire subtree
                     */
                    if (LOGDEBUG && cbtyp != AGT_CB_COMMIT_CHECK) {
                        log_debug("\napply_write_val: "
                                  "Skipping replace node "
                                  "'%s', no changes",
                                  (name) ? name : (const xmlChar *)"--");
                    }
                    applyhere = FALSE;
                } else {
                    retval = val_compare_for_replace(newval, curval);
                    if (retval == 0) {
                        if (LOGDEBUG2 && cbtyp != AGT_CB_COMMIT_CHECK) {
                            log_debug2("\napply_write_val: "
                                       "Skip replace level '%s'",
                                       (name) ? name : (const xmlChar *)"--");
                        }
                        *done = FALSE;
                        applyhere = FALSE;
                    }
                }
            }
        }
    }

    if (applyhere) {
        /* check corner case applying to the config root */
        if (newval && obj_is_root(newval->obj)) {
            ;
        } else if (!add_defs_done && editop != OP_EDITOP_DELETE) {
            if (cbtyp != AGT_CB_COMMIT_CHECK) {
                res = val_add_defaults(newval, FALSE);
                if (res != NO_ERR) {
                    log_error("\nError: add defaults failed");
                    applyhere = FALSE;
                }
            }
        }
    }

    /* apply the requested edit operation */
    if (applyhere) {

        /* check the user callbacks before altering
         * the database
         */
        if (cbtyp == AGT_CB_COMMIT_CHECK) {
            res = NO_ERR;
            lockid = 0;
            if (curval != NULL) {
                res = val_write_ok(curval,
                                   editop,
                                   SES_MY_SID(scb),
                                   TRUE,
                                   &lockid);
                if (res != NO_ERR) {
                    agt_record_error(scb, 
                                     &msg->mhdr, 
                                     NCX_LAYER_OPERATION, 
                                     res, 
                                     NULL, 
                                     NCX_NT_UINT32_PTR, 
                                     &lockid, 
                                     NCX_NT_VAL, 
                                     curval);
                }
            }
            return res;
        }

        if (LOGDEBUG2) {
            log_debug2("\napply_write_val: %s applyhere", name);
        }


        res = handle_user_callback(AGT_CB_APPLY, 
                                   editop,
                                   scb, 
                                   msg, 
                                   newval, 
                                   curval);
        if (res != NO_ERR) {
            return res;
        }

        undo = add_undo_node(msg, 
                             editop,
                             newval,
                             curval, 
                             parent, 
                             NO_ERR, 
                             &res);
        if (res != NO_ERR) {
            return res;
        }

        if (target->cfg_id == NCX_CFGID_RUNNING) {
            handle_audit_record(cur_editop, 
                                scb, 
                                msg,
                                (curval) ? curval : newval);
        }

        if (editop != OP_EDITOP_LOAD) {
            cfg_set_dirty_flag(target);

            if (editop == OP_EDITOP_DELETE) {
                if (parent) {
                    val_set_dirty_flag(parent);
                }
            } else if (curval) {
                val_set_dirty_flag(curval);
            } else if (parent) {
                val_set_dirty_flag(parent);
            }
        }
    

        /* make sure the node is not a virtual value */
        if (curval && val_is_virtual(curval)) {
            return NO_ERR;
        }

        switch (cur_editop) {
        case OP_EDITOP_MERGE:
            val_remove_child(newval);
            if (curval) {
                if (newval->editvars->insertstr) {
                    move_child_node(newval, 
                                    curval, 
                                    parent, 
                                    (msg->rpc_need_undo) ?
                                    undo : NULL);
                } else {
                    freenew = val_merge(newval, curval);
                }
            } else {
                add_child_node(newval, 
                               parent,
                               (msg->rpc_need_undo) ?
                               undo : NULL);
            }

            if (!freenew) {
                val_set_canonical_order(parent);
            }
            break;
        case OP_EDITOP_REPLACE:
        case OP_EDITOP_COMMIT:
            val_remove_child(newval);
            if (curval) {
                if (newval->editvars->insertstr) {
                    move_child_node(newval, 
                                    curval, 
                                    parent, 
                                    (msg->rpc_need_undo) ?
                                    undo : NULL);
                } else {
                    val_set_canonical_order(newval);
                    val_swap_child(newval, curval);

                    if (target->cfg_id == NCX_CFGID_RUNNING) {
                        val_check_swap_resnode(curval, newval);
                    }

                    /* flag the curnode needs to be deleted */
                    rpc_set_undorec_free_curnode(undo);
                }
            } else {
                add_child_node(newval, 
                               parent, 
                               (msg->rpc_need_undo) ?
                               undo : NULL);
                val_set_canonical_order(parent);
            }
            break;
        case OP_EDITOP_CREATE:
            val_remove_child(newval);
            if (curval != NULL) {
                /* there must be a default leaf */
                freenew = val_merge(newval, curval);
            } else {
                freenew = FALSE;
                add_child_node(newval, 
                               parent, 
                               (msg->rpc_need_undo) ?
                               undo : NULL);
                val_set_canonical_order(parent);
            }
            break;
        case OP_EDITOP_LOAD:
            break;
        case OP_EDITOP_DELETE:
            if (curval) {
                if (val_is_default(curval)) {
                    /* need to mark this leaf as a default
                     * instead of actually deleting it
                     */
                    res = val_delete_default_leaf(curval);
                    /* NEED TO RECORD ERROR !!! */
                } else {
                    /* need to really remove the deleted
                     * node so that any commit validation
                     * against the deleted node works
                     *
                     * But also need to keep it in the
                     * undorec in case a user callback for
                     * COMMIT or ROLLBACK is needed
                     */
                    val_remove_child(curval);

                    if (target->cfg_id == NCX_CFGID_RUNNING) {
                        val_check_delete_resnode(curval);
                    }

                    rpc_set_undorec_free_curnode(undo);
                }
            }
            break;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    if (res == NO_ERR && 
        newval != NULL && 
        curval != NULL &&
        newval->btyp == NCX_BT_LIST && 
        cur_editop == OP_EDITOP_MERGE) {
        
        if (newval->editvars) {
            /* move the list entry after the merge is done */
            move_mergedlist_node(newval, 
                                 curval, 
                                 parent, 
                                 (msg->rpc_need_undo) ?
                                 undo : NULL);
        } else {
            SET_ERROR(ERR_INTERNAL_VAL);
            freenew = TRUE;
        }
    }

    if (res == NO_ERR && 
        newval != NULL &&
        obj_is_root(newval->obj) &&
        editop == OP_EDITOP_LOAD) {
        val_remove_child(newval);
        /* val_set_canonical_order(newval); */
        res = cfg_apply_load_root(target, newval);
        if (res != NO_ERR) {
            freenew = TRUE;
        }
    }

    if (freenew) {
        val_free_value(newval);
    }

    return res;

}  /* apply_write_val */


/********************************************************************
* FUNCTION test_apply_write_val
* 
* Execute the AGT_CB_TEST_APPLY phase
*
* INPUTS:
*   parent == parent value of curval and newval
*   newval == new value to apply
*   curval == current instance of value (may be NULL if none)
*   *done  == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*
* OUTPUTS:
*   newval and curval may be moved around to
*       different queues, or get modified
*   *done may be changed from FALSE to TRUE if node is applied here
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    test_apply_write_val (val_value_t  *parent,
                          val_value_t  *newval,
                          val_value_t  *curval,
                          boolean      *done)
{
    const xmlChar   *name;
    val_value_t     *testval;
    agt_profile_t   *profile;
    status_t         res;
    boolean          applyhere, freetest;
    op_editop_t      cur_editop;
    int              retval;

    res = NO_ERR;
    freetest = FALSE;
    testval = NULL;
    name = NULL;

    if (newval) {
        cur_editop = newval->editvars->editop;
        name = newval->name;
    } else if (curval) {
        cur_editop = OP_EDITOP_DELETE;
        name = curval->name;
    } else {
        *done = TRUE;
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (LOGDEBUG3) {
        log_debug3("\ntest_apply_write_val: %s start", name);
    }

    /* check if this node needs the edit operation applied */
    if (*done) {
        applyhere = FALSE;
    } else if (newval->editvars->editop == OP_EDITOP_COMMIT) {
        applyhere = val_get_dirty_flag(newval);
        *done = applyhere;
    } else if (newval->editvars->editop == OP_EDITOP_DELETE) {
        applyhere = TRUE;
        *done = TRUE;
    } else {
        applyhere = apply_this_node(newval->editvars->editop, curval);
        *done = applyhere;

        /* try to make sure subtree has changed in a replace */
        if (applyhere && (cur_editop == OP_EDITOP_REPLACE)) {
            if (newval && curval) {
                testval = val_clone(newval);
                if (testval == NULL) {
                    res = ERR_INTERNAL_MEM;
                } else {
                    freetest = TRUE;

                    /* for replace, it is safe to add the default
                     * nodes because any missing node in new
                     * is supposed to be deleted in the current node,
                     * and a default will get added right back again
                     */
                    res = val_add_defaults(testval, FALSE);
                    if (res != NO_ERR) {
                        *done = TRUE;
                        val_free_value(testval);
                        return res;
                    }
                    val_set_canonical_order(testval);
                    retval = val_compare_ex(testval, curval, TRUE);
                    if (retval == 0) {
                        /* apply here but nothing to do,
                         * so skip this entire subtree
                         */
                        if (LOGDEBUG) {
                            log_debug("\ntest_apply_write_val: "
                                      "Skipping replace node "
                                      "'%s', no changes",
                                      (name) ? name : (const xmlChar *)"--");
                        }
                        applyhere = FALSE;
                    } else {
                        retval = val_compare_for_replace(testval, curval);
                        if (retval == 0) {
                            if (LOGDEBUG2) {
                                log_debug2("\napply_write_val: "
                                           "Skip replace level '%s'",
                                           (name) ? name : 
                                           (const xmlChar *)"--");
                            }
                            *done = FALSE;
                            applyhere = FALSE;
                        }
                    }
                }
            }
        }
    }

    /* apply the requested edit operation */
    if (applyhere) {

        if (LOGDEBUG3) {
            log_debug3("\ntest_apply_write_val: %s start", newval->name);
        }

        /* make sure the node is not a virtual value */
        if (curval && val_is_virtual(curval)) {
            if (testval) {
                val_free_value(testval);
            }
            return NO_ERR;
        }

        switch (newval->editvars->editop) {
        case OP_EDITOP_MERGE:
            if (testval == NULL) {
                testval = val_clone(newval);
            }
            if (!testval) {
                res = ERR_INTERNAL_MEM;
            } else {
                freetest = TRUE;
                if (curval) {
                    if (newval->editvars->insertstr) {
                        move_child_node(testval, 
                                        curval, 
                                        parent, 
                                        NULL);
                        freetest = FALSE;
                    } else {
                        freetest = val_merge(testval, curval);
                    }
                } else {
                    add_child_node(testval, parent, NULL);
                    freetest = FALSE;
                }
            }

            if (!freetest) {
                ; /* val_set_canonical_order(parent); */
            }
            break;
        case OP_EDITOP_REPLACE:
        case OP_EDITOP_COMMIT:
            if (testval == NULL) {
                testval = val_clone(newval);
            }
            if (!testval) {
                res = ERR_INTERNAL_MEM;
            } else {
                freetest = TRUE;
                if (curval) {
                    if (newval->editvars->insertstr) {
                        move_child_node(testval, 
                                        curval, 
                                        parent, 
                                        NULL);
                        freetest = FALSE;
                    } else {
                        /* val_set_canonical_order(testval); */
                        testval->parent = curval->parent;
                        testval->getcb = curval->getcb;
                        dlq_insertAhead(testval, curval);
                        dlq_remove(curval);
                        val_free_value(curval);
                        freetest = FALSE;
                    }
                } else {
                    add_child_node(testval, parent, NULL);
                    /* val_set_canonical_order(parent); */
                    freetest = FALSE;
                }
            }
            break;
        case OP_EDITOP_CREATE:
            if (testval == NULL) {
                testval = val_clone(newval);
            }
            if (!testval) {
                res = ERR_INTERNAL_MEM;
            } else if (curval != NULL) {
                /* there must be a default leaf */
                freetest = val_merge(testval, curval);
            } else {
                add_child_node(testval, parent, NULL);
                /* val_set_canonical_order(parent); */
                freetest = FALSE;
            }
            break;
        case OP_EDITOP_LOAD:
            res = SET_ERROR(ERR_INTERNAL_VAL);
            break;
        case OP_EDITOP_DELETE:
            if (curval) {
                profile = agt_get_profile();
                switch (profile->agt_defaultStyleEnum) {
                case NCX_WITHDEF_REPORT_ALL:
                case NCX_WITHDEF_TRIM:
                    if (val_is_default(curval)) {
                        curval->flags |= VAL_FL_DEFSET;
                    } else {
                        val_remove_child(curval);
                        val_free_value(curval);
                    }
                    break;
                case NCX_WITHDEF_EXPLICIT:
                    val_remove_child(curval);
                    val_free_value(curval);
                    break;
                default:
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                }
            }
            break;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
    } /* else ignore metadata merge */

    if (res == NO_ERR 
        && newval->btyp == NCX_BT_LIST
        && newval->editvars->insertstr 
        && newval->editvars->editop == OP_EDITOP_MERGE) {

        /* move the list entry after the merge is done
         * only the editvars are used from the newval
         * it is not a bug; do not use testval instead
         */
        move_mergedlist_node(newval, curval, parent, NULL);
    }

    if (freetest && testval) {
        val_free_value(testval);
    }

    return res;

}  /* test_apply_write_val */


/********************************************************************
* FUNCTION invoke_simval_cb
* 
* Invoke all the specified agent simple type callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type being invoked
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newval == val_value_t from the PDU
*   curval == current value (if any) from the target config
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*   
* RETURNS:
*   status
*********************************************************************/
static status_t
    invoke_simval_cb (agt_cbtyp_t cbtyp,
                      op_editop_t editop,
                      ses_cb_t  *scb,
                      rpc_msg_t  *msg,
                      cfg_template_t *target,                      
                      val_value_t  *newval,
                      val_value_t  *curval,
                      boolean  done)
{
    val_value_t     *curparent;
    status_t         res;
    ncx_iqual_t      iqual;
    op_editop_t      cureditop;
    uint32           lockid;

    res = NO_ERR;

    /* check the 'operation' attribute in VALIDATE phase */
    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef AGT_VAL_DEBUG
        if (LOGDEBUG4) {
            log_debug4("\ninvoke_simval:validate: %s:%s start", 
                       obj_get_mod_name(newval->obj),
                       newval->name);
        }
#endif

        /* check and adjust the operation attribute */
        iqual = val_get_iqualval(newval);
        res = agt_check_editop(editop, 
                               &newval->editvars->editop, 
                               newval, 
                               curval, 
                               iqual);

        /* check the operation against the object definition
         * and whether or not the entry currently exists
         */
        if (res == NO_ERR) {
            res = agt_check_max_access(newval->editvars->editop, 
                                       obj_get_max_access(newval->obj), 
                                       (curval != NULL));
        }

        /* make sure the node is not partial locked
         * by another session; there is a corner case where
         * all the PDU nodes are OP_EDITOP_NONE, and
         * so nodes that touch a partial lock will never
         * actually request an operation; treat this
         * as an error anyway, since it is too hard
         * to defer the test until later, and this is
         * a useless corner-case so clients should not do it
         */
        if (res == NO_ERR &&
            curval != NULL &&
            target->cfg_id == NCX_CFGID_RUNNING) {

            res = val_write_ok(curval,
                               newval->editvars->editop, 
                               SES_MY_SID(scb),
                               FALSE,
                               &lockid);
            if (res != NO_ERR) {
                agt_record_error(scb, 
                                 &msg->mhdr, 
                                 NCX_LAYER_OPERATION, 
                                 res, 
                                 NULL, 
                                 NCX_NT_UINT32_PTR, 
                                 &lockid, 
                                 NCX_NT_VAL, 
                                 curval);
            }
        }

        /* check the user callback only if there is some
         * operation in affect already
         */
        if (res == NO_ERR && 
            newval->editvars->editop != OP_EDITOP_NONE) {

            res = handle_user_callback(AGT_CB_VALIDATE, 
                                       newval->editvars->editop,
                                       scb, 
                                       msg, 
                                       newval, 
                                       curval);
        }

        /* check the insert operation, if any */
        if (res == NO_ERR) {
            res = check_insert_attr(scb, msg, newval);
            /* any errors already recorded */
        } else {
            /* record error that happened above */
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_CONTENT, 
                             res, 
                             NULL, 
                             NCX_NT_VAL, 
                             newval, 
                             NCX_NT_VAL, 
                             newval);
        }
        break;
    case AGT_CB_TEST_APPLY:
        if (newval) {
            curparent = newval->editvars->curparent;
        } else if (curval) {
            curparent = curval->parent;
        } else {
            curparent = NULL;
        }
        res = test_apply_write_val(curparent, 
                                   newval, 
                                   curval, 
                                   &done);
        break;
    case AGT_CB_APPLY:
    case AGT_CB_COMMIT_CHECK:
        if (newval) {
            curparent = newval->editvars->curparent;
            cureditop = newval->editvars->editop;
        } else {
            curparent = NULL;
            cureditop = editop;
            if (curval) {
                curparent = curval->parent;
                if (cureditop == OP_EDITOP_NONE) {
                    cureditop = curval->editvars->editop;
                }
            }
        }
        res = apply_write_val(cbtyp,
                              cureditop, 
                              scb, 
                              msg, 
                              target, 
                              curparent, 
                              newval, 
                              curval, 
                              &done);
        break;
    case AGT_CB_COMMIT:
    case AGT_CB_ROLLBACK:
        break;
    default:
        /* nothing to do for commit or rollback at this time */
        ;  
    }

    return res;

}  /* invoke_simval_cb */


/********************************************************************
* FUNCTION invoke_cpxval_cb
* 
* Invoke all the specified agent complex type callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type being invoked
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newval == val_value_t from the PDU
*   curval == current value (if any) from the target config
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    invoke_cpxval_cb (agt_cbtyp_t cbtyp,
                      op_editop_t  editop,
                      ses_cb_t  *scb,
                      rpc_msg_t  *msg,
                      cfg_template_t *target,
                      val_value_t  *newval,
                      val_value_t  *curval,
                      boolean done)
{
    val_value_t      *chval, *curch, *nextch, *curparent;
    status_t          res, retres;
    ncx_iqual_t       iqual;
    op_editop_t       cur_editop;
    boolean           initialdone;
    uint32            lockid;

    retres = NO_ERR;
    initialdone = done;
    cur_editop = OP_EDITOP_NONE;
    curparent = NULL;

    /* check the 'operation' attribute in VALIDATE phase */
    switch (cbtyp) {
    case AGT_CB_VALIDATE:

#ifdef AGT_VAL_DEBUG
        if (LOGDEBUG4) {
            log_debug4("\ninvoke_cpxval:validate: %s:%s start", 
                       obj_get_mod_name(newval->obj),
                       newval->name);
        }
#endif

        /* check and adjust the operation attribute */
        iqual = val_get_iqualval(newval);
        res = agt_check_editop(editop, 
                               &newval->editvars->editop, 
                               newval, 
                               curval, 
                               iqual);
        if (res == NO_ERR) {
            res = agt_check_max_access(newval->editvars->editop, 
                                       obj_get_max_access(newval->obj), 
                                       (curval != NULL));
        }

        if (res == NO_ERR) {
            res = check_insert_attr(scb, msg, newval);
            CHK_EXIT(res, retres);
            res = NO_ERR;   /* any error already recorded */
        }

        if (res != NO_ERR) {
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_CONTENT, 
                             res, 
                             NULL, 
                             NCX_NT_VAL, 
                             newval, 
                             NCX_NT_VAL, 
                             newval);
            CHK_EXIT(res, retres);
        }

        /* make sure the node is not partial locked
         * by another session; there is a corner case where
         * all the PDU nodes are OP_EDITOP_NONE, and
         * so nodes that touch a partial lock will never
         * actually request an operation; treat this
         * as an error anyway, since it is too hard
         * to defer the test until later, and this is
         * a useless corner-case so clients should not do it
         */
        if (res == NO_ERR &&
            curval != NULL &&
            target->cfg_id == NCX_CFGID_RUNNING) {

            lockid = 0;
            if (obj_is_root(curval->obj) &&
                newval->editvars->editop == OP_EDITOP_NONE) {
                /* do not check OP=none on the config root */
                ;
            } else {
                res = val_write_ok(curval,
                                   newval->editvars->editop, 
                                   SES_MY_SID(scb),
                                   FALSE,
                                   &lockid);
            }
            if (res != NO_ERR) {
                agt_record_error(scb, 
                                 &msg->mhdr, 
                                 NCX_LAYER_OPERATION, 
                                 res, 
                                 NULL, 
                                 NCX_NT_UINT32_PTR, 
                                 &lockid, 
                                 NCX_NT_VAL, 
                                 curval);
            }
        }

        /* check the user callback only if there is some
         * operation in affect already
         */
        if (res == NO_ERR && 
            newval->editvars->editop != OP_EDITOP_NONE) {

            res = handle_user_callback(AGT_CB_VALIDATE, 
                                       newval->editvars->editop,
                                       scb, 
                                       msg, 
                                       newval, 
                                       curval);
        }

        if (res != NO_ERR) {
            retres = res;
        }
        break;
    case AGT_CB_TEST_APPLY:
        retres = test_apply_write_val(newval->editvars->curparent, 
                                      newval, 
                                      curval, 
                                      &done);
        break;
    case AGT_CB_APPLY:
    case AGT_CB_COMMIT_CHECK:
        if (newval) {
            cur_editop = newval->editvars->editop;
            curparent = newval->editvars->curparent;
        } else if (curval) {
            cur_editop = editop;
            curparent = curval->parent;
        } else {
            retres = SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (retres == NO_ERR) {
            retres = apply_write_val(cbtyp,
                                     cur_editop, 
                                     scb, 
                                     msg, 
                                     target,
                                     curparent, 
                                     newval, 
                                     curval, 
                                     &done);
        }
        break;
    case AGT_CB_COMMIT:
    case AGT_CB_ROLLBACK:
        break;
    default:
        retres = SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (retres == NO_ERR) {
        if (newval) {
            curparent = newval;
        } else if (curval) {
            curparent = curval;
        } else {
            retres = SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    /* check all the child nodes next */
    if (retres == NO_ERR && !done && curparent) {
        for (chval = val_get_first_child(curparent);
             chval != NULL && retres == NO_ERR;
             chval = nextch) {

            nextch = val_get_next_child(chval);

            chval->editvars->curparent = curval;
            if (curval) {
                curch = val_first_child_match(curval, chval);
            } else {
                curch = NULL;
            }
            cur_editop = chval->editvars->editop;
            if (cur_editop == OP_EDITOP_NONE) {
                cur_editop = editop;
            }

            res = invoke_btype_cb(cbtyp, 
                                  cur_editop, 
                                  scb, 
                                  msg, 
                                  target, 
                                  chval, 
                                  curch, 
                                  FALSE);
            if (chval->res == NO_ERR) {
                chval->res = res;
            }
            CHK_EXIT(res, retres);
        }
    }

    if (retres == NO_ERR) {
        if (newval) {
            cur_editop = newval->editvars->editop;
        } else if (curval) {
            cur_editop = editop;
        } else {
            retres = SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    return retres;

}  /* invoke_cpxval_cb */


/********************************************************************
* FUNCTION invoke_btype_cb
* 
* Invoke all the specified agent typedef callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type being invoked
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*          == NULL if cbtyp == AGT_CB_TEST_APPLY
*   newval == val_value_t from the PDU
*   curval == current value (if any) from the target config
*   done   == TRUE if the node manipulation is done
*          == FALSE if none of the parent nodes have already
*             edited the config nodes; just do user callbacks
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    invoke_btype_cb (agt_cbtyp_t cbtyp,
                     op_editop_t editop,
                     ses_cb_t  *scb,
                     rpc_msg_t  *msg,
                     cfg_template_t *target,
                     val_value_t  *newval,
                     val_value_t  *curval,
                     boolean done)
{
    val_value_t   *v_val;
    status_t       res;
    obj_type_t     objtype;

    res = NO_ERR;
    v_val = NULL;

    if (cbtyp==AGT_CB_VALIDATE) {
        if (curval && val_is_virtual(curval)) {
            v_val = val_get_virtual_value(scb, curval, &res);

            if (res == ERR_NCX_SKIPPED) {
                res = NO_ERR;
            } else if (res != NO_ERR) {
                return res;
            }
        }
    }

    if (newval && newval->obj) {
        objtype = newval->obj->objtype;
    } else if (curval && curval->obj) {
        objtype = curval->obj->objtype;
    } else {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* first traverse all the nodes until leaf nodes are reached */
    switch (objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
        res = invoke_simval_cb(cbtyp, 
                               editop, 
                               scb, 
                               msg,
                               target, 
                               newval, 
                               (v_val) ? v_val : curval, 
                               done);
        break;
    case OBJ_TYP_CONTAINER:
    case OBJ_TYP_LIST:
    case OBJ_TYP_RPC:
    case OBJ_TYP_RPCIO:
    case OBJ_TYP_NOTIF:
        res = invoke_cpxval_cb(cbtyp, 
                               editop, 
                               scb, 
                               msg, 
                               target, 
                               newval, 
                               (v_val) ? v_val : curval,
                               done);
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* invoke_btype_cb */


/********************************************************************
* FUNCTION process_undo_entry   AGT_CB_ROLLBACK
* 
* Attempt to rollback an edit that requested rollback-on-error
*
* INPUTS:
*   undo == rpc_undo_rec_t struct to process
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*
*********************************************************************/
static void
    process_undo_entry (rpc_undo_rec_t *undo,
                        ses_cb_t  *scb,
                        rpc_msg_t  *msg,
                        cfg_template_t *target)
{
    val_value_t  *curval;
    status_t      res;

    (void)target;

    if (undo->curnode_clone != NULL) {
        curval = undo->curnode_clone;
    } else {
        curval = undo->curnode;
    }

    switch (undo->editop) {
    case OP_EDITOP_LOAD:
        /* not supported for internal load operation */
        break;    
    case OP_EDITOP_CREATE:
        /* Since 'create' cannot apply to an attribute,
         * the metadata is not checked
         */
        res = handle_user_callback(AGT_CB_ROLLBACK, 
                                   undo->editop,
                                   scb, 
                                   msg, 
                                   undo->newnode, 
                                   curval);

        /* delete the node from the tree
         * unless it is a default leaf
         */
        if (val_is_default(undo->newnode)) {
            undo->newnode->flags |= VAL_FL_DEFSET;
        } else {
            val_remove_child(undo->newnode);
            val_free_value(undo->newnode);
        }
        undo->newnode = NULL;
        break;
    case OP_EDITOP_DELETE:
        /* Since 'delete' cannot apply to an attribute,
         * the metadata is not checked
         */
        res = handle_user_callback(AGT_CB_ROLLBACK, 
                                   undo->editop,
                                   scb,
                                   msg, 
                                   undo->newnode,
                                   curval);

        /* the node is still in the tree,
         * do not need to do anything to undo a delete
         */
        break;
    case OP_EDITOP_MERGE:
    case OP_EDITOP_REPLACE:
        /* call the user rollback handler, if any */
        res = handle_user_callback(AGT_CB_ROLLBACK, 
                                   undo->editop,
                                   scb,
                                   msg, 
                                   undo->newnode,
                                   curval);

        /* check if the old node needs to be swapped back
         * of if the new node is just removed
         */
        if (undo->newnode) {
            if (undo->curnode) {
                /* try to replace old curval */
                if (undo->curnode_clone) {
                    val_swap_child(undo->curnode_clone, 
                                   undo->curnode);

                    if (target->cfg_id == NCX_CFGID_RUNNING) {
                        val_check_swap_resnode(undo->curnode, 
                                               undo->curnode_clone);
                    }

                    undo->curnode_clone = NULL;
                } else if (LOGWARN) {
                    log_warn("\nError: no rollback selected. "
                             "Cannot undo %s operation",
                             (undo->editop == OP_EDITOP_MERGE) ?
                             "merge" : "replace");
                }
            } else {
                /* remove new node */
                val_remove_child(undo->newnode);
                val_free_value(undo->newnode);
                undo->newnode = NULL;
            }
        } /* else should not happen */
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }

    /***** !!!!!  ****
    handle_undo_audit_record(undo->editop,
                             scb, target,
                               undo->curnode,
                             undo->res);
    ***/

}  /* process_undo_entry */


/********************************************************************
* FUNCTION process_undo_list
* 
* Either complete or rollback an edit that requested rollback-on-error
*
* INPUTS:
*   cbtyp == callback type  (AGT_CB_COMMIT or AGT_CB_ROLLBACK)
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*
* OUTPUTS:
*   if errors are encountered (such as undo-failed) then they
*   will be added to the msg->mhdr.errQ
*
* RETURNS:
*   void
*********************************************************************/
static void
    process_undo_list (agt_cbtyp_t cbtyp,
                       ses_cb_t  *scb,
                       rpc_msg_t  *msg,
                       cfg_template_t *target)

{
    rpc_undo_rec_t  *undo;
    status_t         res;

    res = NO_ERR;

    while (!dlq_empty(&msg->rpc_undoQ)) {
        undo = (rpc_undo_rec_t *)dlq_deque(&msg->rpc_undoQ);
        if (cbtyp==AGT_CB_COMMIT) {
            res = handle_user_callback(AGT_CB_COMMIT, 
                                       undo->editop, 
                                       scb, 
                                       msg, 
                                       undo->newnode, 
                                       undo->curnode);
        } else {
            /* rollback the edit operation */
            process_undo_entry(undo, scb, msg, target);
        }
        rpc_free_undorec(undo);
    }

}  /* process_undo_list */


/********************************************************************
* FUNCTION handle_callback
* 
* Invoke all the specified agent typedef callbacks for a 
* source and target and write operation
*
* INPUTS:
*   cbtyp == callback type being invoked
*   editop == parent node edit operation
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   newval == val_value_t from the PDU
*   curval == current value (if any) from the target config
* RETURNS:
*   status
*********************************************************************/
static status_t
    handle_callback (agt_cbtyp_t cbtyp,
                     op_editop_t editop,
                     ses_cb_t  *scb,
                     rpc_msg_t  *msg,
                     cfg_template_t *target,
                     val_value_t  *newval,
                     val_value_t  *curval)
{
    status_t  res;

    /* check if trying to write to a config=false node */
    if (newval != NULL && !val_is_config_data(newval)) {
        res = ERR_NCX_ACCESS_READ_ONLY;
        agt_record_error(scb, 
                         (msg) ? &msg->mhdr : NULL,
                         NCX_LAYER_OPERATION, 
                         res,
                         NULL, 
                         NCX_NT_NONE, 
                         NULL,
                         NCX_NT_VAL, 
                         newval);
        return res;
    }

    /* check if trying to delete a config=false node */
    if (newval == NULL && 
        curval != NULL &&
        !val_is_config_data(curval)) {
        res = ERR_NCX_ACCESS_READ_ONLY;
        agt_record_error(scb, 
                         (msg) ? &msg->mhdr : NULL,
                         NCX_LAYER_OPERATION, 
                         res,
                         NULL, 
                         NCX_NT_NONE, 
                         NULL,
                         NCX_NT_VAL, 
                         curval);
        return res;
    }

    /* this is a config node so check the operation further */
    switch (cbtyp) {
    case AGT_CB_VALIDATE:
    case AGT_CB_APPLY:
    case AGT_CB_TEST_APPLY:
    case AGT_CB_COMMIT_CHECK:
        /* keep checking until all the child nodes have been processed */
        res = invoke_btype_cb(cbtyp, 
                              editop, 
                              scb, 
                              msg, 
                              target, 
                              newval, 
                              curval, 
                              FALSE);
        break;
    case AGT_CB_COMMIT:
    case AGT_CB_ROLLBACK:
        process_undo_list(cbtyp, scb, msg, target);
        res = NO_ERR;
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* handle_callback */


/********************************************************************
* FUNCTION compare_unique_testsets
* 
* Compare 2 Qs of val_unique_t structs
*
* INPUTS:
*   uni1Q == Q of val_unique_t structs for value1
*   uni2Q == Q of val_unique_t structs for value2
*
* RETURNS:
*   TRUE if compare test is equal
*   FALSE if compare test not equal
*********************************************************************/
static boolean
    compare_unique_testsets (dlq_hdr_t *uni1Q,
                             dlq_hdr_t *uni2Q)
{
    val_unique_t  *uni1, *uni2;
    int32          cmpval;

    /* compare the 2 Qs of values */
    uni1 = (val_unique_t *)dlq_firstEntry(uni1Q);
    uni2 = (val_unique_t *)dlq_firstEntry(uni2Q);

    while (uni1 && uni2) {
        cmpval = val_compare(uni1->valptr, uni2->valptr);
        if (cmpval) {
            return FALSE;
        }
        uni1 = (val_unique_t *)dlq_nextEntry(uni1);
        uni2 = (val_unique_t *)dlq_nextEntry(uni2);
    }
    return TRUE;

} /* compare_unique_testsets */


/********************************************************************
* FUNCTION make_unique_testset
* 
* Construct a Q of val_unique_t records, each with the
* current value of the corresponding obj_unique_comp_t
* If a node is not present the test will be stopped,
* ERR_NCX_CANCELED will be returned in the status parm,
* Any nodes in the resultQ will be left there
* and need to be deleted
*
* INPUTS:
*   curval == value to run test for
*          If test needed, all following-sibling nodes will be 
*          checked to see if they are the same list object instance
*          and if they have a complete tuple to compare, 
*   unidef == obj_unique_t to process
*   resultQ == Queue header to store the val_unique_t records
*   freeQ == Queue of free val_unique_t records to check first
*
* OUTPUTS:
*   resultQ has zero or more val_unique_t structs added
*   which need to be freed with the val_free_unique fn
*
* RETURNS:
*   status of the operation, NO_ERR if all nodes found and stored OK
*********************************************************************/
static status_t 
    make_unique_testset (val_value_t *curval,
                         obj_unique_t *unidef,
                         dlq_hdr_t *resultQ,
                         dlq_hdr_t *freeQ)
{
    obj_unique_comp_t  *unicomp;
    val_unique_t       *unival;
    val_value_t        *targval;
    ncx_module_t       *mod;
    status_t            res, retres;

    retres = NO_ERR;

    unicomp = obj_first_unique_comp(unidef);
    if (!unicomp) {
        /* really should not happen */
        return ERR_NCX_CANCELED;
    }

    /* need to get a non-const pointer to the module */
    if (curval->obj == NULL) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    mod = curval->obj->tkerr.mod;

    /* for each unique component, get the descendant
     * node that is specifies and save it in a val_unique_t
     */
    while (unicomp && retres == NO_ERR) {
        res = xpath_find_val_unique(curval, 
                                    mod,
                                    unicomp->xpath,
                                    FALSE,
                                    &targval);

        if (res != NO_ERR) {
            retres = ERR_NCX_CANCELED;
            continue;
        }

        unival = (val_unique_t *)dlq_deque(freeQ);
        if (!unival) {
            unival = val_new_unique();
            if (!unival) {
                retres = ERR_INTERNAL_MEM;
                continue;
            }
        }

        unival->valptr = targval;
        dlq_enque(unival, resultQ);

        unicomp = obj_next_unique_comp(unicomp);
    }

    return retres;

} /* make_unique_testset */


/********************************************************************
* FUNCTION one_unique_stmt_check
* 
* Run the unique-stmt test for the specified list object type
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   curval == value to run test for
*          If test needed, all following-sibling nodes will be 
*          checked to see if they are the same list object instance
*          and if they have a complete tuple to compare, 
*   unidef == obj_unique_t to process
*   uninum == ordinal ID for this unique-stmt
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    one_unique_stmt_check (ses_cb_t *scb,
                           xml_msg_hdr_t *msg,
                           val_value_t *curval,
                           obj_unique_t *unidef,
                           uint32 uninum)
{
    dlq_hdr_t        uni1Q, uni2Q, freeQ;
    val_unique_t    *unival;
    val_value_t     *testval;
    status_t         res, retres;
    boolean          done, done2, errordone;

    retres = NO_ERR;
    dlq_createSQue(&uni1Q);
    dlq_createSQue(&uni2Q);
    dlq_createSQue(&freeQ);
    done = FALSE;
    errordone = FALSE;

    if (LOGDEBUG3) {
        log_debug3("\nunique_chk: %s:%s start %u", 
                   obj_get_mod_name(curval->obj),
                   curval->name, 
                   uninum);
    }

    /* try to get the start set for this list */
    res = make_unique_testset(curval, unidef, &uni1Q, &freeQ);
    if (res == NO_ERR) {
        ;
    } else if (res == ERR_NCX_CANCELED) {
        done = TRUE;
    } else {
        retres = res;
    }

    if (retres == NO_ERR && !done) {
        /* go through all the following-siblings of this list
         * and find all the list entries with the same object;
         * get the unique tuple set and if complete,
         * compare it to the uni1Q of node values
         */

        done2 = FALSE;
        for (testval = (val_value_t *)dlq_nextEntry(curval);
             testval != NULL && !done2;
             testval = (val_value_t *)dlq_nextEntry(testval)) {

            if (testval->obj == curval->obj) {
                res = make_unique_testset(testval, 
                                          unidef, 
                                          &uni2Q, 
                                          &freeQ);
                if (res == NO_ERR) {
                    if (compare_unique_testsets(&uni1Q, &uni2Q)) {
                        /* 2 lists have the same values
                         * so generate an error; only
                         * do this once for the startval
                         * but find all the duplicates
                         * and mark the flags so the
                         * same error message will not
                         * be duplicated in the <rpc-reply>
                         */
                        if (!errordone) {
                            agt_record_unique_error(scb, 
                                                    msg, 
                                                    curval, 
                                                    &uni1Q);
                            errordone = TRUE;
                        }
                        /* curval->res = ERR_NCX_UNIQUE_TEST_FAILED; */
                        testval->res = ERR_NCX_UNIQUE_TEST_FAILED;
                        retres = ERR_NCX_UNIQUE_TEST_FAILED;
                        testval->flags |= VAL_FL_UNIDONE;
                    }
                    dlq_block_enque(&uni2Q, &freeQ);
                } else if (res == ERR_NCX_CANCELED) {
                    dlq_block_enque(&uni2Q, &freeQ);
                } else {
                    agt_record_error(scb, 
                                     msg, 
                                     NCX_LAYER_CONTENT,
                                     res, 
                                     NULL, 
                                     NCX_NT_NONE,
                                     NULL,
                                     NCX_NT_VAL,
                                     curval);
                    done2 = TRUE;
                    retres = res;
                }
            }
        }
    }

    /* cleanup and exit */
    while (!dlq_empty(&uni1Q)) {
        unival = (val_unique_t *)dlq_deque(&uni1Q);
        val_free_unique(unival);
    }

    while (!dlq_empty(&uni2Q)) {
        unival = (val_unique_t *)dlq_deque(&uni2Q);
        val_free_unique(unival);
    }

    while (!dlq_empty(&freeQ)) {
        unival = (val_unique_t *)dlq_deque(&freeQ);
        val_free_unique(unival);
    }

    return retres;

} /* one_unique_stmt_check */


/********************************************************************
* FUNCTION unique_stmt_check
* 
* Check for the proper uniqueness of the tuples within
* the set of list instances for the specified node
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   curval == value to run test for
*          If test needed, all following-sibling nodes will be 
*          checked to see if they are the same list object instance
*          and if they have a complete tuple to compare, 
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    unique_stmt_check (ses_cb_t *scb,
                       xml_msg_hdr_t *msg,
                       val_value_t *curval)
{
    obj_unique_t         *unidef;
    val_value_t           *clearval, *chval;
    uint32                 uninum;
    status_t               res, retres;

    uninum = 0;
    retres = NO_ERR;
    unidef = obj_first_unique(curval->obj);

    if (unidef && !(curval->flags & VAL_FL_UNIDONE)) {
        while (unidef && retres == NO_ERR) {
            ++uninum;

            if (unidef->isconfig) {
                res = one_unique_stmt_check(scb, 
                                            msg, 
                                            curval,
                                            unidef, 
                                            uninum);
                CHK_EXIT(res, retres);
            }

            unidef = obj_next_unique(unidef);
        }
    }

    /* recurse for every child node until leafs are hit */
    for (chval = val_get_first_child(curval);
         chval != NULL && retres == NO_ERR;
         chval = val_get_next_child(chval)) {

        res = unique_stmt_check(scb, msg, chval);
        CHK_EXIT(res, retres);
    }

    if (uninum && retres != NO_ERR) {
        for (clearval = (val_value_t *)dlq_nextEntry(curval);
             clearval != NULL;
             clearval = (val_value_t *)dlq_nextEntry(clearval)) {
            clearval->flags &= ~VAL_FL_UNIDONE;
        }
    }

    return retres;

}  /* unique_stmt_check */


/********************************************************************
* FUNCTION instance_xpath_check
* 
* Check the leafref or instance-identifier leaf or leaf-list node
* Test 'require-instance' for the NCX_BT_LEAFREF
* and NCX_BT_INSTANCE_ID data types
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   val == value node to check
*   root == config root for 'val'
*   layer == NCX layer calling this function (for error purposes only)
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 

*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    instance_xpath_check (ses_cb_t *scb,
                          xml_msg_hdr_t *msg,
                          val_value_t *val,
                          val_value_t *root,
                          ncx_layer_t layer)
{
    xpath_result_t      *result;
    xpath_pcb_t         *xpcb;
    ncx_errinfo_t       *errinfo;
    typ_def_t           *typdef;
    boolean              constrained, fnresult;
    status_t             res, validateres;

    res = NO_ERR;
    errinfo = NULL;
    typdef = obj_get_typdef(val->obj);

    switch (val->btyp) {
    case NCX_BT_LEAFREF:
        /* do a complete parsing to retrieve the
         * instance that matched, just checking the
         * require-instance flag
         */
        xpcb = typ_get_leafref_pcb(typdef);
        constrained = TRUE;

        if (!val->xpathpcb) {
            val->xpathpcb = xpath_clone_pcb(xpcb);
            if (!val->xpathpcb) {
                res = ERR_INTERNAL_MEM;
            }
        }

        if (res == NO_ERR) {
            result = xpath1_eval_xmlexpr(scb->reader, 
                                         val->xpathpcb, 
                                         val, 
                                         root,
                                         FALSE, 
                                         TRUE, 
                                         &res);

            if (res == NO_ERR) {
                if (constrained) {
                    /* check result: the string value in 'val'
                     * must match one of the values in the
                     * result set
                     */
                    fnresult = 
                        xpath1_compare_result_to_string(val->xpathpcb, 
                                                        result, 
                                                        VAL_STR(val), 
                                                        &res);

                    if (res == NO_ERR && !fnresult) {
                        /* did not match any of the 
                         * current instances  (13.5)
                         */
                        res = ERR_NCX_MISSING_VAL_INST;
                    }
                } else {
                    /* just use the leafref xrefdef, but do not use
                     * the custom error-info for the target leaf
                     */
                    res = val_simval_ok(typ_get_xref_typdef(typdef),
                                        VAL_STR(val));
                }
            }
            if (result) {
                xpath_free_result(result);
            }
        }

        if (res != NO_ERR) {
            val->res = res;
            agt_record_error_errinfo(scb,
                                     msg,
                                     layer,
                                     res, 
                                     NULL,
                                     NCX_NT_OBJ,
                                     val->obj, 
                                     NCX_NT_VAL,
                                     val,
                                     errinfo);
        }
        break;
    case NCX_BT_INSTANCE_ID:
        /* do a complete parsing to retrieve the
         * instance that matched, just checking the
         * require-instance flag
         */
        result = NULL;
        constrained = typ_get_constrained(typdef);

        validateres = val->xpathpcb->validateres;
        if (validateres == NO_ERR) {
            result = 
                xpath1_eval_xmlexpr(scb->reader,
                                    val->xpathpcb,
                                    val,
                                    root,
                                    FALSE,
                                    FALSE,
                                    &res);
            if (result) {
                xpath_free_result(result);
            }

            if (!constrained) {
                if (!NEED_EXIT(res)) {
                    res = NO_ERR;
                }
            }

            if (res != NO_ERR) {
                val->res = res;
                agt_record_error(scb, 
                                 msg,
                                 layer,
                                 res, 
                                 NULL,
                                 NCX_NT_OBJ,
                                 val->obj, 
                                 NCX_NT_VAL,
                                 val);
            }
        }
        break;
    default:
        ;
    }

    return res;
    
}  /* instance_xpath_check */


/********************************************************************
* FUNCTION instance_check
* 
* Check for the proper number of object instances for
* the specified value struct. Checks the direct accessible
* children of 'val' only!!!
* 
* The top-level value set passed cannot represent a choice
* or a case within a choice. 
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   obj == object template for child node in valset to check
*   val == val_value_t list, leaf-list, or container to check
*   valroot == root node of the database
*   layer == NCX layer calling this function (for error purposes only)
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    instance_check (ses_cb_t *scb,
                    xml_msg_hdr_t *msg,
                    obj_template_t *obj,
                    val_value_t *val,
                    val_value_t *valroot,
                    ncx_layer_t layer)
{
    val_value_t         *errval;
    xmlChar             *instbuff;
    const ncx_errinfo_t *errinfo;
    ncx_iqual_t          iqual;
    uint32               cnt, i, minelems, maxelems;
    boolean              minset, maxset, minerr, maxerr, cond;
    status_t             res, res2;
    char                 buff[NCX_MAX_NUMLEN];

    /* skip this node if it is non-config */
    if (!obj_is_config(val->obj)) {
        if (LOGDEBUG3) {
            log_debug3("\ninstance_chk: skipping r/o node '%s:%s'",
                       obj_get_mod_name(val->obj),
                       val->name);
        }
        return NO_ERR;
    }

    /* check if the child node is config
     * this function only tests server requirements
     * and ignores mandatory read-only nodes;
     * otherwise the test on candidate would always fail
     */
    if (!obj_is_config(obj)) {
        if (LOGDEBUG3) {
            log_debug3("\ninstance_chk: skipping r/o node '%s:%s'",
                       obj_get_mod_name(obj),
                       obj_get_name(obj));
        }
        return NO_ERR;
    }

    /* check if the child object should be skipped because
     * of false if-feature or when-stmts
     */
    res = val_check_child_conditional(val, 
                                      valroot,
                                      obj,
                                      &cond);
    if (res != NO_ERR) {
        return res;
    }
    if (!cond) {
        if (LOGDEBUG2) {
            log_debug2("\ninstance_chk: skipping false conditional "
                       "node '%s:%s'",
                       obj_get_mod_name(val->obj),
                       val->name);
        }
        return NO_ERR;
    }
                                       
    res = NO_ERR;
    res2 = NO_ERR;
    errinfo = NULL;
    iqual = val_get_cond_iqualval(val, valroot, obj);
    minerr = FALSE;
    maxerr = FALSE;
    minelems = 0;
    maxelems = 0;
    minset = obj_get_min_elements(obj, &minelems);
    maxset = obj_get_max_elements(obj, &maxelems);

    cnt = val_instance_count(val, 
                             obj_get_mod_name(obj),
                             obj_get_name(obj));

    if (LOGDEBUG3) {
        if (!minset) {
            switch (iqual) {
            case NCX_IQUAL_ONE:
            case NCX_IQUAL_1MORE:
                minelems = 1;
                break;
            case NCX_IQUAL_ZMORE:
            case NCX_IQUAL_OPT:
                minelems = 0;
                break;
            default:
                SET_ERROR(ERR_INTERNAL_VAL);
            }
        }

        if (!maxset) {
            switch (iqual) {
            case NCX_IQUAL_ONE:
            case NCX_IQUAL_OPT:
                maxelems = 1;
                break;
            case NCX_IQUAL_1MORE:
            case NCX_IQUAL_ZMORE:
                maxelems = 0;
                break;
            default:
                SET_ERROR(ERR_INTERNAL_VAL);
            }
        }

        if (maxelems) {
            sprintf(buff, "%u", maxelems);
        }

        log_debug3("\ninstance_check '%s:%s' against '%s:%s'\n"
                   "    (cnt=%u, min=%u, max=%s)",
                   obj_get_mod_name(obj),
                   obj_get_name(obj),
                   obj_get_mod_name(val->obj),
                   val->name, 
                   cnt, 
                   minelems, 
                   maxelems ? buff : "unbounded");
    }

    if (minset) {
        if (cnt < minelems) {
            /* not enough instances error */
            minerr = TRUE;
            res = ERR_NCX_MIN_ELEMS_VIOLATION;
            val->res = res;
            if (cnt) {
                /* use the first child instance as the
                 * value node for the error-path
                 */
                errval = val_find_child(val,
                                        obj_get_mod_name(obj),
                                        obj_get_name(obj));
                agt_record_error(scb, 
                                 msg, 
                                 layer, 
                                 res, 
                                 NULL, 
                                 NCX_NT_NONE, 
                                 NULL, 
                                 NCX_NT_VAL, 
                                 errval);
            } else {
                /* need to construct a string error-path */
                instbuff = NULL;
                res2 = val_gen_split_instance_id(msg, 
                                                 val,
                                                 NCX_IFMT_XPATH1,
                                                 obj_get_nsid(obj),
                                                 obj_get_name(obj),
                                                 &instbuff);
                if (res2 == NO_ERR) {
                    agt_record_error(scb, 
                                     msg, 
                                     layer, 
                                     res, 
                                     NULL, 
                                     NCX_NT_NONE, 
                                     NULL, 
                                     NCX_NT_STRING, 
                                     instbuff);
                } else {
                    agt_record_error(scb, 
                                     msg, 
                                     layer, 
                                     res, 
                                     NULL, 
                                     NCX_NT_OBJ, 
                                     obj, 
                                     NCX_NT_VAL, 
                                     val);
                }
                if (instbuff) {
                    m__free(instbuff);
                }
            }
        }
    }

    if (maxset) {
        if (cnt > maxelems) {
            maxerr = TRUE;
            res = ERR_NCX_MAX_ELEMS_VIOLATION;
            val->res = res;

            /* too many instances error
             * need to find all the extra instances
             * and mark the extras as errors or they will
             * not get removed later
             */
            val_set_extra_instance_errors(val, 
                                          obj_get_mod_name(obj),
                                          obj_get_name(obj),
                                          maxelems);
            /* use the first extra child instance as the
             * value node for the error-path
             */
            errval = val_find_child(val,
                                    obj_get_mod_name(obj),
                                    obj_get_name(obj));
            i = 1;
            while (errval && i <= maxelems) {
                errval = val_get_next_child(errval);
                i++;
            }
            if (errval) {
                agt_record_error(scb, 
                                 msg, 
                                 layer, 
                                 res, 
                                 NULL, 
                                 NCX_NT_OBJ, 
                                 obj, 
                                 NCX_NT_VAL, 
                                 errval);
            } else {
                agt_record_error(scb, 
                                 msg, 
                                 layer, 
                                 res, 
                                 NULL, 
                                 NCX_NT_OBJ, 
                                 obj, 
                                 NCX_NT_VAL, 
                                 val);
            }
        }
    }

    switch (iqual) {
    case NCX_IQUAL_ONE:
    case NCX_IQUAL_1MORE:
        if (cnt < 1 && !minerr) {
            /* missing single parameter (13.5) */
            res = ERR_NCX_MISSING_VAL_INST;
            val->res = res;

            /* need to construct a string error-path */
            instbuff = NULL;
            res2 = val_gen_split_instance_id(msg, 
                                             val,
                                             NCX_IFMT_XPATH1,
                                             obj_get_nsid(obj),
                                             obj_get_name(obj),
                                             &instbuff);
            if (res2 == NO_ERR) {
                agt_record_error(scb,
                                 msg,
                                 layer,
                                 res, 
                                 NULL,
                                 NCX_NT_OBJ,
                                 obj, 
                                 NCX_NT_STRING,
                                 instbuff);
            } else {
                agt_record_error(scb,
                                 msg,
                                 layer,
                                 res, 
                                 NULL,
                                 NCX_NT_OBJ,
                                 obj, 
                                 NCX_NT_VAL,
                                 val);
            }
            if (instbuff) {
                m__free(instbuff);
            }
        }
        if (iqual == NCX_IQUAL_1MORE) {
            break;
        }
        /* else fall through */
    case NCX_IQUAL_OPT:
        if (cnt > 1 && !maxerr) {
            /* too many parameters */
            val_set_extra_instance_errors(val, 
                                          obj_get_mod_name(obj),
                                          obj_get_name(obj), 1);
            res = ERR_NCX_EXTRA_VAL_INST;
            val->res = res;

            /* use the first extra child instance as the
             * value node for the error-path
             */
            errval = val_find_child(val,
                                    obj_get_mod_name(obj),
                                    obj_get_name(obj));
            i = 1;
            while (errval && i < maxelems) {
                errval = val_get_next_child(errval);
                i++;
            }
            if (errval) {
                agt_record_error(scb,
                                 msg,
                                 layer,
                                 res, 
                                 NULL,
                                 NCX_NT_OBJ,
                                 obj, 
                                 NCX_NT_VAL,
                                 errval);
            } else {
                agt_record_error(scb,
                                 msg,
                                 layer, 
                                 res, 
                                 NULL,
                                 NCX_NT_OBJ, 
                                 obj, 
                                 NCX_NT_VAL, 
                                 val);
            }
        }
        break;
    case NCX_IQUAL_ZMORE:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
        val->res = res;
        agt_record_error(scb, 
                         msg,
                         layer,
                         res, 
                         NULL,
                         NCX_NT_OBJ,
                         obj, 
                         NCX_NT_VAL,
                         val);

    }

    return res;
    
}  /* instance_check */


/********************************************************************
* FUNCTION choice_check_agt
* 
* Agent version of ncx/val_util.c/choice_check
*
* Check a val_value_t struct against its expected OBJ
* for instance validation:
*
*    - choice validation: 
*      only one case allowed if the data type is choice
*      Only issue errors based on the instance test qualifiers
*
* The input is checked against the specified obj_template_t.
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   choicobj == object template for the choice to check
*   val == parent val_value_t list or container to check
*   valroot == root of database to check
*   layer == NCX layer calling this function (for error purposes only)
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    choice_check_agt (ses_cb_t  *scb,
                      xml_msg_hdr_t *msg,
                      obj_template_t *choicobj,
                      val_value_t *val,
                      val_value_t *valroot,
                      ncx_layer_t   layer)
{
    obj_template_t        *testobj;
    val_value_t           *chval, *testval;
    status_t               res, retres;

    res = NO_ERR;
    retres = NO_ERR;

    if (LOGDEBUG3) {
        log_debug3("\nichoice_check_agt: check "
                   "'%s:%s' against '%s:%s'",
                   obj_get_mod_name(choicobj),
                   obj_get_name(choicobj), 
                   obj_get_mod_name(val->obj),
                   val->name);
    }

    /* Go through all the child nodes for this object
     * and look for choices against the value set to see if each 
     * a choice case is present in the correct number of instances.
     *
     * The current value could never be a OBJ_TYP_CHOICE since
     * those nodes are not stored in the val_value_t tree
     * Instead, it is the parent of the choice object,
     * and the accessible case nodes will be child nodes
     * of that complex parent type
     */
    chval = val_get_choice_first_set(val, choicobj);
    if (!chval) {
        if (obj_is_mandatory(choicobj)) {
            /* error missing choice (13.6) */
            res = ERR_NCX_MISSING_CHOICE;
            if (msg) {
                agt_record_error(scb,
                                 msg,
                                 layer,
                                 res, 
                                 NULL,
                                 NCX_NT_VAL,
                                 val, 
                                 NCX_NT_VAL,
                                 val);
            }
        }
        return res;
    }

    /* else a choice was selected
     * first make sure all the mandatory case 
     * objects are present
     */
    for (testobj = obj_first_child(chval->casobj);
         testobj != NULL;
         testobj = obj_next_child(testobj)) {

        res = instance_check(scb, msg, testobj, val, valroot, layer);
        CHK_EXIT(res, retres);
        /* errors already recorded if other than NO_ERR */
    }

    /* check if any objects from other cases are present */
    testval = val_get_choice_next_set(val, choicobj, chval);
    while (testval) {
        if (testval->casobj != chval->casobj) {
            /* error: extra case object in this choice */
            retres = res = ERR_NCX_EXTRA_CHOICE;
            if (msg) {
                agt_record_error(scb,
                                 msg, 
                                 layer, 
                                 res, 
                                 NULL, 
                                 NCX_NT_OBJ, 
                                 choicobj, 
                                 NCX_NT_VAL, 
                                 testval);
            }
        }
        testval = val_get_choice_next_set(val, choicobj, testval);
    }

    if (val->res == NO_ERR) {
        val->res = retres;
    }

    return retres;

}  /* choice_check_agt */


/********************************************************************
* FUNCTION must_stmt_check
* 
* Check for any must-stmts in the object tree and validate the Xpath
* expression against the complete database 'root' and current
* context node 'curval'
* 
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   root == val_value_t or the target database root to validate
*   curval == val_value_t for the current context node in the tree
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
static status_t 
    must_stmt_check (ses_cb_t *scb,
                     xml_msg_hdr_t *msg,
                     val_value_t *root,
                     val_value_t *curval)
{
    obj_template_t  *obj;
    dlq_hdr_t       *mustQ;
    xpath_pcb_t           *must;
    xpath_result_t        *result;
    val_value_t           *chval;
    status_t               res, retres;

    obj = curval->obj;

    if (!obj_is_config(obj)) {
        return NO_ERR;
    }

    retres = NO_ERR;

    /* execute all the must tests top down, so
     * foo/bar errors are reported before /foo/bar/child
     */
    mustQ = obj_get_mustQ(obj);
    if (mustQ && !dlq_empty(mustQ)) {
        for (must = (xpath_pcb_t *)dlq_firstEntry(mustQ);
             must != NULL;
             must = (xpath_pcb_t *)dlq_nextEntry(must)) {

            res = NO_ERR;
            result = xpath1_eval_expr(must, 
                                      curval, 
                                      root, 
                                      FALSE, 
                                      TRUE, 
                                      &res);
            if (!result || res != NO_ERR) {
                log_error("\nmust_chk: failed for "
                          "%s:%s (%s) expr '%s'",
                          obj_get_mod_name(obj),
                          curval->name,
                          get_error_string(res),
                          must->exprstr);
                if (res == NO_ERR) {
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                }
                agt_record_error_errinfo(scb, 
                                         msg,
                                         NCX_LAYER_CONTENT,
                                         res, 
                                         NULL,
                                         NCX_NT_STRING,
                                         must->exprstr,
                                         NCX_NT_VAL,
                                         curval,
                                         (ncx_errinfo_set
                                          (&must->errinfo)) ?
                                         &must->errinfo : NULL);
                CHK_EXIT(res, retres);
            } else if (!xpath_cvt_boolean(result)) {
                if (LOGDEBUG2) {
                    log_debug2("\nmust_chk: false for %s:%s expr '%s'", 
                               obj_get_mod_name(obj),
                               curval->name,
                               must->exprstr);
                }

                res = ERR_NCX_MUST_TEST_FAILED;
                agt_record_error_errinfo(scb, 
                                         msg,
                                         NCX_LAYER_CONTENT,
                                         res,
                                         NULL,
                                         NCX_NT_STRING,
                                         must->exprstr,
                                         NCX_NT_VAL,
                                         curval,
                                         (ncx_errinfo_set
                                          (&must->errinfo)) ?
                                         &must->errinfo : NULL);
                CHK_EXIT(res, retres);
            } else {
                if (LOGDEBUG3) {
                    log_debug3("\nmust_chk: OK for %s:%s expr '%s'", 
                               obj_get_mod_name(obj),
                               curval->name,
                               must->exprstr);
                }
            }

            if (result) {
                xpath_free_result(result);
            }

            if (res != NO_ERR) {
                curval->res = res;
            }
        }
    }

    /* recurse for every child node until leafs are hit */
    for (chval = val_get_first_child(curval);
         chval != NULL && retres == NO_ERR;
         chval = val_get_next_child(chval)) {

        if (obj_is_root(chval->obj)) {
            /* do not dive into <config> parameters and
             * hit database must-stmts by mistake
             */
            continue;
        }
        res = must_stmt_check(scb, msg, root, chval);
        CHK_EXIT(res, retres);
    }

    return retres;
    
}  /* must_stmt_check */


/********************************************************************
* FUNCTION when_stmt_check
* 
* Check for any when-stmts in the object tree and validate the Xpath
* expression against the complete database 'root' and current
* context node 'curval'.
*
* There are 3 possible when-stmts to evaluate for every cooked node
*
*     object when
*     augment when
*     uses when
*
* Since deleting any node may cause other when-stmts to become 
* false, tis function needs to be called N times until the
* deleteQ is returned empty.
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   root == val_value_t or the target database root to validate
*   curval == val_value_t for the current context node in the tree
*   configmode == TRUE to test config-TRUE
*                 FALSE to test config=FALSE
*   deleteQ  == address of Q for deleted descendants
*   deleteme == address of return delete flag
*   rpcmode == TRUE if this function is being called
*              to flag false when-stmt errors in
*              the RPC input parameters
*              FALSE if this is regular data tree processing mode
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*    deleteQ will have any deleted descendant entries
*         due to false when-stmt expressions
*   *deleteme == TRUE if this node failed its when test
*                and needs to be deleted;
*
* RETURNS:
*   status of the operation, NO_ERR or ERR_INTERNAL_MEM most likely
*********************************************************************/
static status_t 
    when_stmt_check (ses_cb_t *scb,
                     xml_msg_hdr_t *msg,
                     val_value_t *root,
                     val_value_t *curval,
                     boolean configmode,
                     dlq_hdr_t *deleteQ,
                     boolean *deleteme,
                     boolean rpcmode)
{
    obj_template_t        *obj;
    val_value_t           *chval, *nextchild;
    status_t               res, retres;
    boolean                deletechild, condresult;
    uint32                 whencount;

    *deleteme = FALSE;
    obj = curval->obj;

    if (configmode != obj_is_config(obj)) {
        return NO_ERR;
    }

    retres = NO_ERR;
    whencount = 0;
    condresult = FALSE;
    retres = val_check_obj_when(curval,
                                root,
                                curval,
                                obj,
                                &condresult,
                                &whencount);
    if (retres != NO_ERR) {
        log_error("\nError: when_check: failed for "
                  "%s:%s (%s)",
                  obj_get_mod_name(obj),
                  obj_get_name(obj),
                  get_error_string(retres));
    } else if (!condresult) {
        if (rpcmode) {
            agt_record_error(scb,
                             msg,
                             NCX_LAYER_OPERATION,
                             ERR_NCX_RPC_WHEN_FAILED,
                             NULL,
                             NCX_NT_NONE,
                             NULL,
                             NCX_NT_VAL,
                             curval);
        } else {
            if (LOGDEBUG2 && whencount) {
                log_debug2("\nwhen_chk: test false for "
                           "node '%s:%s'", 
                           obj_get_mod_name(obj),
                           curval->name);
            }
        }
        *deleteme = TRUE;
    } else {
        if (LOGDEBUG3 && whencount) {
            log_debug3("\nwhen_chk: test passed for "
                       "node '%s:%s'", 
                       obj_get_mod_name(obj),
                       curval->name);
        }
    }

    if (*deleteme) {
        return NO_ERR;
    }

    /* recurse for every child node until leafs are hit */
    for (chval = val_get_first_child(curval);
         chval != NULL && retres == NO_ERR;
         chval = nextchild) {

        nextchild = val_get_next_child(chval);

        if (!obj_is_config(chval->obj)) {
            continue;
        }

        if (obj_is_root(chval->obj)) {
            /* do not dive into a <config> node parameter
             * while processing an RPC input node
             */
            continue;
        }

        deletechild = FALSE;
        res = when_stmt_check(scb, 
                              msg, 
                              root, 
                              chval, 
                              configmode,
                              deleteQ, 
                              &deletechild,
                              rpcmode);
        CHK_EXIT(res, retres);
        if (res == NO_ERR && deletechild) {
            val_remove_child(chval);
            dlq_enque(chval, deleteQ);
        }
    }

    return retres;
    
}  /* when_stmt_check */


/********************************************************************
* FUNCTION delete_empty_npcontainers
* 
* Check for any empty non-presense containers
* and delete them
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   root == val_value_t or the target database root to validate
*   curval == val_value_t for the current context node in the tree
*   configmode == TRUE to test config-TRUE
*                 FALSE to test config=FALSE
*   deleteQ  == address of Q for deleted descendants
*   deleteme == address of return delete flag
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*    deleteQ will have any deleted descendant entries
*         due to false when-stmt expressions
*   *deleteme == TRUE if this node failed its when test
*                and needs to be deleted;
*
*********************************************************************/
static void
    delete_empty_npcontainers (ses_cb_t *scb,
                               xml_msg_hdr_t *msg,
                               val_value_t *root,
                               val_value_t *curval,
                               boolean configmode,
                               dlq_hdr_t *deleteQ,
                               boolean *deleteme)
{
    obj_template_t  *obj;
    val_value_t           *chval, *nextchild;
    boolean                deletechild;

    *deleteme = FALSE;
    obj = curval->obj;

    if (configmode != obj_is_config(obj)) {
        return;
    }

    if (obj_is_np_container(curval->obj) &&
        !val_get_first_child(curval)) {
        *deleteme = TRUE;
        return;
    }

    /* recurse for every child node until leafs are hit */
    for (chval = val_get_first_child(curval);
         chval != NULL;
         chval = nextchild) {

        nextchild = val_get_next_child(chval);

        delete_empty_npcontainers(scb, 
                                  msg, 
                                  root, 
                                  chval, 
                                  configmode,
                                  deleteQ, 
                                  &deletechild);
        if (deletechild) {
            val_remove_child(chval);
            dlq_enque(chval, deleteQ);
        }
    }

}  /* delete_empty_npcontainers */


/********************************************************************
* FUNCTION delete_dead_nodes
* 
* Delete all the nodes that have false when-stmt exprs
* Also delete empty NP-containers
*
* INPUTS:
*   scb == session control block
*   msg == incoming commit rpc_msg_t in progress
*   root == target database root value node
*   configmode == TRUE for testing config=TRUE nodes only
*              == FALSE for testing config=FALSE nodes only
*   npcontainers == TRUE to delete empty NP containers as well
*                   FALSE to ignore empty NP containers
*
* OUTPUTS:
*   false when-stmt nodes will be deleted from the database
*   
* RETURNS:
*   status
*********************************************************************/
static status_t
    delete_dead_nodes (ses_cb_t  *scb,
                       rpc_msg_t *msg,
                       val_value_t *root,
                       boolean configmode,
                       boolean npcontainers)
{
    cfg_template_t  *cfg;
    val_value_t     *deleteval;
    dlq_hdr_t        deleteQ;
    boolean          deleteme, done, isrunning;
    status_t         res;

    dlq_createSQue(&deleteQ);
    res = NO_ERR;
    done = FALSE;

    cfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (cfg && cfg->root && cfg->root == root) {
        isrunning = TRUE;
    } else {
        isrunning = FALSE;
    }

    if (npcontainers) {
        delete_empty_npcontainers(scb, 
                                  (msg) ? &msg->mhdr : NULL,
                                  root, 
                                  root, 
                                  configmode,
                                  &deleteQ, 
                                  &deleteme);


        while (!dlq_empty(&deleteQ)) {
            /**** NEED TO DEAL WITH ROLLBACK
             **** INSTEAD OF REALLY DELETING THESE NODES
             ****/
            deleteval = (val_value_t *)dlq_deque(&deleteQ);

            if (LOGDEBUG) {
                log_debug("\nagt_val: deleting empty NP "
                          "container node '%s:%s'",
                          obj_get_mod_name(deleteval->obj),
                          deleteval->name);
            }

            if (isrunning) {
                handle_audit_record(OP_EDITOP_DELETE, 
                                    scb, 
                                    msg,
                                    deleteval);
            }

            val_free_value(deleteval);
        }
    }

    while (!done && res == NO_ERR) {

        /* keep checking the root until no more deletes */
        res = when_stmt_check(scb, 
                              (msg) ? &msg->mhdr : NULL,
                              root, 
                              root, 
                              configmode,
                              &deleteQ, 
                              &deleteme,
                              FALSE);

        if (!dlq_empty(&deleteQ)) {
            while (!dlq_empty(&deleteQ)) {
                /**** NEED TO DEAL WITH ROLLBACK
                 **** INSTEAD OF REALLY DELETING THESE NODES
                 ****/
                deleteval = (val_value_t *)dlq_deque(&deleteQ);

                if (LOGDEBUG) {
                    log_debug("\nagt_val: deleting false "
                              "when node '%s:%s'",
                              obj_get_mod_name(deleteval->obj),
                              deleteval->name);
                }

                if (isrunning) {
                    handle_audit_record(OP_EDITOP_DELETE, 
                                        scb, 
                                        msg,
                                        deleteval);
                }

                val_free_value(deleteval);
            }
        } else {
            done = TRUE;
        }
    }

    return res;

}  /* delete_dead_nodes */


/********************************************************************
* FUNCTION apply_commit_deletes
* 
* Apply the requested commit delete operations
*
* Invoke all the AGT_CB_COMMIT callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming commit rpc_msg_t in progress
*   target == target database (NCX_CFGID_RUNNING)
*   candval == value struct from the candidate config
*   runval == value struct from the running config
*
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added 
*   to the msg->mhsr.errQ
*
* RETURNS:
*   none
*********************************************************************/
static status_t
    apply_commit_deletes (ses_cb_t  *scb,
                          rpc_msg_t  *msg,
                          cfg_template_t *target,
                          val_value_t *candval,
                          val_value_t *runval)
{
    val_value_t      *curval, *nextval, *matchval;
    status_t          res;

    res = NO_ERR;

    /* go through running config
     * if the matching node is not in the candidate,
     * then delete that node in the running config as well
     */
    for (curval = val_get_first_child(runval);
         curval != NULL && res == NO_ERR; 
         curval = nextval) {

        nextval = val_get_next_child(curval);

        /* check only database config nodes */
        if (obj_is_data_db(curval->obj) &&
            obj_is_config(curval->obj)) {

            /* check if node deleted in source */
            matchval = val_first_child_match(candval, curval);
            if (!matchval) {
                /* prevent the agt_val code from ignoring this node */
                val_set_dirty_flag(curval);

                /* deleted in the source, so delete in the target */
                res = handle_callback(AGT_CB_APPLY,
                                      OP_EDITOP_DELETE, 
                                      scb, 
                                      msg, 
                                      target, 
                                      NULL, 
                                      curval);
            } else {
                /* else keep this node in target config
                 * but check any child nodes for deletion
                 */
                res = apply_commit_deletes(scb, 
                                           msg, 
                                           target,
                                           matchval, 
                                           curval);
            }
        }  /* else skip non-config database node */
    }

    return res;

}   /* apply_commit_deletes */


/********************************************************************
* FUNCTION check_commit_deletes
* 
* Check the requested commit delete operations
*
* INPUTS:
*   scb == session control block
*   msg == incoming commit rpc_msg_t in progress
*   target == target database (NCX_CFGID_RUNNING)
*   candval == value struct from the candidate config
*   runval == value struct from the running config
*
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added 
*   to the msg->mhsr.errQ
*
* RETURNS:
*   none
*********************************************************************/
static status_t
    check_commit_deletes (ses_cb_t  *scb,
                          rpc_msg_t  *msg,
                          cfg_template_t *target,
                          val_value_t *candval,
                          val_value_t *runval)
{
    val_value_t      *curval, *nextval, *matchval;
    status_t          res;
    uint32            lockid;

    res = NO_ERR;

    /* go through running config
     * if the matching node is not in the candidate,
     * then delete that node in the running config as well
     */
    for (curval = val_get_first_child(runval);
         curval != NULL && res == NO_ERR; 
         curval = nextval) {

        nextval = val_get_next_child(curval);

        /* check only database config nodes */
        if (obj_is_data_db(curval->obj) &&
            obj_is_config(curval->obj)) {

            /* check if node deleted in source */
            matchval = val_first_child_match(candval, curval);
            if (!matchval) {
                /* check if this curval is partially locked */
                lockid = 0;
                res = val_write_ok(curval,
                                   OP_EDITOP_DELETE,
                                   SES_MY_SID(scb),
                                   TRUE,
                                   &lockid);
                if (res != NO_ERR) {
                    agt_record_error(scb, 
                                     &msg->mhdr, 
                                     NCX_LAYER_CONTENT,
                                     res,
                                     NULL,
                                     NCX_NT_UINT32_PTR, 
                                     &lockid,
                                     NCX_NT_VAL, 
                                     curval);
                }
            } else {
                /* else keeping this node in target config
                 * but check any child nodes for deletion
                 */
                res = check_commit_deletes(scb, 
                                           msg, 
                                           target,
                                           matchval, 
                                           curval);
            }
        }  /* else skip non-config database node */
    }

    return res;

}   /* check_commit_deletes */


/******************* E X T E R N   F U N C T I O N S ***************/


/********************************************************************
* FUNCTION agt_val_rpc_xpath_check
* 
* Check for any nodes which are present
* but have false when-stmts associated
* with the node.  These are errors and
* need to be flagged as unknown-element
* 
* Any false nodes will be removed from the input PDU
* and discarded, after the error is recorded.
* This prevents false positives or negatives in
* the agt_val_instance_check, called after this function
*
* Also checks any false must-stmts for nodes
* which are present (after false when removal)
* These are flagged as 'must-violation' errors
* as per YANG, 13.4
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   rpcinput == RPC input node conceptually under rpcroot
*               except this rpcinput has no parent node
*               so a fake one will be termporarily added 
*               to prevent false XPath validation errors
*   rpcroot == RPC method node. 
*              The conceptual parent of this node 
*              is used as the document root (/rpc == /)
*
* OUTPUTS:
*   if false nodes found under :
*     they are deleted
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation
*   NO_ERR if no false when or must statements found
*********************************************************************/
status_t 
    agt_val_rpc_xpath_check (ses_cb_t *scb,
                             xml_msg_hdr_t *msg,
                             val_value_t *rpcinput,
                             obj_template_t *rpcroot)
{
    val_value_t           *method, *deleteval;
    dlq_hdr_t              deleteQ;
    status_t               res, retres;
    boolean                deleteme, done;

#ifdef DEBUG
    if (!rpcinput || !rpcroot) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    retres = NO_ERR;
    method = NULL;
    deleteme = FALSE;
    done = FALSE;
    dlq_createSQue(&deleteQ);

    if (LOGDEBUG3) {
        log_debug3("\nagt_val_rpc_xpathchk: %s:%s start", 
                   obj_get_mod_name(rpcroot),
                   obj_get_name(rpcroot));
    }

    /* make a dummy tree to align with the XPath code */
    method = val_new_value();
    if (!method) {
        return ERR_INTERNAL_MEM;
    }
    val_init_from_template(method, rpcroot);

    /* add the rpc/method-name/input node */
    val_add_child(rpcinput, method);

    /* loop through the PDU over and over until there
     * are no more entries found in the deleteQ
     */
    while (!done) {

        deleteme = FALSE;

        /* keep checking the root until no more deletes */
        res = when_stmt_check(scb, 
                              msg, 
                              method, 
                              rpcinput, 
                              TRUE,
                              &deleteQ, 
                              &deleteme,
                              TRUE);
        if (res != NO_ERR) {
            retres = res;
            if (NEED_EXIT(res)) {
                done = TRUE;
            }
        }

        if (deleteme) {
            /* the input node is not allowed to have 
             * must or when statements
             */
            SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (!dlq_empty(&deleteQ)) {
            while (!dlq_empty(&deleteQ)) {
                deleteval = (val_value_t *)dlq_deque(&deleteQ);
                val_free_value(deleteval);
            }
        } else {
            done = TRUE;
        }
    }

    retres = res;

    /* check if any must expressions apply to
     * descendent-or-self nodes in the rpcinput node
     */
    res = must_stmt_check(scb, msg, method, rpcinput);

    /* cleanup fake RPC method parent node */
    val_remove_child(rpcinput);
    val_free_value(method);

    CHK_EXIT(res, retres);

    return retres;

} /* agt_val_rpc_xpath_check */


/********************************************************************
* FUNCTION agt_val_instance_check
* 
* Check for the proper number of object instances for
* the specified value struct.
* 
* The top-level value set passed cannot represent a choice
* or a case within a choice.
*
* This function is intended for validating PDUs (RPC requests)
* during the PDU processing.  It does not check the instance
* count or must-stmt expressions for any <config> (ncx:root)
* container.  This must be dome with the agt_val_root_check function.
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == xml_msg_hdr t from msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   valset == val_value_t list, leaf-list, or container to check
*   root == database root of 'valset'
*   layer == NCX layer calling this function (for error purposes only)
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
status_t 
    agt_val_instance_check (ses_cb_t *scb,
                            xml_msg_hdr_t *msg,
                            val_value_t *valset,
                            val_value_t *root,
                            ncx_layer_t layer)
{
    obj_template_t  *obj, *chobj;
    val_value_t           *chval;
    status_t               res, retres;

#ifdef DEBUG
    if (!valset) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

#ifdef AGT_VAL_DEBUG
    if (LOGDEBUG4) {
        log_debug4("\nagt_val_instchk: %s:%s start", 
                   obj_get_mod_name(valset->obj),
                   valset->name);
    }
#endif

    retres = NO_ERR;

    obj = valset->obj;

    if (obj_is_cli(obj) || obj_is_abstract(obj)) {
        return NO_ERR;
    }

    if (val_child_cnt(valset)) {
        for (chval = val_get_first_child(valset);
             chval != NULL;
             chval = val_get_next_child(chval)) {

            if (obj_is_root(chval->obj)) {
                continue;
            } else if (!obj_is_leafy(chval->obj)) {
                /* recurse for all object types except leaf and leaf-list */
                res = agt_val_instance_check(scb, 
                                             msg, 
                                             chval, 
                                             root, 
                                             layer);
                CHK_EXIT(res, retres);
            } else if (chval->btyp == NCX_BT_LEAFREF ||
                       chval->btyp == NCX_BT_INSTANCE_ID) {
                res = instance_xpath_check(scb, 
                                           msg, 
                                           chval, 
                                           root, 
                                           layer);
                CHK_EXIT(res, retres);
            }
        }
    }

    /* check all the child nodes for correct number of instances */
    for (chobj = obj_first_child(obj);
         chobj != NULL;
         chobj = obj_next_child(chobj)) {
        
        if (obj_is_root(chobj)) {
            continue;
        }

        if (chobj->objtype == OBJ_TYP_CHOICE) {
            res = choice_check_agt(scb, 
                                   msg, 
                                   chobj, 
                                   valset, 
                                   root, 
                                   layer);
        } else {
            res = instance_check(scb, 
                                 msg, 
                                 chobj, 
                                 valset, 
                                 root, 
                                 layer);
        }
        if (res != NO_ERR && valset->res == NO_ERR) {            
            valset->res = res;
        }
        CHK_EXIT(res, retres);
    }

    return retres;
    
}  /* agt_val_instance_check */


/********************************************************************
* FUNCTION agt_val_root_check
* 
* Check for the proper number of object instances for
* the specified configuration database
* 
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == RPC msg in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   root == val_value_t for the target config being checked
*
* OUTPUTS:
*   if msg not NULL:
*      msg->mhdr.msg_errQ may have rpc_err_rec_t 
*      structs added to it which must be freed by the 
*      caller with the rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
status_t 
    agt_val_root_check (ses_cb_t *scb,
                        rpc_msg_t *msg,
                        val_value_t *root)
{
    ncx_module_t          *mod;
    obj_template_t        *obj, *chobj;
    val_value_t           *chval;
    status_t               res, retres;
    xmlns_id_t             ncxid;
    
#ifdef DEBUG
    if (!root) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(root->obj)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    if (LOGDEBUG3) {
        log_debug3("\nagt_val_root_check: start");
    }

    retres = NO_ERR;
    ncxid = xmlns_ncx_id();
    obj = root->obj;

    /* first need to delete all the false when-stmt
     * config objects and empty NP containers
     * and then see if the config is valid
     */
    res = delete_dead_nodes(scb, msg, root, TRUE, TRUE);
    CHK_EXIT(res, retres);

    /* check the instance counts for the subtrees that are present */
    res = agt_val_instance_check(scb, 
                                 (msg) ? &msg->mhdr : NULL,
                                 root, 
                                 root, 
                                 NCX_LAYER_CONTENT);
    CHK_EXIT(res, retres);

    /* check the must-stmt expressions for the subtrees that are present */
    for (chval = val_get_first_child(root);
         chval != NULL;
         chval = val_get_next_child(chval)) {

        if (!obj_is_config(chval->obj)) {
            continue;
        }

        res = must_stmt_check(scb, 
                              (msg) ? &msg->mhdr : NULL, 
                              root, 
                              chval);
        CHK_EXIT(res, retres);

        res = unique_stmt_check(scb, 
                                (msg) ? &msg->mhdr : NULL, 
                                chval);
        CHK_EXIT(res, retres);
    }

    /* check all the modules in the system for top-level objects and
     * check the instance count and any missing mandatory top-level nodes
     * this is CPU intensive if the agent has a lot of objects
     *
     */
    for (mod = ncx_get_first_module();
         mod != NULL;
         mod = ncx_get_next_module(mod)) {

        /* hack: skip the NCX extensions module that defines objects
         * just for the XSD generation usage
         */
        if (mod->nsid == ncxid) {
            continue;
        }

        for (chobj = ncx_get_first_data_object(mod);
             chobj != NULL;
             chobj = ncx_get_next_data_object(mod, chobj)) {

            if (obj_is_cli(chobj) || 
                obj_is_abstract(chobj) ||
                !obj_is_config(chobj)) {
                continue;
            }

            if (chobj->objtype == OBJ_TYP_CHOICE) {
                res = choice_check_agt(scb, 
                                       (msg) ? &msg->mhdr : NULL, 
                                       chobj, 
                                       root, 
                                       root, 
                                       NCX_LAYER_CONTENT);
            } else {
                res = instance_check(scb, 
                                     (msg) ? &msg->mhdr : NULL, 
                                     chobj, 
                                     root, 
                                     root, 
                                     NCX_LAYER_CONTENT);
            }
            CHK_EXIT(res, retres);
        }
    }

    if (LOGDEBUG3) {
        log_debug3("\nagt_val_root_check: end");
    }

    return retres;
    
}  /* agt_val_root_check */


/********************************************************************
* FUNCTION agt_val_split_root_check
* 
* Check for the proper number of object instances for
* the specified configuration database.  Conceptually
* combine the newroot and root and check that.
* 
* This function is only used if the cfg target is RUNNING
* The CANDIDATE cfg should use the agt_val_root_check
* instead for a pre-commit test
*
* INPUTS:
*   scb == session control block (may be NULL; no session stats)
*   msg == RPC message in progress 
*       == NULL MEANS NO RPC-ERRORS ARE RECORDED
*   newroot == val_value_t for the edit-config config contents
*   root == val_value_t for the target config being checked
*   defop == the starting default-operation value
*
* OUTPUTS:
*   if msg not NULL:
*      msg->msg_errQ may have rpc_err_rec_t structs added to it 
*      which must be freed by the called with the 
*      rpc_err_free_record function
*
* RETURNS:
*   status of the operation, NO_ERR if no validation errors found
*********************************************************************/
status_t 
    agt_val_split_root_check (ses_cb_t *scb,
                              rpc_msg_t  *msg,
                              val_value_t *newroot,
                              val_value_t *root,
                              op_editop_t  defop)
{
    val_value_t     *copyroot;
    status_t         res;

#ifdef DEBUG
    if (!newroot || !root) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(root->obj)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* create a temporary root config clone and do
     * a test apply to that copy.  This will create
     * the proper config for must, instance, choice,
     * and unique tests.  
     *
     * Resource errors due to user callback code
     * can still happen during the real apply
     */
    res = NO_ERR;
    copyroot = val_clone_config_data(root, &res);
    if (!copyroot) {
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_CONTENT,
                         ERR_NCX_OPERATION_FAILED, 
                         NULL,
                         NCX_NT_STRING, 
                         get_error_string(res), 
                         NCX_NT_VAL, 
                         root);
        return res;
    }

    /* do a dummy test apply to the partial config
     * start with the config root, which is a val_value_t node 
     */
    res = handle_callback(AGT_CB_TEST_APPLY, 
                          defop, 
                          scb, 
                          msg, 
                          NULL, 
                          newroot, 
                          copyroot);

    if (res == NO_ERR) {
        res = agt_val_root_check(scb, msg, copyroot);
    }

    val_free_value(copyroot);

    return res;
    
}  /* agt_val_split_root_check */


/********************************************************************
* FUNCTION agt_val_validate_write
* 
* Validate the requested <edit-config> write operation
*
* Check all the embedded operation attributes against
* the default-operation and maintained current operation.
*
* Invoke all the user AGT_CB_VALIDATE callbacks for a 
* 'new value' and 'existing value' pairs, for a given write operation, 
*
* These callbacks are invoked bottom-up, so the first step is to
* step through all the child nodes and traverse the
* 'new' data model (from the PDU) all the way to the leaf nodes
*
* The operation attribute is checked against the real data model
* on the way down the tree, and the user callbacks are invoked
* bottom-up on the way back.  This way, the user callbacks can
* share sub-tree validation routines, and perhaps add additional
* <rpc-error> information, based on the context and specific errors
* reported from 'below'.
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write 
*          == NULL for no actual write acess (validate only)
*   valroot == the val_value_t struct containing the root
*              (NCX_BT_CONTAINER, ncx:root)
*              datatype representing the config root with
*              proposed changes to the target
*   editop == requested start-state write operation
*             (usually from the default-operation parameter)
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added to the msg->rpc_errQ
*
* RETURNS:
*   status of the operation
*********************************************************************/
status_t
    agt_val_validate_write (ses_cb_t  *scb,
                            rpc_msg_t  *msg,
                            cfg_template_t *target,
                            val_value_t  *valroot,
                            op_editop_t  editop)
{
    status_t        res;

#ifdef DEBUG
    if (!scb || !msg || !valroot) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(valroot->obj)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }        
#endif

    if (target) {
        /* check the lock first */
        res = cfg_ok_to_write(target, scb->sid);
        if (res != NO_ERR) {
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_CONTENT,
                             res,
                             NULL,
                             NCX_NT_NONE,
                             NULL, 
                             NCX_NT_VAL, 
                             valroot);
            return res;
        }
    }

    /* the <config> root is just a value node of type 'root'
     * traverse all nodes and check the <edit-config> request
     */
    res = handle_callback(AGT_CB_VALIDATE, 
                          editop,
                          scb, 
                          msg,
                          target,
                          valroot,
                          (target) ? target->root : NULL);

    return res;

}  /* agt_val_validate_write */


/********************************************************************
* FUNCTION agt_val_apply_write
* 
* Apply the requested write operation
*
* Invoke all the AGT_CB_APPLY callbacks for a 
* source and target and write operation
*
* TBD: support for handling nested parmsets independently
*      of the parent parmset is not supported.  This means
*      that independent parmset instances nested within the
*      parent parmset all have to be applied, or none applied
*
* INPUTS:
*   scb == session control block
*   msg == incoming rpc_msg_t in progress
*   target == cfg_template_t for the config database to write
*   pducfg == the 'root' value struct that represents the
*             tree of changes to apply to the target
*   editop == requested start-state write operation
*             (usually from the default-operation parameter)
*
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added to the msg->mhsr.errQ
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_val_apply_write (ses_cb_t  *scb,
                         rpc_msg_t  *msg,
                         cfg_template_t *target,
                         val_value_t    *pducfg,
                         op_editop_t  editop)
{
    status_t              res;

#ifdef DEBUG
    if (!scb || !msg || !target || !pducfg) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!obj_is_root(pducfg->obj)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }        
#endif

    /* start with the config root, which is a val_value_t node */
    res = handle_callback(AGT_CB_APPLY, 
                          editop, 
                          scb, 
                          msg, 
                          target, 
                          pducfg, 
                          target->root);

    if (target->cfg_id == NCX_CFGID_RUNNING) {
        if (res==NO_ERR) {
            /* complete the operation */
            res = handle_callback(AGT_CB_COMMIT, 
                                  editop,
                                  scb, 
                                  msg,
                                  target, 
                                  pducfg,
                                  target->root);
        } else {
            /* rollback the operation */
            res = handle_callback(AGT_CB_ROLLBACK,
                                  editop,
                                  scb, 
                                  msg,
                                  target,
                                  pducfg,
                                  target->root);
        }
    }


    /* first need to delete all the false when-stmt
     * config objects and empty NP containers
     * and then see if the config is valid
     */
    if (res == NO_ERR) {
        res = delete_dead_nodes(scb, 
                                msg, 
                                target->root, 
                                TRUE,
                                FALSE);
    }

    return res;

}  /* agt_val_apply_write */


/********************************************************************
* FUNCTION agt_val_apply_commit
* 
* Apply the requested commit operation
*
* Invoke all the AGT_CB_COMMIT callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming commit rpc_msg_t in progress
*   source == cfg_template_t for the source (candidate)
*   target == cfg_template_t for the config database to 
*             write (running)
*   save_nvstore == TRUE if the mirrored NV-store
*                   should be updated after the commit is done
*                   FALSE if this is the start of a confirmed-commit
*                   so the NV-store update is deferred
*                   Never save to NV-store if :startup is supported
*
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added 
*   to the msg->mhdr.errQ
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_val_apply_commit (ses_cb_t  *scb,
                          rpc_msg_t  *msg,
                          cfg_template_t *source,
                          cfg_template_t *target,
                          boolean save_nvstore)
    
{
    val_value_t      *newval, *nextval, *matchval;
    agt_profile_t    *profile;
    status_t          res;
    boolean           no_startup_errors;

#ifdef DEBUG
    if (!scb || !msg || !source || !target) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    no_startup_errors = dlq_empty(&target->load_errQ);
    profile = agt_get_profile();

#ifdef ALLOW_SKIP_EMPTY_COMMIT
    /* usually only save if the source config was touched */
    if (!cfg_get_dirty_flag(source)) {
        /* no need to merge the candidate into the running
         * config because there are no changes in the candidate
         */
        if (profile->agt_has_startup) {
            /* separate copy-config required to rewrite
             * the startup config file
             */
            if (LOGDEBUG) {
                log_debug("\nSkipping commit, candidate not dirty");
            }
        } else {
            /* there is no distinct startup; need to mirror now;
             * check if the running config had load errors
             * so overwriting it even if the candidate is
             * clean makes sense
             */
            if (no_startup_errors) {
                if (LOGDEBUG) {
                    log_debug("\nSkipping commit, candidate not dirty");
                }
            } else {
                if (LOGINFO) {
                    log_debug("\nSkipping commit, but saving running "
                              "to NV-storage due to load errors");
                }
                res = agt_ncx_cfg_save(target, FALSE);
                if (res != NO_ERR) {
                    /* write to NV-store failed */
                    agt_record_error(scb,
                                     &msg->mhdr, 
                                     NCX_LAYER_OPERATION, 
                                     res, 
                                     NULL, 
                                     NCX_NT_CFG, 
                                     target,
                                     NCX_NT_NONE, 
                                     NULL);
                }
            }
        }
        return res;
    }
#endif

    /* check if any config nodes have been deleted in the target */
    res = apply_commit_deletes(scb, 
                               msg, 
                               target,
                               source->root,
                               target->root);

    /* check if any config nodes have been changed in the target */
    for (newval = val_get_first_child(source->root);
         newval != NULL && res == NO_ERR; 
         newval = nextval) {

        nextval = val_get_next_child(newval);

        if (obj_is_data_db(newval->obj) &&
            obj_is_config(newval->obj)) {

            matchval = val_first_child_match(target->root, newval);

            newval->editvars->curparent = target->root;
            
            res = handle_callback(AGT_CB_APPLY,
                                  OP_EDITOP_COMMIT, 
                                  scb, 
                                  msg, 
                                  target, 
                                  newval, 
                                  matchval);
        }
    }

    if (res==NO_ERR) {
        /* complete the operation */
        res = handle_callback(AGT_CB_COMMIT,
                              OP_EDITOP_COMMIT, 
                              scb, 
                              msg, 
                              target, 
                              source->root,
                              target->root);
    } else {
        /* rollback the operation */
        res = handle_callback(AGT_CB_ROLLBACK,
                              OP_EDITOP_COMMIT, 
                              scb, 
                              msg, 
                              target, 
                              source->root,
                              target->root);
    }

    if (res == NO_ERR && !profile->agt_has_startup) {
        if (save_nvstore) {
            res = agt_ncx_cfg_save(target, FALSE);
            if (res != NO_ERR) {
                /* write to NV-store failed */
                agt_record_error(scb,
                                 &msg->mhdr, 
                                 NCX_LAYER_OPERATION, 
                                 res, 
                                 NULL, 
                                 NCX_NT_CFG, 
                                 target,
                                 NCX_NT_NONE, 
                                 NULL);
            } else {
                /* don't clear the dirty flags in running
                 * unless the save to file  worked
                 */
                val_clean_tree(target->root);
            }
        } else if (LOGDEBUG2) {
            log_debug2("\nagt_val: defer NV-save after commit "
                       "until confirmed");
        }
    }

    return res;

}  /* agt_val_apply_commit */


/********************************************************************
* FUNCTION agt_val_check_commit_locks
* 
* Check if the requested commit operation
* would cause any partial lock violations 
* in the running config
* Invoke all the AGT_CB_COMMIT_CHECK callbacks for a 
* source and target and write operation
*
* INPUTS:
*   scb == session control block
*   msg == incoming commit rpc_msg_t in progress
*   source == cfg_template_t for the source (candidate)
*   target == cfg_template_t for the config database to 
*             write (running)
*
* OUTPUTS:
*   rpc_err_rec_t structs may be malloced and added 
*   to the msg->mhdr.errQ
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_val_check_commit_locks (ses_cb_t  *scb,
                                rpc_msg_t  *msg,
                                cfg_template_t *source,
                                cfg_template_t *target)
{
    status_t          res;

#ifdef DEBUG
    if (!scb || !msg || !source || !target) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;

    if (cfg_first_partial_lock(target) == NULL) {
        /* no need to check for partial-lock violations */
        return NO_ERR;
    }

    /* usually only save if the source config was touched */
    if (!cfg_get_dirty_flag(source)) {
        /* no need to check for partial-lock violations */
        return NO_ERR;
    }

    /* check if any config nodes have been deleted in the target */
    res = check_commit_deletes(scb, 
                               msg, 
                               target,
                               source->root,
                               target->root);
    if (res != NO_ERR) {
        /* error already recorded */
        return res;
    }

    res = handle_callback(AGT_CB_COMMIT_CHECK,
                          OP_EDITOP_COMMIT,
                          scb,
                          msg,
                          target,
                          source->root,
                          target->root);
    return res;

}  /* agt_val_check_commit_locks */


/* END file agt_val.c */
