/*  FILE: xmlns.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
30apr05      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include  "def_reg.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_yangconst
#include  "yangconst.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/
/* max registered namespaces/modules in the database
 * increase to allow more modules loaded at once
 * (this is an arbitrary limit)
 */
#define XMLNS_MAX_NS   4096

#ifdef DEBUG
#define XMLNS_DEBUG 1
#endif

/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

/* INVALID namespace ID */
static xmlns_id_t xmlns_invid = 0;

/* NETCONF namespace ID */
static xmlns_id_t xmlns_ncid = 0;

/* XMLNS namespace ID */
static xmlns_id_t xmlns_nsid = 0;

/* XSD namespace ID */
static xmlns_id_t xmlns_xsid = 0;

/* NETCONF Extensions namespace ID */
static xmlns_id_t xmlns_ncxid = 0;

/* XSD Instance (XSI) namespace ID */
static xmlns_id_t xmlns_xsiid = 0;

/* 1998 XML Namespace ID */
static xmlns_id_t xmlns_xmlid = 0;

/* NETCONF Notifications namespace ID */
static xmlns_id_t xmlns_ncnid = 0;

/* YANG namespace ID */
static xmlns_id_t xmlns_yangid = 0;

/* next ID to allocate */
static xmlns_id_t xmlns_next_id = 1;

/* array of xmlns_t pointers */
static xmlns_t   *xmlns[XMLNS_MAX_NS];

/* module init done flag */
static boolean xmlns_init_done = FALSE;

/********************************************************************
*                                                                   *
*                          M A C R O S                              *
*                                                                   *
*********************************************************************/

/* MACRO valid_id
 *
 * check if the ns_id (I) indicates a valid entry 
 *
 * INPUTS:
 *   I == NS ID to check
 * RETURN
 *   zero if not valid, non-zero if valid
 */
#define valid_id(I) ((I)&&((I)<=XMLNS_MAX_NS)&&(xmlns[(I)-1]) && \
		     (xmlns[(I)-1]->ns_id==(I)))




/********************************************************************
* FUNCTION new_xmlns
* 
* Malloc and init an xmlns_t struct
*
* RETURNS:
*    pointer to new struct, or NULL if memorty error
*********************************************************************/
static xmlns_t *
    new_xmlns (void)
{
    xmlns_t  *rec;

    rec = m__getObj(xmlns_t);
    if (!rec) {
	return NULL;
    }
    memset(rec, 0x0, sizeof(xmlns_t));
    return rec;

} /* new_xmlns */


/********************************************************************
* FUNCTION free_xmlns
* 
* Free an xmlns_t struct
*
* INPUTS:
*    rec == xmlns_t struct to free
*********************************************************************/
static void
    free_xmlns (xmlns_t *rec)
{
    if (rec->ns_pfix) {
	m__free(rec->ns_pfix);
    }
    if (rec->ns_name) {
	m__free(rec->ns_name);
    }
    if (rec->ns_module) {
	m__free(rec->ns_module);
    }
    m__free(rec);

}  /* free_xmlns */


/********************************************************************
*                                                                   *
*                   E X P O R T E D    F U N C T I O N S            *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION xmlns_init
*
* Initialize the module static variables
*
* INPUTS:
*    none
* RETURNS:
*    none
*********************************************************************/
void 
    xmlns_init (void)
{
    int i;

    if (!xmlns_init_done) {

        for (i=0; i<XMLNS_MAX_NS; i++) {
            xmlns[i] = NULL;
        }
        xmlns_init_done = TRUE;
    }

} /* xmlns_init */


