/*  FILE: def_reg.c

   Definition Lookup Registry

   Stores Pointers to Data Structures but does not malloc or free
   those structures.  That must be done outside this registry,
   which justs provides hash table storage of pointers

   Registry node types

     DEF_NT_NSNODE  : Namespace to module pointer

     DEF_NT_FDNODE  : File Descriptor to session control block

     DEF_NT_MODNAME : ncx_module_t lookup

     DEF_NT_OWNNODE   Parent node of module-specific applications

     DEF_NT_DEFNODE : Child node: Module-specific definition

     DEF_NT_CFGNODE : Child node: Configuration data pointer

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
17oct05      abb      begun
11-feb-06    abb      remove application layer; too complicated
18-aug-07    abb      add user variable support for ncxcli

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_bobhash
#include "bobhash.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
/* 256 row, chained entry top hash table */
#define DR_TOP_HASH_SIZE   (hashsize(8))
#define DR_TOP_HASH_MASK   (hashmask(8))

/* 256 row, chained entry module hash table of applications */
#define DR_OWN_HASH_SIZE   (hashsize(8))
#define DR_OWN_HASH_MASK   (hashmask(8))

/* random number to seed the hash function */
#define DR_HASH_INIT       0x7e456289

#define DEF_NUM_CFG_PTRS   4

/********************************************************************
*                                                                   *
*                          T Y P E S                                *
*                                                                   *
*********************************************************************/
/* top-level registry node type */
typedef enum def_nodetyp_t_ {
    DEF_NT_NONE,
    DEF_NT_MODNODE,         /* topht module name */
    DEF_NT_NSNODE,          /* topht namespace URI */
    DEF_NT_PRENODE,         /* topht module prefix */
    DEF_NT_FDNODE,          /* topht file descriptor integer */
    DEF_NT_OWNNODE,         /* topht module name */
    DEF_NT_DEFNODE,         /* ownht module object name */
    DEF_NT_TYPNODE,         /* ownht module typedef name */
    DEF_NT_GRPNODE,         /* ownht module grouping name */
    DEF_NT_CFGAPP,          /* ownht configuration app hdr pointer */
    DEF_NT_CFGNODE          /* ownht configuration data pointer */
} def_nodetyp_t;


/* The def_reg module header that must be the first field in
 * any record stored in any level hash table
 */
typedef struct def_hdr_t_ {
    dlq_hdr_t       qhdr;
    def_nodetyp_t   nodetyp; 
    const xmlChar  *key;
} def_hdr_t;


/* DEF_NT_MODNODE
 * DEF_NT_NSNODE
 * DEF_NT_PRENODE
 *
 * Top tier: Entries that don't have any children 
 */
typedef struct def_topnode_t_ {
    def_hdr_t      hdr;
    void          *dptr;
} def_topnode_t;


/* DEF_NT_FDNODE
 *
 * Top Tier: File Descriptor Mapping Content Struct
 */
typedef struct def_fdmap_t_ {
    xmlChar    num[NCX_MAX_NLEN];
    int        fd;
    ses_cb_t  *scb;
} def_fdmap_t;


/* DEF_NT_OWNNODE:
 *
 * Top tier: module node, has child application hash table 
 */
typedef struct def_ownnode_t_ {
    def_hdr_t      hdr;
    xmlChar       *name;
    dlq_hdr_t      ownht[DR_OWN_HASH_SIZE];
} def_ownnode_t;


/* DEF_NT_DEFNODE:
 *
 * 3rd tier: Definition header containing a
 * hard-wired enumeration for the data type and a 
 * definition name.  Searches must match the datatyp
 * and the data name 
 */
typedef struct def_defnode_t_ {
    def_hdr_t  hdr;
    ncx_node_t  datatyp;
    void       *data;
} def_defnode_t;


/* DEF_NT_CFGNODE: generic container for cfg data 
 *
 * struct containing the data type and N pointers
 * the this corresponding data node in up to N configs
 */
typedef struct def_cfgdata_t_ {
    ncx_node_t   datatyp;
    void       *data[DEF_NUM_CFG_PTRS];
} def_cfgdata_t;


/* DEF_NT_CFGAPP header
 * 
 * 3rd tier: Configuration data header containing a
 * pointer to a configuration parmset
 */
typedef struct def_cfgapp_t_ {
    def_hdr_t       hdr;
    def_cfgdata_t   cfgdata;
} def_cfgapp_t;


/* DEF_NT_CFGNODE header
 * 
 * 3rd tier: Configuration data header containing a
 * pointer to a configuration parmset
 */
typedef struct def_cfgnode_t_ {
    def_hdr_t       hdr;
    const xmlChar  *key2;
    const xmlChar  *key3;
    def_cfgdata_t   cfgdata;
} def_cfgnode_t;



/* Queue of module node pointers to identify all modules
 * with definitions in the registry
 */
typedef struct def_module_t_ {
    dlq_hdr_t hdr;
    const xmlChar *module;
} def_module_t;

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

