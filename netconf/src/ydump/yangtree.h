/*
 * Copyright (c) 2018, Vladimir Vassilev, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */

/*  FILE: yangtree.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Convert YANG module to YANG tree format
*/

#include <xmlstring.h>
#include "ncxtypes.h"
#include "ses.h"
#include "status.h"
#include "yang.h"
#include "yangdump.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
* FUNCTION yangtree_convert_module
* 
*  Convert the YANG module to YANG tree format
*
* INPUTS:
*   pcb == parser control block of module to convert
*          This is returned from ncxmod_load_module_ex
*   cp == convert parms struct to use
*   scb == session to use for output
*
* RETURNS:
*   status
*********************************************************************/
extern status_t 
    yangtree_convert_module(const yang_pcb_t *pcb,
                              const yangdump_cvtparms_t *cp,
                              ses_cb_t *scb);

#ifdef __cplusplus
}  /* end extern 'C' */
#endif
