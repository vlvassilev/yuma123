#ifndef _H_ext
#define _H_ext

/*  FILE: ext.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Extension Handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
05-jan-08    abb      Begun

*/

#include <xmlstring.h>
#include <xmlregexp.h>

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_tk
#include "tk.h"
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

/* One YANG 'extension' definition -- language extension template */
typedef struct ext_template_t_ {
    dlq_hdr_t        qhdr;
    xmlChar         *name;
    xmlChar         *descr;
    xmlChar         *ref;
    xmlChar         *arg;
    ncx_module_t    *mod;        /* const back-ptr to this module */
    tk_token_t      *tk;         /* const back-ptr to start token */
    uint32           linenum;
    xmlns_id_t       nsid;
    ncx_status_t     status;
    boolean          argel;
    boolean          used;                  /* needed by yangdiff */
    dlq_hdr_t        appinfoQ;              /* Q of ncx_appinfo_t */
} ext_template_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/****************** ALLOCATION FUNCTIONS **********************/

/* malloc and init a YANG extension template */
extern ext_template_t *
    ext_new_template (void);

/* free a YANG extension template */
extern void 
    ext_free_template (ext_template_t *ext);

/* clean Q of ext_template_t */
extern void
    ext_clean_extensionQ (dlq_hdr_t *que);

extern ext_template_t *
    ext_find_extension (dlq_hdr_t *que,
			const xmlChar *name);

#endif	    /* _H_ext */
