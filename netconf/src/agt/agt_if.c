/*  FILE: agt_state.c

   interfaces monitoring module

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
18jul09      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include  "agt.h"
#endif

#ifndef _H_agt_cb
#include  "agt_cb.h"
#endif

#ifndef _H_agt_if
#include  "agt_if.h"
#endif

#ifndef _H_agt_rpc
#include  "agt_rpc.h"
#endif

#ifndef _H_agt_util
#include  "agt_util.h"
#endif

#ifndef _H_cfg
#include  "cfg.h"
#endif

#ifndef _H_getcb
#include  "getcb.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncxmod
#include  "ncxmod.h"
#endif

#ifndef _H_ncxtypes
#include  "ncxtypes.h"
#endif

#ifndef _H_rpc
#include  "rpc.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_ses_msg
#include  "ses_msg.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_val_util
#include  "val_util.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define AGT_IF_DEBUG 1
#endif

#define interfaces_MOD        (const xmlChar *)"interfaces"

#define interfaces_MOD_REV    NULL

#define interfaces_N_interfaces      (const xmlChar *)"interfaces"

#define interfaces_N_interface       (const xmlChar *)"interface"

#define interface_N_name              (const xmlChar *)"name"

/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/

static boolean              agt_if_init_done = FALSE;

static boolean              agt_if_not_supported;

static ncx_module_t         *ifmod;

static val_value_t          *myinterfacesval;

static const obj_template_t *myinterfacesobj;


/********************************************************************
* FUNCTION is_interfaces_supported
*
* Check if at least /proc/net/dev exists
*
* RETURNS:
*   TRUE if minimum if support found
*   FALSE if minimum if support not found
*********************************************************************/
static boolean
    is_interfaces_supported (void)
{
    struct stat      statbuf;
    int              ret;

    memset(&statbuf, 0x0, sizeof(statbuf));
    ret = stat("/proc/net/dev", &statbuf);
    if (ret == 0 && S_ISREG(statbuf.st_mode)) {
        return TRUE;
    }

    return FALSE;

} /* is_interfaces_supported */



#if 0
/********************************************************************
* FUNCTION get_interfaces
*
* <get> operation handler for the interfaces NP container
*
* INPUTS:
*    see ncx/getcb.h getcb_fn_t for details
*
* RETURNS:
*    status
*********************************************************************/
static status_t 
    get_interface (ses_cb_t *scb,
                   getcb_mode_t cbmode,
                   val_value_t *virval,
                   val_value_t  *dstval)
{
    FILE                  *meminfofile;
    const obj_template_t  *meminfoobj;
    val_value_t           *parmval;
    char                  *buffer, *readtest;
    boolean                done;
    status_t               res;

    (void)scb;
    res = NO_ERR;

    if (cbmode != GETCB_GET_VALUE) {
        return ERR_NCX_OPERATION_NOT_SUPPORTED;
    }

    meminfoobj = virval->obj;

    /* open the /interfaces/meminfo file for reading */
    meminfofile = fopen("/interfaces/meminfo", "r");
    if (meminfofile == NULL) {
        return errno_to_status();
    }

    /* get a file read line buffer */
    buffer = m__getMem(NCX_MAX_LINELEN);
    if (buffer == NULL) {
        fclose(meminfofile);
        return ERR_INTERNAL_MEM;
    }

    /* loop through the file until done */
    res = NO_ERR;
    done = FALSE;
    while (!done) {
        readtest = fgets(buffer, NCX_MAX_LINELEN, meminfofile);
        if (readtest == NULL) {
            done = TRUE;
            continue;
        }

        if (strlen(buffer) == 1 && *buffer == '\n') {
            ;
        } else {
            res = NO_ERR;
            parmval = make_interfaces_leaf(buffer, meminfoobj, &res);
            if (parmval) {
                val_add_child(parmval, dstval);
            }
        }
    }

    fclose(meminfofile);
    m__free(buffer);

    return res;

} /* get_interfaces */
#endif


