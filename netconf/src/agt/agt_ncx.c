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
/*  FILE: agt_ncx.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
04feb06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_acm
#include "agt_acm.h"
#endif

#ifndef _H_agt_cap
#include "agt_cap.h"
#endif

#ifndef _H_agt_cb
#include "agt_cb.h"
#endif

#ifndef _H_agt_cli
#include "agt_cli.h"
#endif

#ifndef _H_agt_ncx
#include "agt_ncx.h"
#endif

#ifndef _H_agt_rpc
#include "agt_rpc.h"
#endif

#ifndef _H_agt_rpcerr
#include "agt_rpcerr.h"
#endif

#ifndef _H_agt_ses
#include "agt_ses.h"
#endif

#ifndef _H_agt_sys
#include "agt_sys.h"
#endif

#ifndef _H_agt_state
#include "agt_state.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val
#include "agt_val.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_xml_wr
#include  "xml_wr.h"
#endif

#ifndef _H_yangconst
#include  "yangconst.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define AGT_NCX_DEBUG 1


/********************************************************************
*                                                                   *
*                            T Y P E S                              *
*                                                                   *
*********************************************************************/


/* candidate commit control block struct */
typedef struct commit_cb_t_ {
    xmlChar     *cc_backup_source;
    time_t       cc_start_time;
    uint32       cc_cancel_timeout;
    ses_id_t     cc_ses_id;
    boolean      cc_active;
} commit_cb_t;


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/

static boolean          agt_ncx_init_done = FALSE;

static commit_cb_t      commit_cb;


/********************************************************************
* FUNCTION get_validate
*
* get : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_validate (ses_cb_t *scb,
                  rpc_msg_t *msg,
                  xml_node_t *methnode)
{
    cfg_template_t *source;
    val_value_t    *parm;
    status_t        res;

    /* check if the <running> config is ready to read */
    source = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!source) {
        res = ERR_NCX_OPERATION_FAILED;
    } else {
        res = cfg_ok_to_read(source);
    }
    if (res != NO_ERR) {
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res,
                         methnode, 
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
        return res;
    }

    /* check the with-defaults parameter */
    parm = val_find_child(msg->rpc_input,
                          NULL, 
                          NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
        msg->mhdr.withdef = 
            ncx_get_withdefaults_enum(VAL_ENUM_NAME(parm));
    }

    /* check if the optional filter parameter is ok */
    res = agt_validate_filter(scb, msg);
    if (res != NO_ERR) {
        return res;   /* error already recorded */
    }

    /* cache the 2 parameters and the data output callback function 
     * There is no invoke function -- it is handled automatically
     * by the agt_rpc module
     */
    msg->rpc_user1 = source;
    msg->rpc_data_type = RPC_DATA_STD;
    msg->rpc_datacb = agt_output_filter;

    return NO_ERR;

} /* get_validate */


/********************************************************************
* FUNCTION get_config_validate
*
* get-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_config_validate (ses_cb_t *scb,
                         rpc_msg_t *msg,
                         xml_node_t *methnode)
{
    cfg_template_t *source;
    val_value_t    *parm;
    status_t        res;

    /* check if the source config database exists */
    res = agt_get_cfg_from_parm(NCX_EL_SOURCE, 
                                msg, 
                                methnode, 
                                &source);
    if (res != NO_ERR) {
        return res;  /* error already recorded */
    } 

    /* check if this config can be read right now */
    res = cfg_ok_to_read(source);
    if (res != NO_ERR) {
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res,
                         methnode, 
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
        return res;
    }

    /* check the with-defaults parameter */
    parm = val_find_child(msg->rpc_input,
                          NULL, 
                          NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
        msg->mhdr.withdef = 
            ncx_get_withdefaults_enum(VAL_ENUM_NAME(parm));
    }

    /* check if the optional filter parameter is ok */
    res = agt_validate_filter(scb, msg);
    if (res != NO_ERR) {
        return res;   /* error already recorded */
    }

    /* cache the 2 parameters and the data output callback function 
     * There is no invoke function -- it is handled automatically
     * by the agt_rpc module
     */
    msg->rpc_user1 = source;
    msg->rpc_data_type = RPC_DATA_STD;
    msg->rpc_datacb = agt_output_filter;

    return NO_ERR;

} /* get_config_validate */


/********************************************************************
* FUNCTION edit_config_validate
*
* edit-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    edit_config_validate (ses_cb_t *scb,
                          rpc_msg_t *msg,
                          xml_node_t *methnode)
{
    cfg_template_t       *target;
    val_value_t          *val;
    const agt_profile_t  *agt_profile;
    op_editop_t           defop;
    op_errop_t            errop;
    op_testop_t           testop;
    status_t              res;

    testop = OP_TESTOP_NONE;

    /* check if the source config database exists */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, 
                                msg, 
                                methnode, 
                                &target);
    if (res != NO_ERR) {
        return res;  /* error already recorded */
    } 

    agt_profile = agt_get_profile();

    /* get the default-operation parameter */
    val = val_find_child(msg->rpc_input, 
                         NC_MODULE,
                         NCX_EL_DEFAULT_OPERATION);
    if (!val || val->res != NO_ERR) {
        /* set to the default if any error */
        defop = OP_EDITOP_MERGE;
    } else {
        defop = op_defop_id(VAL_STR(val));
    }

    /* get the error-option parameter */
    val = val_find_child(msg->rpc_input, 
                         NC_MODULE,
                         NCX_EL_ERROR_OPTION);
    if (!val || val->res != NO_ERR) {
        /* set to the default if any error */
        errop = OP_ERROP_STOP;
    } else {
        errop = op_errop_id(VAL_STR(val));
    }

    /* the internal processing needs to know if rollback is
     * requested to optimize the undo-prep and cleanup code path
     */
    msg->rpc_err_option = errop;
    if (errop==OP_ERROP_ROLLBACK) {
        msg->rpc_need_undo = TRUE;
    }

    /* Get the test-option parameter:
     *
     * This implementation always runs the validation tests in 
     * the same order, even if the value 'set' is used.
     * The validation stage is never bypassed, even if 'set'
     * is used instead of 'test-then-set'.
     *
     * Get the value to check for the test-only extension
     */
    val = val_find_child(msg->rpc_input, 
                         NC_MODULE,
                         NCX_EL_TEST_OPTION);
    if (!val || val->res != NO_ERR) {
        /* set to the default if any error */
        if (agt_profile->agt_usevalidate) {
            testop = OP_TESTOP_TESTTHENSET;
        } else {
            testop = OP_TESTOP_SET;
        }
    } else if (!agt_profile->agt_usevalidate) {
        res = ERR_NCX_OPERATION_NOT_SUPPORTED;
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res,
                         methnode, 
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_VAL, 
                         val);
    } else {
        testop = op_testop_enum(VAL_STR(val));
    }

    if (res == NO_ERR) {
        /* get the config parameter */
        val = val_find_child(msg->rpc_input, 
                             NC_MODULE,
                             NCX_EL_CONFIG);
        if (val && val->res == NO_ERR) {
            /* validate the <config> element (wrt/ embedded operation
             * attributes) against the existing data model.
             * <rpc-error> records will be added as needed 
             */
            res = agt_val_validate_write(scb, 
                                         msg, 
                                         target, 
                                         val, 
                                         defop);

            /* for continue-on-error, ignore the validate return value
             * in case there are multiple parmsets and not all of them
             * had errors.  Force a NO_ERR return.
             */
            if (!NEED_EXIT(res)) {
                if (errop == OP_ERROP_CONTINUE) {
                    res = NO_ERR;
                }
            }

            /* if this is an edit-config on running
             * or a test-then-set edit on candidate
             * then make a copy of the root and do a
             * complete non-destructive validation
             */
            if (res==NO_ERR &&
                ((target->cfg_id == NCX_CFGID_RUNNING) ||
                 (target->cfg_id == NCX_CFGID_CANDIDATE &&
                  testop == OP_TESTOP_TESTTHENSET))) {
                res = agt_val_split_root_check(scb, 
                                               msg, 
                                               val, 
                                               target->root, 
                                               defop);
            }
        } else if (!val) {
            /* this is reported in agt_val_parse phase */
            res = ERR_NCX_DATA_MISSING;
        } else {
            res = val->res;
        }
    }

    /* save the default operation in 'user1' */
    msg->rpc_user1 = (void *)defop;

    /* save the test option in 'user2' */
    msg->rpc_user2 = (void *)testop;

    return res;

} /* edit_config_validate */


