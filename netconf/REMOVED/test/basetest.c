/*  FILE: basetest.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
28-apr-05    abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_cap
#include "cap.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_op_wrreq
#include "op_wrreq.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_mgr
#include "rpc_mgr.h"
#endif

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


/********************************************************************
*                                                                   *
* FUNCTION reqpdu_test
*                                                                   *
*********************************************************************/
static int reqpdu_test (void)
{
    status_t   res;
    rpc_req_t  *req;

    op_source_t source;
    op_filter_t filter;

    req = rpc_mgr_alloc_req(2000,7);
    if (!req) {
	return 1;
    }

    op_set_src_config(&source, OP_CONFIG_CANDIDATE);
    op_set_filter_xpath(&filter, (const xmlChar *)"/top/users/user");

    res = op_wrreq_get_config(&source, &filter, 
			      rpc_get_attrs(req),
			      rpc_get_buff(req));

    if (res != NO_ERR) {
	return 1;
    }

    buf_print_buffer(rpc_get_buff(req));

    rpc_mgr_free_req(req);

    return 0;

}  /* reqpdu_test */


/********************************************************************
*                                                                   *
*			FUNCTION main				    *
*                                                                   *
*********************************************************************/
int main (int argc, char *argv[])
{
    if (nc_base_init()) {
	print_errors();
	return 0;
    }

    if (reqpdu_test()) {
	print_errors();
    } 

#ifdef BLD_MGR
    cap_printf_XML(cap_get_mgr_caps());
#endif

#ifdef BLD_AGENT
    cap_printf_XML(cap_get_agt_caps());
#endif

    nc_base_cleanup();

    return 0;
}  /* main */

/* END basetest.c */



