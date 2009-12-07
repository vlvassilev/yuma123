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
/*  FILE: yin.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
23feb08      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ext
#include "ext.h"
#endif

#ifndef _H_grp
#include "grp.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncxmod
#include "ncxmod.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_xmlns
#include "xmlns.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

#ifndef _H_xml_val
#include "xml_val.h"
#endif

#ifndef _H_xpath
#include "xpath.h"
#endif

#ifndef _H_xsd_util
#include "xsd_util.h"
#endif

#ifndef _H_yang
#include "yang.h"
#endif

#ifndef _H_yangconst
#include "yangconst.h"
#endif

#ifndef _H_yangdump
#include "yangdump.h"
#endif

#ifndef _H_yangdump_util
#include "yangdump_util.h"
#endif

#ifndef _H_yin
#include "yin.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/
typedef struct yin_mapping_t_ {
    const xmlChar *keyword;
    const xmlChar *argname;       /* may be NULL */
    boolean        elem;
} yin_mapping_t;


/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/
static yin_mapping_t yinmap[] = {
    { YANG_K_ANYXML, YANG_K_NAME, FALSE },
    { YANG_K_ARGUMENT, YANG_K_NAME, FALSE }, 
    { YANG_K_AUGMENT, YANG_K_TARGET_NODE, FALSE },
    { YANG_K_BASE, YANG_K_NAME, FALSE },
    { YANG_K_BELONGS_TO, YANG_K_MODULE, FALSE},
    { YANG_K_BIT, YANG_K_NAME, FALSE},
    { YANG_K_CASE, YANG_K_NAME, FALSE },
    { YANG_K_CHOICE, YANG_K_NAME, FALSE },
    { YANG_K_CONFIG, YANG_K_VALUE, FALSE },
    { YANG_K_CONTACT, YANG_K_INFO, TRUE },
    { YANG_K_CONTAINER, YANG_K_NAME, FALSE },
    { YANG_K_DEFAULT, YANG_K_VALUE, FALSE },
    { YANG_K_DESCRIPTION, YANG_K_TEXT, TRUE },
    { YANG_K_DEVIATE, YANG_K_VALUE, FALSE },
    { YANG_K_DEVIATION, YANG_K_TARGET_NODE, FALSE },
    { YANG_K_ENUM, YANG_K_NAME, FALSE },
    { YANG_K_ERROR_APP_TAG, YANG_K_VALUE, FALSE },
    { YANG_K_ERROR_MESSAGE, YANG_K_VALUE, TRUE },
    { YANG_K_EXTENSION, YANG_K_NAME, FALSE },
    { YANG_K_FEATURE, YANG_K_NAME, FALSE },
    { YANG_K_FRACTION_DIGITS, YANG_K_VALUE, FALSE },
    { YANG_K_GROUPING, YANG_K_NAME, FALSE },
    { YANG_K_IDENTITY, YANG_K_NAME, FALSE },
    { YANG_K_IF_FEATURE, YANG_K_NAME, FALSE },
    { YANG_K_IMPORT, YANG_K_MODULE, FALSE },
    { YANG_K_INCLUDE, YANG_K_MODULE, FALSE },
    { YANG_K_INPUT, NULL, FALSE },
    { YANG_K_KEY, YANG_K_VALUE, FALSE },
    { YANG_K_LEAF, YANG_K_NAME, FALSE },
    { YANG_K_LEAF_LIST, YANG_K_NAME, FALSE },
    { YANG_K_LENGTH, YANG_K_VALUE, FALSE },
    { YANG_K_LIST, YANG_K_NAME, FALSE },
    { YANG_K_MANDATORY, YANG_K_VALUE, FALSE },
    { YANG_K_MAX_ELEMENTS, YANG_K_VALUE, FALSE },
    { YANG_K_MIN_ELEMENTS, YANG_K_VALUE, FALSE },
    { YANG_K_MODULE, YANG_K_NAME, FALSE },
    { YANG_K_MUST, YANG_K_CONDITION, FALSE },
    { YANG_K_NAMESPACE, YANG_K_URI, FALSE },
    { YANG_K_NOTIFICATION, YANG_K_NAME, FALSE },
    { YANG_K_ORDERED_BY, YANG_K_VALUE, FALSE },
    { YANG_K_ORGANIZATION, YANG_K_INFO, TRUE },
    { YANG_K_OUTPUT, NULL, FALSE },
    { YANG_K_PATH, YANG_K_VALUE, FALSE },
    { YANG_K_PATTERN, YANG_K_VALUE, FALSE },
    { YANG_K_POSITION, YANG_K_VALUE, FALSE },
    { YANG_K_PREFIX, YANG_K_VALUE, FALSE },
    { YANG_K_PRESENCE, YANG_K_VALUE, FALSE },
    { YANG_K_RANGE, YANG_K_VALUE, FALSE },
    { YANG_K_REFERENCE, YANG_K_INFO, FALSE },
    { YANG_K_REFINE, YANG_K_TARGET_NODE, FALSE },
    { YANG_K_REQUIRE_INSTANCE, YANG_K_VALUE, FALSE },
    { YANG_K_REVISION, YANG_K_DATE, FALSE },
    { YANG_K_REVISION_DATE, YANG_K_DATE, FALSE },
    { YANG_K_RPC, YANG_K_NAME, FALSE },
    { YANG_K_STATUS, YANG_K_VALUE, FALSE },
    { YANG_K_SUBMODULE, YANG_K_NAME, FALSE },
    { YANG_K_TYPE, YANG_K_NAME, FALSE },
    { YANG_K_TYPEDEF, YANG_K_NAME, FALSE },
    { YANG_K_UNIQUE, YANG_K_TAG, FALSE },
    { YANG_K_UNITS, YANG_K_NAME, FALSE },
    { YANG_K_USES, YANG_K_NAME, FALSE },
    { YANG_K_VALUE, YANG_K_VALUE, FALSE },
    { YANG_K_WHEN, YANG_K_CONDITION, FALSE },
    { YANG_K_YANG_VERSION, YANG_K_VALUE, FALSE },
    { YANG_K_YIN_ELEMENT, YANG_K_VALUE, FALSE },
    { NULL, NULL, FALSE }
};