/********************************************************************
* FUNCTION add_interface_entries
*
* make a val_value_t struct for each interface line found
* in the /proc/net/dev file
* and add it as a child to the specified container value
*
INPUTS:
*   interfaacesval == parent value struct to add each
*   interface list entry
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    add_interface_entries (val_value_t *interfacesval)
{
    (void)interfacesval;

#if 0
    const obj_template_t  *meminfoobj;
    val_value_t           *meminfoval;

    /* find the meminfo object */
    meminfoobj = obj_find_child(myinterfacesobj,
                                interfaces_MOD,
                                interfaces_N_meminfo);
    if (meminfoobj == NULL) {
        return ERR_NCX_DEF_NOT_FOUND;
    }

    /* create meminfo virtual NP container */
    meminfoval = val_new_value();
    if (meminfoval == NULL) {
        return ERR_INTERNAL_MEM;
    }
    val_init_virtual(meminfoval, get_meminfo, meminfoobj);

    /* hand off meminfoval memory here */
    val_add_child(meminfoval, interfacesval);
#endif

    return NO_ERR;

}  /* add_interface_entries */


/************* E X T E R N A L    F U N C T I O N S ***************/


/********************************************************************
* FUNCTION agt_if_init
*
* INIT 1:
*   Initialize the interfaces monitor module data structures
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_if_init (void)
{
    status_t   res;

    if (agt_if_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

#ifdef AGT_IF_DEBUG
    log_debug2("\nagt: Loading interfaces module");
#endif

    ifmod = NULL;
    myinterfacesval = NULL;
    myinterfacesobj = NULL;
    agt_if_not_supported = FALSE;
    agt_if_init_done = TRUE;

    /* check if /interfaces file system supported */
    if (!is_interfaces_supported()) {
        if (LOGDEBUG) {
            log_debug("\nagt_interfaces: no /interfaces support found");
        }
        agt_if_not_supported = TRUE;
        return NO_ERR;
    }

    /* load the netconf-state module */
    res = ncxmod_load_module(interfaces_MOD,
                             interfaces_MOD_REV,
                             &ifmod);

    return res;

}  /* agt_if_init */


/********************************************************************
* FUNCTION agt_if_init2
*
* INIT 2:
*   Initialize the monitoring data structures
*   This must be done after the <running> config is loaded
*
* INPUTS:
*   none
* RETURNS:
*   status
*********************************************************************/
status_t
    agt_if_init2 (void)
{
    cfg_template_t        *runningcfg;
    status_t               res;

    if (!agt_if_init_done) {
        return SET_ERROR(ERR_INTERNAL_INIT_SEQ);
    }

    if (agt_if_not_supported) {
        return NO_ERR;
    }

    res = NO_ERR;

    runningcfg = cfg_get_config_id(NCX_CFGID_RUNNING);
    if (!runningcfg || !runningcfg->root) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* get all the object nodes first */
    myinterfacesobj = obj_find_template_top(ifmod, 
                                            interfaces_MOD,
                                            interfaces_N_interfaces);
    if (!myinterfacesobj) {
        return SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
    }

    /* add /interfaces */
    myinterfacesval = val_new_value();
    if (!myinterfacesval) {
        return ERR_INTERNAL_MEM;
    }
    val_init_from_template(myinterfacesval, 
                           myinterfacesobj);

    /* handing off the malloced memory here */
    val_add_child(myinterfacesval, 
                  runningcfg->root);

    res = add_interface_entries(myinterfacesval);

    return res;

}  /* agt_if_init2 */


/********************************************************************
* FUNCTION agt_if_cleanup
*
* Cleanup the module data structures
*
* INPUTS:
*   
* RETURNS:
*   none
*********************************************************************/
void 
    agt_if_cleanup (void)
{
    if (agt_if_init_done) {
        ifmod = NULL;
        myinterfacesval = NULL;
        myinterfacesval = NULL;
        agt_if_init_done = FALSE;
    }
}  /* agt_if_cleanup */


/* END file agt_if.c */
