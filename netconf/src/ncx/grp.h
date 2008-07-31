#ifndef _H_grp
#define _H_grp

/*  FILE: grp.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Grouping Handler

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
09-dec-07    abb      Begun

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


/* One YANG 'grouping' definition -- sibling set template */
typedef struct grp_template_t_ {
    dlq_hdr_t        qhdr;
    xmlChar         *name;
    xmlChar         *descr;
    xmlChar         *ref;
    ncx_module_t    *mod;        /* const back-ptr to this module */
    tk_token_t      *tk;         /* const back-ptr to start token */
    void            *parent;      /* const back-ptr to parent obj */
    struct grp_template_t_ *parentgrp;    /* direct parent is grp */
    xmlns_id_t       nsid;
    boolean          used;
    boolean          istop;
    ncx_status_t     status;
    uint32           linenum;                /* use after parsing */
    uint32           grpindex;         /* used for XSD generation */
    dlq_hdr_t        typedefQ;             /* Q of typ_template_t */
    dlq_hdr_t        groupingQ;            /* Q of grp_template_t */
    dlq_hdr_t        datadefQ;             /* Q of obj_template_t */
    dlq_hdr_t        appinfoQ;              /* Q of ncx_appinfo_t */

} grp_template_t;



/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/****************** ALLOCATION FUNCTIONS **********************/

/* malloc and init a YANG grouping template */
extern grp_template_t *
    grp_new_template (void);

/* free a YANG grouping template */
extern void 
    grp_free_template (grp_template_t *grp);

/* clean Q of grp_template_t */
extern void
    grp_clean_groupingQ (dlq_hdr_t *que);

extern boolean
    grp_has_typedefs (const grp_template_t *grp);

extern const xmlChar *
    grp_get_mod_name (const grp_template_t *grp);

#endif	    /* _H_grp */
