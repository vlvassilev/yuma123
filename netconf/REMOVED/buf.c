/*  FILE: buf.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
28-apr-05    abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <limits.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_buf
#include "buf.h"
#endif

#ifndef _H_status
#include  "status.h"
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
#define BOGUS_LEN   0x1000


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/

/********************************************************************
* FUNCTION write_attrs
*
* Print the attribute list 
*
* INPUTS:
*    buffer == buffer struct to write
*    attrs
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
static status_t 
    write_attrs (buf_buffer_t *buffer, 
		 xml_attrs_t *attrs)
{
    xml_attr_t  *attr;
    const xmlChar *pfix, *str;
    boolean  first;
    uint32  i;

    if (!buffer || !attrs) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    for (first=TRUE, attr = xml_first_attr(attrs);
	 attr != NULL;
	 attr = xml_next_attr(attr)) {
	if (first) {
	    *buffer->bptr++ = ' ';
	    first = FALSE;
	} else {
	    /* put only 1 attribute per line */
	    *buffer->bptr++ = '\n';
	    for (i=0; i<buffer->indent+2; i++) {
		*buffer->bptr++ = ' ';
	    }
	}

	/* get the attribute prefix */
	if (attr->attr_ns==XMLNS_NULL_NS_ID || 
	    attr->attr_ns==xmlns_get_def_ns()) {
	    pfix = NULL;
	} else {
	    pfix = xmlns_get_ns_prefix(attr->attr_ns);
	}

	/* special case -- xmlns attributes put the prefix after name */
	if (!xml_strcmp(attr->attr_name, XMLNS)) {
	    /* print the 'xmlns' attribute name first */
	    str = attr->attr_name;
	    while ((*buffer->bptr++ = *str++) != 0) {
		;
	    }
	    buffer->bptr--;

	    /* print the prefix */
	    if (pfix) {
		*buffer->bptr++ = ':';
		while ((*buffer->bptr++ = *pfix++) != 0) {
		    ;
		}
		buffer->bptr--;
	    }
	} else {
	    /* print the prefix first */
	    if (pfix) {
		while ((*buffer->bptr++ = *pfix++) != 0) {
		    ;
		}
		buffer->bptr--;
		*buffer->bptr++ = ':';
	    }

	    /* print the attribute name */
	    str = attr->attr_name;
	    while ((*buffer->bptr++ = *str++) != 0) {
		;
	    }
	    buffer->bptr--;
	}

	*buffer->bptr++ = '=';
	*buffer->bptr++ = '"';
	str = attr->attr_val;
	while ((*buffer->bptr++ = *str++) != 0) {
	    ;
	}
	buffer->bptr--;
	*buffer->bptr++ = '"';
    }
    return NO_ERR;

} /* write_attrs */


/********************************************************************
* FUNCTION buf_init_buffer
*
* Initialize the fields in the buf_buffer_t struct
*
* INPUTS:
*    buffer == buffer struct to init
*    buffsize == buffer size to malloc
* RETURNS:
*    NO_ERR if all goes well
*********************************************************************/
status_t 
    buf_init_buffer (buf_buffer_t *buffer, 
		     uint32 buffsize)
{
    if (!buffer) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!buffsize) {
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    buffer->buff = (xmlChar *)m__getMem(buffsize);
    if (!buffer->buff) {
        return SET_ERROR(ERR_INTERNAL_MEM);
    }
    buffer->buff[0] = 0;
    buffer->bufflen = buffsize;
    buffer->bptr = buffer->buff;
    buffer->indent = 0;
    return NO_ERR;

} /* buf_init_buffer */


/********************************************************************
* FUNCTION buf_clean_buffer
*
* Clean the memory allocated in the buffer but not the buf_buffer_t
* itself;
*
* INPUTS:
*   buffer == buffer struct to clean 
* 
* RETURNS:
*    none -- silent errors; would be programming errors only 
*********************************************************************/
void 
    buf_clean_buffer (buf_buffer_t *buffer)
{
    if (buffer) {
	if (buffer->buff) {
	    m__free(buffer->buff);
	    buffer->buff = NULL;
	}
	buffer->bptr = NULL;
	buffer->bufflen = 0;
	buffer->indent = 0;
    }

} /* buf_clean_buffer */


