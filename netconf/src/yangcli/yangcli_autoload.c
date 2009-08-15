/*  FILE: yangcli_autoload.c

   NETCONF YANG-based CLI Tool

   autoload support

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
13-aug-09    abb      begun; started from yangcli.c

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

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_status
#include "status.h"
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

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yangcli
#include "yangcli.h"
#endif

#ifndef _H_yangcli_autoload
#include "yangcli_autoload.h"
#endif

#ifndef _H_yangcli_cmd
#include "yangcli_cmd.h"
#endif

#ifndef _H_yangcli_util
#include "yangcli_util.h"
#endif


/********************************************************************
* FUNCTION autoload_module
* 
* auto-load the specified module
*
* INPUTS:
*   modname == module name
*   revision == module revision
*   devlist  == deviation list
*   retmod == address of return module
*
* OUTPUTS:
*   *retmod == loaded module (if NO_ERR)
*
* RETURNS:
*    status
*********************************************************************/
status_t
    autoload_module (const xmlChar *modname,
                     const xmlChar *revision,
                     ncx_list_t *devlist,
                     ncx_module_t **retmod)
{
    dlq_hdr_t               savedevQ;
    ncx_lmem_t             *listmember;
    status_t                res;
                                      
    res = NO_ERR;
    dlq_createSQue(&savedevQ);

    /* first load any deviations */
    for (listmember = ncx_first_lmem(devlist);
         listmember != NULL && res == NO_ERR;
         listmember = (ncx_lmem_t *)dlq_nextEntry(listmember)) {

        res = ncxmod_load_deviation(listmember->val.str,
                                    &savedevQ);
        if (res != NO_ERR) {
            log_error("\nError: Deviation module %s not loaded (%s)!!",
                      listmember->val.str, 
                      get_error_string(res));
        }
    }

    /* load the requested module now */
    if (res == NO_ERR) {
        res = ncxmod_load_module(modname, 
                                 revision, 
                                 &savedevQ,
                                 retmod);
        if (res != NO_ERR) {
            log_error("\nError: Auto-load for module '%s' failed (%s)",
                      modname, 
                      get_error_string(res));
        }
    }

    ncx_clean_save_deviationsQ(&savedevQ);

    return res;

}  /* autoload_module */



/* END yangcli_autoload.c */