/********************************************************************
* FUNCTION find_yin_mapping
* 
* Find a static yin mapping entry
*
* INPUTS:
*   name == keyword name to find
*
* RETURNS:
*   pointer to found entry, NULL if none found
*********************************************************************/
static const yin_mapping_t *
    find_yin_mapping (const xmlChar *name)
{
    const yin_mapping_t  *mapping;
    int                   i;

    i = 0;
    for (mapping = &yinmap[i];
         mapping != NULL && mapping->keyword != NULL;
         mapping = &yinmap[++i]) {
        if (!xml_strcmp(name, mapping->keyword)) {
            return mapping;
        }
    }
    return NULL;
         
}  /* find_yin_mapping */


/********************************************************************
* FUNCTION start_yin_elem
* 
* Generate a start tag
*
* INPUTS:
*   scb == session control block to use for writing
*   name == element name
*   indent == indent count
*
*********************************************************************/
static void
    start_yin_elem (ses_cb_t *scb,
                const xmlChar *name,
                int32 indent)

{
    ses_putstr_indent(scb, (const xmlChar *)"<", indent);
    ses_putstr(scb, name);

}  /* start_yin_elem */


/********************************************************************
* FUNCTION end_yin_elem
* 
* Generate an end tag
*
* INPUTS:
*   scb == session control block to use for writing
*   name == element name
*   indent == indent count
*
*********************************************************************/
static void
    end_yin_elem (ses_cb_t *scb,
                  const xmlChar *name,
                  int32 indent)

{
    ses_putstr_indent(scb, (const xmlChar *)"</", indent);
    ses_putstr(scb, name);
    ses_putchar(scb, '>');

}  /* end_yin_elem */

#ifdef NOT_YET
/********************************************************************
* FUNCTION start_ext_elem
* 
* Generate a start tag for an extension
*
* INPUTS:
*   scb == session control block to use for writing
*   prefix == prefix string to use
*   name == element name
*   indent == indent count
*
*********************************************************************/
static void
    start_ext_elem (ses_cb_t *scb,
                    const xmlChar *prefix,
                    const xmlChar *name,
                    int32 indent)
{
    ses_putstr_indent(scb, (const xmlChar *)"<", indent);
    ses_putstr(scb, prefix);
    ses_putchar(scb, ':');
    ses_putstr(scb, name);

}  /* start_ext_elem */


/********************************************************************
* FUNCTION end_ext_elem
* 
* Generate an end tag
*
* INPUTS:
*   scb == session control block to use for writing
*   name == element name
*   indent == indent count
*
*********************************************************************/
static void
    end_ext_elem (ses_cb_t *scb,
                  const xmlChar *prefix,
                  const xmlChar *name,
                  int32 indent)
{
    ses_putstr_indent(scb, (const xmlChar *)"</", indent);
    ses_putstr(scb, prefix);
    ses_putchar(scb, ':');
    ses_putstr(scb, name);
    ses_putchar(scb, '>');

}  /* end_ext_elem */
#endif


/********************************************************************
* FUNCTION write_import_xmlns
* 
* Generate an xmlns attribute for the import 
*
* INPUTS:
*   scb == session control block to use for writing
*   imp == ncx_import_t struct to use
*   indent == start indent count
*
*********************************************************************/
static void
    write_import_xmlns (ses_cb_t *scb,
                        ncx_import_t *imp,
                        int32 indent)

