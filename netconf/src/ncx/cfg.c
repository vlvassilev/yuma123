/*  FILE: cfg.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
15apr06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_cfg
#include  "cfg.h"
#endif

#ifndef _H_def_reg
#include  "def_reg.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_ps
#include  "ps.h"
#endif

#ifndef _H_rpc
#include  "rpc.h"
#endif

#ifndef _H_rpc_err
#include  "rpc_err.h"
#endif

#ifndef _H_ses
#include  "ses.h"
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
#define CFG_NUM_STATIC 6

#define CFG_DATETIME_LEN 64

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
static boolean cfg_init_done = FALSE;

static cfg_template_t  *cfg_arr[CFG_NUM_STATIC];

/* static dlq_hdr_t        cfg_dynQ; */



/********************************************************************
* FUNCTION new_cur_datetime
*
* Malloc and set a datetime string to the current time
*
* INPUTS:
*    none
* RETURNS:
*    malloced string or NULL if some error
*    This string needs to be freed by the caller
*********************************************************************/
static xmlChar *
    new_cur_datetime (void)
{
    xmlChar *str;

    str = m__getMem(CFG_DATETIME_LEN);
    if (!str) {
	return NULL;
    }
    *str=0;
    tstamp_datetime(str);
    return str;

} /* new_cur_datetime */


/********************************************************************
* FUNCTION get_template
*
* Get the config template from its name
*
* INPUTS:
*    name == config name
* RETURNS:
*    pointer config template or NULL if not found
*********************************************************************/
static cfg_template_t *
    get_template (const xmlChar *cfgname)
{
    ncx_cfg_t id;

    for (id = NCX_CFGID_RUNNING; id <= NCX_CFGID_ROLLBACK; id++) {
	if (!cfg_arr[id]) {
	    continue;
	}
	if (!xml_strcmp(cfg_arr[id]->name, cfgname)) {
	    return cfg_arr[id];
	}
    }
    return NULL;

} /* get_template */


/********************************************************************
* FUNCTION get_appnode
*
* Get the requested cfg_app_t node or create one
*
* INPUTS:
*    cfg == config struct
*    ownname == owner name
*    appname == application name
* RETURNS:
*    pointer to cfg_app_t node or NULL if error
*********************************************************************/
static cfg_app_t *
    get_appnode (cfg_template_t *cfg,
		 const xmlChar *modname,
		 const xmlChar *appname)
{
    cfg_app_t      *app;
    ncx_appnode_t  *appnode;
    ncx_node_t      dtyp;


    if (!cfg->root) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    for (app = (cfg_app_t *)dlq_firstEntry(&cfg->root->v.appQ);
	 app != NULL;
	 app = (cfg_app_t *)dlq_nextEntry(app)) {
	if (!xml_strcmp(app->appdef->owner, modname) &&
	    !xml_strcmp(app->appdef->appname, appname)) {
	    return app;
	}
    }

    /* not found, check if the ncx_appnode exists */
    dtyp = NCX_NT_APP;
    appnode = def_reg_find_cfgapp(modname, appname, cfg->cfg_id);
    if (!appnode) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    /* cfg appnode is allowed to exist, so create one */
    app = cfg_new_appnode();
    if (!app) {
	return NULL;
    }
    app->appdef = appnode;
    dlq_enque(app, &cfg->root->v.appQ);
    return app;

} /* get_appnode */


/***************** E X P O R T E D    F U N C T I O N S  ***********/


/********************************************************************
* FUNCTION cfg_init
*
* Initialize the config manager
*
* INPUTS:
*    none
* RETURNS:
*    none
*********************************************************************/
void
    cfg_init (void)
{

    uint32  i;

    if (!cfg_init_done) {

	for (i=0; i<CFG_NUM_STATIC; i++) {
	    cfg_arr[i] = NULL;
	}

	cfg_init_done = TRUE;
    }

} /* cfg_init */


/********************************************************************
* FUNCTION cfg_cleanup
*
* Cleanup the config manager
*
* INPUTS:
*    none
* RETURNS:
*    none
*********************************************************************/
void
    cfg_cleanup (void)
{
    ncx_cfg_t   id;

    if (cfg_init_done) {
	for (id=NCX_CFGID_RUNNING; id<=NCX_CFGID_ROLLBACK; id++) {
	    if (cfg_arr[id]) {
		cfg_free_template(cfg_arr[id]);
		cfg_arr[id] = NULL;
	    }
	}
	cfg_init_done = FALSE;
    }

} /* cfg_cleanup */


