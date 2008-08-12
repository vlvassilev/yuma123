/*  FILE: xml_msg.c

   XML Message send and receive

   Deals with generic namespace and xmlns optimization
   and tries to keep changing the default namespace so
   most nested elements do not have prefixes

   Deals with the NETCONF requirement that the attributes
   in <rpc> are returned in <rpc-reply> unchanged.  Although
   XML allows the xnmlns prefixes to change, the same prefixes
   are used in the <rpc-reply> that the NMS provided in the <rpc>.


*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
14jan07      abb      begun; split from agt_rpc.c


*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <memory.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include  "def_reg.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_rpc_err
#include  "rpc_err.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_msg
#include  "xml_msg.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define XML_MSG_DEBUG 1 */

#define MAX_PREFIX_TRIES   26


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


/********************************************************************
* FUNCTION add_pmap
*
* Add a prefix mapping entry
* 
* INPUTS:
*    msg  == message to search
*    newpmap == xmlns_pmap_t struct to add
*
* RETURNS:
*   none
*********************************************************************/
static void
    add_pmap (xml_msg_hdr_t *msg,
	      xmlns_pmap_t *newpmap)
{
    xmlns_pmap_t  *pmap;

    /* add the new prefix mapping */
    for (pmap = (xmlns_pmap_t *)dlq_firstEntry(&msg->prefixQ);
	 pmap != NULL;
	 pmap = (xmlns_pmap_t *)dlq_nextEntry(pmap)) {
	if (newpmap->nm_id < pmap->nm_id) {
	    dlq_insertAhead(newpmap, pmap);
	    return;
	}
    }
    dlq_enque(newpmap, &msg->prefixQ);

}  /* add_pmap */


/********************************************************************
* FUNCTION find_prefix
*
* Find the namespace prefix for the specified namespace ID
* 
* INPUTS:
*    msg  == message to search
*    nsid == namespace ID to find
*
* RETURNS:
*   pointer to prefix if found, else NULL if not found
*********************************************************************/
static const xmlChar *
    find_prefix (xml_msg_hdr_t *msg,
		 xmlns_id_t nsid)
{
    const xmlns_pmap_t  *pmap;

    for (pmap = (const xmlns_pmap_t *)dlq_firstEntry(&msg->prefixQ);
	 pmap != NULL;
	 pmap = (const xmlns_pmap_t *)dlq_nextEntry(pmap)) {

	if (pmap->nm_id == nsid) {
	    return (const xmlChar *)pmap->nm_pfix;
	} else if (pmap->nm_id > nsid) {
	    return NULL;
	}
    }
    return NULL;

}  /* find_prefix */


/********************************************************************
* FUNCTION find_prefix2
*
* Find the namespace prefix for the specified namespace ID
* Search only in the prefix overflow Q
*
* INPUTS:
*    msg  == message to search
*    nsid == namespace ID to find
*
* RETURNS:
*   pointer to prefix if found, else NULL if not found
*********************************************************************/
static const xmlChar *
    find_prefix2 (xml_msg_hdr_t *msg,
		  xmlns_id_t nsid)
{
    const xmlns_pmap_t  *pmap;

    for (pmap = (const xmlns_pmap_t *)dlq_firstEntry(&msg->prefix2Q);
	 pmap != NULL;
	 pmap = (const xmlns_pmap_t *)dlq_nextEntry(pmap)) {

	if (pmap->nm_id == nsid) {
	    return (const xmlChar *)pmap->nm_pfix;
	} else if (pmap->nm_id > nsid) {
	    return NULL;
	}
    }
    return NULL;

}  /* find_prefix2 */


/********************************************************************
* FUNCTION find_prefix_val
*
* Find the namespace prefix for the specified namespace ID
* by the prefix value itself
*
* INPUTS:
*    msg  == message to search
*    pfix == prefix to check for
*
* RETURNS:
*   namespace ID in the mapping or 0 if not found
*********************************************************************/
static xmlns_id_t
    find_prefix_val (xml_msg_hdr_t *msg,
		     const xmlChar *pfix)
{
    const xmlns_pmap_t  *pmap;

    for (pmap = (const xmlns_pmap_t *)dlq_firstEntry(&msg->prefixQ);
	 pmap != NULL;
	 pmap = (const xmlns_pmap_t *)dlq_nextEntry(pmap)) {

	if (!xml_strcmp(pmap->nm_pfix, pfix)) {
	    return pmap->nm_id;
	} 
    }
    return XMLNS_NULL_NS_ID;

}  /* find_prefix_val */


