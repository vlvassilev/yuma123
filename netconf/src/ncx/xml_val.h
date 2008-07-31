#ifndef _H_xml_val
#define _H_xml_val

/*  FILE: xml_val.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

  Utility functions for creating value structs
 
*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
24-nov-06    abb      Begun; split from xsd.c
16-jan-07    abb      Moved from ncxdump/xml_val_util.h
*/

#include <xmlstring.h>

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

/* Build output value: add child node to a struct node */

extern xmlChar *
    xml_val_make_qname (xmlns_id_t  nsid,
			const xmlChar *name);

extern uint32
    xml_val_qname_len (xmlns_id_t  nsid,
		       const xmlChar *name);

extern uint32
    xml_val_sprintf_qname (xmlChar *buff,
			   uint32 bufflen,
			   xmlns_id_t  nsid,
			   const xmlChar *name);

extern status_t
    xml_val_add_attr (const xmlChar *name,
		      xmlns_id_t nsid,
		      xmlChar *attrval,
		      val_value_t *val);

extern status_t
    xml_val_add_cattr (const xmlChar *name,
		       xmlns_id_t nsid,
		       const xmlChar *cattrval,
		       val_value_t *val);

extern val_value_t *
    xml_val_new_struct (const xmlChar *name,
			xmlns_id_t     nsid);

extern val_value_t *
    xml_val_new_string (const xmlChar *name,
			xmlns_id_t     nsid,
			xmlChar *strval);

extern val_value_t *
    xml_val_new_cstring (const xmlChar *name,
			 xmlns_id_t     nsid,
			 const xmlChar *strval);

extern val_value_t *
    xml_val_new_flag (const xmlChar *name,
		      xmlns_id_t     nsid);

#endif	    /* _H_xml_val */
