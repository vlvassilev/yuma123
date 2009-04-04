#ifndef _H_ps
#define _H_ps

/*  FILE: ps.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    parameter set module

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

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_op
#include "op.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

/********************************************************************
*								    *
*			 C O N S T A N T S			    *
*								    *
*********************************************************************/

/* bit definitions for the ps_parmset_t flags field */
#define PS_FL_ERRORS        bit0

#define PS_FL_WARNINGS      bit1

#define PS_FL_VIRTUAL       bit2

/* set the parmset-has-errors flag */
#define SET_PS_ERROR(PS) ((PS)->flags |= PS_FL_ERRORS)

/* check if the parmset has any errors set */
#define PS_ERRORS(PS) ((PS)->flags | PS_FL_ERRORS)

/* set the parmset-has-warning flag */
#define SET_PS_WARNING(PS) ((PS)->flags |= PS_FL_WARNINGS)

/* check if the parmset has any errors set */
#define PS_WARNINGS(PS) ((PS)->flags | PS_FL_WARNINGS)

/* bit definitions for the pflags field in ps_parmset_t */
#define PS_FL_SET         bit0
#define PS_FL_MSET        bit1
#define PS_FL_ERR         bit2

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* nodes in the parmQ can be simple parameters or nested parmsets */
typedef enum ps_nodetyp_t_ {
    PS_NT_NONE,
    PS_NT_PARM,
    PS_NT_PARMSET
} ps_nodetyp_t;


/* One parameter instance */
typedef struct ps_parm_t_ {
    dlq_hdr_t         qhdr;
    ps_nodetyp_t      nodetyp;                         /* == PS_NT_PARM */
    const psd_parm_t *parm;
    op_editop_t       editop;            /* for cfg_load and edit-config */
    uint32            seqid;             /* instance ID for unnamed dups */
    status_t          res;           /* saved status result for the parm */
    val_value_t      *val;
    struct ps_parmset_t_ *parent;      /* back-ptr to create instance ID */
} ps_parm_t;


/* One parameter set instance */
typedef struct ps_parmset_t_ {
    dlq_hdr_t               qhdr;
    ncx_node_t              ntyp;                 /* == NCX_NT_PARMSET */
    ps_nodetyp_t            nodetyp;               /* == PS_NT_PARMSET */
    xmlChar                *instance;   /* instance ID of this parmset */
    void                   *parent;    /* ptr to cfg_app_t for this ps */
    const psd_template_t   *psd;
    xmlChar                *name;
    psd_pstype_t            psd_type;     /* parmset usage, RPC or TOP */
    uint8                  *pflags;          /* 0 .. N-1 array of byte */
    dlq_hdr_t               parmQ;
    uint32                  flags;
    uint32                  curparm;
    op_editop_t             editop;
    status_t                res;
    xmlChar                *lastchange;
} ps_parmset_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern ps_parmset_t * 
    ps_new_parmset (void);

extern void 
    ps_init_parmset (ps_parmset_t *ps);

extern void 
    ps_free_parmset (ps_parmset_t *ps);

extern void 
    ps_clean_parmset (ps_parmset_t *ps);

extern ps_parm_t * 
    ps_new_parm (void);

extern void 
    ps_init_parm (ps_parm_t *parm);

extern void 
    ps_free_parm (ps_parm_t *parm);

extern void 
    ps_clean_parm (ps_parm_t *parm);

/* find first instance of 'name' in 'ps' */
extern ps_parm_t * 
    ps_find_parm (ps_parmset_t *ps,
		 const xmlChar *name);

/* find next instance of 'name' in 'ps' */
extern ps_parm_t * 
    ps_next_parm_instance (ps_parm_t *curparm);

/* return number of instances of 'name' in ps */
extern uint32
    ps_parm_count (ps_parmset_t *ps,
		   const xmlChar *name);

extern ps_parm_t * 
    ps_match_parm (ps_parmset_t *ps,
		   const xmlChar *namestr);

extern ps_parm_t * 
    ps_find_parmnum (ps_parmset_t *ps,
		     uint32 id);

extern boolean
    ps_parmnum_set (const ps_parmset_t *ps,
		    uint32 id);

extern boolean
    ps_parmnum_mset (const ps_parmset_t *ps,
		     uint32 id);

extern boolean
    ps_parmnum_seterr (const ps_parmset_t *ps,
		       uint32 id);

extern ps_parm_t * 
    ps_find_parmcopy (ps_parmset_t *ps,
		      const ps_parm_t *parm);

extern void
    ps_dump_parmset (const ps_parmset_t *ps,
		     int32 startindent);

extern void
    ps_stdout_parmset (const ps_parmset_t *ps,
		       int32 startindent);

extern status_t
    ps_get_parmval (ps_parmset_t *ps,
		    const xmlChar *name,
		    val_value_t **retval);

extern ps_parm_t *
    ps_clone_parm (const ps_parm_t *parm);


extern boolean
    ps_merge_parm (ps_parm_t *src,
		   ps_parm_t *dest);

extern boolean
    ps_merge_parmset (ps_parmset_t *src,
		      ps_parmset_t *dest);


extern void 
    ps_add_parm (ps_parm_t *parm,
		 ps_parmset_t *ps,
		 ncx_merge_t  mergetyp);

extern void 
    ps_add_parm_last (ps_parm_t *parm,
		      ps_parmset_t *ps);

extern void
    ps_set_instseq (ps_parmset_t *ps);

extern void
    ps_setup_parm (ps_parm_t  *ps_parm,
		   ps_parmset_t *ps,
		   const psd_parm_t *psd_parm);

extern status_t
    ps_setup_parmset (ps_parmset_t *ps,
		      const psd_template_t *psd,
		      psd_pstype_t psdtyp);

extern boolean
    ps_check_block_set (const ps_parmset_t *ps,
			const psd_block_t  *block);

extern ps_parm_t *
    ps_choice_first_set (ps_parmset_t *ps,
			 const psd_choice_t *pch);

extern boolean
    ps_check_choice_set (const ps_parmset_t *ps,
			 psd_choiceid_t  id);

extern void
    ps_mark_pflag_set (ps_parmset_t *ps,
		       uint32 id);

extern void
    ps_mark_pflag_err (ps_parmset_t *ps,
		       uint32 id);

extern void
    ps_replace_parms (ps_parmset_t *newps,
		      ps_parmset_t *oldps);


extern void
    ps_remove_parm (ps_parm_t *parm);

extern status_t
    ps_gen_parmset_instance_id (ps_parmset_t  *ps);

extern boolean
    ps_is_vparmset (const ps_parmset_t *ps);

extern void
    ps_replace_vparmset (ps_parmset_t *newps,
			 ps_parmset_t *curps);

extern ps_parm_t *
    ps_start_complex_parm (ps_parmset_t  *ps,
			   const xmlChar *parmname,
			   status_t *res);

extern status_t
    ps_add_simple_parm (ps_parmset_t  *ps,
			const xmlChar *parmname,
			const xmlChar *strval);

extern ps_parmset_t *
    ps_make_new_parmset (const xmlChar *module,
			 const xmlChar *psname);

extern ps_parm_t *
    ps_make_new_parm (ps_parmset_t *ps,
		      const xmlChar *parmname);

extern boolean
    ps_is_empty (const ps_parmset_t *ps);

#endif	    /* _H_ps */
