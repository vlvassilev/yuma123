#ifndef _H_psd
#define _H_psd

/*  FILE: psd.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Parameter Set Definition (PSD) module

  A parameter can be:
    - a simple parameter (psd_parm_t)
    - a choice of multiple options (psd_choice_t)
       - simple parameter
       - block of simple parameters (psd_block_t)

   A simple parameter can be:
    - any NCX data type 
    - any nested PSD

   Any parameter or nested simple or somplex type can have
   an instance qualifier for simple control of multiple instances
    - NCX_IQUAL_NONE == value not set
    - NCX_IQUAL_ONE == exactly 1 instance expected
    - NCX_IQUAL_OPT == zero or 1 instance expected
    - NCX_IQUAL_ZMORE == zero or more instances expected
    - NCX_IQUAL_1MORE == 1 or more instances expected
    
     
    +-----------+
    |    PSD    |     +---------+         +----------+
    |           |     |         |         |          |
    |   parmQ ------> |  Parm   | ------->|  Choice  |---->
    |           |     |         |         |          |
    +-----------+     +---------+         +----------+
                        |     |              |     |
                        |     |              |     |
                        V     V              V     V
                   +------+ +------+    +------+ +-------+
                   |      | |      |    |      | |       |
                   | Type | |  PSD |    | Parm | | Block |
                   |      | |      |    |      | |       |
                   +------+ +------+    +------+ +-------+
                                            |       |
                                            |       |
                                            V       V
                                                +------+  +------+
                                                |      |  |      |
                                                | Parm |->| Parm |-->
                                                |      |  |      |
                                                +------+  +------+
                                                   |         
                                                   |
                                                   V


  Parameters are numbered from 1 to N


*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
14-oct-05    abb      Begun

*/

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* default Parmset Type is 'top' */
#define PSD_DEF_PSTYPE       ((const xmlChar *)"data")

#define PSD_DEF_PSTYPE_ENUM  PSD_TYP_DATA

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

typedef uint32 psd_parmid_t;              /* 0 .. N-1 numbering */

typedef uint32 psd_choiceid_t;            /* 1 .. N numbering */

typedef uint32 psd_blockid_t;             /* 1 .. N numbering */

/* enum mapping of parm.usage field */
typedef enum psd_usage_t_ {
    PSD_UT_NONE,
    PSD_UT_MANDATORY,
    PSD_UT_OPTIONAL,
    PSD_UT_CONDITIONAL
} psd_usage_t;

/* enum mapping of parm.order field */
typedef enum psd_order_t_ {
    PSD_OT_NONE,
    PSD_OT_LOOSE,
    PSD_OT_STRICT
} psd_order_t;


/* enum mapping of parm node type field */
typedef enum psd_nodetyp_t_ {
    PSD_NT_NONE,
    PSD_NT_CHOICE,
    PSD_NT_BLOCK,
    PSD_NT_PARM
} psd_nodetyp_t;


/* enum mapping of data model parm type field */
typedef enum psd_pstype_t_ {
    PSD_TYP_NONE,
    PSD_TYP_DATA,
    PSD_TYP_RPC,
    PSD_TYP_CLI
} psd_pstype_t;


/* struct for a choice header within a parms section */
typedef struct psd_choice_t_ {
    /* qhdr and ntyp fields must be first */
    dlq_hdr_t      qhdr;      
    psd_nodetyp_t  ntyp;
    dlq_hdr_t      choiceQ;   /* Q of psd_block_t or psd_parm_t */
    psd_choiceid_t choice_id;
    uint32         blockcnt;
    psd_parmid_t   start_parm;
    psd_parmid_t   end_parm;
} psd_choice_t;


/* struct for a parm block within a choice section */
typedef struct psd_block_t_ {
    /* qhdr and ntyp fields must be first */
    dlq_hdr_t      qhdr;      
    psd_nodetyp_t  ntyp;
    dlq_hdr_t      blockQ;                   /* Q of psd_parm_t */
    psd_choiceid_t choice_id;
    psd_blockid_t  block_id;
    psd_parmid_t   start_parm;
    psd_parmid_t   end_parm;
} psd_block_t;


/* struct for a single <parm> subtree */
typedef struct psd_parm_t_ {
    /* qhdr and ntyp fields must be first */
    dlq_hdr_t        qhdr;
    psd_nodetyp_t    ntyp;
    ncx_data_class_t dataclass;
    xmlChar         *name;
    xmlChar         *modstr;
    xmlChar         *typname;
    psd_usage_t      usage;
    ncx_access_t     access;
    xmlChar         *descr;
    xmlChar         *condition;
    psd_choiceid_t   choice_id;
    psd_blockid_t    block_id;
    psd_parmid_t     parm_id;                 /* 0 .. N-1 numbering */
    dlq_hdr_t        appinfoQ;    
    void            *pdef;             /* backptr to typ_template_t */
    void            *cbset;       /* cached agt_ps_pcbset_t pointer */
    struct psd_template_t_ *parent;
} psd_parm_t;


