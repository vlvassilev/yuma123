/*  FILE: agt_state.c

   NETCONF State Data Model implementation: Agent Side Support

From draft-07:

identifiers:
container /netconf-state
container /netconf-state/capabilities
leaf-list /netconf-state/capabilities/capability
container /netconf-state/datastores
list /netconf-state/datastores/datastore
container /netconf-state/datastores/datastore/name
choice /netconf-state/datastores/datastore/name/datastore
case /netconf-state/datastores/datastore/name/datastore/running
leaf /netconf-state/datastores/datastore/name/datastore/running/running
case /netconf-state/datastores/datastore/name/datastore/candidate
leaf /netconf-state/datastores/datastore/name/datastore/candidate/candidate
case /netconf-state/datastores/datastore/name/datastore/startup
leaf /netconf-state/datastores/datastore/name/datastore/startup/startup
container /netconf-state/datastores/datastore/locks
choice /netconf-state/datastores/datastore/locks/lockType
case /netconf-state/datastores/datastore/locks/lockType/globalLock
container /netconf-state/datastores/datastore/locks/lockType/globalLock/globalLock
leaf /netconf-state/datastores/datastore/locks/lockType/globalLock/globalLock/lockedBySession
leaf /netconf-state/datastores/datastore/locks/lockType/globalLock/globalLock/lockedTime
case /netconf-state/datastores/datastore/locks/lockType/partialLocks
list /netconf-state/datastores/datastore/locks/lockType/partialLocks/partialLocks
leaf /netconf-state/datastores/datastore/locks/lockType/partialLocks/partialLocks/lockId
leaf /netconf-state/datastores/datastore/locks/lockType/partialLocks/partialLocks/lockedBySession
leaf /netconf-state/datastores/datastore/locks/lockType/partialLocks/partialLocks/lockedTime
leaf-list /netconf-state/datastores/datastore/locks/lockType/partialLocks/partialLocks/select
leaf-list /netconf-state/datastores/datastore/locks/lockType/partialLocks/partialLocks/lockedNodes
container /netconf-state/schemas
list /netconf-state/schemas/schema
leaf /netconf-state/schemas/schema/identifier
leaf /netconf-state/schemas/schema/version
leaf /netconf-state/schemas/schema/format
leaf /netconf-state/schemas/schema/namespace
leaf-list /netconf-state/schemas/schema/location
container /netconf-state/sessions
list /netconf-state/sessions/session
leaf /netconf-state/sessions/session/sessionId
leaf /netconf-state/sessions/session/transport
leaf /netconf-state/sessions/session/username
leaf /netconf-state/sessions/session/sourceHost
leaf /netconf-state/sessions/session/loginTime
leaf /netconf-state/sessions/session/inRpcs
leaf /netconf-state/sessions/session/inBadRpcs
leaf /netconf-state/sessions/session/outRpcErrors
leaf /netconf-state/sessions/session/outNotifications
container /netconf-state/statistics
leaf /netconf-state/statistics/netconfStartTime
leaf /netconf-state/statistics/inBadHellos
leaf /netconf-state/statistics/inSessions
leaf /netconf-state/statistics/droppedSessions
leaf /netconf-state/statistics/inRpcs
leaf /netconf-state/statistics/inBadRpcs
leaf /netconf-state/statistics/outRpcErrors
leaf /netconf-state/statistics/outNotifications
rpc /get-schema
container /get-schema/input
leaf /get-schema/input/identifier
leaf /get-schema/input/version
leaf /get-schema/input/format
container /get-schema/output
anyxml /get-schema/output/data

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>

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

#ifndef _H_agt_rpc
#include  "agt_rpc.h"
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

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_val_util
#include  "val_util.h"
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

#define AGT_STATE_TOP_CONTAINER (const xmlChar *)"netconf-state"

#define AGT_STATE_GET_SCHEMA    (const xmlChar *)"get-schema"

#define AGT_STATE_OBJ_DATASTORE   (const xmlChar *)"datastore"
#define AGT_STATE_OBJ_DATASTORES  (const xmlChar *)"datastores"
#define AGT_STATE_OBJ_LOCKS           (const xmlChar *)"locks"

#define AGT_STATE_OBJ_SCHEMA          (const xmlChar *)"schema"
#define AGT_STATE_OBJ_SCHEMAS         (const xmlChar *)"schemas"

#define AGT_STATE_OBJ_IDENTIFIER      (const xmlChar *)"identifier"
#define AGT_STATE_OBJ_FORMAT          (const xmlChar *)"format"
#define AGT_STATE_OBJ_VERSION         (const xmlChar *)"version"
#define AGT_STATE_OBJ_NAMESPACE       (const xmlChar *)"namespace"
#define AGT_STATE_OBJ_LOCATION        (const xmlChar *)"location"

#define AGT_STATE_OBJ_SESSION         (const xmlChar *)"session"
#define AGT_STATE_OBJ_SESSIONS        (const xmlChar *)"sessions"

#define AGT_STATE_OBJ_SESSIONID       (const xmlChar *)"sessionId"
#define AGT_STATE_OBJ_TRANSPORT       (const xmlChar *)"transport"
#define AGT_STATE_OBJ_USERNAME        (const xmlChar *)"username"
#define AGT_STATE_OBJ_SOURCEHOST      (const xmlChar *)"sourceHost"
#define AGT_STATE_OBJ_LOGINTIME       (const xmlChar *)"loginTime"

#define AGT_STATE_OBJ_IN_RPCS         (const xmlChar *)"inRpcs"
#define AGT_STATE_OBJ_IN_SESSIONS     (const xmlChar *)"inSessions"
#define AGT_STATE_OBJ_IN_BAD_RPCS     (const xmlChar *)"inBadRpcs"
#define AGT_STATE_OBJ_IN_BAD_HELLOS   (const xmlChar *)"inBadHellos"
#define AGT_STATE_OBJ_OUT_RPC_ERRORS  (const xmlChar *)"outRpcErrors"
#define AGT_STATE_OBJ_OUT_NOTIFICATIONS  (const xmlChar *)"outNotifications"
#define AGT_STATE_OBJ_DROPPED_SESSIONS  (const xmlChar *)"droppedSessions"
#define AGT_STATE_OBJ_STATISTICS      (const xmlChar *)"statistics"

#define AGT_STATE_FORMAT_YANG         (const xmlChar *)"yang"

#define AGT_STATE_ENUM_NETCONF        (const xmlChar *)"NETCONF"

/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                            *
*                                                                   *
*********************************************************************/

