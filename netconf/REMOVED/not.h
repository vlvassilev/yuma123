#ifndef _H_not
#define _H_not

/*  FILE: not.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    NCX Core Notification definitions

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
08-nov-05    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
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
    
/* struct to match a single <notif> subtree */
typedef struct not_template_t_ {
    dlq_hdr_t     qhdr;   /* must be first -- used for hash table */
    xmlChar      *name;
    xmlChar      *not_class;
    xmlChar      *not_type;
    xmlChar      *descr;
    xmlChar      *condition;
    xmlns_id_t    nsid;
    dlq_hdr_t     not_dataQ;
    dlq_hdr_t     appinfoQ;
} not_template_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern not_template_t * not_new_template (void);

extern void not_free_template (not_template_t *not);

/* see def_reg.h for the find-a-not-template function */

#endif	    /* _H_not */