/********************************************************************
* FUNCTION edit_config_invoke
*
* edit-config : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    edit_config_invoke (ses_cb_t *scb,
                        rpc_msg_t *msg,
                        xml_node_t *methnode)
{
    cfg_template_t *target;
    val_value_t    *val;
    op_editop_t     defop;
    op_testop_t     testop;
    status_t        res;

    /* get the cached options */
    testop = (op_errop_t)msg->rpc_user2;

    /* quick exit if this is a test-only request */
    if (testop == OP_TESTOP_TESTONLY) {
        return NO_ERR;
    }
    defop = (op_defop_t)msg->rpc_user1;

    /* get the config to write */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &target);
    if (res != NO_ERR) {
        return res;  /* error already recorded */
    } 

    /* get pointer to the config parameter */
    val = val_find_child(msg->rpc_input, 
                         NC_MODULE,
                         NCX_EL_CONFIG);
    if (!val || val->res != NO_ERR) {
        /* set to the default if any error */
        return SET_ERROR(ERR_NCX_OPERATION_FAILED);
    }

    /* apply the <config> into the target config */
    res = agt_val_apply_write(scb, msg, target, val, defop);
    return res;

} /* edit_config_invoke */


/********************************************************************
* FUNCTION copy_config_validate
*
* copy-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    copy_config_validate (ses_cb_t *scb,
                          rpc_msg_t *msg,
                          xml_node_t *methnode)
{
    cfg_template_t     *srccfg, *destcfg;
    const cap_list_t   *mycaps;
    val_value_t        *parm, *srcval;
    status_t            res, retres;

    srccfg = NULL;
    srcval = NULL;
    destcfg = NULL;
    retres = NO_ERR;

    /* get the agent capabilities */
    mycaps = agt_cap_get_caps();
    if (!mycaps) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* get the config to copy from */
    res = agt_get_cfg_from_parm(NCX_EL_SOURCE, 
                                msg, 
                                methnode, 
                                &srccfg);
    if (res == ERR_NCX_SKIPPED) {
        res = agt_get_inline_cfg_from_parm(NCX_EL_SOURCE, 
                                           msg, 
                                           methnode, 
                                           &srcval);
        /* errors already recorded */
        if (res != NO_ERR) {
            retres = res;
        }
    } /* else errors already recorded */

    if (res == NO_ERR && srccfg != NULL) {
        switch (srccfg->cfg_id) {
        case NCX_CFGID_CANDIDATE:
            if (!cap_std_set(mycaps, CAP_STDID_CANDIDATE)) {
                res = ERR_NCX_OPERATION_NOT_SUPPORTED;
            }
            break;
        case NCX_CFGID_RUNNING:
            break;
        case NCX_CFGID_STARTUP:
            if (!cap_std_set(mycaps, CAP_STDID_STARTUP)) {
                res = ERR_NCX_OPERATION_NOT_SUPPORTED;
            }
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (res != NO_ERR) {
            /* cannot use this configuration datastore */
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_OPERATION, 
                             res,
                             methnode, 
                             (srccfg) ? NCX_NT_CFG : NCX_NT_NONE,
                             (const void *)srccfg, 
                             NCX_NT_NONE, 
                             NULL);
            retres = res;
        }
    } else if (res != NO_ERR) {
        retres = res;
    }

    /* get the config to copy to */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, 
                                msg, 
                                methnode, 
                                &destcfg);
    if (res == NO_ERR) {
        switch (destcfg->cfg_id) {
        case NCX_CFGID_CANDIDATE:
            if (!cap_std_set(mycaps, CAP_STDID_CANDIDATE)) {
                res = ERR_NCX_OPERATION_NOT_SUPPORTED;
            }
            break;
        case NCX_CFGID_RUNNING:
            /* an agent may choose not to support this
             * so it is not supported
             * use edit-config instead
             */
            res = ERR_NCX_OPERATION_NOT_SUPPORTED;
            break;
        case NCX_CFGID_STARTUP:
            if (!cap_std_set(mycaps, CAP_STDID_STARTUP)) {
                res = ERR_NCX_OPERATION_NOT_SUPPORTED;
            }
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }

        if (res != NO_ERR) {
            /* cannot use this configuration datastore */
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_OPERATION, 
                             res,
                             methnode, 
                             (destcfg) ? NCX_NT_CFG : NCX_NT_NONE,
                             (const void *)destcfg, 
                             NCX_NT_NONE, 
                             NULL);
            retres = res;
        }
    } else {
        retres = res;
    }

    /* get the with-defaults parameter */
    parm = val_find_child(msg->rpc_input,
                          NULL,
                          NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
        msg->mhdr.withdef = VAL_BOOL(parm);
    }

    /* check source config == dest config */
    if (retres == NO_ERR &&
        srccfg != NULL && 
        srccfg == destcfg) {

        /* invalid operation */
        res = ERR_NCX_OPERATION_NOT_SUPPORTED;
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res,
                         methnode, 
                         (destcfg) ? NCX_NT_CFG : NCX_NT_NONE,
                         (const void *)destcfg, 
                         NCX_NT_NONE, 
                         NULL);
        retres = res;
    }

    /* get the config state; check if lock can be granted
     * based on the current config state
     */
    if (retres == NO_ERR) {
        res = cfg_ok_to_write(destcfg, SES_MY_SID(scb));
        if (res != NO_ERR) {
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_OPERATION, 
                             res,
                             methnode, 
                             (destcfg) ? NCX_NT_CFG : NCX_NT_NONE,
                             (const void *)destcfg, 
                             NCX_NT_NONE, 
                             NULL);
            retres = res;
        }
    }

    if (retres == NO_ERR &&
        srcval != NULL && 
        destcfg->cfg_id != NCX_CFGID_STARTUP) {

        /* validate the <config> element (wrt/ embedded operation
         * attributes) against the existing data model.
         * <rpc-error> records will be added as needed 
         */
        res = agt_val_validate_write(scb, 
                                     msg, 
                                     destcfg, 
                                     srcval, 
                                     OP_EDITOP_REPLACE);

        /* if this is an edit-config on running
         * then make a copy of the root and do a
         * complete non-destructive validation
         */
        if (res==NO_ERR && destcfg->cfg_id == NCX_CFGID_RUNNING) {
            res = agt_val_split_root_check(scb, 
                                           msg, 
                                           srcval, 
                                           destcfg->root, 
                                           OP_EDITOP_REPLACE);
        }
    }
    

    if (retres == NO_ERR) {
        /* save the source and destination config
         * if srccfg is NULL, then the srcval will
         * need to be retrieved again
         */
        msg->rpc_user1 = srccfg;
        msg->rpc_user2 = destcfg;
    }

    return retres;

} /* copy_config_validate */


/********************************************************************
* FUNCTION copy_config_invoke
*
* copy-config : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    copy_config_invoke (ses_cb_t *scb,
                        rpc_msg_t *msg,
                        xml_node_t *methnode)
{
    cfg_template_t *source, *target, *running;
    val_value_t    *sourceval;
    status_t        res;

    res = NO_ERR;
    sourceval = NULL;

    source = (cfg_template_t *)msg->rpc_user1;
    target = (cfg_template_t *)msg->rpc_user2;

    running = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (running == NULL || running->src_url == NULL) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (source == NULL) {
        /* copy from inline data to a database */
        res = agt_get_inline_cfg_from_parm(NCX_EL_SOURCE,
                                           msg,
                                           methnode,
                                           &sourceval);
        if (res == NO_ERR) {
            switch (target->cfg_id) {
            case NCX_CFGID_STARTUP:
                res = agt_ncx_cfg_save_inline(source->src_url,
                                              sourceval);
                break;
            case NCX_CFGID_CANDIDATE:
                res = cfg_fill_candidate_from_inline(sourceval);
                break;
            default:
                res = SET_ERROR(ERR_INTERNAL_VAL);
            }
        }
    } else {
        /* copy from one config to another */
        switch (target->cfg_id) {
        case NCX_CFGID_STARTUP:
            res = agt_ncx_cfg_save(source, FALSE);
            break;
        case NCX_CFGID_CANDIDATE:
            switch (source->cfg_id) {
            case NCX_CFGID_RUNNING:
                /* same as discard-changes */
                res = cfg_fill_candidate_from_running();
                break;
            case NCX_CFGID_STARTUP:
                res = cfg_fill_candidate_from_startup();
                break;
            default:
                res = SET_ERROR(ERR_INTERNAL_VAL);
            }
            break;
        default:
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    if (res != NO_ERR) {
        /* config operation failed */
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION,
                         res, 
                         methnode,
                         NCX_NT_CFG, 
                         target, 
                         NCX_NT_NONE, 
                         NULL);
    }

    return res;

} /* copy_config_invoke */


