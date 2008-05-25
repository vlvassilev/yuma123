/*  FILE: ncx_parse.c


    This module parses an XML instance document which contains
    a valid NCX module as defined in the XSD:

          ----------- TBD --------

    The NCX file is converted to a C ncx_module_t
    data structure during parsing before the definitions
    are finally registered with the def_reg module.


Sample NCX Module


-----------------

<ncx-module>

  <header>
    <name>ModuleName</name>
    <version>moduleVersionString</version>
    <owner>applicationOwnerName</owner>
    <application>applicationName</application>
    <copyright>CopyrightString</copyright>
    <contact-info>Contact Info</contact-info>
   [<description>Optional Module Description</description>]
   [<namespace>OverrideDefaultNamespaceID</namespace>
  </header>

  <imports>
    <import>
      <module>ModuleName</module>
      [<items>OptionalModuleItemList</items>]
    </import>
  </imports>

  <definitions>

    <type>
      <name>
      [<description>]
      <syntax> -- see syntax.h --  </syntax>
    </type>

    <parmset>
      <name>ncpeer</name>
      [<description>Parameters for the NETCONFX peer engine</description>]
      <parms>
        <parm>
          <name>def-binding</name>
          <type>string</type>
          <default>ssh</default>
        </parm>
        <parm>
          <name>binding-order</name>
          <type>list</type>
          <default>ssh beep soap</default>
        </parm>
      </parms>
    </parmset>

    <monitor>
      <name>ncstats</name>
      [<description>Statistics for the NETCONFX peer engine</description>]
      <objects>
        <object>
          <name>nc-stats</name>
          <type>NcStats</type>
        </object>
        <object>
          <name>nc-stats-history</name>
          <type>NcStatsHistory</type>
        </object>
      </objects>
    </monitor>

    <rpc>
      <name>
      <rpc-type> other | config | exec | monitor </rpc-type>
      <in-psd>
      <out-psd>
     [<description>]
    </rpc>

    <event>
      <name>
      <event-type>
      [<event-subtype>]
      <event-psd>
     [<description>]
    </event>

  </definitions>
</ncx-module>


    
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
29oct05      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>

#include <xmlstring.h>
#include <xmlreader.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_dlq
#include  "dlq.h"
#endif

#ifndef _H_evt
#include  "evt.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_ncc_parse
#include "ncc_parse.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncx_parse
#include "ncx_parse.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_tk
#include  "tk.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_typ_parse
#include  "typ_parse.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/* #define NCX_PARSE_DEBUG 1 */


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                           T Y P E S                               *
*                                                                   *
*********************************************************************/
/* callback function for consume_container function */
typedef status_t (*ncx_containedFn_t) 
    (xmlTextReaderPtr reader, ncx_module_t  *mod);


/* replace consume_descr with more generalized consume_opt_string */
#define consume_descr(R, D, A) \
        consume_opt_string(R, NCX_EL_DESCRIPTION, D, A)


/********************************************************************
* FUNCTION consume_opt_string
* 
* Parse an optional string
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   elname == char * to element name
*   parmstr == address of str pointer to get strdup (if NO_ERR)
*   *adv == advance or not
* OUTPUTS
*   *adv == advance or not
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_opt_string (xmlTextReaderPtr reader, 
                    char *elname,
                    xmlChar **parmstr,
                    boolean *adv)
{
    status_t       res;
    const xmlChar *str;
    uint32         len;

    *parmstr = NULL;

    /* Get the optional string node */
    res = xml_consume_string(reader, xmlns_ncx_id(),
        BAD_CAST elname, *adv, &str, &len);
    switch (res) {
    case NO_ERR:
	/* string is present */
        *adv = TRUE;
        str = xml_trim_string(str, &len);
        if (str) {
            /* need to strdup it because it may not end with 0x0 */
            *parmstr = xml_strndup(str, len);
            return (*parmstr) ? NO_ERR : SET_ERROR(ERR_INTERNAL_MEM);
        }
	return NO_ERR;
    case ERR_XML_READER_NODETYP:
    case ERR_XML_READER_WRONGNAME:
	/* not present -- skip ahead because this field is optional */
	*adv = FALSE;
	return NO_ERR;
    default:
        *adv = FALSE;
	return SET_ERROR(res);
    }
    /*NOTREACHED*/
}  /* consume_opt_string */


