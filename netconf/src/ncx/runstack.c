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

#define RUNSTACK_MAX_NEST  16

#define RUNSTACK_BUFFLEN  32000

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* one script run level context entry
 * each time a 'run script' command is
 * encountered, a new stack context is created,
 * unless level[RUNSTACK_MAX_NEST - 1] is reached
 */
typedef struct stack_entry_t_ {
    uint32     level;
    FILE      *fp;
    xmlChar   *source;
    xmlChar   *buff;
    int        bufflen;
    uint32     linenum;
    dlq_hdr_t  parmQ;
    dlq_hdr_t  varQ;
} stack_entry_t;


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

/* nested run command support */
static boolean         runstack_init_done = FALSE;
static uint32          script_level;
static stack_entry_t   runstack[RUNSTACK_MAX_NEST];
static dlq_hdr_t       globalQ;    /* Q of ncx_var_t */
static dlq_hdr_t       zeroQ;      /* Q of ncx_var_t */


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
* FUNCTION runstack_level
* 
*  Get the current stack level
*
* RETURNS:
*   current stack level; 0 --> not in any script
*********************************************************************/
uint32
    runstack_level (void)
{
    return script_level;
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
*   source == file source
*   fp == file pointer
*
* RETURNS:
*   status
*********************************************************************/
status_t
    runstack_push (const xmlChar *source,
		   FILE *fp)
{
    stack_entry_t  *se;
    val_value_t    *val;
    uint32          i;
    status_t        res;

    /* check if this would overflow the runstack */
    if (script_level+1 == RUNSTACK_MAX_NEST) {
	return ERR_NCX_RESOURCE_DENIED;
    }

    /* check if this script is already being invoked
     * and prevent looping until the stack level maxes out
     * !! This test will be fooled by symbolic links !!
     */
    for (i=0; i<script_level; i++) {
	if (!xml_strcmp(source, runstack[i].source)) {
	    return ERR_NCX_DUP_ENTRY;
	}
    }

    /* get the next open slot */
    se = &runstack[script_level];

    se->fp = fp;
    se->bufflen = RUNSTACK_BUFFLEN;
    se->linenum = 0;
    dlq_createSQue(&se->parmQ);
    dlq_createSQue(&se->varQ);

    /* get a new line input buffer */
    se->buff = m__getMem(RUNSTACK_BUFFLEN);
    if (!se->buff) {
	return ERR_INTERNAL_MEM;
    }

    /* set the script source */
    se->source = xml_strdup(source);
    if (!se->source) {
	m__free(se->buff);
	se->buff = NULL;
	return ERR_INTERNAL_MEM;
    }

    /* create the P0 parameter */
    val = make_parmval((const xmlChar *)"0", se->source);
    if (!val) {
	m__free(se->buff);
	se->buff = NULL;
	m__free(se->source);
	se->source = NULL;
	return ERR_INTERNAL_MEM;
    }

    /* need to increment script level now so var_set_move
     * will work, and the correct script parameter queue
     * will be used for the P0 parameter
     */
    script_level++;

    /* ceate a new var entry and add it to the runstack que */
    res = var_set_move((const xmlChar *)"0", 1, ISLOCAL, val);
    if (res != NO_ERR) {
	val_free_value(val);
	m__free(se->buff);
	se->buff = NULL;
	m__free(se->source);
	se->source = NULL;
	script_level--;
	return res;
    }

    log_info("\nrunstack: Starting level %u script %s",
	      script_level, se->source);


    return NO_ERR;

}  /* runstack_push */


/********************************************************************
* FUNCTION runstack_pop
* 
*  Remove a script nest level context from the stack
*  Call just after script is completed
* 
*********************************************************************/
void
    runstack_pop (void)
{
    stack_entry_t  *se;
    ncx_var_t      *var;

    if (script_level == 0) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    se = &runstack[script_level-1];

    if (se->buff) {
	m__free(se->buff);
	se->buff = NULL;
    }
    if (se->fp) {
	fclose(se->fp);
	se->fp = NULL;
    }
    if (se->source) {
	log_info("\nrunstack: Ending level %u script %s",
	      script_level, se->source);

	m__free(se->source);
	se->source = NULL;
    }
    se->bufflen = 0;
    se->linenum = 0;

    while (!dlq_empty(&se->parmQ)) {
	var = (ncx_var_t *)dlq_deque(&se->parmQ);
	var_free(var);
    }

    while (!dlq_empty(&se->varQ)) {
	var = (ncx_var_t *)dlq_deque(&se->varQ);
	var_free(var);
    }
    
    script_level--;

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
    runstack_get_cmd (status_t *res)
{
    stack_entry_t  *se;
    xmlChar        *retstr, *start, *str;
    boolean         done;
    int             len, total;

    if (script_level == 0) {
	*res = SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    /* init locals */
    se = &runstack[script_level-1];
    retstr = NULL;  /* start of return string */
    start = se->buff;
    total = 0;
    done = FALSE;

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
	    /* read line failed */
	    /* ncx_print_errormsg(NULL, NULL, ERR_NCX_FILE_READ); */
	    if (retstr) {
		*res = NO_ERR;
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
	runstack_pop();
    } else {
	log_info("\nrunstack: run line %u, %s\n cmd: %s",
		 se->linenum, se->source, retstr);
    }

    return retstr;

}  /* runstack_get_cmd */


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
dlq_hdr_t *
    runstack_get_que (boolean isglobal)
{
    stack_entry_t  *se;

    /* check global que */
    if (isglobal) {
	return &globalQ;
    }

    /* check level zero local que */
    if (script_level == 0) {
	return &zeroQ;
    }

    /* get the current slot */
    se = &runstack[script_level-1];
    return &se->varQ;

}  /* runstack_get_que */


/********************************************************************
* FUNCTION runstack_get_parm_que
* 
*  Get the parameter queue for the current stack level
*
* RETURNS:
*   que for current stack level, NULL if an error
*********************************************************************/
dlq_hdr_t *
    runstack_get_parm_que (void)
{
    stack_entry_t  *se;

    if (script_level == 0) {
	return NULL;
    }

    /* get the current slot */
    se = &runstack[script_level-1];

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
	script_level = 0;
	dlq_createSQue(&globalQ);
	dlq_createSQue(&zeroQ);
	runstack_init_done = TRUE;
    }
	
} /* runstack_init */		      


/********************************************************************
* FUNCTION runstack_init
* 
*  Init this modulke before using it!!!
*
*********************************************************************/
void
    runstack_cleanup (void)
{
    ncx_var_t *var;

    if (runstack_init_done) {
	while (script_level > 0) {
	    runstack_pop();
	}

	while (!dlq_empty(&globalQ)) {
	    var = (ncx_var_t *)dlq_deque(&globalQ);
	    var_free(var);
	}

	while (!dlq_empty(&zeroQ)) {
	    var = (ncx_var_t *)dlq_deque(&zeroQ);
	    var_free(var);
	}
	runstack_init_done = FALSE;
    }

} /* runstack_cleanup */		      


/* END nunstack.c */
