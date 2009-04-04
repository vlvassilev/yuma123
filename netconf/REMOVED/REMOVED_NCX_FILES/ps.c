/*  FILE: ps.c

		
*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
14oct05      abb      begun

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <parser.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_ps
#include "ps.h"
#endif

#ifndef _H_psd
#include "psd.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include "typ.h"
#endif

#ifndef _H_val
#include "val.h"
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


/********************************************************************
* FUNCTION redo_seqid
* 
* Renumber the sequence IDs for a specific parm number
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
static void
    redo_seqid (ps_parm_t *parm)
{
    ps_parm_t  *p;
    uint32     parmid, id;

    p = parm;
    parmid = parm->parm->parm_id;
    id = parm->seqid;
    while (p && p->parm->parm_id == parmid) {
	p->seqid = id++;
	p = (ps_parm_t *)dlq_nextEntry(p);
    }

}  /* redo_seqid */


/**************    E X T E R N A L   F U N C T I O N S **********/

/********************************************************************
* FUNCTION ps_new_parmset
* 
* Malloc and initialize the fields in a ps_parmset_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
ps_parmset_t * 
    ps_new_parmset (void)
{
    ps_parmset_t  *ps;

    ps = m__getObj(ps_parmset_t);
    if (!ps) {
	return NULL;
    }
    ps_init_parmset(ps);
    return ps;

}  /* ps_new_parmset */


/********************************************************************
* FUNCTION ps_init_parmset
* 
* Initialize the fields in a ps_parmset_t
*
* INPUTS:
*    ps == pointer to the struct to initialize
*********************************************************************/
void
    ps_init_parmset (ps_parmset_t *ps)
{
#ifdef DEBUG
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    (void)memset(ps, 0x0, sizeof(ps_parmset_t));
    dlq_createSQue(&ps->parmQ);
    ps->ntyp = NCX_NT_PARMSET;
    ps->nodetyp = PS_NT_PARMSET;

}  /* ps_init_parmset */


/********************************************************************
* FUNCTION ps_free_parmset
* 
* Scrub the memory in a ps_parmset_t by freeing all
* the sub-fields and then freeing the entire struct itself 
*
* INPUTS:
*    ps == ps_parmset_t data structure to free
*********************************************************************/
void 
    ps_free_parmset (ps_parmset_t *ps)
{
#ifdef DEBUG
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ps_clean_parmset(ps);
    m__free(ps);

}  /* ps_free_parmset */


/********************************************************************
* FUNCTION ps_clean_parmset
* 
* Scrub the memory in a ps_parmset_t by freeing all
* the sub-fields
*
* INPUTS:
*    ps == ps_parmset_t data structure to clean
*********************************************************************/
void 
    ps_clean_parmset (ps_parmset_t *ps)
{
    ps_parm_t   *parm;

#ifdef DEBUG
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (ps->name) {
	m__free(ps->name);
    }
    if (ps->instance) {
	m__free(ps->instance);
    }
    if (ps->pflags) {
	m__free(ps->pflags);
    }
    if (ps->lastchange) {
	m__free(ps->lastchange);
    }

    while (!dlq_empty(&ps->parmQ)) {
	parm = (ps_parm_t *)dlq_deque(&ps->parmQ);
	ps_free_parm(parm);
    }

    ps_init_parmset(ps);

}  /* ps_clean_parmset */


/********************************************************************
* FUNCTION ps_new_parm
* 
* Malloc and initialize the fields in a ps_parm_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
ps_parm_t * 
    ps_new_parm (void)
{
    ps_parm_t  *parm;

    parm = m__getObj(ps_parm_t);
    if (!parm) {
	return NULL;
    }
    ps_init_parm(parm);
    return parm;

}  /* ps_new_parm */


/********************************************************************
* FUNCTION ps_init_parm
* 
* Init the memory in a ps_parm_t
*
* INPUTS:
*    parm == ps_parm_t data structure to init
*********************************************************************/
void 
    ps_init_parm (ps_parm_t *parm)
{
#ifdef DEBUG
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    (void)memset(parm, 0x0, sizeof(ps_parm_t));
    parm->nodetyp = PS_NT_PARM;
    parm->val = val_new_value();

}  /* ps_init_parm */


/********************************************************************
* FUNCTION ps_free_parm
* 
* Scrub the memory in a ps_parm_t by freeing all
* the sub-fields and then freeing the entire struct itself 
*
* INPUTS:
*    parm == ps_parm_t data structure to free
*********************************************************************/
void 
    ps_free_parm (ps_parm_t *parm)
{
#ifdef DEBUG
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ps_clean_parm(parm);
    m__free(parm);
}  /* ps_free_parm */


/********************************************************************
* FUNCTION ps_clean_parm
* 
* Scrub the memory in a ps_parm_t by freeing all
* the sub-fields but DO NOT free the entire struct itself 
*
* INPUTS:
*    parm == ps_parm_t data structure to clean
*********************************************************************/
void 
    ps_clean_parm (ps_parm_t *parm)
{
#ifdef DEBUG
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (parm->val) {
	val_free_value(parm->val);
	parm->val = NULL;
    }
    parm->parm = NULL;
    parm->editop = OP_EDITOP_NONE;
    parm->seqid = 0;
    parm->parent = NULL;
    parm->res = NO_ERR;

    ps_init_parm(parm);

}  /* ps_clean_parm */