/********************************************************************
* FUNCTION consume_header
* 
* Parse the next N nodes as an NCX module header
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mod    == ncx_module_t in progress
*  source == source of this input
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_header (xmlTextReaderPtr reader,
		ncx_module_t  *mod)
{
    boolean        adv;
    status_t       res;
    const xmlChar *str;
    uint32         len;

    /* the next node should be a <header> start tag */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
           BAD_CAST NCX_EL_HEADER, TRUE);
    if (res != NO_ERR) {
	return res;
    }

    /* Get the mandatory <name> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_NAME, TRUE, mod->hdr.modname);
    if (res != NO_ERR) {
	return res;
    }

    /* Get the mandatory <version> field */
    res = xml_consume_string(reader, xmlns_ncx_id(),
	BAD_CAST NCX_EL_VERSION, TRUE, &str, &len);
    if (res != NO_ERR) {
        return NO_ERR;
    }
    str = xml_trim_string(str, &len);
    if (str) {
        if (!len || len > NCX_MAX_NLEN) {
            return SET_ERROR(ERR_NCX_WRONG_LEN);
        }
        xml_strcpy(mod->hdr.version, str);
    } else {
        return SET_ERROR(ERR_NCX_EMPTY_VAL);
    }

    /* Get the mandatory <owner> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
          BAD_CAST NCX_EL_OWNER, TRUE, mod->hdr.owner);
    if (res != NO_ERR) {
	return res;
    }

    /* Get the mandatory <application> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_APPLICATION, TRUE, mod->hdr.appname);
    if (res != NO_ERR) {
	return res;
    }

    /* Check if the optional <copyright> field is present */
    adv = TRUE;
    res = consume_opt_string(reader, NCX_EL_COPYRIGHT, 
         &mod->hdr.copyright, &adv);
    if (res != NO_ERR) {
        return res;
    }

    /* Check if the optional <contact-info> field is present */
    res = consume_opt_string(reader, NCX_EL_CONTACT_INFO, 
         &mod->hdr.contact_info, &adv);
    if (res != NO_ERR) {
        return res;
    }
 
   /* Check if the optional <description> field is present */
    res = consume_descr(reader, &mod->hdr.descr, &adv);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* Check if the optional <namespace> field is present */
    res = consume_opt_string(reader, NCX_EL_NAMESPACE, 
         &mod->hdr.ns, &adv);
    if (res != NO_ERR) {
        return res;
    }

    /* get the 'header' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
            BAD_CAST NCX_EL_HEADER, adv);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    } else {
        return NO_ERR;
    }

}  /* consume_header */


/********************************************************************
* FUNCTION consume_import_items
* 
* Parse the import.items string as a list of item names
* Create an ncx_import_item_t for each one and add it
* to the import->itemQ
*
* INPUTS:
*   import == ncx_import_t struct in progress
*   str    == items string to parse
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_import_items (ncx_import_t  *import,
                      const xmlChar *str)
{
    const xmlChar     *s2;
    ncx_import_item_t *item;
    uint32             len; 
    boolean            done;

    done = FALSE;
    while (!done) {
        /* skip past any whitespace */
        while (xml_isspace(*str)) {
            str++;
        }
        if (!*str) {
            done = TRUE;     /* got to the EOS */
        } else {
            /* start parsing a name string */
            if (!xml_valid_fname_ch(*str)) {
                return SET_ERROR(ERR_NCX_INVALID_NAME);
            }

            /* look for the end of the name */
            s2 = str+1;
            while (xml_valid_name_ch(*s2)) {
                s2++;
            }
    
            /* s2 should now be 1 char past end of name */
            if (!xml_isspace(*s2) && *s2) {
                /* stopped for invalid end of item */
                return SET_ERROR(ERR_NCX_INVALID_NAME);
            }

            /* check for a valid name length */
            len = (uint32)(s2-str);
            if (len > NCX_MAX_NLEN) {
                return SET_ERROR(ERR_NCX_INVALID_NAME);
            }                
            
            /* got a name, so get a new struct to hold it */
            item = ncx_new_import_item();
            if (!item) {
                return SET_ERROR(ERR_INTERNAL_MEM);
            }

            /* save the name string and queue the item,
             * not checking for duplicates since only the
             * first occurrance will get used; 
             * TBD: check anyway just to issue a warning
             */
            xml_strncpy(item->name, str, len);
            dlq_enque(item, &import->itemQ);

            /* setup next name string parse attempt */
            str = s2;
        }
    }
    return NO_ERR;

}  /* consume_import_items */


/********************************************************************
* FUNCTION consume_import
* 
* Parse the next N nodes as an <import> subtree
* Create a ncx_import struct and add it to the specified module
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mod   == module struct that will get the ncx_import_t 
* RETURNS:
*   status of the operation
* SIDE EFFECTS:
*   the xmlreader state is advanced to the next node
*   If NO_ERR is returned, then a ncx_import_t has been
*   created and added to the end of the mod->importQ
*********************************************************************/
static status_t 
consume_import (xmlTextReaderPtr reader,
		ncx_module_t  *mod)
{
    ncx_import_t    *imp;
    uint32           len;
    const xmlChar   *str;
    boolean          adv;
    status_t         res;

    /* the next node should be a 'import' start tag
     * or maybe the 'imports' end tag
     */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
	      BAD_CAST NCX_EL_IMPORT, TRUE);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_WRONGNAME:
    case ERR_XML_READER_NODETYP:
	/* change error code to indicate no partial section found */
	return ERR_PARS_SECDONE;  
    default:
	return SET_ERROR(res);
    }

    /* Get a new ncx_import_t to fill in */
    imp = ncx_new_import();
    if (!imp) {
	return SET_ERROR(ERR_INTERNAL_MEM);	
    }

    /* Get the mandatory 'module' field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			    BAD_CAST NCX_EL_MODULE, 
			    TRUE, imp->module);
    if (res != NO_ERR) {
	ncx_free_import(imp);
	return SET_ERROR(res);
    }

    /* Check if the optional 'items' field is present */
    res = xml_consume_string(reader, xmlns_ncx_id(),
			      BAD_CAST NCX_EL_ITEMS, 
			      TRUE, &str, &len);
    switch (res) {
    case NO_ERR:
	/* items field is present */
        res = consume_import_items(imp, str);
        if (res != NO_ERR) {
            ncx_free_import(imp);
            return res;
        }
	adv = TRUE;
	break;
    case ERR_XML_READER_NODETYP:
    case ERR_XML_READER_WRONGNAME:
	/* not present -- skip ahead because this field is optional */
	adv = FALSE;
	break;
    default:
	ncx_free_import(imp);
	return SET_ERROR(res);
    }

    /* get the 'import' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
          BAD_CAST NCX_EL_IMPORT, adv);
    if (res != NO_ERR) {
	ncx_free_import(imp);
	return SET_ERROR(res);
    }	

    /* success, add the entry at the end of the import queue */
    dlq_enque(imp, &mod->importQ);
    return NO_ERR;

}  /* consume_import */


