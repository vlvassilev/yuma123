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
#ifndef _H_runstack
#define _H_runstack

/*  FILE: runstack.h
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
22-aug-07    abb      Begun; split from ncxcli.c

*/

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

#define RUNSTACK_MAX_PARMS  9


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/********************************************************************
* FUNCTION runstack_level
* 
*  Get the current stack level
*
* RETURNS:
*   current stack level; 0 --> not in any script
*********************************************************************/
extern uint32
    runstack_level (void);


/********************************************************************
* FUNCTION runstack_push
* 
*  Add a script nest level context to the stack
*  Call just after the file is opened and before
*  the first line has been read
* 
*  Still need to add the script parameters (if any)
*  with the runstack_setparm
*
* INPUTS:
*   source == file source
*   fp == file pointer
*
* RETURNS:
*   status
*********************************************************************/
extern status_t
    runstack_push (const xmlChar *source,
		   FILE *fp);

/********************************************************************
* FUNCTION runstack_pop
* 
*  Remove a script nest level context from the stack
*  Call just after script is completed
* 
*********************************************************************/
extern void
    runstack_pop (void);


/********************************************************************
* FUNCTION runstack_get_cmd
* 
*  Read the current runstack context and construct
*  a command string for processing by do_run_script.
*    - Comment lines will be skipped.
*    - Extended lines will be concatenated in the
*      buffer.  If a buffer overflow occurs due to this
*      concatenation, an error will be returned
* 
* INPUTS:
*   res == address of status result
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to the command line to process (should treat as CONST !!!)
*   NULL if some error
*********************************************************************/
extern xmlChar *
    runstack_get_cmd (status_t *res);


/********************************************************************
* FUNCTION runstack_cancel
* 
*  Cancel all running scripts
*
*********************************************************************/
extern void
    runstack_cancel (void);


/********************************************************************
* FUNCTION runstack_get_que
* 
*  Read the current runstack context and figure
*  out which queue to get
*
* INPUTS:
*   isglobal == TRUE if global queue desired
*            == FALSE if the runstack var que is desired
*
* RETURNS:
*   pointer to the requested que
*********************************************************************/
extern dlq_hdr_t *
    runstack_get_que (boolean isglobal);


/********************************************************************
* FUNCTION runstack_get_parm_que
* 
*  Get the parameter queue for the current stack level
*
* RETURNS:
*   que for current stack level, NULL if an error
*********************************************************************/
extern dlq_hdr_t *
    runstack_get_parm_que (void);


/********************************************************************
* FUNCTION runstack_init
* 
*  Must Init this module before using it!!!
*
*********************************************************************/
extern void
    runstack_init (void);


/********************************************************************
* FUNCTION runstack_cleanup
* 
*  Must cleanup this module after using it!!!
*
*********************************************************************/
extern void
    runstack_cleanup (void);


/********************************************************************
* FUNCTION runstack_session_cleanup
* 
* Cleanup after a yangcli session has ended
*
*********************************************************************/
extern void
    runstack_session_cleanup (void);

#endif	    /* _H_runstack */
