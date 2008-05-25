/*  FILE: ncxcli.c

   NETCONF CLI Tool

   See ./README for details

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
08-feb-07    abb      begun;

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

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_ps_parse
#include "ps_parse.h"
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


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define NCXCLI_DEBUG   1
#endif

#define MAX_PROMPT_LEN 32

#define NCXCLI_MAX_NEST  16

#define NCXCLI_MAX_RUNPARMS 9

#define NCXCLI_LINELEN   4095

#define NCXCLI_BUFFLEN  32000

#define NCXCLI_HISTLEN  64000

/* core modules auto-loaded at startup */
#define NCXCLIMOD    ((const xmlChar *)"ncxcli")
#define NCMOD        ((const xmlChar *)"netconf")
#define NCXDTMOD     ((const xmlChar *)"ncxtypes")
#define XSDMOD       ((const xmlChar *)"xsd")
#define SMIMOD       ((const xmlChar *)"smi")

/* CLI parmset for the ncxcli application */
#define NCXCLI_BOOT ((const xmlChar *)"ncxcli")

/* NCXCLI boot parameter names 
 * matches parm clauses in ncxcli-boot parmset in ncxcli.ncx
 */
#define NCXCLI_AGENT       (const xmlChar *)"agent"
#define NCXCLI_BATCHMODE   (const xmlChar *)"batch-mode"
#define NCXCLI_CONF        (const xmlChar *)"conf"
#define NCXCLI_DEF_MODULE  (const xmlChar *)"default-module"
#define NCXCLI_KEY         (const xmlChar *)"key"
/* NCXCLI_HELP */
#define NCXCLI_PASSWORD    (const xmlChar *)"password"
/* NCX_EL_LOG */
/* NCX_EL_LOGAPPEND */
/* NCX_EL_LOGLEVEL */
/* NCX_EL_MODULES */
#define NCXCLI_NO_AUTOCOMP (const xmlChar *)"no-autocomp"
#define NCXCLI_NO_AUTOLOAD (const xmlChar *)"no-autoload"
#define NCXCLI_NO_FIXORDER (const xmlChar *)"no-fixorder"
#define NCXCLI_RUN_SCRIPT  (const xmlChar *)"run-script"
#define NCXCLI_USER        (const xmlChar *)"user"
/* NCX_EL_VERSION */

#define NCXCLI_BRIEF  (const xmlChar *)"brief"



#define DEF_PROMPT ((const xmlChar *)"ncx> ")
#define MORE_PROMPT ((const xmlChar *)"   more> ")

/* NCXCLI top level commands */
#define NCXCLI_CONNECT ((const xmlChar *)"connect")
#define NCXCLI_HELP    ((const xmlChar *)"help")
#define NCXCLI_LOAD    ((const xmlChar *)"load")
#define NCXCLI_QUIT    ((const xmlChar *)"quit")
#define NCXCLI_RUN     ((const xmlChar *)"run")
#define NCXCLI_SAVE    ((const xmlChar *)"save")
#define NCXCLI_SET     ((const xmlChar *)"set")
#define NCXCLI_SHOW    ((const xmlChar *)"show")

#define NCXCLI_NS_URI \
    ((const xmlChar *)"http://netconfcentral.com/ncx/ncxcli")

/* forward decl needed by do_save function */
static void
    conn_command (xmlChar *line);

/* forward decl needed by do_run function */
static void
    top_command (xmlChar *line);

