/*  FILE: agtinst.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
29apr06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

/*
#ifndef _H_agt
#include "agt.h"
#endif
*/

#ifndef _H_agtinst
#include "agtinst.h"
#endif

#ifndef _H_agt_netconfd
#include "agt_netconfd.h"
#endif

/*
#ifndef _H_agt_util
#include "agt_util.h"
#endif
*/

#ifndef _H_status
#include  "status.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agtinst_init
* 
* Initialize the NCX Agent core parmset method routines
* 
* RETURNS:
*   status of the initialization procedure
*********************************************************************/
status_t 
    agtinst_init (void)
{
    status_t  res;


    res = agt_netconfd_init();
    if (res != NO_ERR) {
	return res;
    }

    return res;

}  /* agtinst_init */


/********************************************************************
* FUNCTION agtinst_cleanup
* 
* Cleanup the NCX Agent core parmset method routines
* 
*
*********************************************************************/
void
    agtinst_cleanup (void)
{

    agt_netconfd_cleanup();


}  /* agtinst_cleanup */


/* END file agtinst.c */