/********************************************************************
* FUNCTION delete_config_validate
*
* delete-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    delete_config_validate (ses_cb_t *scb,
                            rpc_msg_t *msg,
                            xml_node_t *methnode)
{
    const agt_profile_t  *prof;
    cfg_template_t       *target;
    status_t              res;
    const void           *errval;
    char                 *errstr;
    ncx_node_t            errtyp;

    /* get the config to delete */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, 
                                msg, 
                                methnode, 
                                &target);
    if (res != NO_ERR) {
        return res;  /* error already recorded */
    } 

    /* get the agent profile */
    prof = agt_get_profile();
    if (!prof) {
        res = SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* check the value provided -- only <startup> supported !!! */
    if (res==NO_ERR) {
        /* check if the startup config is allowed to be deleted
         * and that is the config to be deleted
         */
        if (target->cfg_id == NCX_CFGID_STARTUP) {
            if (!prof->agt_has_startup) {
                res = ERR_NCX_OPERATION_NOT_SUPPORTED;
            }
        } else if (target->cfg_id == NCX_CFGID_CANDIDATE) {
            if (prof->agt_targ != NCX_AGT_TARG_CANDIDATE) {
                res = ERR_NCX_OPERATION_NOT_SUPPORTED;
            }
        } else {
            res = ERR_NCX_INVALID_VALUE;
        }
    }

    /* check if okay to delete this config now */
    if (res == NO_ERR) {
        res = cfg_ok_to_write(target, SES_MY_SID(scb));
    }

    if (res != NO_ERR) {
        errval = agt_get_parmval(NCX_EL_TARGET, msg);
        if (errval) {
            errtyp = NCX_NT_VAL;
        } else {
            errval = NCX_EL_TARGET;
            errtyp = NCX_NT_STRING;
        }

        errstr = strdup("/nc:rpc/nc:delete-config/nc:target");
        agt_record_error(scb, 
                         &msg->mhdr,
                         NCX_LAYER_OPERATION, 
                         res, 
                         methnode, 
                         errtyp, 
                         errval,
                         (errstr) ? NCX_NT_STRING : NCX_NT_NONE,
                         errstr);
        if (errstr) {
            m__free(errstr);
        }
    } else {
        msg->rpc_user1 = target;
    }
    return res;

} /* delete_config_validate */


/********************************************************************
* FUNCTION delete_config_invoke
*
* delete-config : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    delete_config_invoke (ses_cb_t *scb,
                          rpc_msg_t *msg,
                          xml_node_t *methnode)
{
    const agt_profile_t  *profile;
    cfg_template_t       *startup, *target;
    xmlChar              *fname;
    const xmlChar        *startspec;
    char                 *errstr;
    status_t              res;
    int                   retval;

    res = NO_ERR;

    (void)scb;

    target = (cfg_template_t *)msg->rpc_user1;

    if (target->cfg_id == NCX_CFGID_CANDIDATE) {
        /* do a discard-changes */
        if (cfg_get_dirty_flag(target)) {
            res = cfg_fill_candidate_from_running();
        } 
        return res;
    } 

    /* else this must be a request to delete the startup */
    profile = agt_get_profile();

    /* use the user-set startup or default filename */
    if (profile->agt_startup) {
        startspec = profile->agt_startup;
    } else {
        startspec = NCX_DEF_STARTUP_FILE;
    }

    fname = ncxmod_find_data_file(startspec,
                                  FALSE, 
                                  &res);
    if (fname) {
        retval = remove((const char *)fname);
        if (retval != 0) {
            res = errno_to_status();
            errstr = (char *)
                xml_strdup((const xmlChar *)
                           "/nc:rpc/nc:delete-config/nc:target");
            agt_record_error(scb, 
                             &msg->mhdr,
                             NCX_LAYER_OPERATION, 
                             res, 
                             methnode, 
                             NCX_NT_STRING, 
                             fname,
                             (errstr) ? NCX_NT_STRING : NCX_NT_NONE,
                             errstr);
            if (errstr) {
                m__free(errstr);
            }
        } else {
            startup = target;
            if (startup != NULL && startup->root != NULL) {
                val_free_value(startup->root);
                startup->root = NULL;
            }
        }
        m__free(fname);
        return res;
    } else {
        if (LOGWARN) {
            log_warn("\nWarning: cannot find config file '%s' to delete",
                     startspec);
        }
    }

    return NO_ERR;

} /* delete_config_invoke */


/********************************************************************
* FUNCTION lock_validate
*
* lock : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    lock_validate (ses_cb_t *scb,
                   rpc_msg_t *msg,
                   xml_node_t *methnode)
{

    status_t         res;
    cfg_template_t  *cfg;

    /* get the config to lock */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &cfg);
    if (res != NO_ERR) {
        return res;
    }

    /* get the config state; check if lock can be granted
     * based on the current config state
     */
    res = cfg_ok_to_lock(cfg);
    if (res == NO_ERR) {
        /* lock can be granted
         * setup the user1 scratchpad with the cfg to lock 
         */
        msg->rpc_user1 = (void *)cfg;
    } else {
        /* lock probably already held */
        agt_record_error(scb,
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION,
                         res,
                         methnode,
                         NCX_NT_CFG, 
                         cfg,
                         NCX_NT_NONE,
                         NULL);
    }
    return res;

} /* lock_validate */


/********************************************************************
* FUNCTION lock_invoke
*
* lock : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    lock_invoke (ses_cb_t *scb,
                 rpc_msg_t *msg,
                 xml_node_t *methnode)
{
    cfg_template_t   *cfg;
    status_t          res;

    cfg = (cfg_template_t *)msg->rpc_user1;
    res = cfg_lock(cfg, SES_MY_SID(scb), CFG_SRC_NETCONF);
    if (res != NO_ERR) {
        /* config is in a state where locks cannot be granted */
        agt_record_error(scb,
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION,
                         res, 
                         methnode,
                         NCX_NT_NONE, 
                         NULL,
                         NCX_NT_NONE, 
                         NULL);
    }

    return res;

} /* lock_invoke */


/********************************************************************
* FUNCTION unlock_validate
*
* unlock : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    unlock_validate (ses_cb_t *scb,
                     rpc_msg_t *msg,
                     xml_node_t *methnode)
{
    status_t         res;
    cfg_template_t  *cfg;

    /* get the config to lock */
    res = agt_get_cfg_from_parm(NCX_EL_TARGET, msg, methnode, &cfg);
    if (res != NO_ERR) {
        return res;
    }

    /* get the config state; check if lock is already granted
     * based on the current config state
     */
    res = cfg_ok_to_unlock(cfg, SES_MY_SID(scb));
    if (res == NO_ERR) {
        msg->rpc_user1 = (void *)cfg;
    } else {
        agt_record_error(scb,
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION,
                         res, 
                         methnode,
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE,
                         NULL);
    }

    return res;

} /* unlock_validate */


/********************************************************************
* FUNCTION unlock_invoke
*
* unlock : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    unlock_invoke (ses_cb_t *scb,
                   rpc_msg_t *msg,
                   xml_node_t *methnode)
{
    cfg_template_t  *cfg;
    status_t         res;

    cfg = (cfg_template_t *)msg->rpc_user1;
    res = cfg_unlock(cfg, SES_MY_SID(scb));
    if (res != NO_ERR) {
        agt_record_error(scb,
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION,
                         res, 
                         methnode,
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE,
                         NULL);
    }
    return res;

} /* unlock_invoke */


/********************************************************************
* FUNCTION close_session_invoke
*
* close-session : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    close_session_invoke (ses_cb_t *scb,
                          rpc_msg_t *msg,
                          xml_node_t *methnode)
{

    (void)msg;
    (void)methnode;
    agt_ses_request_close(SES_MY_SID(scb), 
                          SES_MY_SID(scb),
                          SES_TR_CLOSED);
    return NO_ERR;

} /* close_session_invoke */


/********************************************************************
* FUNCTION kill_session_validate
*
* kill-session : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    kill_session_validate (ses_cb_t *scb,
                           rpc_msg_t *msg,
                           xml_node_t *methnode)
{
    status_t         res;
    val_value_t     *val;

    res = NO_ERR;

    /* get the session-id parameter */
    val = val_find_child(msg->rpc_input, 
                         NC_MODULE,
                         NCX_EL_SESSION_ID);
    if (!val || val->res != NO_ERR) {
        /* error already recorded in parse phase */
        if (val) {
            return val->res;
        } else {
            return ERR_NCX_OPERATION_FAILED;
        }
    }

    /* make sure the session-id is valid 
     * The RFC forces a kill-session of the current
     * session to be an error, even though agt_ses.c
     * supports this corner-case
     */
    if (VAL_UINT(val) == scb->sid
        || !agt_ses_session_id_valid(VAL_UINT(val))) {
        res = ERR_NCX_INVALID_VALUE;
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res,
                         methnode, 
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
    }

    return res;

}  /* kill_session_validate */


