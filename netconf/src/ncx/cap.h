#ifndef _H_cap
#define _H_cap
/*  FILE: cap.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    NETCONF protocol capabilities

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
28-apr-05    abb      Begun.
*/
#include <xmlstring.h>

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

#define MAX_STD_CAP_NAME_LEN  31

#define CAP_VERSION_LEN  15

/* NETCONF Base Protocol Capability String */
#define CAP_BASE_URN ((const xmlChar *) \
		      "urn:ietf:params:netconf:base:1.0")


/* NETCONF Capability Identifier Base String */
#define CAP_URN ((const xmlChar *)"urn:ietf:params:netconf:capability:")

/* NCX Module Capability Identifier Base String !!! OBSOLETE !!! */
#define CAP_MODURN ((const xmlChar *) \
		    "http://netconfcentral.com/modules/")

#define CAP_SEP_CH   '/'



/************************************************************
 *                                                          *
 * The following 2 sets of definitions must be kept aligned *
 *                                                          *
 ************************************************************/

/* fast lookup -- standard capability bit ID */
#define CAP_BIT_V1            bit0
#define CAP_BIT_WR_RUN        bit1
#define CAP_BIT_CANDIDATE     bit2
#define CAP_BIT_CONF_COMMIT   bit3
#define CAP_BIT_ROLLBACK_ERR  bit4
#define CAP_BIT_VALIDATE      bit5
#define CAP_BIT_STARTUP       bit6
#define CAP_BIT_URL           bit7
#define CAP_BIT_XPATH         bit8
#define CAP_BIT_NOTIFICATION  bit9
#define CAP_BIT_INTERLEAVE    bit10
#define CAP_BIT_PARTIAL_LOCK  bit11
#define CAP_BIT_WITH_DEFAULTS bit12
#define CAP_BIT_NETCONF_MONITORING bit13
#define CAP_BIT_SCHEMA_RETRIEVAL bit14

/* put the version numbers in the capability names for now */
#define CAP_NAME_V1                 ((const xmlChar *)"")
#define CAP_NAME_WR_RUN             ((const xmlChar *)"writable-running:1.0")
#define CAP_NAME_CANDIDATE          ((const xmlChar *)"candidate:1.0")
#define CAP_NAME_CONF_COMMIT        ((const xmlChar *)"confirmed-commit:1.0")
#define CAP_NAME_ROLLBACK_ERR       ((const xmlChar *)"rollback-on-error:1.0")
#define CAP_NAME_VALIDATE           ((const xmlChar *)"validate:1.0")
#define CAP_NAME_STARTUP            ((const xmlChar *)"startup:1.0")
#define CAP_NAME_URL                ((const xmlChar *)"url:1.0")
#define CAP_NAME_XPATH              ((const xmlChar *)"xpath:1.0")
#define CAP_NAME_NOTIFICATION       ((const xmlChar *)"notification:1.0")
#define CAP_NAME_INTERLEAVE         ((const xmlChar *)"interleave:1.0")
#define CAP_NAME_PARTIAL_LOCK       ((const xmlChar *)"partial-lock:1.0")
#define CAP_NAME_WITH_DEFAULTS      ((const xmlChar *)"with-defaults:1.0")
#define CAP_NAME_NETCONF_MONITORING ((const xmlChar *)"netconf-monitoring:1.0")
#define CAP_NAME_SCHEMA_RETRIEVAL   ((const xmlChar *)"schema-retrieval:1.0")

/* some YANG capability details */
#define CAP_REVISION_EQ        (const xmlChar *)"revision="
#define CAP_MODULE_EQ          (const xmlChar *)"module="
#define CAP_FEATURES_EQ        (const xmlChar *)"features="
#define CAP_DEVIATIONS_EQ      (const xmlChar *)"deviations="
#define CAP_SCHEME_EQ          (const xmlChar *)"scheme="
#define CAP_BASIC_EQ           (const xmlChar *)"basic="
#define CAP_SUPPORTED_EQ       (const xmlChar *)"supported="

/********************************************************************
*                                                                   *
*                                    T Y P E S                      *
*                                                                   *
*********************************************************************/

/* NETCONF capability subject types */
typedef enum cap_subjtyp_t_ {
    CAP_SUBJTYP_NONE,
    CAP_SUBJTYP_PROT,       /* capability is a protocol extension */
    CAP_SUBJTYP_DM,                 /* capability is a data model */
    CAP_SUBJTYP_OTHER      /* capability is other than prot or DM */
} cap_subjtyp_t;


