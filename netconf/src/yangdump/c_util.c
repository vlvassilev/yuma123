/*  FILE: c_util.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
24-oct-09    abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_c_util
#include  "c_util.h"
#endif

#ifndef _H_log
#include  "log.h"
#endif

#ifndef _H_ncx
#include  "ncx.h"
#endif

#ifndef _H_ncxmod
#include  "ncxmod.h"
#endif

#ifndef _H_ncxtypes
#include "ncxtypes.h"
#endif

#ifndef _H_ses
#include "ses.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_yangdump
#include "yangdump.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/

/********************************************************************
*                                                                   *
*                       V A R I A B L E S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION new_c_define
* 
* Allocate and fill in a constructed identifier to string binding
* for an object struct of some kind
*
* INPUTS:
*   modname == module name to use
*   defname == definition name to use
*   cdefmode == c definition mode
*********************************************************************/
static c_define_t *
    new_c_define (const xmlChar *modname,
                  const xmlChar *defname,
                  c_mode_t cmode)
{
    c_define_t  *cdef;
    xmlChar     *buffer, *value, *p;
    uint32       len;

    /* get the idstr length */
    len = 0;
    len += xml_strlen(Y_PREFIX);
    len += xml_strlen(modname);

    switch (cmode) {
    case C_MODE_OID:
        len += 3;  /* _N_ */
        break;
    case C_MODE_TYPEDEF:
        len += 2; /* _T */
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
        return NULL;
    }

    len += xml_strlen(defname);

    buffer = m__getMem(len+1);
    if (buffer == NULL) {
        return NULL;
    }

    value = xml_strdup(defname);
    if (value == NULL) {
        m__free(buffer);
        return NULL;
    }

    cdef = m__getObj(c_define_t);
    if (cdef == NULL) {
        m__free(buffer);
        m__free(value);
        return NULL;
    }
    memset(cdef, 0x0, sizeof(c_define_t));

    /* fill in the idstr buffer */
    p = buffer;
    p += xml_strcpy(p, Y_PREFIX);
    p += copy_c_safe_str(p, modname);

    switch (cmode) {
    case C_MODE_OID:
        p += xml_strcpy(p, (const xmlChar *)"_N_");
        break;
    case C_MODE_TYPEDEF:
        p += xml_strcpy(p, (const xmlChar *)"_T");
        break;
    default:
        SET_ERROR(ERR_INTERNAL_VAL);
    }
    p += copy_c_safe_str(p, defname);    

    cdef->idstr = buffer;     /* transfer buffer memory here */
    cdef->valstr = value;     /* transfer value memory here */    
    return cdef;

}  /* new_c_define */


/********************************************************************
* FUNCTION free_c_define
* 
* Free an identifier to string binding
*
* INPUTS:
*   cdef == struct to free
*********************************************************************/
static void
    free_c_define (c_define_t *cdef)
{
    if (cdef->idstr != NULL) {
        m__free(cdef->idstr);
    }
    if (cdef->valstr != NULL) {
        m__free(cdef->valstr);
    }
    m__free(cdef);

}  /* free_c_define */


/********************************************************************
* FUNCTION need_rpc_includes
* 
* Check if the include-stmts for RPC methods are needed
*
* INPUTS:
*   mod == module in progress
*   cp == conversion parameters to use
*
* RETURNS:
*  TRUE if RPCs found
*  FALSE if no RPCs found
*********************************************************************/
boolean
    need_rpc_includes (const ncx_module_t *mod,
                       const yangdump_cvtparms_t *cp)
{
    const ncx_include_t *inc;

    if (obj_any_rpcs(&mod->datadefQ)) {
        return TRUE;
    }

    if (cp->unified) {
        for (inc = (const ncx_include_t *)
                 dlq_firstEntry(&mod->includeQ);
             inc != NULL;
             inc = (const ncx_include_t *)dlq_nextEntry(inc)) {

            if (inc->submod && obj_any_rpcs(&inc->submod->datadefQ)) {
                return TRUE;
            }
        }
    }

    return FALSE;

} /* need_rpc_includes */


