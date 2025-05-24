/*
 * Copyright (c) 2013 - 2016, Vladimir Vassilev, All Rights Reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "val.h"

typedef void* yangrpc_cb_ptr_t;

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/********************************************************************
* FUNCTION yangrpc_init
*
* Function initializing the Yuma environment
*
* MUST BE CALLED BEFORE yangrpc_connect calls
*
* INPUTS:
*   args == space separated list of arguments e.g. "--log-level=debug --display-mode=xml"
*
* RETURNS:
*   status
*********************************************************************/
status_t yangrpc_init(char* args);

/********************************************************************
* FUNCTION yangrpc_connect
*
* Function connecting to YANG modeled device
*
* MUST CALL yangrpc_init FIRST
*
* INPUTS:
*   server == netconf server address
*   ncport == netconf server port
*   user == user name
*   password == plaintext password
*   public_key == path to the public key to be used
*   private_key == path to the private key to be used
*   extra_args == string with extra arguments e.g.
*     "--timeout=30 --dump-session=/tmp/mysession-dump-"
* OUTPUTS:
*   yangrpc_cb == returns pointer to control block with the yangrpc session context
*
* RETURNS:
*   status
*********************************************************************/
status_t yangrpc_connect(const char * const server, uint16_t port,
                         const char * const user,
                         const char * const password,
                         const char * const public_key,
                         const char * const private_key,
                         const char * const extra_args,
                         yangrpc_cb_ptr_t* yangrpc_cb_ptr);

/********************************************************************
* FUNCTION yangrpc_parse_cli
*
* Function parsing RPC encoded in yangcli syntax e.g. "xget /"
* and returning corresponding request_val
*
* INPUTS:
*   server == netconf server address
*   ncport == netconf server port
*   user == user name
*   password == plaintext password
*   public_key == path to the public key to be used
*   private_key == path to the private key to be used
*   extra_args == string with extra arguments e.g.
*     "--timeout=30 --dump-session=/tmp/mysession-dump-"
* OUTPUTS:
*   yangrpc_cb_ptr == returns pointer to control block with the yangrpc session context
*
* RETURNS:
*   status
*********************************************************************/
status_t yangrpc_parse_cli(yangrpc_cb_ptr_t yangrpc_cb_ptr,
                           const char * const cli_cmd_str,
                           val_value_t** request_val);

/********************************************************************
* FUNCTION yangrpc_exec
*
* Function sending the RPC request specified in request_val and returns the RPC reply in reply_val
*
* INPUTS:
*   yangrpc_cb_ptr == control block pointer
*   request_val == RPC request value pointer
* OUTPUTS:
*   reply_val == pointer to RPC reply value pointer
*
* RETURNS:
*   status
*********************************************************************/
status_t yangrpc_exec(yangrpc_cb_ptr_t yangrpc_cb_ptr, val_value_t* request_val, val_value_t** reply_val);

/********************************************************************
* FUNCTION yangrpc_close
*
* Function closing yangrpc session
*
* INPUTS:
*   yangrpc_cb_ptr == control block pointer
*
*********************************************************************/
void yangrpc_close(yangrpc_cb_ptr_t yangrpc_cb_ptr);

/********************************************************************
* FUNCTION yangrpc_cleanup
*
* Function for cleanup of global resources - called last
*
*
*********************************************************************/
void yangrpc_cleanup(void);