/********************************************************************
* FUNCTION consume_type
* 
* Parse the NCX module <type> section
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mod  == ncx_module_t in progress
*   adv == advance or not
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_type (xmlTextReaderPtr reader, 
	      ncx_module_t   *mod,
              boolean adv)
{
    typ_template_t  *typ;
    status_t       res;
    const xmlChar *str;
    uint32  len;

    /* Check for an <type> start node */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
	      BAD_CAST NCX_EL_TYPE, adv);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_WRONGNAME:
    case ERR_XML_READER_NODETYP:
	return ERR_PARS_SECDONE;
    default:
	return SET_ERROR(res);
    }

    /* got a <type> start tag, malloc a new type struct */
    typ = typ_new_type();
    if (!typ) {
	return SET_ERROR(ERR_INTERNAL_MEM);
    }

    /* Get the mandatory <name> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			    BAD_CAST NCX_EL_NAME, 
			    TRUE, typ->name);
    if (res != NO_ERR) {
	typ_free_type(typ);
	return SET_ERROR(res);
    }

    /* Check if the optional <description> field is present */
    adv = TRUE;
    res = consume_descr(reader, &typ->descr, &adv);
    if (res != NO_ERR) {
        typ_free_type(typ);
        return SET_ERROR(res);
    }

    /* get the syntax contents as a string for now */
    res = xml_consume_string(reader, xmlns_ncx_id(),
			      BAD_CAST NCX_EL_SYNTAX, 
			      adv, &str, &len);
    if (res != NO_ERR) {
        typ_free_type(typ);
        return SET_ERROR(res);
    }
    str = xml_trim_string(str, &len);
    if (!str) {
        typ_free_type(typ);
        return SET_ERROR(ERR_NCX_EMPTY_VAL);
    }
    typ->syntax = xml_strndup(str, len);
    if (!typ->syntax) {
        typ_free_type(typ);
        return SET_ERROR(ERR_INTERNAL_MEM);
    }

    /* try to parse the syntax clause into tokens */
    typ->tkc = tk_new_chain();
    if (!typ->tkc) {
        typ_free_type(typ);
        return SET_ERROR(ERR_INTERNAL_MEM);
    }
    res = ncc_tokenize_syntax(typ->syntax, FALSE, typ->tkc);
    if (res != NO_ERR) {
        typ_free_type(typ);
        return SET_ERROR(res);
    }
                
    /* convert the syntax contents to internal format */
    res = typ_parse_syntax_contents(mod, typ->tkc, typ);
    if (res != NO_ERR) {
        typ_free_type(typ);
        return SET_ERROR(res);
    }
    
    /* clean up unneeded memory */
    tk_free_chain(typ->tkc);
    typ->tkc = NULL;

    /* get the 'type' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_TYPE, TRUE);
    if (res != NO_ERR) {
	typ_free_type(typ);
	return SET_ERROR(res);
    }

    /* check for a duplicate entry error in this module */
    if (ncx_find_type(mod, typ->name)) {

#ifdef NCX_PARSE_DEBUG
        printf("\nncx_parse: duplicate type error (%s)", typ->name);
#endif

	typ_free_type(typ);
	return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

#ifdef NCX_PARSE_DEBUG
        printf("\nncx_parse: adding type (%s) to mod (%s)", 
               typ->name, mod->hdr.modname);
#endif

    /* add the type to the module type Que */
    dlq_enque(typ, &mod->typeQ);
    return NO_ERR;

}  /* consume_type */


/********************************************************************
* FUNCTION consume_parm_contents
* 
* Parse the next N nodes as a <parm> element
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   parm   == psd_parm_t  in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_parm_contents (xmlTextReaderPtr reader,
		       psd_parm_t      *parm)
{
    xmlChar    *str;
    boolean     adv;
    status_t    res;

    /* Get the mandatory container <name> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
         BAD_CAST NCX_EL_NAME, TRUE, parm->name);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* Get the mandatory <type> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			      BAD_CAST NCX_EL_TYPE, 
			      TRUE, parm->typname);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* Check for the optional <max-access> field
     * If missing, then set access to the default 'all'
     */
    adv = TRUE;
    res = consume_opt_string(reader, NCX_EL_MAX_ACCESS, &str, &adv);
    if (res != NO_ERR) {
        return res;
    }
    if (str) {
        parm->access = ncx_get_access_enum(str);
        m__free(str);
        if (parm->access==NCX_ACCESS_NONE) {
            return SET_ERROR(ERR_NCX_WRONG_VAL);
        }
    }else {
        parm->access = NCX_ACCESS_ALL;
    }

    /* Get the optional <usage> field 
     * If missing, then set usage to the default 'mandatory'
     */
    res = consume_opt_string(reader, NCX_EL_USAGE, &str, &adv);
    if (res != NO_ERR) {
        return res;
    }
    if (str) {
        parm->usage = psd_get_usage_enum(str);
        m__free(str);
        if (parm->usage==PSD_UT_NONE) {
            return SET_ERROR(ERR_NCX_WRONG_VAL);
        }
    }else {
        parm->usage = PSD_UT_MANDATORY;
    }

    /* Check for the optional <default> field */
    res = consume_opt_string(reader, NCX_EL_DEFAULT, &parm->defstr, &adv);
    if (res != NO_ERR) {
        return res;
    }

    /* Check for the optional <description> field */
    res = consume_descr(reader, &parm->descr, &adv);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* get the 'parm' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
         BAD_CAST NCX_EL_PARM, adv);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }	

    return NO_ERR;

}  /* consume_parm_contents */