/********************************************************************
* FUNCTION kill_session_invoke
*
* kill-session : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    kill_session_invoke (ses_cb_t *scb,
                         rpc_msg_t *msg,
                         xml_node_t *methnode)
{
    val_value_t *val;
    status_t     res;

    /* get the session-id parameter */
    val = val_find_child(msg->rpc_input, 
                         NC_MODULE,
                         NCX_EL_SESSION_ID);
    if (!val || val->res != NO_ERR) {
        /* error already recorded in parse phase */

        if (val) {
            res = val->res;
        } else {
            res = ERR_NCX_OPERATION_FAILED;
        }

        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res,
                         methnode, 
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
    }

    agt_ses_kill_session((ses_id_t)VAL_UINT(val), 
                         scb->sid,
                         SES_TR_KILLED);
    return NO_ERR;

} /* kill_session_invoke */


/********************************************************************
* FUNCTION validate_validate
*
* validate : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    validate_validate (ses_cb_t *scb,
                       rpc_msg_t *msg,
                       xml_node_t *methnode)
{
    cfg_template_t       *target;
    val_value_t          *val, *child, *rootval;
    const xmlChar        *errstr;
    const agt_profile_t  *profile;
    status_t              res;
    boolean               needfullcheck;

    target = NULL;
    res = NO_ERR;
    rootval = NULL;
    errstr = NULL;
    child = NULL;
    needfullcheck = FALSE;
    profile = agt_get_profile();

    if (!profile->agt_usevalidate) {
        res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

    if (res == NO_ERR) {
        /* get the source parameter */
        val = val_find_child(msg->rpc_input, 
                             NC_MODULE,
                             NCX_EL_SOURCE);
        if (!val || val->res != NO_ERR) {
            if (val) {
                res = val->res;
            } else {
                res = ERR_NCX_OPERATION_FAILED;
            }
            errstr = NCX_EL_SOURCE;
        }
    }

    /* determine which variant of the input parameter is present */
    if (res == NO_ERR) {
        child = val_get_first_child(val);
        if (!child || child->res != NO_ERR) {
            if (child) {
                res = child->res;
                errstr = child->name;
            } else {
                res = ERR_NCX_OPERATION_FAILED;
                errstr = NCX_EL_SOURCE;
            }
        }
    }

    if (res == NO_ERR) {
        if (!xml_strcmp(child->name, NCX_EL_RUNNING)) {
            target = cfg_get_config_id(NCX_CFGID_RUNNING);
        } else if (!xml_strcmp(child->name, NCX_EL_CANDIDATE)) {
            if (profile->agt_targ != NCX_AGT_TARG_CANDIDATE) {
                res = ERR_NCX_OPERATION_NOT_SUPPORTED;
                errstr = child->name;
            } else {
                target = cfg_get_config_id(NCX_CFGID_CANDIDATE);
            }
        } else if (!xml_strcmp(child->name, NCX_EL_STARTUP)) {
            if (!profile->agt_has_startup) {
                res = ERR_NCX_OPERATION_NOT_SUPPORTED;
                errstr = child->name;
            } else {
                target = cfg_get_config_id(NCX_CFGID_CANDIDATE);
            }
        } else if (!xml_strcmp(child->name, NCX_EL_URL)) {
            res = ERR_NCX_OPERATION_NOT_SUPPORTED;
            errstr = child->name;
        } else if (!xml_strcmp(child->name, NCX_EL_CONFIG)) {
            needfullcheck = TRUE;
            rootval = child;
        }

        if (res == NO_ERR && !needfullcheck) {
            if (target == NULL || target->root == NULL) {
                res = ERR_NCX_OPERATION_FAILED;
                errstr = child->name;
            } else {
                rootval = target->root;
            }
        }
    }

    if (res != NO_ERR) {
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res, 
                         methnode,
                         (errstr) ? NCX_NT_STRING : NCX_NT_NONE,
                         (errstr) ? errstr : NULL,
                         (rootval) ? NCX_NT_VAL : NCX_NT_NONE, 
                         (rootval) ? rootval : NULL);
        return res;
    }

    /* set the error parameter to gather the most errors */
    msg->rpc_err_option = OP_ERROP_CONTINUE;

    if (needfullcheck) {
        res = agt_val_validate_write(scb,
                                     msg, 
                                     NULL, 
                                     rootval, 
                                     OP_EDITOP_LOAD);
    }

    if (res == NO_ERR) {
        res = agt_val_root_check(scb, msg, rootval);
    }

    return res;

} /* validate_validate */


/********************************************************************
* FUNCTION commit_validate
*
* commit : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    commit_validate (ses_cb_t *scb,
                     rpc_msg_t *msg,
                     xml_node_t *methnode)
{
    cfg_template_t       *candidate, *running;
    const agt_profile_t  *profile;
    status_t              res;
    boolean               errdone;


    res = NO_ERR;
    errdone = FALSE;

    profile = agt_get_profile();

    if (profile->agt_targ != NCX_AGT_TARG_CANDIDATE) {
        res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    } else {
        /* get the candidate config */
        candidate = cfg_get_config_id(NCX_CFGID_CANDIDATE);
        running = cfg_get_config_id(NCX_CFGID_RUNNING);
        if (!candidate || !running) {
            res = SET_ERROR(ERR_INTERNAL_VAL);
        } else {

            /* check if this session is allowed to clear the
             * candidate config now
             */
            res = cfg_ok_to_write(candidate, SES_MY_SID(scb));

            if (res == NO_ERR) {
                /* check if the running config can be written */
                res = cfg_ok_to_write(running, SES_MY_SID(scb));
            }

            if (res == NO_ERR) {
                res = agt_val_root_check(scb, 
                                         msg, 
                                         candidate->root);
                if (res != NO_ERR) {
                    errdone = TRUE;
                }
            }
        }
    }

    if (res != NO_ERR && !errdone) {
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res, 
                         methnode,
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
    }

    return res;

} /* commit_validate */


/********************************************************************
* FUNCTION write_config
*
* Write the specified cfg->root to the the default backup source
*
* INPUTS:
*    filespec == complete path for the output file
*    cfg == config template to write to XML file
* 
* RETURNS:
*    status
*********************************************************************/
static status_t
    write_config (const xmlChar *filespec,
                  cfg_template_t *cfg)
{
    agt_profile_t     *profile;
    status_t           res;
    xml_attrs_t        attrs;

    if (cfg->root == NULL) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    profile = agt_get_profile();

    /* write the new startup config */
    xml_init_attrs(&attrs);

    /* output to the specified file or STDOUT */
    res = xml_wr_check_file(filespec, 
                            cfg->root, 
                            &attrs, 
                            XMLMODE, 
                            WITHHDR, 
                            0,
                            profile->agt_indent,
                            agt_check_save);

    xml_clean_attrs(&attrs);

    return res;

} /* write_config */


