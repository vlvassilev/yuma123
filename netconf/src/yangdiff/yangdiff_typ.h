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
#ifndef _H_yangdiff_typ
#define _H_yangdiff_typ

/*  FILE: yangdiff_typ.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Report differences related to YANG typedefs and types
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
10-jul-08    abb      Split out from yangdiff.c

*/

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_yangdiff
#include "yangdiff.h"
#endif


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern uint32
    type_changed (yangdiff_diffparms_t *cp,
		  typ_def_t *oldtypdef,
		  typ_def_t *newtypdef);

extern uint32
    typedef_changed (yangdiff_diffparms_t *cp,
		     typ_template_t *oldtyp,
		     typ_template_t *newtyp);

extern uint32
    typedefQ_changed (yangdiff_diffparms_t *cp,
		      dlq_hdr_t *oldQ,
		      dlq_hdr_t *newQ);

extern void
    output_one_type_diff (yangdiff_diffparms_t *cp,
			  typ_def_t *oldtypdef,
			  typ_def_t *newtypdef);


extern void
    output_typedefQ_diff (yangdiff_diffparms_t *cp,
			  dlq_hdr_t *oldQ,
			  dlq_hdr_t *newQ);

#endif	    /* _H_yangdiff_typ */
