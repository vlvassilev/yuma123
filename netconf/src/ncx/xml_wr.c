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
/*  FILE: xml_wr.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
24may06      abb      begun; split out from agt_ncx.c
12feb07      abb      split out non-agent specific write fns back to ncx

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>
#include  <string.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncx_num
#include  "ncx_num.h"
#endif

#ifndef _H_ncxconst
#include  "ncxconst.h"
#endif

#ifndef _H_obj
#include  "obj.h"
#endif

#ifndef _H_ses
#include  "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_val
#include  "val.h"
#endif

#ifndef _H_val_util
#include  "val_util.h"
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

#ifndef _H_xml_wr
#include  "xml_wr.h"
#endif

#ifndef _H_xpath
#include  "xpath.h"
#endif

#ifndef _H_xpath_wr
#include  "xpath_wr.h"
#endif

#ifndef _H_xpath_yang
#include  "xpath_yang.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

#ifdef DEBUG
#define XML_WR_DEBUG  1
#endif

#define XML_WR_MAX_LINESTR   34



/********************************************************************
* FUNCTION fit_on_line
*
* Check if the specified value will fit on the current line
* or if a newline is needed first
*
* INPUTS:
*   scb == session control block
*   val == value to check
*
* RETURNS:
*   TRUE if value will fit on current line, FALSE if not
*********************************************************************/
static boolean
    fit_on_line (ses_cb_t *scb,
                 const val_value_t *val)
{
    uint32     vallen, elemlen, prefixlen;
    xmlns_id_t nsid;
    status_t   res;

    /* metavals must be put on 1 line */
    if (val_is_metaval(val)) {
        return TRUE;
    }

    /* make sure leafs are printed without leading and
     * trailing whitespace in a normal session
     */
    if (scb->mode == SES_MODE_XML) {
        if (obj_is_leafy(val->obj)) {
            return TRUE;
        }
    }

    if (!val_fit_oneline(val, SES_LINESIZE(scb))) {
        return FALSE;
    }

    if (ses_get_mode(scb) != SES_MODE_XMLDOC) {
        return TRUE;
    }
    
    /* don't bother generating the actual size unless
     * the session is requesting the XMLDOC mode
     */
    vallen = 0;
    res = val_sprintf_simval_nc(NULL, val, &vallen);
    if (res != NO_ERR) {
        return TRUE;
    }

    prefixlen = 0;
    nsid = val_get_nsid(val);
    if (nsid) {
        prefixlen = 
            (xml_strlen(xmlns_get_ns_prefix(nsid)) * 2) + 2;
    }
    
    elemlen = xml_strlen(val->name) * 2;

    return ((vallen + prefixlen + elemlen + 1) <= ses_line_left(scb)) 
        ? TRUE : FALSE;

}  /* fit_on_line */


/********************************************************************
* FUNCTION write_extern
*
* Write an external file to the session
*
* INPUTS:
*   scb == session control block
*   val == value to write (NCX_BT_EXTERN)
*
* RETURNS:
*   none
*********************************************************************/
static void
    write_extern (ses_cb_t *scb,
                  const val_value_t *val)
{
    FILE               *fil;
    boolean             done, inxml, xmldone, firstline;
    int                 ch, lastch;

    xmldone = FALSE;
    inxml = FALSE;
    done = FALSE;
    firstline = TRUE;
    lastch = 0;

    if (val->v.fname) {
        fil = fopen((const char *)val->v.fname, "r");
        if (fil) {
            while (!done) {
                ch = fgetc(fil);

                if (ch == EOF) {
                    if (lastch) {
                        ses_putchar(scb, (uint32)lastch);
                    }
                    fclose(fil);
                    done = TRUE;
                    continue;
                }

                if (firstline) {
                    /* do not match the first char in the file */
                    if (ch == '\n' && lastch) {
                        /* done with xml checking */
                        firstline = FALSE;
                        xmldone = TRUE;
                    } else if (!xmldone) {
                        /* look for xml declaration and remove it */
                        if (inxml) {
                            /* look for end */
                            if (lastch == '?' && ch == '>') {
                                xmldone = TRUE;
                            }
                        } else {
                            /* look for start */
                            if (lastch == '<' && ch == '?') {
                                inxml = TRUE;
                            }
                        }
                    }

                    /* first time xmldone is true skip this */
                    if (xmldone && !inxml) {
                        if (lastch) {
                            ses_putchar(scb, (uint32)lastch);
                        }
                    }

                    /* setup 3rd loop to print char after '?>' */
                    if (xmldone && inxml) {
                        if (ch != '>') {
                            inxml = FALSE;
                        }
                    }
                } else {
                    if (lastch) {
                        ses_putchar(scb, (uint32)lastch);
                    }
                }

                lastch = ch;
            }
        } else {
            log_error("\nError: open extern var "
                      "file '%s' failed",
                      val->v.fname);
        }
    } else {
        SET_ERROR(ERR_INTERNAL_VAL);
    }
    
}  /* write_extern */


/********************************************************************
* FUNCTION write_intern
*
* Write an internal buffer to the session
*
* INPUTS:
*   scb == session control block
*   val == value to write (NCX_BT_INTERN)
*
* RETURNS:
*   none
*********************************************************************/
static void
    write_intern (ses_cb_t *scb,
                  const val_value_t *val)
{
    if (val->v.intbuff) {
        ses_putstr(scb, val->v.intbuff);
    }
    
}  /* write_intern */


/********************************************************************
* FUNCTION write_xmlns_decl
*
* Write an xmlns declaration
*
* INPUTS:
*   scb == session control block
*   pfix == prefix to use
*   nsid == namespace ID to use for xmlns value
*   indent == actual indent amount; xml_wr_indent will not be checked
*
* RETURNS:
*   none
*********************************************************************/
static void
    write_xmlns_decl (ses_cb_t *scb,
                      const xmlChar *pfix,
                      xmlns_id_t  nsid,
                      int32 indent)
{
    const xmlChar  *val;

    val = xmlns_get_ns_name(nsid);
    if (!val) {
        SET_ERROR(ERR_INTERNAL_VAL);
        return;
    }

    if (indent <= 0) {
        ses_putchar(scb, ' ');
    } else {
        ses_indent(scb, indent);
    }

    /* write the xmlns attribute name */
    ses_putstr(scb, XMLNS);

    /* generate a prefix if this attribute has a namespace ID */
    if (pfix) {
        ses_putchar(scb, ':');
        ses_putstr(scb, pfix);
    }
    ses_putchar(scb, '=');
    ses_putchar(scb, '\"');
    ses_putstr(scb, val);      /* write the namespace URI value */
    ses_putchar(scb, '\"');
    
}  /* write_xmlns_decl */


