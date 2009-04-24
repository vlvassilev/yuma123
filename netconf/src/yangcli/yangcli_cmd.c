/*  FILE: yangcli_cmd.c

   NETCONF YANG-based CLI Tool

   See ./README for details

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
11-apr-09    abb      begun; started from yangcli.c

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

#ifndef _H_cli
#include "cli.h"
#endif

#ifndef _H_conf
#include "conf.h"
#endif

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_mgr
#include "mgr.h"
#endif

#ifndef _H_mgr_io
#include "mgr_io.h"
#endif

#ifndef _H_mgr_rpc
#include "mgr_rpc.h"
#endif

#ifndef _H_mgr_ses
#include "mgr_ses.h"
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

#ifndef _H_obj_help
#include "obj_help.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
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

#ifndef _H_var
#include "var.h"
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

#ifndef _H_xml_wr
#include "xml_wr.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xpath1
#include "xpath1.h"
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

#ifndef _H_yangcli_cmd
#include "yangcli_cmd.h"
#endif

#ifndef _H_yangcli_tab
#include "yangcli_tab.h"
#endif

#ifndef _H_yangcli_util
#include "yangcli_util.h"
#endif


/********************************************************************
* FUNCTION parse_rpc_cli
* 
*  Call the cli_parse for an RPC input value set
* 
* INPUTS:
*   rpc == RPC to parse CLI for
*   line == input line to parse, starting with the parms to parse
*   res == pointer to status output
*
* OUTPUTS: 
*   *res == status
*
* RETURNS:
*    pointer to malloced value set or NULL if none created,
*    may have errors, check *res
*********************************************************************/
static val_value_t *
    parse_rpc_cli (const obj_template_t *rpc,
		   const xmlChar *args,
		   status_t  *res)
{
    const obj_template_t   *obj;
    const xmlChar          *myargv[2];

    /* construct an argv array, 
     * convert the CLI into a parmset 
     */
    obj = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (obj && obj_get_child_count(obj)) {
	myargv[0] = obj_get_name(rpc);
	myargv[1] = args;
	return cli_parse(2, (const char **)myargv, 
			 obj, VALONLY, SCRIPTMODE,
			 get_autocomp(), res);
    } else {
	*res = ERR_NCX_SKIPPED;
	return NULL;
    }
    /*NOTREACHED*/

}  /* parse_rpc_cli */


/********************************************************************
* FUNCTION get_prompt
* 
* Construct the CLI prompt for the current state
* 
* INPUTS:
*   buff == bufffer to hold prompt (zero-terminated string)
*   bufflen == length of buffer (max bufflen-1 chars can be printed
*
*********************************************************************/
static void
    get_prompt (agent_cb_t *agent_cb,
		xmlChar *buff,
		uint32 bufflen)
{
    xmlChar        *p;
    val_value_t    *parm;
    uint32          len;

    if (agent_cb->climore) {
	xml_strncpy(buff, MORE_PROMPT, bufflen);
	return;
    }

    switch (agent_cb->state) {
    case MGR_IO_ST_INIT:
    case MGR_IO_ST_IDLE:
    case MGR_IO_ST_CONNECT:
    case MGR_IO_ST_CONN_START:
    case MGR_IO_ST_SHUT:
	if (agent_cb->cli_fn) {
	    if ((xml_strlen(DEF_FN_PROMPT) 
		 + xml_strlen(agent_cb->cli_fn) + 2) < bufflen) {
		p = buff;
		p += xml_strcpy(p, DEF_FN_PROMPT);
		p += xml_strcpy(p, agent_cb->cli_fn);
		xml_strcpy(p, (const xmlChar *)"> ");
	    } else {
		xml_strncpy(buff, DEF_PROMPT, bufflen);
	    }
	} else {
	    xml_strncpy(buff, DEF_PROMPT, bufflen);
	}
	break;
    case MGR_IO_ST_CONN_IDLE:
    case MGR_IO_ST_CONN_RPYWAIT:
    case MGR_IO_ST_CONN_CANCELWAIT:
    case MGR_IO_ST_CONN_CLOSEWAIT:
    case MGR_IO_ST_CONN_SHUT:
	p = buff;
	len = xml_strncpy(p, (const xmlChar *)"yangcli ", bufflen);
	p += len;
	bufflen -= len;

	if (bufflen == 0) {
	    return;
	}

	parm = NULL;
	if (agent_cb->connect_valset) {
	    parm = val_find_child(agent_cb->connect_valset, 
				  YANGCLI_MOD, 
				  YANGCLI_USER);
	}

	/*
	if (!parm && connect_valset) {
	    parm = val_find_child(connect_valset, 
				  YANGCLI_MOD, 
				  YANGCLI_USER);
	}
	*/

	if (parm) {
	    len = xml_strncpy(p, VAL_STR(parm), bufflen);
	    p += len;
	    bufflen -= len;
	    if (bufflen == 0) {
		return;
	    }
	    *p++ = NCX_AT_CH;
	    --bufflen;
	    if (bufflen == 0) {
		return;
	    }
	}

	parm = NULL;
	if (agent_cb->connect_valset) {
	    parm = val_find_child(agent_cb->connect_valset, 
				  YANGCLI_MOD, 
				  YANGCLI_AGENT);
	}

	/*
	if (!parm && connect_valset) {
	    parm= val_find_child(connect_valset, 
				 YANGCLI_MOD, 
				 YANGCLI_AGENT);
	}
	*/

	if (parm) {
	    len = xml_strncpy(p, VAL_STR(parm), bufflen);
	    p += len;
	    bufflen -= len;
	    if (bufflen == 0) {
		return;
	    }
	}

	if (agent_cb->cli_fn && bufflen > 3) {
	    *p++ = ':';
	    len = xml_strncpy(p, agent_cb->cli_fn, --bufflen);
	    p += len;
	    bufflen -= len;
	}

	if (bufflen > 2) {
	    *p++ = '>';
	    *p++ = ' ';
	    *p = 0;
	    bufflen -= 2;
	}
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	xml_strncpy(buff, DEF_PROMPT, bufflen);	
	break;
    }

}  /* get_prompt */


/********************************************************************
* get_line
*
* Generate a prompt based on the program state and
* use the tecla library read function to handle
* user keyboard input
*
*
* Do not free this line after use, just discard it
* It will be freed by the tecla lib
*
* INPUTS:
*   agent_cb == agent control block to use
*
* RETURNS:
*   static line from tecla read line, else NULL if some error
*********************************************************************/
static xmlChar *
    get_line (agent_cb_t *agent_cb)
{
    xmlChar *line;
    xmlChar prompt[MAX_PROMPT_LEN];

    line = NULL;

    get_prompt(agent_cb, prompt, MAX_PROMPT_LEN-1);

    if (!agent_cb->climore) {
	log_stdout("\n");
    }
    line = (xmlChar *)gl_get_line(agent_cb->cli_gl,
				  (const char *)prompt,
				  NULL, -1);
    if (!line) {
	log_stdout("\nyangcli: Error: gl_get_line failed");
    }

    return line;

} /* get_line */



/********************************************************************
* FUNCTION try_parse_def
* 
* Parse the possibly module-qualified definition (module:def)
* and find the template for the requested definition
*
* INPUTS:
*   agent_cb == agent control block to use
*   modname == module name to try
*   defname == definition name to try
*   dtyp == definition type 
*
* OUTPUTS:
*    *dtyp is set if it started as NONE
*
* RETURNS:
*   pointer to the found definition template or NULL if not found
*********************************************************************/
static void *
    try_parse_def (agent_cb_t *agent_cb,
		   const xmlChar *modname,
		   const xmlChar *defname,
		   ncx_node_t *dtyp)

{
    void          *def;
    ncx_module_t  *mod;

    mod = find_module(agent_cb, modname);
    if (!mod) {
	return NULL;
    }

    def = NULL;
    switch (*dtyp) {
    case NCX_NT_NONE:
	def = ncx_find_object(mod, defname);
	if (def) {
	    *dtyp = NCX_NT_OBJ;
	    break;
	}
	def = ncx_find_grouping(mod, defname);
	if (def) {
	    *dtyp = NCX_NT_GRP;
	    break;
	}
	def = ncx_find_type(mod, defname);
	if (def) {
	    *dtyp = NCX_NT_TYP;
	    break;
	}
	break;
    case NCX_NT_OBJ:
	def = ncx_find_object(mod, defname);
	break;
    case NCX_NT_GRP:
	def = ncx_find_grouping(mod, defname);
	break;
    case NCX_NT_TYP:
	def = ncx_find_type(mod, defname);
	break;
    default:
	def = NULL;
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    return def;
    
} /* try_parse_def */


/********************************************************************
* FUNCTION get_yesno
* 
* Get the user-selected choice, yes or no
*
* INPUTS:
*   agent_cb == agent control block to use
*   prompt == prompt message
*   defcode == default answer code
*      0 == no def answer
*      1 == def answer is yes
*      2 == def answer is no
*   retcode == address of return code
*
* OUTPUTS:
*    *retcode is set if return NO_ERR
*       0 == cancel
*       1 == yes
*       2 == no
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_yesno (agent_cb_t *agent_cb,
	       const xmlChar *prompt,
	       uint32 defcode,
	       uint32 *retcode)
{
    xmlChar                 *myline, *str;
    status_t                 res;
    boolean                  done;

    done = FALSE;
    res = NO_ERR;

    set_completion_state(&agent_cb->completion_state,
			 NULL,
			 NULL,
			 CMD_STATE_YESNO);

    if (prompt) {
	log_stdout("\n%s", prompt);
    }
    log_stdout("\nEnter Y for yes, N for no, or C to cancel:");
    switch (defcode) {
    case YESNO_NODEF:
	break;
    case YESNO_YES:
	log_stdout(" [default: Y]");
	break;
    case YESNO_NO:
	log_stdout(" [default: N]");
	break;
    default:
	res = SET_ERROR(ERR_INTERNAL_VAL);
	done = TRUE;
    }


    while (!done) {

	/* get input from the user STDIN */
	myline = get_cmd_line(agent_cb, &res);
	if (!myline) {
	    done = TRUE;
	    continue;
	}

	/* strip leading whitespace */
	str = myline;
	while (*str && xml_isspace(*str)) {
	    str++;
	}

	/* convert to a number, check [ENTER] for default */
	if (*str) {
	    if (*str == 'Y' || *str == 'y') {
		*retcode = YESNO_YES;
		done = TRUE;
	    } else if (*str == 'N' || *str == 'n') {
		*retcode = YESNO_NO;
		done = TRUE;
	    } else if (*str == 'C' || *str == 'c') {
		*retcode = YESNO_CANCEL;
		done = TRUE;
	    }
	} else {
	    /* default accepted */
	    switch (defcode) {
	    case YESNO_NODEF:
		break;
	    case YESNO_YES:
		*retcode = YESNO_YES;
		done = TRUE;
		break;
	    case YESNO_NO:
		*retcode = YESNO_NO;
		done = TRUE;
		break;
	    default:
		res = SET_ERROR(ERR_INTERNAL_VAL);
		done = TRUE;
	    }
	}
	if (res == NO_ERR && !done) {
	    log_stdout("\nError: invalid value '%s'\n", str);
	}
    }

    return res;

}  /* get_yesno */


/********************************************************************
* FUNCTION get_complex_parm
* 
* Fill the specified parm, which is a complex type
* This function will block on readline to get input from the user
*
* INPUTS:
*   agent_cb == agent control block to use
*   parm == parm to get from the CLI
*   valset == value set being filled
*
* OUTPUTS:
*    new val_value_t node will be added to valset if NO_ERR
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_complex_parm (agent_cb_t *agent_cb,
		      const obj_template_t *parm,
		      val_value_t *valset)
{
    xmlChar          *line;
    val_value_t      *new_parm;
    status_t          res;

    res = NO_ERR;

    log_stdout("\nEnter parameter value %s (%s)", 
	       obj_get_name(parm), 
	       tk_get_btype_sym(obj_get_basetype(parm)));

    /* get a line of input from the user */
    line = get_cmd_line(agent_cb, &res);
    if (!line) {
	return res;
    }

    new_parm = val_new_value();
    if (!new_parm) {
	res = ERR_INTERNAL_MEM;
    } else {
	val_init_from_template(new_parm, parm);
	(void)var_get_script_val(parm,
				 new_parm,
				 line, ISPARM, &res);
	if (res == NO_ERR) {
	    /* add the parm to the parmset */
	    val_add_child(new_parm, valset);
	}
    }

    if (res != NO_ERR) {
	log_stdout("\nyangcli: Error in %s (%s)",
		   obj_get_name(parm), get_error_string(res));
    }

    return res;
    
} /* get_complex_parm */


/********************************************************************
* FUNCTION get_parm
* 
* Fill the specified parm
* Use the old value for the default if any
* This function will block on readline to get input from the user
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method that is being called
*   parm == parm to get from the CLI
*   valset == value set being filled
*   oldvalset == last set of values (NULL if none)
*
* OUTPUTS:
*    new val_value_t node will be added to valset if NO_ERR
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_parm (agent_cb_t *agent_cb,
	      const obj_template_t *rpc,
	      const obj_template_t *parm,
	      val_value_t *valset,
	      val_value_t *oldvalset)
{
    const xmlChar    *def, *parmname, *str;
    const typ_def_t  *typdef;
    val_value_t      *oldparm, *newparm;
    xmlChar          *line, *start, *objbuff, *buff;
    xmlChar          *line2, *start2, *saveline;
    status_t          res;
    ncx_btype_t       btyp;
    boolean           done;
    uint32            len;

    if (!obj_is_mandatory(parm) && !agent_cb->get_optional) {
	return NO_ERR;
    }

    res = NO_ERR;

    if (obj_is_data_db(parm)) {
	objbuff = NULL;
	res = obj_gen_object_id(parm, &objbuff);
	if (res != NO_ERR) {
	    log_error("\nError: generate object ID failed (%s)",
		      get_error_string(res));
	    return res;
	}

	/* let the user know about the new nest level */
	if (obj_is_key(parm)) {
	    str = YANG_K_KEY;
	} else if (obj_is_mandatory(parm)) {
	    str = YANG_K_MANDATORY;
	} else {
	    str = (const xmlChar *)"optional";
	}

	log_stdout("\nFilling %s %s %s:", str,
		   obj_get_typestr(parm), objbuff);
	    
	m__free(objbuff);
    }

    switch (btyp) {
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
	return get_complex_parm(agent_cb, parm, valset);
    default:
	;
    }

    parmname = obj_get_name(parm);
    typdef = obj_get_ctypdef(parm);
    btyp = obj_get_basetype(parm);
    res = NO_ERR;
    oldparm = NULL;
    def = NULL;
  
    done = FALSE;
    while (!done) {
	if (btyp==NCX_BT_EMPTY) {
	    log_stdout("\nShould flag %s be set? [Y, N, %s]", 
		       parmname, DEF_OPTIONS);
	} else {
	    log_stdout("\nEnter %s value for %s <%s>",
		       (const xmlChar *)tk_get_btype_sym(btyp),
		       obj_get_typestr(parm),
		       parmname);
	    log_stdout("\n    %s", DEF_OPTIONS);
	}
	if (oldvalset) {
	    oldparm = val_find_child(oldvalset, 
				     obj_get_mod_name(parm),
				     parmname);
	}

	/* pick a default value, either old value or default clause */
	if (!oldparm) {
	    /* try to get the defined default value */
	    if (btyp != NCX_BT_EMPTY) {
		def = obj_get_default(parm);
		if (!def && (obj_get_nsid(rpc) == xmlns_nc_id() &&
			     (!xml_strcmp(parmname, NCX_EL_TARGET) ||
			      !xml_strcmp(parmname, NCX_EL_SOURCE)))) {
		    /* offer the default target for the NETCONF
		     * <source> and <target> parameters
		     */
		    def = agent_cb->default_target;
		}
	    }
	    if (def) {
		log_stdout(" [%s]\n", def);
	    } else if (btyp==NCX_BT_EMPTY) {
		log_stdout(" [N]\n");
	    }
	} else {
	    /* use the old value for the default */
	    log_stdout(" [");
	    if (btyp==NCX_BT_EMPTY) {
		log_stdout("Y");
	    } else {
		res = val_sprintf_simval_nc(NULL, oldparm, &len);
		if (res != NO_ERR) {
		    return SET_ERROR(res);
		}
		buff = m__getMem(len+1);
		if (!buff) {
		    return ERR_INTERNAL_MEM;
		}
		res = val_sprintf_simval_nc(buff, oldparm, &len);
		if (res == NO_ERR) {
		    log_stdout("%s", buff);
		}
		m__free(buff);
	    }
	    log_stdout("]\n");
	}

	set_completion_state_curparm(&agent_cb->completion_state,
				     parm);

	/* get a line of input from the user */
	line = get_cmd_line(agent_cb, &res);
	if (!line) {
	    return res;
	}

	/* skip whitespace */
	start = line;
	while (*start && xml_isspace(*start)) {
	    start++;
	}

	/* check for question-mark char sequences */
	if (*start == '?') {
	    if (start[1] == '?') {
		/* ?? == full help */
		obj_dump_template(parm, HELP_MODE_FULL, 0,
				  NCX_DEF_INDENT);
	    } else if (start[1] == 'C' || start[1] == 'c') {
		/* ?c or ?C == cancel the operation */
		log_stdout("\n%s command canceled",
			   obj_get_name(rpc));
		return ERR_NCX_CANCELED;
	    } else if (start[1] == 'S' || start[1] == 's') {
		/* ?s or ?S == skip this parameter */
		log_stdout("\n%s parameter skipped",
			   obj_get_name(parm));
		return ERR_NCX_SKIPPED;
	    } else {
		/* ? == normal help mode */
		obj_dump_template(parm, HELP_MODE_NORMAL, 4,
				  NCX_DEF_INDENT);
	    }
	    log_stdout("\n");
	    continue;
	} else {
	    /* top loop to get_parm is only executed once */
	    done = TRUE;
	}
    }

    /* check if any non-whitespace chars entered */
    if (!*start) {
	/* no input, use default or old value or EMPTY_STRING */
	if (def) {
	    /* use default */
	    res = cli_parse_parm_ex(valset, 
				    parm, 
				    def, 
				    SCRIPTMODE, 
				    get_baddata());
	} else if (oldparm) {
	    /* no default, try old value */
	    if (btyp==NCX_BT_EMPTY) {
		res = cli_parse_parm_ex(valset, 
					parm, 
					NULL,
					SCRIPTMODE, 
					get_baddata());
	    } else {
		/* use a copy of the last value */
		newparm = val_clone(oldparm);
		if (!newparm) {
		    res = ERR_INTERNAL_MEM;
		} else {
		    val_add_child(newparm, valset);
		}
	    }
	} else if (btyp == NCX_BT_EMPTY) {
	    res = cli_parse_parm_ex(valset, 
				    parm, 
				    NULL,
				    SCRIPTMODE, 
				    get_baddata());
	} else if (val_simval_ok(obj_get_ctypdef(parm), 
				 EMPTY_STRING) == NO_ERR) {
	    res = cli_parse_parm_ex(valset, 
				    parm, 
				    EMPTY_STRING,
				    SCRIPTMODE, 
				    get_baddata());
	} else {
	    /* data type requires some form of input */
	    res = ERR_NCX_DATA_MISSING;
	}  /* else flag should not be set */
    } else if (btyp==NCX_BT_EMPTY) {
	/* empty data type handled special Y: set, N: leave out */
	if (*start=='Y' || *start=='y') {
	    res = cli_parse_parm_ex(valset, 
				    parm, 
				    NULL, 
				    SCRIPTMODE, 
				    get_baddata());
	} else if (*start=='N' || *start=='n') {
	    ; /* skip; do not add the flag */
	} else if (oldparm) {
	    /* previous value was set, so add this flag */
	    res = cli_parse_parm_ex(valset, 
				    parm, 
				    NULL,
				    SCRIPTMODE, 
				    get_baddata());
	} else {
	    /* some value was entered, other than Y or N */
	    res = ERR_NCX_WRONG_VAL;
	}
    } else {
	/* normal case: input for regular data type */
	res = cli_parse_parm_ex(valset, 
				parm, 
				start,
				SCRIPTMODE, 
				get_baddata());
    }

    if (res != NO_ERR) {
	switch (get_baddata()) {
	case NCX_BAD_DATA_IGNORE:
	case NCX_BAD_DATA_WARN:
	    /* if these modes had error return status then the
	     * problem was not invalid value; maybe malloc error
	     */
	    break;
	case NCX_BAD_DATA_CHECK:
	    if (NEED_EXIT(res)) {
		break;
	    }

	    saveline = (start) ? xml_strdup(start) :
		xml_strdup(EMPTY_STRING);
	    if (!saveline) {
		res = ERR_INTERNAL_MEM;
		break;
	    }

	    done = FALSE;
	    while (!done) {
		log_stdout("\nError: parameter '%s' value '%s' is invalid"
			   "\nShould this value be used anyway? [Y, N, %s]"
			   " [N]", 
			   obj_get_name(parm),
			   (start) ? start : EMPTY_STRING,
			   DEF_OPTIONS);

		/* save the previous value because it is about
		 * to get trashed by getting a new line
		 */

		/* get a line of input from the user */
		line2 = get_cmd_line(agent_cb, &res);
		if (!line2) {
		    m__free(saveline);
		    return res;
		}

		/* skip whitespace */
		start2 = line2;
		while (*start2 && xml_isspace(*start2)) {
		    start2++;
		}

		/* check for question-mark char sequences */
		if (!*start2) {
		    /* default N: try again for a different input */
		    m__free(saveline);
		    res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
		    done = TRUE;
		} else if (*start2 == '?') {
		    if (start2[1] == '?') {
			/* ?? == full help */
			obj_dump_template(parm, HELP_MODE_FULL, 0,
					  NCX_DEF_INDENT);
		    } else if (start2[1] == 'C' || start2[1] == 'c') {
			/* ?c or ?C == cancel the operation */
			log_stdout("\n%s command canceled",
				   obj_get_name(rpc));
			m__free(saveline);
			res = ERR_NCX_CANCELED;
			done = TRUE;
		    } else if (start2[1] == 'S' || start2[1] == 's') {
			/* ?s or ?S == skip this parameter */
			log_stdout("\n%s parameter skipped",
				   obj_get_name(parm));
			m__free(saveline);
			res = ERR_NCX_SKIPPED;
			done = TRUE;
		    } else {
			/* ? == normal help mode */
			obj_dump_template(parm, HELP_MODE_NORMAL, 4,
					  NCX_DEF_INDENT);
		    }
		    log_stdout("\n");
		    continue;
		} else if (*start2 == 'Y' || *start2 == 'y') {
		    /* use the invalid value */
		    res = cli_parse_parm_ex(valset, parm, 
					    saveline, SCRIPTMODE, 
					    NCX_BAD_DATA_IGNORE);
		    m__free(saveline);
		    done = TRUE;
		} else if (*start2 == 'N' || *start2 == 'n') {
		    /* recurse: try again for a different input */
		    m__free(saveline);
		    res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
		    done = TRUE;
		} else {
		    log_stdout("\nInvalid input.");
		}
	    }
	    break;
	case NCX_BAD_DATA_ERROR:
	    log_stdout("\nError: set parameter '%s' failed (%s)\n",
		       obj_get_name(parm),
		       get_error_string(res));
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    return res;
    
} /* get_parm */


/********************************************************************
* FUNCTION get_case
* 
* Get the user-selected case, which is not fully set
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC template in progress
*   cas == case object template header
*   valset == value set to fill
*   oldvalset == last set of values (or NULL if none)
*
* OUTPUTS:
*    new val_value_t nodes may be added to valset
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_case (agent_cb_t *agent_cb,
	      const obj_template_t *rpc,
	      const obj_template_t *cas,
	      val_value_t *valset,
	      val_value_t *oldvalset)
{
    const obj_template_t    *parm;
    val_value_t             *pval;
    status_t                 res;
    boolean                  saveopt;

    /* make sure a case was selected or found */
    if (!obj_is_config(cas) || obj_is_abstract(cas)) {
	log_stdout("\nError: No writable objects to fill for this case");
	return ERR_NCX_SKIPPED;
    }

    saveopt = agent_cb->get_optional;
    agent_cb->get_optional = TRUE;
    res = NO_ERR;

    /* corner-case: user selected a case, and that case has
     * one empty leaf in it; 
     * e.g., <source> and <target> parms
     */
    if (obj_get_child_count(cas) == 1) {
	parm = obj_first_child(cas);
	if (parm && obj_get_basetype(parm)==NCX_BT_EMPTY) {
	    return cli_parse_parm(valset, parm, NULL, FALSE);
	}
    }

    /* finish the selected case */
    for (parm = obj_first_child(cas);
	 parm != NULL && res == NO_ERR;
	 parm = obj_next_child(parm)) {

	if (!obj_is_config(parm) || obj_is_abstract(parm)) {
	    continue;
	}

	pval = val_find_child(valset, 
			      obj_get_mod_name(parm),
			      obj_get_name(parm));
	if (pval) {
	    continue;
	}

	/* node is config and not already set */
	res = get_parm(agent_cb, rpc, parm, valset, oldvalset);

	if (res == ERR_NCX_SKIPPED) {
	    res = NO_ERR;
	}
    }

    agent_cb->get_optional = saveopt;
    return res;

} /* get_case */


