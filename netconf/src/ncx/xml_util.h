#ifndef _H_xml_util
#define _H_xml_util
/*  FILE: xml_util.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    General Utilities that simplify usage of the libxml2 functions

    - xml_node_t allocation
      - xml_new_node
      - xml_init_node
      - xml_free_node
      - xml_clean_node

    - XmlReader utilities
      - xml_get_reader_from_filespec  (parse debug test documents)
      - xml_get_reader_for_session
      - xml_reset_reader_for_session
      - xml_free_reader
      - xml_get_node_name   (xmlparser enumeration for node type)
      - xml_advance_reader
      - xml_consume_node
      - xml_consume_start_node
      - xml_consume_end_node
      - xml_node_match
      - xml_endnode_match
      - xml_docdone
      - xml_dump_node (debug printf)

    - XML Attribute utilities
      - xml_init_attrs
      - xml_add_attr
      - xml_first_attr
      - xml_next_attr
      - xml_find_attr
      - xml_clean_attrs

    - XmlChar string utilites
      - xml_strlen
      - xml_strcpy
      - xml_strncpy
      - xml_strdup
      - xml_strcat
      - xml_strncat
      - xml_strndup
      - xml_ch_strndup
      - xml_strcmp
      - xml_strncmp
      - xml_isspace
      - xml_isspace_str
      - xml_trim_string
      - xml_copy_clean_string
      - xml_convert_char_entity

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
14-oct-05    abb      begun
2-jan-06     abb      rewrite xml_consume_* API to use simpler 
                      xml_node_t
*/


/* From /usr/include/libxml2/libxml/ */
#include <xmlreader.h>
#include <xmlstring.h>

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#define MAX_CHAR_ENT 8

#define XML_START_MSG ((const xmlChar *)"\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>")

#define XML_START_FILMSG ((const xmlChar *)"<?xml version=\"1.0\" encoding=\"UTF-8\"?>")

/********************************************************************
*								    *
*			     T Y P E S				    *
*								    *
*********************************************************************/

/* queue of xml_attr_t */
typedef dlq_hdr_t  xml_attrs_t;


/* represents one attribute */
typedef struct xml_attr_t_ {
    dlq_hdr_t      attr_qhdr;
    xmlns_id_t     attr_ns;
    xmlns_id_t     attr_xmlns_ns;
    const xmlChar *attr_qname;
    const xmlChar *attr_name;
    xmlChar       *attr_dname;   /* full qualified name if any */
    xmlChar       *attr_val;
} xml_attr_t;


/* only 4 types of nodes returned */
typedef enum xml_nodetyp_t_ {
    XML_NT_NONE,
    XML_NT_EMPTY,                           /* standalone empty node */
    XML_NT_START,                         /* start of a complex type */
    XML_NT_END,                             /* end of a complex type */
    XML_NT_STRING                      /* simple string content node */
} xml_nodetyp_t;

    
/* gather node data into a simple struct 
 * If a simple value is present, the the simval pointer will
 * be non-NULL and point at the value string after it has
 * been trimmed and any character entities translated
 */
typedef struct xml_node_t_ {
    xml_nodetyp_t  nodetyp;
    xmlns_id_t     nsid;
    xmlns_id_t     contentnsid;
    const xmlChar *module;
    const xmlChar *qname;
    const xmlChar *elname;
    const xmlChar *simval;               /* may be cleaned val */
    uint32         simlen;
    xmlChar       *simfree;     /* non-NULL if simval is freed */
    int            depth;
    xml_attrs_t    attrs;
} xml_node_t;


/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/


/******************** XML NODE ALLOC *****************/

extern xml_node_t *
    xml_new_node (void);

extern void
    xml_init_node (xml_node_t *node);

extern void
    xml_clean_node (xml_node_t *node);

extern void
    xml_free_node (xml_node_t *node);


/**************** XMLTextReader APIs ******************/

extern status_t
    xml_get_reader_from_filespec (const char *filespec,
				  xmlTextReaderPtr  *reader);

extern status_t
    xml_get_reader_for_session (xmlInputReadCallback readfn,
				xmlInputCloseCallback closefn,
				void *context,
				xmlTextReaderPtr  *reader);

extern status_t
    xml_reset_reader_for_session (xmlInputReadCallback readfn,
				  xmlInputCloseCallback closefn,
				  void *context,
				  xmlTextReaderPtr  reader);

extern void
    xml_free_reader (xmlTextReaderPtr reader);

extern const char * 
    xml_get_node_name (int nodeval);

extern boolean 
    xml_advance_reader (xmlTextReaderPtr reader);