/********************************************************************
* FUNCTION handle_xpath_start_tag
*
* Write a the xmlns attributes needed for the
* namespaces (implied or explicit) in an XPath
* expression
*
* Since all XPath content variables are leafs
* (no mixed content in YANG)
* then it does not matter if any xmlns attributes
* override the prefixes in the message header prefix map
*
* When the XPath expression is generated in xml_wr_check_val
* only default prefixes will be used, assuming that
* this function was called and worked correctly first
*
* INPUTS:
*   scb == session control block
*   msg == top header from message in progress
*   xpathpcb == XPath parser control block to use
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*   retcount == address of return xmlns print count
*
* OUTPUTS:
*   *retcount == number of xmlns directives written
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    handle_xpath_start_tag (ses_cb_t *scb,
                            xml_msg_hdr_t *msg,
                            const xpath_pcb_t *xpathpcb,
                            int32 indent,
                            uint32 *retcount)
{
    const xmlChar       *defpfix, *msgpfix;
    uint32               num_nsids, i;
    xmlns_id_t           cur_nsid;
    xmlns_id_t           nsid_array[XML_WR_MAX_NAMESPACES];
    status_t             res;

    *retcount = 0;

    res = xpath_yang_get_namespaces(xpathpcb,
                                    nsid_array,
                                    XML_WR_MAX_NAMESPACES,
                                    &num_nsids);
    if (res != NO_ERR) {
        /* not expecting anything except a buffer overflow
         * from too many namespaces in the same XPath expr
         */
        return res;
    }

    /* else add all the missing xmlns directives */
    for (i = 0; i < num_nsids; i++) {

        cur_nsid = nsid_array[i];

        /* get the default and message prefixes */
        defpfix = xmlns_get_ns_prefix(cur_nsid);
        if (defpfix == NULL) {
            return SET_ERROR(ERR_INTERNAL_VAL);
        }

        msgpfix = 
            xml_msg_get_prefix_start_tag(msg, 
                                         cur_nsid);
        if (msgpfix != NULL && 
            !xml_strcmp(defpfix, msgpfix)) {
            /* this nsid/prefix mapping already
             * accounted for in the message header
             */
            continue;
        } else {
            /* this element start tag needs an
             * xmlns attribute for this NSID
             */
            write_xmlns_decl(scb,
                             defpfix,
                             cur_nsid,
                             indent);
        }
    }

    *retcount = num_nsids;

    return NO_ERR;

}  /* handle_xpath_start_tag */


/********************************************************************
* FUNCTION write_attrs
*
* Write all the required attributes for this element
*
* INPUTS:
*   scb == session control block
*   msg == header for the rpc_msg_t in progress
*   attrQ == Q of xml_attr_t or val_value_t to write
*   isattrq == TRUE for Q of xml_attr_t
*           == FALSE for Q of val_value_t
*    curelem == value node for current element, if available
*   indent == actual indent amount; xml_wr_indent will not be checked
*   elem_nsid == namespace ID of the parent element
*
*********************************************************************/
static void
    write_attrs (ses_cb_t *scb,
                 xml_msg_hdr_t *msg,
                 const dlq_hdr_t *attrQ,
                 boolean isattrq,
                 val_value_t  *curelem,
                 int32 indent,
                 xmlns_id_t elem_nsid)
{
    const xml_attr_t  *attr;
    val_value_t       *val;
    dlq_hdr_t         *hdr;
    const xmlChar     *pfix, *attr_name, *attr_qname;
    xmlChar           *buffer;
    boolean            xneeded;
    uint32             len, retcount, bufferlen;;
    xmlns_id_t         ns_id, attr_nsid;
    status_t           res;

    ns_id = xmlns_ns_id();
    for (hdr = dlq_firstEntry(attrQ); 
         hdr != NULL;
         hdr = dlq_nextEntry(hdr)) {

        attr = NULL;
        val = NULL;

        /* set up the data fields; len is not precise, ignores prefix */
        if (isattrq) {
            attr = (const xml_attr_t *)hdr;
            attr_nsid = attr->attr_ns;
            attr_name = attr->attr_name;
            attr_qname = attr->attr_qname;
            len = xml_strlen(attr->attr_val) 
                + xml_strlen(attr->attr_name);
        } else {
            val = (val_value_t *)hdr;
            attr_nsid = val->nsid;
            attr_name = val->name;
            attr_qname = NULL;
            len = xml_strlen(val->v.str) 
                + xml_strlen(val->name);
        }

        /* deal with initial indent */
        if (indent < 0) {
            ses_putchar(scb, ' ');
        } else if (len + 4 + SES_LINELEN(scb) 
                   >= SES_LINESIZE(scb)) {
            ses_indent(scb, indent);
        } else {
            ses_putchar(scb, ' ');
        }

        /* generate one attribute name value pair
         *
         * generate a prefix if this attribute has a namespace ID 
         * make sure to skip the XMLNS namespace; this is 
         * handled different than all other attributes 
         *
         */
        /* check if this is an XMLNS directive */
        if (XMLNS_EQ(attr_nsid, ns_id)) {
            /* xmlns:prefix format */
            if (attr_name != attr_qname) {
                /* this is a namespace decl with a prefix */
                ses_putstr(scb, XMLNS);
                ses_putchar(scb, ':');      
            }
        } else if (attr_nsid) {
            /* prefix:attribute-name format */
            pfix = xml_msg_get_prefix(msg, 
                                      elem_nsid, 
                                      attr_nsid, 
                                      curelem,
                                      &xneeded);
            if (xneeded) {
                write_xmlns_decl(scb, 
                                 pfix, 
                                 attr_nsid, 
                                 indent);
            }

            /* deal with indent again */
            if (indent < 0) {
                ses_putchar(scb, ' ');
            } else if (len + 4 + SES_LINELEN(scb) 
                       >= SES_LINESIZE(scb)) {
                ses_indent(scb, indent);
            } else {
                ses_putchar(scb, ' ');
            }

            if (pfix) {
                ses_putstr(scb, pfix);
                ses_putchar(scb, ':');
            }
        } else if (val) {
            /* check if XPath or identityref content */
            if (val->xpathpcb) {
                /* generate all the default xmlns directives needed
                 * for the content following this start tag to be valid
                 */
                retcount = 0;
                res = handle_xpath_start_tag(scb,
                                             msg,
                                             val->xpathpcb,
                                             indent,
                                             &retcount);
                if (res != NO_ERR) {
                    /* not expecting anything except a buffer overflow
                     * from too many namespaces in the same XPath expr
                     */
                    SET_ERROR(res);
                } else if (retcount) {
                    ses_indent(scb, indent);
                }
            } else if (val->btyp == NCX_BT_IDREF) {
                xneeded = FALSE;
                pfix = xml_msg_get_prefix(msg, 
                                          (val->parent) ?
                                          val_get_nsid(val->parent) : 0,
                                          VAL_IDREF_NSID(val), 
                                          val, 
                                          &xneeded);
                if (xneeded) {
                    write_xmlns_decl(scb, 
                                     pfix, 
                                     VAL_IDREF_NSID(val), 
                                     indent);
                }
            }
        }

        ses_putstr(scb, attr_name);
        ses_putchar(scb, '=');
        ses_putchar(scb, '\"');
        if (isattrq) {
            ses_putastr(scb, attr->attr_val, -1);
        } else if (typ_is_string(val->btyp)) {
            ses_putastr(scb, VAL_STR(val), -1);
        } else {
            /* write the simple value meta var the slow way */
            bufferlen = 0;
            res = val_sprintf_simval_nc(NULL, val, &bufferlen);
            if (res != NO_ERR) {
                SET_ERROR(res);
            } else {
                buffer = m__getMem(bufferlen+1);
                if (buffer == NULL) {
                    SET_ERROR(ERR_INTERNAL_MEM);
                } else {
                    res = val_sprintf_simval_nc(buffer,
                                                val,
                                                &bufferlen);
                    if (res != NO_ERR) {
                        SET_ERROR(res);
                    } else {
                        ses_putastr(scb, buffer, -1);
                    }
                    m__free(buffer);
                }
            }
        }
        ses_putchar(scb, '\"');
    }

}  /* write_attrs */


