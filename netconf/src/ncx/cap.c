/*  FILE: cap.c

   The capabilities module constructs two versions/
  
   The cap_list_t version is the internal struct used 
   by the agent or manager.

   The val_value_t version is used to cache the capabilities
   that will actually be used within an rpc_msg_t when the
   manager or agent actually sends a hello message.

   Debugging and schema-discovery module support is also
   provided.

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
28apr05      abb      begun
20sep07      abb      add support for schema-discovery data model

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

#ifndef _H_cap
#include  "cap.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xmlns
#include  "xmlns.h"
#endif

#ifndef _H_xml_util
#include  "xml_util.h"
#endif

#ifndef _H_xml_val
#include  "xml_val.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* controls extra attributes in the <capability> element */
/* #define USE_EXTENDED_HELLO 0 */

#define URL_ORG    (const xmlChar *)"http://netconfcentral.com/"
#define URL_START  (const xmlChar *)"xsd/"


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

/* hardwired list of NETCONF v1.0 capabilities */
static cap_stdrec_t stdcaps[] =
{
  { CAP_STDID_V1, CAP_BIT_V1, CAP_NAME_V1 },
  { CAP_STDID_WRITE_RUNNING, CAP_BIT_WR_RUN, CAP_NAME_WR_RUN },
  { CAP_STDID_CANDIDATE, CAP_BIT_CANDIDATE, CAP_NAME_CANDIDATE },
  { CAP_STDID_CONF_COMMIT, CAP_BIT_CONF_COMMIT, CAP_NAME_CONF_COMMIT },
  { CAP_STDID_ROLLBACK_ERR, CAP_BIT_ROLLBACK_ERR, CAP_NAME_ROLLBACK_ERR },
  { CAP_STDID_VALIDATE, CAP_BIT_VALIDATE, CAP_NAME_VALIDATE },
  { CAP_STDID_STARTUP, CAP_BIT_STARTUP, CAP_NAME_STARTUP },
  { CAP_STDID_URL, CAP_BIT_URL, CAP_NAME_URL },
  { CAP_STDID_XPATH, CAP_BIT_XPATH, CAP_NAME_XPATH },
  { CAP_STDID_LAST_MARKER, 0x0, "" }    /* end-of-list marker */
};


/********************************************************************
* FUNCTION free_cap
*
* Clean and free the fields in a pre-allocated cap_rec_t struct
*
* INPUTS:
*    cap == struct to free
* RETURNS:
*    none, silent programming errors ignored 
*********************************************************************/
static void 
    free_cap (cap_rec_t *cap)
{
    if (cap->cap_uri) {
	m__free(cap->cap_uri);
    }
    if (cap->cap_mod_malloc) {
	m__free(cap->cap_mod_malloc);
    }
    m__free(cap);

}  /* free_cap */


/************** E X T E R N A L   F U N C T I O N S ************/


/********************************************************************
* FUNCTION cap_init_caplist
*
* Initialize the fields in a pre-allocated cap_list_t struct
*
* INPUTS:
*    caplist == struct to initialize
* RETURNS:
*    status, should always be NO_ERR
*********************************************************************/
cap_list_t *
    cap_new_caplist (void)
{
    cap_list_t *caplist;

    caplist = m__getObj(cap_list_t);
    if (caplist) {
	cap_init_caplist(caplist);
    }
    return caplist;

}  /* cap_new_caplist */


/********************************************************************
* FUNCTION cap_init_caplist
*
* Initialize the fields in a pre-allocated cap_list_t struct
*
* INPUTS:
*    caplist == struct to initialize
* RETURNS:
*    status, should always be NO_ERR
*********************************************************************/
void
    cap_init_caplist (cap_list_t *caplist) 
{
#ifdef DEBUG
    if (!caplist) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif
    
    memset(caplist, 0x0, sizeof(cap_list_t));
    caplist->cap_std = 0;
    caplist->cap_protos = NULL;
    dlq_createSQue(&caplist->capQ);

}  /* cap_init_caplist */