{
    if (imp->mod == NULL) {
        return;
    }

    ses_putstr_indent(scb, XMLNS, indent);
    ses_putchar(scb, ':');
    ses_putstr(scb, imp->prefix);
    ses_putchar(scb, '=');
    ses_putchar(scb, '"');
    ses_putstr(scb, ncx_get_modnamespace(imp->mod));
    ses_putchar(scb, '"');

}  /* write_import_xmlns */


/********************************************************************
* FUNCTION write_yin_attr
* 
* Generate an attibute declaration
*
* INPUTS:
*   scb == session control block to use for writing
*   attr == attribute name
*   value == attribute value string
*   indent == start indent count
*
*********************************************************************/
static void
    write_yin_attr (ses_cb_t *scb,
                    const xmlChar *attr,
                    const xmlChar *value,
                    int32 indent)

{
    ses_putstr_indent(scb, attr, indent);
    ses_putchar(scb, '=');
    ses_putchar(scb, '"');
    ses_putstr(scb, value);
    ses_putchar(scb, '"');

}  /* write_yin_attr */


/********************************************************************
* FUNCTION advance_token
* 
* Move to the next token
*
* INPUTS:

*
*********************************************************************/
static status_t
    advance_token (yang_pcb_t *pcb)                   
{
    status_t   res;

    res = TK_ADV(pcb->tkc);
    if (res == NO_ERR) {
        tk_dump_token(pcb->tkc->cur);
    }

    return res;

}  /* advance_token */


/********************************************************************
* FUNCTION write_yin_stmt
*  
* Go through the token chain and write YIN stmts
* recursively if needed, until 1 YANG stmt is handled
*
* INPUTS:
*   pcb == parser control block of module to convert
*          This is returned from ncxmod_load_module_ex
*   cp == conversion parms to use
*   scb == session control block for writing output
*   startindent == start indent count
*   done == address of done return var to use
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    write_yin_stmt (yang_pcb_t *pcb,
                    const yangdump_cvtparms_t *cp,
                    ses_cb_t *scb,
                    int32 startindent,
                    boolean *done)
{
    const yin_mapping_t  *mapping;
    status_t              res;
    boolean               loopdone;

    res = NO_ERR;
    *done = FALSE;

    /* expecting a keyword [string] stmt-end sequence
     * or the very last closing right brace
     */
    if (TK_CUR_TYP(pcb->tkc) == TK_TT_RBRACE) {
        if (tk_next_typ(pcb->tkc) == TK_TT_NONE) {
            *done = TRUE;
            return NO_ERR;
        } else {
            return ERR_NCX_WRONG_TKTYPE;
        }
    } else if (!TK_CUR_ID(pcb->tkc)) {
        return ERR_NCX_WRONG_TKTYPE;
    }

    /* check the keyword type */
    switch (TK_CUR_TYP(pcb->tkc)) {
    case TK_TT_TSTRING:
        /* YANG keyword */
        mapping = find_yin_mapping(TK_CUR_VAL(pcb->tkc));
        if (mapping == NULL) {
            return ERR_NCX_DEF_NOT_FOUND;
        }

        /* output keyword part */
        start_yin_elem(scb, mapping->keyword, startindent);

        /* output [string] part if expected */
        if (mapping->argname == NULL) {
            ses_putchar(scb, '>');
        } else {
            /* move token pointer to the argument string */
            res = advance_token(pcb);
            if (res != NO_ERR) {
                return res;
            }

            /* write the string part */
            if (mapping->elem) {
                ses_putchar(scb, '>');

                /* encode argname,value as an element */
                start_yin_elem(scb, 
                               mapping->argname,
                               startindent + cp->indent);
                ses_putchar(scb, '>');
                ses_putstr_indent(scb,
                                  TK_CUR_VAL(pcb->tkc),
                                  startindent + (cp->indent * 2));
                end_yin_elem(scb, 
                             mapping->argname, 
                             startindent + cp->indent);
            } else {
                /* encode argname,value as an attribute */
                ses_putchar(scb, ' ');
                write_yin_attr(scb,
                               mapping->argname,
                               TK_CUR_VAL(pcb->tkc),
                               -1);
                if (tk_next_typ(pcb->tkc) != TK_TT_SEMICOL) {
                    ses_putchar(scb, '>');
                } /* else end with empty element */
            }
        }

        /* move token pointer to the stmt-end char */
        res = advance_token(pcb);
        if (res != NO_ERR) {
            return res;
        }

        switch (TK_CUR_TYP(pcb->tkc)) {
        case TK_TT_SEMICOL:
            /* advance to next stmt, this one is done */
            res = advance_token(pcb);
            if (res != NO_ERR) {
                return res;
            }
            if (mapping->elem) {
                /* end the complex element */
                end_yin_elem(scb, mapping->keyword, startindent);
            } else {
                /* end the empty element */
                ses_putstr(scb, (const xmlChar *)" />");
            }
            break;
        case TK_TT_LBRACE:
            /* advance to next sub-stmt, this one has child nodes */
            res = advance_token(pcb);
            if (res != NO_ERR) {
                return res;
            }

            /* write the nested sub-stmts as child nodes */
            if (TK_CUR_TYP(pcb->tkc) != TK_TT_RBRACE) {
                loopdone = FALSE;
                while (!loopdone) {
                    res = write_yin_stmt(pcb,
                                         cp,
                                         scb,
                                         startindent + cp->indent,
                                         done);
                    if (res != NO_ERR) {
                        return res;
                    }
                    if (TK_CUR_TYP(pcb->tkc) == TK_TT_RBRACE) {
                        loopdone = TRUE;
                    }
                }
            }

            /* move to next stmt, this one is done */
            res = advance_token(pcb);
            if (res != NO_ERR) {
                return res;
            }

            /* end the complex element */
            end_yin_elem(scb, mapping->keyword, startindent);
            break;
        default:
            return SET_ERROR(ERR_INTERNAL_VAL);
        }
        break;
    case TK_TT_MSTRING:
        /* extension keyword */
        break;
    default:
        return SET_ERROR(ERR_INTERNAL_VAL);
    }
    

    return res;

}   /* write_yin_stmt */


