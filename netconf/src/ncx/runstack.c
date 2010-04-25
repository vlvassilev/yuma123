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
/*  FILE: runstack.c

    Simple stack of script execution contexts
    to support nested 'run' commands within scripts

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
22-aug-07    abb      begun; split from ncxcli.c


*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _H_procdefs
#include "procdefs.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_runstack
#include "runstack.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_var
#include "var.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define RUNSTACK_DEBUG   1
#endif


#define RUNSTACK_BUFFLEN  32000

/********************************************************************
*                                                                   *
*                            T Y P E S                              *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/

/* nested run command support */
static boolean            runstack_init_done = FALSE;
static runstack_context_t defcxt;


/********************************************************************
* FUNCTION make_var
* 
* 
*  Still need to add the script parameters (if any)
*  with the runstack_setparm
*
* INPUTS:
*   source == file source
*   fp == file pointer
*
* RETURNS:
*   pointer to new val struct, NULL if an error
*********************************************************************/
static val_value_t *
    make_parmval (const xmlChar *name,
                  const xmlChar *value)
{
    val_value_t    *val;
    status_t        res;

    /* create the parameter */
    val = val_new_value();
    if (!val) {
        return NULL;
    }

    /* set the string value */
    res = val_set_string(val, name, value);
    if (res != NO_ERR) {
        val_free_value(val);
        return NULL;
    }
    return val;

}  /* make_parmval */


/********************************************************************
* FUNCTION free_stack_entry
* 
* Clean and free a runstack entry
*
* INPUTS:
*   se == stack entry to free
*
* INPUTS:
*    se == stack entry to free
*********************************************************************/
static void
    free_stack_entry (runstack_entry_t *se)
{
    ncx_var_t  *var;

    if (se->buff) {
        m__free(se->buff);
    }
    if (se->fp) {
        fclose(se->fp);
    }
    if (se->source) {
        m__free(se->source);
    }
    while (!dlq_empty(&se->parmQ)) {
        var = (ncx_var_t *)dlq_deque(&se->parmQ);
        var_free(var);
    }
    while (!dlq_empty(&se->varQ)) {
        var = (ncx_var_t *)dlq_deque(&se->varQ);
        var_free(var);
    }
    m__free(se);

}  /* free_stack_entry */


/********************************************************************
* FUNCTION new_stack_entry
* 
* Malloc and init a new runstack entry
*
* INPUTS:
*   source == file source
*
* RETURNS:
*   pointer to new val struct, NULL if an error
*********************************************************************/
static runstack_entry_t *
    new_stack_entry (const xmlChar *source)
{
    runstack_entry_t *se;

    se = m__getObj(runstack_entry_t);
    if (se == NULL) {
        return NULL;
    }

    memset(se, 0x0, sizeof(runstack_entry_t));

    dlq_createSQue(&se->parmQ);
    dlq_createSQue(&se->varQ);

    /* get a new line input buffer */
    se->buff = m__getMem(RUNSTACK_BUFFLEN);
    if (!se->buff) {
        free_stack_entry(se);
        return NULL;
    }

    /* set the script source */
    se->source = xml_strdup(source);
    if (!se->source) {
        free_stack_entry(se);
        return NULL;
    }

    return se;

}  /* new_stack_entry */





/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION runstack_level
* 
*  Get the current stack level

* INPUTS:
*    rcxt == runstack context to use
*
* RETURNS:
*   current stack level; 0 --> not in any script
*********************************************************************/
extern uint32
    runstack_level (runstack_context_t *rcxt)
{
    if (rcxt == NULL) {
        rcxt = &defcxt;
    }
    return rcxt->script_level;

}  /* runstack_level */


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
status_t
    runstack_push (runstack_context_t *rcxt,
                   const xmlChar *source,
                   FILE *fp)
{
    runstack_entry_t  *se;
    val_value_t       *val;
    status_t           res;

#ifdef DEBUG
    if (source == NULL) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (rcxt == NULL) {
        rcxt = &defcxt;
    }

    /* check if this would overflow the runstack */
    if (rcxt->script_level+1 == rcxt->max_script_level) {
        return ERR_NCX_RESOURCE_DENIED;
    }

    se = new_stack_entry(source);
    if (se == NULL) {
        return ERR_INTERNAL_MEM;
    }

    se->fp = fp;
    se->bufflen = RUNSTACK_BUFFLEN;

    /* create the P0 parameter */
    val = make_parmval((const xmlChar *)"0", source);
    if (!val) {
        free_stack_entry(se);
        return ERR_INTERNAL_MEM;
    }

    /* need to increment script level now so var_set_move
     * will work, and the correct script parameter queue
     * will be used for the P0 parameter
     */
    dlq_enque(se, &rcxt->runstackQ);
    rcxt->script_level++;

    /* ceate a new var entry and add it to the runstack que */
    res = var_set_move(rcxt,
                       (const xmlChar *)"0", 
                       1, 
                       VAR_TYP_LOCAL, 
                       val);
    if (res != NO_ERR) {
        val_free_value(val);
        dlq_remove(se);
        free_stack_entry(se);
        rcxt->script_level--;
        return res;
    }

    if (LOGDEBUG) {
        log_debug("\nrunstack: Starting level %u script %s",
                  rcxt->script_level, 
                  source);
    }

    return NO_ERR;

}  /* runstack_push */