/********************************************************************
* FUNCTION begin_elem_val
*
* Write a start or empty XML tag to the specified session
*
* INPUTS:
*   scb == session control block
*   msg == top header from message in progress
*   val  == value node to use
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
* RETURNS:
*   none
*********************************************************************/
static void
    begin_elem_val (ses_cb_t *scb,
                    xml_msg_hdr_t *msg,
                    val_value_t *val,
                    int32 indent)
{
    const xmlChar       *pfix,  *elname;
    const dlq_hdr_t     *attrQ;
    const xpath_pcb_t   *xpathpcb;
    boolean              xneeded, empty, xmlcontent;
    xmlns_id_t           nsid, parent_nsid;
    status_t             res;
    uint32               retcount;

    empty = !val_has_content(val);
    elname = val->name;
    nsid = val->nsid;
    attrQ = &val->metaQ;
    xpathpcb = NULL;

    if (val->parent) {
        parent_nsid = val->parent->nsid;
    } else {
        parent_nsid = 0;
    }

    xmlcontent = (val->btyp == NCX_BT_INSTANCE_ID) ||
        obj_is_xpath_string(val->obj);

    if (xmlcontent) {
        xpathpcb = val_get_const_xpathpcb(val);
    }

    ses_indent(scb, indent);

    /* start the element and write the prefix, if any */
    ses_putchar(scb, '<');
    pfix = xml_msg_get_prefix(msg, 
                              parent_nsid, 
                              nsid, 
                              val, 
                              &xneeded);
    if (pfix) {
        ses_putstr(scb, pfix);
        ses_putchar(scb, ':');
    }

    /* write the element name */
    ses_putstr(scb, elname);

    if (xneeded || xmlcontent || (attrQ && !dlq_empty(attrQ))) {

        if (indent >= 0) {
            indent += ses_indent_count(scb);
        }

        if (attrQ) {
            write_attrs(scb, 
                        msg, 
                        attrQ,
                        FALSE,
                        val,
                        indent,
                        nsid);
        }

        if (xneeded) {
            if (!attrQ || dlq_empty(attrQ)) {
                indent = -1;
            }
            write_xmlns_decl(scb,
                             pfix,
                             nsid,
                             indent);
        }

        if (xpathpcb) {
            /* generate all the default xmlns directives needed
             * for the content following this start tag to be valid
             */
            res = handle_xpath_start_tag(scb,
                                         msg,
                                         xpathpcb,
                                         indent,
                                         &retcount);
            /* don't care about the retcount value 
             * terminating the start tag here no matter what
             */
            if (res != NO_ERR) {
                /* not expecting anything except a buffer overflow
                 * from too many namespaces in the same XPath expr
                 */
                SET_ERROR(res);
            }
        }
    }

    /* finish up the element */
    if (empty) {
        ses_putchar(scb, '/');
    }
    ses_putchar(scb, '>');

    /* hack in XMLDOC mode to get more readable XSD output */
    if (empty && scb->mode==SES_MODE_XMLDOC && indent < 
        (3*ses_indent_count(scb))) {
        ses_putchar(scb, '\n');
    }

}  /* begin_elem_val */


