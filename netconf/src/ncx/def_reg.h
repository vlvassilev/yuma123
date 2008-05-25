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

   Maintains a tiered registry of module specific
   definition such as TYPEs, PSDs, RPCs and NOTs.

   The data structures 'pointed to' by these registry entries
   are not managed in this module.  Deleting a registry entry
   will not delete the 'pointed to' data structure.

   Note that it is up to the module user application
   to decide the granularity of an onwer.

   In NCX, the owner of the module was originially used
   to separate the naming scopes, but this has been changed
   to separate by module instead.  This aligns with the
   requirements for YANG.
 
   Entry types

   NS: 
     Namespace to Module Lookup
     Key: namespace URI
     Data: module name and back pointer

   FD:
     File Desscriptor ID to Session Control Block Ptr
     Key: File Descriptor Index
     Data: Session Ptr attached to that FD

   MODULE: 
     NCX Module Lookup
     Key: module name
     Data: pointer to ncx_module_t in ncx_modQ
  	
   MODULE:
     Parent registry for all data definitions by one module
     Key: module name
     Data: 
       Child node: DEFINITION (keyed by ncx_node_t enum)
          Child type: NCX_NT_PARMSET
          Child type: NCX_NT_RPC
          Child type: NCX_NT_NOT
          Child type: NCX_NT_APP

   MODULE DEFINITIONS:

   TYPE:
     Data structure descriptor
     Key: Type name     
     Data: back pointer to typ_template_t struct

   PSD:
     Parameter Set Definition
     Key: PSD name
     Data: back pointer to psd_template_t struct

   RPC:
     Remote Procedure Call Method Definition
     Key: RPC method name
     Data: back pointer to rpc_template_t struct

   NOT:
     Notification Definition
     Key: NOT name
     Data: back pointer to not_template_t struct

   APP:
     Application Node Definition
     Key: APP name
     Data: back pointer to ncx_appnode_t struct

 CONFIG: 
     Configuration database parmset instances
     Key: module, application, parmset

     All top-level names must be unique within the entry type
     (e.g., MODULE, NAMESPACE, or MODULE)

  All DEF names for the same module must be unique

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
12-07-07     abb      changed owner-based definitions to
                      module-based definitions throughout all code
                      Change OWNER to MODULE

*/

#include <xmlstring.h>

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

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

/********************** MODULE ***************************/

/* add one ncx_module_t registry */
extern status_t 
    def_reg_add_module (ncx_module_t *mod);

/* find an ncx_module_t by its name */
extern ncx_module_t * 
    def_reg_find_module (const xmlChar *modname);

/* remove a module pointer rec from the registry */
extern void
    def_reg_del_module (const xmlChar *modname);

/****************** MODULE DEFINITIONS ***********************/

/* add a module-specific definition */
extern status_t 
    def_reg_add_moddef (const xmlChar *modname,
			const xmlChar *defname,
			ncx_node_t dtyp,
			void  *dptr);

/* find a module-specific definition */
extern void *
    def_reg_find_moddef (const xmlChar *modname,
			 const xmlChar *defname,
			 ncx_node_t  *dtyp);

/* find a module-specific definition, but try all modules */
extern void *
    def_reg_find_any_moddef (const xmlChar **modname,
			     const xmlChar *defname,
			     ncx_node_t  *dtyp);

/* delete a module-specific definition */
extern void
    def_reg_del_moddef (const xmlChar *modname,
			const xmlChar *defname,
			ncx_node_t dtyp);

/************** CONFIG APPLICATION HDR ****************/

extern status_t 
    def_reg_add_cfgapp (const xmlChar *ownname,
			const xmlChar *appname,
			ncx_cfg_t cfgid,
			void *dptr);


/* find an configuration application header
 * even if the modname is NULL (unknown)
 * returns the cfg_app_t struct that matches
 */
extern void *
    def_reg_find_cfgapp (const xmlChar *modname,
			 const xmlChar *appname,
			 ncx_cfg_t cfgid);


extern void
    def_reg_del_cfgapp (const xmlChar *modname,
			const xmlChar *appname,
			int32 cfgid);


/******************** CONFIG DATA *********************/

/* add an configuration definition */
extern status_t 
    def_reg_add_cfgdef (const xmlChar *ownname,
			const xmlChar *appname,
			const xmlChar *defname,
			const xmlChar *instance,
			ncx_cfg_t cfgid, 
			ncx_node_t dtyp,
			void  *dptr);




/* find an configuration definition */
extern void *
    def_reg_find_cfgdef (const xmlChar *ownname,
			 const xmlChar *appname,
			 const xmlChar *defname,
			const xmlChar *instance,
			 ncx_cfg_t cfgid,
			 ncx_node_t  *dtyp);


/* delete an configuration definition 
 *   cfgid == -1 means delete instance for all configs
 *         >= 0  means delete jsust for that config ID
 *   instance == NULL means delete all instances
 */
extern void
    def_reg_del_cfgdef (const xmlChar *ownname,
			const xmlChar *appname,
			const xmlChar *defname,
			const xmlChar *instance,
			int32 cfgid);


#endif	    /* _H_def_reg */