/************** E X T E R N A L   F U N C T I O N S  ***************/



/********************************************************************
* FUNCTION xml_msg_init_hdr
*
* Initialize a new xml_msg_hdr_t struct
*
* INPUTS:
*   msg == xml_msg_hdr_t memory to initialize
* RETURNS:
*   none
*********************************************************************/
void
    xml_msg_init_hdr (xml_msg_hdr_t *msg)
{
#ifdef DEBUG
    if (!msg) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    memset(msg, 0x0, sizeof(xml_msg_hdr_t));
    dlq_createSQue(&msg->prefixQ);
    dlq_createSQue(&msg->prefix2Q);
    dlq_createSQue(&msg->errQ);
    msg->withdef = NCX_DEF_WITHDEF;

} /* xml_msg_init_hdr */


/********************************************************************
* FUNCTION xml_msg_clean_hdr
*
* Clean all the memory used by the specified xml_msg_hdr_t
* but do not free the struct itself
*
* INPUTS:
*   msg == xml_msg_hdr_t to clean
* RETURNS:
*   none
*********************************************************************/
void 
    xml_msg_clean_hdr (xml_msg_hdr_t *msg)
{
    xmlns_pmap_t   *pmap;

#ifdef DEBUG
    if (!msg) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    /* clean prefix queue */
    while (!dlq_empty(&msg->prefixQ)) {
	pmap = (xmlns_pmap_t *)dlq_deque(&msg->prefixQ);
	xmlns_free_pmap(pmap);
    }

    /* clean orphan prefix queue */
    while (!dlq_empty(&msg->prefix2Q)) {
	pmap = (xmlns_pmap_t *)dlq_deque(&msg->prefix2Q);
	xmlns_free_pmap(pmap);
    }

    /* clean error queue */
    rpc_err_clean_errQ(&msg->errQ);

    msg->defns = 0;
    msg->cur_defns = 0;
    msg->last_defns = 0;
    msg->last_defpfix[0] = 0;
    msg->withdef = NCX_DEF_WITHDEF;
    msg->withmeta = NCX_DEF_WITHMETA;

} /* xml_msg_clean_hdr */


/********************************************************************
* FUNCTION xml_msg_get_prefix
*
* Find the namespace prefix for the specified namespace ID
* If it is not there then create one
*
* INPUTS:
*    msg  == message to search
*    parent_nsid == parent namespace ID
*    nsid == namespace ID to find
*    xneeded == pointer to xmlns needed flag output value
*
* OUTPUTS:
*   *xneeded == TRUE if the prefix is new and an xmlns
*               decl is needed in the element being generated
*
* RETURNS:
*   pointer to prefix if found, else NULL if not found
*********************************************************************/
const xmlChar *
    xml_msg_get_prefix (xml_msg_hdr_t *msg,
			xmlns_id_t parent_nsid,
			xmlns_id_t nsid,
			boolean  *xneeded)
{
    xmlns_pmap_t   *pmap;
    xmlns_pmap_t   *newpmap;
    const xmlChar  *pfix;
    status_t        res;
    boolean         done;

#ifdef DEBUG
    if (!msg || !xneeded) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    *xneeded = FALSE;

    /* check if the default namespace is requested */
    if (nsid == msg->defns || nsid==0) {
	return NULL;
    }

    /* check if the last default namespace is requested */
    if (nsid == msg->last_defns) {
	return (msg->last_defpfix[0]) ? msg->last_defpfix : NULL;
    }

    /* see if a prefix is already present in the rpc-reply element */
    pfix = find_prefix(msg, nsid);
    if (pfix) {
	return pfix;
    }

    pfix = find_prefix2(msg, nsid);
    if (pfix) {
	if (parent_nsid != nsid) {
	    *xneeded = TRUE;
	}
	return pfix;
    }

    /* need to create a new prefix map and save it for real */
    newpmap = xmlns_new_pmap();
    if (!newpmap) {
	SET_ERROR(ERR_INTERNAL_MEM);
	return NULL;
    }

    /* generate a prefix ID */
    newpmap->nm_id = nsid;
    res = xml_msg_gen_new_prefix(msg, nsid, newpmap->nm_pfix);
    if (res != NO_ERR) {
	SET_ERROR(res);
	xmlns_free_pmap(newpmap);
	return NULL;
    }

    /* add the new prefix mapping to the prefix2Q */
    done = FALSE;
    for (pmap = (xmlns_pmap_t *)dlq_firstEntry(&msg->prefix2Q);
	 pmap != NULL && !done;
	 pmap = (xmlns_pmap_t *)dlq_nextEntry(pmap)) {
	if (nsid < pmap->nm_id) {
	    dlq_insertAhead(newpmap, pmap);
	    done = TRUE;
	}
    }
    if (!done) {
	dlq_enque(newpmap, &msg->prefix2Q);
    }

    /* xmlns directive will be needed for this new prefix */
    if (parent_nsid != nsid) {
	*xneeded = TRUE;
    }
    return newpmap->nm_pfix;

}  /* xml_msg_get_prefix */