/********************************************************************
* FUNCTION cfg_init_static_db
*
* Initialize the config manager
*
* INPUTS:
*    id   == cfg ID
*    
* RETURNS:
*    none
*********************************************************************/
status_t
    cfg_init_static_db (ncx_cfg_t cfg_id)
{
    cfg_template_t   *cfg;
    const xmlChar    *name;

    if (!cfg_init_done) {
	cfg_init();
    }

#ifdef DEBUG
    if (cfg_id > NCX_CFGID_ROLLBACK) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
    if (cfg_arr[cfg_id]) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* get the hard-wired config name */
    switch (cfg_id) {
    case NCX_CFGID_RUNNING:
	name = NCX_CFG_RUNNING;
	break;
    case NCX_CFGID_CANDIDATE:
	name = NCX_CFG_CANDIDATE;
	break;
    case NCX_CFGID_STARTUP:
	name = NCX_CFG_STARTUP;
	break;
    case NCX_CFGID_ROLLBACK:
	name = NCX_CFG_ROLLBACK;
	break;
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    cfg = cfg_new_template(name, cfg_id);
    if (!cfg) {
	return ERR_INTERNAL_MEM;
    }

    cfg_arr[cfg_id] = cfg;
    return NO_ERR;

} /* cfg_init_static_db */


/********************************************************************
* FUNCTION cfg_new_template
*
* Malloc and initialize a cfg_template_t struct
*
* INPUTS:
*    name == cfg name
*    cfg_id   == cfg ID
* RETURNS:
*    malloced struct or NULL if some error
*    This struct needs to be freed by the caller
*********************************************************************/
cfg_template_t *
    cfg_new_template (const xmlChar *name,
		      ncx_cfg_t cfg_id)
{
    cfg_template_t *cfg;

    cfg = m__getObj(cfg_template_t);
    if (!cfg) {
	return NULL;
    }

    memset(cfg, 0x0, sizeof(cfg_template_t));

    cfg->name = xml_strdup(name);
    if (!cfg->name) {
	m__free(cfg);
	return NULL;
    }

    cfg->cfg_id = cfg_id;
    cfg->cfg_state = CFG_ST_INIT;
    dlq_createSQue(&cfg->load_errQ);
    /* root is still NULL; indicates empty cfg */

    return cfg;

} /* cfg_new_template */


/********************************************************************
* FUNCTION cfg_free_template
*
* Clean and free the cfg_template_t struct
*
* INPUTS:
*    cfg = cfg_template_t to clean and free
* RETURNS:
*    none
*********************************************************************/
void
    cfg_free_template (cfg_template_t *cfg)
{

    rpc_err_rec_t  *err;

#ifdef DEBUG
    if (!cfg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (cfg->name) {
	m__free(cfg->name);
    }
    if (cfg->src_url) {
	m__free(cfg->src_url);
    }
    if (cfg->load_time) {
	m__free(cfg->load_time);
    }
    if (cfg->last_ch_time) {
	m__free(cfg->last_ch_time);
    } 

    while (!dlq_empty(&cfg->load_errQ)) {
	err = (rpc_err_rec_t *)dlq_deque(&cfg->load_errQ);
	rpc_err_free_record(err);
    }

    if (cfg->root) {
	val_free_value(cfg->root);
    }

    m__free(cfg);

} /* cfg_free_template */


/********************************************************************
* FUNCTION cfg_new_appnode
*
* Malloc and Initialize a cfg_app_t struct
*
* RETURNS:
*    pointer to new (empty) app container struct
*********************************************************************/
cfg_app_t *
    cfg_new_appnode (void)
{
    cfg_app_t *app;

    app = m__getObj(cfg_app_t);
    if (!app) {
	return NULL;
    }

    cfg_init_appnode(app);
    return app;

} /* cfg_new_appnode */


/********************************************************************
* FUNCTION cfg_init_appnode
*
* Initialize a pre-malloced cfg_app_t struct
*
* INPUTS:
*   app == cfg_app_t struct to initialize
*
*********************************************************************/
void
    cfg_init_appnode (cfg_app_t *app)
{
#ifdef DEBUG
    if (!app) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(app, 0x0, sizeof(cfg_app_t));
    dlq_createSQue(&app->parmsetQ);

} /* cfg_init_appnode */


/********************************************************************
* FUNCTION cfg_clean_appnode
*
* Clean a cfg_app_t struct
*
* INPUTS:
*   app == cfg_app_t struct to clean
*
*********************************************************************/
void
    cfg_clean_appnode (cfg_app_t *app)
{
    ps_parmset_t *ps;

#ifdef DEBUG
    if (!app) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(&app->parmsetQ)) {
	ps = (ps_parmset_t *)dlq_deque(&app->parmsetQ);
	ps_free_parmset(ps);
    }

    app->appdef = NULL;
    app->parent = NULL;
    app->editop = OP_EDITOP_NONE;

    if (app->last) {
	cfg_free_appnode(app->last);
	app->last = NULL;
    }
	    
} /* cfg_clean_appnode */


/********************************************************************
* FUNCTION cfg_free_appnode
*
* Clean and free a malloced cfg_app_t struct
*
* INPUTS:
*   app == cfg_app_t struct to clean and free
*
*********************************************************************/
void
    cfg_free_appnode (cfg_app_t *app)
{

#ifdef DEBUG
    if (!app) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    cfg_clean_appnode(app);

    m__free(app);
	
} /* cfg_free_appnode */


/********************************************************************
* FUNCTION cfg_set_state
*
* Change the state of the specified static config
*
* INPUTS:
*    cfg_id = Config ID to change
*    new_state == new config state to set 
* RETURNS:
*    status; no-op change (new_state == old_state) is not an error
*********************************************************************/
void
    cfg_set_state (ncx_cfg_t cfg_id,
		   cfg_state_t  new_state)
{
#ifdef DEBUG
    if (cfg_id > NCX_CFGID_ROLLBACK) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    if (!cfg_arr[cfg_id]) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
#endif

    cfg_arr[cfg_id]->cfg_state = new_state;

} /* cfg_set_state */


/********************************************************************
* FUNCTION cfg_get_state
*
* Get the state of the specified static config
*
* INPUTS:
*    cfg_id = Config ID
* RETURNS:
*    config state  (CFG_ST_NONE if some error)
*********************************************************************/
cfg_state_t
    cfg_get_state (ncx_cfg_t cfg_id)
{
#ifdef DEBUG
    if (cfg_id > NCX_CFGID_ROLLBACK) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return CFG_ST_NONE;
    }
#endif

    if (!cfg_arr[cfg_id]) {
	return CFG_ST_NONE;
    }

    return cfg_arr[cfg_id]->cfg_state;

} /* cfg_get_state */


