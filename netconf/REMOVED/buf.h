#ifndef _H_buf
#define _H_buf
/*  FILE: buf.h
*********************************************************************
*                                                                   *
*                         P U R P O S E                             *
*                                                                   *
*********************************************************************

    Simplified buffer control for generating XML-based PDUs

*********************************************************************
*                                                                   *
*                   C H A N G E         H I S T O R Y               *
*                                                                   *
*********************************************************************

date             init     comment
----------------------------------------------------------------------
06-apr-05    abb      Begun.
*/

#include <xmlstring.h>

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                         C O N S T A N T S                         *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                                    T Y P E S                      *
*                                                                   *
*********************************************************************/

/* buf_write_tag_buffer 'tag' parameter values */
typedef enum buf_tagtyp_t_ {
    BUF_TAG_NONE,
    BUF_START_TAG,
    BUF_END_TAG
} buf_tagtyp_t;


/* PDU buffer control header struct */
typedef struct buf_buffer_t_ {
    xmlChar       *buff;
    xmlChar       *bptr;
    uint32         bufflen;
    uint32         indent;
} buf_buffer_t;


/********************************************************************
*                                                                   *
*                        F U N C T I O N S                          *
*                                                                   *
*********************************************************************/

extern status_t buf_init_buffer (buf_buffer_t *buffer,
				 uint32 size);

extern void     buf_clean_buffer (buf_buffer_t *buffer);

extern void     buf_reset_buffer (buf_buffer_t *buffer);

extern status_t buf_write (buf_buffer_t *buffer, 
			   const xmlChar *str);

extern status_t buf_write_indent (buf_buffer_t *buffer, 
				  const xmlChar *str);

extern status_t buf_write_elem (buf_buffer_t *buffer,
				xmlns_id_t ns_id,
				const xmlChar *elem,
				const xmlChar *val,
				xml_attrs_t *attrs);

extern status_t buf_write_tag (buf_buffer_t *buffer,
			       xmlns_id_t ns_id,
			       const xmlChar *elem,
			       buf_tagtyp_t tagtyp,
			       xml_attrs_t *attrs);

extern void buf_print_buffer (const buf_buffer_t *buffer);

extern void buf_inc_indent_level (buf_buffer_t *buffer);

extern void buf_dec_indent_level (buf_buffer_t *buffer);

extern uint32 buf_maxlen (const buf_buffer_t *buffer);


#endif            /* _H_buf */