/********************************************************************
* FUNCTION xml_msg_gen_new_prefix
*
* Generate a new namespace prefix
* 
* INPUTS:
*    msg  == message to search and generate a prefix for
*    nsid == namespace ID to generate prefix for
*
* OUTPUTS:
*   retbuff is filled in with the new prefix if NO_ERR
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    xml_msg_gen_new_prefix (xml_msg_hdr_t *msg,
			    xmlns_id_t  nsid,
			    xmlChar *retbuff)
{
    const xmlChar      *defpfix;
    xmlChar             startch;
    int32               nlen, i;
    xmlChar             numbuff[NCX_MAX_NUMLEN];


    /* first see if a default prefix exists and is available */
    defpfix = xmlns_get_ns_prefix(nsid);
    if (defpfix && *defpfix) {
	if (!find_prefix_val(msg, defpfix)) {
	    xml_strcpy(retbuff, defpfix);
	    return NO_ERR;
	}
    }

    /* default didn't work so generate a prefix
     * ; sprintf the namespace ID 
     */
    nlen = sprintf((char *)numbuff, "%u", (uint32)nsid);
    if (nlen < 0) {
	return ERR_NCX_INVALID_NUM;
    }

    /* copy the number to the prefix buffer w/ trailing zero */
    for (i=0; i<=nlen; i++) {
	retbuff[i+1] = numbuff[i];
    }

    /* set the start letter in the prefix buffer */
    startch = 'n';

    /* try to generate a unique prefix */
    for (i=0; i<=MAX_PREFIX_TRIES; i++) {
	/* adjust the label value */
	retbuff[0] = startch++;
	if (startch > 'z') {
	    startch = 'a';
	}

	/* check if the prefix is in use */
	if (!find_prefix_val(msg, retbuff)) {
	    return NO_ERR;
	}
    }

    return ERR_NCX_OPERATION_FAILED;

}  /* xml_msg_gen_new_prefix */


