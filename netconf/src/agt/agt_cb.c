/*  FILE: agt_cb.c

   Manage Agent callbacks for data model manipulation

*********************************************************************
*                                                                   *
*                  C H A N G E   H I S T O R Y                      *
*                                                                   *
*********************************************************************

date         init     comment
----------------------------------------------------------------------
16apr07      abb      begun; split out from agt_ps.c
01aug08      abb      rewrite for YANG OBJ only data tree

*********************************************************************
*                                                                   *
*                     I N C L U D E    F I L E S                    *
*                                                                   *
*********************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include <memory.h>

#ifndef _H_procdefs
#include  "procdefs.h"
#endif

#ifndef _H_agt
#include "agt.h"
#endif

#ifndef _H_agt_cb
#include "agt_cb.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_obj
#include "obj.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_xpath
#include  "xpath.h"
#endif

/********************************************************************
*                                                                   *
*                       C O N S T A N T S                           *
*                                                                   *
********************************************************************/

#define AGT_CB_DEBUG 1

/********************************************************************
*                                                                   *
*                       V A R I A B L E S			    *
*                                                                   *
*********************************************************************/


/**************    E X T E R N A L   F U N C T I O N S **********/


/********************************************************************
* FUNCTION agt_cb_register_callback
* 
* Register an object specific callback function
*
* INPUTS:
*   defpath == Xpath absolute path to object
*   forall == TRUE if this cbfn is for all values of cbtyp, and
*             the cbtyp parameter will be ignored.
*          == FALSE, then the cbtyp parameter will be used
*             and only that callback type will be set
*   cbtyp == agent callback type
*   cbfn == address of callback function
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    agt_cb_register_callback (const xmlChar *defpath,
			      boolean forall,
			      agt_cbtyp_t cbtyp,
			      agt_cb_fn_t    cbfn)
{
    obj_template_t     *obj;
    agt_cb_fnset_t     *cbset;
    agt_cbtyp_t         id;
    status_t            res;

#ifdef DEBUG
    if (!defpath || !cbfn) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!forall && (cbtyp > AGT_CB_ROLLBACK)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* find the object template for this callback */
    res = xpath_find_schema_target_int(defpath, &obj);
    if (res != NO_ERR) {
	return res;
    }

    /* get the callback set or create a new one */
    cbset = (agt_cb_fnset_t *)obj->cbset;
    if (!cbset) {
	cbset = m__getObj(agt_cb_fnset_t);
	if (!cbset) {
	    return ERR_INTERNAL_MEM;
	}
	memset(cbset, 0x0, sizeof(agt_cb_fnset_t));
	obj->cbset = cbset;
    }

    /* this may overwrite the old callback function, if any */
    if (forall) {
	for (id = 0; id < AGT_NUM_CB; id++) {
	    cbset->cbfn[id] = cbfn;
	}
    } else {
	cbset->cbfn[cbtyp] = cbfn;
    }

    return NO_ERR;

}  /* agt_cb_register_callback */


/********************************************************************
* FUNCTION agt_cb_unregister_callback
* 
* Unregister all callback functions for a specific object
*
* INPUTS:
*   defpath == definition XPath location
*
* RETURNS:
*   none
*********************************************************************/
void
    agt_cb_unregister_callback (const xmlChar *defpath)
{
    obj_template_t   *obj;
    agt_cb_fnset_t   *cbset;
    status_t          res;

#ifdef DEBUG
    if (!defpath) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* find the object template for this callback */
    res = xpath_find_schema_target_int(defpath, &obj);
    if (res != NO_ERR) {
	SET_ERROR(ERR_INTERNAL_VAL);
	return;
    }

    /* get the callback set or create a new one */
    cbset = (agt_cb_fnset_t *)obj->cbset;
    if (cbset) {
	m__free(cbset);
	obj->cbset = NULL;
    }

}  /* agt_cb_unregister_callback */


/* END file agt_cb.c */