/********************************************************************
* FUNCTION consume_parm
* 
* Parse the next N nodes as a 'parm' subtree
* Create a psd_parm struct and add it to the specified psd
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   psd    == parmsetdef struct that will get the psd_parm_t 
* RETURNS:
*   status of the operation
* SIDE EFFECTS:
*   the xmlreader state is advanced to the next node
*   If NO_ERR is returned, then a psd_parm_t has been
*   created and added to the end of the psd->parmQ
*********************************************************************/
static status_t 
consume_parm (xmlTextReaderPtr reader,
	      psd_template_t  *psd)
{
    psd_parm_t      *parm;
    const xmlChar  *str;
    boolean         adv;
    status_t        res;
    
    /* the next node should be a <parm> start tag */
    str = NULL;
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
                      BAD_CAST NCX_EL_PARM, TRUE);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_NODETYP:
    case ERR_XML_READER_WRONGNAME:
	/* change error code to indicate no partial section found */
	return ERR_PARS_SECDONE;  
    default:
	return SET_ERROR(res);
    }

    /* Get a new psd_parm_t to fill in */
    parm = psd_new_parm();
    if (!parm) {
	return SET_ERROR(ERR_INTERNAL_MEM);	
    }

    adv = TRUE;

    /* get the parm contents and end tag */
    res = consume_parm_contents(reader, parm);
    if (res != NO_ERR) {
        psd_free_parm(parm);
        return res;
    }

    /* check if this is a duplicate */
    if (psd_find_parm(psd, parm->name)) {
	psd_free_parm(parm);
	return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }
        
    /* 1st pass parsing success, add the entry at the end of the queue */
    dlq_enque(parm, &psd->parmQ);
    return NO_ERR;

}  /* consume_parm */


/********************************************************************
* FUNCTION consume_psd
* 
* Parse the next N nodes as a <psd> subtree
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mod == module to contain the PSD
*   adv == advance or not
* RETURNS:
*   status of the operation
* SIDE EFFECTS:
*   the xmlreader state is advanced to the next node
*   If NO_ERR is returned, then a psd_template_t has been
*   created and added to the definition registry.
*********************************************************************/
static status_t 
consume_psd (xmlTextReaderPtr reader, 
             ncx_module_t  *mod,
             boolean  adv)
{
    psd_template_t *psd;
    boolean         done;
    status_t        res;
    xmlChar        *str;
    
    /* the next node might be a <parmset> start tag */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
	      BAD_CAST NCX_EL_PARMSET, adv);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_NODETYP:
    case ERR_XML_READER_WRONGNAME:
	/* change error code to indicate no partial section found */
	return ERR_PARS_SECDONE;  
    default:
	return SET_ERROR(res);
    }

    /* Get a new psd_template_t to fill in */
    psd = psd_new_template();
    if (!psd) {
	return SET_ERROR(ERR_INTERNAL_MEM);	
    }

    /* a <psd> container element was found 
     * first get the { app, name, version } tuple
     * that is used as the identifier and hash key
     *
     * The app field is already stored in the module struct
     *
     *  Get the <name> field 
     */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			    BAD_CAST NCX_EL_NAME, 
			    TRUE, psd->name);
    if (res != NO_ERR) {
	psd_free_template(psd);
	return SET_ERROR(res);
    }

    /* Get the optional <order> field 
     * If missing, then set to the default 'loose'
     */
    res = consume_opt_string(reader, NCX_EL_ORDER, &str, &adv);
    if (res != NO_ERR) {
        return res;
    }
    if (str) {
        psd->order = psd_get_order_enum(str);
        m__free(str);
        if (psd->order==PSD_OT_NONE) {
            return SET_ERROR(ERR_NCX_WRONG_VAL);
        }
    }else {
        psd->order = PSD_OT_LOOSE;
    }

    /* Check to see if an optional <description> field is present */
    adv = TRUE;
    res = consume_descr(reader, &psd->descr, &adv);
    if (res != NO_ERR) {
	psd_free_template(psd);
	return SET_ERROR(res);
    }
        
    /* get the mandatory 'parms' start tag */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
          BAD_CAST NCX_EL_PARMS, adv);
    if (res != NO_ERR) {
	psd_free_template(psd);
	return SET_ERROR(res);
    }

    /* get each 'parm' subsection */
    done = FALSE;
    while (!done) {
	res = consume_parm(reader, psd);
	switch (res) {
	case NO_ERR:
	    break;
	case ERR_PARS_SECDONE:
	    done = TRUE;
	    break;
	default:
	    psd_free_template(psd);
	    return res;
	}
    }

    /* get the mandatory 'parms' end tag -- should be current node */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_PARMS, FALSE);
    if (res != NO_ERR) {
	psd_free_template(psd);
	return res;
    }

    /* get the mandatory 'parmset' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_PARMSET, TRUE);
    if (res != NO_ERR) {
	psd_free_template(psd);
	return SET_ERROR(res);
    }

    /* make sure this is not a duplicate */
    if (ncx_find_psd(mod, psd->name)) {
#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: dropping duplicate PSD (%s) for mod.app (%s.%s)", 
           psd->name, mod->hdr.modname, mod->hdr.appname);