/********************************************************************
* FUNCTION cfg_get_config
*
* Get the config struct from its name
*
* INPUTS:
*    cfgname = Config Name
* RETURNS:
*    pointer to config struct or NULL if not found
*********************************************************************/
cfg_template_t *
    cfg_get_config (const xmlChar *cfgname)
{
#ifdef DEBUG
    if (!cfgname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return get_template(cfgname);

} /* cfg_get_config */


/********************************************************************
* FUNCTION cfg_get_config_id
*
* Get the config struct from its ID
*
* INPUTS:
*    cfgid == config ID
* RETURNS:
*    pointer to config struct or NULL if not found
*********************************************************************/
cfg_template_t *
    cfg_get_config_id (ncx_cfg_t cfgid)
{

    if (cfgid <= NCX_CFGID_ROLLBACK) {
	return cfg_arr[cfgid];
    }
    return NULL;

} /* cfg_get_config_id */


/********************************************************************
* FUNCTION cfg_set_target
*
* Set the CFG_FL_TARGET flag in the specified config
*
* INPUTS:
*    cfg_id = Config ID to set as a valid target
* RETURNS:
*    status
*********************************************************************/
void
    cfg_set_target (ncx_cfg_t cfg_id)
{
#ifdef DEBUG
    if (cfg_id > NCX_CFGID_ROLLBACK) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    if (!cfg_arr[cfg_id]) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    cfg_arr[cfg_id]->flags |= CFG_FL_TARGET;

} /* cfg_set_target */


/********************************************************************
* FUNCTION cfg_ok_to_lock
*
* Check if the specified config can be locked right now
*
* INPUTS:
*    cfg = Config template to check 
*
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_ok_to_lock (const cfg_template_t *cfg)
{
    status_t  res;

#ifdef DEBUG
    if (!cfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    switch (cfg->cfg_state) {
    case CFG_ST_READY:
	/* lock can be granted */
	res = NO_ERR;
	break;
    case CFG_ST_PLOCK:
	/* fall through -- TBD -- treat as full lock */
    case CFG_ST_FLOCK:
	/* full lock already held by a session 
	 * get the session ID of the lock holder 
	 */
	res = ERR_NCX_LOCK_DENIED;
	break;
    case CFG_ST_NONE:
    case CFG_ST_INIT:
    case CFG_ST_CLEANUP:
	/* config is in a state where locks cannot be granted */
	res = ERR_NCX_NO_ACCESS_STATE;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);

    }

    return res;

} /* cfg_ok_to_lock */