static boolean              agt_state_init_done = FALSE;

static ncx_module_t         *statemod = NULL;

static val_value_t          *mysessionsval;

static val_value_t          *myschemasval;

static obj_template_t *mysessionobj;

static obj_template_t *myschemaobj;


/********************************************************************
* FUNCTION get_locks
*
* <get> operation handler for the locks NP container
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_locks (ses_cb_t *scb,
               getcb_mode_t cbmode,
               val_value_t *virval,
               val_value_t  *dstval)
{
    val_value_t           *nameval, *targval, *newval, *globallockval;
    obj_template_t  *globallock;
    cfg_template_t        *cfg;
    const xmlChar         *locktime;
    status_t          res;
    ses_id_t          sid;
    boolean           globallocked;
    xmlChar           numbuff[NCX_MAX_NUMLEN+1];

    (void)scb;
    res = NO_ERR;

    if (cbmode == GETCB_GET_VALUE) {
        globallock = obj_find_child(virval->obj,
                                    AGT_STATE_MODULE,
                                    (const xmlChar *)"globalLock");
        if (!globallock) {
            return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
        }

        nameval = val_find_child(virval->parent,
                                 AGT_STATE_MODULE,
                                 (const xmlChar *)"name");
        if (!nameval) {
            return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
        }
        
        targval = val_get_first_child(nameval);
        if (!targval) {
            return SET_ERROR(ERR_NCX_DATA_MISSING);
        }
        cfg = cfg_get_config(targval->name);
        if (!cfg) {
            return SET_ERROR(ERR_NCX_CFG_NOT_FOUND);            
        }

        globallocked = cfg_is_global_locked(cfg);
        if (globallocked) {
            /* make the 2 return value leafs to put into
             * the retval container
             */
            res = cfg_get_global_lock_info(cfg, &sid, &locktime);
            if (res == NO_ERR) {
                /* add locks/globalLock */
                globallockval = val_new_value();
                if (!globallockval) {
                    return ERR_INTERNAL_MEM;
                }
                val_init_from_template(globallockval, globallock);
                val_add_child(globallockval, dstval);

                /* add locks/globalLock/lockedBySession */ 
                sprintf((char *)numbuff, "%u", sid);
                newval = agt_make_leaf(globallock,
                                       (const xmlChar *)"lockedBySession",
                                       numbuff, 
                                       &res);
                if (newval) {
                    val_add_child(newval, globallockval);
                }

                /* add locks/globalLock/lockedTime */ 
                newval = agt_make_leaf(globallock,
                                       (const xmlChar *)"lockedTime",
                                       locktime, 
                                       &res);
                if (newval) {
                    val_add_child(newval, globallockval);
                }
            }
        } else {
            res = ERR_NCX_SKIPPED;
        }
    } else {
        res = ERR_NCX_OPERATION_NOT_SUPPORTED;
    }
    return res;

} /* get_locks */


