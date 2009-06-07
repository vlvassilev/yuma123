#ifndef _H_agt_sys
#define _H_agt_sys
/*  FILE: agt_sys.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

   NETCONF system.yang DM module support

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
04-jun-09    abb      Begun.
*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tstamp
#include "tstamp.h"
#endif


/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

#define AGT_SYS_MODULE     (const xmlChar *)"system"


/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern status_t
    agt_sys_init (void);

extern status_t
    agt_sys_init2 (void);

extern void 
    agt_sys_cleanup (void);

extern void
    agt_sys_send_sysSessionStart (const ses_cb_t *scb);

extern void
    agt_sys_send_sysSessionEnd (const ses_cb_t *scb,
				ses_term_reason_t termreason,
				ses_id_t killedby);

extern void
    agt_sys_send_sysConfigChange (const ses_cb_t *scb,
				  const xmlChar *target,
				  const xmlChar *operation);

#endif            /* _H_agt_sys */
