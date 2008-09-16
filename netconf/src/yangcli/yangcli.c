/*  FILE: yangcli.c

   NETCONF YANG-based CLI Tool

   See ./README for details

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
01-jun-08    abb      begun; started from ncxcli.c

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

#ifndef _H_def_reg
#include "def_reg.h"
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

#ifndef _H_mgr_hello
#include "mgr_hello.h"
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

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xml_val
#include "xml_val.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define YANGCLI_DEBUG   1
#endif

#define MAX_PROMPT_LEN 32

#define YANGCLI_MAX_NEST  16

#define YANGCLI_MAX_RUNPARMS 9

#define YANGCLI_LINELEN   4095

#define YANGCLI_BUFFLEN  32000

#define YANGCLI_HISTLEN  64000

#define YANGCLI_MOD  (const xmlChar *)"yangcli"

/* CLI parmset for the ncxcli application */
#define YANGCLI_BOOT YANGCLI_MOD

/* core modules auto-loaded at startup */
#define NCMOD        (const xmlChar *)"netconf"
#define NCXDTMOD     (const xmlChar *)"ncxtypes"
#define XSDMOD       (const xmlChar *)"xsd"


/* YANGCLI boot parameter names 
 * matches parm clauses in ncxcli-boot parmset in ncxcli.ncx
 */
#define YANGCLI_AGENT       (const xmlChar *)"agent"
#define YANGCLI_BATCHMODE   (const xmlChar *)"batch-mode"
#define YANGCLI_CONF        (const xmlChar *)"conf"
#define YANGCLI_DIR         (const xmlChar *)"dir"
#define YANGCLI_DEF_MODULE  (const xmlChar *)"default-module"
#define YANGCLI_KEY         (const xmlChar *)"key"

#define YANGCLI_PASSWORD    (const xmlChar *)"password"

#define YANGCLI_NO_AUTOCOMP (const xmlChar *)"no-autocomp"
#define YANGCLI_NO_AUTOLOAD (const xmlChar *)"no-autoload"
#define YANGCLI_NO_FIXORDER (const xmlChar *)"no-fixorder"
#define YANGCLI_RUN_SCRIPT  (const xmlChar *)"run-script"
#define YANGCLI_USER        (const xmlChar *)"user"

#define YANGCLI_COMMAND     (const xmlChar *)"command"
#define YANGCLI_COMMANDS    (const xmlChar *)"commands"
#define YANGCLI_GLOBAL      (const xmlChar *)"global"
#define YANGCLI_GLOBALS     (const xmlChar *)"globals"
#define YANGCLI_LOCAL       (const xmlChar *)"local"
#define YANGCLI_LOCALS      (const xmlChar *)"locals"
#define YANGCLI_OBJECTS     (const xmlChar *)"objects"
#define YANGCLI_OPTIONAL    (const xmlChar *)"optional"
#define YANGCLI_CURRENT_VALUE (const xmlChar *)"current-value"

#define YANGCLI_BRIEF  (const xmlChar *)"brief"
#define YANGCLI_FULL   (const xmlChar *)"full"

#define DEF_PROMPT     (const xmlChar *)"yangcli> "
#define MORE_PROMPT    (const xmlChar *)"   more> "

/* YANGCLI top level commands */
#define YANGCLI_CD      (const xmlChar *)"cd"
#define YANGCLI_CONNECT (const xmlChar *)"connect"
#define YANGCLI_FILL    (const xmlChar *)"fill"
#define YANGCLI_HELP    (const xmlChar *)"help"
#define YANGCLI_LOAD    (const xmlChar *)"load"
#define YANGCLI_PWD     (const xmlChar *)"pwd"
#define YANGCLI_QUIT    (const xmlChar *)"quit"
#define YANGCLI_RUN     (const xmlChar *)"run"
#define YANGCLI_SAVE    (const xmlChar *)"save"
#define YANGCLI_SET     (const xmlChar *)"set"
#define YANGCLI_SHOW    (const xmlChar *)"show"

#define YESNO_NODEF  0
#define YESNO_CANCEL 0
#define YESNO_YES    1
#define YESNO_NO     2

#define YANGCLI_NS_URI \
    ((const xmlChar *)"http://netconfcentral.com/ncx/ncxcli")


#define YANGCLI_PR_LLIST (const xmlChar *)"Add another leaf-list?"
#define YANGCLI_PR_LIST (const xmlChar *)"Add another list?"

/* forward decl needed by do_save function */
static void
    conn_command (xmlChar *line);

/* forward decl needed by do_run function */
static void
    top_command (xmlChar *line);

/* forward decl needed by conn_command */
static void
    do_local_command (const obj_template_t *rpc,
		      xmlChar *line,
		      uint32  len);


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

/* session control */
static mgr_io_state_t  state;
static ses_id_t        mysid;

/* CLI and connect parameters */
static val_value_t   *mgr_cli_valset;
static val_value_t   *connect_valset;
static boolean         batchmode;

/* really set to default-module [netconf] */
static xmlChar        *default_module;  

static boolean         helpmode;
static boolean         versionmode;

static val_value_t    *modules;

static log_debug_t     log_level;
static const xmlChar  *default_target;
static boolean         get_optional;
static boolean         autoload;
static boolean         fixorder;
static boolean         autocomp;
static xmlChar        *confname;
static boolean         logappend;
static xmlChar        *logfilename;
static xmlChar        *runscript;
static boolean         runscriptdone;

/* assignment statement support */
static xmlChar        *result_name;
static boolean         result_isglobal;

/* CLI input buffer */
static xmlChar         clibuff[YANGCLI_BUFFLEN];
static boolean         climore;
static GetLine        *cli_gl;

/* program version string */
static char progver[] = "0.7.1";


