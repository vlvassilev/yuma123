#ifndef _H_agt_xml_wr
#define _H_agt_xml_wr

/*  FILE: agt_xml_wr.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Write a val_value_t to a FILE in XML format 

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
15-nov-06    abb      Begun

*/

#include <stdio.h>

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/


/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    agt_xml_wr_value (FILE *fp, 
		      val_value_t *val,
		      xml_attrs_t *attrs,
		      boolean  docmode,
		      boolean  xmlhdr);

extern status_t
    agt_xml_wr_check_value (FILE *fp, 
			    val_value_t *val,
			    xml_attrs_t *attrs,
			    boolean docmode,
			    boolean xmlhdr,
			    ncx_nodetest_fn_t testfn);

#endif	    /* _H_agt_xml_wr */