/********************************************************************
* FUNCTION ps_find_parm
* 
* Search the parmQ for a specified parameter name
* and return the first instance of the parm
* 
* INPUTS:
*   ps == parmset to search (ps->parmQ)
*   name == parm name to find
*
* RETURNS:
*   pointer to the first match if found, NULL if not found
*********************************************************************/
ps_parm_t * 
    ps_find_parm (ps_parmset_t *ps,
		  const xmlChar *name)
{
    ps_parm_t     *parm;

#ifdef DEBUG 
    if (!ps || !name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (ps_parm_t *)dlq_nextEntry(parm)) {
	if (!xml_strcmp(parm->parm->name, name)) {
	    return parm;
	}
    }
    return NULL;

}  /* ps_find_parm */


/********************************************************************
* FUNCTION ps_next_parm_instance
* 
* Start at the current parm and look for the next instance
* of the same pamameter
*
* INPUTS:
*   curparm == current parm
*
* RETURNS:
*   pointer to the next match if found, NULL if not found
*********************************************************************/
ps_parm_t * 
    ps_next_parm_instance (ps_parm_t *curparm)
{
    ps_parm_t     *parm;

#ifdef DEBUG 
    if (!curparm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (parm = (ps_parm_t *)dlq_nextEntry(curparm);
	 parm != NULL;
	 parm = (ps_parm_t *)dlq_nextEntry(parm)) {
	if (!xml_strcmp(parm->parm->name,
			curparm->parm->name)) {
	    return parm;
	}
    }
    return NULL;

}  /* ps_next_parm_instance */


/********************************************************************
* FUNCTION ps_parm_count
* 
* Search the parmQ for a specified parameter name
* and return the number of instances of the parm
* 
* INPUTS:
*   ps == parmset to search (ps->parmQ)
*   name == parm name to find
*
* RETURNS:
*   instance count
*********************************************************************/
uint32
    ps_parm_count (ps_parmset_t *ps,
		   const xmlChar *name)
{
    ps_parm_t     *parm;
    uint32         cnt;

#ifdef DEBUG 
    if (!ps || !name) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    cnt = 0;
    for (parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (ps_parm_t *)dlq_nextEntry(parm)) {
	if (!xml_strcmp(parm->parm->name, name)) {
	    cnt++;
	}
    }
    return cnt;

}  /* ps_parm_count */


/********************************************************************
* FUNCTION ps_match_parm
* 
* Search the parmQ for a specified parameter name fragment
* and return the first instance of the parm that matches
* the name string fir its first N chars
* 
* INPUTS:
*   ps == parmset to search (ps->parmQ)
*   namestr == parm name (or partial parm name) to find
*
* RETURNS:
*   pointer to the first match if found, NULL if not found
*********************************************************************/
ps_parm_t * 
    ps_match_parm (ps_parmset_t *ps,
		   const xmlChar *namestr)
{
    ps_parm_t     *parm;
    uint32         len;

#ifdef DEBUG 
    if (!ps || !namestr) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    len = xml_strlen(namestr);
    for (parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (ps_parm_t *)dlq_nextEntry(parm)) {
	if (!xml_strncmp(parm->parm->name, namestr, len)) {
	    return parm;
	}
    }
    return NULL;

}  /* ps_match_parm */


/********************************************************************
* FUNCTION ps_find_parmnum
* 
* Search the parmQ for a specified parameter ID
* and return the first instance of the parm
* 
* INPUTS:
*   ps == parmset to search (ps->parmQ)
*   id == parm number to find
*
* RETURNS:
*   pointer to the first match if found, NULL if not found
*********************************************************************/
ps_parm_t * 
    ps_find_parmnum (ps_parmset_t *ps,
		     uint32 id)
{
    ps_parm_t     *parm;

#ifdef DEBUG 
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (ps_parm_t *)dlq_nextEntry(parm)) {
	if (parm->parm->parm_id == id) {
	    return parm;
	}
    }
    return NULL;

}  /* ps_find_parmnum */


/********************************************************************
* FUNCTION ps_parmnum_set
* 
* Check the parmset pflags to see if a parm num has been set
* 
* INPUTS:
*   ps == parmset to check
*   id == parm number to check
*
* RETURNS:
*   TRUE if parm is set without errors
*   FALSE if not set of set with errors
*********************************************************************/
boolean
    ps_parmnum_set (const ps_parmset_t *ps,
		    uint32 id)
{
#ifdef DEBUG 
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (id==0 || id > ps->psd->parmcnt) {
	return FALSE;
    }

    if (!ps->pflags) {
	return FALSE;
    }

    return (ps->pflags[id-1] & PS_FL_SET) ? TRUE : FALSE;

}  /* ps_parmnum_set */


/********************************************************************
* FUNCTION ps_parmnum_mset
* 
* Check the parmset pflags to see if a parm num has been set
* with multiple instances
* 
* INPUTS:
*   ps == parmset to check
*   id == parm number to check
*
* RETURNS:
*   TRUE if parm is set multiple times
*   FALSE if not set of set multiple times
*********************************************************************/
boolean
    ps_parmnum_mset (const ps_parmset_t *ps,
		     uint32 id)
{
#ifdef DEBUG 
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (id==0 || id > ps->psd->parmcnt) {
	return FALSE;
    }

    if (!ps->pflags) {
	return FALSE;
    }

    return (ps->pflags[id-1] & PS_FL_MSET) ? TRUE : FALSE;

}  /* ps_parmnum_mset */