/********************************************************************
* FUNCTION xmlns_register_ns
* 
* Register the specified namespace.  Each namespace or prefix
* can only be registered once.  An entry must be removed and
* added back in order to change it.
*
* INPUTS:
*    ns == namespace name
*    pfix == namespace prefix to use (if none provided but needed)
*    modname == name string of module associated with this NS
*
* OUTPUTS:
*    *ns_id contains the ID assigned to the namespace
*
* RETURNS:
*    status, NO_ERR if all okay
*********************************************************************/
status_t 
    xmlns_register_ns (const xmlChar *ns,
		       const xmlChar *pfix,
		       const xmlChar *modname,
		       xmlns_id_t *ns_id)
{
    xmlns_t  *rec;
    char      buff[NCX_MAX_NUMLEN+2];
    uint32    nlen, mlen, plen;
    status_t  res;

#ifdef DEBUG
    if (!ns || !modname || !ns_id) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    nlen = xml_strlen(ns);
    mlen = xml_strlen(modname);

    if (pfix) {
        plen = xml_strlen(pfix);
    } else {
	buff[0] = 'n';
	sprintf(&buff[1], "%d", xmlns_next_id);
	pfix = (xmlChar *)buff;
        plen = xml_strlen(pfix);
    }

#ifdef DEBUG
    if (!nlen || !mlen || !plen) {
	return ERR_NCX_WRONG_LEN;
    }
#endif

    if (!xmlns_next_id || xmlns_next_id > XMLNS_MAX_NS) {
	return ERR_TOO_MANY_ENTRIES;
    }

    /* check duplicate entry */
    if (xmlns_find_ns_by_name(ns)) {
        return ERR_DUP_NS;
    }

    /* all ok - try to malloc the new entry at the end of the list */
    rec = new_xmlns();
    if (!rec) {
	return ERR_INTERNAL_MEM;
    }

    /* copy the prefix string */
    if (pfix) {
	rec->ns_pfix = xml_strdup(pfix);
	if (!rec->ns_pfix) {
	    free_xmlns(rec);
	    return ERR_INTERNAL_MEM;
	}
    }

    /* copy the name namespace URI string */
    rec->ns_name = xml_strdup(ns);
    if (!rec->ns_name) {
	free_xmlns(rec);
	return ERR_INTERNAL_MEM;
    }

    /* copy the module name */
    rec->ns_module = xml_strdup(modname);
    if (!rec->ns_module) {
	free_xmlns(rec);
	return ERR_INTERNAL_MEM;
    }

    /* assign the next XMLNS ID */
    rec->ns_id = xmlns_next_id;

    /* register the entry in the def_reg hash table */
    res = def_reg_add_ns(rec);
    if (res != NO_ERR) {
	free_xmlns(rec);
	return res;
    }

    xmlns[xmlns_next_id-1] = rec;

    /* hack: check if this is one of the cached NS IDs */
    if (!xml_strcmp(ns, NC_URN)) {
	xmlns_ncid = xmlns_next_id;
    } else if (!xml_strcmp(ns, NCX_URN)) {
	xmlns_ncxid = xmlns_next_id;
    } else if (!xml_strcmp(ns, NS_URN)) {
	xmlns_nsid = xmlns_next_id;
    } else if (!xml_strcmp(ns, XSD_URN)) {
	xmlns_xsid = xmlns_next_id;
    } else if (!xml_strcmp(ns, INVALID_URN)) {
	xmlns_invid = xmlns_next_id;
    } else if (!xml_strcmp(ns, XSI_URN)) {
	xmlns_xsiid = xmlns_next_id;
    } else if (!xml_strcmp(ns, XML_URN)) {
	xmlns_xmlid = xmlns_next_id;
    } else if (!xml_strcmp(ns, NCN_URN)) {
	xmlns_ncnid = xmlns_next_id;
    } else if (!xml_strcmp(ns, YANG_URN)) {
	xmlns_yangid = xmlns_next_id;
    }

    log_debug2("\nxmlns_reg: id:%2d mod:%s\turi: %s",
	       rec->ns_id, rec->ns_module, rec->ns_name);


    /* bump the next_id after returning the value used */
    *ns_id = xmlns_next_id++; 
    return NO_ERR;

} /* xmlns_register_ns */


/********************************************************************
* FUNCTION xmlns_get_ns_prefix
*
* Get the prefix for the specified namespace 
*
* INPUTS:
*    ns_id == namespace ID
* RETURNS:
*    pointer to prefix or NULL if bad params
*********************************************************************/
const xmlChar *
    xmlns_get_ns_prefix (xmlns_id_t  ns_id)
{
    if (!valid_id(ns_id)) {
	return (const xmlChar *)"--";
    } else {
	return (const xmlChar *)xmlns[ns_id-1]->ns_pfix;
    }
} /* xmlns_get_ns_prefix */