/********************************************************************
* FUNCTION commit_invoke
*
* commit : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    commit_invoke (ses_cb_t *scb,
                   rpc_msg_t *msg,
                   xml_node_t *methnode)
{
    val_value_t    *confirmedval, *timeoutval;
    cfg_template_t *candidate, *running;
    xmlChar        *fname;
    status_t        res;
    boolean         save_nvstore, errdone;

    res = NO_ERR;
    errdone = FALSE;

    candidate = cfg_get_config_id(NCX_CFGID_CANDIDATE);
    running = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!candidate || !running) {
        res = SET_ERROR(ERR_INTERNAL_VAL);
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION,
                         res,
                         methnode,
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE,
                         NULL);
        return res;
    }

    save_nvstore = TRUE;

    /* get the confirmed parameter */
    confirmedval = val_find_child(msg->rpc_input,
                                  val_get_mod_name(msg->rpc_input),
                                  NCX_EL_CONFIRMED);

    /* get the confirm-timeout parameter */
    timeoutval = val_find_child(msg->rpc_input,
                                val_get_mod_name(msg->rpc_input),
                                NCX_EL_CONFIRM_TIMEOUT);

    /* figure out what to do wrt/ confirmed-commit */
    if (commit_cb.cc_active) {
        /* confirmed-commit already active
         * see if this commit is finishing the
         * confirmed commit or extending the timer
         * and perhaps adding more data to running
         */
        if (confirmedval) {
            /* check same session that started cc */
            if (commit_cb.cc_ses_id != SES_MY_SID(scb)) {
                log_warn("\nWarning: session %u extending "
                         "confirmed-commit started by session %u",
                         SES_MY_SID(scb),
                         commit_cb.cc_ses_id);
            }
            
            /* extend the timer */
            (void)time(&commit_cb.cc_start_time);
            if (timeoutval) {
                commit_cb.cc_cancel_timeout = 
                    VAL_UINT(timeoutval);
            } else {
                commit_cb.cc_cancel_timeout = 
                    NCX_DEF_CONFIRM_TIMEOUT;
            }
            if (LOGDEBUG2) {
                log_debug2("\nConfirmed commit timer extended "
                           "by %u seconds",
                           commit_cb.cc_cancel_timeout);
            }
            save_nvstore = FALSE;
            agt_sys_send_sysConfirmedCommit(scb,
                                            NCX_CC_EVENT_EXTEND);
        } else {
            /* check same session that started cc */
            if (commit_cb.cc_ses_id != SES_MY_SID(scb)) {
                log_warn("\nWarning: session %u completing "
                         "confirmed-commit started by session %u",
                         SES_MY_SID(scb),
                         commit_cb.cc_ses_id);
            } else if (LOGDEBUG2) {
                log_debug2("\nConfirmed commit completed");
            }

            /* finish the confirmed-commit */
            res = agt_ncx_cfg_save(running, FALSE);
            if (res != NO_ERR) {
                errdone = FALSE;
            }
            agt_sys_send_sysConfirmedCommit(scb,
                                            NCX_CC_EVENT_COMPLETE);
            commit_cb.cc_active = FALSE;
            commit_cb.cc_ses_id = 0;
        }
    } else {
        /* check if a new confirmed commit is starting */
        if (confirmedval) {
            /* save the session ID that started this conf-commit */
            commit_cb.cc_ses_id = SES_MY_SID(scb);

            /* set the timer */
            (void)time(&commit_cb.cc_start_time);
            if (timeoutval) {
                commit_cb.cc_cancel_timeout = 
                    VAL_UINT(timeoutval);
            } else {
                commit_cb.cc_cancel_timeout = 
                    NCX_DEF_CONFIRM_TIMEOUT;
            }
            commit_cb.cc_active = TRUE;
            save_nvstore = FALSE;

            if (LOGDEBUG2) {
                log_debug2("\nConfirmed commit started, timeout in "
                           "%u seconds",
                           commit_cb.cc_cancel_timeout);
            }
            agt_sys_send_sysConfirmedCommit(scb,
                                            NCX_CC_EVENT_START);
        } else {
            /* no confirmed commit is starting */
            save_nvstore = TRUE;
        }
    }

    /* make a backup of running to make sure
     * that if this step fails, running config MUST not change
     */ 
    if (commit_cb.cc_backup_source) {
        /* rewrite the existing backup file */
        ;
    } else {
        /* search for the default startup-cfg.xml filename */
        fname = ncxmod_find_data_file(NCX_DEF_BACKUP_FILE, 
                                      FALSE, 
                                      &res);
        if (fname) {
            /* rewrite the existing backup file
             * hand off fname malloced memory here 
             */
            commit_cb.cc_backup_source = fname;
        } else if (running->src_url) {
            /* use the same path as the startup config file
             * if it has already been set; it may be different
             * each boot via the --startup CLI parameter
             */
            res = NO_ERR;
            commit_cb.cc_backup_source = 
                ncxmod_make_data_filespec_from_src(running->src_url,
                                                   NCX_DEF_BACKUP_FILE,
                                                   &res);
        } else {
            /* create a new backup file name
             * should really check the res code
             * to make sure the error is some sort
             * of not-found error, but if it was a fatal
             * malloc error, etc. then the following
             * attempt to create a backup will probably 
             * fail as well
             */
            res = NO_ERR;
            commit_cb.cc_backup_source = 
                ncxmod_make_data_filespec(NCX_DEF_BACKUP_FILE,
                                          &res);
        }
    }

    /* attempt to save the backup, if a file name is available */
    if (res == NO_ERR) {
        res = write_config(commit_cb.cc_backup_source, running);
    }

    if (res == NO_ERR) {
        res = agt_val_apply_commit(scb,
                                   msg, 
                                   candidate, 
                                   running,
                                   save_nvstore);
        if (res != NO_ERR) {
            errdone = TRUE;

            /* restore the config if needed */
            res = agt_ncx_load_backup(commit_cb.cc_backup_source,
                                      running,
                                      commit_cb.cc_ses_id);
            if (res != NO_ERR) {
                errdone = FALSE;
            }
        } else {
            res = cfg_fill_candidate_from_running();
        }
    }

    if (res != NO_ERR && !errdone) {
        agt_record_error(scb,
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION,
                         res,
                         methnode,
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_VAL,
                         candidate->root);
    }

    return res;

} /* commit_invoke */


/********************************************************************
* FUNCTION discard_changes_validate
*
* discard-changes : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    discard_changes_validate (ses_cb_t *scb,
                              rpc_msg_t *msg,
                              xml_node_t *methnode)
{
    cfg_template_t       *candidate;
    const agt_profile_t  *profile;
    status_t              res;

    res = NO_ERR;
    profile = agt_get_profile();

    if (profile->agt_targ != NCX_AGT_TARG_CANDIDATE) {
        res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    } else {
        /* get the candidate config */
        candidate = cfg_get_config_id(NCX_CFGID_CANDIDATE);
        if (!candidate) {
            res = SET_ERROR(ERR_INTERNAL_VAL);
        } else {
            /* check if this session is allowed to invoke now */
            res = cfg_ok_to_write(candidate, SES_MY_SID(scb));
        }
    }

    if (res != NO_ERR) {
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res, 
                         methnode,
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
        return res;
    }

    return res;

} /* discard_changes_validate */


/********************************************************************
* FUNCTION discard_changes_invoke
*
* discard-changes : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    discard_changes_invoke (ses_cb_t *scb,
                           rpc_msg_t *msg,
                           xml_node_t *methnode)
{
    cfg_template_t *candidate;
    status_t        res;

    res = NO_ERR;


    /* get the candidate config */
    candidate = cfg_get_config_id(NCX_CFGID_CANDIDATE);
    if (!candidate) {
        res = SET_ERROR(ERR_INTERNAL_VAL);
    } else if (cfg_get_dirty_flag(candidate)) {
        res = cfg_fill_candidate_from_running();
    }

    if (res != NO_ERR) {
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res, 
                         methnode,
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
    }

    return res;

} /* discard_changes_invoke */


/********************************************************************
* FUNCTION load_config_validate
*
* load-config : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    load_config_validate (ses_cb_t *scb,
                          rpc_msg_t *msg,
                          xml_node_t *methnode)
{
    cfg_template_t  *target;
    val_value_t     *val;
    status_t         res;

    /* This special callback is used by internal NCX functions
     * to load the initial configuration.  The msg->rpc_user1 parameter 
     * has already been set to the address of the cfg_template_t
     * to fill in.
     *
     * NOTE: HACK DEPENDS ON THE agt_rpc_load_config_file to setup
     * the rpc->rpc_user1 parameter
     *
     * make sure this is a DUMMY session, not a real session
     */
    if (scb->type != SES_TYP_DUMMY) {
        res = ERR_NCX_ACCESS_DENIED;
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res,
                         methnode, 
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
        return res;
    }

    target = (cfg_template_t *)msg->rpc_user1;
    if (!target) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* get the <nc:config> parameter */
    val = val_find_child(msg->rpc_input, 
                         NC_MODULE, 
                         NCX_EL_CONFIG);
    if (!val) {
        /* we shouldn't get here if the config param is missing */
        return SET_ERROR(ERR_NCX_OPERATION_FAILED);
    }

    /* errors will be added as needed */
    res = agt_val_validate_write(scb, 
                                 msg, 
                                 target, 
                                 val, 
                                 msg->rpc_top_editop);

    if (res == NO_ERR) {
        res = agt_val_root_check(scb, msg, val);
    }
    msg->rpc_user2 = val;

    return res;

} /* load_config_validate */


