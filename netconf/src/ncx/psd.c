/*  FILE: psd.c

		
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
#include  <stdio.h>
#include  <stdlib.h>
#include  <memory.h>

#include <xmlstring.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_help
#include "help.h"
#endif

#ifndef _H_log
#include "log.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_psd
#include "psd.h"
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

/* #define PSD_DEBUG 1 */


/********************************************************************
* FUNCTION dump_parm
* 
* Print some info for the specified parm
*
* INPUTS:
*   parm == parm def to dump
*   full == TRUE for full info, FALSE for partial info
*   indent == start indent count
*********************************************************************/
static void
    dump_parm (const psd_parm_t *parm,
	       boolean full,
	       uint32 indent)
{
    const char *s;
    const xmlChar *str;
    const typ_template_t *typ;
    uint32  i;

#ifdef DEBUG
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    help_write_lines(parm->name, indent, TRUE);

    typ = (const typ_template_t *)parm->pdef;
    if (typ) {
	log_stdout(" type:%s", parm->typname);
	s = tk_get_btype_sym(typ_get_basetype(&typ->typdef));
	if (s && xml_strcmp((const xmlChar *)s, parm->typname)) {
	    log_stdout(" (%s)", s);
	}

	if (parm->usage != PSD_UT_OPTIONAL) {
	    log_stdout(" mandatory");
	}

	str = typ_get_defval(typ);
	if (str) {
	    log_stdout(" default:%s", (const char *)str);
	}

	if (full) {
	    str = typ_get_units(typ);
	    if (str) {
		help_write_lines((const xmlChar *)"units: ", 
				 indent+2, TRUE);
		log_stdout("%s", (const char *)str);
	    }
	}
    }

    if (parm->descr) {
	if (full) {
	    help_write_lines(parm->descr, indent+2, TRUE);
	} else if (xml_strlen(parm->descr) > 40) {
	    for (i=0; i<indent+2; i++) {
		log_stdout(" ");
	    }
	    for (i=0; i<40; i++) {
		log_write("%c", parm->descr[i]);
	    }
	    log_write("...");
	} else {
	    help_write_lines(parm->descr, indent+2, TRUE);
	}
    }

    if (full) {
	str = ncx_get_data_class_str(parm->dataclass);
	if (str) {
	    if (xml_strcmp(str, (const xmlChar *)"config")) {
		log_stdout("\n    data class: %s", (const char *)str);
	    }
	}

	str = ncx_get_access_str(parm->access);
	if (str) {
	    if (!xml_strcmp(str, (const xmlChar *)"read-only")) {
		log_stdout(" {ro}");
	    }
	}
    }

    if (full || parm->descr) {
	log_stdout("\n");
    }

}  /* dump_parm */


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION psd_new_template
* 
* Malloc and initialize the fields in a psd_template_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
psd_template_t * 
    psd_new_template (void)
{
    psd_template_t  *psd;

    psd = m__getObj(psd_template_t);
    if (!psd) {
	return NULL;
    }

    (void)memset(psd, 0x0, sizeof(psd_template_t));
    dlq_createSQue(&psd->parmQ);
    dlq_createSQue(&psd->appinfoQ);
    return psd;

}  /* psd_new_template */


/********************************************************************
* FUNCTION psd_new_parm
* 
* Malloc and initialize the fields in a psd_parm_t
*
* INPUTS:
*   
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
psd_parm_t * 
    psd_new_parm (void)
{
    psd_parm_t  *parm;

    parm = m__getObj(psd_parm_t);
    if (!parm) {
	return NULL;
    }
    (void)memset(parm, 0x0, sizeof(psd_parm_t));
    parm->ntyp = PSD_NT_PARM;
    dlq_createSQue(&parm->appinfoQ);
    return parm;

}  /* psd_new_parm */


/********************************************************************
* FUNCTION psd_new_choice
* 
* Malloc and initialize the fields in a psd_choice_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
psd_choice_t * 
    psd_new_choice (void)
{
    psd_choice_t  *pch;

    pch = m__getObj(psd_choice_t);
    if (!pch) {
	return NULL;
    }
    (void)memset(pch, 0x0, sizeof(psd_choice_t));
    pch->ntyp = PSD_NT_CHOICE;
    dlq_createSQue(&pch->choiceQ);
    return pch;

}  /* psd_new_choice */


/********************************************************************
* FUNCTION psd_new_block
* 
* Malloc and initialize the fields in a psd_block_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
psd_block_t * 
    psd_new_block (void)
{
    psd_block_t  *pb;

    pb = m__getObj(psd_block_t);
    if (!pb) {
	return NULL;
    }
    (void)memset(pb, 0x0, sizeof(psd_block_t));
    pb->ntyp = PSD_NT_BLOCK;
    dlq_createSQue(&pb->blockQ);
    return pb;

}  /* psd_new_block */


