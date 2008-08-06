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

*/

#include <xmlstring.h>

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include "status.h"
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

/* configCapabilities element */
typedef struct agt_acm_config_caps_t_ {
    boolean globalConfig;
    boolean groupConfig;
    boolean rpcAccessConfig;
    boolean rpcTypeAccessConfig;
    boolean databaseAccessConfig;
    boolean notificationAccessConfig;
} agt_acm_config_caps_t;


/* enumeration for different access control modes */
typedef enum agt_acm_control_mode_t_ {
    AGT_ACM_CM_NONE,
    AGT_ACM_CM_OFF,
    AGT_ACM_CM_WARN,
    AGT_ACM_CM_LOOSE,
    AGT_ACM_CM_STRICT
} agt_acm_control_mode_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern status_t 
    agt_acm_init (void);

extern void 
    agt_acm_cleanup (void);

extern boolean 
    agt_acm_rpc_allowed (const xmlChar *user,
			 const obj_template_t *rpcobj);

extern boolean 
    agt_acm_val_write_allowed (const xmlChar *user,
			       const val_value_t *val);

extern boolean 
    agt_acm_val_read_allowed (const xmlChar *user,
			      const val_value_t *val);


#endif	    /* _H_agt_acm */