/********************************************************************
* FUNCTION get_choice
* 
* Get the user-selected choice, which is not fully set
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC template in progress
*   choic == choice object template header
*   valset == value set to fill
*   oldvalset == last set of values (or NULL if none)
*
* OUTPUTS:
*    new val_value_t nodes may be added to valset
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_choice (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const obj_template_t *choic,
		val_value_t *valset,
		val_value_t *oldvalset)
{
    const obj_template_t    *parm, *cas, *usecase;
    val_value_t             *pval;
    xmlChar                 *myline, *str, *objbuff;
    status_t                 res;
    int                      casenum, num;
    boolean                  first, done, usedef, redo, saveopt;

    if (!obj_is_config(choic) || obj_is_abstract(choic)) {
	log_stdout("\nError: choice '%s' has no configurable parameters",
		   obj_get_name(choic));
	return ERR_NCX_ACCESS_DENIED;
    }

    casenum = 0;
    res = NO_ERR;

    if (obj_is_data_db(choic)) {
	objbuff = NULL;
	res = obj_gen_object_id(choic, &objbuff);
	if (res != NO_ERR) {
	    log_error("\nError: generate object ID failed (%s)",
		      get_error_string(res));
	    return res;
	}

	/* let the user know about the new nest level */
	log_stdout("\nFilling choice %s:", objbuff);
	m__free(objbuff);
    }

    saveopt = agent_cb->get_optional;
    
    if (obj_is_mandatory(choic)) {
	agent_cb->get_optional = TRUE;
    }

    /* first check the partial block corner case */
    pval = val_get_choice_first_set(valset, choic);
    if (pval) {
	/* found something set from this choice, finish the case */
	log_stdout("\nEnter more parameters to complete the choice:");

	cas = pval->casobj;
	if (!cas) {
	    agent_cb->get_optional = saveopt;
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	for (parm = obj_first_child(cas); 
	     parm != NULL;
	     parm = obj_next_child(parm)) {

	    if (!obj_is_config(parm) || obj_is_abstract(parm)) {
		continue;
	    }

	    pval = val_find_child(valset,
				  obj_get_mod_name(parm),
				  obj_get_name(parm));
	    if (pval) {
		continue;   /* node within case already set */
	    }

	    res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
	    switch (res) {
	    case NO_ERR:
		break;
	    case ERR_NCX_SKIPPED:
		res = NO_ERR;
		break;
	    case ERR_NCX_CANCELED:
		agent_cb->get_optional = saveopt;
		return res;
	    default:
		agent_cb->get_optional = saveopt;
		return res;
	    }
	}
	agent_cb->get_optional = saveopt;
	return NO_ERR;
    }

    /* check corner-case -- choice with no cases defined */
    cas = obj_first_child(choic);
    if (!cas) {
	log_stdout("\nNo case nodes defined for choice %s\n",
		   obj_get_name(choic));
	agent_cb->get_optional = saveopt;
	return NO_ERR;
    }


    /* else not a partial block corner case but a normal
     * situation where no case has been selected at all
     */
    log_stdout("\nEnter a number of the selected case statement:\n");

    num = 1;
    usedef = FALSE;

    for (; cas != NULL;
         cas = obj_next_child(cas)) {

	first = TRUE;
	for (parm = obj_first_child(cas);
	     parm != NULL;
	     parm = obj_next_child(parm)) {

	    if (!obj_is_config(parm) || obj_is_abstract(parm)) {
		continue;
	    }

	    if (first) {
		log_stdout("\n  %d: case %s:", 
			   num++, obj_get_name(cas));
		first = FALSE;
	    }

	    log_stdout("\n       %s %s",
		       obj_get_typestr(parm),
		       obj_get_name(parm));
	}
    }

    done = FALSE;
    log_stdout("\n");
    while (!done) {

	redo = FALSE;

	/* Pick a prompt, depending on the choice default case */
	if (obj_get_default(choic)) {
	    log_stdout("\nEnter choice number [%d - %d, %s], "
		       "[ENTER] for default (%s): ",
		       1, num-1, DEF_OPTIONS,
		       obj_get_default(choic));
	} else {
	    log_stdout("\nEnter choice number [%d - %d, %s]: ",
		       1, num-1, DEF_OPTIONS);
	}

	/* get input from the user STDIN */
	myline = get_cmd_line(agent_cb, &res);
	if (!myline) {
	    agent_cb->get_optional = saveopt;
	    return res;
	}

	/* strip leading whitespace */
	str = myline;
	while (*str && xml_isspace(*str)) {
	    str++;
	}

	/* convert to a number, check [ENTER] for default */
	if (!*str) {
	    usedef = TRUE;
	} else if (*str == '?') {
	    redo = TRUE;
	    if (str[1] == '?') {
		obj_dump_template(choic, HELP_MODE_FULL, 0,
				  NCX_DEF_INDENT);
	    } else if (str[1] == 'C' || str[1] == 'c') {
		log_stdout("\n%s command canceled\n",
			   obj_get_name(rpc));
		agent_cb->get_optional = saveopt;
		return ERR_NCX_CANCELED;
	    } else if (str[1] == 'S' || str[1] == 's') {
		log_stdout("\n%s choice skipped\n",
			   obj_get_name(choic));
		agent_cb->get_optional = saveopt;
		return ERR_NCX_SKIPPED;
	    } else {
		obj_dump_template(choic, HELP_MODE_NORMAL, 4,
				  NCX_DEF_INDENT);
	    }
	    log_stdout("\n");
	} else {
	    casenum = atoi((const char *)str);
	    usedef = FALSE;
	}

	if (redo) {
	    continue;
	}

	/* check if default requested */
	if (usedef) {
	    if (obj_get_default(choic)) {
		done = TRUE;
	    } else {
		log_stdout("\nError: Choice does not have a default case\n");
		usedef = FALSE;
	    }
	} else if (casenum < 0 || casenum >= num) {
	    log_stdout("\nError: invalid value '%s'\n", str);
	} else {
	    done = TRUE;
	}
    }

    /* got a valid choice number or use the default case
     * now get the object template for the correct case 
     */
    if (usedef) {
	cas = obj_find_child(choic,
			     obj_get_mod_name(choic),
			     obj_get_default(choic));
    } else {

	num = 1;
	done = FALSE;
	usecase = NULL;

	for (cas = obj_first_child(choic); 
	     cas != NULL && !done;
	     cas = obj_next_child(cas)) {

	    if (!obj_is_config(cas) || obj_is_abstract(cas)) {
		continue;
	    }

	    if (casenum == num) {
		done = TRUE;
		usecase = cas;
	    } else {
		num++;
	    }
	}

	cas = usecase;
    }

    /* make sure a case was selected or found */
    if (!cas) {
	log_stdout("\nError: No case to fill for this choice");
	agent_cb->get_optional = saveopt;
	return ERR_NCX_SKIPPED;
    }

    res = get_case(agent_cb, rpc, cas, valset, oldvalset);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_NCX_SKIPPED:
	res = NO_ERR;
	break;
    case ERR_NCX_CANCELED:
	break;
    default:
	;
    }

    agent_cb->get_optional = saveopt;

    return res;

} /* get_choice */


/********************************************************************
* FUNCTION fill_value
* 
* Malloc and fill the specified value
* Use the last value for the value if it is present and valid
* This function will block on readline for user input if no
* valid useval is given
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method that is being called
*   parm == object for value to fill
*   useval == value to use (or NULL if none)
*   res == address of return status
*
* OUTPUTS:
*    *res == return status
*
* RETURNS:
*    malloced value, filled in or NULL if some error
*********************************************************************/
static val_value_t *
    fill_value (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const obj_template_t *parm,
		val_value_t *useval,
		status_t  *res)
{
    const obj_template_t  *parentobj;
    val_value_t           *dummy, *newval;
    boolean                saveopt;

    switch (parm->objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	break;
    default:
	*res = SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    if (obj_is_abstract(parm)) {
	*res = ERR_NCX_NO_ACCESS_MAX;
	log_error("\nError: no access to abstract objects");
	return NULL;
    }

    /* just copy the useval if it is given */
    if (useval) {
	/* make sure it is a simple value */
	if (!typ_is_simple(useval->btyp)) {
	    *res = ERR_NCX_WRONG_TYPE;
	    log_error("\nError: var '%s' must be a simple type",
		      useval->name);
	    return NULL;
	}

	newval = val_make_simval(val_get_typdef(useval),
				 obj_get_nsid(parm),
				 obj_get_name(parm),
				 VAL_STR(useval),
				 res);
	return newval;
    }	


    /* since the CLI and original NCX language were never designed
     * to support top-level leafs, a dummy container must be
     * created for the new and old leaf or leaf-list entries
     */
    if (parm->parent && !obj_is_root(parm->parent)) {
	parentobj = parm->parent;
    } else {
	parentobj = ncx_get_gen_container();
    }

    dummy = val_new_value();
    if (!dummy) {
	*res = ERR_INTERNAL_MEM;
	log_error("\nError: malloc failed");
	return NULL;
    }
    val_init_from_template(dummy, parentobj);

    agent_cb->cli_fn = obj_get_name(rpc);

    newval = NULL;
    saveopt = agent_cb->get_optional;
    agent_cb->get_optional = TRUE;

    set_completion_state(&agent_cb->completion_state,
			 rpc,
			 parm,
			 CMD_STATE_GETVAL);

    *res = get_parm(agent_cb, rpc, parm, dummy, NULL);
    agent_cb->get_optional = saveopt;

    if (*res == NO_ERR) {
	newval = val_get_first_child(dummy);
	if (newval) {
	    val_remove_child(newval);
	}
    }
    agent_cb->cli_fn = NULL;
    val_free_value(dummy);
    return newval;

} /* fill_value */


/********************************************************************
* FUNCTION fill_valset
* 
* Fill the specified value set with any missing parameters.
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
*   agent_cb == current agent control block (NULL if none)
*   rpc == RPC method that is being called
*   valset == value set to fill
*   oldvalset == last set of values (or NULL if none)
*
* OUTPUTS:
*    new val_value_t nodes may be added to valset
*
* RETURNS:
*    status,, valset may be partially filled if not NO_ERR
*********************************************************************/
static status_t
    fill_valset (agent_cb_t *agent_cb,
		 const obj_template_t *rpc,
		 val_value_t *valset,
		 val_value_t *oldvalset)
{
    const obj_template_t  *parm;
    val_value_t           *val, *oldval;
    xmlChar               *objbuff;
    status_t               res;
    boolean                done;
    uint32                 yesnocode;

    res = NO_ERR;
    agent_cb->cli_fn = obj_get_name(rpc);

    if (obj_is_data_db(valset->obj)) {
	objbuff = NULL;
	res = obj_gen_object_id(valset->obj, &objbuff);
	if (res != NO_ERR) {
	    log_error("\nError: generate object ID failed (%s)",
		      get_error_string(res));
	    return res;
	}

	/* let the user know about the new nest level */
	log_stdout("\nFilling %s %s:",
		   obj_get_typestr(valset->obj), objbuff);
	m__free(objbuff);
    }

    for (parm = obj_first_child(valset->obj);
         parm != NULL && res==NO_ERR;
         parm = obj_next_child(parm)) {

	if (!obj_is_config(parm) || obj_is_abstract(parm)) {
	    continue;
	}

	if (!agent_cb->get_optional && !obj_is_mandatory(parm)) {
	    continue;
	}

	set_completion_state(&agent_cb->completion_state,
			     rpc,
			     parm,
			     CMD_STATE_GETVAL);			     

        switch (parm->objtype) {
        case OBJ_TYP_CHOICE:
	    if (!val_choice_is_set(valset, parm)) {
		res = get_choice(agent_cb, rpc, parm, 
				 valset, oldvalset);
		switch (res) {
		case NO_ERR:
		    break;
		case ERR_NCX_SKIPPED:
		    res = NO_ERR;
		    break;
		case ERR_NCX_CANCELED:
		    break;
		default:
		    ;
		}
	    }
            break;
        case OBJ_TYP_LEAF:
	    val = val_find_child(valset, 
				 obj_get_mod_name(parm),
				 obj_get_name(parm));
	    if (!val) {
		res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
		switch (res) {
		case NO_ERR:
		    break;
		case ERR_NCX_SKIPPED:
		    res = NO_ERR;
		    break;
		case ERR_NCX_CANCELED:
		    break;
		default:
		    ;
		}
	    }
	    break;
	case OBJ_TYP_LEAF_LIST:
	    done = FALSE;
	    while (!done && res == NO_ERR) {
		res = get_parm(agent_cb, rpc, parm, valset, oldvalset);
		switch (res) {
		case NO_ERR:
		    /* prompt for more leaf-list objects */
		    res = get_yesno(agent_cb, YANGCLI_PR_LLIST,
				    YESNO_NO, &yesnocode);
		    if (res == NO_ERR) {
			switch (yesnocode) {
			case YESNO_CANCEL:
			    res = ERR_NCX_CANCELED;
			    break;
			case YESNO_YES:
			    break;
			case YESNO_NO:
			    done = TRUE;
			    break;
			default:
			    res = SET_ERROR(ERR_INTERNAL_VAL);
			}
		    }
		    break;
		case ERR_NCX_SKIPPED:
		    done = TRUE;
		    res = NO_ERR;
		    break;
		case ERR_NCX_CANCELED:
		    break;
		default:
		    ;
		}
	    }
	    break;
	case OBJ_TYP_CONTAINER:
	case OBJ_TYP_NOTIF:
	case OBJ_TYP_RPCIO:
	    /* if the parm is not already set and is not read-only
	     * then try to get a value from the user at the CLI
	     */
	    val = val_find_child(valset, 
				 obj_get_mod_name(parm),
				 obj_get_name(parm));
	    if (val) {
		break;
	    }
			
	    if (oldvalset) {
		oldval = val_find_child(oldvalset, 
					obj_get_mod_name(parm),
					obj_get_name(parm));
	    } else {
		oldval = NULL;
	    }

	    val = val_new_value();
	    if (!val) {
		res = ERR_INTERNAL_MEM;
		log_error("\nError: malloc of new value failed");
		break;
	    } else {
		val_init_from_template(val, parm);
		val_add_child(val, valset);
	    }

	    /* recurse with the child nodes */
	    res = fill_valset(agent_cb, rpc, val, oldval);

	    switch (res) {
	    case NO_ERR:
		break;
	    case ERR_NCX_SKIPPED:
		res = NO_ERR;
		break;
	    case ERR_NCX_CANCELED:
		break;
	    default:
		;
	    }
	    break;
	case OBJ_TYP_LIST:
	    done = FALSE;
	    while (!done && res == NO_ERR) {
		val = val_new_value();
		if (!val) {
		    res = ERR_INTERNAL_MEM;
		    log_error("\nError: malloc of new value failed");
		    continue;
		} else {
		    val_init_from_template(val, parm);
		    val_add_child(val, valset);
		}

		/* recurse with the child node -- NO OLD VALUE
		 * TBD: get keys, then look up old matching entry
		 */
		res = fill_valset(agent_cb, rpc, val, NULL);

		switch (res) {
		case NO_ERR:
		    /* prompt for more list entries */
		    res = get_yesno(agent_cb, YANGCLI_PR_LIST,
				    YESNO_NO, &yesnocode);
		    if (res == NO_ERR) {
			switch (yesnocode) {
			case YESNO_CANCEL:
			    res = ERR_NCX_CANCELED;
			    break;
			case YESNO_YES:
			    break;
			case YESNO_NO:
			    done = TRUE;
			    break;
			default:
			    res = SET_ERROR(ERR_INTERNAL_VAL);
			}
		    }
		    break;
		case ERR_NCX_SKIPPED:
		    res = NO_ERR;
		    break;
		case ERR_NCX_CANCELED:
		    break;
		default:
		    ;
		}
	    }
            break;
        default: 
            res = SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    agent_cb->cli_fn = NULL;
    return res;

} /* fill_valset */


/********************************************************************
 * FUNCTION get_valset
 * 
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the command being processed
 *    line == CLI input in progress
 *    res == address of status result
 *
 * OUTPUTS:
 *    *res is set to the status
 *
 * RETURNS:
 *   malloced valset filled in with the parameters for 
 *   the specified RPC
 *
 *********************************************************************/
static val_value_t *
    get_valset (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		status_t  *res)
{
    const obj_template_t  *obj;
    val_value_t           *valset;
    uint32                 len;

    *res = NO_ERR;
    valset = NULL;
    len = 0;

    set_completion_state(&agent_cb->completion_state,
			 rpc,
			 NULL,
			 CMD_STATE_GETVAL);

    /* skip leading whitespace */
    while (line[len] && xml_isspace(line[len])) {
	len++;
    }

    /* check any non-whitespace entered after RPC method name */
    if (line[len]) {
	valset = parse_rpc_cli(rpc, &line[len], res);
	if (*res == ERR_NCX_SKIPPED) {
	    log_stdout("\nError: no parameters defined for RPC %s",
		       obj_get_name(rpc));
	} else if (*res != NO_ERR) {
	    log_stdout("\nError in the parameters for RPC %s (%s)",
		       obj_get_name(rpc), get_error_string(*res));
	}
    }

    obj = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!obj || !obj_get_child_count(obj)) {
	*res = ERR_NCX_SKIPPED;
	if (valset) {
	    val_free_value(valset);
	}
	return NULL;
    }

    /* check no input from user, so start a parmset */
    if (*res == NO_ERR && !valset) {
	valset = val_new_value();
	if (!valset) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    val_init_from_template(valset, obj);
	}
    }

    /* fill in any missing parameters from the CLI */
    if (*res==NO_ERR && interactive_mode()) {
	*res = fill_valset(agent_cb, rpc, valset, NULL);
    }

    return valset;

}  /* get_valset */


/********************************************************************
* FUNCTION create_session
* 
* Start a NETCONF session and change the program state
* Since this is called in sequence with readline, the STDIN IO
* handler may get called if the user enters keyboard text 
*
* The STDIN handler will not do anything with incoming chars
* while state == MGR_IO_ST_CONNECT
* 
* INPUTS:
*   agent_cb == agent control block to use
*
* OUTPUTS:
*   'agent_cb->mysid' is set to the output session ID, if NO_ERR
*   'agent_cb->state' is changed based on the success of the session setup
*
*********************************************************************/
static void
    create_session (agent_cb_t *agent_cb)
{
    const xmlChar *agent, *username, *password;
    val_value_t   *val;
    status_t       res;
    uint16         port;

    if (agent_cb->mysid) {
	if (mgr_ses_get_scb(agent_cb->mysid)) {
	    SET_ERROR(ERR_INTERNAL_INIT_SEQ);
	    return;
	} else {
	    /* session was reset */
	    agent_cb->mysid = 0;
	}
    }

    /* retrieving the parameters should not fail */
    val =  val_find_child(agent_cb->connect_valset, 
			  YANGCLI_MOD, 
			  YANGCLI_USER);
    if (val && val->res == NO_ERR) {
	username = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    val = val_find_child(agent_cb->connect_valset,
			 YANGCLI_MOD, 
			 YANGCLI_AGENT);
    if (val && val->res == NO_ERR) {
	agent = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    val = val_find_child(agent_cb->connect_valset,
			 YANGCLI_MOD, 
			 YANGCLI_PASSWORD);
    if (val && val->res == NO_ERR) {
	password = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    port = 0;
    val = val_find_child(agent_cb->connect_valset,
			 YANGCLI_MOD, 
			 YANGCLI_PORT);
    if (val && val->res == NO_ERR) {
	port = VAL_UINT16(val);
    }

    log_info("\nyangcli: Starting NETCONF session for %s on %s",
	     username, agent);

    agent_cb->state = MGR_IO_ST_CONNECT;

    /* this function call will cause us to block while the
     * protocol layer connect messages are processed
     */
    res = mgr_ses_new_session(username, password, 
			      agent, port, 
			      &agent_cb->mysid);
    if (res == NO_ERR) {
	agent_cb->state = MGR_IO_ST_CONN_START;
	log_debug("\nyangcli: Start session %d OK for agent '%s'", 
		  agent_cb->mysid, agent_cb->name);
    } else {
	log_info("\nyangcli: Start session failed for user %s on "
		 "%s (%s)\n", username, agent, get_error_string(res));
	agent_cb->state = MGR_IO_ST_IDLE;
    }
    
} /* create_session */



/********************************************************************
* FUNCTION send_copy_config_to_agent
* 
* Send a <copy-config> operation to the agent to support
* the save operation
*
* INPUTS:
*    agent_cb == agent control block to use
*
* OUTPUTS:
*    state may be changed or other action taken
*    config_content is consumed -- freed or transfered
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_copy_config_to_agent (agent_cb_t *agent_cb)
{
    const obj_template_t  *rpc, *input, *child;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *parm, *target, *source;
    ses_cb_t              *scb;
    status_t               res;

    req = NULL;
    reqdata = NULL;
    res = NO_ERR;
    rpc = NULL;

    /* get the <copy-config> template */
    rpc = ncx_find_object(get_netconf_mod(), 
			  NCX_EL_COPY_CONFIG);
    if (!rpc) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* get the 'input' section container */
    input = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!input) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* construct a method + parameter tree */
    reqdata = xml_val_new_struct(obj_get_name(rpc), 
				 obj_get_nsid(rpc));
    if (!reqdata) {
	log_error("\nError allocating a new RPC request");
	return ERR_INTERNAL_MEM;
    }

    /* set the edit-config/input/target node to the default_target */
    child = obj_find_child(input, NC_MODULE, NCX_EL_TARGET);
    parm = val_new_value();
    if (!parm) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);

    target = xml_val_new_flag((const xmlChar *)"startup",
			      obj_get_nsid(child));
    if (!target) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_add_child(target, parm);

    /* set the edit-config/input/default-operation node to 'none' */
    child = obj_find_child(input, NC_MODULE, NCX_EL_SOURCE);
    parm = val_new_value();
    if (!parm) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);

    source = xml_val_new_flag((const xmlChar *)"running",
			      obj_get_nsid(child));
    if (!source) {
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_add_child(source, parm);

    /* allocate an RPC request and send it */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    } else {
	req = mgr_rpc_new_request(scb);
	if (!req) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nError allocating a new RPC request");
	} else {
	    req->data = reqdata;
	    req->rpc = rpc;
	    req->timeout = agent_cb->timeout;
	}
    }
	
    if (res == NO_ERR) {
	if (LOGDEBUG2) {
	    log_debug2("\nabout to send RPC request with reqdata:");
	    val_dump_value(reqdata, NCX_DEF_INDENT);
	}

	/* the request will be stored if this returns NO_ERR */
	res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
    }

    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_copy_config_to_agent */


