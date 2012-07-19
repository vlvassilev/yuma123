/*
 * Copyright (c) 2008 - 2012, Andy Bierman, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_agt_commit_validate_cb
#define _H_agt_commit_validate_cb

/*  FILE: agt_cb.h
*********************************************************************
* P U R P O S E
*********************************************************************
    NETCONF Server Commit Validate callback handler
    This file contains functions to support registering, 
    unregistering and execution of commit validate callbacks. 

*********************************************************************
* C H A N G E	 H I S T O R Y
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
24-oct-11    mp       First draft.
*/

#include <xmlstring.h>

#include "procdefs.h"
#include "status.h"

#include "procdefs.h"
#include "agt.h"
#include "agt_acm.h"
#include "agt_cap.h"
#include "agt_cb.h"
#include "agt_cfg.h"
#include "agt_ncx.h"
#include "agt_util.h"
#include "agt_val.h"
#include "agt_val_parse.h"
#include "cap.h"
#include "cfg.h"
#include "dlq.h"
#include "log.h"
#include "ncx.h"
#include "ncxconst.h"
#include "obj.h"
#include "op.h"
#include "plock.h"
#include "rpc.h"
#include "rpc_err.h"
#include "status.h"
#include "typ.h"
#include "tstamp.h"
#include "val.h"
#include "val_util.h"
#include "xmlns.h"
#include "xpath.h"
#include "xpath_yang.h"
#include "xpath1.h"
#include "yangconst.h"


#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
* T Y P E D E F S
*********************************************************************/
/** Typedef of the commit_validate callback */
typedef status_t (*agt_commit_validate_cb_t)(agt_profile_t *profile, ses_cb_t *scb, xml_msg_hdr_t *msghdr, val_value_t *root);

/********************************************************************
* F U N C T I O N S
*********************************************************************/

/**
 * Initialise the callback commit module.
 */
extern void agt_commit_validate_init( void );

/**
 * Cleanup the callback commit module.
 */
extern void agt_commit_validate_cleanup( void );

/**
 * Register a commit validate callback.
 * This function registers a commit-validate callback that will be
 * called after all changes to the candidate database have been
 * validated. The function will be called before that final SIL commit
 * operation. If a commit validate operation is already registered for
 * the module it will be replaced.
 *
 * \param modname the name of the module registering the callback
 * \param cb the commit complete function.
 * \return the status of the operation.
 */
extern status_t agt_commit_validate_register( const xmlChar *modname,
                                              agt_commit_validate_cb_t cb );

/**
 * Unregister a commit validate callback.
 * This function unregisters a commit-validate callback.
 *
 * \param modname the name of the module unregistering the callback
 */
extern void agt_commit_validate_unregister( const xmlChar *modname );

/**
 * Validate a commit operation.
 * This function simply calls each registered commit validate
 * callback. If a commit validate operation fails the status of the
 * failing operation is returned immediately and no further commit
 * validate callbacks are made.
 *
 * \return the ERR_OK or the status of the first failing callback.
 */
extern status_t agt_commit_validate( agt_profile_t *profile, ses_cb_t *scb, xml_msg_hdr_t *msghdr, val_value_t *root );

#ifdef __cplusplus
}  /* end extern 'C' */
#endif

#endif // _H_agt_commit_validate_cb

