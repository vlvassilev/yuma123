/*  FILE: agt_xml_wr.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
15nov06      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>
#include  <string.h>
#include  <ctype.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt_rpc
#include "agt_rpc.h"
#endif

#ifndef _H_agt_ses
#include "agt_ses.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_agt_val_wr
#include "agt_val_wr.h"
#endif

#ifndef _H_agt_xml_wr
#include "agt_xml_wr.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_val
#include "val.h"
#endif



/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define AGT_XML_WR_DEBUG  1 */

/********************************************************************
* FUNCTION agt_xml_wr_check_value_xml
* 
* Write the specified value to the FILE in XML format
*
* INPUTS:
*    fp == FILE for output
*    val == value for output
*    attrs == top-level attributes to generate
*    docmode == TRUE if XML_DOC output mode should be used
*            == FALSE if XML output mode should be used
*    xmlhdr == TRUE if <?xml?> directive should be output
*            == FALSE if not
*    testfn == callback test function to use
*
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_xml_wr_check_value (FILE *fp, 
			    val_value_t *val,
			    xml_attrs_t *attrs,
			    boolean docmode,
			    boolean xmlhdr,
			    ncx_nodetest_fn_t testfn)
{
    ses_cb_t   *scb;
    rpc_msg_t  *msg;
    status_t    res;
    ses_mode_t  sesmode;
    boolean     anyout;

    res = NO_ERR;
    msg = NULL;
    anyout = FALSE;

    /* get a dummy session control block */
    scb = agt_ses_new_dummy_session();
    if (!scb) {
	res = ERR_INTERNAL_MEM;
    } else {
	scb->fp = fp;
    }

    /* get a dummy output message */
    if (res == NO_ERR) {
	msg = rpc_new_out_msg();
	if (!msg) {
	    res = ERR_INTERNAL_MEM;
	} else {
	    /* hack -- need a queue because there is no top
	     * element which this usually shadows
	     */
	    msg->rpc_in_attrs = attrs;
	}
    }
    
    /* send the XML declaration */
    if (res == NO_ERR && xmlhdr) {
	res = ses_start_msg(scb);
    }

    /* setup an empty prefix map */
    if (res == NO_ERR) {
	anyout = TRUE;
	res = xml_msg_build_prefix_map(&msg->mhdr,
				       msg->rpc_in_attrs, FALSE);
    }

    if (res == NO_ERR && docmode) {
	sesmode = ses_get_mode(scb);
	ses_set_mode(scb, SES_MODE_XMLDOC);
    }

    /* generate the <foo> start tag */
    if (res == NO_ERR) {
	agt_begin_elem(scb, &msg->mhdr, val->nsid, val->name, 
		       attrs, TRUE, 0, FALSE, &msg->mhdr.prefixQ);
    }

    /* output the value */
    if (res == NO_ERR) {
	agt_val_check_write_nc(scb, &msg->mhdr, val, NCX_DEF_INDENT, 
			       FALSE, testfn);
    }

    /* generate the <foo> end tag */
    if (res == NO_ERR) {
	agt_end_elem(scb, &msg->mhdr, val->nsid, val->name, 0);
    }

    /* finish the message, should be NO-OP  */
    if (anyout) {
	ses_finish_msg(scb);
    }

    if (res == NO_ERR && docmode) {
	ses_set_mode(scb, sesmode);
    }

    /* clean up and exit */
    if (msg) {
	rpc_free_msg(msg);
    }
    if (scb) {
	agt_ses_free_dummy_session(scb);
    }

    return res;

} /* agt_xml_wr_check_value */


/********************************************************************
* FUNCTION agt_xml_wr_value
* 
* Write the specified value to the FILE in XML format
*
* INPUTS:
*    fp == FILE for output
*    val == value for output
*    attrs == top-level attributes to generate
*    docmode == TRUE if XML_DOC output mode should be used
*            == FALSE if XML output mode should be used
*    xmlhdr == TRUE if <?xml?> directive should be output
*            == FALSE if not
*
* RETURNS:
*    status
*********************************************************************/
status_t
    agt_xml_wr_value (FILE *fp, 
		      val_value_t *val,
		      xml_attrs_t *attrs,
		      boolean docmode,
		      boolean xmlhdr)
{
    return agt_xml_wr_check_value(fp, val, attrs, 
				  docmode, xmlhdr, NULL);

} /* agt_xml_wr_value */


/* END file agt_xml_wr.c */