/********************************************************************
 * FUNCTION do_save
 * 
 * INPUTS:
 *    agent_cb == agent control block to use
 *
 * OUTPUTS:
 *   copy-config and/or commit operation will be sent to agent
 *
 *********************************************************************/
static void
    do_save (agent_cb_t *agent_cb)
{
    const ses_cb_t   *scb;
    const mgr_scb_t  *mscb;
    xmlChar          *line;
    status_t          res;

    /* get the session info */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    mscb = (const mgr_scb_t *)scb->mgrcb;

    log_info("\nSaving configuration to non-volative storage");

    /* determine which commands to send */
    switch (mscb->targtyp) {
    case NCX_AGT_TARG_NONE:
	log_stdout("\nWarning: No writable targets supported on this agent");
	break;
    case NCX_AGT_TARG_CANDIDATE:
    case NCX_AGT_TARG_CAND_RUNNING:
	line = xml_strdup(NCX_EL_COMMIT);
	if (line) {
	    conn_command(agent_cb, line);
#ifdef NOT_YET
	    if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
		log_stdout(" + copy-config <running> <startup>");
	    }
#endif
	    m__free(line);
	} else {
	    log_stdout("\nError: Malloc failed");
	}
	break;
    case NCX_AGT_TARG_RUNNING:
	if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
	    res = send_copy_config_to_agent(agent_cb);
	    if (res != NO_ERR) {
		log_stdout("\nError: send copy-config failed (%s)",
			   get_error_string(res));
	    }
	} else {
	    log_stdout("\nWarning: No distinct save operation needed "
		       "for this agent");
	}
	break;
    case NCX_AGT_TARG_LOCAL:
	log_stdout("Error: Local URL target not supported");
	break;
    case NCX_AGT_TARG_REMOTE:
	log_stdout("Error: Local URL target not supported");
	break;
    default:
	log_stdout("Error: Internal target not set");
	SET_ERROR(ERR_INTERNAL_VAL);
    }

}  /* do_save */


/********************************************************************
 * FUNCTION do_mgrload (local RPC)
 * 
 * mgrload module=mod-name
 *
 * Get the module parameter and load the specified NCX module
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   specified module is loaded into the definition registry, if NO_ERR
 *
 *********************************************************************/
static void
    do_mgrload (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		uint32  len)
{
    val_value_t     *valset, *val;
    ncx_module_t    *mod;
    modptr_t        *modptr;
    dlq_hdr_t       *mgrloadQ;
    logfn_t          logfn;
    status_t         res;

    val = NULL;
    res = NO_ERR;

    if (interactive_mode()) {
	logfn = log_stdout;
    } else {
	logfn = log_write;
    }
	
    valset = get_valset(agent_cb, 
			rpc, 
			&line[len], 
			&res);

    /* get the module name */
    if (res == NO_ERR) {
	val = val_find_child(valset, 
			     YANGCLI_MOD, 
			     NCX_EL_MODULE);
	if (!val) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else if (val->res != NO_ERR) {
	    res = val->res;
	}
    }

    /* check if the module is loaded already */
    if (res == NO_ERR) {
	mod = ncx_find_module(VAL_STR(val), NULL);
	if (mod) {
	    if (mod->version) {
		(*logfn)("\nModule '%s' revision '%s' already loaded",
			 mod->name, 
			 mod->version);
	    } else {
		(*logfn)("\nModule '%s' already loaded",
			 mod->name);
	    }
	    if (valset) {
		val_free_value(valset);
	    }
	    return;
	}
    }

    /* load the module */
    if (res == NO_ERR) {
	mod = NULL;
	res = ncxmod_load_module(VAL_STR(val), 
				 NULL, 
				 &mod);
	if (res == NO_ERR) {
	    modptr = new_modptr(mod);
	    if (!modptr) {
		res = ERR_INTERNAL_MEM;
	    } else {
		mgrloadQ = get_mgrloadQ();
		dlq_enque(modptr, mgrloadQ);
	    }
	}
    }

    /* print the result to stdout */
    if (res == NO_ERR) {
	(*logfn)("\nLoad module %s OK", VAL_STR(val));
    } else {
	(*logfn)("\nError: Load module failed (%s)",
		 get_error_string(res));
    }
    (*logfn)("\n");

    if (valset) {
	val_free_value(valset);
    }

}  /* do_mgrload */


/********************************************************************
 * FUNCTION do_show_vars (sub-mode of local RPC)
 * 
 * show brief info for all user variables
 *
 * INPUTS:
 *  mode == help mode requested
 *  shortmode == TRUE if printing just global or local variables
 *               FALSE to print everything
 *  isglobal == TRUE if print just globals
 *              FALSE to print just locals
 *              Ignored unless shortmode==TRUE
 * isany == TRUE to choose global or local
 *          FALSE to use 'isglobal' valuse only
 *********************************************************************/
static void
    do_show_vars (help_mode_t mode,
		  boolean shortmode,
		  boolean isglobal,
		  boolean isany)

