/*  FILE: yangcli_util.c

   Utilities for NETCONF YANG-based CLI Tool

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
01-jun-08    abb      begun; started from ncxcli.c
27-mar-09    abb      split out from yangcli.c

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libssh2.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "libtecla.h"

#ifndef _H_procdefs
#include "procdefs.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_mgr
#include "mgr.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_obj
#include "obj.h"
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

#ifndef _H_val_util
#include "val_util.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xml_val
#include "xml_val.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xpath_yang
#include "xpath_yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yangcli
#include "yangcli.h"
#endif

#ifndef _H_yangcli_util
#include "yangcli_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                          T Y P E S                                *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION is_top_command
* 
* Check if command name is a top command
* Must be full name
*
* INPUTS:
*   rpcname == command name to check
*
* RETURNS:
*   TRUE if this is a top command
*   FALSE if not
*********************************************************************/
boolean
    is_top_command (const xmlChar *rpcname)
{
#ifdef DEBUG
    if (!rpcname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (!xml_strcmp(rpcname, YANGCLI_CD)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_CONNECT)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_EVENTLOG)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_FILL)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_HELP)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_HISTORY)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_LIST)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_MGRLOAD)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_PWD)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_QUIT)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_RECALL)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_RUN)) {
	;
    } else if (!xml_strcmp(rpcname, YANGCLI_SHOW)) {
	;
    } else {
	return FALSE;
    }
    return TRUE;

}  /* is_top_command */


/********************************************************************
* FUNCTION new_modptr
* 
*  Malloc and init a new module pointer block
* 
* INPUTS:
*    mod == module to cache in this struct
*    feature_list == feature list from capability
*    deviation_list = deviations list from capability
*
* RETURNS:
*   malloced modptr_t struct or NULL of malloc failed
*********************************************************************/
modptr_t *
    new_modptr (ncx_module_t *mod,
		ncx_list_t *feature_list,
		ncx_list_t *deviation_list)
{
    modptr_t  *modptr;

#ifdef DEBUG
    if (!mod) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    modptr = m__getObj(modptr_t);
    if (!modptr) {
	return NULL;
    }
    memset(modptr, 0x0, sizeof(modptr_t));
    modptr->mod = mod;
    modptr->feature_list = feature_list;
    modptr->deviation_list = deviation_list;

    return modptr;

}  /* new_modptr */


/********************************************************************
* FUNCTION free_modptr
* 
*  Clean and free a module pointer block
* 
* INPUTS:
*    modptr == mod pointer block to free
*              MUST BE REMOVED FROM ANY Q FIRST
*
*********************************************************************/
void
    free_modptr (modptr_t *modptr)
{
#ifdef DEBUG
    if (!modptr) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    m__free(modptr);

}  /* free_modptr */


/********************************************************************
* FUNCTION clear_agent_cb_session
* 
*  Clean the current session data from an agent control block
* 
* INPUTS:
*    agent_cb == control block to use for clearing
*                the session data
*********************************************************************/
void
    clear_agent_cb_session (agent_cb_t *agent_cb)
{
    modptr_t  *modptr;

#ifdef DEBUG
    if (!agent_cb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(&agent_cb->modptrQ)) {
	modptr = (modptr_t *)dlq_deque(&agent_cb->modptrQ);
	free_modptr(modptr);
    }
    agent_cb->mysid = 0;
    agent_cb->state = MGR_IO_ST_IDLE;

    if (agent_cb->connect_valset) {
        val_free_value(agent_cb->connect_valset);
        agent_cb->connect_valset = NULL;
    }

}  /* clear_agent_cb_session */