/********************************************************************
* FUNCTION xmlns_get_ns_name
*
* Get the name for the specified namespace 
*
* INPUTS:
*    ns_id == namespace ID
* RETURNS:
*    pointer to name or NULL if bad params
*********************************************************************/
const xmlChar *
    xmlns_get_ns_name (xmlns_id_t  ns_id)
{
    if (!valid_id(ns_id)) {
	return NULL;
    } else {
	return (const xmlChar *)xmlns[ns_id-1]->ns_name;
    }
} /* xmlns_get_ns_name */



/********************************************************************
* FUNCTION xmlns_find_ns_by_module
*
* Find the NS ID from its module name that registered it
*
* INPUTS:
*    modname == module name string to find
*
* RETURNS:
*    namespace ID or XMLNS_NULL_NS_ID if error
*********************************************************************/
xmlns_id_t 
    xmlns_find_ns_by_module (const xmlChar *modname)
{
    uint32    i;
    xmlns_t  *rec;

#ifdef DEBUG
    if (!modname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return XMLNS_NULL_NS_ID;
    } 
#endif

    for (i=0; i<xmlns_next_id-1; i++) {
	rec = xmlns[i];
	if (rec->ns_module[0]) {
            if (!xml_strcmp(rec->ns_module, modname)) {
                return rec->ns_id;
            }
	}
    }
    return XMLNS_NULL_NS_ID;

} /* xmlns_find_ns_by_module */


/********************************************************************
* FUNCTION xmlns_find_ns_by_prefix
*
* Find the NS ID from its prefix
*
* INPUTS:
*    pfix == pointer to prefix string
* RETURNS:
*    namespace ID or XMLNS_NULL_NS_ID if error
*********************************************************************/
xmlns_id_t 
    xmlns_find_ns_by_prefix (const xmlChar *pfix)
{
    uint32    i;
    xmlns_t  *rec;

#ifdef DEBUG
    if (!pfix) {
	return XMLNS_NULL_NS_ID;
    } 
#endif

    for (i=0; i<xmlns_next_id-1; i++) {
	rec = xmlns[i];
	if (rec->ns_pfix[0]) {
            if (!xml_strcmp(rec->ns_pfix, pfix)) {
                return rec->ns_id;
            }
	}
    }
    return XMLNS_NULL_NS_ID;

} /* xmlns_find_ns_by_prefix */


/********************************************************************
* FUNCTION xmlns_find_ns_by_name
*
* Find the NS ID from its name
*
* INPUTS:
*    name == pointer to name string
* RETURNS:
*    namespace ID or XMLNS_NULL_NS_ID if error
*********************************************************************/
xmlns_id_t 
    xmlns_find_ns_by_name (const xmlChar *name)
{
    xmlns_t  *ns;

#ifdef DEBUG
    if (!name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return XMLNS_NULL_NS_ID;
    } 
#endif

    ns = def_reg_find_ns(name);
    if (ns) {
        return ns->ns_id;
    }

    return XMLNS_NULL_NS_ID;

} /* xmlns_find_ns_by_name */


/********************************************************************
* FUNCTION xmlns_find_ns_by_name_str
*
* Find the NS ID from its name (counted string version)
*
* INPUTS:
*    name == pointer to name string
*    namelen == length of name string
*
* RETURNS:
*    namespace ID or XMLNS_NULL_NS_ID if error
*********************************************************************/
xmlns_id_t 
    xmlns_find_ns_by_name_str (const xmlChar *name,
			       uint32 namelen)
{
    xmlns_t  *ns;
    uint32    i;

#ifdef DEBUG
    if (!name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return XMLNS_NULL_NS_ID;
    } 
    if (!namelen) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return XMLNS_NULL_NS_ID;
    } 