/********************************************************************
* FUNCTION cap_clean_caplist
*
* Clean the fields in a pre-allocated cap_list_t struct
* Memory for caplist not deallocated -- this just cleans fields
*
* INPUTS:
*    caplist == struct to clean
* RETURNS:
*    none, silent programming errors ignored 
*********************************************************************/
void 
    cap_clean_caplist (cap_list_t *caplist)
{
    cap_rec_t  *cap;

#ifdef DEBUG
    if (!caplist) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    caplist->cap_std = 0;
    if (caplist->cap_protos) {
	m__free(caplist->cap_protos);
	caplist->cap_protos = NULL;
    }

    /* drain the capability Q and free the memory */
    cap = (cap_rec_t *)dlq_deque(&caplist->capQ);
    while (cap != NULL) {
	free_cap(cap);
	cap = (cap_rec_t *)dlq_deque(&caplist->capQ);
    }

}  /* cap_clean_caplist */


/********************************************************************
* FUNCTION cap_free_caplist
*
* Clean the fields in a pre-allocated cap_list_t struct
* Then free the caplist memory
*
* INPUTS:
*    caplist == struct to free
*
*********************************************************************/
void 
    cap_free_caplist (cap_list_t *caplist)
{
#ifdef DEBUG
    if (!caplist) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    cap_clean_caplist(caplist);
    m__free(caplist);

}  /* cap_free_caplist */


/********************************************************************
* FUNCTION cap_add_std
*
* Add a standard protocol capability to the list
*
* INPUTS:
*    caplist == capability list that will contain the standard cap 
*    capstd == the standard capability ID
* RETURNS:
*    status, should always be NO_ERR
*********************************************************************/
status_t 
    cap_add_std (cap_list_t *caplist, cap_stdid_t   capstd)
{
#ifdef DEBUG
    if (!caplist) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (capstd < CAP_STDID_LAST_MARKER) {
	m__setbit(caplist->cap_std, stdcaps[capstd].cap_bitnum); 
	return NO_ERR;
    } else {
        return ERR_NCX_WRONG_VAL;
    }
}  /* cap_add_std */


/********************************************************************
* FUNCTION cap_add_stdval
*
* Add a standard protocol capability to the list (val_value_t version)
*
* INPUTS:
*    caplist == capability list that will contain the standard cap 
*    capstd == the standard capability ID
* OUTPUTS:
*    status
*********************************************************************/
status_t
    cap_add_stdval (val_value_t *caplist,
		    cap_stdid_t   capstd)
{
    val_value_t  *capval;
    xmlChar      *str, *p;
    const xmlChar  *pfix, *cap;
    uint32        len;

#ifdef DEBUG
    if (!caplist || capstd >= CAP_STDID_LAST_MARKER) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* setup the string */
    if (capstd==CAP_STDID_V1) {
	pfix = CAP_BASE_URN;
	cap = NULL;
	len = xml_strlen(pfix);
    } else {
	pfix = CAP_URN;
	cap = stdcaps[capstd].cap_name;
	len = xml_strlen(pfix) + xml_strlen(cap);
    }

    /* make the string */
    str = m__getMem(len+1);
    if (!str) {
	return ERR_INTERNAL_MEM;
    } else {
	memset(str, 0x0, len+1);
    }

    /* concat the capability name if not the base string */
    p = str;
    p += xml_strcpy(str, pfix);
    if (cap) {
	xml_strcpy(p, cap);
    }

    /* make the capability element */
    capval = xml_val_new_string(NCX_EL_CAPABILITY,
				xmlns_nc_id(), str);
    if (!capval) {
	m__free(str);
	return ERR_INTERNAL_MEM;
    }

    val_add_child(capval, caplist);
    return NO_ERR;

}  /* cap_add_stdval */


/********************************************************************
* FUNCTION cap_add_std_string
*
* Add a standard protocol capability to the list by URI string
*
* INPUTS:
*    caplist == capability list that will contain the standard cap 
*    uri == the string holding the capability URI
*
* RETURNS:
*    status, NO_ERR if valid STD capability 
*    ERR_NCX_SKIPPED if this is not a standard capability
*    any other result is a non-recoverable error
*********************************************************************/
status_t 
    cap_add_std_string (cap_list_t *caplist, 
			const xmlChar *uri)
{
    const xmlChar *str;
    uint32         caplen;
    cap_stdid_t    stdid;

#ifdef DEBUG
    if (!caplist || !uri) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    caplen = xml_strlen(CAP_URN);

    /* the base capability is a different form than the rest */
    if (!xml_strcmp(uri, CAP_BASE_URN)) {
	return cap_add_std(caplist, CAP_STDID_V1);
    } else if (!xml_strncmp(uri, CAP_URN, caplen)) {
	/* matched the standard capability prefix string;
	 * get the suffix with the capability name and version 
	 */
	str = uri + caplen;

	/* go through the standard capability suffix strings */
	for (stdid=CAP_STDID_WRITE_RUNNING;
	     stdid < CAP_STDID_LAST_MARKER; stdid++) {
	    if (!xml_strcmp(str, stdcaps[stdid].cap_name)) {
		return cap_add_std(caplist, stdid);
	    }
	}
    }
    return ERR_NCX_SKIPPED;

}  /* cap_add_std_string */