/********************************************************************
* FUNCTION write_check_val
* 
* Write an NCX value in XML encoding
* while checking nodes for suppression of output with
* the supplied test fn
*
* !!! NOTE !!!
* 
* This function generates the contents of the val_value_t
* but not the top node itself.  This function is called
* recursively and this is the intended behavior.
*
* To generate XML for an entire val_value_t, including
* the top-level node, use the xml_wr_full_val fn.
*
* If the acm_cache and acm_cbfn fields are set in
* the msg header then access control will be checked
* If FALSE, then nothing will be written to the output session
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   testcb == callback function to use, NULL if not used
*   acmcheck == TRUE if the ACM check should be done
*
* RETURNS:
*   none
*********************************************************************/
static void
    write_check_val (ses_cb_t *scb,
                     xml_msg_hdr_t *msg,
                     val_value_t *val,
                     int32  indent,
                     val_nodetest_fn_t testfn,
                     boolean acmcheck)
{
    const ncx_lmem_t   *listmem;
    const xmlChar      *pfix;
    typ_def_t          *realtypdef;
    val_value_t        *realval;
    val_value_t        *v_val, *chval;
    val_value_t        *useval;
    xmlChar            *binbuff;
    xml_msg_authfn_t    cbfn;
    uint32              len;
    status_t            res;
    boolean             first, wspace, xneeded, acmtest;
    ncx_btype_t         btyp, listbtyp;
    xmlChar             buff[NCX_MAX_NUMLEN];


    /* check the user filter callback function */
    if (testfn) {
        if (!(*testfn)(msg->withdef, TRUE, val)) {
            return;   /* skip this entry */
        }
    }

    if (acmcheck && msg->acm_cbfn) {
        cbfn = (xml_msg_authfn_t)msg->acm_cbfn;
        acmtest = (*cbfn)(msg, scb->username, val);
        if (!acmtest) {
            return;
        }
    }
                                   
    /* check if this is an external file to send */
    if (val->btyp == NCX_BT_EXTERN) {
        write_extern(scb, val);
        return;
    }

    /* check if this is an internal buffer to send */
    if (val->btyp == NCX_BT_INTERN) {
        write_intern(scb, val);
        return;
    }

    v_val = NULL;
    realval = NULL;

    if (val_is_virtual(val)) {
        v_val = val_get_virtual_value(scb, val, &res);
        if (!v_val) {
            if (res != ERR_NCX_SKIPPED) {
                /*** handle inline error ***/
            }
            return;
        }
    }

    useval = (v_val) ? v_val : val;

    btyp = useval->btyp;

    if (btyp == NCX_BT_LEAFREF) {
        realtypdef = typ_get_xref_typdef(val->typdef);
        if (realtypdef) {
            res = NO_ERR;
            btyp = typ_get_basetype(realtypdef);

            switch (btyp) {
            case NCX_BT_STRING:
            case NCX_BT_BINARY:
            case NCX_BT_BOOLEAN:
            case NCX_BT_ENUM:
                break;
            default:
                realval = val_make_simval(realtypdef,
                                          val_get_nsid(useval),
                                          useval->name,
                                          VAL_STR(useval),
                                          &res);
                if (realval) {
                    useval = realval;
                } else {
                    log_error("\nError: write leafref '%s' failed (%s)",
                              useval->name,
                              get_error_string(res));
                }
            }
        } else {
            SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    switch (btyp) {
    case NCX_BT_ENUM:
        if (useval->v.enu.name) {
            ses_putstr(scb, useval->v.enu.name);
        }
        break;
    case NCX_BT_EMPTY:
        if (useval->v.boo) {
            xml_wr_empty_elem(scb,
                              msg,
                              val_get_parent_nsid(useval),
                              useval->nsid,
                              useval->name,
                              -1);
        }
        break;
    case NCX_BT_BOOLEAN:
        if (useval->v.boo) {
            ses_putcstr(scb, NCX_EL_TRUE, indent);
        } else {
            ses_putcstr(scb, NCX_EL_FALSE, indent);
        }
        break;
    case NCX_BT_INT8:
    case NCX_BT_INT16:
    case NCX_BT_INT32:
    case NCX_BT_INT64:
    case NCX_BT_UINT8:
    case NCX_BT_UINT16:
    case NCX_BT_UINT32:
    case NCX_BT_UINT64:
    case NCX_BT_DECIMAL64:
    case NCX_BT_FLOAT64:
        res = ncx_sprintf_num(buff, 
                              &useval->v.num, 
                              btyp, 
                              &len);
        if (res == NO_ERR) {
            ses_putstr(scb, buff); 
        } else {
            SET_ERROR(res);
        }
        break;
    case NCX_BT_INSTANCE_ID:
    case NCX_BT_STRING:
        /* if the content has XPath or QName content, then
         * the begin_elem_val function should have already
         * printed all the required xmlns attributes
         * so just print the string value with prefixes and all
         */
        if (VAL_STR(useval)) {
            if (!fit_on_line(scb, useval) && (indent>0)) {
                ses_indent(scb, indent);
            }
            ses_putcstr(scb, VAL_STR(useval), indent);
        }
        break;
    case NCX_BT_IDREF:
        /* counting on xmlns decl to be in ancestor node
         * because the xneeded node is being ignored here
         */
        pfix = xml_msg_get_prefix(msg,
                                  (useval->parent) 
                                  ? useval->parent->nsid : 0,
                                  useval->v.idref.nsid, 
                                  useval, 
                                  &xneeded);
        if (pfix) {
            ses_putstr(scb, pfix);
            ses_putchar(scb, XMLNS_SEPCH);
        }
        ses_putstr(scb, useval->v.idref.name);
        break;
    case NCX_BT_BINARY:
        if (useval->v.binary.ustr) {
            res = val_sprintf_simval_nc(NULL, useval, &len);
            if (res == NO_ERR) {
                binbuff = m__getMem(len);
                if (!binbuff) {
                    res = ERR_INTERNAL_MEM;
                } else {
                    res = val_sprintf_simval_nc(binbuff, 
                                                useval, 
                                                &len); 
                    if (res == NO_ERR) {
                        ses_putcstr(scb, binbuff, indent);
                    }
                    m__free(binbuff);
                }
            }
        }
        break;
    case NCX_BT_BITS:
    case NCX_BT_SLIST:
        listbtyp = useval->v.list.btyp;
        first = TRUE;
        for (listmem = (const ncx_lmem_t *)
                 dlq_firstEntry(&useval->v.list.memQ);
             listmem != NULL;
             listmem = (const ncx_lmem_t *)dlq_nextEntry(listmem)) {

            wspace = FALSE;

            /* handle indent+double quote or space for non-strings */
            if (typ_is_string(listbtyp)) {

                /* get len and whitespace flag */
                len = xml_strlen_sp(listmem->val.str, &wspace);

                /* check special case -- empty string
                 * handle it here instead of going through the loop
                 */
                if (!len) {
                    if (!first) {
                        ses_putstr(scb, 
                                   (const xmlChar *)" \"\"");
                    } else {
                        ses_putstr(scb, 
                                   (const xmlChar *)"\"\"");
                        first = FALSE;
                    }
                    continue;
                }
            }

            /* handle newline+indent or space between list elements */
            if (first) {
                first = FALSE;
            } else if (SES_LINELEN(scb) > SES_LINESIZE(scb)) {
                ses_indent(scb, indent);
            } else {
                ses_putchar(scb, ' ');
            }           

            /* check if double quotes needed */
            if (wspace) {
                ses_putchar(scb, '\"');
            }

            /* print the list member content as a string */
            if (typ_is_string(listbtyp)) {
                ses_putcstr(scb, listmem->val.str, indent);
            } else if (typ_is_number(listbtyp)) {
                (void)ncx_sprintf_num(buff, 
                                      &listmem->val.num, 
                                      listbtyp, 
                                      &len);
                ses_putcstr(scb, buff, indent);
            } else {
                switch (listbtyp) {
                case NCX_BT_BITS:
                    ses_putstr(scb, listmem->val.bit.name);
                    break;
                case NCX_BT_ENUM:
                    ses_putstr(scb, listmem->val.enu.name); 
                    break;
                case NCX_BT_BOOLEAN:
                    ses_putcstr(scb,
                                (listmem->val.boo) ?
                                NCX_EL_TRUE : NCX_EL_FALSE,
                                indent);
                    break;
                default:
                    SET_ERROR(ERR_INTERNAL_VAL);
                }
            }

            /* check finish quoted string */
            if (wspace) {
                ses_putchar(scb, '\"');
            }
        }
        break;
    case NCX_BT_ANY:
    case NCX_BT_CONTAINER:
    case NCX_BT_LIST:
    case NCX_BT_CHOICE:
    case NCX_BT_CASE:
        for (chval = val_get_first_child(useval);
             chval != NULL;
             chval = val_get_next_child(chval)) {

            xml_wr_full_check_val(scb, 
                                  msg,
                                  chval,
                                  indent,
                                  testfn);
        } 
        break;
    case NCX_BT_LEAFREF:
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }

    if (v_val) {
        val_free_value(v_val);
    }

    if (realval) {
        val_free_value(realval);
    }

}  /* write_check_val */


/************  E X T E R N A L    F U N C T I O N S    **************/


/********************************************************************
* FUNCTION xml_wr_buff
*
* Write some xmlChars to the specified session
*
* INPUTS:
*   scb == session control block to start msg 
*   buff == buffer to write
*   bufflen == number of bytes to write, not including any
*              EOS char at the end of the buffer
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_buff (ses_cb_t *scb,
                 const xmlChar *buff,
                 uint32 bufflen)
{

    uint32  i;

#ifdef DEBUG
    if (!scb || !buff) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    for (i=0; i<bufflen; i++) {
        ses_putchar(scb, *buff++);
    }

}  /* xml_wr_buff */


/********************************************************************
* FUNCTION xml_wr_begin_elem_ex
*
* Write a start or empty XML tag to the specified session
*
* INPUTS:
*   scb == session control block
*   msg == top header from message in progress
*   parent_nsid == namespace ID of the parent element, if known
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   attrQ == Q of xml_attr_t or val_value_t records to write in
*            the element; NULL == none
*   isattrq == TRUE if the qQ contains xml_attr_t nodes
*              FALSE if the Q contains val_value_t nodes (metadata)
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*   empty == TRUE for empty node
*         == FALSE for start node
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_begin_elem_ex (ses_cb_t *scb,
                          xml_msg_hdr_t *msg,
                          xmlns_id_t  parent_nsid,
                          xmlns_id_t  nsid,
                          const xmlChar *elname,
                          const dlq_hdr_t *attrQ,
                          boolean isattrq,
                          int32 indent,
                          boolean empty)
{
    const xmlChar       *pfix;
    boolean              xneeded;

#ifdef DEBUG
    if (!scb || !msg || !elname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    ses_indent(scb, indent);

    /* start the element and write the prefix, if any */
    ses_putchar(scb, '<');
    pfix = xml_msg_get_prefix(msg, 
                              parent_nsid,
                              nsid, 
                              NULL, 
                              &xneeded);
    if (pfix) {
        ses_putstr(scb, pfix);
        ses_putchar(scb, ':');
    }

    /* write the element name */
    ses_putstr(scb, elname);

    if (xneeded || (attrQ && !dlq_empty(attrQ))) {
        if (indent >= 0) {
            indent += ses_indent_count(scb);
        }
        if (attrQ) {
            write_attrs(scb, 
                        msg, 
                        attrQ, 
                        isattrq, 
                        NULL, 
                        indent, 
                        nsid);
        }
        if (xneeded) {
            if (!attrQ || dlq_empty(attrQ)) {
                indent = -1;
            }
            write_xmlns_decl(scb, 
                             pfix, 
                             nsid, 
                             indent);
        }
    }

    /* finish up the element */
    if (empty) {
        ses_putchar(scb, '/');
    }
    ses_putchar(scb, '>');

    /* hack in XMLDOC mode to get more readable XSD output */
    if (empty && scb->mode==SES_MODE_XMLDOC && indent < 
        (3*ses_indent_count(scb))) {
        ses_putchar(scb, '\n');
    }

}  /* xml_wr_begin_elem_ex */


/********************************************************************
* FUNCTION xml_wr_begin_elem
*
* Write a start XML tag to the specified session without attributes
*
* INPUTS:
*   scb == session control block
*   msg == top header from message in progress
*   parent_nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_begin_elem (ses_cb_t *scb,
                       xml_msg_hdr_t *msg,
                       xmlns_id_t  parent_nsid,
                       xmlns_id_t  nsid,
                       const xmlChar *elname,
                       int32 indent)
{
    xml_wr_begin_elem_ex(scb, 
                         msg, 
                         parent_nsid,
                         nsid,
                         elname,
                         NULL,
                         FALSE,
                         indent,
                         FALSE);

} /* xml_wr_begin_elem */


/********************************************************************
* FUNCTION xml_wr_empty_elem
*
* Write an empty XML tag to the specified session without attributes
*
* INPUTS:
*   scb == session control block
*   msg == top header from message in progress
*   parent_nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_empty_elem (ses_cb_t *scb,
                       xml_msg_hdr_t *msg,
                       xmlns_id_t  parent_nsid,
                       xmlns_id_t  nsid,
                       const xmlChar *elname,
                       int32 indent)
{
    xml_wr_begin_elem_ex(scb, 
                         msg, 
                         parent_nsid, 
                         nsid, 
                         elname,
                         NULL, 
                         FALSE, 
                         indent,
                         TRUE);

} /* xml_wr_empty_elem */


/********************************************************************
* FUNCTION xml_wr_end_elem
*
* Write an end tag to the specified session
*
* INPUTS:
*   scb == session control block to start msg 
*   msg == header from message in progress
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   indent == number of chars to indent after a newline
*             will be ignored if indent is turned off
*             in the agent profile
*           == -1 means no newline or indent
*           == 0 means just newline
*
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_end_elem (ses_cb_t *scb,
                  xml_msg_hdr_t *msg,
                  xmlns_id_t  nsid,
                  const xmlChar *elname,
                  int32 indent)
{
    const xmlChar       *pfix;
    boolean              xneeded;

#ifdef DEBUG
    if (!scb || !msg || !elname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    ses_indent(scb, indent);

    /* start the element and write the prefix, if any */
    ses_putchar(scb, '<');
    ses_putchar(scb, '/');
    pfix = xml_msg_get_prefix(msg, 
                              0, 
                              nsid, 
                              NULL, 
                              &xneeded);
    if (pfix) {
        ses_putstr(scb, pfix);
        ses_putchar(scb, ':');
    }

    /* write the element name */
    ses_putstr(scb, elname);
        
    /* finish up the element */
    ses_putchar(scb, '>');

    /* hack in XMLDOC mode to get more readable XSD output */
    if (scb->mode==SES_MODE_XMLDOC && 
        indent==ses_indent_count(scb)) {
        ses_putchar(scb, '\n');
    }

}  /* xml_wr_end_elem */


