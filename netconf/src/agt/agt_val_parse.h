/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_agt_val_parse
#define _H_agt_val_parse

/*  FILE: agt_val_parse.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Parameter Value Parser Module

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
11-feb-06    abb      Begun
06-aug-08    abb      Rewrite for OBJ/VAL only model in YANG
                     Remove app, parmset, and parm from database model
*/

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    agt_val_parse_nc (ses_cb_t  *scb,
		      xml_msg_hdr_t *msg,
		      obj_template_t *obj,
		      const xml_node_t *startnode,
		      ncx_data_class_t  parentdc,
		      val_value_t *retval);


#ifdef DEBUG
extern void
    agt_val_parse_test (const char *testfile);
#endif

#endif	    /* _H_agt_val_parse */