/********************************************************************
* FUNCTION cap_add_module_string
*
* Add a standard protocol capability to the list by URI string
*
* INPUTS:
*    caplist == capability list that will contain the standard cap 
*    uri == the URI string holding the capability identifier
*
* RETURNS:
*    status, NO_ERR if valid STD capability 
*    ERR_NCX_SKIPPED if this is not a module capability
*    any other result is a non-recoverable error
*********************************************************************/
status_t 
    cap_add_module_string (cap_list_t *caplist, 
			   const xmlChar *uri)
{
    cap_rec_t     *cap;
    const xmlChar *p;
    uint32         len;

#ifdef DEBUG
    if (!caplist || !uri) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* get capability prefix string length */
    len = xml_strlen(CAP_MODURN);

    /* the base capability is a different form than the rest */
    if (xml_strncmp(uri, CAP_MODURN, len)) {
	return ERR_NCX_SKIPPED;
    }

    cap = m__getObj(cap_rec_t);
    if (cap == NULL) {
        return ERR_INTERNAL_MEM;
    }
    memset(cap, 0x0, sizeof(cap_rec_t));

    cap->cap_subj = CAP_SUBJTYP_DM;
    cap->cap_uri = xml_strdup(uri);
    if (!cap->cap_uri) {
	free_cap(cap);
        return ERR_INTERNAL_MEM;
    }

    /* parse out the module name
     * get the 'p' var to point at the start of the owner
     */
    p = cap->cap_uri;
    p += len;
    cap->cap_mod = p;

    /* find the end of the module name */
    while (*p && *p != CAP_SEP_CH) {
	p++;
    }

    /* set the length and malloc flag */
    cap->cap_mod_len = (uint32)(p - cap->cap_mod);
    cap->cap_mod_malloc = NULL;

    /* check if stopped on a separator char,
     * if so, the version is expected as the last component
     */
    if (*p) {
	xml_strncpy(cap->cap_ver, ++p, CAP_VERSION_LEN);
    }

    dlq_enque(cap, &caplist->capQ);
    return NO_ERR;

}  /* cap_add_module_string */


/********************************************************************
* FUNCTION cap_add_url
*
* Add the #url capability to the list
*
* INPUTS:
*    caplist == capability list that will contain the standard cap 
*    capstd == the standard capability ID
* RETURNS:
*    status, should always be NO_ERR
*********************************************************************/
status_t 
    cap_add_url (cap_list_t *caplist, 
		 const xmlChar *proto_list)
{
#ifdef DEBUG
    if (!caplist || !proto_list) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    m__setbit(caplist->cap_std, stdcaps[CAP_STDID_URL].cap_bitnum);
    caplist->cap_protos = xml_strdup(proto_list);
    if (caplist->cap_protos==NULL) {
        return ERR_INTERNAL_MEM;
    }
    return NO_ERR;

} /* cap_add_url */


/********************************************************************
* FUNCTION cap_add_mod
*
* Add a module capability to the list
*
* INPUTS:
*    caplist == capability list that will contain the module caps
*    modname == module name
*    modversion == module version string (MAY BE NULL)
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    cap_add_mod (cap_list_t *caplist, 
		 const xmlChar *modname,
		 const xmlChar *modversion)
{
    xmlChar      *str;
    cap_rec_t    *cap;

#ifdef DEBUG
    if (!caplist || !modname) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* construct the module URN string */
    str = cap_make_mod_urn(modname, modversion);
    if (!str) {
	return ERR_INTERNAL_MEM;
    }

    /* malloc a new capability record */
    cap = m__getObj(cap_rec_t);
    if (cap == NULL) {
	m__free(str);
        return ERR_INTERNAL_MEM;
    }
    memset(cap, 0x0, sizeof(cap_rec_t));

    /* fill in the cap_rec_t struct */
    cap->cap_subj = CAP_SUBJTYP_DM;
    cap->cap_uri = str;

    cap->cap_mod_malloc = xml_strdup(modname);
    if (!cap->cap_mod_malloc) {
	free_cap(cap);
	return ERR_INTERNAL_MEM;
    } else {
	cap->cap_mod = cap->cap_mod_malloc;
	cap->cap_mod_len = xml_strlen(cap->cap_mod);
    }

    if (modversion) {
	xml_strncpy(cap->cap_ver, modversion, CAP_VERSION_LEN);
    }

    dlq_enque(cap, &caplist->capQ);
    return NO_ERR;

}  /* cap_add_mod */ 