{
    ncx_var_t    *var;
    val_value_t  *mgrset;
    dlq_hdr_t    *que;
    logfn_t       logfn;
    boolean       first, imode;

    imode = interactive_mode();
    if (imode) {
	logfn = log_stdout;
    } else {
	logfn = log_write;
    }

    mgrset = get_mgr_cli_valset();

    if (mode > HELP_MODE_BRIEF && !shortmode) {
	/* CLI Parameters */
	if (mgrset && val_child_cnt(mgrset)) {
	    (*logfn)("\nCLI Variables\n");
	    if (imode) {
		val_stdout_value(mgrset, NCX_DEF_INDENT);
	    } else {
		val_dump_value(mgrset, NCX_DEF_INDENT);
	    }
	    (*logfn)("\n");
	} else {
	    (*logfn)("\nNo CLI variables\n");
	}
    }

    /* System Script Variables */
    if (!shortmode) {
	que = runstack_get_que(ISGLOBAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {

	    if (var->vartype != VAR_TYP_SYSTEM) {
		continue;
	    }

	    if (first) {
		(*logfn)("\nRead-only system variables");
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		(*logfn)("\n   %s (%s)", 
			 var->name,
			 tk_get_btype_sym(var->val->btyp));
		if (mode == HELP_MODE_FULL) {
		    if (imode) {
			val_stdout_value(var->val, NCX_DEF_INDENT);
		    } else {
			val_dump_value(var->val, NCX_DEF_INDENT);
		    }
		}
	    }
	}
	if (first) {
	    (*logfn)("\nNo read-only system variables");
	}
	(*logfn)("\n");
    }

    /* System Config Variables */
    if (!shortmode || isglobal) {
	que = runstack_get_que(ISGLOBAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {

	    if (var->vartype != VAR_TYP_CONFIG) {
		continue;
	    }

	    if (first) {
		(*logfn)("\nRead-write system variables");
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		(*logfn)("\n   %s (%s)", 
			 var->name,
			 tk_get_btype_sym(var->val->btyp));
		if (mode == HELP_MODE_FULL) {
		    if (imode) {
			val_stdout_value(var->val, NCX_DEF_INDENT);
		    } else {
			val_dump_value(var->val, NCX_DEF_INDENT);
		    }
		}
	    }
	}
	if (first) {
	    (*logfn)("\nNo system config variables");
	}
	(*logfn)("\n");
    }

    /* Global Script Variables */
    if (!shortmode || isglobal) {
	que = runstack_get_que(ISGLOBAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {

	    if (var->vartype != VAR_TYP_GLOBAL) {
		continue;
	    }

	    if (first) {
		(*logfn)("\nGlobal variables");
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		(*logfn)("\n   %s (%s)", 
			 var->name,
			 tk_get_btype_sym(var->val->btyp));
		if (mode == HELP_MODE_FULL) {
		    if (imode) {
			val_stdout_value(var->val, NCX_DEF_INDENT);
		    } else {
			val_dump_value(var->val, NCX_DEF_INDENT);
		    }
		}
	    }
	}
	if (first) {
	    (*logfn)("\nNo global variables");
	}
	(*logfn)("\n");
    }

    /* Local Script Variables */
    if (!shortmode || !isglobal || isany) {
	que = runstack_get_que(ISLOCAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {
	    if (first) {
		(*logfn)("\nLocal variables");
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		/* just print the data type name for complex types */
		(*logfn)("\n   %s (%s)", 
			 var->name,
			 tk_get_btype_sym(var->val->btyp));
		if (mode == HELP_MODE_FULL) {
		    if (imode) {
			val_stdout_value(var->val, NCX_DEF_INDENT);
		    } else {
			val_dump_value(var->val, NCX_DEF_INDENT);
		    }
		}
	    }
	}
	if (first) {
	    (*logfn)("\nNo local variables");
	}
	(*logfn)("\n");
    }

} /* do_show_vars */


/********************************************************************
 * FUNCTION do_show_var (sub-mode of local RPC)
 * 
 * show full info for one user var
 *
 * INPUTS:
 *   name == variable name to find 
 *   isglobal == TRUE if global var, FALSE if local var
 *   isany == TRUE if don't care (global or local)
 *         == FALSE to force local or global with 'isglobal'
 *   mode == help mode requested
 *********************************************************************/
static void
    do_show_var (const xmlChar *name,
		 var_type_t vartype,
		 boolean isany,
		 help_mode_t mode)
{
    const val_value_t *val;
    logfn_t            logfn;
    boolean            imode;

    imode = interactive_mode();
    if (imode) {
	logfn = log_stdout;
    } else {
	logfn = log_write;
    }

    if (isany) {
	/* skipping VAR_TYP_SESSION for now */
	val = var_get_local(name);
	if (!val) {
	    val = var_get(name, VAR_TYP_GLOBAL);
	    if (!val) {
		val = var_get(name, VAR_TYP_CONFIG);
		if (!val) {
		    val = var_get(name, VAR_TYP_SYSTEM);
		}
	    }
	}
    } else {
	val = var_get(name, vartype);
    }

    if (val) {
	if (mode == HELP_MODE_BRIEF) {
	    if (typ_is_simple(val->btyp)) {
		if (imode) {
		    val_stdout_value(val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(val, NCX_DEF_INDENT);
		}
	    } else {
		(*logfn)("\n  %s (complex type)");
	    }
	} else {
	    if (imode) {
		val_stdout_value(val, NCX_DEF_INDENT);
	    } else {
		val_dump_value(val, NCX_DEF_INDENT);
	    }
	}
	(*logfn)("\n");
    } else {
	(*logfn)("\nVariable '%s' not found", name);
    }

} /* do_show_var */


/********************************************************************
 * FUNCTION do_show_module (sub-mode of local RPC)
 * 
 * show module=mod-name
 *
 * INPUTS:
 *    mod == module to show
 *    mode == requested help mode
 * 
 *********************************************************************/
static void
    do_show_module (const ncx_module_t *mod,
		    help_mode_t mode)
{
    help_data_module(mod, mode);

} /* do_show_module */


/********************************************************************
 * FUNCTION do_show_one_module (sub-mode of show modules RPC)
 * 
 * for 1 of N: show modules
 *
 * INPUTS:
 *    mod == module to show
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_show_one_module (ncx_module_t *mod,
			help_mode_t mode)
{
    boolean        anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    if (mode == HELP_MODE_BRIEF) {
	if (imode) {
	    log_stdout("\n  %s", mod->name); 
	} else {
	    log_write("\n  %s", mod->name); 
	}
    } else if (mode == HELP_MODE_NORMAL) {
	if (imode) {
	    if (mod->version) {
		log_stdout("\n  %s:%s/%s", mod->prefix, 
			   mod->name, mod->version);
	    } else {
		log_stdout("\n  %s:%s", mod->prefix, 
			   mod->name);
	    }
	} else {
	    if (mod->version) {
		log_write("\n  %s/%s", mod->name, mod->version);
	    } else {
		log_write("\n  %s", mod->name);
	    }
	}
    } else {
	help_data_module(mod, HELP_MODE_BRIEF);
    }
}  /* do_show_one_module */


/********************************************************************
 * FUNCTION do_show_modules (sub-mode of local RPC)
 * 
 * show modules
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_show_modules (agent_cb_t *agent_cb,
		     help_mode_t mode)
{
    ncx_module_t  *mod;
    modptr_t      *modptr;
    boolean        anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    if (use_agentcb(agent_cb)) {
	for (modptr = (modptr_t *)dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (modptr_t *)dlq_nextEntry(modptr)) {

	    do_show_one_module(modptr->mod, mode);
	    anyout = TRUE;
	}
	for (modptr = (modptr_t *)dlq_firstEntry(get_mgrloadQ());
	     modptr != NULL;
	     modptr = (modptr_t *)dlq_nextEntry(modptr)) {

	    do_show_one_module(modptr->mod, mode);
	    anyout = TRUE;
	}
    } else {
	mod = ncx_get_first_module();
	while (mod) {
	    do_show_one_module(mod, mode);
	    anyout = TRUE;
	    mod = ncx_get_next_module(mod);
	}
    }

    if (anyout) {
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    } else {
	if (imode) {
	    log_stdout("\nyangcli: no modules loaded\n");
	} else {
	    log_error("\nyangcli: no modules loaded\n");
	}
    }

} /* do_show_modules */


/********************************************************************
 * FUNCTION do_show_one_object (sub-mode of show objects local RPC)
 * 
 * show objects: 1 of N
 *
 * INPUTS:
 *    obj == object to show
 *    mode == requested help mode
 *    anyout == address of return anyout status
 *
 * OUTPUTS:
 *    *anyout set to TRUE only if any suitable objects found
 *********************************************************************/
static void
    do_show_one_object (const obj_template_t *obj,
			help_mode_t mode,
			boolean *anyout)
{
    boolean               imode;

    imode = interactive_mode();

    if (obj_is_data_db(obj) && 
	obj_has_name(obj) &&
	!obj_is_hidden(obj) && !obj_is_abstract(obj)) {

	if (mode == HELP_MODE_BRIEF) {
	    if (imode) {
		log_stdout("\n%s:%s",
			   obj_get_mod_name(obj),
			   obj_get_name(obj));
	    } else {
		log_write("\n%s:%s",
			  obj_get_mod_name(obj),
			  obj_get_name(obj));
	    }
	} else {
	    obj_dump_template(obj, mode-1, 0, 0); 
	}
	*anyout = TRUE;
    }

} /* do_show_one_object */


/********************************************************************
 * FUNCTION do_show_objects (sub-mode of local RPC)
 * 
 * show objects
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_show_objects (agent_cb_t *agent_cb,
		     help_mode_t mode)
{
    const ncx_module_t   *mod;
    const obj_template_t *obj;
    const modptr_t       *modptr;
    boolean               anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    if (use_agentcb(agent_cb)) {
	for (modptr = (const modptr_t *)
		 dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    for (obj = ncx_get_first_object(modptr->mod);
		 obj != NULL;
		 obj = ncx_get_next_object(modptr->mod, obj)) {

		do_show_one_object(obj, mode, &anyout);
	    }
	}

	for (modptr = (const modptr_t *)
		 dlq_firstEntry(get_mgrloadQ());
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    for (obj = ncx_get_first_object(modptr->mod);
		 obj != NULL;
		 obj = ncx_get_next_object(modptr->mod, obj)) {

		do_show_one_object(obj, mode, &anyout);
	    }
	}
    } else {
	mod = ncx_get_first_module();
	while (mod) {
	    for (obj = ncx_get_first_object(mod);
		 obj != NULL;
		 obj = ncx_get_next_object(mod, obj)) {

		do_show_one_object(obj, mode, &anyout);
	    }
	    mod = (const ncx_module_t *)ncx_get_next_module(mod);
	}
    }
    if (anyout) {
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    }

} /* do_show_objects */


/********************************************************************
 * FUNCTION do_show (local RPC)
 * 
 * show module=mod-name
 *      modules
 *      def=def-nmae
 *
 * Get the specified parameter and show the internal info,
 * based on the parameter
 *
 * INPUTS:
 * agent_cb == agent control block to use
 *    rpc == RPC method for the show command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 *********************************************************************/
static void
    do_show (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t        *valset, *parm;
    const ncx_module_t *mod;
    status_t            res;
    boolean             imode, done;
    help_mode_t         mode;

    imode = interactive_mode();
    valset = get_valset(agent_cb, rpc, &line[len], &res);

    if (valset && res == NO_ERR) {
	mode = HELP_MODE_NORMAL;

	/* check if the 'brief' flag is set first */
	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      YANGCLI_BRIEF);
	if (parm && parm->res == NO_ERR) {
	    mode = HELP_MODE_BRIEF;
	} else {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD, 
				  YANGCLI_FULL);
	    if (parm && parm->res == NO_ERR) {
		mode = HELP_MODE_FULL;
	    }
	}
	    
	/* get the 1 of N 'showtype' choice */
	done = FALSE;
	parm = val_find_child(valset, 
			      YANGCLI_MOD,
			      YANGCLI_LOCAL);
	if (parm) {
	    do_show_var(VAL_STR(parm), VAR_TYP_LOCAL, FALSE, mode);
	    done = TRUE;
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  YANGCLI_LOCALS);
	    if (parm) {
		do_show_vars(mode, TRUE, FALSE, FALSE);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  YANGCLI_OBJECTS);
	    if (parm) {
		do_show_objects(agent_cb, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  YANGCLI_GLOBAL);
	    if (parm) {
		do_show_var(VAL_STR(parm), 
			    VAR_TYP_GLOBAL, 
			    FALSE, 
			    mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  YANGCLI_GLOBALS);
	    if (parm) {
		do_show_vars(mode, TRUE, TRUE, FALSE);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  YANGCLI_VAR);
	    if (parm) {
		do_show_var(VAL_STR(parm), VAR_TYP_NONE, TRUE, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  YANGCLI_VARS);
	    if (parm) {
		do_show_vars(mode, FALSE, FALSE, TRUE);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  YANGCLI_MODULE);
	    if (parm) {
		mod = find_module(agent_cb, VAL_STR(parm));
		if (mod) {
		    do_show_module(mod, mode);
		} else {
		    if (imode) {
			log_stdout("\nyangcli: module (%s) not loaded",
				   VAL_STR(parm));
		    } else {
			log_error("\nyangcli: module (%s) not loaded",
				  VAL_STR(parm));
		    }
		}
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  YANGCLI_MODULES);
	    if (parm) {
		do_show_modules(agent_cb, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD,
				  NCX_EL_VERSION);
	    if (parm) {
		if (imode) {
		    log_stdout("\nyangcli version %s\n", YANGCLI_PROGVER);
		} else {
		    log_write("\nyangcli version %s\n", YANGCLI_PROGVER);
		}
		done = TRUE;
	    }
	}
    }

    if (valset) {
	val_free_value(valset);
    }

}  /* do_show */


/********************************************************************
 * FUNCTION do_list_one_oid (sub-mode of list oids RPC)
 * 
 * list oids: 1 of N
 *
 * INPUTS:
 *    obj == the object to use
 *********************************************************************/
static void
    do_list_one_oid (const obj_template_t *obj)
{
    xmlChar      *buffer;
    boolean       imode;
    status_t      res;

    if (obj_is_data_db(obj) && 
	obj_has_name(obj) &&
	!obj_is_hidden(obj) && 
	!obj_is_abstract(obj)) {

	imode = interactive_mode();
	buffer = NULL;
	res = obj_gen_object_id(obj, &buffer);
	if (res != NO_ERR) {
	    log_error("\nError: list OID failed (%s)",
		      get_error_string(res));
	} else {
	    if (imode) {
		log_stdout("\n   %s", buffer);
	    } else {
		log_write("\n   %s", buffer);
	    }
	}
	if (buffer) {
	    m__free(buffer);
	}
    }

} /* do_list_one_oid */


/********************************************************************
 * FUNCTION do_list_oid (sub-mode of local RPC)
 * 
 * list oids
 *
 * INPUTS:
 *    obj == object to use
 *    nestlevel to stop at
 *********************************************************************/
static void
    do_list_oid (const obj_template_t *obj,
		 uint32 level)
{
    const obj_template_t  *chobj;

    if (obj_get_level(obj) <= level) {
	do_list_one_oid(obj);
	for (chobj = obj_first_child(obj);
	     chobj != NULL;
	     chobj = obj_next_child(chobj)) {
	    do_list_oid(chobj, level);
	}
    }

} /* do_list_oid */


/********************************************************************
 * FUNCTION do_list_oids (sub-mode of local RPC)
 * 
 * list oids
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mod == the 1 module to use
 *           NULL to use all available modules instead
 *    mode == requested help mode
 *********************************************************************/
static void
    do_list_oids (agent_cb_t *agent_cb,
		  const ncx_module_t *mod,
		  help_mode_t mode)
{
    const modptr_t        *modptr;
    const obj_template_t  *obj;
    boolean                imode;
    uint32                 level;


    switch (mode) {
    case HELP_MODE_NONE:
	return;
    case HELP_MODE_BRIEF:
	level = 1;
	break;
    case HELP_MODE_NORMAL:
	level = 5;
	break;
    case HELP_MODE_FULL:
	level = 999;
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    imode = interactive_mode();

    if (mod) {
	obj = ncx_get_first_object(mod);
	while (obj) {
	    do_list_oid(obj, level);
	    obj = ncx_get_next_object(mod, obj);
	}
    } else if (use_agentcb(agent_cb)) {
	for (modptr = (const modptr_t *)
		 dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    obj = ncx_get_first_object(modptr->mod);
	    while (obj) {
		do_list_oid(obj, level);
		obj = ncx_get_next_object(modptr->mod, obj);
	    }
	}
    } else {
	return;
    }
    if (imode) {
	log_stdout("\n");
    } else {
	log_write("\n");
    }


} /* do_list_oids */


/********************************************************************
 * FUNCTION do_list_one_command (sub-mode of list command RPC)
 * 
 * list commands: 1 of N
 *
 * INPUTS:
 *    obj == object template for the RPC command to use
 *    mode == requested help mode
 *********************************************************************/
static void
    do_list_one_command (const obj_template_t *obj,
			 help_mode_t mode)
{
    if (interactive_mode()) {
	if (mode == HELP_MODE_BRIEF) {
	    log_stdout("\n   %s", obj_get_name(obj));
	} else if (mode == HELP_MODE_NORMAL) {
	    log_stdout("\n   %s:%s",
		       obj_get_mod_prefix(obj),
		       obj_get_name(obj));
	} else {
	    log_stdout("\n   %s:%s",
		       obj_get_mod_name(obj),
		       obj_get_name(obj));
	}
    } else {
	if (mode == HELP_MODE_BRIEF) {
	    log_write("\n   %s", obj_get_name(obj));
	} else if (mode == HELP_MODE_NORMAL) {
	    log_write("\n   %s:%s",
		      obj_get_mod_prefix(obj),
		      obj_get_name(obj));
	} else {
	    log_write("\n   %s:%s",
		      obj_get_mod_name(obj),
		      obj_get_name(obj));
	}
    }

} /* do_list_one_command */


/********************************************************************
 * FUNCTION do_list_objects (sub-mode of local RPC)
 * 
 * list objects
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mod == the 1 module to use
 *           NULL to use all available modules instead
 *    mode == requested help mode
 *********************************************************************/
static void
    do_list_objects (agent_cb_t *agent_cb,
		     const ncx_module_t *mod,
		     help_mode_t mode)
{
    const modptr_t        *modptr;
    const obj_template_t  *obj;
    logfn_t                logfn;
    boolean                anyout;

    anyout = FALSE;
    if (interactive_mode()) {
	logfn = log_stdout;
    } else {
	logfn = log_write;
    }

    if (mod) {
	obj = ncx_get_first_object(mod);
	while (obj) {
	    if (obj_is_data_db(obj) && 
		obj_has_name(obj) &&
		!obj_is_hidden(obj) && 
		!obj_is_abstract(obj)) {
		anyout = TRUE;
		do_list_one_command(obj, mode);
	    }
	    obj = ncx_get_next_object(mod, obj);
	}
    } else {
	if (use_agentcb(agent_cb)) {
	    for (modptr = (const modptr_t *)
		     dlq_firstEntry(&agent_cb->modptrQ);
		 modptr != NULL;
		 modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

		obj = ncx_get_first_object(modptr->mod);
		while (obj) {
		    if (obj_is_data_db(obj) && 
			obj_has_name(obj) &&
			!obj_is_hidden(obj) && 
			!obj_is_abstract(obj)) {
			anyout = TRUE;			
			do_list_one_command(obj, mode);
		    }
		    obj = ncx_get_next_object(modptr->mod, obj);
		}
	    }
	}

	for (modptr = (const modptr_t *)
		 dlq_firstEntry(get_mgrloadQ());
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    obj = ncx_get_first_object(modptr->mod);
	    while (obj) {
		if (obj_is_data_db(obj) && 
		    obj_has_name(obj) &&
		    !obj_is_hidden(obj) && 
		    !obj_is_abstract(obj)) {
		    anyout = TRUE;		    
		    do_list_one_command(obj, mode);
		}
		obj = ncx_get_next_object(modptr->mod, obj);
	    }
	}
    }

    if (!anyout) {
	(*logfn)("\nNo objects found to list");
    }

    (*logfn)("\n");

} /* do_list_objects */


/********************************************************************
 * FUNCTION do_list_commands (sub-mode of local RPC)
 * 
 * list commands
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mod == the 1 module to use
 *           NULL to use all available modules instead
 *    mode == requested help mode
 *********************************************************************/
static void
    do_list_commands (agent_cb_t *agent_cb,
		      const ncx_module_t *mod,
		      help_mode_t mode)
{
    const modptr_t        *modptr;
    const obj_template_t  *obj;
    logfn_t                logfn;
    boolean                imode, anyout;

    imode = interactive_mode();
    if (imode) {
	logfn = log_stdout;
    } else {
	logfn = log_write;
    }

    if (mod) {
	anyout = FALSE;
	obj = ncx_get_first_object(mod);
	while (obj) {
	    if (obj_is_rpc(obj)) {
		do_list_one_command(obj, mode);
		anyout = TRUE;
	    }
	    obj = ncx_get_next_object(mod, obj);
	}
	if (!anyout) {
	    (*logfn)("\nNo commands found in module '%s'",
		     mod->name);
	}
    } else {
	if (use_agentcb(agent_cb)) {
	    (*logfn)("\nAgent Commands:");
	
	    for (modptr = (const modptr_t *)
		     dlq_firstEntry(&agent_cb->modptrQ);
		 modptr != NULL;
		 modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

		obj = ncx_get_first_object(modptr->mod);
		while (obj) {
		    if (obj_is_rpc(obj)) {
			do_list_one_command(obj, mode);
		    }
		    obj = ncx_get_next_object(modptr->mod, obj);
		}
	    }
	}

	(*logfn)("\n\nLocal Commands:");

	obj = ncx_get_first_object(get_yangcli_mod());
	while (obj) {
	    if (obj_is_rpc(obj)) {
		if (use_agentcb(agent_cb)) {
		    /* list a local command */
		    do_list_one_command(obj, mode);
		} else {
		    /* session not active so filter out
		     * all the commands except top command
		     */
		    if (is_top_command(obj_get_name(obj))) {
			do_list_one_command(obj, mode);
		    }
		}
	    }
	    obj = ncx_get_next_object(get_yangcli_mod(), obj);
	}
    }

    (*logfn)("\n");

} /* do_list_commands */


/********************************************************************
 * FUNCTION do_list (local RPC)
 * 
 * list objects   [module=mod-name]
 *      ids
 *      commands
 *
 * List the specified information based on the parameters
 *
 * INPUTS:
 * agent_cb == agent control block to use
 *    rpc == RPC method for the show command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 *********************************************************************/
static void
    do_list (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t        *valset, *parm;
    const ncx_module_t *mod;
    status_t            res;
    boolean             imode, done;
    help_mode_t         mode;

    mod = NULL;
    done = FALSE;

    imode = interactive_mode();

    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (valset && res == NO_ERR) {
	mode = HELP_MODE_NORMAL;

	/* check if the 'brief' flag is set first */
	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      YANGCLI_BRIEF);
	if (parm && parm->res == NO_ERR) {
	    mode = HELP_MODE_BRIEF;
	} else {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD, 
				  YANGCLI_FULL);
	    if (parm && parm->res == NO_ERR) {
		mode = HELP_MODE_FULL;
	    }
	}

	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      YANGCLI_MODULE);
	if (parm && parm->res == NO_ERR) {
	    mod = find_module(agent_cb, VAL_STR(parm));
	    if (!mod) {
		if (imode) {
		    log_stdout("\nError: no module found named '%s'",
			       VAL_STR(parm)); 
		} else {
		    log_write("\nError: no module found named '%s'",
			      VAL_STR(parm)); 
		}
		done = TRUE;
	    }
	}

	/* find the 1 of N choice */
	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD, 
				  YANGCLI_COMMANDS);
	    if (parm) {
		/* do list commands */
		do_list_commands(agent_cb, mod, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD, 
				  YANGCLI_OBJECTS);
	    if (parm) {
		/* do list objects */
		do_list_objects(agent_cb, mod, mode);
		done = TRUE;
	    }
	}

	if (!done) {
	    parm = val_find_child(valset, 
				  YANGCLI_MOD, 
				  YANGCLI_OIDS);
	    if (parm) {
		/* do list oids */
		do_list_oids(agent_cb, mod, mode);
		done = TRUE;
	    }
	}
    }

    if (valset) {
	val_free_value(valset);
    }

}  /* do_list */


/********************************************************************
 * FUNCTION do_help_commands (sub-mode of local RPC)
 * 
 * help commands
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_help_commands (agent_cb_t *agent_cb,
		      help_mode_t mode)
{
    const modptr_t        *modptr;
    const obj_template_t  *obj;
    boolean                anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    if (use_agentcb(agent_cb)) {
	if (imode) {
	    log_stdout("\nAgent Commands:\n");
	} else {
	    log_write("\nAgent Commands:\n");
	}

	for (modptr = (const modptr_t *)
		 dlq_firstEntry(&agent_cb->modptrQ);
	     modptr != NULL;
	     modptr = (const modptr_t *)dlq_nextEntry(modptr)) {

	    obj = ncx_get_first_object(modptr->mod);
	    while (obj) {
		if (obj_is_rpc(obj)) {
		    if (mode == HELP_MODE_BRIEF) {
			obj_dump_template(obj, mode, 1, 0);
		    } else {
			obj_dump_template(obj, mode, 0, 0);
		    }
		    anyout = TRUE;
		}
		obj = ncx_get_next_object(modptr->mod, obj);
	    }
	}
    }

    if (imode) {
	log_stdout("\nLocal Commands:\n");
    } else {
	log_write("\nLocal Commands:\n");
    }

    obj = ncx_get_first_object(get_yangcli_mod());
    while (obj) {
	if (obj_is_rpc(obj)) {
	    if (mode == HELP_MODE_BRIEF) {
		obj_dump_template(obj, mode, 1, 0);
	    } else {
		obj_dump_template(obj, mode, 0, 0);
	    }
	    anyout = TRUE;
	}
	obj = ncx_get_next_object(get_yangcli_mod(), obj);
    }

    if (anyout) {
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    }

} /* do_help_commands */


/********************************************************************
 * FUNCTION do_help
 * 
 * Print the general yangcli help text to STDOUT
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    do_help (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    const typ_template_t *typ;
    const obj_template_t *obj;
    val_value_t          *valset, *parm;
    status_t              res;
    help_mode_t           mode;
    boolean               imode, done;
    ncx_node_t            dtyp;
    uint32                dlen;

    imode = interactive_mode();
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    done = FALSE;
    mode = HELP_MODE_NORMAL;

    /* look for the 'brief' parameter */
    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_BRIEF);
    if (parm && parm->res == NO_ERR) {
	mode = HELP_MODE_BRIEF;
    } else {
	/* look for the 'full' parameter */
	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      YANGCLI_FULL);
	if (parm && parm->res == NO_ERR) {
	    mode = HELP_MODE_FULL;
	}
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_COMMAND);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(agent_cb, &dtyp, VAL_STR(parm), &dlen);
	if (obj && obj->objtype == OBJ_TYP_RPC) {
	    help_object(obj, mode);
	} else {
	    if (imode) {
		log_stdout("\nyangcli: command (%s) not found",
			   VAL_STR(parm));
	    } else {
		log_error("\nyangcli: command (%s) not found",
			  VAL_STR(parm));
	    }
	}
	val_free_value(valset);
	return;
    }

    /* look for the specific definition parameters */
    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_COMMANDS);
    if (parm && parm->res==NO_ERR) {
	do_help_commands(agent_cb, mode);
	val_free_value(valset);
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  NCX_EL_TYPE);
    if (parm && parm->res==NO_ERR) {
	dtyp = NCX_NT_TYP;
	typ = parse_def(agent_cb, &dtyp, VAL_STR(parm), &dlen);
	if (typ) {
	    help_type(typ, mode);
	} else {
	    if (imode) {
		log_stdout("\nyangcli: type definition (%s) not found",
			   VAL_STR(parm));
	    } else {
		log_error("\nyangcli: type definition (%s) not found",
			  VAL_STR(parm));
	    }
	}
	val_free_value(valset);
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  NCX_EL_OBJECT);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(agent_cb, &dtyp, VAL_STR(parm), &dlen);
	if (obj && obj_is_data(obj)) {
	    help_object(obj, mode);
	} else {
	    if (imode) {
		log_stdout("\nyangcli: object definition (%s) not found",
			   VAL_STR(parm));
	    } else {
		log_error("\nyangcli: object definition (%s) not found",
			  VAL_STR(parm));
	    }
	}
	val_free_value(valset);
	return;
    }


    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  NCX_EL_NOTIF);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(agent_cb, &dtyp, VAL_STR(parm), &dlen);
	if (obj && obj->objtype == OBJ_TYP_NOTIF) {
	    help_object(obj, mode);
	} else {
	    if (imode) {
		log_stdout("\nyangcli: notification definition (%s) not found",
			   VAL_STR(parm));
	    } else {
		log_error("\nyangcli: notification definition (%s) not found",
			  VAL_STR(parm));
	    }
	}
	val_free_value(valset);
	return;
    }


    /* no parameters entered except maybe brief or full */
    switch (mode) {
    case HELP_MODE_BRIEF:
    case HELP_MODE_NORMAL:
	log_stdout("\n\nyangcli summary:");
	log_stdout("\n\n  Commands are defined with YANG rpc statements.");
	log_stdout("\n  Use 'help commands' to see current list of commands.");
	log_stdout("\n\n  Global variables are created with 2 dollar signs"
		   "\n  in assignment statements ($$foo = 7).");
	log_stdout("\n  Use 'show globals' to see current list "
		   "of global variables.");
	log_stdout("\n\n  Local variables (within a stack frame) are created"
		   "\n  with 1 dollar sign in assignment"
		   " statements ($foo = $bar).");
	log_stdout("\n  Use 'show locals' to see current list "
		   "of local variables.");
	log_stdout("\n\n  Use 'show vars' to see all program variables.\n");

	if (mode==HELP_MODE_BRIEF) {
	    break;
	}

	obj = ncx_find_object(get_yangcli_mod(), YANGCLI_HELP);
	if (obj && obj->objtype == OBJ_TYP_RPC) {
	    help_object(obj, HELP_MODE_FULL);
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
	break;
    case HELP_MODE_FULL:
	help_program_module(YANGCLI_MOD, 
			    YANGCLI_BOOT, 
			    HELP_MODE_FULL);
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
    }

    val_free_value(valset);

}  /* do_help */


