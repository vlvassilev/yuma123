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
#ifndef _H_yangdiff_grp
#define _H_yangdiff_grp

/*  FILE: yangdiff_grp.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Report differences related to YANG groupings
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
10-jul-08    abb      Split out from yangdiff.c

*/

#ifndef _H_yangdiff
#include "yangdiff.h"
#endif


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern uint32
    groupingQ_changed (yangdiff_diffparms_t *cp,
		       dlq_hdr_t *oldQ,
		       dlq_hdr_t *newQ);

extern void
    output_groupingQ_diff (yangdiff_diffparms_t *cp,
			   dlq_hdr_t *oldQ,
			   dlq_hdr_t *newQ);

#endif	    /* _H_yangdiff_grp */
