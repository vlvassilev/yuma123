/*  FILE: tr_mgr.c

   NETCONF Transport Manager

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
22may05      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_base
#include  "base.h"
#endif

#ifndef _H_rpc
#include  "rpc.h"
#endif

#ifndef _H_xml
#include  "xml.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/
    

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION tr_mgr_init
*
* Initialize the module data and figure out what NETCONF
* transports are available
*
* INPUTS:
*   none
* RETURNS:
*   NO_ERR if all okay (at least one transport installed)
*********************************************************************/
status_t  tr_mgr_init (void) 
{

} /* tr_mgr_init */


/********************************************************************
* FUNCTION tr_mgr_cleanup
*
* Cleanup the module data
*
* INPUTS:
*   none
* RETURNS:
*   none
*********************************************************************/
void  tr_mgr_cleanup (void) 
{

} /* tr_mgr_cleanup */


/* END file tr_mgr.c */