/********************************************************************
* FUNCTION cfg_ok_to_unlock
*
* Check if the specified config can be unlocked right now
*
* INPUTS:
*    cfg = Config template to check 
*    sesid == session ID requesting to unlock the config
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_ok_to_unlock (const cfg_template_t *cfg,
		      ses_id_t sesid)
{
    status_t  res;

#ifdef DEBUG
    if (!cfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    switch (cfg->cfg_state) {
    case CFG_ST_PLOCK:
	/* Partial locks still TBD */
	/* fall through */
    case CFG_ST_FLOCK:
	/* lock is granted
	 * setup the user1 scratchpad with the cfg to lock 
	 */
	if (cfg->locked_by == sesid) {
	    res = NO_ERR;
	} else {
	    res = ERR_NCX_NO_ACCESS_LOCK;
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	/* fall through */
    case CFG_ST_NONE:
    case CFG_ST_INIT:
    case CFG_ST_READY:
    case CFG_ST_CLEANUP:
	/* config is not in a locked state */
	res = ERR_NCX_NO_ACCESS_STATE;
    }

    return res;

} /* cfg_ok_to_unlock */


/********************************************************************
* FUNCTION cfg_ok_to_read
*
* Check if the specified config can be read right now
*
* INPUTS:
*    cfg = Config template to check 
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_ok_to_read (const cfg_template_t *cfg)
{
    status_t  res;

#ifdef DEBUG
    if (!cfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    switch (cfg->cfg_state) {
    case CFG_ST_PLOCK:
    case CFG_ST_FLOCK:
    case CFG_ST_INIT:
    case CFG_ST_READY:
	res = NO_ERR;
	break;
    case CFG_ST_NONE:
    case CFG_ST_CLEANUP:
	/* config is not in a writable state */
	res = ERR_NCX_NO_ACCESS_STATE;
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
	break;
    }

    return res;

} /* cfg_ok_to_read */


/********************************************************************
* FUNCTION cfg_ok_to_write
*
* Check if the specified config can be written right now
*
* This is not an access control check,
* only locks and config state will be checked
*
* INPUTS:
*    cfg = Config template to check 
*    sesid == session ID requesting to write to the config
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_ok_to_write (const cfg_template_t *cfg,
		     ses_id_t sesid)
{
    status_t  res;

#ifdef DEBUG
    if (!cfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check if this is a writable target,
     * except during agent boot
     * except for the <startup> config
     */
    if (cfg->cfg_state != CFG_ST_INIT) {
	if (cfg->cfg_id != NCX_CFGID_STARTUP &&
	    !(cfg->flags & CFG_FL_TARGET)) {
	    return ERR_NCX_NOT_WRITABLE;
	}
    }

    /* check the current config state */
    switch (cfg->cfg_state) {
    case CFG_ST_PLOCK:
	/* Partial locks still TBD */
	/* fall through */
    case CFG_ST_FLOCK:
	if (cfg->locked_by == sesid) {
	    res = NO_ERR;
	} else {
	    res = ERR_NCX_NO_ACCESS_LOCK;
	}
	break;
    case CFG_ST_INIT:
    case CFG_ST_READY:
	res = NO_ERR;
	break;
    case CFG_ST_NONE:
    case CFG_ST_CLEANUP:
	/* config is not in a writable state */
	res = ERR_NCX_NO_ACCESS_STATE;
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	res = ERR_NCX_OPERATION_FAILED;
	break;
    }

    return res;

} /* cfg_ok_to_write */