/********************************************************************
* FUNCTION need_notif_includes
* 
* Check if the include-stmts for notifications are needed
*
* INPUTS:
*   mod == module in progress
*   cp == conversion parameters to use
*
* RETURNS:
*   TRUE if notifcations found
*   FALSE if no notifications found
*********************************************************************/
boolean
    need_notif_includes (const ncx_module_t *mod,
                         const yangdump_cvtparms_t *cp)
{
    const ncx_include_t *inc;

    if (obj_any_notifs(&mod->datadefQ)) {
        return TRUE;
    }

    if (cp->unified) {
        for (inc = (const ncx_include_t *)
                 dlq_firstEntry(&mod->includeQ);
             inc != NULL;
             inc = (const ncx_include_t *)dlq_nextEntry(inc)) {

            if (inc->submod && obj_any_notifs(&inc->submod->datadefQ)) {
                return TRUE;
            }
        }
    }

    return FALSE;

} /* need_notif_includes */


/********************************************************************
* FUNCTION write_c_safe_str
* 
* Generate a string token at the current line location
*
* INPUTS:
*   scb == session control block to use for writing
*   strval == string value
*********************************************************************/
void
    write_c_safe_str (ses_cb_t *scb,
                      const xmlChar *strval)
{
    const xmlChar *s;

    s = strval;
    while (*s) {
        if (*s == '.' || *s == '-' || *s == NCXMOD_PSCHAR) {
            ses_putchar(scb, '_');
        } else {
            ses_putchar(scb, *s);
        }
        s++;
    }

}  /* write_c_safe_str */


/********************************************************************
* FUNCTION copy_c_safe_str
* 
* Generate a string token and copy to buffer
*
* INPUTS:
*   buffer == buffer to write into
*   strval == string value to copy
*
* RETURNS
*   number of chars copied
*********************************************************************/
uint32
    copy_c_safe_str (xmlChar *buffer,
                     const xmlChar *strval)
{
    const xmlChar *s;
    uint32         count;

    count = 0;
    s = strval;

    while (*s) {
        if (*s == '.' || *s == '-' || *s == NCXMOD_PSCHAR) {
            *buffer++ = '_';
        } else {
            *buffer++ = *s;
        }
        s++;
        count++;
    }
    *buffer = 0;

    return count;

}  /* copy_c_safe_str */


/********************************************************************
* FUNCTION write_c_str
* 
* Generate a string token at the current line location
*
* INPUTS:
*   scb == session control block to use for writing
*   strval == string value
*   quotes == quotes style (0, 1, 2)
*********************************************************************/
void
    write_c_str (ses_cb_t *scb,
                 const xmlChar *strval,
                 uint32 quotes)
{
    switch (quotes) {
    case 1:
        ses_putchar(scb, '\'');
        break;
    case 2:
        ses_putchar(scb, '"');
        break;
    default:
        ;
    }

    ses_putstr(scb, strval);

    switch (quotes) {
    case 1:
        ses_putchar(scb, '\'');
        break;
    case 2:
        ses_putchar(scb, '"');
        break;
    default:
        ;
    }

}  /* write_c_str */


/********************************************************************
* FUNCTION write_c_simple_str
* 
* Generate a simple clause on 1 line
*
* INPUTS:
*   scb == session control block to use for writing
*   kwname == keyword name
*   strval == string value
*   indent == indent count to use
*   quotes == quotes style (0, 1, 2)
*********************************************************************/
void
    write_c_simple_str (ses_cb_t *scb,
                        const xmlChar *kwname,
                        const xmlChar *strval,
                        int32 indent,
                        uint32 quotes)
{
    ses_putstr_indent(scb, kwname, indent);
    if (strval) {
        ses_putchar(scb, ' ');
        write_c_str(scb, strval, quotes);
    }

}  /* write_c_simple_str */


