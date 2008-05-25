/*  FILE: mon.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
14nov05      abb      begun

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

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_mon
#include "mon.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xml_util
#include "xml_util.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
*********************************************************************/


/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION mon_new_template
* 
* Malloc and initialize the fields in a mon_template_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
mon_template_t * 
mon_new_template (void)
{
    mon_template_t  *mon;

    mon = m__getObj(mon_template_t);
    if (!mon) {
	return NULL;
    }

    (void)memset(mon, 0x0, sizeof(mon_template_t));
    dlq_createSQue(&mon->objQ);

    return mon;

}  /* mon_new_template */



/********************************************************************
* FUNCTION mon_new_obj
* 
* Malloc and initialize the fields in a mon_obj_t
*
* INPUTS:
*   
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
mon_obj_t * 
mon_new_obj (void)
{
    mon_obj_t  *obj;

    obj = m__getObj(mon_obj_t);
    if (!obj) {
	return NULL;
    }
    (void)memset(obj, 0x0, sizeof(mon_obj_t));
    obj->ntyp = MON_NT_OBJ;
    return obj;

}  /* mon_new_obj */


/********************************************************************
* FUNCTION mon_new_choice
* 
* Malloc and initialize the fields in a mon_choice_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
mon_choice_t * 
mon_new_choice (void)
{
    mon_choice_t  *mch;

    mch = m__getObj(mon_choice_t);
    if (!mch) {
	return NULL;
    }
    (void)memset(mch, 0x0, sizeof(mon_choice_t));
    mch->ntyp = MON_NT_CHOICE;
    dlq_createSQue(&mch->choiceQ);
    return mch;

}  /* mon_new_choice */


/********************************************************************
* FUNCTION mon_new_block
* 
* Malloc and initialize the fields in a mon_block_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
mon_block_t * 
mon_new_block (void)
{
    mon_block_t  *mb;

    mb = m__getObj(mon_block_t);
    if (!mb) {
	return NULL;
    }
    (void)memset(mb, 0x0, sizeof(mon_block_t));
    mb->ntyp = MON_NT_BLOCK;
    dlq_createSQue(&mb->blockQ);
    return mb;

}  /* mon_new_block */


/********************************************************************
* FUNCTION mon_free_template
* 
* Scrub the memory in a mon_template_t by freeing all
* the sub-fields and then freeing the entire struct itself 
*
* INPUTS:
*    mon == mon_template_t data structure to free
*********************************************************************/
void 
mon_free_template (mon_template_t *mon)
{
    mon_hdronly_t  *mhdr;

    if (!mon) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    if (mon->name) {
	m__free(mon->name);
    }
    if (mon->descr) {
	m__free(mon->descr);
    }
    if (mon->condition) {
	m__free(mon->condition);
    }
    if (mon->montype) {
	m__free(mon->montype);
    }

    while (!dlq_empty(&mon->objQ)) {
        mhdr = (mon_hdronly_t *)dlq_deque(&mon->objQ);
        switch (mhdr->ntyp) {
        case MON_NT_CHOICE:
            mon_free_choice((mon_choice_t *)mhdr);
            break;
        case MON_NT_BLOCK:
            mon_free_block((mon_block_t *)mhdr);
            break;
        case MON_NT_OBJ:
            mon_free_obj((mon_obj_t *)mhdr);
            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
            m__free(mhdr);
        }
    }
    m__free(mon);

}  /* mon_free_template */


/********************************************************************
* FUNCTION mon_free_obj
* 
* Scrub the memory in a mon_obj_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    obj == mon_obj_t data structure to free
*********************************************************************/
void 
mon_free_obj (mon_obj_t *obj)
{
    if (!obj) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    if (obj->name) {
        m__free(obj->name);
    }
    if (obj->typname) {
        m__free(obj->typname);
    }
    if (obj->modstr) {
        m__free(obj->modstr);
    }
    if (obj->descr) {
        m__free(obj->descr);
    }
    if (obj->condition) {
        m__free(obj->condition);
    }
    m__free(obj);

}  /* mon_free_obj */


/********************************************************************
* FUNCTION mon_free_block
* 
* Scrub the memory in a mon_block_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    mb == mon_block_t data structure to free
*********************************************************************/
void 
mon_free_block (mon_block_t *mb)
{
    mon_obj_t *obj;

    if (!mb) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    while (!dlq_empty(&mb->blockQ)) {
        obj = (mon_obj_t *)dlq_deque(&mb->blockQ);
        mon_free_obj(obj);
    }
    m__free(mb);

}  /* mon_free_block */


/********************************************************************
* FUNCTION mon_free_choice
* 
* Scrub the memory in a mon_choice_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    mch== mon_choice_t data structure to free
*********************************************************************/
void 
mon_free_choice (mon_choice_t *mch)
{
    mon_hdronly_t *mhdr;

    if (!mch) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
    while (!dlq_empty(&mch->choiceQ)) {
        mhdr = (mon_hdronly_t *)dlq_deque(&mch->choiceQ);
        switch (mhdr->ntyp) {
        case MON_NT_OBJ:
            mon_free_obj((mon_obj_t *)mhdr);
            break;
        case MON_NT_BLOCK:
            mon_free_block((mon_block_t *)mhdr);
            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
            m__free(mhdr);
        }
    }
    m__free(mch);

}  /* mon_free_choice */