/********************************************************************
* FUNCTION xml_wr_string_elem
*
* Write a start tag, simple string content, and an end tag
* to the specified session.  A flag element and
* ename will vary from this format.
*
* Simple content nodes are completed on a single line to
* prevent introduction of extra whitespace
*
* INPUTS:
*   scb == session control block
*   msg == header from message in progress
*   str == simple string to write as element content
*   parent_nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   attrQ == Q of xml_attr_t records to write in
*            the element; NULL == none
*   isattrq == TRUE for Q of xml_attr_t, FALSE for val_value_t
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
*  
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_string_elem (ses_cb_t *scb,
                        xml_msg_hdr_t *msg,
                        const xmlChar *str,
                        xmlns_id_t  parent_nsid,
                        xmlns_id_t  nsid,
                        const xmlChar *elname,
                        const dlq_hdr_t *attrQ,
                        boolean isattrq,
                        int32 indent)
{

#ifdef DEBUG
    if (!scb || !msg || !str || !elname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    xml_wr_begin_elem_ex(scb, 
                         msg, 
                         parent_nsid, 
                         nsid, 
                         elname, 
                         attrQ,
                         isattrq, 
                         indent,
                         FALSE);
    ses_putstr(scb, str);
    xml_wr_end_elem(scb, 
                    msg, 
                    nsid,
                    elname,
                    -1);

}  /* xml_wr_string_elem */