/********************************************************************
* FUNCTION psd_free_template
* 
* Scrub the memory in a psd_template_t by freeing all
* the sub-fields and then freeing the entire struct itself 
*
* INPUTS:
*    psd == psd_template_t data structure to free
*********************************************************************/
void 
    psd_free_template (psd_template_t *psd)
{
    psd_hdronly_t  *phdr;
    ncx_appinfo_t  *appinfo;

#ifdef DEBUG
    if (!psd) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (psd->name) {
	m__free(psd->name);
    }
    if (psd->descr) {
	m__free(psd->descr);
    }
    if (psd->condition) {
	m__free(psd->condition);
    }
    if (psd->pstype) {
	m__free(psd->pstype);
    }
    if (psd->cbset) {
	m__free(psd->cbset);
    }

    while (!dlq_empty(&psd->parmQ)) {
        phdr = (psd_hdronly_t *)dlq_deque(&psd->parmQ);
        switch (phdr->ntyp) {
        case PSD_NT_CHOICE:
            psd_free_choice((psd_choice_t *)phdr);
            break;
        case PSD_NT_BLOCK:
            psd_free_block((psd_block_t *)phdr);
            break;
        case PSD_NT_PARM:
            psd_free_parm((psd_parm_t *)phdr);
            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
            m__free(phdr);
        }
    }

    while (!dlq_empty(&psd->appinfoQ)) {
	appinfo = (ncx_appinfo_t *)dlq_deque(&psd->appinfoQ);
	ncx_free_appinfo(appinfo);
    }

    m__free(psd);

}  /* psd_free_template */


/********************************************************************
* FUNCTION psd_free_parm
* 
* Scrub the memory in a psd_parm_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    parm == psd_parm_t data structure to free
*********************************************************************/
void 
    psd_free_parm (psd_parm_t *parm)
{
#ifdef DEBUG
    if (!parm) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    if (parm->name) {
	m__free(parm->name);
    }
    if (parm->modstr) {
	m__free(parm->modstr);
    }
    if (parm->typname) {
	m__free(parm->typname);
    }
    if (parm->descr) {
	m__free(parm->descr);
    }
    if (parm->condition) {
	m__free(parm->condition);
    }
    if (parm->cbset) {
	m__free(parm->cbset);
    }
    if (!dlq_empty(&parm->appinfoQ)) {

    }

    m__free(parm);

}  /* psd_free_parm */


/********************************************************************
* FUNCTION psd_free_block
* 
* Scrub the memory in a psd_block_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    pb == psd_block_t data structure to free
*********************************************************************/
void 
    psd_free_block (psd_block_t *pb)
{
    psd_parm_t *parm;

#ifdef DEBUG
    if (!pb) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(&pb->blockQ)) {
        parm = (psd_parm_t *)dlq_deque(&pb->blockQ);
        psd_free_parm(parm);
    }

    m__free(pb);

}  /* psd_free_block */


/********************************************************************
* FUNCTION psd_free_choice
* 
* Scrub the memory in a psd_choice_t by freeing all
* the sub-fields and then freeing the entire struct itself 
* The struct must be removed from any queue it is in before
* this function is called.
*
* INPUTS:
*    pch== psd_choice_t data structure to free
*********************************************************************/
void 
    psd_free_choice (psd_choice_t *pch)
{
    psd_hdronly_t *phdr;

#ifdef DEBUG
    if (!pch) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    while (!dlq_empty(&pch->choiceQ)) {
        phdr = (psd_hdronly_t *)dlq_deque(&pch->choiceQ);
        switch (phdr->ntyp) {
        case PSD_NT_PARM:
            psd_free_parm((psd_parm_t *)phdr);
            break;
        case PSD_NT_BLOCK:
            psd_free_block((psd_block_t *)phdr);
            break;
        default:
            SET_ERROR(ERR_INTERNAL_VAL);
            m__free(phdr);
        }
    }

    m__free(pch);

}  /* psd_free_choice */


/********************************************************************
* FUNCTION psd_find_parm
* 
* Search the parmQ for a specified parameter name
* 
* INPUTS:
*   psd == parmsetdef to search (psd->parmQ)
*   name == parm name to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_find_parm (const psd_template_t *psd,
		   const xmlChar *name)
{
    psd_hdronly_t *phdr;
    psd_choice_t  *pch;
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!psd || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&psd->parmQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_CHOICE:
            pch = (psd_choice_t *)phdr;
            parm = psd_search_choice(pch, name);
            if (parm) {
                return parm;
            }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
            if (!xml_strcmp(name, parm->name)) {
                return parm;
            }
            break;
        default: 
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* psd_find_parm */