#endif

    for (i=0; i<xmlns_next_id-1; i++) {
	ns = xmlns[i];
	if (ns->ns_name) {
            if (!xml_strncmp(ns->ns_name, name, namelen)) {
                return ns->ns_id;
            }
	}
    }

    return XMLNS_NULL_NS_ID;

} /* xmlns_find_ns_by_name_str */


/********************************************************************
* FUNCTION xmlns_cleanup
*
* Cleanup module static data
*
* INPUTS:
*    none
* RETURNS:
*    none
*********************************************************************/
void 
    xmlns_cleanup (void)
{
    uint32 i;

    if (xmlns_init_done) {
        for (i=0; i<xmlns_next_id-1; i++) {
	    free_xmlns(xmlns[i]);
            xmlns[i] = NULL;
        }
        xmlns_init_done = FALSE;
    }

}  /* xmlns_cleanup */


/********************************************************************
* FUNCTION xmlns_nc_id
*
* Get the ID for the NETCONF namespace or 0 if it doesn't exist
*
* INPUTS:
*    none
* RETURNS:
*    NETCONF NS ID or 0 if not found
*********************************************************************/
xmlns_id_t 
    xmlns_nc_id (void)
{
    return xmlns_ncid;
}  /* xmlns_nc_id */



/********************************************************************
* FUNCTION xmlns_ncx_id
*
* Get the ID for the NETCONF Extensions namespace or 0 if it doesn't exist
*
* INPUTS:
*    none
* RETURNS:
*    NETCONF-X NS ID or 0 if not found
*********************************************************************/
xmlns_id_t 
    xmlns_ncx_id (void)
{
    return xmlns_ncxid;
}  /* xmlns_ncx_id */

/********************************************************************
* FUNCTION xmlns_ns_id
*
* Get the ID for the XMLNS namespace or 0 if it doesn't exist
*
* INPUTS:
*    none
* RETURNS:
*    XMLNS NS ID or 0 if not found
*********************************************************************/
xmlns_id_t 
    xmlns_ns_id (void)
{
    return xmlns_nsid;
}  /* xmlns_ns_id */


/********************************************************************
* FUNCTION xmlns_inv_id
*
* Get the INVALID namespace ID 
*
* INPUTS:
*    none
* RETURNS:
*    INVALID NS ID or 0 if not set yet
*********************************************************************/
xmlns_id_t 
    xmlns_inv_id (void)
{
    return xmlns_invid;
}  /* xmlns_inv_id */


/********************************************************************
* FUNCTION xmlns_xsi_id
*
* Get the ID for the XSD Instance (XSI) namespace or 0 if it doesn't exist
*
* INPUTS:
*    none
* RETURNS:
*    XSI ID or 0 if not found
*********************************************************************/
xmlns_id_t 
    xmlns_xsi_id (void)
{
    return xmlns_xsiid;
}  /* xmlns_xsi_id */


/********************************************************************
* FUNCTION xmlns_xml_id
*
* Get the ID for the 1998 XML namespace or 0 if it doesn't exist
*
* INPUTS:
*    none
* RETURNS:
*    XML ID or 0 if not found
*********************************************************************/
xmlns_id_t 
    xmlns_xml_id (void)
{
    return xmlns_xmlid;
}  /* xmlns_xml_id */


/********************************************************************
* FUNCTION xmlns_ncn_id
*
* Get the ID for the NETCONF Notifications namespace or 0 if it 
* doesn't exist
*
* INPUTS:
*    none
* RETURNS:
*    NCN ID or 0 if not found
*********************************************************************/
xmlns_id_t 
    xmlns_ncn_id (void)
{
    return xmlns_ncnid;
}  /* xmlns_ncn_id */


/********************************************************************
* FUNCTION xmlns_yang_id
*
* Get the ID for the YANG namespace or 0 if it 
* doesn't exist
*
* INPUTS:
*    none
* RETURNS:
*    YANG ID or 0 if not found
*********************************************************************/
xmlns_id_t 
    xmlns_yang_id (void)
{
    return xmlns_yangid;
}  /* xmlns_yang_id */


