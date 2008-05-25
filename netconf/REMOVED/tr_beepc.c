/*  FILE: tr_beepc.c

   NETCONF Transport : BEEP Client

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
09jun05      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>

#include "rr.h"

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xml
#include  "xml.h"
#endif

#ifndef _H_tr_beepc
#include  "tr_beepc.h"
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
static RRProfileRegistry  *profreg;



/********************************************************************
* FUNCTION add_nc_profile
*
* Add the NETCONF standard profile to the 'profreg'
*
* INPUTS:
*   none
* RETURNS:
*   NO_ERR if all okay 
*********************************************************************/
static status_t  add_nc_profile (void) 
{

} /* add_nc_profile */






/********************************************************************
* FUNCTION tr_beepc_init
*
* Initialize the module data and the Roadrunner BEEP Client
*
* INPUTS:
*   none
* RETURNS:
*   NO_ERR if all okay 
*********************************************************************/
status_t  tr_beepc_init (void) 
{
    int           rr_ok;
    status_t      res;


    GError *error = NULL;

    rr_ok = rr_init(NULL, NULL, &error);
    if (!rr_ok) {
	return ERR_TR_BEEP_INIT;
    }

    profreg = rr_profile_registry_new();
    if (!profreg) {
	(void)rr_exit(&error);
	return ERR_TR_BEEP_INIT;
    }

    res = add_nc_profile();
    if (res != NO_ERR) {
	(void)rr_exit(&error);
	return res;
    }


} /* tr_beepc_init */


/********************************************************************
* FUNCTION tr_beepc_cleanup
*
* Cleanup the module data
*
* INPUTS:
*   none
* RETURNS:
*   none
*********************************************************************/
void  tr_beepc_cleanup (void) 
{
    GError  *error;

    (void)rr_exit(&error);

} /* tr_beepc_cleanup */


/* END file tr_beepc.c */