/* enumerated list of standard capability IDs */
typedef enum cap_stdid_t_ {
    CAP_STDID_V1,
    CAP_STDID_WRITE_RUNNING,
    CAP_STDID_CANDIDATE,
    CAP_STDID_CONF_COMMIT,
    CAP_STDID_ROLLBACK_ERR,
    CAP_STDID_VALIDATE,
    CAP_STDID_STARTUP,
    CAP_STDID_URL,
    CAP_STDID_XPATH,
    CAP_STDID_NOTIFICATION,
    CAP_STDID_INTERLEAVE,
    CAP_STDID_PARTIAL_LOCK,
    CAP_STDID_WITH_DEFAULTS,
    CAP_STDID_NETCONF_MONITORING,
    CAP_STDID_SCHEMA_RETRIEVAL,
    CAP_STDID_LAST_MARKER
} cap_stdid_t;


typedef struct cap_list_t_ {
    uint32          cap_std;         /* bitset of std caps */
    xmlChar        *cap_protos;  /* URL capability protocol list */
    xmlChar        *cap_defstyle;  /* with-defaults 'basic' parm */
    xmlChar        *cap_supported;  /* with-defaults 'supported' parm */
    dlq_hdr_t       capQ;              /* queue of non-std caps */
} cap_list_t;


/* array of this structure for list of standard capabilities */
typedef struct cap_stdrec_t_ {
    cap_stdid_t     cap_idnum;
    uint32          cap_bitnum;
    const xmlChar  *cap_name;
} cap_stdrec_t;


/* queue of this structure for list of enterprise capabilities */
typedef struct cap_rec_t_ {
    dlq_hdr_t      cap_qhdr;
    cap_subjtyp_t  cap_subject;
    xmlChar       *cap_uri;
    uint32         cap_baselen;
    xmlChar       *cap_module;
    xmlChar       *cap_revision;
    ncx_list_t     cap_feature_list;
    ncx_list_t     cap_deviation_list;
} cap_rec_t;


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

/* memory for caplist already allocated -- this just inits fields */
extern cap_list_t *
    cap_new_caplist (void);

/* memory for caplist already allocated -- this just inits fields */
extern void
    cap_init_caplist (cap_list_t *caplist);

/* memory for caplist not deallocated -- this just cleans fields */
extern void 
    cap_clean_caplist (cap_list_t *caplist);

/* memory for caplist not deallocated -- this just cleans fields */
extern void 
    cap_free_caplist (cap_list_t *caplist);


/* add a standard protocol capability to the list */
extern status_t 
    cap_add_std (cap_list_t *caplist, 
		 cap_stdid_t   capstd);

/* add a standard capability to the value struct version */
extern status_t
    cap_add_stdval (val_value_t *caplist,
		    cap_stdid_t   capstd);


extern status_t 
    cap_add_std_string (cap_list_t *caplist, 
			const xmlChar *uri);

extern status_t 
    cap_add_module_string (cap_list_t *caplist, 
			   const xmlChar *uri);

/* add the :url capability to the list */
extern status_t 
    cap_add_url (cap_list_t *caplist, 
		 const xmlChar *proto_list);

/* add the :with-defaults capability to the list */
extern status_t 
    cap_add_withdef (cap_list_t *caplist, 
		     const xmlChar *defstyle);

/* add the :with-defaults cap as a value struct */
extern status_t
    cap_add_withdefval (val_value_t *caplist,
			const xmlChar *defstyle);

/* add a generic enterprise capability */
extern status_t 
    cap_add_ent (cap_list_t *caplist, 
		 const xmlChar *uristr);

/* add a capability string for a data model module */
extern status_t 
    cap_add_modval (val_value_t *caplist, 
		    ncx_module_t *mod);


/* fast search of standard protocol capability set */
extern boolean 
    cap_std_set (const cap_list_t *caplist,
		 cap_stdid_t capstd);

/* linear search of capability list, will check for std uris as well */
extern boolean 
    cap_set (const cap_list_t *caplist,
	     xmlChar *capuri);

#ifdef CPP_DEBUG
/* printf the capability list as an XML sequence */
extern void 
    cap_printf_XML (cap_list_t *caplist);
#endif

/* get the protocols field for the :url capability */
extern const xmlChar *
    cap_get_protos (cap_list_t *caplist);


/* debug function */
extern void
    cap_dump_stdcaps (const cap_list_t *caplist);

/* debug function */
extern void
    cap_dump_modcaps (const cap_list_t *caplist);

/* debug function */
extern void
    cap_dump_entcaps (const cap_list_t *caplist);

/* app processing access */
extern cap_rec_t *
    cap_first_modcap (cap_list_t *caplist);

/* app processing access */
extern cap_rec_t *
    cap_next_modcap (cap_rec_t *curcap);

/* app processing access */
extern void
    cap_split_modcap (cap_rec_t *cap,
		      const xmlChar **module,
		      uint32 *modlen,
		      const xmlChar **version);

/* make the module URI string (for sysCapabilityChange event) */
extern xmlChar *
    cap_make_moduri (ncx_module_t *mod);

#endif            /* _H_cap */