/********************************************************************
* FUNCTION is_top
* 
* Check the state and determine if the top or conn
* mode is active
* 
* INPUTS:
*   agent state to use
*
* RETURNS:
*  TRUE if this is TOP mode
*  FALSE if this is CONN mode (or associated states)
*********************************************************************/
boolean
    is_top (mgr_io_state_t state)
{
    switch (state) {
    case MGR_IO_ST_INIT:
    case MGR_IO_ST_IDLE:
	return TRUE;
    case MGR_IO_ST_CONNECT:
    case MGR_IO_ST_CONN_START:
    case MGR_IO_ST_SHUT:
    case MGR_IO_ST_CONN_IDLE:
    case MGR_IO_ST_CONN_RPYWAIT:
    case MGR_IO_ST_CONN_CANCELWAIT:
    case MGR_IO_ST_CONN_CLOSEWAIT:
    case MGR_IO_ST_CONN_SHUT:
	return FALSE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

}  /* is_top */


/********************************************************************
* FUNCTION use_agentcb
* 
* Check if the agent_cb should be used for modules right now
*
* INPUTS:
*   agent_cb == agent control block to check
*
* RETURNS:
*   TRUE to use agent_cb
*   FALSE if not
*********************************************************************/
boolean
    use_agentcb (agent_cb_t *agent_cb)
{
    if (!agent_cb || is_top(agent_cb->state)) {
	return FALSE;
    } else if (dlq_empty(&agent_cb->modptrQ)) {
	return FALSE;
    }
    return TRUE;
}  /* use_agentcb */


/********************************************************************
* FUNCTION find_module
* 
*  Check the agent_cb for the specified module; if not found
*  then try ncx_find_module
* 
* INPUTS:
*    agent_cb == control block to free
*    modname == module name
*
* RETURNS:
*   pointer to the requested module
*      using the registered 'current' version
*   NULL if not found
*********************************************************************/
ncx_module_t *
    find_module (agent_cb_t *agent_cb,
		 const xmlChar *modname)
{
    modptr_t      *modptr;
    ncx_module_t  *mod;

#ifdef DEBUG
    if (!modname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (use_agentcb(agent_cb)) {
	for (modptr = (modptr_t *)dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (modptr_t *)dlq_nextEntry(modptr)) {

	    if (!xml_strcmp(modptr->mod->name, modname)) {
		return modptr->mod;
	    }
	}
    }

    mod = ncx_find_module(modname, NULL);

    return mod;

}  /* find_module */




/********************************************************************
* FUNCTION get_strparm
* 
* Get the specified string parm from the parmset and then
* make a strdup of the value
*
* INPUTS:
*   valset == value set to check if not NULL
*   modname == module defining parmname
*   parmname  == name of parm to get
*
* RETURNS:
*   pointer to string !!! THIS IS A MALLOCED COPY !!!
*********************************************************************/
xmlChar *
    get_strparm (val_value_t *valset,
		 const xmlChar *modname,
		 const xmlChar *parmname)
{
    val_value_t    *parm;
    xmlChar        *str;

#ifdef DEBUG
    if (!valset || !parmname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif
    
    str = NULL;
    parm = findparm(valset, modname, parmname);
    if (parm) {
	str = xml_strdup(VAL_STR(parm));
	if (!str) {
	    log_error("\nyangcli: Out of Memory error");
	}
    }
    return str;

}  /* get_strparm */


/********************************************************************
* FUNCTION findparm
* 
* Get the specified string parm from the parmset and then
* make a strdup of the value
*
* INPUTS:
*   valset == value set to search
*   modname == optional module name defining the parameter to find
*   parmname  == name of parm to get, or partial name to get
*
* RETURNS:
*   pointer to val_value_t if found
*********************************************************************/
val_value_t *
    findparm (val_value_t *valset,
	      const xmlChar *modname,
	      const xmlChar *parmname)
{
    val_value_t *parm;

#ifdef DEBUG
    if (!parmname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (!valset) {
	return NULL;
    }

    parm = val_find_child(valset, modname, parmname);
    if (!parm && get_autocomp()) {
	parm = val_match_child(valset, modname, parmname);
    }
    return parm;

}  /* findparm */


/********************************************************************
* FUNCTION add_clone_parm
* 
*  Create a parm 
* 
* INPUTS:
*   val == value to clone and add
*   valset == value set to add parm into
*
* RETURNS:
*    status
*********************************************************************/
status_t
    add_clone_parm (const val_value_t *val,
		    val_value_t *valset)
{
    val_value_t    *parm;

#ifdef DEBUG
    if (!val || !valset) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    parm = val_clone(val);
    if (!parm) {
	log_error("\nyangcli: val_clone failed");
	return ERR_INTERNAL_MEM;
    } else {
	val_add_child(parm, valset);
    }
    return NO_ERR;

}  /* add_clone_parm */


/********************************************************************
* FUNCTION is_yangcli_ns
* 
*  Check the namespace and make sure this is an YANGCLI command
* 
* INPUTS:
*   ns == namespace ID to check
*
* RETURNS:
*  TRUE if this is the YANGCLI namespace ID
*********************************************************************/
boolean
    is_yangcli_ns (xmlns_id_t ns)
{
    const xmlChar *modname;

    modname = xmlns_get_module(ns);
    if (modname && !xml_strcmp(modname, YANGCLI_MOD)) {
	return TRUE;
    } else {
	return FALSE;
    }

}  /* is_yangcli_ns */


/********************************************************************
 * FUNCTION clear_result
 * 
 * clear out the pending result info
 *
 * INPUTS:
 *   agent_cb == agent control block to use
 *
 *********************************************************************/
void
    clear_result (agent_cb_t *agent_cb)

{
#ifdef DEBUG
    if (!agent_cb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (agent_cb->result_name) {
	m__free(agent_cb->result_name);
	agent_cb->result_name = NULL;
    }
    if (agent_cb->result_filename) {
	m__free(agent_cb->result_filename);
	agent_cb->result_filename = NULL;
    }

}  /* clear_result */


/********************************************************************
* FUNCTION check_filespec
* 
* Check the filespec string for a file assignment statement
* Save it if it si good
*
* INPUTS:
*    agent_cb == agent control block to use
*    filespec == string to check
*    varname == variable name to use in log_error
*              if this is complex form
*
* OUTPUTS:
*    agent_cb->result_filename will get set if NO_ERR
*
* RETURNS:
*   status
*********************************************************************/
status_t
    check_filespec (agent_cb_t *agent_cb,
		    const xmlChar *filespec,
		    const xmlChar *varname)
{
    const xmlChar *teststr;

#ifdef DEBUG
    if (!agent_cb || !filespec) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (!filespec || !*filespec) {
	if (varname) {
	    log_error("\nError: file assignment variable '%s' "
		      "is empty string", varname);
	} else {
	    log_error("\nError: file assignment filespec "
		      "is empty string");
	}
	return ERR_NCX_INVALID_VALUE;
    }

    /* variable must be a string with only
     * valid filespec chars in it; no spaces
     * are allowed; too many security holes
     * if arbitrary strings are allowed here
     */
    if (val_need_quotes(filespec)) {
	if (varname) {
	    log_error("\nError: file assignment variable '%s' "
		      "contains whitespace (%s)", 
		      varname, filespec);
	} else {
	    log_error("\nError: file assignment filespec '%s' "
		      "contains whitespace", filespec);
	}
	return ERR_NCX_INVALID_VALUE;
    }

    /* check for acceptable chars */
    teststr = filespec;
    while (*teststr) {
	if (*teststr == NCXMOD_PSCHAR ||
	    *teststr == '.' ||
#ifdef WINDOWS
	    *teststr == ':' ||
#endif
	    ncx_valid_name_ch(*teststr)) {
	    teststr++;
	} else {
	    if (varname) {
		log_error("\nError: file assignment variable '%s' "
			  "contains invalid filespec (%s)", 
			  varname, filespec);
	    } else {
		log_error("\nError: file assignment filespec '%s' "
			  "contains invalid filespec", filespec);
	    }
	    return ERR_NCX_INVALID_VALUE;
	}
    }

    /* toss out the old value, if any */
    if (agent_cb->result_filename) {
	m__free(agent_cb->result_filename);
    }

    /* save the filename, may still be an invalid fspec  */
    agent_cb->result_filename = xml_strdup(filespec);
    if (!agent_cb->result_filename) {
	return ERR_INTERNAL_MEM;
    }
    return NO_ERR;

}  /* check_filespec */


/********************************************************************
 * FUNCTION get_instanceid_parm
 * 
 * Validate an instance identifier parameter
 * Return the target object
 * Return a value struct from root containing
 * all the predicate assignments in the stance identifier
 *
 * INPUTS:
 *    agent_cb == agent control block to use (NULL if none)
 *    target == XPath expression for the instance-identifier
 *    schemainst == TRUE if ncx:schema-instance string
 *                  FALSE if instance-identifier
 *    targobj == address of return target object for this expr
 *    targval == address of return pointer to target value
 *               node within the value subtree returned
 *    retres == address of return status
 *
 * OUTPUTS:
 *    *targobj == the object template for the target
 *    *targval == the target node within the returned subtree
 *                from root
 *    *retres == return status for the operation
 *
 * RETURNS:
 *   If NO_ERR:
 *     malloced value node representing the instance-identifier
 *     from root to the targobj
 *  else:
 *    NULL, check *retres
 *********************************************************************/
val_value_t *
    get_instanceid_parm (agent_cb_t *agent_cb,
			 const xmlChar *target,
			 boolean schemainst,
			 obj_template_t **targobj,
			 val_value_t **targval,
			 status_t *retres)
{
    xpath_pcb_t           *xpathpcb;
    val_value_t           *retval;
    status_t               res;

#ifdef DEBUG
    if (!agent_cb || !target || !targobj || !targval || !retres) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    *targobj = NULL;
    *targval = NULL;
    *retres = NO_ERR;

    /* get a parser block for the instance-id */
    xpathpcb = xpath_new_pcb(target);
    if (!xpathpcb) {
	log_error("\nError: malloc failed");
	*retres = ERR_INTERNAL_MEM;
	return NULL;
    }

    /* initial parse into a token chain */
    res = xpath_yang_parse_path(NULL, 
				NULL, 
				XP_SRC_INSTANCEID,
				xpathpcb);
    if (res != NO_ERR) {
	log_error("\nError: parse XPath target '%s' failed",
		  xpathpcb->exprstr);
	xpath_free_pcb(xpathpcb);
	*retres = res;
	return NULL;
    }

    /* validate against the object tree */
    res = xpath_yang_validate_path(NULL, 
				   ncx_get_gen_root(),
				   xpathpcb,
				   schemainst,
				   targobj);
    if (res != NO_ERR) {
	log_error("\nError: validate XPath target '%s' failed",
		  xpathpcb->exprstr);
	xpath_free_pcb(xpathpcb);
	*retres = res;
	return NULL;
    }

    /* have a valid target object, so follow the
     * parser chain and build a value subtree
     * from the XPath expression
     */
    retval = xpath_yang_make_instanceid_val(xpathpcb, 
					    &res,
					    targval);

    xpath_free_pcb(xpathpcb);
    *retres = res;

    return retval;

} /* get_instanceid_parm */


/********************************************************************
* FUNCTION file_is_text
* 
* Check the filespec string for a file assignment statement
* to see if it is text or XML
*
* INPUTS:
*    filespec == string to check
*
* RETURNS:
*   TRUE if text file, FALSE otherwise
*********************************************************************/
boolean
    file_is_text (const xmlChar *filespec)
{
    const xmlChar *teststr;
    uint32         len;

#ifdef DEBUG
    if (!filespec) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    len = xml_strlen(filespec);
    if (len < 5) {
	return FALSE;
    }

    teststr = &filespec[len-1];

    while (teststr > filespec && *teststr != '.') {
	teststr--;
    }

    if (teststr == filespec) {
	return FALSE;
    }

    teststr++;

    if (!xml_strcmp(teststr, NCX_EL_YANG)) {
	return TRUE;
    }

    if (!xml_strcmp(teststr, NCX_EL_TXT)) {
	return TRUE;
    }

    if (!xml_strcmp(teststr, NCX_EL_TEXT)) {
	return TRUE;
    }

    if (!xml_strcmp(teststr, NCX_EL_LOG)) {
	return TRUE;
    }

    return FALSE;

}  /* file_is_text */


/********************************************************************
* FUNCTION interactive_mode
* 
*  Check if the program is in interactive mode
* 
* RETURNS:
*   TRUE if insteractive mode, FALSE if batch mode
*********************************************************************/
boolean
    interactive_mode (void)
{
    return get_batchmode() ? FALSE : TRUE;

}  /* interactive_mode */


/********************************************************************
 * FUNCTION init_completion_state
 * 
 * init the completion_state struct for a new command
 *
 * INPUTS:
 *    completion_state == record to initialize
 *    agent_cb == agent control block to use
 *    cmdstate ==initial  calling state
 *********************************************************************/
void
    init_completion_state (completion_state_t *completion_state,
			   agent_cb_t *agent_cb,
			   command_state_t  cmdstate)
{
#ifdef DEBUG
    if (!completion_state) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(completion_state, 
	   0x0, 
	   sizeof(completion_state_t));
    completion_state->agent_cb = agent_cb;
    completion_state->cmdstate = cmdstate;

}  /* init_completion_state */


/********************************************************************
 * FUNCTION set_completion_state
 * 
 * set the completion_state struct for a new mode or sub-command
 *
 * INPUTS:
 *    completion_state == record to set
 *    rpc == rpc operation in progress (may be NULL)
 *    parm == parameter being filled in
 *    cmdstate ==current calling state
 *********************************************************************/
void
    set_completion_state (completion_state_t *completion_state,
			  obj_template_t *rpc,
			  obj_template_t *parm,
			  command_state_t  cmdstate)
{
#ifdef DEBUG
    if (!completion_state) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    completion_state->cmdstate = cmdstate;
    completion_state->cmdobj = rpc;
    if (rpc) {
	completion_state->cmdinput =
	    obj_find_child(rpc, NULL, YANG_K_INPUT);
    } else {
	completion_state->cmdinput = NULL;
    }
    completion_state->cmdcurparm = parm;

}  /* set_completion_state */


/********************************************************************
 * FUNCTION set_completion_state_curparm
 * 
 * set the current parameter in the completion_state struct
 *
 * INPUTS:
 *    completion_state == record to set
 *    parm == parameter being filled in
 *********************************************************************/
void
    set_completion_state_curparm (completion_state_t *completion_state,
				  obj_template_t *parm)
{
#ifdef DEBUG
    if (!completion_state) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    completion_state->cmdcurparm = parm;

}  /* set_completion_state_curparm */


/* END yangcli_util.c */
