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
#ifndef _H_xpath_yang
#define _H_xpath_yang

/*  FILE: xpath_yang.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    YANG-specific Xpath support

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
13-nov-08    abb      Begun

*/

#include <xmlstring.h>
#include <xmlreader.h>
#include <xmlregexp.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_val
#include "val.h"
#endif


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t
    xpath_yang_parse_path (tk_chain_t *tkc,
			   ncx_module_t *mod,
			   xpath_source_t source,
			   xpath_pcb_t *pcb);

extern status_t
    xpath_yang_validate_path (ncx_module_t *mod,
			      obj_template_t *obj,
			      xpath_pcb_t *pcb,
			      boolean schemainst,
			      obj_template_t **leafobj);

extern status_t
    xpath_yang_validate_xmlpath (xmlTextReaderPtr reader,
				 xpath_pcb_t *pcb,
				 obj_template_t *pathobj,
				 boolean logerrors,
				 obj_template_t **targobj);


extern status_t
    xpath_yang_validate_xmlkey (xmlTextReaderPtr reader,
				xpath_pcb_t *pcb,
				obj_template_t *obj,
				boolean logerrors);


extern val_value_t *
    xpath_yang_make_instanceid_val (xpath_pcb_t *pcb,
				    status_t *retres,
				    val_value_t **deepest);

extern status_t
    xpath_yang_get_namespaces (const xpath_pcb_t *pcb,
			       xmlns_id_t *nsid_array,
			       uint32 max_nsids,
			       uint32 *num_nsids);

#endif	    /* _H_xpath_yang */