/* first tier: module header hash table */
static dlq_hdr_t   topht[DR_TOP_HASH_SIZE];

/* queue of quick look up module names */
static dlq_hdr_t   moduleQ;

/* module init flag */
static boolean     def_reg_init_done = FALSE;


/********************************************************************
* FUNCTION node2def
* 
* Get the def_nodetyp_t enumeration for the specified
* ncx_node_t enumeration
*
* INPUTS:
*    ntyp == ncx_node_t to convert
*
* RETURNS:
*    def_nodetyp_t enum value
*********************************************************************/
static def_nodetyp_t
    node2def (ncx_node_t ntyp)
{
    switch (ntyp) {
    case NCX_NT_GRP:
	return DEF_NT_GRPNODE;
    case NCX_NT_TYP:
	return DEF_NT_TYPNODE;
    case NCX_NT_OBJ:
    default:
	return DEF_NT_DEFNODE;
    }
}  /* node2def */


/********************************************************************
* FUNCTION find_top_node_h
* 
* Find a top level node by its type and key
* Retrieve the hash table index in case the search fails
*
* INPUTS:
*    nodetyp == def_nodetyp_t enum value
*    key == top level node key
* OUTPUTS:
*    *h == hash table index
* RETURNS:
*    void *to the top level entry or NULL if not found
*********************************************************************/
static void * 
    find_top_node_h (def_nodetyp_t nodetyp,
		     const xmlChar *key,
		     uint32 *h)
{
    uint32 len;
    def_hdr_t *hdr;

    len = xml_strlen(key);
    if (!len) {
	return NULL;
    }

    /* get the hash value */
    *h = bobhash(key, len, DR_HASH_INIT);

    /* clear bits to fit the topht array size */
    *h &= DR_TOP_HASH_MASK;

    for (hdr = (def_hdr_t *)dlq_firstEntry(&topht[*h]);
	 hdr != NULL;
	 hdr = (def_hdr_t *)dlq_nextEntry(hdr)) {
        if (hdr->nodetyp==nodetyp && !xml_strcmp(key, hdr->key)) {
            return (void *)hdr;
        }
    }
    return NULL;

}  /* find_top_node_h */


/********************************************************************
* FUNCTION find_top_node
* 
* Find a top level node by its type and key
*
* INPUTS:
*    nodetyp == def_nodetyp_t enum value
*    key == top level node key
* RETURNS:
*    void *to the top level entry or NULL if not found
*********************************************************************/
static void * 
    find_top_node (def_nodetyp_t nodetyp,
		   const xmlChar *key)
{
    uint32  h;
    return find_top_node_h(nodetyp, key, &h);

}  /* find_top_node */


/********************************************************************
* FUNCTION add_top_node
*   
* add one top-level node to the registry
* !!! NOT FOR OWNNODE ENTRIES !!!
*
* INPUTS:
*    nodetyp == internal node type enum
*    key == address of key string inside 'ptr'
*    ptr == struct pointer to store in the registry
* RETURNS:
*    status of the operation
*********************************************************************/
static status_t 
    add_top_node (def_nodetyp_t  nodetyp,
		  const xmlChar *key, 
		  void *ptr)
{
    uint32 h;
    def_topnode_t *top;

    if (!*key) {
	return SET_ERROR(ERR_INTERNAL_VAL);  /* zero key len */
    }

    /* check if the entry already exists */
    top = (def_topnode_t *)find_top_node_h(nodetyp, key, &h);
    if (top) {
        return ERR_NCX_DUP_ENTRY;
    }

    /* create a new def_topnode_t struct and initialize it */
    top = m__getObj(def_topnode_t);
    if (top == NULL) {
	return ERR_INTERNAL_MEM;
    }
    (void)memset(top, 0x0, sizeof(def_topnode_t));
    top->hdr.nodetyp = nodetyp;
    top->hdr.key = key;
    top->dptr = ptr;

    /* add the topnode to the topht */
    dlq_enque(top, &topht[h]);
    return NO_ERR;

} /* add_top_node */


/********************************************************************
* FUNCTION find_own_child_h
* 
* Find a module level node by its type and key
* Retrieve the hash table index in case the search fails
*
* INPUTS:
*    own == def_ownnode_t module of this definition
*    nodetyp == def_nodetyp_t enum value
*               DEF_NT_TYPNODE   (typedef)
*               DEF_NT_GRPNODE   (typedef)
*               DEF_NT_DEFNODE   (module-global def)
*
*        These owner-specific node types handled elsewhere
*               DEF_NT_APPNODE   (application header)
*               DEF_NT_CFGNODE   (config data)
*
*    key == top level node key
*    h == address of return hash index
*
* OUTPUTS:
*   *h == hash table index
*
* RETURNS:
*    void *to the module level entry or NULL if not found
*********************************************************************/
static void * 
    find_own_child_h (def_ownnode_t  *own,
		      def_nodetyp_t nodetyp,
		      const xmlChar *key,
		      uint32 *h)
{
    uint32 len;
    def_hdr_t *hdr;

    len = xml_strlen(key);
    if (!len) {
	return NULL;
    }
    if (nodetyp==DEF_NT_NONE) {
	return NULL;
    }

    /* get the hash value */
    *h = bobhash(key, len, DR_HASH_INIT);

    /* clear bits to fit the ownht array size */
    *h &= DR_OWN_HASH_MASK;

    for (hdr = (def_hdr_t *)dlq_firstEntry(&own->ownht[*h]);
	 hdr != NULL;
	 hdr = (def_hdr_t *)dlq_nextEntry(hdr)) {
        if (hdr->nodetyp==nodetyp && !xml_strcmp(key, hdr->key)) {
            return (void *)hdr;
        }
    }
    return NULL;

}  /* find_own_child_h */