/********************************************************************
 * FUNCTION do_start_script (sub-mode of run local RPC)
 * 
 * run script=script-filespec
 *
 * run the specified script
 *
 * INPUTS:
 *   agent_cb == agent control block to use
 *   source == file source
 *   valset == value set for the run script parameters
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    do_start_script (agent_cb_t *agent_cb,
		     const xmlChar *source,
		     val_value_t *valset)
{
    xmlChar       *str, *fspec;
    FILE          *fp;
    val_value_t   *parm;
    status_t       res;
    int            num;
    xmlChar        buff[4];

    /* saerch for the script */
    fspec = ncxmod_find_script_file(source, &res);
    if (!fspec) {
	return res;
    }

    /* open a new runstack frame if the file exists */
    fp = fopen((const char *)fspec, "r");
    if (!fp) {
	m__free(fspec);
	return ERR_NCX_MISSING_FILE;
    } else {
	res = runstack_push(fspec, fp);
	m__free(fspec);
	if (res != NO_ERR) {
	    fclose(fp);
	    return res;
	}
    }
    
    /* setup the numeric script parameters */
    memset(buff, 0x0, 4);
    buff[0] = 'P';

    /* add the P1 through P9 parameters that are present */
    for (num=1; num<=YANGCLI_MAX_RUNPARMS; num++) {
	buff[1] = '0' + num;
	parm = (valset) ? val_find_child(valset, 
					 YANGCLI_MOD, 
					 buff) : NULL;
	if (parm) {
	    /* store P7 named as ASCII 7 */
	    res = var_set_str(buff+1, 1, parm, VAR_TYP_LOCAL);
	    if (res != NO_ERR) {
		runstack_pop();
		return res;
	    }
	}
    }

    /* execute the first command in the script */
    str = runstack_get_cmd(&res);
    if (str && res == NO_ERR) {
	/* execute the line as an RPC command */
	if (is_top(agent_cb->state)) {
	    top_command(agent_cb, str);
	} else {
	    conn_command(agent_cb, str);
	}
    }

    return res;

}  /* do_start_script */


/********************************************************************
 * FUNCTION do_run (local RPC)
 * 
 * run script=script-filespec
 *
 *
 * Get the specified parameter and run the specified script
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the show command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    do_run (agent_cb_t *agent_cb,
	    const obj_template_t *rpc,
	    const xmlChar *line,
	    uint32  len)
{
    val_value_t  *valset, *parm;
    status_t      res;

    res = NO_ERR;

    /* get the 'script' parameter */
    valset = get_valset(agent_cb, rpc, &line[len], &res);

    if (valset && res == NO_ERR) {
	/* there is 1 parm */
	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      NCX_EL_SCRIPT);
	if (!parm) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else if (parm->res != NO_ERR) {
	    res = parm->res;
	} else {
	    /* the parm val is the script filespec */
	    res = do_start_script(agent_cb, VAL_STR(parm), valset);
	    if (res != NO_ERR) {
		log_write("\nError: start script %s failed (%s)",
			  obj_get_name(rpc),
			  get_error_string(res));
	    }
	}
    }

    if (valset) {
	val_free_value(valset);
    }

    return res;

}  /* do_run */




/********************************************************************
 * FUNCTION pwd
 * 
 * Print the current working directory
 *
 * INPUTS:
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    pwd (void)
{
    char             *buff;
    boolean           imode;

    imode = interactive_mode();

    buff = m__getMem(YANGCLI_BUFFLEN);
    if (!buff) {
	if (imode) {
	    log_stdout("\nMalloc failure\n");
	} else {
	    log_write("\nMalloc failure\n");
	}
	return;
    }

    if (!getcwd(buff, YANGCLI_BUFFLEN)) {
	if (imode) {
	    log_stdout("\nGet CWD failure\n");
	} else {
	    log_write("\nGet CWD failure\n");
	}
	m__free(buff);
	return;
    }

    if (imode) {
	log_stdout("\nCurrent working directory is %s\n", buff);
    } else {
	log_write("\nCurrent working directory is %s\n", buff);
    }

    m__free(buff);

}  /* pwd */


/********************************************************************
 * FUNCTION do_pwd
 * 
 * Print the current working directory
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    do_pwd (agent_cb_t *agent_cb,
	    const obj_template_t *rpc,
	    const xmlChar *line,
	    uint32  len)
{
    val_value_t      *valset;
    status_t          res;
    boolean           imode;

    imode = interactive_mode();
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (res == NO_ERR || res == ERR_NCX_SKIPPED) {
	pwd();
    }
    if (valset) {
	val_free_value(valset);
    }

}  /* do_pwd */


/********************************************************************
 * FUNCTION do_cd
 * 
 * Change the current working directory
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    do_cd (agent_cb_t *agent_cb,
	   const obj_template_t *rpc,
	   const xmlChar *line,
	   uint32  len)
{
    val_value_t      *valset, *parm;
    status_t          res;
    int               ret;
    boolean           imode;

    imode = interactive_mode();
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_DIR);
    if (!parm || parm->res != NO_ERR) {
	val_free_value(valset);
	return;
    }

    ret = chdir((const char *)VAL_STR(parm));
    if (ret) {
	if (imode) {
	    log_stdout("\nChange CWD failure (%s)\n",
		       get_error_string(errno_to_status()));
	} else {
	    log_write("\nChange CWD failure (%s)\n",
		      get_error_string(errno_to_status()));
	}
    } else {
	pwd();
    }

    val_free_value(valset);

}  /* do_cd */


/********************************************************************
 * FUNCTION do_fill
 * 
 * Fill an object for use in a PDU
 *
 * INPUTS:
 *    agent_cb == agent control block to use (NULL if none)
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   is usually part of an assignment statement
 *
 *********************************************************************/
static void
    do_fill (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t           *valset, *parm, *newparm, *curparm;
    val_value_t           *targval;
    obj_template_t        *targobj;
    const xmlChar         *target;
    status_t               res;
    boolean                imode, save_getopt;

    valset = get_valset(agent_cb, 
			rpc, 
			&line[len], 
			&res);
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    target = NULL;
    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  NCX_EL_TARGET);
    if (!parm || parm->res != NO_ERR) {
	val_free_value(valset);
	return;
    } else {
	target = VAL_STR(parm);
    }


    save_getopt = agent_cb->get_optional;

    imode = interactive_mode();
    newparm = NULL;
    curparm = NULL;
    targobj = NULL;
    targval = NULL;

#define OLDWAY 1
#ifdef OLDWAY
    res = xpath_find_schema_target_int(target, &targobj);
    if (res != NO_ERR) {
	log_error("\nError: Object '%s' not found", target);
	val_free_value(valset);
	return;
    }	
#endif

#ifdef NEWWAY
    valroot = get_instanceid_parm(agent_cb,
				  target,
				  &targobj,
				  &targval,
				  &res);
    if (!valroot) {
	val_free_value(valset);
	return;
    }
#endif

    /* find the value to use as content value or template, if any */
    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_VALUE);
    if (parm && parm->res == NO_ERR) {
	curparm = parm;
    }

    /* find the --optional flag */
    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_OPTIONAL);
    if (parm && parm->res == NO_ERR) {
	agent_cb->get_optional = TRUE;
    }

    /* fill in the value based on all the parameters */
    switch (targobj->objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	newparm = fill_value(agent_cb, 
			     rpc, 
			     targobj, 
			     curparm, 
			     &res);
	break;
    case OBJ_TYP_CHOICE:
	newparm = val_new_value();
	if (!newparm) {
	    log_error("\nError: malloc failure");
	    res = ERR_INTERNAL_MEM;
	} else {
	    val_init_from_template(newparm, targobj);
	    
	    res = get_choice(agent_cb, 
			     rpc, 
			     targobj, 
			     newparm, 
			     curparm);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
	    }
	}
	break;
    case OBJ_TYP_CASE:
	newparm = val_new_value();
	if (!newparm) {
	    log_error("\nError: malloc failure");
	    res = ERR_INTERNAL_MEM;
	} else {
	    val_init_from_template(newparm, targobj);

	    res = get_case(agent_cb, 
			   rpc, 
			   targobj, 
			   newparm, 
			   curparm);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
	    }
	}
	break;
    default:
	newparm = val_new_value();
	if (!newparm) {
	    log_error("\nError: malloc failure");
	    res = ERR_INTERNAL_MEM;
	} else {
	    val_init_from_template(newparm, targobj);
	    res = fill_valset(agent_cb, 
			      rpc, 
			      newparm, 
			      curparm);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
	    }
	}
    }

    /* check save result or clear it */
    if (res == NO_ERR) {
	if (agent_cb->result_name || 
	    agent_cb->result_filename) {
	    /* save the filled in value */
	    res = finish_result_assign(agent_cb, 
				       newparm, 
				       NULL);
	    newparm = NULL;
	}
    } else {
	clear_result(agent_cb);
    }

    /* cleanup */
    val_free_value(valset);
    if (newparm) {
	val_free_value(newparm);
    }
    agent_cb->get_optional = save_getopt;

}  /* do_fill */


/********************************************************************
* FUNCTION add_content
* 
* Add the config nodes to the parent
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method in progress
*   config_content == the node associated with the target
*             to be used as content nested within the 
*             <config> element
*   curobj == the current object node for config_content, going
*                 up the chain to the root object.
*                 First call should pass config_content->obj
*   dofill == TRUE if interactive mode and mandatory parms
*             need to be specified (they can still be skipped)
*             FALSE if no checking for mandatory parms
*             Just fill out the minimum path to root from
*             the 'config_content' node
*   curtop == address of steady-storage node to add the
*             next new level
*
* OUTPUTS:
*    config node is filled in with child nodes
*    curtop is set to the latest top value
*       It is not needed after the last call and should be ignored
*
* RETURNS:
*    status; config_content is NOT freed if returning an error
*********************************************************************/
static status_t
    add_content (agent_cb_t *agent_cb,
		 const obj_template_t *rpc,
		 val_value_t *config_content,
		 const obj_template_t *curobj,
		 boolean dofill,
		 val_value_t **curtop)
{

    const obj_key_t       *curkey;
    val_value_t           *newnode, *keyval, *lastkey;
    status_t               res;
    boolean                done, content_used;

    res = NO_ERR;
    newnode = NULL;

    set_completion_state(&agent_cb->completion_state,
			 rpc,
			 curobj,
			 CMD_STATE_GETVAL);


    /* add content based on the current node type */
    switch (curobj->objtype) {
    case OBJ_TYP_LEAF:
    case OBJ_TYP_LEAF_LIST:
	if (curobj != config_content->obj) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	if (!obj_is_key(config_content->obj)) {
	    val_add_child(config_content, *curtop);
	    *curtop = config_content;
	}
	break;
    case OBJ_TYP_LIST:
	/* get all the key nodes for the current object,
	 * if they do not already exist
	 */
	if (curobj == config_content->obj) {
	    val_add_child(config_content, *curtop);
	    *curtop = config_content;
	    return NO_ERR;
	}

	/* else need to fill in the keys for this content layer */
	newnode = val_new_value();
	if (!newnode) {
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(newnode, curobj);
	val_add_child(newnode, *curtop);
	*curtop = newnode;
	content_used = FALSE;

	lastkey = NULL;
	for (curkey = obj_first_ckey(curobj);
	     curkey != NULL;
	     curkey = obj_next_ckey(curkey)) {

	    keyval = val_find_child(*curtop,
				    obj_get_mod_name(curkey->keyobj),
				    obj_get_name(curkey->keyobj));
	    if (!keyval) {
		if (curkey->keyobj == config_content->obj) {
		    keyval = config_content;
		    val_insert_child(keyval, lastkey, *curtop);
		    content_used = TRUE;
		    lastkey = keyval;
		    res = NO_ERR;
		} else if (dofill) {
		    res = get_parm(agent_cb, rpc, 
				   curkey->keyobj, *curtop, NULL);
		    if (res == ERR_NCX_SKIPPED) {
			res = NO_ERR;
		    } else if (res != NO_ERR) {
			return res;
		    } else {
			keyval = val_find_child(*curtop,
						obj_get_mod_name
						(curkey->keyobj),
						obj_get_name
						(curkey->keyobj));
			if (!keyval) {
			    return SET_ERROR(ERR_INTERNAL_VAL);
			}
			val_remove_child(keyval);
			val_insert_child(keyval, lastkey, *curtop);
			lastkey = keyval;
		    } /* else skip this key (for debugging agent) */
		}  /* else --nofill; skip this node */
	    }
	}

	/* wait until all the key leafs are accounted for before
	 * changing the *curtop pointer
	 */
	if (content_used) {
	    *curtop = config_content;
	}
	break;
    case OBJ_TYP_CONTAINER:
	if (curobj == config_content->obj) {
	    val_add_child(config_content, *curtop);
	    *curtop = config_content;
	} else {
	    newnode = val_new_value();
	    if (!newnode) {
		return ERR_INTERNAL_MEM;
	    }
	    val_init_from_template(newnode, curobj);
	    val_add_child(newnode, *curtop);
	    *curtop = newnode;
	}
	break;
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
	if (curobj != config_content->obj) {
	    /* nothing to do for the choice level if the target is a case */
	    res = NO_ERR;
	    break;
	}
	done = FALSE;
	while (!done) {
	    newnode = val_get_first_child(config_content);
	    if (newnode) {
		val_remove_child(newnode);
		res = add_content(agent_cb, 
				  rpc, 
				  newnode, 
				  newnode->obj, 
				  dofill,
				  curtop);
		if (res != NO_ERR) {
		    val_free_value(newnode);
		    done = TRUE;
		}
	    } else {
		done = TRUE;
	    }
	}
	if (res == NO_ERR) {
	    val_free_value(config_content);
	}
	*curtop = newnode;
	break;
    default:
	/* any other object type is an error */
	res = SET_ERROR(ERR_INTERNAL_VAL);
    }

    return res;

}  /* add_content */


/********************************************************************
* FUNCTION add_config_from_content_node
* 
* Add the config node content for the edit-config operation
* Build the <config> nodfe top-down, by recursing bottom-up
* from the node to be edited.
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method in progress
*   config_content == the node associated with the target
*             to be used as content nested within the 
*             <config> element
*   curobj == the current object node for config_content, going
*                 up the chain to the root object.
*                 First call should pass config_content->obj
*   config == the starting <config> node to add the data into
*   curtop == address of stable storage for current add-to node
*            This pointer MUST be set to NULL upon first fn call
* OUTPUTS:
*    config node is filled in with child nodes
*
* RETURNS:
*    status; config_content is NOT freed if returning an error
*********************************************************************/
static status_t
    add_config_from_content_node (agent_cb_t *agent_cb,
				  const obj_template_t *rpc,
				  val_value_t *config_content,
				  const obj_template_t *curobj,
				  val_value_t *config,
				  val_value_t **curtop)
{
    const obj_template_t  *parent;
    status_t               res;

    /* get to the root of the object chain */
    parent = obj_get_cparent(curobj);
    if (parent && !obj_is_root(parent)) {
	res = add_config_from_content_node(agent_cb,
					   rpc,
					   config_content,
					   parent,
					   config,
					   curtop);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* set the current target, working down the stack
     * on the way back from the initial dive
     */
    if (!*curtop) {
	/* first time through to this point */
	*curtop = config;
    }

    res = add_content(agent_cb, 
		      rpc, 
		      config_content, 
		      curobj, 
		      TRUE,
		      curtop);

    return res;

}  /* add_config_from_content_node */


/********************************************************************
* FUNCTION add_filter_from_content_node
* 
* Add the filter node content for the get or get-config operation
* Build the <filter> node top-down, by recursing bottom-up
* from the node to be edited.
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == RPC method in progress
*   get_content == the node associated with the target
*             to be used as content nested within the 
*             <filter> element
*   curobj == the current object node for get_content, going
*                 up the chain to the root object.
*                 First call should pass get_content->obj
*   filter == the starting <filter> node to add the data into
*   dofill == TRUE for fill mode
*             FALSE to skip filling any nodes
*   curtop == address of stable storage for current add-to node
*            This pointer MUST be set to NULL upon first fn call
* OUTPUTS:
*    filter node is filled in with child nodes
*
* RETURNS:
*    status; get_content is NOT freed if returning an error
*********************************************************************/
static status_t
    add_filter_from_content_node (agent_cb_t *agent_cb,
				  const obj_template_t *rpc,
				  val_value_t *get_content,
				  const obj_template_t *curobj,
				  val_value_t *filter,
				  boolean dofill,
				  val_value_t **curtop)
{
    const obj_template_t  *parent;
    status_t               res;

    /* get to the root of the object chain */
    parent = obj_get_cparent(curobj);
    if (parent && !obj_is_root(parent)) {
	res = add_filter_from_content_node(agent_cb,
					   rpc,
					   get_content,
					   parent,
					   filter,
					   dofill,
					   curtop);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* set the current target, working down the stack
     * on the way back from the initial dive
     */
    if (!*curtop) {
	/* first time through to this point */
	*curtop = filter;
    }

    res = add_content(agent_cb, 
		      rpc, 
		      get_content, 
		      curobj, 
		      dofill,
		      curtop);
    return res;

}  /* add_filter_from_content_node */


/********************************************************************
* FUNCTION send_edit_config_to_agent
* 
* Send an <edit-config> operation to the agent
*
* Fills out the <config> node based on the config_target node
* Any missing key nodes will be collected (via CLI prompt)
* along the way.
*
* INPUTS:
*   agent_cb == agent control block to use
*   config_content == the node associated with the target
*             to be used as content nested within the 
*             <config> element
*   timeoutval == timeout value to use
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*
*    !!! config_content is consumed -- freed or transfered to a PDU
*    !!! that will be freed later
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_edit_config_to_agent (agent_cb_t *agent_cb,
			       val_value_t *config_content,
			       uint32 timeoutval)
{
    const obj_template_t  *rpc, *input, *child;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *parm, *target, *dummy_parm;
    ses_cb_t              *scb;
    status_t               res;

    req = NULL;
    reqdata = NULL;
    res = NO_ERR;

    if (!agent_cb->default_target) {
	log_error("\nError: no <edit-config> target available on agent");
	val_free_value(config_content);
	return ERR_NCX_OPERATION_FAILED;
    }

    /* get the <edit-config> template */
    rpc = ncx_find_object(get_netconf_mod(), 
			  NCX_EL_EDIT_CONFIG);
    if (!rpc) {
	val_free_value(config_content);
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* get the 'input' section container */
    input = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!input) {
	val_free_value(config_content);
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* construct a method + parameter tree */
    reqdata = xml_val_new_struct(obj_get_name(rpc), 
				 obj_get_nsid(rpc));
    if (!reqdata) {
	val_free_value(config_content);
	log_error("\nError allocating a new RPC request");
	return ERR_INTERNAL_MEM;
    }

    /* set the edit-config/input/target node to the default_target */
    child = obj_find_child(input, NC_MODULE,
			   NCX_EL_TARGET);
    parm = val_new_value();
    if (!parm) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);

    target = xml_val_new_flag(agent_cb->default_target,
			      obj_get_nsid(child));
    if (!target) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }

    val_add_child(target, parm);

    /* set the edit-config/input/default-operation node to 'none' */
    child = obj_find_child(input, NC_MODULE,
			   NCX_EL_DEFAULT_OPERATION);
    parm = val_new_value();
    if (!parm) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);
    res = val_set_simval(parm,
			 obj_get_ctypdef(child),
			 obj_get_nsid(child),
			 obj_get_name(child),
			 NCX_EL_NONE);

    if (res != NO_ERR) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return res;
    }

    if (agent_cb->testoption != OP_TESTOP_NONE) {
	/* set the edit-config/input/test-option node to
	 * the user-specified value
	 */
	child = obj_find_child(input, NC_MODULE,
			       NCX_EL_TEST_OPTION);
	parm = val_new_value();
	if (!parm) {
	    val_free_value(config_content);
	    val_free_value(reqdata);
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(parm, child);
	val_add_child(parm, reqdata);
	res = val_set_simval(parm,
			     obj_get_ctypdef(child),
			     obj_get_nsid(child),
			     obj_get_name(child),
			     op_testop_name(agent_cb->testoption));
	if (res != NO_ERR) {
	    val_free_value(config_content);
	    val_free_value(reqdata);
	    return res;
	}
    }

    if (agent_cb->erroption != OP_ERROP_NONE) {
	/* set the edit-config/input/error-option node to
	 * the user-specified value
	 */
	child = obj_find_child(input, NC_MODULE,
			       NCX_EL_ERROR_OPTION);
	parm = val_new_value();
	if (!parm) {
	    val_free_value(config_content);
	    val_free_value(reqdata);
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(parm, child);
	val_add_child(parm, reqdata);
	res = val_set_simval(parm,
			     obj_get_ctypdef(child),
			     obj_get_nsid(child),
			     obj_get_name(child),
			     op_errop_name(agent_cb->erroption));
	if (res != NO_ERR) {
	    val_free_value(config_content);
	    val_free_value(reqdata);
	    return res;
	}
    }

    /* set the edit-config/input/config node to the
     * config_content, but after filling in any
     * missing nodes from the root to the target
     */
    child = obj_find_child(input, NC_MODULE,
			   NCX_EL_CONFIG);
    parm = val_new_value();
    if (!parm) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, child);
    val_add_child(parm, reqdata);

    dummy_parm = NULL;
    res = add_config_from_content_node(agent_cb,
				       rpc, config_content,
				       config_content->obj,
				       parm, &dummy_parm);
    if (res != NO_ERR) {
	val_free_value(config_content);
	val_free_value(reqdata);
	return res;
    }

    if (agent_cb->fixorder) {
	/* must set the order of a root container seperately */
	val_set_canonical_order(parm);
    }

    /* !!! config_content consumed at this point !!!
     * allocate an RPC request and send it 
     */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    } else {
	req = mgr_rpc_new_request(scb);
	if (!req) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nError allocating a new RPC request");
	} else {
	    req->data = reqdata;
	    req->rpc = rpc;
	    req->timeout = timeoutval;
	}
    }
	
    if (res == NO_ERR) {
	if (LOGDEBUG2) {
	    log_debug2("\nabout to send RPC request with reqdata:");
	    val_dump_value(reqdata, NCX_DEF_INDENT);
	}

	/* the request will be stored if this returns NO_ERR */
	res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
    }

    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_edit_config_to_agent */


