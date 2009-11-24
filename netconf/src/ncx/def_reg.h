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
#ifndef _H_def_reg
#define _H_def_reg

/*  FILE: def_reg.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

   Definition Registry module

   Provides fast tiered lookup for data structures used to
   process NCX messages.

   The data structures 'pointed to' by these registry entries
   are not managed in this module.  Deleting a registry entry
   will not delete the 'pointed to' data structure.

   Entry types

   NS: 
     Namespace to Module Lookup
     Key: namespace URI
     Data: module name and back pointer

   FD:
     File Desscriptor ID to Session Control Block Ptr
     Key: File Descriptor Index
     Data: Session Ptr attached to that FD

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
14-oct-05    abb      Begun
11-nov-05    abb      re-design to add OWNER as top-level instead of APP
11-feb-06    abb      remove application layer; too complicated
12-jul-07    abb      changed owner-based definitions to
                      module-based definitions throughout all code
                      Change OWNER to MODULE
19-feb-09    abb      Give up on module/mod_def/cfg_def because
                      too complicated once import-by-revision added
                      Too much memory usage as well.
*/

#include <xmlstring.h>

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* initialize the module -- SHOULD call once before use 
 * This will be called inline from other APIs if not done
 */
extern void 
    def_reg_init (void);

/* cleanup the module -- SHOULD call once after use 
 * to free malloced memory
 */
extern void 
    def_reg_cleanup (void);

/*********************** NS ***************************/

/* add one xmlns_t to the registry */
extern status_t 
    def_reg_add_ns (xmlns_t  *ns);

/* find a xmlns_t by its value (name) */
extern xmlns_t * 
    def_reg_find_ns (const xmlChar *nsname);

/* unregister a xmlns_t */
extern void
    def_reg_del_ns (const xmlChar *nsname);

/*********************** SCB ***************************/

/* add one FD to ses_cb_t mapping to the registry */
extern status_t 
    def_reg_add_scb (int fd,
		     ses_cb_t *scb);

/* find a xmlns_t by its value (name) */
extern ses_cb_t * 
    def_reg_find_scb (int fd);

/* unregister a xmlns_t */
extern void
    def_reg_del_scb (int fd);

#endif	    /* _H_def_reg */