/********************************************************************
* FUNCTION xml_msg_build_prefix_map
*
* Build a queue of xmlns_pmap_t records for the current message
* 
* INPUTS:
*    msg == message in progrss
*    attrs == the top-level attrs list (e;g, rpc_in_attrs)
*    addncid == TRUE if a prefix entry for the NC namespace
*                should be added
*            == FALSE if the NC nsid should not be added
*    addncxid == TRUE if a prefix entry for the NCX namespace
*                should be added
*             == FALSE if the NCX nsid should not be added
* OUTPUTS:
*   msg->prefixQ will be populated as needed,
*   could be partially populated if some error returned
*
*   XMLNS Entries for NETCONF and NCX will be added if they 
*   are not present
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xml_msg_build_prefix_map (xml_msg_hdr_t *msg,
			      xml_attrs_t *attrs,
			      boolean addncid,
			      boolean addncxid)
{
    xml_attr_t      *attr;
    xmlns_pmap_t    *pmap, *newpmap;
    xmlns_t         *nsrec;
    xmlns_id_t       ncid, ncxid, nsid;
    uint32           plen;
    boolean          found;
    status_t         res, retres;
    xmlChar          buff[XMLNS_MAX_PREFIX_SIZE+1];

    retres = NO_ERR;

    /* look for any xmlns directives in the attrs list
     * and reuse the prefix that was used in the request
     */
    for (attr = (xml_attr_t *)xml_first_attr(attrs);
	 attr != NULL;
	 attr = (xml_attr_t *)xml_next_attr(attr)) {

	/* make sure this is an XMLNS attribute with or wo a prefix */
	if (xml_strncmp(XMLNS, attr->attr_qname, XMLNS_LEN)) {
	    continue;
	}

	/* find the namespace associated with the prefix */
	nsrec = def_reg_find_ns(attr->attr_val);
	if (!nsrec) {
	    /* this is an unknown namespace; error already
	     * handled, or possibly not error if not used
	     */
	    SET_ERROR(ERR_INTERNAL_VAL);
	    continue;
	}

	/* check if this attribute has a prefix */
	if (attr->attr_qname == attr->attr_name) {
	    /* no prefix in the name so this must be the
	     * default namespace; set the msg->defns
	     */
	    attr->attr_xmlns_ns = nsrec->ns_id;
	    msg->defns = nsrec->ns_id;
	    continue;
	}

	/* make sure there isn't an entry for this namespace already
	 * The manager can enter multiple prefixed xmlns decls
	 * but the agent will use only one mapping per namespace
	 */
	found = FALSE;
	for (pmap = (xmlns_pmap_t *)dlq_firstEntry(&msg->prefixQ);
	     pmap != NULL && !found;
	     pmap = (xmlns_pmap_t *)dlq_nextEntry(pmap)) {
	    if (pmap->nm_id == nsrec->ns_id) {
		found = TRUE;
	    }
	}
	if (found) {
	    continue;
	}

	/* all okay so far; get the prefix len */
	plen = xml_strlen(attr->attr_name);
	plen = min(plen, XMLNS_MAX_PREFIX_SIZE);

	/* get a new prefix map */
	newpmap = xmlns_new_pmap();
	if (!newpmap) {
	    retres = ERR_INTERNAL_MEM;
	    continue;
	}

	/* save the prefix and the xmlns ID */
	xml_strncpy(newpmap->nm_pfix, attr->attr_name, plen);
	newpmap->nm_id = nsrec->ns_id;
	attr->attr_xmlns_ns = nsrec->ns_id;

	/* keep the queue in ascending namespace ID order
	 * to speed up searches a little bit on large rpc-replys
	 */
	add_pmap(msg, newpmap);
    }

    /* now add the basic xmlns directives needed for a NETCONF
     * response, w/ or wo/ NCX extensions
     */
    res = NO_ERR;
    ncid = xmlns_nc_id();
    nsid = xmlns_ns_id();
    ncxid = xmlns_ncx_id();

    /* make sure XMLNS decl for NETCONF is in the map */
    if (addncid &&
	msg->defns != ncid && !find_prefix(msg, ncid)) {
	if (!msg->defns) {
	    /* make NETCONF the default NS */
	    res = xml_add_xmlns_attr(attrs, ncid, NULL);
	    if (res == NO_ERR) {
		msg->defns = ncid;
	    }
	} else {
	    /* add a prefix an xmlns attr for NETCONF */
	    res = xml_msg_gen_new_prefix(msg, ncid, buff);
	    if (res == NO_ERR) {
		res = xml_add_xmlns_attr(attrs, ncid, buff);
	    }
	    if (res == NO_ERR) {
		/* create a new prefix map */
		newpmap = xmlns_new_pmap();
		if (!newpmap) {
		    res = SET_ERROR(ERR_INTERNAL_MEM);
		} else {
		    newpmap->nm_id = ncid;
		    xml_strcpy(newpmap->nm_pfix, buff);
		    add_pmap(msg, newpmap);
		}
	    }
	}
    }
    if (res != NO_ERR) {
	retres = res;
    }

    /* make sure XMLNS decl for NCX is in the map 
     * try even if errors in setting up the NETCONF pmap 
     * Only add this NS decl if there are errors to generate
     */
    if (addncxid &&
	msg->defns != ncxid && !find_prefix(msg, ncxid)) {
	/* add a prefix an xmlns attr for NCX */
	res = xml_msg_gen_new_prefix(msg, ncxid, buff);
	if (res == NO_ERR) {
	    res = xml_add_xmlns_attr(attrs, ncxid, buff);
	}
	if (res == NO_ERR) {
	    /* create a new prefix map */
	    newpmap = xmlns_new_pmap();
	    if (!newpmap) {
		res = SET_ERROR(ERR_INTERNAL_MEM);
	    } else {
		newpmap->nm_id = ncxid;
		xml_strcpy(newpmap->nm_pfix, buff);
		add_pmap(msg, newpmap);
	    }
	}
    }

    return res;

}  /* xml_msg_build_prefix_map */