/********************************************************************
* FUNCTION load_config_invoke
*
* load-config : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    load_config_invoke (ses_cb_t *scb,
                        rpc_msg_t *msg,
                        xml_node_t *methnode)
{
    cfg_template_t  *target;
    val_value_t     *val;
    status_t         res;

    if (scb->type != SES_TYP_DUMMY) {
        res = ERR_NCX_ACCESS_DENIED;
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res,
                         methnode, 
                         NCX_NT_NONE, 
                         NULL, 
                         NCX_NT_NONE, 
                         NULL);
        return res;
    }

    /* This special callback is used by internal NCX functions
     * to load the initial configuration. 
     */

    res = NO_ERR;
    target = (cfg_template_t *)msg->rpc_user1;
    val = (val_value_t *)msg->rpc_user2;

    if (!target || !val) {
        res = SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* load the <config> into the target config */
    if (res == NO_ERR) {
        res = agt_val_apply_write(scb, 
                                  msg, 
                                  target, 
                                  val, 
                                  OP_EDITOP_LOAD);
    }

    val_clean_tree(target->root);

    return res;

} /* load_config_invoke */


/********************************************************************
* FUNCTION load_invoke
*
* load module : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    load_invoke (ses_cb_t *scb,
                 rpc_msg_t *msg,
                 xml_node_t *methnode)
{
    val_value_t           *modval, *revval, *devval, *newval;
    ncx_module_t          *mod, *testmod;
    xmlChar               *moduri;
    agt_profile_t         *agt_profile;
    status_t               res;
    boolean                module_added, errdone;

    res = NO_ERR;
    newval = NULL;
    mod = NULL;
    errdone = FALSE;
    module_added = FALSE;
    agt_profile = agt_get_profile();

    /* mandatory module name */
    modval = val_find_child(msg->rpc_input, 
                            AGT_SYS_MODULE,
                            NCX_EL_MODULE);
    if (!modval || modval->res != NO_ERR) {
        /* error already recorded */
        return ERR_NCX_OPERATION_FAILED;
    }

    /* optional revision data string */
    revval = val_find_child(msg->rpc_input, 
                            AGT_SYS_MODULE,
                            NCX_EL_REVISION);
    if (revval && revval->res != NO_ERR) {
        /* error already recorded */
        return ERR_NCX_OPERATION_FAILED;
    }

    /* check for any version of this module already loaded */
    mod = ncx_find_module(VAL_STR(modval), NULL);
    if (!mod) {
        for (devval = val_find_child(msg->rpc_input, 
                                     AGT_SYS_MODULE,
                                     NCX_EL_DEVIATION);
             devval != NULL && res == NO_ERR;
             devval = val_find_next_child(msg->rpc_input, 
                                          AGT_SYS_MODULE,
                                          NCX_EL_DEVIATION,
                                          devval)) {

            res = ncxmod_load_deviation(VAL_STR(devval),
                                        &agt_profile->agt_savedevQ);
            if (res != NO_ERR) {
                agt_record_error(scb, 
                                 &msg->mhdr, 
                                 NCX_LAYER_OPERATION, 
                                 res,
                                 methnode, 
                                 NCX_NT_NONE, 
                                 NULL, 
                                 NCX_NT_VAL, 
                                 devval);
                errdone = TRUE;
            }
        }


        if (res == NO_ERR) {
#ifdef STATIC_SERVER
            res = ncxmod_load_module(VAL_STR(modval), 
                                     (revval) ? VAL_STR(revval) : NULL, 
                                     &agt_profile->agt_savedevQ,
                                     &mod);
#else
            res = agt_load_sil_code(VAL_STR(modval), 
                                    (revval) ? VAL_STR(revval) : NULL,
                                    TRUE);
            if (res == ERR_NCX_SKIPPED) {
                log_warn("\nWarning: SIL code for module '%s' not found",
                         VAL_STR(modval));
                res = ncxmod_load_module(VAL_STR(modval), 
                                         (revval) ? VAL_STR(revval) : NULL, 
                                         &agt_profile->agt_savedevQ,
                                         &mod);
            }
            if (res == NO_ERR) {
                mod = ncx_find_module(VAL_STR(modval),
                                      (revval) ? VAL_STR(revval) : NULL);
                if (mod == NULL) {
                    res = SET_ERROR(ERR_INTERNAL_VAL);
                }
            }
#endif
            if (res == NO_ERR) {
                module_added = TRUE;
            } else {
                agt_record_error(scb, 
                                 &msg->mhdr, 
                                 NCX_LAYER_OPERATION, 
                                 res,
                                 methnode, 
                                 NCX_NT_NONE, 
                                 NULL, 
                                 NCX_NT_VAL, 
                                 modval);
                errdone = TRUE;
            }
        }
    } else if (revval) {
        /* some version of the module is already loaded
         * try again to get the exact version requested 
         */
        testmod = ncx_find_module(VAL_STR(modval),
                                  VAL_STR(revval));
        if (testmod) {
            mod = testmod;
        } else {
            res = ERR_NCX_WRONG_VERSION;
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_OPERATION, 
                             res,
                             methnode, 
                             NCX_NT_NONE, 
                             NULL, 
                             NCX_NT_VAL, 
                             revval);
            errdone = TRUE;
        }
    }

    /* generate the return value */
    if (res == NO_ERR && mod) {
        newval = val_make_string(val_get_nsid(modval),
                                 NCX_EL_MOD_REVISION,
                                 (mod->version) ? 
                                 mod->version : EMPTY_STRING);
        if (newval == NULL) {
            res = ERR_INTERNAL_MEM;
        }
    }

    if (res == NO_ERR && mod && module_added) {
        res = agt_state_add_module_schema(mod);
    }

    if (res == NO_ERR && mod && module_added) {
        res = agt_cap_add_module(mod);
    }

    if (res == NO_ERR && mod && module_added) {
        moduri = cap_make_moduri(mod);
        if (!moduri) {
            res = ERR_INTERNAL_MEM;
        } else {
            agt_sys_send_sysCapabilityChange(scb, 
                                             TRUE,
                                             moduri);
            m__free(moduri);
        }
    }

    if (res != NO_ERR) {
        if (!errdone) {
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_OPERATION, 
                             res,
                             methnode, 
                             NCX_NT_NONE, 
                             NULL, 
                             NCX_NT_VAL, 
                             modval);
        }
        if (newval != NULL) {
            val_free_value(newval);
        }
    } else {
        /* pass off newval memory here */
        if (newval != NULL) {
            msg->rpc_data_type = RPC_DATA_YANG;
            dlq_enque(newval, &msg->rpc_dataQ);
        }
    }

    return res;

} /* load_invoke */


/********************************************************************
* FUNCTION restart_invoke
*
* restart : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    restart_invoke (ses_cb_t *scb,
                    rpc_msg_t *msg,
                    xml_node_t *methnode)
{
    xmlChar   timebuff[TSTAMP_MIN_SIZE];

    (void)msg;
    (void)methnode;

    tstamp_datetime(timebuff);

    log_write("\n\n**************"
              "\nNotice: restart requested\n   by %s "
              "on session %u at %s\n\n",
              scb->username,
              scb->sid,
              timebuff);

    agt_request_shutdown(NCX_SHUT_RESTART);
    return NO_ERR;

} /* restart_invoke */


/********************************************************************
* FUNCTION shutdown_invoke
*
* shutdown : invoke callback
* 
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    shutdown_invoke (ses_cb_t *scb,
                     rpc_msg_t *msg,
                     xml_node_t *methnode)
{
    xmlChar   timebuff[TSTAMP_MIN_SIZE];

    (void)msg;
    (void)methnode;

    tstamp_datetime(timebuff);

    log_write("\n\n*****************************"
              "\nNotice: shutdown requested\n    by %s "
              "on session %u at %s\n\n",
              scb->username,
              scb->sid,
              timebuff);

    agt_request_shutdown(NCX_SHUT_EXIT);
    return NO_ERR;

} /* shutdown_invoke */