/********************************************************************
* FUNCTION cap_add_ent
*
* Add an enterprise capability to the list
*
* INPUTS:
*    caplist == capability list that will contain the module caps
*    uristr == URI string to add
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    cap_add_ent (cap_list_t *caplist, 
		 const xmlChar *uristr)
{
    cap_rec_t    *cap;

#ifdef DEBUG
    if (!caplist || !uristr) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* malloc a new capability record */
    cap = m__getObj(cap_rec_t);
    if (cap == NULL) {
        return ERR_INTERNAL_MEM;
    }
    memset(cap, 0x0, sizeof(cap_rec_t));

    /* fill in the cap_rec_t struct */
    cap->cap_subj = CAP_SUBJTYP_OTHER;
    cap->cap_uri = xml_strdup(uristr);
    if (!cap->cap_uri) {
	free_cap(cap);
	return ERR_INTERNAL_MEM;
    }

    dlq_enque(cap, &caplist->capQ);
    return NO_ERR;

}  /* cap_add_mod */ 


/********************************************************************
* FUNCTION cap_add_modval
*
* Add a module capability to the list (val_value_t version)
*
* INPUTS:
*    caplist == capability list that will contain the enterprise cap 
*    modname == module name
*    modversion == module version string
*
* RETURNS:
*    status
*********************************************************************/
status_t 
    cap_add_modval (val_value_t *caplist, 
		    const xmlChar *modname,
		    const xmlChar *modversion)
{
    xmlChar      *str;
    val_value_t  *capval;

#ifdef DEBUG
    if (!caplist || !modname) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* construct the module URN string */
    str = cap_make_mod_urn(modname, modversion);
    if (!str) {
	return ERR_INTERNAL_MEM;
    }

    /* make the capability element */
    capval = xml_val_new_cstring(NCX_EL_CAPABILITY,
				 xmlns_nc_id(), str);
    if (!capval) {
	m__free(str);
	return ERR_INTERNAL_MEM;
    }

    val_add_child(capval, caplist);

    m__free(str);

    return NO_ERR;

}  /* cap_add_modval */ 