/********************************************************************
* FUNCTION xml_msg_check_xmlns_attr
*
* Check the default NS and the prefix map in the msg;
* 
* INPUTS:
*    msg == message in progress
*    nsid == namespace ID to check
*    badns == namespace URI of the bad namespace
*             used if the nsid is the INVALID marker
*    attrs == Q to hold the xml_attr_t, if generated

* OUTPUTS:
*   msg->prefixQ will be populated as needed,
*   could be partially populated if some error returned
*  
*   XMLNS attr entry may be added to the attrs Q
*
* RETURNS:
*   status
*********************************************************************/
status_t
    xml_msg_check_xmlns_attr (xml_msg_hdr_t *msg, 
			      xmlns_id_t nsid,
			      const xmlChar *badns,
			      xml_attrs_t  *attrs)

{		      
    const xmlChar *pfix;
    xmlChar        buff[XMLNS_MAX_PREFIX_SIZE+1];
    status_t       res;
    xml_attr_t    *attr;

    /* no namespace is always ok, and if it is the same as the
     * current default, then nothing to do
     */
    if (msg->defns == nsid || !nsid) {
	return NO_ERR;
    }

    /* not the default, see if prefix already set for this NS */
    pfix = find_prefix(msg, nsid);
    if (pfix) {
	return NO_ERR;
    }

    /* check if this namespace ID already covered because the
     * xmlns decl is already present in the attrs list parameter
     */
    for (attr = (xml_attr_t *)dlq_firstEntry(attrs);
	 attr != NULL;
	 attr = (xml_attr_t *)dlq_nextEntry(attr)) {
	if (attr->attr_xmlns_ns == nsid) {
	    return NO_ERR;
	}
    }

    /* not already covered */
    res = xml_msg_gen_new_prefix(msg, nsid, buff);
    if (res == NO_ERR) {
	if (nsid == xmlns_inv_id()) {
	    res = xml_add_inv_xmlns_attr(attrs, nsid, buff, badns);
	} else {
	    res = xml_add_xmlns_attr(attrs, nsid, buff);
	}
    }

    return res;

}  /* xml_msg_check_xmlns_attr */


/********************************************************************
* FUNCTION xml_msg_clean_prefixq
*
* Clean a Q of xmlns_pmap_t structs
*
* INPUTS:
*   prefixQ == queue to clean
*
*********************************************************************/
void
    xml_msg_clean_prefixq (dlq_hdr_t *prefixQ) 
{
    xmlns_pmap_t *pmap;

#ifdef DEBUG
    if (!prefixQ) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(prefixQ)) {
	pmap = (xmlns_pmap_t *)dlq_deque(prefixQ);
	xmlns_free_pmap(pmap);
    }

}  /* xml_msg_clean_prefixq */


/* END file xml_msg.c */