/********************************************************************
* FUNCTION xml_wr_qname_elem
*
* Write a start tag, QName string content, and an end tag
* to the specified session.
*
* The ses_start_msg must be called before this
* function, in order for it to allow any writes
*
* INPUTS:
*   scb == session control block
*   msg == header from message in progres
*   val_nsid == namespace ID of the QName prefix
*   str == local-name part of the QName
*   parent_nsid == namespace ID of the parent element
*   nsid == namespace ID of the element to write
*   elname == unqualified name of element to write
*   attrQ == Q of xml_attr_t records to write in
*            the element; NULL == none
*   isattrq == TRUE for Q of xml_attr_t, FALSE for val_value_t
*   indent == number of chars to indent after a newline
*           == -1 means no newline or indent
*           == 0 means just newline
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_qname_elem (ses_cb_t *scb,
                       xml_msg_hdr_t *msg,
                       xmlns_id_t val_nsid,
                       const xmlChar *str,
                       xmlns_id_t  parent_nsid,
                       xmlns_id_t  nsid,
                       const xmlChar *elname,
                       const dlq_hdr_t *attrQ,
                       boolean isattrq,
                       int32 indent)
{
    boolean         xneeded;
    const xmlChar  *pfix;

#ifdef DEBUG
    if (!scb || !msg || !str || !elname) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    xml_wr_begin_elem_ex(scb,
                         msg,
                         parent_nsid, 
                         nsid,
                         elname,
                         attrQ,
                         isattrq, 
                         indent,
                         FALSE);

    /* counting on xmlns decl to be in ancestor node
     * because the xneeded node is being ignored here
     */
    pfix = xml_msg_get_prefix(msg,
                              parent_nsid,
                              val_nsid, 
                              NULL,
                              &xneeded);
    if (pfix) {
        ses_putstr(scb, pfix);
        ses_putchar(scb, XMLNS_SEPCH);
    }
    ses_putstr(scb, str);

    xml_wr_end_elem(scb,
                    msg,
                    nsid,
                    elname,
                    -1);

}  /* xml_wr_qname_elem */


/********************************************************************
* FUNCTION xml_wr_check_val
* 
* Write an NCX value in XML encoding
* while checking nodes for suppression of output with
* the supplied test fn
*
* !!! NOTE !!!
* 
* This function generates the contents of the val_value_t
* but not the top node itself.  This function is called
* recursively and this is the intended behavior.
*
* To generate XML for an entire val_value_t, including
* the top-level node, use the xml_wr_full_val fn.
*
* If the acm_cache and acm_cbfn fields are set in
* the msg header then access control will be checked
* If FALSE, then nothing will be written to the output session
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   testcb == callback function to use, NULL if not used
*   
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_check_val (ses_cb_t *scb,
                      xml_msg_hdr_t *msg,
                      val_value_t *val,
                      int32  indent,
                      val_nodetest_fn_t testfn)
{

#ifdef DEBUG
    if (!scb || !msg || !val) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif
    write_check_val(scb, msg, val, indent, testfn, TRUE);

}  /* xml_wr_check_val */


