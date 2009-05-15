#ifndef _H_agt_acm
#define _H_agt_acm

/*  FILE: agt_acm.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Agent Access Control handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
03-feb-06    abb      Begun
14-may-09    abb      add per-msg cache to speed up performance
*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
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

/* 1 group that the user is a member */
typedef struct agt_acm_group_t_ {
    dlq_hdr_t         qhdr;
    xmlns_id_t        groupnsid;
    const xmlChar    *groupname;
} agt_acm_group_t;

/* list of group identities that the user is a member */
typedef struct agt_acm_usergroups_t_ {
    dlq_hdr_t         qhdr;
    const xmlChar    *username;
    dlq_hdr_t         groupQ;   /* Q of group_ptr_t */
} agt_acm_usergroups_t;

/* cache for 1 NACM moduleRule entry */
typedef struct agt_acm_modrule_t_ {
    dlq_hdr_t       qhdr;
    xmlns_id_t      nsid;
    val_value_t    *modrule;  /* back-ptr */
} agt_acm_modrule_t;

/* cache for 1 NACM dataRule entry */
typedef struct agt_acm_datarule_t_ {
    dlq_hdr_t           qhdr;
    xpath_result_t     *result;
    val_value_t        *datarule;   /* back-ptr */
} agt_acm_datarule_t;

/* NACM cache control block */
typedef struct agt_acm_cache_t_ {
    agt_acm_usergroups_t *usergroups;
    val_value_t          *nacmroot;     /* back-ptr */
    val_value_t          *rulesval;     /* back-ptr */
    uint32                groupcnt;
    boolean               defset;
    boolean               defpermit;
    boolean               modruleset;
    boolean               dataruleset;
    dlq_hdr_t             modruleQ;     /* Q of agt_acm_modrule_t */
    dlq_hdr_t             dataruleQ;    /* Q of agt_acm_datarule_t */
} agt_acm_cache_t;

    
/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_acm_init (void);

extern status_t 
    agt_acm_init2 (void);

extern void 
    agt_acm_cleanup (void);

extern boolean 
    agt_acm_rpc_allowed (rpc_msg_t *msg,
			 const xmlChar *user,
			 const obj_template_t *rpcobj);

extern boolean 
    agt_acm_val_write_allowed (rpc_msg_t *msg,
			       const xmlChar *user,
			       const val_value_t *val);

extern boolean 
    agt_acm_val_read_allowed (rpc_msg_t *msg,
			      const xmlChar *user,
			      const val_value_t *val);


extern status_t
    agt_acm_init_msg_cache (rpc_msg_t *msg);

extern void
    agt_acm_clear_msg_cache (rpc_msg_t *msg);



#endif	    /* _H_agt_acm */