/********************************************************************
 * FUNCTION add_filter_attrs
 * 
 * Add the type and possibly the select 
 * attribute to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    selectstr == select value string to use, (type=xpath)
 *               == NULL for type = subtree
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_filter_attrs (val_value_t *val,
		      const xmlChar *selectstr)
{
    val_value_t          *metaval;

    /* create a value node for the meta-value */
    metaval = val_make_string(0, NCX_EL_TYPE,
			      (selectstr) 
			      ?  NCX_EL_XPATH : NCX_EL_SUBTREE);
    if (!metaval) {
	return ERR_INTERNAL_MEM;
    }
    dlq_enque(metaval, &val->metaQ);

    if (selectstr) {
	metaval = val_make_string(0, NCX_EL_SELECT,  selectstr);
	if (!metaval) {
	    return ERR_INTERNAL_MEM;
	}
	dlq_enque(metaval, &val->metaQ);
    }

    return NO_ERR;

} /* add_filter_attrs */


/********************************************************************
* FUNCTION send_get_to_agent
* 
* Send an <get> operation to the specified agent
*
* Fills out the <filter> node based on the config_target node
* Any missing key nodes will be collected (via CLI prompt)
* along the way.
*
* INPUTS:
*   agent_cb == agent control block to use
*   get_content == the node associated with the target
*             to be used as content nested within the 
*             <filter> element
*             == NULL to use selectstr instead
*   selectstr == XPath select string
*             == NULL to use get_content instead
*   source == optional database source 
*             <candidate>, <running>
*   timeoutval == timeout value to use
*   dofill == TRUE if interactive mode and mandatory parms
*             need to be specified (they can still be skipped)
*             FALSE if no checking for mandatory parms
*             Just fill out the minimum path to root from
*             the 'get_content' node
*   withdef == the desired with-defaults parameter
*              It may be ignored or altered, depending on
*              whether the agent supports the capability or not
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*
*    !!! get_content is consumed -- freed or transfered to a PDU
*    !!! that will be freed later
*
*    !!! source is consumed -- freed or transfered to a PDU
*    !!! that will be freed later
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    send_get_to_agent (agent_cb_t *agent_cb,
		       val_value_t *get_content,
		       const xmlChar *selectstr,
		       val_value_t *source,
		       uint32 timeoutval,
		       boolean dofill,
		       ncx_withdefaults_t withdef)
{
    const obj_template_t  *rpc, *input, *withdefobj;
    val_value_t           *reqdata, *filter;
    val_value_t           *withdefval, *dummy_parm;
    mgr_rpc_req_t         *req;
    ses_cb_t              *scb;
    mgr_scb_t             *mscb;
    status_t               res;

    req = NULL;
    reqdata = NULL;
    res = NO_ERR;
    input = NULL;

    /* get the <get> or <get-config> input template */
    rpc = ncx_find_object(get_netconf_mod(),
			  (source) ? 
			  NCX_EL_GET_CONFIG : NCX_EL_GET);
    if (rpc) {
	input = obj_find_child(rpc, NULL, YANG_K_INPUT);
    }

    if (!input) {
	if (source) {
	    val_free_value(source);
	}
	if (get_content) {
	    val_free_value(get_content);
	}
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* construct a method + parameter tree */
    reqdata = xml_val_new_struct(obj_get_name(rpc), 
				 obj_get_nsid(rpc));
    if (!reqdata) {
	if (source) {
	    val_free_value(source);
	}
	if (get_content) {
	    val_free_value(get_content);
	}
	log_error("\nError allocating a new RPC request");
	return ERR_INTERNAL_MEM;
    }

    /* add /get-star/input/source */
    if (source) {
	val_add_child(source, reqdata);
    }


    /* add /get-star/input/filter */
    if (get_content) {
	/* set the get/input/filter node to the
	 * get_content, but after filling in any
	 * missing nodes from the root to the target
	 */
	filter = xml_val_new_struct(NCX_EL_FILTER, xmlns_nc_id());
    } else {
	filter = xml_val_new_flag(NCX_EL_FILTER, xmlns_nc_id());
    }
    if (!filter) {
	if (get_content) {
	    val_free_value(get_content);
	}
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }
    val_add_child(filter, reqdata);

    res = add_filter_attrs(filter, selectstr);
    if (res != NO_ERR) {
	if (get_content) {
	    val_free_value(get_content);
	}
	val_free_value(reqdata);
	return ERR_INTERNAL_MEM;
    }

    /* add the content to the filter element
     * building the path from the content node
     * to the root; fill if dofill is true
     */
    if (get_content) {
	dummy_parm = NULL;
	res = add_filter_from_content_node(agent_cb,
					   rpc, 
					   get_content,
					   get_content->obj,
					   filter, 
					   dofill,
					   &dummy_parm);
	if (res != NO_ERR && res != ERR_NCX_SKIPPED) {
	    /*  val_free_value(get_content); already freed! */
	    val_free_value(reqdata);
	    return res;
	}
	res = NO_ERR;
    }

    /* get the session control block */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	res = SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* !!! get_content consumed at this point !!!
     * check if the with-defaults parmaeter should be added
     */
    if (res == NO_ERR) {
	mscb = mgr_ses_get_mscb(scb);
	if (cap_std_set(&mscb->caplist, CAP_STDID_WITH_DEFAULTS)) {
	    switch (withdef) {
	    case NCX_WITHDEF_NONE:
		break;
	    case NCX_WITHDEF_TRIM:
	    case NCX_WITHDEF_EXPLICIT:
		/*** !!! NEED TO CHECK IF TRIM / EXPLICT 
		 *** !!! REALLY SUPPORTED IN THE caplist
		 ***/
		/* fall through */
	    case NCX_WITHDEF_REPORT_ALL:
		/* it is OK to send a with-defaults to this agent */
		withdefobj = obj_find_child(input, NULL,
					    NCX_EL_WITH_DEFAULTS);
		if (!withdefobj) {
		    SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
		} else {
		    withdefval 
			= val_make_simval(obj_get_ctypdef(withdefobj),
					  obj_get_nsid(withdefobj),
					  obj_get_name(withdefobj),
					  ncx_get_withdefaults_string(withdef),
					  &res);
		    if (withdefval) {
			val_add_child(withdefval, reqdata);
		    }
		}
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	} else {
	    log_warn("\nWarning: 'with-defaults' "
		     "capability not-supported so parameter ignored");
	}
    }

    if (res == NO_ERR) {
	/* allocate an RPC request and send it */
	req = mgr_rpc_new_request(scb);
	if (!req) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nError allocating a new RPC request");
	} else {
	    req->data = reqdata;
	    req->rpc = rpc;
	    req->timeout = timeoutval;
	}
    }
	
    if (res == NO_ERR) {
	if (LOGDEBUG2) {
	    log_debug2("\nabout to send RPC request with reqdata:");
	    val_dump_value(reqdata, NCX_DEF_INDENT);
	}

	/* the request will be stored if this returns NO_ERR */
	res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
    }

    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

    return res;

} /* send_get_to_agent */


/********************************************************************
 * FUNCTION get_content_from_choice
 * 
 * Get the content input for the EditOps from choice
 *
 * If the choice is 'from-cli' and this is interactive mode
 * then the fill_valset will be called to get input
 * based on the 'target' parameter also in the valset
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the load command
 *    valset == parsed CLI valset
 *    getoptional == TRUE if optional nodes are desired
 *    dofill == TRUE to fill the content,
 *              FALSE to skip fill phase
 * RETURNS:
 *   malloced result of the choice; this is the content
 *   that will be affected by the edit-config operation
 *   via create, merge, or replace
 *********************************************************************/
static val_value_t *
    get_content_from_choice (agent_cb_t *agent_cb,
			     const obj_template_t *rpc,
			     val_value_t *valset,
			     boolean getoptional,
			     boolean dofill)
{
    val_value_t           *parm, *curparm, *newparm;
    const val_value_t     *userval;
    obj_template_t        *targobj;
    const xmlChar         *fromstr;
    var_type_t             vartype;
    boolean                iscli, isselect, saveopt;
    status_t               res;

    /* init locals */
    targobj = NULL;
    iscli = FALSE;
    isselect = FALSE;
    fromstr = NULL;
    res = NO_ERR;
    vartype = VAR_TYP_NONE;

    /* look for the 'from' parameter variant */
    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_VARREF);
    if (parm) {
	fromstr = VAL_STR(parm);
    } else {
	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      NCX_EL_SELECT);
	if (parm) {
	    isselect = TRUE;
	    fromstr = VAL_STR(parm);
	} else {
	    iscli = TRUE;
	}
    }

    if (iscli) {
	saveopt = agent_cb->get_optional;
	agent_cb->get_optional = getoptional;

	/* from CLI -- look for the 'target' parameter */
	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      NCX_EL_TARGET);
	if (!parm) {
	    log_error("\nError: target parameter is missing");
	    agent_cb->get_optional = saveopt;
	    return NULL;
	}

	res = xpath_find_schema_target_int(VAL_STR(parm), 
					   &targobj);
	if (res != NO_ERR) {
	    log_error("\nError: Object '%s' not found", 
		      VAL_STR(parm));
	    agent_cb->get_optional = saveopt;
	    return NULL;
	}	

	curparm = NULL;
	parm = val_find_child(valset, 
			      YANGCLI_MOD, 
			      YANGCLI_VALUE);
	if (parm && parm->res == NO_ERR) {
	    curparm = var_get_script_val(targobj, 
					 NULL, 
					 VAL_STR(parm),
					 ISPARM, 
					 &res);
	    if (!curparm || res != NO_ERR) {
		log_error("\nError: Script value '%s' invalid (%s)", 
			  VAL_STR(parm), 
			  get_error_string(res)); 
		agent_cb->get_optional = saveopt;
		return NULL;
	    }
	    if (curparm->obj != targobj) {
		log_error("\nError: current value '%s' "
			  "object type is incorrect.",
			  VAL_STR(parm));
		agent_cb->get_optional = saveopt;
		return NULL;
	    }
	}

	switch (targobj->objtype) {
	case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    if (dofill) {
		newparm = fill_value(agent_cb, 
				     rpc, 
				     targobj, 
				     curparm, 
				     &res);
	    } else {
		newparm = val_new_value();
		if (!newparm) {
		    log_error("\nError: malloc failure");
		    res = ERR_INTERNAL_MEM;
		} else {
		    val_init_from_template(newparm, targobj);
		}
	    }
	    break;
	case OBJ_TYP_CHOICE:
	    newparm = val_new_value();
	    if (!newparm) {
		log_error("\nError: malloc failure");
		res = ERR_INTERNAL_MEM;
	    } else {
		val_init_from_template(newparm, targobj);

		if (dofill) {
		    res = get_choice(agent_cb, 
				     rpc, 
				     targobj,
				     newparm, 
				     curparm);
		}
	    }
	    break;
	case OBJ_TYP_CASE:
	    newparm = val_new_value();
	    if (!newparm) {
		log_error("\nError: malloc failure");
		res = ERR_INTERNAL_MEM;
	    } else {
		val_init_from_template(newparm, targobj);
		if (dofill) {
		    res = get_case(agent_cb, 
				   rpc, 
				   targobj,
				   newparm, 
				   curparm);
		}
	    }
	    break;
	default:
	    newparm = val_new_value();
	    if (!newparm) {
		log_error("\nError: malloc failure");
		res = ERR_INTERNAL_MEM;
	    } else {
		val_init_from_template(newparm, targobj);

		if (dofill) {
		    res = fill_valset(agent_cb, 
				      rpc,
				      newparm, 
				      curparm);
		}
	    }
	}

	agent_cb->get_optional = saveopt;
	if (res == ERR_NCX_SKIPPED) {
	    if (newparm) {
		val_free_value(newparm);
	    }
	    newparm = 
		xml_val_new_flag(obj_get_name(targobj),
				 obj_get_nsid(targobj));
	    if (!newparm) {
		log_error("\nError: malloc failure");
	    } else {
		/* need to set the real object so the
		 * path to root will be built correctly
		 */
		newparm->obj = targobj;
	    }
	    return newparm;
	} else if (res != NO_ERR) {
	    if (newparm) {
		val_free_value(newparm);
	    }
	    return NULL;
	} else {
	    return newparm;
	}
    } else if (isselect) {
	newparm = val_make_string(xmlns_nc_id(),
				  NCX_EL_SELECT, 
				  fromstr);
	if (!newparm) {
	    log_error("\nError: make string failed");
	}
	return newparm;
    } else {
	/* from global or local variable */
	if (!fromstr) {
	    ;  /* should not be NULL */
	} else if (*fromstr == '$' && fromstr[1] == '$') {
	    /* $$foo */
	    vartype = VAR_TYP_GLOBAL;
	    fromstr += 2;
	} else if (*fromstr == '$') {
	    /* $foo */
	    vartype = VAR_TYP_LOCAL;
	    fromstr++;
	} else {
	    /* 'foo' : just assume local, not error */
	    vartype = VAR_TYP_LOCAL;
	}
	if (fromstr) {
	    userval = var_get(fromstr, vartype);
	    if (!userval) {
		log_error("\nError: variable '%s' not found", 
			  fromstr);
		return NULL;
	    } else {
		newparm = val_clone(userval);
		if (!newparm) {
		    log_error("\nError: valloc failed");
		}
		return newparm;
	    }
	}
    }
    return NULL;

}  /* get_content_from_choice */