/********************************************************************
* FUNCTION xml_wr_val
* 
* output val_value_t node contents only
* Write an NCX value node in XML encoding
* See xml_wr_check_write for full details of this fn.
* It is the same, except a NULL testfn is supplied.
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_val (ses_cb_t *scb,
                xml_msg_hdr_t *msg,
                val_value_t *val,
                int32  indent)
{
    xml_wr_check_val(scb, 
                     msg, 
                     val, 
                     indent, 
                     NULL);

}  /* xml_wr_val */


/********************************************************************
* FUNCTION xml_wr_full_check_val
* 
* generate entire val_value_t *w/filter)
* Write an entire val_value_t out as XML, including the top level
* Using an optional testfn to filter output
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   testcb == callback function to use, NULL if not used
*   
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_full_check_val (ses_cb_t *scb,
                           xml_msg_hdr_t *msg,
                           val_value_t *val,
                           int32  indent,
                           val_nodetest_fn_t testfn)
{
    val_value_t       *vir;
    val_value_t       *out;
    val_value_t       *realval;
    typ_def_t         *realtypdef;
    xml_msg_authfn_t   cbfn;
    status_t           res;
    boolean            acmtest;
    ncx_btype_t        btyp;

#ifdef DEBUG
    if (!scb || !msg || !val) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    vir = NULL;
    realval = NULL;

    if (val_is_virtual(val)) {
        if (testfn) {
            if (!(*testfn)(msg->withdef, FALSE, val)) {
                return;
            }
        }

        vir = val_get_virtual_value(scb, val, &res);
        if (!vir) {
            if (res != ERR_NCX_SKIPPED) {
                /*** handle inline error ***/
            }
            return;
        }
        out = vir;
    } else {
        out = val;
    }

    /* check the user filter callback function */
    if (testfn) {
        if (!(*testfn)(msg->withdef, TRUE, out)) {
            if (vir) {
                val_free_value(vir);
            }
            return;   /* skip this entry: filtered */
        }
    }

    if (msg->acm_cbfn) {
        cbfn = (xml_msg_authfn_t)msg->acm_cbfn;
        acmtest = (*cbfn)(msg, scb->username, val);
        if (!acmtest) {
            if (vir) {
                val_free_value(vir);
            }
            return;  /* skip this entry: access-denied */
        }
    }

    if (out->btyp == NCX_BT_LEAFREF) {
        realtypdef = typ_get_xref_typdef(out->typdef);
        if (realtypdef) {
            res = NO_ERR;
            btyp = typ_get_basetype(realtypdef);

            switch (btyp) {
            case NCX_BT_STRING:
            case NCX_BT_BINARY:
            case NCX_BT_BOOLEAN:
            case NCX_BT_ENUM:
                break;
            default:
                realval = val_make_simval(realtypdef,
                                          val_get_nsid(out),
                                          out->name,
                                          VAL_STR(out),
                                          &res);
                if (realval) {
                    out = realval;
                } else {
                    log_error("\nError: write leafref '%s' failed (%s)",
                              out->name,
                              get_error_string(res));
                }
            }
        } else {
            SET_ERROR(ERR_INTERNAL_VAL);
        }
    }

    /* check if this is a false (not present) flag */
    if (out->btyp==NCX_BT_EMPTY && !VAL_BOOL(out)) {
        if (vir) {
            val_free_value(vir);
        }
        if (realval) {
            val_free_value(realval);
        }
        return;
    }

    /* write the value node contents or an empty node if none */
    if (out->btyp == NCX_BT_IDREF) {
        /* write a complete QName element */
        xml_wr_qname_elem(scb, 
                          msg, 
                          out->v.idref.nsid,
                          out->v.idref.name,
                          (out->parent) ? out->parent->nsid : 0,
                          out->nsid, 
                          out->name,
                          &out->metaQ, 
                          FALSE, 
                          indent);
    } else if (val_has_content(out)) {
        /* write the top-level start node */
        begin_elem_val(scb, 
                       msg, 
                       out, 
                       indent);

        /* write the value node contents; skip ACM on this node */
        write_check_val(scb, 
                        msg, 
                        out, 
                        indent+ses_indent_count(scb), 
                        testfn,
                        FALSE);

        /* write the top-level end node */
        xml_wr_end_elem(scb, 
                        msg, 
                        out->nsid, 
                        out->name, 
                        fit_on_line(scb, out) ? -1 : indent);
    } else {
        /* write the top-level empty node */
        begin_elem_val(scb, 
                       msg, 
                       out, 
                       indent);
    }

    if (vir) {
        val_free_value(vir);
    }
    if (realval) {
        val_free_value(realval);
    }

}  /* xml_wr_full_check_val */


/********************************************************************
* FUNCTION xml_wr_full_val
* 
* generate entire val_value_t
* Write an entire val_value_t out as XML, including the top level
*
* INPUTS:
*   scb == session control block
*   msg == xml_msg_hdr_t in progress
*   val == value to write
*   indent == start indent amount if indent enabled
*   
* RETURNS:
*   none
*********************************************************************/
void
    xml_wr_full_val (ses_cb_t *scb,
                     xml_msg_hdr_t *msg,
                     val_value_t *val,
                     int32  indent)
{
    xml_wr_full_check_val(scb, 
                          msg, 
                          val, 
                          indent, 
                          NULL);
                                
} /* xml_wr_full_val */


