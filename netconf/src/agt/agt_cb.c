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

#ifndef _H_agt_rpc
#include "agt_rpc.h"
#endif

#ifndef _H_agt_rpcerr
#include "agt_rpcerr.h"
#endif

#ifndef _H_agt_util
#include "agt_util.h"
#endif

#ifndef _H_def_reg
#include "def_reg.h"
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

#ifndef _H_rpc
#include "rpc.h"
#endif

#ifndef _H_rpc_err
#include "rpc_err.h"
#endif

#ifndef _H_status
#include  "status.h"
#endif

#ifndef _H_typ
#include  "typ.h"
#endif

#ifndef _H_val
#include  "val.h"
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
* FUNCTION agt_cb_register_ps_callback
* 
* Register a parmset specific callback function
*
* INPUTS:
*   module == defname module name
*   defname == parmset name OR type name
*   forall == TRUE if this cbfn is for all values of cbtyp, and
*             the cbtyp parameter will be ignored.
*          == FALSE, then the cbtyp parameter will be used
*             and only that callback type will be set
*   cbtyp == agent parmset callback type
*   cbfn == address of callback function
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    agt_cb_register_ps_callback (const xmlChar *module,
				 const xmlChar *defname,
				 boolean forall,
				 agt_cbtyp_t cbtyp,
				 agt_cb_pscb_t    cbfn)
{
    psd_template_t   *psd;
    agt_cb_pscbset_t   *cbset;
    ncx_node_t         deftyp;
    agt_cbtyp_t    id;

#ifdef DEBUG
    if (!module || !defname || !cbfn) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!forall && (cbtyp > AGT_CB_ROLLBACK)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* find the PSD template for this callback */
    deftyp = NCX_NT_PSD;
    psd = def_reg_find_moddef(module, defname, &deftyp);
    if (!psd) {
	return ERR_NCX_UNKNOWN_PSD;
    }

    /* get the callback set or create a new one */
    cbset = (agt_cb_pscbset_t *)psd->cbset;
    if (!cbset) {
	cbset = m__getObj(agt_cb_pscbset_t);
	if (!cbset) {
	    return ERR_INTERNAL_MEM;
	}
	memset(cbset, 0x0, sizeof(agt_cb_pscbset_t));
	psd->cbset = cbset;
    }

    /* this may overwrite the old callback function, if any */
    if (forall) {
	for (id = AGT_CB_LOAD; id <= AGT_CB_ROLLBACK; id++) {
	    cbset->pscb[id] = cbfn;
	}
    } else {
	cbset->pscb[cbtyp] = cbfn;
    }

    return NO_ERR;

}  /* agt_cb_register_ps_callback */


/********************************************************************
* FUNCTION agt_cb_register_parm_callback
* 
* Register a parm-specific callback function
*
* INPUTS:
*   module == defname module name
*   psdname == parmset definition name
*   defname == parm name
*   forall == TRUE if this cbfn is for all values of cbtyp, and
*             the cbtyp parameter will be ignored.
*          == FALSE, then the cbtyp parameter will be used
*             and only that callback type will be set
*   cbtyp == agent parmset callback type
*   cbfn == address of callback function
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    agt_cb_register_parm_callback (const xmlChar *module,
				   const xmlChar *psdname,
				   const xmlChar *defname,
				   boolean forall,
				   agt_cbtyp_t cbtyp,
				   agt_cb_pcb_t    cbfn)
{
    psd_template_t    *psd;
    psd_parm_t        *parm;
    agt_cb_pcbset_t   *cbset;
    ncx_node_t         deftyp;
    agt_cbtyp_t    id;

#ifdef DEBUG
    if (!module || !psdname || !defname || !cbfn) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!forall && (cbtyp > AGT_CB_ROLLBACK)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* find the type template for this callback */
    deftyp = NCX_NT_PSD;
    psd = def_reg_find_moddef(module, psdname, &deftyp);
    if (!psd) {
	return ERR_NCX_UNKNOWN_PSD;
    }
    parm = psd_find_parm(psd, defname);
    if (!parm) {
	return ERR_NCX_UNKNOWN_PARM;
    }

    /* get the callback set or create a new one */
    cbset = (agt_cb_pcbset_t *)parm->cbset;
    if (!cbset) {
	cbset = m__getObj(agt_cb_pcbset_t);
	if (!cbset) {
	    return ERR_INTERNAL_MEM;
	}
	memset(cbset, 0x0, sizeof(agt_cb_pcbset_t));
	parm->cbset = cbset;
    }

    /* this may overwrite the old callback function, if any */
    if (forall) {
	for (id = AGT_CB_LOAD; id <= AGT_CB_ROLLBACK; id++) {
	    cbset->pcb[id] = cbfn;
	}
    } else {
	cbset->pcb[cbtyp] = cbfn;
    }

    return NO_ERR;

}  /* agt_cb_register_parm_callback */


