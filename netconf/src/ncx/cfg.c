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

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
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

#ifndef _H_xpath
#include  "xpath.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#define CFG_NUM_STATIC 3

#define CFG_DATETIME_LEN 64

#define MAX_CFGID   NCX_CFGID_STARTUP

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

    for (id = NCX_CFGID_RUNNING; id <= MAX_CFGID; id++) {
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
* FUNCTION free_template
*
* Clean and free the cfg_template_t struct
*
* INPUTS:
*    cfg = cfg_template_t to clean and free
* RETURNS:
*    none
*********************************************************************/
static void
    free_template (cfg_template_t *cfg)
{

    rpc_err_rec_t  *err;

#ifdef DEBUG
    if (!cfg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (cfg->root) {
	val_free_value(cfg->root);
    }

    if (cfg->name) {
	m__free(cfg->name);
    }
    if (cfg->src_url) {
	m__free(cfg->src_url);
    }
    if (cfg->load_time) {
	m__free(cfg->load_time);
    }
    if (cfg->lock_time) {
	m__free(cfg->lock_time);
    }
    if (cfg->last_ch_time) {
	m__free(cfg->last_ch_time);
    } 

    while (!dlq_empty(&cfg->load_errQ)) {
	err = (rpc_err_rec_t *)dlq_deque(&cfg->load_errQ);
	rpc_err_free_record(err);
    }

    m__free(cfg);

} /* free_template */


/********************************************************************
* FUNCTION new_template
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
static cfg_template_t *
    new_template (const xmlChar *name,
		  ncx_cfg_t cfg_id)
{
    ncx_module_t          *mod;
    cfg_template_t        *cfg;
    obj_template_t        *cfgobj;

    cfgobj = NULL;
    mod = ncx_find_module(NCX_EL_NETCONF, NULL);
    if (mod) {
	cfgobj = ncx_find_object(mod, NCX_EL_CONFIG);
    }
    if (!cfgobj) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

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

    cfg->lock_time = m__getMem(TSTAMP_MIN_SIZE);
    if (!cfg->lock_time) {
	m__free(cfg->name);
	m__free(cfg);
	return NULL;
    } else {
	memset(cfg->lock_time, 0x0, TSTAMP_MIN_SIZE);
    }

    cfg->cfg_id = cfg_id;
    cfg->cfg_state = CFG_ST_INIT;
    dlq_createSQue(&cfg->load_errQ);

    if (cfg_id != NCX_CFGID_CANDIDATE) {
	cfg->root = val_new_value();
	if (!cfg->root) {
	    free_template(cfg);
	    return NULL;
	}
	
	/* finish setting up the <config> root value */
	val_init_from_template(cfg->root, cfgobj);
    }  /* else root will be set next with val_clone_config */

    return cfg;

} /* new_template */


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
	for (id=NCX_CFGID_RUNNING; id <= MAX_CFGID; id++) {
	    if (cfg_arr[id]) {
		free_template(cfg_arr[id]);
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
    if (cfg_id > MAX_CFGID) {
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
    default:
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    cfg = new_template(name, cfg_id);
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
    if (cfg_id > MAX_CFGID) {
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
    if (cfg_id > MAX_CFGID) {
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

    if (cfgid <= MAX_CFGID) {
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
*
*********************************************************************/
void
    cfg_set_target (ncx_cfg_t cfg_id)
{
#ifdef DEBUG
    if (cfg_id > MAX_CFGID) {
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
* FUNCTION cfg_fill_candidate_from_running
*
* Fill the <candidate> config with the config contents
* of the <running> config
*
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_fill_candidate_from_running (void)
{
    cfg_template_t  *running, *candidate;
    status_t         res;

#ifdef DEBUG
    if (!cfg_arr[NCX_CFGID_RUNNING] ||
	!cfg_arr[NCX_CFGID_CANDIDATE]) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    running = cfg_arr[NCX_CFGID_RUNNING];
    candidate = cfg_arr[NCX_CFGID_CANDIDATE];

    if (!running->root) {
	return ERR_NCX_DATA_MISSING;
    }

    if (candidate->root) {
	val_free_value(candidate->root);
	candidate->root = NULL;
    }

    res = NO_ERR;
    candidate->root = 
	val_clone_config_data(running->root, &res);

    if (res == NO_ERR) {
	/* clear the candidate dirty flag */
	candidate->flags &= ~CFG_FL_DIRTY;
    }
    return res;

} /* cfg_fill_candidate_from_running */


/********************************************************************
* FUNCTION cfg_fill_candidate_from_startup
*
* Fill the <candidate> config with the config contents
* of the <startup> config
*
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_fill_candidate_from_startup (void)
{
    cfg_template_t  *startup, *candidate;
    status_t         res;

#ifdef DEBUG
    if (!cfg_arr[NCX_CFGID_CANDIDATE] ||
        !cfg_arr[NCX_CFGID_STARTUP]) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    startup = cfg_arr[NCX_CFGID_STARTUP];
    candidate = cfg_arr[NCX_CFGID_CANDIDATE];

    if (!startup->root) {
	return ERR_NCX_DATA_MISSING;
    }

    if (candidate->root) {
	val_free_value(candidate->root);
	candidate->root = NULL;
    }

    candidate->root = val_clone(startup->root);
    if (candidate->root == NULL) {
        res = ERR_INTERNAL_MEM;
    } else {
        res = NO_ERR;
    }

    /* clear the candidate dirty flag */
    candidate->flags &= ~CFG_FL_DIRTY;

    return res;

} /* cfg_fill_candidate_from_startup */


/********************************************************************
* FUNCTION cfg_fill_candidate_from_inline
*
* Fill the <candidate> config with the config contents
* of the <config> inline XML node
*
* INPUTS:
*   newroot == new root for the candidate config
*
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_fill_candidate_from_inline (val_value_t *newroot)
{
    cfg_template_t  *candidate;
    status_t         res;

#ifdef DEBUG
    if (newroot == NULL) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!cfg_arr[NCX_CFGID_CANDIDATE]) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    candidate = cfg_arr[NCX_CFGID_CANDIDATE];

    if (candidate->root) {
	val_free_value(candidate->root);
	candidate->root = NULL;
    }

    candidate->root = val_clone(newroot);
    if (candidate->root == NULL) {
        res = ERR_INTERNAL_MEM;
    } else {
        res = NO_ERR;
    }

    /* clear the candidate dirty flag */
    candidate->flags &= ~CFG_FL_DIRTY;

    return res;

} /* cfg_fill_candidate_from_inline */


/********************************************************************
* FUNCTION cfg_set_dirty_flag
*
* Mark the config as 'changed'
*
* INPUTS:
*    cfg == configuration template to set
*
*********************************************************************/
void
    cfg_set_dirty_flag (cfg_template_t *cfg)
{
#ifdef DEBUG
    if (!cfg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    cfg->flags |= CFG_FL_DIRTY;

}  /* cfg_set_dirty_flag */


/********************************************************************
* FUNCTION cfg_get_dirty_flag
*
* Get the config dirty flag value
*
* INPUTS:
*    cfg == configuration template to check
*
*********************************************************************/
boolean
    cfg_get_dirty_flag (const cfg_template_t *cfg)
{
#ifdef DEBUG
    if (!cfg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    return (cfg->flags & CFG_FL_DIRTY) ? TRUE : FALSE;

}  /* cfg_get_dirty_flag */


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
	if (cfg->cfg_id == NCX_CFGID_CANDIDATE) {
	    /* lock cannot be granted if any changes
	     * to the candidate are already made
	     */
	    res = (cfg_get_dirty_flag(cfg)) ? 
		ERR_NCX_CANDIDATE_DIRTY : NO_ERR;
	} else {
	    /* lock can be granted if state is ready */
	    res = NO_ERR;
	}
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

    res = NO_ERR;

    /* check if this is a writable target,
     * except during agent boot
     * except for the <startup> config
     */
    if (cfg->cfg_state != CFG_ST_INIT) {
	switch (cfg->cfg_id) {
	case NCX_CFGID_RUNNING:
	case NCX_CFGID_CANDIDATE:
	case NCX_CFGID_STARTUP:
	    break;
	default:
	    if (!(cfg->flags & CFG_FL_TARGET)) {
		res = ERR_NCX_NOT_WRITABLE;
	    }
	}	
    }

    if (res != NO_ERR) {
	return res;
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
* FUNCTION cfg_is_global_locked
*
* Check if the specified config has ab active global lock
*
* INPUTS:
*    cfg = Config template to check 
*
* RETURNS:
*    TRUE if global lock active, FALSE if not
*********************************************************************/
boolean
    cfg_is_global_locked (const cfg_template_t *cfg)
{

#ifdef DEBUG
    if (!cfg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    return (cfg->cfg_state == CFG_ST_FLOCK) ? TRUE : FALSE;

} /* cfg_is_global_locked */


/********************************************************************
* FUNCTION cfg_get_global_lock_info
*
* Get the current global lock info
*
* INPUTS:
*    cfg = Config template to check 
*    sid == address of return session ID
*    locktime == address of return locktime pointer
*
* OUTPUTS:
*    *sid == session ID of lock holder
*    *locktime == pointer to lock time string
*
* RETURNS:
*    status, NCX_ERR_SKIPPED if not locked
*********************************************************************/
status_t
    cfg_get_global_lock_info (const cfg_template_t *cfg,
			      ses_id_t  *sid,
			      const xmlChar **locktime)
{

#ifdef DEBUG
    if (!cfg || !sid || !locktime) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    *sid = 0;
    *locktime = NULL;

    if (cfg->cfg_state == CFG_ST_FLOCK) {
	*sid = cfg->locked_by;
	*locktime = cfg->lock_time;
	return NO_ERR;
    } else {
	return ERR_NCX_SKIPPED;
    }
    /*NOTREACHED*/

} /* cfg_get_global_lock_info */


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
	tstamp_datetime(cfg->lock_time);
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

        /* sec 8.3.5.2 requires a discard-changes
         * when a lock is released on the candidate
         */
        if (cfg->cfg_id == NCX_CFGID_CANDIDATE) {
            res = cfg_fill_candidate_from_running();
        }
    }

    return res;

} /* cfg_unlock */


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
    status_t        res;

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

            /* sec 8.3.5.2 requires a discard-changes
             * when a lock is released on the candidate
             */
            if (cfg->cfg_id == NCX_CFGID_CANDIDATE) {
                res = cfg_fill_candidate_from_running();
                if (res != NO_ERR) {
                    log_error("\nError: discard-changes failed (%s)",
                              get_error_string(res));
                }
            }
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
* FUNCTION cfg_find_datanode
*
* Find the specified data node instance,
* using absolute path XPath and default prefix names.
* A missing prefix is an error
* The expression must start from root
*
* INPUTS:
*   target == XPath expression for single target to find
*   cfgid == ID of configuration to use
*
* RETURNS:
*   pointer to found node, or NULL if not found
*********************************************************************/
val_value_t *
    cfg_find_datanode (const xmlChar *target,
		       ncx_cfg_t  cfgid)
{
    cfg_template_t  *cfg;
    val_value_t     *retval;
    status_t         res;

#ifdef DEBUG
    if (!target) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    cfg = cfg_get_config_id(cfgid);
    if (!cfg) {
	return NULL;
    }

    res = xpath_find_val_target(cfg->root, NULL,
				target, &retval);
    if (res == NO_ERR) {
	return retval;
    } else {
	return NULL;
    }
    

} /* cfg_find_datanode */


/********************************************************************
* FUNCTION cfg_find_modrel_datanode
*
* Find the specified data node instance,
* using absolute path XPath and module-relative prefix names.
* A missing prefix is defaulted to the specified module
* The expression must start from root
*
* INPUTS:
*   mod  == module to use for the default and prefix evaluation
*   target == XPath expression for single target to find
*   cfgid == ID of configuration to use
*
* RETURNS:
*   pointer to found node, or NULL if not found
*********************************************************************/
val_value_t *
    cfg_find_modrel_datanode (ncx_module_t *mod,
			      const xmlChar *target,
			      ncx_cfg_t  cfgid)
{
    cfg_template_t  *cfg;
    val_value_t     *retval;
    status_t         res;

#ifdef DEBUG
    if (!target) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    retval = NULL;
    cfg = cfg_get_config_id(cfgid);
    if (!cfg) {
	return NULL;
    }

    res = xpath_find_val_target(cfg->root, mod,
				target, &retval);
    if (res == NO_ERR) {
	return retval;
    } else {
	return NULL;
    }

} /* cfg_find_modrel_datanode */


/********************************************************************
* FUNCTION cfg_apply_load_root
*
* Apply the AGT_CB_APPLY function for the OP_EDITOP_LOAD operation
*
* INPUTS:
*    cfg == config target
*    newroot == new config tree
*
* RETURNS:
*    status
*********************************************************************/
status_t
    cfg_apply_load_root (cfg_template_t *cfg,
			 val_value_t *newroot)
{
    if (cfg->root && val_child_cnt(cfg->root)) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    } else if (cfg->root) {
        val_free_value(cfg->root);
        cfg->root = NULL;
        cfg->root = newroot;
    } else {
        cfg->root = newroot;
    }

    /* set the load_time and last_ch_time timestamps */
    cfg->load_time = new_cur_datetime();
    if (cfg->load_time) {
	cfg->last_ch_time = xml_strdup(cfg->load_time);
	if (!cfg->last_ch_time) {
	    return ERR_INTERNAL_MEM;
	}
    } else {
	return ERR_INTERNAL_MEM;
    }

    return NO_ERR;

} /* cfg_apply_load_root */


/* END file cfg.c */