/********************************************************************
* FUNCTION psd_first_parm
* 
* Search the parmQ for the first parameter
* 
* INPUTS:
*   psd == parmsetdef to search (psd->parmQ)
*
* RETURNS:
*   pointer to the first parm node if found, NULL if not found
*********************************************************************/
const psd_parm_t * 
    psd_first_parm (const psd_template_t *psd)
{
    const psd_hdronly_t *phdr, *phdr2;
    const psd_choice_t  *pch;
    const psd_block_t   *pb;
    const psd_parm_t    *parm;

#ifdef DEBUG
    if (!psd) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    phdr = (const psd_hdronly_t *)dlq_firstEntry(&psd->parmQ);
    if (phdr) {
        switch (phdr->ntyp) {
        case PSD_NT_CHOICE:
            pch = (const psd_choice_t *)phdr;
	    phdr2 = (const psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
	    if (phdr2) {
		switch (phdr2->ntyp) {
		case PSD_NT_BLOCK:
		    pb = (const psd_block_t *)phdr2;
		    parm = (const psd_parm_t *)dlq_firstEntry(&pb->blockQ);
		    if (parm) {
			return parm;
		    }
		    break;
		case PSD_NT_PARM:
		    parm = (const psd_parm_t *)phdr2;
		    return parm;
		default:
		    SET_ERROR(ERR_INTERNAL_VAL);
		    return NULL;
		}
	    }
	    break;
	case PSD_NT_PARM:
	    parm = (const psd_parm_t *)phdr;
	    return parm;
	default: 
	    SET_ERROR(ERR_INTERNAL_VAL);
	    return NULL;
	}
    }
    return NULL;

}  /* psd_first_parm */


/********************************************************************
* FUNCTION psd_next_parm
* 
* Search the parmQ for the first parameter
* 
* INPUTS:
*   parm == parameter to search
*
* RETURNS:
*   pointer to the next parm node if found, NULL if not found
*********************************************************************/
const psd_parm_t * 
    psd_next_parm (const psd_parm_t *parm)
{

#ifdef DEBUG
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const psd_parm_t *)
	psd_find_parmnum(parm->parent,
			 parm->parm_id+1);

}  /* psd_next_parm */


/********************************************************************
* FUNCTION psd_find_parm_str
* 
* Search the parmQ for a specified parameter name
* Uses a strncmp compare to avoid a buffer copy
* 
* INPUTS:
*   psd == parmsetdef to search (psd->parmQ)
*   name  == string containing the parm name to find
*   namelen == number of chars in the name
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_find_parm_str (const psd_template_t *psd,
		       const xmlChar *name,
		       uint32 namelen)
{
    psd_hdronly_t *phdr;
    psd_choice_t  *pch;
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!psd || !name || !namelen) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }

    if (!namelen) {
        SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&psd->parmQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_CHOICE:
            pch = (psd_choice_t *)phdr;
            parm = psd_search_choice_str(pch, name, namelen);
            if (parm) {
                return parm;
            }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
            if (xml_strlen(parm->name)==namelen &&
		!xml_strncmp(name, parm->name, namelen)) {
                return parm;
            }
            break;
        default: 
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* psd_find_parm_str */


/********************************************************************
* FUNCTION psd_match_parm_str
* 
* Search the parmQ for a specified parameter name
* Uses a strncmp compare to avoid a buffer copy
* 
* INPUTS:
*   psd == parmsetdef to search (psd->parmQ)
*   name  == string containing the parm name to find
*   namelen == number of chars in the name
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_match_parm_str (const psd_template_t *psd,
			const xmlChar *name,
			uint32 namelen)
{
    psd_hdronly_t *phdr;
    psd_choice_t  *pch;
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!psd || !name || !namelen) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }

    if (!namelen) {
        SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&psd->parmQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_CHOICE:
            pch = (psd_choice_t *)phdr;
            parm = psd_match_choice_str(pch, name, namelen);
            if (parm) {
                return parm;
            }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
            if (!xml_strncmp(name, parm->name, namelen)) {
                return parm;
            }
            break;
        default: 
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* psd_match_parm_str */


/********************************************************************
* FUNCTION psd_find_parmnum
* 
* Search the parmQ for a specified parameter number
* 
* INPUTS:
*   psd == parmsetdef to search (psd->parmQ)
*   parmnum == parm number to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_find_parmnum (const psd_template_t *psd,
		      psd_parmid_t parmnum)
{
    psd_hdronly_t *phdr;
    psd_choice_t  *pch;
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!psd) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&psd->parmQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_CHOICE:
            pch = (psd_choice_t *)phdr;
            parm = psd_search_choicenum(pch, parmnum);
            if (parm) {
                return parm;
            }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
            if (parmnum == parm->parm_id) {
                return parm;
            }
            break;
        default: 
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* psd_find_parmnum */


