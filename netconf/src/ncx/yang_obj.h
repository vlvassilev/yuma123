/*
 * Copyright (c) 2009, 2010, Andy Bierman
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_yang_obj
#define _H_yang_obj

/*  FILE: yang_obj.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    YANG Module parser support:

        - data-def-stmt support
          - container-stmt
          - leaf-stmt
          - leaf-list-stmt
          - list-stmt
          - choice-stmt
          - uses-stmt
          - augment-stmt


*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
11-def-07    abb      Begun; start from yang_parse.c

*/

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_grp
#include "grp.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

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


/********************************************************************
* FUNCTION yang_obj_consume_datadef
* 
* Parse the next N tokens as a data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_consume_datadef (yang_pcb_t *pcb,
                              tk_chain_t *tkc,
			      ncx_module_t  *mod,
			      dlq_hdr_t *que,
			      obj_template_t *parent);


/********************************************************************
* FUNCTION yang_obj_consume_datadef_grp
* 
* Parse the next N tokens as a data-def-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the first keyword, starting the specific
* data definition
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*   que == queue will get the obj_template_t 
*   parent == parent object or NULL if top-level data-def-stmt
*   grp == grp_template_t containing 'que'
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_consume_datadef_grp (yang_pcb_t *pcb,
                                  tk_chain_t *tkc,
				  ncx_module_t  *mod,
				  dlq_hdr_t *que,
				  obj_template_t *parent,
				  grp_template_t *grp);


/********************************************************************
* FUNCTION yang_obj_consume_rpc
* 
* Parse the next N tokens as a rpc-stmt
* Create a obj_template_t struct and add it to the specified module
*
* First pass of a 3 pass compiler
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'rpc' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new RPC added to module
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_consume_rpc (yang_pcb_t *pcb,
                          tk_chain_t *tkc,
			  ncx_module_t  *mod);


/********************************************************************
* FUNCTION yang_obj_consume_notification
* 
* Parse the next N tokens as a notification-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'notification' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new notification added to module
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_consume_notification (yang_pcb_t *pcb,
                                   tk_chain_t *tkc,
				   ncx_module_t  *mod);

/********************************************************************
* FUNCTION yang_obj_consume_augment
* 
* Parse the next N tokens as a top-level augment-stmt
* Create a obj_template_t struct and add it to the specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'augment' keyword
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new augment object added to module 
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_consume_augment (yang_pcb_t *pcb,
                              tk_chain_t *tkc,
			      ncx_module_t  *mod);


/********************************************************************
* FUNCTION yang_obj_consume_deviation
* 
* Parse the next N tokens as a top-level deviation-stmt
* Create a obj_deviation_t struct and add it to the 
* specified module
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* Current token is the 'deviation' keyword
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain
*   mod == module in progress
*
* OUTPUTS:
*   new deviation struct added to module deviationQ
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_consume_deviation (yang_pcb_t *pcb,
                                tk_chain_t *tkc,
				ncx_module_t  *mod);


/********************************************************************
* FUNCTION yang_obj_resolve_datadefs
* 
* First pass object validation
*
* Analyze the entire datadefQ within the module struct
* Finish all the clauses within this struct that
* may have been defered because of possible forward references
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_resolve_datadefs (yang_pcb_t *pcb,
                               tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *datadefQ);


/********************************************************************
* FUNCTION yang_obj_resolve_uses
* 
* Second pass object validation
*
* Expand and validate any uses clauses within any objects
* within the datadefQ
*
* Search through the entire datadefQ tree(s) and call
* the expand_uses() function
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_resolve_uses (yang_pcb_t *pcb,
                           tk_chain_t *tkc,
			   ncx_module_t  *mod,
			   dlq_hdr_t *datadefQ);


/********************************************************************
* FUNCTION yang_obj_resolve_augments
* 
*
* Third pass object validation
*
* Expand and validate any augment clauses within any objects
* within the datadefQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_resolve_augments (yang_pcb_t *pcb,
                               tk_chain_t *tkc,
			       ncx_module_t  *mod,
			       dlq_hdr_t *datadefQ);


/********************************************************************
* FUNCTION yang_obj_resolve_deviations
* 
*
* Validate any deviation statements within the 
* module deviationQ
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_resolve_deviations (yang_pcb_t *pcb,
				 tk_chain_t *tkc,
				 ncx_module_t  *mod);


/********************************************************************
* FUNCTION yang_obj_resolve_final
* 
* Fourth pass object validation
*
* Check various final stage errors and warnings
* within a single file
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   pcb == parser control block
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*   ingrouping == TRUE if this object being resolved
*                 from yang_grp_resolve_final
*              == FALSE otherwise
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_resolve_final (yang_pcb_t *pcb,
                            tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    dlq_hdr_t *datadefQ,
                            boolean ingrouping);


/********************************************************************
* FUNCTION yang_obj_resolve_xpath
* 
* Fifth (and final) pass object validation
*
* Check all leafref, must, and when XPath expressions
* to make sure they are well-formed
*
* Checks the cooked objects, and skips all groupings
* uses, and augment nodes
*
* MUST BE CALLED AFTER yang_obj_resolve_final
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_resolve_xpath (tk_chain_t *tkc,
			    ncx_module_t  *mod,
			    dlq_hdr_t *datadefQ);


/********************************************************************
* FUNCTION yang_obj_check_leafref_loops
* 
* Check all leafref objects for hard-wired object loops
* Must be done after yang_obj_resolve_xpath
*
* Error messages are printed by this function!!
* Do not duplicate error messages upon error return
*
* INPUTS:
*   tkc == token chain from parsing (needed for error msgs)
*   mod == module in progress
*   datadefQ == Q of obj_template_t structs to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t 
    yang_obj_check_leafref_loops (tk_chain_t *tkc,
				  ncx_module_t  *mod,
				  dlq_hdr_t *datadefQ);


/********************************************************************
* FUNCTION yang_obj_remove_deleted_nodes
* 
* Find any nodes marked for deletion and remove them
*
* INPUTS:
*   pcb == parser control block to use
*   tkc == token parse chain to use for errors
*   mod == module in progress; transfer to this deviationQ
*   datadefQ == current object datadef Q to check
*
* RETURNS:
*   status of the operation
*********************************************************************/
extern status_t
    yang_obj_remove_deleted_nodes (yang_pcb_t *pcb,
                                   tk_chain_t *tkc,
                                   ncx_module_t *mod,
                                   dlq_hdr_t *datadefQ);

#ifdef __cplusplus
}  /* end extern 'C' */
#endif


#endif	    /* _H_yang_obj */