/********************************************************************
* FUNCTION find_own_child
* 
* Find a module level node by its type and key
*
* INPUTS:
*    own == def_module_t
*    nodetyp == def_nodetyp_t enum value
*    key == top level node key
*
* RETURNS:
*    void *to the top level entry or NULL if not found
*********************************************************************/
static void * 
    find_own_child (def_ownnode_t  *own,
		    def_nodetyp_t nodetyp,
		    const xmlChar *key)
{
    uint32  h;
    return find_own_child_h(own, nodetyp, key, &h);

}  /* find_own_child */


/********************************************************************
* FUNCTION add_mod_node
* 
* Add a def_ownnode_t to the topht hash table
* If it already exists then just return the existing entry
*
* INPUTS:
*    modname == module name 
*
* RETURNS:
*    the new def_ownnode_t or a pointer to the existing entry
*    OR NULL if the operation failed
*********************************************************************/
static def_ownnode_t * 
    add_mod_node (const xmlChar *modname)
{
    uint32 h, i;
    def_ownnode_t *own;
    def_module_t   *modulenode;

    own = (def_ownnode_t *)find_top_node_h(DEF_NT_OWNNODE, modname, &h);
    if (own) {
        return own;
    }

    /* create a new def_ownnode_t struct and initialize it */
    own = m__getObj(def_ownnode_t);
    if (own == NULL) {
	return NULL;
    }

    /* start a new module node for the quick list */
    modulenode = m__getObj(def_module_t);
    if (modulenode == NULL) {
	m__free(own);
	return NULL;
    }

    /* fill in the hash table module node */
    (void)memset(own, 0x0, sizeof(def_ownnode_t));
    own->hdr.nodetyp = DEF_NT_OWNNODE;
    own->name = xml_strdup(modname);
    if (!own->name) {
	m__free(own);
	m__free(modulenode);
	return NULL;
    }
    own->hdr.key = own->name;
    for (i=0; i<DR_OWN_HASH_SIZE; i++) {
	dlq_createSQue(&own->ownht[i]);
    }

    /* add the ownnode to the topht */
    dlq_enque(own, &topht[h]);

    /* fill in the quick list module node and add to moduleQ */
    (void)memset(modulenode, 0x0, sizeof(def_module_t));
    modulenode->module = own->name;
    dlq_enque(modulenode, &moduleQ);

    return own;

}  /* add_mod_node */


/********************************************************************
* FUNCTION free_modnode
* 
* Destroy a def_ownnode_t in the topht hash table
* Entry has already been removed from any queue
*
* INPUTS:
*    own == application header
* RETURNS:
*    none
*********************************************************************/
static void 
    free_modnode (def_ownnode_t  *own)
{
    uint32         i;
    def_hdr_t     *hdr;

    /* go through the own->ownht and purge all entries */
    for (i=0; i<DR_OWN_HASH_SIZE; i++) {
        while (!dlq_empty(&own->ownht[i])) { 
	    hdr = (def_hdr_t *)dlq_deque(&own->ownht[i]);
	    switch (hdr->nodetyp) {
	    case DEF_NT_DEFNODE:
		m__free(hdr);
		break;
	    case DEF_NT_TYPNODE:
		m__free(hdr);
		break;
	    case DEF_NT_GRPNODE:
		m__free(hdr);
		break;
	    case DEF_NT_CFGAPP:
		m__free(hdr);
		break;
	    case DEF_NT_CFGNODE:
		m__free(hdr);
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
		m__free(hdr);
	    }
        }
    }

    if (own->name) {
	m__free(own->name);
    }
    m__free(own);

}  /* free_modnode */