#endif
            psd_free_template(psd);
            return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: adding PSD (%s) to mod.app (%s.%s)", 
           psd->name, mod->hdr.modname, mod->hdr.appname);
#endif

    /* Got a valid entry, add it to the mod->psdQ */
    dlq_enque(psd, &mod->psdQ);
    return NO_ERR;

}  /* consume_psd */








/********************************************************************
* FUNCTION consume_obj_contents
* 
* Parse the next N nodes as <object> contents
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   obj   == mon_obj_t  in progress
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_obj_contents (xmlTextReaderPtr reader,
		       mon_obj_t      *obj)
{
    boolean         adv;
    status_t        res;

    /* Get the mandatory container <name> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
         BAD_CAST NCX_EL_NAME, TRUE, obj->name);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* Get the mandatory <type> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			      BAD_CAST NCX_EL_TYPE, 
			      TRUE, obj->typname);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* Check for the optional <description> field */
    adv = TRUE;
    res = consume_descr(reader, &obj->descr, &adv);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }	

    /* get the 'obj' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
         BAD_CAST NCX_EL_OBJECT, adv);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }	

    return NO_ERR;

}  /* consume_obj_contents */


/********************************************************************
* FUNCTION consume_obj
* 
* Parse the next N nodes as a <object> subtree
* Create a mon_obj_t struct and add it to the specified MON
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mon    == mon_template struct that will get the mon_obj_t 
* RETURNS:
*   status of the operation
* SIDE EFFECTS:
*   the xmlreader state is advanced to the next node
*   If NO_ERR is returned, then a mon_obj_t has been
*   created and added to the end of the mon->objQ
*********************************************************************/
static status_t 
consume_obj (xmlTextReaderPtr reader,
	     mon_template_t  *mon)
{
    mon_obj_t      *obj;
    const xmlChar  *str;
    boolean         adv;
    status_t        res;
    
    /* the next node should be a <object> start tag */
    str = NULL;
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
                      BAD_CAST NCX_EL_OBJECT, TRUE);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_NODETYP:
    case ERR_XML_READER_WRONGNAME:
	/* change error code to indicate no partial section found */
	return ERR_PARS_SECDONE;  
    default:
	return SET_ERROR(res);
    }

    /* Get a new mon_obj_t to fill in */
    obj = mon_new_obj();
    if (!obj) {
	return SET_ERROR(ERR_INTERNAL_MEM);	
    }

    adv = TRUE;

    /* get the object contents and end tag */
    res = consume_obj_contents(reader, obj);
    if (res != NO_ERR) {
        mon_free_obj(obj);
        return res;
    }

    /* check if this is a duplicate */
    if (mon_find_obj(mon, obj->name)) {
	mon_free_obj(obj);
	return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }
        
    /* 1st pass parsing success, add the entry at the end of the queue */
    dlq_enque(obj, &mon->objQ);
    return NO_ERR;

}  /* consume_obj */


/********************************************************************
* FUNCTION consume_mon
* 
* Parse an NCX module <monitor> section
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mod  == ncx_module_t in progress
*   adv == advance or not
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_mon (xmlTextReaderPtr reader, 
             ncx_module_t   *mod,
             boolean adv)
{
    status_t        res;
    mon_template_t *mon;
    boolean         done;

    /* Check for a <monitor> start node */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
	      BAD_CAST NCX_EL_MONITOR, adv);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_WRONGNAME:
    case ERR_XML_READER_NODETYP:
	return ERR_PARS_SECDONE;
    default:
	return SET_ERROR(res);
    }

    /* got a start tag, malloc a new monitor struct */
    mon = mon_new_template();
    if (!mon) {
	return SET_ERROR(ERR_INTERNAL_MEM);
    }

    /* Get the mandatory <name> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			    BAD_CAST NCX_EL_NAME, 
			    TRUE, mon->name);
    if (res != NO_ERR) {
	mon_free_template(mon);
	return SET_ERROR(res);
    }

    /* Check if the optional <description> field is present */
    adv = TRUE;
    res = consume_descr(reader, &mon->descr, &adv);
    if (res != NO_ERR) {
	mon_free_template(mon);
	return SET_ERROR(res);
    }

    /* get the mandatory 'objects' start tag */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
          BAD_CAST NCX_EL_OBJECTS, adv);
    if (res != NO_ERR) {
	mon_free_template(mon);
	return SET_ERROR(res);
    }

    /* get each 'object' subsection */
    done = FALSE;
    while (!done) {
	res = consume_obj(reader, mon);
	switch (res) {
	case NO_ERR:
	    break;
	case ERR_PARS_SECDONE:
	    done = TRUE;
	    break;
	default:
	    mon_free_template(mon);
	    return res;
	}
    }

    /* get the mandatory 'objects' end tag -- should be current node */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_OBJECTS, FALSE);
    if (res != NO_ERR) {
	mon_free_template(mon);
	return res;
    }

    /* get the mandatory 'monitor' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_MONITOR, TRUE);
    if (res != NO_ERR) {
	mon_free_template(mon);
	return SET_ERROR(res);
    }

    /* make sure this is not a duplicate */
    if (ncx_find_mon(mod, mon->name)) {
#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: dropping duplicate MON (%s) for mod.app (%s.%s)", 
           mon->name, mod->hdr.modname, mod->hdr.appname);