/********************************************************************
* FUNCTION runstack_pop
* 
*  Remove a script nest level context from the stack
*  Call just after script is completed
* 
* INPUTS:
*     rcxt == runstack context to use
*********************************************************************/
void
    runstack_pop (runstack_context_t *rcxt)
{
    runstack_entry_t  *se;

    if (rcxt == NULL) {
        rcxt = &defcxt;
    }

    if (rcxt->script_level == 0) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }

    se = (runstack_entry_t *)dlq_lastEntry(&rcxt->runstackQ);
    if (se == NULL) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }

    dlq_remove(se);

    if (se->source && LOGDEBUG) {
        log_debug("\nrunstack: Ending level %u script %s",
                  rcxt->script_level, 
                  se->source);
    }
    
    free_stack_entry(se);

    rcxt->script_level--;

}  /* runstack_pop */


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
xmlChar *
    runstack_get_cmd (runstack_context_t *rcxt,
                      status_t *res)
{
    runstack_entry_t  *se;
    xmlChar           *retstr, *start, *str;
    boolean            done;
    int                len, total;

#ifdef DEBUG
    if (res == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    if (rcxt == NULL) {
        rcxt = &defcxt;
    }

    if (rcxt->script_level == 0) {
        *res = SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }

    /* init locals */
    se = (runstack_entry_t *)dlq_lastEntry(&rcxt->runstackQ);
    if (se == NULL) {
        *res = SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }

    retstr = NULL;  /* start of return string */
    start = se->buff;
    total = 0;
    done = FALSE;

    if (rcxt->script_cancel) {
        if (LOGINFO) {
            log_info("\nScript '%s' canceled", se->source);
        }
        done = TRUE;
        if (rcxt->script_level <= 1) {
            rcxt->script_cancel = FALSE;
        }
    }

    /* get a command line, handling comment and continuation lines */
    while (!done) {

        /* check overflow error */
        if (total==se->bufflen) {
            *res = ERR_BUFF_OVFL;
            /* do not allow truncated command to execute */
            retstr = NULL;
            done = TRUE;
            continue;
        }

        /* read the next line from the file */
        if (!fgets((char *)start, se->bufflen-total, se->fp)) {

            int errint = feof(se->fp);

            if (retstr) {
                *res = NO_ERR;
            } else if (errint) {
                *res = ERR_NCX_EOF;
            } else {
                *res = ERR_NCX_READ_FAILED;
            }
            done = TRUE;
            continue;
        }

        se->linenum++;  /* used for error messages */

        len = (int)xml_strlen(start);
        
        /* get rid of EOLN if present */
        if (len && start[len-1]=='\n') {
            start[--len] = 0;
        }

        /* check blank line */
        str = start;
        while (*str && xml_isspace(*str)) {
            str++;
        }
        if (!*str) {
            if (retstr) {
                /* return the string we have so far */
                *res = NO_ERR;
                done = TRUE;
            } else {
                /* try again */
                continue;
            }
        }
                
        /* check first line or line continuation in progress */
        if (!retstr) {
            /* retstr not set yet, allowed to have a comment line here */
            if (*str == '#') {
                /* got a comment, try for another line */
                *start = 0;
                continue;
            } else {
                /* start the return string and keep going */
                str = start;
                retstr = start;
            }
        }

        /* check line continuation */

        if (len && start[len-1]=='\\') {
            /* get rid of the final backslash */
            total += len-1;
            start[len-1] = 0;
            start += len-1;
            /* get another line full */
        } else {
            *res = NO_ERR;
            done = TRUE;
        }
    }

    if (!retstr) {
        runstack_pop(rcxt);
    } else if (LOGDEBUG) {
        log_debug("\nrunstack: run line %u, %s\n cmd: %s",
                  se->linenum, 
                  se->source, 
                  retstr);
    }

    return retstr;

}  /* runstack_get_cmd */


/********************************************************************
* FUNCTION runstack_cancel
* 
*  Cancel all running scripts
*
* INPUTS:
*     rcxt == runstack context to use
*********************************************************************/
void
    runstack_cancel (runstack_context_t *rcxt)
{
    if (rcxt == NULL) {
        rcxt = &defcxt;
    }
    if (rcxt->script_level) {
        rcxt->script_cancel = TRUE;
    }
}  /* runstack_cencel */


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
dlq_hdr_t *
    runstack_get_que (runstack_context_t *rcxt,
                      boolean isglobal)
{
    runstack_entry_t  *se;

    if (rcxt == NULL) {
        rcxt = &defcxt;
    }

    /* check global que */
    if (isglobal) {
        return &rcxt->globalQ;
    }

    /* check level zero local que */
    if (rcxt->script_level == 0) {
        return &rcxt->zeroQ;
    }

    /* get the current slot */
    se = (runstack_entry_t *)dlq_lastEntry(&rcxt->runstackQ);
    if (se == NULL) {
        return NULL;
    }
    return &se->varQ;

}  /* runstack_get_que */


/********************************************************************
* FUNCTION runstack_get_parm_que
* 
*  Get the parameter queue for the current stack level
*
* INPUTS:
*     rcxt == runstack context to use
*
* RETURNS:
*   que for current stack level, NULL if an error
*********************************************************************/
dlq_hdr_t *
    runstack_get_parm_que (runstack_context_t *rcxt)
{
    runstack_entry_t  *se;

    if (rcxt == NULL) {
        rcxt = &defcxt;
    }

    if (rcxt->script_level == 0) {
        return NULL;
    }

    /* get the current slot */
    se = (runstack_entry_t *)dlq_lastEntry(&rcxt->runstackQ);
    if (se == NULL) {
        return NULL;
    }

    return &se->parmQ;

}  /* runstack_get_parm_que */


/********************************************************************
* FUNCTION runstack_init
* 
*  Must Init this module before using it!!!
*
*********************************************************************/
void
    runstack_init (void)
{
    if (!runstack_init_done) {
        runstack_init_context(&defcxt);
        runstack_init_done = TRUE;
    }
} /* runstack_init */                 


/********************************************************************
* FUNCTION runstack_cleanup
* 
*  Must cleanup this module after using it!!!
*
*********************************************************************/
void
    runstack_cleanup (void)
{
    if (runstack_init_done) {
        runstack_clean_context(&defcxt);
        runstack_init_done = FALSE;
    }
} /* runstack_cleanup */                      


/********************************************************************
* FUNCTION runstack_clean_context
* 
*  INPUTS:
*     rcxt == runstack context to clean, but not free
*
*********************************************************************/
void
    runstack_clean_context (runstack_context_t *rcxt)
{
    ncx_var_t *var;

#ifdef DEBUG
    if (rcxt == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    while (rcxt->script_level > 0) {
        runstack_pop(rcxt);
    }

    while (!dlq_empty(&rcxt->globalQ)) {
        var = (ncx_var_t *)dlq_deque(&rcxt->globalQ);
        var_free(var);
    }

    while (!dlq_empty(&rcxt->zeroQ)) {
        var = (ncx_var_t *)dlq_deque(&rcxt->zeroQ);
        var_free(var);
    }

} /* runstack_clean_context */ 


/********************************************************************
* FUNCTION runstack_free_context
* 
*  INPUTS:
*     rcxt == runstack context to free
*
*********************************************************************/
void
    runstack_free_context (runstack_context_t *rcxt)
{
#ifdef DEBUG
    if (rcxt == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    runstack_clean_context(rcxt);
    m__free(rcxt);

} /* runstack_free_context */ 


/********************************************************************
* FUNCTION runstack_init_context
* 
* Initialize a pre-malloced runstack context
*
*  INPUTS:
*     rcxt == runstack context to free
*
*********************************************************************/
void
    runstack_init_context (runstack_context_t *rcxt)
{
#ifdef DEBUG
    if (rcxt == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    memset(rcxt, 0x0, sizeof(runstack_context_t));
    dlq_createSQue(&rcxt->globalQ);
    dlq_createSQue(&rcxt->zeroQ);
    dlq_createSQue(&rcxt->runstackQ);
    rcxt->max_script_level = RUNSTACK_MAX_NEST;

} /* runstack_init_context */ 


/********************************************************************
* FUNCTION runstack_new_context
* 
*  Malloc a new runstack context
*
*  RETURNS:
*     malloced and initialized runstack context
*
*********************************************************************/
runstack_context_t *
    runstack_new_context (void)
{
    runstack_context_t *rcxt;

    rcxt = m__getObj(runstack_context_t);
    if (rcxt == NULL) {
        return NULL;
    }
    runstack_init_context(rcxt);
    return rcxt;

} /* runstack_new_context */ 


/********************************************************************
* FUNCTION runstack_session_cleanup
* 
* Cleanup after a yangcli session has ended
*
*  INPUTS:
*     rcxt == runstack context to use
*
*********************************************************************/
void
    runstack_session_cleanup (runstack_context_t *rcxt)
{
    runstack_entry_t   *se;

    if (rcxt == NULL) {
        rcxt = &defcxt;
    }

    var_cvt_generic(&rcxt->globalQ);
    var_cvt_generic(&rcxt->zeroQ);
    
    for (se = (runstack_entry_t *)dlq_firstEntry(&rcxt->runstackQ);
         se != NULL;
         se = (runstack_entry_t *)dlq_nextEntry(se)) {
        var_cvt_generic(&se->varQ);
    }

} /* runstack_session_cleanup */


/* END runstack.c */