/********************************************************************
* FUNCTION register_nc_callbacks
*
* Register the agent callback functions for the NETCONF RPC methods 
*
* RETURNS:
*    status, NO_ERR if all registered okay
*********************************************************************/
static status_t 
    register_nc_callbacks (void)
{
    status_t  res;

    /* get */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_GET),
                                  AGT_RPC_PH_VALIDATE,
                                  get_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    /* get-config */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_GET_CONFIG),
                                  AGT_RPC_PH_VALIDATE,
                                  get_config_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* edit-config */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_EDIT_CONFIG),
                                  AGT_RPC_PH_VALIDATE,
                                  edit_config_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_EDIT_CONFIG),
                                  AGT_RPC_PH_INVOKE,
                                  edit_config_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    /* copy-config */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_COPY_CONFIG),
                                  AGT_RPC_PH_VALIDATE,
                                  copy_config_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_COPY_CONFIG),
                                  AGT_RPC_PH_INVOKE,
                                  copy_config_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* delete-config */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_DELETE_CONFIG),
                                  AGT_RPC_PH_VALIDATE,
                                  delete_config_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_DELETE_CONFIG),
                                  AGT_RPC_PH_INVOKE,
                                  delete_config_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* lock */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_LOCK),
                                  AGT_RPC_PH_VALIDATE,
                                  lock_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_LOCK),
                                  AGT_RPC_PH_INVOKE,
                                  lock_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }


    /* unlock */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_UNLOCK),
                                  AGT_RPC_PH_VALIDATE,
                                  unlock_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_UNLOCK),
                                  AGT_RPC_PH_INVOKE,
                                  unlock_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* close-session
     * no validate for close-session 
     */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_CLOSE_SESSION),
                                  AGT_RPC_PH_INVOKE,
                                  close_session_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* kill-session */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_KILL_SESSION),
                                  AGT_RPC_PH_VALIDATE,
                                  kill_session_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_KILL_SESSION),
                                  AGT_RPC_PH_INVOKE,
                                  kill_session_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* validate :validate capability */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_VALIDATE),
                                  AGT_RPC_PH_VALIDATE,
                                  validate_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* commit :candidate capability */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_COMMIT),
                                  AGT_RPC_PH_VALIDATE,
                                  commit_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_COMMIT),
                                  AGT_RPC_PH_INVOKE,
                                  commit_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* discard-changes :candidate capability */
    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_DISCARD_CHANGES),
                                  AGT_RPC_PH_VALIDATE,
                                  discard_changes_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE,
                                  op_method_name(OP_DISCARD_CHANGES),
                                  AGT_RPC_PH_INVOKE,
                                  discard_changes_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* load-config extension */
    res = agt_rpc_register_method(NC_MODULE, 
                                  NCX_EL_LOAD_CONFIG,
                                  AGT_RPC_PH_VALIDATE, 
                                  load_config_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    res = agt_rpc_register_method(NC_MODULE, 
                                  NCX_EL_LOAD_CONFIG,
                                  AGT_RPC_PH_INVOKE,
                                  load_config_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* load module extension */
    res = agt_rpc_register_method(AGT_SYS_MODULE, 
                                  NCX_EL_LOAD,
                                  AGT_RPC_PH_INVOKE,  
                                  load_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* restart extension */
    res = agt_rpc_register_method(AGT_SYS_MODULE, 
                                  NCX_EL_RESTART,
                                  AGT_RPC_PH_INVOKE,  
                                  restart_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* shutdown extension */
    res = agt_rpc_register_method(AGT_SYS_MODULE, 
                                  NCX_EL_SHUTDOWN,
                                  AGT_RPC_PH_INVOKE,  
                                  shutdown_invoke);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* no-op extension */
    agt_rpc_support_method(AGT_SYS_MODULE, NCX_EL_NO_OP);

    return NO_ERR;

} /* register_nc_callbacks */


/********************************************************************
* FUNCTION unregister_nc_callbacks
*
* Unregister the agent callback functions for the NETCONF RPC methods 
*
*********************************************************************/
static void
    unregister_nc_callbacks (void)
{
    /* get */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_GET));

    /* get-config */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_GET_CONFIG));

    /* edit-config */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_EDIT_CONFIG));

    /* copy-config */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_COPY_CONFIG));

    /* delete-config */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_DELETE_CONFIG));

    /* lock */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_LOCK));

    /* unlock */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_UNLOCK));

    /* close-session */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_CLOSE_SESSION));

    /* kill-session */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_KILL_SESSION));

    /* validate */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_VALIDATE));

    /* commit */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_COMMIT));

    /* discard-changes */
    agt_rpc_unregister_method(NC_MODULE, 
                              op_method_name(OP_DISCARD_CHANGES));

    /* load-config extension */
    agt_rpc_unregister_method(NC_MODULE, NCX_EL_LOAD_CONFIG);

    /* load module extension */
    agt_rpc_unregister_method(AGT_SYS_MODULE, NCX_EL_LOAD);

    /* restart extension */
    agt_rpc_unregister_method(AGT_SYS_MODULE, NCX_EL_RESTART);

    /* shutdown extension */
    agt_rpc_unregister_method(AGT_SYS_MODULE, NCX_EL_SHUTDOWN);

    /* no-op extension */
    agt_rpc_unsupport_method(AGT_SYS_MODULE, NCX_EL_NO_OP);

} /* unregister_nc_callbacks */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_ncx_init
* 
* Initialize the NCX Agent standard method routines
* 
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    agt_ncx_init (void)
{
    status_t  res;

    if (!agt_ncx_init_done) {

        res = register_nc_callbacks();
        if (res != NO_ERR) {
            unregister_nc_callbacks();
            return res;
        }

        memset(&commit_cb, 0x0, sizeof(commit_cb_t));

        agt_ncx_init_done = TRUE;
    }
    return NO_ERR;

}  /* agt_ncx_init */


/********************************************************************
* FUNCTION agt_ncx_cleanup
*
* Cleanup the NCX Agent standard method routines
* 
* TBD -- put platform-specific agent cleanup here
*
*********************************************************************/
void
    agt_ncx_cleanup (void)
{
    if (agt_ncx_init_done) {

        unregister_nc_callbacks();

        if (commit_cb.cc_backup_source) {
            m__free(commit_cb.cc_backup_source);
        }
        memset(&commit_cb, 0x0, sizeof(commit_cb_t));

        agt_ncx_init_done = FALSE;
    }
}   /* agt_ncx_cleanup */