/********************************************************************
* FUNCTION cfg_lock
*
* Lock the specified config.
* This will not really have an effect unless the
* CFG_FL_TARGET flag in the specified config is also set
*
* INPUTS:
*    cfg = Config template to lock
*    locked_by == session ID of the lock owner
*    lock_src == enum classifying the lock source
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_lock (cfg_template_t *cfg,
	      ses_id_t locked_by,
	      cfg_source_t  lock_src)
{
    status_t  res;

#ifdef DEBUG
    if (!cfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = cfg_ok_to_lock(cfg);

    if (res == NO_ERR) {
	cfg->cfg_state = CFG_ST_FLOCK;
	cfg->locked_by = locked_by;
	cfg->lock_src = lock_src;
    }

    return res;

} /* cfg_lock */


/********************************************************************
* FUNCTION cfg_unlock
*
* Unlock the specified config.
*
* INPUTS:
*    cfg = Config template to unlock
*    locked_by == session ID of the lock owner
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_unlock (cfg_template_t *cfg,
		ses_id_t locked_by)
{
    status_t  res;

#ifdef DEBUG
    if (!cfg) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = cfg_ok_to_unlock(cfg, locked_by);

    if (res == NO_ERR) {
	cfg->cfg_state = CFG_ST_READY;
	cfg->locked_by = 0;
	cfg->lock_src = CFG_SRC_NONE;
    }

    return res;

} /* cfg_unlock */