#endif
            mon_free_template(mon);
            return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: adding MON (%s) to mod.app (%s.%s)", 
           mon->name, mod->hdr.modname, mod->hdr.appname);
#endif
                              
    /* get the 'monitor' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_MONITOR, TRUE);
    if (res != NO_ERR) {
	mon_free_template(mon);
	return SET_ERROR(res);
    }

    /* add the mon_template_t to the module MON Que */
    dlq_enque(mon, &mod->monQ);
    return NO_ERR;

}  /* consume_monitor */


/********************************************************************
* FUNCTION consume_rpc_contents
* 
* Parse contents of one NCX module <rpc> section
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   rpc == rpc_template_t to fill in
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_rpc_contents (xmlTextReaderPtr reader, 
                      rpc_template_t   *rpc)
{
    status_t       res;
    const xmlChar *str;
    xmlChar       *dup;
    boolean        adv;
    uint32         len;

    /* Get the mandatory <name> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			    BAD_CAST NCX_EL_NAME, 
			    TRUE, rpc->name);
    if (res != NO_ERR) {
	return res;
    }

    /* Get the mandatory <rpc-type> field */
    res = xml_consume_string(reader, xmlns_ncx_id(),
			      BAD_CAST NCX_EL_RPC_TYPE, 
			      TRUE, &str, &len);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }
    str = xml_trim_string(str, &len);
    if (!str) {
        return SET_ERROR(ERR_NCX_EMPTY_VAL);
    }
    dup = xml_strndup(str, len);
    if (!dup) {
        return SET_ERROR(ERR_INTERNAL_MEM);
    }
    if (!xml_strcmp(BAD_CAST NCX_EL_RT_OTHER, dup)) {
        rpc->rpc_typ = RPC_TYP_OTHER;
    } else if (!xml_strcmp(BAD_CAST NCX_EL_RT_CONFIG, dup)) {
        rpc->rpc_typ = RPC_TYP_CONFIG;
    } else if (!xml_strcmp(BAD_CAST NCX_EL_RT_EXEC, dup)) {
        rpc->rpc_typ = RPC_TYP_EXEC;
    } else if (!xml_strcmp(BAD_CAST NCX_EL_RT_MONITOR, dup)) {
        rpc->rpc_typ = RPC_TYP_MONITOR;
    } else if (!xml_strcmp(BAD_CAST NCX_EL_RT_DEBUG, dup)) {
        rpc->rpc_typ = RPC_TYP_DEBUG;
    } else {
        m__free(dup);
        return SET_ERROR(ERR_NCX_WRONG_VAL);
    }
    m__free(dup);

    /* Get the mandatory <in-psd> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			    BAD_CAST NCX_EL_INPSD, 
                           TRUE, rpc->in_psd);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* Get the mandatory <out-data> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
			    BAD_CAST NCX_EL_OUTDATA, 
                           TRUE, rpc->out_data);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* Check if the optional <description> field is present */
    adv = TRUE;
    res = consume_descr(reader, &rpc->descr, &adv);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* get the 'rpc' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_RPC, adv);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    return NO_ERR;

} /* consume_rpc_contents */


/********************************************************************
* FUNCTION consume_rpc
* 
* Parse one NCX module <rpc> section
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mod == ncx_module_t in progress
*   adv == advance or not
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_rpc (xmlTextReaderPtr reader, 
	     ncx_module_t   *mod,
             boolean  adv)
{
    rpc_template_t  *rpc;
    status_t       res;

    /* Check for an <rpc> start node */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
	      BAD_CAST NCX_EL_RPC, adv);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_WRONGNAME:
    case ERR_XML_READER_NODETYP:
	return ERR_PARS_SECDONE;
    default:
	return SET_ERROR(res);
    }

    /* got an <rpc> start tag, malloc a new rpc struct */
    rpc = rpc_new_template();
    if (!rpc) {
	return SET_ERROR(ERR_INTERNAL_MEM);
    }

    /* get the rest of the rpc, including the end tag */
    res = consume_rpc_contents(reader, rpc);
    if (res != NO_ERR) {
	rpc_free_template(rpc);
        return res;
    }

    /* make sure this is not a duplicate */
    if (ncx_find_rpc(mod, rpc->name)) {
#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: tossing duplicate RPC (%s) from mod.app (%s.%s)", 
           rpc->name, mod->hdr.modname, mod->hdr.appname);