/********************************************************************
* FUNCTION interactive_mode
* 
*  Check if the program is in interactive mode
* 
* RETURNS:
*   TRUE if insteractive mode, FALSE if script or batch mode
*********************************************************************/
static boolean
    interactive_mode (void)
{
    return (batchmode || runstack_level()) ? FALSE : TRUE;

}  /* interactive_mode */


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
static val_value_t *
    findparm (val_value_t *valset,
	      const xmlChar *modname,
	      const xmlChar *parmname)
{
    val_value_t *parm;

    if (!valset) {
	return NULL;
    }

    parm = val_find_child(valset, modname, parmname);
    if (!parm && autocomp) {
	parm = val_match_child(valset, modname, parmname);
    }
    return parm;

}  /* findparm */


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
static xmlChar *
    get_strparm (val_value_t *valset,
		 const xmlChar *modname,
		 const xmlChar *parmname)
{
    val_value_t    *parm;
    xmlChar        *str;
    
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
* FUNCTION add_clone_parm
* 
*  Create a parm 
* 
* INPUTS:
*   obj == object template for the parameter to clone
*   valset == value set to add parm to
*   valstr == string value of the value to add
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    add_clone_parm (const obj_template_t *obj,
		    val_value_t *valset,
		    const xmlChar *valstr)
{
    val_value_t    *parm;
    status_t        res;

    parm = val_new_value();
    if (!parm) {
	log_error("\nyangcli: malloc failed in clone value");
	return ERR_INTERNAL_MEM;
    }
    val_init_from_template(parm, obj); 

    res = val_set_simval(parm,
			 obj_get_ctypdef(obj),
			 obj_get_nsid(obj),
			 obj_get_name(obj),
			 valstr);
    if (res != NO_ERR) {
	log_error("\nyangcli: set value failed %s (%s)",
		  (valstr) ? valstr : (const xmlChar *)"--",
		  get_error_string(res));
	val_free_value(parm);
    } else {
	val_add_child(parm, valset);
    }
    return res;

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
static boolean
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
* FUNCTION is_top
* 
*  Check the state and determine if the top or conn
* mode is active
* 
* RETURNS:
*  TRUE if this is TOP mode
*  FALSE if this is CONN mode (or associated states)
*********************************************************************/
static boolean
    is_top (void)
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
* FUNCTION check_assign_statement
* 
* Check if the command line is an assignment statement
* 
* E.g.,
*
*   $foo = $bar
*   $foo = get-config filter=@filter.xml
*   $foo = "quoted string literal"
*   $foo = [<inline><xml/></inline>]
*   $foo = @datafile.xml
*
* INPUTS:
*   line == command line string to expand
*   len  == address of number chars parsed so far in line
*   getrpc == address of return flag to parse and execute an
*            RPC, which will be assigned to the var found
*            in the 'result' module variable
*
* OUTPUTS:
*   *len == number chars parsed in the assignment statement
*   *getrpc == TRUE if the rest of the line represent an
*              RPC command that needs to be evaluated
*
* SIDE EFFECTS:
*    If this is an 'rpc' assignment statement, 
*    (*dorpc == TRUE && *len > 0 && return NO_ERR),
*    then the global result and result_pending variables will
*    be set upon return.  For the other (direct) assignment
*    statement variants, the statement is completely handled
*    here.
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    check_assign_statement (const xmlChar *line,
			    uint32 *len,
			    boolean *getrpc)
{
    const xmlChar         *str, *name;
    const val_value_t     *curval;
    const obj_template_t  *obj;
    val_value_t           *val;
    uint32                 nlen, tlen;
    boolean                isglobal;
    status_t               res;

    /* save start point in line */
    str = line;
    *len = 0;
    *getrpc = FALSE;

    /* check if a varref is being made */
    res = var_check_ref(str, ISLEFT, &tlen, &isglobal, &name, &nlen);
    if (res != NO_ERR) {
	/* error in the varref */
	return res;
    } else if (tlen == 0) {
	/* returned not a varref */
	*getrpc = TRUE;
	return NO_ERR;
    }

    /* else got a valid varref, get the data type, which
     * will also indicate if the variable exists yet
     */
    curval = var_get_str(name, nlen, isglobal);

    /* keep parsing the assignment string */
    str += tlen;

    /* skip any more whitespace */
    while (*str && xml_isspace(*str)) {
	str++;
    }

    /* check end of string */
    if (!*str) {
	return ERR_NCX_DATA_MISSING;
    }

    /* check for the equals sign assignment char */
    if (*str == NCX_ASSIGN_CH) {
	/* move past assignment char */
	str++;
    } else {
	return ERR_NCX_WRONG_TKTYPE;
    }

    /* skip any more whitespace */
    while (*str && xml_isspace(*str)) {
	str++;
    }

    /* check end of string */
    if (!*str) {
	/* got $foo =  EOLN
         * treat this as a request to unset the variable
	 */
	var_unset(name, nlen, isglobal);
	*len = str - line;
	return NO_ERR;
    }

    /* the variable name and equals sign is parsed
     * and the current char is either '$', '"', '<',
     * or a valid first name
     *
     *      $foo = blah
     *             ^
     */
    obj = (curval) ? curval->obj : NULL;

    /* get the script or CLI input as a new val_value_t struct */
    val = var_check_script_val(obj, str, ISTOP, &res);
    if (val) {
	/* this is a plain assignment statement */
	res = var_set_str(name, nlen, val, isglobal);
	if (res != NO_ERR) {
	    val_free_value(val);
	}
    } else if (res==NO_ERR) {
	/* this is as assignment to the results
	 * of an RPC function call 
	 */
	if (result_name) {
	    log_error("\nyangcli: result already pending for %s",
		      result_name);
	    m__free(result_name);
	    result_name = NULL;
	}

	/* save the variable result name */
	result_name = xml_strndup(name, nlen);
	if (!result_name) {
	    *len = 0;
	    res = ERR_INTERNAL_MEM;
	} else {
	    result_isglobal = isglobal;
	    *len = str - line;
	    *getrpc = TRUE;
	}
    } else {
	/* there was some error in the statement processing */
	*len = 0;
    }

    return res;

}  /* check_assign_statement */


/********************************************************************
* FUNCTION parse_rpc_cli
* 
*  Call the ps_parse_cli for an RPC input parmset
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
    if (!obj) {
	*res = ERR_NCX_SKIPPED;
	return NULL;
    }

    myargv[0] = obj_get_name(rpc);
    myargv[1] = args;
    return cli_parse(2, (const char **)myargv, 
		     obj, VALONLY, SCRIPTMODE, autocomp, res);

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
    get_prompt (xmlChar *buff,
		uint32 bufflen)
{
    xmlChar        *p;
    val_value_t    *parm;
    uint32          len;

    if (!buff || !bufflen) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    if (climore) {
	xml_strncpy(buff, MORE_PROMPT, bufflen);
	return;
    }

    switch (state) {
    case MGR_IO_ST_INIT:
    case MGR_IO_ST_IDLE:
    case MGR_IO_ST_CONNECT:
    case MGR_IO_ST_CONN_START:
    case MGR_IO_ST_SHUT:
	xml_strncpy(buff, DEF_PROMPT, bufflen);	
	break;
    case MGR_IO_ST_CONN_IDLE:
    case MGR_IO_ST_CONN_RPYWAIT:
    case MGR_IO_ST_CONN_CANCELWAIT:
    case MGR_IO_ST_CONN_CLOSEWAIT:
    case MGR_IO_ST_CONN_SHUT:
	p = buff;
	len = xml_strcpy(p, (const xmlChar *)"yangcli ");
	p += len;
	bufflen -= len;

	parm = NULL;
	if (connect_valset) {
	    parm = val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_USER);
	}
	if (parm) {
	    len = xml_strncpy(p, VAL_STR(parm), bufflen);
	    p += len;
	    bufflen -= len;
	    *p++ = NCX_AT_CH;
	    --bufflen;
	}

	parm = NULL;
	if (connect_valset) {
	    parm= val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_AGENT);
	}
	if (parm) {
	    len = xml_strncpy(p, VAL_STR(parm), bufflen-3);
	    p += len;
	    bufflen -= len;
	}

	*p++ = '>';
	*p++ = ' ';
	*p = 0;
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
* RETURNS:
*   static line from tecla read line, else NULL if some error
*********************************************************************/
static xmlChar *
    get_line (void)
{
    xmlChar *line;
    xmlChar prompt[MAX_PROMPT_LEN];

    line = NULL;
    get_prompt(prompt, MAX_PROMPT_LEN-1);

    if (!climore) {
	log_stdout("\n");
    }
    line = (xmlChar *)gl_get_line(cli_gl,
				  (const char *)prompt,
				  NULL, -1);
    if (!line) {
	log_stdout("\nyangcli: Error: gl_get_line failed");
    }

    return line;

} /* get_line */


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
*   res == address of status result
*
* OUTPUTS:
*   *res == function result status
*
* RETURNS:
*   pointer to the command line to process (should treat as CONST !!!)
*   NULL if some error
*********************************************************************/
static xmlChar *
    get_cmd_line (status_t *res)
{
    xmlChar        *start, *str;
    boolean         done;
    int             len, total, maxlen;


    /* init locals */
    total = 0;
    str = NULL;
    maxlen = sizeof(clibuff);
    done = FALSE;
    start = clibuff;

    /* get a command line, handling comment and continuation lines */
    while (!done) {

	/* read the next line from the user */
	str = get_line();
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
	    climore = TRUE;
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

    climore = FALSE;
    if (*res == NO_ERR) {
	return clibuff;
    } else {
	return NULL;
    }

}  /* get_cmd_line */


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
static void *
    parse_def (ncx_node_t *dtyp,
	       xmlChar *line,
	       uint32 *len)
{
    void          *def;
    xmlChar       *start, *p, *q, oldp, oldq;
    const xmlChar *module, *defname;
    uint32         modlen;
    
    def = NULL;
    q = NULL;
    modlen = 0;
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

    /* search for an module prefix if found a separator */
    if (*p == ':') {

	/* use an explicit module name or prefix ion YANG */
	modlen = p - start;
	q = p+1;
	while (*q && !xml_isspace(*q)) {
	    q++;
	}
	*len = q - line;

	oldq = *q;
	*q = 0;
	oldp = *p;
	*p = 0;

	module = start;
	defname = p+1;
    } else {
	/* no module prefix, use default module, if any */
	*len = p - line;

	oldp = *p;
	*p = 0;

	/* try the default module, which will be NULL
	 * unless set by the default-module CLI param
	 */
	module = NULL;
	defname = start;
    }

    /* look in the registry for the definition name 
     * first check if only the user supplied a module name
     */
    if (module) {
	def = def_reg_find_moddef(module, defname, dtyp);

	/* if an explicit module is given, then no others
	 * can be tried, but check partial command
	 */
	if (!def && autoload) {
	    switch (*dtyp) {
	    case NCX_NT_NONE:
	    case NCX_NT_OBJ:
		def = ncx_match_any_rpc(module, defname);
		if (def) {
		    *dtyp = NCX_NT_OBJ;
		}
	    default:
		;
	    }
	}
    } else {
	/* no module given, first try default module */
	if (default_module) {
	    def = def_reg_find_moddef(default_module, defname, dtyp);
	}

	/* if not found, try module 'netconf' if not already done */
	if (!def && (!default_module || 
		     xml_strcmp(default_module, NC_MODULE))) {
	    def = def_reg_find_moddef(NC_MODULE, defname, dtyp);
	}

	/* if not found, try module 'ncx' if not already done */
	if (!def && (!default_module || 
		     xml_strcmp(default_module, YANGCLI_MOD))) {
	    def = def_reg_find_moddef(YANGCLI_MOD, defname, dtyp);
	}

	/* if not found, try any module */
	if (!def) {
	    def = def_reg_find_any_moddef(&module, defname, dtyp);
	}

	/* if not found, try a partial RPC command name */
	if (!def && autoload) {
	    switch (*dtyp) {
	    case NCX_NT_NONE:
	    case NCX_NT_OBJ:
		def = ncx_match_any_rpc(NULL, defname);
		if (def) {
		    *dtyp = NCX_NT_OBJ;
		}
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


/********************************************************************
* FUNCTION get_yesno
* 
* Get the user-selected choice, yes or no
*
* INPUTS:
*   prompt == prompt message
*   defcode == default answer code
*      0 == no def answer
*      1 == yes def answer
*      2 == no def answer
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
    get_yesno (const xmlChar *prompt,
	       uint32 defcode,
	       uint32 *retcode)
{
    xmlChar                 *myline, *str;
    status_t                 res;
    boolean                  done;


    res = NO_ERR;

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
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    done = FALSE;
    while (!done) {

	/* get input from the user STDIN */
	myline = get_cmd_line(&res);
	if (!myline) {
	    return res;
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
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	}
	if (!done) {
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
    get_complex_parm (const obj_template_t *parm,
		      val_value_t *valset)
{
    xmlChar          *line;
    val_value_t      *new_parm;
    status_t          res;

    res = NO_ERR;

    log_stdout("\nEnter complex value %s (%s)", 
	       obj_get_name(parm), 
	       tk_get_btype_sym(obj_get_basetype(parm)));

    /* get a line of input from the user */
    line = get_cmd_line(&res);
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
    get_parm (const obj_template_t *rpc,
	      const obj_template_t *parm,
	      val_value_t *valset,
	      val_value_t *oldvalset)
{
    const xmlChar *def, *parmname;
    val_value_t   *oldparm, *newparm;
    xmlChar       *line, *start;
    status_t       res;
    ncx_btype_t    btyp;

    if (!obj_is_mandatory(parm) && !get_optional) {
	return NO_ERR;
    }

    parmname = obj_get_name(parm);
    btyp = obj_get_basetype(parm);

    switch (btyp) {
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
	return get_complex_parm(parm, valset);
    default:
	;
    }

    res = NO_ERR;
    oldparm = NULL;
    def = NULL;
  
    if (btyp==NCX_BT_EMPTY) {
	log_stdout("\nShould flag %s be set?", parmname);
    } else {
	log_stdout("\nEnter value for %s (%s)", parmname, 
		   tk_get_btype_sym(btyp));
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
		def = default_target;
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
	    val_dump_value(oldparm, -1);
	}
	log_stdout("]\n");
    }

    /* get a line of input from the user */
    line = get_cmd_line(&res);
    if (!line) {
	return res;
    }

    /* skip whitespace */
    start = line;
    while (*start && xml_isspace(*start)) {
	start++;
    }

    /* check if any non-whitespace chars entered */
    if (!*start) {
	/* no input, use default or old value */
	if (def) {
	    /* use default */
	    res = cli_parse_parm(valset, parm, def, SCRIPTMODE);
	} else if (oldparm) {
	    if (btyp==NCX_BT_EMPTY) {
		res = cli_parse_parm(valset, parm, NULL, SCRIPTMODE);
	    } else {
		/* use a copy of the last value */
		newparm = val_clone(oldparm);
		if (!newparm) {
		    res = ERR_INTERNAL_MEM;
		} else {
		    val_add_child(newparm, valset);
		}
	    }
	} else if (btyp != NCX_BT_EMPTY) {
	    res = ERR_NCX_DATA_MISSING;
	}  /* else flag should not be set */
    } else if (btyp==NCX_BT_EMPTY) {
	if (*start=='Y' || *start=='y') {
	    res = cli_parse_parm(valset, parm, NULL, SCRIPTMODE);
	} else if (*start=='N' || *start=='n') {
	    ; /* skip; so not add the flag */
	} else if (!*start && oldparm) {
	    /* default was set, so add this flag */
	    res = cli_parse_parm(valset, parm, NULL, SCRIPTMODE);
	} else {
	    res = ERR_NCX_WRONG_VAL;
	}
    } else {
	res = cli_parse_parm(valset, parm, start, SCRIPTMODE);
    }

    return res;
    
} /* get_parm */


/********************************************************************
* FUNCTION get_choice
* 
* Get the user-selected choice, which is not fully set
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
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
    get_choice (const obj_template_t *rpc,
		const obj_template_t *choic,
		val_value_t *valset,
		val_value_t *oldvalset)
{
    const obj_template_t    *parm, *cas, *usecase;
    val_value_t             *pval;
    xmlChar                 *myline, *str;
    status_t                 res;
    int                      casenum, num;
    boolean                  first, done, usedef;


    res = NO_ERR;

    if (!obj_is_config(choic)) {
	log_stdout("\nError: choice '%s' has no configurable parameters",
		   obj_get_name(choic));
	return ERR_NCX_ACCESS_DENIED;
    }

    /* first check the partial block corner case */
    pval = val_get_choice_first_set(valset, choic);
    if (pval) {
	/* found something set from this choice, finish the case */
	log_stdout("\nEnter more parameters to complete the choice:");

	cas = pval->casobj;
	if (!cas) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	for (parm = obj_first_child(cas); 
	     parm != NULL;
	     parm = obj_next_child(parm)) {

	    if (!obj_is_config(parm)) {
		continue;
	    }

	    pval = val_find_child(valset,
				  obj_get_mod_name(parm),
				  obj_get_name(parm));
	    if (pval) {
		continue;   /* node within case already set */
	    }

	    res = get_parm(rpc, parm, valset, oldvalset);
	    if (res != NO_ERR) {
		log_stdout("\nError: get parm '%s' failed (%s)",
			   obj_get_name(parm),
			   get_error_string(res));
		return res;
	    }
	}
	return NO_ERR;
    }

    /* check corner-case -- choice with no cases defined */
    cas = obj_first_child(choic);
    if (!cas) {
	log_stdout("\nNo case nodes defined for choice %s\n",
		   obj_get_name(choic));
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

	    if (!obj_is_config(parm)) {
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
	/* Pick a prompt, depending on the choice default case */
	if (obj_get_default(choic)) {
	    log_stdout("\nEnter choice number (%d - %d), "
		       "[ENTER] for default (%s),"
		       " or 0 to cancel", 1, num-1, 
		       obj_get_default(choic));
	} else {
	    log_stdout("\nEnter choice number (%d - %d),"
		       "or 0 to cancel", 1, num-1);
	}

	/* get input from the user STDIN */
	myline = get_cmd_line(&res);
	if (!myline) {
	    return res;
	}

	/* strip leading whitespace */
	str = myline;
	while (*str && xml_isspace(*str)) {
	    str++;
	}

	/* convert to a number, check [ENTER] for default */
	if (*str) {
	    casenum = atoi((const char *)str);
	    usedef = FALSE;
	} else {
	    usedef = TRUE;
	}

	/* check if default requested */
	if (usedef) {
	    if (obj_get_default(choic)) {
		done = TRUE;
	    } else {
		log_stdout("\nError: Choice does not have a default case\n");
		usedef = FALSE;
	    }
	} else if (casenum == 0) {
	    log_stdout("\nChoice canceled\n");
	    return ERR_NCX_SKIPPED;
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

	    if (!obj_is_config(cas)) {
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
    if (!cas || !obj_is_config(cas)) {
	log_stdout("\nError: No case to fill for this choice");
	return ERR_NCX_SKIPPED;
    }

    /* finish the seclected case */
    for (parm = obj_first_child(cas);
	 parm != NULL;
	 parm = obj_next_child(parm)) {

	if (!obj_is_config(parm)) {
	    continue;
	}

	pval = val_find_child(valset, 
			      obj_get_mod_name(parm),
			      obj_get_name(parm));
	if (pval) {
	    continue;
	}

	/* node is config and not already set */
	if (obj_get_basetype(parm) == NCX_BT_EMPTY) {
	    res = cli_parse_parm(valset, parm, NULL, SCRIPTMODE);
	} else {
	    res = get_parm(rpc, parm, valset, oldvalset);
	}

	if (res != NO_ERR) {
	    log_stdout("\nError: get parm '%s' failed (%s)",
		       obj_get_name(parm),
		       get_error_string(res));
	    return res;
	}
    }

    return NO_ERR;

} /* get_choice */


/********************************************************************
* FUNCTION fill_valset
* 
* Fill the specified value set with any missing parameters.
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
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
    fill_valset (const obj_template_t *rpc,
		 val_value_t *valset,
		 val_value_t *oldvalset)
{
    const obj_template_t  *parm;
    val_value_t           *val, *oldval;
    status_t               res, retres;
    boolean                done, first;
    uint32                 yesnocode;

    retres = NO_ERR;
    for (parm = obj_first_child(valset->obj);
         parm != NULL;
         parm = obj_next_child(parm)) {

	if (!obj_is_config(parm)) {
	    continue;
	}

        switch (parm->objtype) {
        case OBJ_TYP_CHOICE:
	    if (val_choice_is_set(valset, parm)) {
		continue;
	    }

	    if (get_optional || obj_is_mandatory(parm)) {
		res = get_choice(rpc, parm, valset, oldvalset);
		if (res != NO_ERR) {
		    retres = res;
		}
	    }
            break;
        case OBJ_TYP_LEAF:
	case OBJ_TYP_LEAF_LIST:
	    /* if the parm is not already set and is not read-only
	     * then try to get a value from the user at the CLI
	     */
	    done = FALSE;
	    while (!done) {
		val = val_find_child(valset, 
				     obj_get_mod_name(parm),
				     obj_get_name(parm));

		if (!val || parm->objtype==OBJ_TYP_LEAF_LIST) {
		    res = get_parm(rpc, parm, valset, oldvalset);
		    if (res != NO_ERR) {
			log_stdout("\nWarning: Parameter %s has errors (%s)",
				   obj_get_name(parm), 
				   get_error_string(res));
			retres = res;
		    }
		}
		if (parm->objtype == OBJ_TYP_LEAF) {
		    done = TRUE;
		} else {
		    /* prompt for more leaf-list objects */
		    res = get_yesno(YANGCLI_PR_LLIST,
				    YESNO_NO, &yesnocode);
		    if (res != NO_ERR) {
			return res;
		    }
		    switch (yesnocode) {
		    case YESNO_CANCEL:
			return ERR_NCX_SKIPPED;
		    case YESNO_YES:
			break;
		    case YESNO_NO:
			done = TRUE;
			break;
		    default:
			return SET_ERROR(ERR_INTERNAL_VAL);
		    }
		}
	    }
	    break;
	case OBJ_TYP_CONTAINER:
	case OBJ_TYP_NOTIF:
	case OBJ_TYP_RPCIO:
	case OBJ_TYP_LIST:
	    done = FALSE;
	    first = TRUE;
	    while (!done) {
		/* if the parm is not already set and is not read-only
		 * then try to get a value from the user at the CLI
		 */
		if (get_optional || obj_is_mandatory(parm)) {
		    if (oldvalset) {
			oldval = val_find_child(oldvalset, 
						obj_get_mod_name(parm),
						obj_get_name(parm));
		    } else {
			oldval = NULL;
		    }

		    if (first) {
			val = val_find_child(valset, 
					     obj_get_mod_name(parm),
					     obj_get_name(parm));
		    } else {
			val = NULL;
			first = FALSE;
		    }
		    if (!val) {
			val = val_new_value();
			if (!val) {
			    retres = ERR_INTERNAL_MEM;
			    break;
			} else {
			    val_init_from_template(val, parm);
			    val_add_child(val, valset);
			}
		    }

		    res = fill_valset(rpc, val, oldval);
		}

		if (parm->objtype == OBJ_TYP_LIST) {
		    /* prompt for more leaf-list objects */
		    res = get_yesno(YANGCLI_PR_LIST,
				    YESNO_NO, &yesnocode);
		    if (res != NO_ERR) {
			return res;
		    }
		    switch (yesnocode) {
		    case YESNO_CANCEL:
			return ERR_NCX_SKIPPED;
		    case YESNO_YES:
			break;
		    case YESNO_NO:
			done = TRUE;
			break;
		    default:
			return SET_ERROR(ERR_INTERNAL_VAL);
		    }
		} else {
		    done = TRUE;
		}
	    }
            break;
	case OBJ_TYP_RPC:
        default: 
            retres = SET_ERROR(ERR_INTERNAL_VAL);
        }
    }
    return retres;

} /* fill_valset */


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
* Inputs are derived from the module variables in connect_valset:
*    agent == NETCONF agent address string
*    username == SSH2 user name string
*    password == SSH2 password
*
* OUTPUTS:
*   'mysid' is set to the output session ID, if NO_ERR
*   'state' is changed based on the success of the session setup
*
*********************************************************************/
static void
    create_session (void)
{
    const xmlChar *agent, *username, *password;
    val_value_t   *val;
    status_t       res;

    if (mysid) {
	if (mgr_ses_get_scb(mysid)) {
	    SET_ERROR(ERR_INTERNAL_INIT_SEQ);
	    return;
	} else {
	    /* session was reset */
	    mysid = 0;
	}
    }

    /* retrieving the parameters should not fail */
    val =  val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_USER);
    if (val && val->res == NO_ERR) {
	username = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    val = val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_AGENT);
    if (val && val->res == NO_ERR) {
	agent = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    val = val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_PASSWORD);
    if (val && val->res == NO_ERR) {
	password = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    log_info("\nyangcli: Starting NETCONF session for %s on %s",
	     username, agent);

    state = MGR_IO_ST_CONNECT;

    /* this fnction call will cause us to block while the
     * protocol layer connect messages are processed
     */
    res = mgr_ses_new_session(username, password, agent, &mysid);
    if (res == NO_ERR) {
	state = MGR_IO_ST_CONN_START;
	log_debug("\nyangcli: Start session %d OK", mysid);
    } else {
	log_info("\nyangcli: Start session failed for user %s on "
		 "%s (%s)\n", username, agent, get_error_string(res));
	state = MGR_IO_ST_IDLE;
    }
    
} /* create_session */


/********************************************************************
 * FUNCTION get_valset
 * 
 * INPUTS:
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
    get_valset (const obj_template_t *rpc,
		 const xmlChar *line,
		 status_t  *res)
{
    const obj_template_t  *obj;
    val_value_t           *valset;
    uint32                 len;

    *res = NO_ERR;
    valset = NULL;
    len = 0;

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
    if (!obj) {
	*res = ERR_NCX_SKIPPED;
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
	*res = fill_valset(rpc, valset, NULL);
    }

    return valset;

}  /* get_valset */


/********************************************************************
 * FUNCTION do_connect
 * 
 * INPUTS:
 *   rpc == rpc header for 'connect' command
 *   line == input text from readline call, not modified or freed here
 *   start == byte offset from 'line' where the parse RPC method
 *            left off.  This is eiother empty or contains some 
 *            parameters from the user
 *   cli == TRUE if this is a connect request from the CLI
 *       == FALSE if this is from top-level command input
 *
 * OUTPUTS:
 *   connect_valsetparms may be set 
 *   create_session may be called
 *
 *********************************************************************/
static void
    do_connect (const obj_template_t *rpc,
		const xmlChar *line,
		uint32 start,
		boolean  cli)
{
    const obj_template_t  *obj;
    val_value_t           *valset;
    status_t               res;
    boolean       s1, s2, s3;
    ncx_node_t    dtyp;

    /* retrieve the 'connect' RPC template, if not done already */
    if (!rpc) {
	dtyp = NCX_NT_OBJ;
	rpc = (const obj_template_t *)
	    def_reg_find_moddef(YANGCLI_MOD, YANGCLI_CONNECT, &dtyp);
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
     * unless this is a cll from the program startup and all
     * of the parameters are entered
     */
    if (!valset) {
	s1 = s2 = s3 = FALSE;
	if (cli) {
	    s1 = val_find_child(connect_valset, YANGCLI_MOD, 
				YANGCLI_AGENT) ? TRUE : FALSE;
	    s2 = val_find_child(connect_valset, YANGCLI_MOD,
				YANGCLI_USER) ? TRUE : FALSE;
	    s3 = (val_find_child(connect_valset, 
				 YANGCLI_MOD, YANGCLI_PASSWORD) ||
		  val_find_child(connect_valset, 
				 YANGCLI_MOD, YANGCLI_KEY)) ? TRUE : FALSE;
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

    /* if anything entered, try to get any missing params in ps */
    if (valset) {
	if (interactive_mode()) {
	    (void)fill_valset(rpc, valset, connect_valset);
	}
	if (connect_valset) {
	    val_free_value(connect_valset);
	}
	connect_valset = valset;
    } else if (!cli) {
	if (interactive_mode()) {
	    (void)fill_valset(rpc, connect_valset, NULL);
	}
    }
	
    /* hack: make sure the 3 required parms are set instead of
     * full validation of the parmset
     */
    s1 = val_find_child(connect_valset, YANGCLI_MOD, 
			YANGCLI_AGENT) ? TRUE : FALSE;
    s2 = val_find_child(connect_valset, YANGCLI_MOD,
			YANGCLI_USER) ? TRUE : FALSE;
    s3 = (val_find_child(connect_valset, 
			 YANGCLI_MOD, YANGCLI_PASSWORD) ||
	  val_find_child(connect_valset, 
			 YANGCLI_MOD, YANGCLI_KEY)) ? TRUE : FALSE;

    /* check if all params present yet */
    if (s1 && s2 && s3) {
	create_session();
    } else {
	log_write("\nError: Connect failed due to missing parameters");
	state = MGR_IO_ST_IDLE;
    }

}  /* do_connect */


/********************************************************************
 * FUNCTION do_save
 * 
 * INPUTS:
 *    none (connect_valset needs to be valid)
 *
 * OUTPUTS:
 *   copy-config and/or commit operation will be sent to agent
 *
 *********************************************************************/
static void
    do_save (void)
{
    const ses_cb_t   *scb;
    const mgr_scb_t  *mscb;
    xmlChar          *line;

    /* get the session info */
    scb = mgr_ses_get_scb(mysid);
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
	    conn_command(line);
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
	    line = xml_strdup((const xmlChar *)
			      "copy-config target=startup source=running");
	    if (line) {
		conn_command(line);
		m__free(line);
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
 * FUNCTION do_load (local RPC)
 * 
 * load module=mod-name
 *
 * Get the module parameter and load the specified NCX module
 *
 * INPUTS:
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   specified module is loaded into the definition registry, if NO_ERR
 *
 *********************************************************************/
static void
    do_load (const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t  *valset, *val;
    status_t      res;

    val = NULL;
    res = NO_ERR;

    valset = get_valset(rpc, &line[len], &res);

    /* get the module name */
    if (res == NO_ERR) {
	if (valset) {
	    val = val_find_child(valset, YANGCLI_MOD, NCX_EL_MODULE);
	}
	if (!val) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else if (val->res != NO_ERR) {
	    res = val->res;
	}
    }

    /* load the module */
    if (res == NO_ERR) {
	res = ncxmod_load_module(VAL_STR(val));
    }

    /* print the result to stdout */
    if (res == NO_ERR) {
	log_stdout("\nLoad module %s OK", VAL_STR(val));
    } else {
	log_stdout("\nError: Load module failed (%s)",
	       get_error_string(res));
    }

    if (valset) {
	val_free_value(valset);
    }

}  /* do_load */


/********************************************************************
 * FUNCTION do_show_env_var (sub-mode of local RPC)
 * 
 * show enviromnment var
 *
 * INPUTS:
 *  brief == TRUE if brief report desired
 *           FALSE if full report desired
 *********************************************************************/
static void
    do_show_env_var (const char *varname)
{
    const char *envvar;

    envvar = getenv(varname);
    if (interactive_mode()) {
	if (envvar) {
	    log_stdout("\n  %s = %s", varname, envvar);
	} else {
	    log_stdout("\n  %s not set", varname);
	}
    } else {
	if (envvar) {
	    log_write("\n  %s = %s", varname, envvar);
	} else {
	    log_write("\n  %s not set", varname);
	}
    }
} /* do_show_env_var */


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
 *********************************************************************/
static void
    do_show_vars (help_mode_t mode,
		  boolean shortmode,
		  boolean isglobal)

{
    ncx_var_t  *var;
    dlq_hdr_t  *que;
    boolean     first, imode;

    imode = interactive_mode();

    if (mode > HELP_MODE_BRIEF && !shortmode) {
	/* CLI Parameters */
	if (mgr_cli_valset && val_child_cnt(mgr_cli_valset)) {
	    if (imode) {
		log_stdout("\nCLI Variables\n");
		val_stdout_value(mgr_cli_valset, NCX_DEF_INDENT);
		log_stdout("\n");
	    } else {
		log_write("\nCLI Variables\n");
		val_dump_value(mgr_cli_valset, NCX_DEF_INDENT);
		log_write("\n");
	    }
	} else {
	    if (imode) {
		log_stdout("\nNo CLI variables\n");
	    } else {
		log_write("\nNo CLI variables\n");
	    }
	}

	/* Program Environment Variables */
	if (imode) {
	    log_stdout("\nEnvironment Variables\n");
	} else {
	    log_write("\nEnvironment Variables\n");
	}

	do_show_env_var(NCXMOD_PWD);
	do_show_env_var(USER_HOME);
	do_show_env_var(NCXMOD_HOME);
	do_show_env_var(NCXMOD_MODPATH);
	do_show_env_var(NCXMOD_DATAPATH);
	do_show_env_var(NCXMOD_RUNPATH);
	log_write("\n");
    }

    /* Global Script Variables */
    if (!shortmode || isglobal) {
	que = runstack_get_que(ISGLOBAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {
	    if (first) {
		if (imode) {
		    log_stdout("\nGlobal Variables");
		} else {
		    log_write("\nGlobal Variables");
		}
		first = FALSE;
	    }
	    if (typ_is_simple(var->val->btyp)) {
		if (imode) {
		    val_stdout_value(var->val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(var->val, NCX_DEF_INDENT);
		}
	    } else {
		if (imode) {
		    log_stdout("\n  %s = (%s)", 
			       var->name,
			       tk_get_btype_sym(var->val->btyp));
		} else {
		    log_write("\n  %s = (%s)", 
			      var->name,
			      tk_get_btype_sym(var->val->btyp));
		}
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
	    if (imode) {
		log_stdout("\nNo global variables");
	    } else {
		log_write("\nNo global variables");
	    }
	}
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    }

    /* Local Script Variables */
    if (!shortmode || !isglobal) {
	que = runstack_get_que(ISLOCAL);
	first = TRUE;
	for (var = (ncx_var_t *)dlq_firstEntry(que);
	     var != NULL;
	     var = (ncx_var_t *)dlq_nextEntry(var)) {
	    if (first) {
		if (imode) {
		    log_stdout("\nLocal Variables");
		} else {
		    log_write("\nLocal Variables");
		}
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
		if (imode) {
		    log_stdout("\n  %s = (%s)", 
			       var->name,
			       tk_get_btype_sym(var->val->btyp));
		} else {
		    log_write("\n  %s = (%s)", 
			      var->name,
			      tk_get_btype_sym(var->val->btyp));
		}
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
	    if (imode) {
		log_stdout("\nNo local variables");
	    } else {
		log_write("\nNo local variables");
	    }
	}
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
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
 *   mode == help mode requested
 *********************************************************************/
static void
    do_show_var (const xmlChar *name,
		 boolean isglobal,
		 help_mode_t mode)
{
    const val_value_t *val;
    boolean            imode;

    imode = interactive_mode();

    val = var_get(name, isglobal);
    if (val) {
	if (mode == HELP_MODE_BRIEF) {
	    if (typ_is_simple(val->btyp)) {
		if (imode) {
		    val_stdout_value(val, NCX_DEF_INDENT);
		} else {
		    val_dump_value(val, NCX_DEF_INDENT);
		}
	    } else {
		if (imode) {
		    log_stdout("\n  %s (complex type)");
		} else {
		    log_write("\n  %s (complex type)");
		}
	    }
	} else {
	    if (imode) {
		val_stdout_value(val, NCX_DEF_INDENT);
	    } else {
		val_dump_value(val, NCX_DEF_INDENT);
	    }
	}
    } else {
	if (imode) {
	    log_stdout("\nVariable %s not found", name);
	} else {
	    log_write("\nVariable %s not found", name);
	}
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
 * FUNCTION do_show_modules (sub-mode of local RPC)
 * 
 * show modules
 *
 * INPUTS:
 *    mod == first module to show
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_show_modules (const ncx_module_t *mod,
		     help_mode_t mode)
{
    boolean anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    while (mod) {
	if (mode == HELP_MODE_BRIEF) {
	    if (imode) {
		log_stdout("\n  %s/%s", mod->name, mod->version);
	    } else {
		log_write("\n  %s/%s", mod->name, mod->version);
	    }
	} else {
	    help_data_module(mod, HELP_MODE_BRIEF);
	}
	anyout = TRUE;
	mod = (const ncx_module_t *)ncx_get_next_module(mod);
    }

    if (anyout) {
	if (imode) {
	    log_stdout("\n");
	} else {
	    log_write("\n");
	}
    }

} /* do_show_modules */


/********************************************************************
 * FUNCTION do_show_objects (sub-mode of local RPC)
 * 
 * show objects
 *
 * INPUTS:
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_show_objects (help_mode_t mode)
{
    const ncx_module_t   *mod;
    const obj_template_t *obj;
    boolean               anyout, imode;

    mod = ncx_get_first_module();

    imode = interactive_mode();
    anyout = FALSE;

    while (mod) {
	for (obj = ncx_get_first_object(mod);
	     obj != NULL;
	     obj = ncx_get_next_object(mod, obj)) {

	    if (obj_is_data_db(obj) && 
		obj_has_name(obj) &&
		!obj_is_hidden(obj) && !obj_is_abstract(obj)) {

		obj_dump_template(obj, mode, 0, 0); 
		anyout = TRUE;
	    }
	}
	mod = (const ncx_module_t *)ncx_get_next_module(mod);
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
 *    rpc == RPC method for the show command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 *********************************************************************/
static void
    do_show (const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t        *valset, *parm;
    const ncx_module_t *mod;
    status_t            res;
    boolean             imode;
    help_mode_t         mode;

    imode = interactive_mode();
    valset = get_valset(rpc, &line[len], &res);

    if (valset && res == NO_ERR) {
	mode = HELP_MODE_NORMAL;

	/* check if the 'brief' flag is set first */
	parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_BRIEF);
	if (parm && parm->res == NO_ERR) {
	    mode = HELP_MODE_BRIEF;
	} else {
	    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_FULL);
	    if (parm && parm->res == NO_ERR) {
		mode = HELP_MODE_FULL;
	    }
	}
	    
	/* there is 1 parm which is a choice of N */
	parm = val_get_first_child(valset);
	if (parm) {
	    if (!xml_strcmp(parm->name, YANGCLI_LOCAL)) {
		do_show_var(VAL_STR(parm), ISLOCAL, mode);
	    } else if (!xml_strcmp(parm->name, YANGCLI_LOCALS)) {
		do_show_vars(mode, TRUE, FALSE);
	    } else if (!xml_strcmp(parm->name, YANGCLI_OBJECTS)) {
		do_show_objects(mode);
	    } else if (!xml_strcmp(parm->name, YANGCLI_GLOBAL)) {
		do_show_var(VAL_STR(parm), ISGLOBAL, mode);
	    } else if (!xml_strcmp(parm->name, YANGCLI_GLOBALS)) {
		do_show_vars(mode, TRUE, TRUE);
	    } else if (!xml_strcmp(parm->name, NCX_EL_VARS)) {
		do_show_vars(mode, FALSE, FALSE);
	    } else if (!xml_strcmp(parm->name, NCX_EL_MODULE)) {
		mod = ncx_find_module(VAL_STR(parm));
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
	    } else if (!xml_strcmp(parm->name, NCX_EL_MODULELIST)) {
		mod = ncx_get_first_module();
		if (mod) {
		    do_show_modules(mod, TRUE);
		} else {
		    if (imode) {
			log_stdout("\nyangcli: no modules loaded");
		    } else {
			log_error("\nyangcli: no modules loaded");
		    }
		}
	    } else if (!xml_strcmp(parm->name, NCX_EL_MODULES)) {
		mod = ncx_get_first_module();
		if (mod) {
		    do_show_modules(mod, mode);
		} else {
		    if (imode) {
			log_stdout("\nyangcli: no modules loaded");
		    } else {
			log_error("\nyangcli: no modules loaded");
		    }
		}
	    } else if (!xml_strcmp(parm->name, NCX_EL_VERSION)) {
		if (imode) {
		    log_stdout("\nyangcli version %s\n", progver);
		} else {
		    log_write("\nyangcli version %s\n", progver);
		}
	    } else {
		/* else some internal error */
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	} else {
	    log_write("\nError: at least one parameter expected");
	}
    }

    if (valset) {
	val_free_value(valset);
    }

}  /* do_show */


/********************************************************************
 * FUNCTION do_help_commands (sub-mode of local RPC)
 * 
 * help commands
 *
 * INPUTS:
 *    mod == first module to show help
 *    mode == requested help mode
 *
 *********************************************************************/
static void
    do_help_commands (help_mode_t mode)
{
    const ncx_module_t    *mod;
    const obj_template_t  *obj;
    boolean anyout, imode;


    mod = ncx_find_module(YANGCLI_MOD);
    if (!mod) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    imode = interactive_mode();
    anyout = FALSE;

    obj = ncx_get_first_object(mod);
    while (obj) {
	if (obj_is_rpc(obj)) {
	    obj_dump_template(obj, mode, 0, 0);
	    anyout = TRUE;
	}
	obj = ncx_get_next_object(mod, obj);
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
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    do_help (const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    const typ_template_t *typ;
    const obj_template_t *obj;
    val_value_t          *valset, *parm;
    status_t              res;
    help_mode_t           mode;
    boolean               imode;
    ncx_node_t            dtyp;
    uint32                dlen;

    imode = interactive_mode();
    valset = get_valset(rpc, &line[len], &res);
    if (!valset || valset->res != NO_ERR) {
	return;
    }

    mode = HELP_MODE_NORMAL;

    /* look for the 'brief' parameter */
    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_BRIEF);
    if (parm && parm->res == NO_ERR) {
	mode = HELP_MODE_BRIEF;
    } else {
	/* look for the 'full' parameter */
	parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_FULL);
	if (parm && parm->res == NO_ERR) {
	    mode = HELP_MODE_FULL;
	}
    }

    parm = val_find_child(valset, YANGCLI_MOD, 
			  YANGCLI_COMMAND);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(&dtyp, VAL_STR(parm), &dlen);
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
	return;
    }

    /* look for the specific definition parameters */
    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_COMMANDS);
    if (parm && parm->res==NO_ERR) {
	do_help_commands(mode);
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_TYPE);
    if (parm && parm->res==NO_ERR) {
	dtyp = NCX_NT_TYP;
	typ = parse_def(&dtyp, VAL_STR(parm), &dlen);
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
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_OBJECT);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(&dtyp, VAL_STR(parm), &dlen);
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
	return;
    }


    parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_NOTIF);
    if (parm && parm->res == NO_ERR) {
	dtyp = NCX_NT_OBJ;
	obj = parse_def(&dtyp, VAL_STR(parm), &dlen);
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
	return;
    }


    /* no parameters entered except maybe brief or full */
    if (mode == HELP_MODE_FULL) {
	help_program_module(YANGCLI_MOD, YANGCLI_BOOT, 
			    HELP_MODE_NORMAL);
    } else {
	dtyp = NCX_NT_OBJ;
	obj = (const obj_template_t *)
	    def_reg_find_moddef(YANGCLI_MOD, YANGCLI_HELP, &dtyp);
	if (obj && obj->objtype == OBJ_TYP_RPC) {
	    help_object(obj, HELP_MODE_FULL);
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

}  /* do_help */


/********************************************************************
 * FUNCTION do_start_script (sub-mode of run local RPC)
 * 
 * run script=script-filespec
 *
 * run the specified script
 *
 * INPUTS:
 *   source == file source
 *   valset == value set for the run script parameters
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    do_start_script (const xmlChar *source,
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
	parm = (valset) ? val_find_child(valset, YANGCLI_MOD, buff) : NULL;
	if (parm) {
	    /* store P7 named as ASCII 7 */
	    res = var_set_str(buff+1, 1, parm, ISLOCAL);
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
	if (is_top()) {
	    top_command(str);
	} else {
	    conn_command(str);
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
 *    rpc == RPC method for the show command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * RETURNS:
 *    status
 *********************************************************************/
static status_t
    do_run (const obj_template_t *rpc,
	    const xmlChar *line,
	    uint32  len)
{
    val_value_t  *valset, *parm;
    status_t      res;

    res = NO_ERR;

    /* get the 'script' parameter */
    valset = get_valset(rpc, &line[len], &res);

    if (valset && res == NO_ERR) {
	/* there is 1 parm */
	parm = val_find_child(valset, YANGCLI_MOD, NCX_EL_SCRIPT);
	if (!parm) {
	    res = ERR_NCX_DEF_NOT_FOUND;
	} else if (parm->res != NO_ERR) {
	    res = parm->res;
	} else {
	    /* the parm val is the script filespec */
	    res = do_start_script(VAL_STR(parm), valset);
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
 * FUNCTION do_startup_script
 * 
 * Process run-script CLI parameter
 *
 * SIDE EFFECTS:
 *   runstack start with the runscript script if no errors
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    do_startup_script (void)
{
    const obj_template_t *rpc;
    xmlChar              *line, *p;
    status_t              res;
    ncx_node_t            dtyp;
    uint32                linelen;

    /* make sure there is a runscript string */
    if (!runscript || !*runscript) {
	return ERR_NCX_INVALID_VALUE;
    }

    /* get the 'run' RPC method template */
    dtyp = NCX_NT_OBJ;
    rpc = (const obj_template_t *)
	def_reg_find_moddef(YANGCLI_MOD, YANGCLI_RUN, &dtyp);
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
    res = do_run(rpc, line, 0);
    return res;

}  /* do_startup_script */


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
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    len == offset into line buffer to start parsing
 *
 * OUTPUTS:
 *   log_stdout global help message
 *
 *********************************************************************/
static void
    do_pwd (const obj_template_t *rpc,
	    const xmlChar *line,
	    uint32  len)
{
    val_value_t      *valset;
    status_t          res;
    boolean           imode;

    imode = interactive_mode();
    valset = get_valset(rpc, &line[len], &res);
    if (res == NO_ERR || res == ERR_NCX_SKIPPED) {
	pwd();
    }

}  /* do_pwd */


/********************************************************************
 * FUNCTION do_cd
 * 
 * Change the current working directory
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
    do_cd (const obj_template_t *rpc,
	   const xmlChar *line,
	   uint32  len)
{
    val_value_t      *valset, *parm;
    status_t          res;
    int               ret;
    boolean           imode;

    imode = interactive_mode();
    valset = get_valset(rpc, &line[len], &res);
    if (!valset || valset->res != NO_ERR) {
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, YANGCLI_DIR);
    if (!parm || parm->res != NO_ERR) {
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
    
}  /* do_cd */


/********************************************************************
 * FUNCTION do_fill
 * 
 * Fill an object for use in a PDU
 *
 * INPUTS:
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
    do_fill (const obj_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    val_value_t           *valset, *parm, *newparm, *curparm;
    obj_template_t        *targobj;
    const xmlChar         *target;
    status_t               res;
    boolean                imode, save_getopt;


    valset = get_valset(rpc, &line[len], &res);
    if (!valset || valset->res != NO_ERR) {
	return;
    }

    parm = val_find_child(valset, YANGCLI_MOD, 
			  NCX_EL_TARGET);
    if (!parm || parm->res != NO_ERR) {
	return;
    } else {
	target = VAL_STR(parm);
    }

    save_getopt = get_optional;
    imode = interactive_mode();
    newparm = NULL;
    curparm = NULL;

    res = xpath_find_schema_target_int(target, &targobj);
    if (res != NO_ERR) {
	log_error("\nError: Object '%s' not found", target);
	return;
    }	

    parm = val_find_child(valset, YANGCLI_MOD, 
			  YANGCLI_CURRENT_VALUE);
    if (parm && parm->res == NO_ERR) {
	curparm = var_get_script_val(targobj, NULL, 
				     VAL_STR(parm),
				     ISPARM, &res);
	if (!curparm || res != NO_ERR) {
	    log_error("\nError: Script value '%s' invalid (%s)", 
		      VAL_STR(parm), get_error_string(res)); 
	    return;
	}
    }

    parm = val_find_child(valset, YANGCLI_MOD, 
			  YANGCLI_OPTIONAL);
    if (parm && parm->res == NO_ERR) {
	get_optional = TRUE;
    }

    newparm = val_new_value();
    if (!newparm) {
	log_error("\nError: malloc failure");
    } else {
	val_init_from_template(newparm, targobj);

	res = fill_valset(rpc, newparm, curparm);
	if (res == NO_ERR) {
	    if (result_name) {
		/* save the filled in value */
		res = var_set_move(result_name, xml_strlen(result_name),
				   result_isglobal, newparm);
		if (res != NO_ERR) {
		    val_free_value(newparm);

		    log_error("\nError: set result for '%s' failed (%s)",
			      result_name, get_error_string(res));
		}
		newparm = NULL;

		/* clear the result flag */
		m__free(result_name);
		result_name = NULL;
	    }
	} else {
	    if (result_name) {
		m__free(result_name);
		result_name = NULL;
	    }
	}
    }

    /* cleanup */
    if (newparm) {
	val_free_value(newparm);
    }
    if (curparm) {
	val_free_value(curparm);
    }
    get_optional = save_getopt;

}  /* do_fill */


/********************************************************************
* FUNCTION do_local_command
* 
* Handle local RPC operations from yangcli.ncx
*
* INPUTS:
*   rpc == template for the local RPC
*   line == input command line from user
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
static void
    do_local_command (const obj_template_t *rpc,
		      xmlChar *line,
		      uint32  len)
{
    const xmlChar *rpcname;

    rpcname = obj_get_name(rpc);

    if (!xml_strcmp(rpcname, YANGCLI_CONNECT)) {
	do_connect(rpc, line, len, FALSE);
    } else if (!xml_strcmp(rpcname, YANGCLI_FILL)) {
	do_fill(rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_HELP)) {
	do_help(rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_LOAD)) {
	do_load(rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_QUIT)) {
	state = MGR_IO_ST_SHUT;
	mgr_request_shutdown();
    } else if (!xml_strcmp(rpcname, YANGCLI_RUN)) {
	(void)do_run(rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_SHOW)) {
	do_show(rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_PWD)) {
	do_pwd(rpc, line, len);
    } else if (!xml_strcmp(rpcname, YANGCLI_CD)) {
	do_cd(rpc, line, len);
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
*   line == input command line from user
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
static void
    top_command (xmlChar *line)
{
    const obj_template_t  *rpc;
    uint32                 len;
    ncx_node_t             dtyp;
    status_t               res;

    if (!xml_strlen(line)) {
	return;
    }

    dtyp = NCX_NT_OBJ;
    rpc = (const obj_template_t *)parse_def(&dtyp, line, &len);
    if (!rpc) {
	if (result_name) {
	    /* this is just a string assignment */
	    res = var_set_from_string(result_name,
				      line, result_isglobal);
	    if (res != NO_ERR) {
		log_error("\nyangcli: Error setting variable %s (%s)",
			  result_name, get_error_string(res));
	    }

	    /* clear the result flag either way */
	    m__free(result_name);
	    result_name = NULL;
	} else {
	    /* this is an unknown command */
	    log_error("\nError: Unrecognized command");
	}
	return;
    }

    /* check  handful of yangcli commands */
    if (is_yangcli_ns(obj_get_nsid(rpc))) {
	do_local_command(rpc, line, len);
    } else {
	log_error("\nError: Not connected to agent."
		  "\nLocal commands only in this mode.");
    }

} /* top_command */


/********************************************************************
 * FUNCTION yangcli_reply_handler
 * 
 * 
 * INPUTS:
 *   scb == session receiving RPC reply
 *   req == original request returned for freeing or reusing
 *   rpy == reply received from the agent (for checking then freeing)
 *
 * RETURNS:
 *   none
 *********************************************************************/
static void
    yangcli_reply_handler (ses_cb_t *scb,
			  mgr_rpc_req_t *req,
			  mgr_rpc_rpy_t *rpy)
{
    val_value_t  *val;
    status_t      res;
    
    if (rpy && rpy->reply) {
	log_debug("\nRPC Reply %s for session %d:\n",
		  rpy->msg_id, scb->sid);
	if (LOGDEBUG) {
	    val_dump_value(rpy->reply, 0);
	}
	log_debug("\n");

	if (result_name) {
	    /* save the data element if it exists */
	    val = val_first_child_name(rpy->reply, NCX_EL_DATA);
	    if (!val) {
		/* no, so save the entire reply */
		val = rpy->reply;
		rpy->reply = NULL;
	    } else {
		dlq_remove(val);
	    }
	    res = var_set_move(result_name, xml_strlen(result_name),
			       result_isglobal, val);
	    if (res != NO_ERR) {
		log_error("\nyangcli reply: set result failed (%s)",
			  get_error_string(res));
	    }

	    /* clear the result flag */
	    m__free(result_name);
	    result_name = NULL;
	}
    }

    if (state == MGR_IO_ST_CONN_CLOSEWAIT) {
	mysid = 0;
	state = MGR_IO_ST_IDLE;
    } else if (state == MGR_IO_ST_CONN_RPYWAIT) {
	state = MGR_IO_ST_CONN_IDLE;
    } /* else leave state at its current value */

    /* free the request and reply */
    mgr_rpc_free_request(req);
    if (rpy) {
	mgr_rpc_free_reply(rpy);
    }

}  /* yangcli_reply_handler */


/********************************************************************
* FUNCTION conn_command
* 
* Connection level command handler
*
* INPUTS:
*   line == input command line from user
*
* OUTPUTS:
*    state may be changed or other action taken
*    the line buffer is NOT consumed or freed by this function
*
*********************************************************************/
static void
    conn_command (xmlChar *line)
{
    const obj_template_t  *rpc, *input;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata, *valset, *parm;
    ses_cb_t              *scb;
    uint32                 len, linelen;
    status_t               res;
    boolean                shut, load;
    ncx_node_t             dtyp;

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
    rpc = (const obj_template_t *)parse_def(&dtyp, line, &len);
    if (!rpc) {
	if (result_name) {
	    /* this is just a string assignment */
	    res = var_set_from_string(result_name,
				      line, result_isglobal);
	    if (res != NO_ERR) {
		log_error("\nyangcli: Error setting variable %s (%s)",
			  result_name, get_error_string(res));
	    }

	    /* clear the result flag either way */
	    m__free(result_name);
	    result_name = NULL;
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
	} else if (!xml_strcmp(obj_get_name(rpc), YANGCLI_SAVE)) {
	    if (len < linelen) {
		log_error("\nWarning: Extra characters ignored (%s)",
		       &line[len]);
	    }
	    do_save();
	} else {
	    do_local_command(rpc, line, len);
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

    input = obj_find_child(rpc, NULL, YANG_K_INPUT);

    /* check if any params are expected */
    if (res == NO_ERR && input) {
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
	if (res == NO_ERR && !valset && input) {
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
		res = fill_valset(rpc, valset, NULL);
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
	scb = mgr_ses_get_scb(mysid);
	if (!scb) {
	    res = SET_ERROR(ERR_INTERNAL_PTR);
	} else {
	    req = mgr_rpc_new_request(scb);
	    if (!req) {
		res = ERR_INTERNAL_MEM;
		log_error("\nError allocating a new RPC request");
	    } else {
		req->data = reqdata;
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
	state = MGR_IO_ST_CONN_CLOSEWAIT;
    } else {
	state = MGR_IO_ST_CONN_RPYWAIT;
    }

} /* conn_command */


/********************************************************************
* FUNCTION process_cli_input
*
* Process the param line parameters against the hardwired
* parmset for the ncxmgr program
*
* INPUTS:
*    argc == argument count
*    argv == array of command line argument strings
*
* OUTPUTS:
*    global vars that are present are filled in, with parms 
*    gathered or defaults
*
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
static status_t
    process_cli_input (int argc,
		       const char *argv[])
{
    const obj_template_t  *obj;
    val_value_t           *parm;
    status_t               res;
    ncx_node_t             dtyp;

    /* check no command line parms */
    if (argc <= 1) {
	mgr_cli_valset = NULL;
	return NO_ERR;
    }

    /* find the parmset definition in the registry */
    dtyp = NCX_NT_OBJ;
    obj = (const obj_template_t *)
	def_reg_find_moddef(YANGCLI_MOD, YANGCLI_BOOT, &dtyp);
    if (!obj) {
	res = ERR_NCX_NOT_FOUND;
    }

    /* parse the command line against the PSD */    
    mgr_cli_valset = cli_parse(argc, argv, obj,
			       FULLTEST, PLAINMODE,
			       autocomp, &res);
    if (!mgr_cli_valset || res != NO_ERR) {
	return res;
    }

    /* next get any params from the conf file */
    confname = get_strparm(mgr_cli_valset, YANGCLI_MOD, YANGCLI_CONF);
    if (confname) {
	res = conf_parse_val_from_filespec(confname, 
					   mgr_cli_valset,
					   TRUE, TRUE);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /****************************************************
     * go through the yangcli params in order,
     * after setting up the logging parameters
     ****************************************************/

    /* get the log-level parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_LOGLEVEL);
    if (parm && parm->res == NO_ERR) {
	log_level = 
	    log_get_debug_level_enum((const char *)VAL_STR(parm));
	if (log_level == LOG_DEBUG_NONE) {
	    return ERR_NCX_INVALID_VALUE;
	} else {
	    log_set_debug_level(log_level);
	}
    }

    /* get the log-append parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_LOGAPPEND);
    if (parm && parm->res == NO_ERR) {
	logappend = TRUE;
    }

    /* get the log parameter if present */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_LOG);
    if (parm && parm->res == NO_ERR) {
	res = NO_ERR;
	logfilename = xml_strdup(VAL_STR(parm));
	if (!logfilename) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nyangcli: Out of Memory error");
	} else {
	    res = log_open((const char *)logfilename, logappend, FALSE);
	    if (res != NO_ERR) {
		log_error("\nyangcli: Could not open logfile %s",
			  logfilename);
	    }
	}
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the agent parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_AGENT);
    if (parm && parm->res == NO_ERR) {
	/* save to the connect_valset parmset */
	res = add_clone_parm(parm->obj, connect_valset, VAL_STR(parm));
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the batch-mode parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_BATCHMODE);
    if (parm && parm->res == NO_ERR) {
	batchmode = TRUE;
    }

    /* get the default module for unqualified module addesses */
    default_module = get_strparm(mgr_cli_valset, 
				 YANGCLI_MOD, 
				 YANGCLI_DEF_MODULE);

    /* get the help flag */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_HELP);
    if (parm && parm->res == NO_ERR) {
	helpmode = TRUE;
    }

    /* get the key parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_KEY);
    if (parm && parm->res == NO_ERR) {
	/* save to the connect_valset parmset */
	res = add_clone_parm(parm->obj, connect_valset, VAL_STR(parm));
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the password parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_PASSWORD);
    if (parm && parm->res == NO_ERR) {
	/* save to the connect_valset parmset */
	res = add_clone_parm(parm->obj, connect_valset, VAL_STR(parm));
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the modules parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_MODULES);
    if (parm && parm->res == NO_ERR) {
	modules = val_clone(parm);
	if (!modules) {
	    return ERR_INTERNAL_MEM;
	}
    }

    /* get the no-autocomp parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_NO_AUTOCOMP);
    if (parm && parm->res == NO_ERR) {
	autocomp = FALSE;
    }

    /* get the no-autoload parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_NO_AUTOLOAD);
    if (parm && parm->res == NO_ERR) {
	autoload = FALSE;
    }

    /* get the no-fixorder parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_NO_FIXORDER);
    if (parm && parm->res == NO_ERR) {
	fixorder = FALSE;
    }

    /* get the run-script parameter */
    runscript = get_strparm(mgr_cli_valset, YANGCLI_MOD, YANGCLI_RUN_SCRIPT);

    /* get the user name */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, YANGCLI_USER);
    if (parm && parm->res == NO_ERR) {
	res = add_clone_parm(parm->obj, connect_valset, VAL_STR(parm));
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the version parameter */
    parm = val_find_child(mgr_cli_valset, YANGCLI_MOD, NCX_EL_VERSION);
    if (parm && parm->res == NO_ERR) {
	versionmode = TRUE;
    }

    return NO_ERR;

} /* process_cli_input */


/********************************************************************
 * FUNCTION load_base_schema 
 * 
 * Load the following NCX modules:
 *   yangcli
 *
 * RETURNS:
 *     status
 *********************************************************************/
static status_t
    load_base_schema (void)
{
    status_t   res;

    log_debug2("\nYangcli: Loading NCX yangcli-cli Parmset");

    /* load in the agent boot parameter definition file */
    res = ncxmod_load_module(YANGCLI_MOD);

    return res;

}  /* load_base_schema */


/********************************************************************
 * FUNCTION load_core_schema 
 * 
 * Load the following NCX modules:
 *   xsd
 *   ncxtypes
 *   netconf
 *   inetAddress
 *
 * RETURNS:
 *     status
 *********************************************************************/
static status_t
    load_core_schema (void)
{
    status_t   res;

    log_debug2("\nNcxmgr: Loading NETCONF Module");

    /* load in the XSD data types */
    res = ncxmod_load_module(XSDMOD);
    if (res != NO_ERR) {
	return res;
    }

    /* load in the NCX data types */
    res = ncxmod_load_module(NCXDTMOD);
    if (res != NO_ERR) {
	return res;
    }

    /* load in the NETCONF data types and RPC methods */
    res = ncxmod_load_module(NCMOD);
    if (res != NO_ERR) {
	return res;
    }

    return NO_ERR;

}  /* load_core_schema */


/********************************************************************
* FUNCTION report_capabilities
* 
* Generate a start session report, listing the capabilities
* of the NETCONF agent
* 
* INPUTS:
*  scb == session control block
*
*********************************************************************/
static void
    report_capabilities (const ses_cb_t *scb)
{
    const mgr_scb_t    *mscb;
    const xmlChar      *agent;
    const val_value_t  *parm;

    mscb = (const mgr_scb_t *)scb->mgrcb;

    parm = val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_AGENT);
    if (parm && parm->res == NO_ERR) {
	agent = VAL_STR(parm);
    } else {
	agent = (const xmlChar *)"--";
    }

    log_stdout("\n\nNETCONF session established for %s on %s",
	   scb->username, mscb->target ? mscb->target : agent);

    log_stdout("\n\nAgent Protocol Capabilities");
    cap_dump_stdcaps(&mscb->caplist);

    log_stdout("\n\nAgent Module Capabilities");
    cap_dump_modcaps(&mscb->caplist, TRUE);

    log_stdout("\n\nAgent Enterprise Capabilities");
    cap_dump_entcaps(&mscb->caplist);
    log_stdout("\n");

    log_stdout("\nDefault target set to: ");
    switch (mscb->targtyp) {
    case NCX_AGT_TARG_NONE:
	default_target = NULL;
	log_stdout("none");
	break;
    case NCX_AGT_TARG_CANDIDATE:
	default_target = NCX_EL_CANDIDATE;
	log_stdout("<candidate>");
	break;
    case NCX_AGT_TARG_RUNNING:
	default_target = NCX_EL_RUNNING;	
	log_stdout("<running>");
	break;
    case NCX_AGT_TARG_CAND_RUNNING:
	log_stdout("<candidate> (<running> also supported)");
	break;
    case NCX_AGT_TARG_LOCAL:
	default_target = NULL;
	log_stdout("none -- local file");	
	break;
    case NCX_AGT_TARG_REMOTE:
	default_target = NULL;
	log_stdout("none -- remote file");	
	break;
    default:
	default_target = NULL;
	SET_ERROR(ERR_INTERNAL_VAL);
	log_stdout("none -- unknown (%d)", mscb->targtyp);
	break;
    }

    log_stdout("\nSave operation mapped to: ");
    switch (mscb->targtyp) {
    case NCX_AGT_TARG_NONE:
	log_stdout("none");
	break;
    case NCX_AGT_TARG_CANDIDATE:
    case NCX_AGT_TARG_CAND_RUNNING:
	log_stdout("commit");
	if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
	    log_stdout(" + copy-config <running> <startup>");
	}
	break;
    case NCX_AGT_TARG_RUNNING:
	if (mscb->starttyp == NCX_AGT_START_DISTINCT) {
	    log_stdout("copy-config <running> <startup>");
	} else {
	    log_stdout("none");
	}	    
	break;
    case NCX_AGT_TARG_LOCAL:
    case NCX_AGT_TARG_REMOTE:
	/* no way to assign these enums from the capabilities alone! */
	if (cap_std_set(&mscb->caplist, CAP_STDID_URL)) {
	    log_stdout("copy-config <running> <url>");
	} else {
	    log_stdout("none");
	}	    
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	log_stdout("none");
	break;
    }
    log_stdout("\n");
    
} /* report_capabilities */


/********************************************************************
* FUNCTION check_module_capabilities
* 
* Check the modules reported by the agent
* If autoload is TRUE then load any missing modules
* otherwise just warn which modules are missing
* Also check for wrong module module and version errors
*
* INPUTS:
*  scb == session control block
*
*********************************************************************/
static void
    check_module_capabilities (const ses_cb_t *scb)
{
    const mgr_scb_t    *mscb;
    const ncx_module_t *mod;
    const cap_rec_t    *cap;
    const xmlChar      *module, *version;
    uint32              modlen;
    status_t            res;
    xmlChar             namebuff[NCX_MAX_NLEN+1];

    mscb = (const mgr_scb_t *)scb->mgrcb;

    log_info("\n\nChecking Agent Modules...");

    cap = cap_first_modcap(&mscb->caplist);

    while (cap) {
	cap_split_modcap(cap,
			 &module,
			 &modlen,
			 &version);

	xml_strncpy(namebuff, module, modlen);
	mod = ncx_find_module(namebuff);
	if (!mod) {
	    if (autoload) {
		res = ncxmod_load_module(namebuff);
		if (res != NO_ERR) {
		    log_error("\nyangcli error: Module %s not loaded (%s)!!",
			      namebuff, get_error_string(res));
		}
	    } else {
		log_info("\nyangcli warning: Module %s not loaded!!",
			 namebuff);
	    }
	}

	if (mod) {
	    if (xml_strcmp(mod->version, version)) {
		log_warn("\nyangcli warning: Module %s "
			 "has different version on agent!! (%s)",
			 namebuff, mod->version);
	    }
	}

	cap = cap_next_modcap(cap);
    }
    
} /* check_module_capabilities */


/********************************************************************
* yangcli_stdin_handler
*
* Temp: Calling readline which will block other IO while the user
*       is editing the line.  This is okay for this CLI application
*       but not multi-session applications
*
* RETURNS:
*   new program state
*********************************************************************/
static mgr_io_state_t
    yangcli_stdin_handler (void)
{
     xmlChar       *line;
    ses_cb_t       *scb;
    boolean         getrpc;
    status_t        res;
    uint32          len;

    if (mgr_shutdown_requested()) {
	state = MGR_IO_ST_SHUT;
    }

    switch (state) {
    case MGR_IO_ST_INIT:
	return state;
    case MGR_IO_ST_IDLE:
    case MGR_IO_ST_CONN_IDLE:
	break;
    case MGR_IO_ST_CONN_START:
	/* waiting until <hello> processing complete */
	scb = mgr_ses_get_scb(mysid);
	if (!scb) {
	    /* session startup failed */
	    state = MGR_IO_ST_IDLE;
	} else if (scb->state == SES_ST_IDLE && dlq_empty(&scb->outQ)) {
	    /* incoming hello OK and outgoing hello is sent */
	    state = MGR_IO_ST_CONN_IDLE;
	    report_capabilities(scb);
	    check_module_capabilities(scb);
	} else {
	    /* still setting up session */
	    return state;
	}
	break;
    case MGR_IO_ST_CONN_RPYWAIT:
	/* check if session was dropped by remote peer */
	scb = mgr_ses_get_scb(mysid);
	if (!scb || scb->state == SES_ST_SHUTDOWN_REQ) {
	    if (scb) {
		(void)mgr_ses_free_session(mysid);
	    }
	    mysid = 0;
	    state = MGR_IO_ST_IDLE;
	} else  {
	    /* keep waiting for reply */
	    return state;
	}
	break;
    case MGR_IO_ST_CONNECT:
    case MGR_IO_ST_SHUT:
    case MGR_IO_ST_CONN_CANCELWAIT:
    case MGR_IO_ST_CONN_SHUT:
    case MGR_IO_ST_CONN_CLOSEWAIT:
	/* do not accept chars in these states */
	return state;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return state;
    }

    /* check a batch-mode corner-case, nothing else to do */
    if (batchmode && !runscript) {
	mgr_request_shutdown();
	return state;
    }

    /* check the run-script parameters */
    if (runscript && !runscriptdone) {
	runscriptdone = TRUE;
	res = do_startup_script();
	if (res != NO_ERR) {
	    mgr_request_shutdown();
	    return state;
	}
    }

    /* get a line of user input */
    if (runstack_level()) {
	/* get one line of script text */
	line = runstack_get_cmd(&res);
	if (!line || res != NO_ERR) {
	    if (batchmode) {
		mgr_request_shutdown();
	    }
	    return state;
	}
    } else {
	/* block until user enters some input */
	line = get_cmd_line(&res);
	if (!line) {
	    return state;
	}
    }

    /* check if this is an assignment statement */
    res = check_assign_statement(line, &len, &getrpc);
    if (res != NO_ERR) {
	log_error("\nyangcli: Variable assignment failed (%s) (%s)",
		  line, get_error_string(res));
    } else if (getrpc) {
	switch (state) {
	case MGR_IO_ST_IDLE:
	    /* waiting for top-level commands */
	    top_command(&line[len]);
	    break;
	case MGR_IO_ST_CONN_IDLE:
	    /* waiting for session commands */
	    conn_command(&line[len]);
	    break;
	case MGR_IO_ST_CONN_RPYWAIT:
	    /* waiting for RPC reply while more input typed */
	    break;
	case MGR_IO_ST_CONN_CANCELWAIT:
	    break;
	default:
	    break;
	}
    }

    return state;

} /* yangcli_stdin_handler */


/********************************************************************
 * FUNCTION yangcli_init
 * 
 * Init the NCX CLI application
 * 
 * INPUTS:
 *   argc == number of strings in argv array
 *   argv == array of command line strings
 * 
 * RETURNS:
 *   status
 *********************************************************************/
static status_t 
    yangcli_init (int argc,
	      const char *argv[])
{
    const obj_template_t *obj;
    ncx_lmem_t           *lmem;
    val_value_t          *parm;
    ncx_node_t            dtyp;
    status_t              res;

#ifdef YANGCLI_DEBUG
    int   i;
#endif

    /* set the default debug output level */
#ifdef DEBUG
    log_level = LOG_DEBUG_DEBUG;
#else
    log_level = LOG_DEBUG_INFO;
#endif

    /* init the module static vars */
    state = MGR_IO_ST_INIT;
    mysid = 0;
    mgr_cli_valset = NULL;
    connect_valset = NULL;
    batchmode = FALSE;
    default_module = NULL;
    helpmode = FALSE;
    versionmode = FALSE;
    modules = NULL;
    default_target = NULL;
    get_optional = FALSE;
    autoload = TRUE;
    fixorder = TRUE;
    autocomp = TRUE;
    logappend = FALSE;
    logfilename = NULL;
    runscript = NULL;
    runscriptdone = FALSE;
    result_name = NULL;
    result_isglobal = FALSE;
    memset(clibuff, 0x0, sizeof(clibuff));
    climore = FALSE;

    /* get a read line context with a history buffer 
    * change later to not get allocated if batch mode active
    */
    cli_gl = new_GetLine(YANGCLI_LINELEN, YANGCLI_HISTLEN);
    if (!cli_gl) {
	return ERR_INTERNAL_MEM;
    }

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     */
    res = ncx_init(NCX_SAVESTR, log_level, "\nStarting yangcli...");
    if (res != NO_ERR) {
	return res;
    }

#ifdef YANGCLI_DEBUG
    if (argc>1 && LOGDEBUG2) {
        log_debug2("\nCommand line parameters:");
        for (i=0; i<argc; i++) {
            log_debug2("\n   arg%d: %s", i, argv[i]);
        }
    }
#endif

    /* at this point, modules that need to read config
     * params can be initialized
     */

    /* Load the yangcli base module */
    res = load_base_schema();
    if (res != NO_ERR) {
	return res;
    }

    /* Initialize the Netconf Manager Library */
    res = mgr_init();
    if (res != NO_ERR) {
	return res;
    }

    /* init the connect parmset */
    dtyp = NCX_NT_OBJ;
    obj = (const obj_template_t *)
	def_reg_find_moddef(YANGCLI_MOD, YANGCLI_CONNECT, &dtyp);
    if (!obj) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    obj = obj_find_child(obj, NULL, YANG_K_INPUT);
    if (!obj) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* treat the connect-to-agent parmset special
     * it is saved for auto-start plus restart parameters
     * Setup an empty parmset to hold the connect parameters
     */
    connect_valset = val_new_value();
    if (!connect_valset) {
	return ERR_INTERNAL_MEM;
    } else {
	val_init_from_template(connect_valset, obj);
    }

    /* CMD-P1) Get any command line parameters */
    res = process_cli_input(argc, argv);
    if (res != NO_ERR) {
	return res;
    }

    /* CMD-P2) Get any environment string parameters */
    /***/

    /* CMD-P3) Get any PS file parameters */
    /***/

    /* check print version */
    if (versionmode && !helpmode) {
	log_stdout("\nyangcli version %s\n", progver);
    }

    /* check print help and exit */
    if (helpmode) {
	log_stdout("\nyangcli version %s", progver);
	help_program_module(YANGCLI_MOD, YANGCLI_BOOT, HELP_MODE_FULL);
    }

    /* check quick exit */
    if (helpmode || versionmode) {
	return NO_ERR;
    }

    /* set the CLI handler */
    mgr_io_set_stdin_handler(yangcli_stdin_handler);

    /* Load the NETCONF, XSD, SMI and other core modules */
    if (autoload) {
	res = load_core_schema();
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* check if any explicitly listed modules should be loaded */
    if (modules) {
	lmem = ncx_first_lmem(&VAL_LIST(modules));
	while (lmem) {

	    log_info("\nyangcli: Loading requested module %s", 
		     NCX_LMEM_STRVAL(lmem));

	    res = ncxmod_load_module((const xmlChar *)NCX_LMEM_STRVAL(lmem));
	    if (res != NO_ERR) {
		log_info("\n load failed (%s)", get_error_string(res));
	    } else {
		log_info("\n load OK");
	    }

	    lmem = (ncx_lmem_t *)dlq_nextEntry(lmem);
	}
    }


    /* check to see if a session should be auto-started 
     * The yangcli_stdin_handler will call the finish_start_session
     * function when the user enters a line of keyboard text
     */
    state = MGR_IO_ST_IDLE;
    if (connect_valset) {
	parm = val_find_child(connect_valset, YANGCLI_MOD, YANGCLI_AGENT);
	if (parm && parm->res == NO_ERR) {
	    do_connect(NULL, NULL, 0, TRUE);
	}
    }

    return NO_ERR;

}  /* yangcli_init */


/********************************************************************
 * FUNCTION yangcli_cleanup
 * 
 * 
 * 
 *********************************************************************/
static void
    yangcli_cleanup (void)
{
    log_debug2("\nShutting down yangcli\n");

    /* cleanup the user edit buffer */
    if (cli_gl) {
	(void)del_GetLine(cli_gl);
	cli_gl = NULL;
    }

    /* Cleanup the Netconf Agent Library */
    mgr_cleanup();

    /* cleanup the NCX engine and registries */
    ncx_cleanup();

    /* clean and reset all module static vars */
    state = MGR_IO_ST_NONE;
    mysid = 0;

    if (mgr_cli_valset) {
	val_free_value(mgr_cli_valset);
	mgr_cli_valset = NULL;
    }

    if (connect_valset) {
	val_free_value(connect_valset);
	connect_valset = NULL;
    }

    if (default_module) {
	m__free(default_module);
	default_module = NULL;
    }

    default_target = NULL;

    if (confname) {
	m__free(confname);
	confname = NULL;
    }

    if (logfilename) {
	m__free(logfilename);
	logfilename = NULL;
    }

    if (runscript) {
	m__free(runscript);
	runscript = NULL;
    }

    if (modules) {
	val_free_value(modules);
	modules = NULL;
    }

    if (result_name) {
	m__free(result_name);
	result_name = NULL;
    }

    log_close();

}  /* yangcli_cleanup */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
*                                                                   *
*			FUNCTION main				    *
*                                                                   *
*********************************************************************/
int 
    main (int argc, 
	  const char *argv[])
{
    status_t   res;

    res = yangcli_init(argc, argv);
    if (res != NO_ERR) {
	log_error("\nYangcli: init returned error (%s)\n", 
		  get_error_string(res));
    } else if (!(helpmode || versionmode)) {
	log_stdout("\nyangcli version %s\n", progver);
	res = mgr_io_run();
	if (res != NO_ERR) {
	    log_error("\nmgr_io failed (%d)\n", res);
	}
    }

    print_errors();
    yangcli_cleanup();

    return 0;

} /* main */


/* END yangcli.c */