/********************************************************************
* FUNCTION buf_reset_buffer
*
* Reset buffer to an empty buffer state
*
* INPUTS:
*   buffer == buffer struct to reset
* 
* RETURNS:
*    none, silent programming errors ignored
*********************************************************************/
void 
    buf_reset_buffer (buf_buffer_t *buffer)
{
    if (buffer) {
	buffer->bptr = buffer->buff;
	buffer->indent = 0;
    }

} /* buf_reset_buffer */


/********************************************************************
* FUNCTION buf_write
*
* Write N chars to the PDU buffer if it will fit
* Note that this function ignores the indent level
* and does not insert a newln and indent spaces.
*
* INPUTS:
*   buffer == buffer to write
*   str == string to write to buffer
* RETURNS:
*    NO_ERR if write went okay, error otherwise
*********************************************************************/
status_t 
    buf_write (buf_buffer_t *buffer, 
	       const xmlChar *str)
{
    uint32  i, len;

    if (!buffer || !str) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* check if the string will fit in the buffer */
    len = xml_strlen(str);
    if ((buffer->bptr + len) >= (buffer->buff + buffer->bufflen)) {
        return SET_ERROR(ERR_BUFF_OVFL);
    }

    /* copy the string and move the write pointer */
    for (i=0; i<len; i++) {
	*buffer->bptr++ = *str++;
    }
    *buffer->bptr = 0;
    return NO_ERR;

} /* buf_write */


/********************************************************************
* FUNCTION buf_write_indent
*
* Write N chars to the PDU buffer if it will fit
* First write a newln and indent spaces
* Then write the provided string;
* 
* For multi-line content:
*   After each newln the current indent_cnt spaces are inserted
*
* INPUTS:
*   buffer == buffer to write
*   str == string to write to buffer
* RETURNS:
*    NO_ERR if write went okay, error otherwise
*********************************************************************/
status_t 
    buf_write_indent (buf_buffer_t *buffer, 
		      const xmlChar *str)
{
    uint32  i, len, nl, indent_cnt, siz;

    if (!buffer || !str) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    len = xml_strlen(str);

    /* find the number of newlns in the buffer */
    for (i=0, nl=0; i<len; i++) {
	if (str[i] == '\n') {
	    nl++;
	}
    }

    /* check if the string will fit in the buffer */
    indent_cnt = buffer->indent * xmlns_indent_amount();

    siz = (len+indent_cnt+1) + (nl * indent_cnt);

    if ((buffer->bptr + siz) >= 
	 (buffer->buff + buffer->bufflen)) {
        return SET_ERROR(ERR_BUFF_OVFL);
    }

    /* write the newln and indent spaces */
    *buffer->bptr++ = '\n';
    for (i=0; i<indent_cnt; i++) {
	*buffer->bptr++ = ' ';
    }

    /* copy the string and move the write pointer */
    for (i=0; i<len; i++) {
	if ((*buffer->bptr++ = *str++) == '\n') {
	    for (nl=0; nl<indent_cnt; nl++) {
		*buffer->bptr++ = ' ';
	    }
	}
    }
    *buffer->bptr = 0;
    return NO_ERR;

} /* buf_write_indent */


