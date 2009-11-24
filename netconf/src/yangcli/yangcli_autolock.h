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
#ifndef _H_yangcli_autolock
#define _H_yangcli_autolock

/*  FILE: yangcli_autolock.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
13-augr-09    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_yangcli
#include "yangcli.h"
#endif


/********************************************************************
*								    *
*		      F U N C T I O N S 			    *
*								    *
*********************************************************************/
extern status_t
    handle_get_locks_request_to_server (server_cb_t *server_cb,
                                        boolean first,
                                        boolean *done);

extern status_t
    handle_release_locks_request_to_server (server_cb_t *server_cb,
                                            boolean first,
                                            boolean *done);

extern void
    handle_locks_cleanup (server_cb_t *server_cb);


extern boolean
    check_locks_timeout (server_cb_t *server_cb);

extern status_t
    send_discard_changes_pdu_to_server (server_cb_t *server_cb);


extern status_t
    do_get_locks (server_cb_t *server_cb,
                  obj_template_t *rpc,
                  const xmlChar *line,
                  uint32  len);

extern status_t
    do_release_locks (server_cb_t *server_cb,
                      obj_template_t *rpc,
                      const xmlChar *line,
                      uint32  len);

extern void
    clear_lock_cbs (server_cb_t *server_cb);

#endif	    /* _H_yangcli_autolock */