/********************************************************************
* FUNCTION make_datastore_val
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
    make_datastore_val (const xmlChar *confname,
                        obj_template_t *confobj,
                        status_t *res)
{
    obj_template_t  *nameobj, *testobj;
    val_value_t           *confval, *nameval, *leafval;

    nameobj = obj_find_child(confobj, 
                             AGT_STATE_MODULE, 
                             NCX_EL_NAME);
    if (!nameobj) {
        *res = SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }

    /* create datastore node */
    confval = val_new_value();
    if (!confval) {
        *res = ERR_INTERNAL_MEM;
        return NULL;
    }
    val_init_from_template(confval, confobj);

    /* create datastore/name */
    nameval = val_new_value();
    if (!nameval) {
        val_free_value(confval);
        *res = ERR_INTERNAL_MEM;
        return NULL;
    }
    val_init_from_template(nameval, nameobj);
    val_add_child(nameval, confval);
    
    /* create datastore/name/<config-name> */
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

    /* create datastore/locks */
    testobj = obj_find_child(confobj, 
                             AGT_STATE_MODULE, 
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
    val_init_virtual(leafval, get_locks, testobj);
    val_add_child(leafval, confval);

    *res = NO_ERR;
    return confval;

} /* make_datastore_val */


/********************************************************************
* FUNCTION make_schema_val
*
* make a val_value_t struct for a specified module
*
INPUTS:
*   mod == module control block to use
*   schemaobj == <schema> object to use
*   res == address of return status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*   malloced value struct or NULL if some error
*********************************************************************/
static val_value_t *
    make_schema_val (ncx_module_t *mod,
                     obj_template_t *schemaobj,
                     status_t *res)
{
    val_value_t           *schemaval, *childval;

    *res = NO_ERR;

    /* create schema node */
    schemaval = val_new_value();
    if (!schemaval) {
        *res = ERR_INTERNAL_MEM;
        return NULL;
    }
    val_init_from_template(schemaval, schemaobj);

    /* create schema/identifier */
    childval = agt_make_leaf(schemaobj,
                             AGT_STATE_OBJ_IDENTIFIER,
                             ncx_get_modname(mod), 
                             res);
    if (!childval) {
        val_free_value(schemaval);
        return NULL;
    }
    val_add_child(childval, schemaval);

    /* create schema/version */
    childval = agt_make_leaf(schemaobj,
                             AGT_STATE_OBJ_VERSION,
                             ncx_get_modversion(mod), 
                             res);
    if (!childval) {
        val_free_value(schemaval);
        return NULL;
    }
    val_add_child(childval, schemaval);

    /* create schema/format */
    childval = agt_make_leaf(schemaobj,
                             AGT_STATE_OBJ_FORMAT,
                             AGT_STATE_FORMAT_YANG, 
                             res);
    if (!childval) {
        val_free_value(schemaval);
        return NULL;
    }
    val_add_child(childval, schemaval);

    /* create schema/namespace */
    childval = agt_make_leaf(schemaobj,
                             AGT_STATE_OBJ_NAMESPACE,
                             ncx_get_modnamespace(mod), 
                             res);
    if (!childval) {
        val_free_value(schemaval);
        return NULL;
    }
    val_add_child(childval, schemaval);

    /* create schema/location */
    childval = agt_make_leaf(schemaobj,
                             AGT_STATE_OBJ_LOCATION,
                             AGT_STATE_ENUM_NETCONF, 
                             res);
    if (!childval) {
        val_free_value(schemaval);
        return NULL;
    }
    val_add_child(childval, schemaval);

    *res = val_gen_index_chain(schemaobj, schemaval);
    if (*res != NO_ERR) {
        val_free_value(schemaval);
        return NULL;
    }

    return schemaval;

} /* make_schema_val */