/********************************************************************
 * FUNCTION add_one_operation_attr
 * 
 * Add the nc:operation attribute to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    op == edit operation to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_one_operation_attr (val_value_t *val,
			    op_editop_t op)
{
    const obj_template_t *operobj;
    const xmlChar        *editopstr;
    val_value_t          *metaval;
    status_t              res;

    /* get the internal nc:operation object */
    operobj = ncx_find_object(get_netconf_mod(), 
			      NC_OPERATION_ATTR_NAME);
    if (!operobj) {
	return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* create a value node for the meta-value */
    metaval = val_new_value();
    if (!metaval) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(metaval, operobj);

    /* get the string value for the edit operation */
    editopstr = op_editop_name(op);
    if (!editopstr) {
	val_free_value(metaval);
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* set the meta variable value and other fields */
    res = val_set_simval(metaval,
			 obj_get_ctypdef(operobj),
			 obj_get_nsid(operobj),
			 obj_get_name(operobj),
			 editopstr);

    if (res != NO_ERR) {
	val_free_value(metaval);
	return res;
    }

    dlq_enque(metaval, &val->metaQ);
    return NO_ERR;

} /* add_one_operation_attr */


/********************************************************************
 * FUNCTION add_operation_attr
 * 
 * Add the nc:operation attribute to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    op == edit operation to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_operation_attr (val_value_t *val,
			op_editop_t op)
{
    val_value_t          *childval;
    status_t              res;

    res = NO_ERR;

    switch (val->obj->objtype) {
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
	for (childval = val_get_first_child(val);
	     childval != NULL;
	     childval = val_get_next_child(childval)) {

	    res = add_one_operation_attr(childval, op);
	    if (res != NO_ERR) {
		return res;
	    }
	}
	break;
    default:
	res = add_one_operation_attr(val, op);
    }

    return res;

} /* add_operation_attr */


/********************************************************************
 * FUNCTION add_one_insert_attrs
 * 
 * Add the yang:insert attribute(s) to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    insop == insert operation to use
 *    edit_target == edit target to use for key or value attr
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_one_insert_attrs (val_value_t *val,
			  op_insertop_t insop,
			  const xmlChar *edit_target)
{
    const obj_template_t *operobj;
    const xmlChar        *insopstr;
    val_value_t          *metaval;
    ncx_node_t            dtyp;
    status_t              res;
    xmlns_id_t            yangid;

    yangid = xmlns_yang_id();

    /* get the internal nc:operation object */
    dtyp = NCX_NT_OBJ;
    operobj = ncx_get_gen_string();
    if (!operobj) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    insopstr = op_insertop_name(insop);

    /* create a value node for the meta-value */
    metaval = val_new_value();
    if (!metaval) {
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(metaval, operobj);
    val_set_qname(metaval, yangid,
		  YANG_K_INSERT,
		  xml_strlen(YANG_K_INSERT));

    /* set the meta variable value and other fields */
    res = val_set_simval(metaval,
			 metaval->typdef,
			 yangid,
			 YANG_K_INSERT,
			 insopstr);
    if (res != NO_ERR) {
	val_free_value(metaval);
	return res;
    } else {
	dlq_enque(metaval, &val->metaQ);
    }

    if (insop == OP_INSOP_BEFORE || insop == OP_INSOP_AFTER) {
	/* create a value node for the meta-value */
	metaval = val_new_value();
	if (!metaval) {
	    return ERR_INTERNAL_MEM;
	}
	val_init_from_template(metaval, operobj);

	/* set the attribute name */
	if (val->obj->objtype==OBJ_TYP_LEAF_LIST) {
	    val_set_qname(metaval, yangid,
			  YANG_K_VALUE,
			  xml_strlen(YANG_K_VALUE));
	} else {
	    val_set_qname(metaval, yangid,
			  YANG_K_KEY,
			  xml_strlen(YANG_K_KEY));
	}

	/* set the meta variable value and other fields */
	res = val_set_simval(metaval,
			     metaval->typdef,
			     yangid, NULL,
			     edit_target);
	if (res != NO_ERR) {
	    val_free_value(metaval);
	    return res;
	} else {
	    dlq_enque(metaval, &val->metaQ);
	}
    }	  

    return NO_ERR;

} /* add_one_insert_attrs */


/********************************************************************
 * FUNCTION add_insert_attrs
 * 
 * Add the yang:insert attribute(s) to a value node
 *
 * INPUTS:
 *    val == value node to set
 *    insop == insert operation to use
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    add_insert_attrs (val_value_t *val,
		      op_insertop_t insop,
		      const xmlChar *edit_target)
{
    val_value_t          *childval;
    status_t              res;

    res = NO_ERR;

    switch (val->obj->objtype) {
    case OBJ_TYP_CHOICE:
    case OBJ_TYP_CASE:
	for (childval = val_get_first_child(val);
	     childval != NULL;
	     childval = val_get_next_child(childval)) {

	    res = add_one_insert_attrs(childval, insop,
				       edit_target);
	    if (res != NO_ERR) {
		return res;
	    }
	}
	break;
    default:
	res = add_one_insert_attrs(val, insop, edit_target);
    }

    return res;

} /* add_insert_attrs */


/********************************************************************
 * FUNCTION do_edit
 * 
 * Edit some database object on the agent
 * operation attribute:
 *   create/delete/merge/replace
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   an edit-config operation is sent to the agent
 *
 *********************************************************************/
static void
    do_edit (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len,
	     op_editop_t editop)
{
    val_value_t           *valset, *content, *parm;
    status_t               res;
    uint32                 timeoutval;
    boolean                getoptional, dofill;

    /* init locals */
    res = NO_ERR;
    content = NULL;
    dofill = TRUE;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_OPTIONAL);
    if (parm && parm->res == NO_ERR) {
	getoptional = TRUE;
    } else {
	getoptional = agent_cb->get_optional;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_NOFILL);
    if (parm && parm->res == NO_ERR) {
	dofill = FALSE;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, 
				      rpc, 
				      valset,
				      getoptional,
				      dofill);
    if (!content) {
	val_free_value(valset);
	return;
    }


    /* add nc:operation attribute to the value node */
    res = add_operation_attr(content, editop);
    if (res != NO_ERR) {
	log_error("\nError: Creation of nc:operation"
		  " attribute failed");
	val_free_value(valset);
	val_free_value(content);
	return;
    }

    /* construct an edit-config PDU with default parameters */
    res = send_edit_config_to_agent(agent_cb, 
				    content, 
				    timeoutval);
    if (res != NO_ERR) {
	log_error("\nError: send %s operation failed (%s)",
		  op_editop_name(editop),
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_edit */


/********************************************************************
 * FUNCTION do_insert
 * 
 * Insert a database object on the agent
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   an edit-config operation is sent to the agent
 *
 *********************************************************************/
static void
    do_insert (agent_cb_t *agent_cb,
	       const obj_template_t *rpc,
	       const xmlChar *line,
	       uint32  len)
{
    val_value_t      *valset, *content, *tempval, *parm;
    const xmlChar    *edit_target;
    op_editop_t       editop;
    op_insertop_t     insertop;
    status_t          res;
    uint32            timeoutval;
    boolean           getoptional, dofill;

    /* init locals */
    res = NO_ERR;
    content = NULL;
    dofill = TRUE;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_OPTIONAL);
    if (parm && parm->res == NO_ERR) {
	getoptional = TRUE;
    } else {
	getoptional = agent_cb->get_optional;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_NOFILL);
    if (parm && parm->res == NO_ERR) {
	dofill = FALSE;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, 
				      rpc, 
				      valset,
				      getoptional,
				      dofill);
    if (!content) {
	val_free_value(valset);
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    /* get the insert order */
    tempval = val_find_child(valset, 
			     YANGCLI_MOD,
			     YANGCLI_ORDER);
    if (tempval && tempval->res == NO_ERR) {
	insertop = op_insertop_id(VAL_ENUM_NAME(tempval));
    } else {
	insertop = OP_INSOP_LAST;
    }

    /* get the edit-config operation */
    tempval = val_find_child(valset, 
			     YANGCLI_MOD,
			     YANGCLI_OPERATION);
    if (tempval && tempval->res == NO_ERR) {
	editop = op_editop_id(VAL_ENUM_NAME(tempval));
    } else {
	editop = OP_EDITOP_MERGE;
    }

    /* get the edit-target parameter only if the
     * order is 'before' or 'after'; ignore otherwise
     */
    tempval = val_find_child(valset, 
			     YANGCLI_MOD,
			     YANGCLI_EDIT_TARGET);
    if (tempval && tempval->res == NO_ERR) {
	edit_target = VAL_STR(tempval);
    } else {
	edit_target = NULL;
    }

    /* check if the edit-target is needed */
    switch (insertop) {
    case OP_INSOP_BEFORE:
    case OP_INSOP_AFTER:
	if (!edit_target) {
	    log_error("\nError: edit-target parameter missing");
	    val_free_value(content);
	    val_free_value(valset);
	    return;
	}
	break;
    default:
	;
    }

    /* add nc:operation attribute to the value node */
    res = add_operation_attr(content, editop);
    if (res != NO_ERR) {
	log_error("\nError: Creation of nc:operation"
		  " attribute failed");
    }

    /* add yang:insert attribute and possibly a key or value
     * attribute as well
     */
    if (res == NO_ERR) {
	res = add_insert_attrs(content, insertop,
			       edit_target);
	if (res != NO_ERR) {
	    log_error("\nError: Creation of yang:insert"
		  " attribute(s) failed");
	}
    }

    /* send the PDU, hand off the content node */
    if (res == NO_ERR) {
	/* construct an edit-config PDU with default parameters */
	res = send_edit_config_to_agent(agent_cb, content, timeoutval);
	if (res != NO_ERR) {
	    log_error("\nError: send create operation failed (%s)",
		      get_error_string(res));
	}
	content = NULL;
    }

    /* cleanup and exit */
    if (content) {
	val_free_value(content);
    }

    val_free_value(valset);

}  /* do_insert */


/********************************************************************
 * FUNCTION do_sget
 * 
 * Get some running config and/or state data with the <get> operation,
 * using an optional subtree
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   a <get> operation is sent to the agent
 *
 *********************************************************************/
static void
    do_sget (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t           *valset, *content, *parm;
    status_t               res;
    uint32                 timeoutval;
    boolean                dofill, getoptional;
    ncx_withdefaults_t     withdef;

    /* init locals */
    res = NO_ERR;
    content = NULL;
    dofill = TRUE;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_NOFILL);
    if (parm && parm->res == NO_ERR) {
	dofill = FALSE;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_OPTIONAL);
    if (parm && parm->res == NO_ERR) {
	getoptional = TRUE;
    } else {
	getoptional = agent_cb->get_optional;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
	withdef = ncx_get_withdefaults_enum(VAL_STR(parm));
    } else {
	withdef = agent_cb->withdefaults;
    }
    
    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, 
				      rpc, 
				      valset,
				      getoptional,
				      dofill);
    if (!content) {
	val_free_value(valset);
	return;
    }

    /* construct a get PDU with the content as the filter */
    res = send_get_to_agent(agent_cb, 
			    content, 
			    NULL, 
			    NULL, 
			    timeoutval, 
			    dofill,
			    withdef);
    if (res != NO_ERR) {
	log_error("\nError: send get operation failed (%s)",
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_sget */


/********************************************************************
 * FUNCTION do_sget_config
 * 
 * Get some config data with the <get-config> operation,
 * using an optional subtree
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   a <get-config> operation is sent to the agent
 *
 *********************************************************************/
static void
    do_sget_config (agent_cb_t *agent_cb,
		    const obj_template_t *rpc,
		    const xmlChar *line,
		    uint32  len)
{
    val_value_t        *valset, *content, *source, *parm;
    status_t            res;
    uint32              timeoutval;
    boolean             dofill, getoptional;
    ncx_withdefaults_t  withdef;

    dofill = TRUE;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset,
			  YANGCLI_MOD,
			  YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_OPTIONAL);
    if (parm && parm->res == NO_ERR) {
	getoptional = TRUE;
    } else {
	getoptional = agent_cb->get_optional;
    }

    parm = val_find_child(valset,
			  YANGCLI_MOD,
			  YANGCLI_NOFILL);
    if (parm && parm->res == NO_ERR) {
	dofill = FALSE;
    }

    source = val_find_child(valset, NULL, NCX_EL_SOURCE);
    if (!source) {
	log_error("\nError: mandatory source parameter missing");
	val_free_value(valset);
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD,
			  NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
	withdef = ncx_get_withdefaults_enum(VAL_STR(parm));
    } else {
	withdef = agent_cb->withdefaults;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, 
				      rpc, 
				      valset,
				      getoptional,
				      dofill);
    if (!content) {
	val_free_value(valset);
	return;
    }

    /* hand off this malloced node to send_get_to_agent */
    val_remove_child(source);
    val_change_nsid(source, xmlns_nc_id());

    /* construct a get PDU with the content as the filter */
    res = send_get_to_agent(agent_cb, 
			    content, 
			    NULL, 
			    source, 
			    timeoutval, 
			    dofill,
			    withdef);
    if (res != NO_ERR) {
	log_error("\nError: send get-config operation failed (%s)",
		  get_error_string(res));
    }

    val_free_value(valset);

}  /* do_sget_config */


/********************************************************************
 * FUNCTION do_xget
 * 
 * Get some running config and/or state data with the <get> operation,
 * using an optional XPath filter
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   a <get> operation is sent to the agent
 *
 *********************************************************************/
static void
    do_xget (agent_cb_t *agent_cb,
	     const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    const ses_cb_t      *scb;
    const mgr_scb_t     *mscb;
    val_value_t         *valset, *content, *parm;
    const xmlChar       *str;
    status_t             res;
    uint32               retcode, timeoutval;
    ncx_withdefaults_t   withdef;

    /* get the session info */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    mscb = (const mgr_scb_t *)scb->mgrcb;

    /* init locals */
    res = NO_ERR;
    content = NULL;

    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset,
			  YANGCLI_MOD, 
			  YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
	withdef = ncx_get_withdefaults_enum(VAL_STR(parm));
    } else {
	withdef = agent_cb->withdefaults;
    }

    /* check if the agent supports :xpath */
    if (!cap_std_set(&mscb->caplist, CAP_STDID_XPATH)) {
	switch (agent_cb->baddata) {
	case NCX_BAD_DATA_IGNORE:
	    break;
	case NCX_BAD_DATA_WARN:
	    log_warn("\nWarning: agent does not have :xpath support");
	    break;
	case NCX_BAD_DATA_CHECK:
	    retcode = 0;
	    log_warn("\nWarning: agent does not have :xpath support");
	    res = get_yesno(agent_cb,
			    (const xmlChar *)"Send request anyway?",
			    YESNO_NO, &retcode);
	    if (res == NO_ERR) {
		switch (retcode) {
		case YESNO_CANCEL:
		case YESNO_NO:
		    res = ERR_NCX_CANCELED;
		    break;
		case YESNO_YES:
		    break;
		default:
		    res = SET_ERROR(ERR_INTERNAL_VAL);
		}
	    }
	    break;
	case NCX_BAD_DATA_ERROR:
	    log_error("\nError: agent does not have :xpath support");
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    break;
	case NCX_BAD_DATA_NONE:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* check any error so far */
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, 
				      rpc, 
				      valset,
				      FALSE,
				      FALSE);
    if (content) {
	if (content->btyp == NCX_BT_STRING && VAL_STR(content)) {
	    str = VAL_STR(content);
	    while (*str && *str != '"') {
		str++;
	    }
	    if (*str) {
		log_error("\nError: select string cannot "
			  "contain a double quote");
	    } else {
		/* construct a get PDU with the content
		 * as the filter 
		 */
		res = send_get_to_agent(agent_cb, 
					NULL, 
					VAL_STR(content), 
					NULL, 
					timeoutval,
					FALSE,
					withdef);
		if (res != NO_ERR) {
		    log_error("\nError: send get operation"
			      " failed (%s)",
			      get_error_string(res));
		}
	    }
	} else {
	    log_error("\nError: xget content wrong type");
	}
	val_free_value(content);
    }

    val_free_value(valset);

}  /* do_xget */


/********************************************************************
 * FUNCTION do_xget_config
 * 
 * Get some config data with the <get-config> operation,
 * using an optional XPath filter
 *
 * INPUTS:
 *    agent_cb == agent control block to use
 *    rpc == RPC method for the create command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   the completed data node is output and
 *   a <get-config> operation is sent to the agent
 *
 *********************************************************************/
static void
    do_xget_config (agent_cb_t *agent_cb,
		    const obj_template_t *rpc,
		    const xmlChar *line,
		    uint32  len)
{
    const ses_cb_t      *scb;
    const mgr_scb_t     *mscb;
    val_value_t         *valset, *content, *source, *parm;
    const xmlChar       *str;
    status_t             res;
    uint32               retcode, timeoutval;
    ncx_withdefaults_t   withdef;

    /* get the session info */
    scb = mgr_ses_get_scb(agent_cb->mysid);
    if (!scb) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }
    mscb = (const mgr_scb_t *)scb->mgrcb;

    /* init locals */
    res = NO_ERR;
    content = NULL;
      
    /* get the command line parameters for this command */
    valset = get_valset(agent_cb, rpc, &line[len], &res);
    if (!valset || res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  YANGCLI_TIMEOUT);
    if (parm && parm->res == NO_ERR) {
	timeoutval = VAL_UINT(parm);
    } else {
	timeoutval = agent_cb->timeout;
    }

    /* check if the agent supports :xpath */
    if (!cap_std_set(&mscb->caplist, CAP_STDID_XPATH)) {
	switch (agent_cb->baddata) {
	case NCX_BAD_DATA_IGNORE:
	    break;
	case NCX_BAD_DATA_WARN:
	    log_warn("\nWarning: agent does not have :xpath support");
	    break;
	case NCX_BAD_DATA_CHECK:
	    retcode = 0;
	    log_warn("\nWarning: agent does not have :xpath support");
	    res = get_yesno(agent_cb,
			    (const xmlChar *)"Send request anyway?",
			    YESNO_NO, &retcode);
	    if (res == NO_ERR) {
		switch (retcode) {
		case YESNO_CANCEL:
		case YESNO_NO:
		    res = ERR_NCX_CANCELED;
		    break;
		case YESNO_YES:
		    break;
		default:
		    res = SET_ERROR(ERR_INTERNAL_VAL);
		}
	    }
	    break;
	case NCX_BAD_DATA_ERROR:
	    log_error("\nError: agent does not have :xpath support");
	    res = ERR_NCX_OPERATION_NOT_SUPPORTED;
	    break;
	case NCX_BAD_DATA_NONE:
	default:
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    /* check any error so far */
    if (res != NO_ERR) {
	if (valset) {
	    val_free_value(valset);
	}
	return;
    }

    /* get the source parameter */
    source = val_find_child(valset, NULL, NCX_EL_SOURCE);
    if (!source) {
	log_error("\nError: mandatory source parameter missing");
	val_free_value(valset);
	return;
    }

    parm = val_find_child(valset, 
			  YANGCLI_MOD, 
			  NCX_EL_WITH_DEFAULTS);
    if (parm && parm->res == NO_ERR) {
	withdef = ncx_get_withdefaults_enum(VAL_STR(parm));
    } else {
	withdef = agent_cb->withdefaults;
    }

    /* get the contents specified in the 'from' choice */
    content = get_content_from_choice(agent_cb, 
				      rpc, 
				      valset,
				      FALSE,
				      FALSE);
    if (content) {
	if (content->btyp == NCX_BT_STRING && VAL_STR(content)) {
	    str = VAL_STR(content);
	    while (*str && *str != '"') {
		str++;
	    }
	    if (*str) {
		log_error("\nError: select string cannot "
			  "contain a double quote");
	    } else {
		/* hand off this malloced node to send_get_to_agent */
		val_remove_child(source);
		val_change_nsid(source, xmlns_nc_id());

		/* construct a get PDU with the content as the filter */
		res = send_get_to_agent(agent_cb, 
					NULL, 
					VAL_STR(content), 
					source,
					timeoutval,
					FALSE,
					withdef);
		if (res != NO_ERR) {
		    log_error("\nError: send get-config "
			      "operation failed (%s)",
			      get_error_string(res));
		}
	    }
	} else {
	    log_error("\nError: xget content wrong type");
	}
	val_free_value(content);
    }

    val_free_value(valset);

}  /* do_xget_config */


/********************************************************************
* FUNCTION do_local_conn_command
* 
* Handle local connection mode RPC operations from yangcli.yang
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == template for the local RPC
*   line == input command line from user
*   len == line length
*
* OUTPUTS:
*    agent_cb->state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
* RETURNS:
*    NO_ERR if a RPC was executed
*    ERR_NCX_SKIPPED if no command was invoked
*********************************************************************/
static status_t
    do_local_conn_command (agent_cb_t *agent_cb,
			   const obj_template_t *rpc,
			   xmlChar *line,
			   uint32  len)
{
    const xmlChar *rpcname;

    rpcname = obj_get_name(rpc);

    if (!xml_strcmp(rpcname, YANGCLI_CREATE)) {
	do_edit(agent_cb, rpc, line, len, OP_EDITOP_CREATE);
    } else if (!xml_strcmp(rpcname, YANGCLI_DELETE)) {
	do_edit(agent_cb, rpc, line, len, OP_EDITOP_DELETE);
    } else if (!xml_strcmp(rpcname, YANGCLI_INSERT)) {
	do_insert(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_MERGE)) {
	do_edit(agent_cb, rpc, line, len, OP_EDITOP_MERGE);
    } else if (!xml_strcmp(rpcname, YANGCLI_REPLACE)) {
	do_edit(agent_cb, rpc, line, len, OP_EDITOP_REPLACE);
    } else if (!xml_strcmp(rpcname, YANGCLI_SAVE)) {
	if (len < xml_strlen(line)) {
	    log_error("\nWarning: Extra characters ignored (%s)",
		      &line[len]);
	}
	do_save(agent_cb);
    } else if (!xml_strcmp(rpcname, YANGCLI_SGET)) {
	do_sget(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_SGET_CONFIG)) {
	do_sget_config(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_XGET)) {
	do_xget(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_XGET_CONFIG)) {
	do_xget_config(agent_cb, rpc, line, len);
    } else {
	return ERR_NCX_SKIPPED;
    }
    return NO_ERR;

} /* do_local_conn_command */


/********************************************************************
* FUNCTION do_local_command
* 
* Handle local RPC operations from yangcli.yang
*
* INPUTS:
*   agent_cb == agent control block to use
*   rpc == template for the local RPC
*   line == input command line from user
*   len == length of line in bytes
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
static void
    do_local_command (agent_cb_t *agent_cb,
		      const obj_template_t *rpc,
		      xmlChar *line,
		      uint32  len)
{
    const xmlChar *rpcname;

    rpcname = obj_get_name(rpc);

    if (!xml_strcmp(rpcname, YANGCLI_CD)) {
	do_cd(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_CONNECT)) {
	do_connect(agent_cb, rpc, line, len, FALSE);
    } else if (!xml_strcmp(rpcname, YANGCLI_FILL)) {
	do_fill(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_HELP)) {
	do_help(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_LIST)) {
	do_list(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_MGRLOAD)) {
	do_mgrload(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_PWD)) {
	do_pwd(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_QUIT)) {
	agent_cb->state = MGR_IO_ST_SHUT;
	mgr_request_shutdown();
    } else if (!xml_strcmp(rpcname, YANGCLI_RUN)) {
	(void)do_run(agent_cb, rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_SHOW)) {
	do_show(agent_cb, rpc, line, len);
    } else {
	log_error("\nError: The %s command is not allowed in this mode",
		   rpcname);
    }
} /* do_local_command */



/********************************************************************
* FUNCTION top_command
* 
* Top-level command handler
*
* INPUTS:
*   agent_cb == agent control block to use
*   line == input command line from user
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
void
    top_command (agent_cb_t *agent_cb,
		 xmlChar *line)
{
    const obj_template_t  *rpc;
    uint32                 len;
    ncx_node_t             dtyp;
    status_t               res;

#ifdef DEBUG
    if (!agent_cb || !line) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (!xml_strlen(line)) {
	return;
    }

    dtyp = NCX_NT_OBJ;
    rpc = (const obj_template_t *)parse_def(agent_cb,
					    &dtyp, line, &len);
    if (!rpc) {
	if (agent_cb->result_name || agent_cb->result_filename) {
	    res = finish_result_assign(agent_cb, NULL, line);
	} else {
	    /* this is an unknown command */
	    log_error("\nError: Unrecognized command");
	}
	return;
    }

    /* check  handful of yangcli commands */
    if (is_yangcli_ns(obj_get_nsid(rpc))) {
	do_local_command(agent_cb, rpc, line, len);
    } else {
	log_error("\nError: Not connected to agent."
		  "\nLocal commands only in this mode.");
    }

} /* top_command */


/********************************************************************
* FUNCTION conn_command
* 
* Connection level command handler
*
* INPUTS:
*   agent_cb == agent control block to use
*   line == input command line from user
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
void
    conn_command (agent_cb_t *agent_cb,
		  xmlChar *line)
{
    const obj_template_t  *rpc, *input;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *valset, *parm;
    ses_cb_t              *scb;
    uint32                 len, linelen;
    status_t               res;
    boolean                shut, load;
    ncx_node_t             dtyp;

#ifdef DEBUG
    if (!agent_cb || !line) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    req = NULL;
    reqdata = NULL;
    valset = NULL;
    res = NO_ERR;
    shut = FALSE;
    load = FALSE;

    /* make sure there is something to parse */
    linelen = xml_strlen(line);
    if (!linelen) {
	return;
    }

    /* get the RPC method template */
    dtyp = NCX_NT_OBJ;
    rpc = (const obj_template_t *)parse_def(agent_cb,
					    &dtyp, line, &len);
    if (!rpc) {
	if (agent_cb->result_name || agent_cb->result_filename) {
	    res = finish_result_assign(agent_cb, NULL, line);
	} else {
	    /* this is an unknown command */
	    log_stdout("\nUnrecognized command");
	}
	return;
    }

    /* check local commands */
    if (is_yangcli_ns(obj_get_nsid(rpc))) {
	if (!xml_strcmp(obj_get_name(rpc), YANGCLI_CONNECT)) {
	    log_stdout("\nError: Already connected");
	} else {
	    res = do_local_conn_command(agent_cb, rpc, line, len);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
		do_local_command(agent_cb, rpc, line, len);
	    }
	}
	return;
    }

    /* else treat this as an RPC request going to the agent
     * first construct a method + parameter tree 
     */
    reqdata = xml_val_new_struct(obj_get_name(rpc), 
				 obj_get_nsid(rpc));
    if (!reqdata) {
	log_error("\nError allocating a new RPC request");
	res = ERR_INTERNAL_MEM;
    }

    /* should find an input node */
    input = obj_find_child(rpc, NULL, YANG_K_INPUT);

    /* check if any params are expected */
    if (res == NO_ERR && input && obj_get_child_count(input)) {
	while (line[len] && xml_isspace(line[len])) {
	    len++;
	}

	if (len < linelen) {
	    valset = parse_rpc_cli(rpc, &line[len], &res);
	    if (res != NO_ERR) {
		log_error("\nError in the parameters for RPC %s (%s)",
			  obj_get_name(rpc), get_error_string(res));
	    }
	}

	/* check no input from user, so start a parmset */
	if (res == NO_ERR && !valset) {
	    valset = val_new_value();
	    if (!valset) {
		res = ERR_INTERNAL_MEM;
	    } else {
		val_init_from_template(valset, input);
	    }
	}

	/* fill in any missing parameters from the CLI */
	if (res == NO_ERR) {
	    if (interactive_mode()) {
		res = fill_valset(agent_cb, rpc, valset, NULL);
		if (res == ERR_NCX_SKIPPED) {
		    res = NO_ERR;
		}
	    }
	}

	/* go through the parm list and move the values 
	 * to the reqdata struct. 
	 */
	if (res == NO_ERR) {
	    parm = val_get_first_child(valset);
	    while (parm) {
		val_remove_child(parm);
		val_add_child(parm, reqdata);
		parm = val_get_first_child(valset);
	    }
	}
    }

    /* check the close-session corner cases */
    if (res == NO_ERR && !xml_strcmp(obj_get_name(rpc), 
				     NCX_EL_CLOSE_SESSION)) {
	shut = TRUE;
    }
	    
    /* allocate an RPC request and send it */
    if (res == NO_ERR) {
	scb = mgr_ses_get_scb(agent_cb->mysid);
	if (!scb) {
	    res = SET_ERROR(ERR_INTERNAL_PTR);
	} else {
	    req = mgr_rpc_new_request(scb);
	    if (!req) {
		res = ERR_INTERNAL_MEM;
		log_error("\nError allocating a new RPC request");
	    } else {
		req->data = reqdata;
		req->rpc = rpc;
		req->timeout = agent_cb->timeout;
	    }
	}
	
	if (res == NO_ERR) {
	    if (LOGDEBUG2) {
		log_debug2("\nabout to send RPC request with reqdata:");
		val_dump_value(reqdata, NCX_DEF_INDENT);
	    }

	    /* the request will be stored if this returns NO_ERR */
	    res = mgr_rpc_send_request(scb, req, yangcli_reply_handler);
	}
    }

    if (valset) {
	val_free_value(valset);
    }

    if (res != NO_ERR) {
	if (req) {
	    mgr_rpc_free_request(req);
	} else if (reqdata) {
	    val_free_value(reqdata);
	}
    } else if (shut) {
	agent_cb->state = MGR_IO_ST_CONN_CLOSEWAIT;
    } else {
	agent_cb->state = MGR_IO_ST_CONN_RPYWAIT;
    }

} /* conn_command */