/********************************************************************
* FUNCTION ps_parmnum_seterr
* 
* Check the parmset pflags to see if a parm num has been set
* or if an error occurred whilte trying to set this parmnum
*
* INPUTS:
*   ps == parmset to check
*   id == parm number to check
*
* RETURNS:
*   TRUE if parm is set with or without errors
*   FALSE if not set at all
*********************************************************************/
boolean
    ps_parmnum_seterr (const ps_parmset_t *ps,
		       uint32 id)
{
#ifdef DEBUG 
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    if (id==0 || id > ps->psd->parmcnt) {
	return FALSE;
    }

    if (!ps->pflags) {
	return FALSE;
    }

    return (ps->pflags[id-1] & (PS_FL_SET | PS_FL_ERR)) ? TRUE : FALSE;

}  /* ps_parmnum_seterr */


/********************************************************************
* FUNCTION ps_find_parmcopy
* 
* Search the parmQ for a specified parameter name and value
* 
* INPUTS:
*   ps == parmset to search (ps->parmQ)
*   parm == parm copy (from PDU) to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
ps_parm_t * 
    ps_find_parmcopy (ps_parmset_t *ps,
		      const ps_parm_t *parm)
{

    ps_parm_t  *p;

#ifdef DEBUG 
    if (!ps || !parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (p = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 p != NULL;
	 p = (ps_parm_t *)dlq_nextEntry(p)) {
	if (!xml_strcmp(p->parm->name, parm->parm->name)) {
	    if (!p->val || !parm->val) {
		return p;
	    }
	    if (!val_compare(p->val, parm->val)) {
		return p;
	    }
	}
    }
    return NULL;

}  /* ps_find_parmcopy */