/********************************************************************
* FUNCTION make_session_val
*
* make a val_value_t struct for a specified session
*
INPUTS:
*   scb == session control block to use
*   sessionobj == <session> object to use
*   res == address of return status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*   malloced value struct or NULL if some error
*********************************************************************/
static val_value_t *
    make_session_val (ses_cb_t *scb,
                      obj_template_t *sessionobj,
                      status_t *res)
{
    val_value_t           *sessionval, *childval;
    xmlChar               numbuff[NCX_MAX_NUMLEN];

    *res = NO_ERR;

    /* create session node */
    sessionval = val_new_value();
    if (!sessionval) {
        *res = ERR_INTERNAL_MEM;
        return NULL;
    }
    val_init_from_template(sessionval, sessionobj);

    /* create session/sessionId */
    sprintf((char *)numbuff, "%u", scb->sid);
    childval = agt_make_leaf(sessionobj,
                             AGT_STATE_OBJ_SESSIONID,
                             numbuff, 
                             res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);

    /* create session/transport */
    childval = agt_make_leaf(sessionobj,
                             AGT_STATE_OBJ_TRANSPORT,
                             ses_get_transport_name(scb->transport),
                             res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);
    
    /* create session/username */
    childval = agt_make_leaf(sessionobj,
                             AGT_STATE_OBJ_USERNAME,
                             scb->username,
                             res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);

    /* create session/sourceHost */
    childval = agt_make_leaf(sessionobj,
                             AGT_STATE_OBJ_SOURCEHOST,
                             scb->peeraddr,
                             res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);

    /* create session/loginTime */
    childval = agt_make_leaf(sessionobj,
                             AGT_STATE_OBJ_LOGINTIME,
                             scb->start_time,
                             res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);

    /* create session/inRpcs */
    childval = agt_make_virtual_leaf(sessionobj,
                                     AGT_STATE_OBJ_IN_RPCS,
                                     agt_ses_get_session_inRpcs,
                             res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);

    /* create session/inBadRpcs */
    childval = agt_make_virtual_leaf(sessionobj,
                                     AGT_STATE_OBJ_IN_BAD_RPCS,
                                     agt_ses_get_session_inBadRpcs,
                                     res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);

    /* create session/outRpcErrors */
    childval = agt_make_virtual_leaf(sessionobj,
                                     AGT_STATE_OBJ_OUT_RPC_ERRORS,
                                     agt_ses_get_session_outRpcErrors,
                                     res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);

    /* create session/outNotifications */
    childval = agt_make_virtual_leaf(sessionobj,
                                     AGT_STATE_OBJ_OUT_NOTIFICATIONS,
                                     agt_ses_get_session_outNotifications,
                                     res);
    if (!childval) {
        val_free_value(sessionval);
        return NULL;
    }
    val_add_child(childval, sessionval);

    *res = val_gen_index_chain(sessionobj, sessionval);
    if (*res != NO_ERR) {
        val_free_value(sessionval);
        return NULL;
    }

    return sessionval;

} /* make_session_val */


