/*
 * Copyright (c) 2024, Vladimir Vassilev, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_agt123_fd_event_cb
#define _H_agt123_fd_event_cb

/*  FILE: agt123_fd_event_cb.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

Registration of file descriptor event callbacks to the server select

*/

#include "status.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* file descriptor event callback function
 *
 * Process the file descriptor event
 *
 * INPUTS:
 *    fd == file descriptor
 *
 * RETURNS:
 *     0 == normal exit
 *    -1 == error exit, unregister callback upon return
 */
typedef int (*agt_fd_event_cb_fn_t) (int fd);


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/********************************************************************
 * FUNCTION agt_fd_event_cb_register
 * 
 * Register event file descriptor in the select loop with
 * corresponding callback
 * 
 * INPUTS:
 *   fd == file descriptor number
 *   cb_fn == callback function pointer
 *********************************************************************/
extern void
    agt_fd_event_cb_register(int fd, agt_fd_event_cb_fn_t cb_fn);

/********************************************************************
 * FUNCTION agt_fd_event_cb_unregister
 * 
 * Unregister event file descriptor from the select loop
 * 
 * INPUTS:
 *   fd == file descriptor number
 *********************************************************************/
extern void
    agt_fd_event_cb_unregister(int fd);

#ifdef __cplusplus
}  /* end extern 'C' */
#endif

#endif	    /* _H_agt_fd_event_cb */