/********************************************************************
* FUNCTION xmlns_get_module
*
* get the module name of the namespace ID
*
* INPUTS:
*    nsid == namespace ID to check
* RETURNS:
*    none
*********************************************************************/
const xmlChar *
    xmlns_get_module (xmlns_id_t  nsid)
{
    if (!xmlns_init_done) {
        xmlns_init();
        return NULL;
    }
    if (!valid_id(nsid)) {
	return NULL;
    }
    return (const xmlChar *)xmlns[nsid-1]->ns_module;

}  /* xmlns_get_module */


/********************************************************************
* FUNCTION xmlns_xs_id
*
* Get the ID for the XSD namespace or 0 if it doesn't exist
*
* INPUTS:
*    none
* RETURNS:
*    XSD NS ID or 0 if not found
*********************************************************************/
xmlns_id_t 
    xmlns_xs_id (void)
{
    return xmlns_xsid;
}  /* xmlns_xs_id */

/********************************************************************
* FUNCTION xmlns_new_pmap
*
* malloc and initialize a new xmlns_pmap_t struct
*
* INPUTS:
*   buffsize == size of the prefix buffer to allocate 
*               within this pmap (0 == do not malloc yet)
*
* RETURNS:
*    pointer to new struct or NULL if malloc error
*********************************************************************/
xmlns_pmap_t *
    xmlns_new_pmap (uint32 buffsize)
{
    xmlns_pmap_t *pmap;

    pmap = m__getObj(xmlns_pmap_t);
    if (!pmap) {
	return NULL;
    }
    memset(pmap, 0x0, sizeof(xmlns_pmap_t));

    if (buffsize) {
	pmap->nm_pfix = m__getMem(buffsize);
	if (!pmap->nm_pfix) {
	    m__free(pmap);
	    return NULL;
	} else {
	    memset(pmap->nm_pfix, 0x0, buffsize);
	}
    }

    return pmap;

}  /* xmlns_new_pmap */


/********************************************************************
* FUNCTION xmlns_free_pmap
*
* free a xmlns_pmap_t struct
*
* INPUTS:
*   pmap == prefix map struct to free
*
*********************************************************************/
void
    xmlns_free_pmap (xmlns_pmap_t *pmap)
{
#ifdef DEBUG
    if (!pmap) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (pmap->nm_pfix) {
	m__free(pmap->nm_pfix);
    }
    m__free(pmap);

}  /* xmlns_free_pmap */


/********************************************************************
* FUNCTION xmlns_new_qname
*
* malloc and initialize a new xmlns_qname_t struct
*
* RETURNS:
*    pointer to new struct or NULL if malloc error
*********************************************************************/
xmlns_qname_t *
    xmlns_new_qname (void)
{
    xmlns_qname_t *qname;

    qname = m__getObj(xmlns_qname_t);
    if (!qname) {
	return NULL;
    }
    memset(qname, 0x0, sizeof(xmlns_qname_t));
    return qname;

}  /* xmlns_new_qname */


/********************************************************************
* FUNCTION xmlns_free_qname
*
* free a xmlns_qname_t struct
*
* INPUTS:
*   qname == QName struct to free
*
*********************************************************************/
void
    xmlns_free_qname (xmlns_qname_t *qname)
{
#ifdef DEBUG
    if (!qname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    m__free(qname);

}  /* xmlns_free_qname */


/********************************************************************
* FUNCTION xmlns_ids_equal
*
* compare 2 namespace IDs only if they are both non-zero
* and return TRUE if they are equal
*
* INPUTS:
*   ns1 == namespace ID 1
*   ns2 == namespace ID 2
*
* RETURNS:
*  TRUE if equal or both IDs are not zero
*  FALSE if both IDs are non-zero and they are different
*********************************************************************/
boolean
    xmlns_ids_equal (xmlns_id_t ns1,
		     xmlns_id_t ns2)
{
    if (ns1 && ns2) {
	return (ns1==ns2) ? TRUE : FALSE;
    } else {
	return TRUE;
    }

}  /* xmlns_ids_equal */


/* END file xmlns.c */