/********************************************************************
* FUNCTION agt_cb_register_typ_callback
* 
* Register a type specific callback function
*
* INPUTS:
*   module == defname module name
*   defname == parmset name OR type name
*   forall == TRUE if this cbfn is for all values of cbtyp, and
*             the cbtyp parameter will be ignored.
*          == FALSE, then the cbtyp parameter will be used
*             and only that callback type will be set
*   cbtyp == agent parmset callback type
*   cbfn == address of callback function
*
* RETURNS:
*   status
*********************************************************************/
status_t 
    agt_cb_register_typ_callback (const xmlChar *module,
				  const xmlChar *defname,
				  boolean forall,
				  agt_cbtyp_t cbtyp,
				  agt_cb_tcb_t    cbfn)
{
    typ_template_t   *typ;
    agt_cb_tcbset_t   *tcbset;
    ncx_node_t         deftyp;
    agt_cbtyp_t    id;

#ifdef DEBUG
    if (!module || !defname || !cbfn) {
	return SET_ERROR(ERR_INTERNAL_PTR);
    }
    if (!forall && (cbtyp > AGT_CB_ROLLBACK)) {
	return SET_ERROR(ERR_INTERNAL_VAL);
    }
#endif

    /* find the type template for this callback */
    deftyp = NCX_NT_TYP;
    typ = def_reg_find_moddef(module, defname, &deftyp);
    if (!typ) {
	return ERR_NCX_UNKNOWN_TYPE;
    }

    /* get the callback set or create a new one */
    tcbset = (agt_cb_tcbset_t *)typ->typdef.cbset;
    if (!tcbset) {
	tcbset = m__getObj(agt_cb_tcbset_t);
	if (!tcbset) {
	    return ERR_INTERNAL_MEM;
	}
	memset(tcbset, 0x0, sizeof(agt_cb_tcbset_t));
	typ->typdef.cbset = tcbset;
    }

    /* this may overwrite the old callback function, if any */
    if (forall) {
	for (id = AGT_CB_LOAD; id <= AGT_CB_ROLLBACK; id++) {
	    tcbset->tcb[id] = cbfn;
	}
    } else {
	tcbset->tcb[cbtyp] = cbfn;
    }

    return NO_ERR;

}  /* agt_cb_register_typ_callback */


/********************************************************************
* FUNCTION agt_cb_unregister_ps_callback
* 
* Unregister all callback functions for a specific parmset
*
* INPUTS:
*   module == defname module name
*   defname == parmset name
*
* RETURNS:
*   none
*********************************************************************/
void
    agt_cb_unregister_ps_callback (const xmlChar *module,
				   const xmlChar *defname)
{
    psd_template_t   *psd;
    ncx_node_t        deftyp;

#ifdef DEBUG
    if (!module || !defname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* find the PSD or type template for this callback */
    deftyp =  NCX_NT_PSD;
    psd = def_reg_find_moddef(module, defname, &deftyp);
    if (!psd) {
	SET_ERROR(ERR_NCX_UNKNOWN_PSD);
	return;
    }
    if (psd->cbset) {
	m__free(psd->cbset);
	psd->cbset = NULL;
    }

}  /* agt_cb_unregister_ps_callback */


/********************************************************************
* FUNCTION agt_cb_unregister_parm_callback
* 
* Unregister all callback functions for a specific parm 
*
* INPUTS:
*   module == defname module name
*   psdname ==  parmset name
*   defname == parm name
*
* RETURNS:
*   none
*********************************************************************/
void
    agt_cb_unregister_parm_callback (const xmlChar *module,
				     const xmlChar *psdname,
				     const xmlChar *defname)
{
    psd_template_t   *psd;
    psd_parm_t       *parm;
    ncx_node_t         deftyp;

#ifdef DEBUG
    if (!module || !psdname || !defname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* find the PSD template first */
    deftyp =  NCX_NT_PSD;
    psd = def_reg_find_moddef(module, psdname, &deftyp);
    if (!psd) {
	SET_ERROR(ERR_NCX_UNKNOWN_PSD);
	return;
    }
    parm = psd_find_parm(psd, defname);
    if (!parm) {
	SET_ERROR(ERR_NCX_UNKNOWN_PARM);
	return;
    }

    if (parm->cbset) {
	m__free(parm->cbset);
	parm->cbset = NULL;
    }

}  /* agt_cb_unregister_parm_callback */


/********************************************************************
* FUNCTION agt_cb_unregister_typ_callback
* 
* Unregister all callback functions for a specific typedef 
*
* INPUTS:
*   module == defname module name
*   defname == type name
*
* RETURNS:
*   none
*********************************************************************/
void
    agt_cb_unregister_typ_callback (const xmlChar *module,
				    const xmlChar *defname)
{
    typ_template_t   *typ;
    ncx_node_t         deftyp;

#ifdef DEBUG
    if (!module || !defname) {
	SET_ERROR(ERR_INTERNAL_PTR);
	return;
    }
#endif

    /* unregister a typdef based callback */
    deftyp =  NCX_NT_TYP;
    typ = def_reg_find_moddef(module, defname, &deftyp);
    if (!typ) {
	SET_ERROR(ERR_NCX_UNKNOWN_TYPE);
	return;
    }
    if (typ->typdef.cbset) {
	m__free(typ->typdef.cbset);
	typ->typdef.cbset = NULL;
    }

}  /* agt_cb_unregister_typ_callback */


/* END file agt_cb.c */