/********************************************************************
*
* FUNCTION write_identifier
* 
* Generate an identifier
*
*  #module_DEFTYPE_idname
*
* INPUTS:
*   scb == session control block to use for writing
*   modname == module name start-string to use
*   defpart == internal string for deftype part
*   idname == identifier name
*
*********************************************************************/
void
    write_identifier (ses_cb_t *scb,
                      const xmlChar *modname,
                      const xmlChar *defpart,
                      const xmlChar *idname)
{
    ses_putstr(scb, Y_PREFIX);
    write_c_safe_str(scb, modname);
    ses_putchar(scb, '_');
    if (defpart != NULL) {
        ses_putstr(scb, defpart);
        ses_putchar(scb, '_');
    }
    write_c_safe_str(scb, idname);

}  /* write_identifier */


/********************************************************************
* FUNCTION write_ext_include
* 
* Generate an include statement for an external file
*
*  #include <foo,h>
*
* INPUTS:
*   scb == session control block to use for writing
*   hfile == H file name == file name to include (foo.h)
*
*********************************************************************/
void
    write_ext_include (ses_cb_t *scb,
                       const xmlChar *hfile)
{
    ses_putstr(scb, POUND_INCLUDE);
    ses_putstr(scb, (const xmlChar *)"<");
    ses_putstr(scb, hfile);
    ses_putstr(scb, (const xmlChar *)">\n");

}  /* write_ext_include */


/********************************************************************
* FUNCTION write_ncx_include
* 
* Generate an include statement for an NCX file
*
*  #ifndef _H_foo
*  #include "foo,h"
*
* INPUTS:
*   scb == session control block to use for writing
*   modname == module name to include (foo)
*
*********************************************************************/
void
    write_ncx_include (ses_cb_t *scb,
                       const xmlChar *modname)
{
    ses_putstr(scb, POUND_IFNDEF);
    ses_putstr(scb, BAR_H);
    ses_putstr(scb, modname);
    ses_putstr(scb, POUND_INCLUDE);
    ses_putchar(scb, '"');
    ses_putstr(scb, modname);
    ses_putstr(scb, (const xmlChar *)".h\"");
    ses_putstr(scb, POUND_ENDIF);
    ses_putchar(scb, '\n');

}  /* write_ncx_include */


/********************************************************************
* FUNCTION save_oid_cdefine
* 
* Generate a #define binding for a definition and save it in the
* specified Q of c_define_t structs
*
* INPUTS:
*   cdefineQ == Q of c_define_t structs to use
*   modname == module name to use
*   defname == object definition name to use
*
* OUTPUTS:
*   a new c_define_t is allocated and added to the cdefineQ
*   if returning NO_ERR;
*
* RETURNS:
*   status; duplicate C identifiers not supported yet
*      foo-1 -->  foo_1
*      foo.1 -->  foo_1
*   An error message will be generated if this type of error occurs
*********************************************************************/
status_t
    save_oid_cdefine (dlq_hdr_t *cdefineQ,
                      const xmlChar *modname,
                      const xmlChar *defname)
{
    c_define_t    *newcdef, *testcdef;
    status_t       res;
    int32          retval;

#ifdef DEBUG
    if (cdefineQ == NULL || modname == NULL || defname == NULL) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    newcdef = new_c_define(modname, defname, C_MODE_OID);
    if (newcdef == NULL) {
        return ERR_INTERNAL_MEM;
    }

    /* keep the cdefineQ sorted by the idstr value */
    res = NO_ERR;
    for (testcdef = (c_define_t *)dlq_firstEntry(cdefineQ);
         testcdef != NULL;
         testcdef = (c_define_t *)dlq_nextEntry(testcdef)) {

        retval = xml_strcmp(newcdef->idstr, testcdef->idstr);
        if (retval == 0) {
            if (xml_strcmp(newcdef->valstr, testcdef->valstr)) {
                /* error - duplicate ID with different original value */
                res = ERR_NCX_INVALID_VALUE;
                log_error("\nError: C idenitifer conflict between "
                          "'%s' and '%s'",
                          newcdef->valstr,
                          testcdef->valstr);
            } /* else duplicate is not a problem */
            free_c_define(newcdef);
            return res;
        } else if (retval < 0) {
            dlq_insertAhead(newcdef, testcdef);
            return NO_ERR;
        } /* else try next entry */
    }

    /* new last entry */
    dlq_enque(newcdef, cdefineQ);
    return NO_ERR;

}  /* save_oid_cdefine */