/********************************************************************
* FUNCTION cap_make_mod_urn
*
* Construct and malloc a module capability URN string
*
* INPUTS:
*    caplist == capability list that will contain the module caps
*    modname == module name
*    modversion == module version string (MAY BE NULL)
*
* RETURNS:
*    status
*********************************************************************/
xmlChar *
    cap_make_mod_urn (const xmlChar *modname,
		      const xmlChar *modversion)
{
    uint32        len;
    xmlChar      *str, *p;

#ifdef DEBUG
    if (!modname) {
	SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* construct the module URN string */
    if (modversion) {
	len = xml_strlen(CAP_MODURN) + 
	    xml_strlen(modname) + 1 + xml_strlen(modversion);
    } else {
	len = xml_strlen(CAP_MODURN) + xml_strlen(modname);
    }
    str = m__getMem(len+1);
    if (!str) {
	return NULL;
    }
    p = str;
    p += xml_strcpy(p, CAP_MODURN);
    p += xml_strcpy(p, modname);
    if (modversion) {
	*p++ = CAP_SEP_CH;
	xml_strcpy(p, modversion);
    }

    return str;

}  /* cap_make_mod_urn */ 


/********************************************************************
* FUNCTION cap_make_mod_url
*
* Construct and malloc a module schema URL string
*
* INPUTS:
*    caprec == cap_rec_t for the module capability
*
* RETURNS:
*    status
*********************************************************************/
xmlChar *
    cap_make_mod_url (const cap_rec_t *caprec)
{
    uint32        len;
    xmlChar      *str, *p;

#ifdef DEBUG
    if (!caprec) {
	SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* http://netconfcentral.com/xsd/module.xsd */
    len = xml_strlen(URL_ORG) + 
	xml_strlen(URL_START) + caprec->cap_mod_len + 4;

    str = m__getMem(len+1);
    if (!str) {
	return NULL;
    }

    p = str;
    p += xml_strcpy(p, URL_ORG);
    p += xml_strcpy(p, URL_START);
    p += xml_strncpy(p, caprec->cap_mod, caprec->cap_mod_len);
    p += xml_strcpy(p, (const xmlChar *)".xsd");

    return str;

}  /* cap_make_mod_url */ 


/********************************************************************
* FUNCTION cap_std_set
*
* fast search of standard protocol capability set
*
* INPUTS:
*    caplist == capability list to check
*    capstd == the standard capability ID
* RETURNS:
*    TRUE if indicated std capability is set, FALSE if not
*********************************************************************/
boolean 
    cap_std_set (const cap_list_t *caplist, 
		 cap_stdid_t capstd)
{
    if (!caplist) {
        return FALSE;
    }
    if (capstd < CAP_STDID_LAST_MARKER) {
	return (m__bitset(caplist->cap_std, stdcaps[capstd].cap_bitnum))
		? TRUE : FALSE;
    } else {
	return FALSE;
    }
}  /* cap_std_set */


/********************************************************************
* FUNCTION cap_set
*
* linear search of capability list, will check for std uris as well
*
* INPUTS:
*    caplist == capability list to check
*    capuri == the capability URI to find
* RETURNS:
*    TRUE if indicated capability is set, FALSE if not
*********************************************************************/
boolean 
    cap_set (const cap_list_t *caplist, 
	     xmlChar *capuri) 
{
    xmlChar    *str;
    cap_rec_t  *cap;
    int         i;
    uint32      len;

    if (!caplist || !capuri) {
        return FALSE;
    }

    /* check if this is the NETCONF V1 Base URN capability */
    if (!xml_strcmp(capuri, NC_URN)) {
	return (m__bitset(caplist->cap_std, 
			  stdcaps[CAP_STDID_V1].cap_bitnum)) ?
	    TRUE : FALSE;
    }

    /* check if this is a NETCONF standard capability */
    len = xml_strlen(CAP_URN);
    if ((xml_strlen(capuri) > len+1) && !xml_strncmp(capuri, 
	     (const xmlChar *) CAP_URN, len)) {

	/* set str to the 'capability-name:version-number' */
	str = &capuri[len];    

	/* check all the capability names */
	for (i=1; i<CAP_STDID_LAST_MARKER; i++) {
	    if (!xml_strcmp(str, stdcaps[i].cap_name)) {
		return (m__bitset(caplist->cap_std, 
				  stdcaps[i].cap_bitnum)) ?
		    TRUE : FALSE;
	    }
	}
    }

    /* check the enterprise capability queue */
    for (cap=(cap_rec_t *)dlq_firstEntry(&caplist->capQ);
	 cap != NULL; cap=(cap_rec_t *)dlq_nextEntry(cap)) {
	if (!xml_strcmp(cap->cap_uri, capuri)) {
	    return TRUE;
	}
    }
    return FALSE;

}  /* cap_set */


/********************************************************************
* FUNCTION cap_get_protos
*
* get the #url capability protocols list if it exists
*
* INPUTS:
*    caplist == capability list to check
* RETURNS:
*    pointer to protocols string if any, or NULL if not
*********************************************************************/
const xmlChar *
    cap_get_protos (cap_list_t *caplist)
{
#ifdef DEBUG
    if (!caplist) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const xmlChar *)caplist->cap_protos;

} /* cap_get_protos */


/********************************************************************
* FUNCTION cap_dump_stdcaps
*
* Printf the standard protocol capabilities list
*
* INPUTS:
*    caplist == capability list to print
*
*********************************************************************/
void
    cap_dump_stdcaps (const cap_list_t *caplist)
{
    cap_stdid_t  capid;

#ifdef DEBUG
    if (!caplist) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (cap_std_set(caplist, CAP_STDID_V1)) {
	log_write("\n   Protocol Version: RFC 4741");
    } else {
	log_write("\n   Protocol Version: Unknown");
    }

    for (capid = CAP_STDID_WRITE_RUNNING;
	 capid < CAP_STDID_LAST_MARKER;	 capid++) {

	if (cap_std_set(caplist, capid)) {
	    log_write("\n   %s", stdcaps[capid].cap_name);
	}
    }

} /* cap_dump_stdcaps */


/********************************************************************
* FUNCTION cap_dump_modcaps
*
* Printf the standard data model module capabilities list
*
* INPUTS:
*    caplist == capability list to print
*    checkdb == TRUE if the local DB should be checked
*               and any missing modules or difference in 
*               versions is reported
*********************************************************************/
void
    cap_dump_modcaps (const cap_list_t *caplist,
		      boolean checkdb)
{
    const cap_rec_t *cap;
    boolean anycaps;

#ifdef DEBUG
    if (!caplist) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    anycaps = FALSE;

    for (cap = (cap_rec_t *)dlq_firstEntry(&caplist->capQ);
	 cap != NULL;
	 cap = (cap_rec_t *)dlq_nextEntry(cap)) {

	if (cap->cap_subj != CAP_SUBJTYP_DM) {
	    continue;
	}

	anycaps = TRUE;
	
	log_write("\n   ");
	if (cap->cap_mod) {
	    log_write("%s", cap->cap_mod);
	} else {
	    log_write("%s", cap->cap_uri);
	}

	if (checkdb) {
	    ;
	}
    }

    if (!anycaps) {
	log_write("\n   None");
    }

} /* cap_dump_modcaps */


/********************************************************************
* FUNCTION cap_first_modcap
*
* Get the first module capability in the list
*
* INPUTS:
*    caplist == capability list to check
*
* RETURNS:
*  pointer to first record, to use for next record
*  NULL if no first record
*********************************************************************/
const cap_rec_t *
    cap_first_modcap (const cap_list_t *caplist)
{
    const cap_rec_t *cap;

#ifdef DEBUG
    if (!caplist) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (cap = (cap_rec_t *)dlq_firstEntry(&caplist->capQ);
	 cap != NULL;
	 cap = (cap_rec_t *)dlq_nextEntry(cap)) {

	if (cap->cap_subj != CAP_SUBJTYP_DM) {
	    continue;
	}
	return cap;
    }
    return NULL;

} /* cap_first_modcap */


/********************************************************************
* FUNCTION cap_next_modcap
*
* Get the next module capability in the list
*
* INPUTS:
*    curcap == current mod_cap entry
*
* RETURNS:
*  pointer to next record
*  NULL if no next record
*********************************************************************/
const cap_rec_t *
    cap_next_modcap (const cap_rec_t *curcap)
{
    const cap_rec_t *cap;

#ifdef DEBUG
    if (!curcap) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (cap = (cap_rec_t *)dlq_nextEntry(curcap);
	 cap != NULL;
	 cap = (cap_rec_t *)dlq_nextEntry(cap)) {

	if (cap->cap_subj != CAP_SUBJTYP_DM) {
	    continue;
	}
	return cap;
    }
    return NULL;

} /* cap_next_modcap */


/********************************************************************
* FUNCTION cap_split_modcap
*
* Split the modcap string into 3 parts
*
* INPUTS:
*    cap ==  capability rec to parse
*    owner == address of return owner name
*    ownerlen == address of return owner name length
*    module == address of return module name
*    modlen == address of return module name length
*    version == address of return module version string
*
* OUTPUTS:
*    *owner == return owner name
*    *ownerlen == return module name length
*    *module == return module name
*    *modlen == return module name length
*    *version == return module version string
*
* RETURNS:
*    status
*********************************************************************/
void
    cap_split_modcap (const cap_rec_t *cap,
		      const xmlChar **module,
		      uint32 *modlen,
		      const xmlChar **version)
{

#ifdef DEBUG
    if (!cap || !module || !modlen || !version) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    *module = cap->cap_mod;
    *modlen = cap->cap_mod_len;
    *version = cap->cap_ver;

} /* cap_split_modcap */


/********************************************************************
* FUNCTION cap_dump_entcaps
*
* Printf the enterprise capabilities list
*
* INPUTS:
*    caplist == capability list to print
*
*********************************************************************/
void
    cap_dump_entcaps (const cap_list_t *caplist)
{
    const cap_rec_t *cap;
    boolean  anycaps;

#ifdef DEBUG
    if (!caplist) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    anycaps = FALSE;

    for (cap = (cap_rec_t *)dlq_firstEntry(&caplist->capQ);
	 cap != NULL;
	 cap = (cap_rec_t *)dlq_nextEntry(cap)) {

	if (cap->cap_subj != CAP_SUBJTYP_DM) {
	    anycaps = TRUE;
	    log_write("\n   %s", cap->cap_uri);
	}
    }

    if (!anycaps) {
	log_write("\n   None");
    }

} /* cap_dump_entcaps */


/* END file cap.c */