/* forward decl needed by conn_command */
static void
    do_local_command (const rpc_template_t *rpc,
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
static ps_parmset_t   *mgr_cli_ps;
static ps_parmset_t   *connect_ps;

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
static xmlChar         clibuff[NCXCLI_BUFFLEN];
static boolean         climore;
static GetLine        *cli_gl;

/* program version string */
static char progver[] = "0.6";


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
*   ps == parmset to check (may be NULL)
*   parmname  == name of parm to get of partial name to get
*
* RETURNS:
*   pointer to ps_parm_t if found
*********************************************************************/
static ps_parm_t *
    findparm (ps_parmset_t *ps,
	      const xmlChar *parmname)
{
    ps_parm_t *parm;

    if (!ps) {
	return NULL;
    }

    parm = ps_find_parm(ps, parmname);
    if (!parm && autocomp) {
	parm = ps_match_parm(ps, parmname);
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
*   ps == parmset to check if not NULL
*   parmname  == name of parm to get
*
* RETURNS:
*   pointer to string !!! THIS IS A MALLOCED COPY !!!
*********************************************************************/
static xmlChar *
    get_strparm (ps_parmset_t *ps,
		 const xmlChar *parmname)
{
    ps_parm_t    *parm;
    xmlChar      *str;

    str = NULL;
    parm = findparm(ps, parmname);
    if (parm) {
	str = xml_strdup(VAL_STR(parm->val));
	if (!str) {
	    log_error("\nncxcli: Out of Memory error");
	}
    }
    return str;

}  /* get_strparm */


/********************************************************************
* FUNCTION add_parm
* 
*  Add a parm to a ps_parmset_t based on the
*  value of the 'fixorder' global parameter
* 
* INPUTS:
*   parm == parm to add
*   ps == parmset to add parm to
*
*********************************************************************/
static void
    add_parm (ps_parm_t *parm,
	      ps_parmset_t *ps)
{
    if (fixorder) {
	ps_add_parm(parm, ps, NCX_MERGE_FIRST);
    } else {
	ps_add_parm_last(parm, ps);
    }

}  /* add_parm */


/********************************************************************
* FUNCTION add_clone_parm
* 
*  Create a parm 
* 
* INPUTS:
*   psd_parm == parm def to add
*   ps == parmset to add parm to
*   valstr == string value of the value to add
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    add_clone_parm (psd_parm_t *psd_parm,
		    ps_parmset_t *ps,
		    const xmlChar *valstr)
{
    typ_template_t *typ;
    ps_parm_t *parm;
    status_t   res;

    parm = ps_new_parm();
    if (!parm) {
	log_error("\nncxcli: malloc failed in clone value");
	return ERR_INTERNAL_MEM;
    }
    ps_setup_parm(parm, ps, psd_parm); 

    typ = psd_get_parm_template(psd_parm);
    res = val_set_simval(parm->val,
			 &typ->typdef,
			 psd_get_parm_nsid(psd_parm),
			 psd_parm->name,
			 valstr);
    if (res != NO_ERR) {
	log_error("\nncxcli: set value failed %s (%s)",
		  (valstr) ? valstr : (const xmlChar *)"--",
		  get_error_string(res));
	ps_free_parm(parm);
	return res;
    }

    add_parm(parm, ps);
    return NO_ERR;

}  /* add_clone_parm */


/********************************************************************
* FUNCTION is_ncxcli_ns
* 
*  Check the namespace and make sure this is an NCXCLI command
* 
* INPUTS:
*   ns == namespace ID to check
*
* RETURNS:
*  TRUE if this is the NCXCLI namespace ID
*********************************************************************/
static boolean
    is_ncxcli_ns (xmlns_id_t ns)
{
    const xmlChar *uri;

    uri = xmlns_get_ns_name(ns);
    if (uri && !xml_strcmp(uri, NCXCLI_NS_URI)) {
	return TRUE;
    } else {
	return FALSE;
    }

}  /* is_ncxcli_ns */


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
*   $foo = <inline><xml/></inline>
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
    const xmlChar     *str, *name;
    const val_value_t *curval;
    const typ_def_t   *typdef;
    val_value_t       *val;
    uint32             nlen, tlen;
    boolean            isglobal;
    status_t           res;

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
    if (curval) {
	typdef = curval->typdef;
    } else {
	typdef = NULL;
    }

    /* get the script input as a new val_value_t struct */
    val = var_get_script_val(typdef, NULL, 
			     0, NULL, str, ISTOP, &res);
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
	    log_error("\nncxcli: result already pending for %s",
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
*  pointer to malloced parmset or NULL if none created,
*  may have errors, check *res
*********************************************************************/
static ps_parmset_t *
    parse_rpc_cli (const rpc_template_t *rpc,
		   const xmlChar *args,
		   status_t  *res)
{
    const xmlChar    *myargv[2];

    /* construct an argv array, 
     * convert the CLI into a parmset 
     */
    if (!rpc->in_psd) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }

    myargv[0] = rpc->name;
    myargv[1] = args;
    return ps_parse_cli(2, (const char **)myargv, 
			rpc->in_psd,
			VALONLY, SCRIPTMODE, autocomp, res);

}  /* parse_rpc_cli */


/********************************************************************
* FUNCTION get_prompt
* 
* Construct the CLI prompt for the current state
* 
* INPUTS:
*   buff == bufffer to hold prompt (zero-terminated string)
*   bulllen == length of buffer (max bufflen-1 chars can be printed
*
*********************************************************************/
static void
    get_prompt (xmlChar *buff,
		uint32 bufflen)
{
    xmlChar      *p;
    ps_parm_t    *parm;
    uint32        len;

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
	len = xml_strcpy(p, (const xmlChar *)"ncx ");
	p += len;
	bufflen -= len;

	parm = ps_find_parm(connect_ps, NCXCLI_USER);
	if (parm) {
	    len = xml_strncpy(p, VAL_STR(parm->val), bufflen);
	    p += len;
	    bufflen -= len;
	    *p++ = NCX_AT_CH;
	    --bufflen;
	}

	parm= ps_find_parm(connect_ps, NCXCLI_AGENT);
	if (parm) {
	    len = xml_strncpy(p, VAL_STR(parm->val), bufflen-3);
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
	log_stdout("\nncxcli: Error: Readline failed");
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
*       (NCX_NT_RPC, NCX_NT_PSD, NCX_NT_TYP, etc)
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
	    case NCX_NT_RPC:
		def = ncx_match_any_rpc(module, defname);
		if (def) {
		    *dtyp = NCX_NT_RPC;
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
		     xml_strcmp(default_module, NCXCLIMOD))) {
	    def = def_reg_find_moddef(NCXCLIMOD, defname, dtyp);
	}

	/* if not found, try any module */
	if (!def) {
	    def = def_reg_find_any_moddef(&module, defname, dtyp);
	}

	/* if not found, try a partial RPC command name */
	if (!def && autoload) {
	    switch (*dtyp) {
	    case NCX_NT_NONE:
	    case NCX_NT_RPC:
		def = ncx_match_any_rpc(NULL, defname);
		if (def) {
		    *dtyp = NCX_NT_RPC;
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
* FUNCTION get_complex_parm
* 
* Fill the specified parm, which is a complex type
* This function will block on readline to get input from the user
*
* INPUTS:
*   parm == parm to get from the CLI
*   ps == parmset being filled
*
* OUTPUTS:
*    new ps_parm_t node will be added to ps of NO_ERR
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_complex_parm (psd_parm_t *parm,
		      ps_parmset_t *ps)
{
    xmlChar       *line;
    ps_parm_t     *new_parm;
    status_t       res;

    res = NO_ERR;

    log_stdout("\nEnter complex value %s (%s)", 
	       parm->name, parm->typname);

    /* get a line of input from the user */
    line = get_cmd_line(&res);
    if (!line) {
	return res;
    }

    new_parm = ps_new_parm();
    if (!new_parm) {
	res = ERR_INTERNAL_MEM;
    } else {
	ps_setup_parm(new_parm, ps, parm);
	(void)var_get_script_val(psd_get_typdef(parm),
				 new_parm->val,
				 psd_get_parm_nsid(parm),
				 parm->name, line,
				 ISPARM, &res);
	if (res == NO_ERR) {
	    /* add the parm to the parmset */
	    add_parm(new_parm, ps);
	}
    }

    if (res != NO_ERR) {
	log_stdout("\nncxcli: Error in %s (%s)",
		   parm->name, get_error_string(res));
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
*   ps == parmset being filled
*   oldps == last set of values (NULL if none)
*
* OUTPUTS:
*    new ps_parm_t node will be added to ps of NO_ERR
*
* RETURNS:
*    status
*********************************************************************/
static status_t
    get_parm (const rpc_template_t *rpc,
	      psd_parm_t *parm,
	      ps_parmset_t *ps,
	      ps_parmset_t *oldps)
{
    const xmlChar *def;
    ps_parm_t     *oldparm;
    xmlChar       *line, *start;
    status_t       res;
    ncx_btype_t    btyp;

    if (parm->usage==PSD_UT_OPTIONAL && !get_optional) {
	return NO_ERR;
    }

    btyp = psd_parm_basetype(parm);

    switch (btyp) {
    case NCX_BT_ANY:
    case NCX_BT_ROOT:
	return get_complex_parm(parm, ps);
    default:
	;
    }

    res = NO_ERR;
    oldparm = NULL;
    def = NULL;
  
    if (btyp==NCX_BT_EMPTY) {
	log_stdout("\nShould flag %s be set?", parm->name);
    } else {
	log_stdout("\nEnter value for %s (%s)", parm->name, parm->typname);
    }
    if (oldps) {
	oldparm = ps_find_parm(oldps, parm->name);
    }

    /* pick a default value, either old value or default clause */
    if (!oldparm) {
	/* try to get the defined default value */
	if (btyp != NCX_BT_EMPTY) {
	    def = typ_get_defval(parm->pdef);
	    if (!def && (rpc->nsid == xmlns_nc_id() &&
			 (!xml_strcmp(parm->name, NCX_EL_TARGET) ||
			  !xml_strcmp(parm->name, NCX_EL_SOURCE)))) {
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
	    val_dump_value(oldparm->val, -1);
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
	    res = ps_parse_cli_parm(ps, parm, def, SCRIPTMODE);
	} else if (oldparm) {
	    if (btyp==NCX_BT_EMPTY) {
		res = ps_parse_cli_parm(ps, parm, NULL, SCRIPTMODE);
	    } else {
		/* use a copy of the last value */
		res = ps_parse_add_clone(ps, oldparm);
	    }
	} else if (btyp != NCX_BT_EMPTY) {
	    res = ERR_NCX_DATA_MISSING;
	}  /* else flag should not be set */
    } else if (btyp==NCX_BT_EMPTY) {
	if (*start=='Y' || *start=='y') {
	    res = ps_parse_cli_parm(ps, parm, NULL, SCRIPTMODE);
	} else if (*start=='N' || *start=='n') {
	    ; /* skip; so not add the flag */
	} else if (!*start && oldparm) {
	    /* default was set, so add this flag */
	    res = ps_parse_cli_parm(ps, parm, NULL, SCRIPTMODE);
	} else {
	    res = ERR_NCX_WRONG_VAL;
	}
    } else {
	res = ps_parse_cli_parm(ps, parm, start, SCRIPTMODE);
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
*   pch == PSD Choice template header
*   ps == parmset to fill
*   oldps == last set of values (or NULL if none)
*
* OUTPUTS:
*    new ps_parm_t nodes may be added to ps
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    get_choice (const rpc_template_t *rpc,
		const psd_choice_t *pch,
		ps_parmset_t *ps,
		ps_parmset_t *oldps)
{
    psd_parm_t    *parm;
    psd_hdronly_t *phdr;
    const psd_block_t *pb;
    ps_parm_t     *pval;
    xmlChar       *myline, *str;
    status_t       res;
    int            i, num;
    psd_parmid_t   parmid;


    /* first check the partial block corner case */
    pval = ps_choice_first_set(ps, pch);
    if (pval) {
	/* finish the block */
	log_stdout("\nEnter more parameters to complete the choice:");

	pb = psd_find_blocknum(ps->psd, pval->parm->choice_id,
			       pval->parm->block_id);
	if (!pb) {
	    return SET_ERROR(ERR_INTERNAL_VAL);
	}
	for (parmid = pb->start_parm; 
	     parmid <= pb->end_parm; parmid++) {
	    if (ps_parmnum_set(ps, parmid)) {
		continue;
	    }
	    parm = psd_find_parmnum(ps->psd, parmid);
	    if (!parm) {
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	    if (!psd_parm_writable(parm)) {
		continue;
	    }
	    res = get_parm(rpc, parm, ps, oldps);
	    if (res != NO_ERR) {
		ncx_print_errormsg(NULL, NULL, res);
	    }
	}
	return NO_ERR;
    }

    /* else not a partial block corner case but a normal
     * case whre no choice has been made at all
     */
    log_stdout("\nEnter a number of the selected choice:\n");

    for (num = 1, phdr = (psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr), num++) {
        switch (phdr->ntyp) {
        case PSD_NT_BLOCK:
            pb = (psd_block_t *)phdr;
	    log_stdout("\n  %d: block %d:", num, pb->block_id);
	    for (parmid = pb->start_parm; 
		 parmid <= pb->end_parm; parmid++) {
		parm = psd_find_parmnum(ps->psd, parmid);
		if (!parm) {
		    return SET_ERROR(ERR_INTERNAL_VAL);
		}
		log_stdout("\n    %s (%s)", parm->name, parm->typname);
	    }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
	    log_stdout("\n  %d: parm %s (%s)", 
		   num, parm->name, parm->typname);
            break;
        case PSD_NT_CHOICE:
	    /* should not see a CHOICE node at this level */
	    /* fall through */
        default: 
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    /* Get a line of input which should just be a number */
    log_stdout("\n\nEnter choice number (%d - %d)", 1, num-1);
    myline = get_cmd_line(&res);
    if (!myline) {
	return res;
    }
    str = myline;
    while (*str && xml_isspace(*str)) {
	str++;
    }
    if (*str) {
	i = atoi((const char *)str);
    } else {
	i = 0;
    }

    if (i < 1 || i >= num) {
	res = ERR_NCX_DATA_MISSING;
	ncx_print_errormsg(NULL, NULL, res);
	return res;
    }

    /* got a valid choice number, now get the corr. choice header */
    phdr = (psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
    for (num = 1; num < i; num++) {
	phdr = (psd_hdronly_t *)dlq_nextEntry(phdr);
    }

    /* get the parameter or finish the block specified */
    switch (phdr->ntyp) {
    case PSD_NT_BLOCK:
	pb = (psd_block_t *)phdr;
	for (parmid = pb->start_parm; 
	     parmid <= pb->end_parm; parmid++) {
	    if (ps_parmnum_set(ps, parmid)) {
		continue;
	    }
	    parm = psd_find_parmnum(ps->psd, parmid);
	    if (!parm) {
		return SET_ERROR(ERR_INTERNAL_VAL);
	    }
	    if (!psd_parm_writable(parm)) {
		continue;
	    }
	    res = get_parm(rpc, parm, ps, oldps);
	    if (res != NO_ERR) {
		ncx_print_errormsg(NULL, NULL, res);
	    }
	}
	break;
    case PSD_NT_PARM:
	/* if the parm is not already set and is not read-only
	 * then try to get a value from the user at the CLI
	 */
	parm = (psd_parm_t *)phdr;
	if (!ps_parmnum_set(ps, parm->parm_id)) {
	    if (psd_parm_writable(parm)) {
		if (psd_parm_basetype(parm) == NCX_BT_EMPTY) {
		    /* user selected a flag, so no need to get a value */
		    res = ps_parse_cli_parm(ps, parm, NULL, SCRIPTMODE);
		} else {
		    res = get_parm(rpc, parm, ps, oldps);
		    if (res != NO_ERR) {
			ncx_print_errormsg(NULL, NULL, res);
		    }
		}
	    }
	}
	break;
    case PSD_NT_CHOICE:
	/* should not see a CHOICE node at this level */
	/* fall through */
    default: 
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    return NO_ERR;

} /* get_choice */


/********************************************************************
* FUNCTION fill_parmset
* 
* Fill the specified parmset with any missing parameters.
* Use values from the last set (if any) for defaults.
* This function will block on readline if mandatory parms
* are needed from the CLI
*
* INPUTS:
*   rpc == RPC method that is being called
*   ps == parmset to fill
*   oldps == last set of values (or NULL if none)
*
* OUTPUTS:
*    new ps_parm_t nodes may be added to ps
*
* RETURNS:
*    status,, ps may be partially filled if not NO_ERR
*********************************************************************/
static status_t
    fill_parmset (const rpc_template_t *rpc,
		  ps_parmset_t *ps,
		  ps_parmset_t *oldps)
{
    psd_hdronly_t *phdr;
    const psd_choice_t  *pch;
    psd_parm_t    *parm;
    status_t       res, retres;

    retres = NO_ERR;
    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&ps->psd->parmQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_CHOICE:
            pch = (const psd_choice_t *)phdr;
	    if (!ps_check_choice_set(ps, pch->choice_id)) {
		if (get_optional || psd_choice_required(pch)) {
		    res = get_choice(rpc, pch, ps, oldps);
		    if (res != NO_ERR) {
			retres = res;
		    }
		}
	    }
            break;
        case PSD_NT_PARM:
	    /* if the parm is not already set and is not read-only
	     * then try to get a value from the user at the CLI
	     */
            parm = (psd_parm_t *)phdr;
	    if (!ps_find_parmnum(ps, parm->parm_id)) {
		if (psd_parm_writable(parm)) {
		    res = get_parm(rpc, parm, ps, oldps);
		    if (res != NO_ERR) {
			log_stdout("\nWarning: Parameter %s has errors (%s)",
			       parm->name, get_error_string(res));
			retres = res;
		    }
		}
	    }
            break;
        case PSD_NT_BLOCK:
	    /* should not see a BLOCK node at this level */
	    /* fall through */
        default: 
            retres = SET_ERROR(ERR_INTERNAL_VAL);
        }
    }
    return retres;

} /* fill_parmset */


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
* Inputs are derived from the module variables in connect_ps:
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
    status_t    res;
    const xmlChar *agent, *username, *password;
    val_value_t *val;

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
    res = ps_get_parmval(connect_ps, NCXCLI_USER, &val);
    if (res == NO_ERR) {
	username = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    res = ps_get_parmval(connect_ps, NCXCLI_AGENT, &val);
    if (res == NO_ERR) {
	agent = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    res = ps_get_parmval(connect_ps, NCXCLI_PASSWORD, &val);
    if (res == NO_ERR) {
	password = VAL_STR(val);
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    log_info("\nncxcli: Starting NETCONF session for %s on %s",
	     username, agent);

    state = MGR_IO_ST_CONNECT;

    /* this fnction call will cause us to block while the
     * protocol layer connect messages are processed
     */
    res = mgr_ses_new_session(username, password, agent, &mysid);
    if (res == NO_ERR) {
	state = MGR_IO_ST_CONN_START;
	log_debug("\nncxcli: Start session %d OK", mysid);
    } else {
	log_info("\nncxcli: Start session failed for user %s on "
		 "%s (%s)\n", username, agent, get_error_string(res));
	state = MGR_IO_ST_IDLE;
    }
    
} /* create_session */


/********************************************************************
 * FUNCTION get_parmset
 * 
 * INPUTS:
 *    rpc == RPC method for the load command
 *    line == CLI input in progress
 *    res == address of status result
 *
 * OUTPUTS:
 *    *res is set to the status
 *
 * RETURNS:
 *   malloced parmset filled in with the parameters for 
 *   the specified RPC
 *
 *********************************************************************/
static ps_parmset_t *
    get_parmset (const rpc_template_t *rpc,
		 const xmlChar *line,
		 status_t  *res)
{
    ps_parmset_t *ps;
    uint32        len;

    *res = NO_ERR;
    ps = NULL;
    len = 0;

    /* skip leading whitespace */
    while (line[len] && xml_isspace(line[len])) {
	len++;
    }

    /* check only whitespace entered after RPC method name */
    if (line[len]) {
	ps = parse_rpc_cli(rpc, &line[len], res);
	if (*res != NO_ERR) {
	    log_stdout("\nError in the parameters for RPC %s (%s)",
		   rpc->name, get_error_string(*res));
	}
    }

    /* check no input from user, so start a parmset */
    if (*res == NO_ERR && !ps) {
	ps = ps_new_parmset();
	if (!ps) {
	    *res = ERR_INTERNAL_MEM;
	} else {
	    *res = ps_setup_parmset(ps, rpc->in_psd, PSD_TYP_RPC);
	}
    }

    /* fill in any missing parameters from the CLI */
    if (*res==NO_ERR && interactive_mode()) {
	*res = fill_parmset(rpc, ps, NULL);
    }

    return ps;

}  /* get_parmset */


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
 *   connect_psparms may be set 
 *   create_session may be called
 *
 *********************************************************************/
static void
    do_connect (const rpc_template_t *rpc,
		const xmlChar *line,
		uint32 start,
		boolean  cli)
{
    status_t   res;
    ps_parmset_t *ps;
    boolean    s1, s2, s3;
    ncx_node_t   dtyp;

    /* retrieve the 'connect' RPC template, if not done already */
    if (!rpc) {
	dtyp = NCX_NT_RPC;
	rpc = (const rpc_template_t *)
	    def_reg_find_moddef(NCXCLIMOD, NCXCLI_CONNECT, &dtyp);
	if (!rpc) {
	    log_write("\nError finding the 'connect' RPC method");
	    return;
	}
    }	    

    /* process any parameters entered on the command line */
    ps = NULL;
    if (line) {
	while (line[start] && xml_isspace(line[start])) {
	    start++;
	}
	if (line[start]) {
	    ps = parse_rpc_cli(rpc, &line[start], &res);
	    if (!ps || res != NO_ERR) {
		log_write("\nError in the parameters for RPC %s (%s)",
		       rpc->name, get_error_string(res));
		return;
	    }
	}
    }

    /* get an empty parmset and use the old set for defaults 
     * unless this is a cll from the program startup and all
     * of the parameters are entered
     */
    if (!ps) {
	s1 = s2 = s3 = FALSE;
	if (cli) {
	    s1 = ps_find_parm(connect_ps, NCXCLI_AGENT) ? TRUE : FALSE;
	    s2 = ps_find_parm(connect_ps, NCXCLI_USER) ? TRUE : FALSE;
	    s3 = (ps_find_parm(connect_ps, NCXCLI_PASSWORD) ||
		  ps_find_parm(connect_ps, NCXCLI_KEY)) ? TRUE : FALSE;
	}
	if (!(s1 && s2 && s3)) {
	    ps = ps_new_parmset();
	    if (ps) {
		ps_setup_parmset(ps, rpc->in_psd, PSD_TYP_RPC);
	    } /* else out of memory error !!! */
	} /* else use all of connect_ps */
    }

    /* if anything entered, try to get any missing params in ps */
    if (ps) {
	if (interactive_mode()) {
	    (void)fill_parmset(rpc, ps, connect_ps);
	}
	if (connect_ps) {
	    ps_free_parmset(connect_ps);
	}
	connect_ps = ps;
    } else if (!cli) {
	if (interactive_mode()) {
	    (void)fill_parmset(rpc, connect_ps, NULL);
	}
    }
	
    /* hack: make sure the 3 required parms are set instead of
     * full validation of the parmset
     */
    s1 = ps_find_parm(connect_ps, NCXCLI_AGENT) ? TRUE : FALSE;
    s2 = ps_find_parm(connect_ps, NCXCLI_USER) ? TRUE : FALSE;
    s3 = (ps_find_parm(connect_ps, NCXCLI_PASSWORD) ||
	  ps_find_parm(connect_ps, NCXCLI_KEY)) ? TRUE : FALSE;

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
 *    none (connect_ps needs to be valid)
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
    do_load (const rpc_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    ps_parmset_t *ps;
    val_value_t  *val;
    status_t      res;

    ps = get_parmset(rpc, &line[len], &res);

    /* get the module name */
    if (res == NO_ERR) {
	res = ps_get_parmval(ps, NCX_EL_MODULE, &val);
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

    if (ps) {
	ps_free_parmset(ps);
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
 *  brief == TRUE if brief report desired
 *           FALSE if full report desired
 *********************************************************************/
static void
    do_show_vars (boolean brief)
{
    ncx_var_t  *var;
    dlq_hdr_t  *que;
    boolean     first, imode;

    imode = interactive_mode();

    if (!brief) {
	/* CLI Parameters */
	if (mgr_cli_ps && !ps_is_empty(mgr_cli_ps)) {
	    if (imode) {
		log_stdout("\nCLI Variables\n");
		ps_stdout_parmset(mgr_cli_ps, NCX_DEF_INDENT);
		log_stdout("\n");
	    } else {
		log_write("\nCLI Variables\n");
		ps_dump_parmset(mgr_cli_ps, NCX_DEF_INDENT);
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
	    /* just print the data type name for complex types */
	    if (imode) {
		log_stdout("\n  %s = (%s)", 
			   var->val->name,
			   tk_get_btype_sym(var->val->btyp));
	    } else {
		log_write("\n  %s = (%s)", 
			  var->val->name,
			  tk_get_btype_sym(var->val->btyp));
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

    que = runstack_get_que(ISLOCAL);
    first = TRUE;
    for (var = (ncx_var_t *)dlq_firstEntry(que);
	 var != NULL;
	 var = (ncx_var_t *)dlq_nextEntry(var)) {
	if (first) {
	    if (imode) {
		log_stdout("\nInteractive Shell Variables");
	    } else {
		log_write("\nInteractive Shell Variables");
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
			   var->val->name,
			   tk_get_btype_sym(var->val->btyp));
	    } else {
		log_write("\n  %s = (%s)", 
			  var->val->name,
			  tk_get_btype_sym(var->val->btyp));
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

} /* do_show_vars */


/********************************************************************
 * FUNCTION do_show_var (sub-mode of local RPC)
 * 
 * show full info for one user var
 *
 * INPUTS:
 *   name == variable name to find 
 *   isglobal == TRUE if global var, FALSE if local var
 *   brief == TRUE to print brief info
 *         == FALSE to print complete value
 *********************************************************************/
static void
    do_show_var (const xmlChar *name,
		 boolean isglobal,
		 boolean brief)
{
    const val_value_t *val;
    boolean            imode;

    imode = interactive_mode();

    val = var_get(name, isglobal);
    if (val) {
	if (brief) {
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
 *    brief == TRUE if brief report desired
 *             FALSE if full report desired
 *********************************************************************/
static void
    do_show_module (const ncx_module_t *mod,
		    boolean brief)
{
    help_data_module(mod, !brief);

} /* do_show_module */


/********************************************************************
 * FUNCTION do_show_modules (sub-mode of local RPC)
 * 
 * show modules
 *
 * INPUTS:
 *    mod == first module to show
 *    brief == TRUE if brief report
 *          == FALSE if full report
 *********************************************************************/
static void
    do_show_modules (const ncx_module_t *mod,
		     boolean brief)
{
    boolean anyout, imode;

    imode = interactive_mode();
    anyout = FALSE;

    while (mod) {
	if (brief) {
	    if (imode) {
		log_stdout("\n  %s/%s/%s", mod->owner,
			   mod->name, mod->version);
	    } else {
		log_write("\n  %s/%s/%s", mod->owner,
			  mod->name, mod->version);
	    }
	} else {
	    help_data_module(mod, PARTIAL);
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
    do_show (const rpc_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    ps_parmset_t *ps;
    ps_parm_t    *parm;
    const ncx_module_t *mod;
    status_t      res;
    boolean       brief, imode;

    imode = interactive_mode();
    ps = get_parmset(rpc, &line[len], &res);

    if (ps && res == NO_ERR) {
	brief = FALSE;

	/* check if the 'brief' flag is set first */
	if (ps_find_parm(ps, NCXCLI_BRIEF)) {
	    brief = TRUE;
	}
	    
	/* there is 1 parm which is a choice of N */
	parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	if (parm) {
	    if (!xml_strcmp(parm->parm->name, NCX_EL_LOCAL)) {
		do_show_var(VAL_STR(parm->val), ISLOCAL, brief);
	    } else if (!xml_strcmp(parm->parm->name, NCX_EL_GLOBAL)) {
		do_show_var(VAL_STR(parm->val), ISGLOBAL, brief);
	    } else if (!xml_strcmp(parm->parm->name, NCX_EL_VARS)) {
		do_show_vars(brief);
	    } else if (!xml_strcmp(parm->parm->name, NCX_EL_MODULE)) {
		mod = ncx_find_module(VAL_STR(parm->val));
		if (mod) {
		    do_show_module(mod, brief);
		} else {
		    if (imode) {
			log_stdout("\nncxcli: module (%s) not loaded",
				   VAL_STR(parm->val));
		    } else {
			log_error("\nncxcli: module (%s) not loaded",
				  VAL_STR(parm->val));
		    }
		}
	    } else if (!xml_strcmp(parm->parm->name, NCX_EL_MODULELIST)) {
		mod = ncx_get_first_module();
		if (mod) {
		    do_show_modules(mod, TRUE);
		} else {
		    if (imode) {
			log_stdout("\nncxcli: no modules loaded");
		    } else {
			log_error("\nncxcli: no modules loaded");
		    }
		}
	    } else if (!xml_strcmp(parm->parm->name, NCX_EL_MODULES)) {
		mod = ncx_get_first_module();
		if (mod) {
		    do_show_modules(mod, brief);
		} else {
		    if (imode) {
			log_stdout("\nncxcli: no modules loaded");
		    } else {
			log_error("\nncxcli: no modules loaded");
		    }
		}
	    } else if (!xml_strcmp(parm->parm->name, NCX_EL_VERSION)) {
		if (imode) {
		    log_stdout("\nncxcli version %s\n", progver);
		} else {
		    log_write("\nncxcli version %s\n", progver);
		}
	    } else {
		/* else some internal error */
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    }

    if (ps) {
	ps_free_parmset(ps);
    }

}  /* do_show */


/********************************************************************
 * FUNCTION do_help
 * 
 * Print the general ncxcli help text to STDOUT
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
    do_help (const rpc_template_t *rpc,
	     const xmlChar *line,
	     uint32  len)
{
    typ_template_t *typ;
    psd_template_t *psd;
    rpc_template_t *rpcdef;
    ps_parmset_t   *ps;
    ps_parm_t      *parm;
    status_t        res;
    boolean         full, imode;
    ncx_node_t      dtyp;
    uint32          dlen;

    imode = interactive_mode();
    ps = get_parmset(rpc, &line[len], &res);

    /* look for the 'brief' parameter */
    parm = ps_find_parm(ps, NCXCLI_BRIEF);
    full = (parm) ? FALSE : TRUE;

    /* look for the specific definition parameters */
    parm = ps_find_parm(ps, NCX_EL_TYPE);
    if (parm) {
	dtyp = NCX_NT_TYP;
	typ = parse_def(&dtyp, VAL_STR(parm->val), &dlen);
	if (typ) {
	    help_type(typ, full);
	} else {
	    if (imode) {
		log_stdout("\nncxcli: type definition (%s) not found",
			   VAL_STR(parm->val));
	    } else {
		log_error("\nncxcli: type definition (%s) not found",
			  VAL_STR(parm->val));
	    }
	}
	return;
    }

    parm = ps_find_parm(ps, NCX_EL_PARMSET);
    if (parm) {
	dtyp = NCX_NT_PSD;
	psd = parse_def(&dtyp, VAL_STR(parm->val), &dlen);
	if (psd) {
	    help_parmset(psd, full);
	} else {
	    if (imode) {
		log_stdout("\nncxcli: parmset definition (%s) not found",
			   VAL_STR(parm->val));
	    } else {
		log_error("\nncxcli: parmset definition (%s) not found",
			  VAL_STR(parm->val));
	    }
	}
	return;
    }

    parm = ps_find_parm(ps, NCX_EL_RPC);
    if (parm) {
	dtyp = NCX_NT_RPC;
	rpcdef = parse_def(&dtyp, VAL_STR(parm->val), &dlen);
	if (rpcdef) {
	    help_rpc(rpcdef, full);
	} else {
	    if (imode) {
		log_stdout("\nncxcli: type definition (%s) not found",
			   VAL_STR(parm->val));
	    } else {
		log_error("\nncxcli: type definition (%s) not found",
			  VAL_STR(parm->val));
	    }
	}
	return;
    }

#ifdef NOT_YET
    parm = ps_find_parm(ps, NCX_EL_NOTIF);
    if (parm) {
	dtyp = NCX_NT_NOT;
	not = parse_def(&dtyp, VAL_STR(parm->val), &dlen);
	if (not) {
	    help_notif(not, full);
	} else {
	    if (imode) {
		log_stdout("\nncxcli: notification definition (%s) not found",
			   VAL_STR(parm->val));
	    } else {
		log_error("\nncxcli: notification definition (%s) not found",
			  VAL_STR(parm->val));
	    }
	}
	return;
    }
#endif

    /* none of the definition parameters are present, so 
     * print the general program help text
     */
    if (imode) {
	log_stdout("\nncxcli version %s", progver);
    } else {
	log_write("\nncxcli version %s", progver);
    }
    help_program_module(NCXCLIMOD, NCXCLI_BOOT, full);

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
 *   ps == parmset for the run script parameters
 *
 * RETURNS:
 *   status
 *********************************************************************/
static status_t
    do_start_script (const xmlChar *source,
		     ps_parmset_t *ps)
{
    xmlChar     *str, *fspec;
    FILE        *fp;
    ps_parm_t   *parm;
    status_t     res;
    int          num;
    xmlChar      buff[4];

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
    for (num=1; num<=NCXCLI_MAX_RUNPARMS; num++) {
	buff[1] = '0' + num;
	parm = ps_find_parm(ps, buff);
	if (parm) {
	    /* store P7 named as ASCII 7 */
	    res = var_set_str(buff+1, 1, parm->val, ISLOCAL);
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
 *********************************************************************/
static void
    do_run (const rpc_template_t *rpc,
	    const xmlChar *line,
	    uint32  len)
{
    ps_parmset_t *ps;
    ps_parm_t    *parm;
    status_t      res;

    /* get the 'script' parameter */
    ps = get_parmset(rpc, &line[len], &res);

    if (ps && res == NO_ERR) {
	/* there is 1 parm */
	parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	if (parm) {
	    if (!xml_strcmp(parm->parm->name, NCX_EL_SCRIPT)) {
		/* the parm val is the script filespec */
		res = do_start_script(VAL_STR(parm->val), ps);
		if (res != NO_ERR) {
		    ncx_print_errormsg(NULL, NULL, res);
		}
	    } else {
		/* else some internal error */
		SET_ERROR(ERR_INTERNAL_VAL);
	    }
	} else {
	    SET_ERROR(ERR_INTERNAL_VAL);
	}
    } /* else error already handled */

    if (ps) {
	ps_free_parmset(ps);
    }

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
    rpc_template_t *rpc;
    ps_parmset_t *ps;
    ps_parm_t    *parm;
    xmlChar      *line, *p;
    status_t      res;
    ncx_node_t    dtyp;
    uint32        linelen;

    /* make sure there is a runscript string */
    if (!runscript || !*runscript) {
	return ERR_NCX_INVALID_VALUE;
    }

    /* get the 'run' RPC method template */
    dtyp = NCX_NT_RPC;
    rpc = (rpc_template_t *)
	def_reg_find_moddef(NCXCLIMOD, NCXCLI_RUN, &dtyp);
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
    
    /* fill=in the parmset for the input parameters */
    res = NO_ERR;
    ps = get_parmset(rpc, line, &res);

    m__free(line);
    line = NULL;

    if (ps && res == NO_ERR) {
	/* there is 1 parm */
	parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	if (parm) {
	    if (!xml_strcmp(parm->parm->name, NCX_EL_SCRIPT)) {
		/* the parm val is the script filespec */
		res = do_start_script(VAL_STR(parm->val), ps);
		if (res != NO_ERR) {
		    ncx_print_errormsg(NULL, NULL, res);
		}
	    } else {
		/* else some internal error */
		res = SET_ERROR(ERR_INTERNAL_VAL);
	    }
	} else {
	    res = SET_ERROR(ERR_INTERNAL_VAL);
	}
    } /* else error already handled */

    if (ps) {
	ps_free_parmset(ps);
    }

    return res;

}  /* do_startup_script */


/********************************************************************
* FUNCTION do_local_command
* 
* Handle local RPC operations from ncxcli.ncx
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
    do_local_command (const rpc_template_t *rpc,
		   xmlChar *line,
		   uint32  len)
{
    if (!xml_strcmp(rpc->name, NCXCLI_CONNECT)) {
	do_connect(rpc, line, len, FALSE);
    } else if (!xml_strcmp(rpc->name, NCXCLI_HELP)) {
	do_help(rpc, line, len);
    } else if (!xml_strcmp(rpc->name, NCXCLI_LOAD)) {
	do_load(rpc, line, len);
    } else if (!xml_strcmp(rpc->name, NCXCLI_QUIT)) {
	state = MGR_IO_ST_SHUT;
	mgr_request_shutdown();
    } else if (!xml_strcmp(rpc->name, NCXCLI_RUN)) {
	do_run(rpc, line, len);
    } else if (!xml_strcmp(rpc->name, NCXCLI_SHOW)) {
	do_show(rpc, line, len);
    } else {
	log_error("\nError: The %s command is not allowed in this mode",
		   rpc->name);
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
    const rpc_template_t  *rpc;
    uint32                 len;
    ncx_node_t             dtyp;
    status_t               res;

    if (!xml_strlen(line)) {
	return;
    }

    dtyp = NCX_NT_RPC;
    rpc = (const rpc_template_t *)parse_def(&dtyp, line, &len);
    if (!rpc) {
	if (result_name) {
	    /* this is just a string assignment */
	    res = var_set_from_string(result_name,
				      line, result_isglobal);
	    if (res != NO_ERR) {
		log_error("\nncxcli: Error setting variable %s (%s)",
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

    /* check  handful of ncxcli commands */
    if (is_ncxcli_ns(rpc->nsid)) {
	do_local_command(rpc, line, len);
    } else {
	log_error("\nError: Not connected to agent."
		  "\nLocal commands only in this mode.");
    }

} /* top_command */


/********************************************************************
 * FUNCTION ncxcli_reply_handler
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
    ncxcli_reply_handler (ses_cb_t *scb,
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
		log_error("\nncxcli reply: set result failed (%s)",
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

}  /* ncxcli_reply_handler */


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
    const rpc_template_t  *rpc;
    mgr_rpc_req_t         *req;
    val_value_t           *reqdata;
    ps_parmset_t          *ps;
    ps_parm_t             *parm;
    ses_cb_t              *scb;
    uint32                 len, linelen;
    status_t               res;
    boolean                shut, load;
    ncx_node_t             dtyp;

    req = NULL;
    reqdata = NULL;
    ps = NULL;
    res = NO_ERR;
    shut = FALSE;
    load = FALSE;

    /* make sure there is something to parse */
    linelen = xml_strlen(line);
    if (!linelen) {
	return;
    }

    /* get the RPC method template */
    dtyp = NCX_NT_RPC;
    rpc = (const rpc_template_t *)parse_def(&dtyp, line, &len);
    if (!rpc) {
	if (result_name) {
	    /* this is just a string assignment */
	    res = var_set_from_string(result_name,
				      line, result_isglobal);
	    if (res != NO_ERR) {
		log_error("\nncxcli: Error setting variable %s (%s)",
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
    if (is_ncxcli_ns(rpc->nsid)) {
	if (!xml_strcmp(rpc->name, NCXCLI_CONNECT)) {
	    log_stdout("\nError: Already connected");
	} else if (!xml_strcmp(rpc->name, NCXCLI_SAVE)) {
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
    reqdata = xml_val_new_struct(rpc->name, rpc->nsid);
    if (!reqdata) {
	log_error("\nError allocating a new RPC request");
	res = ERR_INTERNAL_MEM;
    }
	
    /* check if any params are expected */
    if (res == NO_ERR && rpc->in_psd) {
	while (line[len] && xml_isspace(line[len])) {
	    len++;
	}

	if (len < linelen) {
	    ps = parse_rpc_cli(rpc, &line[len], &res);
	    if (res != NO_ERR) {
		log_error("\nError in the parameters for RPC %s (%s)",
		       rpc->name, get_error_string(res));
	    }
	}

	/* check no input from user, so start a parmset */
	if (res == NO_ERR && !ps) {
	    ps = ps_new_parmset();
	    if (!ps) {
		res = ERR_INTERNAL_MEM;
	    } else {
		res = ps_setup_parmset(ps, rpc->in_psd, PSD_TYP_RPC);
	    }
	}

	/* fill in any missing parameters from the CLI */
	if (res == NO_ERR) {
	    if (interactive_mode()) {
		res = fill_parmset(rpc, ps, NULL);
	    }
	}

	/* go through the parm list and move the values 
	 * to the reqdata struct. The ps_parm_t is
	 * designed to allow the parm->val node 
	 * to be moved to another tree and persist
	 * after the ps_parm_t node has been freed
	 */
	if (res == NO_ERR) {
	    for (parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
		 parm != NULL;
		 parm = (ps_parm_t *)dlq_nextEntry(parm)) {
		if (parm->val) {
		    val_add_child(parm->val, reqdata);
		    parm->val = NULL;
		}
	    }
	}
    }

    /* check the close-session corner cases */
    if (res == NO_ERR && !xml_strcmp(rpc->name, NCX_EL_CLOSE_SESSION)) {
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
	    /* the request will be stored if this returns NO_ERR */
	    res = mgr_rpc_send_request(scb, req, ncxcli_reply_handler);
	}
    }

    if (ps) {
	ps_free_parmset(ps);
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
    psd_template_t  *psd;
    ps_parm_t       *parm;
    psd_parm_t      *psd_parm;
    status_t         res;
    ncx_node_t       dtyp;

    /* check no command line parms */
    if (argc <= 1) {
	mgr_cli_ps = NULL;
	return NO_ERR;
    }

    /* find the parmset definition in the registry */
    dtyp = NCX_NT_PSD;
    psd = (psd_template_t *)
	def_reg_find_moddef(NCXCLIMOD, NCXCLI_BOOT, &dtyp);
    if (!psd) {
	res = ERR_NCX_NOT_FOUND;
	
    }

    /* parse the command line against the PSD */    
    mgr_cli_ps = ps_parse_cli(argc, argv, psd, 
			      FULLTEST, PLAINMODE,
			      autocomp, &res);
    if (!mgr_cli_ps) {
	return res;
    }

    /* next get any params from the conf file */
    confname = get_strparm(mgr_cli_ps, NCXCLI_CONF);
    if (confname) {
	res = conf_parse_ps_from_filespec(confname, 
					  mgr_cli_ps, TRUE, TRUE);
	if (res != NO_ERR) {
	    return res;
	}
    }

    /****************************************************
     * go through the ncxcli params in order,
     * after setting up the logging parameters
     ****************************************************/

    /* get the log-level parameter */
    parm = ps_find_parm(mgr_cli_ps, NCX_EL_LOGLEVEL);
    if (parm) {
	log_level = 
	    log_get_debug_level_enum((const char *)VAL_STR(parm->val));
	if (log_level == LOG_DEBUG_NONE) {
	    return ERR_NCX_INVALID_VALUE;
	} else {
	    log_set_debug_level(log_level);
	}
    }

    /* get the log-append parameter */
    parm = ps_find_parm(mgr_cli_ps, NCX_EL_LOGAPPEND);
    if (parm) {
	logappend = TRUE;
    }

    /* get the log parameter if present */
    parm = ps_find_parm(mgr_cli_ps, NCX_EL_LOG);
    if (parm && VAL_STR(parm->val)) {
	res = NO_ERR;
	logfilename = xml_strdup(VAL_STR(parm->val));
	if (!logfilename) {
	    res = ERR_INTERNAL_MEM;
	    log_error("\nncxcli: Out of Memory error");
	} else {
	    res = log_open((const char *)logfilename, logappend, FALSE);
	    if (res != NO_ERR) {
		log_error("\nncxcli: Could not open logfile %s",
			  logfilename);
	    }
	}
	if (res != NO_ERR) {
	    return res;
	}
    }


    /* get the agent parameter */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_AGENT);
    if (parm) {
	/* save to the connect_ps parmset */
	psd_parm = psd_find_parm(connect_ps->psd, NCXCLI_AGENT);
	res = add_clone_parm(psd_parm, connect_ps, VAL_STR(parm->val));
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the batch-mode parameter */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_BATCHMODE);
    if (parm) {
	batchmode = TRUE;
    }

    /* get the default module for unqualified module addesses */
    default_module = get_strparm(mgr_cli_ps, NCXCLI_DEF_MODULE);

    /* get the help flag */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_HELP);
    if (parm) {
	helpmode = TRUE;
    }

    /* get the key parameter */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_KEY);
    if (parm) {
	/* save to the connect_ps parmset */
	psd_parm = psd_find_parm(connect_ps->psd, NCXCLI_KEY);
	res = add_clone_parm(psd_parm, connect_ps, VAL_STR(parm->val));
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the password parameter */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_PASSWORD);
    if (parm) {
	/* save to the connect_ps parmset */
	psd_parm = psd_find_parm(connect_ps->psd, NCXCLI_PASSWORD);	
	res = add_clone_parm(psd_parm, connect_ps, VAL_STR(parm->val));
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the modules parameter */
    parm = ps_find_parm(mgr_cli_ps, NCX_EL_MODULES);
    if (parm) {
	modules = val_clone(parm->val);
    }

    /* get the no-autocomp parameter */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_NO_AUTOCOMP);
    if (parm) {
	autocomp = FALSE;
    }

    /* get the no-autoload parameter */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_NO_AUTOLOAD);
    if (parm) {
	autoload = FALSE;
    }

    /* get the no-fixorder parameter */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_NO_FIXORDER);
    if (parm) {
	fixorder = FALSE;
    }

    /* get the run-script parameter */
    runscript = get_strparm(mgr_cli_ps, NCXCLI_RUN_SCRIPT);

    /* get the user name */
    parm = ps_find_parm(mgr_cli_ps, NCXCLI_USER);
    if (parm) {
	psd_parm = psd_find_parm(connect_ps->psd, NCXCLI_USER);	
	res = add_clone_parm(psd_parm, connect_ps, VAL_STR(parm->val));
	if (res != NO_ERR) {
	    return res;
	}
    }

    /* get the version parameter */
    parm = ps_find_parm(mgr_cli_ps, NCX_EL_VERSION);
    if (parm) {
	versionmode = TRUE;
    }

    return NO_ERR;

} /* process_cli_input */


/********************************************************************
 * FUNCTION load_base_schema 
 * 
 * Load the following NCX modules:
 *   ncxcli
 *
 * RETURNS:
 *     status
 *********************************************************************/
static status_t
    load_base_schema (void)
{
    status_t   res;

    log_debug2("\nNcxcli: Loading NCX ncxcli-cli Parmset");

    /* load in the agent boot parameter definition file */
    res = ncxmod_load_module(NCXCLIMOD);
    if (res != NO_ERR) {
	return res;
    }

    return NO_ERR;

}  /* load_base_schema */


/********************************************************************
 * FUNCTION load_core_schema 
 * 
 * Load the following NCX modules:
 *   xsd
 *   smi
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

    /* load in the SMI data types */
    res = ncxmod_load_module(SMIMOD);
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
    const mgr_scb_t  *mscb;
    const xmlChar *agent;
    const ps_parm_t *parm;

    mscb = (const mgr_scb_t *)scb->mgrcb;

    parm = ps_find_parm(connect_ps, NCXCLI_AGENT);
    if (parm) {
	agent = VAL_STR(parm->val);
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
    const xmlChar      *owner, *module, *version;
    uint32              ownerlen, modlen;
    status_t            res;
    xmlChar             namebuff[NCX_MAX_NLEN+1];

    mscb = (const mgr_scb_t *)scb->mgrcb;

    log_info("\n\nChecking Agent Modules...");

    cap = cap_first_modcap(&mscb->caplist);

    while (cap) {
	cap_split_modcap(cap,
			 &owner,
			 &ownerlen,
			 &module,
			 &modlen,
			 &version);

	xml_strncpy(namebuff, module, modlen);
	mod = ncx_find_module(namebuff);
	if (!mod) {
	    if (autoload) {
		res = ncxmod_load_module(namebuff);
		if (res != NO_ERR) {
		    log_error("\nncxcli error: Module %s not loaded (%s)!!",
			      namebuff, get_error_string(res));
		}
	    } else {
		log_info("\nncxcli warning: Module %s not loaded!!",
			 namebuff);
	    }
	}

	if (mod) {
	    if (xml_strncmp(mod->owner, owner, ownerlen)) {
		log_warn("\nncxcli warning: Module %s "
			 "has different owner on agent!! (%s)",
			 namebuff, mod->owner);
	    }
	    if (xml_strcmp(mod->version, version)) {
		log_warn("\nncxcli warning: Module %s "
			 "has different version on agent!! (%s)",
			 namebuff, mod->version);
	    }
	}

	cap = cap_next_modcap(cap);
    }
    
} /* check_module_capabilities */


/********************************************************************
* ncxcli_stdin_handler
*
* Temp: Calling readline which will block other IO while the user
*       is editing the line.  This is okay for this CLI application
*       but not multi-session applications
*
* RETURNS:
*   new program state
*********************************************************************/
static mgr_io_state_t
    ncxcli_stdin_handler (void)
{
    xmlChar    *line;
    ses_cb_t   *scb;
    boolean     getrpc;
    status_t    res;
    uint32      len;

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
	if (!mgr_ses_get_scb(mysid)) {
	    mysid = 0;
	    state = MGR_IO_ST_IDLE;
	} else {
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
	log_error("\nncxcli: Variable assignment failed (%s) (%s)",
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

} /* ncxcli_stdin_handler */


/********************************************************************
 * FUNCTION ncxcli_init
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
    ncxcli_init (int argc,
	      const char *argv[])
{
    status_t    res;
    ncx_lmem_t *lmem;
    ps_parm_t  *parm;
    ncx_node_t  dtyp;
    const psd_template_t *psd;

#ifdef NCXCLI_DEBUG
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
    mgr_cli_ps = NULL;
    connect_ps = NULL;
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
    cli_gl = new_GetLine(NCXCLI_LINELEN, NCXCLI_HISTLEN);
    if (!cli_gl) {
	return ERR_INTERNAL_MEM;
    }

    /* initialize the NCX Library first to allow NCX modules
     * to be processed.  No module can get its internal config
     * until the NCX module parser and definition registry is up
     */
    res = ncx_init(NCX_SAVESTR, log_level, "\nStarting ncxcli...");
    if (res != NO_ERR) {
	return res;
    }

#ifdef NCXCLI_DEBUG
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

    /* Load the ncxcli base module */
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
    dtyp = NCX_NT_PSD;
    psd = (const psd_template_t *)
	def_reg_find_moddef(NCXCLIMOD,
			    (const xmlChar *)"NcxCliConnectPS",
			    &dtyp);
    if (!psd) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* treat the connect-to-agent parmset special
     * it is saved for auto-start plus restart parameters
     * Setup an empty parmset to hold the connect parameters
     */
    connect_ps = ps_new_parmset();
    if (!connect_ps) {
	return ERR_INTERNAL_MEM;
    } else {
	res = ps_setup_parmset(connect_ps, psd, PSD_TYP_RPC);
	if (res != NO_ERR) {
	    return res;
	}
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
	log_stdout("\nncxcli version %s\n", progver);
    }

    /* check print help and exit */
    if (helpmode) {
	log_stdout("\nncxcli version %s", progver);
	help_program_module(NCXCLIMOD, NCXCLI_BOOT, FULL);
    }

    /* check quick exit */
    if (helpmode || versionmode) {
	return NO_ERR;
    }

    /* set the CLI handler */
    mgr_io_set_stdin_handler(ncxcli_stdin_handler);

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

	    log_info("\nncxcli: Loading requested module %s", 
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
     * The ncxcli_stdin_handler will call the finish_start_session
     * function when the user enters a line of keyboard text
     */
    state = MGR_IO_ST_IDLE;
    if (connect_ps) {
	parm = ps_find_parm(connect_ps, NCXCLI_AGENT);
	if (parm) {
	    do_connect(NULL, NULL, 0, TRUE);
	}
    }

    return NO_ERR;

}  /* ncxcli_init */


/********************************************************************
 * FUNCTION ncxcli_cleanup
 * 
 * 
 * 
 *********************************************************************/
static void
    ncxcli_cleanup (void)
{
    log_debug2("\nShutting down ncxcli\n");

    /* cleanup the user edit buffer */
    if (cli_gl) {
	(void)del_GetLine(cli_gl);
	cli_gl = NULL;
    }

    /* Cleanup the Netconf Agent Library */
    mgr_cleanup();

    /* cleanup the NCX engine and registries */
    ncx_cleanup();

    /* cleanup the NETCONF operation attribute */
    ncx_clean_operation_attr();

    /* clean and reset all module static vars */
    state = MGR_IO_ST_NONE;
    mysid = 0;

    if (mgr_cli_ps) {
	ps_free_parmset(mgr_cli_ps);
	mgr_cli_ps = NULL;
    }

    if (connect_ps) {
	ps_free_parmset(connect_ps);
	connect_ps = NULL;
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

}  /* ncxcli_cleanup */


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

    res = ncxcli_init(argc, argv);
    if (res != NO_ERR) {
	log_error("\nNcxcli: init returned error (%s)\n", 
		  get_error_string(res));
    } else if (!(helpmode || versionmode)) {
	log_stdout("\nncxcli version %s\n", progver);
	res = mgr_io_run();
	if (res != NO_ERR) {
	    log_error("\nmgr_io failed (%d)\n", res);
	}
    }

    print_errors();
    ncxcli_cleanup();

    return 0;

} /* main */


/* END ncxcli.c */