/********************************************************************
* FUNCTION psd_find_blocknum
* 
* Search the psd_choice_t for a specified block number
* Return the psd_block_t that matches the blocknum parameter
* INPUTS:
*   psd == psd_template_t to search
*   choicenum == choice number to find
*   blocknum == block number within that choice to find
*
* RETURNS:
*   pointer to the block if found, NULL if not found
*********************************************************************/
const psd_block_t * 
    psd_find_blocknum (const psd_template_t *psd,
		       psd_choiceid_t choicenum,
		       psd_blockid_t  blocknum)
{
    const psd_hdronly_t   *hdr, *hdr2;
    const psd_choice_t    *pch;
    const psd_block_t     *block;

#ifdef DEBUG
    if (!psd) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    /* make sure the IDs are valid */
    if (!choicenum || !blocknum || (choicenum > psd->choicecnt)) {
	return NULL;
    }

    /* should find a choice with the specified ID */
    for (hdr = (const psd_hdronly_t *)dlq_firstEntry(&psd->parmQ);
         hdr != NULL;
         hdr = (const psd_hdronly_t *)dlq_nextEntry(hdr)) {
	if (hdr->ntyp != PSD_NT_CHOICE) {
	    continue;
	}

	/* got a choice, check the ID */
	pch = (const psd_choice_t *)hdr;
	if (pch->choice_id != choicenum) {
	    continue;
	}

	/* got ther right choice ID, check if the block exists */
	if (blocknum > pch->blockcnt) {
	    return NULL;
	}

	/* find the block within this choice */
	for (hdr2 = (const psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
	     hdr2 != NULL;
	     hdr2 = (const psd_hdronly_t *)dlq_nextEntry(hdr2)) {
	    if (hdr2->ntyp == PSD_NT_BLOCK) {
		block = (const psd_block_t *)hdr2;
		if (block->block_id == blocknum) {
		    return block;
		}
	    }
	}
       
	/* if we get here the entry is not found */
	return NULL;
    }

    return NULL;   /* no entries in the parmQ at all */

}  /* psd_find_blocknum */


/********************************************************************
* FUNCTION psd_search_block
* 
* Search the psd_block_t for a specified parameter name
* Return the psd_parm_t that matches the name parameter
* INPUTS:
*   pb == psd_block_t to search
*   name == parm name to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_search_block (const psd_block_t *pb,
		      const xmlChar *name)
{
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!pb || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (parm = (psd_parm_t *)dlq_firstEntry(&pb->blockQ);
         parm != NULL;
         parm = (psd_parm_t *)dlq_nextEntry(parm)) {
        if (!xml_strcmp(name, parm->name)) {
            return parm;
        }
    }

    return NULL;   /* not found */

}  /* psd_search_block */


/********************************************************************
* FUNCTION psd_search_block_str
* 
* Search the psd_block_t for a specified parameter name
* Return the psd_parm_t that matches the name parameter
* INPUTS:
*   pb == psd_block_t to search
*   name == parm name to find
*   namelen == length of name string to compare
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_search_block_str (const psd_block_t *pb,
			  const xmlChar *name,
			  uint32 namelen)
{
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!pb || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (parm = (psd_parm_t *)dlq_firstEntry(&pb->blockQ);
         parm != NULL;
         parm = (psd_parm_t *)dlq_nextEntry(parm)) {
        if (xml_strlen(parm->name)==namelen &&
	    !xml_strncmp(name, parm->name, namelen)) {
            return parm;
        }
    }

    return NULL;   /* not found */

}  /* psd_search_block_str */


/********************************************************************
* FUNCTION psd_match_block_str
* 
* Search the psd_block_t for a specified parameter name
* Return the psd_parm_t that matches the name parameter
* INPUTS:
*   pb == psd_block_t to search
*   name == parm name to find
*   namelen == length of name string to compare
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_match_block_str (const psd_block_t *pb,
			 const xmlChar *name,
			 uint32 namelen)
{
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!pb || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (parm = (psd_parm_t *)dlq_firstEntry(&pb->blockQ);
         parm != NULL;
         parm = (psd_parm_t *)dlq_nextEntry(parm)) {
        if (!xml_strncmp(name, parm->name, namelen)) {
            return parm;
        }
    }

    return NULL;   /* not found */

}  /* psd_match_block_str */


/********************************************************************
* FUNCTION psd_search_blocknum
* 
* Search the psd_block_t for a specified parameter number
* Return the psd_parm_t that matches the parameter number
* INPUTS:
*   pb == psd_block_t to search
*   parmnum == parm number to find
*
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_search_blocknum (const psd_block_t *pb,
			 psd_parmid_t parmnum)
{
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!pb) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (parmnum < pb->start_parm || parmnum > pb->end_parm) {
	return NULL;
    }

    for (parm = (psd_parm_t *)dlq_firstEntry(&pb->blockQ);
         parm != NULL;
         parm = (psd_parm_t *)dlq_nextEntry(parm)) {
        if (parmnum == parm->parm_id) {
            return parm;
        }
    }

    return NULL;   /* not found */

}  /* psd_search_blocknum */