/* struct for a header template */
typedef struct psd_hdronly_t_ {
    dlq_hdr_t      qhdr;
    psd_nodetyp_t  ntyp;
} psd_hdronly_t;

    
/* struct to match a single NCX 'parmset' definition */
typedef struct psd_template_t_ {
    dlq_hdr_t      qhdr;
    xmlChar       *name;
    xmlChar       *descr;
    xmlChar       *condition;
    xmlChar       *pstype;
    psd_pstype_t   psd_type;
    xmlns_id_t     nsid;
    const xmlChar *module;
    const xmlChar *app;
    dlq_hdr_t      parmQ;
    uint32         choicecnt;
    uint32         parmcnt;
    uint32         linenum;
    psd_order_t    order;
    ncx_data_class_t dataclass;     /* NCX_DC_CONFIG if any such parms */
    void          *cbset;           /* agt_ps_cbset_t pointer */
    dlq_hdr_t      appinfoQ;
} psd_template_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern psd_template_t *
    psd_new_template (void);

extern psd_choice_t *
    psd_new_choice (void);

extern psd_block_t * 
    psd_new_block (void);

extern psd_parm_t *
    psd_new_parm (void);

extern void 
    psd_free_template (psd_template_t *psd);

extern void 
    psd_free_choice (psd_choice_t *pch);

extern void 
    psd_free_block (psd_block_t *pb);

extern void 
    psd_free_parm (psd_parm_t *parm);

extern psd_parm_t * 
    psd_find_parm (const psd_template_t *psd,
		   const xmlChar *name);

extern const psd_parm_t * 
    psd_first_parm (const psd_template_t *psd);

extern const psd_parm_t * 
    psd_next_parm (const psd_parm_t *parm);

extern psd_parm_t * 
    psd_find_parm_str (const psd_template_t *psd,
		       const xmlChar *name,
		       uint32 namelen);

extern psd_parm_t * 
    psd_match_parm_str (const psd_template_t *psd,
			const xmlChar *name,
			uint32 namelen);

extern psd_parm_t * 
    psd_find_parmnum (const psd_template_t *psd,
		      psd_parmid_t parmnum);

extern const psd_block_t * 
    psd_find_blocknum (const psd_template_t *psd,
		       psd_choiceid_t choicenum,
		       psd_blockid_t  blocknum);

extern psd_parm_t * 
    psd_search_block (const psd_block_t *pb,
		      const xmlChar *name);

extern psd_parm_t * 
    psd_search_block_str (const psd_block_t *pb,
			  const xmlChar *name,
			  uint32 namelen);

extern psd_parm_t * 
    psd_match_block_str (const psd_block_t *pb,
			 const xmlChar *name,
			 uint32 namelen);

extern psd_parm_t * 
    psd_search_blocknum (const psd_block_t *pb,
			 psd_parmid_t parmnum);

extern psd_parm_t * 
    psd_search_choice (const psd_choice_t *pch,
		       const xmlChar *name);

extern psd_parm_t * 
    psd_search_choice_str (const psd_choice_t *pch,
			   const xmlChar *name,
			   uint32 namelen);

extern psd_parm_t * 
    psd_match_choice_str (const psd_choice_t *pch,
			  const xmlChar *name,
			  uint32 namelen);

extern psd_parm_t * 
    psd_search_choicenum (const psd_choice_t *pch,
			  psd_parmid_t parmnum);

/* see def_reg.h for the find-a-psd-template function */

extern const xmlChar *
    psd_get_usage_str (psd_usage_t usage);

extern psd_usage_t 
    psd_get_usage_enum (const xmlChar *str);

extern psd_order_t 
    psd_get_order_enum (const xmlChar *str);

extern status_t
    psd_locate_template (const ncx_module_t  *mod,
			 const xmlChar *modstr,
			 const xmlChar *psdname,
			 psd_template_t  **pptr);

extern boolean
    psd_parm_writable (const psd_parm_t *parm);

extern boolean
    psd_parm_required (const psd_parm_t *parm);

extern boolean
    psd_block_required (const psd_block_t *block);

extern boolean
    psd_choice_required (const psd_choice_t *choice);

extern const psd_parm_t *
    psd_first_choice_parm (const psd_template_t *psd,
			   const psd_choice_t *psd_choice);

extern ncx_btype_t
    psd_parm_basetype (const psd_parm_t *parm);

extern boolean
    psd_condition_met (const xmlChar *condstr);

extern uint32
    psd_parm_count (const psd_template_t *psd);

extern void
    psd_dump_parms (const psd_template_t *psd,
		    boolean full,
		    uint32 indent);

extern typ_def_t *
    psd_get_typdef (const psd_parm_t *parm);

extern xmlns_id_t
    psd_get_parm_nsid (const psd_parm_t *parm);

extern xmlns_id_t
    psd_get_parmset_nsid (const psd_template_t *psd);

extern typ_template_t *
    psd_get_parm_template (const psd_parm_t *parm);

#endif	    /* _H_psd */
