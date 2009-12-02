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
#ifndef _H_agt_state
#define _H_agt_state
/*  FILE: agt_state.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   NETCONF State Monitoring Data Model Module support

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
24-feb-09    abb      Begun.
*/

#include <xmlstring.h>

#ifndef _H_agt_not
#include "agt_not.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif


/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

#define AGT_STATE_MODULE      (const xmlChar *)"ietf-netconf-monitoring"

/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern status_t
    agt_state_init (void);

extern status_t
    agt_state_init2 (void);

extern void 
    agt_state_cleanup (void);

extern status_t
    agt_state_add_session (ses_cb_t *scb);

extern void
    agt_state_remove_session (ses_id_t sid);

extern status_t
    agt_state_add_module_schema (ncx_module_t *mod);

extern status_t
    agt_state_add_subscription (agt_not_subscription_t *sub);

extern void
    agt_state_remove_subscription (ses_id_t sid);


#endif            /* _H_agt_state */