/********************************************************************
* FUNCTION psd_search_choice
* 
* Search the psd_choice_t for a specified parameter name
* Return the psd_parm_t that matches the name parameter
* INPUTS:
*   pch == psd_choice_t to search
*   name == parm name to find
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_search_choice (const psd_choice_t *pch,
		       const xmlChar *name)
{
    psd_hdronly_t *phdr;
    psd_block_t   *pb;
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!pch || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_BLOCK:
            pb = (psd_block_t *)phdr;
            parm = psd_search_block(pb, name);
            if (parm) {
                return parm;
            }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
            if (!xml_strcmp(name, parm->name)) {
                return parm;
            }
            break;
        default:
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* psd_search_choice */


/********************************************************************
* FUNCTION psd_search_choice_str
* 
* Search the psd_choice_t for a specified parameter name
* Return the psd_parm_t that matches the name parameter
* INPUTS:
*   pch == psd_choice_t to search
*   name == parm name to find
*   namelen == length of name
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_search_choice_str (const psd_choice_t *pch,
			   const xmlChar *name,
			   uint32 namelen)
{
    psd_hdronly_t *phdr;
    psd_block_t   *pb;
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!pch || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (!namelen) {
        SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_BLOCK:
            pb = (psd_block_t *)phdr;
            parm = psd_search_block_str(pb, name, namelen);
            if (parm) {
                return parm;
            }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
            if (xml_strlen(parm->name)==namelen &&
		!xml_strncmp(name, parm->name, namelen)) {
                return parm;
            }
            break;
        default:
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* psd_search_choice_str */


/********************************************************************
* FUNCTION psd_match_choice_str
* 
* Search the psd_choice_t for a specified parameter name
* Return the psd_parm_t that matches the name parameter
* INPUTS:
*   pch == psd_choice_t to search
*   name == parm name to find
*   namelen == length of name
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_match_choice_str (const psd_choice_t *pch,
			  const xmlChar *name,
			   uint32 namelen)
{
    psd_hdronly_t *phdr;
    psd_block_t   *pb;
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!pch || !name) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
    if (!namelen) {
        SET_ERROR(ERR_INTERNAL_VAL);
	return NULL;
    }
#endif

    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_BLOCK:
            pb = (psd_block_t *)phdr;
            parm = psd_match_block_str(pb, name, namelen);
            if (parm) {
                return parm;
            }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
            if (!xml_strncmp(name, parm->name, namelen)) {
                return parm;
            }
            break;
        default:
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* psd_match_choice_str */


/********************************************************************
* FUNCTION psd_search_choicenum
* 
* Search the psd_choice_t for a specified parameter number
* Return the psd_parm_t that matches the parameter number
* INPUTS:
*   pch == psd_choice_t to search
*   name == parm name to find
* RETURNS:
*   pointer to the node if found, NULL if not found
*********************************************************************/
psd_parm_t * 
    psd_search_choicenum (const psd_choice_t *pch,
			  psd_parmid_t parmnum)
{
    psd_hdronly_t *phdr;
    psd_block_t   *pb;
    psd_parm_t    *parm;

#ifdef DEBUG
    if (!pch) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    if (parmnum < pch->start_parm || parmnum > pch->end_parm) {
	return NULL;
    }

    for (phdr = (psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
         phdr != NULL;
         phdr = (psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_BLOCK:
            pb = (psd_block_t *)phdr;
            parm = psd_search_blocknum(pb, parmnum);
            if (parm) {
                return parm;
            }
            break;
        case PSD_NT_PARM:
            parm = (psd_parm_t *)phdr;
            if (parmnum == parm->parm_id) {
                return parm;
            }
            break;
        default:
            /* should be an error, just keep looking */
            break;
        }
    }
    return NULL;   /* not found */

}  /* psd_search_choicenum */


/********************************************************************
* FUNCTION psd_get_usage_str
* 
* Get the string name of a psd_usage_t enum
* 
* INPUTS:
*   usage == enum value
*
* RETURNS:
*   string value
*********************************************************************/
const xmlChar * 
    psd_get_usage_str (psd_usage_t usage)
{

    switch (usage) {
    case PSD_UT_NONE:        return (const xmlChar *) "none";
    case PSD_UT_MANDATORY:   return NCX_EL_USAGE_M;
    case PSD_UT_OPTIONAL:    return NCX_EL_USAGE_O;
    case PSD_UT_CONDITIONAL: return NCX_EL_USAGE_C;
    default:                 return (const xmlChar *) "illegal";
    }
    /*NOTREACHED*/

}  /* psd_get_usage_str */


/********************************************************************
* FUNCTION psd_get_usage_enum
* 
* Get the enum for the string name of a psd_usage_t enum
* 
* INPUTS:
*   str == string name of the enum value 
*
* RETURNS:
*   enum value
*********************************************************************/
psd_usage_t
    psd_get_usage_enum (const xmlChar *str)
{

#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return PSD_UT_NONE;
    }
#endif

    if (!xml_strcmp(NCX_EL_USAGE_M, str)) {
        return PSD_UT_MANDATORY;
    } else if (!xml_strcmp(NCX_EL_USAGE_O, str)) {
        return PSD_UT_OPTIONAL;
    } else if (!xml_strcmp(NCX_EL_USAGE_C, str)) {
        return PSD_UT_CONDITIONAL;
    } else {
        return PSD_UT_NONE;
    }
    /*NOTREACHED*/

}  /* psd_get_usage_enum */