/********************************************************************
* FUNCTION save_path_cdefine
* 
* Generate a #define binding for a definition and save it in the
* specified Q of c_define_t structs
*
* INPUTS:
*   cdefineQ == Q of c_define_t structs to use
*   modname == base module name to use
*   obj == object struct to use to generate path
*
* OUTPUTS:
*   a new c_define_t is allocated and added to the cdefineQ
*   if returning NO_ERR;
*
* RETURNS:
*   status; duplicate C identifiers not supported yet
*      foo-1/a/b -->  foo_1_a_b
*      foo.1/a.2 -->  foo_1_a_b
*   An error message will be generated if this type of error occurs
*********************************************************************/
status_t
    save_path_cdefine (dlq_hdr_t *cdefineQ,
                       const xmlChar *modname,
                       obj_template_t *obj)
{
    c_define_t    *newcdef, *testcdef;
    xmlChar       *buffer;
    status_t       res;
    int32          retval;

#ifdef DEBUG
    if (cdefineQ == NULL || modname == NULL || obj == NULL) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    buffer = NULL;
    res = obj_gen_object_id(obj, &buffer);
    if (res != NO_ERR) {
        return res;
    }

    newcdef = new_c_define(modname, buffer, C_MODE_TYPEDEF);
    m__free(buffer);
    buffer = NULL;
    if (newcdef == NULL) {
        return ERR_INTERNAL_MEM;
    }
    newcdef->obj = obj;

    /* keep the cdefineQ sorted by the idstr value */
    res = NO_ERR;
    for (testcdef = (c_define_t *)dlq_firstEntry(cdefineQ);
         testcdef != NULL;
         testcdef = (c_define_t *)dlq_nextEntry(testcdef)) {

        retval = xml_strcmp(newcdef->idstr, testcdef->idstr);
        if (retval == 0) {
            if (xml_strcmp(newcdef->valstr, testcdef->valstr)) {
                /* error - duplicate ID with different original value */
                res = ERR_NCX_INVALID_VALUE;
                log_error("\nError: C idenitifer conflict between "
                          "'%s' and '%s'",
                          newcdef->valstr,
                          testcdef->valstr);
            } /* else duplicate is not a problem */
            free_c_define(newcdef);
            return res;
        } else if (retval < 0) {
            dlq_insertAhead(newcdef, testcdef);
            return NO_ERR;
        } /* else try next entry */
    }

    /* new last entry */
    dlq_enque(newcdef, cdefineQ);
    return NO_ERR;

}  /* save_path_cdefine */


/********************************************************************
* FUNCTION find_path_cdefine
* 
* Find a #define binding for a definition in the
* specified Q of c_define_t structs
*
* INPUTS:
*   cdefineQ == Q of c_define_t structs to use
*   obj == object struct to find
*
* RETURNS:
*   pointer to found entry
*   NULL if not found
*********************************************************************/
c_define_t *
    find_path_cdefine (dlq_hdr_t *cdefineQ,
                       obj_template_t *obj)
{
    c_define_t    *testcdef;

#ifdef DEBUG
    if (cdefineQ == NULL || obj == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    /* find the cdefineQ sorted by the object struct back-ptr */
    for (testcdef = (c_define_t *)dlq_firstEntry(cdefineQ);
         testcdef != NULL;
         testcdef = (c_define_t *)dlq_nextEntry(testcdef)) {

        if (testcdef->obj == obj) {
            return testcdef;
        }
    }

    return NULL;

}  /* find_path_cdefine */


/********************************************************************
* FUNCTION clean_cdefineQ
* 
* Clean a Q of c_define_t structs
*
* INPUTS:
*   cdefineQ == Q of c_define_t structs to use
*********************************************************************/
void
    clean_cdefineQ (dlq_hdr_t *cdefineQ)
{
    c_define_t    *cdef;

#ifdef DEBUG
    if (cdefineQ == NULL) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    while (!dlq_empty(cdefineQ)) {
        cdef = (c_define_t *)dlq_deque(cdefineQ);
        free_c_define(cdef);
    }

}  /* clean_cdefineQ */



/* END c_util.c */