/********************************************************************
* FUNCTION xml_wr_check_open_file
* 
* Write the specified value to an open FILE in XML format
*
* INPUTS:
*    fp == open FILE control block
*    val == value for output
*    attrs == top-level attributes to generate
*    docmode == TRUE if XML_DOC output mode should be used
*            == FALSE if XML output mode should be used
*    xmlhdr == TRUE if <?xml?> directive should be output
*            == FALSE if not
*    startindent == starting indent point
*    indent == indent amount (0..9 spaces)
*    testfn == callback test function to use
*
* RETURNS:
*    status
*********************************************************************/
status_t
    xml_wr_check_open_file (FILE *fp, 
                            val_value_t *val,
                            xml_attrs_t *attrs,
                            boolean docmode,
                            boolean xmlhdr,
                            int32 startindent,
                            int32  indent,
                            val_nodetest_fn_t testfn)
{
    ses_cb_t   *scb;
    rpc_msg_t  *msg;
    xml_attrs_t myattrs;
    status_t    res;
    ses_mode_t  sesmode;
    boolean     anyout, fitoneline, hascontent;

#ifdef DEBUG
    if (!fp || !val) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    res = NO_ERR;
    msg = NULL;
    anyout = FALSE;
    hascontent = TRUE;
    indent = min(indent, 9);
    sesmode = SES_MODE_NONE;
    xml_init_attrs(&myattrs);

    /* get a dummy session control block */
    scb = ses_new_dummy_scb();
    if (!scb) {
        res = ERR_INTERNAL_MEM;
    } else {
        scb->fp = fp;
        scb->indent = indent;
    }

    /* get a dummy output message */
    if (res == NO_ERR) {
        msg = rpc_new_out_msg();
        if (!msg) {
            res = ERR_INTERNAL_MEM;
        } else {
            /* hack -- need a queue because there is no top
             * element which this usually shadows
             */
            msg->rpc_in_attrs = (attrs) ? attrs : &myattrs;
        }
    }

    /* XML output mode will add more whitespace if this is ncxdump calling */
    if (res == NO_ERR && docmode) {
        sesmode = ses_get_mode(scb);
        ses_set_mode(scb, SES_MODE_XMLDOC);
    }
    
    /* send the XML declaration */
    if (res == NO_ERR && xmlhdr) {
        res = ses_start_msg(scb);
        if (res == NO_ERR) {
            anyout = TRUE;
        }
    }

    /* setup an empty prefix map */
    if (res == NO_ERR) {
        res = xml_msg_build_prefix_map(&msg->mhdr,
                                       msg->rpc_in_attrs, 
                                       FALSE, 
                                       FALSE);
    }

    /* cannot use xml_wr_full_val because that
     * function assumes the attrQ is val_value_t
     * but it is really xml_attr_t
     *
     * !!! not handling i-i and XPath strings yet !!!
     */
    if (res == NO_ERR) {
        if (!val_has_content(val)) {
            /* print empty element */
            xml_wr_begin_elem_ex(scb, 
                                 &msg->mhdr,
                                 0, 
                                 val->nsid, 
                                 val->name, 
                                 attrs, 
                                 TRUE, 
                                 startindent, 
                                 TRUE);
            hascontent = FALSE;
        } else if (val->btyp == NCX_BT_IDREF) {
            /* print start QName element */
            xml_wr_qname_elem(scb, 
                              &msg->mhdr,
                              val->v.idref.nsid,
                              val->v.idref.name, 
                              0,
                              val->nsid, 
                              val->name,
                              attrs, 
                              TRUE, 
                              startindent);
        } else {
            /* print start normal string node element */
            xml_wr_begin_elem_ex(scb, 
                                 &msg->mhdr,
                                 0, 
                                 val->nsid, 
                                 val->name, 
                                 attrs, 
                                 TRUE, 
                                 startindent, 
                                 FALSE);
        }

        anyout = TRUE;

        if (hascontent) {
            fitoneline = fit_on_line(scb, val);

            /* output the contents of the value */
            xml_wr_check_val(scb, 
                             &msg->mhdr, 
                             val, 
                             (fitoneline) ? -1 : indent, 
                             testfn);

            /* generate the <foo> end tag */
            xml_wr_end_elem(scb, 
                            &msg->mhdr, 
                            val->nsid, 
                            val->name,
                            (fitoneline) ? -1 : startindent);
        }
    }

    /* finish the message, should be NO-OP  */
    if (anyout) {
        ses_finish_msg(scb);
    }

    if (res == NO_ERR && docmode) {
        ses_set_mode(scb, sesmode);
    }

    /* clean up and exit */
    if (msg) {
        rpc_free_msg(msg);
    }
    if (scb) {
        scb->fp = NULL;   /* do not close the file */
        ses_free_scb(scb);
    }
    xml_clean_attrs(&myattrs);

    return res;

} /* xml_wr_check_open_file */


/********************************************************************
* FUNCTION xml_wr_check_file
* 
* Write the specified value to a FILE in XML format
*
* INPUTS:
*    filespec == exact path of filename to open
*    val == value for output
*    attrs == top-level attributes to generate
*    docmode == TRUE if XML_DOC output mode should be used
*            == FALSE if XML output mode should be used
*    xmlhdr == TRUE if <?xml?> directive should be output
*            == FALSE if not
*    startindent == starting indent point
*    indent == indent amount (0..9 spaces)
*    testfn == callback test function to use
*
* RETURNS:
*    status
*********************************************************************/
status_t
    xml_wr_check_file (const xmlChar *filespec, 
                       val_value_t *val,
                       xml_attrs_t *attrs,
                       boolean docmode,
                       boolean xmlhdr,
                       int32 startindent,
                       int32  indent,
                       val_nodetest_fn_t testfn)
{
    FILE       *fp;
    status_t    res;

#ifdef DEBUG
    if (!filespec || !val || !attrs) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    fp = fopen((const char *)filespec, "w");
    if (!fp) {
        log_error("\nError: Cannot open XML file '%s'", filespec);
        return ERR_FIL_OPEN;
    }
    res = xml_wr_check_open_file(fp,
                                 val,
                                 attrs,
                                 docmode,
                                 xmlhdr,
                                 startindent,
                                 indent,
                                 testfn);
    fclose(fp);

    return res;

} /* xml_wr_check_file */


/********************************************************************
* FUNCTION xml_wr_file
* 
* Write the specified value to a FILE in XML format
*
* INPUTS:
*    filespec == exact path of filename to open
*    val == value for output
*    attrs == top-level attributes to generate
*    docmode == TRUE if XML_DOC output mode should be used
*            == FALSE if XML output mode should be used
*    xmlhdr == TRUE if <?xml?> directive should be output
*            == FALSE if not
*    startindent == starting indent point
*    indent == indent amount (0..9 spaces)
*
* RETURNS:
*    status
*********************************************************************/
status_t
    xml_wr_file (const xmlChar *filespec,
                 val_value_t *val,
                 xml_attrs_t *attrs,
                 boolean docmode,
                 boolean xmlhdr,
                 int32 startindent,
                 int32 indent)
{
#ifdef DEBUG
    if (!filespec || !val || !attrs) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    return xml_wr_check_file(filespec, 
                             val, 
                             attrs, 
                             docmode, 
                             xmlhdr, 
                             startindent,
                             indent, 
                             NULL);

} /* xml_wr_file */



/* END file xml_wr.c */
