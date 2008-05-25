#ifndef _H_mon
#define _H_mon

/*  FILE: mon.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    Monitor set definition module

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
14-oct-05    abb      Begun

*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* default Monitor Type is 'top' */
#define MON_DEF_MONTYPE        ((const xmlChar *)"top")

#define MON_DEF_MONTYPE_ENUM   MON_TYP_TOP

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* enum mapping of object node type field */
typedef enum mon_nodetyp_t_ {
    MON_NT_NONE,
    MON_NT_CHOICE,
    MON_NT_BLOCK,
    MON_NT_OBJ
} mon_nodetyp_t;


/* enum mapping of data model monitor type */
typedef enum mon_type_t_ {
    MON_TYP_NONE,
    MON_TYP_TOP,
    MON_TYP_INTERFACE
} mon_type_t;


/* enum mapping of object user object type field */
typedef enum mon_objtyp_t_ {
    MON_OT_NONE,
    MON_OT_TYPE,
    MON_OT_MONITOR
} mon_objtyp_t;


/* struct for a choice header within an objects section */
typedef struct mon_choice_t_ {
    /* qhdr and ntyp fields must be first */
    dlq_hdrT       qhdr;      
    mon_nodetyp_t  ntyp;
    uint32         choice_id;
    uint32         blockcnt;
    dlq_hdrT       choiceQ;
} mon_choice_t;


/* struct for an object block within a choice section */
typedef struct mon_block_t_ {
    /* qhdr and ntyp fields must be first */
    dlq_hdrT       qhdr;      
    mon_nodetyp_t  ntyp;
    uint32         blockid;
    dlq_hdrT       blockQ;
} mon_block_t;


/* struct for a single <object> subtree */
typedef struct mon_obj_t_ {
    /* qhdr and ntyp fields must be first */
    dlq_hdrT       qhdr;
    mon_nodetyp_t  ntyp;
    xmlChar       *name;
    xmlChar       *typname;
    xmlChar       *modstr;
    xmlChar       *descr;
    xmlChar       *condition;
    uint32         flags;
    uint32         objid;
    uint32         blockid;
    uint32         choiceid;
    mon_objtyp_t   otyp;
    void          *odef;
} mon_obj_t;


/* struct for a header template */
typedef struct mon_hdronly_t_ {
    dlq_hdrT       qhdr;
    mon_nodetyp_t  ntyp;
} mon_hdronly_t;

    
/* struct to match a single <monitor> subtree */
typedef struct mon_template_t_ {
    dlq_hdrT       qhdr;   /* must be first -- used for hash table */
    xmlChar       *name;
    xmlChar       *descr;
    xmlChar       *condition;
    xmlChar       *montype;
    mon_type_t     mon_type;
    dlq_hdrT       objQ;
    uint32         objcnt;
    uint32         choicecnt;
    uint32         flags;
} mon_template_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern mon_template_t * 
    mon_new_template (void);

extern mon_choice_t *
    mon_new_choice (void);

extern mon_block_t * 
    mon_new_block (void);

extern mon_obj_t * 
    mon_new_obj (void);

extern void 
    mon_free_template (mon_template_t *mon);

extern void 
    mon_free_choice (mon_choice_t *mch);

extern void 
    mon_free_block (mon_block_t *mb);

extern void 
    mon_free_obj (mon_obj_t *obj);

extern mon_obj_t * 
    mon_find_obj (mon_template_t *mon,
		  const xmlChar *name);

extern mon_obj_t * 
    mon_search_block (mon_block_t *mb,
		      const xmlChar *name);

extern mon_obj_t * 
    mon_search_choice (mon_choice_t *mch,
		       const xmlChar *name);


/* see def_reg.h for the find-an-obj-template function */

extern status_t
    mon_locate_template (ncx_module_t  *mod,
			 const xmlChar *modstr,
			 const xmlChar *monname,
			 mon_template_t  **pptr);


#endif	    /* _H_mon */
