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

/* this is an arbitrary limit, but it must match the
 * yangcli:run rpc P1 - Pn variables, currently set to 9
 * $1 to $9 parameters passed by yangcli to next script
 */
#define RUNSTACK_MAX_PARMS  9


/* this is an arbitrary limit to limit resources
 * and run-away scripts that are called recursively
 */
#define RUNSTACK_MAX_NEST   512

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* one script run level context entry
 * each time a 'run script' command is
 * encountered, a new stack context is created,
 * unless max_script_level is reached
 */
typedef struct runstack_entry_t_ {
    dlq_hdr_t    qhdr;
    uint32       level;
    FILE        *fp;
    xmlChar     *source;
    xmlChar     *buff;
    int          bufflen;
    uint32       linenum;
    dlq_hdr_t    parmQ;         /* Q of ncx_var_t */
    dlq_hdr_t    varQ;          /* Q of ncx_var_t */
} runstack_entry_t;


typedef struct runstack_context_ {
    xmlChar           *name;
    boolean            script_cancel;
    uint32             script_level;
    uint32             max_script_level;
    dlq_hdr_t          runstackQ;    /* Q of runstack_entry_t */
    dlq_hdr_t          globalQ;             /* Q of ncx_var_t */
    dlq_hdr_t          zeroQ;               /* Q of ncx_var_t */
} runstack_context_t;


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
* INPUTS:
*    rcxt == runstack context to use
*
* RETURNS:
*   current stack level; 0 --> not in any script
*********************************************************************/
extern uint32
    runstack_level (runstack_context_t *rcxt);


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
*   rcxt == runstack context to use
*   source == file source
*   fp == file pointer
*
* RETURNS:
*   status
*********************************************************************/
extern status_t
    runstack_push (runstack_context_t *rcxt,
                   const xmlChar *source,
		   FILE *fp);

/********************************************************************
* FUNCTION runstack_pop
* 
*  Remove a script nest level context from the stack
*  Call just after script is completed
* 
* INPUTS:
*     rcxt == runstack context to use
*********************************************************************/
extern void
    runstack_pop (runstack_context_t *rcxt);


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
*   rcxt == runstack context to use
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
    runstack_get_cmd (runstack_context_t *rcxt,
                      status_t *res);


/********************************************************************
* FUNCTION runstack_cancel
* 
*  Cancel all running scripts
*
* INPUTS:
*     rcxt == runstack context to use
*********************************************************************/
extern void
    runstack_cancel (runstack_context_t *rcxt);


/********************************************************************
* FUNCTION runstack_get_que
* 
*  Read the current runstack context and figure
*  out which queue to get
*
* INPUTS:
*   rcxt == runstack context to use
*   isglobal == TRUE if global queue desired
*            == FALSE if the runstack var que is desired
*
* RETURNS:
*   pointer to the requested que
*********************************************************************/
extern dlq_hdr_t *
    runstack_get_que (runstack_context_t *rcxt,
                      boolean isglobal);


/********************************************************************
* FUNCTION runstack_get_parm_que
* 
*  Get the parameter queue for the current stack level
*
* INPUTS:
*   rcxt == runstack context to use
*
* RETURNS:
*   que for current stack level, NULL if an error
*********************************************************************/
extern dlq_hdr_t *
    runstack_get_parm_que (runstack_context_t *rcxt);


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
* FUNCTION runstack_clean_context
* 
*  INPUTS:
*     rcxt == runstack context to clean, but not free
*
*********************************************************************/
extern void
    runstack_clean_context (runstack_context_t *rcxt);


/********************************************************************
* FUNCTION runstack_free_context
* 
*  INPUTS:
*     rcxt == runstack context to free
*
*********************************************************************/
extern void
    runstack_free_context (runstack_context_t *rcxt);


/********************************************************************
* FUNCTION runstack_init_context
* 
* Initialize a pre-malloced runstack context
*
*  INPUTS:
*     rcxt == runstack context to free
*
*********************************************************************/
extern void
    runstack_init_context (runstack_context_t *rcxt);


/********************************************************************
* FUNCTION runstack_new_context
* 
*  Malloc a new runstack context
*
*  RETURNS:
*     malloced and initialized runstack context
*
*********************************************************************/
extern runstack_context_t *
    runstack_new_context (void);


/********************************************************************
* FUNCTION runstack_session_cleanup
* 
* Cleanup after a yangcli session has ended
*
*  INPUTS:
*     rcxt == runstack context to use
*
*********************************************************************/
extern void
    runstack_session_cleanup (runstack_context_t *rcxt);


#endif	    /* _H_runstack */