#endif
            rpc_free_template(rpc);
            return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: adding RPC (%s) to mod.app (%s.%s)", 
           rpc->name, mod->hdr.modname, mod->hdr.appname);
#endif

    /* add the rpc to the module RPC Que */
    dlq_enque(rpc, &mod->rpcQ);
    return NO_ERR;

}  /* consume_rpc */


/********************************************************************
* FUNCTION consume_event_contents
* 
* Parse contents of one NCX module <event> section
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   evt == evt_template_t to fill in
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_event_contents (xmlTextReaderPtr reader, 
                        evt_template_t   *evt)
{
    status_t       res;
    boolean        adv;

    /* Get the mandatory <name> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
         BAD_CAST NCX_EL_NAME, TRUE, evt->name);
    if (res != NO_ERR) {
	return res;
    }

    /* Get the mandatory <event-class> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_EVENT_CLASS, TRUE, evt->evt_class);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* Get the mandatory <event-type> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_EVENT_TYPE, TRUE, evt->evt_type);
    if (res != NO_ERR) {
        return SET_ERROR(res);
    }

    /* Get the optional <event-data> field */
    res = xml_consume_name(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_EVENT_DATA, TRUE, evt->evt_data);
    switch (res) {
    case NO_ERR:
        evt->use_evt_data = TRUE;
        adv = TRUE;
        break;
    case ERR_XML_READER_WRONGNAME:
        evt->use_evt_data = FALSE;
        adv = FALSE;
        break;
    default:
        return SET_ERROR(res);
    }

    /* Check if the optional <description> field is present */
    res = consume_descr(reader, &evt->descr, &adv);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    /* get the 'event' end tag */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
        BAD_CAST NCX_EL_EVENT, adv);
    if (res != NO_ERR) {
	return SET_ERROR(res);
    }

    return NO_ERR;

} /* consume_event_contents */


/********************************************************************
* FUNCTION consume_event
* 
* Parse one NCX module <event> section
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mod == ncx_module_t in progress
*   adv == advance or not
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_event (xmlTextReaderPtr reader, 
               ncx_module_t   *mod,
               boolean  adv)
{
    evt_template_t  *evt;
    status_t       res;

    /* Check for an <event> start node */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
	      BAD_CAST NCX_EL_EVENT, adv);
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_WRONGNAME:
    case ERR_XML_READER_NODETYP:
	return ERR_PARS_SECDONE;
    default:
	return SET_ERROR(res);
    }

    /* got a start tag, malloc a new event struct */
    evt = evt_new_template();
    if (!evt) {
	return SET_ERROR(ERR_INTERNAL_MEM);
    }

    res = consume_event_contents(reader, evt);
    if (res != NO_ERR) {
	evt_free_template(evt);
        return res;
    }

    /* make sure this is not a duplicate */
    if (ncx_find_event(mod, evt->name)) {
#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: tossing duplicate EVT (%s) from mod.app (%s.%s)", 
           evt->name, mod->hdr.modname, mod->hdr.appname);
#endif
            evt_free_template(evt);
            return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

#ifdef NCX_PARSE_DEBUG
    printf("\nncx_parse: adding EVT (%s) to mod.app (%s.%s)", 
           evt->name, mod->hdr.modname, mod->hdr.appname);
#endif

    /* add the rpc to the module RPC Que */
    dlq_enque(evt, &mod->eventQ);
    return NO_ERR;

}  /* consume_event */


/********************************************************************
* FUNCTION consume_definition
* 
* Parse an NCX module definition section construct, such as <type>
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   mod == ncx_module_t in progress
*
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t
consume_definition (xmlTextReaderPtr reader, 
                    ncx_module_t *mod)
{
    status_t res;

    /* brute force through all the options */
    res = consume_type(reader, mod, TRUE);
    if (res == NO_ERR || res != ERR_PARS_SECDONE) {
        return res;
    }

    res = consume_psd(reader, mod, FALSE);
    if (res == NO_ERR || res != ERR_PARS_SECDONE) {
        return res;
    }

    res = consume_mon(reader, mod, FALSE);
    if (res == NO_ERR || res != ERR_PARS_SECDONE) {
        return res;
    }

    res = consume_rpc(reader, mod, FALSE);
    if (res == NO_ERR || res != ERR_PARS_SECDONE) {
        return res;
    }

    res = consume_event(reader, mod, FALSE);
    if (res == NO_ERR || res != ERR_PARS_SECDONE) {
        return res;
    }

    return ERR_PARS_SECDONE;

} /* consume_definition */


/********************************************************************
* FUNCTION consume_container
* 
* Parse the NCX module contain section, such as <imports> section
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   advance == TRUE if the parser should advance to the next node
*           == FALSE if the parser is already on the right start node
* RETURNS:
*   status of the operation
*********************************************************************/
static status_t 
consume_container (xmlTextReaderPtr reader, 
		   ncx_module_t   *mod,
		   boolean *advance,
		   const xmlChar *elname,
		   ncx_containedFn_t  cbfn)
{
    status_t  res;
    boolean   done;

    /* Check for a start node */
    res = xml_consume_start_node(reader, xmlns_ncx_id(), elname, *advance);

    *advance = FALSE;   /* setup error return */
    switch (res) {
    case NO_ERR:
	break;
    case ERR_XML_READER_WRONGNAME:
    case ERR_XML_READER_NODETYP:
	return NO_ERR;
    default:
	return SET_ERROR(res);
    }

    /* process each child subtree */
    done = FALSE;
    while (!done) {
	res = (*cbfn)(reader, mod);
	switch (res) {
	case NO_ERR:
	    break;
	case ERR_PARS_SECDONE:
	    done = TRUE;
	    break;
	default:
	    return res;
	}
    }

    /* get the end tag, should be the current node */
    res = xml_consume_end_node(reader, xmlns_ncx_id(), elname, FALSE);
    if (res != NO_ERR) {
        *advance = FALSE;
        return SET_ERROR(res);
    } else {
        *advance = TRUE;
        return NO_ERR;
    }
    /*NOTREACHED*/
}  /* consume_container */



