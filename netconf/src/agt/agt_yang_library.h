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
/*  FILE: agt_yang_library.h

   NETCONF ietf-yang-library.yang Data Model defs: Server Side Support

*/

#include "dlq.h"
#include "ncxtypes.h"
#include "obj.h"
#include "ses.h"
#include "status.h"
#include "tstamp.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

/********************************************************************
* FUNCTION agt_yang_library_init
*
* INIT 1:
*   Initialize the yang library data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
extern status_t
    agt_yang_library_init (void);


/********************************************************************
* FUNCTION agt_yang_library_init2
*
* INIT 2:
*   Initialize the yang library data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
extern status_t
    agt_yang_library_init2 (void);

/********************************************************************
* FUNCTION agt_yang_library_cleanup
*
* Cleanup the module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
extern void 
    agt_yang_library_cleanup (void);


#ifdef __cplusplus
}  /* end extern 'C' */
#endif

/* END file agt_yang_library.h */