/********************************************************************
* FUNCTION psd_get_order_enum
* 
* Get the enum for the string name of a psd_order_t enum
* 
* INPUTS:
*   str == string name of the enum value 
*
* RETURNS:
*   enum value
*********************************************************************/
psd_order_t
    psd_get_order_enum (const xmlChar *str)
{
#ifdef DEBUG
    if (!str) {
        SET_ERROR(ERR_INTERNAL_PTR);
        return PSD_OT_NONE;
    }
#endif

    if (!xml_strcmp(NCX_EL_ORDER_L, str)) {
        return PSD_OT_LOOSE;
    } else if (!xml_strcmp(NCX_EL_ORDER_S, str)) {
        return PSD_OT_STRICT;
    } else {
        return PSD_OT_NONE;
    }
    /*NOTREACHED*/

}  /* psd_get_order_enum */


/********************************************************************
* FUNCTION psd_locate_template
* 
* Search the current module, and then the module import path,
* for the psd_template_t struct for the specified PSD name.
*
* INPUTS:
*     mod == ncx_module_t for the construct using this PSD name
*     modstr == name of only module to use; NULL if not used
*     psdname == name of PSD to find
* OUTPUTS:
*    *pptr == pointer to the located template, if NO_ERR
* RETURNS:
*    status
*********************************************************************/
status_t
    psd_locate_template (const ncx_module_t  *mod,
			 const xmlChar *modstr,
			 const xmlChar *psdname,
			 psd_template_t  **pptr)
{
    psd_template_t  *psd;
    ncx_node_t       dtyp;

#ifdef DEBUG
    if (!mod || !psdname || !pptr) {
        return SET_ERROR(ERR_INTERNAL_PTR);
    }
#endif

    dtyp = NCX_NT_PSD;
    if (!modstr) {
        /* First look in the mod->psdQ we have so far 
         * It does not include the PSD we are building, so
         * there is no need for a special check for that corner case
         */
        for (psd = (psd_template_t *)dlq_firstEntry(&mod->psdQ);
             psd != NULL;
             psd = (psd_template_t *)dlq_nextEntry(psd)) {
            if (!xml_strcmp(psd->name, psdname)) {
                *pptr = psd;
                return NO_ERR;
            }
        }

        /* PSD name not found, now go through the imports list 
         * for any match in an items list; ask for PSDs only
         */
        *pptr = (psd_template_t *)
	    ncx_locate_import(mod, psdname, &dtyp);
    } else {
        *pptr = (psd_template_t *)
            ncx_locate_modqual_import(modstr, psdname, mod->diffmode, &dtyp);
    }
    return *pptr ? NO_ERR : ERR_NCX_DEF_NOT_FOUND;

}  /* psd_locate_template */


/********************************************************************
* FUNCTION psd_parm_writable
* 
* Make sure the specified parameter is not read-only
*
* INPUTS:
*   parm == psd_parm_t to check
*
* RETURNS:
*   TRUE if parm required; FALSE otherwise
*********************************************************************/
boolean
    psd_parm_writable (const psd_parm_t *parm)
{
#ifdef DEBUG
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    /* first check parameter class */
    switch (parm->dataclass) {
    case NCX_DC_NONE:
    case NCX_DC_CONFIG:
    case NCX_DC_TCONFIG:
	return TRUE;
    case NCX_DC_STATE:
	return FALSE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }

}  /* psd_parm_writable */


