/*
 * Copyright (c) 2017, Vladimir Vassilev, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */


#include <xmlstring.h>

#include "ses.h"
#include "status.h"
#include "yangcli.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
*								    *
*		      F U N C T I O N S 			    *
*								    *
*********************************************************************/

extern status_t
    yang_library_start_get_module_set (server_cb_t *server_cb,
                                ses_cb_t *scb);
extern status_t
    yang_library_handle_rpc_reply (server_cb_t *server_cb,
                               ses_cb_t *scb,
                               val_value_t *reply,
                               boolean anyerrors);

status_t make_get_yang_library_modules_state_reqdata(server_cb_t *server_cb,
                              ses_cb_t *scb,
                              obj_template_t** out_rpc,
                              val_value_t** out_reqdata);

status_t get_yang_library_modules_state_reply_to_searchresult_entries(server_cb_t * server_cb,
                              ses_cb_t *scb,
                              val_value_t* reply);

status_t get_yang_library_modules_state_reply_to_searchresult_entries(server_cb_t * server_cb,
                              ses_cb_t *scb,
                              val_value_t* reply);

#ifdef __cplusplus
}  /* end extern 'C' */
#endif

