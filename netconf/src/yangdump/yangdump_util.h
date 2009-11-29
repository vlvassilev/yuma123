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
#ifndef _H_yangdump_util
#define _H_yangdump_util

/*  FILE: yangdump_util.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  YANGDUMP utilities
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
23-oct-09    abb      Begun

*/

#ifndef _H_ses
#include "ses.h"
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

extern void 
    write_banner (void);

extern void 
    write_banner_session (ses_cb_t *scb);

extern uint32
    find_reference (const xmlChar *buffer,
                    const xmlChar **ref,
                    uint32 *reflen);

#endif	    /* _H_yangdump_util */