/********************************************************************
 * FUNCTION do_startup_script
 * 
 * Process run-script CLI parameter
 *
 * INPUTS:
 *   agent_cb == agent control block to use
 *
 * SIDE EFFECTS:
 *   runstack start with the runscript script if no errors
 * RETURNS:
 *   status
 *********************************************************************/
status_t
    do_startup_script (agent_cb_t *agent_cb)
{
    const obj_template_t *rpc;
    const xmlChar        *runscript;
    xmlChar              *line, *p;
    status_t              res;
    uint32                linelen;

#ifdef DEBUG
    if (!agent_cb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    runscript = get_runscript();

    /* make sure there is a runscript string */
    if (!runscript || !*runscript) {
	return ERR_NCX_INVALID_VALUE;
    }

    /* get the 'run' RPC method template */
    rpc = ncx_find_object(get_yangcli_mod(), YANGCLI_RUN);
    if (!rpc) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* create a dummy command line 'script <runscipt-text>' */
    linelen = xml_strlen(runscript) + xml_strlen(NCX_EL_SCRIPT) + 1;
    line = m__getMem(linelen+1);
    if (!line) {
	return ERR_INTERNAL_MEM;
    }
    p = line;
    p += xml_strcpy(p, NCX_EL_SCRIPT);
    *p++ = ' ';
    xml_strcpy(p, runscript);
    
    /* fill in the value set for the input parameters */
    res = do_run(agent_cb, rpc, line, 0);
    return res;

}  /* do_startup_script */


/********************************************************************
* FUNCTION get_cmd_line
* 
*  Read the current runstack context and construct
*  a command string for processing by do_run.
*    - Extended lines will be concatenated in the
*      buffer.  If a buffer overflow occurs due to this
*      concatenation, an error will be returned
* 
* INPUTS:
*   agent_cb == agent control block to use
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
    get_cmd_line (agent_cb_t *agent_cb,
		  status_t *res)
{
    xmlChar        *start, *str, *clibuff;
    boolean         done;
    int             len, total, maxlen;

#ifdef DEBUG
    if (!agent_cb || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* init locals */
    clibuff = agent_cb->clibuff;
    total = 0;
    str = NULL;
    maxlen = YANGCLI_BUFFLEN;
    done = FALSE;
    start = clibuff;

    /* get a command line, handling comment and continuation lines */
    while (!done) {

	/* read the next line from the user */
	str = get_line(agent_cb);
	if (!str) {
	    *res = ERR_NCX_READ_FAILED;
	    done = TRUE;
	    continue;
	}

	/* find end of string */
	len = xml_strlen(str);
	
	/* get rid of EOLN if present */
	if (len && str[len-1]=='\n') {
	    str[--len] = 0;
	}

	/* check line continuation */
	if (len && str[len-1]=='\\') {
	    /* get rid of the final backslash */
	    str[--len] = 0;
	    agent_cb->climore = TRUE;
	} else {
	    /* done getting lines */
	    *res = NO_ERR;
	    done = TRUE;
	}

	/* copy the string to the clibuff */
	if (total + len < maxlen) {
	    xml_strcpy(start, str);
	    start += len;
	    total += len;
	} else {
	    *res = ERR_BUFF_OVFL;
	    done = TRUE;
	}
	    
	str = NULL;
    }

    agent_cb->climore = FALSE;
    if (*res == NO_ERR) {
	return clibuff;
    } else {
	return NULL;
    }

}  /* get_cmd_line */


/********************************************************************
 * FUNCTION do_connect
 * 
 * INPUTS:
 *   agent_cb == agent control block to use
 *   rpc == rpc header for 'connect' command
 *   line == input text from readline call, not modified or freed here
 *   start == byte offset from 'line' where the parse RPC method
 *            left off.  This is eiother empty or contains some 
 *            parameters from the user
 *   cli == TRUE if this is a connect request from the CLI
 *       == FALSE if this is from top-level command input
 *
 * OUTPUTS:
 *   connect_valset parms may be set 
 *   create_session may be called
 *
 *********************************************************************/
void
    do_connect (agent_cb_t *agent_cb,
		const obj_template_t *rpc,
		const xmlChar *line,
		uint32 start,
		boolean  cli)
{
    const obj_template_t  *obj;
    val_value_t           *valset, *connect_valset;
    status_t               res;
    boolean                s1, s2, s3;

#ifdef DEBUG
    if (!agent_cb) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    connect_valset = get_connect_valset();

    /* retrieve the 'connect' RPC template, if not done already */
    if (!rpc) {
	rpc = ncx_find_object(get_yangcli_mod(), 
			      YANGCLI_CONNECT);
	if (!rpc) {
	    log_write("\nError finding the 'connect' RPC method");
	    return;
	}
    }	    

    obj = obj_find_child(rpc, NULL, YANG_K_INPUT);
    if (!obj) {
	SET_ERROR(ERR_INTERNAL_VAL);
	log_write("\nError finding the connect RPC 'input' node");	
	return;
    }

    /* process any parameters entered on the command line */
    valset = NULL;
    if (line) {
	while (line[start] && xml_isspace(line[start])) {
	    start++;
	}
	if (line[start]) {
	    valset = parse_rpc_cli(rpc, &line[start], &res);
	    if (!valset || res != NO_ERR) {
		log_write("\nError in the parameters for RPC %s (%s)",
			  obj_get_name(rpc), get_error_string(res));
		return;
	    }
	}
    }

    /* get an empty parmset and use the old set for defaults 
     * unless this is a cli from the program startup and all
     * of the parameters are entered
     */
    if (!valset) {
	s1 = s2 = s3 = FALSE;
	if (cli) {
	    s1 = val_find_child(connect_valset, 
				YANGCLI_MOD, 
				YANGCLI_AGENT) ? TRUE : FALSE;
	    s2 = val_find_child(connect_valset, 
				YANGCLI_MOD,
				YANGCLI_USER) ? TRUE : FALSE;
	    s3 = val_find_child(connect_valset, 
				YANGCLI_MOD,
				YANGCLI_PASSWORD) ? TRUE : FALSE;
	}
	if (!(s1 && s2 && s3)) {
	    valset = val_new_value();
	    if (!valset) {
		log_write("\nError: malloc failure");
		return;
	    } else {
		val_init_from_template(valset, obj);
	    }
	} /* else use all of connect_valset */
    }

    /* complete the connect valset if needed
     * and transfer it to the agent_cb version
     */
    res = NO_ERR;
    if (valset) {
	/* try to get any missing params in valset */
	if (interactive_mode()) {
	    res = fill_valset(agent_cb, rpc, valset, connect_valset);
	    if (res == ERR_NCX_SKIPPED) {
		res = NO_ERR;
	    }
	}

	/* save the malloced valset */
	if (res == NO_ERR) {
	    if (agent_cb->connect_valset) {
		val_free_value(agent_cb->connect_valset);
	    }
	    agent_cb->connect_valset = valset;
	}
    } else if (!cli) {
	if (interactive_mode()) {
	    if (!agent_cb->connect_valset) {
		agent_cb->connect_valset = val_new_value();
		if (!agent_cb->connect_valset) {
		    log_write("\nError: malloc failure");
		    return;
		} else {
		    val_init_from_template(agent_cb->connect_valset,
					   obj);
		}
	    }
	    (void)fill_valset(agent_cb, rpc,
			      agent_cb->connect_valset, 
			      connect_valset);
	}
    } else {
	if (agent_cb->connect_valset) {
	    val_free_value(agent_cb->connect_valset);
	}
	agent_cb->connect_valset = val_clone(connect_valset);
	if (!agent_cb->connect_valset) {
	    res = ERR_INTERNAL_MEM;
	}
    }

    /* check result so far */
    if (res != NO_ERR) {
	log_write("\nError: Connect failed (%s)", 
		  get_error_string(res));
	agent_cb->state = MGR_IO_ST_IDLE;
	if (valset) {
	    val_free_value(valset);
	}
    } else {
	/* make sure the 3 required parms are set */
	s1 = val_find_child(agent_cb->connect_valset,
			    YANGCLI_MOD, 
			    YANGCLI_AGENT) ? TRUE : FALSE;
	s2 = val_find_child(agent_cb->connect_valset, 
			    YANGCLI_MOD,
			    YANGCLI_USER) ? TRUE : FALSE;
	s3 = val_find_child(agent_cb->connect_valset, 
			    YANGCLI_MOD,
			    YANGCLI_PASSWORD) ? TRUE : FALSE;

	/* check if all params present yet */
	if (s1 && s2 && s3) {
	    create_session(agent_cb);
	} else {
	    log_write("\nError: Connect failed due to missing parameters");
	    agent_cb->state = MGR_IO_ST_IDLE;
	}
    }

}  /* do_connect */


/********************************************************************
* FUNCTION parse_def
* 
* Definitions have two forms:
*   def       (default module used)
*   module:def (explicit module name used)
*   prefix:def (if prefix-to-module found, explicit module name used)
*
* Parse the possibly module-qualified definition (module:def)
* and find the template for the requested definition
*
* INPUTS:
*   agent_cb == agent control block to use
*   dtyp == definition type 
*       (NCX_NT_OBJ or  NCX_NT_TYP)
*   line == input command line from user
*   len  == pointer to output var for number of bytes parsed
*
* OUTPUTS:
*    *dtyp is set if it started as NONE
*    *len == number of bytes parsed
*
* RETURNS:
*   pointer to the found definition template or NULL if not found
*********************************************************************/
void *
    parse_def (agent_cb_t *agent_cb,
	       ncx_node_t *dtyp,
	       xmlChar *line,
	       uint32 *len)
{
    void           *def;
    xmlChar        *start, *p, *q, oldp, oldq;
    const xmlChar  *prefix, *defname, *modname;
    ncx_module_t   *mod;
    obj_template_t *obj;
    modptr_t       *modptr;
    uint32          prelen;
    xmlns_id_t      nsid;
    
    def = NULL;
    q = NULL;
    oldq = 0;
    prelen = 0;
    *len = 0;
    start = line;

    /* skip any leading whitespace */
    while (*start && xml_isspace(*start)) {
	start++;
    }

    p = start;

    /* look for a colon or EOS or whitespace to end method name */
    while (*p && (*p != ':') && !xml_isspace(*p)) {
	p++;
    }

    /* make sure got something */
    if (p==start) {
	return NULL;
    }

    /* search for a module prefix if a separator was found */
    if (*p == ':') {

	/* use an explicit module prefix in YANG */
	prelen = p - start;
	q = p+1;
	while (*q && !xml_isspace(*q)) {
	    q++;
	}
	*len = q - line;

	oldq = *q;
	*q = 0;
	oldp = *p;
	*p = 0;

	prefix = start;
	defname = p+1;
    } else {
	/* no module prefix, use default module, if any */
	*len = p - line;

	oldp = *p;
	*p = 0;

	/* try the default module, which will be NULL
	 * unless set by the default-module CLI param
	 */
	prefix = NULL;
	defname = start;
    }

    /* look in the registry for the definition name 
     * first check if only the user supplied a module name
     */
    if (prefix) {
	modname = NULL;
	nsid = xmlns_find_ns_by_prefix(prefix);
	if (nsid) {
	    modname = xmlns_get_module(nsid);
	}
	if (modname) {
	    def = try_parse_def(agent_cb,
				modname, defname, dtyp);
	} else {
	    log_error("\nError: no module found for prefix '%s'", 
		      prefix);
	}
    } else {
	def = try_parse_def(agent_cb,
			    YANGCLI_MOD, 
			    defname, 
			    dtyp);

	if (!def && get_default_module()) {
	    def = try_parse_def(agent_cb,
				get_default_module(), 
				defname, 
				dtyp);
	}
	if (!def && (!get_default_module() ||
		     xml_strcmp(get_default_module(), 
				NC_MODULE))) {

	    def = try_parse_def(agent_cb,
				NC_MODULE, 
				defname, 
				dtyp);
	}

	/* if not found, try any module */
	if (!def) {
	    /* try any of the agent modules first */
	    if (use_agentcb(agent_cb)) {
		for (modptr = (modptr_t *)
			 dlq_firstEntry(&agent_cb->modptrQ);
		     modptr != NULL && !def;
		     modptr = (modptr_t *)dlq_nextEntry(modptr)) {

		    def = try_parse_def(agent_cb, 
					modptr->mod->name, 
					defname, 
					dtyp);
		}
	    }

	    /* try any of the manager-loaded modules */
	    for (mod = ncx_get_first_module();
		 mod != NULL && !def;
		 mod = ncx_get_next_module(mod)) {

		def = try_parse_def(agent_cb, 
				    mod->name, 
				    defname, 
				    dtyp);
	    }
	}

	/* if not found, try a partial RPC command name */
	if (!def && get_autoload()) {
	    switch (*dtyp) {
	    case NCX_NT_NONE:
	    case NCX_NT_OBJ:
		if (use_agentcb(agent_cb)) {
		    for (modptr = (modptr_t *)
			     dlq_firstEntry(&agent_cb->modptrQ);
			 modptr != NULL && !def;
			 modptr = (modptr_t *)dlq_nextEntry(modptr)) {

			def = ncx_match_any_rpc(modptr->mod->name, 
						defname);
			if (def) {
			    *dtyp = NCX_NT_OBJ;
			}
		    }
		}
		if (!def) {
		    def = ncx_match_any_rpc(NULL, defname);
		    if (def) {
			obj = (obj_template_t *)def;
			if (obj_get_nsid(obj) == xmlns_nc_id()) {
			    /* matched a NETCONF RPC and not connected;
			     * would have matched in the use_agentcb()
			     * code above if this is a partial NC op
			     */
			    def = NULL;
			} else {
			    *dtyp = NCX_NT_OBJ;
			}
		    }
		}
		break;
	    default:
		;
	    }
	}
    }

    /* restore string as needed */
    *p = oldp;
    if (q) {
	*q = oldq;
    }

    return def;
    
} /* parse_def */


/* END yangcli_cmd.c */