/********************************************************************
* FUNCTION psd_parm_required
* 
* Check if the specified parameter is required to be present
*
* INPUTS:
*   parm == psd_parm_t to check
*
* RETURNS:
*   TRUE if parm required; FALSE otherwise
*********************************************************************/
boolean
    psd_parm_required (const psd_parm_t *parm)
{
#ifdef DEBUG
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    /* first check parameter class */
    switch (parm->dataclass) {
    case NCX_DC_NONE:
    case NCX_DC_CONFIG:
    case NCX_DC_TCONFIG:
	break;
    case NCX_DC_STATE:
	return FALSE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }
		
    /* next check parameter usage */
    switch (parm->usage) {
    case PSD_UT_CONDITIONAL:
	if (!psd_condition_met(parm->condition)) {
	    return FALSE;
	}
	break;
    case PSD_UT_MANDATORY:
	break;
    case PSD_UT_OPTIONAL:
	return FALSE;
    default:
	SET_ERROR(ERR_INTERNAL_VAL);
	return FALSE;
    }


    /* if we get here, parameter usage is required
     * now check if the typdef allows zero instances
     */
    switch (typ_get_iqualval((const typ_template_t *)parm->pdef)) {
    case NCX_IQUAL_ONE:
    case NCX_IQUAL_1MORE:
	return TRUE;
    default:
	return FALSE;
    }

}  /* psd_parm_required */


/********************************************************************
* FUNCTION psd_block_required
* 
* Check if the specified parameter block is required to be present
*
* INPUTS:
*   block == psd_block_t to check
*
* RETURNS:
*   TRUE if parm required; FALSE otherwise
*********************************************************************/
boolean
    psd_block_required (const psd_block_t *block)
{
    const psd_parm_t *parm;

#ifdef DEBUG
    if (!block) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    for (parm = (psd_parm_t *)dlq_firstEntry(&block->blockQ);
	 parm != NULL;
	 parm = (psd_parm_t *)dlq_nextEntry(parm)) {

	if (psd_parm_required(parm)) {
	    return TRUE;
	}
    }
    return FALSE;

}  /* psd_block_required */


/********************************************************************
* FUNCTION psd_choice_required
* 
* Check if the specified choice is required to be present
* Choice is optional if it contains any optional writable parms
* Otherwise it is required
*
* INPUTS:
*   choice == psd_choice_t to check
*
* RETURNS:
*   TRUE if choice required; FALSE otherwise
*********************************************************************/
boolean
    psd_choice_required (const psd_choice_t *choice)
{
    const psd_hdronly_t *hdr;

#ifdef DEBUG
    if (!choice) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return FALSE;
    }
#endif

    for (hdr = (psd_hdronly_t *)dlq_firstEntry(&choice->choiceQ);
	 hdr != NULL;
	 hdr = (psd_hdronly_t *)dlq_nextEntry(hdr)) {
	switch (hdr->ntyp) {
	case PSD_NT_BLOCK:
	    if (!psd_block_required((const psd_block_t *)hdr)) {
		return FALSE;
	    }
	    break;
	case PSD_NT_PARM:
	    if (!psd_parm_required((const psd_parm_t *)hdr)) {
		return FALSE;
	    }
	    break;
	default:
	    SET_ERROR(ERR_INTERNAL_VAL);
	    break;
	}
    }
    return TRUE;

}  /* psd_choice_required */


/********************************************************************
* FUNCTION psd_first_choice_parm
* 
* Get the first parm in this choice for error reporting purposes
*
* INPUTS:
*   psd == PSD to check
*   choice == psd_choice_t to check
*
* RETURNS:
*   pointer to first parm or NULL if none
*********************************************************************/
const psd_parm_t *
    psd_first_choice_parm (const psd_template_t *psd,
			   const psd_choice_t *psd_choice)
{
#ifdef DEBUG
    if (!psd_choice) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }
#endif

    return (const psd_parm_t *)
	psd_find_parmnum(psd, psd_choice->start_parm);

}  /* psd_first_choice_parm */


/********************************************************************
* FUNCTION psd_parm_basetype
* 
* Get the parameter basetype
*
* INPUTS:
*   parm == psd_parm_t to check
*
* RETUENS:
*   base type of parm
*********************************************************************/
ncx_btype_t
    psd_parm_basetype (const psd_parm_t *parm)
{
    const typ_template_t *typ;

#ifdef DEBUG
    if (!parm) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NCX_BT_NONE;
    }
#endif

    typ = (const typ_template_t *)parm->pdef;

#ifdef DEBUG
    if (!typ) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NCX_BT_NONE;
    }
#endif

    return typ_get_basetype(&typ->typdef);

}  /* psd_parm_basetype */


/********************************************************************
* FUNCTION psd_condition_met  (!!!TBD!!!)
* 
* Check the condition string and decide if the 
* condition is TRUE or FALSE
*
* INPUTS:
*   condstr == condition string to check
*
* RETURNS:
*  TRUE if condition met; FALSE otherwise
*********************************************************************/
boolean
    psd_condition_met (const xmlChar *condstr)
{
    if (condstr) {
	return TRUE;   /****/
    } else {
	return TRUE;
    }

}  /* psd_condition_met */


/********************************************************************
* FUNCTION psd_parm_count
* 
* Return thenumber of parms defined
*
* INPUTS:
*   psd == parmset def to check
*
* RETURNS:
*   number of parms defined
*********************************************************************/
uint32
    psd_parm_count (const psd_template_t *psd)
{
#ifdef DEBUG
    if (!psd) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return 0;
    }
