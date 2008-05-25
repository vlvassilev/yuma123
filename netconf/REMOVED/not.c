/*  FILE: not.c

		
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

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_dlq
#include "dlq.h"
#endif

#ifndef _H_ncx
#include "ncx.h"
#endif

#ifndef _H_ncxconst
#include "ncxconst.h"
#endif

#ifndef _H_not
#include "not.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include  "typ.h"
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
* FUNCTION not_new_template
* 
* Malloc and initialize the fields in a not_template_t
*
* RETURNS:
*   pointer to the malloced and initialized struct or NULL if an error
*********************************************************************/
not_template_t *
not_new_template (void)
{
    not_template_t  *not;

    not = m__getObj(not_template_t);
    if (!not) {
	return NULL;
    }

    (void)memset(not, 0x0, sizeof(not_template_t));
    dlq_createSQue(&not->not_dataQ);
    dlq_createSQue(&not->appinfoQ);
    return not;

}  /* not_new_template */


/********************************************************************
* FUNCTION not_free_template
* 
* Scrub the memory in a not_template_t by freeing all
* the sub-fields and then freeing the entire struct itself 
*
* MUST remove this struct from the mod->notifQ before calling
*
* INPUTS:
*    not == not_template_t data structure to free
*********************************************************************/
void 
not_free_template (not_template_t *not)
{
    ncx_appinfo_t  *appinfo;

    if (!not) {
        SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }

    if (not->name) {
        m__free(not->name);
    }
    if (not->not_class) {
        m__free(not->not_class);
    }
    if (not->not_type) {
        m__free(not->not_type);
    }
    if (not->descr) {
        m__free(not->descr);
    }
    if (not->condition) {
        m__free(not->condition);
    }

    while (!dlq_empty(&not->not_dataQ)) {
        ed = (typ_notdata_t *)dlq_deque(&not->not_dataQ);
        typ_free_notdata(ed);
    }

    while (!dlq_empty(&not->appinfoQ)) {
	appinfo = (ncx_appinfo_t *)dlq_deque(&not->appinfoQ);
	ncx_free_appinfo(appinfo);
    }

    m__free(not);

}  /* not_free_template */


/* END file not.c */