/********************************************************************
* FUNCTION agt_ncx_cfg_load
*
* Load the specifed config from the indicated source
* Called just once from agt.c at boot or reload time!
*
* This function should only be used to load an empty config
* in CFG_ST_INIT state
*
* INPUTS:
*    cfg = Config template to load data into
*    cfgloc == enum for the config source location
*    cfgparm == string parameter used in different ways 
*               depending on the cfgloc value
*     For cfgloc==CFG_LOC_FILE, this is a system-dependent filespec
* 
* OUTPUTS:
*    errQ contains any rpc_err_rec_t structs (if non-NULL)
*
* RETURNS:
*    overall status; may be the last of multiple error conditions
*********************************************************************/
status_t
    agt_ncx_cfg_load (cfg_template_t *cfg,
                      cfg_location_t cfgloc,
                      const xmlChar *cfgparm)
{
    cfg_template_t  *startup;
    val_value_t     *copystartup;
    status_t         res;

#ifdef DEBUG
    if (!cfg) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (cfg->cfg_state != CFG_ST_INIT) {
        return SET_ERROR(ERR_NCX_CFG_STATE);
    }
#endif

    startup = NULL;
    copystartup = NULL;

    cfg->cfg_loc = cfgloc;
    if (cfgparm) {
        cfg->src_url = xml_strdup(cfgparm);
        if (!cfg->src_url) {
            return ERR_INTERNAL_MEM;
        }
    }

    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    switch (cfgloc) {
    case CFG_LOC_INTERNAL:
        break;
    case CFG_LOC_FILE:
        if (!cfg->src_url) {
            res = ERR_INTERNAL_MEM;
        } else {
            /* the cfgparm should be a filespec of an XML config file */
            res = agt_rpc_load_config_file(cfgparm, cfg, TRUE, 0);
            if (res == NO_ERR && 
                cfg->root != NULL &&
                cfg->cfg_id != NCX_CFGID_STARTUP) {
                startup = cfg_get_config_id(NCX_CFGID_STARTUP);
                if (startup != NULL) {
                    copystartup = val_clone(cfg->root);
                    if (copystartup == NULL) {
                        log_error("\nError: create <startup> config failed");
                    } else {
                        if (startup->root != NULL) {
                            val_free_value(startup->root);
                        }
                        startup->root = copystartup;
                        copystartup = NULL;
                    }
                }
            }
        }
        break;
    case CFG_LOC_NAMED:
        break;
    case CFG_LOC_LOCAL_URL:
        break;
    case CFG_LOC_REMOTE_URL:
        break;
    default:
        res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

} /* agt_ncx_cfg_load */


/********************************************************************
* FUNCTION agt_ncx_cfg_save
*
* Save the specified cfg to the its startup source, which should
* be stored in the cfg struct
*
* INPUTS:
*    cfg  = Config template to save from
*    bkup = TRUE if the current startup config should
*           be saved before it is overwritten
*         = FALSE to just overwrite the old startup cfg
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_ncx_cfg_save (cfg_template_t *cfg,
                      boolean bkup)
{
    cfg_template_t    *startup, *running;
    val_value_t       *copystartup;
    const xmlChar     *filename, *yumahome;
    xmlChar           *filebuffer;
    agt_profile_t     *profile;
    status_t           res;
    xml_attrs_t        attrs;

#ifdef DEBUG
    if (!cfg) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!cfg->root) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    filename = NULL;
    filebuffer = NULL;
    startup = NULL;
    copystartup = NULL;
    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    profile = agt_get_profile();

    switch (cfg->cfg_loc) {
    case CFG_LOC_INTERNAL:
        break;
    case CFG_LOC_NONE:   /* candidate config */
    case CFG_LOC_FILE:
        if (bkup) {
            /* remove any existing backup */
            /****/

            /* rename the current startup to the backup */
            /****/
        } 

        /* save the new startup database, if there is one */
        res = NO_ERR;
        running = cfg_get_config_id(NCX_CFGID_RUNNING);
        if (running == NULL) {
            SET_ERROR(ERR_INTERNAL_VAL);
        }
        startup = cfg_get_config_id(NCX_CFGID_STARTUP);
        if (startup != NULL) {
            copystartup = val_clone_config_data(cfg->root, &res);
            if (copystartup == NULL) {
                return res;
            }
        }

        if (res == NO_ERR) {
            yumahome = ncxmod_get_yuma_home();

            /* get the right filespec to use
             *
             * 1) use the existing filespec
             * 2) use the startup filespec
             * 3) use the running filespec
             * 4) use $YUMA_HOME/data/startup-cfg.xml
             * 5) use $HOME/.yuma/startup-cfg.xml
             */
            if (cfg->src_url != NULL) {
                filename = cfg->src_url;
            } else if (startup && startup->src_url) {
                filename = startup->src_url;
            } else if (running && running->src_url) {
                filename = running->src_url;
            } else if (yumahome != NULL) {
                filebuffer = 
                    ncx_get_source(NCX_YUMA_HOME_STARTUP_FILE,
                                   &res);
                filename = filebuffer;
            } else {
                filebuffer = 
                    ncx_get_source(NCX_DOT_YUMA_STARTUP_FILE,
                                   &res);
                filename = filebuffer;
            }
            if (res == NO_ERR) {
                if (LOGDEBUG) {
                    log_debug("\nWriting <%s> config to file '%s'",
                              cfg->name,
                              filename);
                }
                /* write the new startup config */
                xml_init_attrs(&attrs);

                /* output to the specified file or STDOUT */
                res = xml_wr_check_file(filename,
                                        cfg->root, 
                                        &attrs,
                                        XMLMODE, 
                                        WITHHDR, 
                                        0,
                                        profile->agt_indent,
                                        agt_check_save);

                xml_clean_attrs(&attrs);

                if (res == NO_ERR && copystartup != NULL) {
                    /* toss the old startup and save the new one */
                    if (startup->root) {
                        val_free_value(startup->root);
                    }
                    startup->root = copystartup;
                    copystartup = NULL;
                }
            }
        }
        break;
    case CFG_LOC_NAMED:
        break;
    case CFG_LOC_LOCAL_URL:
        break;
    case CFG_LOC_REMOTE_URL:
        break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (copystartup) {
        val_free_value(copystartup);
    }

    if (filebuffer) {
        m__free(filebuffer);
    }

    return res;

} /* agt_ncx_cfg_save */


/********************************************************************
* FUNCTION agt_ncx_cfg_save_inline
*
* Save the specified cfg to the its startup source, which should
* be stored in the cfg struct
*
* INPUTS:
*    source_url == filespec where to save newroot 
*    newroot == value root to save
*
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_ncx_cfg_save_inline (const xmlChar *source_url,
                             val_value_t *newroot)
{
    agt_profile_t     *profile;
    cfg_template_t    *startup;
    val_value_t       *copystartup;
    status_t           res;
    xml_attrs_t        attrs;

#ifdef DEBUG
    if (!source_url || !newroot) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    startup = NULL;
    copystartup = NULL;
    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    profile = agt_get_profile();

    startup = cfg_get_config_id(NCX_CFGID_STARTUP);
    if (startup != NULL) {
        copystartup = val_clone_config_data(newroot, &res);
        if (copystartup == NULL) {
            return res;
        }
    }

    if (res == NO_ERR) {
        /* write the new startup config */
        xml_init_attrs(&attrs);

        /* output to the specified file or STDOUT */
        res = xml_wr_check_file(source_url, 
                                newroot, 
                                &attrs, 
                                XMLMODE, 
                                WITHHDR, 
                                0,
                                profile->agt_indent,
                                agt_check_save);

        xml_clean_attrs(&attrs);

        if (res == NO_ERR && copystartup != NULL) {
            /* toss the old startup and save the new one */
            if (startup->root) {
                val_free_value(startup->root);
            }
            startup->root = copystartup;
            copystartup = NULL;
        }
    }

    if (copystartup) {
        val_free_value(copystartup);
    }

    return res;

} /* agt_ncx_cfg_save_inline */


/********************************************************************
* FUNCTION agt_ncx_load_backup
*
* Load a backup config into the specified config template
*
* INPUTS:
*    filespec == complete path for the input file
*    cfg == config template to load
*    use_sid == session ID of user to use
*
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_ncx_load_backup (const xmlChar *filespec,
                         cfg_template_t *cfg,
                         ses_id_t  use_sid)
{
    status_t           res;

    res = agt_rpc_load_config_file(filespec, 
                                   cfg, 
                                   FALSE, 
                                   use_sid);
    return res;

} /* agt_ncx_load_backup */


/********************************************************************
* FUNCTION agt_ncx_cc_active
*
* Check if a confirmed-commit is active, and the timeout
* may need to be processed
*
* RETURNS:
*    TRUE if confirmed-commit is active
*    FALSE otherwise
*********************************************************************/
boolean
    agt_ncx_cc_active (void)
{

    return commit_cb.cc_active;

} /* agt_ncx_cc_active */


/********************************************************************
* FUNCTION agt_ncx_cc_ses_id
*
* Check if a confirmed-commit is active, and the timeout
* may need to be processed
*
* RETURNS:
*    TRUE if confirmed-commit is active
*    FALSE otherwise
*********************************************************************/
ses_id_t
    agt_ncx_cc_ses_id (void)
{

    return commit_cb.cc_ses_id;

} /* agt_ncx_cc_ses_id */


/********************************************************************
* FUNCTION agt_ncx_check_cc_timeout
*
* Check if a confirmed-commit has timed out, and needs to be canceled
*
*********************************************************************/
void
    agt_ncx_check_cc_timeout (void)
{

    time_t            timenow;
    double            timediff;

    if (!commit_cb.cc_active) {
        return;
    }

    (void)time(&timenow);
    timediff = difftime(timenow, commit_cb.cc_start_time);

    if (timediff >= (double)commit_cb.cc_cancel_timeout) {
        if (LOGDEBUG) {
            log_debug("\nConfirmed-commit timeout");
        }
        agt_ncx_cancel_confirmed_commit(NULL, NCX_CC_EVENT_TIMEOUT);
    }

} /* agt_ncx_check_cc_timeout */


/********************************************************************
* FUNCTION agt_ncx_cancel_confirmed_commit
*
* Cancel the confirmed-commit in progress and rollback
* to the backup-cfg.xml file
*
* INPUTS:
*   scb == session control block making this change, may be NULL
*   event == confirmEvent enumeration value to use
*
*********************************************************************/
void
    agt_ncx_cancel_confirmed_commit (ses_cb_t  *scb,
                                     ncx_confirm_event_t event)
{
    cfg_template_t  *running;
    status_t         res;

    if (!commit_cb.cc_active) {
        return;
    }

    running = cfg_get_config_id(NCX_CFGID_RUNNING);

    if (LOGDEBUG) {
        log_debug("\nConfirmed-commit canceled");
    }

    /* restore the config if needed */
    res = agt_ncx_load_backup(commit_cb.cc_backup_source,
                              running,
                              commit_cb.cc_ses_id);
    if (res != NO_ERR) {
        log_error("\nError: restore running config failed (%s)",
                  get_error_string(res));

    }

    agt_sys_send_sysConfirmedCommit(scb, event);

    commit_cb.cc_active = FALSE;
    commit_cb.cc_ses_id = 0;

} /* agt_ncx_cancel_confirmed_commit */



/* END file agt_ncx.c */
