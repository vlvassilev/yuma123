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
#ifndef _H_yin
#define _H_yin

/*  FILE: yin.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Convert YANG module to YIN format
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
06-dec-09    abb      Begun; split out from yangdump dir

*/

#include <xmlstring.h>


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

typedef struct yin_mapping_t_ {
    const xmlChar *keyword;
    const xmlChar *argname;       /* may be NULL */
    boolean        elem;
} yin_mapping_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern const yin_mapping_t *
    yin_find_mapping (const xmlChar *name);

#endif	    /* _H_yin */