/********************************************************************
* FUNCTION write_yin_contents
*  
* Go through the token chain and write all the
* YIN elements according to algoritm in sec. 11
*
* INPUTS:
*   pcb == parser control block of module to convert
*          This is returned from ncxmod_load_module_ex
*   cp == conversion parms to use
*   scb == session control block for writing output
*
* RETURNS:
*   status
*********************************************************************/
static status_t
    write_yin_contents (yang_pcb_t *pcb,
                        const yangdump_cvtparms_t *cp,
                        ses_cb_t *scb)
{
    status_t       res;
    boolean        done;
    int            i;

    tk_reset_chain(pcb->tkc);
    res = NO_ERR;

    /* need to skip the first 3 tokens because the
     * [sub]module name { tokens have already been
     * handled as a special case 
     */
    for (i = 0; i < 4; i++) {
        res = advance_token(pcb);
        if (res != NO_ERR) {
            return res;
        }
    }

    done = FALSE;
    while (!done && res == NO_ERR) {
        res = write_yin_stmt(pcb, 
                             cp, 
                             scb, 
                             cp->indent,
                             &done);
    }

    return res;

}   /* write_yin_contents */


/*********     E X P O R T E D   F U N C T I O N S    **************/


/********************************************************************
* FUNCTION yin_convert_module
*  
*  The YIN namespace will be the default namespace
*  The imported modules will use the xmlprefix in use
*  which is the YANG prefix unless it is a duplicate
*
* INPUTS:
*   pcb == parser control block of module to convert
*          This is returned from ncxmod_load_module_ex
*   cp == conversion parms to use
*   scb == session control block for writing output
*
* RETURNS:
*   status
*********************************************************************/
status_t
    yin_convert_module (yang_pcb_t *pcb,
                        const yangdump_cvtparms_t *cp,
                        ses_cb_t *scb)
{
    ncx_import_t  *import;
    status_t       res;
    int32          indent;

    res = NO_ERR;
    indent = cp->indent;

    /* start the XML document */
    ses_putstr(scb, XML_START_MSG);

    /* write the top-level start-tag for [sub]module
     * do this special case so the XMLNS attributes
     * can be generated as well as the 'name' attribute
     */
    start_yin_elem(scb,
                   (pcb->top->ismod) 
                   ? YANG_K_MODULE : YANG_K_SUBMODULE,
                   0);
    ses_putchar(scb, ' ');

    /* write the name attribute */
    write_yin_attr(scb, YANG_K_NAME, pcb->top->name, indent);

    /* write the YIN namespace decl as the default namespace */
    write_yin_attr(scb, XMLNS, YIN_URN, indent);

    /* write the xmlns decls for all the imports used
     * by this [sub]module
     */
    for (import = (ncx_import_t *)dlq_firstEntry(&pcb->top->importQ);
         import != NULL;
         import = (ncx_import_t *)dlq_nextEntry(import)) {
        if (import->used) {
            write_import_xmlns(scb, import, indent);
        }
    }

    /* finish the top-level start tag */
    ses_putchar(scb, '>');


    res = write_yin_contents(pcb, cp, scb);

    /* finish the top-level element */

    end_yin_elem(scb,
                 (pcb->top->ismod) 
                 ? YANG_K_MODULE : YANG_K_SUBMODULE,
                 0);

    return res;

}   /* yin_convert_module */


/* END file yin.c */
