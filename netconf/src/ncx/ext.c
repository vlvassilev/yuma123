/*  FILE: ext.c

                
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
05jan08      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
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

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_tk
#include "tk.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif


/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                         V A R I A B L E S                         *
*                                                                   *
*********************************************************************/


/********************************************************************
* FUNCTION ext_new_template
* 
* Malloc and initialize the fields in a ext_template_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
ext_template_t * 
    ext_new_template (void)
{
    ext_template_t  *ext;

    ext = m__getObj(ext_template_t);
    if (!ext) {
        return NULL;
    }
    (void)memset(ext, 0x0, sizeof(ext_template_t));
    dlq_createSQue(&ext->appinfoQ);
    ext->status = NCX_STATUS_CURRENT;   /* default */
    return ext;

}  /* ext_new_template */


/********************************************************************
* FUNCTION ext_free_template
* 
* Scrub the memory in a ext_template_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    ext == ext_template_t data structure to free
*********************************************************************/
void 
    ext_free_template (ext_template_t *ext)
{
#ifdef DEBUG
    if (!ext) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    if (ext->name) {
        m__free(ext->name);
    }
    if (ext->descr) {
        m__free(ext->descr);
    }
    if (ext->ref) {
        m__free(ext->ref);
    }
    if (ext->arg) {
        m__free(ext->arg);
    }

    ncx_clean_appinfoQ(&ext->appinfoQ);

    m__free(ext);

}  /* ext_free_template */


/********************************************************************
* FUNCTION ext_clean_extensionQ
* 
* Clean a queue of ext_template_t structs
*
* INPUTS:
*    que == Q of ext_template_t data structures to free
*********************************************************************/
void 
    ext_clean_extensionQ (dlq_hdr_t *que)
{
    ext_template_t *ext;

#ifdef DEBUG
    if (!que) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return;
    }
#endif

    while (!dlq_empty(que)) {
        ext = (ext_template_t *)dlq_deque(que);
        ext_free_template(ext);
    }

}  /* ext_clean_groupingQ */


/********************************************************************
* FUNCTION ext_find_extension
* 
* Search a queue of ext_template_t structs for a given name
*
* INPUTS:
*    que == Q of ext_template_t data structures to search
*    name == name string to find
*
* RETURNS:
*   pointer to found entry, or NULL if not found
*********************************************************************/
ext_template_t *
    ext_find_extension (dlq_hdr_t *que,
                        const xmlChar *name)
{
    ext_template_t *ext;

#ifdef DEBUG
    if (!que || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return NULL;
    }
#endif

    for (ext = (ext_template_t *)dlq_firstEntry(que);
         ext != NULL;
         ext = (ext_template_t *)dlq_nextEntry(ext)) {

        if (!xml_strcmp(ext->name, name)) {
            return ext;
        }
    }
    return NULL;

}  /* ext_find_extension */


/* END ext.c */