#if 0
/********************************************************************
* FUNCTION find_cfgdef
*   
* find an configuration definition
*
* INPUTS:
*    ownnode == ownnode to search for the registry application 
*    appname == application name to find
*    len1 == length of application name
*    defname == definition name to find
*    len2 == length of defname
*    instance == instance string ID of this definition 
*                NULL means it is a static parmset
*    len3     == length of instance ID string
*    dtyp == ncx_node_t enumeration for the data node type
*    h == non-NULL if the hash value is desired
*
* OUTPUTS:
*   if h is non-NULL, *h is set to the hash value, 
*      even if the specified entry is not found
*
* RETURNS:
*    pointer to the internal config data node, or NULL if not found
*********************************************************************/
static def_cfgnode_t *
    find_cfgdef (def_ownnode_t *own,
		 const xmlChar *appname,
		 uint32         len1,
		 const xmlChar *defname,
		 uint32         len2,
		 const xmlChar *instance,
		 uint32         len3,
		 ncx_node_t      dtyp,
		 uint32        *h)
{
    def_cfgnode_t *def;
    uint32 h1, h2, h3;

    /* get the hash value */
    h1 = bobhash(appname, len1, DR_HASH_INIT);
    h2 = bobhash(defname, len2, DR_HASH_INIT);
    h3 = (instance) ? bobhash(instance, len3, DR_HASH_INIT) : 0;
    h1 = (h1 ^ h2 ^ h3) & DR_OWN_HASH_MASK;
    if (h) {
	*h = h1;
    }

    /* check if the entry exists */
    for (def = (def_cfgnode_t *)dlq_firstEntry(&own->ownht[h1]);
	 def != NULL;
	 def = (def_cfgnode_t *)dlq_nextEntry(def)) {
	if (def->hdr.nodetyp==DEF_NT_CFGNODE &&
	    !xml_strcmp(appname, def->hdr.key) &&
	    !xml_strcmp(defname, def->key2)) {

	    if (instance) {
		if (!def->key3) {
		    /* this is the 'top' instance */
		    continue;
		}

		/* check if the instance IDs match */
		if (xml_strcmp(instance, def->key3)) {
		    /* not a match */
		    continue;
		}
	    } else if (def->key3) {
		/* looking for 'top', found an instanced version */
		continue;
	    }		

	    /* check an extry already exists, but different type */
	    if (dtyp != NCX_NT_NONE && def->cfgdata.datatyp != dtyp) {
		return NULL;
	    }

	    /* this is a match */
	    return def;
	}
    }

    return NULL;

} /* find_cfgdef */
#endif


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION def_reg_init
* 
* Initialize the def_reg module
*
* RETURNS:
*    none
*********************************************************************/
void 
    def_reg_init (void)
{
    uint32 i;

    if (!def_reg_init_done) {
	/* initialize the application hash table */
	for (i=0; i<DR_TOP_HASH_SIZE; i++) {
	    dlq_createSQue(&topht[i]);
	}
	dlq_createSQue(&moduleQ);
	def_reg_init_done = TRUE;
    }  /* else already done */

}  /* def_reg_init */


/********************************************************************
* FUNCTION def_reg_cleanup
* 
* Cleanup all the malloced memory in this module
* and return the module to an uninitialized state
* After this fn, the def_reg_init fn could be called again
*
* INPUTS:
*    none
* RETURNS:
*    none
*********************************************************************/
void 
    def_reg_cleanup (void)
{
    uint32 i;
    def_hdr_t     *hdr;
    def_topnode_t *topnode;
    def_module_t   *modulenode;

    if (!def_reg_init_done) {
	return;
    }

    /* cleanup the top hash table */
    for (i=0; i<DR_TOP_HASH_SIZE; i++) {
	while (!dlq_empty(&topht[i])) {
	    hdr = (def_hdr_t *)dlq_deque(&topht[i]);
	    switch (hdr->nodetyp) {
	    case DEF_NT_MODNODE:
	    case DEF_NT_NSNODE:
	    case DEF_NT_PRENODE:
		m__free(hdr);
		break;
	    case DEF_NT_FDNODE:
		/* free the def_fdnode_t struct first */
		topnode = (def_topnode_t *)hdr;
		m__free(topnode->dptr);
		m__free(topnode);
		break;
	    case DEF_NT_OWNNODE:
		free_modnode((def_ownnode_t *)hdr);
		break;
	    default:
		SET_ERROR(ERR_INTERNAL_VAL);
		m__free(hdr);  /* free it anyway */
	    }
	}
    }
    (void)memset(topht, 0x0, sizeof(dlq_hdr_t)*DR_TOP_HASH_SIZE);

    /* clean the module name Q */
    while (!dlq_empty(&moduleQ)) {
	modulenode = (def_module_t *)dlq_deque(&moduleQ);
	m__free(modulenode);
    }

    def_reg_init_done = FALSE;

}  /* def_reg_cleanup */


/********************************************************************
* FUNCTION def_reg_add_ns
*   
* add one xmlns_t to the registry
*
* INPUTS:
*    ns == namespace record to add
* RETURNS:
*    status of the operation
*********************************************************************/
status_t 
    def_reg_add_ns (xmlns_t *ns)
{
#ifdef DEBUG
    if (!ns) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return add_top_node(DEF_NT_NSNODE, ns->ns_name, ns);

} /* def_reg_add_ns */


