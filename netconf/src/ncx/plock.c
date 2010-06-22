/*
 * Copyright (c) 2010, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: plock.c

   Partial lock control block
   Support for RFC 5717 operations

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
21jun10      abb      begun

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

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_cfg
#include  "cfg.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_plock
#include  "plock.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tstamp
#include  "tstamp.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_xpath
#include  "xpath.h"
#endif

#ifndef _H_xpath1
#include  "xpath1.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
#ifdef DEBUG
#define PLOCK_DEBUG 1
#endif


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
/* the ID zero will not get used */
static uint32 last_id = 0;


/********************************************************************
* FUNCTION plock_new_cb
*
* Create a new partial lock control block
*
* INPUTS:
*   res == address of return status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*   pointer to initialized PLCB, or NULL if some error
*   this struct must be freed by the caller
*********************************************************************/
plock_cb_t *
    plock_new_cb (status_t *res)
{
    plock_cb_t     *plcb;

#ifdef DEBUG
    if (res == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* temp design: only 4G partial locks supported
     * until server stops giving out partial locks
     */
    if (last_id == NCX_MAX_UINT) {
        *res = ERR_NCX_RESOURCE_DENIED;
        return NULL;
    }

    plcb = m__getObj(plock_cb_t);
    if (plcb == NULL) {
        *res = ERR_INTERNAL_MEM;
        return NULL;
    }
    memset(plcb, 0x0, sizeof(plock_cb_t));

    plcb->plock_id = ++last_id;
    tstamp_datetime(plcb->plock_time);

    return plcb;

}  /* plock_new_cb */


/********************************************************************
* FUNCTION plock_free_cb
*
* Free a partial lock control block
*
* INPUTS:
*   plcb == partial lock control block to free
*
*********************************************************************/
void
    plock_free_cb (plock_cb_t *plcb)
{
#ifdef DEBUG
    if (plcb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (plcb->plock_pcb != NULL) {
        xpath_free_pcb(plcb->plock_pcb);
    }

    if (plcb->plock_result != NULL) {
        xpath_free_result(plcb->plock_result);
    }

    m__free(plcb);

}  /* plock_free_cb */


/********************************************************************
* FUNCTION plock_reset_id
*
* Set the next ID number back to the start
* Only the caller maintaining a queue of plcb
* can decide if the ID should rollover
*
*********************************************************************/
void
    plock_reset_id (void)
{
    if (last_id == NCX_MAX_UINT) {
        last_id = 0;
    }

}  /* plock_reset_id */


/********************************************************************
* FUNCTION plock_get_id
*
* Get the lock ID for this partial lock
*
* INPUTS:
*   plcb == partial lock control block to use
*
*********************************************************************/
plock_id_t
    plock_get_id (plock_cb_t *plcb)
{
#ifdef DEBUG
    if (plcb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    return plcb->plock_id;

}  /* plock_get_id */


/********************************************************************
* FUNCTION plock_get_sid
*
* Get the session ID holding this partial lock
*
* INPUTS:
*   plcb == partial lock control block to reset
*
*********************************************************************/
uint32
    plock_get_sid (plock_cb_t *plcb)
{
#ifdef DEBUG
    if (plcb == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return 0;
    }
#endif

    return plcb->plock_sesid;

}  /* plock_get_sid */


/* END file plock.c */