extern status_t 
    xml_node_match (const xml_node_t *node,
		    xmlns_id_t nsid,
		    const xmlChar *elname,
		    xml_nodetyp_t  nodetyp);

extern status_t
    xml_endnode_match (const xml_node_t *startnode,
		       const xml_node_t *endnode);

extern boolean 
    xml_docdone (xmlTextReaderPtr reader);

extern void
    xml_dump_node (const xml_node_t *node);


/******************** ATTRIBUTE APIs ******************/

extern void
    xml_init_attrs (xml_attrs_t *attrs);

extern xml_attr_t *
    xml_new_attr (void);

extern void
    xml_free_attr (xml_attr_t *attr);

extern status_t
    xml_add_attr (xml_attrs_t *attrs, 
		  xmlns_id_t  ns_id,
		  const xmlChar *attr_name,
		  const xmlChar *attr_val);

extern status_t
    xml_add_qattr (xml_attrs_t *attrs, 
		   xmlns_id_t  ns_id,
		   const xmlChar *attr_qname,
		   uint32  plen,
		   const xmlChar *attr_val);

extern status_t
    xml_add_xmlns_attr (xml_attrs_t *attrs, 
			xmlns_id_t  ns_id,
			const xmlChar *pfix);

extern status_t
    xml_add_inv_xmlns_attr (xml_attrs_t *attrs, 
			    xmlns_id_t  ns_id,
			    const xmlChar *pfix,
			    const xmlChar *nsval);

extern xml_attr_t *
    xml_first_attr (xml_attrs_t  *attrs);

extern xml_attr_t *
    xml_get_first_attr (const xml_node_t  *node);

extern xml_attr_t *
    xml_next_attr (xml_attr_t  *attr);

extern xml_attr_t *
    xml_find_attr (xml_node_t   *node,
		   xmlns_id_t    nsid,                  
		   const xmlChar *attrname);

extern xml_attr_t *
    xml_find_attr_q (xml_attrs_t *attrs,
		     xmlns_id_t        nsid,
		     const xmlChar    *attrname);

extern const xml_attr_t *
    xml_find_ro_attr (const xml_node_t *node,
		   xmlns_id_t        nsid,
		      const xmlChar    *attrname);

extern void
    xml_clean_attrs (xml_attrs_t  *attrs);


/******************** XMLCHAR STRING APIs ******************/

extern uint32
    xml_strlen (const xmlChar *str);

/* get length and check if any whitespace at the same time */
extern uint32 
    xml_strlen_sp (const xmlChar *str,
		   boolean *sp);

extern uint32
    xml_strcpy (xmlChar *copyTo, 
		const xmlChar *copyFrom);

extern uint32
    xml_strncpy (xmlChar *copyTo, 
		 const xmlChar *copyFrom,
		 uint32  maxlen);

extern xmlChar * 
    xml_strdup (const xmlChar *copyFrom);

extern xmlChar * 
    xml_strcat (xmlChar *appendTo, 
		const xmlChar *appendFrom);

extern xmlChar * 
    xml_strncat (xmlChar *appendTo,
		 const xmlChar *appendFrom,
		 uint32 maxlen);

extern xmlChar * 
    xml_strndup (const xmlChar *copyFrom, 
		 uint32 maxlen);

extern char * 
    xml_ch_strndup (const char *copyFrom, 
		    uint32 maxlen);

extern int 
    xml_strcmp (const xmlChar *s1, 
		const xmlChar *s2);

extern int 
    xml_strncmp (const xmlChar *s1, 
		 const xmlChar *s2,
		 uint32 maxlen);

extern boolean
    xml_isspace (uint32 ch);

extern boolean
    xml_isspace_str (const xmlChar *str);


/************************* NCX STRING APIs **************************/

extern const xmlChar *
    xml_trim_string (const xmlChar *str, 
		     uint32 *len);

extern int 
    xml_strcmp_nosp (const xmlChar *s1, 
		     const xmlChar *s2);

extern xmlChar *
    xml_copy_clean_string (const xmlChar *str);

extern xmlChar
    xml_convert_char_entity (const xmlChar *str, 
			     uint32 *used);

/******************* SPECIAL NAMESPACE APIs ************************/

extern status_t
    xml_check_ns (xmlTextReaderPtr reader,
		  const xmlChar   *elname,
		  xmlns_id_t       *id,
		  uint32           *pfix_len,
		  const xmlChar   **badns);

extern void
    xml_check_qname_content (xmlTextReaderPtr reader,
			     xml_node_t      *node);


#endif	    /* _H_xml_util */