/********************************************************************
* FUNCTION make_statistics_val
*
* make a val_value_t struct for the global statistics block
*
INPUTS:
*   statisticsobj == <statistics> object to use
*   res == address of return status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*   malloced value struct or NULL if some error
*********************************************************************/
static val_value_t *
    make_statistics_val (obj_template_t *statisticsobj,
                         status_t *res)
{
    val_value_t            *statsval, *childval;
    xmlChar                tbuff[TSTAMP_MIN_SIZE+1];

    /* create statistics node */
    statsval = val_new_value();
    if (!statsval) {
        *res = ERR_INTERNAL_MEM;
        return NULL;
    }
    val_init_from_template(statsval, statisticsobj);

    /* add statistics/netconfStartTime static leaf */
    tstamp_datetime(tbuff);
    childval = agt_make_leaf(statisticsobj,
                             (const xmlChar *)"netconfStartTime",
                             tbuff,
                             res);
    if (!childval) {
        val_free_value(statsval);
        return NULL;
    }
    val_add_child(childval, statsval);

    /* add statistics/inBadHellos virtual leaf */
    childval = agt_make_virtual_leaf(statisticsobj,
                                     AGT_STATE_OBJ_IN_BAD_HELLOS,
                                     agt_ses_get_inBadHellos,
                                     res);
    if (!childval) {
        val_free_value(statsval);
        return NULL;
    }
    val_add_child(childval, statsval);

    /* add statistics/inSessions virtual leaf */
    childval = agt_make_virtual_leaf(statisticsobj,
                                     AGT_STATE_OBJ_IN_SESSIONS,
                                     agt_ses_get_inSessions,
                                     res);
    if (!childval) {
        val_free_value(statsval);
        return NULL;
    }
    val_add_child(childval, statsval);

    /* add statistics/droppedSessions virtual leaf */
    childval = agt_make_virtual_leaf(statisticsobj,
                                     AGT_STATE_OBJ_DROPPED_SESSIONS,
                                     agt_ses_get_droppedSessions,
                                     res);
    if (!childval) {
        val_free_value(statsval);
        return NULL;
    }
    val_add_child(childval, statsval);

    /* add statistics/inRpcs virtual leaf */
    childval = agt_make_virtual_leaf(statisticsobj,
                                     AGT_STATE_OBJ_IN_RPCS,
                                     agt_ses_get_inRpcs,
                                     res);
    if (!childval) {
        val_free_value(statsval);
        return NULL;
    }
    val_add_child(childval, statsval);

    /* add statistics/inBadRpcs virtual leaf */
    childval = agt_make_virtual_leaf(statisticsobj,
                                     AGT_STATE_OBJ_IN_BAD_RPCS,
                                     agt_ses_get_inBadRpcs,
                                     res);
    if (!childval) {
        val_free_value(statsval);
        return NULL;
    }
    val_add_child(childval, statsval);

    /* add statistics/outRpcErrors virtual leaf */
    childval = agt_make_virtual_leaf(statisticsobj,
                                     AGT_STATE_OBJ_OUT_RPC_ERRORS,
                                     agt_ses_get_outRpcErrors,
                                     res);
    if (!childval) {
        val_free_value(statsval);
        return NULL;
    }
    val_add_child(childval, statsval);

    /* add statistics/outNotifications virtual leaf */
    childval = agt_make_virtual_leaf(statisticsobj,
                                     AGT_STATE_OBJ_OUT_NOTIFICATIONS,
                                     agt_ses_get_outNotifications,
                                     res);
    if (!childval) {
        val_free_value(statsval);
        return NULL;
    }
    val_add_child(childval, statsval);

    return statsval;

}  /* make_statistics_val */