/********************************************************************
* FUNCTION def_reg_find_ns
*   
* find one xmlns_t in the registry
*
* INPUTS:
*    nsname == namespace ID to find
* RETURNS:
*    pointer to xmlns_t or NULL if not found
*********************************************************************/
xmlns_t * 
    def_reg_find_ns (const xmlChar *nsname)
{
    def_topnode_t *nsdef;

#ifdef DEBUG
    if (!nsname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    nsdef = find_top_node(DEF_NT_NSNODE, nsname);
    return (nsdef) ? (xmlns_t *)nsdef->dptr : NULL;

} /* def_reg_find_ns */


/********************************************************************
* FUNCTION def_reg_del_ns
*   
* delete one ncx_module from the registry
*
* INPUTS:
*    nsname == namespace name to delete
* RETURNS:
*    none
*********************************************************************/
void
    def_reg_del_ns (const xmlChar *nsname)
{
    def_topnode_t *nsdef;

#ifdef DEBUG
    if (!nsname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    nsdef = find_top_node(DEF_NT_NSNODE, nsname);
    if (nsdef) {
        dlq_remove(nsdef);
        m__free(nsdef);
    }
} /* def_reg_del_ns */


/********************************************************************
* FUNCTION def_reg_add_scb
*   
* add one FD to SCB mapping to the registry
*
* INPUTS:
*    fd == file descriptor to add
*    session == ses_cb_t for the session
* RETURNS:
*    status of the operation
*********************************************************************/
status_t 
    def_reg_add_scb (int fd,
		     ses_cb_t *scb)
{
    def_fdmap_t *fdmap;
    int          ret;
    status_t     res;

#ifdef DEBUG
    if (!scb) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* create an FD-to-SCB mapping */
    fdmap = m__getObj(def_fdmap_t);
    if (!fdmap) {
	return ERR_INTERNAL_MEM;
    }
    memset(fdmap, 0x0, sizeof(def_fdmap_t));

    /* get a string key */
    ret = sprintf((char *)fdmap->num, "%d", fd);
    if (ret <= 0) {
	m__free(fdmap);
	return ERR_NCX_INVALID_NUM;
    }

    /* set the mapping */
    fdmap->fd = fd;
    fdmap->scb = scb;
    
    /* save the string-keyed mapping entry */
    res = add_top_node(DEF_NT_FDNODE, fdmap->num, fdmap);
    if (res != NO_ERR) {
	m__free(fdmap);
    }
    return res;

} /* def_reg_add_scb */


/********************************************************************
* FUNCTION def_reg_find_scb
*   
* find one FD-to-SCB mapping in the registry
*
* INPUTS:
*    fd == file descriptor ID to find
* RETURNS:
*    pointer to ses_cb_t or NULL if not found
*********************************************************************/
ses_cb_t * 
    def_reg_find_scb (int fd)
{
    def_topnode_t *fddef;
    def_fdmap_t   *fdmap;
    int            ret;
    xmlChar        buff[NCX_MAX_NLEN];

    ret = sprintf((char *)buff, "%d", fd);
    if (ret <= 0) {
	return NULL;
    }
    
    fddef = find_top_node(DEF_NT_FDNODE, buff);
    if (!fddef) {
	return NULL;
    }
    fdmap = fddef->dptr;
    return fdmap->scb;

} /* def_reg_find_scb */


/********************************************************************
* FUNCTION def_reg_del_scb
*   
* delete one FD to SCB mapping from the registry
*
* INPUTS:
*    fd == file descriptor index to delete
* RETURNS:
*    none
*********************************************************************/
void
    def_reg_del_scb (int fd)
{
    def_topnode_t *fddef;
    int            ret;
    xmlChar        buff[NCX_MAX_NLEN];

    ret = sprintf((char *)buff, "%d", fd);
    if (ret <= 0) {
	return;
    }

    fddef = find_top_node(DEF_NT_FDNODE, buff);
    if (fddef) {
        dlq_remove(fddef);
        m__free(fddef->dptr);  /* free the def_fdmap_t */
	m__free(fddef);
    }
} /* def_reg_del_scb */


/********************************************************************
* FUNCTION def_reg_add_module
*   
* add one ncx_module_t to the registry
*
* INPUTS:
*    mod == ncx_module to add
* RETURNS:
*    status of the operation
*********************************************************************/
status_t 
    def_reg_add_module (ncx_module_t *mod)
{
    status_t  res;


#ifdef DEBUG
    if (!mod) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = add_top_node(DEF_NT_MODNODE, mod->name, mod);
    if (res == NO_ERR) {
	res = add_top_node(DEF_NT_PRENODE, mod->prefix, mod);
	if (res != NO_ERR) {
	    log_error("\nError: Duplicate prefix (%s) in module %s",
		      mod->prefix, mod->name);
	}
    }
    return res;

} /* def_reg_add_module */


/********************************************************************
* FUNCTION def_reg_find_module
*   
* find one ncx_modulet in the registry
*
* INPUTS:
*    modname == module name to find
*
* RETURNS:
*    pointer to struct or NULL if not found
*********************************************************************/
ncx_module_t * 
    def_reg_find_module (const xmlChar *modname)
{
    def_topnode_t *moddef;

#ifdef DEBUG
    if (!modname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    moddef = find_top_node(DEF_NT_MODNODE, modname);
    return (moddef) ? (ncx_module_t *)moddef->dptr : NULL;

} /* def_reg_find_module */


/********************************************************************
* FUNCTION def_reg_find_module_prefix
*   
* find one ncx_modulet in the registry
*
* INPUTS:
*    prefix == official prefix for module to find
*
* RETURNS:
*    pointer to struct or NULL if not found
*********************************************************************/
ncx_module_t * 
    def_reg_find_module_prefix (const xmlChar *prefix)
{
    def_topnode_t *predef;

#ifdef DEBUG
    if (!prefix) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    predef = find_top_node(DEF_NT_PRENODE, prefix);
    return (predef) ? (ncx_module_t *)predef->dptr : NULL;

} /* def_reg_find_module_prefix */


/********************************************************************
* FUNCTION def_reg_del_module
*   
* delete one ncx_module from the registry
*
* INPUTS:
*    modname == module name to delete
* RETURNS:
*    none
*********************************************************************/
void
    def_reg_del_module (const xmlChar *modname)
{
    def_topnode_t *moddef, *predef;
    ncx_module_t  *mod;

#ifdef DEBUG
    if (!modname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    moddef = find_top_node(DEF_NT_MODNODE, modname);
    if (moddef) {
        dlq_remove(moddef);
	mod = (ncx_module_t *)moddef->dptr;

	predef = find_top_node(DEF_NT_PRENODE, mod->prefix);
	if (predef) {
	    dlq_remove(predef);
	    m__free(predef);
	}

        m__free(moddef);
    }

} /* def_reg_del_module */


/********************************************************************
* FUNCTION def_reg_add_moddef
*   
* Add an module-global definition
* INPUTS:
*    modname == module name of the registry
*    defname == address of the definition key
*    dtyp == data type enumeration
*    dptr == data pointer to add
*
* RETURNS:
*    NO_ERR if added okay;
*********************************************************************/
status_t 
    def_reg_add_moddef (const xmlChar *modname,
			const xmlChar *defname,
			ncx_node_t dtyp,
			void  *dptr)
{
    def_ownnode_t *own;
    def_defnode_t *def;
    uint32         h, len;
    def_nodetyp_t  ntyp;

#ifdef DEBUG
    /* check the parameters */
    if (!modname || !defname || !dptr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (dtyp==NCX_NT_NONE) {
	return SET_ERROR(ERR_INTERNAL_VAL);	
    }
#endif

    /* check the module name length */
    len = xml_strlen(modname);
    if (len==0 || len>NCX_MAX_NLEN) {
	return ERR_NCX_WRONG_LEN;
    }

    /* check the definition name length */
    len = xml_strlen(defname);
    if (len==0 || len>NCX_MAX_NLEN) {
	return ERR_NCX_WRONG_LEN;
    }

    /* get the module node or create a new one */
    own = add_mod_node(modname);
    if (!own) {
	return ERR_INTERNAL_MEM;
    }

    /* get the definition hash value */
    h = bobhash(defname, len, DR_HASH_INIT);

    /* clear bits to fit the ownht array size */
    h &= DR_OWN_HASH_MASK;

    ntyp = node2def(dtyp);

    /* check if the entry exists */
    for (def = (def_defnode_t *)dlq_firstEntry(&own->ownht[h]);
	 def != NULL;
	 def = (def_defnode_t *)dlq_nextEntry(def)) {
	if (def->hdr.nodetyp==ntyp &&
	    !xml_strcmp(defname, def->hdr.key)) {
	    return ERR_NCX_ENTRY_EXISTS;
	}
    }

    /* create a new def_defnode_t struct and initialize it */
    def = m__getObj(def_defnode_t);
    if (def == NULL) {
	return ERR_INTERNAL_MEM;
    }
    (void)memset(def, 0x0, sizeof(def_defnode_t));
    def->hdr.nodetyp = ntyp;
    def->hdr.key = defname;
    def->datatyp = dtyp;
    def->data = dptr;

    /* add the defnode to the ownht */
    dlq_enque(def, &own->ownht[h]);
    return NO_ERR;

}  /* def_reg_add_moddef */


/********************************************************************
* FUNCTION def_reg_find_moddef
*   
*   Get a void * to a module-specific definition of the specified kind
*   or any kind if *dtyp == NCX_NT_NONE
*
* INPUTS:
*    modname == registry module to find
*    defname == definition name to find
*    *dtyp == definition type to find (NCX_NT_NONE==find any)
*
* OUTPUTS:
*    *dtyp == type of data found if input was NCX_NT_NONE
*
* RETURNS:
*    void * to the found data or NULL if not found or wrong type
*********************************************************************/
void *
    def_reg_find_moddef (const xmlChar *modname,
			 const xmlChar *defname,
			 ncx_node_t  *dtyp)
{
    def_ownnode_t *own;
    def_defnode_t *def;
    def_nodetyp_t  ntyp;

#ifdef DEBUG
    if (!modname || !defname || !dtyp) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_NT_NONE;
    }
#endif

    ntyp = node2def(*dtyp);

    /* look for the entry */
    own = (def_ownnode_t *)find_top_node(DEF_NT_OWNNODE, modname);
    if (own) {
	def = (def_defnode_t *)find_own_child(own, ntyp, defname);
	if (def) {
	    if (*dtyp==NCX_NT_NONE) {
		*dtyp = def->datatyp;
		return def->data;
	    } else if (*dtyp != def->datatyp) {
		return NULL;
	    } else {
		return def->data;
	    }
        }
    }
    return NULL;

}  /* def_reg_find_moddef */


/********************************************************************
* FUNCTION def_reg_find_any_moddef
*   
*   Get a void * to a module-specific definition of the specified kind
*   or any kind if *dtyp == NCX_NT_NONE
*
* INPUTS:
*    modname == address of registry module found
*    defname == definition name to find
*    *dtyp == definition type to find (NCX_NT_NONE==find any)
*
* OUTPUTS:
*    Set only if return value is non-NULL:
*      *modname == module string found 
*      *dtyp == type of data found if input was NCX_NT_NONE
*
* RETURNS:
*    void * to the found data or NULL if not found or wrong type
*********************************************************************/
void *
    def_reg_find_any_moddef (const xmlChar **modname,
			     const xmlChar *defname,
			     ncx_node_t  *dtyp)
{
    def_ownnode_t *own;
    def_module_t   *modulenode;
    def_defnode_t *def;
    def_nodetyp_t  ntyp;

#ifdef DEBUG
    if (!modname || !defname || !dtyp) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NCX_NT_NONE;
    }
#endif

    ntyp = node2def(*dtyp);

    /* go through all the module names in the moduleQ */
    for (modulenode = (def_module_t *)dlq_firstEntry(&moduleQ);
	 modulenode != NULL;
	 modulenode = (def_module_t *)dlq_nextEntry(modulenode)) {

	*modname = modulenode->module;

	/* look for the entry */
	own = (def_ownnode_t *)find_top_node(DEF_NT_OWNNODE, *modname);
	if (own) {
	    def = (def_defnode_t *)
		find_own_child(own, ntyp, defname);
	    if (def) {
		if (*dtyp==NCX_NT_NONE) {
		    *dtyp = def->datatyp;
		    return def->data;
		} else if (*dtyp == def->datatyp) {
		    return def->data;
		} /* else keep looking */
	    }
	} else {
	    /* should not happen */
	    SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
	}
    }
    return NULL;

}  /* def_reg_find_any_moddef */


/********************************************************************
* FUNCTION def_reg_del_moddef
*   
* Remove a module definition
*
* INPUTS:
*    modname == registry module to find
*    defname == definition name to delete
*    dtyp == definition type for 'defname'
*
* RETURNS:
*    none
*********************************************************************/
void
    def_reg_del_moddef (const xmlChar *modname,
			const xmlChar *defname,
			ncx_node_t dtyp)
{
    def_ownnode_t *own;
    def_defnode_t *def;
    def_nodetyp_t  ntyp;

#ifdef DEBUG
    if (!modname || !defname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    ntyp = node2def(dtyp);

    own = (def_ownnode_t *)find_top_node(DEF_NT_OWNNODE, modname);
    if (own) {
	def = (def_defnode_t *)
	    find_own_child(own, ntyp, defname);
	if (def) {
	    dlq_remove(def);
	    m__free(def);
	}
    }
}  /* def_reg_del_moddef */

#if 0
/********************************************************************
* FUNCTION def_reg_add_cfgdef
*   
* add an configuration definition 
*
* INPUTS:
*    modname == registry module to add
*    defname == definition name to add
*    instance == instance string ID of this definition 
*                NULL means it is a static parmset
*    cfgid == ncx_cfg_t of the cfg to add this node for
*    dtyp == ncx_node_t enumeration for the data node type
*    dptr == pointer to the data node to store
* RETURNS:
*    status, NO_ERROR if all ok and does not already exist
*********************************************************************/
status_t 
    def_reg_add_cfgdef (const xmlChar *modname,
			const xmlChar *defname,
			const xmlChar *instance,
			ncx_cfg_t cfgid, 
			ncx_node_t dtyp,
			void  *dptr)
{
    def_ownnode_t *own;
    def_cfgnode_t *def;
    uint32 h, len1, len2, len3;

#ifdef DEBUG
    /* check the parameters */
    if (!modname || !appname || !defname || !dptr) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (dtyp==NCX_NT_NONE) {
	return SET_ERROR(ERR_INTERNAL_VAL);	
    }

    /* check the module name length */
    len1 = xml_strlen(modname);
    if (len1==0 || len1>NCX_MAX_NLEN) {
	return ERR_NCX_WRONG_LEN;
    }
    if (cfgid >= DEF_NUM_CFG_PTRS) {
	return ERR_NCX_INVALID_VALUE;
    }
#endif

    /* check the definition name length */
    len1 = xml_strlen(appname);
    if (len1==0 || len1>NCX_MAX_NLEN) {
	return ERR_NCX_WRONG_LEN;
    }

    /* check the definition name length */
    len2 = xml_strlen(defname);
    if (len2==0 || len2>NCX_MAX_NLEN) {
	return ERR_NCX_WRONG_LEN;
    }

    /* check for an instance ID */
    if (instance) {
	len3 = xml_strlen(instance);
	if (len3==0) {
	    return ERR_NCX_WRONG_LEN;
	}
    } else {
	len3 = 0;
    }

    /* get the module node or create a new one */
    own = add_mod_node(modname);
    if (!own) {
	return ERR_INTERNAL_MEM;
    }

    /* see if the def node already exists */
    def = find_cfgdef(own, appname, len1, defname, len2, 
		      instance, len3, dtyp, &h);
    if (def) {
	def->cfgdata.data[cfgid] = dptr;
	return NO_ERR;
    }

    /* else create a new def_defnode_t struct and initialize it */
    def = m__getObj(def_cfgnode_t);
    if (def == NULL) {
	return ERR_INTERNAL_MEM;
    }
    (void)memset(def, 0x0, sizeof(def_cfgnode_t));
    def->hdr.nodetyp = DEF_NT_CFGNODE;
    def->hdr.key = appname;
    def->key2 = defname;
    def->key3 = instance;
    def->cfgdata.datatyp = dtyp;
    def->cfgdata.data[cfgid] = dptr;

    /* add the new defnode to the ownht */
    dlq_enque(def, &own->ownht[h]);
    return NO_ERR;

} /* def_reg_add_cfgdef */


/********************************************************************
* FUNCTION def_reg_find_cfgdef
*   
* find an configuration definition
*
* INPUTS:
*    modname == registry module to find
*    appname == registry application to find
*    defname == definition name to find
*    instance == instance string ID of this definition 
*                NULL means it is a static parmset
*    cfgid == cfg_id_t of the cfg to check
*    *dtyp == ncx_node_t enumeration for the data node type
*
* OUTPUTS:
*   *dtyp is set to the actual data type found.
*    this is only relevant if the value was NCX_NT_NONE
*    upon input.  
*
* RETURNS:
*    pointer to the data node that is stored, or NULL if not found
*    or any error
*********************************************************************/
void *
    def_reg_find_cfgdef (const xmlChar *modname,
			 const xmlChar *appname,
			 const xmlChar *defname,
			 const xmlChar *instance,
			 ncx_cfg_t cfgid,
			 ncx_node_t  *dtyp)
{
    def_ownnode_t *own;
    def_cfgnode_t *def;
    uint32         len;

#ifdef DEBUG
    /* check the parameters */
    if (!modname || !appname || !defname || !dtyp) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }

    if (cfgid >= DEF_NUM_CFG_PTRS) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    /* look for the entry */
    own = (def_ownnode_t *)find_top_node(DEF_NT_OWNNODE, modname);
    if (!own) {
	return NULL;
    }

    len = (instance) ? xml_strlen(instance) : 0;

    def = find_cfgdef(own, appname, xml_strlen(appname), 
		      defname, xml_strlen(defname), 
		      instance, len, *dtyp, NULL);
    if (def && def->cfgdata.data[cfgid]) {
	/* entry found and data type is correct */
	*dtyp = def->cfgdata.datatyp;
	return def->cfgdata.data[cfgid];
    }

    return NULL;

} /* def_reg_find_cfgdef */


/********************************************************************
* FUNCTION def_reg_del_cfgdef
*   
* delete a config data pointer (or all pointers)
* to a stored config data item
*
* INPUTS:
*    modname == registry module to find
*    appname == application name to find
*    defname == definition name to delete
*    instance == instance string ID of this definition 
*                NULL means it is a static parmset
*    cfgid == < 0 to delete all entries
*             > 0 to delete a specific config data pointer
* RETURNS:
*    none
*********************************************************************/
void
    def_reg_del_cfgdef (const xmlChar *modname,
			const xmlChar *appname,
			const xmlChar *defname,
			const xmlChar *instance,
			int32 cfgid)
{
    def_ownnode_t *own;
    def_cfgnode_t *def;
    uint32         len;

#ifdef DEBUG
    if (!modname || !appname || !defname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
    if (cfgid >= DEF_NUM_CFG_PTRS) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }
#endif

    own = (def_ownnode_t *)find_top_node(DEF_NT_OWNNODE, modname);
    if (!own) {
	return;
    }

    len = (instance) ? xml_strlen(instance) : 0;

    def = find_cfgdef(own, appname, xml_strlen(appname), 
		      defname, xml_strlen(defname), 
		      instance, len, NCX_NT_NONE, NULL);
    if (def) {
	if (cfgid < 0) {
	    /* delete the entire entry */
	    dlq_remove(def);
	    m__free(def);
	} else {
	    /* remove the data pointer for 1 config */
	    def->cfgdata.data[cfgid] = NULL;
	}
    }

}  /* def_reg_del_cfgdef */
#endif

/* END file def_reg.c */
