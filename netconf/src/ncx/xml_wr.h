#ifndef _H_xml_wr
#define _H_xml_wr

/*  FILE: xml_wr.h
*********************************************************************
*								    *
*			 P U R P O S E				    *
*								    *
*********************************************************************

    XML Write functions

*********************************************************************
*								    *
*		   C H A N G E	 H I S T O R Y			    *
*								    *
*********************************************************************

date	     init     comment
----------------------------------------------------------------------
12-feb-07    abb      Begun; split out from xml_wr_util.c

*/

#ifndef _H_cfg
#include "cfg.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include "status.h"
#endif

#ifndef _H_val
#include "val.h"
#endif

#ifndef _H_val_util
#include "val_util.h"
#endif

#ifndef _H_xml_msg
#include "xml_msg.h"
#endif

/* isattrq labels */
#define ATTRQ   TRUE
#define METAQ   FALSE

/* empty labels */
#define START  FALSE
#define EMPTY  TRUE

/* docmode labels */
#define XMLMODE FALSE
#define DOCMODE TRUE

/* xmlhdr labels */
#define NOHDR FALSE
#define WITHHDR TRUE

/********************************************************************
*								    *
*			F U N C T I O N S			    *
*								    *
*********************************************************************/

extern void
    xml_wr_buff (ses_cb_t *scb,
		 const xmlChar *buff,
		 uint32 bufflen);

extern void
    xml_wr_begin_elem_ex (ses_cb_t *scb,
			  xml_msg_hdr_t *msg,
			  xmlns_id_t  parent_nsid,
			  xmlns_id_t  nsid,
			  const xmlChar *elname,
			  const dlq_hdr_t *attrQ,
			  boolean isattrq,
			  int32 indent,
			  boolean empty);


extern void
    xml_wr_begin_elem_val (ses_cb_t *scb,
			   xml_msg_hdr_t *msg,
			   const val_value_t *val,
			   int32 indent,
			   boolean empty);

extern void
    xml_wr_begin_elem (ses_cb_t *scb,
		       xml_msg_hdr_t *msg,
		       xmlns_id_t  parent_nsid,
		       xmlns_id_t  nsid,
		       const xmlChar *elname,
		       int32 indent);


extern void
    xml_wr_empty_elem (ses_cb_t *scb,
		       xml_msg_hdr_t *msg,
		       xmlns_id_t  parent_nsid,
		       xmlns_id_t  nsid,
		       const xmlChar *elname,
		       int32 indent);


extern void
    xml_wr_end_elem (ses_cb_t *scb,
		     xml_msg_hdr_t *msg,
		     xmlns_id_t  nsid,
		     const xmlChar *elname,
		     int32 indent);


extern void
    xml_wr_string_elem (ses_cb_t *scb,
			xml_msg_hdr_t *msg,
			const xmlChar *str,
			xmlns_id_t  parent_nsid,
			xmlns_id_t  nsid,
			const xmlChar *elname,
			const dlq_hdr_t *attrQ,
			boolean isattrq,
			int32 indent);

extern void
    xml_wr_qname_elem (ses_cb_t *scb,
		       xml_msg_hdr_t *msg,
		       xmlns_id_t val_nsid,
		       const xmlChar *str,
		       xmlns_id_t  parent_nsid,
		       xmlns_id_t  nsid,
		       const xmlChar *elname,
		       const dlq_hdr_t *attrQ,
		       boolean isattrq,
		       int32 indent);


/* output val_value_t node contents only (w/filter) */
extern void
    xml_wr_check_val (ses_cb_t *scb,
		      xml_msg_hdr_t *msg,
		      val_value_t *val,
		      int32  indent,
		      val_nodetest_fn_t testfn);


/* output val_value_t node contents only */
extern void
    xml_wr_val (ses_cb_t *scb,
		xml_msg_hdr_t *msg,
		val_value_t *val,
		int32 indent);

/* generate entire val_value_t *w/filter) */
extern void
    xml_wr_full_check_val (ses_cb_t *scb,
			   xml_msg_hdr_t *msg,
			   val_value_t *val,
			   int32  indent,
			   val_nodetest_fn_t testfn);

/* generate entire val_value_t */
extern void
    xml_wr_full_val (ses_cb_t *scb,
		     xml_msg_hdr_t *msg,
		     val_value_t *val,
		     int32  indent);

extern status_t
    xml_wr_check_file (const xmlChar *filespec, 
		       val_value_t *val,
		       xml_attrs_t *attrs,
		       boolean docmode,
		       boolean xmlhdr,
		       int32 indent,
		       val_nodetest_fn_t testfn);


extern status_t
    xml_wr_file (const xmlChar *filespec, 
		 val_value_t *val,
		 xml_attrs_t *attrs,
		 boolean docmode,
		 boolean xmlhdr,
		 int32 indent);


#endif	    /* _H_xml_wr */