#endif

    return psd->parmcnt;

}  /* psd_parm_count */



/********************************************************************
* FUNCTION psd_dump_parms
* 
* Print the contents of the parm Q for a specified PSD
*
* INPUTS:
*   psd == parmset def to dump
*   full == TRUE for full info, FALSE for partial info
*********************************************************************/
void
    psd_dump_parms (const psd_template_t *psd,
		    boolean full,
		    uint32 indent)
{
    const psd_hdronly_t *phdr, *phdr2;
    const psd_choice_t  *pch;
    const psd_block_t   *pb;
    const psd_parm_t    *parm;
    boolean              anyout;

#ifdef DEBUG
    if (!psd) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    anyout = FALSE;
    for (phdr = (const psd_hdronly_t *)dlq_firstEntry(&psd->parmQ);
         phdr != NULL;
         phdr = (const psd_hdronly_t *)dlq_nextEntry(phdr)) {
        switch (phdr->ntyp) {
        case PSD_NT_CHOICE:
            pch = (const psd_choice_t *)phdr;
	    help_write_lines((const xmlChar *)"<choice>",
			     indent, TRUE);

	    for (phdr2 = (const psd_hdronly_t *)dlq_firstEntry(&pch->choiceQ);
		 phdr2 != NULL;
		 phdr2 = (const psd_hdronly_t *)dlq_nextEntry(phdr2)) {
		switch (phdr2->ntyp) {
		case PSD_NT_BLOCK:
		    help_write_lines((const xmlChar *)"<block>",
				     indent+2, TRUE);
		    pb = (const psd_block_t *)phdr2;
		    for (parm = (const psd_parm_t *)
			     dlq_firstEntry(&pb->blockQ);
			 parm != NULL;
			 parm = (const psd_parm_t *)dlq_nextEntry(parm)) {
			dump_parm(parm, full, indent+4);
		    }
		    help_write_lines((const xmlChar *)"</block>\n",
				     indent+2, TRUE);
		    break;
		case PSD_NT_PARM:
		    parm = (const psd_parm_t *)phdr2;
		    dump_parm(parm, full, indent+2);
		    break;
		default:
		    SET_ERROR(ERR_INTERNAL_VAL);
		    break;
		}
	    }
	    help_write_lines((const xmlChar *)"</choice>\n",
			     indent, TRUE);
            break;
        case PSD_NT_PARM:
            parm = (const psd_parm_t *)phdr;
	    dump_parm(parm, full, indent);
            break;
        default: 
	    SET_ERROR(ERR_INTERNAL_VAL);
            break;
        }
	anyout = TRUE;
    }
    if (anyout) {
	log_stdout("\n");
    }

}  /* psd_dump_parms */


/********************************************************************
* FUNCTION psd_get_typdef
* 
* Get the typdef for the specified parm type
*
* INPUTS:
*   parm == parm to check
*
* RETURNS:
*   pointer to typdef for this parameter or NULL if internal error
*********************************************************************/
typ_def_t *
    psd_get_typdef (const psd_parm_t *parm)
{
    typ_template_t *typ;

    typ = (typ_template_t *)parm->pdef;
    if (typ) {
	return &typ->typdef;
    } else {
	return NULL;
    }

} /* psd_get_typdef */


/********************************************************************
* FUNCTION psd_get_parm_nsid
* 
* Get the namespace ID for the specified parm
*
* INPUTS:
*   parm == parm to check
*
* RETURNS:
*    namespace ID for this parm
*********************************************************************/
xmlns_id_t 
    psd_get_parm_nsid (const psd_parm_t *parm)
{

    if (parm->parent) {
	return parm->parent->nsid;
    } else {
	SET_ERROR(ERR_INTERNAL_VAL);
	return 0;
    }

} /* psd_get_parm_nsid */


/********************************************************************
* FUNCTION psd_get_parmset_nsid
* 
* Get the namespace ID for the specified parmset
*
* INPUTS:
*   psd == parmset to check
*
* RETURNS:
*    namespace ID for this parmset
*********************************************************************/
xmlns_id_t 
    psd_get_parmset_nsid (const psd_template_t *psd)
{

    return psd->nsid;

} /* psd_get_parmset_nsid */


/********************************************************************
* FUNCTION psd_get_parm_template
* 
* Get the typ_template_t for the specified parm
*
* INPUTS:
*   parm == parm to check
*
* RETURNS:
*    pointer to the type template
*********************************************************************/
typ_template_t *
    psd_get_parm_template (const psd_parm_t *parm)
{
    if (parm) {
	return (typ_template_t *)parm->pdef;
    } else {
	SET_ERROR(ERR_INTERNAL_PTR);
	return NULL;
    }

} /* psd_get_parm_template */


/* END file psd.c */