/********************************************************************
* FUNCTION buf_write_elem
*
* Write the specified simple element and optional value to the buffer
* A newline plus 'indent_cnt' spaces will be output before the
* element.  The NETCONF namespace prefix will be added if needed.
*
* INPUTS:
*   buffer == buffer struct to fill
*   ns_id == namespace ID of the element
*   elem == element name
*   val == element value string; NULL if empty element 
*   attrs == queue of attributes to print in the start tag, or NULL if none
* RETURNS:
*    NO_ERR if parameters and write okay; error otherwise
*********************************************************************/
status_t 
    buf_write_elem (buf_buffer_t *buffer,
		    xmlns_id_t ns_id,
		    const xmlChar *elem,
		    const xmlChar *val,
		    xml_attrs_t *attrs)
{
    uint32  i, len, elen, plen, indent_cnt;
    const xmlChar   *pfix;
    status_t      res;

    if (!buffer || !elem) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    indent_cnt = buffer->indent * xmlns_indent_amount();
    pfix = xmlns_get_ns_prefix(ns_id);
    if (pfix) {
	plen = xml_strlen(pfix) + 1;              /* prefix and colon */
    } else {
	plen = 0;                                    /* no prefix */
    }
    elen = xml_strlen(elem);

    len = indent_cnt + 1;           /* account for newln and spaces */
    len += (elen + 2);              /* account for start tag <elem> */
    len += plen;          /* account for start-tag prefix and colon */

    if (val) {
	len += xml_strlen(val);         /* account for element content */
	len += (elen + 3);          /* account for end tag </elem> */
	len += plen;       /* account for end-tag prefix and colon */
    } else {
	len += 1;          /* account for slash in empty start tag */
    }
    if (attrs) {                         
	len += xml_sizeof_attrs(attrs);  /* account for attributes */
    }

    if ((buffer->bptr + len) >= (buffer->buff + buffer->bufflen)) {
        return SET_ERROR(ERR_BUFF_OVFL);
    }

    /* write the newline and indent spaces */
    *buffer->bptr++ = '\n';
    for (i=0; i<indent_cnt; i++) {
	*buffer->bptr++ = ' ';
    }

    /* write the start-tag, content, and end-tag */
    if (pfix) {
	buffer->bptr += xmlStrPrintf(buffer->bptr, 
	     BOGUS_LEN, (const xmlChar *)"<%s:%s", pfix, elem);
    } else {
	buffer->bptr += xmlStrPrintf(buffer->bptr, 
	     BOGUS_LEN, (const xmlChar *)"<%s", elem);
    }
    if (attrs) {
	res = write_attrs(buffer, attrs);
	if (res != NO_ERR) {
	    return res;
	}
    }
    if (val) {
	/* finish the start tag, write the val and end tag */
	*buffer->bptr++ = '>';  
	while ((*buffer->bptr++ = *val++) != 0) {;}   /* copy val */
	buffer->bptr--;                 /* back up the buffer ptr */
	if (pfix) {                       /* write end tag */
	    buffer->bptr += xmlStrPrintf(buffer->bptr, 
		 BOGUS_LEN, (const xmlChar *)"</%s:%s>", pfix, elem);
	} else {
	    buffer->bptr += xmlStrPrintf(buffer->bptr, 
		 BOGUS_LEN, (const xmlChar *)"</%s>", elem);
	}
    } else {
	/* finish the empty start tag */
	*buffer->bptr++ = '/';
	*buffer->bptr++ = '>';
    }
    *buffer->bptr = 0;                    /* terminate string */
    return NO_ERR;

}  /* buf_write_elem */