/********************************************************************
* FUNCTION ps_dump_parmset
* 
* Printf a ps_parmset_t struct contents to logfile or stdout
*
* INPUTS:
*    ps == ps_parmset_t data structure to dump
*    startindent == number of chars to indent this parmset
*********************************************************************/
void
    ps_dump_parmset (const ps_parmset_t *ps,
		     int32 startindent)
{
    const ps_parm_t  *parm;

#ifdef DEBUG 
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ncx_printf_indent(startindent);
    log_write("%s {", 
	      (ps->name) ? (const char *)ps->name :
	      (const char *)ps->psd->name);

    for (parm = (const ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (const ps_parm_t *)dlq_nextEntry(parm)) {
	val_dump_value(parm->val, 
		       (startindent >= 0) ? 
		       startindent+NCX_DEF_INDENT : startindent);
    }

    ncx_printf_indent(startindent);
    log_write("}");
    
}  /* ps_dump_parmset */


/********************************************************************
* FUNCTION ps_stdout_parmset
* 
* Printf a ps_parmset_t struct contents to stdout
*
* INPUTS:
*    ps == ps_parmset_t data structure to dump
*    startindent == number of chars to indent this parmset
*********************************************************************/
void
    ps_stdout_parmset (const ps_parmset_t *ps,
		       int32 startindent)
{
    const ps_parm_t  *parm;

#ifdef DEBUG 
    if (!ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    ncx_stdout_indent(startindent);
    log_stdout("%s {", 
	       (ps->name) ? (const char *)ps->name :
	       (const char *)ps->psd->name);

    for (parm = (const ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (const ps_parm_t *)dlq_nextEntry(parm)) {
	val_stdout_value(parm->val, 
			 (startindent >= 0) ? 
			 startindent+NCX_DEF_INDENT : startindent);
    }

    ncx_stdout_indent(startindent);
    log_stdout("}");
    
}  /* ps_stdout_parmset */


/********************************************************************
* FUNCTION ps_get_parmval
* 
* Search the parmQ for a specified parameter name and return a pointer
* to the value
* 
* INPUTS:
*   ps == parmset to search (ps->parmQ)
*   name == parm name to find
*   retval == pointer to receive the return value
*
* OUTPUTS:
*   *retval == the value struct (if NO_ERR return)
* RETURNS:
*   status
*********************************************************************/
status_t
    ps_get_parmval (ps_parmset_t *ps,
		    const xmlChar *name,
		    val_value_t **retval)
{
    ps_parm_t     *parm;

#ifdef DEBUG 
    if (!ps || !name || !retval) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    for (parm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (ps_parm_t *)dlq_nextEntry(parm)) {
	if (!xml_strcmp(parm->parm->name, name)) {
	    *retval = parm->val;
	    return NO_ERR;
	}
    }
    return ERR_NCX_NOT_FOUND;

}  /* ps_get_parmval */


/********************************************************************
* FUNCTION ps_clone_parm
* 
* Make a clone of the specified parm for insertion or backup
* 
* INPUTS:
*   parm == parm to clone
*
* RETURNS:
*   pointer to the cloned parm or NULL if malloc error
*********************************************************************/
ps_parm_t * 
    ps_clone_parm (const ps_parm_t *parm)
{

    ps_parm_t  *p;

#ifdef DEBUG 
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* inline copy of ps_new_parm and ps_init_parm to
     * avoid the call the vcal_new_value
     */
    p = m__getObj(ps_parm_t);
    if (!p) {
	return NULL;
    }
    (void)memset(p, 0x0, sizeof(ps_parm_t));
    p->nodetyp = PS_NT_PARM;

    /* clone the parm value */
    p->val = val_clone(parm->val);
    if (!p->val) {
	m__free(p);
	return NULL;
    }
    
    /* set the rest of the parm fields */
    p->parm = parm->parm;
    p->parent = parm->parent;

    return p;

}  /* ps_clone_parm */


/********************************************************************
* FUNCTION ps_merge_parm
* 
* Merge src parm into dest parm (! MUST be same type !)
* Any meta vars in src are also merged into dest
*
* INPUTS:
*    src == parm to merge from
*
*       !!! destructive -- entries will be moved, not copied !!!
*       !!! Must be dequeued before calling this function !!!
*       !!! Must not use src pointer value again if *freesrc == FALSE

*    dest == parm to merge into
*
* RETURNS:
*       TRUE if the source parm needs to be deleted because the
*          memory was not transfered to the parent val childQ.
*       FALSE if the source value should not be freed because 
*         the memory is still in use, but transferred to the target
*********************************************************************/
boolean
    ps_merge_parm (ps_parm_t *src,
		   ps_parm_t *dest)
{
    ncx_iqual_t      iqual;
    ncx_merge_t      mergetyp;
    boolean          dupsok;

#ifdef DEBUG
    if (!src || !dest) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return TRUE;
    }
#endif

    /* do not merge a virtual parameter !!! */
    if (val_is_virtual(dest->val)) {
	return TRUE;
    }

    /* check if the type allows multiple instances,
     * in which case a merge is really needed
     * Otherwise the current value will be replaced
     * unless it is a list or other multi-part data type
     */
    iqual = typ_get_iqualval_def(dest->val->typdef);
    switch (iqual) {
    case NCX_IQUAL_ONE:
    case NCX_IQUAL_OPT:
	return val_merge(src->val, dest->val);
    case NCX_IQUAL_1MORE:
    case NCX_IQUAL_ZMORE:
	/* need to add the additional value to the parent;
	 * do not free the source that is about to be inserted 
	 */
	mergetyp = typ_get_mergetype(dest->val->typdef);
	dupsok = val_duplicates_allowed(dest->val);

	if (!dupsok) {
	    if (ps_find_parmcopy(dest->parent, src)) {
		return TRUE;  /* src is a duplicate, so free it */
	    }
	}

	ps_add_parm(src, dest->parent, mergetyp);
	return FALSE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return TRUE;
    }
    /*NOTREACHED*/

}  /* ps_merge_parm */


/********************************************************************
* FUNCTION ps_merge_parmset
* 
* Merge src parmset into dest parmset (! MUST be same type !)
*
* INPUTS:
*    src == parmset to merge from
*
*       !!! destructive -- parm entries will be moved, not copied !!!
*
*    dest == parmset to merge into
* RETURNS:
*   TRUE if newps should be deleted after this call
*   FALSE if newps should not be deleted
*********************************************************************/
boolean
    ps_merge_parmset (ps_parmset_t *src,
		      ps_parmset_t *dest)
{
    ps_parm_t  *newparm, *curparm, *nextparm;

#ifdef DEBUG
    if (!src || !dest) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return TRUE;
    }
#endif

    /* go through the src parmset move all the parameters
     * to the dest parmset
     */
    for (newparm = (ps_parm_t *)dlq_firstEntry(&src->parmQ);
	 newparm != NULL; ) {

	nextparm = (ps_parm_t *)dlq_nextEntry(newparm);

	dlq_remove(newparm);

	/* check if new parm exists already */
	curparm = ps_find_parmnum(dest, newparm->parm->parm_id);
	if (!curparm) {
	    /* add the parm instead of merging it */
	    ps_add_parm(newparm, dest, NCX_MERGE_FIRST);
	} else {
	    /* merge newparm with curparm */
	    if (ps_merge_parm(newparm, curparm)) {
		ps_free_parm(newparm);
	    }
	}
	
	newparm = nextparm;
    }

    return TRUE;

}  /* ps_merge_parmset */


/********************************************************************
* FUNCTION ps_add_parm
* 
* Add a completed ps_parm_t struct to the parent parmset
*
* INPUTS:
*    parm == ps_parm_t data structure to add
*    ps   == ps_parmset to add parm to
*    mergetyp == requested merge method
*
*********************************************************************/
void 
    ps_add_parm (ps_parm_t *parm,
		 ps_parmset_t *ps,
		 ncx_merge_t  mergetyp)
{
    ps_parm_t *p, *first, *last;
    boolean    done;
    int32      ret;

#ifdef DEBUG
    if (!parm || !ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    parm->parent = ps;
    parm->seqid = 0;
    ps_mark_pflag_set(ps, parm->parm->parm_id);
    if (val_is_virtual(parm->val)) {
	ps->flags |= PS_FL_VIRTUAL;
    }

    first = NULL;
    done = FALSE;

    /* look for the first parm with a higher ID */
    for (p = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 p != NULL && !done;
	 p = (ps_parm_t *)dlq_nextEntry(p)) {
	if (p->parm->parm_id >= parm->parm->parm_id) {
	    first = p;
	    done = TRUE;
	}
    }

    /* check if this is going to be the first entry or
     * a new last entry with the highest parm ID
     */
    if (!first) {
	dlq_enque(parm, &ps->parmQ);
	return;
    }

    /* check if this is the first instance of this parm */
    if (first->parm->parm_id != parm->parm->parm_id) {
	dlq_insertAhead(parm, first);
	return;
    }

    /* first is the same parm ID, so use the merge type
     * to figure out where to put the new parm wrt/
     * all the instances with the same ID
     */
    switch (mergetyp) {
    case NCX_MERGE_FIRST:
	dlq_insertAhead(parm, first);
	break;
    case NCX_MERGE_LAST:
	last = p = first;
	while (p && p->parm->parm_id == parm->parm->parm_id) {
	    last = p;
	    p = (ps_parm_t *)dlq_nextEntry(p);
	}
	dlq_insertAfter(parm, last);
	break;
    case NCX_MERGE_SORT:
	last = p = first;
	while (p && p->parm->parm_id == parm->parm->parm_id) {
	    ret = val_compare(parm->val, p->val);
	    if (ret < 0) {
		dlq_insertAhead(parm, p);
		redo_seqid(first);
		return;
	    }		
	    last = p;
	    p = (ps_parm_t *)dlq_nextEntry(p);
	}
	dlq_insertAfter(parm, last);
	break;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    redo_seqid(first);

}  /* ps_add_parm */


/********************************************************************
* FUNCTION ps_add_parm_last
* 
* Add a completed ps_parm_t struct to the parent parmset
* Add as the new last entry, do not order the parms
* correctly like ps_add_parm
*
* Used for debug testing in ncxcli
*
* INPUTS:
*    parm == ps_parm_t data structure to add
*    ps   == ps_parmset to add parm to
*
*********************************************************************/
void 
    ps_add_parm_last (ps_parm_t *parm,
		      ps_parmset_t *ps)
{
#ifdef DEBUG
    if (!parm || !ps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    parm->parent = ps;
    parm->seqid = 0;

    if (val_is_virtual(parm->val)) {
	ps->flags |= PS_FL_VIRTUAL;
    }

    ps_mark_pflag_set(ps, parm->parm->parm_id);
    dlq_enque(parm, &ps->parmQ);

}  /* ps_add_parm_last */


/********************************************************************
* FUNCTION ps_set_instseq
* 
* Set the instseq field in each ps_parm_t struct
*
* INPUTS:
*   ps == parmset to set
*
*********************************************************************/
void
    ps_set_instseq (ps_parmset_t *ps)
{
    uint32  i, cnt, seqid;
    ps_parm_t  *psparm;
    psd_parm_t *parm;

    cnt = ps->psd->parmcnt;
    for (i=1; i <= cnt; i++) {
	if (ps_parmnum_mset(ps, i)) {
	    /* this parm was set more than once */
	    parm = psd_find_parmnum(ps->psd, i);
	    seqid = 1;

	    /* find all instances of the parm and set its sequence ID */
	    for (psparm = (ps_parm_t *)dlq_firstEntry(&ps->parmQ);
		 psparm != NULL;
		 psparm = (ps_parm_t *)dlq_nextEntry(psparm)) {
		if (psparm->parm == parm) {
		    psparm->seqid = seqid++;
		}
	    }
	}
    }

}  /* ps_set_instseq */


/********************************************************************
* FUNCTION ps_setup_parm
* 
* Setup an initialized ps_parm struct
*
* INPUTS:
*   ps_parm == parm instance struct to set up
*   ps == parmset containing this parm instance
*   psd_parm == parm definition template for this parameter
*
* OUTPUTS:
*   Some important *ps_parm fields are filled in
*
* RETURNS:
*   none
*********************************************************************/
void
    ps_setup_parm (ps_parm_t  *ps_parm,
		   ps_parmset_t *ps,
		   const psd_parm_t *psd_parm)
{
    ps_parm->parm = psd_parm;
    ps_parm->val->name = psd_parm->name;
    ps_parm->val->dname = NULL;
    ps_parm->parent = ps;
    ps_parm->val->parent = ps_parm;
    ps_parm->val->parent_typ = NCX_NT_PARM;
    
}  /* ps_setup_parm */


/********************************************************************
* FUNCTION ps_setup_parmset
* 
* Setup an initialized ps_parmset_t struct
*
* INPUTS:
*   ps == parmset to setup
*   psd == parmset definition template for this parmset
*   psdtyp == PSD classification
*
* OUTPUTS:
*   Some important *ps fields are filled in
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ps_setup_parmset (ps_parmset_t *ps,
		      const psd_template_t *psd,
		      psd_pstype_t psdtyp)
{
    ps->psd = psd;
    ps->psd_type = psdtyp;

    if (psd->parmcnt) {
	ps->pflags = m__getMem(psd->parmcnt);
	if (!ps->pflags) {
	    /* non-recoverable error */
	    return ERR_INTERNAL_MEM;
	} else {
	    memset(ps->pflags, 0x0, psd->parmcnt);
	}
    }
    return NO_ERR;
    
}  /* ps_setup_parmset */


/********************************************************************
* FUNCTION ps_check_block_set
* 
* Check if the choice block has any parameters set
*
* INPUTS:
*   ps == parmset
*   block == block to check
* RETURNS:
*   TRUE if and parms in the block set; FALSE if all not set
*********************************************************************/
boolean
    ps_check_block_set (const ps_parmset_t *ps,
			const psd_block_t  *block)
{
    psd_parmid_t      parmid;
    
    for (parmid = block->start_parm; 
	 parmid <= block->end_parm; parmid++) {
	if (ps_parmnum_set(ps, parmid)) {
	    return TRUE;
	}
    }
    return FALSE;
    
}  /* ps_check_block_set */


/********************************************************************
* FUNCTION ps_choice_first_set
* 
* Return the first set parm in the indicated choice or NULL if none
*
* INPUTS:
*   ps == parmset
*   pch == choice template from PSD
*
* RETURNS:
*   ps_parm_t * pointer if found or NULL if not found
*********************************************************************/
ps_parm_t *
    ps_choice_first_set (ps_parmset_t *ps,
			 const psd_choice_t *pch)
{
    psd_parmid_t     parmid;

    for (parmid = pch->start_parm; 
	 parmid <= pch->end_parm; parmid++) {
	if (ps_parmnum_set(ps, parmid)) {
	    return ps_find_parmnum(ps, parmid);
	}
    }
    return NULL;
    
}  /* ps_choice_first_set */


/********************************************************************
* FUNCTION ps_check_choice_set
* 
* Check if the choice has been completely set
* If some writable parms out of a block are missing
* then this will return FALSE
*
* INPUTS:
*   ps == parmset
*   id == choice ID to find
*
* RETURNS:
*   TRUE if the choice has been properly and completely set
*   FALSE if the choice has not been completely set
*********************************************************************/
boolean
    ps_check_choice_set (const ps_parmset_t *ps,
			 psd_choiceid_t  id)
{
    const ps_parm_t *parm;
    const psd_parm_t *parmdef;
    const psd_block_t *block;
    psd_parmid_t      parmid;
    
    /* first found parm in the specified choice will set the choice */
    for (parm = (const ps_parm_t *)dlq_firstEntry(&ps->parmQ);
	 parm != NULL;
	 parm = (const ps_parm_t *)dlq_nextEntry(parm)) {
	if (parm->parm->choice_id == id) {
	    /* check if this is a block or a single parm */
	    if (parm->parm->block_id) {
		block = psd_find_blocknum(ps->psd, id, 
					  parm->parm->block_id);
		if (!block) {
		    SET_ERROR(ERR_INTERNAL_VAL);
		    return FALSE;
		}
		for (parmid = block->start_parm; 
		     parmid <= block->end_parm; parmid++) {
		    parmdef = psd_find_parmnum(ps->psd, parmid);
		    if (!parmdef) {
			SET_ERROR(ERR_INTERNAL_VAL);
			return FALSE;
		    }
		    if (psd_parm_writable(parmdef)) {
			if (!ps_parmnum_set(ps, parmid)) {
			    /* partial block, choice not complete */
			    return FALSE;
			}
		    }
		}
		/* all parms in the choice are set */
		return TRUE;
	    } else {
		/* one choice is selected; don't know if it 
		 * is writable, may be an error or valid
		 */
		return TRUE;
	    }
	}
    }
    return FALSE;
    
}  /* ps_check_choice_set */


/********************************************************************
* FUNCTION ps_mark_pflag_set
* 
* Set the proper pflags field; mark a parm value as set
*
* INPUTS:
*   ps == ps_parmset_t struct in progress
*   id == psd_parm->parm_id being set
*
*********************************************************************/
void
    ps_mark_pflag_set (ps_parmset_t *ps,
		       uint32 id)
{
    if (id == 0 || id > ps->psd->parmcnt) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    if (ps->pflags[id-1] & PS_FL_SET) {
	ps->pflags[id-1] |= PS_FL_MSET;
    } else {
	ps->pflags[id-1] |= PS_FL_SET;
    }
}  /* ps_mark_pflag_set */


/********************************************************************
* FUNCTION ps_mark_pflag_err
* 
* Set the proper pflags field; mark a parm value that has errors
*
* INPUTS:
*   ps == ps_parmset_t struct in progress
*   id == psd_parm->parm_id with errors
*
*********************************************************************/
void
    ps_mark_pflag_err (ps_parmset_t *ps,
		       uint32 id)
{
    if (id == 0 || id > ps->psd->parmcnt) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    ps->pflags[id-1] |= PS_FL_ERR;

}  /* ps_mark_pflag_err */


/********************************************************************
* FUNCTION ps_replace_parms
* 
*   For all the parms in newps:
*      Delete any instances in oldps
*      Move the newps parm value to oldps
*   Existing parms in oldps not in newps will not be replaced
*
* INPUTS:
*   newps == ps_parmset_t with parms to move
*   oldps == ps_parmset_t with parms top replace
*
* OUTPUTS:
*   ps_parm_t entries may be moved from newps to oldps
*   The new_ps parmset should be freed after this operation
*
* RETUENS:
*   none
*********************************************************************/
void 
    ps_replace_parms (ps_parmset_t *newps,
		      ps_parmset_t *oldps)
{
    ps_parm_t *p1, *p2, *p3;
    boolean   done;
    uint32    id1, id2;

#ifdef DEBUG
    if (!newps || !oldps) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif


    for (p1 = (ps_parm_t *)dlq_firstEntry(&newps->parmQ);
	 p1 != NULL;  ) {

	id1 = p1->parm->parm_id;

	/* clean out all the current instances of this parm */
	done = FALSE;
	for (p2 = (ps_parm_t *)dlq_firstEntry(&oldps->parmQ);
	     p2 != NULL && !done; ) {
	    id2 = p2->parm->parm_id;

	    if (id1 == id2) {
		p3 = (ps_parm_t *)dlq_nextEntry(p2);
		dlq_remove(p2);
		ps_free_parm(p2);
		p2 = p3;
	    } else if (id1 < id2) {
		p2 = (ps_parm_t *)dlq_nextEntry(p2);
	    } else {
		done = TRUE;
	    }
	}
	oldps->pflags[id1-1] = 0;
	
	p2 = (ps_parm_t *)dlq_nextEntry(p1);
	dlq_remove(p1);
	ps_add_parm(p1, oldps, NCX_MERGE_LAST);
	p1 = p2;

    }

}  /* ps_replace_parms */


/********************************************************************
* FUNCTION ps_remove_parm
* 
* Remove the specified ps_parm_t from its parameter set
* Only call if the parm has been properly added to a parmset!!
*
* INPUTS:
*   parm == parm node to remove
*
*********************************************************************/
void
    ps_remove_parm (ps_parm_t *parm)
{
    ps_parmset_t  *ps;
    ps_parm_t     *pnext, *pnext2;
    uint32         parmslot;

    dlq_remove(parm);
    ps = parm->parent;
    parmslot = parm->parm->parm_id-1;

    /* clear the pflags for this parmnum */
    ps->pflags[parmslot] = 0;

    /* reset the flags that need to be set */
    pnext = ps_find_parmnum(ps, parmslot+1);
    if (pnext) {
	ps->pflags[parmslot] |= PS_FL_SET;

	pnext2 = (ps_parm_t *)dlq_nextEntry(pnext);
	if (pnext2 && pnext2->parm->parm_id==parmslot+1) {
	    ps->pflags[parmslot] |= PS_FL_MSET;
	}

	redo_seqid(pnext);
    }

} /* ps_remove_parm */


/********************************************************************
* FUNCTION ps_gen_parmset_instance_id
* 
* Malloc and Generate the instance ID string for a
* parmset node created by the agent instead of the manager
* 
* INPUTS:
*   ps == parmset node to generate the instance ID for
*
* SIDE EFFECTS:
*   ps->instance contains a malloced buffer with the instance ID
*   
* RETURNS:
*   status
*********************************************************************/
status_t
    ps_gen_parmset_instance_id (ps_parmset_t *ps)
{
    xmlChar            *str;
    uint32              len;


#ifdef DEBUG 
    if (!ps) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    if (ps->instance) {
	m__free(ps->instance);
	ps->instance = NULL;
    }

    /* brute force build the simple instance ID for the
     * virtual parmset.  [application.parmset]
     * The val_gen_instance_id function cannot be used because the vparmset
     * does not have a parent at the time the instance ID is generated
     */
    len = xml_strlen(ps->psd->app) + xml_strlen(ps->psd->name) + 1;

    /* get a buffer to fit the instance ID string */
    ps->instance = (xmlChar *)m__getMem(len+1);
    if (!ps->instance) {
	return ERR_INTERNAL_MEM;
    }

    /* build the instance ID */
    str = ps->instance;
    str += xml_strcpy(str, ps->psd->app);
    *str++ = '.';   /* C format instance ID */
    xml_strcpy(str, ps->psd->name);

    return NO_ERR;

}  /* ps_gen_parmset_instance_id */


/********************************************************************
* FUNCTION ps_is_vparmset
* 
* Return TRUE if the parmset has any virtual parms
* 
* INPUTS:
*   ps == parmset node to check
*
* RETURNS:
*   TRUE if vparmset, FALSE otherwise
*********************************************************************/
boolean
    ps_is_vparmset (const ps_parmset_t *ps)
{

    return (ps->flags & PS_FL_VIRTUAL) ? TRUE : FALSE;

} /* ps_is_vparmset */


/********************************************************************
* FUNCTION ps_replace_vparmset
* 
* Replace the contents of curps with newps, but preserve
* all the virtual parameters in curps
* 
* INPUTS:
*   newps == parmset node to replace with
*   curps == parmset node to replace
*
*********************************************************************/
void
    ps_replace_vparmset (ps_parmset_t *newps,
			 ps_parmset_t *curps)
{
    ps_parm_t  *newparm, *curparm, *nextparm;

    /* first go through and remove all the parameters
     * that are not virtual parameters
     */
    for (curparm = (ps_parm_t *)dlq_firstEntry(&curps->parmQ);
	 curparm != NULL; ) {
	nextparm = (ps_parm_t *)dlq_nextEntry(curparm);

	if (!val_is_virtual(curparm->val)) {
	    ps_remove_parm(curparm);
	}

	curparm = nextparm;
    }

    /* next go through the new parmset move all the parameters
     * that are not virtual parameters to the curps
     */
    for (newparm = (ps_parm_t *)dlq_firstEntry(&newps->parmQ);
	 newparm != NULL; ) {
	nextparm = (ps_parm_t *)dlq_nextEntry(newparm);

	dlq_remove(newparm);

	/* check if new parm is a virtual parm */
	curparm = ps_find_parmnum(curps, newparm->parm->parm_id);
	if (!curparm) {
	    /* not virtual, add as normal */
	    ps_add_parm(newparm, curps, NCX_MERGE_FIRST);
	} else {
	    /* is virtual, toss and continue */
	    ps_free_parm(newparm);
	}
	
	newparm = nextparm;
    }

} /* ps_replace_vparmset */


/********************************************************************
* FUNCTION ps_start_complex_parm
* 
* Create a parm struct and set it up
* This will not create child nodes, just the value struct
* within the ps_parm_t itself
*
* INPUTS:
*   ps == parmset to contain the parm instance
*   parmname == name of the parameter to create
*   res == address of return status
*
* OUTPUTS:
*   *res == return status
*
* RETURNS:
*   pointer to the malloced ps_parm_t, which has NOT been
*     added to the parmset yet, just allocated, setup and ready
*     to set the value, then call ps_add_parm
*   NULL if some error
*********************************************************************/
ps_parm_t *
    ps_start_complex_parm (ps_parmset_t  *ps,
			   const xmlChar *parmname,
			   status_t *res)
{
    ps_parm_t *newparm;
    const psd_parm_t *psd_parm;
    typ_template_t  *typ;

#ifdef DEBUG
    if (!ps || !parmname || !res) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* get the parm template */
    psd_parm = psd_find_parm(ps->psd, parmname);
    if (!psd_parm) {
	*res = ERR_NCX_DEF_NOT_FOUND;
	return NULL;
    }

    /* get the type template for the parm */
    typ = psd_get_parm_template(psd_parm);
    if (!typ) {
	*res = SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }

    /* allocate a new parm struct */
    newparm = ps_new_parm();
    if (!newparm) {
	*res = ERR_INTERNAL_MEM;
	return NULL;
    }

    /* setup the parm */
    ps_setup_parm(newparm, ps, psd_parm);

    /* setup the val_value_t struct in the parm */
    val_init_from_template(newparm->val, typ);

    *res = NO_ERR;
    return newparm;
    
}  /* ps_start_complex_parm */


/********************************************************************
* FUNCTION ps_add_simple_parm
* 
* Create a parm struct and set it up
* Then set the simple value 
*
* INPUTS:
*   ps == parmset to contain the parm instance
*   parmname == name of the parameter to create
*   strval == string value for the parm to be converted
*             to a val_value_t based on the type template
*             for 'parmname'
*
* SIDE EFFECTS:
*  new parm is added to the parmset if return is NO_ERR
*
* RETURNS:
*   status
*********************************************************************/
status_t
    ps_add_simple_parm (ps_parmset_t  *ps,
			const xmlChar *parmname,
			const xmlChar *strval)
{
    ps_parm_t         *newparm;
    const psd_parm_t  *psd_parm;
    typ_template_t    *typ;
    typ_def_t         *typdef;
    status_t           res;

#ifdef DEBUG
    if (!ps || !parmname) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    /* get the parm template */
    psd_parm = psd_find_parm(ps->psd, parmname);
    if (!psd_parm) {
	return ERR_NCX_DEF_NOT_FOUND;
    }

    /* get the type template for the parm */
    typ = psd_get_parm_template(psd_parm);
    if (!typ) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }

    typdef = &typ->typdef;

    /* allocate a new parm struct */
    newparm = ps_new_parm();
    if (!newparm) {
	return ERR_INTERNAL_MEM;
    }

    /* setup the parm */
    ps_setup_parm(newparm, ps, psd_parm);

    /* setup the val_value_t struct in the parm */
    val_init_from_template(newparm->val, typ);

    /* set the value */
    res = val_set_simval(newparm->val, typdef, 
			 psd_get_parm_nsid(psd_parm),
			 parmname, strval);
    if (res != NO_ERR) {
	ps_free_parm(newparm);
    } else {
	ps_add_parm(newparm, ps, NCX_MERGE_FIRST);
    }

    return res;
    
}  /* ps_add_simple_parm */