/********************************************************************
* FUNCTION cfg_add_parmset
*
* Add a parmset and possibly a  new app to a cfg_template_t
* This will fail if the parmset already exists
*
* INPUTS:
*    cfg = Config template to load data into
*    ps == ps_parmset_t to add
*
*      **** THIS PARAM IS TRANSFERRED TO THE CFG -- NOT DUPLICATED
*      **** THE CALLER MUST NOT FREE THE PARMSET UNTIL THE CFG
*      **** ITSELF IS FREED
*    
*    loadby == session ID calling this function 
*           == SES_NULL_SID if internal call to add a parmset
* 
* RETURNS:
*    overall status; may be the last of multiple error conditions
*********************************************************************/
status_t
    cfg_add_parmset (cfg_template_t *cfg,
		     ps_parmset_t *ps,
		     ses_id_t     loadby)
{
    ps_parmset_t   *cfgps;
    const psd_template_t *psd;
    cfg_app_t      *cfgapp;
    status_t        res;

#ifdef DEBUG
    if (!cfg || !ps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* check if this session can write to this config */
    if (loadby != SES_NULL_SID) {
	res = cfg_ok_to_write(cfg, loadby);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* check if the app node already exists */
    cfgps = cfg_get_parmset(cfg, ps);
    if (cfgps) {
	return ERR_NCX_ENTRY_EXISTS;
    }

    psd = ps->psd;

    /* add this parmset to the config first 
     * It is not really visible until it is added to the def_reg 
     */
    cfgapp = get_appnode(cfg, psd->module, psd->app);
    if (!cfgapp) {
	return ERR_INTERNAL_MEM;
    }

    /* add this parmset to the def registry */
    res = def_reg_add_cfgdef(psd->module, psd->app, psd->name, 
	     ps->instance, cfg->cfg_id, NCX_NT_PARMSET, ps);
    if (res == NO_ERR) {
	dlq_enque(ps, &cfgapp->parmsetQ);
	ps->parent = cfgapp;
    }

    return res;

} /* cfg_add_parmset */


/********************************************************************
* FUNCTION cfg_get_parmset
*
* Get the specified parmset form the specified config
*
* INPUTS:
*    cfg = Config template to search
*    ps == ps_parmset_t (from PDU) to find in the real config
*
* RETURNS:
*    pointer to the matching parmset from the config, or NULL
*    if not found
*
*********************************************************************/
ps_parmset_t *
    cfg_get_parmset (cfg_template_t *cfg,
		     ps_parmset_t *ps)
{
    ps_parmset_t          *cfgps;
    const psd_template_t  *psd;
    ncx_node_t             dtyp;

#ifdef DEBUG
    if (!cfg || !ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    psd = ps->psd;

    /* look up the requested parmset in the def_reg */
    dtyp = NCX_NT_PARMSET;
    cfgps = (ps_parmset_t *)def_reg_find_cfgdef(psd->module, 
	psd->app, psd->name, ps->instance, cfg->cfg_id, &dtyp);
    
    return cfgps;

} /* cfg_get_parmset */


/********************************************************************
* FUNCTION cfg_del_parmset
*
* Delete the specified parmset form the specified config
*
* INPUTS:
*    cfg = Config template to delete from
*    ps == ps_parmset_t (from PDU) to find in the real config
*          and delete
*    delcheck == TRUE if an error should be returned if
*      an attempt to delete a non-existent entry is made
* RETURNS:
*    statis
*********************************************************************/
status_t 
    cfg_del_parmset (cfg_template_t *cfg,
		     ps_parmset_t *ps,
		     boolean  delcheck)
{
    ps_parmset_t          *cfgps;
    const psd_template_t  *psd;
    ncx_node_t             dtyp;

#ifdef DEBUG
    if (!cfg || !ps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    psd = ps->psd;

    /* look up the requested parmset in the def_reg */
    dtyp = NCX_NT_PARMSET;
    cfgps = (ps_parmset_t *)def_reg_find_cfgdef(psd->module, 
	psd->app, psd->name, ps->instance, cfg->cfg_id, &dtyp);

    if (cfgps) {
	/* remove from the config */
	dlq_remove(cfgps);

	/* remove from the registry */
	def_reg_del_cfgdef(psd->module, psd->app, psd->name, 
	    ps->instance, (int32)cfg->cfg_id);

	return NO_ERR;
    } 

    return (delcheck) ? ERR_NCX_DATA_MISSING : NO_ERR;

} /* cfg_del_parmset */


/********************************************************************
* FUNCTION cfg_load_root
*
* Called via cfg_load, after the config data has been parsed
* and validated.
*
* This function should only be used to load an empty config
* in CFG_ST_INIT state
*
* INPUTS:
*    cfg = Config template to load data into
*    cfg->root is ready to load into the definition registry
* 
* OUTPUTS:
*    errQ contains any rpc_err_rec_t structs (if non-NULL)
*    def_reg calls are made to add the top-level parmsets to the
*    registry 
*
*   *** TBD NESTED PARMSET ROOTS ****
*
* RETURNS:
*    overall status; may be the last of multiple error conditions
*********************************************************************/
status_t
    cfg_load_root (cfg_template_t *cfg)
{
    cfg_app_t    *app;
    ps_parmset_t *ps;
    status_t      res, retres;

#ifdef DEBUG
    if (!cfg || !cfg->root) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (cfg->cfg_state != CFG_ST_INIT) {
	return SET_ERROR(ERR_NCX_CFG_STATE);
    }
#endif

    /* finish setting up the <config> root value */
    cfg->root->name = NCX_EL_CONFIG;
    cfg->root->nsid = xmlns_nc_id();
    cfg->root->typdef = typ_get_basetype_typdef(NCX_BT_ROOT);
    retres = NO_ERR;

    /* go through the root value and apply the requested
     * write operation 
     */
    for (app = (cfg_app_t *)dlq_firstEntry(&cfg->root->v.appQ);
	 app != NULL;
	 app = (cfg_app_t *)dlq_nextEntry(app)) {

	for (ps = (ps_parmset_t *)dlq_firstEntry(&app->parmsetQ);
	     ps != NULL;
	     ps = (ps_parmset_t *)dlq_nextEntry(ps)) {

	    res = def_reg_add_cfgdef(app->appdef->owner,
		app->appdef->appname, ps->psd->name, 
	        ps->instance, cfg->cfg_id, NCX_NT_PARMSET, ps);
	    if (res != NO_ERR) {
		return res;
	    }
	}
    }

    /* set the load_time and last_ch_time timestamps */
    if (retres == NO_ERR) {
	cfg->load_time = new_cur_datetime();
	if (cfg->load_time) {
	    cfg->last_ch_time = xml_strdup(cfg->load_time);
	    if (!cfg->last_ch_time) {
		retres = ERR_INTERNAL_MEM;
	    }
	} else {
	    retres = ERR_INTERNAL_MEM;
	}
    }

    return retres;

} /* cfg_load_root */


/********************************************************************
* FUNCTION cfg_get_appnode
*
* Get the specified cfg_app_t node from the root of the specified config
*
* INPUTS:
*    cfg = Config template to search
*    owner == owner name
*    appname == application node name
*
* RETURNS:
*    pointer to the matching appnode from the config, or NULL
*    if not found
*
*********************************************************************/
cfg_app_t *
    cfg_get_appnode (cfg_template_t *cfg,
		     const xmlChar *owner,
		     const xmlChar *appname)
{

    cfg_app_t  *app;

#ifdef DEBUG
    if (!cfg || !owner || !appname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (cfg->root) {
	app = def_reg_find_cfgapp(owner, appname, cfg->cfg_id);
    } else {
	app = NULL;
    }
    return app;

} /* cfg_get_appnode */


/********************************************************************
* FUNCTION cfg_release_locks
*
* Release any configuration locks held by the specified session
*
* INPUTS:
*    sesid == session ID to check for
*
*********************************************************************/
void
    cfg_release_locks (ses_id_t sesid)
{
    uint32  i;
    cfg_template_t *cfg;

    if (!cfg_init_done) {
	return;
    }

    for (i=0; i<CFG_NUM_STATIC; i++) {
	cfg = cfg_arr[i];
	if (cfg && cfg->locked_by == sesid) {
	    cfg->cfg_state = CFG_ST_READY;
	    cfg->locked_by = 0;
	    cfg->lock_src = CFG_SRC_NONE;
	    log_info("\ncfg forced unlock on %s config, held by session %d",
		     cfg->name, sesid);
	}
    }

} /* cfg_release_locks */


/********************************************************************
* FUNCTION cfg_get_lock_list
*
* Get a list of all the locks held by a session
*
* INPUTS:
*    sesid == session ID to check for any locks
*    retval == pointer to malloced and initialized NCX_BT_SLIST
*
* OUTPUTS:
*   *retval is filled in with any lock entryies or left empty
*   if none found
* 
*********************************************************************/
void
    cfg_get_lock_list (ses_id_t sesid,
		       val_value_t *retval)
{

    ncx_lmem_t *lmem;
    uint32      i;

#ifdef DEBUG
    if (!retval) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* dummy session ID does not bother with locks */
    if (!sesid) {
	return;
    }

    for (i=0; i<CFG_NUM_STATIC; i++) {
	if (cfg_arr[i] && cfg_arr[i]->locked_by == sesid) {
	    lmem = ncx_new_lmem();
	    if (lmem) {
		lmem->val.str = xml_strdup(cfg_arr[i]->name);
		if (lmem->val.str) {
		    ncx_insert_lmem(&retval->v.list, lmem, NCX_MERGE_LAST);
		} else {
		    ncx_free_lmem(lmem, NCX_BT_STRING);
		}
	    }
	}
    }

} /* cfg_get_lock_list */


/********************************************************************
* FUNCTION cfg_get_app_dataclass
*
* Get the data-class value for the specified application node
*
* INPUTS:
*    app == app header node to check
*
* RETURNS:
*    
*********************************************************************/
ncx_data_class_t
    cfg_get_app_dataclass (const cfg_app_t *app)
{
    const ps_parmset_t *ps;
    ncx_data_class_t    appclass;

#ifdef DEBUG
    if (!app) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NCX_DC_NONE;
    }
#endif

    appclass = NCX_DC_STATE;
    for (ps = (const ps_parmset_t *)dlq_firstEntry(&app->parmsetQ);
	 ps != NULL;
	 ps = (const ps_parmset_t *)dlq_nextEntry(ps)) {
	if (ps->psd->dataclass < appclass) {
	    appclass = ps->psd->dataclass;
	}
    }
    return appclass;

} /* cfg_get_app_dataclass */


/* END file cfg.c */