/********************************************************************
* FUNCTION get_schema_validate
*
* get-schema : validate params callback
*
* INPUTS:
*    see rpc/agt_rpc.h
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_schema_validate (ses_cb_t *scb,
                         rpc_msg_t *msg,
                         xml_node_t *methnode)
{
    ncx_module_t    *findmod;
    const xmlChar   *identifier, *version, *format;
    val_value_t     *validentifier, *valversion, *valformat;
    xmlns_id_t       nsid;
    status_t         res;

    res = NO_ERR;
    findmod = NULL;
    identifier = NULL;
    version = NULL;
    format = NULL;
    nsid = 0;

    /* get the identifier parameter */
    validentifier = val_find_child(msg->rpc_input, 
                                   AGT_STATE_MODULE,
                                   AGT_STATE_OBJ_IDENTIFIER);
    if (validentifier && validentifier->res == NO_ERR) {
        identifier = VAL_STR(validentifier);
    }

    /* get the version parameter */
    valversion = val_find_child(msg->rpc_input, 
                                AGT_STATE_MODULE,
                                AGT_STATE_OBJ_VERSION);
    if (valversion && valversion->res == NO_ERR) {
        version = VAL_STR(valversion);
    }

    /* get the format parameter */
    valformat = val_find_child(msg->rpc_input, 
                               AGT_STATE_MODULE,
                               AGT_STATE_OBJ_FORMAT);
    if (valformat && valformat->res == NO_ERR) {
        format = VAL_IDREF_NAME(valformat);
        nsid = VAL_IDREF_NSID(valformat);
    }

    if (!identifier || !version || !format) {
        /* should already be reported */
        return ERR_NCX_MISSING_PARM;
    }

    /* check the identifier: must be valid module name */
    if (!ncx_valid_name2(identifier)) {
        res = ERR_NCX_NO_MATCHES;
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res, 
                         methnode, 
                         NCX_NT_STRING, 
                         identifier,
                         NCX_NT_VAL, 
                         validentifier);
    }

    /* do not check the revision now 
     * it must be an exact match if non-empty string
     */
    if (!*version) {
        /* send NULL instead of empty string */
        version = NULL;
    }

    /* check format parameter: only YANG supported for now */
    if (xml_strcmp(format, AGT_STATE_FORMAT_YANG) ||
        nsid != statemod->nsid) {

        res = ERR_NCX_VALUE_NOT_SUPPORTED;
        agt_record_error(scb, 
                         &msg->mhdr, 
                         NCX_LAYER_OPERATION, 
                         res, 
                         methnode, 
                         NCX_NT_STRING, 
                         format,
                         NCX_NT_VAL, 
                         valformat);

    }

    if (res == NO_ERR) {
        findmod = ncx_find_module(identifier, version);
        if (!findmod) {
            res = ERR_NCX_NO_MATCHES;
            agt_record_error(scb, 
                             &msg->mhdr, 
                             NCX_LAYER_OPERATION, 
                             res, 
                             methnode, 
                             NCX_NT_STRING, 
                             identifier,
                             NCX_NT_VAL, 
                             validentifier);
        } else {
            /* save the found module and format
             * setup the automatic output using the callback
             * function in agt_util.c
             */
            msg->rpc_user1 = (void *)findmod;
            msg->rpc_user2 = (void *)NCX_MODFORMAT_YANG;
            msg->rpc_data_type = RPC_DATA_STD;
            msg->rpc_datacb = agt_output_schema;
        }
    }

    return res;

} /* get_schema_validate */


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
    agt_profile_t   *agt_profile;
    status_t         res;

    if (agt_state_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

#ifdef AGT_STATE_DEBUG
    log_debug2("\nagt: Loading netconf-state module");
#endif

    agt_profile = agt_get_profile();

    /* load the netconf-state module */
    res = ncxmod_load_module(AGT_STATE_MODULE, 
                             NULL, 
                             &agt_profile->agt_savedevQ,
                             &statemod);
    if (res != NO_ERR) {
        return res;
    }

    mysessionsval = NULL;
    myschemasval = NULL;
    mysessionobj = NULL;
    myschemaobj = NULL;
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
    obj_template_t  *topobj, *confsobj, *confobj;
    obj_template_t  *sessionsobj, *statisticsobj;
    obj_template_t  *schemasobj;
    val_value_t           *topval, *capsval;
    val_value_t           *confsval, *confval;
    val_value_t           *sessionsval, *statisticsval;
    cfg_template_t        *runningcfg;
    ncx_module_t          *mod;
    status_t  res;

    if (!agt_state_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    /* set up get-schema RPC operation */
    res = agt_rpc_register_method(AGT_STATE_MODULE,
                                  AGT_STATE_GET_SCHEMA,
                                  AGT_RPC_PH_VALIDATE,
                                  get_schema_validate);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!runningcfg || !runningcfg->root) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* get all the object nodes first */
    topobj = obj_find_template_top(statemod, 
                                   AGT_STATE_MODULE,
                                   AGT_STATE_TOP_CONTAINER);
    if (!topobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    confsobj = obj_find_child(topobj, 
                              AGT_STATE_MODULE, 
                              AGT_STATE_OBJ_DATASTORES);
    if (!confsobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    confobj = obj_find_child(confsobj,
                             AGT_STATE_MODULE,
                             AGT_STATE_OBJ_DATASTORE);
    if (!confobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    schemasobj = obj_find_child(topobj,
                                AGT_STATE_MODULE,
                                AGT_STATE_OBJ_SCHEMAS);
    if (!schemasobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    myschemaobj = obj_find_child(schemasobj,
                                 AGT_STATE_MODULE,
                                 AGT_STATE_OBJ_SCHEMA);
    if (!myschemaobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    sessionsobj = obj_find_child(topobj,
                                 AGT_STATE_MODULE,
                                 AGT_STATE_OBJ_SESSIONS);
    if (!sessionsobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    mysessionobj = obj_find_child(sessionsobj,
                                  AGT_STATE_MODULE,
                                  AGT_STATE_OBJ_SESSION);
    if (!mysessionobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    statisticsobj = obj_find_child(topobj,
                                   AGT_STATE_MODULE,
                                   AGT_STATE_OBJ_STATISTICS);
    if (!statisticsobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* add /ietf-netconf-state */
    topval = val_new_value();
    if (!topval) {
        return ERR_INTERNAL_MEM;
    }
    val_init_from_template(topval, topobj);

    /* handing off the malloced memory here */
    val_add_child(topval, runningcfg->root);

    /* add /ietf-netconf-state/capabilities */
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

    /* add /ietf-netconf-state/datastores */
    confsval = val_new_value();
    if (!confsval) {
        return ERR_INTERNAL_MEM;
    }
    val_init_from_template(confsval, confsobj);
    val_add_child(confsval, topval);

    /* add /ietf-netconf-state/datastores/datastore[1] */
    if (agt_cap_std_set(CAP_STDID_CANDIDATE)) {
        confval = make_datastore_val(NCX_EL_CANDIDATE,
                                     confobj,
                                     &res);
        if (!confval) {
            return res;
        }
        val_add_child(confval, confsval);
    }

    /* add /ietf-netconf-state/datastores/datastore[2] */
    confval = make_datastore_val(NCX_EL_RUNNING,
                                 confobj,
                                 &res);
    if (!confval) {
        return res;
    }
    val_add_child(confval, confsval);

    /* add /ietf-netconf-state/datastores/datastore[3] */
    if (agt_cap_std_set(CAP_STDID_STARTUP)) {
        confval = make_datastore_val(NCX_EL_STARTUP,
                                     confobj,
                                     &res);
        if (!confval) {
            return res;
        }
        val_add_child(confval, confsval);
    }

    /* add /ietf-netconf-state/schemas */
    myschemasval = val_new_value();
    if (!myschemasval) {
        return ERR_INTERNAL_MEM;
    }
    val_init_from_template(myschemasval, schemasobj);
    val_add_child(myschemasval, topval);

    /* add all the /ietf-netconf-state/schemas/schema nodes */
    for (mod = ncx_get_first_module();
         mod != NULL;
         mod = ncx_get_next_module(mod)) {
        res = agt_state_add_module_schema(mod);
        if (res != NO_ERR) {
            return res;
        }
    }

    /* add /ietf-netconf-state/sessions */
    sessionsval = val_new_value();
    if (!sessionsval) {
        return ERR_INTERNAL_MEM;
    }
    val_init_from_template(sessionsval, sessionsobj);
    val_add_child(sessionsval, topval);
    mysessionsval = sessionsval;

    /* add /ietf-netconf-state/statistics */
    statisticsval = make_statistics_val(statisticsobj,
                                        &res);
    if (!statisticsval) {
        return ERR_INTERNAL_MEM;
    }
    val_add_child(statisticsval, topval);

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
        mysessionsval = NULL;
        myschemasval = NULL;
        mysessionobj = NULL;
        myschemaobj = NULL;

        agt_rpc_unregister_method(AGT_STATE_MODULE, 
                                  AGT_STATE_GET_SCHEMA);

        agt_state_init_done = FALSE;
    }
}  /* agt_state_cleanup */


/********************************************************************
* FUNCTION agt_state_add_session
*
* Add a session entry to the netconf-state DM
*
* INPUTS:
*   scb == session control block to use for the info
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_state_add_session (ses_cb_t *scb)
{
    val_value_t *session;
    status_t     res;

    res = NO_ERR;
    session = make_session_val(scb, mysessionobj, &res);
    if (!session) {
        return res;
    }

    val_add_child(session, mysessionsval);
    return NO_ERR;

}  /* agt_state_add_session */


/********************************************************************
* FUNCTION agt_state_remove_session
*
* Remove a session entry from the netconf-state DM
*
* INPUTS:
*   sid == session ID to find and delete
*
*********************************************************************/
void
    agt_state_remove_session (ses_id_t sid)
{
    val_value_t  *sessionval, *idval;

    if (mysessionsval == NULL) {
        /* cleanup already done */
        return;
    }

    for (sessionval = val_get_first_child(mysessionsval);
         sessionval != NULL;
         sessionval = val_get_next_child(sessionval)) {

        idval = val_find_child(sessionval, 
                               AGT_STATE_MODULE, 
                               AGT_STATE_OBJ_SESSIONID);
        if (!idval) {
            SET_ERROR(ERR_INTERNAL_VAL);
        } else if (VAL_UINT(idval) == sid) {
            dlq_remove(sessionval);
            val_free_value(sessionval);
            return;
        }
    }
    /* session already removed -- ignore the error */

}  /* agt_state_remove_session */


/********************************************************************
* FUNCTION agt_state_add_module_schema
*
* Add a schema entry to the netconf-state DM
*
* INPUTS:
*   mod == module to add
*
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_state_add_module_schema (ncx_module_t *mod)
{
    val_value_t *schema;
    status_t     res;

    res = NO_ERR;
    schema = make_schema_val(mod, myschemaobj, &res);
    if (!schema) {
        return res;
    }

    val_add_child(schema, myschemasval);
    return res;

}  /* agt_state_add_module_schema */


/* END file agt_state.c */