/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION ncx_parse_from_reader
* 
* Parse, generate and register one ncx_module_t struct
* from an XML instance document stream containing one NCX module,
* which conforms to the NcxModule XSD.
*
* Although the entire ncx-module could be considered a parmset
* that parsing path is not used because there are some
* chicken-and-egg corner-cases that can occur.  Instead,
* this simple hard-wired parse code path is used.
*
* INPUTS:
*   reader == XmlReader already initialized from File, Memory,
*             or whatever
*   advance == TRUE if the parser should advance to the next node
*           == FALSE if the parser is already on the right start node
*   source  == source of this input (could be filespec)
* RETURNS:
*   status of the operation
* SIDE EFFECTS:
*   if parse success, then the module contents are registered
*   in the def_reg module.  The module itself is also registered
*********************************************************************/
status_t 
ncx_parse_from_reader (xmlTextReaderPtr reader, 
                       boolean *advance,
                       const char *source)
{
    ncx_module_t  *mod;
    status_t       res;

    if (!reader || !advance) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }

    /* the top-level node should be 'ncx-module' */
    res = xml_consume_start_node(reader, xmlns_ncx_id(),
	  BAD_CAST NCX_EL_NCXMODULE, *advance);
    if (res != NO_ERR) {
	*advance = FALSE;
	return res;
    }

    /* start a new ncx_module_t struct */
    mod = ncx_new_module();
    if (!mod) {
	*advance = TRUE;
	return SET_ERROR(ERR_INTERNAL_MEM);
    }

    /* save the source of this ncx-module for monitor / debug */
    mod->hdr.source = strdup(source);
    if (!mod->hdr.source) {
        ncx_free_module(mod);
        return SET_ERROR(ERR_INTERNAL_MEM);
    }
    mod->syntax = NCX_SYNTAX_NCX;

    /* the first section is the mandatory module <header> */
    res = consume_header(reader, mod);
    if (res != NO_ERR) {
	ncx_free_module(mod);
	return res;
    }

    /* check if this module is already loaded 
     * Only allow 1 version of the module to be present
     * This could change later, but requires too many
     * complexities managing and validating duplicate definitions
     */
    if (def_reg_find_module(mod->hdr.modname)) {
        ncx_free_module(mod);
        return SET_ERROR(ERR_NCX_DUP_ENTRY);
    }

    /* Check for an optional <imports> section */
    *advance = TRUE;
    res = consume_container(reader, mod, advance, 
			    BAD_CAST NCX_EL_IMPORTS,
			    consume_import);
    if (res != NO_ERR) {
	ncx_free_module(mod);
	return res;
    }

    /* Check for an optional <definitions> section */
    res = consume_container(reader, mod, advance, 
			    BAD_CAST NCX_EL_DEFINITIONS,
			    consume_definition);
    if (res != NO_ERR) {
	ncx_free_module(mod);
	return res;
    }

    /* the current node should be the 'ncx-module' end node
     * check that this is an element node 
     */
    res = xml_consume_end_node(reader, xmlns_ncx_id(),
	BAD_CAST NCX_EL_NCXMODULE, *advance);
    *advance = (res==NO_ERR) ? TRUE : FALSE;
    if (res != NO_ERR) {
        ncx_free_module(mod);
        return res;
    }

    /* add the definitions to the def_reg hash table */
    res = ncx_add_to_registry(mod);
    if (res != NO_ERR) {
        ncx_free_module(mod);
    }
    return res;

}  /* ncx_parse_from_reader */


/********************************************************************
* FUNCTION ncx_parse_from_filespec
* 
*
* INPUTS:
*   filespec == absolute path or relative path
*               This string is used as-is without adjustment.
* RETURNS:
*   status of the operation
*********************************************************************/
status_t 
ncx_parse_from_filespec (const char *filespec)
{
    status_t   res, res2;
    xmlTextReaderPtr  xmlreader;
    boolean    adv;

    if (!filespec) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    } 

    res = xml_get_reader_from_filespec(filespec, &xmlreader);
    if (res != NO_ERR) {
        return res;
    }

    adv = TRUE;
    res = ncx_parse_from_reader(xmlreader, &adv, filespec);
    if (res == NO_ERR) {
	/* check that we reached the end of the document */
	res2 = xml_check_docdone(xmlreader, adv);
    }

    /* for now get a new reader every time to simplify cleanup */
    xml_free_reader(xmlreader);

    /* ignoring 'res2' extra nodes error for now */
    return res;

}  /* ncx_parse_from_filespec */

/* END file ncx_parse.c */
