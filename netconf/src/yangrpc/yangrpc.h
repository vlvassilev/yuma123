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

typedef struct dummy {int dummy;} yangrpc_cb_t;

yangrpc_cb_t* yangrpc_connect(char* server, uint16_t port, char* user, char* password, char* publick_key, char* private_key);
status_t yangrpc_exec(yangrpc_cb_t *server_cb, val_value_t* request_val, val_value_t** reply_val);
void yangrpc_close(yangrpc_cb_t *yangrpc_cb);