/********************************************************************
* FUNCTION mon_find_obj
* 
* Search the objQ for a specified parameter name
* 
* INPUTS:
*   mon == objsetdef to search (mon->objQ)
*   name == obj name to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
mon_obj_t * 
mon_find_obj (mon_template_t *mon,
	       const xmlChar *name)
{
    mon_hdronly_t *mhdr;
    mon_choice_t  *mch;
    mon_obj_t     *obj;

    if (!mon || !name) {
	return NULL;
    }

    for (mhdr = (mon_hdronly_t *)dlq_firstEntry(&mon->objQ);
         mhdr != NULL;
         mhdr = (mon_hdronly_t *)dlq_nextEntry(mhdr)) {
        switch (mhdr->ntyp) {
        case MON_NT_CHOICE:
            mch = (mon_choice_t *)mhdr;
            obj = mon_search_choice(mch, name);
            if (obj) {
                return obj;
            }
            break;
        case MON_NT_OBJ:
            obj = (mon_obj_t *)mhdr;
            if (!xml_strcmp(name, obj->name)) {
                return obj;
            }
            break;
        default: 
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;

}  /* mon_find_obj */


/********************************************************************
* FUNCTION mon_search_block
* 
* Search the mon_block_t for a specified parameter name
* Return the mon_obj_t that matches the name parameter
* INPUTS:
*   mb == mon_block_t to search
*   name == object name to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
mon_obj_t * 
mon_search_block (mon_block_t *mb,
                  const xmlChar *name)
{
    mon_obj_t    *obj;

    if (!mb || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    for (obj = (mon_obj_t *)dlq_firstEntry(&mb->blockQ);
         obj != NULL;
         obj = (mon_obj_t *)dlq_nextEntry(obj)) {
        if (!xml_strcmp(name, obj->name)) {
            return obj;
        }
    }
    return NULL;   /* not found */

}  /* mon_search_block */


/********************************************************************
* FUNCTION mon_search_choice
* 
* Search the mon_choice_t for a specified parameter name
* Return the mon_obj_t that matches the name parameter
* INPUTS:
*   mch == mon_choice_t to search
*   name == object name to find
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
mon_obj_t * 
mon_search_choice (mon_choice_t *mch,
                   const xmlChar *name)
{
    mon_hdronly_t *mhdr;
    mon_block_t   *mb;
    mon_obj_t     *obj;

    if (!mch || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }

    for (mhdr = (mon_hdronly_t *)dlq_firstEntry(&mch->choiceQ);
         mhdr != NULL;
         mhdr = (mon_hdronly_t *)dlq_nextEntry(mhdr)) {
        switch (mhdr->ntyp) {
        case MON_NT_BLOCK:
            mb = (mon_block_t *)mhdr;
            obj = mon_search_block(mb, name);
            if (obj) {
                return obj;
            }
            break;
        case MON_NT_OBJ:
            obj = (mon_obj_t *)mhdr;
            if (!xml_strcmp(name, obj->name)) {
                return obj;
            }
            break;
        default:
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* mon_search_choice */



/********************************************************************
* FUNCTION mon_locate_template
* 
* Search the current module, and then the module import path,
* for the mon_template_t struct for the specified MON name.
*
* INPUTS:
*     mod == ncx_module_t for the construct using this MON name
*     modstr == name of only module to use; NULL if not used
*     monname == name of monitor set to find
* OUTPUTS:
*    *pptr == pointer to the located template, if NO_ERR
* RETURNS:
*    status
*********************************************************************/
status_t
    mon_locate_template (ncx_module_t  *mod,
			 const xmlChar *modstr,
			 const xmlChar *monname,
			 mon_template_t  **pptr)
{
    mon_template_t  *mon;
    ncx_node_t       dtyp;

    if (!mod || !monname || !pptr) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }

    dtyp = NCX_NT_MON;
    if (!modstr) {
        /* First look in the mod->monQ we have so far 
         * It does not include the MON we are building, so
         * there is no need for a special check for that corner case
         */
        for (mon = (mon_template_t *)dlq_firstEntry(&mod->monQ);
             mon != NULL;
             mon = (mon_template_t *)dlq_nextEntry(mon)) {
            if (!xml_strcmp(mon->name, monname)) {
                *pptr = mon;
                return NO_ERR;
            }
        }

        /* MON name not found, now go through the imports list 
         * for any match in an items list; ask for MONs only
         */
        *pptr = (mon_template_t *)
	    ncx_locate_import(mod, monname, &dtyp);
    } else {
        *pptr = (mon_template_t *)
            ncx_locate_modqual_import(modstr, monname, &dtyp);

    }
    return *pptr ? NO_ERR : ERR_NCX_DEF_NOT_FOUND;

}  /* mon_locate_template */

/* END file mon.c */