/********************************************************************
* FUNCTION buf_write_tag
*
* Write the specified element start tag or eng tagto the buffer
* A newline plus 'indent_cnt' spaces will be output before the
* tag.  The NETCONF namespace prefix will be added if needed.
*
* INPUTS:
*   buffer == buffer struct to fill
*   elem == element name
*   tagtyp == BUF_START_TAG if start-tag or BUF_END_TAG for end-tag
*   attrs == queue of attributes to print in the tag, or NULL if none
* RETURNS:
*    NO_ERR if parameters and write okay; error otherwise
*********************************************************************/
status_t 
    buf_write_tag (buf_buffer_t *buffer,
		   xmlns_id_t ns_id,
		   const xmlChar *elem,
		   buf_tagtyp_t tagtyp,
		   xml_attrs_t  *attrs)
{
    uint32  i, len, elen, indent_cnt;
    const xmlChar   *pfix;
    status_t      res;
    
    /* check parameters */
    if (!buffer || !elem) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    switch (tagtyp) {
    case BUF_START_TAG:
	len = 0;      
	break;
    case BUF_END_TAG:
	len = 1;          /* account for the slash in the end-tag */
	break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }

    /* check if the string will fit in the buffer */
    indent_cnt = buffer->indent * xmlns_indent_amount();
    len += (indent_cnt + 1);       /* account for newln and spaces */
    elen = xml_strlen(elem);
    len += (elen + 2);                    /* account for <element> */
    pfix = xmlns_get_ns_prefix(ns_id);
    if (pfix) {
	len += (xml_strlen(pfix) + 1);    /* account for prefix and colon */
    }
    if (attrs) {
	len += xml_sizeof_attrs(attrs);
    }

    if ((buffer->bptr + len) >= (buffer->buff + buffer->bufflen)) {
        return SET_ERROR(ERR_BUFF_OVFL);
    }

    /* write the newline and indent spaces */
    *buffer->bptr++ = '\n';
    for (i=0; i<indent_cnt; i++) {
	*buffer->bptr++ = ' ';
    }

    /* write the tag */
    *buffer->bptr++ = '<';
    if (tagtyp==BUF_END_TAG) {
	*buffer->bptr++ = '/';
    }
    if (pfix) {
	buffer->bptr += xmlStrPrintf(buffer->bptr, 
	     BOGUS_LEN, (const xmlChar *)"%s:%s", pfix, elem);
    } else {
	buffer->bptr += xmlStrPrintf(buffer->bptr, 
	     BOGUS_LEN, (const xmlChar *)"%s", elem);
    }
    if (attrs) {
	res = write_attrs(buffer, attrs);
	if (res != NO_ERR) {
	    return NO_ERR;
	}
    }
    *buffer->bptr++ = '>';
    *buffer->bptr = 0;                
    return NO_ERR;

}  /* buf_write_tag */


/********************************************************************
* FUNCTION buf_print_buffer
*
* DEBUG: Print the buffer contents to STDOUT

* INPUTS:
*   buffer == buffer struct to printf
*********************************************************************/
void 
    buf_print_buffer (const buf_buffer_t *buffer)
{
    if (!buffer || !buffer->buff) {
	printf("\nBuffer NULL or uninitialized!\n");
    } else if (!buffer->buff[0]) {
	printf("\nBuffer Empty!\n");
    } else {
	printf("\n%s\n", buffer->buff);
    }
}   /* buf_print_buffer */


/********************************************************************
* FUNCTION buf_inc_indent_level
*
* Increnent the indent level for the specified buffer

* INPUTS:
*   buffer == buffer struct to increment the indent level
* RETURNS:
*   none, DEBUG programming errors printed to stdout
*********************************************************************/
void 
    buf_inc_indent_level (buf_buffer_t *buffer)
{
    if (!buffer || !buffer->buff) {
#ifdef CPP_DEBUG
	printf("\nbuf_inc: NULL pointers\n");
#endif
        return;
    }

    if (buffer->indent < UINT_MAX) {
	buffer->indent++;
    } 
#ifdef CPP_DEBUG
    else {
	printf("\nbuf_inc: UINT_MAX reached\n");
    }
#endif

}   /* buf_inc_indent_level */


/********************************************************************
* FUNCTION buf_dec_indent_level
*
* Decrement the indent level for the specified buffer

* INPUTS:
*   buffer == buffer struct to decrement the indent level
* RETURNS:
*   none, DEBUG programming errors printed to stdout
*********************************************************************/
void 
    buf_dec_indent_level (buf_buffer_t *buffer)
{
    if (!buffer || !buffer->buff) {
#ifdef CPP_DEBUG
	printf("\nbuf_dec: NULL pointers\n");
#endif
        return;
    }
    if (buffer->indent) {
	buffer->indent--;
    }
#ifdef CPP_DEBUG
    else {
	printf("\nbuf_dec: decrement at zero error\n");
    }
#endif

}   /* buf_dec_indent_level */


/********************************************************************
* FUNCTION buf_maxlen
*
* Get the buffer size

* INPUTS:
*   buffer == buffer struct to check
*********************************************************************/
uint32 
    buf_maxlen (const buf_buffer_t *buffer)
{
    if (!buffer || !buffer->buff) {
	return 0;
    } else {
	return buffer->bufflen;
    }
    /*NOTREACHED*/
}   /* buf_maxlen */


/* END buf.c */



