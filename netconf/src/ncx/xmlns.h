/*
 * Copyright (c) 2009, Netconf Central, Inc.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
#ifndef _H_xmlns
#define _H_xmlns
/*  FILE: xmlns.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    XML namespace support

    Applications will register namespaces in order to process
    XML requests containing elements in different namespaces,
    as required by the NETCONF protocol and XML 1.0.

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
30-apr-05    abb      Begun.
*/

#include <xmlstring.h>

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/

#define XMLNS_NULL_NS_ID          0

#define XMLNS                     ((const xmlChar *)"xmlns")
#define XMLNS_LEN                 5

#define XMLNS_SEPCH               ':'

/* only compare if both elements have namespaces specified
 * If either one is 'no namespace', then it is considered a match
 */
#define XMLNS_EQ(NS1,NS2)     (((NS1) && (NS2)) && ((NS1)==(NS2)))

/********************************************************************
*                                                                   *
*                             T Y P E S                             *
*                                                                   *
*********************************************************************/

/* integer handle for registered namespaces */
typedef uint32 xmlns_id_t;


/* represents one QName data element */
typedef struct xmlns_qname_t_ {
    xmlns_id_t       nsid;
    const xmlChar   *name;
} xmlns_qname_t;

/* represents one registered namespace */
typedef struct xmlns_t_ {
    xmlns_id_t   ns_id;
    xmlChar     *ns_pfix;
    xmlChar     *ns_name;
    xmlChar     *ns_module;
    struct ncx_module_t_ *ns_mod;
} xmlns_t;


/* represents one namespace prefix mapping */
typedef struct xmlns_pmap_t_ {
    dlq_hdr_t      qhdr;
    xmlns_id_t     nm_id;
    xmlChar       *nm_pfix;
    boolean        nm_topattr;
} xmlns_pmap_t;


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern void 
    xmlns_init (void);

extern void 
    xmlns_cleanup (void);

extern status_t 
    xmlns_register_ns (const xmlChar *ns,
		       const xmlChar *pfix,
		       const xmlChar *modname,
		       void *modptr,
		       xmlns_id_t *ns_id);

extern const xmlChar * 
    xmlns_get_ns_prefix (xmlns_id_t ns_id);

extern const xmlChar * 
    xmlns_get_ns_name (xmlns_id_t ns_id);

extern xmlns_id_t
    xmlns_find_ns_by_module (const xmlChar *modname);

extern xmlns_id_t  
    xmlns_find_ns_by_prefix (const xmlChar *pfix);

extern xmlns_id_t  
    xmlns_find_ns_by_name (const xmlChar *name);

extern xmlns_id_t  
    xmlns_find_ns_by_name_str (const xmlChar *name,
			       uint32 namelen);


/* get the NETCONF NS ID */
extern xmlns_id_t  
    xmlns_nc_id (void);

/* get the NCX NS ID */
extern xmlns_id_t  
    xmlns_ncx_id (void);

/* get the XMLNS NS ID */
extern xmlns_id_t  
    xmlns_ns_id (void);

/* get the INVALID NS ID */
extern xmlns_id_t  
    xmlns_inv_id (void);

/* get the XSD NS ID */
extern xmlns_id_t 
    xmlns_xs_id (void);

/* get the XSI NS ID */
extern xmlns_id_t 
    xmlns_xsi_id (void);

extern xmlns_id_t 
    xmlns_xml_id (void);

/* get the NETCONF Notifications NS ID */
extern xmlns_id_t 
    xmlns_ncn_id (void);

/* get the YANG NS ID */
extern xmlns_id_t 
    xmlns_yang_id (void);

/* get module name that registered this namespace */
extern const xmlChar *
    xmlns_get_module (xmlns_id_t  nsid);

extern void *
    xmlns_get_modptr (xmlns_id_t nsid);

extern void
    xmlns_set_modptrs (const xmlChar *modname,
		       void *modptr);
extern xmlns_pmap_t *
    xmlns_new_pmap (uint32 buffsize);

extern void
    xmlns_free_pmap (xmlns_pmap_t *pmap);

extern xmlns_qname_t *
    xmlns_new_qname (void);

extern void
    xmlns_free_qname (xmlns_qname_t *qname);

extern boolean
    xmlns_ids_equal (xmlns_id_t ns1,
		     xmlns_id_t ns2);

#endif            /* _H_xmlns */