/********************************************************************
* FUNCTION ps_make_new_parmset
* 
* Create a parmset struct and set it up
* based on the owner and parmset name
*
* After this function, parmaeters may be added, then the parmset
* added to an application
*
* INPUTS:
*   module == module name of parmset definition (really module name)
*   psname == name of the parmset definition
*
* RETURNS:
*   malloced and initialized ps_parmset_t if no error,
*   NULL if some error
*********************************************************************/
ps_parmset_t *
    ps_make_new_parmset (const xmlChar *module,
			 const xmlChar *psname)
{
    const psd_template_t *psd;
    ps_parmset_t         *ps;
    ncx_node_t            dtyp;

#ifdef DEBUG
    if (!module || !psname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    dtyp = NCX_NT_PSD;
    psd = (psd_template_t *)def_reg_find_moddef(module, psname, &dtyp);
    if (!psd) {
	SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
	return NULL;
    }

    ps = ps_new_parmset();
    if (!ps) {
	return NULL;
    }

    ps_setup_parmset(ps, psd, psd->psd_type);
    return ps;

} /* ps_make_new_parmset */


/********************************************************************
* FUNCTION ps_make_new_parm
* 
* Create a parm struct and set it up
* based on the parmset and parm name
*
* After this function, the parm->val field should be set
* and then ps_add_parm called to add it to the parmset
*
* INPUTS:
*   ps == parmset that will get the a new parm
*   parmname == name of the parm definition
*
* RETURNS:
*   malloced and initialized ps_parm_t if no error,
*   NULL if some error
*********************************************************************/
ps_parm_t *
    ps_make_new_parm (ps_parmset_t *ps,
		      const xmlChar *parmname)
{
    ps_parm_t      *parm;
    psd_parm_t     *psd_parm;
    typ_template_t  *typ;

#ifdef DEBUG
    if (!ps || !parmname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    psd_parm = psd_find_parm(ps->psd, parmname);
    if (!psd_parm) {
	SET_ERROR(ERR_NCX_DEF_NOT_FOUND);
	return NULL;
    }

    parm = ps_new_parm();
    if (!parm) {
	return NULL;
    }

    ps_setup_parm(parm, ps, psd_parm);

    typ = psd_get_parm_template(psd_parm);
    
    val_init_from_template(parm->val, typ);

    return parm;

} /* ps_make_new_parm */


/********************************************************************
* FUNCTION ps_is_empty
* 
* Return TRUE if the parmset is empty
*        FALSE if any parms at all are present
*
* INPUTS:
*   ps == parmset node to check
*
* RETURNS:
*   TRUE if empty, FALSE otherwise
*********************************************************************/
boolean
    ps_is_empty (const ps_parmset_t *ps)
{

    return dlq_empty(&ps->parmQ);

} /* ps_is_empty */


/* END file ps.c */
